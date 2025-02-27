## General Notes

- All ranges have an inclusive start and exclusive end
- Using float constants in inline assembly when compiling with `-ipa file` will cause them all to be placed at the start of the `.sdata2` section, which will stop matching being possible when other data is supposed to be inbetween them. The `manual_sdata2_ranges` override can be used to work around this by referring to them with labels instead (defined through CW's colon syntax or `relsymdefs` in orderfloats).

## Binary yml formats

### Dol yml

- `path`: path to binary
- `section_defs`: optional section definition overrides (see below)
- `r13`: \_SDA\_BASE\_ address from r13
- `r2`: \_SDA2\_BASE\_ address from r2

### Rel yml

- `path`: path to binary
- `address`: address the binary loads at
- `bss_address`: address the binary's bss allocates at
- `section_defs`: optional section definition overrides (see below)

### section_defs format

The format is:
```yml
section_defs:
    text:
        - name: section1
          prop1: val
          ...
        ...
    data:
        ...
    bss:
        ...
```

All properties other than name are optional. They are:
- `attr`: attributes given to the section in disassembly
- `nobits`: tags a section with `@nobits` in disassembly
- `balign`: the number of bytes to align the start of a slice to. Defaults to 8 for data and 0 for text
- `bss_forced_size`: forces the size of a bss section, required when two bss sections are back to back.
- `bss_start_align`: forces the start of a bss section to be aligned by an amount

## relextern

Pre-scans rel binaries for any labels they reference in other binaries to be passed into analysis as extra labels.

## Analyser

The analyser analyses the code in a binary to identify things like relocations, functions, labels and jumptables. This is required for the disassembler.

### Extra labels

The `--extra-labels` flag allows for labels in this binary referenced by external binaries to be given, allowing them to be output even if they're never referenced by this binary. These can be generated with the relextern tool.

### Thorough value following

The `--thorough` flag disables tracking of multiple @h(a) values at once to ensure each can explore the full control flow of a function. It's probably not needed for any mwcc compiled code, just gcc.

For example, analysis without the flag would miss the second upper being paired with the lower in this case
```
block1:
    lis rX, sym@ha
    b block2
...
block2:
    addi rY, rX, sym@l
...
block3:
    lis rX, sym@ha
    b block2 
```

### Analyser Overrides

Takes overrides from a yml to correct mistakes and improve output.

#### blocked_pointers / blocked_pointer_ranges

Addresses where the value stored should not be considered a pointer.

For example, if
```
lbl_80100000:
    .4byte lbl_80004000
```
is actually just the hex value 0x80004000 and not a pointer to lbl_80004000, then adding 0x80100000 to the blocked pointers would give
```
lbl_80100000:
    .4byte 0x80004000
```

#### blocked_targets / blocked_target_ranges

Addresses that potential pointers to shouldn't be considered as pointers.

For example, if the address 0x80004000 should never be pointed to, then adding 0x80004000 to the blocked targets would turn
```
lbl_80100000:
    .4byte lbl_80004000
...
lbl_80200000:
    .4byte lbl_80004000
...
```
into
```
lbl_80100000:
    .4byte 0x80004000
...
lbl_80200000:
    .4byte 0x80004000
...
```

#### sdata_sizes

Unlike in normal data sections, where fields of larger symbols (structs, classes, arrays, etc.) are accessed by first loading the start address of the symbol and then adding offsets, sdata section symbol fields are loaded directly by adding the field offset to the symbol's offset from r13/r2. This override tells the analyser to treat addresses in a certain range after the symbol as part of the symbol.

For example, setting 0x80100000 to size 4 turns
```
    lbz r3, lbl_80100000@sda21 (r2)
    lbz r4, lbl_80100001@sda21 (r2)
    lbz r5, lbl_80100002@sda21 (r2)
    lbz r6, lbl_80100003@sda21 (r2)
...
lbl_80100000:
    .byte 0x00
lbl_80100001:
    .byte 0x00
lbl_80100002:
    .byte 0x00
lbl_80100003:
    .byte 0x00
```
into
```
    lbz r3, lbl_80100000@sda21 (r2)
    lbz r4, (lbl_80100000+1)@sda21 (r2)
    lbz r5, (lbl_80100000+2)@sda21 (r2)
    lbz r6, (lbl_80100000+3)@sda21 (r2)
...
lbl_80100000:
    .4byte 0x00000000
```

#### forced_types

When analysis gets the type of a symbol wrong, this can be used to override it. The available types are `FUNCTION`, `LABEL` and `DATA`.

## Disassembler

The disassembler outputs assembly code in various forms, as well as C jumptable workarounds.

### Full Disassembly

When no flags are given, the disassembler disassembles a full binary into one assembly file for manual inspection. Can be re-assembled, but isn't much use for building an actual decomp.

### Slice Disassembly

When given `--slice start end`, the disassembler outputs a 'slice' of a section to assembly, for use on undecompiled code in decomps.

### C Jumptable Generation

When given `--jumptable addr`, the disassembler outputs a C jumptable workaround, for use on undecompiled jumptables in partially decompiled files.

### Function generation

When given `--function addr`, the disassembler outputs the assembly of a single function for use with decomp.me, m2c, or just manual inspection.

#### Extra Data

When given `--extra` with `--function`, extra data referenced by the function is included. This is currently limited to jumptables.

#### Inline function generation

When given `--inline` with `--function`, the disassembler outputs the assembly of a single function in mwcc inline assembly format, for use on undecompiled functions in partially decompiled files.

### Source Name

When given `--source-name name` with `--function` or `--jumptable`, the disassembler uses this as the source file name to get file-local symbols from in the symbols yml.

### Quiet Disassembly (--quiet)

When given `--quiet`, the dissassembler won't print progress updates.

### Disassember Overrides

Takes overrides from a yml to correct mistakes and improve output.

#### manual_sdata2_ranges

Addresses where inline assembly generated should reference constant floats / doubles by their label instead of their value. This is neccessary for files with other data mixed in with the float constants when compiling with ipa file, and must be paired with an orderfloats not using the `--asm` flag.

For example,
```
    lfs f1, 1.0
```
where 1.0 is at 80100000 and 80100000 is added to the blocked addresses would become
```
    lfs f1, lbl_80100000
```

#### global_manual_floats

Signals that all inline assembly should reference constant floats / doubles by their label instead of their value. This is neccessary for versions of CW older than 4199 60831, and must be paired with an orderfloats not using the `--asm` flag.

#### trim_ctors / trim_dtors

The CW linker's behaviour with the .ctors and .dtors sections is quite weird. In some circumstances, the linker will automatically generate the zeros at the end of the .ctors and .dtors sections. These overrides allow for them to be removed from the disassembly to account for that.

### symbol_aligns

Sets the alignment to give the symbol at an address when disassembling it.

For example,
```yml
symbol_aligns:
    0x80100000: 0x20
```
will turn
```
.global lbl_80100000
lbl_80100000:
```
into
```
.balign 0x20
.global lbl_80100000
lbl_80100000:
```

### Symbol Map

Takes a symbol yml of the format
```yml
category1:
    0xaddr1: name1
    0xaddr2: name2
    ...

category2:
...
```

#### Global Category

The category `global` gives symbols that apply anywhere.

#### Binary Categories

The binary categories (the filename as passed in command line arguments without leading folders) give symbols that apply anywhere in a binary.

#### Source File Categories

The source file categories (source file path as passed in with `--source-name`) give symbols that apply to the source file.

## assetrip

Rips an embedded asset from a binary.

## assetinc

Generates a C include file with the contents of a binary file as a u8 array.

## binarydiff

Prints any differing sections in a dol or rel file, and any differing relocations in a rel file. Takes a binary yml for the matching binary, and the path of the testing binary (properties from the yml are re-used for this too).

## elf2dol

Converts an elf file to a dol file. Automatically strips segments with `p_vaddr` as 0.

## elf2rel

Converts a partially linked elf file to a rel file, taking the elf of the dol file to link against. Automatically strips sections named `forcestrip` or `relsymdef`, as well as reading symbols from `relsymdef`.

### Section count

The `--num-sections` flag can be used to add extra empty sections in the header.

### Symbol Definitions

If the `REL_SYMBOL_AT` macro is used to get `relsymdef` symbols, then the `--base-rel`, `--base-rel-addr` and `--base-rel-bss` args must be provided to give the addresses context.

## forcefilesgen

Adds a list of files to the FORCEFILES section of an LCF file by replacing the string `PPCDIS_FORCEFILES`.

## forceactivegen

Adds functions from an external labels pickle to the FORCEACTIVE section of an LCF file by replacing the string `PPCDIS_FORCEACTIVE`.

## orderfloats

Generates a dummy function to order the float / double constants in a file.

### .sdata2 mode 

The `--sda` flag places the floats in `.sdata2` instead of `.rodata`.

### --asm

Makes the dummy function use inline asm, which is required with `-ipa file` when not using 'manual' floats.

## orderstrings

Generates a dummy function to order the string constants in a file.

### Encoding

When given `--enc name`, the encoding can be changed to any encoding supported by python. This applies to both the string parsing and the output file encoding.

### Pooling

If the code is compiled with `-str pool`, then `--pool` needs to be passed so that the tool searches for non-aligned strings in the range.

## slices

Gives utilities for querying a slices yml. The output format for describing a source is a json in one of the following formats:
- For decompiled code, path to source file
- For undecompiled code, list of [section name, start addr, end addr]

### Order Sources

When given `--order-sources`, the tool orders the sources in a slice yml by address to give the order they should be passed to the linker in, and fills in gaps with assembly slice source descriptions. The output is a list of source descriptions.

### Containing

When given `--containing addr`, the tool gives the source file containing an address. The output is a single source description.

### Slice YML Format

The format of a slice yml is:

```yml
sourceName1:
    section1: [start, end]
    section2: [start, end]
    ...

sourceName2:
...
```

## symbols

Gives utilities for querying a symbols yml.

### Get name

When given `--get-name addr`, the tool gives the name of the symbol at that address, or null if not found.

### Get address

When given `--get-addr name`, the tool gives the address of the symbol with that name, or null if not found.

### Source name

When given `--source-name name`, the tool uses this as the source file name to get file-local symbols from the symbols yml.

### Binary name

When given `--binary path`, the tool uses the binary name in this to get binary-local symbols from the symbols yml.
