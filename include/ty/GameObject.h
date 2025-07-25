#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "types.h"
#include "common/Heap.h"
//#include "common/KromeIni.h"
#include "common/MKSceneManager.h"
#include "ty/tools.h"
#include "ty/tytypes.h"
#include "ty/MessageIds.h"

#define GOMSG_Last 10
#define In_WaterVolume 0x40000000

// Search Mask Flags
#define GOID_Platform 0x40

struct GameObject;

struct MKMessage_GOBJ : MKMessage {
	GameObject* pObj;
};

// Ty 2 has a Message named "PlatformMoveMsg"
// Might be called "Message_PlatformMove" based on Jimmy Neutron?
// might be good to move this to Platform.h?
struct PlatformMoveMsg : MKMessage {
    Vector* trans;
    Vector* rot;
    Vector* vec;
    Matrix* mat;
};

struct ModuleInfoBase;

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

struct ModuleInfoBase {
    ModuleInfoBase() {
        pData = NULL;
    }

    virtual void Init(void) = 0;
    virtual void* ConstructObject(void*) = 0;
    void Reset(void);
    static void AddToModuleList(ModuleInfoBase*);

    int GetInstanceSize(void) {
        return pData->instanceSize;
    }

    ModuleInfoBaseObject* pData;
    static ModuleInfoBase* pList;
};

struct DescriptorIterator;

enum GameObjDescFlags {
    GODF_DYNAMIC                = 1 << 0, // 0x1
    MODULE_ALLOCATION_OVERRIDE  = 1 << 20, // 0x100000
};

struct GameObjDesc : MKPropDescriptor {
    virtual void Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags);
    
    virtual void Load(KromeIni* pIni);

    virtual void* ConstructObject(void* mem) {
        return pModule->ConstructObject(mem); // weak virtual, placed in Bird.cpp
    }

    u8* SetUpMem(u8* pMem);
    void LoadObjects(KromeIni* pIni, KromeIniLine* pLine);
    GameObject* CreateObject(void);
    DescriptorIterator Begin(void);

    char descrName[0x20];
    char modelName[0x20];
    ModuleInfoBase* pModule;
    int instanceCount;
    GameObject* pInstances; // Memory allocated to all Objects of this Descriptor
    u8* pCurrInst;
    GameObjDesc* unk80;

    bool TestFlag(GameObjDescFlags testFlags) {
        return testFlags & flags;
    }
};

struct GameObject : MKProp {
    static void InitModule(void);
    static void DeinitModule(void);
    static void UpdateModule(void);
    static void DrawModule(void);
    static int* Allocate(void);
    static void Deallocate(GameObject* pObj);
    uint CalcDetailLevel(void);
    static int GetMessageIdFromString(char* string);

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);

    bool IsInWater(void) {
        return flags & In_WaterVolume;
    }

    Vector* GetPos(void) {
        return pLocalToWorld->Row3();
    }
};

inline void* operator new(size_t size, void* mem) {
    return (void*)mem;
}

// flags for ModuleInfoBaseObject to define module function overrides
// Figure out better names for these
#define Module_UpdateOverride   1
#define Module_DrawOverride     2
#define Module_AllocateOverride 8

/// @brief Templated ModuleInfoBase class which contains information about an object
/// @tparam T Object type
template <typename T>
struct ModuleInfo : ModuleInfoBase {
    virtual void Init(void) {
        if (pData == NULL) {
            pData = (ModuleInfoBaseObject*)Heap_MemAlloc(sizeof(ModuleInfoBaseObject));
        }

        Reset();

        pData->flags = 0;

        pData->InitModule = T::InitModule;
        pData->DeinitModule = T::DeinitModule;
        pData->pUpdateModule = T::UpdateModule;
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

/// @brief Iterator class to iterate over objects from a GameObjDesc
struct DescriptorIterator {
    u8* pCurr; // Current GameObject pointer
    u8* pEnd;

    /// @brief Returns a pointer to a GameObject (May return NULL)!
    /// @param None
    /// @return GameObject pointer or NULL
    GameObject* operator*(void) {
        if (pCurr < pEnd) {
            return (GameObject*)pCurr;
        } else {
            return NULL;
        }
    }

    /// @brief Updates the current memory pointer to move to the next GameObject
    void operator++(int) {
        pCurr += static_cast<GameObjDesc*>(((GameObject *)pCurr)->pDescriptor)->pModule->pData->instanceSize;
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

#endif // GAMEOBJECT_H
