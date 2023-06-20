#ifndef RANGSTONE_H
#define RANGSTONE_H

#include "ty/props/StaticProp.h"
#include "ty/Messages.h"

struct RangStone : StaticProp {
    EventMessage OnSuccess;
    int rangHidden; // 0 = rang shown, 1 = rang hidden

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);
    void HideRang(void);
};

void RangStone_LoadResources(KromeIni* pIni);

#endif // RANGSTONE_H
