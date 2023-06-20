#ifndef SOUNDPROP_H
#define SOUNDPROP_H

#include "types.h"
#include "ty/soundbank.h"
#include "common/Vector.h"
#include "ty/GameObject.h"
#include "ty/CommonGameObjectFlags.h"

struct SoundProp : GameObject {
	Matrix localToWorld;
	SoundEventHelper unk7C;
	int unk80;
	int unk84;
	int unk88;
	int minDelay;
	int maxDelay;
	CommonGameObjFlagsComponent gameObjFlags;
	
	virtual bool LoadLine(KromeIniLine* pLine);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Message(MKMessage* pMsg);
	virtual void Init(GameObjDesc* pDesc);
	virtual void Deinit(void);
	void Play(void);
	Vector* GetPos(void) {
		return localToWorld.Row3();
	}
};

void SoundProp_LoadResources(KromeIni*);

#endif // SOUNDPROP_H
