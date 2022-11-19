#ifndef COMMON_VECTOR
#define COMMON_VECTOR

//#include "common/Matrix.h"

struct Matrix;

// uses unions for rgba and pitch yaw roll?
struct Vector {
	float x;
	float y;
	float z;
	float w;
	
	float Magnitude();
	float Normalise(Vector*);
	void ClampMagnitude(Vector*, float);
	void Cross(Vector*, Vector*);
	void Projection(Vector*, Vector*);
	void InterpolateLinear(Vector*, Vector*, float);
	void ApplyMatrix(Vector*, Matrix*);
	void ApplyMatrixW(Vector*, Matrix*);
	void ApplyRotMatrix(Vector*, Matrix*);
	void ApplyTransMatrix(Vector*, Matrix*);
	void CClamp(Vector*, float, float);
	void NormaliseRot(Vector*);
    inline float MagSquared() { return x*x + y*y + z*z; }
    inline float Dot(Vector* pVector) {
        return x * pVector->x + y * pVector->y + z * pVector->z;
    }
    inline float DistSq(Vector* pVector) {
        float dx = x - pVector->x;
        float dy = y - pVector->y;
        float dz = z - pVector->z;
        return dx * dx + dy * dy + dz * dz;
    }
	inline void Set(float x1, float y1, float z1) {
		x = x1;
		y = y1;
		z = z1;
	}
	inline void Set(float x1, float y1, float z1, float w1) {
		x = x1;
		y = y1;
		z = z1;
		w = w1;
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
    void Scale(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}
};

#endif COMMON_VECTOR