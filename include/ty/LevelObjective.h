#ifndef LEVELOBJECTIVE_H
#define LEVELOBJECTIVE_H

#include "ty/GameObject.h"
#include "ty/Messages.h"

struct LevelObjective : GameObject {
    EventMessage OnSuccess;
    EventMessage OnFailure;
    EventMessage OnIncrement;
    EventMessage OnStart;
    EventMessage OnAbort;
    bool bActive;
    bool bComplete;
    short unk66;
    short unk68;
    short unk6A;
    char* pMatName;
    Material* pMaterial;
    Vector pos;
    bool unk84;

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    virtual void Failure(void);
    virtual void Success(void);
    virtual void Increment(void);
    virtual void Abort(void);
    virtual void Activate(void);
    virtual void Deactivate(void);

    static bool GetStatus(short*, short*, Material**, bool);

    static LevelObjective* pCurObjective;
};

#endif // LEVELOBJECTIVE_H
