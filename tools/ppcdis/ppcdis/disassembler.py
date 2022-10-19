"""
Disassembler for assembly code (re)generation
"""

from dataclasses import dataclass
from hashlib import sha1
import json
from typing import Dict, List, Set, Tuple

import capstone
from capstone.ppc import *

from .analyser import Reloc, RelocType
from .binarybase import BinaryReader, BinarySection, SectionType
from .binarylect import LECTReader
from .csutil import DummyInstr, sign_half, cs_disasm 
from .instrcats import (labelledBranchInsns, conditionalBranchInsns, upperInsns, lowerInsns,
                       signExtendInsns, storeLoadInsns, renamedInsns)
from .overrides import OverrideManager
from .slices import Slice
from .symbols import SymbolGetter, has_bad_chars, is_mangled
from .fileutil import load_from_pickle

class DisassemblyOverrideManager(OverrideManager):
    """Disassembly category OverrideManager"""

    def load_yml(self, yml: Dict):
        """Loads data from a disassembly overrides yaml file"""

        # Load categories
        self._mfr = self._make_ranges(yml.get("manual_sdata2_ranges", []))
        self._gmf = yml.get("global_manual_floats", False)
        self._tc = yml.get("trim_ctors", False)
        self._td = yml.get("trim_dtors", False)

    def is_manual_sdata2(self, addr: int):
        """Checks if the symbol at an address should be made relative to r2 for manual handling
        in inline assembly"""

        return self._gmf or self._check_range(self._mfr, addr)
    
    def should_trim_ctors(self):
        """Checks if terminating zeros should be removed from .ctors disassembly"""

        return self._tc

    def should_trim_dtors(self):
        """Checks if terminating zeros should be removed from .ctors disassembly"""

        return self._td

class RelocGetter:
    """Class to handle relocation lookup"""

    def __init__(self, binary: BinaryReader, sym: SymbolGetter, reloc_path: str):
        # Backup binary reference
        self._bin = binary

        # Load from file
        dat = load_from_pickle(reloc_path)

        # Parse references
        self._refs = {}
        for addr, ref in dat["references"].items():
            self._refs[addr] = Reloc(RelocType(ref["type"]), ref["target"], ref["offset"])

        # Parse jump tables
        self._jt_sizes = {}
        self._jt = {}
        for addr, jt in dat["jumptables"].items():
            # Save size
            self._jt_sizes[addr] = jt["size"]

            # Get all jumps
            entries = binary.read_word_array(addr, jt["size"] // 4)

            # Update dicts
            for i, target in enumerate(entries):
                # Create reloc for jump table entry
                self._jt[addr + i * 4] = addr

                # Create target label
                sym.notify_jt_target(target)
    
    def get_jumptable_size(self, addr: int) -> int:
        """Gets the size of a jumptable in bytes, or None if it's not known"""

        return self._jt_sizes.get(addr)

    def get_reference_at(self, addr: int) -> Reloc:
        """Checks the reference info from an address, if any"""

        return self._refs.get(addr)
    
    def check_jt_at(self, addr: int) -> bool:
        """Returns whether an address is in a jump table"""

        return addr in self._jt
    
    def get_containing_jumptable(self, addr: int) -> int:
        """Returns the address of the jumptable containing an address"""

        return self._jt[addr]

    def get_referencing_jumptables(self, start: int, end: int) -> List[int]:
        """Gets the jumptables referencing a function"""

        ret = []
        for addr in self._jt_sizes:
            # Get first target
            target = self._bin.read_word(addr)

            # Save if referencing this function
            if start <= target < end:
                ret.append(addr)
        
        return ret

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

        # Add .global and symbol name if required
        prefix = []
        name = sym.get_name(self.instr.address, hashable, True)
        if name is not None:
            if sym.is_global(self.instr.address):
                # Don't include function label in inline asm
                if not inline:
                    prefix.append(f"\n.global {name}\n{name}:")
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
            elif referenced is not None:
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
                    assert self._bin.addr_is_local(ref.target), f"SDA reference outside of binary " \
                        f"at {instr.address:x} with {ref} (probably code with r2 overwritten, " \
                        f"if so then add a blocked_pointers override for 0x{instr.address:x})"
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

    def _process_unaligned_data(self, addr: int, dat: bytes, unaligned: List[int]) -> str:
        """Takes bytes of data and converts them to text"""

        # If starting on a word, check this whole word isn't meant to be a pointer
        if addr & 3 == 0:
            ref = self._rlc.get_reference_at(addr)
            if ref is not None:
                print(f"Warning: reference to {ref.target:x} at {addr:x} ignored, "
                        "data is split below word alignment")
        
        # Split into individual bytes if a non-aligned reference falls within this word
        ret = []
        for i in range(len(dat)):
            instr = DummyInstr(addr + i, dat[i:i+1])
            ret.append(DisasmLine(instr, ".byte", hex(dat[i])).to_txt(self._sym))
            if unaligned[0] == addr + i:
                unaligned.pop(0)

        return ret

    ###########
    # General #
    ###########

    def _disasm_range(self, sec: BinarySection, start: int, end: int, inline=False,
                      hashable=False, referenced=None) -> str:
        """Disassembles a range of assembly or data"""

        sec.assert_slice_bounds(start, end)
        assert start < end, f"Start address {start:x} after end address {end:x}" 
        assert sec.addr <= start < end <= sec.addr + sec.size, \
            f"Disassembly {start:x}-{end:x} crosses bounds of section {sec.name}"

        if sec.type == SectionType.TEXT:
            # Disassemble
            lines = cs_disasm(start, self._bin.read(start, end - start), self._quiet)

            # Apply fixes and relocations
            nofralloc = "nofralloc\n" if inline else ""
            return nofralloc + '\n'.join([
                self._process_instr(lines[addr], inline, hashable, referenced)
                for addr in lines
            ])

        elif sec.type in (SectionType.DATA, SectionType.BSS):
            assert not inline, "Only text can be disassembled for inline assembly"
            assert not hashable, "Only text can be disassembled for hashing"

            # Trim if required
            if (
                sec.name == ".ctors" and self._ovr.should_trim_ctors() or
                sec.name == ".dtors" and self._ovr.should_trim_dtors()
            ):
                while self._bin.read_word(end - 4) == 0:
                    end -= 4

            # Setup
            ret = []
            unaligned = self._sym.get_unaligned_in(start, end)
            unaligned.append(0xffff_ffff) # Hack so that [0] can always be read

            # The ppcdis slice system doesn't support unaligned slices, but other projects using
            # the python api require it, so the disassembler has support for it.
            # The balign of all data sections must be set to 0 if using this

            # Disassemble starting unaligned data
            if start & 3 != 0:
                # Calculate length
                rounded = (start + 3) & ~3
                size = rounded - start

                # Disassemble
                dat = self._bin.read(start, size)
                ret.extend(self._process_unaligned_data(start, dat, unaligned))

                # Move to aligned start
                start = rounded
            
            # Prepare end unaligned data
            if end & 3 != 0:
                rounded = end & ~3
                end_size = end - rounded
                end = rounded
            else:
                end_size = 0

            # Disassemble aligned data
            for p in range(start, end, 4):
                dat = self._bin.read(p, 4)
                if unaligned[0] < p + 4 or p + 4 > end:
                    ret.extend(self._process_unaligned_data(p, dat, unaligned))
                else:
                    ret.append(self._process_data(p, dat))
            
            # Disassemble end unaligned data
            if end_size > 0:
                dat = self._bin.read(end, end_size)
                ret.extend(self._process_unaligned_data(end, dat, unaligned))

            return '\n'.join(ret)

    ##########
    # Slices #
    ##########

    def slice_to_text(self, section: BinarySection, sl: Slice) -> str:
        """Outputs the disassembly of a slice to text"""

        self._print(f"Disassemble slice {sl.start:x}-{sl.end:x}")

        # DEVKITPPC r40+ can give issues with slices not starting with symbols
        if self._sym.get_name(sl.start, miss_ok=True) is None:
            self._sym.create_slice_label(sl.start)

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
        start, end = self._sym.get_containing_function(addr)
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
    
    ##############
    # Jumptables #
    ##############

    def output_jumptable(self, path: str, addr: int):
        """Outputs a jumptable C workaround to a file"""

        with open(path, 'w') as f:
            f.write(self.jumptable_to_text(addr))
    
    def jumptable_to_text(self, addr: int) -> str:
        """Outputs a jumptable C workaround to a text"""

        self._print(f"Disassemble jumptable {addr:x}")

        # Get jumptable size and name
        size = self._rlc.get_jumptable_size(addr)
        jt_sym = self._sym.get_name(addr)
         
        # Get targets
        targets = [
            self._bin.read_word(i)
            for i in range(addr, addr + size, 4)
        ]

        # Declare labels
        decl = '\n'.join(
            f"void jump_{target:x}();"
            for target in targets
        )

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
            decl,
            f"void (*{jt_sym}[])() = {{",
            '\n'.join(
                f"    jump_{target:x},"
                for target in targets
            ),
            "};",
            extra
        ))

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
        
        funcs = self._sym.get_functions_in_range(section.addr, end)

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
