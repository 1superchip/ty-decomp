#ifndef _DOLPHIN_GX_H
#define _DOLPHIN_GX_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif // ifdef __cplusplus

typedef struct GXColor {
	u8 r, g, b, a;
} GXColor;

typedef struct GXColorS10 {
	s16 r, g, b, a;
} GXColorS10;

struct Mtx44 {
    float data[4][4];
};

struct Mtx24 {
    float data[2][4];
};

struct Mtx23 {
    float data[2][3];
};

struct GXTexObj {
    int unk[8];
};

struct GXTlutObj {
    u32 data[3];
};

struct GXLightObj {
    int unk0[3];
    uint color;
    uint unk10[12];
};

// Compare types.
typedef enum _SDK_GXCompare {
	GX_NEVER,   // Always false.
	GX_LESS,    //<
	GX_EQUAL,   //=
	GX_LEQUAL, //<=
	GX_GREATER, //>
	GX_NEQUAL, //!=
	GX_GEQUAL, //>=
	GX_ALWAYS   // Always true.
} GXCompare;

#define _GX_TF_CTF 0x20
#define _GX_TF_ZTF 0x10

typedef enum _GXTexFmt {
    GX_TF_I4 = 0x0,
    GX_TF_I8 = 0x1,
    GX_TF_IA4 = 0x2,
    GX_TF_IA8 = 0x3,
    GX_TF_RGB565 = 0x4,
    GX_TF_RGB5A3 = 0x5,
    GX_TF_RGBA8 = 0x6,
	GX_TF_CI8 = 0x9,
    GX_TF_CMPR = 0xE,

    GX_CTF_R4 = 0x0 | _GX_TF_CTF,
    GX_CTF_RA4 = 0x2 | _GX_TF_CTF,
    GX_CTF_RA8 = 0x3 | _GX_TF_CTF,
    GX_CTF_YUVA8 = 0x6 | _GX_TF_CTF,
    GX_CTF_A8 = 0x7 | _GX_TF_CTF,
    GX_CTF_R8 = 0x8 | _GX_TF_CTF,
    GX_CTF_G8 = 0x9 | _GX_TF_CTF,
    GX_CTF_B8 = 0xA | _GX_TF_CTF,
    GX_CTF_RG8 = 0xB | _GX_TF_CTF,
    GX_CTF_GB8 = 0xC | _GX_TF_CTF,

    GX_TF_Z8 = 0x1 | _GX_TF_ZTF,
    GX_TF_Z16 = 0x3 | _GX_TF_ZTF,
    GX_TF_Z24X8 = 0x6 | _GX_TF_ZTF,

    GX_CTF_Z4 = 0x0 | _GX_TF_ZTF | _GX_TF_CTF,
    GX_CTF_Z8M = 0x9 | _GX_TF_ZTF | _GX_TF_CTF,
    GX_CTF_Z8L = 0xA | _GX_TF_ZTF | _GX_TF_CTF,
    GX_CTF_Z16L = 0xC | _GX_TF_ZTF | _GX_TF_CTF,

    GX_TF_A8 = GX_CTF_A8,
} GXTexFmt;

/*
typedef enum _GXTexFmt {
	GX_TF_I4,		// 4-bit intensity
	GX_TF_I8,		// 8-bit intensity
	GX_TF_IA4,		// 8-bit intensity + alpha (4+4).
	GX_TF_IA8,		// 16-bit intensity + alpha (8+8).
	GX_TF_RGB565,	// 16-bit RGB.
	GX_TF_RGB5A3,	// When MSB=1, RGB555 format (opaque), and when MSB=0, RGBA4443 format (transparent).
	GX_TF_RGBA8,	// 32-bit RGB.
	GX_TF_CMPR,		// Compressed 4-bit texel.
	GX_TF_Z8,		// Unsigned 8-bit Z. For texture copies, specify the upper 8 bits of Z.
	GX_TF_Z16,		// Unsigned 16-bit Z. For texture copies, specify the upper 16 bits of Z.
	GX_TF_Z24X8,	// Unsigned 24-bit (32-bit texture) Z. For texture copies, copy the 24-bit Z and 0xff.
	GX_CTF_R4,		// 4-bit red. For copying 4 bits from red.
	GX_CTF_RA4,		// 4-bit red + 4-bit alpha. For copying 4 bits from red, 4 bits from alpha.
	GX_CTF_RA8,		// 8-bit red + 8-bit alpha. For copying 8 bits from red, 8 bits from alpha.
	GX_CTF_A8,		// 8-bit alpha. For copying 8 bits from alpha.
	GX_CTF_R8,		// 8-bit red. For copying 8 bits from red.
	GX_CTF_G8,		// 8-bit green. For copying 8 bits from green.
	GX_CTF_B8,		// 8-bit blue. For copying 8 bits from blue.
	GX_CTF_RG8,		// 8-bit red +8-bit green. For copying 8 bits from red, 8 bits from green.
	GX_CTF_GB8,		// 8-bit green +8-bit blue. For copying 8 bits from green, 8 bits from blue.
	GX_CTF_Z4,		// 4-bit Z. For copying the 4 upper bits from Z.
	GX_CTF_Z8M,		// 8-bit Z (median byte). For copying the middle 8 bits of Z.
	GX_CTF_Z8L,		// 8-bit Z (lower byte). For copying the lower 8 bits of Z.
	GX_CTF_Z16L		// 16-bit Z (lower portion). For copying the lower 16 bits of Z.
} GXTexFmt;
*/

void GXSetTevColor(int, GXColor);
void GXSetTevDirect(int);
void GXSetDstAlpha(int, u8);
void GXSetBlendMode(int, int, int, int);
void GXSetAlphaCompare(int, int, int, int, int);
void GXSetTexCoordGen2(int, int, int, int, int, int);
void GXSetTevKColorSel(int, int);
void GXSetTevKAlphaSel(int, int);
void GXSetTevSwapMode(int, int, int);
void GXSetTevColorS10(int, GXColorS10);
void GXSetTevKColor(int, GXColor);
void GXInitTexObj(GXTexObj*, void*, u16, u16, GXTexFmt, u8, u8, u8);
void GXInitTexObjLOD(GXTexObj*, float, float, float, int, int, uint, char, int);
void GXLoadTexObj(GXTexObj*, int);
void GXLoadTlut(GXTlutObj*, int);
void GXSetNumTexGens(u8);
void GXSetNumTevStages(u8);
void GXSetNumIndStages(u8);
void GXLoadTexMtxImm(float*, int, int); // takes a Matrix pointer 2x4 or 3x4
void GXSetTexCoordGen2(int, int, int, int, int, int);
// next two functions aren't from GX?
void C_MTXOrtho(Mtx44* m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
void C_MTXIdentity(Mtx44*);
void GXSetIndTexMtx(int, float*, u8);
void GXSetIndTexOrder(int, int, int);
void GXSetTevIndWarp(int, int, int, int, int);
void GXSetIndTexCoordScale(int, int, int);
void GXSetBlendMode(int, int, int, int);
void GXSetZMode(u8, GXCompare, u8);
void GXSetZCompLoc(bool);
void GXSetTevOp(int, int);
void GXSetColorUpdate(int);
void GXInitTexObjWrapMode(GXTexObj *, int, int);
void GXGetTexObjAll(GXTexObj*, void**, u16*, u16*, u8*, u8*, u8*, u8*);
void GXBegin(int, int, int);
void GXGetProjectionv(float*);
void GXSetProjectionv(float*);
void GXSetProjection(float*, int);
void GXClearVtxDesc(void);
void GXSetVtxDesc(int, int);
void GXSetTexCopySrc(int, int, int, int);
void GXSetTexCopyDst(int, int, GXTexFmt, int);
void GXCopyTex(void*, int);
void GXSetCopyFilter(u8, u8*, int, u8*);
void GXSetCurrentMtx(int);
void GXInitTlutObj(GXTlutObj*, void*, int, int);
void GXInitTexObjCI(GXTexObj*, void*, u16, u16, GXTexFmt, u8, u8, u32);
void GXSetTevOrder(int, int, int, int);
void GXSetTevColorIn(int, int, int, int, int);
void GXSetTevColorOp(int, int, int, int, int, int);
void GXSetTevAlphaIn(int, int, int, int, int);
void GXSetTevAlphaOp(int, int, int, int, int, int);
void GXLoadPosMtxImm(float*, int);
void GXLoadNrmMtxImm(float*, int);
void GXInitLightColor(GXLightObj*, GXColor);
void GXInitLightPos(GXLightObj*, float, float, float);
void GXLoadLightObjImm(GXLightObj*, int);
void GXSetChanAmbColor(int, GXColor);
void GXSetAlphaUpdate(int);
void GXSetFog(u8, GXColor, f32, f32, f32, f32);
void GXSetArray(int, void*, int);
void GXSetChanMatColor(int, GXColor);
void GXSetChanCtrl(int, int, int, int, int, int, int);
void GXCallDisplayList(void*, int);

extern volatile union {
    u8 c;
    short s;
    int i;
    void * p;
    float f;
} WGPIPE : 0xcc008000;

static inline void GXPosition3f32(f32 x,f32 y,f32 z)
{
	WGPIPE.f = x;
	WGPIPE.f = y;
	WGPIPE.f = z;
}

static inline void GXColor4u8(u8 r,u8 g,u8 b,u8 a)
{
	WGPIPE.c = r;
	WGPIPE.c = g;
	WGPIPE.c = b;
	WGPIPE.c = a;
}

static inline void GXTexCoord2f32(f32 s,f32 t)
{
	WGPIPE.f = s;
	WGPIPE.f = t;
}

#ifdef __cplusplus
};
#endif // ifdef __cplusplus

#endif