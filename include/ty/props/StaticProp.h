#include "ty/GameObject.h"
#include "ty/RangeCheck.h"
#include "common/Vector.h"

extern struct DisplayStruct {
    float unk[16];
} gDisplay;

/*struct NameFlagPair {
    char* name;
    int flag;
};
bool LoadLevel_LoadBool(KromeIniLine*, char*, bool*);
bool LoadLevel_LoadFlags(KromeIniLine*, char*, NameFlagPair*, int, int*);
bool LoadLevel_LoadVector(KromeIniLine*, char*, Vector*);
bool LoadLevel_LoadString(KromeIniLine*, char*, char*, int, int);*/

#define FX_Shake 1
#define FX_WaterRipple 2
#define FX_SpawnLeaf 4
#define FX_Dynamic 8
#define FX_Rotate 16

struct CollisionInfo {
	bool bEnabled;
	int flags;
	MKProp* pProp;
};

struct StaticPropDescriptor : GameObjDesc {
	char subObjectName[0x20]; // subobject used for collision
	bool bDynamic;
	bool bUseGroundColor;
	LODDescriptor lodDesc;
	int collisionInfoFlags;
	
	virtual void Init(ModuleInfoBase*, char*, char*, int, int);
	virtual void Load(KromeIni*);
};

struct StaticProp : GameObject {
	bool collide;
	CollisionInfo collisionInfo;
	LODManager lodManager;
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Draw(void);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
	StaticPropDescriptor* GetDesc(void) {
		return descr_cast<StaticPropDescriptor*>(pDescriptor);
	}
    Vector* GetPos(void) {
        return pModel->matrices[0].Row3();
    }

    static Vector loadInfo[2];
};

struct StaticFXPropDesc : StaticPropDescriptor {
	int effectFlags;
	Vector autoRotate;
	char rotateSubObj[0x20];
	// might be wrong
	virtual void Init(ModuleInfoBase* pMod, char* c, char* y, int t, int s) {
        StaticPropDescriptor::Init(pMod, c, y, t, s);
        effectFlags = 0;
        autoRotate.SetZero();
        rotateSubObj[0] = '\0';
    }
	virtual void Load(KromeIni *pIni) {
		StaticPropDescriptor::Load(pIni);
		KromeIniLine *pLine = pIni->GotoLine(modelName, NULL);
		if (pLine != NULL) {
			while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
				if (pLine->pFieldName != NULL) {
					// this matches rodata ordering at the beginning
					const Vector unused_vectors[5] = { {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, 
						{0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} };
					NameFlagPair flagsTmp[5] = {{"shake", FX_Shake}, {"waterRipple", FX_WaterRipple}, {"spawnLeaf", FX_SpawnLeaf},
					{"dynamic", FX_Dynamic}, {"rotate", FX_Rotate}};
					if (LoadLevel_LoadFlags(pLine, "effectFlags", flagsTmp, 5, &effectFlags) == false) {
						bool foundLine = LoadLevel_LoadVector(pLine, "autoRotate", &autoRotate);
						if (foundLine != false) {
							float speed = gDisplay.unk[3];
							autoRotate.x *= speed;
							autoRotate.y *= speed;
							autoRotate.z *= speed;
						} else {
							LoadLevel_LoadString(pLine, "rotateSubObj", rotateSubObj, sizeof(rotateSubObj), 0);
						}
					}
				}
				pLine = pIni->GetLineWithLine(pLine);
			}
		}
	}
	
};

struct StaticFXProp : StaticProp {
	Vector unk58;
	Vector rot; // default prop rotation
	// autoRotation is the rotation that is constantly updated by the description's autoRotate parameter
	// this is added to the prop rotation (rot) on stack and rotates the default matrix in the prop's model
	Vector autoRotation; // autoRotate is added to this vector
	Vector waterCollisionPos; // position of water that was collided with
	bool bCollidesWithWater : 1;
	u8 b1 : 1;
	bool bVisible : 1;
	bool bDefaultVisible : 1;
	u8 b4 : 1;
	u8 b5 : 1;
	u8 b6 : 1;
	u8 b7 : 1;
	int unk9C;
	int rotateSubObjIndex;
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Draw(void);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
	void UpdateShake(void);
	void UpdateWaterRipple(void);
	void UpdateDropLeaf(void);
	void UpdateRotate(void);
	void Show(bool);
	StaticFXPropDesc* GetDesc(void) {
		return descr_cast<StaticFXPropDesc*>(pDescriptor);
	}
    Vector* GetPos(void) {
        return pModel->matrices[0].Row3();
    }

    static bool bTempVisible;
};

void StaticProp_LoadResources(KromeIni*);

extern "C" void memset(void*, int, int);

/*
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
}*/