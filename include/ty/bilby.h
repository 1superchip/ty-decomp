#ifndef BILBY_H
#define BILBY_H

#include "ty/soundbank.h"
#include "ty/Shatter.h"
#include "common/Model.h"
#include "common/Collision.h"
#include "common/MKAnimScript.h"
#include "common/Vector.h"

enum BilbyState {
    BS_0 = 0,
    BS_1 = 1,
    BS_2 = 2,
};

enum BilbyType {
    BT_0 = 0,
    BT_1 = 1,
    BT_2 = 2,
    BT_3 = 3,
    BT_4 = 4,
    BT_MAX
};

struct BilbyLoadInfo {
    Vector pos;
    Vector rot;
    BilbyType type;
};

void Bilby_LoadResources(void);
void Bilby_Init(void);
void Bilby_Deinit(void);
void Bilby_Add(BilbyLoadInfo* loadInfo);
void Bilby_Update(void);
void Bilby_Draw(void);
void Bilby_HideAll(void);
void Bilby_ShowAll(void);

struct BilbyInternalStruct {
    Vector unk0;
    Vector unk10;
};

struct BilbyStruct {
    BilbyType mType;
    Model* pModel;
    Model* pModel2;
    Vector mPos;
    Vector mRot;
    float unk2C;
    bool unk30;
    bool unk31;
    bool unk32;
    BilbyState mState;
    short unk38;
    short unk3A;
    short unk3C;
    short unk3E;
    short unk40;
    int unk44;
    int unk48;
    uint mNextTime;
    SoundEventHelper mSoundHelper;
    ShatterStruct* pShatter;
    CollisionInfo mCollisionInfo;
    MKAnimScript mAnimScript;
    BilbyInternalStruct unk84[5];
    int unk124;
    int unk128;
    MKAnim** unk12C;
    bool bInitialised;

    void Init(BilbyLoadInfo* loadInfo);
    void Idle(void);
    void Rescued(void);
    Vector* GetPos(void);
    void SetState(BilbyState newState);
};

void Bilby_SetRescued(BilbyType type, bool r4);
bool Bilby_Rescued(BilbyType type);

void Bilby_EnableCheatLines(bool);

#endif // BILBY_H
