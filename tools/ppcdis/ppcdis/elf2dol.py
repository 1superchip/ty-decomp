"""
Converts an ELF to a DOL file
"""

from elftools.elf.constants import P_FLAGS
from elftools.elf.elffile import ELFFile

def elf_to_dol(in_path: str, out_path: str):
    """Converts an ELF file to a DOL file"""
    
    with open(in_path, 'rb') as src, open(out_path, 'wb') as out:
        write_u32 = lambda u32: out.write(int.to_bytes(u32, 4, 'big'))

        # Load elf
        elf = ELFFile(src)

        # Find wanted segments
        seg_idxs = [
            i for i in range(elf.num_segments())
            if elf.get_segment(i)["p_vaddr"] != 0
        ]

        # Bring text segments to front
        seg_idxs.sort(key = lambda i: elf.get_segment(i)["p_flags"] & P_FLAGS.PF_X == 0)

        # Write dummy header
        out.write(bytes(0x100))

        # Process segments
        text_n = 0
        data_n = 7
        bss_start = None
        bss_end = None
        offsets = [0 for _ in range(18)]
        addrs = [0 for _ in range(18)]
        sizes = [0 for _ in range(18)]
        for i in seg_idxs:
            seg = elf.get_segment(i)
            initialised = seg["p_filesz"] > 0

            if initialised:
                text = seg["p_flags"] & P_FLAGS.PF_X
                if text:
                    idx = text_n
                    text_n += 1
                else:
                    idx = data_n
                    data_n += 1

                offsets[idx] = out.tell()
                addrs[idx] = seg["p_vaddr"]
                sizes[idx] = seg["p_memsz"]
                out.write(seg.data())
            else:
                if bss_start is None:
                    bss_start = seg["p_vaddr"]
                    bss_end = seg["p_vaddr"] + seg["p_memsz"]
                else:
                    bss_start = min(bss_start, seg["p_vaddr"])
                    bss_end = max(bss_end, seg["p_vaddr"] + seg["p_memsz"])

        # Fill in header
        out.seek(0)
        for arr in (offsets, addrs, sizes):
            for val in arr:
                write_u32(val)
        write_u32(bss_start)
        write_u32(bss_end - bss_start)
        write_u32(elf["e_entry"])
