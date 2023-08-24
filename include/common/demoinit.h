#ifndef COMMON_DEMOINIT
#define COMMON_DEMOINIT

#include "Dolphin/gx.h"

extern u32 gCPUCycles;
extern u32 gGXCycles;

extern "C" void DEMOInit(_GXRenderModeObj* prmodeObj);
extern "C" void DEMOBeforeRender(void);
extern "C" void DEMODoneRender(void);
extern "C" GXRenderModeObj* DEMOGetRenderModeObj(void);
extern "C" void* DEMOGetCurrentBuffer(void);

#endif // COMMON_DEMOINIT
