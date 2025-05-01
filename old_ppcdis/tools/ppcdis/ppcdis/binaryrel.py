"""
Binary reader for REL files
"""

from dataclasses import dataclass
from enum import IntEnum, unique
from struct import pack
from typing import Dict, List, Tuple

from .binarybase import BinaryReader, BinarySection, SectionDef, SectionType
from .binarydol import DolReader
from .fileutil import load_from_yaml_str

@unique
class RelOffs(IntEnum):
    """Offsets of fields in the rel header"""

    MODULE_ID = 0x0
    # next = 0x4
    # prev = 0x8
    NUM_SECTIONS = 0xc
    SECTIONS_OFFSET = 0x10
    NAME_OFFSET = 0x14
    NAME_SIZE = 0x18
    VERSION_OFFSET = 0x1c
    BSS_SIZE = 0x20
    REL_OFFSET = 0x24
    IMP_OFFSET = 0x28
    IMP_SIZE = 0x2c
    PROLOG_SECTION = 0x30
    EPILOG_SECTION = 0x31
    UNRESOLVED_SECTION = 0x32
    BSS_SECTION = 0x33
    PROLOG = 0x34
    EPILOG = 0x38
    UNRESOLVED = 0x3c
    ALIGN = 0x40
    BSS_ALIGN = 0x44
    FIX_SIZE = 0x48

class RelSize(IntEnum):
    """Sizes of structs in the rel"""

    SECTION_ENTRY = 8
    IMP_ENTRY = 8
    RELOC_ENTRY = 8

@unique
class RelType(IntEnum):
    """Types of RelReloc"""

    ADDR32 = 1
    ADDR16_LO = 4
    ADDR16_HA = 6
    REL24 = 10
    REL14 = 11
    RVL_NONE = 201
    RVL_SECT = 202
    RVL_STOP = 203

@dataclass(eq=True)
class RelReloc:
    """Container for one relocation"""

    target_module: int
    offset: int
    t: RelType
    section: int
    addend: int
    write_addr: int = None

    def to_binary(self, relative_offset: int) -> bytearray:
        """Gets the binary representation of the relocation"""

        return RelReloc.quick_binary(relative_offset, self.t, self.section, self.addend)

    def quick_binary(relative_offset: int, t: int, section: int, addend: int) -> bytearray:
        """Gets the binary representation of a relocation"""

        return bytearray(pack(">HBBI", relative_offset, t, section, addend))

class RelBinarySection(BinarySection):
    """Custom BinarySection that tracks its index in the rel header"""

    def __init__(self, rel_idx: int, *parent_args):
        super().__init__(*parent_args)
        self.rel_idx = rel_idx

default_section_defs = load_from_yaml_str("""
text:
  - name: .text
data:
  - name: .ctors
    balign: 0
  - name: .dtors
    balign: 0
  - name: .rodata
  - name: .data
bss:
  - name: .bss
""")

class RelReader(BinaryReader):
    def __init__(self, dol: DolReader, path: str, base_addr: int, bss_addr: int,
                 section_defs: Dict, func_prefix: str, label_prefix: str, data_prefix: str):
        # Handle params
        self.dol = dol
        self._base_addr = base_addr
        self._bss_addr = bss_addr
        self._section_defs_raw = section_defs
        if section_defs is None:
            section_defs = default_section_defs
        self._section_defs = [
            SectionDef.parse(section_defs["text"]),
            SectionDef.parse(section_defs["data"]),
            SectionDef.parse(section_defs["bss"])
        ]

        # Keep an internal map of the sections including the empty ones (for reloc indices)
        self._rel_sections: List[RelBinarySection] = []

        # Call parent constructor
        super().__init__(path, func_prefix, label_prefix, data_prefix)

        # Read relocs
        self._read_relocs()

        # Read module id
        self.module_id = self.read_word(RelOffs.MODULE_ID, True)

        # Save external rels by id
        self._rels: Dict[int, RelReader] = {self.module_id : self}
        
    def _read_relocs(self):
        """Reads the relocation data into _relocs"""

        # Init dict and list
        self.addr_relocs: Dict[int, RelReloc] = {} # Map by target address
        self.ordered_relocs: List[RelReloc] = [] # List in original order

        # Iterate over all imps
        imp_offs = self.read_word(RelOffs.IMP_OFFSET, True)
        imp_size = self.read_word(RelOffs.IMP_SIZE, True)
        for imp in range(imp_offs, imp_offs + imp_size, RelSize.IMP_ENTRY):
            # Read imp
            module = self.read_word(imp, True)
            rel_offs = self.read_word(imp + 4, True)

            # Iterate over relocs in imp
            write_sec: BinarySection = None
            write_offs = 0
            while True:
                # Parse reloc
                try:
                    rel = RelReloc(
                        module,
                        self.read_half(rel_offs + 0, True),
                        RelType(self.read_byte(rel_offs + 2, True)),
                        self.read_byte(rel_offs + 3, True),
                        self.read_word(rel_offs + 4, True)
                    )
                except ValueError:
                    assert 0, f"Unsupported relocation type {self.read_byte(rel_offs + 2, True)}"

                self.ordered_relocs.append(rel)

                # Apply offset
                write_offs += rel.offset
                
                # Handle reloc
                if rel.t == RelType.RVL_SECT:
                    # Change section and reset offset
                    write_sec = self._rel_sections[rel.section]
                    write_offs = 0
                elif rel.t == RelType.RVL_STOP:
                    # Stop reading this reloc list
                    break
                elif rel.t != RelType.RVL_NONE:
                    # Save reloc for later
                    write_addr = write_sec.addr + write_offs
                    self.addr_relocs[write_addr] = rel
                    rel.write_addr = write_addr
                
                # Move to next relocc
                rel_offs += RelSize.RELOC_ENTRY

    def read(self, addr: int, size: int, is_offset=False) -> bytes:
        """Override to apply relocations to reads"""

        # Get raw data
        dat = super().read(addr, size, is_offset)

        # Don't apply relocations to internal reads
        if is_offset:
            return dat

        # Apply all relocations within data
        i = 0
        ret = bytearray()
        while i < len(dat):
            # Apply relocation if found
            if addr + i in self.addr_relocs:
                # Get reloc
                rel = self.addr_relocs[addr + i]

                target = self.get_reloc_target(rel)

                if rel.t == RelType.ADDR32:
                    # Replace with address
                    ret.extend(int.to_bytes(target, 4, 'big'))
                    skip = 4
                elif rel.t == RelType.ADDR16_LO:
                    # Replace with address@l
                    ret.extend(int.to_bytes(target & 0xffff, 2, 'big'))
                    skip = 2
                elif rel.t == RelType.ADDR16_HA:
                    # Replace with address@ha
                    upper = (target >> 16) & 0xffff
                    if target & 0x8000:
                        upper += 1
                    ret.extend(int.to_bytes(upper, 2, 'big'))
                    skip = 2
                elif rel.t == RelType.REL24:
                    # Insert delta
                    delta_mask = 0x3ff_fffc
                    delta = (target - (addr + i)) & delta_mask
                    val = int.from_bytes(dat[i:i+4], 'big') & ~delta_mask
                    ret.extend(int.to_bytes(val | delta, 4, 'big'))
                    skip = 4
                elif rel.t == RelType.REL14:
                    # Insert delta
                    delta_mask = 0xfffc
                    delta = (target - (addr + i)) & delta_mask
                    val = int.from_bytes(dat[i:i+4], 'big') & ~delta_mask
                    ret.extend(int.to_bytes(val | delta, 4, 'big'))
                    skip = 4
                else:
                    assert 0, f"Unsupported relocation made it into _relocs {addr + i:x} {rel}"

                assert len(dat) - i >= skip, \
                    f"Relocation at {addr+i:x} cut off by read boundaries {addr:x}-{addr+size:x}"
                i += skip

            else:
                # No relocation, just copy byte
                ret.append(dat[i])
                i += 1

        return ret

    def _get_sections(self) -> List[BinarySection]:
        """Finds the sections in a binary"""

        # Get section definitions
        text_defs, data_defs, bss_defs = self._section_defs
        assert len(bss_defs) <= 1, "Rel only supports 1 bss section"
        bss_def = bss_defs[0]
         
        # Read header
        bss_size = self.read_word(RelOffs.BSS_SIZE, True)
        section_count = self.read_word(RelOffs.NUM_SECTIONS, True)
        sections_offset = self.read_word(RelOffs.SECTIONS_OFFSET, True)

        # Iterate over sections
        ret = []
        text_n = 0
        data_n = 0
        for i, offs in enumerate(range(
            sections_offset,
            sections_offset + RelSize.SECTION_ENTRY * section_count,
            RelSize.SECTION_ENTRY
        )):
            # Read entry
            sec_offs = self.read_word(offs, True)
            sec_size = self.read_word(offs + 4, True)

            if sec_offs == 0:
                # Empty section
                if sec_size == 0:
                    # Record only in internal table for indices in relocs
                    self._rel_sections.append(None)
                    continue

                # BSS section
                sec = RelBinarySection(i, ".bss", SectionType.BSS, 0, self._bss_addr, bss_size,
                                       bss_def.attr, bss_def.nobits, bss_def.balign)
                ret.append(sec)
                self._rel_sections.append(sec)
            else:
                if sec_offs & 1:
                    # Text Section
                    assert text_n < len(text_defs), "Not enough text sections defined"
                    sec_type = SectionType.TEXT
                    sec_name = text_defs[text_n].name
                    sec_attr = text_defs[text_n].attr
                    sec_nobits = text_defs[text_n].nobits
                    sec_balign = text_defs[text_n].balign
                    text_n += 1
                else:
                    # Data section
                    assert data_n < len(data_defs), "Not enough data sections defined"
                    sec_type = SectionType.DATA
                    sec_name = data_defs[data_n].name
                    sec_attr = data_defs[data_n].attr
                    sec_nobits = data_defs[data_n].nobits
                    sec_balign = data_defs[data_n].balign
                    data_n += 1
                sec_offs &= ~3
                sec_addr = self._base_addr + sec_offs
                sec = RelBinarySection(i, sec_name, sec_type, sec_offs, sec_addr, sec_size,
                                       sec_attr, sec_nobits, sec_balign)
                ret.append(sec)
                self._rel_sections.append(sec)

        assert text_n == len(text_defs), "Too many text sections defined"
        assert data_n == len(data_defs), "Too many data sections defined"

        return ret

    def get_entries(self) -> List[Tuple[int, str]]:
        """Returns all entry functions"""

        return [
            (
                self._rel_sections[self.read_byte(RelOffs.PROLOG_SECTION, True)].addr
                + self.read_word(RelOffs.PROLOG, True),
                "_prolog"
            ),
            (
                self._rel_sections[self.read_byte(RelOffs.EPILOG_SECTION, True)].addr
                + self.read_word(RelOffs.EPILOG, True),
                "_epilog"
            ),
            (
                self._rel_sections[self.read_byte(RelOffs.UNRESOLVED_SECTION, True)].addr
                + self.read_word(RelOffs.UNRESOLVED, True),
                "_unresolved"
            ),
        ]
    
    def _get_external_binaries(self) -> List[BinaryReader]:
        """Returns the external dol if given"""

        if self.dol is not None:
            return [self.dol]
        else:
            return []
    
    def addr_to_sec_offs(self, addr: int) -> Tuple[int, int]:
        """Converts an address to an offset into a section (for relocations)"""

        sec = self.find_section_containing(addr)
        return sec.rel_idx, addr - sec.addr

    def load_other(self, path: str) -> "RelReader":
        """Loads another binary of the same type with the same settings"""

        return RelReader(self.dol, path, self._base_addr, self._bss_addr, self._section_defs_raw,
                         self.func_prefix, self.label_prefix, self.data_prefix)
    
    def get_reloc_target(self, reloc: RelReloc) -> int:
        """Calculates the target address of a relocation"""

        # Calculate target
        if reloc.target_module == 0:
            # Dol - absolute address
            return reloc.addend
        else:
            # Rel - offset into section
            rel = self._rels.get(reloc.target_module)
            assert rel is not None, f"Relocation against unknown rel {reloc.target_module}"
            return rel._rel_sections[reloc.section].addr + reloc.addend

    def register_external_rel(self, rel: "RelReader"):
        """Adds an external rel file to link against"""

        assert rel.module_id not in self._rels, f"Duplicate module id {rel.module_id}"
        self._rels[rel.module_id] = rel
        self._externs.append(rel)

    def validate_reloc(self, addr: int, target: int, local_only=False) -> bool:
        """Override to only accept relocations if they're listed in the rel"""

        return (
            super().validate_reloc(addr, target, local_only) and
            (addr in self.addr_relocs or addr + 2 in self.addr_relocs)
        )
