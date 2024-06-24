#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include "common/MKSceneManager.h"
#include "ty/GameObject.h"

struct GameObjectManager {
    GameObjDesc* pDescs;
    bool bLevelInitialised;
    void* pObjectMem;
    int objectMemSize;
    
    void Init(void);
    void Deinit(void);
    void InitLevel(void);
    void DeinitLevel(void);
    void LoadLevel(KromeIni*);
    void AddObject(GameObject*, Matrix*, BoundingVolume*);
    void AddObject(GameObject*, Model*);
    void RemoveObject(GameObject*);
    void UpdateModules(void);
    void DrawModules(void);
    void SendMessage(MKMessage*, uint, Vector*, float, bool);
    void SendMessageToAll(MKMessage* pMsg, int mask);
    GameObject* GetObjectFromID(uint id);
    void AddDescriptor(GameObjDesc*);
    GameObjDesc* FindDescriptor(char*);
    int GetObjectsInRange(GameObject** pObjects, int, Vector* pPt, float radius, int);
    GameObject* GetClosestObjectInRange(Vector* pPt, float radius, int);
    DescriptorIterator Begin(void);
    int CountEntities(KromeIni*, KromeIniLine*);
};

extern GameObjectManager objectManager;

#endif // GAMEOBJECTMANAGER_H
