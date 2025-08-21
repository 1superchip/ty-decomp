#include "ty/camera/GameCameraTools.h"
#include "common/Debug.h"

float GCT_MIN_HEIGHT_DELTA = 100.0f;

Vector clear = {0.0f, 0.0f, 0.0f, 0.0f};

struct GCTHeightStruct {
    uint flags; // Collision flags
    float unk4;
};
#define MAX_HEIGHT_INDEX (7)
static GCTHeightStruct heightTable[MAX_HEIGHT_INDEX + 1] = {
    {0x0004, 0.0f},
    {0x0080, 0.0f},
    {0x0400, 0.0f},
    {0x0008, 0.0f},
    {0x0800, 150.0f},
    {0x2000, 0.0f},
    {0x1000000, 0.0f},
    {0x0000, 0.0f} // must be a final entry
};

float GCT_GetSafeHeight(uint collideFlags) {
    float ret = GCT_MIN_HEIGHT_DELTA;
    for (int i = 0; i < MAX_HEIGHT_INDEX; i++) {
        if ((heightTable[i].flags & collideFlags) && (heightTable[i].unk4 > ret)) {
            ret = heightTable[i].unk4;
        }
    }
    return ret;
}

void GCT_Dampen(Vector* arg0, Vector* arg1, float f1) {
    Vector damping;
    Vector force;
    
    damping.Scale(arg0, 2.0f * f1);
    force.Scale(arg1, f1 * f1);
    arg0->Add(&force);
    arg0->Subtract(&damping);
}

extern "C" int stricmp(char*, char*);

/// @brief GCT Helper function to load an int from an ini
/// @param pLine Line to test
/// @param pName Field name to load data from
/// @param pVal Output pointer for value
/// @return 
bool GCT_LoadInt(KromeIniLine* pLine, char* pName, int* pVal) {
    if (stricmp(pLine->pFieldName, pName) == 0 && pLine->elementCount >= 1) {
        return pLine->AsInt(0, pVal);
    }
    return false;
}

/// @brief GCT Helper function to load a float from an ini
/// @param pLine Line to test
/// @param pName Field name to load data from
/// @param pVal Output pointer for value
/// @return 
bool GCT_LoadFloat(KromeIniLine* pLine, char* pName, float* pVal) {
    if (stricmp(pLine->pFieldName, pName) == 0 && pLine->elementCount >= 1) {
        return pLine->AsFloat(0, pVal);
    }
    return false;
}

/// @brief Inlined GCT Helper function to load a Vector from an ini (x, y, z, 0.0f)
/// @param pLine Line to test
/// @param pName Field name to load data from
/// @param pVal Output Vector
/// @return 
bool GCT_LoadVector(KromeIniLine* pLine, char* pName, Vector* pVal) {
    if (stricmp(pLine->pFieldName, pName) == 0 && pLine->elementCount >= 3) {
        pVal->w = 0.0f; // w = 0.0f
        return pLine->AsFloat(0, &pVal->x) && pLine->AsFloat(1, &pVal->y) && pLine->AsFloat(2, &pVal->z);
    }
    return false;
}

// Resets a CollisionResult struct
void GCT_ClearCollisionResult(CollisionResult* pCr) {
    pCr->pos.SetZero();
    pCr->normal.SetZero();
    pCr->color.SetZero();
    pCr->pModel = NULL;
    pCr->itemIdx = -1;
    pCr->collisionFlags = 0;
    pCr->pInfo = NULL;
    pCr->unk40 = 0.0f;
}

// Basically the same code as Tools_TestFloor in Tools.cpp
bool GCT_TestFloor(Vector* r3, float f1, float f2, int collisionFlags,
        bool bCollisionMode, CollisionResult* pCr) {
    
    Vector start = {r3->x, r3->y + f1, r3->z};
    Vector end = {start.x, start.y - f2, start.z};

    return Collision_RayCollide(&start, &end, pCr,
        bCollisionMode ? COLLISION_MODE_ALL : COLLISION_MODE_POLY, collisionFlags);
}

// Exact same as the stripped Vector method...
bool GCT_IntersectingPoint2D(Vector* param_1, Vector* param_2, Vector* param_3, Vector* param_4, Vector* pOut) {
    float x0 = param_1->x;
    float z0 = param_1->z;
    float x2 = param_2->x;
    float z2 = param_2->z;
    float x3 = param_3->x;
    float z3 = param_3->z;
    float x4 = param_4->x;
    float z4 = param_4->z;
    float num    = (((x4 - x3) * (z0 - z3)) - ((z4 - z3) * (x0 - x3)));
    float fVar12 = (((z4 - z3) * (x2 - x0)) - ((x4 - x3) * (z2 - z0)));
    float num_2  = (((x2 - x0) * (z0 - z3)) - ((z2 - z0) * (x0 - x3)));
    float fVar13 = (((z4 - z3) * (x2 - x0)) - ((x4 - x3) * (z2 - z0)));
    if (fVar12 != 0.0f) {
        pOut->x = x0 + (num / fVar12) * (x2 - x0);
        pOut->z = z0 + (num / fVar12) * (z2 - z0);
        pOut->y = 0.0f;
        return true;
    } else if (fVar13 != 0.0f) {
        pOut->x = x3 + (num_2 / fVar13) * (x4 - x3);
        pOut->z = z3 + (num_2 / fVar13) * (z4 - z3);
        pOut->y = 0.0f;
        return true;
    }
    return false;
}

void GCT_BuildQuadratic(Vector* r3, Vector* r4, Vector* r5, int r6, Quadratic* pQuad) {
    Vector toStart;
    Vector toMiddle;
    Vector toEnd;
    Vector middle;
    toStart.Sub(r3, r5);
    toEnd.Sub(r4, r5);
    float mag = (toStart.Magnitude() + toEnd.Magnitude()) / 2.0f;
    toMiddle.Add(&toStart, &toEnd);
    toMiddle.Normalise();
    toMiddle.Scale(mag);
    if (r6 != 0) {
        pQuad->SetPoints(&toStart, &toMiddle, &toEnd);
    } else {
        middle.Add(r5, &toMiddle);
        pQuad->SetPoints(r3, &middle, r4);
    }
}

/// @brief Unused
/// @param pStr String to draw
/// @param xPos X position of string
/// @param yPos Y position of string
void GCT_DrawText(char* pStr, float xPos, float yPos) {
    volatile float orderFloats = -1.0f; // need this to order floats
    gpDebugFont->DrawText(pStr, xPos, yPos, 1.0f, 1.0f, FONT_JUSTIFY_4, 0x80606060);
}

void CameraOverride_GetContainterPoints(Matrix* pMtx, Vector* pVec, Vector* pVec1, Vector* pVec2,
        Vector* pVec3, Vector* pVec4) {
    pVec3->Add(pVec, pMtx->Row0());
    pVec2->Sub(pVec, pMtx->Row0());
    *pVec1 = *pVec2;
    pVec1->Add(pMtx->Row1());
    *pVec4 = *pVec3;
    pVec4->Add(pMtx->Row1());
}

/// @brief Initiates a GCT_Camera with CameraOverrideCamera load info
/// @param pOverrideLoadInfo Load Info
void GCT_Camera::Init(CameraOverrideCameraLoadInfo* pOverrideLoadInfo) {
    mCameraSource = pOverrideLoadInfo->mCameraSource;
    mCameraTarget = pOverrideLoadInfo->mCameraTarget;
    mTargetDiff.Sub(&mCameraTarget, &mCameraSource);
    unk40.Inverse(&mTargetDiff);
    mDirToTarget.Normalise(&mTargetDiff);
    unk50.Inverse(&mDirToTarget);
    unk60 = mCameraSource.y - mCameraTarget.y;
    unk64 = CameraTools_VectorDistXZ(&mCameraSource, &mCameraTarget);
}

void GCT_Camera::Deinit(void) {};

/// @brief Load CameraOverride Trigger Information from an Ini
/// @param pLine KromeIniLine to read from
/// @return 
bool CameraOverrideTriggerLoadInfo::LoadLine(KromeIniLine* pLine) {
    if (stricmp(pLine->pFieldName, "sphere") == 0) {
        mShape = TRIGGERLOADSHAPE_SPHERE;
    } else if (stricmp(pLine->pFieldName, "box") == 0) {
        mShape = TRIGGERLOADSHAPE_BOX;
    } else if (stricmp(pLine->pFieldName, "pos") == 0) {
        if (mShape == TRIGGERLOADSHAPE_SPHERE) {
            GCT_LoadVector(pLine, "pos", &mSpherePos);
        } else if (mShape == TRIGGERLOADSHAPE_BOX) {
            GCT_LoadVector(pLine, "pos", &mBoxPos);
        } else {
            // Unknown Trigger shape!
            return false;
        }
    } else if (stricmp(pLine->pFieldName, "pitch") == 0) {
        pLine->AsFloat(0, &mTriggerRot.x); // load pitch
    } else if (stricmp(pLine->pFieldName, "yaw") == 0) {
        pLine->AsFloat(0, &mTriggerRot.y); // load yaw
    } else if (stricmp(pLine->pFieldName, "width") == 0) {
        pLine->AsFloat(0, &mBoxExtents.x); // load Box Width (X length)
    } else if (stricmp(pLine->pFieldName, "height") == 0) {
        pLine->AsFloat(0, &mBoxExtents.y); // load Box Height (Y length)
    } else if (stricmp(pLine->pFieldName, "depth") == 0) {
        pLine->AsFloat(0, &mBoxExtents.z); // load Box Depth (Z length)
    } else if (stricmp(pLine->pFieldName, "radius") == 0) {
        pLine->AsFloat(0, &mRadius); // load sphere radius
    } else {
        return false;
    }
    return true;
}

bool CameraOverrideCameraLoadInfo::LoadLine(KromeIniLine* pLine) {
    if (stricmp(pLine->pFieldName, "camera") != 0) {
        if (!GCT_LoadVector(pLine, "cameraSource", &mCameraSource) &&
            !GCT_LoadVector(pLine, "cameraTarget", &mCameraTarget)) {
            return false;
        }
    }
    return true;
}

void GCT_Trigger::Load(CameraOverrideTriggerLoadInfo* pTriggerLoadInfo) {
    if (pTriggerLoadInfo->mShape != TRIGGERLOADSHAPE_SPHERE) {
        // Box trigger shape
        mPos = pTriggerLoadInfo->mBoxPos;
        mRot = pTriggerLoadInfo->mTriggerRot;
        mExtents = pTriggerLoadInfo->mBoxExtents;
        mVolume.v1.Set(mExtents.x / -2.0f, mExtents.y / -2.0f, mExtents.z / -2.0f);
        mVolume.v2.Set(mExtents.x, mExtents.y, mExtents.z);
        mMtx.SetIdentity();
        mMtx.SetRotationPYR(&mRot);
        mMtx.Row3()->Copy(&mPos);
        mInvMtx.InverseSimple(&mMtx);
        mTriggerShape = TRIGGERSHAPE_BOX;
        mRadius = 0.0f;
        mRadiusSq = 0.0f;
    } else {
        // Sphere trigger shape
        mPos = pTriggerLoadInfo->mSpherePos;
        mRadius = pTriggerLoadInfo->mRadius;
        mRadiusSq = mRadius * mRadius;
        mInvMtx.SetIdentity();
        mRot.SetZero();
        mExtents.SetZero();
        mVolume.v1.SetZero();
        mVolume.v2.SetZero();
        mTriggerShape = TRIGGERSHAPE_SPHERE;
    }
}

void CameraOverride_CreateScaledDirectionMatrix(Matrix* pMtx, Vector* pFrom, Vector* pTo, float xScale, float yScale) {
    Vector up = {0.0f, 1.0f, 0.0f, 0.0f};
    Vector x;
    Vector y;
    Vector z;

    z.Sub(pTo, pFrom);
    float zLen = z.Normalise();

    float zDotUp = __fabs(z.Dot(&up));

    if (zDotUp > 0.99f) {
        x.Set(1.0f, 0.0f, 0.0f);
        y.Cross(&x, &z);
    } else {
        x.Cross(&z, &up);
        x.Normalise();
        y.Cross(&x, &z);
    }

    pMtx->SetIdentity();

    pMtx->Row0()->Scale(&x, xScale);
    pMtx->Row1()->Scale(&y, yScale);
    pMtx->Row2()->Scale(&z, zLen);

    pMtx->Row3()->Copy(pFrom);
}

/// @brief Tests if a point is inside a GCT_Trigger
/// @param pPoint Point to test
/// @return True if point is in, otherwise false
bool GCT_Trigger::TestPoint(Vector* pPoint) {
    if (mTriggerShape != TRIGGERSHAPE_SPHERE) {
        // Box Trigger
        Vector transformedPoint;
        transformedPoint.ApplyMatrix(pPoint, &mInvMtx);
        if (BoundingVolume_CheckPoint(&mVolume, &transformedPoint)) {
            return true;
        }
    } else {
        // Sphere Trigger
        if (CameraTools_VectorDistSq(&mPos, pPoint) < mRadiusSq) {
            return true;
        }
    }
    
    return false;
}

void GCT_WaypointPlaneManager::Init(WayPointLoadInfo* pLoadInfo, float f1, float f2) {
    mWaypoints.Load(pLoadInfo, Tools_WayPoints::LOAD_MODE_1);

    Vector spherePos = {0.0f, 0.0f, 0.0f, 0.0f};
    for (int i = 0; i < mWaypoints.unk104; i++) {
        spherePos.Add(&mWaypoints.vecs[i]);
    }

    spherePos.Scale(1.0f / (float)mWaypoints.unk104);

    float closest = 0.0f;
    for (int i = 0; i < mWaypoints.unk104; i++) {
        float dist = CameraTools_VectorDist(&spherePos, &mWaypoints.vecs[i]) + f1;
        if (dist > closest) {
            closest = dist;
        }
    }

    mPlanePoint = spherePos;
    mPlanePoint.w = closest;
    mNumPlanes = mWaypoints.unk104;
    mpPlanes = (GCT_WaypointPlane*)Heap_MemAlloc(mNumPlanes * sizeof(GCT_WaypointPlane));
    Matrix nextScaledDirMatrix;
    Vector nextRB;
    Vector nextRT;
    Vector nextLT;
    Vector nextLB;
    Matrix prevScaledDirMatrix;
    Vector prevRB;
    Vector prevRT;
    Vector prevLT;
    Vector prevLB;
    int i;
    for (i = 1; i < mWaypoints.unk104; i++) {
        Vector* nextVec = &mWaypoints.vecs[i + 1];
        Vector* prevVec = &mWaypoints.vecs[i - 1];
        Vector* currVec = &mWaypoints.vecs[i];
        GCT_WaypointPlane* pPlane = &mpPlanes[i];
        CameraOverride_CreateScaledDirectionMatrix(&nextScaledDirMatrix, currVec,
            nextVec, f1, f2);
        CameraOverride_GetContainterPoints(&nextScaledDirMatrix, currVec, &nextLT, &nextLB, &nextRB, &nextRT);
        CameraOverride_CreateScaledDirectionMatrix(&prevScaledDirMatrix, currVec,
            prevVec, -f1, f2);
        CameraOverride_GetContainterPoints(&prevScaledDirMatrix, currVec, &prevLT, &prevLB, &prevRB, &prevRT);
        CameraTools_CalcVectorVector(&nextLB, &prevLB, &pPlane->unk0);
        CameraTools_CalcVectorVector(&nextLT, &prevLT, &pPlane->unk10);
        CameraTools_CalcVectorVector(&nextRB, &prevRB, &pPlane->unk20);
        CameraTools_CalcVectorVector(&nextRT, &prevRT, &pPlane->unk30);
        pPlane->mpNextPlane = NULL;
    }
    
    for (i = 0; i < mNumPlanes - 2; i++) {
        mpPlanes[i].mpNextPlane = &mpPlanes[i + 1];
        mpPlanes[i].unk50 = CameraTools_VectorDist(&mWaypoints.vecs[i], &mWaypoints.vecs[i + 1]);
        CameraTools_CalcVectorVector(&mWaypoints.vecs[i], &mWaypoints.vecs[i + 1], &mpPlanes[i].unk40);
        float temp = 0.0f;
        float dist = CameraTools_VectorDist(&mpPlanes[i].unk40, &mpPlanes[i].unk0);
        if (dist > temp) {
            temp = dist;
        }

        dist = CameraTools_VectorDist(&mpPlanes[i].unk40, &mpPlanes[i].unk10);
        if (dist > temp) {
            temp = dist;
        }

        dist = CameraTools_VectorDist(&mpPlanes[i].unk40, &mpPlanes[i].unk20);
        if (dist > temp) {
            temp = dist;
        }

        dist = CameraTools_VectorDist(&mpPlanes[i].unk40, &mpPlanes[i].unk30);
        if (dist > temp) {
            temp = dist;
        }

        dist = CameraTools_VectorDist(&mpPlanes[i].unk40, &mpPlanes[i].mpNextPlane->unk0);
        if (dist > temp) {
            temp = dist;
        }

        dist = CameraTools_VectorDist(&mpPlanes[i].unk40, &mpPlanes[i].mpNextPlane->unk10);
        if (dist > temp) {
            temp = dist;
        }

        dist = CameraTools_VectorDist(&mpPlanes[i].unk40, &mpPlanes[i].mpNextPlane->unk20);
        if (dist > temp) {
            temp = dist;
        }

        dist = CameraTools_VectorDist(&mpPlanes[i].unk40, &mpPlanes[i].mpNextPlane->unk30);
        if (dist > temp) {
            temp = dist;
        }

        mpPlanes[i].unk40.w = temp;
    }

    unk120 = 0.0f;
    for (int i = 1; i < mWaypoints.unk104 - 2; i++) {
        unk120 += mpPlanes[i].unk50;
    }
}

/// @brief Deinits a GCT_WaypointPlaneManager
/// @param  None
void GCT_WaypointPlaneManager::Deinit(void) {
    Heap_MemFree(mpPlanes);
    // should set mpPlanes to NULL?
    mNumPlanes = 0;
}

inline bool TestPlane(Vector* p0, Vector* p1, Vector* p2, Vector* p3) {
    Vector a;
    Vector b;
    Vector c;
    Vector d;
    a.Sub(p1, p2);
    b.Sub(p3, p2);
    c.Cross(&a, &b);
    d.Sub(p0, p2);
    return c.Dot(&d) > 0.0f;
}

bool InPlane3D(Vector* p0, Vector* p1, Vector* p2, Vector* p3, Vector* p4) {
    if (TestPlane(p0, p4, p1, p2) && 
        TestPlane(p0, p1, p2, p3) &&
        TestPlane(p0, p2, p3, p4) &&
        TestPlane(p0, p3, p4, p1)) {
        return true;
    }

    return false;
}

bool GCT_WaypointPlaneManager::IsWithinContainer(Vector* pVector, GCT_WaypointPlane* pPlane) {
    if (pPlane->mpNextPlane == NULL) {
        return false;
    }

    if (CameraTools_VectorDistSq(pVector, &pPlane->unk40) > pPlane->unk40.w * pPlane->unk40.w) {
        return false;
    }

    if (InPlane3D(pVector, &pPlane->unk10, &pPlane->unk30,
            &pPlane->mpNextPlane->unk30, &pPlane->mpNextPlane->unk10) &&
        InPlane3D(pVector, &pPlane->mpNextPlane->unk0, &pPlane->mpNextPlane->unk20,
            &pPlane->unk20, &pPlane->unk0) &&
        InPlane3D(pVector, &pPlane->unk0, &pPlane->unk20,
            &pPlane->unk30, &pPlane->unk10) &&
        InPlane3D(pVector, &pPlane->mpNextPlane->unk20, &pPlane->mpNextPlane->unk0,
            &pPlane->mpNextPlane->unk10, &pPlane->mpNextPlane->unk30) && 
        InPlane3D(pVector, &pPlane->mpNextPlane->unk0, &pPlane->unk0,
            &pPlane->unk10, &pPlane->mpNextPlane->unk10) &&
        InPlane3D(pVector, &pPlane->unk20, &pPlane->mpNextPlane->unk20,
            &pPlane->mpNextPlane->unk30, &pPlane->unk30)) {
        return true;
    }

    return false;
}

/// @brief Tests a GCT_WaypointPlaneManager for a point
/// @param pPoint Point to test
/// @return Index of the plane that was found to contain the point
int GCT_WaypointPlaneManager::TestPoint(Vector* pPoint) {
    if (CameraTools_VectorDistSq(pPoint, &mPlanePoint) < mPlanePoint.w * mPlanePoint.w) {
        for (int i = 1; i < mNumPlanes - 2; i++) {
            if (IsWithinContainer(pPoint, &mpPlanes[i])) {
                return i;
            }
        }
    }

    return -1;
}

bool GCT_WaypointPlaneManager::SetTargetLinePos(Vector* r4, int r5, int r6, Vector* r7) {
    Vector currPlanePos;
    Vector prevPlanePos;
    Vector pathDir;
    Vector line1EndCurr;
    Vector line1EndPrev;
    
    Vector* p = &mWaypoints.vecs[r5];
    Vector* p1 = &mWaypoints.vecs[r6];
    pathDir.Sub(p, p1);
    float len = pathDir.Normalise();
    line1EndCurr.Scale(&pathDir, len);
    line1EndCurr.Add(r4); // {f7, unused, f8}
    bool b = GCT_IntersectingPoint2D(r4, &line1EndCurr, &mpPlanes[r5].unk0, &mpPlanes[r5].unk20, &currPlanePos);
    line1EndPrev.Scale(&pathDir, -len);
    line1EndPrev.Add(r4);
    bool c = GCT_IntersectingPoint2D(r4, &line1EndPrev, &mpPlanes[r6].unk0, &mpPlanes[r6].unk20, &prevPlanePos);

    if (!c && !b) {
        return false;
    }

    prevPlanePos.y = r4->y;
    currPlanePos.y = r4->y;
    float b8 = CameraTools_VectorDistXZ(&currPlanePos, &prevPlanePos);
    float bC = CameraTools_VectorDistXZ(r4, &prevPlanePos);
    r7->Scale(&pathDir, len * (bC / b8));
    r7->Add(p1);

    return true;
}

bool GCT_WaypointPlaneManager::SetSourceLinePos(Vector* r4, float f1, int r5, int r6, Vector* r7) {
    Vector* pVec_param6 = &mWaypoints.vecs[r6];
    Vector* pVec_param5 = &mWaypoints.vecs[r5];
    u8 temp = r5 > r6 ? true : false;
    Vector safePos = *r4;
    if (f1 > 0.0f) {
        int r28 = r5;
        Vector curr = *pVec_param5;
        Vector prev = *pVec_param6;
        while (f1 > 0.0f) {
            Vector dirToPrev;
            dirToPrev.Sub(&prev, &safePos);
            float len = dirToPrev.Normalise();
            if (f1 > len) {
                safePos = prev;
                curr = prev;
                if (temp) {
                    r28--;
                } else {
                    r28++;
                }
                prev = mWaypoints.vecs[r28];
            } else {
                dirToPrev.Scale(f1);
                safePos.Add(&dirToPrev);
            }
            f1 -= len;
        }
    } else {
        int r28 = r5;
        Vector curr = *pVec_param6;
        Vector prev = *pVec_param5;
        f1 *= -1.0f;
        while (f1 > 0.0f) {
            Vector dirToPrev;
            dirToPrev.Sub(&prev, &safePos);
            float len = dirToPrev.Normalise();
            if (f1 > len) {
                safePos = prev;
                curr = prev;
                if (temp) {
                    r28++;
                } else {
                    r28--;
                }
                prev = mWaypoints.vecs[r28];
            } else {
                dirToPrev.Scale(f1);
                safePos.Add(&dirToPrev);
            }
            f1 -= len;
        }
    }

    r7->Copy(&safePos);

    return true;
}

float GCT_WaypointPlaneManager::GetTimeAlongPath(Vector* pPoint, int r5) {
    if (r5 <= 1) {
        return 0.0f;
    }

    int prev = r5 - 1;

    float dist = CameraTools_VectorDist(pPoint, &mWaypoints.vecs[prev]);

    for (int i = 1; i < prev; i++) {
        dist += mpPlanes[i].unk50;
    }

    return dist / unk120;
}

bool GCT_WaypointPlaneManager::GetPointAlongPath(float f1, Vector* pPoint) {
    if (f1 < 0.0f || f1 > 1.0f) {
        return false;
    }

    f1 = unk120 * f1;
    float f2 = 0.0f;
    
    for (int i = 1; i < mWaypoints.unk104 - 2; i++) {
        float f0 = f2 + mpPlanes[i].unk50;
        if (f0 > f1) {
            float f31 = (f1 - f2) / mpPlanes[i].unk50;
            Vector* nextPoint = &mWaypoints.vecs[i + 1];
            Vector* currPoint = &mWaypoints.vecs[i];
            Vector dir;
            dir.Sub(nextPoint, currPoint);
            float len = dir.Normalise();
            dir.Scale(len * f31);
            pPoint->Add(currPoint, &dir);
            return true;
        }

        f2 = f0;
    }

    return false;
}

