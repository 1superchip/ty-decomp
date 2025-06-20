#include "types.h"
#include "common/Heap.h"
#include "common/Str.h"
#include "ty/GameObjectManager.h"

// from different files
int strnicmp(char const*, char const*, int);
extern "C" void memset(void*, int, int);
extern "C" void strcpy(char*, char*);
extern "C" int stricmp(char*, char*);

GameObjectManager objectManager;

void GameObjectManager::Init(void) {
    bLevelInitialised = false;
    pDescs = NULL;
    pObjectMem = NULL;
    objectMemSize = 0;
}

void GameObjectManager::Deinit(void) {
    return;
}

void GameObjectManager::InitLevel(void) {
    if (bLevelInitialised == false) {
        GameObjDesc* descriptor = pDescs;
        while (descriptor != NULL) {
            descriptor->instanceCount = 0;
            descriptor->pInstances = NULL;
            descriptor->pModule->Reset();
            descriptor = descriptor->unk80;
        }
        
        pObjectMem = NULL;
        objectMemSize = 0;
        bLevelInitialised = true;
    }
}

void GameObjectManager::DeinitLevel(void) {
    GameObject* pObj;

    if (bLevelInitialised == false) {
        return;
    }

    DescriptorIterator it = Begin();
    while (*it) {
        (*it)->Deinit();

        it++;
    }

    gSceneManager.Deinit();

    GameObjDesc* pNextDesc = (GameObjDesc*)pDescs;
    while (pNextDesc != NULL) {
        if (pNextDesc->pModule->pData->bUpdate != false) {
            pNextDesc->pModule->pData->DeinitModule();
            pNextDesc->pModule->pData->bUpdate = false;
            pNextDesc->pModule->pData->unk18 = 0;
            pNextDesc->instanceCount = 0;
            pNextDesc->pCurrInst = NULL;
            pNextDesc->pInstances = NULL;
        }

        pNextDesc = pNextDesc->unk80;
    }

    if (pObjectMem != NULL) {
        Heap_MemFree(pObjectMem);
    }
    
    pObjectMem = NULL;
    objectMemSize = 0;
    bLevelInitialised = false;
}

char* RemStaticPrefix(char* str) {
    if (strnicmp(str, "static", sizeof("static") - 1) == 0) {
        return Str_Printf(str + sizeof("static") - 1);
    }

    return Str_Printf(str);
}

void GameObjectManager::LoadLevel(KromeIni* pIni) {
    objectMemSize = 0;

    KromeIniLine* pLine = pIni->GotoLine(NULL, NULL);
    while (pLine != NULL) {

        if (pLine->section != NULL) {
            char* str = RemStaticPrefix(pLine->section); // remove "static" from string
            GameObjDesc* pDesc = FindDescriptor(str);
            if (pDesc != NULL) {
                int count = CountEntities(pIni, pLine);
                pDesc->instanceCount += count;
                pDesc->pModule->pData->unk18 += count;
                if (!pDesc->TestFlag(MODULE_ALLOCATION_OVERRIDE)) {
                    objectMemSize += count * pDesc->pModule->pData->instanceSize;
                }
            }
        }

        pLine = pIni->GetLineWithLine(pLine);
    }

    if (objectMemSize != 0) {
        pObjectMem = Heap_MemAlloc(objectMemSize + 4); // + strlen("end")
        memset(pObjectMem, 0xCF, objectMemSize);
        strcpy((char*)pObjectMem + objectMemSize, "end");
    } else {
        pObjectMem = NULL;
    }

    GameObject* pObj = static_cast<GameObject*>(pObjectMem);
    GameObjDesc* pDesc = pDescs;
    while (pDesc != NULL) {
        if (!pDesc->TestFlag(MODULE_ALLOCATION_OVERRIDE)) {
            pObj = (GameObject*)pDesc->SetUpMem((u8*)pObj);
        }

        pDesc = pDesc->unk80;
    }

    pLine = pIni->GotoLine(NULL, NULL);
    while (pLine != NULL) {
        if (pLine->section != NULL) {
            char* str = RemStaticPrefix(pLine->section);
            GameObjDesc* pDesc = FindDescriptor(str);
            if (pDesc != NULL) {
                pDesc->LoadObjects(pIni, pLine);
            }
        }
        
        pLine = pIni->GetLineWithLine(pLine);
    }
}

void GameObjectManager::AddObject(GameObject* pObj, Matrix* pLTW, BoundingVolume* pBV) {
    pObj->pLocalToWorld = pLTW;
    pObj->pDescriptor->pVolume = pBV;

    static BoundingVolume bv = {
        {0.0f, 0.0f, 0.0f, 0.0f}, 
        {0.0f, 0.0f, 0.0f, 0.0f}
    };

    if (pBV == NULL) {
        pObj->pDescriptor->pVolume = &bv;
    }

    if (pObj->pLocalToWorld == NULL) {
        pObj->pDescriptor->flags = (pObj->pDescriptor->flags & ~MKPROP_TypeMask) | MKPROP_Global;
    }
    
    gSceneManager.AddProp(pObj);
}

void GameObjectManager::AddObject(GameObject* pObj, Model* pModel) {
    pObj->pModel = pModel;
    AddObject(pObj, &pModel->matrices[0], pModel->GetModelVolume());
}

void GameObjectManager::RemoveObject(GameObject* pObj) {
    gSceneManager.RemoveProp(pObj);
}

void GameObjectManager::UpdateModules(void) {
    ModuleInfoBase* pInfo = ModuleInfoBase::pList;
    while (pInfo) {
        if (pInfo->pData->bUpdate) {
            pInfo->pData->pUpdateModule();
        }
        pInfo = pInfo->pData->pNext;
    }
}

void GameObjectManager::DrawModules(void) {
    ModuleInfoBase* pInfo = ModuleInfoBase::pList;
    while (pInfo) {
        if (pInfo->pData->bUpdate) {
            pInfo->pData->pDrawModule();
        }
        pInfo = pInfo->pData->pNext;
    }
}

void GameObjectManager::SendMessage(MKMessage* pMsg, uint mask, Vector* pPt, float radius, bool param_6) {
    gSceneManager.SendMessage(pMsg, mask, param_6, pPt, radius);
}

void GameObjectManager::SendMessageToAll(MKMessage* pMsg, int mask) {
    gSceneManager.SendMessage(pMsg, mask, true, NULL, 0.0f);
}

GameObject* GameObjectManager::GetObjectFromID(uint id) {
    DescriptorIterator it = Begin();
    
    while (*it) {
        if ((*it)->uniqueID == id) {
            return *it;
        }

        it++;
    }

    return NULL;
}

void GameObjectManager::AddDescriptor(GameObjDesc* pDesc) {
    pDesc->unk80 = pDescs;
    pDescs = pDesc;
}

GameObjDesc* GameObjectManager::FindDescriptor(char* name) {
    GameObjDesc* pDesc = pDescs;
    while (pDesc != NULL) {
        if (stricmp(name, pDesc->descrName) == 0) {
            return pDesc;
        }
        
        pDesc = pDesc->unk80;
    }
    
    return NULL;
}

// Returns the number of objects in range and places the GameObject pointers in ppObjects
int GameObjectManager::GetObjectsInRange(GameObject** ppObjects, int maxCount, Vector* pPt, float radius, int searchMask) {
    searchMask = (searchMask != 0) ? searchMask : -1;
    return gSceneManager.GetPropsInRange((MKProp**)ppObjects, maxCount, pPt, radius, searchMask, -1, false);
}

#define MAX_SEARCH_GOBJS (0x400)

// Returns the closest object in range
GameObject* GameObjectManager::GetClosestObjectInRange(Vector* pPt, float radius, int searchMask) {
    GameObject* objects[MAX_SEARCH_GOBJS];

    int count = GetObjectsInRange(objects, MAX_SEARCH_GOBJS, pPt, radius, searchMask); // Total GOBJS = 0x400

    GameObject* pClosestObj = NULL;

    float minRadius = Sqr<float>(radius) * 2.0f;

    // Loop over all GameObjects that were found
    for (int i = 0; i < count; i++) {
        Vector v;
        v.Sub(pPt, objects[i]->GetPos());

        float distSq = v.MagSquared();

        if (distSq < minRadius) {
            minRadius = distSq;
            pClosestObj = objects[i];
        }
    }

    return pClosestObj;
}

DescriptorIterator GameObjectManager::Begin(void) {
    DescriptorIterator git = {(u8*)pObjectMem, (u8*)objectManager.pObjectMem + objectManager.objectMemSize};
    return git;
}

int GameObjectManager::CountEntities(KromeIni* pIni, KromeIniLine* pIniLine) {
    int count = 0;
    KromeIniLine* pLine = pIni->GetLineWithLine(pIniLine);
    // iterate until line is NULL or section is not NULL
    // a new section means a new object type in this case
    while (pLine != NULL && pLine->section == NULL) {
        while (pLine->comment == NULL && pLine->pFieldName == NULL) {
            pLine = pIni->GetLineWithLine(pLine);
        }

        count++;
        
        while (pLine != NULL && (pLine->pFieldName != NULL || pLine->comment != NULL)) {
            pLine = pIni->GetLineWithLine(pLine);
        }

        while (pLine != NULL && pLine->pFieldName == NULL && pLine->section == NULL) {
            pLine = pIni->GetLineWithLine(pLine);
        }
    }

    return count;
}
