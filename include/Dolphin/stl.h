#ifndef _DOLPHIN_STL_H
#define _DOLPHIN_STL_H

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

#ifndef size_t
typedef unsigned long size_t;
#endif

__declspec(section ".init") void* memcpy(void*, const void*, size_t);
__declspec(section ".init") void __fill_mem(void*, int, size_t);
__declspec(section ".init") void* memset(void*, int, size_t);

// typedef struct __va_list_struct {
// 	char gpr;
// 	char fpr;
// 	char* input_arg_area;
// 	char* reg_save_area;
// } va_list[1];

// void* __va_arg(va_list, int);

// #define va_start(ARG, VA_LIST)    ((void)ARG, __builtin_va_info(&VA_LIST))
// #define va_end(VA_LIST)           ((void)VA_LIST)
// #define va_arg(VA_LIST, ARG_TYPE) (*(ARG_TYPE*)) __va_arg(VA_LIST, _var_arg_typeof(ARG_TYPE))

#ifdef __MWERKS__
typedef struct {
	char gpr;
	char fpr;
	char reserved[2];
	char* input_arg_area;
	char* reg_save_area;
} __va_list[1];
typedef __va_list va_list;

#ifndef __MWERKS__
extern void __builtin_va_info(va_list*);
#endif

void* __va_arg(va_list v_list, unsigned char type);

#define va_start(ap, fmt) ((void)fmt, __builtin_va_info(&ap))
#define va_arg(ap, t)     (*((t*)__va_arg(ap, _var_arg_typeof(t))))
#define va_end(ap)        (void)0

#else
typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#endif

/*
 * --INFO--
 * Address:	........
 * Size:	0000E0 or 0000E4, depending on param.
 */
#define DEFINE__PRINT(unit) \
	inline static void _Print(char*, ...) { printf(unit); }

int printf(const char*, ...);
int vprintf(const char*, va_list);
int sprintf(char*, char*, ...);
int snprintf(char*, size_t, const char*, ...);
int vsnprintf(char*, size_t, const char*, va_list);
int vsprintf(char* s, const char* format, va_list arg);

int rand();

void* memcpy(void* dest, const void* src, size_t n);

#ifdef __cplusplus
};
#endif // ifdef __cplusplus

#endif