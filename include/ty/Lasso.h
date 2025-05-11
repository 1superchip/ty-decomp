#ifndef LASSO_H
#define LASSO_H

#include "ty/tools.h"
#include "ty/Quadratic.h"
#include "ty/StateMachine.h"
#include "common/Model.h"
#include "common/Material.h"
#include "common/Animation.h"
#include "common/MKAnimScript.h"
#include "common/Vector.h"

struct Emu;
struct FrillBike;

void Lasso_LoadResources(void);

enum LassoState {
    LS_Idle             = 0,
    LS_Throw            = 1,
    LS_PullInRope       = 2,
    LS_CaughtEmu        = 3,
    LS_CaughtWaterTank  = 4,
    LS_CaughtFrillBike  = 5,
    LS_Snapped          = 6
};

struct Lasso {
    Vector unk0;
    Vector unk10;

    float unk20;

    StateMachine<Lasso> fsm;

    Model* unk34;

    CircularQueue<Vector> ropeQueue;

    Material* pTensionMat;
    Material* pRopeMat;

    Emu* pEmu;
    int unk54;
    FrillBike* pFrillBike;

    float unk5C;
    float unk60;
    float unk64;
    float unk68;
    float unk6C;
    int unk70;
    bool unk74;
    bool unk75;
    bool unk76;

    float unk78;

    Vector unk7C;
    Vector unk8C;
    Vector unk9C;
    Vector unkAC;

    Quadratic mQuadratic;

    float unkFC;
    float unk100;
    float unk104;
    float unk108;
    float unk10C;
    float unk110;
    int unk114;


    void Init(void);
    void Deinit(void);
    void Reset(void);
    void Update(Vector*, Vector*, float);

    void Draw(void);

    void GetControlPoints(Vector*, Vector*, Vector*);

    void InitLassoThrow(void);
    void LassoThrow(void);
    void DeinitLassoThrow(void);

    void InitLassoPullIn(void);
    void LassoPullIn(void);
    void DeinitLassoPullIn(void);

    void InitLassoCaughtEmu(void);
    void LassoCaughtEmu(void);
    void DeinitLassoCaughtEmu(void);

    void InitLassoCaughtWaterTank(void);
    void LassoCaughtWaterTank(void);
    void DeinitLassoCaughtWaterTank(void);

    void InitLassoCaughtFrillBike(void);
    void LassoCaughtFrillBike(void);
    void DeinitLassoCaughtFrillBike(void);

    void InitLassoSnapped(void);
    void LassoSnapped(void);
    void DeinitLassoSnapped(void);
    
    void CheckPullBike(void);
    void Snap(void);

    void LassoDrawSimple(void);
    void LassoDraw(void);

    void ThrowLasso(void);
    void InitIdle(void);

    bool NearRope(Vector*, float, Vector*, Vector*);

    void PullRope(float);
};

void DrawRope(CircularQueue<Vector>*, Material*, float);

#endif // LASSO_H
