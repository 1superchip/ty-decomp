#ifndef COMMON_MATERIAL
#define COMMON_MATERIAL

#include "common/KromeIni.h"
#include "common/Texture.h"
#include "common/Vector.h"
#include "common/Matrix.h"

extern GXColor Material_MixedColor;

#define Blend_Opaque        0
#define Blend_Blend         1
#define Blend_Alpha         2
#define Blend_Additive      3
#define Blend_Subtractive   4

#define Flag_Invisible  0x20
#define Flag_NoCollide  0x40
#define Flag_DoubleSided 0x80
#define Flag_AlphaMask  0x100
#define Flag_Scroll     0x12000
#define Flag_Animate    0x22000
#define Flag_Rotate     0x42000
#define Flag_SinRotate  0x82000
#define Flag_EnvRotate  0x102000

#define Type_Decal          0
#define Type_EdgeFadeout    1
#define Type_EnvMap         2
#define Type_Prelit         3
#define Type_Caustic        4
#define Type_AlphaFog       5
#define Type_IndirectWater  8

// TODO define collision flags here

struct Material {
    char name[0x20];
    int collisionFlags;
    int texture_filterType;
    int type;
    int blendMode;
    int flags;
    int grass;
    int effect;
    Vector color;
    int referenceCount;
    Material* pOverlayMat;
    Texture* unk54;
    Texture* unk58;
    float unk5C; // used in ref0 parameter for GXSetAlphaCompare
    Matrix unk60;
    // 0xA0 - 0xAC could be a Vector?
    // TODO figure out names for 0xA0 - 0xAC
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
    Vector indirectWaterVec; // may not be a Vector?
    int unkE0;
    
    static void InitModule(void);
    static void DeinitModule(void);
    char* InitFromMatDefs(char*);
    static Material* Create(char*);
    void Destroy(void);
    static Material* Find(char*);
    static Material* CreateFromRawData(char*, void*, int, int, int);
    static Material* CreateRenderTarget(char*, int, int, int);
    static Material* CreateMpegTarget(char* pName, void* pData, int width, int height);
    static Material** GetMaterialList(void);
    void SetTextureAlias(Material*);
    void SetTextureAlias(Texture*);
    void Use(void);
    static void UseNone(int);
    void CaptureDrawBuffer(float, float, float, float);
    void Update(void);
    void ScrollUVOffset(float, float);
    static void UpdateCounter(void);

    void ClearFlags(int _flags) {
        flags &= ~_flags;
    }
    void SetFlags(int _flags) {
        flags |= _flags;
    }
    // void ConditionallySetFlags(int _flags, bool cond) {
    //     if (cond == false) {
    //         if (!(flags & _flags)) {
    //             flags |= 0x1000;
    //             flags |= ~_flags;
    //         }
    //     } else if (flags & _flags) {
    //         flags |= (_flags | 0x1000);
    //     }
    // }

    // if (cond) flags |= _flags
    // else flags &= ~_flags
    void ConditionallySetRemoveFlags(int _flags, bool cond) {
        // debug build is quite different
        if (cond) {
            flags |= _flags;
        } else {
            flags &= ~_flags;
        }
    }
    Texture* GetTexture(void) {
        if (unk58 != NULL) {
            return unk58;
        }
        return unk54;
    }

    void SetMatrixX(float xVal) {
        flags |= 0x2000;
        unk60.data[3][0] = xVal;
    }

    static Material* pCurrMat[2];
    static int frameCounter;
    static int updateEnabled;
};

#endif // COMMON_MATERIAL
