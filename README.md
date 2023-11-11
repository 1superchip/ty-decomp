# ty1-decomp [![Discord Channel][discord-badge]][discord] ![dol progress]
[discord]: https://discord.gg/hKx3FJJgrV
[discord-badge]: https://img.shields.io/discord/727908905392275526?color=%237289DA&logo=discord&logoColor=%23FFFFFF
[dol progress]: https://img.shields.io/endpoint?label=dol&url=https%3A%2F%2Fprogress.decomp.club%2Fdata%2Fty%2Fus%2FCode%2F%3Fmode%3Dshield%26measure%3Dcode

Decompilation of Ty the Tasmanian Tiger NTSC-U (GTYE69)

## Building
Dump a copy of the game
 - Place main.dol in *orig/*
 - Place CodeWarrior 1.3.2 in *tools/1.3.2/*, 1.2.5 in *tools/1.2.5/*, and 1.2.5n in *tools/1.2.5n/*
 - Install DevkitPPC, Ninja and Python
	- DevkitPPC r39
	- Ninja 1.3
 - Install the required modules from requirements.txt (`pip install -r requirements.txt`) 
 - Run configure.py
 - Run ninja
 
## Checking Progress
Run progress.py after a successful build to see the current .text section progress

## Credits
 - SeekyCt for ppcdis and spm decomp