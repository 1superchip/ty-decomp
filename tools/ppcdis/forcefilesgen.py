"""
Add forcefiles entries to an LCF file
"""

from argparse import ArgumentParser

from ppcdis import apply_forcefiles

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Add forcefiles entries to an LCF file")
    parser.add_argument("lcf_path", type=str, help="LCF input path")
    parser.add_argument("out_path", type=str, help="LCF output path")
    parser.add_argument("forcefiles", type=str, nargs='*', help="LCF output path")
    args = parser.parse_args()

    with open(args.lcf_path) as f:
        txt = f.read()

    txt = apply_forcefiles(txt, args.forcefiles)

    with open(args.out_path, 'w') as f:
        f.write(txt)
