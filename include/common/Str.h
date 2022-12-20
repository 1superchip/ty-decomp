#ifndef COMMON_STR
#define COMMON_STR

char* Str_Printf(char*, ...);
char* Str_CopyString(char*, int);
char* Str_FindChar(char*, int);

#define STR_BUFFER_SIZE 0x4000

#endif // COMMON_STR