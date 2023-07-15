#ifndef _STRING_H_
#define _STRING_H_

// #include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define offsetof(type, member) ((size_t) & (((type*)0)->member))

/* These break 1.2.5 */
//typedef __typeof__(sizeof(0)) size_t;
//typedef __typeof__((char*)0 - (char*)0) ptrdiff_t;
typedef unsigned long size_t;
typedef long ptrdiff_t;
#ifndef NULL
#define NULL 0L
#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma section code_type ".init"
void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* dst, int val, size_t n);
void __fill_mem(void* dst, int val, size_t n);
#pragma section code_type

size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t num);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strncat(char* dest, const char* src, size_t n);

#ifdef __cplusplus
}
#endif

#endif