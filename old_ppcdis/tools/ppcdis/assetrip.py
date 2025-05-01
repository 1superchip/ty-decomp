"""
Rips an asset from a binary
"""

from argparse import ArgumentParser

from ppcdis import load_binary_yml, rip_asset

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Rip an asset from a binary")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("start_addr", type=hex_int, help="Asset start address")
    parser.add_argument("end_addr", type=hex_int, help="Asset end address")
    parser.add_argument("out_path", type=str, help="Binary output path")
    args = parser.parse_args()

    # Read asset
    binary = load_binary_yml(args.binary_path)
    dat = rip_asset(binary, args.start_addr, args.end_addr)

    # Output
    with open(args.out_path, 'wb') as f:
        f.write(dat)
