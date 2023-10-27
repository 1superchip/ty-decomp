#ifndef PLATFORM_H
#define PLATFORM_H

#include "ty/props/StaticProp.h"
#include "types.h"
/*#include "common/KromeIni.h"
#include "common/Vector.h"*/
#include "common/PtrList.h"

void Platform_LoadResources(KromeIni*);

struct PlatformRider {
	GameObject* pObject; // might be Platform*
	int platformID;
	bool unk8; // has attachment?
	
	void Init(void);
	bool LoadLine(KromeIniLine*);
	void Resolve(void);
	void Attach(GameObject*);
	void Detach(GameObject*);
	void ToLocal(Vector*);
	void ToWorld(Vector*);
	void ToLocalDir(Vector*);
	void ToWorldDir(Vector*);
	void UpdateDynamicAttach(CollisionResult* pCr, GameObject* pObj);
};

struct PlatformDesc : StaticPropDescriptor {
	float maxMag;
	float radius;
	float maxTilt;
	float rotVel;
	float maxShadowHeight;
	
	void Init(ModuleInfoBase*, char*, char*, int, int);
	void Load(KromeIni*);
};

struct Platform : StaticProp {
	Vector* unk58;
	Vector mDefaultRot;
	Vector scale;
	Vector mCurrRot;
	// list of all objects attached to this platform
	PtrList<GameObject> attachments;
	int numAttached; // number of attached objects
	PlatformRider rider;
	Vector unkA0;
	Vector unkB0;
	Vector unkC0;
	bool unkD0;
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
	void BeginUpdate(void);
	void EndUpdate(void);
	void UpdateTilt(void);
	void UpdateRotationMatrix(void);
	void UpdateAttached(void);
	void UpdateShadow(void);
	void Attach(GameObject*);
	void Detach(GameObject*);
	void PushTy(int);
	
	PlatformDesc* GetDesc(void) {
		return descr_cast<PlatformDesc*>(pDescriptor);
	}
};

#endif // PLATFORM_H
