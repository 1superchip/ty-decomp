#ifndef COMMON_DIRECTLIGHT
#define COMMON_DIRECTLIGHT

#include "common/Vector.h"
#include "common/Matrix.h"

struct DirectLight {
	Matrix mDirMatrix;
	Vector mNewColors[3];
	Vector mNewAmbient;

	Vector mLightDirs[3];
	Vector mLightColors[3];
	Vector mAmbient;
	
	static void InitModule(void);
	static void DeinitModule(void);
	void Init(void);
	void Set(Vector*);
	void SetLight(int, Vector*, Vector*);
	void SetAmbient(Vector*);
	void RecalcMatrices(void);
	
	static DirectLight* pDefaultLight;
};

#endif // COMMON_DIRECTLIGHT
