"""
Utility for progress calculation
"""

from argparse import ArgumentParser

from ppcdis import calc_progress_info, dump_to_json_str, load_binary_yml, load_slice_yaml

PROGRESS_JSON_VERSION = 2

if __name__ == "__main__":
    parser = ArgumentParser(description="Calculate progress info for a project")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("labels_path", type=str, help="Labels pickle input path")
    parser.add_argument("slices_path", type=str, help="Slices yml input path")
    args = parser.parse_args()

    # Load data
    binary = load_binary_yml(args.binary_path)
    sources = load_slice_yaml(args.slices_path, binary.sections)

    decomp_slices_sizes, total_sizes, symbol_sizes = calc_progress_info(binary, sources,
                                                                        args.labels_path)

    # Output
    print(dump_to_json_str({
        # Protocol version
        "version": PROGRESS_JSON_VERSION,
        # Size of all slices coming from C code in each section
        "decomp_slices_sizes": decomp_slices_sizes,
        # Size of all slices in each section
        "total_sizes": total_sizes,
        # Size of each symbol
        "symbol_sizes": symbol_sizes
    }))
