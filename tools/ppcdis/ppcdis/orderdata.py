"""
Helpers to generate data ordering includes
"""

from .binarybase import BinaryReader
from .binaryrel import RelReader

def order_floats(binary: BinaryReader, start_addr: int, end_addr: int, use_asm=False,
                 use_sda=False, double=False):
    """Generates a float/double order dummy for a file"""

    if double:
        func = binary.read_double
        action = "lfd f1, {}" if use_asm else "__dummy_double({})"
        name = f"order_doubles_{start_addr:x}"
        size = 8
        t = "f64"
    else:
        func = binary.read_float
        action = "lfs f1, {}" if use_asm else "__dummy_float({})"
        name = f"order_floats_{start_addr:x}"
        size = 4
        t = "f32"

    asm = "asm " if use_asm else ""
    sda = f" - 0x{binary.r2:x}" if use_sda else ""

    floats = [(func(addr), addr) for addr in range(start_addr, end_addr, size)]

    if isinstance(binary, RelReader):
        at = '\n'.join((
            "extern {t} {lab};",
            "REL_SYMBOL_AT({lab}, 0x{addr:x})"
        ))
    else:
        at = "const {t} {lab} : 0x{addr:x}{sda};"

    return '\n'.join((
        "#ifndef SHIFTABLE",
        f"void {name}();",
        "#pragma push",
        "#pragma force_active on",
        f"{asm}void FORCESTRIP {name}() {{",
        '\n'.join(
            f"    {action.format(fl)};"
            for fl, addr in floats
        ),
        "}",
        "#pragma pop",
        '\n'.join(
            at.format(t=t, lab=f"{binary.data_prefix}{addr:x}", addr=addr, sda=sda)
            for fl, addr in floats
        ),
        "#else",
        '\n'.join(
            f"static const {t} {binary.data_prefix}{addr:x} = {fl};"
            for fl, addr in floats
        ),
        "#endif\n"
    ))

def order_strings(binary: BinaryReader, start_addr: int, end_addr: int, pool=False, enc="utf8"):
    """Generates a string order dummy for a file"""

    curStr = bytearray()
    strs = []
    addrs = [start_addr] if pool else []
    for addr in range(start_addr, end_addr):
        if not pool and len(curStr) == 0:
            if (addr & 3) != 0:
                continue
            addrs.append(addr)
        c = binary.read_byte(addr)
        if c != 0:
            if c == ord('\n'):
                curStr.extend(b'\\n')
            elif c == ord('\t'):
                curStr.extend(b'\\t')
            elif c == ord('"'):
                curStr.extend(b'\\"')
            elif c == ord('\\'):
                curStr.extend(b'\\\\')
            else:
                curStr.append(c)
        else:
            strs.append(curStr.decode(enc))
            curStr = bytearray()
    assert len(curStr) == 0, "Non-terminating string at end"

    func = f"order_strings_{start_addr:x}"

    if isinstance(binary, RelReader):
        fmt = '\n'.join((
            "extern char {lab}[];",
            "REL_SYMBOL_AT({lab}, 0x{addr:x})"
        ))
    else:
        fmt = "char {lab}[] : 0x{addr:x};"
    matching_at = '\n'.join([
        fmt.format(lab=f"{binary.data_prefix}{addr:x}", addr=addr)
        for addr in addrs
    ])

    if pool:
        shiftable_at = '\n'.join((
            f"static char {binary.data_prefix}{start_addr:x}[] = {{",
            '\n'.join(
                f"    \"{str}\\0\""
                for str in strs
            )
        ))
    else:
        shiftable_at = '\n'.join(
            f"static char {binary.data_prefix}{addr:x}[] = \"{str}\";"
            for addr, str in zip(addrs, strs)
        )
    
    return '\n'.join((
        "#ifndef SHIFTABLE",
        matching_at,
        f"void {func}();",
        "#pragma push",
        "#pragma force_active on",
        f"void FORCESTRIP {func}() {{",
        '\n'.join((
            f"    __dummy_str(\"{str}\");"
            for str in strs
        )),
        "}",
        "#pragma pop",
        "#else",
        shiftable_at,
        "};",
        "#endif\n"
    ))
