#include "Dolphin/gx.h"
#include "Dolphin/gx/GXPriv.h"

extern float cosf(float x);
extern float sqrtf(float x);

#define GX_LARGE_NUMBER 1.0e+18f;

void GXInitLightAttn(GXLightObj *lt_obj, f32 a0, f32 a1, f32 a2, f32 k0, f32 k1, f32 k2) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  obj->a0 = a0;
  obj->a1 = a1;
  obj->a2 = a2;
  obj->k0 = k0;
  obj->k1 = k1;
  obj->k2 = k2;
}

void GXInitLightAttnA(GXLightObj *lt_obj, f32 a0, f32 a1, f32 a2) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  obj->a0 = a0;
  obj->a1 = a1;
  obj->a2 = a2;
}

void GXGetLightAttnA(const GXLightObj *lt_obj, f32 *a0, f32 *a1, f32 *a2) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  *a0 = obj->a0;
  *a1 = obj->a1;
  *a2 = obj->a2;
}

void GXInitLightAttnK(GXLightObj *lt_obj, f32 k0, f32 k1, f32 k2) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  obj->k0 = k0;
  obj->k1 = k1;
  obj->k2 = k2;
}

void GXGetLightAttnK(const GXLightObj *lt_obj, f32 *k0, f32 *k1, f32 *k2) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  *k0 = obj->k0;
  *k1 = obj->k1;
  *k2 = obj->k2;
}

#define PI 3.14159265358979323846F

void GXInitLightSpot(GXLightObj *lt_obj, f32 cutoff, GXSpotFn spot_func) {
  f32 a0, a1, a2, r, d, cr;
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  if (cutoff <= 0.0f || cutoff > 90.0f) {
    spot_func = GX_SP_OFF;
  }

  r = cutoff * PI / 180.0f;
  cr = cosf(r);

  switch (spot_func) {
  case GX_SP_FLAT:
    a0 = -1000.0f * cr;
    a1 = 1000.0f;
    a2 = 0.0f;
    break;
  case GX_SP_COS:
    a1 = 1.0f / (1.0f - cr);
    a0 = -cr * a1;
    a2 = 0.0f;
    break;
  case GX_SP_COS2:
    a2 = 1.0f / (1.0f - cr);
    a0 = 0.0f;
    a1 = -cr * a2;
    break;
  case GX_SP_SHARP:
    d = 1.0F / ((1.0F - cr) * (1.0F - cr));
    a0 = cr * (cr - 2.0F) * d;
    a1 = 2.0F * d;
    a2 = -d;
    break;
  case GX_SP_RING1:
    d = 1.0f / ((1.0f - cr) * (1.0F - cr));
    a2 = -4.0f * d;
    a0 = a2 * cr;
    a1 = 4.0f * (1.0f + cr) * d;
    break;
  case GX_SP_RING2:
    d = 1.0f / ((1.0f - cr) * (1.0F - cr));
    a0 = 1.0f - 2.0f * cr * cr * d;
    a1 = 4.0f * cr * d;
    a2 = -2.0f * d;
    break;
  case GX_SP_OFF:
  default:
    a0 = 1.0f;
    a1 = 0.0f;
    a2 = 0.0f;
    break;
  }

  obj->a0 = a0;
  obj->a1 = a1;
  obj->a2 = a2;
}

void GXInitLightDistAttn(GXLightObj *lt_obj, f32 ref_dist, f32 ref_br, GXDistAttnFn dist_func) {
  f32 k0, k1, k2;
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  if (ref_dist < 0.0F) {
    dist_func = GX_DA_OFF;
  }

  if (ref_br <= 0.0F || ref_br >= 1.0F) {
    dist_func = GX_DA_OFF;
  }

  switch (dist_func) {
  case GX_DA_GENTLE:
    k0 = 1.0F;
    k1 = (1.0F - ref_br) / (ref_br * ref_dist);
    k2 = 0.0F;
    break;
  case GX_DA_MEDIUM:
    k0 = 1.0f;
    k1 = 0.5f * (1.0f - ref_br) / (ref_br * ref_dist);
    k2 = 0.5f * (1.0f - ref_br) / (ref_br * ref_dist * ref_dist);
    break;
  case GX_DA_STEEP:
    k0 = 1.0f;
    k1 = 0.0f;
    k2 = (1.0f - ref_br) / (ref_br * ref_dist * ref_dist);
    break;
  case GX_DA_OFF:
  default:
    k0 = 1.0f;
    k1 = 0.0f;
    k2 = 0.0f;
    break;
  }

  obj->k0 = k0;
  obj->k1 = k1;
  obj->k2 = k2;
}

void GXInitLightPos(GXLightObj *lt_obj, f32 x, f32 y, f32 z) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  obj->px = x;
  obj->py = y;
  obj->pz = z;
}

void GXGetLightPos(const GXLightObj *lt_obj, f32 *x, f32 *y, f32 *z) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  *x = obj->px;
  *y = obj->py;
  *z = obj->pz;
}

void GXInitLightDir(GXLightObj *lt_obj, f32 nx, f32 ny, f32 nz) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  obj->nx = -nx;
  obj->ny = -ny;
  obj->nz = -nz;
}

void GXGetLightDir(const GXLightObj *lt_obj, f32 *nx, f32 *ny, f32 *nz) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  *nx = -(obj->nx);
  *ny = -(obj->ny);
  *nz = -(obj->nz);
}

void GXInitSpecularDir(GXLightObj *lt_obj, f32 nx, f32 ny, f32 nz) {
  f32 mag;
  f32 vx, vy, vz;
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  vx = -nx;
  vy = -ny;
  vz = (-nz + 1.0F);
  mag = vx * vx + vy * vy + vz * vz;

  if (mag != 0.0f) {
    mag = 1.0f / sqrtf(mag);
  }

  obj->px = vx * mag;
  obj->py = vy * mag;
  obj->pz = vz * mag;

  obj->nx = nx * -GX_LARGE_NUMBER;
  obj->ny = ny * -GX_LARGE_NUMBER;
  obj->nz = nz * -GX_LARGE_NUMBER;
}

void GXInitSpecularDirHA(GXLightObj *lt_obj, f32 nx, f32 ny, f32 nz, f32 hx, f32 hy, f32 hz) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;

  obj->nx = hx;
  obj->ny = hy;
  obj->nz = hz;

  obj->px = nx * -GX_LARGE_NUMBER;
  obj->py = ny * -GX_LARGE_NUMBER;
  obj->pz = nz * -GX_LARGE_NUMBER;
}

void GXInitLightColor(GXLightObj *lt_obj, GXColor color) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  obj->color = (color.r << 24) | (color.g << 16) | (color.b << 8) | color.a;
}

void GXGetLightColor(const GXLightObj *lt_obj, GXColor *color) {
  GXLightObjInt *obj = (GXLightObjInt *)lt_obj;
  *(u32 *)color = obj->color;
}

static inline void PushLight(const register GXLightObjInt *lt_obj, register void *dest) {
  register u32 zero, color;
  register f32 a0_a1, a2_k0, k1_k2;
  register f32 px_py, pz_dx, dy_dz;

  asm
      {
        lwz     color, 12(lt_obj)
        xor     zero, zero, zero
        psq_l   a0_a1, 16(lt_obj), 0, 0
        psq_l   a2_k0, 24(lt_obj), 0, 0
        psq_l   k1_k2, 32(lt_obj), 0, 0
        psq_l   px_py, 40(lt_obj), 0, 0
        psq_l   pz_dx, 48(lt_obj), 0, 0
        psq_l   dy_dz, 56(lt_obj), 0, 0
        
        stw     zero,  0(dest)
        stw     zero,  0(dest)
        stw     zero,  0(dest)
        stw     color, 0(dest)
        psq_st  a0_a1, 0(dest), 0, 0
        psq_st  a2_k0, 0(dest), 0, 0
        psq_st  k1_k2, 0(dest), 0, 0
        psq_st  px_py, 0(dest), 0, 0
        psq_st  pz_dx, 0(dest), 0, 0
        psq_st  dy_dz, 0(dest), 0, 0
      }
}

#if DEBUG
#define WRITE_SOME_LIGHT_REG1(val, addr)                                                                                                             \
    do {                                                                                                                                             \
        u32 xfData = val;                                                                                                                            \
        GX_WRITE_U32(val);                                                                                                                           \
        VERIF_MTXLIGHT(addr, xfData);                                                                                                                \
    } while (0)

#define WRITE_SOME_LIGHT_REG2(val, addr)                                                                                                             \
    do {                                                                                                                                             \
        f32 xfData = val;                                                                                                                            \
        GX_WRITE_F32(val);                                                                                                                           \
        VERIF_MTXLIGHT(addr, *(u32 *)&xfData);                                                                                                       \
    } while (0)
#else
#define WRITE_SOME_LIGHT_REG1(val, addr) GX_WRITE_U32(val)
#define WRITE_SOME_LIGHT_REG2(val, addr) GX_WRITE_F32(val)
#endif

void GXLoadLightObjImm(GXLightObj *lt_obj, GXLightID light) {
    unsigned long addr;
    unsigned long idx;
    struct GXLightObjInt *obj;

    obj = (struct GXLightObjInt *)lt_obj;

    switch (light) {
        case GX_LIGHT0:
            idx = 0;
            break;
        case GX_LIGHT1:
            idx = 1;
            break;
        case GX_LIGHT2:
            idx = 2;
            break;
        case GX_LIGHT3:
            idx = 3;
            break;
        case GX_LIGHT4:
            idx = 4;
            break;
        case GX_LIGHT5:
            idx = 5;
            break;
        case GX_LIGHT6:
            idx = 6;
            break;
        case GX_LIGHT7:
            idx = 7;
            break;
        default:
            idx = 0;
            break;
    }

    addr = idx * 0x10 + 0x600;
    GX_WRITE_U8(0x10);
    GX_WRITE_U32(addr | 0xF0000);

    WRITE_SOME_LIGHT_REG1(0, addr);
    WRITE_SOME_LIGHT_REG1(0, addr + 1);
    WRITE_SOME_LIGHT_REG1(0, addr + 2);
    WRITE_SOME_LIGHT_REG1(obj->color, addr + 3);
    WRITE_SOME_LIGHT_REG2(obj->a0, addr + 4);
    WRITE_SOME_LIGHT_REG2(obj->a1, addr + 5);
    WRITE_SOME_LIGHT_REG2(obj->a2, addr + 6);
    WRITE_SOME_LIGHT_REG2(obj->k0, addr + 7);
    WRITE_SOME_LIGHT_REG2(obj->k1, addr + 8);
    WRITE_SOME_LIGHT_REG2(obj->k2, addr + 9);
    WRITE_SOME_LIGHT_REG2(obj->px, addr + 10);
    WRITE_SOME_LIGHT_REG2(obj->py, addr + 11);
    WRITE_SOME_LIGHT_REG2(obj->pz, addr + 12);
    WRITE_SOME_LIGHT_REG2(obj->nx, addr + 13);
    WRITE_SOME_LIGHT_REG2(obj->ny, addr + 14);
    WRITE_SOME_LIGHT_REG2(obj->nz, addr + 15);

    gx->bpSentNot = 1;
}

void GXSetChanAmbColor(GXChannelID chan, GXColor amb_color) {
  u32 reg = 0;
  u32 rgb;
  u32 colIdx;
  u32 alpha;

  switch (chan) {
      case GX_COLOR0:
          alpha = gx->ambColor[0] & 0xFF;
          SET_REG_FIELD(reg, 8, 0, alpha);
          SET_REG_FIELD(reg, 8, 8, amb_color.b);
          SET_REG_FIELD(reg, 8, 16, amb_color.g);
          SET_REG_FIELD(reg, 8, 24, amb_color.r);
          colIdx = 0;
          break;
      case GX_COLOR1:
          alpha = gx->ambColor[1] & 0xFF;
          SET_REG_FIELD(reg, 8, 0, alpha);
          SET_REG_FIELD(reg, 8, 8, amb_color.b);
          SET_REG_FIELD(reg, 8, 16, amb_color.g);
          SET_REG_FIELD(reg, 8, 24, amb_color.r);
          colIdx = 1;
          break;
      case GX_ALPHA0:
          reg = gx->ambColor[0];
          SET_REG_FIELD(reg, 8, 0, amb_color.a);
          colIdx = 0;
          break;
      case GX_ALPHA1:
          reg = gx->ambColor[1];
          SET_REG_FIELD(reg, 8, 0, amb_color.a);
          colIdx = 1;
          break;
      case GX_COLOR0A0:
          SET_REG_FIELD(reg, 8, 0, amb_color.a);
          SET_REG_FIELD(reg, 8, 8, amb_color.b);
          SET_REG_FIELD(reg, 8, 16, amb_color.g);
          SET_REG_FIELD(reg, 8, 24, amb_color.r);
          colIdx = 0;
          break;
      case GX_COLOR1A1:
          SET_REG_FIELD(reg, 8, 0, amb_color.a);
          SET_REG_FIELD(reg, 8, 8, amb_color.b);
          SET_REG_FIELD(reg, 8, 16, amb_color.g);
          SET_REG_FIELD(reg, 8, 24, amb_color.r);
          colIdx = 1;
          break;
      default:
          return;
  }

  GX_WRITE_XF_REG(colIdx + 10, reg);
  gx->bpSentNot = 1;
  gx->ambColor[colIdx] = reg;
}

void GXSetChanMatColor(GXChannelID chan, GXColor mat_color) {
    u32 reg = 0;
    u32 alpha;
    u32 colIdx;

    switch (chan) {
        case GX_COLOR0:
            alpha = gx->matColor[0] & 0xFF;
            SET_REG_FIELD(reg, 8, 0, alpha);
            SET_REG_FIELD(reg, 8, 8, mat_color.b);
            SET_REG_FIELD(reg, 8, 16, mat_color.g);
            SET_REG_FIELD(reg, 8, 24, mat_color.r);
            colIdx = 0;
            break;
        case GX_COLOR1:
            alpha = gx->matColor[1] & 0xFF;
            SET_REG_FIELD(reg, 8, 0, alpha);
            SET_REG_FIELD(reg, 8, 8, mat_color.b);
            SET_REG_FIELD(reg, 8, 16, mat_color.g);
            SET_REG_FIELD(reg, 8, 24, mat_color.r);
            colIdx = 1;
            break;
        case GX_ALPHA0:
            reg = gx->matColor[0];
            SET_REG_FIELD(reg, 8, 0, mat_color.a);
            colIdx = 0;
            break;
        case GX_ALPHA1:
            reg = gx->matColor[1];
            SET_REG_FIELD(reg, 8, 0, mat_color.a);
            colIdx = 1;
            break;
        case GX_COLOR0A0:
            SET_REG_FIELD(reg, 8, 0, mat_color.a);
            SET_REG_FIELD(reg, 8, 8, mat_color.b);
            SET_REG_FIELD(reg, 8, 16, mat_color.g);
            SET_REG_FIELD(reg, 8, 24, mat_color.r);
            colIdx = 0;
            break;
        case GX_COLOR1A1:
            SET_REG_FIELD(reg, 8, 0, mat_color.a);
            SET_REG_FIELD(reg, 8, 8, mat_color.b);
            SET_REG_FIELD(reg, 8, 16, mat_color.g);
            SET_REG_FIELD(reg, 8, 24, mat_color.r);
            colIdx = 1;
            break;
        default:
            return;
    }

    GX_WRITE_XF_REG(colIdx + 12, reg);
    gx->bpSentNot = 1;
    gx->matColor[colIdx] = reg;
}

void GXSetNumChans(u8 nChans) {

  SET_REG_FIELD(gx->genMode, 3, 4, nChans);
  GX_WRITE_XF_REG(9, nChans);
  gx->dirtyState |= 4;
}

void GXSetChanCtrl(GXChannelID chan, GXBool enable, GXColorSrc amb_src, GXColorSrc mat_src,
                   u32 light_mask, GXDiffuseFn diff_fn, GXAttnFn attn_fn) {
    u32 reg;
    u32 idx;

    if (chan == 4)
        idx = 0;
    else if (chan == 5)
        idx = 1;
    else
        idx = chan;

    reg = 0;
    SET_REG_FIELD(reg, 1, 1, enable);
    SET_REG_FIELD(reg, 1, 0, mat_src);
    SET_REG_FIELD(reg, 1, 6, amb_src);
    SET_REG_FIELD(reg, 1, 2, (light_mask & GX_LIGHT0) != 0);
    SET_REG_FIELD(reg, 1, 3, (light_mask & GX_LIGHT1) != 0);
    SET_REG_FIELD(reg, 1, 4, (light_mask & GX_LIGHT2) != 0);
    SET_REG_FIELD(reg, 1, 5, (light_mask & GX_LIGHT3) != 0);
    SET_REG_FIELD(reg, 1, 11, (light_mask & GX_LIGHT4) != 0);
    SET_REG_FIELD(reg, 1, 12, (light_mask & GX_LIGHT5) != 0);
    SET_REG_FIELD(reg, 1, 13, (light_mask & GX_LIGHT6) != 0);
    SET_REG_FIELD(reg, 1, 14, (light_mask & GX_LIGHT7) != 0);
    SET_REG_FIELD(reg, 2, 7, (attn_fn == 0) ? 0 : diff_fn);
    SET_REG_FIELD(reg, 1, 9, (attn_fn != 2));
    SET_REG_FIELD(reg, 1, 10, (attn_fn != 0));

    GX_WRITE_XF_REG(idx + 14, reg);
    gx->bpSentNot = 1;
    if (chan == GX_COLOR0A0) {
        GX_WRITE_XF_REG(16, reg);
    }
    else if (chan == GX_COLOR1A1) {
        GX_WRITE_XF_REG(17, reg);
    }
}
