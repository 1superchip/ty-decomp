#include "types.h"
#include "Dolphin/os.h"
#include "Dolphin/vi.h"
#include "common/System_GC.h"
#include "common/Str.h"
#include "common/FileSys.h"
#include "common/File.h"
#include "common/Animation.h"
#include "common/MKAnimScript.h"
#include "common/PtrListDL.h"
#include "common/demoinit.h"
#include "common/Debug.h"
#include "common/MKSceneManager.h"
#include "common/Collision.h"
#include "common/Translation.h"
#include "common/Camera.h"
#include "common/Heap.h"
#include "common/Blitter.h"
#include "common/DiscErrors.h"
#include "common/Input.h"
#include "common/MKRumble.h"
#include "common/Water_GC.h"
#include "common/MKPackage.h"
#include "common/Video.h"
#include "common/MKShadow.h"
#include "common/MKGrass.h"

extern "C" void memset(void*, int, int);

#define IDLE_THREAD_STACK_SIZE (0x8000)

static char IdleThreadStack[IDLE_THREAD_STACK_SIZE];
RenderState gRenderState;
static Camera debugCamera;
static char* ppCmdLineArgStrings[20];


static int pf_enabled = 0;
OSThread* pMainThread;
int resetState = 0;
static PtrListDL<ZCheckRequest> zCheckRequests;
static int gSysOptions = 0;
static bool bExit = false;


MKDefaults gMKDefaults = {
    {0}, // padding 0x0
    true, // unk7
    0x100000, // unk8
    0x40000, // unkC
    false, // lockTo30
    1.0f, // unk14
    0, // screenshot
    512, // materialCount
    2, // materialTextureFilterType
    Type_Decal, // materialType
    0, // materialFlags
    Blend_Blend, // materialBlendMode
    0x80000000, // collisionFlags
    0.5f, // materialAlphaRef
    0x400, // unk38
    0x200, // unk3C
    0x20, // unk40
    2500, // numModelInstances
    256, // numModelTemplateInstances
    16384, // unk4C, gMKDefaults.mdl.maxVerticesArray
    5500, // unk50, some heap variable which went unused
    0, // unk54
    0x100000, // unk58
    200, // debugLine3D_count
    300, // debugParticle_count
    100, // debugBox_count
    50, // debugSphere_count
    30, // debugMessage_count
    100, // debugMessage3D_count
    2, // unk74
    5, // grassPtrListDLCount
    0, // unk7C
};

Display gDisplay = {
    2, // region
    0, // unk4
    0.0f, // fps
    0.0f, // dt
    0, // unk10
    0, // unk14
    {0}, // unk18
    0, // unk28
    0, // unk2C
    {0}, // unk30
    0.0f, // orthoXSize
    0.0f // orthoYSize
};
int gFrameCounter = 0;
int gDrawCounter = 0;
static bool bDebugEnabled = false;
static bool bDebugMenuActive = false;
static bool bDebugModeActive = false;
static View* pGameView;
static int cmdLineArgCount = 0;

static char* gFillModeOptStrings[9] = {
    "Solid",
    "Unfogged",
    "Untextured",
    "Wireframe",
    "Point",
    "UploadOnly",
    "NoUploads",
    "Strips"
};

static int stopFrame = -1;

char* gpBuildVersion = "<not set>";

extern void Game_InitSystem(void);
extern void MKMemoryCard_InitModule(void);
extern "C" void Sound_InitModule(void);

void System_InitModule(void*, char* pCmdLineArgs) {
    pMainThread = OSGetCurrentThread();

    memset((void*)&gRenderState, 0, sizeof(RenderState));

    gRenderState.alpha = -1;
    
    if (pCmdLineArgs) {
        System_InitCommandLineArgs(pCmdLineArgs);
    }

    Game_InitSystem();
    Heap_InitModule(gMKDefaults.unk50);
    bDebugEnabled = true;
    Debug_InitModule();
    File_InitModule();
    FileSys_InitModule();
    MKPackage_InitModule();
    Font::InitModule();
    DirectLight::InitModule();
    View::InitModule();
    Input_InitModule();
    Texture::InitModule();
    Material::InitModule();
    MKMemoryCard_InitModule();
    Model_InitModule();
    Animation_InitModule();
    MKAnimScript_InitModule();
    Blitter_InitModule();
    Translation_InitModule();
    Camera::InitModule();
    Collision_InitModule();
    MKSceneManager::InitModule();
    Sound_InitModule();
    Water_InitModule();
    Video_InitModule();

    gSysOptions = 0;

    gFrameCounter = 0;

    gDrawCounter = 0;

    debugCamera.Init();
    
    Debug_SetFont(
        Font::Create("DebugFont1",
            "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
            1.0f, 1.8f, 0.0f
        )
    );

    MKShadow_InitModule();

    MKGrass_InitTypes("grass_types.ini");
}

extern "C" void Sound_DeinitModule(void);
extern void MKMemoryCard_DeinitModule(void);

// might be missing inlines that cause the inline depth limit
// to be reached for main
#pragma dont_inline on
void System_DeinitModule(void) {
    debugCamera.Deinit();
    gpDebugFont->Destroy();
    Debug_SetFont(NULL);
    MKGrass_DeinitTypes();
    MKShadow_DeinitModule();
    MKSceneManager::DeinitModule();
    Sound_DeinitModule();
    Water_DeinitModule();
    Video_DeinitModule();
    Collision_DeinitModule();
    Camera::DeinitModule();
    Translation_DeinitModule();
    Blitter_DeinitModule();
    Model_DeinitModule();
    MKAnimScript_DeinitModule();
    Animation_DeinitModule();
    MKMemoryCard_DeinitModule();
    Material::DeinitModule();
    Texture::DeinitModule();
    Input_DeinitModule();
    View::DeinitModule();
    DirectLight::DeinitModule();
    Font::DeinitModule();
    MKPackage_DeinitModule();
    FileSys_DeinitModule();
    zCheckRequests.Deinit();
    System_DeinitCommandLineArgs();
    Debug_DeinitModule();
    Heap_DeinitModule();
}
#pragma dont_inline off

void System_GameDraw(void);
bool System_Update_Normal(void);
extern void Game_Init(void);
extern void Game_Deinit(void);
extern "C" void Sound_Update(void);
extern "C" int VIGetRetraceCount(void);
extern int Game_Update(void);
extern "C" int OSGetConsoleSimulatedMemSize(void);
extern void Game_Draw(void);
extern void MKMemoryCard_Update(void);

// Main game loop function
void System_DoGameLoop(void) {
    bool inputMapping = Input_IsKeyMappingEnabled();
    
    while (true) {
        Model::Purge();
        Input_Update(true);
        FileSys_Update();
        Sound_Update();
        Water_Update();

        if (stopFrame != -1 && gFrameCounter >= stopFrame) {
            stopFrame = -1;
            bDebugModeActive = true;
        }

        MKMemoryCard_Update();

        if (bDebugEnabled != false) {
            if (Input_WasButtonPressed(CHAN_0, 3, NULL)) {
                if (bDebugMenuActive || Input_GetButtonState(CHAN_0, 6, NULL)) {
                    bDebugMenuActive = !bDebugMenuActive;
                    if (!bDebugMenuActive) {
                        Input_ClearPadData();
                    }
                }
            }
        }

        Input_EnableKeyMapping(inputMapping);

        if (System_Update_Normal()) {
            return;
        }

        DEMODoneRender();
        System_GameDraw();
    }
}

bool System_Update_Normal(void) {
    static int lastCount = VIGetRetraceCount();

    int newRetraceCount = VIGetRetraceCount();

    uint deltaRetraceCount = newRetraceCount - lastCount;

    lastCount = newRetraceCount;

    if (deltaRetraceCount > 4) {
        deltaRetraceCount = 4;
    }

    while (deltaRetraceCount-- != 0) {
        Material::UpdateCounter();

        if (Game_Update() || bExit) {
            return true;
        }

        gFrameCounter++;

        if (deltaRetraceCount != 0) {
            Input_Update(false);
            Sound_Update();
        }
    }

    return false;
}

// this function looks fine based on july 1st and jimmy neutron
// Game Drawing function
void System_GameDraw(void) {
    // Call DEMOBeforeRender()
    DEMOBeforeRender();

    // Use Default View
    View::GetDefault()->Use();
    Matrix m;
    m.SetIdentity();
    Game_Draw();

    static Material* pTestCapture;
    static int createmat = 0;

    if (createmat == 0) {
        pTestCapture = Material::Create("capture");
        createmat++;
    }

    pGameView = View::GetCurrent();
    Material::UseNone(-1);
    Collision_Draw();
    
    if (pf_enabled != 0) {
        int totalMemLeft = OSGetConsoleSimulatedMemSize() - OSCheckHeap(0);

        char* debugStr = Str_Printf(
            "%3d /%3d MEM:%.2fK MK:%d BLD:%s",
            gCPUCycles / 81000, 
            gGXCycles / 81000,
            totalMemLeft / 1024.0f, 
            105, 
            gpBuildVersion
        );
        
        gpDebugFont->DrawText(
            debugStr, 
            320.0f, 495.0f, 
            1.0f, 1.0f,
            FONT_JUSTIFY_5, 
            COLOR_WHITE
        );
    }
    
    gDrawCounter++;
}

void System_EnableDebugMode(bool bDebugMode) {
    if (bDebugEnabled) {
        bDebugModeActive = bDebugMode;
    }
}

void System_SetDebugCameraSpeedMultiplier(float speed) {
    debugCamera.unkC = speed;
}

void System_SetDisplayMode(int _region, int r4, int r5, int r6, int r7) {
    gDisplay.region = _region;

    if (VIGetTvFormat() == VI_PAL) {
        gDisplay.fps = 50.0f;
    } else {
        gDisplay.fps = 60.0f;
    }

    gDisplay.dt = 1.0f / gDisplay.fps;
    gDisplay.unk10 = r4;
    gDisplay.unk14 = r5;
    gDisplay.unk28 = 640;
    gDisplay.unk2C = 512;
}

void System_SetOrthoSize(int orthoXSize, int orthoYSize) {
    gDisplay.orthoXSize = (float)orthoXSize;
    gDisplay.orthoYSize = (float)orthoYSize;
}

extern "C" char* strchr(char*, char);
extern "C" int strlen(char*);
extern "C" void strncpy(char*, char*, int);
extern "C" void* malloc(size_t);

void System_InitCommandLineArgs(char* pCmdLineArgs) {
    char* r30;
    char* pCmdLine = pCmdLineArgs;
    int len;
    
    // Skip over spaces to get to next data
    while (*pCmdLine == ' ') {
        pCmdLine++;
    }

    while (true) {
        r30 = pCmdLine;
        if (*pCmdLine == 0) {
            return;
        }

        char* pFoundSpace = strchr(pCmdLine, ' ');
        pCmdLine = pFoundSpace;

        if (pCmdLine != NULL) {
            len = pCmdLine - r30;
            pCmdLine++;
        } else {
            len = strlen(r30);
        }

        ppCmdLineArgStrings[cmdLineArgCount] = (char*)malloc(len + 1);

        strncpy(ppCmdLineArgStrings[cmdLineArgCount], r30, len);

        ppCmdLineArgStrings[cmdLineArgCount][len] = '\0';

        cmdLineArgCount++;

        if (pCmdLine == NULL) {
            break;
        }
    }
}

extern "C" void free(void*);

void System_DeinitCommandLineArgs(void) {
    for (int i = 0; i < cmdLineArgCount; i++) {
        free(ppCmdLineArgStrings[i]);
    }

    cmdLineArgCount = 0;
}

extern int strnicmp(const char*, const char*, int);

char* System_GetCommandLineParameter(char* cmd) {
    int len = strlen(cmd);
    
    for (int i = 0; i < cmdLineArgCount; i++) {
        if (strnicmp(cmd, ppCmdLineArgStrings[i], len) == 0) {
            if (ppCmdLineArgStrings[i][len] == '=') {
                return &ppCmdLineArgStrings[i][len + 1];
            } else if (ppCmdLineArgStrings[i][len] == '\t') {
                return gNullStr;
            } else if (ppCmdLineArgStrings[i][len] == ' ') {
                return gNullStr;
            } else if (ppCmdLineArgStrings[i][len] == '\0') {
                return gNullStr;
            }
        }
    }

    return NULL;
}

void System_SetScreenOffset(float, float) {

}

ZCheckRequest* System_CreateZRequest(void) {
    ZCheckRequest* pCheck = zCheckRequests.GetNextEntry();
    pCheck->unk10 = 0;
    pCheck->unk18 = 0;
    pCheck->unk20 = -1000.0f;
    pCheck->unk24 = -1000.0f;
    pCheck->depth = 0.0f;
    return pCheck;
}

void System_DestroyZRequest(ZCheckRequest* pRequest) {
    zCheckRequests.Destroy(pRequest);
}

extern "C" void GXPeekZ(u16 x, u16 y, u32* z);
#define Z_BUFFER_DEPTH (16777216.0f - 1.0f) // 2^24 - 1

/// @brief Updates zCheckRequests every frame for Screen Z Buffer checking
/// @param  None
void System_CheckZRequests(void) {
    ZCheckRequest** ppRequests = zCheckRequests.GetMem();
    while (*ppRequests != NULL) {
        ZCheckRequest* pCheck = *ppRequests;

        if (pCheck->unk10 == 0) {
            return;
        }

        pCheck->unk0.w = 1.0f;
        
        Vector spC;
        spC.ApplyMatrixW(&pCheck->unk0, &pCheck->unk14->unk1C8);
        if (spC.w == 0.0f) {
            pCheck->unk18 = 0;
            pCheck->unk2C = 0.0f;
            pCheck->unk28 = spC.w;
            pCheck->unk20 = -1000.0f;
            pCheck->unk24 = -1000.0f;
            pCheck->depth = 0.0f;
            return;
        }

        int r26 = gDisplay.unk10;
        int r31 = VIGetTvFormat() == VI_PAL ? 496 : 448;

        float f5 = 1.0f / spC.w;
        float f1 = (spC.x * f5) * (float)r26;
        float f3 = (spC.y * f5) * r31;
        
        IntVector vec;
        vec.x = (r26 >> 1) + (int)(f1 * 0.5f);
        vec.y = r31 - ((r31 >> 1) + (int)(f3 * 0.5f));
        vec.z = (spC.z * f5) * Z_BUFFER_DEPTH;

        if (f5 < 0.0f || vec.x < 0 || vec.x >= r26 || vec.y < 0 || vec.y >= r31) {
            pCheck->unk18 = 0;
            pCheck->unk2C = f5;
            pCheck->unk28 = 1.0f / f5;
            pCheck->unk20 = -1000.0f;
            pCheck->unk24 = -1000.0f;
            pCheck->depth = 0.0f;
            return;
        }

        pCheck->unk18 = 1;
        pCheck->unk20 = (float)vec.x;
        pCheck->unk24 = (float)vec.y;
        pCheck->unk2C = f5;
        pCheck->unk28 = 1.0f / f5;

        switch (pCheck->unk10) {
            case 1:
            case 2:
            pCheck->depth = 0.0f;
            for (int y = -6; y <= 6; y++) {
                for (int x = -6; x <= 6; x++) {
                    int yPos = vec.y + y;
                    int xPos = vec.x + x;
                    if (yPos >= 0 && yPos < r31 && xPos >= 0 && vec.x + x < r26) {
                        u32 z;
                        GXPeekZ(xPos, yPos, &z);
                        if (z >= vec.z) {
                            pCheck->depth += 0.00591716f;
                        }
                    } 
                }
            }
            break;
        }
        ppRequests++;
    }
}

static bool bHeapAllocated = false;

void* operator new(size_t size) {
    if (bHeapAllocated) {
        return OSAllocFromHeap(__OSCurrHeap, size);
    }

    return malloc(size);
}

void operator delete(void* pMem) {
    OSFreeToHeap(__OSCurrHeap, pMem);
}

template <typename T>
void ByteReverse(T&);

void ByteReverseVector(Vector &vec) {
    ByteReverse<float>(vec.x);
    ByteReverse<float>(vec.y);
    ByteReverse<float>(vec.z);
    ByteReverse<float>(vec.w);
}

static int ErrorHandled = 0;
extern "C" void OSDumpContext(OSContext*);
extern "C" void exit(int);

static void MyErrorHandler(u16 r3, OSContext* pContext, ...) {
    if (ErrorHandled == 0) {
        ErrorHandled = 1;
        OSDumpContext(pContext);
        exit(0);
    }
}

#define ALIGN_UP(x, a) (((x) + (a - 1)) & ~(a - 1))
extern void XFONTInit(u32, u32);
extern void XFONTSetFrameBuffer(void*);
extern void XFONTSetFgColor(u32);
extern "C" void GXPokeZMode(GXBool doCompare, GXCompare func, GXBool doUpdate);
extern "C" void GXPokeBlendMode(GXBlendMode mode, GXBlendFactor srcFactor, GXBlendFactor destFactor, GXLogicOp op);
extern "C" int strlen(char*);
extern "C" void strcpy(char*, char*);
extern "C" void strcat(char*, char*);

extern "C" int main(int argc, char* argv[]) {
    static char cmdLine[256] = {};

    // Initiate Error handlers
    OSSetErrorHandler(OS_ERROR_PROTECTION, MyErrorHandler);
    OSSetErrorHandler(OS_ERROR_DSI, MyErrorHandler);
    OSSetErrorHandler(OS_ERROR_ISI, MyErrorHandler);
    // Set OS_ERROR_ALIGNMENT error handler 2x?
    // probably just copy and pasting?
    OSSetErrorHandler(OS_ERROR_ALIGNMENT, MyErrorHandler); // @bug Should this be 4 instead of 5?
    OSSetErrorHandler(OS_ERROR_ALIGNMENT, MyErrorHandler);
    
    DEMOInit(NULL);

    bHeapAllocated = true;

    // Initiate XFONT
    GXRenderModeObj* pRModeObj = DEMOGetRenderModeObj();
    XFONTInit(384, ((u16)ALIGN_UP((u16)pRModeObj->fbWidth, 16) * pRModeObj->xfbHeight) * sizeof(u16));
    XFONTSetFrameBuffer(DEMOGetCurrentBuffer());
    XFONTSetFgColor(0);

    zCheckRequests.Init(4, sizeof(ZCheckRequest));
    
    if (VIGetTvFormat() == VI_PAL) {
        System_SetDisplayMode(0, 512, 528, 0, 24);
    } else {
        System_SetDisplayMode(1, 512, 480, 0, 24);
    }
    
    // Set Idle function
    OSSetIdleFunction(System_IdleFunction, NULL, 
        IdleThreadStack + IDLE_THREAD_STACK_SIZE, IDLE_THREAD_STACK_SIZE);
    
    // Initiate GX
    Matrix mtx;
    mtx.SetIdentity();
    GXLoadPosMtxImm(mtx.data, GX_PNMTX1);

    GXSetLineWidth(6, GX_TO_ZERO);
    GXSetPointSize(4, GX_TO_ZERO);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaUpdate(true);
    GXSetCurrentMtx(GX_PNMTX0);
    GXSetDstAlpha(GX_ENABLE, 127);
    GXPokeZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
    GXPokeBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);

    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_S8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_S16, 12);
    
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxAttrFmt(GX_VTXFMT1, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    
    GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_CLR0, GX_CLR_RGB, GX_RGB565, 0);
    GXSetVtxAttrFmt(GX_VTXFMT2, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetChanAmbColor(GX_COLOR0A0, (GXColor){0, 0, 0, 0});
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);

    int i;
    int argStrLen = 0;
    for (i = 1; i < argc; i++) {
        argStrLen += strlen(argv[i]);
    }

    if (argc >= 2) {
        strcpy(cmdLine, argv[1]);
    }

    for (i = 2; i < argc; i++) {
        strcat(cmdLine, " ");
        strcat(cmdLine, argv[i]);
    }

    System_InitModule(NULL, cmdLine);
    System_SetOrthoSize(640, 512);
    Game_Init();
    System_DoGameLoop();
    Game_Deinit();

    // might be missing some functions which get inlined into main
    // because System_DeinitModule gets inlined currently
    // a pragma is currently used to force the function
    // not to inline
    System_DeinitModule();
    return 0;
}

extern "C" void Sound_DeinitModule(void);
extern "C" void OSResetSystem(int reset, u32 resetCode, BOOL forceMenu);
int MKMemoryCard_GetStatus(int);

static bool System_AnyMemCardBusy(void) {
    bool ret = false;

    for (int i = 0; i < 2; i++) {
        if (MKMemoryCard_GetStatus(i) == 1) {
            ret = true;
            break;
        }
    }

    return ret;
}

void System_DoReset(bool r3, bool r4, bool bDeinitSounds) {
    OSReport("Resetting system...\n");
    GXDrawDone();
    VISetBlack(1);
    VIFlush();
    VIWaitForRetrace();
    Input_StopAllVibration();

    if (bDeinitSounds) {
        Sound_DeinitModule();
    }

    PADRecalibrate(PAD_CHAN0_BIT | PAD_CHAN1_BIT | PAD_CHAN2_BIT | PAD_CHAN3_BIT);

    if (r3) {
        OSResetSystem(1, 0, r4);
    } else {
        OSResetSystem(0, 0, 0);
    }
}

extern void Sound_SysResumeVoices(void);
extern void Sound_SysPauseVoices(void);

void System_IdleFunction(void* r3) {
    while (true) {
        static u64 timePadLast = 0;
        static s64 timeDiscClosed = OSGetTime();
        
        static bool bDiscJustClosed = false;
        static bool bMainThreadSuspened = false;

        switch (DVDGetDriveStatus()) {
            case DVD_STATE_END:
            case DVD_STATE_BUSY:
                DiscErr_ClearError();
                if (OSIsThreadSuspended(pMainThread) && bMainThreadSuspened) {
                    Sound_SysResumeVoices();
                    OSResumeThread(pMainThread);
                    bMainThreadSuspened = false;
                }
                break;
            case DVD_STATE_FATAL_ERROR:
                DiscErr_SetError(0);
                Sound_SysPauseVoices();
                if (!bMainThreadSuspened) {
                    OSSuspendThread(pMainThread);
                    bMainThreadSuspened = true;
                }
                break;
            case DVD_STATE_WAITING:
                if (bDiscJustClosed) {
                    timeDiscClosed = OSGetTime();
                }
                break;
            case DVD_STATE_NO_DISK:
                DiscErr_SetError(1);
                Sound_SysPauseVoices();
                if (!bMainThreadSuspened) {
                    OSSuspendThread(pMainThread);
                    bMainThreadSuspened = true;
                }
                break;
            case DVD_STATE_COVER_OPEN:
                DiscErr_SetError(2);
                if (!bMainThreadSuspened) {
                    Sound_SysPauseVoices();
                    Input_StopAllVibration();
                    OSSuspendThread(pMainThread);
                    bMainThreadSuspened = true;
                }
                timeDiscClosed = OSGetTime();
                bDiscJustClosed = true;
                break;
            case DVD_STATE_WRONG_DISK:
                DiscErr_SetError(4);
                Sound_SysPauseVoices();
                if (!bMainThreadSuspened) {
                    OSSuspendThread(pMainThread);
                    bMainThreadSuspened = true;
                }
                break;
            case DVD_STATE_RETRY:
                DiscErr_SetError(3); // "Disk could not be read"
                Sound_SysPauseVoices();
                if (!bMainThreadSuspened) {
                    OSSuspendThread(pMainThread);
                    bMainThreadSuspened = true;
                }
                break;
        }

        if (DiscErr_IsError()) {
            DiscErr_DrawErrorString();
        } else if (bDiscJustClosed) {
            if (OSTicksToSeconds(OSGetTime() - timeDiscClosed) > 7) {
                bDiscJustClosed = false;
            }
        }

        int resetButtonState = OSGetResetButtonState();
        if (resetButtonState != 0) {
            resetState = 1;
        } else if (resetState == 1 || resetState == 3) {
            if (!System_AnyMemCardBusy()) {
                resetState = 2;

                if (DiscErr_IsError() || bDiscJustClosed) {
                    System_DoReset(true, false, false);
                } else {
                    System_DoReset(false, false, true);
                }

                resetState = 4;
                timePadLast = OSGetTime();
            } else {
                resetState = 3;
            }
        }

        if (resetState == 4) {
            break;
        }
    }

    OSSetIdleFunction(NULL, NULL, NULL, 0);
}
