#ifndef LINEOFSIGHT_H
#define LINEOFSIGHT_H

#include "types.h"
#include "common/Vector.h"

extern float ApproxMag(Vector*, Vector*); // should be in Tools.h?

struct LineOfSightObject {
    u8 flags;
    int unk4; // this needs to be set manually by the caller of Update to reset?
    float unk8;
    float unkC;
    float radius;
    float unk14;
    float fovRatio;
    float unk1C;
    float unk20;
    Vector startPos;
    Vector rotation;
    Vector targetPos;
    // horizontal direction between targetPos and startPos
    // calculated by (targetPos - startPos).Normalise()
    Vector horizDir; // y element should be set to 0.0f
    int unk64;
    float maxRayMag;
    float unk6C;
    float unk70;
    float unk74;
    void Init(u8, float, float, float, float, float, float, float);
    bool Update(Vector* pStartPos, Vector* pTargetPos, Vector* pRotation);
    bool OutsideGOV(void);
    void Reset(Vector* pStartPos, Vector* pTargetPos, Vector* pRotation); // maybe declared within the header?
};

#endif // LINEOFSIGHT_H