"""
Add forceactive entries to an LCF file from relextern output
"""

from argparse import ArgumentParser

from ppcdis import apply_forceactive, load_binary_yml

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Add relextern labels to forceactive in an LCF file")
    parser.add_argument("lcf_path", type=str, help="LCF input path")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("labels_path", type=str, help="Labels pickle input path")
    parser.add_argument("symbols_path", type=str, help="Symbols yml input path")
    parser.add_argument("externs_path", type=str, help="Extern abels pickle input path")
    parser.add_argument("out_path", type=str, help="LCF output path")
    args = parser.parse_args()

    with open(args.lcf_path) as f:
        txt = f.read()

    binary = load_binary_yml(args.binary_path)
    txt = apply_forceactive(binary, args.symbols_path, args.labels_path, args.externs_path, txt)

    with open(args.out_path, 'w') as f:
        f.write(txt)
