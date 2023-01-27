#ifndef COLLAPSIBLE_H
#define COLLAPSIBLE_H

#include "ty/props/StaticProp.h"
#include "common/KromeIni.h"

// move this to the proper header
struct ShatterStruct {
	char padding_0x0[0x24];
	Model* pModel;
	void Fall(void);
};
ShatterStruct* Shatter_Add(Model*, float, float, int);

struct Collapsible : StaticProp {
	int state;
	int unk5C;
	Vector defaultTrans; // model translation
	Vector scale;
	Vector defaultRot;
	ShatterStruct* shatter;
	
	virtual void LoadDone(void);
	virtual void Reset(void) {
		pModel->matrices[0].SetTranslation(&defaultTrans);
		pModel->matrices[0].SetRotationPYR(&defaultRot);
		pModel->matrices[0].Scale(&pModel->matrices[0], &scale);
		state = 0;
		unk5C = 0;
		collisionInfo.bEnabled = true;
		collisionInfo.flags = 0;
		collisionInfo.pProp = NULL;
	}
	virtual void Update(void);
	virtual void Draw(void) {
		if (!Hidden()) {
			// if the Collapsible isn't hidden, draw it
			StaticProp::Draw();
		}
	}
	
	virtual void Init(GameObjDesc* pDesc) {
		StaticProp::Init(pDesc);
		state = 0;
		shatter = Shatter_Add(pModel, 1.0f, 0.7f, 0x78);
		Reset();
	}
	
	bool Hidden(void) {
		return state == 3 || state == 2;
	}
};

void Collapsible_LoadResources(KromeIni*);

#endif // COLLAPSIBLE_H