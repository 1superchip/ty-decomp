#include "ty/GameObjectManager.h"

void WaterVolume_LoadResources(KromeIni* pIni);
bool WaterVolume_IsWithin(Vector*, float*);

struct WaterVolume : GameObject {
	Matrix matrix;
	Matrix waterVolumeMatrix;
	float unkBC;
	float unkC0;
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
};