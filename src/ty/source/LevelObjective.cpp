#include "ty/LevelObjective.h"
#include "ty/GameObjectManager.h"

struct UnkLevelInfo {
    Model* pModel;
    char padding0[0x10];
    char padding1[0x24];
};

struct GlobalVar {
    char padding0[0x2B8];
    int randSeed;
    char padding1[0x44];
    UnkLevelInfo levels[8];
    char padding2[0x50];
    int nmbrOfGroundModels;
    char padding3[0xAC];
    Vector color;
    char padding4[0x17C];
    Material* pShadowMat;
};

extern GlobalVar gb;

LevelObjective* LevelObjective::pCurObjective;

void LevelObjective::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    unk66 = 0;
    unk68 = 0;
    OnSuccess.Init();
    OnFailure.Init();
    OnIncrement.Init();
    OnStart.Init();
    OnAbort.Init();
    pMaterial = NULL;
    pMatName = NULL;
    pCurObjective = NULL;
}

void LevelObjective::Deinit(void) {
    if (pCurObjective == this) {
        pCurObjective = NULL;
    }
    unk66 = 0;
    unk68 = 0;
    if (pMaterial) {
        pMaterial->Destroy();
        pMaterial = NULL;
    }
    GameObject::Deinit();
}

void LevelObjective::Reset(void) {
    unk68 = 0;
    bActive = false;
    bComplete = false;
    if (unk84) {
        unk84 = false;
    }
    GameObject::Reset();
}

bool LevelObjective::LoadLine(KromeIniLine* pLine) {
    return OnSuccess.LoadLine(pLine, "OnSuccess") ||
        OnFailure.LoadLine(pLine, "OnFailure") ||
        OnIncrement.LoadLine(pLine, "OnIncrement") ||
        OnStart.LoadLine(pLine, "OnStart") ||
        OnAbort.LoadLine(pLine, "OnAbort") ||
        LoadLevel_LoadVector(pLine, "pos", &pos) ||
        GameObject::LoadLine(pLine);
}

void LevelObjective::LoadDone(void) {
	objectManager.AddObject(this, NULL, NULL);
	GameObject::LoadDone();
    unk84 = true;
    if (pMatName && *pMatName != '\0') {
        pMaterial = Material::Create(pMatName);
    }
}

void LevelObjective::Message(MKMessage* pMessage) {
    switch (pMessage->unk0) {
        case 2:
            Reset();
            break;
        case 10:
            Activate();
            break;
        case 11:
            Deactivate();
            break;
        case 0x16:
            if (bComplete || !bActive) return;
            Increment();
            break;
        case 0x30:
            Abort();
            break;
        case MKMSG_Resolve:             
            OnSuccess.Resolve();
            OnFailure.Resolve();
            OnIncrement.Resolve();
            OnStart.Resolve();
            OnAbort.Resolve();
    }
    GameObject::Message(pMessage);
}

void LevelObjective::Activate(void) {
    bComplete = false;
    bActive = true;
    OnStart.Send();
}

void LevelObjective::Deactivate(void) {
    bActive = false;
}

extern void Hud_ShowSpecialPickups(void);
extern void SoundBank_Play(int, Vector*, uint);

void LevelObjective::Increment(void) {
    OnIncrement.Send();
    Hud_ShowSpecialPickups();
    // inline here to get the level id
    switch (*(int*)&gb.padding4[0x11C]) {
        case 4:
            SoundBank_Play(0x22C, NULL, 0);
            break;
        case 6:
            SoundBank_Play(0x22D, NULL, 0);
            break;
        case 8:
            SoundBank_Play(0x22E, NULL, 0);
            break;
        case 9:
            SoundBank_Play(0x22F, NULL, 0);
            break;
        case 10:
            SoundBank_Play(0x230, NULL, 0);
            break;
        case 12:
            SoundBank_Play(0x231, NULL, 0);
            break;
        case 13:
            SoundBank_Play(0x232, NULL, 0);
            break;
        case 14:
            SoundBank_Play(0x233, NULL, 0);
            break;
        case 0x17:
            SoundBank_Play(0x234, NULL, 0);
            break;
        default:
            SoundBank_Play(0x1B5, NULL, 0);
            break;
    }
    pCurObjective = this;
    if (++unk68 >= unk66) {
        Success();
    }
}

void LevelObjective::Failure(void) {
    OnFailure.Send();
    bActive = false;
}

void LevelObjective::Success(void) {
    OnSuccess.Send();
    bActive = false;
    unk84 = false;
}

void LevelObjective::Abort(void) {
    if (!bActive) return;
    OnAbort.Send();
    bActive = false;
}

// gets information about the current objective
bool LevelObjective::GetStatus(short* r3, short* r4, Material** ppOutMat, bool r6) {
    if (!pCurObjective || pCurObjective->bComplete) {
        return false;
    }
    if (!r6 && *(u8*)&gb.padding4[0x128] && pCurObjective->unk68 == pCurObjective->unk66) {
        return false;
    }
    if (r3) {
        *r3 = pCurObjective->unk68;
    }
    if (r4) {
        *r4 = pCurObjective->unk66;
    }
    if (ppOutMat) {
        *ppOutMat = pCurObjective->pMaterial;
    }
    return true;
}
