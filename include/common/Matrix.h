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
};


#endif // COMMON_MATRIX