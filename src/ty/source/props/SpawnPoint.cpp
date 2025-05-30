#include "ty/props/SpawnPoint.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"
#include "ty/Ty.h"

static GameObjDesc spawnDesc;
static ModuleInfo<SpawnPoint> spawnModInfo;
char* spawnname = "spawnpoint";

void SpawnPoint_LoadResources(KromeIni* pIni) {
    spawnDesc.Init(&spawnModInfo, spawnname, spawnname, 8, 2);
    objectManager.AddDescriptor(&spawnDesc);
}

void SpawnPoint::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    fromLevel = 0; // default fromLevel to 0
    tySpawnPos.SetZero(); // default spawn position to 0, 0, 0
    tySpawnRot.SetZero(); // default spawn rotation to 0, 0, 0
}

bool SpawnPoint::LoadLine(KromeIniLine* pLine) {
    return GameObject::LoadLine(pLine) || LoadLevel_LoadVector(pLine, "pos", &tySpawnPos) ||
        LoadLevel_LoadVector(pLine, "rot", &tySpawnRot) ||
        LoadLevel_LoadInt(pLine, "fromLevel", &fromLevel);
}

void SpawnPoint::LoadDone(void) {
	GameObject::LoadDone();
	objectManager.AddObject(this, NULL, NULL);
}

void SpawnPoint::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Resolve:
            if (fromLevel == gb.mGameData.pSaveData->previousLevel) {
                ty.mSpawnPos = tySpawnPos;
                ty.mSpawnRot = tySpawnRot;
            }
            break;
    }
    GameObject::Message(pMsg);
}
