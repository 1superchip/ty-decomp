#include "ty/GameObjectManager.h"

void DDACheckpoint_LoadResources(KromeIni* pIni);

struct DDACheckpoint : GameObject {
	int Number;
	
	bool LoadLine(KromeIniLine*);
	void Message(MKMessage*);
	void Init(GameObjDesc*);
};