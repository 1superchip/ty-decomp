#include "types.h"
#include "common/Matrix.h"
#include "common/StdMath.h"
#include "common/QuatRotation.h"

extern "C" double atan2(double, double);

void Matrix::CopyRotation(Matrix* pMatrix) {
    *Row0() = *pMatrix->Row0();
    *Row1() = *pMatrix->Row1();
    *Row2() = *pMatrix->Row2();
}

void Matrix::CopyTranslation(Matrix* pMatrix) {
    *Row3() = *pMatrix->Row3();
}

void Matrix::SetIdentity(void) {
	Row0()->Set(1.0f, 0.0f, 0.0f, 0.0f);
	Row1()->Set(0.0f, 1.0f, 0.0f, 0.0f);
	Row2()->Set(0.0f, 0.0f, 1.0f, 0.0f);
	Row3()->Set(0.0f, 0.0f, 0.0f, 1.0f);
}

void Matrix::SetTranslation(Vector* pVector) {
    Row3()->Set(pVector->x, pVector->y, pVector->z, 1.0f);
}

void Matrix::SetRotationToNone(void) {
	Row0()->Set(1.0f, 0.0f, 0.0f, 0.0f);
	Row1()->Set(0.0f, 1.0f, 0.0f, 0.0f);
	Row2()->Set(0.0f, 0.0f, 1.0f, 0.0f);
}

void Matrix::SetRotationQ(QuatRotation* pQuaternion) {
    Vector tempq;
    tempq = pQuaternion->quat;
    
    data[0][0] = 1.0f - 2.0f * (tempq.y * tempq.y + tempq.z * tempq.z);
    data[0][1] = 2.0f * (tempq.x * tempq.y - tempq.w * tempq.z);
    data[0][2] = 2.0f * (tempq.x * tempq.z + tempq.w * tempq.y);

    data[1][0] = 2.0f * (tempq.x * tempq.y + tempq.w * tempq.z);
    data[1][1] = 1.0f - 2.0f * (tempq.x * tempq.x + tempq.z * tempq.z);
    data[1][2] = 2.0f * (tempq.y * tempq.z - tempq.w * tempq.x);

    data[2][0] = 2.0f * (tempq.x * tempq.z - tempq.w * tempq.y);
    data[2][1] = 2.0f * (tempq.y * tempq.z + tempq.w * tempq.x);
    data[2][2] = 1.0f - 2.0f * (tempq.x * tempq.x + tempq.y * tempq.y);

    data[0][3] = data[1][3] = data[2][3] = 0.0f;
}

void Matrix::SetRotationPitch(float pitch) {
    float s = _table_sinf(pitch);
    float t = _table_cosf(pitch);
    Row0()->Set(1.0f, 0.0f, 0.0f, 0.0f);
    Row1()->Set(0.0f, t, -s, 0.0f);
    Row2()->Set(0.0f, s, t, 0.0f);
}

void Matrix::SetRotationYaw(float yaw) {
    float s = _table_sinf(yaw);
    float t = _table_cosf(yaw);
    Row0()->Set(t, 0.0f, s, 0.0f);
    Row1()->Set(0.0f, 1.0f, 0.0f, 0.0f);
    Row2()->Set(-s, 0.0f, t, 0.0f);
}

void Matrix::SetRotationRoll(float roll) {
    float s = _table_sinf(roll);
    float t = _table_cosf(roll);
    Row0()->Set(t, -s, 0.0f, 0.0f);
    Row1()->Set(s, t, 0.0f, 0.0f);
    Row2()->Set(0.0f, 0.0f, 1.0f, 0.0f);
}

void Matrix::SetRotationPYR(Vector* pAngles) {
    Vector tempv;
    Matrix tempm;
    tempv.x = pAngles->x;
    tempv.y = pAngles->y;
    tempv.z = pAngles->z;
    SetRotationPitch(tempv.x);
    tempm.SetRotationYaw(tempv.y);
    Multiply3x3(&tempm);
    tempm.SetRotationRoll(tempv.z);
    Multiply3x3(&tempm);
}

void Matrix::SetRotationToScale(float scale) {
    Row0()->Set(scale, 0.0f, 0.0f, 0.0f);
    Row1()->Set(0.0f, scale, 0.0f, 0.0f);
    Row2()->Set(0.0f, 0.0f, scale, 0.0f);
}

void Matrix::SetRotationToScaleXYZ(Vector* pScale) {
    Vector tempv;
    tempv.Copy(pScale);
    Row0()->Set(tempv.x, 0.0f, 0.0f, 0.0f);
    Row1()->Set(0.0f, tempv.y, 0.0f, 0.0f);
    Row2()->Set(0.0f, 0.0f, tempv.z, 0.0f);
}

void Matrix::Transpose(Matrix* pMatrix) {
    Matrix tempm;
    tempm.Row0()->Set(pMatrix->data[0][0], pMatrix->data[1][0], pMatrix->data[2][0], pMatrix->data[3][0]);
    tempm.Row1()->Set(pMatrix->data[0][1], pMatrix->data[1][1], pMatrix->data[2][1], pMatrix->data[3][1]);
    tempm.Row2()->Set(pMatrix->data[0][2], pMatrix->data[1][2], pMatrix->data[2][2], pMatrix->data[3][2]);
    tempm.Row3()->Set(pMatrix->data[0][3], pMatrix->data[1][3], pMatrix->data[2][3], pMatrix->data[3][3]);
    *this = tempm;
}

void Matrix::Transpose3x3(Matrix* pMatrix) {
    Matrix tempm;
    tempm.Row0()->Set(pMatrix->data[0][0], pMatrix->data[1][0], pMatrix->data[2][0], 0.0f);
    tempm.Row1()->Set(pMatrix->data[0][1], pMatrix->data[1][1], pMatrix->data[2][1], 0.0f);
    tempm.Row2()->Set(pMatrix->data[0][2], pMatrix->data[1][2], pMatrix->data[2][2], 0.0f);
    tempm.CopyTranslation(pMatrix);
    *this = tempm;
}

// InverseTranspose or Adjoint of a matrix?
// This is a "simple" inverse function for an Orthogonal matrix
// The tranpose of an orthogonal matrix is equal to its inverse
// this only works as the inverse function for an orthogonal matrix
void Matrix::InverseSimple(Matrix* pMatrix) {
    Matrix tempm;
    tempm.Transpose3x3(pMatrix);
    tempm.data[3][0] = -pMatrix->Row3()->Dot(pMatrix->Row0());
    tempm.data[3][1] = -pMatrix->Row3()->Dot(pMatrix->Row1());
    tempm.data[3][2] = -pMatrix->Row3()->Dot(pMatrix->Row2());
    tempm.data[3][3] = 1.0f;
    *this = tempm;
}

void Matrix::Multiply(Matrix *pMatrix1, Matrix *pMatrix2) {
    Matrix tempm;

    tempm.data[0][0] = pMatrix1->data[0][0] * pMatrix2->data[0][0] + pMatrix1->data[0][1] * pMatrix2->data[1][0] + pMatrix1->data[0][2] * pMatrix2->data[2][0];
    tempm.data[0][1] = pMatrix1->data[0][0] * pMatrix2->data[0][1] + pMatrix1->data[0][1] * pMatrix2->data[1][1] + pMatrix1->data[0][2] * pMatrix2->data[2][1];
    tempm.data[0][2] = pMatrix1->data[0][0] * pMatrix2->data[0][2] + pMatrix1->data[0][1] * pMatrix2->data[1][2] + pMatrix1->data[0][2] * pMatrix2->data[2][2];
    tempm.data[1][0] = pMatrix1->data[1][0] * pMatrix2->data[0][0] + pMatrix1->data[1][1] * pMatrix2->data[1][0] + pMatrix1->data[1][2] * pMatrix2->data[2][0];
    tempm.data[1][1] = pMatrix1->data[1][0] * pMatrix2->data[0][1] + pMatrix1->data[1][1] * pMatrix2->data[1][1] + pMatrix1->data[1][2] * pMatrix2->data[2][1];
    tempm.data[1][2] = pMatrix1->data[1][0] * pMatrix2->data[0][2] + pMatrix1->data[1][1] * pMatrix2->data[1][2] + pMatrix1->data[1][2] * pMatrix2->data[2][2];
    tempm.data[2][0] = pMatrix1->data[2][0] * pMatrix2->data[0][0] + pMatrix1->data[2][1] * pMatrix2->data[1][0] + pMatrix1->data[2][2] * pMatrix2->data[2][0];
    tempm.data[2][1] = pMatrix1->data[2][0] * pMatrix2->data[0][1] + pMatrix1->data[2][1] * pMatrix2->data[1][1] + pMatrix1->data[2][2] * pMatrix2->data[2][1];
    tempm.data[2][2] = pMatrix1->data[2][0] * pMatrix2->data[0][2] + pMatrix1->data[2][1] * pMatrix2->data[1][2] + pMatrix1->data[2][2] * pMatrix2->data[2][2];
    tempm.data[3][0] = pMatrix1->data[3][0] * pMatrix2->data[0][0] + pMatrix1->data[3][1] * pMatrix2->data[1][0] + pMatrix1->data[3][2] * pMatrix2->data[2][0] + pMatrix2->data[3][0];
    tempm.data[3][1] = pMatrix1->data[3][0] * pMatrix2->data[0][1] + pMatrix1->data[3][1] * pMatrix2->data[1][1] + pMatrix1->data[3][2] * pMatrix2->data[2][1] + pMatrix2->data[3][1];
    tempm.data[2][3] = 0.0f;
    tempm.data[1][3] = 0.0f;
    tempm.data[0][3] = 0.0f;
    tempm.data[3][2] = pMatrix1->data[3][0] * pMatrix2->data[0][2] + pMatrix1->data[3][1] * pMatrix2->data[1][2] + pMatrix1->data[3][2] * pMatrix2->data[2][2] + pMatrix2->data[3][2];
    tempm.data[3][3] = 1.0f;
    *this = tempm;
}

void Matrix::Multiply3x3(Matrix* pMatrix1, Matrix* pMatrix2) {
    Matrix tempm;
    tempm.Row0()->x = pMatrix1->data[0][0] * pMatrix2->data[0][0] + pMatrix1->data[0][1] * pMatrix2->data[1][0] + pMatrix1->data[0][2] * pMatrix2->data[2][0];
    tempm.Row0()->y = pMatrix1->data[0][0] * pMatrix2->data[0][1] + pMatrix1->data[0][1] * pMatrix2->data[1][1] + pMatrix1->data[0][2] * pMatrix2->data[2][1];
    tempm.Row0()->z = pMatrix1->data[0][0] * pMatrix2->data[0][2] + pMatrix1->data[0][1] * pMatrix2->data[1][2] + pMatrix1->data[0][2] * pMatrix2->data[2][2];
    
    tempm.Row1()->x = pMatrix1->data[1][0] * pMatrix2->data[0][0] + pMatrix1->data[1][1] * pMatrix2->data[1][0] + pMatrix1->data[1][2] * pMatrix2->data[2][0];
    tempm.Row1()->y = pMatrix1->data[1][0] * pMatrix2->data[0][1] + pMatrix1->data[1][1] * pMatrix2->data[1][1] + pMatrix1->data[1][2] * pMatrix2->data[2][1];
    tempm.Row1()->z = pMatrix1->data[1][0] * pMatrix2->data[0][2] + pMatrix1->data[1][1] * pMatrix2->data[1][2] + pMatrix1->data[1][2] * pMatrix2->data[2][2];
    
    tempm.Row2()->x = pMatrix1->data[2][0] * pMatrix2->data[0][0] + pMatrix1->data[2][1] * pMatrix2->data[1][0] + pMatrix1->data[2][2] * pMatrix2->data[2][0];
    tempm.Row2()->y = pMatrix1->data[2][0] * pMatrix2->data[0][1] + pMatrix1->data[2][1] * pMatrix2->data[1][1] + pMatrix1->data[2][2] * pMatrix2->data[2][1];
    tempm.Row2()->z = pMatrix1->data[2][0] * pMatrix2->data[0][2] + pMatrix1->data[2][1] * pMatrix2->data[1][2] + pMatrix1->data[2][2] * pMatrix2->data[2][2];

    // set W of each row to 0.0f
    tempm.Row2()->w = 0.0f;
    tempm.Row1()->w = 0.0f;
    tempm.Row0()->w = 0.0f;

    // copy rows
    *Row0() = *tempm.Row0();
    *Row1() = *tempm.Row1();
    *Row2() = *tempm.Row2();
}

void Matrix::Multiply4x4(Matrix *pMatrix1, Matrix *pMatrix2) {
    Matrix tempm;
    float m00 = pMatrix1->data[0][0];
    float m01 = pMatrix1->data[0][1];
    float m02 = pMatrix1->data[0][2];
    float m03 = pMatrix1->data[0][3];
    float m10 = pMatrix1->data[1][0];
    float m11 = pMatrix1->data[1][1];
    float m12 = pMatrix1->data[1][2];
    float m13 = pMatrix1->data[1][3];
    float m20 = pMatrix1->data[2][0];
    float m21 = pMatrix1->data[2][1];
    float m22 = pMatrix1->data[2][2];
    float m23 = pMatrix1->data[2][3];
    float m30 = pMatrix1->data[3][0];
    float m31 = pMatrix1->data[3][1];
    float m32 = pMatrix1->data[3][2];
    float m33 = pMatrix1->data[3][3];

    float b00 = pMatrix2->data[0][0];
    float b01 = pMatrix2->data[0][1];
    float b02 = pMatrix2->data[0][2];
    float b03 = pMatrix2->data[0][3];
    float b10 = pMatrix2->data[1][0];
    float b11 = pMatrix2->data[1][1];
    float b12 = pMatrix2->data[1][2];
    float b13 = pMatrix2->data[1][3];
    float b20 = pMatrix2->data[2][0];
    float b21 = pMatrix2->data[2][1];
    float b22 = pMatrix2->data[2][2];
    float b23 = pMatrix2->data[2][3];
    float b30 = pMatrix2->data[3][0];
    float b31 = pMatrix2->data[3][1];
    float b32 = pMatrix2->data[3][2];
    float b33 = pMatrix2->data[3][3];
    
    tempm.data[0][0] = m00 * b00 + m01 * b10 + m02 * b20 + m03 * b30;
    tempm.data[0][1] = m00 * b01 + m01 * b11 + m02 * b21 + m03 * b31;
    tempm.data[0][2] = m00 * b02 + m01 * b12 + m02 * b22 + m03 * b32;
    tempm.data[0][3] = m00 * b03 + m01 * b13 + m02 * b23 + m03 * b33;
    
    tempm.data[1][0] = m10 * b00 + m11 * b10 + m12 * b20 + m13 * b30;
    tempm.data[1][1] = m10 * b01 + m11 * b11 + m12 * b21 + m13 * b31;
    tempm.data[1][2] = m10 * b02 + m11 * b12 + m12 * b22 + m13 * b32;
    tempm.data[1][3] = m10 * b03 + m11 * b13 + m12 * b23 + m13 * b33;

    tempm.data[2][0] = m20 * b00 + m21 * b10 + m22 * b20 + m23 * b30;
    tempm.data[2][1] = m20 * b01 + m21 * b11 + m22 * b21 + m23 * b31;
    tempm.data[2][2] = m20 * b02 + m21 * b12 + m22 * b22 + m23 * b32;
    tempm.data[2][3] = m20 * b03 + m21 * b13 + m22 * b23 + m23 * b33;

    tempm.data[3][0] = m30 * b00 + m31 * b10 + m32 * b20 + m33 * b30;
    tempm.data[3][1] = m30 * b01 + m31 * b11 + m32 * b21 + m33 * b31;
    tempm.data[3][2] = m30 * b02 + m31 * b12 + m32 * b22 + m33 * b32;
    tempm.data[3][3] = m30 * b03 + m31 * b13 + m32 * b23 + m33 * b33;
    *this = tempm;
}

void Matrix::Scale(Matrix* pMatrix, float scalar) {
    Row0()->Set(
        scalar * pMatrix->data[0][0], scalar * pMatrix->data[0][1],
        scalar * pMatrix->data[0][2], pMatrix->data[0][3]
    );
    Row1()->Set(
        scalar * pMatrix->data[1][0], scalar * pMatrix->data[1][1],
        scalar * pMatrix->data[1][2], pMatrix->data[1][3]
    );
    Row2()->Set(
        scalar * pMatrix->data[2][0], scalar * pMatrix->data[2][1],
        scalar * pMatrix->data[2][2], pMatrix->data[2][3]
    );
    Row3()->Set(pMatrix->data[3][0], pMatrix->data[3][1], pMatrix->data[3][2], pMatrix->data[3][3]);
}

void Matrix::Scale(Matrix* pMatrix, Vector* pScale) {
    Vector tempv;
    tempv.Copy(pScale);
    Row0()->Set(
        tempv.x * pMatrix->data[0][0], tempv.x * pMatrix->data[0][1],
        tempv.x * pMatrix->data[0][2], pMatrix->data[0][3]
    );
    Row1()->Set(
        tempv.y * pMatrix->data[1][0], tempv.y * pMatrix->data[1][1],
        tempv.y * pMatrix->data[1][2], pMatrix->data[1][3]
    );
    Row2()->Set(
        tempv.z * pMatrix->data[2][0], tempv.z * pMatrix->data[2][1],
        tempv.z * pMatrix->data[2][2], pMatrix->data[2][3]
    );
    Row3()->Set(pMatrix->data[3][0], pMatrix->data[3][1], pMatrix->data[3][2], pMatrix->data[3][3]);
}

void Matrix::Translate(Matrix* pMatrix, Vector* pVector) {
    Row3()->Add(pMatrix->Row3(), pVector);
    Row3()->w = pMatrix->Row3()->w;
    *Row0() = *pMatrix->Row0();
    *Row1() = *pMatrix->Row1();
    *Row2() = *pMatrix->Row2();
}

void Matrix::RotateQ(Matrix* pMatrix, QuatRotation* pQuaternion) {
    Matrix tempm;
    tempm.SetRotationQ(pQuaternion);
    tempm.Row3()->Set(0.0f, 0.0f, 0.0f, 1.0f);
    Multiply4x4(pMatrix, &tempm);
}

void Matrix::RotatePitch(Matrix* pMatrix, float ang) {
    Matrix tempm;
    tempm.SetRotationPitch(ang);
    tempm.Row3()->Set(0.0f, 0.0f, 0.0f, 1.0f);
    Multiply4x4(pMatrix, &tempm);
}

void Matrix::RotateYaw(Matrix* pMatrix, float ang) {
    Matrix tempm;
    tempm.SetRotationYaw(ang);
    tempm.Row3()->Set(0.0f, 0.0f, 0.0f, 1.0f);
    Multiply4x4(pMatrix, &tempm);
}

void Matrix::RotateRoll(Matrix* pMatrix, float ang) {
    Matrix tempm;
    tempm.SetRotationRoll(ang);
    tempm.Row3()->Set(0.0f, 0.0f, 0.0f, 1.0f);
    Multiply4x4(pMatrix, &tempm);
}

void Matrix::RotatePYR(Matrix* pMatrix, Vector* pAngles) {
    Matrix tempm;
    tempm.SetRotationPYR(pAngles);
    tempm.Row3()->Set(0.0f, 0.0f, 0.0f, 1.0f);
    Multiply4x4(pMatrix, &tempm);
}

void Matrix::GetRotationPYR(Vector* pAngles) {
    Matrix tempm;
    tempm = *this;
    if (Abs<float>(tempm.Row0()->x) < 0.001f && Abs<float>(tempm.Row0()->y) < 0.001f) {
        pAngles->z = 0.0f;
    } else if (tempm.Row0()->x > 0.0f) {
        pAngles->z = atan2(-tempm.Row0()->y, tempm.Row0()->x);
    } else if (tempm.Row0()->x < 0.0f) {
        pAngles->z = atan2(tempm.Row0()->y, -tempm.Row0()->x);
    }
    tempm.RotateRoll(-pAngles->z);
    pAngles->y = atan2(tempm.Row0()->z, tempm.Row0()->x);
    tempm.RotateYaw(-pAngles->y);
    pAngles->x = atan2(tempm.Row2()->y, tempm.Row2()->z);
}

void Matrix::Inverse(Matrix* pMatrix) {
    Vector pos = *pMatrix->Row3();
    float m00 = pMatrix->data[0][0];
    float m01 = pMatrix->data[0][1];
    float m02 = pMatrix->data[0][2];
    float m10 = pMatrix->data[1][0];
    float m11 = pMatrix->data[1][1];
    float m12 = pMatrix->data[1][2];
    float m20 = pMatrix->data[2][0];
    float m21 = pMatrix->data[2][1];
    float m22 = pMatrix->data[2][2];
    data[0][0] = (m11 * m22) - (m21 * m12);
    data[1][0] = (m20 * m12) - (m10 * m22);
    data[2][0] = (m10 * m21) - (m20 * m11);
    float det = m00 * data[0][0] + m01 * data[1][0] + m02 * data[2][0];
    data[0][1] = (m21 * m02) - (m01 * m22);
    data[1][1] = (m00 * m22) - (m20 * m02);
    data[2][1] = (m20 * m01) - (m00 * m21);
    data[0][2] = (m01 * m12) - (m11 * m02);
    data[1][2] = (m10 * m02) - (m00 * m12);
    data[2][2] = (m00 * m11) - (m10 * m01);
    if (det) {
        Scale(1.0f / det);
    }
    pos.ApplyRotMatrix(this);
    data[3][0] = -pos.x;
    data[3][1] = -pos.y;
    data[3][2] = -pos.z;
}
