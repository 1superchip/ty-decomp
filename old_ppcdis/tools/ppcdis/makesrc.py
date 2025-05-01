"""
Disassembler for assembly code (re)generation
"""

from argparse import ArgumentParser

from ppcdis import Disassembler, load_binary_yml
from ppcdis.slices import load_slice_yaml

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Disassemble a binary")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("labels_path", type=str, help="Labels pickle input path")
    parser.add_argument("relocs_path", type=str, help="Relocs pickle input path")
    parser.add_argument("slices_path", type=str, help="Slices yml input path")
    parser.add_argument("source_name", type=str, help="Source file name")
    parser.add_argument("-m", "--symbol-map-path", type=str, help="Symbol map input path")
    parser.add_argument("-o", "--overrides", help="Overrides yml path")
    parser.add_argument("-q", "--quiet", action="store_true", help="Don't print log")
    parser.add_argument("-d", "--data", action="store_true", help="Include data")
    parser.add_argument("-p", "--base_path", type=str, default='',
                        help="Base path to add to source paths in yml")
    args = parser.parse_args()

    binary = load_binary_yml(args.binary_path)
    sources = load_slice_yaml(args.slices_path, binary.sections, args.base_path)

    src = None
    for s in sources:
        if s.source == args.source_name:
            src = s
            break
    assert src is not None, f"Slices for {args.source_name} not found"

    dis = Disassembler(binary, args.labels_path, args.relocs_path, args.symbol_map_path,
                       args.overrides, args.source_name, args.quiet)
    dis.output_skeleton(args.source_name, src, args.data)
