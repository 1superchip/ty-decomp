#include "types.h"
#include "common/QuatRotation.h"
#include "common/StdMath.h" // _table_sinf

void QuatRotation::ConvertRotation(AxisRotation* pRotation) {
    float wRot = 0.5f * pRotation->unk10;
    float sin = _table_sinf(wRot);
    quat.x = sin * pRotation->rot.x;
    quat.y = sin * pRotation->rot.y;
    quat.z = sin * pRotation->rot.z;
    quat.w = _table_cosf(wRot);
}

void QuatRotation::ConvertNormal(Vector* pNormal, float r) {
    float wRot = 0.5f * r;
    float sin = _table_sinf(wRot);
    quat.x = sin * pNormal->x;
    quat.y = sin * pNormal->y;
    quat.z = sin * pNormal->z;
    quat.w = _table_cosf(wRot);
}

void QuatRotation::ConvertVector(Vector* pVector) {
    float vecMag = pVector->Magnitude();
    if (vecMag > 0.000001f) {
        float sin = vecMag * 0.5f;
        float norm = _table_sinf(sin) / vecMag;
        quat.x = norm * pVector->x;
        quat.y = norm * pVector->y;
        quat.z = norm * pVector->z;
        quat.w = _table_cosf(sin);
        return;
    }
    quat.SetZero();
    quat.w = 1.0f;
}

void QuatRotation::Multiply(QuatRotation *pQuaternion1, QuatRotation *pQuaternion2) {
    Vector sp0;
    Vector sp10;
    sp0.w = (pQuaternion2->quat.w * pQuaternion1->quat.w) - pQuaternion2->quat.Dot(&pQuaternion1->quat);
    sp0.Scale(&pQuaternion1->quat, pQuaternion2->quat.w);
    sp10.Scale(&pQuaternion2->quat, pQuaternion1->quat.w);
    sp0.Add(&sp10);
    sp10.Cross(&pQuaternion2->quat, &pQuaternion1->quat);
    sp0.Add((Vector*)&sp0, &sp10); // cast is needed to force stack usage
    quat = sp0;
    quat.w = sp0.w;
}