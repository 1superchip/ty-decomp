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
inline T Clamp(T min, T x, T max) {
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

inline float SmoothCenteredCurve(float x) {
	return 1.0f - Sqr<float>(x - 0.5f) * 4.0f;
}

int RandomIR(int* pSeed, int min, int max); // returns a ranged random number
int RandomI(int* pSeed);
float RandomFR(int* pSeed, float min, float max);
float _table_sinf(float theta);
float NormaliseAngle(float);

#define PI (3.1415928f)
#define PI2 (1.5707964f)
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
/*
// unclamped
public static float Remap(this float aValue, float aIn1, float aIn2, float aOut1, float aOut2)
{
	float t = (aValue - aIn1) / (aIn2 - aIn1);
	return aOut1 + (aOut2 - aOut1) * t;
}
*/
// Smoothing function?
inline float AdjustFloat(float x, float param_2, float t) {
	// Adjusts x on the range of [0, 1] to the interval of [param_2 * t, param_2 * t - t + 1] f
	return (param_2 - x) * t + x;
}

#endif // COMMON_STDMATH
