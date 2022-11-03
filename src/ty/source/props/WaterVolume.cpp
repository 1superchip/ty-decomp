#include "types.h"
#include "ty/props/WaterVolume.h"

extern GameObjectManager objectManager;

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

CritterField2LoadInfo wvLoadInfo = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0.0f};
BoundingVolume waterBoundingVolume = { {-0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f} };

void WaterVolume_LoadResources(KromeIni* pIni) {
    waterVolumeDesc.Init((ModuleInfoBase*)&waterVolumeModule, "WaterVolume", "WaterVolume", 0, 0);
    objectManager.AddDescriptor(&waterVolumeDesc);
}

void WaterVolume::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    wvLoadInfo.Init(1.0f);
    unkC0 = 0.0f;
    unkBC = 0.0f;
    matrix.SetIdentity();
    waterVolumeMatrix.SetIdentity();
}

void WaterVolume::Deinit(void) {
    GameObject::Deinit();
}

bool WaterVolume::LoadLine(KromeIniLine* pLine) {
    bool ret = false;
    if (GameObject::LoadLine(pLine) || wvLoadInfo.LoadLine(pLine)) {
        ret = true;
    }
    return ret;
}

void WaterVolume::LoadDone(void) {
    GameObject::LoadDone();
    wvLoadInfo.LoadDone(&waterVolumeMatrix);
    matrix.Inverse(&waterVolumeMatrix);
    Vector temp = waterBoundingVolume.v1;
    temp.ApplyMatrix(&temp, &waterVolumeMatrix);
    Vector temp1;
    temp1.x = waterBoundingVolume.v1.x + waterBoundingVolume.v2.x;
    temp1.y = waterBoundingVolume.v1.y + waterBoundingVolume.v2.y;
    temp1.z = waterBoundingVolume.v1.z + waterBoundingVolume.v2.z;
    temp1.ApplyMatrix(&temp1, &waterVolumeMatrix);
    float fVar = temp1.y;
    float fVar1 = temp.y;
    unkBC = (fVar1 < fVar) ? temp.y : temp1.y;
    fVar1 = temp1.y;
    fVar = temp.y;
    unkC0 = (fVar > fVar1) ? fVar : fVar1;
    objectManager.AddObject(this, NULL, NULL);
}

inline bool BoundingVolume_CheckPoint(BoundingVolume *volume, Vector *point) {
    bool isWithin = false;
    if ((point->x >= volume->v1.x) && point->x <= volume->v1.x + volume->v2.x) {
        if ((point->y >= volume->v1.y) && point->y <= volume->v1.y + volume->v2.y) {
            if ((point->z >= volume->v1.z) && point->z <= volume->v1.z + volume->v2.z) {
                isWithin = true;
            }
        }
    }
    return isWithin;
}

bool WaterVolume_IsWithin(Vector *point, float *arg1) {
    Vector transformedPoint;
    bool isWithin = false;
    float fVar1 = -1e+11f;
    BeginStruct descStruct = waterVolumeDesc.Begin();
    while (descStruct.GetPointers()) {
        WaterVolume *volume = (WaterVolume *)descStruct.GetPointers();
        if (point->y < volume->unkC0 && point->y > volume->unkBC) {
            transformedPoint.ApplyMatrix(point, &volume->matrix);
            if (BoundingVolume_CheckPoint(&waterBoundingVolume, &transformedPoint) != false) {
                fVar1 = (fVar1 > volume->unkC0) ? fVar1 : volume->unkC0;
                if (arg1 != NULL) {
                    *arg1 = fVar1;
                    isWithin = true;
                } else {
                    return true;
                }
            }
        }
        descStruct.UpdatePointers();
    }
    return isWithin;
}