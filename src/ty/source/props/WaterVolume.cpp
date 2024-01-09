#include "types.h"
#include "common/StdMath.h"
#include "ty/props/WaterVolume.h"

static ModuleInfo<WaterVolume> waterVolumeModule;
static GameObjDesc waterVolumeDesc;

// decomp the file this is in
struct CritterField2LoadInfo {
    Vector pos;
    Vector rot;
    Vector scale;
    float unk;

    void Init(float);
	bool LoadLine(KromeIniLine*);
    void LoadDone(Matrix*);
};

CritterField2LoadInfo wvLoadInfo = {
    {0.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 0.0f}, 
    {1.0f, 1.0f, 1.0f, 1.0f}, 
    0.0f
};
BoundingVolume waterBoundingVolume = {
    {-0.5f, -0.5f, -0.5f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f}
};

void WaterVolume_LoadResources(KromeIni* pIni) {
    waterVolumeDesc.Init((ModuleInfoBase*)&waterVolumeModule, "WaterVolume", "WaterVolume", 0, 0);
    objectManager.AddDescriptor(&waterVolumeDesc);
}

void WaterVolume::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    wvLoadInfo.Init(1.0f);
    mMaxY = 0.0f;
    mMinY = 0.0f;
    mInvMtx.SetIdentity();
    mWorldMtx.SetIdentity();
}

void WaterVolume::Deinit(void) {
    GameObject::Deinit();
}

bool WaterVolume::LoadLine(KromeIniLine* pLine) {
    return GameObject::LoadLine(pLine) || wvLoadInfo.LoadLine(pLine);
}

void WaterVolume::LoadDone(void) {
    GameObject::LoadDone();
    wvLoadInfo.LoadDone(&mWorldMtx);

    mInvMtx.Inverse(&mWorldMtx);

    // Corner 1 (origin corner)
    Vector c1 = waterBoundingVolume.v1;
    c1.ApplyMatrix(&mWorldMtx);

    // Corner 2
    Vector c2;
    c2.Add(&waterBoundingVolume.v1, &waterBoundingVolume.v2);
    c2.ApplyMatrix(&mWorldMtx);

    // Set Minimum and Maximum y coordinates of this volume
    mMinY = Min<float>(c1.y, c2.y);
    mMaxY = Max<float>(c1.y, c2.y);

    // Add Object
    objectManager.AddObject(this, NULL, NULL);
}

bool WaterVolume_IsWithin(Vector *point, float *arg1) {
    Vector transformedPoint;
    bool isWithin = false;
    float closest = -1e+11f;
    DescriptorIterator itr = waterVolumeDesc.Begin();
    while (itr.GetPointers()) {
        WaterVolume *volume = (WaterVolume *)itr.GetPointers();
        // initial check to see if the point y position is less than the maximum y and
        // greater than the minimum y of the current WaterVolume
        if (point->y < volume->mMaxY && point->y > volume->mMinY) {
			// applying waterVolumeMatrix to transformedPoint gets the original point
            transformedPoint.ApplyMatrix(point, &volume->mInvMtx);
            if (BoundingVolume_CheckPoint(&waterBoundingVolume, &transformedPoint) != false) {
                closest = Max<float>(closest, volume->mMaxY);
                if (arg1 != NULL) {
                    *arg1 = closest;
                    isWithin = true;
                } else {
                    return true;
                }
            }
        }
        itr.UpdatePointers();
    }
    return isWithin;
}
