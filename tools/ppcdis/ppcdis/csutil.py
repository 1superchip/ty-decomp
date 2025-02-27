"""
Capstone helpers
"""

from typing import OrderedDict, Tuple
from dataclasses import dataclass
import struct

from capstone import Cs, CS_ARCH_PPC, CsInsn, CS_MODE_32, CS_MODE_BIG_ENDIAN
from capstone.ppc import *

from .instrcats import (blacklistedInsns, firstGprWriteInsns, firstLastGprWriteInsns, 
                       lastGprWriteInsns)

@dataclass
class DummyInstr:
    """Dummy instruction class for data lines"""

    address: int
    bytes: bytes

def sign_half(half: int) -> int:
    """Sign extends a 16-bit int"""

    return struct.unpack(">h", struct.pack(">H", half))[0]

def check_overwrites(instr: CsInsn) -> Tuple[int]:
    """Returns all GPRs overwritten by an instruction"""

    if instr.id in firstGprWriteInsns:
        return (instr.operands[0].reg,)
    elif instr.id in firstLastGprWriteInsns:
        return (instr.operands[0].reg, instr.operands[1].mem.base)
    elif instr.id in lastGprWriteInsns:
        return (instr.operands[1].mem.base,)
    elif instr.id == PPC_INS_LMW:
        return range(instr.operands[0].reg, PPC_REG_R31 + 1)
    elif instr.id == PPC_INS_LSWI:
        n = (instr.operands[2].imm + 3) // 4
        return range(instr.operands[0].reg, instr.operands[0].reg + n)
    else:
        return ()

def cs_should_ignore(instr: CsInsn) -> bool:
    """Checks if an instruction output by capstone should be ignored"""

    # Instructions capstone gets wrong
    if instr.id in blacklistedInsns:
        return True

    # Flag wouldn't be preserved by assembler, probably data anyway
    if instr.id == PPC_INS_BDNZ:
        return instr.bytes[0] & 1 == 1
    
    # GCC assembler refuses
    if instr.id == PPC_INS_LMW:
        return instr.operands[0].reg < instr.operands[1].mem.base
    
    return False

def cs_disasm(addr: int, dat: bytes) -> OrderedDict[int, CsInsn]:
    """Disassembles code into an ordered dict of CsInsns"""

    cs = Cs(CS_ARCH_PPC, CS_MODE_32 | CS_MODE_BIG_ENDIAN)
    cs.detail = True

    ret = OrderedDict()
    i = 0
    while i < len(dat):
        # Get capstone to disassemble as many as possible
        for instr in cs.disasm(dat[i:], addr + i, (len(dat) - i) // 4):
            if cs_should_ignore(instr):
                instr = DummyInstr(instr.address, instr.bytes)

            ret[instr.address] = instr
            i += 4

        # Skip instruction capstone failed
        if i < len(dat):
            val = dat[i:i + 4]
            ret[addr + i] = DummyInstr(addr + i, val)
            i += 4

    return ret
