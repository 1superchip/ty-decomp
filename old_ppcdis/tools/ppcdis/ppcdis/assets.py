"""
Helpers for binary asset processing
"""

from .binarybase import BinaryReader

def rip_asset(binary: BinaryReader, start_addr: int, end_addr: int) -> bytes:
    """Rips an asset from a binary"""

    return binary.read(start_addr, end_addr - start_addr)

def format_bytes(dat: bytes, width: int) -> str:
    """Outputs data as a C byte array"""

    return ', \n'.join(
        ', '.join(
            f"0x{x:02x}" for x in dat[i:i+width]
        )
        for i in range(0, len(dat), width)
    )
