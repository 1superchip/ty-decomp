#include "types.h"
//#include "common/KromeIni.h"
#include "common/MKSceneManager.h"

#define GOMSG_Last 0x10
#define In_WaterVolume 0x40000000

struct ModuleInfoBase;

struct ModuleInfoBaseObject {
	void (*InitModule)(void);
	void (*DeinitModule)(void);
	void (*pUpdateModule)(void);
	void (*pDrawModule)(void);
	int* (*pAllocate)(void);
	void (*pDeallocate)(void);
	int unk18;
	int instanceSize;
	bool bUpdate;
	int flags;
	ModuleInfoBase* pNext;
};

struct ModuleInfoBase {
	// has an empty vtable except for RTTI, probably due to inheritance
    // is this correct?
    virtual void Init(ModuleInfoBase*) = 0;
    virtual void* ConstructObject(void*) = 0;
	void Reset(void);
	void AddToModuleList(ModuleInfoBase*);
	
	ModuleInfoBaseObject* pData;

    static ModuleInfoBase* pList;
};

// not used in this file
template <typename T>
struct ModuleInfo {
	virtual void Init(ModuleInfoBase*);
    virtual void* ConstructObject(void*);
};

struct GameObject;

// fix this???
struct BeginStruct {
    u8* unk0;
    u8* unk4;
    GameObject* GetPointers(void) {
        GameObject* ret;
        if (unk0 < unk4) {
            ret = (GameObject*)unk0;
        } else {
            ret = NULL;
        }
        return ret;
    }
};

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
	
	// all of these except for CalcDetailLevel might be static
	void InitModule(void);
	void DeinitModule(void);
	void UpdateModule(void);
	void DrawModule(void);
	int Allocate(void);
	void Deallocate(GameObject*);
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