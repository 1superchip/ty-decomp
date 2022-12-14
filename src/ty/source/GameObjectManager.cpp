#include "types.h"
#include "common/Heap.h"
#include "common/Str.h"
#include "ty/GameObjectManager.h"

// from different files
extern MKSceneManager gSceneManager;
int strnicmp(char const*, char const*, int);
extern "C" void memset(void*, int, int);
extern "C" void strcpy(char*, char*);
extern "C" int stricmp(char*, char*);

GameObjectManager objectManager;

void GameObjectManager::Init(void) {
	bLevelInitialised = false;
	pDescs = NULL;
	unk8 = NULL;
	unkC = 0;
}

void GameObjectManager::Deinit(void) {
	return;
}

void GameObjectManager::InitLevel(void) {
    if (bLevelInitialised == false) {
        GameObjDesc* descriptor = pDescs;
        while (descriptor != NULL) {
            descriptor->unk74 = NULL;
            descriptor->unk78 = NULL;
            descriptor->pModule->Reset();
            descriptor = descriptor->unk80;
        }
        unk8 = 0;
        unkC = 0;
        bLevelInitialised = true;
    }
}

void GameObjectManager::DeinitLevel(void) {
    GameObject* pObj;
    if (bLevelInitialised == false) {
        return;
    }
    BeginStruct begin = Begin();
    while (begin.GetPointers()) {
        begin.GetPointers()->Deinit();
        begin.UpdatePointers();
    }

    gSceneManager.Deinit();
    GameObjDesc* pNextDesc = (GameObjDesc*)pDescs;
    while (pNextDesc != NULL) {
        if (pNextDesc->pModule->pData->bUpdate != false) {
            pNextDesc->pModule->pData->DeinitModule();
            pNextDesc->pModule->pData->bUpdate = false;
            pNextDesc->pModule->pData->unk18 = 0;
            pNextDesc->unk74 = NULL;
            pNextDesc->unk7C = NULL;
            pNextDesc->unk78 = NULL;
        }
        pNextDesc = pNextDesc->unk80;
    }
    if (unk8 != NULL) {
        Heap_MemFree(unk8);
    }
    unk8 = NULL;
    unkC = NULL;
    bLevelInitialised = false;
}

// would this function go in KromeIni?
char* RemStaticPrefix(char* str) {
    if (strnicmp(str, "static", 6) == 0) {
        str = Str_Printf(str + 6);
    } else {
        str = Str_Printf(str);
    }
    return str;
}

void GameObjectManager::LoadLevel(KromeIni* pIni) {
    unkC = NULL;
    KromeIniLine* pLine = pIni->GotoLine(NULL, NULL);
    while (pLine != NULL) {
        if (pLine->section != NULL) {
            char* str = RemStaticPrefix(pLine->section);
            GameObjDesc* pDesc = FindDescriptor(str);
            if (pDesc != NULL) {
                int count = CountEntities(pIni, pLine);
                pDesc->unk74 += count;
                pDesc->pModule->pData->unk18 += count;
                if ((pDesc->flags & 0x100000) == false) {
                    unkC += count * pDesc->pModule->pData->instanceSize;
                }
            }
        }
        pLine = pIni->GetLineWithLine(pLine);
    }
    if (unkC != 0) {
        unk8 = Heap_MemAlloc(unkC + 4);
        memset(unk8, 0xCF, unkC);
        strcpy((char*)unk8 + unkC, "end");
    } else {
        unk8 = NULL;
    }
    GameObject* pObj = static_cast<GameObject*>(unk8);
    GameObjDesc* pDesc = static_cast<GameObjDesc*>(pDescs);
    while (pDesc != NULL) {
        if ((pDesc->flags & 0x100000) == false) {
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

void CheckVolume(GameObject* pObj, BoundingVolume* pBV) {
    static BoundingVolume bv = { {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} };
    if (pBV == NULL) {
        pObj->pDescriptor->pVolume = &bv;
    }
}

void GameObjectManager::AddObject(GameObject* pObj, Matrix* pLTW, BoundingVolume* pBV) {
    pObj->pLocalToWorld = pLTW;
    pObj->pDescriptor->pVolume = pBV;
	CheckVolume(pObj, pBV);
    if (pObj->pLocalToWorld == NULL) {
        pObj->pDescriptor->flags = (pObj->pDescriptor->flags & ~0x3) | 2;
    }
    gSceneManager.AddProp(pObj);
}

void GameObjectManager::AddObject(GameObject* pObj, Model* pModel) {
    pObj->pModel = pModel;
    BoundingVolume* pModelVolume = pModel->GetBoundingVolume(-1);
    pObj->pLocalToWorld = &pModel->matrices[0];
    GameObjDesc* pDesc = static_cast<GameObjDesc*>(pObj->pDescriptor);
    pObj->pDescriptor->pVolume = pModelVolume;
	CheckVolume(pObj, pModelVolume);
    if (pObj->pLocalToWorld == NULL) {
        pObj->pDescriptor->flags = (pObj->pDescriptor->flags & ~0x3) | 2;
    }
    gSceneManager.AddProp(pObj);
}

void GameObjectManager::RemoveObject(GameObject* pObj) {
	gSceneManager.RemoveProp(pObj);
}

extern ModuleInfoBase* ModuleInfoBase::pList;

void GameObjectManager::UpdateModules(void) {
    ModuleInfoBase* pInfo = ModuleInfoBase::pList;
    while (pInfo != NULL) {
        if (pInfo->pData->bUpdate != false) {
            pInfo->pData->pUpdateModule();
        }
        pInfo = pInfo->pData->pNext;
    }
}

void GameObjectManager::DrawModules(void) {
    ModuleInfoBase* pInfo = ModuleInfoBase::pList;
    while (pInfo != NULL) {
        if (pInfo->pData->bUpdate != false) {
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
    BeginStruct begin = Begin();
    while (begin.GetPointers()) {
        if (begin.GetPointers()->unk10 == id) {
            return begin.GetPointers();
        }
        begin.UpdatePointers();
    }
    return NULL;
}

void GameObjectManager::AddDescriptor(GameObjDesc* pDesc) {
    pDesc->unk80 = (GameObjDesc*)pDescs;
    pDescs = pDesc;
}

GameObjDesc* GameObjectManager::FindDescriptor(char* name) {
    GameObjDesc* pDesc = (GameObjDesc*)pDescs;
    while (pDesc != NULL) {
        if (stricmp(name, pDesc->descrName) == 0) {
            return pDesc;
        }
        pDesc = pDesc->unk80;
    }
    return NULL;
}

void GameObjectManager::GetObjectsInRange(GameObject** ppObjects, int param_2, Vector* pPt, float radius, int param_5) {
    param_5 = (param_5 != 0) ? param_5 : -1;
    gSceneManager.GetPropsInRange((MKProp**)ppObjects, param_2, pPt, radius, param_5, -1, 0);
}

GameObject* GameObjectManager::GetClosestObjectInRange(Vector* pPt, float radius, int param_2) {
    GameObject* objects[0x400];
    param_2 = (param_2 != 0) ? param_2 : -1;
    int count = gSceneManager.GetPropsInRange((MKProp**)&objects, 0x400, pPt, radius, param_2, -1, false);
    GameObject* pClosestObj = NULL;
    float minRadius = radius * radius * 2.0f;
    for(int i = 0; i < count; i++) {
        Vector *pLTWTrans = objects[i]->pLocalToWorld->Row3();
        if (pPt->DistSq(pLTWTrans) < minRadius) {
            minRadius = pPt->DistSq(pLTWTrans);
            pClosestObj = objects[i];
        }
    }
    return pClosestObj;
}

BeginStruct GameObjectManager::Begin(void) {
    BeginStruct beginRet = {0, 0};
    beginRet.unk0 = (u8*)unk8;
    beginRet.unk4 = (u8*)objectManager.unk8 + (int)objectManager.unkC;
    return beginRet;
}

int GameObjectManager::CountEntities(KromeIni* pIni, KromeIniLine* pIniLine) {
    int count = 0;
    KromeIniLine *pLine = pIni->GetLineWithLine(pIniLine);
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