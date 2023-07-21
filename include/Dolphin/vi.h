#ifndef _DOLPHIN_VI
#define _DOLPHIN_VI

#include <Dolphin/gx/GXStruct.h>
#include <Dolphin/vifuncs.h>

#ifdef __cplusplus
extern "C" {
#endif

void VIInit(void);
void VIConfigure(GXRenderModeObj* rm);
void VIFlush(void);
u32 VIGetTvFormat(void);
void VISetNextFrameBuffer(void* fb);
void VIWaitForRetrace(void);
void VISetBlack(BOOL black);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_VI
