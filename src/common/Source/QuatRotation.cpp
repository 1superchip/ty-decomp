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

void QuatRotation::Multiply(QuatRotation *arg1, QuatRotation *arg2) {
    Vector sp0;
    Vector sp10;
    Vector* p0 = &sp0; // this line is needed to get CW to use stack rather than float registers
    p0->w = (arg2->quat.w * arg1->quat.w) - arg2->quat.Dot(&arg1->quat);
    ((Vector*)p0)->Scale(&arg1->quat, arg2->quat.w); // this cast is needed...
    sp10.Scale(&arg2->quat, arg1->quat.w);
    p0->Add(&sp10);
    sp10.Cross(&arg2->quat, &arg1->quat);
    p0->Add(&sp10);
    quat = sp0;
    quat.w = p0->w; // fake?
}