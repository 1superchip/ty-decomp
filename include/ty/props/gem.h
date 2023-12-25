#ifndef GEM_H
#define GEM_H

#include "ty/GameObject.h"
// #include "common/Material.h"
#include "common/ParticleSystem.h"
#include "ty/StructList.h"
#include "common/KromeIni.h"
#include "common/Matrix.h"

struct GemParticleSysInfo {
	float unk0;
	float unk4;
	float unk8;
	ParticleEnvelope envelopes[4];
	void (*updateFunc)(ParticleSystem*);
};

struct ElementInfo {
	char* pModelName;
	char* pMaterialName;
	char* pMaterialName1;
	Model* pModel;
	Material* pMaterial;
	Material* pParticleMaterial;
};

enum ElementType {
	ELEMENT_FIRE 		= 0,
	ELEMENT_ICE 		= 1,
	ELEMENT_AIR 		= 2,
	ELEMENT_RAINBOW		= 3,
	ELEMENT_EARTH 		= 4,
	MAX_GEM_ELEMENTS	= 5,
};

enum GemState {};
enum GemType {};

struct GemPickupData {
	Vector unk0;
	float unk10;
};

struct GemModelDrawDataSubStruct {
	float unkData[0x11];
};

struct GemModelDrawData {
	Vector unk0;
	bool unk10;
	float unk14; // screen x
	float unk18; // screen y
	float unk1C;
	float unk20;
	float unk24;
	float unk28; // some transform point value
	Blitter_Image imgs[5];
	float unk180_array[5][2];
};

void Gem_LoadResources(KromeIni* pIni);

void Gem_DrawModel(Vector* pPos, Vector* pScale, ElementType type);
void Gem_SetElement(ElementType newType);

void Gem_HideAll(void);
void Gem_ShowAll(void);
int Gem_GetCount(void);

void Gem_PickupParticle_LoadResources(void);
void Gem_PickupParticle_Draw(void);
void Gem_PickupParticle_SpawnParticles(Vector*);
Material* Gem_GetParticleMaterial(ElementType type);

void Gem_ParticleSystem_Init(void);
void Gem_ParticleSystem_Deinit(void);
void Gem_ParticleSystem_Update(void);
void Gem_FireCustomUpdate(ParticleSystem* pSys);

#define GEMS_MAXOPALS 300

/// @brief Opal object
struct Gem : GameObject {
	Blitter_Particle mParticle;
	int unk6C;
	GemState mState;
	Vector pos;
	float unk84[3];
	
	// mLerpTime is used when spawning and magnetised
	float mLerpTime; // used for Linear Interpolation
	Vector unk94; // mFinalPos?
	Vector mCollisionNormal;
	Matrix unkB4;
	s16 unkF4;
	u8 unkF6b0 : 1;
	bool unkF6b1 : 1;
	u8 unkF6b2 : 1;
	u8 unkF6b3 : 1;
	u8 unkF6b4 : 1;
	u8 unkF6b5 : 1;
	u8 unkF6b6 : 1;
	u8 unkF6b7 : 1;
	float yOffsetAngle;
	float unkFC;
	char unk100[12];
	
	static int totalGems;
	static Gem** gemPtrList;

	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Draw(void);
	virtual void Init(GameObjDesc* pDesc);
	virtual void Deinit(void);
	
	static void PostDraw(void*);
	void Spawn(void);
	void SpawnStatic(void);
	void CalcShadowPos(void);
	void SetState(GemState);
	void Collecting(void);
	void Magnetised(void);
	void Spawning(void);
	void Idle(void);
	void Collect(void);
	bool CheckMagnetism(float);
	float GetMagneticRangeSqr(void);
	bool UpdateCollection(float dist);
};

Gem* Gem_Add(GemType, Vector* pPos, Vector* r5);
void Gem_DeleteList(void);

extern StructList<Gem> spawnedGemList;

#endif // GEM_H
