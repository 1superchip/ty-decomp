"""
Generates a float/double order dummy for a file
"""

from argparse import ArgumentParser

from ppcdis import load_binary_yml, order_floats

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Generate a float/double order workaround")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("start_addr", type=hex_int, help="Floats start address")
    parser.add_argument("end_addr", type=hex_int, help="Floats end address")
    parser.add_argument("out_path", type=str, help="Text output path")
    parser.add_argument("--double", action='store_true', help="Double mode")
    parser.add_argument("--sda", action='store_true', help="Signals floats should be in sdata2")
    parser.add_argument("--asm", action='store_true', help="Declare the floats in inline asm")
    args = parser.parse_args()

    binary = load_binary_yml(args.binary_path)

    txt = order_floats(binary, args.start_addr, args.end_addr, args.asm, args.sda, args.double)

    with open(args.out_path, 'w', encoding="shift-jis") as f:
        f.write(txt)
