#include "types.h"
#include "common/Heap.h"
//#include "common/KromeIni.h"
#include "common/MKSceneManager.h"

#define GOMSG_Last 0x10
#define In_WaterVolume 0x40000000

struct ModuleInfoBaseObject;

struct ModuleInfoBase {
    ModuleInfoBase()  {
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
            pData->flags |= 1;
        }
        if (pData->pDrawModule != GameObject::DrawModule) {
            pData->flags |= 2;
        }
        if (pData->pAllocate != GameObject::Allocate) {
            pData->flags |= 8;
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