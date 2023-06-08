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
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
    void Play(void);
};

void SoundProp_LoadResources(KromeIni*);

#endif // SOUNDPROP_H
