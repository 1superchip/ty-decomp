# ppcdis

GC/Wii PowerPC disassembly & decompilation tools.

These tools are focused on automatically generating as much as possible, and doing it in a way that can be regenerated at any time. This makes things more future-proof (for example, projects have often had to go back and do a lot of manual labelling when adding new binaries like rels, whereas that's handled automatically here), reduces bloat in the github repo, and reduces manual editing (which, in the case of undecompiled assembly, can be a big time saver - renaming symbols, splitting files, and correcting pointers for shiftability can be very tedious in most existing setups, whereas they're just quick yml edits here). The pointer tracking in disassembly is also more in-depth than that of doldisasm.py, so more of the work towards shiftability should be done already.

**These tools in an early state and therefore large changes may still happen. No backwards compatibility will be guaranteed.**

See [TOOLS.md](TOOLS.md) for information on each tool. (Documentation is currently pretty lacking, so feel free to reach out to me for help)

See the [Super Paper Mario Decompilation](https://github.com/SeekyCt/spm-decomp) for an example of a project using these tools.

## APIs

- The main API for these tools is the command line API
- A python API is exposed in the `ppcdis` folder for pure python build systems to make use of too
    - Generally, any new features for the python API should also be exposed by the command line API too
    - Importing anything that isn't included in `__init__.py` isn't officially supported
    - Like the rest of the project, the API won't be guaranteed any backwards compatibility (and is likely to change more than the command line API will). **This means users should not be required to install this globally (use something like venv instead), and that installs should be tied to a specific working version for the project**
- Installing the folder with pip can be used for the python API, but the command line API does not require this

## Credits

- camthesaxman for writing the original doldisasm.py
- riidefi, terorie and stebler for the [Mario Kart Wii Decompilation's tools](https://github.com/riidefi/mkw), which heavily inspired this
- All contributors to Tockdom's [DOL](https://wiki.tockdom.com/wiki/DOL_(File_Format)) and [REL](https://wiki.tockdom.com/wiki/REL_(File_Format)) file format documentation
