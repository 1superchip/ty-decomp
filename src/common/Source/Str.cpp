#include "types.h"
#include "common/Str.h"
#include "Dolphin/stl.h"

static char buffer[STR_BUFFER_SIZE];

char gNullStr[4] = "";

bool gAssertBool;
static int bufferIndex = 0;

extern "C" int strlen(char*);

// bufferIndex inline

// Returns a pointer to the formatted string
// fmt is the format "%s"
char* Str_Printf(char* fmt, ...) {
    // this function can overflow the string buffer
    // bufferIndex is set to 0 when (bufferIndex + 0x400) > STR_BUFFER_SIZE
    // meaning that if bufferIndex <= (STR_BUFFER_SIZE - 0x400) before printing a string
    // longer than the remaining space in the buffer (STR_BUFFER_SIZE - bufferIndex)
    // the buffer overflows

    char* currStr = &buffer[bufferIndex];
    va_list args;
    
    va_start(args, fmt);
    vsprintf(currStr, fmt, args);
    
    int len = strlen(currStr);
    bufferIndex += (len + 0x10) & ~0xF;
    if (bufferIndex + 0x400 > STR_BUFFER_SIZE) {
        bufferIndex = 0;
    }

    va_end(args);
    return currStr;
}

char* Str_CopyString(char* string, int len) {
    // same buffer overflow as Str_Printf
    
    char* bufString = &buffer[bufferIndex];
    int idx = 0;
    
    while (idx < len && string[idx] != '\0') {
        bufString[idx] = string[idx];
        idx++;
    }

    bufString[idx] = '\0';

    // round up to a multiple of 16
    // if idx is a multiple of 16, idx is rounded to the next (0 -> 16, 16 -> 32)
    bufferIndex += (idx + 0x10) & ~(0x10 - 1);
    
    if (bufferIndex + 0x400 > STR_BUFFER_SIZE) {
        bufferIndex = 0;
    }
    
    return bufString;
}

char* Str_FindChar(char* str, int val) {
    while (*str != '\0') {
        if (*str == val) {
            return str;
        }
        str++;
    }
    return NULL;
}
