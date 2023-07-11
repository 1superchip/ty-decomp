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
template <typename T>
inline T Abs(T x) {
    return (x < 0) ? -x : x;
}

// may not have been a template
// this can always be changed later
template <typename T>
inline T Clamp(T x, T min, T max) {
    return (x < min) ? min : (x > max) ? max : x;
}

/// @brief Returns the sign of a number, returns 1.0f for 0.0f
/// @param x Number to return the sign of
/// @return Sign of x
template <typename T>
inline T GetSign(T x) {
    if (x < 0) {
        return -1.0;
    }
    return 1.0;
}

int RandomIR(int* pSeed, int min, int max); // returns a ranged random number
int RandomI(int* pSeed);
float RandomFR(int* pSeed, float min, float max);
float _table_sinf(float theta);
float NormaliseAngle(float);

#define PI 3.1415928f
#define PI2 1.5707964f
#define _table_cosf(angle) _table_sinf((angle) + PI2)

inline float DegToRad(float deg) {
	return deg * (PI / 180.0f);
}

extern inline float sqrtf(float x)
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

// Smoothing function?
inline float AdjustFloat(float param_1, float param_2, float param_3) {
	return (param_2 - param_1) * param_3 + param_1;
}

#endif // COMMON_STDMATH