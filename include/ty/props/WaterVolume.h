#include "ty/GameObjectManager.h"

void WaterVolume_LoadResources(KromeIni* pIni);
bool WaterVolume_IsWithin(Vector*, float*);

struct WaterVolume : GameObject {
	Matrix matrix;
	Matrix waterVolumeMatrix; // pos in ini is Row3 of this matrix
	float unkBC;
	float unkC0;
	virtual bool LoadLine(KromeIniLine* pLine);
	virtual void LoadDone(void);
	virtual void Init(GameObjDesc* pDesc);
	virtual void Deinit(void);
};