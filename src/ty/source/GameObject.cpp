#include "types.h"
#include "ty/GameObject.h"
#include "ty/MessageMap.h"
#include "ty/props/WaterVolume.h"
#include "common/Str.h"

extern "C" void memset(void*, int, int);
extern "C" void strncpy(char*, char*, int);
extern "C" int stricmp(char*, char*);

ModuleInfoBase* ModuleInfoBase::pList;

void ModuleInfoBase::Reset(void) {
    pData->unk18 = 0;
    pData->bUpdate = false;
}

void ModuleInfoBase::AddToModuleList(ModuleInfoBase* pModule) {
    if ((pModule->pData->flags & 3) == 0) {
        return;
    }

    ModuleInfoBase* list = ModuleInfoBase::pList;
    ModuleInfoBase* list1 = list;

    while (list != NULL) {
        if (list == pModule) {
            return;
        }

        list = list->pData->pNext;
    }
    
    pModule->pData->pNext = list1;
    ModuleInfoBase::pList = pModule;
}

void GameObject::Init(GameObjDesc* pDesc) {
    pDescriptor = pDesc;
    pModel = NULL;
    pNext = NULL;
    pPrev = NULL;
    pNextOfThisType = NULL;
    uniqueID = 0;
    unk14 = 0.0f;
    unk1C = 1.0f;
    detailLevel = 0;
    distSquared = 0.0f;
    pLocalToWorld = NULL;
    pNextUpdated = NULL;
    pPrevUpdated = NULL;
    rejectionResult = 0;
    flags = 0;
}

void GameObject::Deinit(void) {
    if (pModel != NULL) {
        pModel->Destroy();
        pModel = NULL;
    }
}

void GameObject::Update(void) {
    return;
}

void GameObject::Draw(void) {
    if (pModel != NULL) {
        pModel->Draw(NULL);
    }
}

void GameObject::Reset(void) {
    return;
}

void GameObject::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MKMSG_Reset:
            Reset();
            break;
        case MKMSG_Deinit:
            Deinit();
            break;
        case MSG_UNK_2:
            if (pLocalToWorld != NULL) {
                Vector centre = {0.0f, 0.0f, 0.0f, 1.0f};
                if (pDescriptor->pVolume != NULL) {
                    pDescriptor->pVolume->GetCentre(&centre);
                }
                centre.ApplyMatrix(pLocalToWorld);
                if (WaterVolume_IsWithin(&centre, NULL) != false) {
                    flags |= In_WaterVolume;
                }
            }
            break;
    }
}

bool GameObject::LoadLine(KromeIniLine* pLine) {
    return LoadLevel_LoadInt(pLine, "ID", &uniqueID);
}

void GameObject::LoadDone(void) {
    return;
}

void GameObject::InitModule(void) {
    return;
}

void GameObject::DeinitModule(void) {
    return;
}

void GameObject::UpdateModule(void) {
    return;
}

void GameObject::DrawModule(void) {
    return;
}

int* GameObject::Allocate(void) {
    return NULL;
}

void GameObject::Deallocate(GameObject* pObj) {
    return;
}

uint GameObject::CalcDetailLevel(void) {
    
    float distanceRatio;
    float scaledLevel;
    
    distanceRatio = distSquared / Sqr<float>(pDescriptor->maxDrawDist);

    scaledLevel = (1.0f - Sqr<float>(1.0f - distanceRatio)) * 8.0f;
    
    detailLevel = (char)Min<int>(8, scaledLevel);

    return detailLevel;
}

// these structs are from Messages.cpp
extern MessageMap mkMessageMap;
extern MessageMap globalMessageMap;

int GameObject::GetMessageIdFromString(char* string) {
    int result = mkMessageMap.GetIdFromString(string);
    
    if (result == 0) {
        result = globalMessageMap.GetIdFromString(string);
        if (stricmp(string, "Reset") == 0) {
            return -1;
        }
    }

    return result;
}

void GameObjDesc::Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags) {
    pMod->Init();

    unk80 = NULL;
    flags = _flags;
    searchMask = _searchMask;
    pModule = pMod;

    if (pMod->pData->flags & Module_AllocateOverride) {
        // if the module has a custom Allocation function, set the flags in the object descriptor
        flags |= MODULE_ALLOCATION_OVERRIDE;
    }

    flags |= 0x8 | 0x4; // (1 << 3) || (1 << 2)

    maxUpdateDist = 6000.0f;
    maxDrawDist = 6000.0f;
    maxScissorDist = 1000.0f;

    drawLayer = 1;

    strncpy(descrName, pDescrName, sizeof(descrName));

    Tools_StripExtension(modelName, pMdlName);

    pName = descrName;

    pProps = NULL;
}

u8* GameObjDesc::SetUpMem(u8* pMem) {
    if (instanceCount > 0) {
        pInstances = (GameObject*)pMem;
        memset((void*)pInstances, instanceCount, instanceCount * pModule->pData->instanceSize);
        pCurrInst = (u8*)pInstances;
        pMem += instanceCount * pModule->pData->instanceSize;
    }
    
    return pMem;
}

void GameObjDesc::LoadObjects(KromeIni* pIni, KromeIniLine* pLine) {
    GameObject* pObj;

    if (pModule->pData->bUpdate == false) {
        pModule->pData->InitModule();
        pModule->pData->bUpdate = true;
    }

    pLine = pIni->GetLineWithLine(pLine);
    while (pLine != NULL && pLine->section == NULL) {
        while (pLine->comment == NULL && pLine->pFieldName == NULL) {
            pLine = pIni->GetLineWithLine(pLine);
        }

        pObj = CreateObject();
        pObj->Init(this);

        while (pLine != NULL && (pLine->pFieldName || pLine->comment)) {
            if (pLine->pFieldName != NULL) {
                pObj->LoadLine(pLine);
            }

            pLine = pIni->GetLineWithLine(pLine);
        }

        pObj->LoadDone();

        while (pLine != NULL && pLine->pFieldName == NULL && pLine->section == NULL) {
            pLine = pIni->GetLineWithLine(pLine);
        }
        
    }
}

GameObject* GameObjDesc::CreateObject(void) {
    if (TestFlag(MODULE_ALLOCATION_OVERRIDE)) {
        // if the descriptor's module has a custom allocation function use it
        return (GameObject*)ConstructObject(pModule->pData->pAllocate());
    }
    
    void* mem = pCurrInst;
    pCurrInst = pCurrInst + pModule->pData->instanceSize;
    return (GameObject*)ConstructObject(mem);
}

void GameObjDesc::Load(KromeIni* pIni) {
    KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);
    while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
        if (pLine->pFieldName != NULL) {
            gAssertBool = LoadLevel_LoadInt(pLine, "drawLayer", &drawLayer) ||
                LoadLevel_LoadFloat(pLine, "maxDrawDist", &maxDrawDist) ||
                LoadLevel_LoadFloat(pLine, "maxScissorDist", &maxScissorDist) ||
                LoadLevel_LoadFloat(pLine, "maxUpdateDist", &maxUpdateDist);
        }
        pLine = pIni->GetLineWithLine(pLine);
    }
}

DescriptorIterator GameObjDesc::Begin(void) {
    DescriptorIterator it = {(u8*)pInstances, (u8*)pInstances + (pModule->pData->instanceSize * instanceCount)};
    return it;
}
