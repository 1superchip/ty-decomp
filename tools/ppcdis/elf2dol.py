"""
Converts an ELF to a DOL file
"""

from argparse import ArgumentParser

from ppcdis import elf_to_dol

if __name__ == "__main__":
    parser = ArgumentParser(description="Convert ELF to DOL")
    parser.add_argument("input", type=str, help="ELF input path")
    parser.add_argument("-o", "--out", type=str, help="DOL output path")
    args = parser.parse_args()

    in_path = args.input

    if args.out is None:
        if in_path.endswith(".elf"):
            out_path = in_path.replace(".elf", ".dol")
        else:
            out_path = in_path + ".dol"
    else:
        out_path = args.out

    elf_to_dol(in_path, out_path)
