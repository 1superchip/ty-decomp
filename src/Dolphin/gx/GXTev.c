#include "dolphin/gx.h"

#include "dolphin/gx/GXPriv.h"

void GXSetTevOp(GXTevStageID id, GXTevMode mode) {
    GXTevColorArg carg = GX_CC_RASC;
    GXTevAlphaArg aarg = GX_CA_RASA;

    if (id != GX_TEVSTAGE0) {
        carg = GX_CC_CPREV;
        aarg = GX_CA_APREV;
    }

    switch (mode) {
        case GX_MODULATE:
            GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_TEXC, carg, GX_CC_ZERO);
            GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_TEXA, aarg, GX_CA_ZERO);
            break;
        case GX_DECAL:
            GXSetTevColorIn(id, carg, GX_CC_TEXC, GX_CC_TEXA, GX_CC_ZERO);
            GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, aarg);
            break;
        case GX_BLEND:
            GXSetTevColorIn(id, carg, GX_CC_ONE, GX_CC_TEXC, GX_CC_ZERO);
            GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_TEXA, aarg, GX_CA_ZERO);
            break;
        case GX_REPLACE:
            GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
            GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
            break;
        case GX_PASSCLR:
            GXSetTevColorIn(id, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, carg);
            GXSetTevAlphaIn(id, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, aarg);
            break;
    }

    GXSetTevColorOp(id, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaOp(id, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
}

void GXSetTevColorIn(GXTevStageID stage, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c, GXTevColorArg d) {
    u32 *pTevReg;

    pTevReg = &gx->tevc[stage];
    SET_REG_FIELD(*pTevReg, 4, 12, a);
    SET_REG_FIELD(*pTevReg, 4, 8, b);
    SET_REG_FIELD(*pTevReg, 4, 4, c);
    SET_REG_FIELD(*pTevReg, 4, 0, d);

    GX_WRITE_RAS_REG(*pTevReg);
    gx->bpSentNot = 0;
}

void GXSetTevAlphaIn(GXTevStageID stage, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c, GXTevAlphaArg d) {
    u32 *pTevReg;

    pTevReg = &gx->teva[stage];
    SET_REG_FIELD(*pTevReg, 3, 13, a);
    SET_REG_FIELD(*pTevReg, 3, 10, b);
    SET_REG_FIELD(*pTevReg, 3, 7, c);
    SET_REG_FIELD(*pTevReg, 3, 4, d);

    GX_WRITE_RAS_REG(*pTevReg);
    gx->bpSentNot = 0;
}

void GXSetTevColorOp(GXTevStageID stage, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp,
                     GXTevRegID out_reg) {
  u32* tevReg;

  tevReg = &gx->tevc[stage];
  SET_REG_FIELD(*tevReg, 1, 18, op & 1);
  if (op <= 1) {
    SET_REG_FIELD(*tevReg, 2, 20, scale);
    SET_REG_FIELD(*tevReg, 2, 16, bias);
  } else {
    SET_REG_FIELD(*tevReg, 2, 20, (op >> 1) & 3);
    SET_REG_FIELD(*tevReg, 2, 16, 3);
  }
  SET_REG_FIELD(*tevReg, 1, 19, clamp & 0xFF);
  SET_REG_FIELD(*tevReg, 2, 22, out_reg);

  GX_WRITE_RAS_REG(*tevReg);
  gx->bpSentNot = 0;
}

void GXSetTevAlphaOp(GXTevStageID stage, GXTevOp op, GXTevBias bias, GXTevScale scale, GXBool clamp,
                     GXTevRegID out_reg) {
  u32* tevReg;

  tevReg = &gx->teva[stage];
  SET_REG_FIELD(*tevReg, 1, 18, op & 1);
  if (op <= 1) {
    SET_REG_FIELD(*tevReg, 2, 20, scale);
    SET_REG_FIELD(*tevReg, 2, 16, bias);
  } else {
    SET_REG_FIELD(*tevReg, 2, 20, (op >> 1) & 3);
    SET_REG_FIELD(*tevReg, 2, 16, 3);
  }
  SET_REG_FIELD(*tevReg, 1, 19, clamp & 0xFF);
  SET_REG_FIELD(*tevReg, 2, 22, out_reg);

  GX_WRITE_RAS_REG(*tevReg);
  gx->bpSentNot = 0;
}

void GXSetTevColor(GXTevRegID id, GXColor color) {
  u32 regRA;
  u32 regBG;

  regRA = 0;
  SET_REG_FIELD(regRA, 11, 0, color.r);
  SET_REG_FIELD(regRA, 11, 12, color.a);
  SET_REG_FIELD(regRA, 8, 24, 224 + id * 2);

  regBG = 0;
  SET_REG_FIELD(regBG, 11, 0, color.b);
  SET_REG_FIELD(regBG, 11, 12, color.g);
  SET_REG_FIELD(regBG, 8, 24, 225 + id * 2);

  GX_WRITE_RAS_REG(regRA);
  GX_WRITE_RAS_REG(regBG);
  GX_WRITE_RAS_REG(regBG);
  GX_WRITE_RAS_REG(regBG);

  gx->bpSentNot = 0;
}

void GXSetTevColorS10(GXTevRegID id, GXColorS10 color) {
  u32 sRG;
  u32 sBA;
  u32 regRA;
  u32 regBG;

  regRA = 0;
  SET_REG_FIELD(regRA, 11, 0, color.r & 0x7FF);
  SET_REG_FIELD(regRA, 11, 12, color.a & 0x7FF);
  SET_REG_FIELD(regRA, 8, 24, 224 + id * 2);

  regBG = 0;
  SET_REG_FIELD(regBG, 11, 0, color.b & 0x7FF);
  SET_REG_FIELD(regBG, 11, 12, color.g & 0x7FF);
  SET_REG_FIELD(regBG, 8, 24, 225 + id * 2);

  GX_WRITE_RAS_REG(regRA);
  GX_WRITE_RAS_REG(regBG);
  GX_WRITE_RAS_REG(regBG);
  GX_WRITE_RAS_REG(regBG);

  gx->bpSentNot = 0;
}

void GXSetTevKColor(GXTevKColorID id, GXColor color) {
  u32 regRA;
  u32 regBG;

  regRA = 0;
  SET_REG_FIELD(regRA, 8, 0, color.r);
  SET_REG_FIELD(regRA, 8, 12, color.a);
  SET_REG_FIELD(regRA, 4, 20, 8);
  SET_REG_FIELD(regRA, 8, 24, 224 + id * 2);

  regBG = 0;
  SET_REG_FIELD(regBG, 8, 0, color.b);
  SET_REG_FIELD(regBG, 8, 12, color.g);
  SET_REG_FIELD(regBG, 4, 20, 8);
  SET_REG_FIELD(regBG, 8, 24, 225 + id * 2);

  GX_WRITE_RAS_REG(regRA);
  GX_WRITE_RAS_REG(regBG);
  gx->bpSentNot = 0;
}

void GXSetTevKColorSel(GXTevStageID stage, GXTevKColorSel sel) {
  u32 *Kreg;

  Kreg = &gx->tevKsel[stage >> 1];
  if (stage & 1) {
    SET_REG_FIELD(*Kreg, 5, 14, sel);
  } else {
    SET_REG_FIELD(*Kreg, 5, 4, sel);
  }

  GX_WRITE_RAS_REG(*Kreg);
  gx->bpSentNot = 0;
}

void GXSetTevKAlphaSel(GXTevStageID stage, GXTevKAlphaSel sel) {
  u32 *Kreg;

  Kreg = &gx->tevKsel[stage >> 1];
  if (stage & 1) {
    SET_REG_FIELD(*Kreg, 5, 19, sel);
  } else {
    SET_REG_FIELD(*Kreg, 5, 9, sel);
  }

  GX_WRITE_RAS_REG(*Kreg);
  gx->bpSentNot = 0;
}

void GXSetTevSwapMode(GXTevStageID stage, GXTevSwapSel ras_sel, GXTevSwapSel tex_sel) {
  u32 *pTevReg;

  pTevReg = &gx->teva[stage];
  SET_REG_FIELD(*pTevReg, 2, 0, ras_sel);
  SET_REG_FIELD(*pTevReg, 2, 2, tex_sel);

  GX_WRITE_RAS_REG(*pTevReg);
  gx->bpSentNot = 0;
}

void GXSetTevSwapModeTable(GXTevSwapSel table, GXTevColorChan red, GXTevColorChan green,
                           GXTevColorChan blue, GXTevColorChan alpha) {
  u32 *Kreg;

  int index = table * 2;

  Kreg = &gx->tevKsel[index];

  SET_REG_FIELD(*Kreg, 2, 0, red);
  SET_REG_FIELD(*Kreg, 2, 2, green);

  GX_WRITE_RAS_REG(*Kreg);

  Kreg = &gx->tevKsel[table * 2 + 1];
  SET_REG_FIELD(*Kreg, 2, 0, blue);
  SET_REG_FIELD(*Kreg, 2, 2, alpha);

  GX_WRITE_RAS_REG(*Kreg);
  gx->bpSentNot = 0;
}

void GXSetAlphaCompare(GXCompare comp0, u8 ref0, GXAlphaOp op, GXCompare comp1, u8 ref1) {
  u32 reg;

  reg = 0;

  SET_REG_FIELD(reg, 8, 0, ref0);
  SET_REG_FIELD(reg, 8, 8, ref1);
  SET_REG_FIELD(reg, 3, 16, comp0);
  SET_REG_FIELD(reg, 3, 19, comp1);
  SET_REG_FIELD(reg, 2, 22, op);

  // reg &= 0xFFFFFF;
  reg = (reg & 0x00FFFFFF) | 0xF3000000;

  GX_WRITE_RAS_REG(reg);
  gx->bpSentNot = 0;
}

void GXSetZTexture(GXZTexOp op, GXTexFmt fmt, u32 bias) {
  u32 zenv0;
  u32 zenv1;
  u32 type;

  zenv0 = 0;
  SET_REG_FIELD(zenv0, 24, 0, bias);
  SET_REG_FIELD(zenv0, 8, 24, 0xF4);

  zenv1 = 0;
  switch (fmt) {
  case GX_TF_Z8:
    type = 0;
    break;
  case GX_TF_Z16:
    type = 1;
    break;
  case GX_TF_Z24X8:
    type = 2;
    break;
  default:

    type = 2;
    break;
  }

  SET_REG_FIELD(zenv1, 2, 0, type);
  SET_REG_FIELD(zenv1, 2, 2, op);
  SET_REG_FIELD(zenv1, 8, 24, 0xF5);

  GX_WRITE_RAS_REG(zenv0);
  GX_WRITE_RAS_REG(zenv1);
  gx->bpSentNot = 0;
}

void GXSetTevOrder(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID color) {
  u32 *ptref;
  u32 tmap;
  u32 tcoord;
  static int c2r[] = {0, 1, 0, 1, 0, 1, 7, 5, 6};

  ptref = &gx->tref[stage / 2];
  gx->texmapId[stage] = map;

  tmap = map & ~GX_TEX_DISABLE;
  tmap = (tmap >= GX_MAX_TEXMAP) ? GX_TEXMAP0 : tmap;

  if (coord >= GX_MAX_TEXCOORD) {
    tcoord = GX_TEXCOORD0;
    gx->tevTcEnab = gx->tevTcEnab & ~(1 << stage);
  } else {
    tcoord = coord;
    gx->tevTcEnab = gx->tevTcEnab | (1 << stage);
  }

  if (stage & 1) {
    SET_REG_FIELD(*ptref, 3, 12, tmap);
    SET_REG_FIELD(*ptref, 3, 15, tcoord);
    SET_REG_FIELD(*ptref, 3, 19, (color == GX_COLOR_NULL) ? 7 : c2r[color]);
    SET_REG_FIELD(*ptref, 1, 18, (map != GX_TEXMAP_NULL && !(map & GX_TEX_DISABLE)));
  } else {
    SET_REG_FIELD(*ptref, 3, 0, tmap);
    SET_REG_FIELD(*ptref, 3, 3, tcoord);
    SET_REG_FIELD(*ptref, 3, 7, (color == GX_COLOR_NULL) ? 7 : c2r[color]);
    SET_REG_FIELD(*ptref, 1, 6, (map != GX_TEXMAP_NULL && !(map & GX_TEX_DISABLE)));
  }

  GX_WRITE_RAS_REG(*ptref);
  gx->bpSentNot = 0;
  gx->dirtyState |= 1;
}

void GXSetNumTevStages(u8 nStages) {

  SET_REG_FIELD(gx->genMode, 4, 10, nStages - 1);
  gx->dirtyState |= 4;
}
