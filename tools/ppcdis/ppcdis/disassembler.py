"""
Disassembler for assembly code (re)generation
"""

from dataclasses import dataclass
from hashlib import sha1
import json
from typing import Dict, Set, Tuple

import capstone
from capstone.ppc import *

from .analyser import RelocType
from .binarybase import BinaryReader, BinarySection, SectionType
from .binarylect import LECTReader
from .csutil import DummyInstr, sign_half, cs_disasm 
from .instrcats import (labelledBranchInsns, conditionalBranchInsns, upperInsns, lowerInsns,
                       signExtendInsns, storeLoadInsns, renamedInsns)
from .overrides import OverrideManager
from .slices import Slice, Source
from .relocs import RelocGetter
from .symbols import SymbolGetter, has_bad_chars, is_mangled

class DisassemblyOverrideManager(OverrideManager):
    """Disassembly category OverrideManager"""

    def load_yml(self, yml: Dict):
        """Loads data from a disassembly overrides yaml file"""

        # Load categories
        self._manual_float_ranges = self._make_ranges(yml.get("manual_sdata2_ranges", []))
        self._global_manual_floats = yml.get("global_manual_floats", False)
        self._trim_ctors = yml.get("trim_ctors", False)
        self._trim_dtors = yml.get("trim_dtors", False)
        self._symbol_aligns = yml.get("symbol_aligns", {})

    def is_manual_sdata2(self, addr: int) -> bool:
        """Checks if the symbol at an address should be made relative to r2 for manual handling
        in inline assembly"""

        return self._global_manual_floats or self._check_range(self._manual_float_ranges, addr)
    
    def should_trim_ctors(self) -> bool:
        """Checks if terminating zeros should be removed from .ctors disassembly"""

        return self._trim_ctors

    def should_trim_dtors(self) -> bool:
        """Checks if terminating zeros should be removed from .ctors disassembly"""

        return self._trim_dtors
    
    def get_symbol_align(self, addr: int) -> int:
        """Gets the alignment a symbol should have, or 0 if none"""

        return self._symbol_aligns.get(addr, 0)

class ReferencedTracker:
    """Tracker for symbols referenced by a portion of assembly (for forward declarations)"""

    def __init__(self):
        self._referenced = set()
        self._mangled = set()
    
    def notify(self, addr: int, name: str):
        """Tracks that an address has been referenced"""

        # Skip if already in
        if addr in self._referenced:
            return

        # Add to main list
        self._referenced.add((addr, name))

        # Add to mangled if needed
        if is_mangled(name):
            self._mangled.add((addr, name))

    def get_referenced(self) -> Set[Tuple[int, str]]:
        """Gets all symbols referenced in tracking"""

        return self._referenced

    def get_mangled_referenced(self) -> Set[Tuple[int, str]]:
        """Gets all mangled symbols referenced in tracking"""

        return self._mangled

@dataclass
class DisasmLine:
    """Handler for one line of disassembled text"""

    instr: capstone.CsInsn
    mnemonic: str
    operands: str
 
    def to_txt(self, sym: SymbolGetter, inline=False, hashable=False, referenced=None) -> str:
        """Gets the disassembly text for a line"""

        # Add symbol name if required
        prefix = []
        name = sym.get_name(self.instr.address, hashable, True)
        if name is not None:
            if sym.is_global(self.instr.address):
                # Globals other than mid function entries should already be labelled
                if sym.is_mid_function_entry(self.instr.address):
                    if inline:
                        prefix.append(f"entry {name}")
                        if referenced is not None:
                            referenced.notify(self.instr.address, name)
                    else:
                        prefix.append(f".global {name}")
                        prefix.append(f"{name}:")
            else:
                prefix.append(f"{name}:")

        # Add jumptable label if required
        # .global affects branch hints, so a new label is created for this
        # TODO: generate symbol names for this in hashable?
        if not hashable and sym.check_jt_label(self.instr.address):
            jump = f"jump_{self.instr.address:x}"
            if inline:
                prefix.append(f"entry {jump}")
                if referenced is not None:
                    referenced.notify(self.instr.address, jump)
            else:
                prefix.append(f".global {jump}")
                prefix.append(f"{jump}:")

        if len(prefix) > 0:
            prefix.append('')
        prefix = '\n'.join(prefix)

        # Add address & bytes if required        
        if not hashable:
            comment = f"/* {self.instr.address:X} {self.instr.bytes.hex().upper()} */ "
        else:
            comment = ""

        # Add main data
        return f"{prefix}{comment}{self.mnemonic:<12}{self.operands}"
        
class Disassembler:
    def __init__(self, binary: BinaryReader, labels_path: str, reloc_path: str, symbols_path=None,
                 overrides_path=None, source_name=None, quiet=False):
        self._bin = binary
        self._sym = SymbolGetter(symbols_path, source_name, labels_path, binary)
        self._rlc = RelocGetter(binary, self._sym, reloc_path)
        self._ovr = DisassemblyOverrideManager(overrides_path)
        self._quiet = quiet
    
    def _print(self, msg: str):
        """Prints a message if not in quiet mode"""

        if not self._quiet:
            print(msg)

    ################
    # Instructions #
    ################

    def _process_branch_hint(self, instr: capstone.CsInsn, line: DisasmLine):
        """Adds +/- to show conditional branch hints"""

        # Check hint bit
        dat = int.from_bytes(instr.bytes, 'big')
        likely = (dat >> 21) & 1 == 1

        # Handle destination
        if instr.id in labelledBranchInsns:
            dest = instr.operands[-1].imm
            delta = dest - instr.address

            # Flip hint for negative branches
            # GCC assembler assumes the delta is positive if branching to a global
            if delta < 0 and not self._sym.is_global(dest):
                likely = not likely
        
        # Append to mnemonic
        line.mnemonic = instr.mnemonic + ('+' if likely else '-')

    def _process_labelled_branch(self, instr: capstone.CsInsn, line: DisasmLine, inline=False,
                                 hashable=False, referenced=None):
        """Replaces the destination of a branch with its symbol, and creates that if necessary"""

        # Preserve condition register usage if present
        if instr.op_str.startswith("cr"):
            cr = instr.reg_name(instr.operands[0].reg) + ', '
        else:
            cr = ''
        dest = instr.operands[-1].imm
        
        # Get symbol name
        name = self._sym.get_name(dest, hashable)

        # Use hardcoded address if needed
        # TODO: make shiftable somehow
        if inline:
            if has_bad_chars(name):
                delta = dest - instr.address
                sign = '+' if delta >= 0 else '-'
                name = f"*{sign}0x{abs(delta):x}"
        
            # Add to referenced list
            elif referenced is not None and self._sym.is_global(dest):
                referenced.notify(dest, name)

        # Replace destination with label name
        line.operands = cr + name
    
    def _process_signed_immediate(self, instr: capstone.CsInsn, line: DisasmLine):
        """Sign extends the immediate when capstone misses it"""

        others = instr.op_str[:instr.op_str.rfind(' ')]
        signed = sign_half(instr.operands[-1].imm)
        
        line.operands = others + ' ' + hex(signed)
    
    def _process_upper(self, instr: capstone.CsInsn, line: DisasmLine, inline=False,
                       hashable=False, referenced=None):
        """Replaces the immediate of a lis with a reference"""

        # Get reference info
        ref = self._rlc.get_reference_at(instr.address)

        # Just a normal lis, no reference
        if ref is None:
            return

        # Get symbol name
        name = self._sym.get_name(ref.target, hashable)

        if inline:
            # Use hardcoded address if needed
            # TODO: make shiftable somehow
            if has_bad_chars(name):
                name = hex(ref.target)
            
            # Add to referenced list
            elif referenced is not None:
                referenced.notify(ref.target, name)

        # Add reference to operands
        dest = instr.reg_name(instr.operands[0].reg)
        sym = name + ref.format_offs()
        if ref.t == RelocType.NORMAL:
            rel = "h"
        elif ref.t == RelocType.ALGEBRAIC:
            rel = "ha"
        else:
            assert 0, f"Bad reloc {instr.address:x} -> {ref}"
        line.operands = f"{dest}, {sym}@{rel}"
    
    def _process_lower(self, instr: capstone.CsInsn, line: DisasmLine, inline=False,
                       hashable=False, referenced=None):
        """Replaces the immediate of a lower instruction with a reference"""

        # Get reference info
        ref = self._rlc.get_reference_at(instr.address)

        # Just a normal instruction, no reference
        if ref is None:
            return

        # Get register info
        dest = instr.reg_name(instr.operands[0].reg)
        if instr.id in storeLoadInsns:
            reg = instr.operands[1].mem.base
        else:
            reg = instr.operands[1].reg
        reg_name = instr.reg_name(reg)

        # Get symbol name
        name = self._sym.get_name(ref.target, hashable)

        if inline:
            # Use hardcoded address if needed
            # TODO: make shiftable somehow
            if has_bad_chars(name):
                name = hex(ref.target)
        
            # Add to reference list
            elif referenced is not None:
                referenced.notify(ref.target, name)

        sym = name + ref.format_offs()

        # Different syntax for inline asm SDA
        if inline and ref.t == RelocType.SDA:
            if reg == PPC_REG_R2 and self._ovr.is_manual_sdata2(ref.target):
                if instr.id in storeLoadInsns:
                    line.operands = f"{dest}, {sym} (r2)"
                else:
                    line.operands = f"{dest}, r2, {sym}"
            else:
                if reg == PPC_REG_R2 and instr.id in (PPC_INS_LFS, PPC_INS_LFD):
                    if instr.id == PPC_INS_LFS:
                        sym = self._bin.read_float(ref.target)
                    else:
                        sym = self._bin.read_double(ref.target)
                line.operands = f"{dest}, {sym}"
                if instr.id == PPC_INS_ADDI:
                    line.mnemonic = "la"
        else:
            # Handle relocation type
            if ref.t == RelocType.NORMAL:
                rel = '@l'
            else:
                assert reg in (PPC_REG_R13, PPC_REG_R2), f"Bad reloc {instr.address:x} -> {ref}"
                if isinstance(self._bin, LECTReader):
                    rel = "-_SDA2_BASE_" if reg == PPC_REG_R2 else "-_SDA_BASE_"
                else:
                    rel = '@sda21'
                    reg_name = "0"

            # Update operands
            if instr.id in storeLoadInsns:
                line.operands = f"{dest}, {sym}{rel}({reg_name})"
            else:
                line.operands = f"{dest}, {reg_name}, {sym}{rel}"

    def _process_instr(self, instr: capstone.CsInsn, inline=False, hashable=False, referenced=None
                      ) -> str:
        """Takes a capstone instruction and converts it to text"""

        if not isinstance(instr, DummyInstr):
            ret = DisasmLine(instr, instr.mnemonic, instr.op_str)

            if instr.id in labelledBranchInsns:
                self._process_labelled_branch(instr, ret, inline, hashable, referenced)

            if instr.id in conditionalBranchInsns:
                self._process_branch_hint(instr, ret)

            if instr.id in signExtendInsns:
                self._process_signed_immediate(instr, ret)

            if instr.id in upperInsns:
                self._process_upper(instr, ret, inline, hashable, referenced)
            
            if instr.id in lowerInsns or instr.id in storeLoadInsns:
                self._process_lower(instr, ret, inline, hashable, referenced)
            
            if instr.id in renamedInsns:
                ret.mnemonic = renamedInsns[instr.id] + ' '
        else:
            ret = DisasmLine(instr, ("opword" if inline else ".4byte"), f"0x{instr.bytes.hex()}")
        
        return ret.to_txt(self._sym, inline, hashable, referenced)
    
    ########
    # Data #
    ########

    def _process_data(self, addr: int, val: bytes, enable_ref=True) -> str:
        """Takes a word of data and converts it to text"""

        ops = f"0x{val.hex()}"
        if enable_ref:
            if self._rlc.check_jt_at(addr):
                if isinstance(self._bin, LECTReader):
                    target = int.from_bytes(val, 'big', signed=True)
                    jt = self._rlc.get_containing_jumptable(addr)
                    target += jt
                    ops = f"jump_{target:x} - jtbl_{jt:x}"
                else:
                    target = int.from_bytes(val, 'big')
                    ops = f"jump_{target:x}"
            else:
                ref = self._rlc.get_reference_at(addr)
                if ref is not None:
                    assert ref.t == RelocType.NORMAL, f"Bad reloc {addr:x} -> {ref}"
                    ops = self._sym.get_name(ref.target) + ref.format_offs()

        instr = DummyInstr(addr, val)
        return DisasmLine(instr, ".4byte", ops).to_txt(self._sym)

    def _process_unaligned_byte(self, addr: int, val: bytes) -> str:
        """Takes a byte of data and converts it to text"""

        # If starting on a word, check this whole word isn't meant to be a pointer
        if addr & 3 == 0:
            ref = self._rlc.get_reference_at(addr)
            if ref is not None:
                print(f"Warning: reference to {ref.target:x} at {addr:x} ignored, "
                        "data is split below word alignment")
        
        # Split into individual bytes if a non-aligned reference falls within this word
        instr = DummyInstr(addr, val)
        return DisasmLine(instr, ".byte", f"0x{val.hex()}").to_txt(self._sym)

    ###########
    # General #
    ###########

    def _disasm_function(self, addr: int, inline=False, hashable=False, referenced=None) -> str:
        """Disassembles a single function of text"""

        # Get end address
        size = self._sym.get_size(addr)

        # Disassemble
        lines = cs_disasm(addr, self._bin.read(addr, size))

        # Apply fixes and relocations
        return '\n'.join([
            self._process_instr(lines[addr], inline, hashable, referenced)
            for addr in lines
        ])

    def _disasm_data(self, sec: BinarySection, addr: int) -> str:
        """Disassembles a single symbol of data"""

        # Get end address
        end = addr + self._sym.get_size(addr)

        # Trim if required
        if (
            sec.name == ".ctors" and self._ovr.should_trim_ctors() or
            sec.name == ".dtors" and self._ovr.should_trim_dtors()
        ):
            while self._bin.read_word(end - 4) == 0:
                end -= 4

        # Disassemble
        ret = []
        while addr < end:
            if addr & 3 != 0 or end - addr < 4:
                val = self._bin.read(addr, 1)
                ret.append(self._process_unaligned_byte(addr, val))
                addr += 1
            else:
                dat = self._bin.read(addr, 4)
                ret.append(self._process_data(addr, dat))
                addr += 4

        return '\n'.join(ret)

    def _disasm_symbol(self, sec: BinarySection, addr: int, inline=False, hashable=False,
                       referenced=None) -> str:
        """Disassembles a single symbol of assembly or data"""

        # Add align if required
        alignment = self._ovr.get_symbol_align(addr)
        align = f".balign {alignment}\n" if alignment != 0 else ""

        # Add .global and symbol name if required
        name = self._sym.get_name(addr, hashable, True)
        assert name is not None and self._sym.is_global(addr)
        
        suffix = ""
        
        if inline:
            prefix = "nofralloc\n" if sec.type == SectionType.TEXT else ""
        else:
            if sec.name == ".ctors" or sec.name == ".dtors":
                sym_type_dir = ""
            else:
                sym_type = "@function" if sec.type == SectionType.TEXT else "@object"
                sym_type_dir = f"\n.type {name}, {sym_type}"
                
                suffix = f"\n.size {name}, . - {name}\n"
                
            prefix = f"\n.global {name}{sym_type_dir}\n{align}{name}:\n"
        
        if sec.type == SectionType.TEXT:
            return prefix + self._disasm_function(addr, inline, hashable, referenced) + suffix
        else:
            return prefix + self._disasm_data(sec, addr) + suffix

    def _disasm_range(self, sec: BinarySection, start: int, end: int, inline=False,
                      hashable=False, referenced=None) -> str:
        """Disassembles a range of assembly or data"""

        sec.assert_slice_bounds(start, end)
        assert start < end, f"Start address {start:x} after end address {end:x}" 
        assert sec.addr <= start < end <= sec.addr + sec.size, \
            f"Disassembly {start:x}-{end:x} crosses bounds of section {sec.name}"

        if sec.type in (SectionType.DATA, SectionType.BSS):
            assert not inline, "Only text can be disassembled for inline assembly"
            assert not hashable, "Only text can be disassembled for hashing"
 
        addrs = self._sym.get_globals_in_range(start, end)
 
        assert len(addrs) > 0 and addrs[0] == start, f"Expected symbol at {start:x}"
        assert end == (sec.addr + sec.size) or self._sym.is_global(end, True), \
            f"Expected symbol at {start:x}"
 
        return '\n'.join(
            self._disasm_symbol(sec, addr, inline, hashable, referenced)
            for addr in addrs
        )

    ##########
    # Slices #
    ##########

    def slice_to_text(self, section: BinarySection, sl: Slice) -> str:
        """Outputs the disassembly of a slice to text"""

        self._print(f"Disassemble slice {sl.start:x}-{sl.end:x}")

        # TODO: only mkw should need these, add flag & assert otherwise?

        # DEVKITPPC r40+ can give issues with slices not starting with symbols
        if not self._sym.is_global(sl.start, miss_ok=True):
            self._sym.create_slice_label(sl.start, section.type == SectionType.TEXT)

        # Symbol based disassembly needs one at the end
        if not self._sym.is_global(sl.end, miss_ok=True):
            self._sym.create_slice_label(sl.end, section.type == SectionType.TEXT)

        return (
            ".include \"macros.inc\"\n\n" +
            section.get_start_text() +
            section.get_balign_text() +
            self._disasm_range(section, sl.start, sl.end) +
            "\n"
        )
    
    def output_slice(self, path: str, start: int, end: int):
        """Outputs a slice's disassembly to a file"""

        # Make slice
        section = self._bin.find_section_containing(start)
        sl = Slice(start, end, section.name)

        with open(path, 'w') as f:
            f.write(self.slice_to_text(section, sl))

    #############
    # Functions #
    #############

    def function_to_text_with_referenced(
        self, addr: int, inline=False, extra=False, hashable=False, declare_mangled=False
    ) -> Tuple[str, Set[Tuple[int, str]]]:
        """Outputs the disassembly of a single function to text, and all addresses it referenced
        
        Inline changes the output to CW inline asm syntax
        Extra includes referenced jumptables
        Hashable replaces symbol names with incrementing numeric ids in order of reference
        """

        self._print(f"Disassemble function {addr:x}")

        if hashable:
            self._sym.reset_hash_naming()

        # Get function bounds
        start, end = self._sym.get_containing_symbol(addr)
        assert addr == start, f"Expected function at {addr:x}" 

        # Get section
        sec = self._bin.find_section_containing(addr, True)

        # Disassemble and format
        if inline:
            referenced = ReferencedTracker()
        else:
            assert not declare_mangled, "declare_mangled is for inline asm only"
            referenced = None
        ret = [self._disasm_range(sec, start, end, inline, hashable, referenced).lstrip('\n')]

        # Namespaced objects can't be accessed in inline assembly, so their
        # mangled names with extern "C" are used as a workaround
        if declare_mangled:
            for _, name in referenced.get_mangled_referenced():
                ret.insert(0, f"    UNKNOWN_FUNCTION({name});")

        # Add jumptables if wanted
        if extra:
            # Get jumptables
            jumptables = self._rlc.get_referencing_jumptables(start, end)

            if len(jumptables) > 0:
                # Add section declarations
                func_sec = self._bin.find_section_containing(addr).name
                jt_sec = self._bin.find_section_containing(jumptables[0]).name
                ret.insert(0, f".section {func_sec}\n")
                ret.append(f"\n.section {jt_sec}\n")

                # Add jumptables
                for jt in jumptables:
                    # Get jumptable size and name
                    size = self._rlc.get_jumptable_size(jt)
                    jt_sym = self._sym.get_name(jt)
                    
                    # Get targets
                    targets = [
                        f"jump_{self._bin.read_word(i):x}"
                        for i in range(jt, jt + size, 4)
                    ]

                    # Add entry
                    ret.append('\n'.join((
                        f".global {jt_sym}",
                        f"{jt_sym}:",
                        '\n'.join(
                            f"    .4byte {target}"
                            for target in targets
                        )
                    )))
        
        return '\n'.join(ret), None if referenced is None else referenced.get_referenced()
    
    def function_to_text(self, addr: int, inline=False, extra=False, hashable=False,
                         declare_mangled=False) -> str:
        """Outputs the disassembly of a single function to text
        
        Inline changes the output to CW inline asm syntax
        Extra includes referenced jumptables
        Hashable replaces symbol names with incrementing numeric ids in order of reference
        """

        txt, _ = self.function_to_text_with_referenced(addr, inline, extra, hashable, declare_mangled)

        return txt

    def output_function(self, path: str, addr: int, inline: bool, extra: bool):
        """Outputs a function's disassembly to a file"""

        with open(path, 'w') as f:
            f.write(self.function_to_text(addr, inline, extra, False, inline))

    ####################
    # Full disassembly #
    ####################

    def _section_to_txt(self, section: BinarySection) -> str:
        """Outputs the disassembly of a single section to text"""
 
        self._print(f"Disassemble section {section.name}")

        return (
            section.get_start_text() +
            self._disasm_range(section, section.addr, section.addr + section.size)
        )

    def output(self, path: str):
        """Outputs disassembly to a file"""

        with open(path, 'w') as f:
            f.write(
                ".include \"macros.inc\"\n\n" +
                '\n\n'.join([
                    self._section_to_txt(sec)
                    for sec in self._bin.sections
                ])
                + '\n'
            )
    
    ###############
    # C Functions #
    ###############

    def make_function_skeletons(self, start: int, end: int) -> str:
        # Init output
        ret = []

        # Get functions
        funcs = self._sym.get_globals_in_range(start, end)

        for addr in funcs:
            # Get size of function
            size = self._sym.get_size(addr)

            # Add jumptable includes before
            for jt in self._rlc.get_referencing_jumptables(addr, addr + size):
                ret.append(f"#include \"jumptable/{jt:x}.inc\"")

            # Output function dummy
            name = self._sym.get_name(addr)
            ret.append(f"asm UNKNOWN_FUNCTION({name})\n{{\n    #include \"asm/{addr:x}.s\"\n}}\n")

        return '\n'.join(ret)

    ##########
    # C Data #
    ##########

    def data_to_text_with_referenced(self, addr: int, width=4, const=False) -> \
        Tuple[str, Set[Tuple[int, str]]]:

        self._print(f"Disassemble data {addr:x}")

        # Use jumptable function instead if needed
        size = self._sym.get_size(addr)
        if self._rlc.check_jt_at(addr):
            assert addr & 3 == 0 and size == self._rlc.get_jumptable_size(addr), \
                f"Invalid jumptable parameters"
            
            return self.jumptable_to_text_with_referenced(addr)

        # Check alignment requirements
        if addr & 3 != 0 or size & 3 != 0:
            unit = 1
            t = "u8"
        else:
            unit = 4
            t = "u32"

        # Add const if needed
        if const:
            t = f"const {t}"
        
        # Disassemble values
        referenced = ReferencedTracker()
        vals = []
        for p in range(addr, addr+size, unit):
            if unit == 4:
                ref = self._rlc.get_reference_at(p)
                if ref is not None:
                    # Output reference if needed
                    assert ref.t == RelocType.NORMAL, f"Bad reloc {p:x} -> {ref}"
                    assert ref.offs == 0, f"Can't use pointer offsets in C {p:x} -> {ref}"
                    name = self._sym.get_name(ref.target)
                    val = f"(u32)&{name}"
                    referenced.notify(ref.target, name)
                else:
                    # Output raw value otherwise
                    val = f"{self._bin.read_word(p):#010x}"
            else:
                # Warn if ignoring references
                ref = self._rlc.get_reference_at(p)
                if ref is not None:
                    print(f"Warning: reference to {ref.target:x} at {addr:x} ignored, "
                            "data is split below word alignment")

                val = f"{self._bin.read_byte(p):#04x}"

            vals.append(val)

        # Format
        lines = []
        for i in range(0, len(vals), width):
            lines.append('    ' + ', '.join(vals[i:i+width]))
        body = ',\n'.join(lines)
        sym = self._sym.get_name(addr)
        txt = f"{t} {sym}[] = {{\n{body}\n}};"

        return txt, referenced.get_referenced()

    def data_to_text(self, addr: int, width=4, const=False) -> str:
        """Outputs a single data symbol as a C u32 array (or u8 if required)"""

        txt, _ = self.data_to_text_with_referenced(addr, width, const)

        return txt

    def output_jumptable(self, path: str, addr: int):
        """Outputs a jumptable C workaround to a file"""

        with open(path, 'w') as f:
            f.write(self.jumptable_to_text(addr))
    
    def jumptable_to_text_with_referenced(self, addr: int) -> Tuple[str, Set[Tuple[int, str]]]:
        """Outputs a jumptable C workaround and all labels it references"""

        self._print(f"Disassemble jumptable {addr:x}")

        referenced = ReferencedTracker()

        # Get jumptable size and name
        size = self._rlc.get_jumptable_size(addr)
        jt_sym = self._sym.get_name(addr)
         
        # Get targets
        targets = []
        for i in range(addr, addr + size, 4):
            target = self._bin.read_word(i)
            targets.append(target)
            referenced.notify(target, f"jump_{target:x}")

        # For some reason, CW will align pointer arrays to 8 bytes if they have an even length,
        # but doesn't do this for jumptables
        if addr & 7 and len(targets) % 2 == 0:
            final = targets.pop(-1)
            extra_addr = addr + len(targets) * 4
            name = f"jthack_{extra_addr:x}"
            extra = '\n'.join((
                f"__declspec(section \".data\") void (*{name})() = jump_{final:x};",
                "#pragma push",
                "#pragma force_active on",
                f"DUMMY_POINTER({name})",
                "#pragma pop"
            ))
        else:
            extra = ""

        return '\n'.join((
            f"void (*{jt_sym}[])() = {{",
            '\n'.join(
                f"    jump_{target:x},"
                for target in targets
            ),
            "};",
            extra
        )), referenced.get_referenced()

    def jumptable_to_text(self, addr: int) -> str:
        """Outputs a jumptable C workaround"""

        txt, targets = self.jumptable_to_text_with_referenced(addr)

        # Declare labels
        decl = '\n'.join(
            f"void {name}();"
            for target, name in targets
        )

        return '\n'.join((decl, txt))

    def make_data_dummies(self, start: int, end: int, width=8, const=False) -> str:
        # Init output
        ret = []

        # Get symbols
        funcs = self._sym.get_globals_in_range(start, end)

        # Output data dummies
        for addr in funcs:
            ret.append(self.data_to_text(addr, width, const))

        return '\n'.join(ret)
    
    def output_data_dummies(self, path: str, start: int, end: int, width=8):
        with open(path, 'w') as f:
            f.write(self.make_data_dummies(start, end, width))

    
    #########################
    # Source File Skeletons # 
    #########################

    def output_skeleton(self, path: str, src: Source, include_data=False, width=8):
        # Initialise output
        text_out = []
        data_out = []

        # Add sections
        for sec_name, sl in src.slices.items():
            # Get section
            sec = self._bin.get_section_by_name(sec_name)

            # Check type
            if sec.type == SectionType.TEXT:
                text_out.append(self.make_function_skeletons(sl.start, sl.end))
            elif include_data:
                data_out.append(f"// {sec_name}")
                # TODO: make this a BinarySection property or something
                const = sec_name in (".rodata", ".sdata2", ".sbss2")
                data_out.append(self.make_data_dummies(sl.start, sl.end, width, const))

        with open(path, 'w') as f:
            f.write('\n\n'.join(data_out + text_out))

    ###########
    # Hashing #
    ###########

    def function_to_hash(self, addr: int) -> str:
        txt = self.function_to_text(addr, hashable=True)
        return sha1(txt.encode()).hexdigest()    

    def _section_to_hashes(self, section: BinarySection, no_addrs=False) -> str:
        """Outputs the hashes of a single section to text"""
 
        self._print(f"Hash section {section.name}")

        if section.name == ".init":
            rci = self._bin.get_rom_copy_info()
        else:
            rci = None
        if rci is not None:
            end = rci
        else:
            end = section.addr + section.size
        
        funcs = self._sym.get_globals_in_range(section.addr, end)

        if not no_addrs:
            return json.dumps(
                {hex(addr) : self.function_to_hash(addr) for addr in funcs},
                indent=4
            )
        else:
            return json.dumps(
                [self.function_to_hash(addr) for addr in funcs],
                indent=4
            )

    def output_hashes(self, path: str, no_addrs=False):
        """Outputs hashes to a file"""

        with open(path, 'w') as f:
            f.write(
                '\n'.join([
                    sec.name + '\n' + self._section_to_hashes(sec, no_addrs)
                    for sec in self._bin.sections
                    if sec.type == SectionType.TEXT
                ])
                + '\n'
            )
