"""
Binary reader for REL files
"""

from dataclasses import dataclass
from enum import IntEnum, unique
from typing import Dict, List, Tuple

from .binarybase import BinaryReader, BinarySection, SectionDef, SectionType
from .binaryrel import RelReader

@unique
class LECTOffs(IntEnum):
    """Offsets of fields in the rel header"""

    BASE_ADDR = 0xc
    ENTRY = 0x10

@dataclass
class _LECTSectionDef:
    offs: int

@dataclass
class LECTSectionDef(SectionDef, _LECTSectionDef):
    """Expansion of SectionDef for LECT sections"""

class LECTReader(BinaryReader):
    def __init__(self, rel: RelReader, path: str, section_defs: Dict, func_prefix: str,
                 label_prefix: str, data_prefix: str):
        # Handle params
        self.rel = rel
        self._section_defs_raw = section_defs
        self._section_defs = LECTSectionDef.parse(section_defs)

        # Call parent constructor
        super().__init__(path, func_prefix, label_prefix, data_prefix)

    def _get_sections(self) -> List[BinarySection]:
        """Finds the sections in a binary"""

        # Get base address
        self._base_addr = self.read_word(LECTOffs.BASE_ADDR, True)

        # Calculate section sizes
        offsets = [d.offs for d in self._section_defs] + [len(self._dat)]
        sizes = [offsets[i+1] - offsets[i] for i in range(len(offsets) - 1)]

        # Make sections
        return [
            BinarySection(d.name, SectionType.TEXT if i == 0 else SectionType.DATA, d.offs,
                          self._base_addr + d.offs, sizes[i], d.attr, d.nobits, d.balign)
            for i, d in enumerate(self._section_defs)
        ]

    def get_entries(self) -> List[Tuple[int, str]]:
        """Returns all entry functions"""

        return [(self.read_word(LECTOffs.ENTRY, True), "lect_main")]
    
    def _get_external_binaries(self) -> List[BinaryReader]:
        """Returns the external dol if given"""

        if self.rel is not None:
            return [self.rel] + self.rel._externs
        else:
            return []
    
    def load_other(self, path: str) -> "LECTReader":
        """Loads another binary of the same type with the same settings"""

        return LECTReader(self.rel, path, self._section_defs_raw, self.func_prefix,
                          self.label_prefix, self.data_prefix)
