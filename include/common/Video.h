#ifndef COMMON_VIDEO
#define COMMON_VIDEO

#include "Dolphin/thp/THPPlayer.h"
#include "common/Material.h"

struct Video {
    bool bActive;
    void* pMem;
    THPVideoInfo videoInfo;
    THPAudioInfo audioInfo;
    Material* pVideoMat;
};

void Video_InitModule(void);
void Video_DeinitModule(void);
Video* Video_CreateStreaming(char* pName, Material* pMaterial, bool, int track);
bool Video_Update(Video* pVideo);
void Video_Destroy(Video* pVideo);
void Video_Draw(Video* pVideo, float z, float f2);

#endif // COMMON_VIDEO
