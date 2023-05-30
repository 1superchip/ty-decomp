#include "ty/props/Z1_RainbowScales.h"
#include "ty/GameData.h"
#include "ty/GameObjectManager.h"

extern struct GlobalVar {
    char unk[0x104];
    GameData gameData;
} gb;

static GameObjDesc rsObjectiveObjectiveDesc;
static ModuleInfo<RainbowScaleObjective> rsObjectiveModule;

void Z1RainbowScaleObjective_LoadResources(KromeIni* pIni) {
    rsObjectiveObjectiveDesc.Init(&rsObjectiveModule, "RainbowScaleObjective", "RainbowScaleObjective", 0x201, 0);
    objectManager.AddDescriptor(&rsObjectiveObjectiveDesc);
}

void RainbowScaleObjective::Init(GameObjDesc* pDesc) {
    LevelObjective::Init(pDesc);
    bActive = true;
    bComplete = false;
    bStarted = false;
}

void RainbowScaleObjective::Reset(void) {
    LevelObjective::Reset();
    bActive = true;
    bComplete = false;
    bStarted = false;
}

void RainbowScaleObjective::Update(void) {
    if (!bActive) {
        return;
    }
    if (gb.gameData.GetLevelCollectedGemCount() == 1 && !bStarted) {
        OnStart.Send();
        bActive = true;
        bStarted = true;
    }
    if (gb.gameData.GetLevelCollectedGemCount() == unk66) {
        OnSuccess.Send();
        bComplete = true;
        bActive = false;
    }
}

void RainbowScaleObjective::Message(MKMessage* pMessage) {
    switch (pMessage->unk0) {
        case 2:
            GameObjDesc* pOpalDesc = objectManager.FindDescriptor("Opal");
            DescriptorIterator itr = pOpalDesc->Begin();
            while (itr.GetPointers()) {
                unk66++;
                itr.UpdatePointers();
            }
    }
    LevelObjective::Message(pMessage);
}
