#include "types.h"
#include "common/StdMath.h"
#include "ty/props/SoundProp.h"
#include "ty/GameObjectManager.h"
#include "common/System_GC.h"
#include "ty/global.h"

extern int SoundBank_ResolveSoundEventIndex(char*);
extern "C" int Sound_IsVoicePlaying(int);

static GameObjDesc soundPropDesc;
static ModuleInfo<SoundProp> soundPropModule;

void SoundProp_LoadResources(KromeIni* pIni) {
	soundPropDesc.Init(&soundPropModule, "SoundProp", "SoundProp", 1, 0);
	soundPropDesc.maxDrawDist = 0.0f;
	soundPropDesc.maxScissorDist = 0.0f;
	soundPropDesc.maxUpdateDist = 1500.0f;
	objectManager.AddDescriptor(&soundPropDesc);
}

void SoundProp::Init(GameObjDesc* pDesc) {
	GameObject::Init(pDesc);
    pModel = NULL;
	unk84 = -1;
	unk7C.Init();
	maxDelay = 0;
	minDelay = 0;
    unk88 = 0;
	localToWorld.SetIdentity();
	gameObjFlags.Init(GameObjFlags_All);
}

void SoundProp::Deinit(void) {
	GameObject::Deinit();
	unk7C.Deinit();
}

bool SoundProp::LoadLine(KromeIniLine* pLine) {
    Vector rot;
    char string[50];
    if (LoadLevel_LoadVector(pLine, "rot", &rot) != false) {
        localToWorld.SetRotationYaw(rot.y);
        return true;
    }
    if (LoadLevel_LoadString(pLine, "sound", string, 30, 0) != false) {
        char* pString = string;
        if (string[0] == '*') {
            pString++;
        }
        unk84 = SoundBank_ResolveSoundEventIndex(pString);
        return true;
    }
    return GameObject::LoadLine(pLine) ||
        LoadLevel_LoadVector(pLine, "pos", localToWorld.Row3()) || 
        LoadLevel_LoadInt(pLine, "minDelay", &minDelay) || LoadLevel_LoadInt(pLine, "maxDelay", &maxDelay)||
        gameObjFlags.LoadLine(pLine);
}


void SoundProp::LoadDone(void) {
	GameObject::LoadDone();
	objectManager.AddObject(this, &localToWorld, NULL);
}

void SoundProp::Reset(void) {
	GameObject::Reset();
    unk7C.Reset();
	unk88 = 0;
	unk80 = 1;
	gameObjFlags.Reset();
}

void SoundProp::Update(void) {
    switch (unk80) {
        case 1:
            if (gameObjFlags.CheckFlags(GameObjFlags_Active)) {
                Play();
                unk80 = 2;
            }
            break;
        case 2:
            unk7C.Update(unk84, false, true, this, NULL, -1.0f, 0);
            if (!gameObjFlags.CheckFlags(GameObjFlags_Active)) {
                unk7C.Stop();
                unk80 = 1;
                break;
            }
            if (unk7C.unk0 == -1 || (Sound_IsVoicePlaying(unk7C.unk0) == false)) {
                unk7C.unk0 = -1;
                unk80 = 3;
            }
            break;
        case 3:
            if (--unk88 <= 0) {
                unk80 = 1;
            }
            break;
    }
}

void SoundProp::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case -4:
            if (unk80 != 0) {
                unk7C.Stop();
                unk80 = 1;
            }
            break;
        case 2:
            Reset();
            break;
        case 10:
            unk80 = 1;
            gameObjFlags.Set(GameObjFlags_Active);
            break;
        case 11:
            gameObjFlags.Clear(GameObjFlags_Active);
            break;
    }
    gameObjFlags.Message(pMsg);
    GameObject::Message(pMsg);
}

void SoundProp::Play(void) {
    unk7C.Update(unk84, false, true, this, NULL, -1.0f, 0);
    unk88 = RandomIR(&gb.mRandSeed, minDelay, maxDelay) * (int)gDisplay.displayFreq;
}
