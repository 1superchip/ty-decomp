#ifndef CAMERAOVERRIDE_H
#define CAMERAOVERRIDE_H

#include "common/PtrListDL.h"
#include "ty/camera/GameCameraTools.h"

enum CameraOverrideCreateType {
    CREATE_OVERRIDE_DIR        = 0x2711,
    CREATE_OVERRIDE_FIXEDPOINT = 0x2712,
    CREATE_OVERRIDE_TOPDOWN    = 0x2713,
    CREATE_OVERRIDE_HIGH       = 0x2714,
    CREATE_OVERRIDE_PATH       = 0x2715,
    CREATE_OVERRIDE_RAIL       = 0x2716,
    CREATE_OVERRIDE_FIXEDRAIL  = 0x2717,
    CREATE_OVERRIDE_SPIRAL     = 0x2718,
};

struct CameraOverride;
struct CameraDescriptor {
    Vector unk0;
    Vector unk10;
    Vector mNewSource;
    Vector mIdealTarget;
    Vector unk40;
    Vector mNewTarget;
    char padding_unk60[0x10];
    Vector unk70;
    Vector unk80;
    bool unk90;
    int unk94[2]; // needs to be an array because of the struct copy
    int flags; // 0x9C
};

struct CameraOverrideManagerData {
    int mMaxOverrides;
    PtrListDL<CameraOverride> mOverrideList;
    CameraOverride* pCurr; // Current Override
    float mTime;
    float unk1;
    Quadratic mQuadratic;
    Vector unk54;
    Vector unk64;
};

struct CameraOverrideManager {
    CameraOverrideManagerData* pData;
    char padding[0x18];
    int mOverrideType;
    
    CameraOverride* Create(CameraOverrideCreateType, void*);
    void InitManager(int numOverrides);
    void DeinitManager(void);
    void ResetManager(void);
    //TestPoint__21CameraOverrideManagerFP6VectorbP16CameraDescriptor
    CameraOverride* TestPoint(Vector* pPoint, bool bCheckActivate, CameraDescriptor*);
    int EnableAndInitOverride(CameraOverride*, CameraDescriptor*);
    void UpdateOverride(CameraDescriptor*);
    void DisableAndDeinitOverride(CameraOverride*, CameraDescriptor*, int);
    CameraOverrideCreateType GetOverrideType(CameraOverride* pOverride);
    CameraOverride* GetCurr(void);
};

#define CAMERAOVERRIDE_NEWSOURCE_Y              (0x4) // Descriptor mNewTarget.y != old descriptor mNewTarget.y
#define CAMERAOVERRIDE_NEWSOURCE_XZ             (0x2) // Either X or Z is different
#define CAMERAOVERRIDE_NEWTARGET_XZ             (0x8) // Either X or Z is different
#define CAMERAOVERRIDE_NEWTARGET_NEWSOURCE_Y    (0x8) // New Source Y is not equal to New Target Y
// CAMERAOVERRIDE_NEWTARGET_NEWSOURCE_Y CAMERAOVERRIDE_TARGET_SOURCE_Y_DIFF

typedef void(CameraOverride::*CameraOverrideStateFunc)(void);
typedef void(CameraOverride::*CameraOverrideStateCreateFunc)(void*);
typedef int(CameraOverride::*CameraOverrideStateInitFunc)(void);
typedef bool(CameraOverride::*CameraOverrideStateUpdateFunc)(void);
typedef int(CameraOverride::*CameraOverrideStateCondFunc)(void);

struct CameraOverrideStateFuncs {
    CameraOverrideStateCreateFunc   Create;
    CameraOverrideStateInitFunc     Init;
    CameraOverrideStateUpdateFunc   Update;
    CameraOverrideStateCondFunc     CheckCondition;
};

struct CameraOverrideStateMachine {
    CameraOverride* mpOverride;
    int mOverrideState;
    CameraOverrideStateFuncs* mpStates;

    void Init(CameraOverride* pOverride, CameraOverrideStateFuncs* pStates, int overrideState) {
        mpOverride = pOverride;
        mpStates = pStates;
        mOverrideState = overrideState;
    }

    void CreateState(void* pLoadInfo) {
        if (mpStates[mOverrideState].Create) {
            (mpOverride->*mpStates[mOverrideState].Create)(pLoadInfo);
        }
    }

    int InitState(void) {
        if (mpStates[mOverrideState].Init) {
            return (mpOverride->*mpStates[mOverrideState].Init)();
        }

        return true;
    }

    bool UpdateState(void) {
        if (mpStates[mOverrideState].Update) {
            return (mpOverride->*mpStates[mOverrideState].Update)();
        }

        return false;
    }

    int CheckCondition(void) {
        if (mpStates[mOverrideState].CheckCondition) {
            return (mpOverride->*mpStates[mOverrideState].CheckCondition)();
        }

        return true;
    }
};

// Base Type, // might be DirectionOverride?
struct CameraOverrideCamType {
    char padding[0xC];
    GCT_Camera mCamera;
    GCT_Trigger mTrigger;
};

struct CameraOverrideLoadInfoBase {

};

enum CameraOverrideType {
    
};

struct CameraOverride {
    bool unk0;
    Tools_WayPoints* mpWayPoints;
    bool unk8;
    int unkC;
    int unk10;
    int unk14;
    int unk18;
    CameraOverrideType mOverrideType;
    CameraOverrideManager* mpManager; // Manager of this override
    CameraOverrideStateMachine mStateMachine; // State machine
    CameraOverrideCamType* unk30;
    int unk34;
    int unk38;
    int unk3C;
    int unk40;
    int unk44;
    int unk48;
    
    //Create__14CameraOverrideF24CameraOverrideCreateTypePvP21CameraOverrideManager
    void Create(CameraOverrideCreateType, void*, CameraOverrideManager*);
    //Init__14CameraOverrideFP16CameraDescriptor
    int Init(CameraDescriptor* pDesc);
    //Deinit__14CameraOverrideFP16CameraDescriptori
    bool Deinit(CameraDescriptor*, int);
    //Update__14CameraOverrideFP16CameraDescriptor
    bool Update(CameraDescriptor* pDesc);
    //Reset__14CameraOverrideFv
    void Reset(void);
    //SetCameraDescriptor__14CameraOverrideFP16CameraDescriptor
    void SetCameraDescriptor(CameraDescriptor*);
    //UpdateModifyType__14CameraOverrideFv
    void UpdateModifyType(void);

    // start of state methods?
    //Direction_Create__14CameraOverrideFPv
    void Direction_Create(void*);
    //Direction_Init__14CameraOverrideFv
    int Direction_Init(void);
    //Direction_Update__14CameraOverrideFv
    bool Direction_Update(void);
    //Direction_CanActivate__14CameraOverrideFv
    int Direction_CanActivate(void);
    
    //FixedPoint_Create__14CameraOverrideFPv
    void FixedPoint_Create(void*);
    //FixedPoint_Init__14CameraOverrideFv
    int FixedPoint_Init(void);
    //FixedPoint_Update__14CameraOverrideFv
    bool FixedPoint_Update(void);

    //TopDown_Create__14CameraOverrideFPv
    void TopDown_Create(void*);
    //TopDown_Init__14CameraOverrideFv
    int TopDown_Init(void);
    //TopDown_Update__14CameraOverrideFv
    bool TopDown_Update(void);

    //High_Create__14CameraOverrideFPv
    void High_Create(void*);
    //High_Update__14CameraOverrideFv
    bool High_Update(void);

    //Path_Create__14CameraOverrideFPv
    void Path_Create(void*);
    //Path_Init__14CameraOverrideFv
    int Path_Init(void);
    //Path_Update__14CameraOverrideFv
    bool Path_Update(void);

    //Rail_Create__14CameraOverrideFPv
    void Rail_Create(void*);
    //Rail_Init__14CameraOverrideFv
    int Rail_Init(void);
    //Rail_Update__14CameraOverrideFv
    bool Rail_Update(void);

    //Spiral_Create__14CameraOverrideFPv
    void Spiral_Create(void*);
    //Spiral_Update__14CameraOverrideFv
    bool Spiral_Update(void);

    //WaypointOverride_Init__14CameraOverrideFP24GCT_WaypointPlaneManager
    bool WaypointOverride_Init(GCT_WaypointPlaneManager*);
    //WaypointOverride_Update__14CameraOverrideFP24GCT_WaypointPlaneManager
    bool WaypointOverride_Update(GCT_WaypointPlaneManager*);

    //Destroy__14CameraOverrideFv
    void Destroy(void);

    void SetUnk0Bool(bool val) {
        unk0 = val;
    }
    bool GetUnk0Bool(void) {
        return unk0;
    }

    Tools_WayPoints* GetWayPointStruct(void) {
        return mpWayPoints;
    }

    void SetWayPointStruct(Tools_WayPoints* pWayPoints, int unkCount) {
        mpWayPoints = pWayPoints;
        mpWayPoints->unk100 = unkCount;
    }

    int GetCurrentWayPointIndex(void) {
        return GetWayPointStruct()->unk100;
    }

    int GetPrevOrNextWayPointIndex(void) {
        return unk0 ?
            GetWayPointStruct()->unk100 - 1 : GetWayPointStruct()->unk100 + 1;
    }

    Vector* GetCurrentWayPoint(void) {
        return GetWayPointStruct()->GetCurrent();
    }

    /// @brief Returns the previous or next waypoint vector
    /// @param  None
    /// @return Previous or next waypoint vector
    Vector* GetNextWayPoint(void) {
        if (unk0) {
            return GetWayPointStruct()->GetPrev();
        } else {
            return GetWayPointStruct()->GetNext();
        }
    }

    void UpdateWayPointStructIndex(void) {
        if (unk0) {
            GetWayPointStruct()->Add1Index();
        } else {
            GetWayPointStruct()->Subtract1Index();
        }
    }

    void UpdateWayPointStructIndexOpposite(void) {
        if (unk0) {
            GetWayPointStruct()->Subtract1Index();
        } else {
            GetWayPointStruct()->Add1Index();
        }
    }


    //Not static?
    //IdealTarget__14CameraOverrideFv, weak
    // this function may have been defined 
    // in CameraOverride.cpp
    static Vector* IdealTarget(void) {
        return &pCamDesc->mIdealTarget;
    }

    //NewTarget__14CameraOverrideFv, weak
    // this function may have been defined 
    // in CameraOverride.cpp
    static Vector* NewTarget(void) {
        return &pCamDesc->mNewTarget;
    }

    // NewSource__14CameraOverrideFv, weak
    static Vector* NewSource(void) {
        return &pCamDesc->mNewSource;
    }

    // SetTargetPosition__14CameraOverrideFv, weak
    static void SetTargetPosition(void) {
        *NewTarget() = *IdealTarget();
    }

    bool TestPoint(Vector*);

    int CanActivate(void) {
        if (unk10) {
            return unk14 < 1;
        } else {
            return mStateMachine.CheckCondition();
        }
    }

    bool GetTypeFromCreateType(CameraOverrideCreateType createType, CameraOverrideType* pOverrideType) {
        CameraOverrideType type;
        switch (createType) {
            case CREATE_OVERRIDE_DIR:
                type = (CameraOverrideType)1;
                break;
            case CREATE_OVERRIDE_FIXEDPOINT:
                type = (CameraOverrideType)2;
                break;
            case CREATE_OVERRIDE_TOPDOWN:
                type = (CameraOverrideType)3;
                break;
            case CREATE_OVERRIDE_HIGH:
                type = (CameraOverrideType)4;
                break;
            case CREATE_OVERRIDE_PATH:
                type = (CameraOverrideType)5;
                break;
            case CREATE_OVERRIDE_RAIL:
                type = (CameraOverrideType)6;
                break;
            case CREATE_OVERRIDE_SPIRAL:
                type = (CameraOverrideType)6;
                break;
            default:
                return false;
        }

        *pOverrideType = type;
        return true;
    }


    static CameraDescriptor camDescBefore;
    static CameraDescriptor* pCamDesc;
};


#endif // CAMERAOVERRIDE_H
