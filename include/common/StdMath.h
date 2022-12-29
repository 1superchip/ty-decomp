#ifndef COMMON_STDMATH
#define COMMON_STDMATH

#include "types.h"

template <typename T>
inline T Min(T x, T y) {
    return (x < y) ? x : y;
}
template <typename T>
inline T Max(T x, T y) {
    return (x > y) ? x : y;
}
template <typename T>
inline T Sqr(T x) {
    return x * x;
}

// may not have been a template
// this can always be changed later
template <typename T>
inline T Clamp(T x, T min, T max) {
    return (x < min) ? min : (x > max) ? max : x;
}

union sindata {
    float xf;
    int xi;
};

int RandomIR(int*, int, int);
int RandomI(int*);
float RandomFR(int*, float, float);
float _table_sinf(float);
float NormaliseAngle(float);

#define PI2 1.5707964f
#define _table_cosf(angle) _table_sinf(angle + PI2)

#endif // COMMON_STDMATH