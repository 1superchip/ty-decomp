#include "ty/props/D1_Prop.h"
#include "common/System_GC.h"
#include "ty/global.h"
#include "ty/GameObjectManager.h"

extern void Credits_Init(void);
extern void Credits_Deinit(void);
extern void Credits_Reset(void);
enum MusicType {};
void SoundBank_PlayMusic(MusicType, float, float);
extern "C" void Sound_MusicPause(bool);
enum PauseScreen_State {};
void PauseScreen_Init(PauseScreen_State);

static GameObjDesc d1PropDesc;
static ModuleInfo<D1Prop> d1PropModule;
static D1Prop* gpD1Prop = NULL;

void D1_Prop_LoadResources(KromeIni* pIni) {
    d1PropDesc.Init(&d1PropModule, "D1Prop", "D1Prop", 1, 2);
    d1PropDesc.Load(pIni);
    objectManager.AddDescriptor(&d1PropDesc);
}

void D1Prop::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    bActive = false;
    unk3D = false;
    Credits_Init();
    OnCompletion.Init();
    OnCompletion100.Init();
    gpD1Prop = this;
}

bool D1Prop::LoadLine(KromeIniLine* pLine) {
    return GameObject::LoadLine(pLine) || OnCompletion.LoadLine(pLine, "OnCompletion") ||
        OnCompletion100.LoadLine(pLine, "OnCompletion100");
}

void D1Prop::LoadDone(void) {
    GameObject::LoadDone();
    objectManager.AddObject(this, NULL, NULL);
}

void D1Prop::Deinit(void) {
    Credits_Deinit();
    GameObject::Deinit();
    gpD1Prop = NULL;
}

void D1Prop::Reset(void) {
    GameObject::Reset();
    Credits_Reset();
    bActive = false;
    unk3D = false;
}

void D1Prop::Update(void) {
    if (bActive) return;
}

void D1Prop::Draw(void) {
    return;
}

void D1Prop::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Resolve:
            OnCompletion.Resolve();
            OnCompletion100.Resolve();
            break;
        case MSG_Activate:
            if (System_GetCommandLineParameter("-introonly") == NULL && gb.autoLevelSwitch == false) {
                SoundBank_PlayMusic((MusicType)7, 0.5f, 0.0f);
                gb.bOnPauseScreen = true;
                PauseScreen_Init((PauseScreen_State)0x11);
                Sound_MusicPause(false);
                bActive = true;
            }
            break;
    }
}

void D1PropEndOfCredits(void) {
    if (gb.autoLevelSwitch == false) {
        if (gb.mGameData.GetGameCompletePercent() == 100) {
            gpD1Prop->OnCompletion100.Send();
        } else {
            gpD1Prop->OnCompletion.Send();
        }
    }
    
    gpD1Prop->bActive = false;
}
