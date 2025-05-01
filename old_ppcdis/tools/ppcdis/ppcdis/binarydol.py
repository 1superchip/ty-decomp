"""
Binary reader for DOL files
"""

from typing import Dict, List, Tuple

from .binarybase import BinaryReader, BinarySection, SectionDef, SectionType
from .fileutil import load_from_yaml_str

default_section_defs = load_from_yaml_str("""
text:
  - name: .init
  - name: .text
data:
  - name: extab_
    attr: a
  - name: extabindex_
    attr: a
  - name: .ctors
    balign: 0
  - name: .dtors
    balign: 0
  - name: .rodata
  - name: .data
  - name: .sdata
  - name: .sdata2
bss:
  - name: .bss
  - name: .sbss
  - name: .sbss2
""")

TEXT_COUNT = 7
DATA_COUNT = 11

OFFS_TEXT_OFFSETS = 0x0
OFFS_DATA_OFFSETS = 0x1c
OFFS_TEXT_ADDRESSES = 0x48
OFFS_DATA_ADDRESSES = 0x64
OFFS_TEXT_SIZES = 0x90
OFFS_DATA_SIZES = 0xac
OFFS_BSS_START = 0xd8
OFFS_BSS_SIZE = 0xdc
OFFS_ENTRY = 0xe0

class DolReader(BinaryReader):
    def __init__(self, path: str, r13: int, r2: int, section_defs: Dict, func_prefix: str,
                 label_prefix: str, data_prefix: str):
        self._section_defs_raw = section_defs
        if section_defs is None:
            section_defs = default_section_defs
        self._section_defs = [
            SectionDef.parse(section_defs["text"]),
            SectionDef.parse(section_defs["data"]),
            SectionDef.parse(section_defs["bss"])
        ]
        self.r13 = r13
        self.r2 = r2
        super().__init__(path, func_prefix, label_prefix, data_prefix)

    def _get_sections(self) -> List[BinarySection]:
        """Finds the sections in a binary"""

        # Get section definitions
        text_defs, data_defs, bss_defs = self._section_defs

        # Get text sections
        text_offsets = self.read_word_array(OFFS_TEXT_OFFSETS, TEXT_COUNT, True)
        assert all(offs == 0 for offs in text_offsets[len(text_defs):]), \
               "Not enough text sections defined"
        text_offsets = text_offsets[:len(text_defs)]
        assert all(offs != 0 for offs in text_offsets), "Too many text sections defined"
        text_addresses = self.read_word_array(OFFS_TEXT_ADDRESSES, len(text_offsets), True)
        text_sizes = self.read_word_array(OFFS_TEXT_SIZES, len(text_offsets), True)

        # Get data sections
        data_offsets = self.read_word_array(OFFS_DATA_OFFSETS, DATA_COUNT, True)
        assert all(offs == 0 for offs in data_offsets[len(data_defs):]), \
               "Not enough data sections defined"
        data_offsets = data_offsets[:len(data_defs)]
        assert all(offs != 0 for offs in data_offsets), "Too many data sections defined"
        data_addresses = self.read_word_array(OFFS_DATA_ADDRESSES, len(data_offsets), True)
        data_sizes = self.read_word_array(OFFS_DATA_SIZES, len(data_offsets), True)
        
        # Makes section list
        sections = [
            BinarySection(sdef.name, SectionType.TEXT, offs, addr, size, sdef.attr, sdef.nobits,
                          sdef.balign)
            for sdef, offs, addr, size in zip(text_defs, text_offsets, text_addresses, text_sizes)
        ] + [
            BinarySection(sdef.name, SectionType.DATA, offs, addr, size, sdef.attr, sdef.nobits,
                          sdef.balign)
            for sdef, offs, addr, size in zip(data_defs, data_offsets, data_addresses, data_sizes)
        ]
        sections.sort(key=lambda s: s.addr)

        # Prepare bss sections
        bss_start = self.read_word(OFFS_BSS_START, True)
        bss_size = self.read_word(OFFS_BSS_SIZE, True)
        bss_end = bss_start + bss_size

        # Add unlisted bss sections where size can be inferred from cutting
        bss_n = 0
        sections_with_bss = []
        for section in sections:
            # Section cutting bss range into sub-section
            if bss_start < section.addr and bss_n < len(bss_defs):
                sdef = bss_defs[bss_n]

                # TODO: support back to back bss in between sections
                assert sdef.bss_forced_size is None, "Error: bss_forced_size is only " \
                    "supported for sections after data currently."

                # Align start
                if sdef.bss_start_align is not None:
                    mask = sdef.bss_start_align - 1
                    bss_start = (bss_start + mask) & ~mask
                
                # Handle early end
                end = min(section.addr, bss_end)

                sections_with_bss.append(
                    BinarySection(sdef.name, SectionType.BSS, 0, bss_start,
                                  end - bss_start, sdef.attr, sdef.nobits, sdef.balign)
                )

                bss_n += 1
                if end == bss_end:
                    bss_start = end
                else:
                    bss_start = section.addr + section.size

            # Back-to-back sections cutting bss range into 1 sub-section
            elif bss_start == section.addr:
                bss_start = section.addr + section.size

            sections_with_bss.append(section)

        # Add remaining definitions
        for bss_n in range(bss_n, len(bss_defs)):
            sdef = bss_defs[bss_n]

            assert bss_start < bss_end, f"Reached end of bss with only {bss_n} sections, expected {len(bss_defs)}. " \
                "You may want to remove some sections with section_defs. (Most common cause is .sbss2 not existing)"

            # Align start
            if sdef.bss_start_align is not None:
                mask = sdef.bss_start_align - 1
                bss_start = (bss_start + mask) & ~mask

            # Get size
            size = sdef.bss_forced_size
            if size is None:
                if bss_n + 1 == len(bss_defs):
                    size = bss_end - bss_start
                else:
                    assert False, f"Unknown size for {sdef.name}, either remove the section " \
                        "or set bss_forced_size"

            sections_with_bss.append(
                BinarySection(sdef.name, SectionType.BSS, 0, bss_start, size,
                              sdef.attr, sdef.nobits, sdef.balign)
            )
            bss_start += size

        assert bss_start >= bss_end, f"BSS end not reached, only found up to {bss_start:x} but " \
            f"expected {bss_end:x}"
        assert bss_start <= bss_end, f"BSS end exceeded, found up to {bss_start:x} but " \
            f"expected {bss_end:x}"

        return sections_with_bss

    def get_entries(self) -> List[Tuple[int, str]]:
        """Returns all entry functions"""
        
        return [(self.read_word(OFFS_ENTRY, True), "__start")]
    
    def get_rom_copy_info(self) -> int:
        """Gets the start address of the rom copy info in the .init section
        None if not found / irrelevant for this binary"""

        # Try get .init section
        init = self.get_section_by_name(".init")
        if init is None:
            return None

        # Try find the .init section address repeated twice
        for addr in range(init.addr, init.addr + init.size - 8, 4):
            # First copy
            if self.read_word(addr) == init.addr:
                # Second copy
                if self.read_word(addr + 4) == init.addr:
                    return addr
        
        # Not found
        return None

    def load_other(self, path: str) -> "DolReader":
        """Loads another binary of the same type with the same settings"""

        return DolReader(path, self.r13, self.r2, self._section_defs_raw, self.func_prefix,
                         self.label_prefix, self.data_prefix)
