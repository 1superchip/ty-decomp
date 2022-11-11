#include "types.h"
#include "common/StdMath.h"
#include "ty/props/SoundProp.h"
#include "ty/GameObjectManager.h"

extern GameObjectManager objectManager;
extern struct DisplayStruct {
    float unk[16];
} gDisplay;
extern struct GlobalVar {
    int unk[483];
    float unk78C;
} gb;
extern void SoundBank_Stop(int*);
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
	unk7C.unk0 = -1;
	maxDelay = 0;
	minDelay = 0;
    unk88 = 0;
	localToWorld.SetIdentity();
	defaultFlags = 7;
	gameObjFlags.Init((CommonGameObjFlags)(GameObjFlags_Active | GameObjFlags_Enabled | GameObjFlags_Visible));
}

void SoundProp::Deinit(void) {
	GameObject::Deinit();
	SoundBank_Stop(&unk7C.unk0);
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
	SoundBank_Stop(&unk7C.unk0);
	unk88 = 0;
	unk80 = 1;
	gameObjFlags.flags = defaultFlags;
}

void SoundProp::Update(void) {
    switch (unk80) {
        case 1:
            if (gameObjFlags.flags & GameObjFlags_Active) {
                Play();
                unk80 = 2;
            }
            break;
        case 2:
            unk7C.Update(unk84, false, true, this, 0, -1.0f, 0);
            if (!(gameObjFlags.flags & GameObjFlags_Active)) {
                SoundBank_Stop(&unk7C.unk0);
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
                SoundBank_Stop(&unk7C.unk0);
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
    }
    gameObjFlags.Message(pMsg);
    GameObject::Message(pMsg);
}

void SoundProp::Play(void) {
    unk7C.Update(unk84, false, true, this, 0, -1.0f, 0);
    unk88 = RandomIR(&gb.unk[174], minDelay, maxDelay) * (int)gDisplay.unk[2];
}