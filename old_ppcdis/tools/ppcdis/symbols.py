"""
Helpers for address naming
"""

from argparse import ArgumentParser

from ppcdis import dump_to_json_str, load_binary_yml, load_from_yaml, lookup, reverse_lookup

if __name__ == "__main__":
    parser = ArgumentParser(description="Query a symbols yml file")
    hex_int = lambda s: int(s, 16)
    parser.add_argument("symbol_map_path", type=str, help="Symbol map input path")
    parser.add_argument("--get-name", type=hex_int, help="Get symbol name for address")
    parser.add_argument("--get-addr", type=str, help="Get address for symbol name")
    parser.add_argument("-b", "--binary", type=str, help="Binary input yml path")
    parser.add_argument("-n", "--source-name", type=str, help="Source C/C++ file name")
    parser.add_argument("-r", "--readable", action="store_true",
                        help="Output as text rather than json")
    args = parser.parse_args()

    assert (args.get_name, args.get_addr).count(None) == 1, \
           "One of --get-name and --get-addr is required"

    # Load binary
    binary = load_binary_yml(args.binary).name if args.binary is not None else None

    # Load symbols
    yml = load_from_yaml(args.symbol_map_path)

    if args.get_addr is not None:
        addr = reverse_lookup(yml, binary, args.source_name, args.get_addr)
        if args.readable:
            assert addr is not None, "Not found"
            print(hex(addr))
        else:
            print(dump_to_json_str(addr))
    else:
        name = lookup(yml, binary, args.source_name, args.get_name)
        if args.readable:
            assert name is not None, "Not found"
            print(name)
        else:
            print(dump_to_json_str(name))
