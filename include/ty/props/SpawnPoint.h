#ifndef SPAWNPOINT_H
#define SPAWNPOINT_H

#include "ty/GameObject.h"

void SpawnPoint_LoadResources(KromeIni* pIni);

// Object to hold information about where ty spawns in a level such as position and rotation
struct SpawnPoint : GameObject {
    int fromLevel; // default = 0
    Vector tySpawnPos; // default = 0, 0, 0
    Vector tySpawnRot; // default = 0, 0, 0

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);
};

#endif // SPAWNPOINT_H
