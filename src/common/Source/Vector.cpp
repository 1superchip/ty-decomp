#include "types.h"
#include "common/Vector.h"
#include "common/Matrix.h"
#include "common/StdMath.h"

Vector gXAxis = {1.0f, 0.0f, 0.0f, 0.0f};
Vector gYAxis = {0.0f, 1.0f, 0.0f, 0.0f};
Vector gZAxis = {0.0f, 0.0f, 1.0f, 0.0f};

float Vector::Magnitude(void) {
    return sqrtf(MagSquared());
}

/*
Unused, stripped 
bool Vector::IsNormalised();
// the comparison uses the wrong float?
*/
bool Vector__IsNormalised(Vector* pThis) {
    return Abs<float>(pThis->MagSquared() - 1.0f) < 0.0f;//_unkgp_0x4240;
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

void Vector::Cross(Vector* pVector, Vector* pVector1) {
    Vector tmp;
    tmp.x = (pVector1->y * pVector->z) - (pVector1->z * pVector->y);
    tmp.y = (pVector1->z * pVector->x) - (pVector1->x * pVector->z);
    tmp.z = (pVector1->x * pVector->y) - (pVector1->y * pVector->x);
    x = tmp.x;
    y = tmp.y;
    z = tmp.z;
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

void Vector::InterpolateLinear(Vector* pFrom, Vector* pTo, float fraction) {
    Vector diff;
    diff.Sub(pTo, pFrom);
    diff.Scale(fraction);
    x = pFrom->x + diff.x;
    y = pFrom->y + diff.y;
    z = pFrom->z + diff.z;
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

void Vector::ApplyRotMatrix(Vector* pVector, Matrix* pMatrix) {
    Vector temp;
    float vx;
    float vy;
    float vz;

    vx = pVector->x;
    vy = pVector->y;
    vz = pVector->z;
    float mx = (vx * pMatrix->data[0][0]) + (vy * pMatrix->data[1][0]) + (vz * pMatrix->data[2][0]);
    float my = (vx * pMatrix->data[0][1]) + (vy * pMatrix->data[1][1]) + (vz * pMatrix->data[2][1]);
    float mz = (vx * pMatrix->data[0][2]) + (vy * pMatrix->data[1][2]) + (vz * pMatrix->data[2][2]);

    temp.Set(mx, my, mz);
    Copy(&temp);
}

void Vector::ApplyTransMatrix(Vector* pVector, Matrix* pMatrix) {
    x = pVector->x + pMatrix->data[3][0];
    y = pVector->y + pMatrix->data[3][1];
    z = pVector->z + pMatrix->data[3][2];
}

void Vector::CClamp(Vector* pColour, float min, float max) {
    x = Clamp<float>(pColour->x, min, max);
    y = Clamp<float>(pColour->y, min, max);
    z = Clamp<float>(pColour->z, min, max);
    w = Clamp<float>(pColour->w, min, max);
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