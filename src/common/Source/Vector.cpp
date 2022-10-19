#include "types.h"
#include "common/Vector.h"

// .data
Vector gXAxis = {1.0f, 0.0f, 0.0f, 0.0f};
Vector gYAxis = {0.0f, 1.0f, 0.0f, 0.0f};
Vector gZAxis = {0.0f, 0.0f, 1.0f, 0.0f};

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

// from Kinematics.h
inline float kin_sqrtf(float x) {
    static const double _half=.5;
	static const double _three=3.0;
    double guess = __frsqrte(x);
    guess = _half * guess * (_three - ( x * (guess * guess)));
    guess = _half * guess * (_three - ( x * (guess * guess)));
    volatile float y = ( x * (_half * guess * (_three - ( x * (guess * guess)))));
    return y;
}

float Vector::Magnitude(void) {
    return sqrtf(MagSquared());
}

// order floats so 1.0f is before 0.000001f
bool OrderFloats(float x) {
	return x > 1.0f;
}
float Vector::Normalise(Vector* pVector) {
    float len = pVector->MagSquared();
    if (len > 0.000001f) {
        len = sqrtf(len);
        float inv = 1.0f / len;
        x = inv * pVector->x;
        y = inv * pVector->y;
        z = inv * pVector->z;
    } else {
        len = 0.0f;
    }
    return len;
}

void Vector::ClampMagnitude(Vector* pVector, float maxMag) {
    float vecMagSq = pVector->MagSquared();
    if (vecMagSq > maxMag * maxMag) {
        float clampMag = vecMagSq;
        clampMag = (clampMag > 0.0f) ? kin_sqrtf(clampMag) : clampMag;
        maxMag /= clampMag;
        x = maxMag * pVector->x;
        y = maxMag * pVector->y;
        z = maxMag * pVector->z;
        return;
    }
    x = pVector->x;
    y = pVector->y;
    z = pVector->z;
    w = pVector->w;
}

void Vector::Cross(Vector* pVector, Vector* pVector1) {
    float cx = (pVector1->y * pVector->z) - (pVector1->z * pVector->y);
    float cy = (pVector1->z * pVector->x) - (pVector1->x * pVector->z);
    float cz = (pVector1->x * pVector->y) - (pVector1->y * pVector->x);
    x = cx;
    y = cy;
    z = cz;
}

void Vector::Projection(Vector* pVector, Vector* pVector1) {
    float sqMag = pVector1->MagSquared();
    if (sqMag > 0.000001f) {
        float dot = pVector->Dot(pVector1) / sqMag;
        x = dot * pVector1->x;
        y = dot * pVector1->y;
        z = dot * pVector1->z;
        return;
    }
    z = 0.0f;
    y = 0.0f;
    x = 0.0f;
}

// https://decomp.me/scratch/Z0Prr
// operators?
// still fake?
inline void sub(Vector& a, Vector &b, Vector& c) {
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
}

void Vector::InterpolateLinear(Vector* pFrom, Vector* pTo, float fraction) {
    Vector diff;
	// sub inline isn't needed
	// it might clean it up though
    diff.x = pTo->x - pFrom->x;
    diff.y = pTo->y - pFrom->y;
    diff.z = pTo->z - pFrom->z;
    diff.x *= fraction;
    diff.y *= fraction;
    diff.z *= fraction;
    x = pFrom->x + diff.x;
    y = pFrom->y + diff.y;
    z = pFrom->z + diff.z;
}

void Vector::ApplyMatrix(Vector* pVector, Matrix* pMatrix) {
    float temp_f10;
    float temp_f11;
    float temp_f13;

    temp_f10 = pVector->x;
    temp_f11 = pVector->y;
    temp_f13 = pVector->z;
    float mx = pMatrix->data[3][0] + ((temp_f13 * pMatrix->data[2][0]) + ((temp_f10 * pMatrix->data[0][0]) + (temp_f11 * pMatrix->data[1][0])));
    float my = pMatrix->data[3][1] + ((temp_f13 * pMatrix->data[2][1]) + ((temp_f10 * pMatrix->data[0][1]) + (temp_f11 * pMatrix->data[1][1])));
    float mz = pMatrix->data[3][2] + ((temp_f13 * pMatrix->data[2][2]) + ((temp_f10 * pMatrix->data[0][2]) + (temp_f11 * pMatrix->data[1][2])));
    float mw = pMatrix->data[3][3] + ((temp_f13 * pMatrix->data[2][3]) + ((temp_f10 * pMatrix->data[0][3]) + (temp_f11 * pMatrix->data[1][3])));

    x = mx;
    y = my;
    z = mz;
    w = mw;
}

void Vector::ApplyMatrixW(Vector* pVector, Matrix* pMatrix) {
    float temp_f13;
    float temp_f9;
    float temp_f31;
    float temp_f11;

    temp_f11 = pVector->x;
    temp_f9 = pVector->y;
    temp_f13 = pVector->z;
    temp_f31 = pVector->w;
    float mx = (temp_f31 * pMatrix->data[3][0]) + ((temp_f13 * pMatrix->data[2][0]) + ((temp_f11 * pMatrix->data[0][0]) + (temp_f9 * pMatrix->data[1][0])));
    float my = (temp_f31 * pMatrix->data[3][1]) + ((temp_f13 * pMatrix->data[2][1]) + ((temp_f11 * pMatrix->data[0][1]) + (temp_f9 * pMatrix->data[1][1])));
    float mz = (temp_f31 * pMatrix->data[3][2]) + ((temp_f13 * pMatrix->data[2][2]) + ((temp_f11 * pMatrix->data[0][2]) + (temp_f9 * pMatrix->data[1][2])));
    float mw = (temp_f31 * pMatrix->data[3][3]) + ((temp_f13 * pMatrix->data[2][3]) + ((temp_f11 * pMatrix->data[0][3]) + (temp_f9 * pMatrix->data[1][3])));

    x = mx;
    y = my;
    z = mz;
    w = mw;
}

void Vector::ApplyRotMatrix(Vector* pVector, Matrix* pMatrix) {
    float vx;
    float vy;
    float vz;

    vx = pVector->x;
    vy = pVector->y;
    vz = pVector->z;
    float mx = (vz * pMatrix->data[2][0]) + ((vx * pMatrix->data[0][0]) + (vy * pMatrix->data[1][0]));
    float my = (vz * pMatrix->data[2][1]) + ((vx * pMatrix->data[0][1]) + (vy * pMatrix->data[1][1]));
    float mz = (vz * pMatrix->data[2][2]) + ((vx * pMatrix->data[0][2]) + (vy * pMatrix->data[1][2]));

    x = mx;
    y = my;
    z = mz;
}

void Vector::ApplyTransMatrix(Vector* pVector, Matrix* pMatrix) {
    x = pVector->x + pMatrix->data[3][0];
    y = pVector->y + pMatrix->data[3][1];
    z = pVector->z + pMatrix->data[3][2];
}

void Vector::CClamp(Vector* pVector, float arg1, float arg2) {
    float clamp = pVector->x;
    if (clamp < arg1) {
        clamp = arg1;
    } else if (clamp > arg2) {
        clamp = arg2;
    }
    x = clamp;
    
    clamp = pVector->y;
    if (clamp < arg1) {
        clamp = arg1;
    } else if (clamp > arg2) {
        clamp = arg2;
    }
    y = clamp;
    
    clamp = pVector->z;
    if (clamp < arg1) {
        clamp = arg1;
    } else if (clamp > arg2) {
        clamp = arg2;
    }
    z = clamp;
    
    clamp = pVector->w;
    if (clamp < arg1) {
        clamp = arg1;
    } else if (clamp > arg2) {
        clamp = arg2;
    } else {
        clamp = clamp;
    }
    w = clamp;
}

extern "C" double fmod(float, double);

void Vector::NormaliseRot(Vector* pVector) {
    pVector->x = fmod(pVector->x, 6.2831854820251461);
    pVector->y = fmod(pVector->y, 6.2831854820251461);
    pVector->z = fmod(pVector->z, 6.2831854820251461);

    if (pVector->x < 0.0f) {
        pVector->x += 6.2831855f;
    }
    if (pVector->y < 0.0f) {
        pVector->y += 6.2831855f;
    }
    if (pVector->z < 0.0f) {
        pVector->z += 6.2831855f;
    }
}