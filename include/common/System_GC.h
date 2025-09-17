#ifndef COMMON_SYSTEM_GC
#define COMMON_SYSTEM_GC

#include "common/View.h"
#include "common/Material.h"
#include "common/Vector.h"

extern char* gpBuildVersion;

struct ZCheckRequest {
    Vector unk0;
    int unk10;
    View* unk14;
    int unk18;
    float depth;
    float unk20;
    float unk24;
    float unk28;
    float unk2C;
};

/// @brief Contains default system values
struct MKDefaults {
    char padding[0x7];
    bool unk7;
    int unk8; // used for scene dma in the ps2 build
    int unkC; // used for texture dma in the ps2 build
    bool lockTo30; // lockTo30
    float unk14;
    int screenshot;

    // Number of Material instances
    int materialCount;

    // Start of Material default values
    // Default Material Texture Filter Type
    int materialTextureFilterType;
    // Default Material Type
    int materialType;
    // Default Material Flags
    int materialFlags;
    // Default Material Blend Mode
    int materialBlendMode;
    // Default Material Collision Flags
    int collisionFlags;
    // Default Material Alpha Ref
    float materialAlphaRef;

    int unk38;
    int unk3C;
    int unk40;
    int numModelInstances; // Model instance count
    int numModelTemplateInstances; // Model Template instance count
    int unk4C; // gMKDefaults.mdl.maxVerticesArray
    int unk50;
    int unk54;
    int unk58;

    // Start of DebugInfo counts
    // These variables are used in DebugInfo_Init
    // to initialize PtrListDL structs

    // DebugInfo Line3D count
    int debugLine3D_count; // name = "maxLines"?
    // DebugInfo Particle count
    int debugParticle_count; // name = "maxParticles"?
    // DebugInfo Box count
    int debugBox_count;
    // DebugInfo Sphere count
    int debugSphere_count; // name = "maxSpheres"?
    // DebugInfo Message count
    int debugMessage_count; // name = "maxScreenMessages"?
    // DebugInfo Message3D count
    int debugMessage3D_count; // name = "max3DMessages"?

    int unk74;
    int grassPtrListDLCount;
    int unk7C;
};

extern MKDefaults gMKDefaults;

struct RenderState {
    Material* pDefaultMaterial;
    int fillState; // shadow related?
    int unk8;
    int unkC;
    Texture* pDefaultTexture;
    int unk14;
    int alpha;
};

extern RenderState gRenderState;

enum RENDERSTATE_DATA_TYPE {
    RENDERSTATE_MATERIAL    = 0,
    RENDERSTATE_FILLSTATE   = 1,
    RENDERSTATE_TEXTURE     = 4,
};

inline int GetRenderStateData(RENDERSTATE_DATA_TYPE type) {
    return ((int*)&gRenderState.pDefaultMaterial)[type];
}

// not sure where to put this
inline void SetRenderStateData(RENDERSTATE_DATA_TYPE type, void* data) {
    Material::UseNone(-1);
    ((int*)&gRenderState.pDefaultMaterial)[type] = (int)data;
}

struct Display {
    int region; // Default = 2
    int unk4;
    float fps; // 50 for PAL, 60 for NTSC
    float dt; // 1.0f / fps
    int unk10;
    int unk14;
    char padding_0x18[0x10];
    int unk28;
    int unk2C;
    char padding_0x30[0x34];
    float orthoXSize;
    float orthoYSize;
};

extern Display gDisplay;

extern "C" int main(int argc, char* argv[]);

void System_InitModule(void*, char* pCmdLineArgs);
void System_DeinitModule(void);
void System_EnableDebugMode(bool bDebugMode);
void System_SetDebugCameraSpeedMultiplier(float speed);
void System_InitCommandLineArgs(char* pCmdLineArgs);
void System_DeinitCommandLineArgs(void);
char* System_GetCommandLineParameter(char*);
void System_SetScreenOffset(float, float);
ZCheckRequest* System_CreateZRequest(void);
void System_DestroyZRequest(ZCheckRequest*);
void System_CheckZRequests(void);
void System_DoReset(bool r3, bool r4, bool r5);
void System_IdleFunction(void* r3);

void ByteReverseVector(Vector &vec);

void* operator new(size_t size);
void operator delete(void* pMem);

#endif // COMMON_SYSTEM_GC
