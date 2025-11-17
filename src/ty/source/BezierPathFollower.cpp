#include "ty/BezierPathFollower.h"
#include "ty/tools.h"

void DebugInfo_Line3d(char*, Vector*, Vector*, int, bool);
void DebugInfo_Sphere(char*, Vector*, float, int, Vector*);

void BezierPathFollower::Init(Vector* r4, int r5, float f1, float f2, bool r6) {
    mSpline.Init(r5 + 1, true);

    int i;

    for (i = 0; i < r5; i++) {
        mSpline.AddNode(&r4[i]);
    }

    if (r6) {
        mSpline.AddNode(&r4[0]);
    } else {
        mSpline.AddNode(&r4[r5 - 1]);
    }

    if (r6) {
        mSpline.MergeEnds();
    }

    unk14 = f1;
    unk24 = r6;
    unk1C = f2;

    unk28 = (f1 * 4.0f) / 5.0f;
    unk2C = (f1 * 6.0f) / 5.0f;

    unk30 = 0.2f;
    unk34 = 0.1f;
    unk38 = 10000.0f;

    unk20 = 0.0f;

    for (i = 0; i < mSpline.nodeIndex - 1; i++) {
        unk20 += ApproxMag(&mSpline.mpPoints[i].mPos, &mSpline.mpPoints[i + 1].mPos);
    }

    Reset();
}

void BezierPathFollower::Reset(void) {
    unk18 = 0;
    unk3C.SetZero();
    unk4C.SetZero();
}

void BezierPathFollower::Deinit(void) {
    mSpline.Deinit();
}

// Stripped
void BezierPathFollower::DrawPath(float f1) {
    Vector oldPos;
    Vector newPathPos;
    
    oldPos = mSpline.GetPosition(0.0f);
    oldPos.y += f1;

    for (float t = 0.0f; t < 1.0f; t += 0.005f) {
        newPathPos = mSpline.GetPosition(t);
        newPathPos.y += f1;
        
        DebugInfo_Line3d("-paths", &oldPos, &newPathPos, 1, false);
        oldPos = newPathPos;
    }

    for (int i = 0; i < mSpline.mNumPoints; i++) {
        newPathPos = mSpline.mpPoints[i].mPos;
        newPathPos.y += f1;
        DebugInfo_Sphere("-paths", &newPathPos, 20.0f, 1, NULL);
    }
}

// Stripped
void BezierPathFollower::DrawVelocities(float f1) {
    Vector oldPos;
    Vector newPathPos;

    for (float t = 0.0f; t < 1.0f; t += 0.01f) {
        oldPos = mSpline.GetPosition(t);
        
        newPathPos = mSpline.GetVelocity(t);
        newPathPos.Scale(0.1f);
        newPathPos.Add(&oldPos);
        
        oldPos.y += f1;
        newPathPos.y += f1;
        
        DebugInfo_Line3d("-paths", &oldPos, &newPathPos, 1, false);
    }

    for (int i = 0; i < mSpline.mNumPoints; i++) {
        oldPos = mSpline.mpPoints[i].mPos;
        
        newPathPos = mSpline.mpPoints[i].unk10;
        newPathPos.Scale(0.1f);
        newPathPos.Add(&oldPos);

        oldPos.y += f1;
        newPathPos.y += f1;
        
        DebugInfo_Line3d("-paths", &oldPos, &newPathPos, 1, false);
        DebugInfo_Sphere("-paths", &oldPos, 20.0f, 1, NULL);
    }
}

Vector BezierPathFollower::GetNewPosition(Vector* p, Vector* p1) {
    if (!unk14) {
        return *p;
    }

    unk0 = false;

    SplinePoint* pPoints = mSpline.mpPoints;

    if (p->IsInsideSphere(&pPoints[unk18 + 1].mPos, unk1C)) {
        if (unk18 + 1 == mSpline.mNumPoints - 1) {
            unk0 = true;
        }

        if (!unk24 && (unk18 + 1 == mSpline.mNumPoints - 1)) {
            p1->SetZero();

            return pPoints[unk18 + 1].mPos;
        }

        unk18++;

        if (unk18 == mSpline.mNumPoints - 1) {
            unk18 = 0;
        }
    }

    Vector startPos = *p;
    Vector startVel = *p1;
    Vector endPos = pPoints[unk18 + 1].mPos;
    Vector endVel = pPoints[unk18 + 1].unk10;

    float mag0 = ApproxMag(&endPos, &pPoints[unk18].mPos);
    float mag1 = ApproxMag(&endPos, &startPos);

    float f31 = (unk14 * mag0) / mag1;

    float f7 = 0.0f;
    if (mag0) {
        f7 = mag1 / mag0;
    }

    startVel.Scale(f7 / unk14);
    endVel.Scale(f7);

    *p1 = Spline_GetVelocity(
        &startPos, &startVel,
        &endPos, &endVel,
        f31
    );

    p1->Scale(f31);

    unk4C = *p1;

    unk3C = Spline_GetPosition(
        &startPos, &startVel,
        &endPos, &endVel,
        f31
    );

    return unk3C;
}

bool BezierPathFollower::AtLastPoint(void) {
    return unk0;
}

extern "C" double atan2(double, double);

void BezierPathFollower::GetStartPositionAndRotation(Vector* p, Vector* p1) {
    SplinePoint* pPoints = mSpline.mpPoints;

    *p = pPoints[0].mPos;

    p1->y = atan2(pPoints[0].unk10.x, -pPoints[0].unk10.z);
}

Vector BezierPathFollower::GetPreviousNode(void) {
    return mSpline.mpPoints[unk18].mPos;
}

Vector BezierPathFollower::GetNextNode(void) {
    return mSpline.mpPoints[unk18 + 1].mPos;
}

float BezierPathFollower::GetPositionAlongPath(Vector* p) {
    int r31 = -1;

    float closestDist = Sqr<float>(10000.0f);

    int i;

    for (i = 0; i < mSpline.nodeIndex; i++) {
        float dist = SquareDistance(p, &mSpline.mpPoints[i].mPos);

        if (dist < closestDist) {
            r31 = i;
            closestDist = dist;
        }
    }

    if (r31 == -1) {
        return 0.0f;
    }

    if (unk24 && r31 == 0) {
        if (
            SquareDistance(&mSpline.mpPoints[r31 + 1].mPos, p) >
            SquareDistance(&mSpline.mpPoints[mSpline.nodeIndex - 2].mPos, p)
        ) {
            r31 = mSpline.nodeIndex - 1;
        }
    }

    float f28 = 0.0f;
    for (i = 0; i < r31; i++) {
        f28 += ApproxMag(&mSpline.mpPoints[i].mPos, &mSpline.mpPoints[i + 1].mPos);
    }

    Vector diff;
    diff.Sub(p, &mSpline.mpPoints[r31].mPos);

    Vector dir;
    if (r31 == 0) {
        dir.Sub(&mSpline.mpPoints[r31 + 1].mPos, &mSpline.mpPoints[r31].mPos);
    } else if (r31 == mSpline.nodeIndex - 1) {
        dir.Sub(&mSpline.mpPoints[r31].mPos, &mSpline.mpPoints[r31 - 1].mPos);
    } else if (diff.Dot(&mSpline.mpPoints[r31].unk10) < 0.0f) {
        dir.Sub(&mSpline.mpPoints[r31].mPos, &mSpline.mpPoints[r31 - 1].mPos);
    } else {
        dir.Sub(&mSpline.mpPoints[r31 + 1].mPos, &mSpline.mpPoints[r31].mPos);
    }

    dir.Normalise();

    f28 += diff.Dot(&dir);

    return f28;
}

void BezierPathFollower::MatchVelocityTo(Vector* p, Vector* p1) {
    float f31 = GetPositionAlongPath(p);
    float f1 = GetPositionAlongPath(&unk3C);

    float f2 = unk38 * Clamp<float>(0.0f, 1.0f - (f31 / unk20), 1.0f);

    float f4 = Abs<float>(f31 - f1);

    if (f4 < f2) {
        return;
    }

    float magDiff = p1->MagSquared() - unk4C.MagSquared();
    if (f31 > f1 && magDiff > 0.0f) {
        unk14 += (unk2C - unk28) * unk30;
    } else if (f31 < f1 && magDiff < 0.0f) {
        unk14 -= (unk2C - unk28) * unk34;
    }

    if (unk14 > unk2C) {
        unk14 = unk2C;
    } else if (unk14 < unk28) {
        unk14 = unk28;
    }
}

Vector BezierPathFollower::GetOnPathPosition(void) {
    float mag0 = ApproxMag(&unk3C, &mSpline.mpPoints[unk18 + 1].mPos);
    float mag1 = ApproxMag(&mSpline.mpPoints[unk18].mPos, &mSpline.mpPoints[unk18 + 1].mPos);
    
    return Spline_GetPosition(
        &mSpline.mpPoints[unk18].mPos, &mSpline.mpPoints[unk18].unk10, 
        &mSpline.mpPoints[unk18 + 1].mPos, &mSpline.mpPoints[unk18 + 1].unk10, 
        1.0f - (mag0 / mag1)
    );
}
