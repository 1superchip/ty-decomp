"""
Tools for LCF preprocessing
"""

from typing import List

from .binarybase import BinaryReader
from .symbols import LabelManager, SymbolGetter

def apply_forceactive(binary: BinaryReader, symbols_path: str, labels_path: str, externs_path: str,
                      txt: str) -> str:
    """Add forceactive entries to an LCF file from relextern output"""

    sym = SymbolGetter(symbols_path, None, labels_path, binary)
    externs = LabelManager(externs_path)

    return txt.replace(
        "PPCDIS_FORCEACTIVE",
        '\n'.join(
            sym.get_name(addr)

            for addr in externs.get_addrs()
            if binary.contains_addr(addr)
        )
    )

def apply_forcefiles(txt: str, files: List[str]) -> str:
    """Add forcefiles entries to an LCF file"""

    return txt.replace("PPCDIS_FORCEFILES", '\n'.join(files))
