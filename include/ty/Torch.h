#ifndef TORCH_H
#define TORCH_H

#include "ty/props/StaticProp.h"
#include "ty/WobbleTexture.h"
#include "ty/soundbank.h"
#include "ty/heatflare.h"

struct ParticleSystem;

extern int pTorchPlayingSFX;

void Torch_LoadResources(KromeIni* pIni);

enum TorchState {
    TORCH_IDLE  = 1,
    TORCH_HIT   = 2
};

enum TorchRotationSpeed {};

struct Torch : StaticProp {
    virtual void Init(GameObjDesc* pDesc);
    virtual void LoadDone(void);
    virtual void Deinit(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Message(MKMessage*);

    void Idle(void);
    void Hit(void);
    void SetState(TorchState, int);
    void EmitFire(void);
    void UpdateShadow(void);
    void CheckForHit(void);

    Vector mRot;
    Vector mDefaultRot;
    Vector mDefaultScale; // X component is used for flames
    Vector mFlamePos; // Flame Position
    float mRotInc; // How much rotation is updated by
    float mFloorY; // not exactly?
    float mCeilingY; // not exactly?
    float mWaterY; // Water Y position below the Torch
    float unkA8;
    int unkAC;
    int unkB0;
    int mFrozenTimer;
    int mFlameRefIndex;
    SoundEventHelper mSoundHelper;
    int mRotSpeedSetting; // Setting to determine how fast/slow the rotation should change
    bool bEmitFire; // shadow/sound related??
    bool bFoundWater;
    TorchState mState;
    int unkCC; // Unset field!!
    HeatFlareInfo* pHeatFlare;
    ParticleSystem* mpParticleSys0;
    ParticleSystem* mpParticleSys1;
    int unkDC;
    Material* mpWobbleTexMat;
    WobbleTexture mWobbleTex;
    Vector mLightCol;
    float unk10C;
    bool bEmitFire2; // Not sure why this is used?
};

#endif // TORCH_H
