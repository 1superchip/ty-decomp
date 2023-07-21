#ifndef _DOLPHIN_VIFUNCS
#define _DOLPHIN_VIFUNCS

#include <Dolphin/types.h>

#include <Dolphin/vitypes.h>

#ifdef __cplusplus
extern "C" {
#endif

u32 VIGetNextField(void);
VIRetraceCallback VISetPreRetraceCallback(VIRetraceCallback callback);
VIRetraceCallback VISetPostRetraceCallback(VIRetraceCallback callback);
u32 VIGetDTVStatus(void);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_VIFUNCS
