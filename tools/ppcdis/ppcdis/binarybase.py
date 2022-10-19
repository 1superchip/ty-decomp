"""
Class to read data from a binary
"""

from abc import ABC, abstractmethod
from bisect import bisect_right
from dataclasses import dataclass
from enum import Enum, unique
from hashlib import sha1
import os
from struct import unpack
from typing import Dict, List, Tuple

# Dummy offset used to indicate a read from a bss section
OFFS_BSS = -1

@unique
class SectionType(Enum):
    """Types of section"""

    TEXT = 0
    DATA = 1
    BSS = 2 # not included in binary

@dataclass
class SectionDef:
    """Section property definitions container"""

    name: str
    attr: str = None
    nobits: bool = False
    balign: int = None

    @classmethod
    def parse(cls, defs):
        if isinstance(defs, Dict):
            print("Warning: the dict section_defs syntax is deprecated")
            return [
                cls(name, **(dat if dat is not None else {}))
                for name, dat in defs.items()
            ]
        else:
            return [
                cls(**(d))
                for d in defs
            ]


@dataclass
class BinarySection:
    """Container for information about a program binary section"""

    name: str
    type: SectionType
    offset: int
    addr: int
    size: int
    attr: str = None
    nobits: bool = False
    balign: int = None

    def contains_offs(self, offs: int) -> bool:
        """Checks if the section contains an offset relative to the whole binary"""

        return self.offset <= offs < self.offset + self.size

    def contains_addr(self, addr: int) -> bool:
        """Checks if the section contains an address in RAM"""

        return self.addr <= addr < self.addr + self.size
    
    def addr_to_offs(self, addr: int) -> int:
        """Converts an address in RAM contained by this section into a whole-binary offset"""

        assert self.contains_addr(addr), f"Address {addr:x} is not in section {self.name}"

        if self.type != SectionType.BSS:
            return addr - self.addr + self.offset
        else:
            return OFFS_BSS
    
    def get_start_text(self) -> str:
        """Gets the text to start a section in disassembly"""

        # Add name
        parts = [f".section {self.name}"]
        
        # Add attr
        if self.attr is not None:
            parts.append(f"\"{self.attr}\"")
        elif self.nobits:
            parts.append("\"\"")

        # Add nobits
        if self.nobits:
            parts.append("@nobits")
        
        return ", ".join(parts) + '\n'

    def validate_slice_bound(self, addr: int) -> bool:
        """Checks if a slice start/end addr is valid based on balign"""

        # Get balign
        balign = self.get_balign()

        # Skip if none
        if balign == 0:
            return True

        # Allow unaligned if it ends at the section end
        if addr == self.addr + self.size:
            return True

        # Check alignment
        mask = balign - 1
        return (addr & mask) == 0

    def assert_slice_bounds(self, start: int, end: int):
        """Asserts the start and end of a slice are aligned correctly"""

        if self.type == SectionType.TEXT:
            # For text, just check word alignment
            assert start & 3 == 0, f"{self.name} slice start {start:x} not aligned to 4"
            assert end & 3 == 0, f"{self.name} slice end {end:x} not aligned to 4"
        else:
            balign = self.get_balign()
            # For data, check it matches balign
            assert self.validate_slice_bound(start), \
                f"Start address {start:x} isn't aligned with {self.name} balign of {balign}"
            assert self.validate_slice_bound(end), \
                f"End address {end:x} isn't aligned with {self.name} balign of {balign}"

    def get_balign(self) -> int:
        """Gets the balign amount of slices in this section"""

        if self.balign is None:
            # Use default
            return 8 if self.type != SectionType.TEXT else 0
        else:
            # Use custom value
            return self.balign

    def get_balign_text(self) -> str:
        """Gets the balign text to start a slice with in disassembly"""

        # Get balign
        balign = self.get_balign()

        # Emit text if needed
        if balign != 0:
            return f"\n.balign {balign}\n"
        else:
            return ""

    def __repr__(self) -> str:
        """String representation for debugging"""

        return (f"BinarySection({self.name}, {self.type}, 0x{self.offset:x}, 0x{self.addr:x}, "
               f"0x{self.size:x}, {self.attr})")

class BinaryReader(ABC):
    def __init__(self, path: str, func_prefix: str, label_prefix: str, data_prefix: str):
        with open(path, 'rb') as f:
            self._dat = f.read()
        
        self.name = os.path.split(path)[-1]
        self.sections = self._get_sections()
        self._sec_addrs = sorted([sec.addr for sec in self.sections])
        self._externs = self._get_external_binaries()
        self.func_prefix = func_prefix
        self.label_prefix = label_prefix
        self.data_prefix = data_prefix

    @abstractmethod
    def _get_sections(self) -> List[BinarySection]:
        """Finds the sections in a binary"""

        raise NotImplementedError
    
    def _get_external_binaries(self) -> List["BinaryReader"]:
        """Finds the other loaded binaries"""

        return []
    
    @abstractmethod
    def get_entries(self) -> List[Tuple[int, str]]:
        """Returns all entry functions"""

        raise NotImplementedError

    def _addr_to_offs(self, addr: int) -> int:
        """Address to binary offset"""

        sec = self.find_section_containing(addr, True)

        assert sec is not None, f"Address {addr:x} is not local"

        return sec.addr_to_offs(addr)
    
    def find_section_containing(self, addr: int, local_only=False) -> BinarySection:
        """Finds the section containing an address"""

        # Search this binary first
        idx = bisect_right(self._sec_addrs, addr)
        if idx != 0:
            sec = self.sections[idx-1]
            if sec.contains_addr(addr):
                return sec
        
        # Search external binaries if enabled
        if not local_only:
            for ext in self._externs:
                ret = ext.find_section_containing(addr, True)
                if ret is not None:
                    return ret
        
        # Not found
        return None

    def validate_addr(self, addr: int, local_only=False) -> bool:
        """Checks if an address is a meaningful place to be pointed to"""

        # Check if local
        sec = self.find_section_containing(addr, True)
        if sec is None:
            # Check if in any extern if allowed
            if not local_only:
                return any(ext.validate_addr(addr, True) for ext in self._externs)
            else:
                return False
        elif sec.type == SectionType.TEXT:
            # Text addresses will be 4-byte aligned
            # TODO: LECT code might not follow this
            return addr & 3 == 0
        else:
            # Valid data address
            return True
    
    def contains_addr(self, addr: int) -> bool:
        """Checks if an address is contained by this binary"""

        return self.find_section_containing(addr, True) is not None

    def addr_is_local(self, addr: int) -> bool:
        """Checks if an address belongs to this binary"""

        return self.find_section_containing(addr, True) is not None
    
    def get_section_by_name(self, name: str) -> BinarySection:
        """Gets a section by name, None if it doesn't exist"""
        
        for sec in self.sections:
            if sec.name == name:
                return sec
        else:
            return None

    def read(self, addr: int, size: int, is_offset=False) -> bytes:
        """Reads bytes from an address"""
        
        # Convert to offset
        if is_offset:
            offs = addr
        else:
            offs = self._addr_to_offs(addr)

        # Read
        if offs != OFFS_BSS:
            return self._dat[offs:offs+size]
        else:
            return bytes(size)

    def read_int(self, addr: int, size: int, is_offset=False, signed=False) -> int:
        """Reads an integer at an address"""

        return int.from_bytes(self.read(addr, size, is_offset), "big", signed=signed)

    def read_byte(self, addr: int, is_offset=False, signed=False) -> int:
        """Reads a byte at an address"""

        return self.read_int(addr, 1, is_offset, signed)

    def read_half(self, addr: int, is_offset=False, signed=False) -> int:
        """Reads a halfword at an address"""

        return self.read_int(addr, 2, is_offset, signed)

    def read_word(self, addr: int, is_offset=False, signed=False) -> int:
        """Reads a word at an address"""

        return self.read_int(addr, 4, is_offset, signed)
    
    def read_word_array(self, addr: int, length: int, is_offset=False) -> List[int]:
        """Reads a word array at an address"""

        return [self.read_word(addr + (i * 4), is_offset) for i in range(length)]
    
    def read_float(self, addr: int, is_offset=False) -> float:
        """Reads a float at an address"""

        dat = self.read(addr, 4, is_offset)
        return unpack(">f", dat)[0]
    
    def read_double(self, addr: int, is_offset=False) -> float:
        """Reads a float double at an address"""

        dat = self.read(addr, 8, is_offset)
        return unpack(">d", dat)[0]
    
    def get_rom_copy_info(self) -> int:
        """Gets the start address of the rom copy info in the .init section
        None if not found / irrelevant for this binary"""

        return None

    def section_sha(self, section: BinarySection) -> bytes:
        """Hashes a section in this binary"""
        
        if section.type == SectionType.BSS:
            return section.size
        else:
            dat = self.read(section.offset, section.size, True)
            return sha1(dat).digest()

    @abstractmethod
    def load_other(self, path: str) -> "BinaryReader":
        """Loads another binary of the same type with the same settings"""

        raise NotImplementedError
