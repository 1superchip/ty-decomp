#ifndef TYPES_H
#define TYPES_H

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef signed long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long size_t;
typedef unsigned long long u64;

typedef unsigned short ushort;
typedef unsigned int uint;

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef float f32;
typedef double f64;
typedef volatile f32 vf32;
typedef volatile f64 vf64;

typedef int BOOL;
#define TRUE  1
#define FALSE 0

struct MTX34
{
    float mMtx[3][4];
};

#ifndef nullptr
#define nullptr 0
#endif

#ifndef NULL
#define NULL 0
#endif

// Array size define
#define ARRAY_SIZE(o) (sizeof((o)) / sizeof(*(o)))

#ifdef __cplusplus
extern "C" {
#endif

#undef ARRAY_SIZE
#define ARRAY_SIZE(arr) static_cast<int>(sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
}
#endif

#endif // !TYPES_H
