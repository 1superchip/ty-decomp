#include "ty/props/SpawnPoint.h"
#include "ty/GameObjectManager.h"
#include "ty/GameData.h"

extern struct GlobalVar {
    char unk[0x104];
    GameData gameData;
} gb;

extern struct Ty {
    char padding[0xa58];
    Vector spawnPos;
    Vector spawnRot;
    bool unk844;
    bool unk845;
    bool unk846;
    float unk848;
    float unk84C;
    char unk1[0x34];
    int unk884;
} ty;

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

void SpawnPoint::Message(MKMessage* pMessage) {
    switch (pMessage->unk0) {
        case 1:
            if (fromLevel == gb.gameData.pSaveData->levelAB4) {
                ty.spawnPos = tySpawnPos;
                ty.spawnRot = tySpawnRot;
            }
    }
    GameObject::Message(pMessage);
}