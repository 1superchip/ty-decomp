"""
Converts an ELF to a REL file
"""

from argparse import ArgumentParser

from ppcdis import RelLinker

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Convert ELF to REL")
    parser.add_argument("rel_input", type=str, help="REL ELF input path")
    parser.add_argument("dol_input", type=str, help="DOL ELF input path")
    parser.add_argument("ext_rels", type=str, nargs='*',
                        help="External REL ELFs to link to, format is a list of module id, path")
    parser.add_argument("-o", "--out", type=str, help="REL output path")
    parser.add_argument("-m", "--module-id", type=int, default=1, help="Output module ID")
    parser.add_argument("-n", "--num-sections", type=int, help="Forced number of sections")
    parser.add_argument("--name-offset", type=hex_int, default=0, help="Forced name offset")
    parser.add_argument("--name-size", type=hex_int, default=0, help="Forced name size")
    parser.add_argument("-r", "--base-rel", type=str, help="Base rel yml for sym defs")
    parser.add_argument("-i", "--ignore-missing", action="store_true",
                        help="For debugging, don't error when a symbol is not found")
    parser.add_argument("-s", "--ignore-sections", nargs='+', default=[],
                        help="PLF sections to ignore")
    args = parser.parse_args()

    dol_path = args.dol_input

    in_path = args.rel_input

    if args.out is None:
        if in_path.endswith(".plf"):
            out_path = in_path.replace(".plf", ".rel")
        else:
            out_path = in_path + ".rel"
    else:
        out_path = args.out
    
    module_id = args.module_id

    num_sections = args.num_sections

    linker = RelLinker(dol_path, in_path, module_id, args.ext_rels, num_sections,
                       args.name_offset, args.name_size, args.base_rel, args.ignore_missing, args.ignore_sections)
    linker.link_rel(out_path)
