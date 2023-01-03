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

inline float sqrtf(float x)
{
	static const double _half=.5;
	static const double _three=3.0;
	volatile float y;
	if(x > 0.0f)
	{
		double guess = __frsqrte((double)x);   // returns an approximation to
		guess = _half*guess*(_three - guess*guess*x);  // now have 12 sig bits
		guess = _half*guess*(_three - guess*guess*x);  // now have 24 sig bits
		guess = _half*guess*(_three - guess*guess*x);  // now have 32 sig bits
		y=(float)(x*guess);
		return y;
	}
	return x;
}

inline float kin_sqrtf(float x) {
    static const double _half=.5;
	static const double _three=3.0;
    double guess = __frsqrte(x);
    guess = _half * guess * (_three - ( x * (guess * guess)));
    guess = _half * guess * (_three - ( x * (guess * guess)));
    volatile float y = ( x * (_half * guess * (_three - ( x * (guess * guess)))));
    return y;
}

#endif // COMMON_STDMATH