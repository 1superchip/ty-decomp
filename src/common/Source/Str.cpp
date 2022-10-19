#include "common/Str.h"

// .bss
char buffer[STR_BUFFER_SIZE];

// .sdata
char gNullStr[] = "";

// .sbss
bool gAssertBool;
int bufferIndex;

extern "C" int strlen(char*);

char* Str_Printf(char* str, ...) {
    char* currStr = &buffer[bufferIndex];
    va_list args;
    
    va_start(args, str);
    vsprintf(currStr, str, args);
    
    int len = strlen(currStr);
    bufferIndex += (len + 0x10) & 0xFFFFFFF0; // think this is rounding
    if (bufferIndex + 0x400 > STR_BUFFER_SIZE) {
        bufferIndex = 0;
    }
    va_end(args);
    return currStr;
}

char* Str_CopyString(char* string, int len) {
    char* bufString = &buffer[bufferIndex];
    int idx = 0;
    
    while(idx < len && string[idx] != 0) {
        bufString[idx] = string[idx];
        idx++;
    }

    bufString[idx] = 0;
    bufferIndex += (idx + 0x10) & 0xFFFFFFF0; // think this is rounding

    if (bufferIndex + 0x400 > STR_BUFFER_SIZE) {
        bufferIndex = 0;
    }
    
    return bufString;
}

char* Str_FindChar(char* str, int val) {
    while(*str != 0) {
        if (*str == val) {
            return str;
        }
        str++;
    }
    return 0;
}