#ifndef COMMON_DIRECTLIGHT
#define COMMON_DIRECTLIGHT

// sizeof 0xf0
// clean this up
struct DirectLight {
	Vector pad[8];
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
