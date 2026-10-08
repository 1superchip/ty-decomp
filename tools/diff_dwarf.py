
from typing import Dict, List, Tuple

import subprocess

import os

script_dir = os.path.dirname(os.path.realpath(__file__))
root_dir = os.path.abspath(os.path.join(script_dir, ".."))

ORIG_ELF = os.path.join(root_dir, "orig", "GTYE69", "files", "TY_REL.elf")

BUILT_ELF = os.path.join(root_dir, "build", "GTYE69", "main.elf")

DTK = os.path.join(root_dir, "build", "tools", "dtk.exe")

def dump_dwarf(elf_path: str) -> str:
    if not os.path.exists(elf_path):
        return ""

    command = [DTK, "dwarf", "dump", "--no-color", "--include-erased", elf_path]

    output = subprocess.run(command, capture_output=True, text=True, shell=True)

    if output:
        return output.stdout.splitlines()

    return ""

import difflib
def compare_files(lines1, lines2):
    diff = difflib.unified_diff(lines1, lines2)
    return '\n'.join(diff)

def main():
    orig_dwarf = dump_dwarf(ORIG_ELF)
    built_dwarf = dump_dwarf(BUILT_ELF)

    print(compare_files(orig_dwarf, built_dwarf))

main()
