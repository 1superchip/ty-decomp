#ifndef GUIDEPARTICLE_H
#define GUIDEPARTICLE_H

#include "ty/GameObject.h"

void GuideParticle_LoadResources(KromeIni* pIni);
void GuideParticle_Show(void);
void GuideParticle_Hide(void);
bool GuideParticle_HasPassedThoughAll(void);

struct GuideParticle : GameObject {
    float unk3C;
    Matrix unk40;
    int unk80;
    int unk84;

    int unk88;

    static int gateTimeOutCounter;

    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);

    static void UpdateModule(void);
};

#endif // GUIDEPARTICLE_H
