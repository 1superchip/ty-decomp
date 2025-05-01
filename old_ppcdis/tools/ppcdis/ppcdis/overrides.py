"""
Handling of user-provided hints
"""
    
from abc import ABC, abstractmethod
from typing import Dict, List

from .fileutil import load_from_yaml

class OverrideManager(ABC):
    """Class to handle user-provided information"""

    def __init__(self, path: str):
        # Load yml if given
        if path is not None:
            yml = load_from_yaml(path)
        else:
            yml = {}
        
        self.load_yml(yml)

    @abstractmethod
    def load_yml(self, yml: Dict):
        """Loads data from an overrides yaml file"""

        raise NotImplementedError
    
    def _make_ranges(self, ranges: List[List[int]]) -> List[range]:
        """Converts a list of start-end pairs into ranges"""

        return [
            range(start, end)
            for start, end in ranges
        ]

    def _make_size_ranges(self, ranges: List[List[int]]) -> List[range]:
        """Converts a list of start-size pairs into ranges"""

        return [
            range(start, start + size)
            for start, size in ranges
        ]

    def _check_range(self, ranges: List[range], val: int) -> bool:
        """Checks if any range in a list contains a value"""

        return any(val in r for r in ranges)

    def _find_ranges(self, ranges: List[range], val: int) -> List[range]:
        """Finds the ranges in a list containing a value"""

        return [r for r in ranges if val in r]
