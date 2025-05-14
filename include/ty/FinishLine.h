#ifndef FINISHLINE_H
#define FINISHLINE_H

#include "ty/GameObject.h"
#include "common/MKAnimScript.h"

void FinishLine_Show(void);
void FinishLine_Hide(void);
bool FinishLine_IsCrossedBy(Vector*);

enum FinishLineState {
    FLS_0 = 0,
    FLS_1 = 1,
    FLS_2 = 2,
};

/// @brief Finish Line object in Time Attacks
struct FinishLineStruct : GameObject {
    bool bOn;
    FinishLineState state;
    char padding[8];
    MKAnimScript animScript;
    Matrix mtx;
    Vector scale;
    bool unkBC;

    void Idle(void);
    void PostFinish(void);
    void SetState(FinishLineState);
    bool IsCrossedBy(Vector*);

    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Reset(void);
    virtual void Message(MKMessage* pMsg);
};

#endif // FINISHLINE_H
