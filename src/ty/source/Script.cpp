#include "ty/Script.h"
#include "ty/GameObjectManager.h"
#include "common/System_GC.h"
#include "common/Str.h"
#include "ty/global.h"

static ModuleInfo<ScriptProp> scriptModule;
static GameObjDesc scriptPropDesc;
static ModuleInfo<ConditionalScriptProp> conditionalScriptModule;
static GameObjDesc conditionalScriptDesc;

void Script_LoadResources(KromeIni* pIni) {
	scriptPropDesc.Init(&scriptModule, "Script", "Script", 8, 0);
	objectManager.AddDescriptor(&scriptPropDesc);
	conditionalScriptDesc.Init(&conditionalScriptModule, "ConditionalScript", "ConditionalScript", 8, 0);
	objectManager.AddDescriptor(&conditionalScriptDesc);
}

bool ScriptProp::LoadLine(KromeIniLine* pLine) {
    for(int i = 0; i < MessageCount; i++) {
        if (messages[i].LoadLine(pLine, Str_Printf("Message%d", i))) {
            return true;
        }
    }
    return LoadLevel_LoadFloat(pLine, "Delay", &Delay) ||
        LoadLevel_LoadBool(pLine, "bDelayEachMessage", &bDelayEachMessage) ||
        LoadLevel_LoadBool(pLine, "bEnabled", &bEnabled) ||
        GameObject::LoadLine(pLine);
}

void ScriptProp::LoadDone(void) {
	GameObject::LoadDone();
	objectManager.AddObject(this, NULL, NULL);
}

void ScriptProp::Init(GameObjDesc* pDesc) {
	GameObject::Init(pDesc);
	Delay = 0.0f;
	unk40 = 0.0f;
	unk44 = false;
	bEnabled = true;
	unk47 = true;
	bDelayEachMessage = false;
	currentMessageIndex = 0;
	for (int i = 0; i < MessageCount; i++) {
		messages[i].Init();
	}
}

void ScriptProp::Update(void) {
	if (unk44) {
		unk40 -= gDisplay.updateFreq;
		if (!(unk40 <= 0.0f)) {
			return;
		}
		if (bDelayEachMessage) {
			messages[currentMessageIndex++].Send();
			if (currentMessageIndex == MessageCount) {
				unk44 = false;
				return;
			}
			unk40 = Delay;
			return;
		}
		Execute();
		unk44 = false;
	}
}

void ScriptProp::Reset(void) {
	unk44 = false;
	bEnabled = unk47;
	unk40 = 0.0f;
	currentMessageIndex = 0;
}

void ScriptProp::Message(MKMessage* pMsg) {
	switch (pMsg->unk0) {
		case 10:
			if (!bEnabled) {
				return;
			}
			if (Delay <= 0.0f) {
				Execute();
				return;
			}
			unk44 = true;
			unk40 = Delay;
			currentMessageIndex = 0;
			return;
		case 11:
			unk44 = false;
			return;
        case 12:
			bEnabled = true;
			break;
		case 13:
			bEnabled = false;
			break;
		case MKMSG_Resolve:
			for (int i = 0; i < MessageCount; i++) {
				messages[i].Resolve();
			}
			break;
	}
	GameObject::Message(pMsg);
}

void ScriptProp::Execute(void) {
	for (int i = 0; i < MessageCount; i++) {
		messages[i].Send();
	}
}

void ConditionalScriptProp::Init(GameObjDesc* pDesc) {
	ScriptProp::Init(pDesc);
	condition = 0;
	bExecuteOnStart = true;
	bNegative = false;
}

bool ConditionalScriptProp::LoadLine(KromeIniLine* pLine) {
	return LoadLevel_LoadInt(pLine, "condition", &condition) ||
		LoadLevel_LoadBool(pLine, "bExecuteOnStart", &bExecuteOnStart) ||
		LoadLevel_LoadBool(pLine, "bNegative", &bNegative) ||
		ScriptProp::LoadLine(pLine);
}

void ConditionalScriptProp::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case 2:
            if (bExecuteOnStart && CheckConditions()) {
                MKMessage msg = {10};
                ScriptProp::Message(&msg);
            }
            break;
        case 10:
            if (CheckConditions()) {
                ScriptProp::Message(pMsg);
            }
            return;
    }
    ScriptProp::Message(pMsg);
}

bool ConditionalScriptProp::CheckConditions(void) {
    bool bConditionMet = false;
    switch (condition) {
        case Conditon_LearntToSwim:
            if (gb.mGameData.CheckLearntToSwim()) {
                bConditionMet = true;
            }
            break;
        case Conditon_HasBothRangs:
            if (gb.mGameData.HasBothRangs()) {
                bConditionMet = true;
            }
            break;
        case 2:
            if (gb.mGameData.CheckCurrentLevelThunderEgg(3)) {
                bConditionMet = true;
            }
            break;
        case 3:
			// Check if the zones have been completed by defeating the zone boss (Conditions 3 - 7)
            if (gb.mGameData.IsZoneCompleted(1)) {
                bConditionMet = true;
            }
            break;
        case 4:
            if (gb.mGameData.IsZoneCompleted(2)) {
                bConditionMet = true;
            }
            break;
        case 5:
            if (gb.mGameData.IsZoneCompleted(3)) {
                bConditionMet = true;
            }
            break;
        case 6:
            if (gb.mGameData.IsZoneCompleted(4)) {
                bConditionMet = true;
            }
            break;
        case 7:
            if (gb.mGameData.IsZoneCompleted(5)) {
                bConditionMet = true;
            }
            break;
        case 8:
            if (gb.mGameData.HasLevelBeenEntered(gb.mGameData.GetCurrentLevel())) {
                bConditionMet = true;
            }
            break;
        case Conditon_GameComplete:
			// Check if the game has been 100% complete (Condition 9)
            if (gb.mGameData.GetGameCompletePercent() == 100) {
                bConditionMet = true;
            }
            break;
        case 10:
            if (gb.mGameData.HasBoomerang((BoomerangType)9)) {
                bConditionMet = true;
            }
            break;
        case 11:
			// check if the zone hasn't been completed but the (boss level?) has been entered (Conditions 11 - 14)
            if (gb.mGameData.GetLevelEnterCount(7) && !gb.mGameData.IsZoneCompleted(1)) {
                bConditionMet = true;
            }
            break;
        case 12:
            if (gb.mGameData.GetLevelEnterCount(0x13) && !gb.mGameData.IsZoneCompleted(2)) {
                bConditionMet = true;
            }
            break;
        case 13:
            if (gb.mGameData.GetLevelEnterCount(15) && !gb.mGameData.IsZoneCompleted(3)) {
                bConditionMet = true;
            }
            break;
        case 14:
            if (gb.mGameData.GetLevelEnterCount(0x17) && !gb.mGameData.IsZoneCompleted(5)) {
                bConditionMet = true;
            }
            break;
        case 15:
            if (gb.mGameData.CheckZone_Unk0(5)) {
                bConditionMet = true;
            }
            break;
        case Conditon_DefaultTrue:
            bConditionMet = true;
			break;
        case 17:
        case 18:
            break;
    }
    if ((bConditionMet && !bNegative) || (!bConditionMet && bNegative)) {
        return true;
    }
    return false;
}
