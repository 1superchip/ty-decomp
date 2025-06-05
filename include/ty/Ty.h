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

};

// No symbol
struct TyMediumMachine {

    typedef void(Ty::*Func1)(void);
    typedef void(Ty::*Func2)(void);
    typedef void(Ty::*Func3)(void);
    typedef void(Ty::*Func4)(void);
    typedef void(Ty::*Func5)(void);

    struct State {
        Func1 func1;
        Func2 func2;
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

    void CallDeinit(void) {

    }

    void Update(Ty* pActor, bool arg2) {

    }

    void SetNewState(TyMedium nState, bool arg2) {

    }
};

enum HeroActorState {

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
        if (arg2) {
            unk14 = newState;
        } else if (unk10 != newState) {
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

    char padding348[0x828 - 0x5B4];
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
    Material* unkF6C;
    Material* pMatEyeballs;
    Material* rangTrailsMat;
    CircularQueue<int> unkF78; // Fix type
    CircularQueue<int> unkF88; // Fix type
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

    // finish


    virtual bool IsBiting(void);

    virtual bool InWater(void) {
        return false;
    }

    virtual bool IsClaiming(void);
    virtual void SetFindItem(Vector*, SpecialPickupStruct*);

    void SetBunyip(Bunyip*);
    void PostLoadInit(void);
    void AddShadowLight(Vector*, float);
    void SetAbsolutePosition(Vector*, int, float, bool);
    void SetBounceOffFromPos(Vector*, float, bool);
    void Hurt(HurtType, DDADamageCause, bool, Vector*, float);
    void SetKnockBackFromDir(Vector*, float, KnockBackType);

    bool TryChangeState(bool, HeroActorState);
    bool TryChangeState(int, HeroActorState);

    void SetMedium(TyMedium medium) {
        // mMediumMachine.SetNewState(medium, false);
        // mMediumMachine.Update(this, false);
    }

    TyMedium GetMedium(void) {

    }

    bool TyOnPlatform(void) {

    }

    void SetWarpHide(void) {
        
    }
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
