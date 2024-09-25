#ifndef _DOLPHIN_GXVERT
#define _DOLPHIN_GXVERT

#include <Dolphin/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GXFIFO_ADDR 0xCC008000

typedef union {
	u8 u8;
	u16 u16;
	u32 u32;
	u64 u64;
	s8 s8;
	s16 s16;
	s32 s32;
	s64 s64;
	f32 f32;
	f64 f64;
} PPCWGPipe;

volatile PPCWGPipe GXWGFifo : GXFIFO_ADDR;

extern volatile union {
    u8 c;
    short s;
    int i;
    void * p;
    float f;
} WGPIPE : 0xcc008000;

static inline void GXPosition3f32(f32 x, f32 y, f32 z) {
	GXWGFifo.f32 = x;
	GXWGFifo.f32 = y;
	GXWGFifo.f32 = z;
}

static inline void GXColor4u8(u8 r, u8 g, u8 b, u8 a) {
	GXWGFifo.u8 = r;
	GXWGFifo.u8 = g;
	GXWGFifo.u8 = b;
	GXWGFifo.u8 = a;
}

static inline void GXTexCoord2f32(f32 s, f32 t) {
	GXWGFifo.f32 = s;
	GXWGFifo.f32 = t;
}

static inline void GXColor1u16(u16 v) {
	GXWGFifo.u16 = v;
}

static inline void GXColor1u32(const u32 v) {
  GXWGFifo.u32 = v;
}

static inline void GXEnd(void) {}

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_GXVERT
