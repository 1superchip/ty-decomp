#include "common/KromeIni.h"
#include "common/Texture.h"

struct Material {
	char name[0x20];
    int collisionFlags;
    int texture_filterType;
    int type;
    int blendMode;
    int stateFlags;
    int grass;
    int effect;
    Vector color;
    int referenceCount;
    Material* unk50;
    Texture* unk54;
    Texture* unk58;
    float unk5C;
    Matrix unk60;
    float unkA0;
    float unkA4;
    float unkA8;
    float angle;
    float unkB0;
    float unkB4;
    float unkB8;
    float unkBC;
    float unkC0;
    int frameCounter1;
    int mipBias;
    bool unkCC;
    bool unkCD;
    Vector indirectWaterVec;
	
	char* InitFromMatDefs(char*);
	static Material* Create(char*);
	void Destroy(void);
	static Material* Find(char*);
	static void InitModule(void);
	static void DeinitModule(void);
	static Material* CreateFromRawData(char*, void*, int, int, int);
	static Material* CreateRenderTarget(char*, int, int, int);
	void SetTextureAlias(Material*);
	void SetTextureAlias(Texture*);
	static Material* GetMaterialList(void);
	static Material* CreateMpegTarget(char*, void*, int, int);
	void Use(void);
	static void UseNone(int);
	void CaptureDrawBuffer(float, float, float, float);
	void Update(void);
	void ScrollUVOffset(float, float);
	static void UpdateCounter(void);

    void ClearFlags(int flags) {
        stateFlags &= ~flags;
    }
    void SetFlags(int flags) {
        stateFlags |= flags;
    }

    static int frameCounter;
	static int updateEnabled;
};