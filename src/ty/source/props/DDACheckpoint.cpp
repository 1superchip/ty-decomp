#include "types.h"
#include "ty/props/DDACheckpoint.h"
#include "ty/DDA.h"

extern GameObjectManager objectManager;
extern DDASession dda;
bool LoadLevel_LoadInt(KromeIniLine*, char*, int*);

static GameObjDesc ddaCheckpointDesc;
static ModuleInfo<DDACheckpoint> ddaCheckpointModule;

void DDACheckpoint_LoadResources(KromeIni* pIni) {
	ddaCheckpointDesc.Init(&ddaCheckpointModule, "DDACheckpoint", "DDACheckpoint", 0, 0);
	objectManager.AddDescriptor(&ddaCheckpointDesc);
}

void DDACheckpoint::Init(GameObjDesc* pDesc) {
	GameObject::Init(pDesc);
	Number = -1;
}

bool DDACheckpoint::LoadLine(KromeIniLine* pLine) {
	bool ret = false;
	if (LoadLevel_LoadInt(pLine, "Number", &Number) || GameObject::LoadLine(pLine)) {
		ret = true;
	}
	return ret;
}

void DDACheckpoint::Message(MKMessage* pMsg) {
	switch (pMsg->unk0) {
        case 10:
            dda.NewCheckpoint(Number);
            break;
        default:
            GameObject::Message(pMsg);
    }
}