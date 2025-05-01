"""
Helpers for disassembly splitting
"""

from collections import defaultdict
from dataclasses import dataclass
from typing import Dict, List, Tuple, Union

from .binarybase import BinaryReader, BinarySection
from .fileutil import load_from_yaml
from .symbols import LabelManager

@dataclass(frozen=True)
class Slice:
    """A range of addresses within a section for splitting"""

    # Start address of range, inclusive
    start: int

    # End address of range, exclusive
    end: int

    # Containing section name
    section: str

    # Source file the slice is contained in, if any
    source: str = None

    def __repr__(self):
        """String representation for debugging"""

        source = '' if self.source is None else self.source + ' '
        return f"Slice({source}{self.start:x}-{self.end:x})"

    def __contains__(self, val):
        """Checks if an address falls in the slice's range"""

        return self.start <= val < self.end

# Description of a source exposed to project build script
# If decompiled: path to source file
# Else: section, start addr, end addr 
SourceDesc = Union[str, Tuple[str, int, int]]

@dataclass
class Source:
    """Internal grouping of slices into a source file"""

    source: str
    slices: Dict[str, Slice]

    def __repr__(self):
        """String representation for debugging"""

        if self.source is not None:
            return self.source
        else:
            sl = [sl for _, sl in self.slices.items()][0]
            return str(sl)

    def __contains__(self, addr: int):
        """Checks if an address is contained by any slice in the source"""

        return any(addr in self.slices[sec] for sec in self.slices)

    def __lt__(self, other: "Source"):
        """Checks if any slice in this source comes before any slice in the other"""

        return any(
            sec in other.slices and self.slices[sec].start < other.slices[sec].start
            for sec in self.slices
        )
    
    def describe(self) -> SourceDesc:
        """Generates the description for the project build script"""

        if self.source is not None:
            # Give source name
            return self.source
        else:
            # Return properties of only slice
            assert len(self.slices) == 1, f"Undecompiled source has multiple slices"
            sl = [sl for _, sl in self.slices.items()][0]
            return sl.section, sl.start, sl.end
    
def load_slice_yaml(path: str, sections: List[BinarySection], base_path='') -> List[Source]:
    """Loads sources from a yaml file & fills in gaps"""
    
    # Iterate over all sources in yaml
    sources = []
    slices = defaultdict(list)
    yml = load_from_yaml(path, [])
    sec_map = {sec.name : sec for sec in sections}
    for source in yml:
        # Add slices to the source & their sections
        src = Source(base_path + source, {})
        sources.append(src)
        for sec_name, (start, end) in yml[source].items():
            # Create slice
            sl = Slice(start, end, sec_name, base_path + source)

            # Validate
            sec = sec_map[sec_name]
            sec.assert_slice_bounds(start, end)
            assert start < end, f"Backwards slice {start:x}-{end:x} ({source})"
            assert sec.addr <= sl.start < sl.end <= sec.addr + sec.size, \
                f"Slice {sl} isn't within bounds of section {sec.name}"

            # Register
            slices[sec_name].append(sl)
            src.slices[sec_name] = sl

    # Fill in gaps 
    extra = fill_sections(slices, sections)

    # Convert back to list & return
    return sources + extra

def cover_range(start: int, end: int, section: str) -> List[Source]:
    """Make source(s) to cover a range"""

    ret = []

    # For some reason, mwld will align even-word-count ctors entries to 8
    if section == ".ctors" and (end - start) % 8 == 0 and start % 8 != 0:
        # Add an extra 4-byte slice so that both word counts are odd
        ret.append(
            Source(
                None,
                {section : Slice(start, start + 4, section)} 
            )
        )
        start += 4
    
    # Add main range
    ret.append(
        Source(
            None,
            {section : Slice(start, end, section)}
        )
    )
    
    return ret

def fill_sections(slices: Dict[str, List[Slice]], sections: List[BinarySection]) -> List[Source]:
    """Fills in the gaps in a list of slices with new sources"""

    ret = []
    for section in sections:
        # Get slices
        sls = sorted(slices.get(section.name, []), key=lambda sl: sl.start)

        # Fill gaps in section
        pos = section.addr
        for sl in sls:
            assert sl.start >= pos, f"Overlapping slice {sl}"

            # Create slice before this one if needed
            if sl.start > pos:
                ret.extend(cover_range(pos, sl.start, section.name))

            # Move to end
            pos = sl.end

        # Add final slice if needed
        if pos < section.addr + section.size:
            ret.extend(cover_range(pos, section.addr + section.size, section.name))

    return ret

def order_sources(sources: List[Source]) -> List[SourceDesc]:
    """Orders source files for linking from a group of slices"""

    ret = sources[:]
    changed = True
    while changed:
        changed = False
        for i in range(1, len(ret)):
            for j in range(i):
                if ret[i] < ret[j]:
                    changed = True
                    ret[i], ret[j] = ret[j], ret[i]

    return [source.describe() for source in ret]

def find_containing_source(sources: List[Source], addr: int) -> str:
    """Finds the source containing an addr
    Empty string if not decompiled, None if not found"""

    # Try find
    for src in sources:
        if addr in src:
            return src.describe()
    
    # Not found
    return None

def calc_progress_info(binary: BinaryReader, sources: List[Source], labels_path: str
                      ) -> Tuple[Dict[str, int], Dict[str, int], int]:
    """Calculates decompiled slices size, total section size, and all symbol sizes for each section"""

    # Load labels pickle
    labels = LabelManager(labels_path, binary)

    # Add slice sizes
    decomp_slices_sizes = defaultdict(lambda: 0)
    total_sizes = defaultdict(lambda: 0)
    for source in sources:
        for sec_name, sl in source.slices.items():
            # Add size
            size = sl.end - sl.start
            total_sizes[sec_name] += size
            if source.source is not None:
                decomp_slices_sizes[sec_name] += size
    
    # Add any missing sections to decomp
    for sec in total_sizes:
        if sec not in decomp_slices_sizes:
            decomp_slices_sizes[sec] = 0

    return decomp_slices_sizes, total_sizes, labels.get_sizes()
