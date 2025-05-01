"""
Helpers for disassembly splitting
"""

from argparse import ArgumentParser

from ppcdis import (dump_to_json_str, find_containing_source, load_binary_yml, load_slice_yaml,
                    order_sources)

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Query a slice yml")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("slices_path", type=str, help="Slices yml input path")
    parser.add_argument("-o", "--order-sources", action="store_true",
                        help="Output the ordered source files in json")
    parser.add_argument("-c", "--containing", type=hex_int,
                        help="Output the source containing an address")
    parser.add_argument("-p", "--base_path", type=str, default='',
                        help="Base path to add to source paths in yml")
    args = parser.parse_args()

    # Load slices
    binary = load_binary_yml(args.binary_path)
    sources = load_slice_yaml(args.slices_path, binary.sections, args.base_path)

    if args.order_sources:
        assert args.containing is None, "Order sources mode and containing mode are incompatible"

        # Output source order
        sources = order_sources(sources)
        print(dump_to_json_str(sources))

    elif args.containing is not None:
        assert not args.order_sources, "Order sources mode and containing mode are incompatible"

        sl = find_containing_source(sources, args.containing)
        print(dump_to_json_str(sl))

    else:
        assert 0, "Either --order-sources or --containing must be used, see -h for more"
