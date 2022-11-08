#include "types.h"
#include "common/QuatRotation.h"
#include "common/StdMath.h" // _table_sinf

void QuatRotation::ConvertRotation(AxisRotation* pRotation) {
    float wRot = 0.5f * pRotation->unkC;
    float sin = _table_sinf(wRot);
    quat.x = sin * pRotation->rot.x;
    quat.y = sin * pRotation->rot.y;
    quat.z = sin * pRotation->rot.z;
    quat.w = _table_sinf(1.5707964f + wRot);
}

void QuatRotation::ConvertNormal(Vector* pNormal, float r) {
    float wRot = 0.5f * r;
    float sin = _table_sinf(wRot);
    quat.x = sin * pNormal->x;
    quat.y = sin * pNormal->y;
    quat.z = sin * pNormal->z;
    quat.w = _table_sinf(1.5707964f + wRot);
}

void QuatRotation::ConvertVector(Vector* pVector) {
    float vecMag = pVector->Magnitude();
    if (vecMag > 0.000001f) {
        float sin = vecMag * 0.5f;
        float norm = _table_sinf(sin) / vecMag;
        quat.x = norm * pVector->x;
        quat.y = norm * pVector->y;
        quat.z = norm * pVector->z;
        quat.w = _table_sinf(1.5707964f + sin);
        return;
    }
    quat.z = 0.0f;
    quat.y = 0.0f;
    quat.x = 0.0f;
    quat.w = 1.0f;
}

// last function
// nonmatching 
// https://decomp.me/scratch/cue99