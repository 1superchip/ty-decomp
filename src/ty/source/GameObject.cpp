#include "types.h"
#include "ty/GameObject.h"
#include "ty/MessageMap.h"
#include "ty/props/WaterVolume.h"

extern "C" void memset(void*, int, int);
extern "C" void strncpy(char*, char*, int);
extern "C" int stricmp(char*, char*);

extern bool gAssertBool; // from Str.cpp

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
    unk10 = 0; // ID?
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
	return;
}

void GameObject::Update(void) {
	return;
}

void GameObject::Draw(void) {
	if (pModel != NULL) {
		pModel->Draw(NULL);
	}
	return;
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
        case 2:
            if (pLocalToWorld != NULL) {
                Vector vec = {0.0f, 0.0f, 0.0f, 1.0f};
                if (pDescriptor->pVolume != NULL) {
                    vec.x = pDescriptor->pVolume->v1.x + 0.5f * pDescriptor->pVolume->v2.x;
                    vec.y = pDescriptor->pVolume->v1.y + 0.5f * pDescriptor->pVolume->v2.y;
                    vec.z = pDescriptor->pVolume->v1.z + 0.5f * pDescriptor->pVolume->v2.z;
                }
                vec.ApplyMatrix(&vec, pLocalToWorld);
                if (WaterVolume_IsWithin(&vec, NULL) != false) {
                    flags |= In_WaterVolume;
                }
            }
            break;
    }
}

bool GameObject::LoadLine(KromeIniLine* pLine) {
	return LoadLevel_LoadInt(pLine, "ID", &unk10);
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

// maybe a ternary would match this as well as DrawDynamicProps?
uint GameObject::CalcDetailLevel(void) {
    int _detail = 8;
    int detail = (int)(8.0f * (1.0f - (1.0f - distSquared / (pDescriptor->maxDrawDist * pDescriptor->maxDrawDist)) * (1.0f - distSquared / (pDescriptor->maxDrawDist * pDescriptor->maxDrawDist))));
    if (detail <= 8) {
        _detail = detail;
    }
    detailLevel = (char)_detail;
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
        flags |= 0x100000;
    }
    flags |= 0xC;
    maxUpdateDist = 6000.0f;
    maxDrawDist = 6000.0f;
    maxScissorDist = 1000.0f;
    drawLayer = 1;
    strncpy(descrName, pDescrName, 0x20);
    Tools_StripExtension(modelName, pMdlName);
    pName = descrName;
    pNext = NULL;
}

u8* GameObjDesc::SetUpMem(u8* pMem) {
    if (0 < unk74) {
        unk78 = (GameObject*)pMem;
        memset((void*)unk78, unk74, unk74 * pModule->pData->instanceSize);
        unk7C = (u8*)unk78;
        pMem += unk74 * pModule->pData->instanceSize;
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
    if (flags & 0x100000) {
		// if the descriptor's module has a custom allocation function use it
        return (GameObject*)ConstructObject(pModule->pData->pAllocate());
    }
    void* mem = unk7C;
    unk7C = unk7C + pModule->pData->instanceSize;
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

BeginStruct GameObjDesc::Begin(void) {
    BeginStruct stack = {(u8*)unk78, (u8*)unk78 + (pModule->pData->instanceSize * unk74)};
    return stack;
}