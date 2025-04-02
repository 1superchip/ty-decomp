#ifndef BUNYIP_H
#define BUNYIP_H

#include "ty/props/AnimatingProp.h"

void Bunyip_LoadResources(KromeIni*);

bool Bunyip_On(void);
bool Bunyip_Activate(void);
int Bunyip_GetLastEvent(void);
bool Bunyip_IsLastEventGroundHit(void);

enum BunyipState {
    BUNYIP_STATE_0      = 0,
    BUNYIP_APPEAR       = 1,
    BUNYIP_IDLE         = 2,
    BUNYIP_STATE_3      = 3,
    BUNYIP_STATE_4      = 4,
    BUNYIP_ROAR         = 5,
    BUNYIP_PUNCH        = 6,
    BUNYIP_STATE_7      = 7,
    BUNYIP_DISAPPEAR    = 8,
};

struct Bunyip : AnimatingProp {
    Vector mRot;
    Vector mDefaultPos;
    Vector unkB0;

    char unkC0[4];

    int unkC4;
    int mHandIndex; // Punching hand node index
    int bunyipLifeSpan;

    char unkD0[4];

    float mScale;
    bool unkD8;
    bool unkD9; // state change flag?
    bool unkDA;
    char unkDB;
    SoundEventHelper mSoundHelper;
    BunyipState mState;
    int unkE4;

    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);

    void Appear(void);
    void Idle(void);
    void Roar(void);
    void PunchCombo(void);
    void Disappear(void);
    void SetState(BunyipState);
    void UpdatePos(Vector* pPos, Vector* pRot);
    void CheckEvents(void);
    bool HasPunched(Vector* pPoint, float radius);

    int StateInline(void) {
        return (mState > 1 && mState != BUNYIP_DISAPPEAR) ? true : false;
    }
};

#endif // BUNYIP_H
