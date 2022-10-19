"""
Generates a string order dummy for a file
"""

from argparse import ArgumentParser

from ppcdis import load_binary_yml, order_strings

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Generate a string order workaround")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("start_addr", type=hex_int, help="Stringbase start address")
    parser.add_argument("end_addr", type=hex_int, help="Stringbase end address")
    parser.add_argument("out_path", type=str, help="Text output path")
    parser.add_argument("--enc", "-e", type=str, default="utf8",
                        help="String & output file encoding")
    parser.add_argument("--pool", "-p", action="store_true",
                        help="Expect '-str pool' format strings")
    args = parser.parse_args()

    binary = load_binary_yml(args.binary_path)

    txt = order_strings(binary, args.start_addr, args.end_addr, args.pool, args.enc)

    with open(args.out_path, 'w', encoding=args.enc) as f:
        f.write(txt)
