#ifndef D1PROP_H
#define D1PROP_H

#include "ty/GameObject.h"
#include "common/KromeIni.h"
#include "ty/Messages.h"

struct D1Prop : GameObject {
	bool bActive;
	bool unk3D;
	EventMessage OnCompletion100;
	EventMessage OnCompletion;
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Draw(void);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
};

void D1_Prop_LoadResource(KromeIni*);
void D1PropEndOfCredits(void);

#endif // D1PROP_H