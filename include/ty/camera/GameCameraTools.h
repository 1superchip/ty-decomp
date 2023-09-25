#ifndef GAMECAMERATOOLS_H
#define GAMECAMERATOOLS_H

#include "common/Collision.h"
#include "ty/tools.h"
#include "ty/Quadratic.h"
#include "common/KromeIni.h"

// Not sure where to put this, only seems to be used by Camera related functions
inline float CameraTools_VectorDistXZ(Vector* pVec, Vector* pVec2) {
    return sqrtf(
        ((pVec->x - pVec2->x) * (pVec->x - pVec2->x)) + 
        ((pVec->z - pVec2->z) * (pVec->z - pVec2->z))
    );
}

// Calculates the distance between 2 vectors
inline float CameraTools_VectorDist(Vector* p, Vector* p1) {
    float dx = p->x - p1->x;
    float dy = p->y - p1->y;
    float dz = p->z - p1->z;
    return sqrtf((dx * dx) + (dy * dy) + (dz * dz));
}

// Calculates Squared 3D distances between 2 vectors
inline float CameraTools_VectorDistSq(Vector* pVec, Vector* pVec2) {
    float dx = (pVec->x - pVec2->x);
    float dy = (pVec->y - pVec2->y);
    float dz = (pVec->z - pVec2->z);
    return (dx * dx) + (dy * dy) + (dz * dz);
}

// Calculates the center point of 2 vectors
inline void CameraTools_CalcVectorVector(Vector* p0, Vector* p1, Vector* pCenter) {
    pCenter->x = (p0->x + p1->x) * 0.5f;
    pCenter->y = (p0->y + p1->y) * 0.5f;
    pCenter->z = (p0->z + p1->z) * 0.5f;
}

extern float GCT_MIN_HEIGHT_DELTA;
extern Vector clear;

float GCT_GetSafeHeight(uint);
void GCT_Dampen(Vector*, Vector*, float);
bool GCT_LoadInt(KromeIniLine* pLine, char* pName, int* pVal);
bool GCT_LoadFloat(KromeIniLine* pLine, char* pName, float* pVal);
void GCT_ClearCollisionResult(CollisionResult*);
bool GCT_TestFloor(Vector*, float f1, float f2, int collisionFlags, bool bCollisionMode, CollisionResult* pCr);
void GCT_BuildQuadratic(Vector*, Vector*, Vector*, int, Quadratic*);

/// @brief Load info struct used by GCT_Camera
struct CameraOverrideCameraLoadInfo {
    Vector mCameraSource;
    Vector mCameraTarget;
    bool LoadLine(KromeIniLine*);
};

struct GCT_Camera {
    Vector mCameraSource;
    Vector mCameraTarget;
    Vector mTargetDiff;
    Vector mDirToTarget; // normalized direction (cameraTarget - cameraSource)
    Vector unk40; // -unk30
    Vector unk50;
    float unk60;
    float unk64;

    void Init(CameraOverrideCameraLoadInfo*);
    void Deinit(void);
};

// Doesn't exist in symbols
enum TriggerLoadInfo_Shape {
    TRIGGERLOADSHAPE_SPHERE = 0,
    TRIGGERLOADSHAPE_BOX    = 1
};

enum GCT_TriggerShape {
    TRIGGERSHAPE_SPHERE = 0,
    TRIGGERSHAPE_BOX    = 1,
};

struct CameraOverrideTriggerLoadInfo {
    TriggerLoadInfo_Shape mShape;
    Vector mSpherePos;
    float mRadius;
    Vector mBoxPos;
    Vector mBoxExtents; // {width, height, depth}
    Vector mTriggerRot; // {mPitch, mYaw}
    Vector mExtents;
    
    bool LoadLine(KromeIniLine*);
};

/// @brief Struct to test if points are in a region of space for GCT_Cameras
struct GCT_Trigger {
    Vector mPos;
    Vector mRot;
    Vector mExtents;
    Matrix mMtx;
    Matrix mInvMtx;
    BoundingVolume mVolume;
    GCT_TriggerShape mTriggerShape;
    float mRadius;
    float mRadiusSq;

    void Load(CameraOverrideTriggerLoadInfo*);
    bool TestPoint(Vector*);
};

struct GCT_WaypointPlane {
    Vector unk0;
    Vector unk10;
    Vector unk20;
    Vector unk30;
    Vector unk40;
    float unk50;
    GCT_WaypointPlane* mpNextPlane;
};

struct GCT_WaypointPlaneManager {
    GCT_WaypointPlane* mpPlanes;
    Tools_WayPoints mWaypoints;
    int mNumPlanes;
    Vector mPlanePoint; // W is radius!
    float unk120;
    void Init(WayPointLoadInfo*, float, float);
    void Deinit(void);
    bool IsWithinContainer(Vector*, GCT_WaypointPlane*);
    int TestPoint(Vector* pPoint);
    bool SetTargetLinePos(Vector*, int, int, Vector*);
    bool SetSourceLinePos(Vector*, float, int, int, Vector*);
    float GetTimeAlongPath(Vector*, int);
    bool GetPointAlongPath(float, Vector*);
};

bool InPlane3D(Vector*, Vector*, Vector*, Vector*, Vector*);

#endif // GAMECAMERATOOLS_H
