#include "ty/LineOfSight.h"
#include "common/Collision.h"
#include "common/StdMath.h"

void LineOfSightObject::Init(u8 defaultFlags, float param_2, float param_3, float _maxRayMag, 
        float _radius, float param_6, float _fovRatio, float param_8) {
    flags = defaultFlags;
    unk4 = 0;
    length = 0.0f;
    unkC = 0.0f;
    startPos.SetZero();
    targetPos.SetZero();
    horizDir.SetZero();
    radius = _radius;
    unk14 = param_6;
    fovRatio = _fovRatio;
    unk64 = 0;
    maxRayMag = _maxRayMag;
    unk6C = param_2;
    unk70 = param_3;
    unk74 = param_8;
}

void LineOfSightObject::Reset(Vector* pStartPos, Vector* pTargetPos, Vector* pRotation) {
    unk4 = 1;
    startPos.Copy(pStartPos);
    rotation.Copy(pRotation);
    targetPos.Copy(pTargetPos);
    length = ApproxMag(&targetPos, &startPos);
    horizDir.Sub(&targetPos, &startPos);
    horizDir.y = 0.0f; // horizontal direction
    horizDir.Scale(1.0f / length);
    if (!(flags & 1)) {
        unk64 = (int)(length / maxRayMag);
        unk64++;
        unkC = length / (float)unk64;
    }
    if (!(flags & 2)) {
        normalizedYDir = (targetPos.y - startPos.y) / length;
    }
}

bool LineOfSightObject::Update(Vector* pStartPos, Vector* pTargetPos, Vector* pRotation) {
    // these may be in the wrong order
    CollisionResult cr; // 0x28
    Vector start;
    Vector end;
    Vector testPos;
    if (!unk4) {
        Reset(pStartPos, pTargetPos, pRotation);
    }
    flags &= ~0x4;
    flags &= ~0x8;
    flags &= ~0x10;
    if (length > unk70) {
        unk4 = 0;
        return true;
    }
    flags |= 8;
    if (fovRatio == 1.0f) {
        unk4 = 0;
        return true;
    }
    if ((fovRatio < 1.0f) && (fovRatio > rotation.Dot(&horizDir))) {
        unk4 = 0;
        flags |= 0x10;
        return true;
    }
    if (length <= unk6C) {
        unk4 = 0;
        flags |= 4;
        return true;
    }
    if (OutsideGOV()) {
        unk4 = 0;
        return true;
    }
    if (flags & 1) {
        unk4 = 0;
        flags |= 4;
        return true;
    }
    start = startPos;
    end = targetPos;
    end.y = 0.0f;
    start.y = 0.0f;
    float m = (float)unk4 * unkC;
    start.Scale(&horizDir, m - unkC);
    end.Scale(&horizDir, m);
    start.Add(&startPos);
    end.Add(&startPos);
    start.y += unk14;
    end.y += unk14;
    bool sweepResult = Collision_SweepSphereCollide(&start, &end,
        radius, &cr, COLLISION_MODE_ALL, 0);
    if (unk4 == unk64 && sweepResult) {
        float maxRadius = 2.0f * radius;
        if (ApproxMag(&targetPos, &cr.pos) <= maxRadius) {
            end.Scale(&cr.normal, maxRadius);
            start.Add(&end);
            end.Add(&cr.pos);
            end.y = start.y;
            sweepResult = Collision_SweepSphereCollide(&start, &end,
                radius, &cr, COLLISION_MODE_ALL, 0);
        }
    }
    if (sweepResult) {
        unk4 = 0;
        return true;
    }
    if (++unk4 > unk64) {
        unk4 = 0;
        flags |= 4;
        return true;
    }
    return false;
}

bool LineOfSightObject::OutsideGOV(void) {
    return (!(flags & 2) && Abs<float>(normalizedYDir) > unk74);
}
