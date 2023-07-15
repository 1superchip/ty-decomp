
static unsigned long int next = 1;

int rand(void) {
    next = (next * 0x41c64e6d) + 0x3039;
    return (next >> 16) & 0x7FFF;
}
