#ifndef SHATTERABLE_H
#define SHATTERABLE_H

#include "ty/props/StaticProp.h"

struct ShatterStruct {
    void Explode(Vector*, float, float);
};

#define SHATTERABLE_SEARCHMASK (0x100)

void Shatterable_LoadResources(KromeIni* pIni);

struct ShatterableDescriptor : StaticPropDescriptor {
    float upVel;
    float power;
    float spinRate;
    float bounce;
    float gravity;
    int life;
    
    virtual void Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags);
    virtual void Load(KromeIni* pIni);
};

struct ShatterableFXPropDesc : ShatterableDescriptor {
    int effectFlags;
    
    virtual void Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags) {
        ShatterableDescriptor::Init(pMod, pMdlName, pDescrName, _searchMask, _flags);

        effectFlags = 0;
    }

    virtual void Load(KromeIni* pIni) {
        ShatterableDescriptor::Load(pIni);

        KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);

        if (pLine) {

            while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {

                if (pLine->pFieldName) {
                    NameFlagPair flagsTmp = {"slide", 1};
                    LoadLevel_LoadFlags(pLine, "effectFlags", &flagsTmp, 1, &effectFlags);
                }
    
                pLine = pIni->GetLineWithLine(pLine);
            }
        }
    }
};

struct Shatterable : StaticProp {
    int unk58;
    int unk5C;
    ShatterStruct* shatter;
    EventMessage OnShatter;

    virtual void Init(GameObjDesc* pDesc);
    virtual bool LoadLine(KromeIniLine*);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Draw(void);
    virtual void Message(MKMessage* pMsg);

    void PlayShatterSound(void);

    ShatterableDescriptor* GetDesc(void) {
        return descr_cast<ShatterableDescriptor*>(pDescriptor);
    }
};


struct ShatterableFX : Shatterable {
    ParticleSystem* pSystem;

    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    virtual void Update(void);

    ShatterableFXPropDesc* GetDesc(void) {
        return descr_cast<ShatterableFXPropDesc*>(pDescriptor);
    }
};

bool Shatterable_IsMember(MKPropDescriptor* pDesc);

#endif // SHATTERABLE_H
