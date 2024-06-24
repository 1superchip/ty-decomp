#ifndef COMMON_TEXTURE
#define COMMON_TEXTURE

#include "types.h"
#include "common/Vector.h"
#include "Dolphin/gx.h"

extern Vector Texture_Color;
extern bool Texture_bColourKey;
extern int Texture_filterType;
extern bool Texture_IsAlias;

struct TexFile {
    int fmt;
    int width;
    int height;
    int padding[5];
    u16 unk20;
};

struct Texture {
    char name[0x20];
    int width;
    int height;
    int referenceCount;
    bool bMpegTarget; // only set to true by CreateMpegTarget
    void* pYData; // Pointer to Y data of Video YUV data
    void* pUData; // Pointer to U data of Video YUV data
    void* pVData; // Pointer to V data of Video YUV data
    void* pFileData;
    bool bTlut;
    GXTexObj texObj;
    GXTlutObj tlutObj;
    int unk70;
    static bool initialised;

    static void InitModule(void);
    static void DeinitModule(void);
    static Texture* Create(char*);
    void Destroy(void);
    void Use(void);
    static Texture* Find(char*);
    static Texture* CreateRenderTarget(char*, int, int, int);
    static Texture* CreateFromRawData(char* pName, void* pRawData, int format, int width, int height);
    static Texture* CreateMpegTarget(char*, void*, int, int);
};

#endif // COMMON_TEXTURE