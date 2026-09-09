#include "Dolphin/gx.h"
#include "Dolphin/mtx.h"

#include "Dolphin/gx/GXPriv.h"

void GXProject(f32 x, f32 y, f32 z, const f32 mtx[3][4], const f32 *pm, const f32 *vp, f32 *sx, f32 *sy, f32 *sz)
{
    Vec peye;
    f32 xc;
    f32 yc;
    f32 zc;
    f32 wc;

    ASSERTMSGLINE(0xA8, pm && vp && sx && sy && sz, "GXGet*: invalid null pointer");

    peye.x = mtx[0][3] + ((mtx[0][2] * z) + ((mtx[0][0] * x) + (mtx[0][1] * y)));
    peye.y = mtx[1][3] + ((mtx[1][2] * z) + ((mtx[1][0] * x) + (mtx[1][1] * y)));
    peye.z = mtx[2][3] + ((mtx[2][2] * z) + ((mtx[2][0] * x) + (mtx[2][1] * y)));
    if (pm[0] == 0.0f) {
        xc = (peye.x * pm[1]) + (peye.z * pm[2]);
        yc = (peye.y * pm[3]) + (peye.z * pm[4]);
        zc = pm[6] + (peye.z * pm[5]);
        wc = 1.0f / -peye.z;
    }
    else {
        xc = pm[2] + (peye.x * pm[1]);
        yc = pm[4] + (peye.y * pm[3]);
        zc = pm[6] + (peye.z * pm[5]);
        wc = 1.0f;
    }
    *sx = (vp[2] / 2.0f) + (vp[0] + (wc * (xc * vp[2] / 2.0f)));
    *sy = (vp[3] / 2.0f) + (vp[1] + (wc * (-yc * vp[3] / 2.0f)));
    *sz = vp[5] + (wc * (zc * (vp[5] - vp[4])));
}

inline void WriteProjPS(const register f32 src[6], register volatile void *dst) {
  register f32 ps_0, ps_1, ps_2;

  asm {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_st ps_0,  0(dst), 0, 0
        psq_st ps_1,  0(dst), 0, 0
        psq_st ps_2,  0(dst), 0, 0
  }
}

inline void Copy6Floats(const register f32 src[6], register f32 dst[6]) {
  register f32 ps_0, ps_1, ps_2;

  asm {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_st ps_0,  0(dst), 0, 0
        psq_st ps_1,  8(dst), 0, 0
        psq_st ps_2, 16(dst), 0, 0
  }
}

inline void __GXSetProjection(void) {
  u32 reg = 0x00061020;

  GX_WRITE_U8(0x10);
  GX_WRITE_U32(reg);
  GX_WRITE_F32(gx->projMtx[0]);
  GX_WRITE_F32(gx->projMtx[1]);
  GX_WRITE_F32(gx->projMtx[2]);
  GX_WRITE_F32(gx->projMtx[3]);
  GX_WRITE_F32(gx->projMtx[4]);
  GX_WRITE_F32(gx->projMtx[5]);
  GX_WRITE_U32(gx->projType);
}

void GXSetProjection(const Mtx44 proj, GXProjectionType type) {
  gx->projType = type;

  gx->projMtx[0] = proj[0][0];
  gx->projMtx[2] = proj[1][1];
  gx->projMtx[4] = proj[2][2];
  gx->projMtx[5] = proj[2][3];

  if (type == GX_ORTHOGRAPHIC) {
    gx->projMtx[1] = proj[0][3];
    gx->projMtx[3] = proj[1][3];
  } else {
    gx->projMtx[1] = proj[0][2];
    gx->projMtx[3] = proj[1][2];
  }

  __GXSetProjection();

  gx->bpSentNot = GX_TRUE;
}

void GXSetProjectionv(const f32 *proj) {
  gx->projType = proj[0];
  gx->projMtx[0] = proj[1];
  gx->projMtx[1] = proj[2];
  gx->projMtx[2] = proj[3];
  gx->projMtx[3] = proj[4];
  gx->projMtx[4] = proj[5];
  gx->projMtx[5] = proj[6];

  __GXSetProjection();
  gx->bpSentNot = GX_TRUE;
}

void GXGetProjectionv(f32* ptr) {
  ptr[0] = gx->projType;
  ptr[1] = gx->projMtx[0];
  ptr[2] = gx->projMtx[1];
  ptr[3] = gx->projMtx[2];
  ptr[4] = gx->projMtx[3];
  ptr[5] = gx->projMtx[4];
  ptr[6] = gx->projMtx[5];
}

static void WriteMTXPS4x3(register const Mtx src, register volatile void *dst) {
  register f32 ps_0, ps_1, ps_2, ps_3, ps_4, ps_5;

  asm {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_l  ps_3, 24(src), 0, 0
        psq_l  ps_4, 32(src), 0, 0
        psq_l  ps_5, 40(src), 0, 0

        psq_st ps_0, 0(dst),  0, 0
        psq_st ps_1, 0(dst),  0, 0
        psq_st ps_2, 0(dst),  0, 0
        psq_st ps_3, 0(dst),  0, 0
        psq_st ps_4, 0(dst),  0, 0
        psq_st ps_5, 0(dst),  0, 0
  }
}

static void WriteMTXPS3x3from3x4(register const Mtx src, register volatile void *dst) {
  register f32 ps_0, ps_1, ps_2, ps_3, ps_4, ps_5;

  asm {
        psq_l  ps_0,  0(src), 0, 0
        lfs    ps_1,  8(src)
        psq_l  ps_2, 16(src), 0, 0
        lfs    ps_3, 24(src)
        psq_l  ps_4, 32(src), 0, 0
        lfs    ps_5, 40(src)

        psq_st ps_0, 0(dst),  0, 0
        stfs   ps_1, 0(dst)
        psq_st ps_2, 0(dst),  0, 0
        stfs   ps_3, 0(dst)
        psq_st ps_4, 0(dst),  0, 0
        stfs   ps_5, 0(dst)
  }
}

static void WriteMTXPS4x2(register const Mtx src, register volatile void *dst) {
  register f32 ps_0, ps_1, ps_2, ps_3;

  asm {
        psq_l  ps_0,  0(src), 0, 0
        psq_l  ps_1,  8(src), 0, 0
        psq_l  ps_2, 16(src), 0, 0
        psq_l  ps_3, 24(src), 0, 0

        psq_st ps_0, 0(dst),  0, 0
        psq_st ps_1, 0(dst),  0, 0
        psq_st ps_2, 0(dst),  0, 0
        psq_st ps_3, 0(dst),  0, 0
  }
}

#pragma peephole off
#pragma dont_inline on

void GXLoadPosMtxImm(const Mtx mtx, u32 id) {
  u32 reg;
  u32 addr;

  addr = id * 4;
  reg = addr | 0xB0000;

  GX_WRITE_U8(0x10);
  GX_WRITE_U32(reg);
  WriteMTXPS4x3(mtx, &__GXWGFifo.f32);
}

void GXLoadNrmMtxImm(const Mtx mtx, u32 id) {
  u32 reg;
  u32 addr;

  addr = id * 3 + 0x400;
  reg = addr | 0x80000;

  GX_WRITE_U8(0x10);
  GX_WRITE_U32(reg);
  WriteMTXPS3x3from3x4((void *)mtx, &__GXWGFifo.f32);
}

void GXSetCurrentMtx(u32 id) {

  SET_REG_FIELD(gx->matIdxA, 6, 0, id);
  __GXSetMatrixIndex(GX_VA_PNMTXIDX);
}

void GXLoadTexMtxImm(const f32 mtx[][4], u32 id, GXTexMtxType type) {
  u32 reg;
  u32 addr;
  u32 count;

  if (id >= GX_PTTEXMTX0) {
    addr = (id - GX_PTTEXMTX0) * 4 + 0x500;
  } else {
    addr = id * 4;
  }
  count = (type == GX_MTX2x4) ? 8 : 12;
  reg = addr | ((count - 1) << 16);

  GX_WRITE_U8(0x10);
  GX_WRITE_U32(reg);

  if (type == GX_MTX3x4) {
    WriteMTXPS4x3(mtx, &__GXWGFifo.f32);
  } else {
    WriteMTXPS4x2(mtx, &__GXWGFifo.f32);
  }
}

void GXSetViewportJitter(f32 left, f32 top, f32 wd, f32 ht, f32 nearz, f32 farz, u32 field) {
  f32 sx;
  f32 sy;
  f32 sz;
  f32 ox;
  f32 oy;
  f32 oz;
  f32 zmin;
  f32 zmax;
  u32 reg;

  if (field == 0) {
    top -= 0.5f;
  }

  sx = wd / 2.0f;
  sy = -ht / 2.0f;
  ox = 342.0f + (left + (wd / 2.0f));
  oy = 342.0f + (top + (ht / 2.0f));
  zmin = 1.6777215e7f * nearz;
  zmax = 1.6777215e7f * farz;
  sz = zmax - zmin;
  oz = zmax;

  gx->vpLeft = left;
  gx->vpTop = top;
  gx->vpWd = wd;
  gx->vpHt = ht;
  gx->vpNearz = nearz;
  gx->vpFarz = farz;

  if (gx->fgRange != 0) {
    __GXSetRange(nearz, gx->fgSideX);
  }

  reg = 0x5101A;
  GX_WRITE_U8(0x10);
  GX_WRITE_U32(reg);
  GX_WRITE_F32(sx);
  GX_WRITE_F32(sy);
  GX_WRITE_F32(sz);
  GX_WRITE_F32(ox);
  GX_WRITE_F32(oy);
  GX_WRITE_F32(oz);

  gx->bpSentNot = GX_TRUE;
}

void GXSetViewport(f32 left, f32 top, f32 width, f32 height, f32 nearZ, f32 farZ) {
  GXSetViewportJitter(left, top, width, height, nearZ, farZ, 1);
}

void GXGetViewportv(f32* ptr) {
  ptr[0] = gx->vpLeft;
  ptr[1] = gx->vpTop;
  ptr[2] = gx->vpWd;
  ptr[3] = gx->vpHt;
  ptr[4] = gx->vpNearz;
  ptr[5] = gx->vpFarz;
}

void GXSetScissor(u32 left, u32 top, u32 wd, u32 ht) {
  u32 tp;
  u32 lf;
  u32 bm;
  u32 rt;

  tp = top + 342;
  lf = left + 342;
  bm = tp + ht - 1;
  rt = lf + wd - 1;

  SET_REG_FIELD(gx->suScis0, 11, 0, tp);
  SET_REG_FIELD(gx->suScis0, 11, 12, lf);
  SET_REG_FIELD(gx->suScis1, 11, 0, bm);
  SET_REG_FIELD(gx->suScis1, 11, 12, rt);

  GX_WRITE_RAS_REG(gx->suScis0);
  GX_WRITE_RAS_REG(gx->suScis1);
  gx->bpSentNot = 0;
}

void GXGetScissor(u32 *left, u32 *top, u32 *width, u32 *height) {
  u32 y1 = (gx->suScis0 & 0x0007FF) >> 0;
  u32 x1 = (gx->suScis0 & 0x7FF000) >> 12;
  u32 y2 = (gx->suScis1 & 0x0007FF) >> 0;
  u32 x2 = (gx->suScis1 & 0x7FF000) >> 12;

  *left = x1 - 0x156;
  *top = y1 - 0x156;
  *width = (x2 - x1) + 1;
  *height = (y2 - y1) + 1;
}

void GXSetScissorBoxOffset(s32 x_off, s32 y_off) {
  u32 reg = 0;
  u32 hx;
  u32 hy;

  hx = (u32)(x_off + 342) >> 1;
  hy = (u32)(y_off + 342) >> 1;

  SET_REG_FIELD(reg, 10, 0, hx);
  SET_REG_FIELD(reg, 10, 10, hy);
  SET_REG_FIELD(reg, 8, 24, 0x59);
  GX_WRITE_RAS_REG(reg);
  gx->bpSentNot = 0;
}

void GXSetClipMode(GXClipMode mode) {

  GX_WRITE_XF_REG(5, mode);
  gx->bpSentNot = 1;
}

void __GXSetMatrixIndex(GXAttr matIdxAttr) {
  if (matIdxAttr < GX_VA_TEX4MTXIDX) {
    GX_WRITE_SOME_REG4(8, 0x30, gx->matIdxA, -12);
    GX_WRITE_XF_REG(24, gx->matIdxA);
  } else {
    GX_WRITE_SOME_REG4(8, 0x40, gx->matIdxB, -12);
    GX_WRITE_XF_REG(25, gx->matIdxB);
  }
  gx->bpSentNot = 1;
}
