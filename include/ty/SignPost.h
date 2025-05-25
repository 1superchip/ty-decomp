#ifndef SIGNPOST_H
#define SIGNPOST_H

#include "ty/props/StaticProp.h"
#include "common/MKAnimScript.h"

struct DetourData {
    MKAnimScript anim;
    char padding[0x8];
};

extern DetourData detourData;

void SignPost_LoadResources(KromeIni* pIni);
void SignPost_HideAll(void);
void SignPost_ShowAll(void);

// May not have been an enum
enum SignPostType {
    SP_TYPE_0       = 0,
    SP_TYPE_1       = 1,
    SP_DETOUR       = 2,
    SP_CONSTRUCTION = 3,
    SP_DIRECTION2   = 4,
    SP_DIRECTION    = 5,
    SP_BILLBOARD    = 6
};

struct SignPostDesc : StaticPropDescriptor {
    SignPostType type;
};

enum SignPostState {
    SPS_0 = 0,
    SPS_1 = 1,
    SPS_2 = 2,
};

struct SignPost : StaticProp {
    Vector mRot; // Current rotation
    Vector mDefaultRot; // Default Rotation
    int light1_index;
    int light2_index;
    int mRotSetting;
    int unk84;
    int unk88;
    int unk8C;
    float mRotInc;
    bool unk94;
    bool unk95;
    SignPostState mState;
    Vector centrePos;

    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);

    void Idle(void);
    void Hit(void);

    void SetState(SignPostState);

    SignPostDesc* GetDesc(void) {
        return descr_cast<SignPostDesc*>(pDescriptor);
    }
};

struct TaSignPost : SignPost {
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
};

#endif // SIGNPOST_H
