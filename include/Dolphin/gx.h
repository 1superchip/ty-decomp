#ifndef _DOLPHIN_GX
#define _DOLPHIN_GX

// Not sure where this would go
// void GXSetProjectionv(float*);

struct Mtx44 {
    float data[4][4];
};

struct Mtx23 {
    float data[2][3];
};

struct Mtx24 {
    float data[2][4];
};

#include <Dolphin/gx/GXEnum.h>
#include <Dolphin/gx/GXStruct.h>

#include <Dolphin/gx/GXBump.h>
#include <Dolphin/gx/GXCommandList.h>
#include <Dolphin/gx/GXCull.h>
#include <Dolphin/gx/GXDispList.h>
// #include <Dolphin/gx/GXDraw.h>
#include <Dolphin/gx/GXFifo.h>
#include <Dolphin/gx/GXFrameBuffer.h>
#include <Dolphin/gx/GXGeometry.h>
#include <Dolphin/gx/GXGet.h>
#include <Dolphin/gx/GXLighting.h>
#include <Dolphin/gx/GXManage.h>
// #include <Dolphin/gx/GXPerf.h>
#include <Dolphin/gx/GXPixel.h>
#include <Dolphin/gx/GXTev.h>
#include <Dolphin/gx/GXTexture.h>
#include <Dolphin/gx/GXTransform.h>
#include <Dolphin/gx/GXVert.h>

#endif // _DOLPHIN_GX
