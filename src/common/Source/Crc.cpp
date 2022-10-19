#include "types.h"
#include "common/Crc.h"

// .sbss
bool crcTableComputed;

// .bss
int crcTable[0x100];

// exists in Ty 3 and Spyro
void Crc_MakeTable(void) {
    uint c;
    if (crcTableComputed == false) {
        // compute crcTable
        for (int i = 0; i < 256; i++) {
            c = i;
            for (int j = 0; j < 8; j++) {
                if (c & 1) {
                    c = (c >> 1) ^ 0xEDB88320;
                } else {
                    c >>= 1;
                }
            }
            crcTable[i] = c;
        }
        // set it to computed so it isn't computed again
        crcTableComputed = true;
    }
}

//https://decomp.me/scratch/IcQXq
// is this Crc_Update__FUiPCvi?
inline u32 makeCrcTable(const u8* buf, int len) {
    u32 crc = 0xffffffff;
	Crc_MakeTable();

    for (int i = 0; i < len; i++) {
	    crc = crcTable[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
	}
    return crc ^ 0xffffffff;
}

u32 Crc_Calculate(const void *buf, int len) {
    return makeCrcTable((const u8*)buf, len);
}