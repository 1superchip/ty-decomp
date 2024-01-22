#include "types.h"
#include "common/Str.h"
#include "Dolphin/stl.h"

static char buffer[STR_BUFFER_SIZE];

char gNullStr[4] = "";

bool gAssertBool;
static int bufferIndex;

extern "C" int strlen(char*);

// bufferIndex inline

// Returns a pointer to the formatted string
// fmt is the format "%s"
// this function takes variadic args
char* Str_Printf(char* fmt, ...) {
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
    char* bufString = &buffer[bufferIndex];
    int idx = 0;
    
    while(idx < len && string[idx] != '\0') {
        bufString[idx] = string[idx];
        idx++;
    }

    bufString[idx] = '\0';
    bufferIndex += (idx + 0x10) & ~0xF;
    if (bufferIndex + 0x400 > STR_BUFFER_SIZE) {
        bufferIndex = 0;
    }
    
    return bufString;
}

char* Str_FindChar(char* str, int val) {
    while(*str != '\0') {
        if (*str == val) {
            return str;
        }
        str++;
    }
    return NULL;
}
