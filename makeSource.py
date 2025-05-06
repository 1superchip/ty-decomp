
# this file is meant to create a C file in the source directory
# given a filepath

import common as c
import os
import re

import yaml
try:
    from yaml import CLoader as Loader
except ImportError:
    from yaml import Loader

def load_from_yaml(path: str, default=None):
    """Loads an object from a yaml file"""

    if default is None:
        default = {}
    with open(path) as f:
        ret = yaml.load(f.read(), Loader)
        if ret is None:
            ret = default
        return ret

import argparse

parser = argparse.ArgumentParser(description="Pass filepath from dol_slices.yml")
parser.add_argument('filepath')
args = parser.parse_args()

dolSlices = load_from_yaml(c.DOL_SLICES)

commandParts = [
    # "python "
    c.DISASSEMBLER,
    c.DOL_YML,
    c.DOL_LABELS,
    c.DOL_RELOCS,
    "asm/" + args.filepath.split('/')[-1].replace(".cpp", ".s"),
    f"-s {hex(dolSlices[args.filepath]['.text'][0])} {hex(dolSlices[args.filepath]['.text'][1])}",
    f"-m {c.GAME_SYMBOLS}",
    f"-o {c.DISASM_OVERRIDES}"
]

executeString = ' '.join(commandParts)

# print(executeString)
c.get_cmd_stdout(executeString)

funcNameRe = re.compile(r"((\.global )(.+))")
funcNameRejectJumpRe = re.compile(r"((\.global (?!(?:jump.+)$))(.+))")
print(c.DOL_SRCDIR + '/' + args.filepath)

if os.path.exists(c.DOL_SRCDIR + '/' + args.filepath) == True:
    print("Cannot generate existing file!")
    exit()

with open(c.DOL_SRCDIR + '/' + args.filepath, "w+") as fd:
    with open("asm/" + args.filepath.split('/')[-1].replace(".cpp", ".s"), "rt+") as srcFd:
        asmText = srcFd.readlines()
        asmText.remove(".include \"macros.inc\"\n")
        asmText.remove(".section .text\n")
        regexedText = []
        regexedText.append("// Assembly file in " + "asm/" + args.filepath.split('/')[-1].replace(".cpp", ".s"))
        foundExtern = False
        for line in asmText:
            replaceStr = "extern \"C\" void \g<3>(void) {}\n// \g<1>"
            newLine = re.sub(funcNameRejectJumpRe, replaceStr, line)
            if "extern" not in newLine and newLine != '\n':
                newLine = '// ' + newLine
            regexedText.append(newLine)
        fd.writelines(regexedText)