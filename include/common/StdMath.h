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
    } else {
        return 1.0;
    }
}

inline float SmoothCenteredCurve(float x) {
    return 1.0f - Sqr<float>(x - 0.5f) * 4.0f;
}

int RandomIR(int* pSeed, int min, int max); // returns a ranged random number
int RandomI(int* pSeed);
float RandomFR(int* pSeed, float min, float max);
float _table_sinf(float theta);
float NormaliseAngle(float);

#define PI  (3.14159265358979323846f)
#define PI2 (1.5707964f)

#define _table_cosf(angle) _table_sinf((angle) + PI2)

inline float DegToRad(float deg) {
    return deg * (PI / 180.0f);
}

#ifdef __MWERKS__
extern inline float sqrtf(float x) {
    static const double _half=.5;
    static const double _three=3.0;
    volatile float y;

    if (x > 0.0f) {
        double guess = __frsqrte((double)x);   // returns an approximation to
        guess = _half*guess*(_three - guess*guess*x);  // now have 12 sig bits
        guess = _half*guess*(_three - guess*guess*x);  // now have 24 sig bits
        guess = _half*guess*(_three - guess*guess*x);  // now have 32 sig bits
        y = (float)(x*guess);
        return y;
    } else {
        return x;
    }
}
#else
extern float sqrtf(float x);
#endif

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

// no code that uses this has been decompiled yet
// Could have been an inline version of GetAngleDifference which was defined in StdMath.cpp
// Returns the smallest angle needed for ang1 to reach ang2
// Returns a value between [-PI, PI]
inline float GetSmallestAngle(float ang1, float ang2) {
    float ang = NormaliseAngle(ang2) - NormaliseAngle(ang1);

    if (ang > PI) {
        return ang - (2.0f * PI);
    } else if (ang < -PI) {
        return ang + (2.0f * PI);
    } else {
        return ang;
    }
}

/// @brief Interpolates between fromAngle and toAngle with smooth
/// @param fromAngle Angle to interpolate from
/// @param toAngle Angle to interpolate to
/// @param smooth Amount to interpolate
/// @return 
inline float SmoothToAngle(float fromAngle, float toAngle, float smooth) {
    return NormaliseAngle(fromAngle + (GetSmallestAngle(fromAngle, toAngle) * smooth));
}


/// @brief Simple angle normalisation function
/// @param angle Angle to normalise
/// @return Normalised angle
inline float Simple_NormaliseAngle(float angle) {
    if (angle < 0.0f) {
        return angle + (2.0f * PI);
    } else if (angle > 2.0f * PI) {
        return angle - (2.0f * PI);
    } else {
        return angle;
    }
}

// Defined as global in Rhino.cpp and inlined in Rhino::InitTurnAround 3x
// Normalises the difference between ang1 and ang2 to [-PI, PI]
inline float AngleDifference(float ang1, float ang2) {
    float angle = Abs<float>(NormaliseAngle(ang1) - NormaliseAngle(ang2));

    if (angle > PI) {
        // (PI, 2 * PI) -> (PI, 0)
        return angle - (2.0f * PI);
    } else {
        return angle;
    }
}

#endif // COMMON_STDMATH
