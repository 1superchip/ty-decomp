#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include "ty/GameObject.h"
#include "ty/RangeCheck.h"
#include "ty/CollisionObject.h"

void Checkpoint_LoadResources(KromeIni* pIni);

enum CheckpointState {
    CPS_0 = 0,
    CPS_1 = 1,
    CPS_2 = 2,
    CPS_3 = 3,
    CPS_4 = 4,
    CPS_5 = 5,
};

struct CheckpointStruct : GameObject {
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Draw(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Message(MKMessage* pMsg);

    void SetState(CheckpointState nState);

    void Dormant(void);
    void Activating(void);
    void SpringOpen(void);

    Vector* GetPos(void) {
        return pModel->matrices[0].Row3();
    }

    LODManager mLodManager;
    Model* unk48;
    Animation* unk4C;

    Vector rot;
    Vector pos;
    float scale;

    CheckpointState mState;
    int unk78;
    int unk7C;

    int unk80;
    int unk84;
    int unk88;
    int unk8C;

    Vector cameraSource; // 0x90

    MKAnimScript mAnimScript;

    TyCollisionInfo mCollisionInfo;
};

#endif // CHECKPOINT_H
