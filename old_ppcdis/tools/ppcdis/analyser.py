"""
Analyser for initial project creation
"""

from argparse import ArgumentParser

from ppcdis import Analyser, load_binary_yml

if __name__ == "__main__":
    hex_int = lambda s: int(s, 16)
    parser = ArgumentParser(description="Analyse a binary for its labels and relocations")
    parser.add_argument("binary_path", type=str, help="Binary input yml path")
    parser.add_argument("labels_path", type=str, help="Labels pickle output path")
    parser.add_argument("relocs_path", type=str, help="Relocs pickle output path")
    parser.add_argument("-l", "--extra-labels", nargs='+', help="List of extra label paths")
    parser.add_argument("-o", "--overrides", help="Overrides yml path")
    parser.add_argument("--thorough", action="store_true", help="Thorough pointer following")
    parser.add_argument("-q", "--quiet", action="store_true", help="Don't print log")
    args = parser.parse_args()

    binary = load_binary_yml(args.binary_path)

    anl = Analyser(binary, args.overrides, args.extra_labels, args.thorough, args.quiet)
    anl.output(args.labels_path, args.relocs_path)
