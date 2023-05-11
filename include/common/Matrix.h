#ifndef COMMON_MATRIX
#define COMMON_MATRIX

#include "common/Vector.h"
#include "common/QuatRotation.h"

struct Matrix {
	float data[4][4];
	
	void CopyRotation(Matrix*);
	void CopyTranslation(Matrix*);
	void SetIdentity(void);
	void SetTranslation(Vector*);
	void SetRotationToNone(void);
	void SetRotationQ(QuatRotation*);
	void SetRotationPitch(float);
	void SetRotationYaw(float);
	void SetRotationRoll(float);
	void SetRotationPYR(Vector*);
	void SetRotationToScale(float);
	void SetRotationToScaleXYZ(Vector*);
	void Transpose(Matrix*);
	void Transpose3x3(Matrix*);
	void InverseSimple(Matrix*);
	void Multiply(Matrix*, Matrix*);
	void Multiply3x3(Matrix*, Matrix*);
	void Multiply4x4(Matrix*, Matrix*);
	void Scale(Matrix*, float);
	void Scale(Matrix*, Vector*);
	void Translate(Matrix*, Vector*);
	void RotateQ(Matrix*, QuatRotation*);
	void RotatePitch(Matrix*, float);
	void RotateYaw(Matrix*, float);
	void RotateRoll(Matrix*, float);
	void RotatePYR(Matrix*, Vector*);
	void GetRotationPYR(Vector*);
	void Inverse(Matrix*);
	void SetLookAt(Vector* arg1, Vector* arg2) {
		Vector cross;
		Vector up;
		Vector normalizedDiff;
		normalizedDiff.x = arg2->x - arg1->x;
		normalizedDiff.y = arg2->y - arg1->y;
		normalizedDiff.z = arg2->z - arg1->z;
		normalizedDiff.Normalise(&normalizedDiff);
		up.Set(0.0f, 1.0f, 0.0f);
		cross.Cross(&normalizedDiff, &up);
		cross.Normalise(&cross);
		up.Cross(&cross, &normalizedDiff);
		Row0()->Copy(&cross);
		data[0][3] = 0.0f;
		Row1()->Copy(&up);
		data[1][3] = 0.0f;
		Row2()->Copy(&normalizedDiff);
		data[2][3] = 0.0f;
	}

    void RotateYaw(float yaw) {
        RotateYaw(this, yaw);
    }
    void RotateRoll(float roll) {
        RotateRoll(this, roll);
    }
	Vector* Row0(void) {
		return (Vector*)&data[0][0];
	}
	Vector* Row1(void) {
		return (Vector*)&data[1][0];
	}
	Vector* Row2(void) {
		return (Vector*)&data[2][0];
	}
	Vector* Row3(void) {
		return (Vector*)&data[3][0];
	}
    void Multiply(Matrix* pOther) {
        Multiply(this, pOther);
    }
    void Translate(Vector* pVector) {
        Translate(this, pVector);
    }
	void InverseSimple(void) {
		InverseSimple(this);
	}
	void Scale(Vector* pScale) {
		Scale(this, pScale);
	}
};


#endif // COMMON_MATRIX