#ifndef COMMON_STR
#define COMMON_STR

extern bool gAssertBool;
extern char gNullStr[];

char* Str_Printf(char* fmt, ...);
char* Str_CopyString(char* string, int len);
char* Str_FindChar(char* str, int val);

#define STR_BUFFER_SIZE 0x4000

#endif // COMMON_STR
