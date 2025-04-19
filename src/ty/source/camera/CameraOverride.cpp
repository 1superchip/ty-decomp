#include "ty/camera/CameraOverride.h"
#include "ty/tytypes.h"

CameraDescriptor CameraOverride::camDescBefore;

CameraDescriptor* CameraOverride::pCamDesc = NULL;

static float INITIAL_SPEED_DEFAULT = 0.02f;

static CameraOverrideStateFuncs cameraOverrideTypes[7] = {
    {
        &CameraOverride::Direction_Create, 
        &CameraOverride::Direction_Init,
        &CameraOverride::Direction_Update, 
        &CameraOverride::Direction_CanActivate
    },
    {
        &CameraOverride::FixedPoint_Create, 
        &CameraOverride::FixedPoint_Init,
        &CameraOverride::FixedPoint_Update
    },
    {
        &CameraOverride::TopDown_Create, 
        &CameraOverride::TopDown_Init,
        &CameraOverride::TopDown_Update
    },
    {
        &CameraOverride::High_Create, 
        NULL, 
        &CameraOverride::High_Update
    },
    {
        &CameraOverride::Path_Create, 
        &CameraOverride::Path_Init, 
        &CameraOverride::Path_Update
    },
    {
        &CameraOverride::Rail_Create, 
        &CameraOverride::Rail_Init, 
        &CameraOverride::Rail_Update
    },
    {
        &CameraOverride::Spiral_Create, 
        NULL, 
        &CameraOverride::Spiral_Update
    },
};

CameraOverride* CameraOverrideManager::Create(CameraOverrideCreateType createType, void* r5) {
    CameraOverride* pOverride = pData->mOverrideList.GetNextEntry();
    pOverride->Create(createType, r5, this);
    return pOverride;
}

extern "C" void memset(void*, int, int);

/// @brief Initiates a CameraOverrideManager
/// @param numOverrides Number of Overrides for the Manager
void CameraOverrideManager::InitManager(int numOverrides) {
    if (numOverrides != 0) {
        pData = (CameraOverrideManagerData*)Heap_MemAlloc(sizeof(CameraOverrideManagerData));
        memset(pData, 0, sizeof(CameraOverrideManagerData));
        pData->mMaxOverrides = numOverrides;
        pData->mOverrideList.Init(pData->mMaxOverrides, sizeof(CameraOverride));
    } else {
        pData = NULL;
    }
}

void CameraOverrideManager::DeinitManager(void) {
    if (pData) {

        CameraOverride** ppOverrides = pData->mOverrideList.GetMem();
        while (ppOverrides && *ppOverrides) {
            (*ppOverrides)->Destroy();
            ppOverrides++;
        }

        pData->mOverrideList.Deinit();
        Heap_MemFree((void*)pData);
        pData = NULL;
    }
}

/// @brief Resets all Overrides in the Manager's list
/// @param  None
void CameraOverrideManager::ResetManager(void) {
    if (pData) {

        CameraOverride** ppOverrides = pData->mOverrideList.GetMem();
        while (ppOverrides && *ppOverrides) {
            (*ppOverrides)->Reset();
            ppOverrides++;
        }
    }
}

/// @brief 
/// @param pPoint Point to test
/// @param bCheckActivate Check if the override can activate
/// @param pDescriptor 
/// @return Found CameraOverride, returns NULL for None
CameraOverride* CameraOverrideManager::TestPoint(Vector* pPoint, bool bCheckActivate, CameraDescriptor* pDescriptor) {
    #define MAX_OVERRIDES (5)
    CameraOverride* pOverrides[MAX_OVERRIDES];

    int foundOverrides = 0;

    for (int i = 0; i < MAX_OVERRIDES; i++) {
        pOverrides[i] = NULL;
    }

    pOverrides[0]->SetCameraDescriptor(pDescriptor);
    
    CameraOverride** ppOverrides = pData->mOverrideList.GetMem();
    while (ppOverrides && *ppOverrides) {
        CameraOverride* pCurrOverride = *ppOverrides;
        if (pCurrOverride->TestPoint(pPoint) && (foundOverrides < MAX_OVERRIDES - 1)) {
            if (bCheckActivate) {
                if (pCurrOverride->CanActivate()) {
                    pOverrides[foundOverrides] = pCurrOverride;
                    foundOverrides++;
                }
            } else {
                pOverrides[foundOverrides] = pCurrOverride;
                foundOverrides++;
            }
        }
        ppOverrides++;
    }

    if (foundOverrides == 0) {
        return NULL;
    } else if (foundOverrides == 1) {
        return pOverrides[0];
    } else if (foundOverrides > 1) {
        int unk5C = 0;

        for (int i = 0; i < foundOverrides; i++) {
            unk5C = Max<int>(unk5C, pOverrides[i]->unkC);
        }

        for (int i = 0; i < foundOverrides; i++) {
            if (pOverrides[i]->unkC == unk5C) {
                return pOverrides[i];
            }
        }
    }

    return NULL;
}

int CameraOverrideManager::EnableAndInitOverride(CameraOverride* pOverride, CameraDescriptor* pDescriptor) {
    int ret = pOverride->Init(pDescriptor);
    if (ret) {
        pData->pCurr = pOverride;
    }

    return ret;
}

void CameraOverrideManager::UpdateOverride(CameraDescriptor* pDescriptor) {
    pData->pCurr->Update(pDescriptor);
}

// Deinits the current override and sets it to NULL
void CameraOverrideManager::DisableAndDeinitOverride(CameraOverride* pOverride,
        CameraDescriptor* pDescriptor, int r6) {
    pData->pCurr->Deinit(pDescriptor, r6);
    pData->pCurr = NULL;
}

CameraOverrideCreateType CameraOverrideManager::GetOverrideType(CameraOverride* pOverride) {
    return (CameraOverrideCreateType)mOverrideType;
}

CameraOverride* CameraOverrideManager::GetCurr(void) {
    return pData->pCurr;
}

void CameraOverride::Create(CameraOverrideCreateType createType, void* pLoadInfo, CameraOverrideManager* pManager) {
    // Vector* pVecs = (Vector*)pLoadInfo;
    unk30 = NULL;
    unk38 = NULL;
    unk34 = NULL;
    unk3C = NULL;
    unk40 = NULL;
    unk44 = NULL;
    mpManager = pManager;
    mOverrideType = (CameraOverrideType)7;
    if (!GetTypeFromCreateType(createType, &mOverrideType)) {
        if (createType == CREATE_OVERRIDE_FIXEDRAIL) {
            mOverrideType = (CameraOverrideType)5;
            Vector stackVecs[16];
            for (int i = 0; i < 32; i++) {
                // stackVecs[i] = pVecs[i];
            }
        }
    }
    mStateMachine.Init(this, cameraOverrideTypes, (int)mOverrideType);
    mStateMachine.CreateState(pLoadInfo);
}

int CameraOverride::Init(CameraDescriptor* pDesc) {
    SetCameraDescriptor(pDesc);
    unk18 = 0;
    int ret = mStateMachine.InitState();
    UpdateModifyType();
    return ret;
}

bool CameraOverride::Deinit(CameraDescriptor* pDesc, int r5) {
    SetCameraDescriptor(pDesc);
    if (r5) {
        unk14++;
    }
    return true;
}

bool CameraOverride::Update(CameraDescriptor* pDesc) {
    SetCameraDescriptor(pDesc);
    unk18++;
    bool ret = mStateMachine.UpdateState();
    UpdateModifyType();
    return ret;
}

void CameraOverride::Reset(void) {
    unk14 = 0;
}

void CameraOverride::SetCameraDescriptor(CameraDescriptor* pNewDesc) {
    pCamDesc = pNewDesc;
    camDescBefore = *pNewDesc;
    pNewDesc->flags = 0;
}

void CameraOverride::UpdateModifyType(void) {
    CameraDescriptor* pDesc = pCamDesc;
    CameraDescriptor* pPrevDesc = &camDescBefore;

    if (pDesc->mNewSource.y != pPrevDesc->mNewSource.y) {
        // current descriptor's new source y is not equal to previous
        // descriptor's new source y
        pCamDesc->flags |= 0x4;
    }

    if (pDesc->mNewSource.x != pPrevDesc->mNewSource.x ||
        pDesc->mNewSource.z != pPrevDesc->mNewSource.z) {
        // Current descriptor's horizontal new source position is not equal
        // to the previous descriptor's horizontal new source position
        pCamDesc->flags |= 0x2;
    }

    if (pDesc->mNewSource.y != pPrevDesc->mNewTarget.y) {
        // current descriptor's new source y is not equal to previous
        // descriptor's new target y
        pCamDesc->flags |= 0x10;
    }

    if (pDesc->mNewTarget.x != pPrevDesc->mNewTarget.x ||
        pDesc->mNewTarget.z != pPrevDesc->mNewTarget.z) {
        // Current descriptor's horizontal new target position is not equal
        // to the previous descriptor's horizontal new target position
        pCamDesc->flags |= 0x8;
    }
}

void CameraOverride::Direction_Create(void* pInitData) {
    int* initData = (int*)pInitData;
    unk10 = initData[1];
    unkC = initData[0];
    unk14 = 0;
    unk18 = 0;
    unk30 = (CameraOverrideCamType*)Heap_MemAlloc(sizeof(CameraOverrideCamType));

    unk30->mCamera.Init((CameraOverrideCameraLoadInfo*)&initData[0x50 / 4]);
    unk30->mTrigger.Load((CameraOverrideTriggerLoadInfo*)&initData[0x8 / 4]);

}

int CameraOverride::Direction_Init(void) {
    
}

bool CameraOverride::Direction_Update(void) {

}

extern float cameraOverrideDot;

// need to clean up
int CameraOverride::Direction_CanActivate(void) {
    if (!(((int*)unk30)[0])) {
        Vector* pVec = (Vector*)((char*)unk30 + 0x3C);
        if (pCamDesc->unk80.Dot(pVec) < cameraOverrideDot) {
            return false;
        }
    }

    return true;
}


void CameraOverride::FixedPoint_Create(void* p) {

}

int CameraOverride::FixedPoint_Init(void) {

}

bool CameraOverride::FixedPoint_Update(void) {

}

void CameraOverride::TopDown_Create(void* p) {

}

int CameraOverride::TopDown_Init(void) {

}

bool CameraOverride::TopDown_Update(void) {

}

// start of weak functions due to TopDown_Update__14CameraOverrideFv being recursive

// extern "C" void IdealTarget__14CameraOverrideFv(void) {}


// extern "C" void NewTarget__14CameraOverrideFv(void) {}


// extern "C" void GCT_MagSqrXZ__FP6VectorP6Vector(void) {}


// extern "C" void __as__6VectorFRC6Vector(void) {}


// extern "C" void NewSource__14CameraOverrideFv(void) {}


// extern "C" void Interpolate__9QuadraticFf(void) {}


// extern "C" void Min<f>__Fff(void) {}


// extern "C" void SetTargetPosition__14CameraOverrideFv(void) {}

// end of weak funcs due to TopDown_Update__14CameraOverrideFv

void CameraOverride::High_Create(void* p) {

}

bool CameraOverride::High_Update(void) {

}

void CameraOverride::Path_Create(void* p) {

}

int CameraOverride::Path_Init(void) {

}

bool CameraOverride::Path_Update(void) {

}

void CameraOverride::Rail_Create(void* p) {

}

int CameraOverride::Rail_Init(void) {
    
}

bool CameraOverride::Rail_Update(void) {

}

void CameraOverride::Spiral_Create(void* p) {

}

bool CameraOverride::Spiral_Update(void) {

}

bool CameraOverride::WaypointOverride_Init(GCT_WaypointPlaneManager* pWaypointManager) {
    mpManager->pData->unk54.SetZero();
    mpManager->pData->unk64.SetZero();
    
    SetWayPointStruct(
        pWaypointManager->GetWayPointManager(),
        pWaypointManager->TestPoint(CameraOverride::IdealTarget())
    );

    SetUnk0Bool(true);

    Vector dir;
    dir.Sub(GetCurrentWayPoint(), GetNextWayPoint());
    dir.Normalise();

    if (dir.Dot(&pCamDesc->unk70) < 0.0f) {
        SetUnk0Bool(true);
    } else {
        SetUnk0Bool(false);
    }

    if (GetUnk0Bool() && pCamDesc->unk90) {
        UpdateWayPointStructIndex();
    }

    if (pCamDesc->unk80.Dot(&pCamDesc->unk70) < 0.0f) {
        unk8 = GetUnk0Bool();
    } else {
        unk8 = !GetUnk0Bool();
    }

    return true;
}

bool CameraOverride::WaypointOverride_Update(GCT_WaypointPlaneManager* pWaypointManager) {
    SetTargetPosition();

    Vector dir;
    dir.Sub(GetCurrentWayPoint(), GetNextWayPoint());
    dir.Normalise();

    if (!(pCamDesc->unk70.Dot(&dir) < 0.0f)) {
        UpdateWayPointStructIndexOpposite();
        
        if (GetUnk0Bool()) {
            SetUnk0Bool(false);
        } else {
            SetUnk0Bool(true);
        }
    }

    pWaypointManager->SetTargetLinePos(
        CameraOverride::NewTarget(),
        GetCurrentWayPointIndex(),
        GetPrevOrNextWayPointIndex(),
        &mpManager->pData->unk64
    );

    int r31 = GetCurrentWayPointIndex();
    int foundPoint = pWaypointManager->TestPoint(NewTarget());
    if (GetUnk0Bool()) {
        r31--;
    }
    
    if (foundPoint == -1) {
        return false;
    }

    if (foundPoint != r31) {
        UpdateWayPointStructIndex();
    }
    
    return true;
}

void CameraOverride::Destroy(void) {

}

// Camera__14CameraOverrideFv = weak

// extern "C" void Camera__14CameraOverrideFv(void) {}

// Trigger__14CameraOverrideFv = weak
// extern "C" void Trigger__14CameraOverrideFv(void) {}


// SnapToSource__14CameraOverrideFv = weak
// extern "C" void SnapToSource__14CameraOverrideFv(void) {}


// HoldTyMoveYawInitially__14CameraOverrideFv = weak
// extern "C" void HoldTyMoveYawInitially__14CameraOverrideFv(void) {}


// TestPoint__14CameraOverrideFP6Vector = weak
// extern "C" void TestPoint__14CameraOverrideFP6Vector(void) {}
