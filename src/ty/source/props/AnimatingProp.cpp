#include "ty/props/AnimatingProp.h"
#include "ty/GameObjectManager.h"
#include "common/Str.h"
#include "common/system_extras.h"
int SoundBank_ResolveSoundEventIndex(char*);

static ModuleInfo<AnimatingProp> animPropModInfo;

void AnimatingProp_LoadResources(KromeIni* pIni) {
    AnimatingPropDesc animPropDescDefault;

    animPropDescDefault.Init(
        &animPropModInfo, 
        "", "", 
        1, 0, 
        (Tools_AnimEntry*)NULL, 0, 
        (Tools_AnimEvent*)NULL, 0, 
        1, GameObjFlags_All
    );

    LoadAnimPropDescriptors<AnimatingPropDesc>(pIni, "AnimatingProps", &animPropDescDefault);
}

void AnimatingPropDesc::Init(ModuleInfoBase* pModuleBase, char* param_2, char* param_3, int param_4, int param_5,
        Tools_AnimEntry* pAnimEntry, int animEntryCount, Tools_AnimEvent* pAnimEvent, int animEventCount, bool param_10, u16 _defaultGameObjFlags) {
    StaticPropDescriptor::Init(pModuleBase, param_2, param_3, param_4, param_5);
    defaultGameObjFlags = _defaultGameObjFlags;
    soundEventIndex = -1;
    nmbrOfSoundEvents = 0;
    pSoundEvents = NULL;

    if (stricmp(param_2, "") != 0) {
        mAnimScript.Init(modelName);
        mAnimDesc.Init(pAnimEntry, animEntryCount, &mAnimScript);
        mAnimEventDesc.Init(pAnimEvent, animEventCount, &mAnimScript);
    } else {
        mAnimDesc.pEntries = pAnimEntry;
        mAnimDesc.unk4 = animEntryCount;
        mAnimEventDesc.pEvents = pAnimEvent;
        mAnimEventDesc.mCount = animEventCount;
    }
    
    unk10C = param_10;
}

void AnimatingPropDesc::Load(KromeIni* pIni) {
    StaticPropDescriptor::Load(pIni);
    KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);
    char* pString = NULL;
    while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
        if (pLine->pFieldName != NULL) {
            if (stricmp(pLine->pFieldName, "idleSound") == 0 && pLine->AsString(0, &pString)) {
                soundEventIndex = SoundBank_ResolveSoundEventIndex(pString);
            } else if (stricmp(pLine->pFieldName, "soundEvents") == 0 && pLine->elementCount > 0) {
                nmbrOfSoundEvents = pLine->elementCount;
                pSoundEvents = (SoundEvent*)Heap_MemAlloc(nmbrOfSoundEvents * sizeof(SoundEvent));
                for (int i = 0; i < nmbrOfSoundEvents; i++) {
                    pSoundEvents[i].animEventName = mAnimScript.GetEventByName(Str_Printf("SoundEvent%d", i + 1));
                    pLine->AsString(i, &pString);
                    pSoundEvents[i].soundEventIndex = SoundBank_ResolveSoundEventIndex(pString);
                }
            }
        }
        pLine = pIni->GetLineWithLine(pLine);
    }
}

void AnimatingProp::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);

    pModel = Model::Create(
        GetDesc()->mAnimScript.GetMeshName(), 
        GetDesc()->mAnimScript.GetAnimName()
    );

    pModel->renderType = 3;

    collisionInfo.Init(true, GetDesc()->collisionInfoFlags, this);
    lodManager.Init(pModel, 0, &GetDesc()->lodDesc);
    collide = true;
    mAnimScript.Init(&GetDesc()->mAnimScript);
    mAnimManager.Init(&GetDesc()->mAnimDesc);
    mAnimEventManager.Init(&GetDesc()->mAnimEventDesc);
    gameObjFlags.Init((CommonGameObjFlags)GetDesc()->defaultGameObjFlags);
    soundHelper.Init();
}

bool AnimatingProp::LoadLine(KromeIniLine* pLine) {
    return StaticProp::LoadLine(pLine) || gameObjFlags.LoadLine(pLine);
}

void AnimatingProp::LoadDone(void) {
    StaticProp::LoadDone();
    
    if (GetDesc()->unk10C) {
        // set initial animation to idling animation
        MKAnim* pIdleAnim = mAnimScript.GetAnim("idle");
        if (pIdleAnim) {
            mAnimScript.SetAnim(pIdleAnim);
        }
    }

    gameObjFlags.SetDefaultFlags();
}

void AnimatingProp::Reset(void) {
    GameObject::Reset();
    gameObjFlags.Reset();
    collisionInfo.bEnabled = gameObjFlags.CheckFlags(GameObjFlags_Enabled) && gameObjFlags.CheckFlags(GameObjFlags_Visible);
    mAnimScript.Reset();
    soundHelper.Reset();
}

void AnimatingProp::Update(void) {
    bool flagsCheck = gameObjFlags.CheckFlags(GameObjFlags_Enabled) && gameObjFlags.CheckFlags(GameObjFlags_Visible);

    if (GetDesc()->soundEventIndex > -1) {
        soundHelper.Update(GetDesc()->soundEventIndex, false, flagsCheck && mAnimScript.currAnim, this, NULL, -1.0f, 0);
    }

    if (flagsCheck) {
        mAnimScript.Animate();
        mAnimScript.Apply(pModel->pAnimation);
        mAnimEventManager.Update(&mAnimScript);
        if (GetDesc()->nmbrOfSoundEvents > 0) {
            int i = 0;
            char* event;
            while (event = mAnimScript.GetEvent(i)) {
                for (int j = 0; j < GetDesc()->nmbrOfSoundEvents; j++) {
                    if (event == GetDesc()->pSoundEvents[j].animEventName) {
                        SoundEventHelper soundEvent;
                        soundEvent.unk0 = -1;
                        soundEvent.Update(GetDesc()->pSoundEvents[j].soundEventIndex, false, true,
                            this, NULL, -1.0f, 0);
                        break;
                    }
                }
                i++;
            }
        }
    }
}

void AnimatingProp::Draw(void) {
    if (gameObjFlags.CheckFlags(GameObjFlags_Enabled) && gameObjFlags.CheckFlags(GameObjFlags_Visible)) {
        StaticProp::Draw();
    }
}

void AnimatingProp::Message(MKMessage* pMsg) {
    gameObjFlags.Message(pMsg);
    collisionInfo.bEnabled = gameObjFlags.CheckFlags(GameObjFlags_Enabled) &&
        gameObjFlags.CheckFlags(GameObjFlags_Visible);
    switch (pMsg->unk0) {
        case MSG_Activate:
            Activate(true);
            break;
        case MSG_Deactivate:
            Activate(false);
            break;
        case MSG_Enable:
            Enable(true);
            break;
        case MSG_Disable:
            Enable(false);
            break;
        case MSG_Show:
            Show(true);
            break;
        case MSG_Hide:
            Show(false);
            break;
    }
    
    GameObject::Message(pMsg);

    if (pMsg->unk0 == -4 || !collisionInfo.bEnabled) {
        soundHelper.Reset();
    }
}

void AnimatingProp::Show(bool bShow) {
    if (bShow) {
        gameObjFlags.Set(GameObjFlags_Visible);
    } else {
        gameObjFlags.Clear(GameObjFlags_Visible);
    }
}

void AnimatingProp::Activate(bool bActivate) {
    if (bActivate) {
        gameObjFlags.Set(GameObjFlags_Active);
    } else {
        gameObjFlags.Clear(GameObjFlags_Active);
    }
}

void AnimatingProp::Enable(bool bEnable) {
    if (bEnable) {
        gameObjFlags.Set(GameObjFlags_Enabled);
    } else {
        gameObjFlags.Clear(GameObjFlags_Enabled);
    }
}
