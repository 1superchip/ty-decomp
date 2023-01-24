#ifndef DDACHECKPOINT_H
#define DDACHECKPOINT_H

#include "ty/GameObjectManager.h"

void DDACheckpoint_LoadResources(KromeIni* pIni);

struct DDACheckpoint : GameObject {
	int Number;
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
};

#endif // DDACHECKPOINT_H