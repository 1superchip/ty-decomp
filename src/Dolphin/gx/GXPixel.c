#include "Dolphin/gx.h"
#include "Dolphin/mtx.h"

#include "Dolphin/gx/GXPriv.h"

void GXSetFog(GXFogType type, f32 startz, f32 endz, f32 nearz, f32 farz, GXColor color) {
  u32 fogclr;
  u32 fog0;
  u32 fog1;
  u32 fog2;
  u32 fog3;
  f32 A;
  f32 B;
  f32 B_mant;
  f32 C;
  f32 a;
  f32 c;
  u32 B_expn;
  u32 b_m;
  u32 b_s;
  u32 a_hex;
  u32 c_hex;
  {
    if (farz == nearz || endz == startz) {
      A = 0.0f;
      B = 0.5f;
      C = 0.0f;
    } else {
      A = (farz * nearz) / ((farz - nearz) * (endz - startz));
      B = farz / (farz - nearz);
      C = startz / (endz - startz);
    }

    B_mant = B;
    B_expn = 0;
    while (B_mant > 1.0) {
      B_mant /= 2.0f;
      B_expn++;
    }
    while (B_mant > 0.0f && B_mant < 0.5) {
      B_mant *= 2.0f;
      B_expn--;
    }

    a = A / (f32)(1 << (B_expn + 1));
    b_m = 8.388638e6f * B_mant;
    b_s = B_expn + 1;
    c = C;

    fog1 = 0;
    SET_REG_FIELD(fog1, 24, 0, b_m);
    SET_REG_FIELD(fog1, 8, 24, 0xEF);

    fog2 = 0;
    SET_REG_FIELD(fog2, 5, 0, b_s);
    SET_REG_FIELD(fog2, 8, 24, 0xF0);
  }

  a_hex = *(u32 *)&a;
  c_hex = *(u32 *)&c;

  fog0 = 0;
  SET_REG_FIELD(fog0, 11, 0, (a_hex >> 12) & 0x7FF);
  SET_REG_FIELD(fog0, 8, 11, (a_hex >> 23) & 0xFF);
  SET_REG_FIELD(fog0, 1, 19, (a_hex >> 31));
  SET_REG_FIELD(fog0, 8, 24, 0xEE);

  fog3 = 0;
  SET_REG_FIELD(fog3, 11, 0, (c_hex >> 12) & 0x7FF);
  SET_REG_FIELD(fog3, 8, 11, (c_hex >> 23) & 0xFF);
  SET_REG_FIELD(fog3, 1, 19, (c_hex >> 31));
  SET_REG_FIELD(fog3, 1, 20, 0);
  SET_REG_FIELD(fog3, 3, 21, type);
  SET_REG_FIELD(fog3, 8, 24, 0xF1);

  fogclr = 0;
  SET_REG_FIELD(fogclr, 8, 0, color.b);
  SET_REG_FIELD(fogclr, 8, 8, color.g);
  SET_REG_FIELD(fogclr, 8, 16, color.r);
  SET_REG_FIELD(fogclr, 8, 24, 0xF2);

  GX_WRITE_RAS_REG(fog0);
  GX_WRITE_RAS_REG(fog1);
  GX_WRITE_RAS_REG(fog2);
  GX_WRITE_RAS_REG(fog3);
  GX_WRITE_RAS_REG(fogclr);
  gx->bpSentNot = 0;
}

void GXSetFogRangeAdj(GXBool enable, u16 center, const GXFogAdjTable *table) {
  u32 i;
  u32 range_adj;
  u32 range_c;

  if (enable) {

    for (i = 0; i < 10; i += 2) {
      range_adj = 0;
      SET_REG_FIELD(range_adj, 12, 0, table->r[i]);
      SET_REG_FIELD(range_adj, 12, 12, table->r[i + 1]);
      SET_REG_FIELD(range_adj, 8, 24, (i >> 1) + 0xE9);
      GX_WRITE_RAS_REG(range_adj);
    }
  }
  range_c = 0;
  SET_REG_FIELD(range_c, 10, 0, center + 342);
  SET_REG_FIELD(range_c, 1, 10, enable);
  SET_REG_FIELD(range_c, 8, 24, 0xE8);
  GX_WRITE_RAS_REG(range_c);
  gx->bpSentNot = 0;
}

void GXSetBlendMode(GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor,
                    GXLogicOp op) {
  SET_REG_FIELD(gx->cmode0, 1, 0, (type == GX_BM_BLEND || type == GX_BM_SUBTRACT));
  SET_REG_FIELD(gx->cmode0, 1, 11, (type == GX_BM_SUBTRACT));
  SET_REG_FIELD(gx->cmode0, 1, 1, (type == GX_BM_LOGIC));
  SET_REG_FIELD(gx->cmode0, 4, 12, op);
  SET_REG_FIELD(gx->cmode0, 3, 8, src_factor);
  SET_REG_FIELD(gx->cmode0, 3, 5, dst_factor);
  SET_REG_FIELD(gx->cmode0, 8, 24, 0x41);
  GX_WRITE_RAS_REG(gx->cmode0);
  gx->bpSentNot = 0;
}

void GXSetColorUpdate(GXBool update_enable) {
  SET_REG_FIELD(gx->cmode0, 1, 3, update_enable);
  GX_WRITE_RAS_REG(gx->cmode0);
  gx->bpSentNot = 0;
}

void GXSetAlphaUpdate(GXBool update_enable) {
  SET_REG_FIELD(gx->cmode0, 1, 4, update_enable);
  GX_WRITE_RAS_REG(gx->cmode0);
  gx->bpSentNot = 0;
}

void GXSetZMode(GXBool compare_enable, GXCompare func, GXBool update_enable) {
  SET_REG_FIELD(gx->zmode, 1, 0, compare_enable);
  SET_REG_FIELD(gx->zmode, 3, 1, func);
  SET_REG_FIELD(gx->zmode, 1, 4, update_enable);
  GX_WRITE_RAS_REG(gx->zmode);
  gx->bpSentNot = 0;
}

void GXSetZCompLoc(GXBool before_tex) {

  SET_REG_FIELD(gx->peCtrl, 1, 6, before_tex);
  GX_WRITE_RAS_REG(gx->peCtrl);
  gx->bpSentNot = 0;
}

void GXSetPixelFmt(GXPixelFmt pix_fmt, GXZFmt16 z_fmt) {
  u32 oldPeCtrl;
  u8 aa;
  static u32 p2f[8] = {0, 1, 2, 3, 4, 4, 4, 5};

  oldPeCtrl = gx->peCtrl;

  SET_REG_FIELD(gx->peCtrl, 3, 0, p2f[pix_fmt]);
  SET_REG_FIELD(gx->peCtrl, 3, 3, z_fmt);

  if (oldPeCtrl != gx->peCtrl) {
    GX_WRITE_RAS_REG(gx->peCtrl);
    if (pix_fmt == GX_PF_RGB565_Z16)
      aa = 1;
    else
      aa = 0;
    SET_REG_FIELD(gx->genMode, 1, 9, aa);
    gx->dirtyState |= 4;
  }

  if (p2f[pix_fmt] == 4) {
    SET_REG_FIELD(gx->cmode1, 2, 9, (pix_fmt - 4) & 0x3);
    SET_REG_FIELD(gx->cmode1, 8, 24, 0x42);
    GX_WRITE_RAS_REG(gx->cmode1);
  }

  gx->bpSentNot = 0;
}

void GXSetDither(GXBool dither) {
  SET_REG_FIELD(gx->cmode0, 1, 2, dither);
  GX_WRITE_RAS_REG(gx->cmode0);
  gx->bpSentNot = 0;
}

void GXSetDstAlpha(GXBool enable, u8 alpha) {
  SET_REG_FIELD(gx->cmode1, 8, 0, alpha);
  SET_REG_FIELD(gx->cmode1, 1, 8, enable);
  GX_WRITE_RAS_REG(gx->cmode1);
  gx->bpSentNot = 0;
}

void GXSetFieldMask(GXBool odd_mask, GXBool even_mask) {
  u32 reg;

  reg = 0;
  SET_REG_FIELD(reg, 1, 0, even_mask);
  SET_REG_FIELD(reg, 1, 1, odd_mask);
  SET_REG_FIELD(reg, 8, 24, 0x44);
  GX_WRITE_RAS_REG(reg);
  gx->bpSentNot = 0;
}

void GXSetFieldMode(GXBool field_mode, GXBool half_aspect_ratio) {
  u32 reg;

  SET_REG_FIELD(gx->lpSize, 1, 22, half_aspect_ratio);
  GX_WRITE_RAS_REG(gx->lpSize);
  __GXFlushTextureState();
  reg = field_mode | 0x68000000;
  GX_WRITE_RAS_REG(reg);
  __GXFlushTextureState();
}
