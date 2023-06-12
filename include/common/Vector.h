#ifndef COMMON_VECTOR
#define COMMON_VECTOR

//#include "common/Matrix.h"
#include "common/StdMath.h"

struct Matrix;

// uses unions for rgba and pitch yaw roll?
struct Vector {
	float x;
	float y;
	float z;
	float w;
	
	float Magnitude();
	float Normalise(Vector*);
	void ClampMagnitude(Vector* pVector, float);
	void Cross(Vector*, Vector*);
	void Projection(Vector*, Vector*);
	void InterpolateLinear(Vector*, Vector*, float);
	void ApplyMatrix(Vector*, Matrix*);
	void ApplyMatrixW(Vector*, Matrix*);
	void ApplyRotMatrix(Vector* pVector, Matrix* pMatrix);
	void ApplyTransMatrix(Vector*, Matrix*);
	void CClamp(Vector*, float, float);
	void NormaliseRot(Vector*);
    inline float MagSquared() { return x*x + y*y + z*z; }
    inline float Dot(Vector* pVector) {
        return x * pVector->x + y * pVector->y + z * pVector->z;
    }
    inline float QuatDot(Vector* pVector) {
        return x * pVector->x + y * pVector->y + z * pVector->z + w * pVector->w;
    }
    inline float DistSq(Vector* pVector) {
        float dx = x - pVector->x;
        float dy = y - pVector->y;
        float dz = z - pVector->z;
        return dx * dx + dy * dy + dz * dz;
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
		x = pOther->x * -1.0f;
		y = pOther->y * -1.0f;
		z = pOther->z * -1.0f;
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
    inline bool Equals(Vector* pOther) {
        return x == pOther->x && y == pOther->y && z == pOther->z;
    }
	// possible place this in another file?
	// doesn't really need to be in the vector class
	// Collision.h or StdMath.h?
    bool CheckSphereRadius(Vector* pCentre, float radius) {
        return (x - pCentre->x) * (x - pCentre->x) + 
            (y - pCentre->y) * (y - pCentre->y) + 
            (z - pCentre->z) * (z - pCentre->z) < radius * radius;
    }
	/* this might be the correct version since the July 1st build needs the inline "CheckSphereRadius" to be used in
	// Tools_CapsuleTest(Vector* pVec, Vector* pVec1, float f1, float f2, Vector* pVec2) but the current doesn't match on GC
	*/
    // bool CheckSphereRadius(Vector* pCentre, float radius) {
	// 	// this might be closer?
	// 	// need to check debug build
    //     return DistSq(pCentre) < Sqr<float>(radius);
    // }
	// this might be the correct inline?
    // bool CheckSphereRadius(Vector* pCentre, float radius) {
    //     float dx = (x - pCentre->x);
    //     float dy = (y - pCentre->y);
    //     float dz = (z - pCentre->z);
    //     return dx * dx + dy * dy + dz * dz < radius * radius;
    // }
    float Normalise(void) {
        return Normalise(this);
    }
};


inline bool CompareVectors(Vector* pVec, Vector* pVec1) {
	if (pVec->x == pVec1->x && pVec->y == pVec1->y && pVec->z == pVec1->z)
		return true;
	return false;
}

#endif // COMMON_VECTOR