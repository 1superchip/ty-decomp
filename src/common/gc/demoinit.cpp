#include "Dolphin/gx.h"
#include "Dolphin/os.h"
#include "Dolphin/vi.h"

extern "C" {
    void OSInit(void);
    void DVDInit(void);
    void memset(void*, int, int);
    void GXPokeARGB(u16, u16, int);
    u32 OSGetPhysicalMemSize(void);
    u32 OSGetConsoleSimulatedMemSize(void);
    float GXGetYScaleFactor(int, int);
    u32 VIGetRetraceCount(void);
};

extern _GXRenderModeObj GXNtsc480IntDf;
extern _GXRenderModeObj GXPal528IntDf;
extern _GXRenderModeObj GXMpal480IntDf;

#define ALIGN_UP(x, a) (((x) + (a - 1)) & ~(a - 1))

// extern u8 rawCaptureTexData[0x20000];
extern u16 rawCaptureTexData[0x10000];
void System_CheckZRequests(void);
extern u8 gMKDefaults[];

static void __DEMOInitRenderMode(_GXRenderModeObj*);
static void __DEMOInitMem(void);
static void __DEMOInitGX(void);

static char pBuffer[0x84000] ATTRIBUTE_ALIGN(32);
static char rmodeobj[60];

static void* DefaultFifo;
static GXFifoObj* DefaultFifoObj;
static GXRenderModeObj* rmode;
static bool bNeedAFlip;
int gCPUCycles;
int gGXCycles;

static void RetraceCallback(u32 param_1) {
    if (bNeedAFlip) {
        GXCopyDisp(pBuffer, 1);
        GXFlush();
        bNeedAFlip = false;
    }
}

extern "C" void DEMOInit(_GXRenderModeObj* prmodeObj) {
    OSInit();
    DVDInit();
    VIInit();
    __DEMOInitRenderMode(prmodeObj);
    __DEMOInitMem();
    VIConfigure(rmode);
    DefaultFifo = OSAllocFromHeap(__OSCurrHeap, 0xc8020);
    memset(DefaultFifo, 0, 0xc8020);
    DCFlushRange(DefaultFifo, 0xc8020);
    DefaultFifoObj = GXInit(DefaultFifo, 0xc8000);
    __DEMOInitGX();
    VISetBlack(1);
    VISetNextFrameBuffer(pBuffer);
    VIFlush();
    VIWaitForRetrace();
    VIWaitForRetrace();
    VISetPostRetraceCallback(RetraceCallback);
    VISetBlack(0);
    VIFlush();
    GXDrawDone();
    char* rawCaptureData = (char*)rawCaptureTexData + 0x130;
    for(int i = 160; i < 288; i++) {
        for(u32 j = 192; j < 320; j++) {
            u16 pixel = rawCaptureTexData[*(u8*)((int)rawCaptureData ^ 3) ^ 1];
            u32 finalPixel = (0xFF000000 | ((pixel << 19) & 0xF80000)) | 
                ((pixel >> 5) << 11) & 0xF800 | (((pixel >> 10) << 3) & 0xF8);
            GXPokeARGB(j, i, finalPixel);
            rawCaptureData++;
        }
    }
    bNeedAFlip = true;
}

static void  __DEMOInitRenderMode(_GXRenderModeObj* pRMode) {
    if (pRMode != NULL) {
        rmode = pRMode;
    } else {
        switch (VIGetTvFormat()) {
            case 0:
                rmode = &GXNtsc480IntDf;
                break;
            case 1:
                rmode = &GXPal528IntDf;
                break;
            case 2:
                rmode = &GXMpal480IntDf;
                break;
            default:
                OSPanic("demoinit.cpp", 0xed, "DEMOInit: invalid TV format\n");
        }
        rmode->fbWidth = 0x200;
        GXAdjustForOverscan(rmode, (GXRenderModeObj*)rmodeobj, 0, 0x10);
        rmode = (GXRenderModeObj*)&rmodeobj;
    }
}

static void __DEMOInitMem(void) {
    OSReport("Simulated Memory Size = %dMB\nPhysical Memory Size = %dMB\n",
        OSGetConsoleSimulatedMemSize() / 0x100000, OSGetPhysicalMemSize() / 0x100000);
    void* lo = OSGetArenaLo();
    void* hi = OSGetArenaHi();
    void* arenaLo = OSInitAlloc(lo, hi, 1);
    OSSetArenaLo(arenaLo);
    arenaLo = (void *)ALIGN_UP((int)arenaLo, 32);
    OSSetCurrentHeap(OSCreateHeap(arenaLo, (void*)((int)hi & ~(32 - 1))));
    OSSetArenaLo((void*)((int)hi & ~(32 - 1)));
    memset(pBuffer,0,sizeof(pBuffer));
    DCStoreRange(pBuffer, sizeof(pBuffer));
}

static void __DEMOInitGX(void) {
    GXSetViewport(0.0f, 0.0f, (float)rmode->fbWidth, (float)rmode->efbHeight, 0.0f, 1.0f);
    GXSetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    u16 r31 = GXSetDispCopyYScale(GXGetYScaleFactor(rmode->efbHeight, rmode->xfbHeight));
    GXSetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GXSetDispCopyDst(rmode->fbWidth, r31);
    GXSetCopyFilter(rmode->aa, rmode->sample_pattern, 1, rmode->vfilter);
    if (rmode->aa != 0) {
        GXSetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
    } else {
        GXColor copyClrColor = {0, 0, 0, 127};
        GXSetCopyClear(copyClrColor, 0xffffff);
        GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    }
    GXCopyDisp(pBuffer, 1);
    GXSetDispCopyGamma(GX_GM_1_0);
}

extern "C" void DEMOBeforeRender(void) {
    if (rmode->field_rendering) {
        GXSetViewportJitter(0.0f, 0.0f, (float)rmode->fbWidth, (float)rmode->efbHeight, 0.0f, 1.0f, VIGetNextField());
    } else {
        GXSetViewport(0.0f, 0.0f, (float)rmode->fbWidth, (float)rmode->efbHeight, 0.0f, 1.0f);
    }
    GXInvalidateVtxCache();
    GXInvalidateTexAll();
}

extern "C" void DEMODoneRender(void) {
    static int lastCPUCycles = 0;
    gCPUCycles = OSTicksToCycles(OSGetTick()) - lastCPUCycles;
    GXDrawDone();
    System_CheckZRequests();
    gGXCycles = OSTicksToCycles(OSGetTick()) - lastCPUCycles;
    if (gMKDefaults[0x10] != 0 && VIGetRetraceCount() % 2) {
        VIWaitForRetrace();
    }
    bNeedAFlip = true;
    VIWaitForRetrace();
    lastCPUCycles = OSTicksToCycles(OSGetTick());
}


extern "C" GXRenderModeObj* DEMOGetRenderModeObj(void) {
    return rmode;
}

extern "C" void* DEMOGetCurrentBuffer(void) {
    return pBuffer;
}