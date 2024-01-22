
#include "common/Video.h"
#include "common/Blitter.h"
#include "common/Heap.h"

extern "C" {
    void sprintf(char*, char*, ...);
};

extern u8 GetVideoVolume(void);
extern volatile THPPlayer ActivePlayer;

Video currentVideo;

bool bRenderTargetUpdated = false;

static char videoVolume = 127;

static int Video_GetYUVData(void**, void**, void**);

enum sound_state {};
static void Video_SwitchSoundState(sound_state);

void Video_InitModule(void) {
    currentVideo.bActive = false;
    currentVideo.pMem = false;
    currentVideo.pVideoMat = NULL;
}

void Video_DeinitModule(void) {
    THPPlayerStop();
    THPPlayerClose();
    THPPlayerQuit();
    if (currentVideo.pMem) {
        Heap_MemFree(currentVideo.pMem);
        currentVideo.pMem = NULL;
    }
    currentVideo.bActive = false;
}

Video* Video_CreateStreaming(char* pName, Material* pMaterial, bool, int track) {
    char buffer[256];
    Video_SwitchSoundState((sound_state)1);
    sprintf(buffer, "video/%s.thp", pName);
    THPPlayerOpen(buffer, 0);
    currentVideo.pMem = Heap_MemAlloc(THPPlayerCalcNeedMemory());
    THPPlayerSetBuffer((u8*)currentVideo.pMem);
    THPPlayerGetVideoInfo(&currentVideo.videoInfo);
    THPPlayerGetAudioInfo(&currentVideo.audioInfo);
    currentVideo.bActive = true;
    THPPlayerPrepare(0, 0, track);
    THPPlayerPlay();
    if (pMaterial == NULL) {
        // if the material parameter is NULL, create a new Mpeg material
        pMaterial = Material::CreateMpegTarget(
            pName, NULL,
            currentVideo.videoInfo.mXSize, currentVideo.videoInfo.mYSize
        );
    }
    currentVideo.pVideoMat = pMaterial;
    Texture* tex = pMaterial->unk54;
    void** ppTexYData = &tex->pYData;
    void** ppTexUData = &tex->pUData;
    void** ppTexVData = &tex->pVData;
    u32 end = ActivePlayer.mRetraceCount + 60;
    u64 start = ActivePlayer.mRetraceCount;
    while (!ActivePlayer.mDispTextureSet && start != end) {}
    Video_GetYUVData(ppTexYData, ppTexUData, ppTexVData);
    return &currentVideo;
}

bool Video_Update(Video* pVideo) {
    Texture* tex = currentVideo.pVideoMat->unk54;
    Video_GetYUVData(&tex->pYData, &tex->pUData, &tex->pVData);
    bRenderTargetUpdated = true;
    switch (THPPlayerGetState()) {
        case 1:
        case 2:
        case 4:
            THPPlayerSetVolume(GetVideoVolume(), 0);
            return true;
        case 3:
        case 0:
            return false;
        case 5:
            THPPlayerStop();
            THPPlayerClose();
            if (currentVideo.pMem) {
                Heap_MemFree(currentVideo.pMem);
                currentVideo.pMem = NULL;
            }
            break;
    }
    return false;
}

void Video_Destroy(Video* pVideo) {
    if (pVideo != NULL && currentVideo.bActive) {
        THPPlayerStop();
        THPPlayerClose();
        if (currentVideo.pMem) {
            Heap_MemFree(currentVideo.pMem);
            currentVideo.pMem = NULL;
        }
        if (currentVideo.pVideoMat) {
            currentVideo.pVideoMat->Destroy();
            currentVideo.pVideoMat = NULL;
        }
        currentVideo.bActive = false;
        Video_SwitchSoundState((sound_state)0);
    }
}

void Video_Draw(Video* pVideo, float z, float f2) {
    Blitter_Image img;

    if (!bRenderTargetUpdated) {
        Video_Update(pVideo);
    }
    currentVideo.pVideoMat->Use();

    float x = (f2 * 640.0f) * 0.5f;
    float y = (f2 * 512.0f) * 0.5f;
    
    img.color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    img.uv0 = 0.0f;
    img.uv1 = 0.0f;
    img.uv2 = 1.0f;
    img.uv3 = 1.0f;

    img.startX = -x;
    img.startY = -y;
    img.endX = x + 640.0f;
    img.endY = y + 512.0f;
    img.z = z;
    img.Draw(1);

    bRenderTargetUpdated = false;
}

static int Video_GetYUVData(void** ppYTexture, void** ppUTexture, void** ppVTexture) {
    if (ActivePlayer.mIsOpen && ActivePlayer.mState != 0 && ActivePlayer.mDispTextureSet) {
        *ppYTexture = ActivePlayer.mDispTextureSet->mYTexture;
        *ppUTexture = ActivePlayer.mDispTextureSet->mUTexture;
        *ppVTexture = ActivePlayer.mDispTextureSet->mVTexture;
        return (ActivePlayer.mDispTextureSet->mFrameNumber + ActivePlayer.mInitReadFrame) % 
            ActivePlayer.mHeader.mNumFrames;
    }
    return -1;
}

extern int Sound_CurrentState;

static void Video_SwitchSoundState(sound_state state) {
    if (state == Sound_CurrentState) {
        return;
    }
    switch (state) {
        case 1:
            videoVolume = GetVideoVolume();
            THPPlayerInit(1);
            Sound_CurrentState = 1;
            break;
        case 0:
            THPPlayerQuit();
            Sound_CurrentState = 0;
            break;
    }
}
