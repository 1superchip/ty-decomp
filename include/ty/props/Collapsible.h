#ifndef COLLAPSIBLE_H
#define COLLAPSIBLE_H

#include "ty/props/StaticProp.h"
#include "common/KromeIni.h"

// move this to the proper header
struct ShatterStruct {
    char padding_0x0[0x24];
    Model* pModel;
    void Fall(void);
};
ShatterStruct* Shatter_Add(Model*, float, float, int);

enum CollapsibleState {
    COLLAPSIBLE_STATE_0 = 0,
    COLLAPSIBLE_STATE_1 = 1,
    COLLAPSIBLE_STATE_2 = 2,
    COLLAPSIBLE_STATE_3 = 3,
    COLLAPSIBLE_STATE_4 = 4,
};

struct Collapsible : StaticProp {
    CollapsibleState state;
    int unk5C;
    Vector defaultTrans; // model translation
    Vector scale;
    Vector defaultRot;
    ShatterStruct* shatter;
    
    virtual void LoadDone(void);

    virtual void Reset(void) {
        pModel->matrices[0].SetTranslation(&defaultTrans);
        pModel->matrices[0].SetRotationPYR(&defaultRot);
        pModel->matrices[0].Scale(&scale);
        state = COLLAPSIBLE_STATE_0;
        unk5C = 0;
        collisionInfo.bEnabled = true;
        collisionInfo.flags = 0;
        collisionInfo.pProp = NULL;
    }

    virtual void Update(void);

    virtual void Draw(void) {
        if (!Hidden()) {
            // if the Collapsible isn't hidden, draw it
            StaticProp::Draw();
        }
    }
    
    virtual void Init(GameObjDesc* pDesc) {
        StaticProp::Init(pDesc);
        state = COLLAPSIBLE_STATE_0;
        shatter = Shatter_Add(pModel, 1.0f, 0.7f, 0x78);
        Reset();
    }
    
    bool Hidden(void) {
        return state == COLLAPSIBLE_STATE_3 || state == COLLAPSIBLE_STATE_2;
    }
};

void Collapsible_LoadResources(KromeIni*);

#endif // COLLAPSIBLE_H