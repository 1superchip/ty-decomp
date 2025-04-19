#include "ty/props/RangStone.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"

static ModuleInfo<RangStone> RangStoneModuleInfo;
static StaticPropDescriptor RangStoneDesc;

static char* szRangObj = "A_rang";

void RangStone_LoadResources(KromeIni* pIni) {
    RangStoneDesc.Init(&RangStoneModuleInfo, "prop_0531_RangStone", "RangStone", 1, 0);
    RangStoneDesc.Load(pIni);
    objectManager.AddDescriptor(&RangStoneDesc);
}

void RangStone::Init(GameObjDesc* pDesc) {
    collide = true;
    collisionInfo.Enable();
    StaticProp::Init(pDesc);
    lodManager.Init(pModel, 0, &GetDesc()->lodDesc);
    OnSuccess.Init();
    rangHidden = 0;
}

bool RangStone::LoadLine(KromeIniLine* pLine) {
    return OnSuccess.LoadLine(pLine, "OnSuccess") || StaticProp::LoadLine(pLine);
}

void RangStone::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Resolve:
            OnSuccess.Resolve();
            break;
        case MSG_UNK_2:
            if (gb.mGameData.HasBothRangs()) {
                HideRang();
            }
            break;
        case MSG_GotBothRangs:
            if (rangHidden == 0) {
                HideRang();
                OnSuccess.Send();
            }
            break;
    }

    GameObject::Message(pMsg);
}

void RangStone::HideRang(void) {
    int n; // subobject index
    
    if (pModel->SubObjectExists(szRangObj, &n)) {
        pModel->EnableSubObject(n, false);
    }

    rangHidden = 1;
}
