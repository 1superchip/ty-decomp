"""
pyelftools substitutes for performance reasons
"""

from dataclasses import dataclass
from io import FileIO
from struct import unpack
from typing import Dict, List, Set, Tuple

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
from elftools.elf.sections import SymbolTableSection

SymNameMap = Dict[str, "Symbol"]
SymIdMap = Dict[int, "Symbol"]
SymBadNames = Set[str]

SHN_UNDEF = 0
SHN_ABS = 0xfff1
SHN_COMMON = 0xfff2

@dataclass
class Symbol:
    """pyelftools symbol substitute"""
    
    name: str
    st_value: int
    st_size: int
    st_info: int # TODO: bitfields
    st_other: int # TODO: bitfields
    st_shndx: int
    
    def map_symbols(f: FileIO, elf: ELFFile) -> Tuple[SymNameMap, SymBadNames, SymIdMap]:
        """Loads symbols from an ELF file into dicts mapped by name and id"""
        # Get symbol table
        symtab: SymbolTableSection = elf.get_section_by_name(".symtab")

        # Parse symbol table
        symbols = {}
        symbols_id = {}
        duplicates = set()
        for i in range(symtab.num_symbols()):
            # Read in symbol bytes
            f.seek(symtab["sh_offset"] + (i * symtab["sh_entsize"]))
            dat = f.read(symtab["sh_entsize"])

            # Parse bytes
            st_name, st_value, st_size, st_info, st_other, st_shndx = unpack(">IIIBBH", dat)
            name = symtab.stringtable.get_string(st_name)
            sym = Symbol(name, st_value, st_size, st_info, st_other, st_shndx)

            # Add to dicts
            if sym.name != "":
                if sym.name in symbols:
                    duplicates.add(sym.name)
                symbols[sym.name] = sym
            symbols_id[i] = sym
        
        return symbols, duplicates, symbols_id

@dataclass
class Relocation:
    """pyelftools relocation substitute"""

    r_offset: int
    r_info_sym: int
    r_info_type: int
    r_addend: int

    def read_relocs(f: FileIO, rela: RelocationSection) -> List["Relocation"]:
        """Loads relocations from a rela section in an ELF file"""

        # Iterate relocations
        relocs = []
        for i in range(rela.num_relocations()):
            # Read in reloc bytes
            f.seek(rela._offset + (i * rela.entry_size))
            dat = f.read(rela.entry_size)

            # Parse bytes
            r_offset, r_info, r_addend = unpack(">III", dat)
            r_info_sym = r_info >> 8
            r_info_type = r_info & 0xff
            rel = Relocation(r_offset, r_info_sym, r_info_type, r_addend)

            # Add to output
            relocs.append(rel)

        return relocs
    
    def map_relocs(f: FileIO, rela: RelocationSection) -> Dict[int, "Relocation"]:
        relocs = Relocation.read_relocs(f, rela)
        return {
            reloc.r_offset : reloc
            for reloc in relocs
        }
