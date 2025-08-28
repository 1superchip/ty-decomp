#ifndef ANIMATINGPROP_H
#define ANIMATINGPROP_H

#include "ty/props/StaticProp.h"
#include "ty/soundbank.h"
#include "ty/CommonGameObjectFlags.h"
#include "common/MKAnimScript.h"
#include "ty/tools.h"

struct AnimSoundEvent {
    int soundEventIndex;
    char* animEventName;
};

void AnimatingProp_LoadResources(KromeIni* pIni);

struct AnimatingPropDesc : StaticPropDescriptor {
    MKAnimScript mAnimScript;
    Tools_AnimDesc mAnimDesc;
    Tools_AnimEventDesc mAnimEventDesc;
    bool unk10C;
    bool unk10D;
    u16 defaultGameObjFlags;
    int soundEventIndex;
    int nmbrOfSoundEvents;
    AnimSoundEvent* pSoundEvents;

    void Init(ModuleInfoBase *pModuleBase, char *param_2, char *param_3, int param_4,
            int param_5, Tools_AnimEntry *pAnimEntry, int animEntryCount, Tools_AnimEvent *pAnimEvent, int animEventCount,
            bool param_10, u16 _defaultGameObjFlags);
    virtual void Load(KromeIni* pIni);
};

struct AnimatingProp : StaticProp {
    MKAnimScript mAnimScript;
    Tools_AnimManager mAnimManager;
    Tools_AnimEventManager mAnimEventManager;
    SoundEventHelper soundHelper;
    CommonGameObjFlagsComponent gameObjFlags;

    virtual void Init(GameObjDesc* pDesc);
    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Show(bool bShow);
    virtual void Activate(bool bActivate);
    virtual void Enable(bool bEnable);
    
    void SetAnim(uint index) { // this function could be wrong, need to decomp code that uses it
        mAnimScript.SetAnim(mAnimManager.GetAnim(index));
    }

    AnimatingPropDesc* GetDesc(void) {
        return descr_cast<AnimatingPropDesc*>(pDescriptor);
    }
};

extern "C" void memcpy(void*, void*, int);

template <typename T>
void LoadAnimPropDescriptors(KromeIni* pIni, char* name, T* pDesc) {
    GameObjDesc* desc = NULL;
    KromeIniLine* pLine = pIni->GotoLine(name, NULL);

    while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
        char* pExactName = NULL;

        if (pLine->pFieldName != NULL && pLine->AsString(0, &pExactName) != false) {

            T* pCurrDesc = (T*)Heap_MemAlloc(sizeof(T));

            memset(pCurrDesc, 0, sizeof(T));

            new ((void*)pCurrDesc) T;

            Tools_AnimEntry* pAnimEntries = NULL;
            if (pDesc->mAnimDesc.unk4) {
                pAnimEntries = (Tools_AnimEntry*)Heap_MemAlloc(pDesc->mAnimDesc.unk4 * sizeof(Tools_AnimEntry));
                memcpy(pAnimEntries, (void*)pDesc->mAnimDesc.pEntries, pDesc->mAnimDesc.unk4 * sizeof(Tools_AnimEntry));
            }

            Tools_AnimEvent* pAnimEventEntries = NULL;
            if (pDesc->mAnimEventDesc.mCount) {
                pAnimEventEntries = (Tools_AnimEvent*)Heap_MemAlloc(pDesc->mAnimEventDesc.mCount * sizeof(Tools_AnimEvent));
                memcpy(pAnimEventEntries, (void*)pDesc->mAnimEventDesc.pEvents, pDesc->mAnimEventDesc.mCount * sizeof(Tools_AnimEvent));
            }

            pCurrDesc->Init(pDesc->pModule, pExactName, pLine->pFieldName, pDesc->searchMask, pDesc->flags,
                pAnimEntries, pDesc->mAnimDesc.unk4, pAnimEventEntries, pDesc->mAnimEventDesc.mCount, pDesc->unk10C, pDesc->defaultGameObjFlags);
            pCurrDesc->unk80 = desc;
            desc = (GameObjDesc*)pCurrDesc;
        }

        pLine = pIni->GetLineWithLine(pLine);
    }
    
    while (desc != NULL) {
        GameObjDesc* p = desc;
        desc = p->unk80;
        p->Load(pIni);
        objectManager.AddDescriptor(p);
    }
}

#endif // ANIMATINGPROP_H
