

// sizeof 0xf0
// clean this up
struct DirectLight {
	Vector pad[8];
    Vector lightArray[6]; // colors
	Vector ambient;
	
	static void InitModule(void);
	static void DeinitModule(void);
	void Init(void);
	void Set(Vector*);
	void SetLight(int, Vector*, Vector*);
	void SetAmbient(Vector*);
	void RecalcMatrices(void);
	
	static DirectLight* pDefaultLight;
};

extern DirectLight* DirectLight::pDefaultLight;
