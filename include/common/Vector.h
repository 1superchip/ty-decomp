#ifndef COMMON_VECTOR
#define COMMON_VECTOR

//#include "common/Matrix.h"
#include "common/StdMath.h"

struct Matrix;
struct QuatRotation;

// uses unions for rgba and pitch yaw roll?
struct Vector {
    float x;
    float y;
    float z;
    float w;
    
    float Magnitude(void);
    float Normalise(Vector* pVector);
    void ClampMagnitude(Vector* pVector, float maxMag);
    void Cross(Vector* pVector1, Vector* pVector2);
    void Projection(Vector*, Vector*);
    void InterpolateLinear(Vector* pFrom, Vector* pTo, float fraction);
    void ApplyMatrix(Vector* pVector, Matrix* pMatrix);
    void ApplyMatrixW(Vector* pVector, Matrix* pMatrix);
    void ApplyRotMatrix(Vector* pVector, Matrix* pMatrix);
    void ApplyTransMatrix(Vector* pVector, Matrix* pMatrix);
    void CClamp(Vector*, float, float);
    void NormaliseRot(Vector* pVector);

    void ApplyQuaternion(Vector* pVector, QuatRotation* pQuaternion); // Unused

    inline float MagSquared() {
        return x*x + y*y + z*z;
    }
    
    inline float Dot(Vector* pVector) {
        return x * pVector->x + y * pVector->y + z * pVector->z;
    }

    inline float QuatDot(Vector* pVector) {
        return x * pVector->x + y * pVector->y + z * pVector->z + w * pVector->w;
    }

    inline void Set(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }

    inline void Set(float _x, float _y, float _z, float _w) {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    void Copy(Vector* src) {
        x = src->x;
        y = src->y;
        z = src->z;
    }

    void Add(Vector* pSrc, Vector* pImm) {
        x = pSrc->x + pImm->x;
        y = pSrc->y + pImm->y;
        z = pSrc->z + pImm->z;
    }

    // Adds pOther to this
    void Add(Vector* pOther) {
        x += pOther->x;
        y += pOther->y;
        z += pOther->z;
    }

    // this = pSrc - pOther
    void Sub(Vector* pSrc, Vector* pOther) {
        x = pSrc->x - pOther->x;
        y = pSrc->y - pOther->y;
        z = pSrc->z - pOther->z;
    }
    
    void Subtract(Vector* pOther) {
        x -= pOther->x;
        y -= pOther->y;
        z -= pOther->z;
    }

    void Scale(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    void Scale(Vector* pVector, float scalar) {
        x = scalar * pVector->x;
        y = scalar * pVector->y;
        z = scalar * pVector->z;
    }

    void Scale(Vector* pVector, Vector* pVector1) {
        x = pVector->x * pVector1->x;
        y = pVector->y * pVector1->y;
        z = pVector->z * pVector1->z;
    }

    void SetZero(void) {
        x = y = z = 0.0f;
    }

    void Inverse(Vector* pOther) {
        this->Scale(pOther, -1.0f);
    }

    void Inverse(void) {
        Inverse(this);
    }

    void CMultiply(Vector* pOther, float scalar) {
        x = pOther->x * scalar;
        y = pOther->y * scalar;
        z = pOther->z * scalar;
        w = pOther->w * scalar;
    }

    void CMultiply(float scalar) {
        CMultiply(this, scalar);
    }

    void Cross(Vector* pOther) {
        Cross(this, pOther);
    }

    void ApplyMatrix(Matrix* pMatrix) {
        ApplyMatrix(this, pMatrix);
    }

    void ClampMagnitude(float maxMag) {
        ClampMagnitude(this, maxMag);
    }

    void ApplyRotMatrix(Matrix* pMatrix) {
        ApplyRotMatrix(this, pMatrix);
    }

    void NormaliseRot(void) {
        NormaliseRot(this);
    }

    void ApplyQuaternion(QuatRotation* pQuaternion) {
        ApplyQuaternion(this, pQuaternion);
    }

    inline bool Equals(Vector* pOther) {
        return x == pOther->x && y == pOther->y && z == pOther->z;
    }

    // is this Vector::IsNear(Vector* pVector, float distance) from Sunny Garcia? that was defined in 
    // Vector.cpp and this is an inline. Camera::Update calls Vector::IsNear in Sunny Garcia where the debug build
    // of Ty calls an inline that doesn't have any assertions whereas Vector::IsNear has assertions
    bool IsInsideSphere(Vector* pCentre, float radius) {
        float dx = (x - pCentre->x);
        float dy = (y - pCentre->y);
        float dz = (z - pCentre->z);
        return dx * dx + dy * dy + dz * dz < radius * radius;
    }

    float Normalise(void) {
        return Normalise(this);
    }

    // Used in a few cases to get the Alpha of a color vector
    float* GetAlpha(void) {
        return &w;
    }
#if mips
// PS2 GCC has mips defined
// Vector structs are aligned to 0x10 in the July 1st build
} __attribute__((aligned(16)));
#else
};
#endif

inline float SquareDistance(const Vector* p, const Vector* pVector) {
    float dx = p->x - pVector->x;
    float dy = p->y - pVector->y;
    float dz = p->z - pVector->z;
    return dx * dx + dy * dy + dz * dz;
}

inline float Dist2D(Vector* pVec, Vector* pVec1) {
    // might be a Vector method?
    return (Sqr<float>(pVec->x - pVec1->x) + Sqr<float>(pVec->z - pVec1->z));
}


inline bool CompareVectors(Vector* pVec, Vector* pVec1) {
    if (pVec->x == pVec1->x && pVec->y == pVec1->y && pVec->z == pVec1->z)
        return true;
    return false;
}

extern Vector gXAxis;
extern Vector gYAxis;
extern Vector gZAxis;

#endif // COMMON_VECTOR
