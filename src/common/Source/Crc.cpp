#include "types.h"
#include "common/Crc.h"

static bool crcTableComputed = false;
static int crcTable[0x100];

static void Crc_MakeTable(void) {
    uint c;
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

uint Crc_Update(uint crc, const void* buf, int len) {
    if (!crcTableComputed) { // only compute CRC table if it hasn't been computed
        Crc_MakeTable();
    }
    for (int i = 0; i < len; i++) {
        crc = crcTable[(crc ^ ((u8*)buf)[i]) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

u32 Crc_Calculate(const void *buf, int len) {
    return Crc_Update(0xFFFFFFFF, buf, len) ^ 0xFFFFFFFF;
}