#ifndef TY_H
#define TY_H

#include "ty/Hero.h"
#include "ty/tytypes.h"
#include "ty/AutoTargetStruct.h"
#include "ty/NodeOverride.h"
#include "ty/BoomerangManager.h"
#include "ty/CollisionObject.h"
#include "ty/tools.h"
#include "ty/DDA.h"
#include "ty/Quadratic.h"
#include "ty/global.h"

struct Bunyip;

struct Ty;

struct BoomerangAnimInfo {
    Model* unk0;
    bool unk4;
    MKAnimScript unk8;

    void Init(void);
    void Deinit(void);
};

struct HeadTurningInfo {
    NodeOverride mNodeOverride;
    char unk4C[0xC];
    float unk58;
    float unk5C;
    float unk60;
    float unk64;
    bool unk68;
    Vector tyPos;
    int unk7C;
    int unk80;
    int unk84;
    int unk88;
    int unk8C;

    void Init(Vector tyPos);
};

enum KnockBackType {
    KB_TYPE_0 = 0,
};

enum TyHeads {

};

// Ty movement state: Air, Water, Land, UnderWater
enum TyMedium {
    TY_MEDIUM_0 = 0,
    TY_MEDIUM_1 = 1,
    TY_MEDIUM_2 = 2,
    TY_MEDIUM_3 = 3,
};

// No symbol
struct TyMediumMachine {

    typedef void(Ty::*Func1)(void);
    typedef void(Ty::*DeinitFunc)(void);
    typedef void(Ty::*Func3)(void);
    typedef void(Ty::*Func4)(void);
    typedef void(Ty::*Func5)(void);

    struct State {
        Func1 func1;
        DeinitFunc Deinit;
        Func3 func3;
        Func4 func4;
        Func5 func5;
    };

    // uses PTMFs
    int unk0;
    int unk4;
    int unk8;
    State* pStates;

    // Initializes the machine
    void Init(State* _pStates, int state) {
        pStates = _pStates;
        unk0 = -1;
        unk8 = -1;
        unk4 = state;
    }

    void CallDeinit(Ty* pTy) {
        if (unk0 != -1) {
            if (pStates[unk0].Deinit) {
                (pTy->*pStates[unk0].Deinit)();
            }
        }

        unk0 = -1;
    }

    void Update(Ty* pActor, bool arg2) {

    }

    void SetNewState(TyMedium nState, bool arg2) {

    }

    TyMedium GetUnk0(void) {
        return (TyMedium)unk0;
    }
};

// TODO: Finish this
enum HeroActorState {
    TY_AS_0 = 0,
    TY_AS_1 = 1,
    TY_AS_2 = 2,
    TY_AS_3 = 3,
    TY_AS_4 = 4,
    TY_AS_5 = 5,
    TY_AS_6 = 6,
    TY_AS_7 = 7,
    TY_AS_8 = 8,
    TY_AS_9 = 9,
    TY_AS_10 = 10,
    TY_AS_11 = 11,
    TY_AS_12 = 12,
    TY_AS_13 = 13,
    TY_AS_14 = 14,
    TY_AS_15 = 15,
    TY_AS_16 = 16,
    TY_AS_17 = 17,
    TY_AS_18 = 18,
    TY_AS_19 = 19,
    TY_AS_20 = 20,
    TY_AS_21 = 21,
    TY_AS_22 = 22,
    TY_AS_23 = 23,
    TY_AS_24 = 24,
    TY_AS_25 = 25,
    TY_AS_26 = 26,
    TY_AS_27 = 27,
    TY_AS_28 = 28,
    TY_AS_29 = 29,
    TY_AS_30 = 30,
    TY_AS_31 = 31,
    TY_AS_32 = 32,
    TY_AS_33 = 33,
    TY_AS_34 = 34,
    TY_AS_35 = 35,
    TY_AS_36 = 36,
    TY_AS_37 = 37,
    TY_AS_38 = 38,
    TY_AS_39 = 39,
    TY_AS_40 = 40,
    TY_AS_41 = 41,
    TY_AS_42 = 42,
    TY_AS_43 = 43,
    TY_AS_44 = 44,
    TY_AS_45 = 45,
    TY_AS_46 = 46,
    TY_AS_47 = 47,
    TY_AS_48 = 48,
    TY_AS_49 = 49,
    TY_AS_50 = 50,
    TY_AS_51 = 51,
};

struct TyFSM {
    // uses PTMFs

    typedef void(Ty::*InitFunc)(void);
    typedef void(Ty::*DeinitFunc)(void);
    typedef void(Ty::*ActiveFunc)(void);
    typedef void(Ty::*DrawFunc)(void);
    typedef void(Ty::*EventFunc)(char*);

    struct State {
        InitFunc    Init;
        DeinitFunc  Deinit;
        ActiveFunc  Active;
        DrawFunc    Draw;
        EventFunc   Event;
        int         medium; // Not sure?
    };

    int unk0;
    int unk4;
    int unk8;
    int unkC;
    int unk10;
    int unk14;
    State* pStates;

    // defined in Ty.cpp
    void Init(State* _pStates, HeroActorState newState) {
        pStates = _pStates;
        unk14 = newState;
        unk8 = 0;
        unk10 = 0;
        unk0 = 0;
        unk4 = 0;
    }

    void SetState(HeroActorState newState, bool arg2) {
        if (arg2 || unk10 != newState) {
            unk14 = newState;
        }
    }

    void Set(HeroActorState heroState) {
        SetState(heroState, false);
    }

    int GetUnk0(void) {
        return unk0;
    }

    int GetState(void) {
        return unk10;
    }

    bool SolidSurfaceState(int);
    bool BiteState(int);
    bool SwimmingState(int);
    bool IdleState(int);
    bool KnockBackState(int);
    bool MoveState(int);
    bool SneakState(int);
    bool WaterSurfaceState(int);
    bool FirstPersonState(int);

    bool AirState(int);

    bool SolidSurfaceState(void) {
        return SolidSurfaceState(GetState());
    }

    bool BiteState(void) {
        return BiteState(GetState());
    }

    bool SwimmingState(void) {
        return SwimmingState(GetState());
    }

    bool IdleState(void) {
        return IdleState(GetState());
    }

    bool KnockBackState(void) {
        return KnockBackState(GetState());
    }

    bool MoveState(void) {
        return MoveState(GetState());
    }

    bool SneakState(void) {
        return SneakState(GetState());
    }

    bool WaterSurfaceState(void) {
        return WaterSurfaceState(GetState());
    }

    bool FirstPersonState(void) {
        return FirstPersonState(GetState());
    }

    // needs to be defined before GetState?
    int GetStateEx(void) {
        return GetState();
    }

    void Update(Ty*);

    void DeinitState(Ty* pTy) {
        if (unk10 != -1) {
            if (pStates[unk10].Deinit) {
                (pTy->*pStates[unk10].Deinit)();
            }
        }

        unk10 = -1;
    }

    void Draw(Ty* pTy) {
        if (!gb.unkFE && unk10 != -1) {
            if (pStates[unk10].Draw) {
                (pTy->*pStates[unk10].Draw)();
            }
        }
    }
};

struct FloorInfo {
    bool bUnderFeet;
    bool bOn;
    bool bValid;
    bool unk3;
    CollisionResult res;

    float GetDiff(Vector* pPos) {
        return bValid ? pPos->y - res.pos.y : 10000.0f;
    }

    uint GetCollisionFlags(void) {
        return bValid ? res.collisionFlags : 0;
    }
};

struct TyContext {
    struct WaterInfo {
        bool bValid;
        Vector pos;

        void Update(Vector*, Vector*);
    };

    WaterInfo water;
    FloorInfo floor;
    CollisionResult results[4];
    int resultIndex;
    bool unk170;
    CollisionResult unk174;
    bool resetYVelocity;
    CollisionResult unk1BC;
    float jumpGravity;
    float jumpMaxGravity;

    void Init(float f1, float f2) {
        jumpGravity = f1;
        jumpMaxGravity = f2;

        Reset();
    }

    void Reset(void) {
        floor.unk3 = false;
        floor.bUnderFeet = false;
        floor.bOn = false;
        floor.bValid = false;
        water.bValid = false;

        unk170 = false;
        resetYVelocity = false;
    }

    void SetJumpGravity(float f1, float f2) {
        jumpGravity = f1;
        jumpMaxGravity = f2;
    }

    float GetYDistanceToWater(Vector* p) {
        return water.pos.y - p->y;
    }

    // was this defined in the header?
    void VelocityInline(Vector* pVelocity, float f1) {
        Vector g = {0.0f, -jumpGravity, 0.0f, 0.0f};

        if (floor.bOn) {
            Tools_ApplyFrictionAndGravity(pVelocity, &g, &floor.res.normal, f1);
        } else {
            pVelocity->Add(&g);
        }

        pVelocity->y = Clamp<float>(-jumpMaxGravity, pVelocity->y, jumpMaxGravity);
    }
};

struct LedgePullUpData {
    bool bOnGround;
    bool normalCheck;

    Vector unk4;
    Vector pos;

    float unk24;

    CollisionInfo* pInfo;
    GameObject* pGameObject;
    Vector nodeMatrixTranslation;
    Vector tyVelocity;

    StateMachine<LedgePullUpData> fsm;

    bool UpdateCheck(void);
    void LedgeMoved(Vector*, float);
    void Grab(void);
    void Hang(void);
    void InitPull(void);
    void Pull(void);
    void Jump(void);
    void PushOff(void);
};

struct LearnToDiveData {
    StateMachine<LearnToDiveData> fsm;

    int unk10;
    char unk14;
    int unk18;
};

struct TyBite {
    StateMachine<TyBite> fsm;
    Matrix unk10[3];

    int unkD0;
    float speed;
    float discreteAcceleration;
    float gravity;
    float unkE0;
    int unkE4;
    int superBiteCharge;
    ParticleSystem* pSystem;
    Material* pMat;
    float unkF4;
    float tyPosY;
    Vector tyPos; // y is updated with Ty's radius
    int unk10C;
    float unk110;
    Vector targetPos;
    float targetDist;
    Model* targetedModel;
    Vector targetDir;
    float unk13C;
    float unk140;
    bool bCollisionHit;
    bool unk145;
    int unk148;
    float tyRadius;
    Vector* unk150;
    bool isTargeting;
    bool unk155;
};

struct WaterSlideManager;
struct WaterSlideData {
    float unk0;
    int unk4;
    float* unk8;
    float unkC;
    float unk10;
    float unk14;
    int unk18;
    int unk1C;
    WaterSlideManager* pWSMan;
    StateMachine<WaterSlideData> slideFsm;
};

struct TrailPoint {
    Vector unk0;
    Vector unk10;
};

struct DustTrail {
    char padding[0x18];
};

struct Ty : Hero {

    int unk320;
    Vector unk324;

    int unk334;
    Vector unk338;

    int unk348;
    Vector unk34C;

    int unk35C;
    Vector unk360;

    int unk370;
    Vector unk374;

    int unk384;
    Vector unk388;

    int unk398;
    Vector unk39C;

    int unk3AC;
    Vector unk3B0;

    int unk3C0;
    Vector unk3C4;

    int unk3D4;
    Vector unk3D8;

    int unk3E8;
    Vector unk3EC; // specialPickupShowPos

    int unk3FC;
    Vector unk400;

    int unk410; // eyeRefPointIdx
    Vector unk414;

    int unk424;
    Vector unk428;

    int unk438;
    Vector unk43C;

    int unk44C;
    Vector unk450;

    int unk460;
    Vector unk464;

    int unk474;
    Vector unk478;

    int unk488;
    Vector unk48C;

    int unk49C;
    Vector unk4A0;
    
    int unk4B0;
    char padding4B4[0x10]; // Vector?

    int A_Head_idx; // 0x4C4
    int A_BiteHead_idx; // 0x4C8

    MKAnimScript animScript; // 0x4CC
    MKAnimScript unk4EC;
    MKAnimScript unk50C;

    bool unk52C;

    float unk530;

    MKAnimScript unk534;
    MKAnimScript rangPropLeftAnimScript;
    MKAnimScript rangPropRightAnimScript;
    char padding594[0x20]; // MKAnimScript?

    MKAnim* walkAnim;

    char padding348[0x828 - 0x5B8];
    uint unk828;
    BoomerangType mBoomerangType;
    TyContext mContext;
    TyFSM mFsm;
    int unkA54;
    Vector mSpawnPos;
    Vector mSpawnRot;
    int unkA78;
    Vector rangLight0;
    Vector rangLight1;
    char unkA9C[0x318]; // ReflectionStruct
    ParticleSystem* pSystems[3];
    int unkDC0;
    ParticleSystem* unkDC4;
    int breathMist;
    int randomNum;
    int unkDD0;
    int unkDD4;
    int unkDD8;
    Bubble mBubble;
    Vector unkDE0;
    float unkDF0;
    Vector ambientLight;
    Vector lights[3];
    Vector levelDataColours[3];
    Vector color;
    char unkE74[240];
    int unkF64;
    int unkF68;
    Material* pMatEyes;
    Material* pMatEyeballs;
    Material* rangTrailsMat;
    CircularQueue<TrailPoint> mRangTrails[2];
    bool unkF98;
    HeadTurningInfo mHeadTurningInfo;
    RainbowEffect mTyRainbowEffect;
    char unk1114;
    BoomerangManager mBoomerangManager;
    BoomerangAnimInfo unk11F8;
    BoomerangAnimInfo unk1220;
    int boomerangButton; // direction to change boomerangs
    bool unk124C;
    TyBite tyBite;
    Bunyip* pBunyip;
    int unk13AC;
    KnockBackType eKnockBackType;
    int unk13B4;
    float unk13B8;
    Vector knockbackDir;
    Vector knockbackPos;
    float unk13DC;
    Vector lastVelocity;
    Rotation mRot;
    float pitch;
    float roll;
    float unk140C;
    float magnitude;
    float unk1414;
    bool bPositionChange;
    int unk141C;
    int unk1420;
    ColObjDescriptor colObjDesc;
    CollisionObject tyColObj;
    // some structure here
    Vector directVelocity;
    Vector normal;
    int directvel_to_velocity_interpolation;
    bool unk167C;
    char unk167D;
    bool unk167E;
    float somVelMaxMag;
    bool fallDive;
    uint unk1688;
    int unk168C;
    int unk1690;
    float unk1694;
    int unk1698;
    int unk169C;
    int unk16A0;
    int unk16A4;

    AutoTargetStruct mAutoTarget;

    float jumpYStep;

    int unk18B8;

    float startFallY;

    int unk18C0;

    float jumpTurnFactor;
    float moveSmoothing;
    float totalGravity;

    int unk18D0;
    
    bool bFallingToIce;

    char padding_0x18D5[3];
    bool unk18D8;
    bool unk18D9;
    bool unk18DA;

    LedgePullUpData mLedgePullUpData;
    float initialVelocity;

    bool bLaunch;

    LearnToDiveData mLearnToDive;

    float unk1960;
    float unk1964;

    bool unk1968;

    Quadratic mQuadratic;

    WaterSlideData mWaterSlide;

    float unk19E0;
    char unk19E4[0x4];
    float unk19E8;

    CircularQueue<TrailPoint> mIceTrails[2];
    bool unk1A0C;

    Vector edgeVector;

    DustTrail mDustTrail;

    Vector deltaKnockbackPos;
    
    int unk1A48;
    float unk1A4C;
    
    Vector fallGuideInfluenceDir;
    float fallGuideInfluence;

    uint cameraState;

    float unk1A68;
    float unk1A6C;
    float unk1A70;

    int unk1A74;

    Vector lastSafePos;

    SpecialPickupStruct* pSpecialPickup;

    GameObject* pGameObject;

    TyMediumMachine mMediumMachine;

    virtual void Init(GameObjDesc* pDesc) {
        GameObject::Init(pDesc);
    }

    virtual void Deinit(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);

    virtual bool IsBiting(void);

    virtual bool InWater(void) {
        return false;
    }

    virtual bool IsClaiming(void);
    virtual void SetFindItem(Vector*, SpecialPickupStruct*);

    void LoadResources(void);

    void Init(void);
    void InitEvents(void);
    void PostLoadInit(void);
    
    void ResetVars(void);

    // Damage related
    void StartDeath(HurtType, bool);
    void Hurt(HurtType, DDADamageCause, bool, Vector*, float);

    void SetBunyip(Bunyip*);
    void AddShadowLight(Vector*, float);
    void SetAbsolutePosition(Vector*, int, float, bool);
    void SetBounceOffFromPos(Vector*, float, bool);
    void SetKnockBackFromDir(Vector*, float, KnockBackType);

    bool TryChangeState(bool, HeroActorState);
    bool TryChangeState(int, HeroActorState);

    void SetMedium(TyMedium medium) {
        // mMediumMachine.SetNewState(medium, false);
        // mMediumMachine.Update(this, false);
    }

    TyMedium GetMedium(void) {
        return mMediumMachine.GetUnk0();
    }

    bool TyOnPlatform(void) {
        return false;
    }

    void SetWarpHide(void) {
        
    }

    // Medium States
    void WaterMediumInit(void);
    void WaterMediumUpdate(void);
    void WaterMediumDeinit(void);

    void UnderWaterMediumInit(void);
    void UnderWaterMediumUpdate(void);
    void UnderWaterMediumDeinit(void);
    
    void AirMediumInit(void);
    void AirMediumUpdate(void);
    void AirMediumDeinit(void);
    
    void LandMediumInit(void);
    void LandMediumUpdate(void);
    void LandMediumDeinit(void);
    //

    // Rang Management
    void InitRangChange(void);
    void DeinitRangChange(void);
    void RangChange(void);

    void RangChangeTransition(void);

    void SwapRangs(char*);

    void InitTwirlRang(void);
    void TwirlRang(void);

    bool IsAbleToGlide(void);
    //
};

extern Ty ty;


// Not sure where to put this:
#include "ty/tools.h"

// this function is responsible for "@117" in 18 tus
inline float GetFloorHelper(float x, float y, float z, CollisionResult* pCr, float yDiff, bool bCollisionMode) {
    Vector v = {
        x, y, z, 0.0f
    };

    return Tools_GetFloor(v, pCr, yDiff, bCollisionMode, 0);
}

#endif // TY_H
