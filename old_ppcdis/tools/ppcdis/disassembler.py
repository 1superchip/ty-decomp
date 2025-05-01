"""
Disassembler for assembly code (re)generation
"""

from argparse import ArgumentParser

from ppcdis import Disassembler, load_binary_yml

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Disassemble a binary")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("labels_path", type=str, help="Labels pickle input path")
    parser.add_argument("relocs_path", type=str, help="Relocs pickle input path")
    parser.add_argument("output_paths", type=str, nargs='+', help="Disassembly output path(s)")
    parser.add_argument("-m", "--symbol-map-path", type=str, help="Symbol map input path")
    parser.add_argument("-o", "--overrides", help="Overrides yml path")
    parser.add_argument("-s", "--slice", type=hex_int, nargs='+',
                        help="Disassemble slices (give start & end pairs)")
    parser.add_argument("-j", "--jumptable", type=hex_int, nargs='+',
                        help="Generate jumptable workarounds (give starts)")
    parser.add_argument("-f", "--function", type=hex_int, nargs='+',
                        help="Disassemble individual functions (give starts)")
    parser.add_argument("--hash", action="store_true", help="Output hashes of all functions")
    parser.add_argument("-i", "--inline", action="store_true",
                        help="For --function, disassemble as CW inline asm")
    parser.add_argument("-e", "--extra", action="store_true",
                        help="For --function, include referenced jumptables")
    parser.add_argument("-n", "--source-name", type=str,
                        help="For --function or --jumptable, source C/C++ file name")
    parser.add_argument("-q", "--quiet", action="store_true", help="Don't print log")
    parser.add_argument("--no-addr", action="store_true",
                        help="For --hash, don't include addresses in output file")
    parser.add_argument("-d", "--data-dummy", type=hex_int, nargs='+',
                        help="Generate source data dummmies (give start & end pairs)")
    args = parser.parse_args()

    incompatibles = (args.slice, args.function, args.jumptable, args.hash)
    if len(incompatibles) - (incompatibles.count(None) + incompatibles.count(False)) > 1:
        assert 0, "Invalid combination of --slice, --function, --jumptable and --hash"
    if args.inline:
        assert args.function, "Inline mode can only be used with --function"
        assert not args.extra, "Inline mode can't be used with --extra"
    if args.source_name is not None:
        assert any((args.function, args.jumptable, args.data_dummy)), \
            "Source name can only be used with --function, --jumptable or --data-dummy"
    if args.no_addr:
        assert args.hash, "No addr can only be used with hash mode"

    binary = load_binary_yml(args.binary_path)

    dis = Disassembler(binary, args.labels_path, args.relocs_path, args.symbol_map_path,
                       args.overrides, args.source_name, args.quiet)
    if args.slice is not None:
        assert len(args.slice) % 2 == 0, "Missisg slice end address"
        assert len(args.slice) // 2 == len(args.output_paths), \
            "Number of slices must equal number of output paths"
        for path, start, end in zip(args.output_paths, *[iter(args.slice)]*2):
            dis.output_slice(path, start, end)
    elif args.function is not None:
        assert len(args.function) == len(args.output_paths), \
            "Number of function addresses must equal number of output paths"
        for path, addr in zip(args.output_paths, args.function):
            dis.output_function(path, addr, args.inline, args.extra)
    elif args.jumptable is not None:
        assert len(args.jumptable) == len(args.output_paths), \
            "Number of jumptable addresses must equal number of output paths"
        for path, addr in zip(args.output_paths, args.jumptable):
            dis.output_jumptable(path, addr)
    elif args.hash:
        assert len(args.output_paths) == 1, "--hash only takes 1 output"
        dis.output_hashes(args.output_paths[0], args.no_addr)
    elif args.data_dummy is not None:
        assert len(args.data_dummy) % 2 == 0, "Missing data dummy end address"
        assert len(args.data_dummy) // 2 == len(args.output_paths), \
            "Number of slices must equal number of output paths"
        for path, start, end in zip(args.output_paths, *[iter(args.data_dummy)]*2):
            dis.output_data_dummies(path, start, end)
    else:
        assert len(args.output_paths) == 1, "Full disassembly only takes 1 output"
        dis.output(args.output_paths[0])
