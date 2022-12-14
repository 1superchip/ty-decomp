#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "types.h"
#include "common/Heap.h"
//#include "common/KromeIni.h"
#include "common/MKSceneManager.h"

#define GOMSG_Last 0x10
#define In_WaterVolume 0x40000000

struct ModuleInfoBaseObject;

struct ModuleInfoBase {
    ModuleInfoBase() {
		pData = NULL;
    }
    virtual void Init(ModuleInfoBase*) = 0;
    virtual void* ConstructObject(void*) = 0;
	void Reset(void);
	static void AddToModuleList(ModuleInfoBase*);
	
	ModuleInfoBaseObject* pData;
    static ModuleInfoBase* pList;
};

struct GameObject;
struct BeginStruct;

struct GameObjDesc : MKPropDescriptor {
	virtual void Init(ModuleInfoBase*, char*, char*, int, int);
	virtual void Load(KromeIni*);
	virtual void* ConstructObject(void* mem) {
		return pModule->ConstructObject(mem); // weak virtual, placed in Bird.cpp
	};
	u8* SetUpMem(u8*);
	void LoadObjects(KromeIni*, KromeIniLine*);
    GameObject* CreateObject(void);
	
	BeginStruct Begin(void);
	
	char descrName[0x20];
	char modelName[0x20];
	ModuleInfoBase* pModule;
	int unk74;
	GameObject* unk78;
	u8* unk7C;
	GameObjDesc* unk80;
};

struct GameObject : MKProp {
	
	static void InitModule(void);
	static void DeinitModule(void);
	static void UpdateModule(void);
	static void DrawModule(void);
	static int* Allocate(void);
	static void Deallocate(GameObject*);
	uint CalcDetailLevel(void);
	
	static int GetMessageIdFromString(char*);
	
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Draw(void);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
};


struct ModuleInfoBaseObject {
	void (*InitModule)(void);
	void (*DeinitModule)(void);
	void (*pUpdateModule)(void);
	void (*pDrawModule)(void);
	int* (*pAllocate)(void);
	void (*pDeallocate)(GameObject*);
	int unk18; // entity count?
	int instanceSize;
	bool bUpdate;
	int flags;
	ModuleInfoBase* pNext;
};

inline void* operator new(size_t size, void* mem) {
    return (void*)mem;
}

// flags for ModuleInfoBaseObject to define module function overrides
// Figure out better names for these
#define Module_UpdateOverride   1
#define Module_DrawOverride     2
#define Module_AllocateOverride 8

template <typename T>
struct ModuleInfo : ModuleInfoBase {
	
	virtual void Init(ModuleInfoBase*) {
        if (pData == NULL) {
            pData = (ModuleInfoBaseObject*)Heap_MemAlloc(sizeof(ModuleInfoBaseObject));
        }
        Reset();
        pData->flags = 0;
        pData->InitModule = GameObject::InitModule;
        pData->DeinitModule = GameObject::DeinitModule;
        pData->pUpdateModule = GameObject::UpdateModule;
        pData->pDrawModule = GameObject::DrawModule;
        pData->pAllocate = GameObject::Allocate;
        pData->pDeallocate = GameObject::Deallocate;
        pData->instanceSize = sizeof(T);
        if (pData->pUpdateModule != GameObject::UpdateModule) {
            pData->flags |= Module_UpdateOverride;
        }
        if (pData->pDrawModule != GameObject::DrawModule) {
            pData->flags |= Module_DrawOverride;
        }
        if (pData->pAllocate != GameObject::Allocate) {
            pData->flags |= Module_AllocateOverride;
        }
        AddToModuleList(this);
    }
    virtual void* ConstructObject(void* ptr) {
        return new(ptr) T;
    }
};

struct BeginStruct {
    u8* unk0;
    u8* unk4;
    GameObject* GetPointers(void) {
        return (unk0 < unk4) ? (GameObject*)unk0 : NULL;
    }
    void UpdatePointers(void) {
        unk0 += static_cast<GameObjDesc*>(((GameObject *)unk0)->pDescriptor)->pModule->pData->instanceSize;
    }
};

template <typename T>
T descr_cast(MKPropDescriptor* pDesc) {
    return static_cast<T>(pDesc);
}

template <typename T>
void LoadDescriptors(KromeIni* pIni, char* name, T* pDesc) {
    GameObjDesc* desc = NULL;
    KromeIniLine* pLine = pIni->GotoLine(name, NULL);
    GameObjDesc* p = desc;
    while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
        char* pString = NULL;
        if (pLine->pFieldName != NULL && pLine->AsString(0, &pString) != false) {
            p = (GameObjDesc*)Heap_MemAlloc(sizeof(T));
            memset(p, 0, sizeof(T));
            new ((void*)p) T;
            static_cast<T*>(p)->Init(pDesc->pModule, pString, pLine->pFieldName, pDesc->searchMask, pDesc->flags);
            static_cast<GameObjDesc*>(p)->unk80 = desc;
            desc = (GameObjDesc*)p;
        }
        pLine = pIni->GetLineWithLine(pLine);
    }
    while (desc != NULL) {
        GameObjDesc* p = desc;
        desc = p->unk80;
        p->Load(pIni);
        objectManager.AddDescriptor(p);
    }
}

// place these in the proper header

struct NameFlagPair {
    char* name;
    int flag;
};
bool LoadLevel_LoadBool(KromeIniLine*, char*, bool*);
bool LoadLevel_LoadFlags(KromeIniLine*, char*, NameFlagPair*, int, int*);
bool LoadLevel_LoadVector(KromeIniLine*, char*, Vector*);
bool LoadLevel_LoadInt(KromeIniLine*, char*, int*);
bool LoadLevel_LoadFloat(KromeIniLine*, char*, float*);
bool LoadLevel_LoadString(KromeIniLine*, char*, char*, int, int);

enum CommonGameObjFlags {
    GameObjFlags_Active = 1,
    GameObjFlags_Enabled = 2,
    GameObjFlags_Visible = 4,
	GameObjFlags_All = 7
};

struct CommonGameObjFlagsComponent {
    u16 flags;

    void Clear(CommonGameObjFlags objFlags) {
        flags &= ~objFlags;
    }
    void Set(CommonGameObjFlags objFlags) {
        flags |= objFlags;
    }
	void Init(CommonGameObjFlags objFlags) {
		flags = objFlags;
	}
    bool LoadFlag(KromeIniLine* pLine, char* str, CommonGameObjFlags objFlags) {
        bool tmp = false;
        bool levelRet = LoadLevel_LoadBool(pLine, str, &tmp);
        if (levelRet != false) {
            if (tmp) {
                Set(objFlags);
            } else {
                Clear(objFlags);
            }
            return true;
        }
        return false;
    }
    bool LoadLine(KromeIniLine* pLine) {
        return LoadFlag(pLine, "bActive", GameObjFlags_Active) || 
            LoadFlag(pLine, "bEnabled", GameObjFlags_Enabled) ||
            LoadFlag(pLine, "bVisible", GameObjFlags_Visible);
    }
	void Message(MKMessage* pMsg) {
        switch (pMsg->unk0) {
            case 10:
                Set(GameObjFlags_Active);
                break;
            case 11:
                Clear(GameObjFlags_Active);
                break;
            case 14:
                Set(GameObjFlags_Visible);
                break;
            case 15:
                Clear(GameObjFlags_Visible);
                break;
            case 12:
                Set(GameObjFlags_Enabled);
                break;
            case 13:
                Clear(GameObjFlags_Enabled);
                break;
        }
    }
};

#endif // GAMEOBJECT_H