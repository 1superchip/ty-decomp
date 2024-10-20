#ifndef BUNYIPSTONE_H
#define BUNYIPSTONE_H

#include "ty/props/StaticProp.h"

void BunyipStone_LoadResources(KromeIni* pIni);

enum BunyipStone_BallState {
    BS_BALLSTATE_SHOW   = 0, // Balls rise out of the stone
    BS_BALLSTATE_UPDATE = 1, // Balls wobble and rotate
    BS_BALLSTATE_HIDE   = 2, // Balls drop into the stone
};

// Object used to activate the Bunyip when Ty walks over it
struct BunyipStone : StaticProp {
    Vector mBallRot1;
    Vector mBallRot2;
    
    int cooldownTimer;

    // 0 is the larger center ball
    // 1 and 2 are the smaller balls that rotate around the center
    int stoneMatrixIndices[3];

    float centerBallYWobble; // Angle that controls the Y position of the center ball

    float unk8C;
    float unk90;
    float unk94;
    float unk98;

    BunyipStone_BallState mBallState;

    virtual void LoadDone(void);
    virtual void Init(GameObjDesc* pDesc);
    virtual void Reset(void);
    virtual void Update(void);
    void UpdateBalls(void);
    void OrbitBalls(void);
};

#endif // BUNYIPSTONE_H
