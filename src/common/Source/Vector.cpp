#include "types.h"
#include "common/Vector.h"
#include "common/Matrix.h"
#include "common/StdMath.h"

Vector gXAxis = {1.0f, 0.0f, 0.0f, 0.0f};
Vector gYAxis = {0.0f, 1.0f, 0.0f, 0.0f};
Vector gZAxis = {0.0f, 0.0f, 1.0f, 0.0f};

/// @brief sqrtf(x * x + y * y + z * z)
/// @param  None
/// @return Magnitude of this
float Vector::Magnitude(void) {
    return sqrtf(MagSquared());
}

/*
Unused, stripped 
bool Vector::IsNormalised();
// the comparison uses the wrong float?
*/
bool Vector__IsNormalised(Vector* pThis) {
    return Abs<float>(pThis->MagSquared() - 1.0f) < 0.00002f;
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
		float mag = maxMag / sqrtf(vecMagSq);
        x = mag * pVector->x;
        y = mag * pVector->y;
        z = mag * pVector->z;
        return;
    }
    *this = *pVector;
}

void Vector::Cross(Vector* pVector1, Vector* pVector2) {
    Vector tempv;
    tempv.x = (pVector2->y * pVector1->z) - (pVector2->z * pVector1->y);
    tempv.y = (pVector2->z * pVector1->x) - (pVector2->x * pVector1->z);
    tempv.z = (pVector2->x * pVector1->y) - (pVector2->y * pVector1->x);
    Copy(&tempv);
}

void Vector::Projection(Vector* pVector1, Vector* pVector2) {
    float sqMag = pVector2->MagSquared();
    if (sqMag > 0.000001f) {
        float dot = pVector1->Dot(pVector2);
		Scale(pVector2, dot / sqMag);
        return;
    }
	SetZero();
}

/// @brief Interpolates from pFrom to pTo: this = pFrom + fraction * (pTo - pFrom)
/// @param pFrom Starting point to interpolate from
/// @param pTo Ending point to interpolate to
/// @param fraction Interpolation fraction between 0.0f and 1.0f [0.0f, 1.0f]
void Vector::InterpolateLinear(Vector* pFrom, Vector* pTo, float fraction) {
    Vector tempv;
    tempv.Sub(pTo, pFrom);
    tempv.Scale(fraction);
    Add(pFrom, &tempv);
}

void Vector::ApplyMatrix(Vector* pVector, Matrix* pMatrix) {
    Vector tmp;
    float src_x;
    float src_y;
    float src_z;

    src_x = pVector->x;
    src_y = pVector->y;
    src_z = pVector->z;
    float mx = (src_x * pMatrix->data[0][0]) + (src_y * pMatrix->data[1][0]) + (src_z * pMatrix->data[2][0]) + pMatrix->data[3][0];
    float my = (src_x * pMatrix->data[0][1]) + (src_y * pMatrix->data[1][1]) + (src_z * pMatrix->data[2][1]) + pMatrix->data[3][1];
    float mz = (src_x * pMatrix->data[0][2]) + (src_y * pMatrix->data[1][2]) + (src_z * pMatrix->data[2][2]) + pMatrix->data[3][2];
    float mw = (src_x * pMatrix->data[0][3]) + (src_y * pMatrix->data[1][3]) + (src_z * pMatrix->data[2][3]) + pMatrix->data[3][3];

    tmp.Set(mx, my, mz, mw);
    *this = tmp;
}

void Vector::ApplyMatrixW(Vector* pVector, Matrix* pMatrix) {
    Vector tmp;
    float src_y;
    float src_x;
    float src_z;
    float src_w;

    src_x = pVector->x;
    src_y = pVector->y;
    src_z = pVector->z;
    src_w = pVector->w;
    float mx = (src_x * pMatrix->data[0][0]) + (src_y * pMatrix->data[1][0]) + (src_z * pMatrix->data[2][0]) + (src_w * pMatrix->data[3][0]);
    float my = (src_x * pMatrix->data[0][1]) + (src_y * pMatrix->data[1][1]) + (src_z * pMatrix->data[2][1]) + (src_w * pMatrix->data[3][1]);
    float mz = (src_x * pMatrix->data[0][2]) + (src_y * pMatrix->data[1][2]) + (src_z * pMatrix->data[2][2]) + (src_w * pMatrix->data[3][2]);
    float mw = (src_x * pMatrix->data[0][3]) + (src_y * pMatrix->data[1][3]) + (src_z * pMatrix->data[2][3]) + (src_w * pMatrix->data[3][3]);

    tmp.Set(mx, my, mz, mw);
    *this = tmp;
}

/// @brief Applies a rotation matrix to pVector and stores the rotated vector in this
/// @param pVector Source vector that rotation is applied to
/// @param pMatrix Rotation Matrix to use
void Vector::ApplyRotMatrix(Vector* pVector, Matrix* pMatrix) {
    Vector temp;
    
    temp.x = (pVector->x * pMatrix->data[0][0]) + (pVector->y * pMatrix->data[1][0]) + (pVector->z * pMatrix->data[2][0]);
    temp.y = (pVector->x * pMatrix->data[0][1]) + (pVector->y * pMatrix->data[1][1]) + (pVector->z * pMatrix->data[2][1]);
    temp.z = (pVector->x * pMatrix->data[0][2]) + (pVector->y * pMatrix->data[1][2]) + (pVector->z * pMatrix->data[2][2]);
    
    Copy(&temp);
}

/// @brief Applies the translation of pMatrix to pVector and places the result in this (this = pVector + pMatrix->data[3])
/// @param pVector Vector to apply Matrix Translation
/// @param pMatrix Translation Matrix
void Vector::ApplyTransMatrix(Vector* pVector, Matrix* pMatrix) {
    x = pVector->x + pMatrix->data[3][0];
    y = pVector->y + pMatrix->data[3][1];
    z = pVector->z + pMatrix->data[3][2];
}

/// @brief Clamps each component of a Vector to the range of [min, max]
/// @param pColour Input vector
/// @param min Minimum value for each component
/// @param max Maximum value for each component
void Vector::CClamp(Vector* pColour, float min, float max) {
    x = Clamp<float>(min, pColour->x, max);
    y = Clamp<float>(min, pColour->y, max);
    z = Clamp<float>(min, pColour->z, max);
    w = Clamp<float>(min, pColour->w, max);
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