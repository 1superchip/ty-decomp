#include "ty/GameObjectManager.h"

void DDACheckpoint_LoadResources(KromeIni* pIni);

struct DDACheckpoint : GameObject {
	int Number;
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
};