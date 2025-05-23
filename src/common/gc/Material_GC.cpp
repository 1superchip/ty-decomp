#include "types.h"
#include "common/Material.h"
#include "common/System_GC.h"
#include "common/FileSys.h"
#include "common/PtrListDL.h"
#include "common/Heap.h"
#include "Dolphin/gx.h"
#include "common/Texture.h"
#include "common/Str.h"
#include "common/View.h"
#include "common/StdMath.h"
#include "common/Water_GC.h"
#include "common/Debug.h"
#include "common/demoinit.h"
#include "Dolphin/vi.h"

typedef f32 (*MtxPtr)[4];

// struct Mtx44 {
//     float data[4][4];
// };

// EXTERNS
extern "C" void C_MTXOrtho(Mtx44* m, f32 t, f32 b, f32 l, f32 r, f32 n, f32 f);
extern "C" void C_MTXIdentity(Mtx44*);
extern "C" void strcpy(char*, char*);
extern "C" int stricmp(char*, char*);
extern "C" {
    double fmod(double, double);
};

// End EXTERNS

#include "assets/Material_GC/rawCaptureTexData.inc"

static KromeIni materialIni __attribute__ ((aligned (32)));
static GXTexObj restorationTexObj __attribute__ ((aligned (16)));
static char restorationTexData[32768];
static GXTexObj rawCaptureTexObj;
char captureTexData[32768];

static PtrListDL<Material> materials;
static int pPCMatChanges = 0;
Material* Material::pCurrMat[2];
static void* pCaptureTexture = NULL;
int Material::frameCounter = 0;
int Material::updateEnabled = 0;

GXColor Material_MixedColor = {0xff, 0xff, 0xff, 0xff};

// extra 16 bytes to match rodata length
const Vector MaterialGC_rodata_hack = {};

// (0x100000 | 0x80000 | 0x40000 | 0x20000 | 0x10000 | 0x2000) = 0x1f2000

char* Material::InitFromMatDefs(char* pName) {
    char* zwrite;
    char* pTextureName;
    char* pOverlayName;
    int blend;
    char* pBlend;
    char textureName[sizeof(name)];

    texture_filterType = gMKDefaults.materialTextureFilterType;
    type = gMKDefaults.materialType;
    flags = gMKDefaults.materialFlags;
    blendMode = gMKDefaults.materialBlendMode;
    collisionFlags = gMKDefaults.collisionFlags;
    grass = 0;
    effect = 0;
    mipBias = -0x42;
    angle = 0.0f;
    unkA8 = 0.0f;
    unkA4 = 0.0f;
    unkA0 = 0.0f;
    frameCounter1 = frameCounter;
    color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    unk5C = 0.0f;
    pOverlayMat = NULL;
    unk60.SetIdentity();
    unk58 = NULL;
    unk54 = NULL;
    pOverlayName = NULL;
    unkCC = false;
    unkCD = false;
    indirectWaterVec.z = 0.005f;
    indirectWaterVec.w = 0.005f;
    indirectWaterVec.x = 25.0f;
    indirectWaterVec.y = 50.0f;

    strcpy(name, Str_CopyString(pName, sizeof(name) - 1));
    pTextureName = NULL;
    KromeIniLine* pLine = materialIni.GotoLine(name, NULL);
    if (pLine != NULL) {
        do {
            pLine = materialIni.GetCurrentLine();
            if (pLine == NULL) {
                break;
            }

            if (pLine->pFieldName != NULL) {
                if (stricmp(pLine->pFieldName, "blend") == 0) {
                    if (pLine->elementCount == 0) {
                        materialIni.Warning("Missing blend value!");
                    } else if (pLine->AsInt(0, &blend) != false) {
                        switch (blend) {
                        case 0:
                            blendMode = Blend_Opaque;
                            ClearFlags(8);
                            ClearFlags(0x10);
                            break;
                        case 1:
                            blendMode = Blend_Additive;
                            ClearFlags(8);
                            SetFlags(0x10);
                            break;
                        case 2:
                            blendMode = Blend_Subtractive;
                            ClearFlags(8);
                            SetFlags(0x10);
                            break;
                        case 5:
                            blendMode = Blend_Alpha;
                            ClearFlags(8);
                            SetFlags(0x10);
                            break;
                        case 6:
                            blendMode = Blend_Blend;
                            ClearFlags(8);
                            ClearFlags(0x10);
                            break;
                        default:
                            materialIni.Warning(Str_Printf("Bad blend type %d", blend));
                            break;
                        }
                    } else {
                        pLine->AsString(0, &pBlend);
                        if (stricmp(pBlend, "opaque") == 0) {
                            blendMode = Blend_Opaque;
                            ClearFlags(8);
                            ClearFlags(0x10);
                        } else if (stricmp(pBlend, "additive") == 0) {
                            blendMode = Blend_Additive;
                            ClearFlags(8);
                            SetFlags(0x10);
                        } else if (stricmp(pBlend, "subtractive") == 0) {
                            blendMode = Blend_Subtractive;
                            ClearFlags(8);
                            SetFlags(0x10);
                        } else if (stricmp(pBlend, "alpha") == 0) {
                            blendMode = Blend_Alpha;
                            ClearFlags(8);
                            SetFlags(0x10);
                        } else if (stricmp(pBlend, "blend") == 0) {
                            blendMode = Blend_Blend;
                            ClearFlags(8);
                            ClearFlags(0x10);
                        } else {
                            materialIni.Warning(Str_Printf("Unknown blend mode %s", pBlend));
                        }
                    }
                } else if (stricmp(pLine->pFieldName, "filter_gc") == 0) {
                    if (pLine->elementCount == 0) {
                        materialIni.Warning("Missing filter value!");
                    } else {
                        int tmp;
                        if (pLine->AsInt(0, &tmp) != false) {
                            switch (tmp) {
                            case 0:
                                texture_filterType = 0;
                                break;
                            case 1:
                                texture_filterType = 1;
                                break;
                            case 5:
                                texture_filterType = 2;
                                break;
                            default:
                                materialIni.Warning(Str_Printf("Bad filter type %d", tmp));
                                break;
                            }
                        } else {
                            char* pType;
                            pLine->AsString(0, &pType);
                            materialIni.Warning(Str_Printf("Unknown filter type %s", pType));
                        }
                    }
                } else if (stricmp(pLine->pFieldName, "type") == 0) {
                    if (pLine->elementCount == 0) {
                        materialIni.Warning("Missing type value!");
                    } else {
                        char* pType;
                        pLine->AsString(0, &pType);
                        if (stricmp(pType, "decal") == 0) {
                            type = Type_Decal;
                        } else if (stricmp(pType, "edgefadeout") == 0) {
                            type = Type_EdgeFadeout;
                        } else if (stricmp(pType, "envmap") == 0) {
                            type = Type_EnvMap;
                        } else if (stricmp(pType, "prelit") == 0) {
                            type = Type_Prelit;
                        } else if (stricmp(pType, "caustic") == 0) {
                            type = Type_Caustic;
                        } else if (stricmp(pType, "alphaFog") == 0) {
                            type = Type_AlphaFog;
                        } else if (stricmp(pType, "indirectWater") == 0) {
                            type = Type_IndirectWater;
                            flags |= 0x80;
                        } else {
                            materialIni.Warning(Str_Printf("Unknown type mode %s", pType));
                        }
                    }
                } else if (stricmp(pLine->pFieldName, "alias") == 0) {
                    if (pLine->elementCount == 0) {
                        materialIni.Warning("Missing alias");
                    } else {
                        Texture_IsAlias = true;
                        pLine->AsString(0, &pTextureName);
                        strcpy(textureName, pTextureName);
                    }
                } else if (stricmp(pLine->pFieldName, "invisible") == 0) {
                    int tmp;
                    if (pLine->AsFlag(0, &tmp) != false) {
                        if (tmp != 0) {
                            flags |= Flag_Invisible;
                        } else {
                            flags &= ~Flag_Invisible;
                        }
                    } else {
                        materialIni.Warning("Bad data");
                    }
                } else if (stricmp(pLine->pFieldName, "doublesided") == 0) {
                    int tmp;
                    if (pLine->AsFlag(0, &tmp) != false) {
                        ConditionallySetRemoveFlags(Flag_DoubleSided, tmp);
                        SetFlags(0x800);
                    } else {
                        materialIni.Warning("Bad data");
                    }
                } else if (stricmp(pLine->pFieldName, "mipBias") == 0) {
                    if (pLine->AsInt(0, &mipBias) == false) {
                        materialIni.Warning("Missing mip bias");
                    }
                    mipBias -= 0x42;
                } else if (stricmp(pLine->pFieldName, "id") == 0) {
					// collision flags for material
                    if (pLine->AsInt(0, &collisionFlags) == false) {
                        materialIni.Warning("Missing ID");
                    }
                } else if (stricmp(pLine->pFieldName, "nocollide") == 0) {
                    int tmp;
                    if (pLine->AsFlag(0, &tmp) != false) {
                        ConditionallySetRemoveFlags(Flag_NoCollide, tmp);
                        SetFlags(Flag_NoCollide); // why set it here???
                    } else {
                        materialIni.Warning("Bad data");
                    }
                } else if (stricmp(pLine->pFieldName, "address") == 0) {
                    int tmp;
                    if (pLine->AsFlag(0, &tmp) != false) {
                        ConditionallySetRemoveFlags(0x2, tmp);
                        ConditionallySetRemoveFlags(0x4, tmp);
                    } else {
                        materialIni.Warning("Bad data");
                    }
                } else if (stricmp(pLine->pFieldName, "clampUV") == 0) {
                    int clampU;
                    int clampV;
                    pLine->AsFlag(0, &clampU);
                    pLine->AsFlag(1, &clampV);
                    ConditionallySetRemoveFlags(0x2, clampU);
                    ConditionallySetRemoveFlags(0x4, clampV);
                } else if (stricmp(pLine->pFieldName, "effect") == 0) {
                    char* pEffect;
                    if (pLine->AsString(0, &pEffect) != false) { // first field of line is the name as a string
                        pLine->AsInt(1, &effect);                // second field of line is an int
                        if (stricmp(pEffect, "grass") == 0) {
                            grass = 1;
                        }
                    }
                } else if (stricmp(pLine->pFieldName, "masked") == 0) {
                    int val;
                    pLine->AsFlag(0, &val);
                    if (val != 0) {
                        SetFlags(Flag_AlphaMask);
                        unk5C = gMKDefaults.materialAlphaRef; // set alpha ref to default
                    } else {
                        ClearFlags(Flag_AlphaMask);
                    }
                } else if (stricmp(pLine->pFieldName, "envmap") == 0) {
                    type = Type_EnvMap;
                } else if (stricmp(pLine->pFieldName, "indirectWater") == 0) {
                    type = Type_IndirectWater;
                    if (pLine->elementCount < 6) {
                        materialIni.Warning("Missing parameters!");
                    }
                    // switch to go to last element in line and then read every element until element 0
                    int val;
                    switch (pLine->elementCount) {
                        default:
                        case 5:
                            pLine->AsFloat(5, &indirectWaterVec.y);
                            // fallthrough
                        case 4:
                            pLine->AsFloat(4, &indirectWaterVec.x);
                            // fallthrough
                        case 3:
                            pLine->AsFloat(3, &indirectWaterVec.w);
                            // fallthrough
                        case 2:
                            pLine->AsFloat(2, &indirectWaterVec.z);
                            // fallthrough
                        case 1:
                            pLine->AsFlag(1, &val);
                            unkCD = val;
                            // fallthrough
                        case 0:
                            pLine->AsFlag(0, &val);
                            unkCC = val;
                            if (pLine->elementCount == 0) {
                                materialIni.Warning("Missing type value!");
                            }
                            break;
                    }
                } else if (stricmp(pLine->pFieldName, "color") == 0) {
                    pLine->AsFloat(0, &color.x);
                    pLine->AsFloat(1, &color.y);
                    pLine->AsFloat(2, &color.z);
                    if (pLine->AsFloat(3, &color.w) == false) {
                        color.w = 1.0f;
                    }
                } else if (stricmp(pLine->pFieldName, "overlay") == 0) {
                    if (pLine->AsString(0, &pOverlayName) == false) {
                        materialIni.Warning("Missing overlay name");
                    }
                } else if (stricmp(pLine->pFieldName, "ambient") == 0) {
                    float amb;
                    pLine->AsFloat(0, &amb);
                    if (amb > 0.0f) {
                        // if ambient light is greater than 0.0f, the material is prelit
                        type = Type_Prelit;
                    }
                } else if (stricmp(pLine->pFieldName, "aref") == 0) {
                    pLine->AsFloat(0, &unk5C); // alpha reference0 for parameter 2 of GXSetAlphaCompare
                } else if (stricmp(pLine->pFieldName, "zread") == 0) {
                    int zread;
                    if (pLine->AsInt(0, &zread) == false) {
                        materialIni.Warning("Missing on/off");
                    } else {
                        ConditionallySetRemoveFlags(0x8, zread == 0);
                    }
                } else if (stricmp(pLine->pFieldName, "zwrite") == 0) {
                    int zwrite;
                    if (pLine->AsInt(0, &zwrite) == false) {
                        materialIni.Warning("Missing on/off");
                    } else {
                        ConditionallySetRemoveFlags(0x10, zwrite == 0);
                    }
                } else if (stricmp(pLine->pFieldName, "alias") == 0) {
                    if (pLine->AsString(0, &pTextureName) == false) {
                        materialIni.Warning("Missing texture name");
                    } else {
                        Texture_IsAlias = true;
                    }
                } else if (stricmp(pLine->pFieldName, "animate") == 0) {
                    if (flags & 0x1f2000) {
                        materialIni.Warning("animate cannot be combined with animate/scrolling/rotating");
                    } else if (!(pLine->AsFloat(0, &unkA0) | pLine->AsFloat(1, &unkA4) | pLine->AsFloat(2, &unkA8))) {
                        materialIni.Warning("Missing parameters for 'animate'");
                    } else {
                        flags |= Flag_Animate;
                    }
                } else if (stricmp(pLine->pFieldName, "scroll") == 0) {
                    if (flags & 0x1f2000) {
                        materialIni.Warning("scroll cannot be combined with animate/scrolling/rotating");
                    } else if (!(pLine->AsFloat(0, &unkA0) | pLine->AsFloat(1, &unkA4))) {
                        materialIni.Warning("Missing parameters");
                    } else {
                        flags |= Flag_Scroll;
                    }
                } else if (stricmp(pLine->pFieldName, "rotate") == 0) {
                    if (flags & 0x1f2000) {
                        materialIni.Warning("rotate cannot be combined with animate/scrolling/rotating");
                    } else if (!(pLine->AsFloat(0, &unkA0) | pLine->AsFloat(1, &unkA4) | pLine->AsFloat(2, &unkA8))) {
                        materialIni.Warning("Missing parameters");
                    } else {
                        flags |= Flag_Rotate;
                    }
                } else if (stricmp(pLine->pFieldName, "envscroll") == 0) {
                    if (flags & 0x1f2000) {
                        materialIni.Warning("rotate cannot be combined with animate/scrolling/rotating");
                    } else {
                        if (pLine->elementCount == 0) {
                            // set default envscroll values if none are provided
                            unkA0 = 1.0f;
                            unkA4 = -1.0f;
                            unkB4 = 0.25f;
                            unkB8 = 0.5f;
                            unkBC = 0.0f;
                            unkC0 = 0.25f;
                        } else if (!(pLine->AsFloat(0, &unkA0) | pLine->AsFloat(1, &unkA4) | 
                                pLine->AsFloat(2, &unkB4) | pLine->AsFloat(3, &unkB8) | 
                                pLine->AsFloat(4, &unkBC) | pLine->AsFloat(5, &unkC0))) {
                            materialIni.Warning("Missing parameters");
                            goto end;
                        }
                        flags |= Flag_EnvRotate;
                    }
                } else if (stricmp(pLine->pFieldName, "sinrotate") == 0) {
                    if (flags & 0x1f2000) {
                        materialIni.Warning("rotate cannot be combined with animate/scrolling/rotating");
                    } else if (!(pLine->AsFloat(0, &unkA0) | pLine->AsFloat(1, &unkA4) | 
                        pLine->AsFloat(2, &unkA8) | pLine->AsFloat(3, &unkB0))) {
                        materialIni.Warning("Missing parameters");
                    } else {
                        flags |= Flag_SinRotate;
                    }
                } else if (stricmp(pLine->pFieldName, "matrix") == 0) {
                    if (flags & 0x1f2000) {
                        materialIni.Warning("rotate cannot be combined with animate/scrolling/rotating");
                    } else {
                        flags |= 0x2000;
                    }
                } else {
                    materialIni.Warning("Unknown field name");
                }
            }
            end:
            pLine = materialIni.GetNextLine();
        } while (pLine != NULL && pLine->section == NULL);
    }

    if (pTextureName != NULL) {
        strcpy(textureName, pTextureName);
    }

    if (pOverlayName != NULL) {
        pOverlayMat = Material::Create(pOverlayName);
    }

    if (pTextureName != NULL) {
        return Str_CopyString(textureName, sizeof(textureName));
    }
    
    return name;
}

Material* Material::Create(char* pName) {
    Material* pFoundMat = Material::Find(pName);
    if (pFoundMat != NULL) {
        pFoundMat->referenceCount++;
    } else {
        pFoundMat = materials.GetNextEntry();
        pFoundMat->referenceCount = 1;
        char* textureName = pFoundMat->InitFromMatDefs(pName);
        Texture_bColourKey = (pFoundMat->flags & Flag_AlphaMask) == Flag_AlphaMask;
        Texture_filterType = pFoundMat->texture_filterType;
        Texture_Color = pFoundMat->color;

        if (!FileSys_Exists(Str_Printf("%s.tga", textureName), NULL) && 
            !FileSys_Exists(Str_Printf("%s.gtx", textureName), NULL)) {
            // if neither texture file exists, use the texture from the debug font material
            pFoundMat->unk54 = gpDebugFont->pFontMaterial->GetTexture();
            pFoundMat->unk54->referenceCount++;
        } else {
            pFoundMat->unk54 = Texture::Create(textureName);
        }

        Texture_bColourKey = false;
        Texture_IsAlias = false;

        if (pFoundMat->unk54 != NULL) {
            _GXTexWrapMode wrap_t = (pFoundMat->flags & 0x4) ? GX_CLAMP : GX_REPEAT;
            _GXTexWrapMode wrap_s = (pFoundMat->flags & 0x2) ? GX_CLAMP : GX_REPEAT;
            GXInitTexObjWrapMode(&pFoundMat->unk54->texObj, wrap_s, wrap_t);
        }
    }

    return pFoundMat;
}

Material* Material::CreateRenderTarget(char* pName, int arg1, int arg2, int arg3) {
    Material* pMaterial;
    pMaterial = materials.GetNextEntry();
    pMaterial->referenceCount = 1;
    pMaterial->InitFromMatDefs(pName);
    pMaterial->unk54 = Texture::CreateRenderTarget(pName, arg1, arg2, arg3);
    return pMaterial;
}

// pragma is needed to match this recursive function
#pragma inline_max_size(560)
void Material::Destroy(void) {
    if (--referenceCount != 0) {
        return;
    }

    if (pOverlayMat != NULL) {
        pOverlayMat->Destroy();
    }

    if (unk54 != NULL) {
        unk54->Destroy();
    }

    if (unk58 != NULL) {
        unk58->Destroy();
    }
    
    materials.Destroy(this);
}
#pragma inline_max_size reset

/// @brief Iterates through the material instance list to find an already created material
/// @param pName Name of material to search for
/// @return NULL if a Material does not exist otherwise the Material
Material* Material::Find(char* pName) {
    Material** list;

    char* tmpName = Str_CopyString(pName, 31);

    list = materials.GetMem();

    while (*list) {
        if (stricmp((*list)->name, tmpName) == 0) {
            return *list;
        }

        list++;
    }

    return NULL;
}

// Initialises material list and initiates texobjs from this file
void Material::InitModule(void) {

	materials.Init(gMKDefaults.materialCount, sizeof(Material));

    if (FileSys_Exists("global.mad", NULL) != false) {
        materialIni.Init("global.mad");
    } else {
        materialIni.Init(NULL);
    }

    pCurrMat[0] = pCurrMat[1] = NULL;
    pPCMatChanges = NULL;

    frameCounter = 0;
    updateEnabled = 1;

    CreateFromRawData("capture", (void*)&captureTexData, 2, 128, 128); // GX_TF_RGB565
    
    GXInitTexObj(&restorationTexObj, (void*)&restorationTexData, 128, 128, GX_TF_RGB565, GX_REPEAT, GX_REPEAT, GX_FALSE);
    GXInitTexObjLOD(&restorationTexObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXInitTexObj(&rawCaptureTexObj, (void*)&rawCaptureTexData, 256, 256, GX_TF_RGB565, GX_REPEAT, GX_REPEAT, GX_FALSE);
}

void Material::DeinitModule(void) {
    materialIni.Deinit();
    materials.Deinit();
}

Material* Material::CreateFromRawData(char* pName, void* pData, int texFmt, int texWidth, int texHeight) {
    Material* pMat = materials.GetNextEntry();
    pMat->referenceCount = 1;
    pMat->InitFromMatDefs(pName);
    pMat->unk54 = Texture::CreateFromRawData(pName, pData, texFmt, texWidth, texHeight);
    return pMat;
}

void Material::SetTextureAlias(Material* arg1) {
    Texture* alias = arg1->GetTexture();

    if (alias == unk54) {
        alias = NULL;
    }

    if (unk58 != NULL) {
        unk58->Destroy();
    }
    
    unk58 = alias;
    
    if (alias != NULL) {
        alias->referenceCount++;
    }

    SetFlags(0x1000);
}

void Material::SetTextureAlias(Texture* pTexAlias) {
    if (pTexAlias == unk54) {
        pTexAlias = NULL;
    }

    if (unk58 != NULL) {
        unk58->Destroy();
    }

    unk58 = pTexAlias;

    if (pTexAlias != NULL) {
        pTexAlias->referenceCount++;
    }

    flags |= 0x1000;
}

Material** Material::GetMaterialList(void) {
	return materials.GetMem();
}

Material* Material::CreateMpegTarget(char* pName, void* pData, int width, int height) {
    Material* pMat = materials.GetNextEntry();
    pMat->referenceCount = 1;
    pMat->InitFromMatDefs(pName);
    pMat->type = Type_Prelit;
    pMat->blendMode = Blend_Opaque;
    pMat->flags |= (2 | 4);
    pMat->unk54 = Texture::CreateMpegTarget(pName, pData, width, height);
    return pMat;
}

void Material::Use(void) {
    GXTexObj texObj;
    GXTexObj texObj1;
    GXTexObj texObj2;
    static uint currentMixedColor = 0;

    if (this == pCurrMat[0] && !(flags & 0x1000)) {
        if (*(int*)&Material_MixedColor == currentMixedColor) {
            return;
        }
    }

    ClearFlags(0x1000);

    pCurrMat[0] = this;
    if (flags & 0x1f0000 && frameCounter1 != frameCounter) {
        Update();
    }
    
    Texture* defTex = (Texture*)GetRenderStateData(RENDERSTATE_TEXTURE);
    Texture* pTex = defTex ? defTex : GetTexture();
    
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);

    currentMixedColor = *(int*)&Material_MixedColor;
    int numTexGens = 1;
    int numTevStages = 1;
    int numIndStages = 0;

    if (*(uint*)&Material_MixedColor != -1) {
        GXColor color = Material_MixedColor;
        GXSetTevColor(GX_TEVREG0, color);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_A0, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        numTevStages = 2;
    }

    GXSetTevDirect(GX_TEVSTAGE0);
    
    if (gRenderState.alpha < 0 || gRenderState.alpha > 255) {
        GXSetDstAlpha(GX_ENABLE, 127);
    } else {
        GXSetDstAlpha(GX_ENABLE, gRenderState.alpha);
    }

    if (pTex->bMpegTarget != false) {
        // setup GX for video
		
        GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
        GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 255);

        numTexGens = 2;
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

        numTevStages = 4;

        // setup Tev stage 0
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_A0);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
        GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
        GXSetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);

        // setup Tev stage 1
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
        GXSetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);

        // setup Tev stage 2
        GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
        GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
        GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevSwapMode(GX_TEVSTAGE2, GX_TEV_SWAP0, GX_TEV_SWAP0);

        // setup Tev stage 3
        GXSetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE3, GX_CC_APREV, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevSwapMode(GX_TEVSTAGE3, GX_TEV_SWAP0, GX_TEV_SWAP0);
        GXSetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K2);
		
        GXSetTevColorS10(GX_TEVREG0, (GXColorS10){0xffa6, 0x0000, 0xff8e, 0x0087});
        // set KColors for Tev stages 0, 1, 2
        GXSetTevKColor(GX_KCOLOR0, (GXColor){0, 0, 226, 88});
        GXSetTevKColor(GX_KCOLOR1, (GXColor){179, 0, 0, 182});
        GXSetTevKColor(GX_KCOLOR2, (GXColor){255, 0, 255, 128});
		
        GXInitTexObj(&texObj, pTex->pYData, pTex->width, pTex->height, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texObj, GX_TEXMAP0);
        
        GXInitTexObj(&texObj1, pTex->pUData, pTex->width >> 1, pTex->height >> 1, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texObj1, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texObj1, GX_TEXMAP1);
        
        GXInitTexObj(&texObj2, pTex->pVData, pTex->width >> 1, pTex->height >> 1, GX_TF_I8, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(&texObj2, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texObj2, GX_TEXMAP2);
    } else {
        switch (type) {
            case Type_EnvMap:
                Matrix multMat;
                View* currView = View::GetCurrent();
                multMat = currView->unk88;
                multMat.Multiply3x3(&multMat, &currView->unkC8);
                Matrix multImm;
                multImm.Row0()->Set(0.5f, 0.0f, 0.0f, 0.0f);
                multImm.data[1][0] = 0.0f;
                multImm.data[1][1] = -0.5f;
                multImm.data[1][2] = 0.0f;
                multImm.data[1][3] = 0.0f;
                multImm.data[2][0] = 0.0f;
                multImm.data[2][1] = 0.0f;
                multImm.data[2][2] = 1.0f;
                multImm.data[2][3] = 0.0f;
                multMat.Multiply3x3(&multMat, &multImm);
                multMat.data[3][0] = 0.5f;
                multMat.data[3][1] = 0.5f;
                multMat.data[3][2] = 0.0f;
                multMat.data[3][3] = 1.0f;
                multMat.Transpose(&multMat);
                GXLoadTexMtxImm(multMat.data, GX_TEXMTX0, GX_MTX2x4);
                GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
                numTexGens = 1;
                pTex->Use();
                break;
            case Type_IndirectWater:
                bWaterUpdate = true;
                GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
                GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO);
                GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
                pTex->Use();
                GXLoadTexObj(&waterIndTexObj, GX_TEXMAP1);

                numTexGens = 2;
                numTevStages = 1;
                numIndStages = 1;

                static float uOffset = 0.5f;
                static float vOffset = 0.5f;

                if (unkCC) {
                    // Move the water texture with the camera
                    Mtx44 ortho;
                    Vector vec = View::GetCurrent()->mCamPos;
                    C_MTXOrtho(
                        &ortho, 
                        25000.0f + vec.z, 
                        -25000.0f + vec.z,
                        -25000.0f + vec.x, 
                        25000.0f + vec.x, 
                        0.0f, 1000.0f
                    );
                    
                    float tmp02 = ortho.data[0][2];
                    float tmp12 = ortho.data[1][2];
                    float tmp22 = ortho.data[2][2];

                    ortho.data[0][2] = ortho.data[0][1];
                    ortho.data[1][2] = ortho.data[1][1];
                    ortho.data[2][2] = ortho.data[2][1];

                    ortho.data[0][1] = tmp02;
                    ortho.data[1][1] = tmp12;
                    ortho.data[2][1] = tmp22;

                    Mtx44 ident;
                    C_MTXIdentity(&ident);
                    ident.data[0][2] = uOffset;
                    ident.data[1][2] = vOffset;
                    ident.data[2][2] = 1.0f;
                    GXLoadTexMtxImm(ortho.data, GX_TEXMTX0, GX_MTX3x4);
                    GXLoadTexMtxImm(ident.data, GX_PTTEXMTX0, GX_MTX3x4);
                    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTTEXMTX0);
                } else {
                    Mtx24 texMat = {
                        unk60.data[0][0], unk60.data[1][0], unk60.data[2][0], unk60.data[3][0],
                        unk60.data[0][1], unk60.data[1][1], unk60.data[2][1], unk60.data[3][1]
                    };
                    GXLoadTexMtxImm(texMat.data, GX_TEXMTX0, GX_MTX2x4);
                    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
                }

                if (unkCD) {
                    Mtx44 texMtx;
                    // indirectWaterVec.x/.y are the texture scales for the water "waves"
                    // the larger the value, the more subdivisions there are within the water texture
                    float xcomp = 25000.0f / indirectWaterVec.x;
                    float ycomp = 25000.0f / indirectWaterVec.y;
                    C_MTXOrtho(&texMtx, xcomp, -xcomp, -ycomp, ycomp, 0.0f, 1e+09f);
                    float tmp02 = texMtx.data[0][2];
                    float tmp12 = texMtx.data[1][2];
                    float tmp22 = texMtx.data[2][2];
                    texMtx.data[0][2] = texMtx.data[0][1];
                    texMtx.data[1][2] = texMtx.data[1][1];
                    texMtx.data[2][2] = texMtx.data[2][1];
                    texMtx.data[0][1] = tmp02;
                    texMtx.data[1][1] = tmp12;
                    texMtx.data[2][1] = tmp22;
                    Mtx44 ident;
                    C_MTXIdentity(&ident);
                    ident.data[0][2] = uOffset;
                    ident.data[1][2] = vOffset;
                    ident.data[2][2] = 1.0f;
                    GXLoadTexMtxImm(texMtx.data, GX_TEXMTX1, GX_MTX3x4);
                    GXLoadTexMtxImm(ident.data, GX_PTTEXMTX0, GX_MTX3x4);
                    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1, GX_FALSE, GX_PTTEXMTX0);
                } else {
                    Mtx24 mtx = {
                        indirectWaterVec.x, 0.0f,               0.0f, 0.0f, 
                        0.0f,               indirectWaterVec.y, 0.0f, 0.0f
                    };
                    GXLoadTexMtxImm(mtx.data, GX_TEXMTX1, GX_MTX2x4);
                    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
                }

                Mtx23 mat23 = {
                    indirectWaterVec.z, 0.0f,               1.0f,
                    0.0f,               indirectWaterVec.w, 1.0f
                };

                GXSetIndTexMtx(GX_ITM_0, mat23.data, 1);
                GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
                GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
                GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
                break;
            default:
                Mtx24 mat24 = {
                    unk60.data[0][0], unk60.data[1][0], unk60.data[2][0], unk60.data[3][0],
                    unk60.data[0][1], unk60.data[1][1], unk60.data[2][1], unk60.data[3][1]
                };
                GXLoadTexMtxImm(mat24.data, GX_TEXMTX0, GX_MTX2x4);
                GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
                pTex->Use();
                break;
        }

        switch (blendMode) {
            case Blend_Opaque:
            case Blend_Blend:
            case Blend_Alpha:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                break;
            case Blend_Additive:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
                break;
            case Blend_Subtractive:
                GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ZERO, GX_BL_ZERO, GX_LO_NOOP);
                break;
        }

        int uVar8 = flags & 0x10;
        int uVar6 = flags & 0x8;
        if (View::GetCurrent()->bDisableZWrite != false) {
            uVar8 = 1;
        }
        GXSetZMode(uVar6 == 0, GX_LEQUAL, uVar8 == 0);
        bool zCompSetting = false;
        if ((flags & Flag_AlphaMask) || blendMode != Blend_Opaque) {
            zCompSetting = true;
        }
        GXSetZCompLoc(zCompSetting == false ? 1 : 0);
        // (currentMixedColor & 0xff) masks the alpha
        int r = unk5C * (currentMixedColor & 0xff);
        GXSetAlphaCompare(GX_GREATER, r, GX_AOP_AND, GX_ALWAYS, 255);
    }

    GXSetNumTexGens(numTexGens);
    GXSetNumTevStages(numTevStages);
    GXSetNumIndStages(numIndStages);
}

// arg0 is unused, always passed as -1
void Material::UseNone(int arg0) {
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetZCompLoc(1);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 255);
    GXSetColorUpdate(GX_ENABLE);

    if (gRenderState.alpha < 0 || gRenderState.alpha > 255) {
        GXSetDstAlpha(GX_ENABLE, 127);
    } else {
        GXSetDstAlpha(GX_ENABLE, gRenderState.alpha);
    }

    GXSetNumTevStages(1);
    GXSetNumIndStages(0);
    GXSetNumTexGens(1);

    GXSetTevDirect(GX_TEVSTAGE0);

    pCurrMat[0] = NULL;
}

void Material::CaptureDrawBuffer(float arg1, float arg2, float arg3, float arg4) {
    void* imgPtr;
    u16 width;
    u16 height;
    int format;
    int wrap_s;
    int wrap_t;
    u8 mipmap;

    Texture* pTex = GetTexture();
    if (pTex->pFileData == NULL) {
        return;
    }

    GXGetTexObjAll(&pTex->texObj, &imgPtr, &width, &height, (u8*)&format, (u8*)&wrap_s, (u8*)&wrap_t, (u8*)&mipmap);
    pCaptureTexture = pTex;
    _GXRenderModeObj* rmodeObj = DEMOGetRenderModeObj();
    GXSetCopyFilter(0, 0, 0, 0);
    float proj[4][4];

    // Get original projection matrix
    float projection[7];
    GXGetProjectionv(projection);

    // Create Orthographic projection matrix
    proj[0][0] = 1.0f / (float)(gDisplay.unk10 / 2);
    proj[0][1] = 0.0f;
    proj[0][2] = 0.0f;
    proj[0][3] = -1.0f;
    proj[1][0] = 0.0f;
    proj[1][1] = -1.0f / (float)((VIGetTvFormat() == VI_PAL) ? 248 : 224);
    proj[1][2] = 0.0f;
    proj[1][3] = 1.0f;
    proj[2][0] = 0.0f;
    proj[2][1] = 0.0f;
    proj[2][2] = 1.0f;
    proj[2][3] = -1.0f;
    proj[3][0] = 0.0f;
    proj[3][1] = 0.0f;
    proj[3][2] = 0.0f;
    proj[3][3] = 1.0f;
    GXSetProjection(proj, GX_ORTHOGRAPHIC); // Use Orthographic projection matrix
    GXSetCurrentMtx(GX_PNMTX1);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    Material::UseNone(-1);

    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);

    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);

    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_AND);

    GXSetDstAlpha(GX_ENABLE, 255);

    Mtx24 texMat = {
        1.0f, 0.0f, 0.0f, 0.0f, 
        0.0f, 1.0f, 0.0f, 0.0f
    };

    GXLoadTexMtxImm(texMat.data, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCopySrc(0, 0, 128, 128);
    GXSetTexCopyDst(128, 128, GX_TF_RGB565, GX_FALSE);
    GXCopyTex((void*)&restorationTexData, GX_FALSE);
    GXSetTexCopySrc(0, 0, 512, 512);
    GXSetTexCopyDst(256, 256, GX_TF_RGB565, GX_TRUE);
    GXCopyTex((void*)&rawCaptureTexData, GX_FALSE);
    GXLoadTexObj(&rawCaptureTexObj, GX_TEXMAP0);

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);

    GXPosition3f32(0.0f, 0.0f, 0.0f);
    GXColor4u8(255, 255, 255, 255);
    GXTexCoord2f32(0.0f, 0.0f);
    
    GXPosition3f32(0.0f, (float)height, 0.0f);
    GXColor4u8(255, 255, 255, 255);
    GXTexCoord2f32(0.0f, 0.875f);
    
    GXPosition3f32((float)width, 0.0f, 0.0f);
    GXColor4u8(255, 255, 255, 255);
    GXTexCoord2f32(1.0f, 0.0f);
    
    GXPosition3f32((float)width, (float)height, 0.0f);
    GXColor4u8(255, 255, 255, 255);
    GXTexCoord2f32(1.0f, 0.875f);

    GXSetTexCopySrc(0, 0, width, height);
    GXSetTexCopyDst(width, height, (GXTexFmt)format, GX_FALSE);
    GXCopyTex(imgPtr, GX_TRUE);
    GXLoadTexObj(&restorationTexObj, GX_TEXMAP0);

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);

    GXPosition3f32(0.0f, 0.0f, 0.0f);
    GXColor4u8(255, 255, 255, 255);
    GXTexCoord2f32(0.0f, 0.0f);
    
    GXPosition3f32(0.0f, (float)height, 0.0f);
    GXColor4u8(255, 255, 255, 255);
    GXTexCoord2f32(0.0f, 1.0f);
    
    GXPosition3f32((float)width, 0.0f, 0.0f);
    GXColor4u8(255, 255, 255, 255);
    GXTexCoord2f32(1.0f, 0.0f);
    
    GXPosition3f32((float)width, (float)height, 0.0f);
    GXColor4u8(255, 255, 255, 255);
    GXTexCoord2f32(1.0f, 1.0f);

    GXSetProjectionv(projection);
    GXSetCurrentMtx(GX_PNMTX0);

    Material::UseNone(-1);

    GXSetCopyFilter(rmodeObj->aa, rmodeObj->sample_pattern, GX_TRUE, &rmodeObj->vfilter[0]);
}

void Material::Update(void) {
    float frameSpeed = gDisplay.dt * (float)(frameCounter - frameCounter1);
    frameCounter1 = frameCounter;
    int matFlags = flags;
    if (matFlags & 0x10000) {
        unk60.Row3()->x = fmod(unk60.data[3][0] + (frameSpeed * unkA0), 1.0);
        unk60.Row3()->y = fmod(unk60.data[3][1] + (frameSpeed * unkA4), 1.0);
    } else if (matFlags & 0x20000) {
        angle += frameSpeed;
        if (angle > unkA8) {
            angle -= unkA8;
            unk60.Row3()->x += unkA0;
            unk60.Row3()->y += unkA4;
            if (unk60.data[3][0] >= 0.99f) {
                unk60.data[3][0] = 0.0f;
            }
            
            if (unk60.data[3][1] >= 0.99f) {
                unk60.data[3][1] = 0.0f;
            }
        }
    } else if (matFlags & 0x40000) {
        Matrix mat;
        Matrix mat1;
        angle += frameSpeed * unkA8;
        NormaliseAngle(angle);
        Vector vec = {0.0f, 0.0f, 0.0f, 0.0f};
        vec.x = -unkA0;
        vec.y = -unkA4;
        mat.SetRotationToNone();
        mat.SetTranslation(&vec);
        Vector vec1 = {0.0f, 0.0f, 0.0f, 0.0f};
        vec1.x = unkA0;
        vec1.y = unkA4;
        mat1.SetRotationRoll(angle);
        mat1.SetTranslation(&vec1);
        unk60.Multiply4x4(&mat, &mat1);
    } else if (matFlags & 0x80000) {
        Matrix mat;
        Matrix mat1;
        angle += frameSpeed * unkA8;
        NormaliseAngle(angle);
        float roll = _table_sinf(angle) * DegToRad(unkB0);
        Vector vec = {0.0f, 0.0f, 0.0f, 0.0f};
        vec.x = -unkA0;
        vec.y = -unkA4;
        mat.SetRotationToNone();
        mat.SetTranslation(&vec);
        Vector vec1 = {0.0f, 0.0f, 0.0f, 0.0f};
        vec1.x = unkA0;
        vec1.y = unkA4;
        mat1.SetRotationRoll(roll);
        mat1.SetTranslation(&vec1);
        unk60.Multiply4x4(&mat, &mat1);
    } else if (matFlags & 0x100000) {
        Vector pyr;
        unk60.SetIdentity();
        View::GetCurrent()->unk48.GetRotationPYR(&pyr);
        unk60.data[0][0] = unkB4;
        unk60.data[1][1] = unkB8;
        unk60.data[3][0] = unkBC + (1 / (2 * PI)) * pyr.y * unkA0;
        unk60.data[3][1] = unkC0 + (1 / (2 * PI)) * pyr.x * unkA4;
    }
}

void Material::ScrollUVOffset(float arg1, float arg2) {
    arg1 = unk60.data[3][0] + arg1;
    if (arg1 < 0.0f) {
        arg1 += 1.0f;
    } else if (arg1 >= 1.0f) {
        arg1 -= 1.0f;
    }

    arg2 = unk60.data[3][1] + arg2;
    if (arg2 < 0.0f) {
        arg2 += 1.0f;
    } else if (arg2 >= 1.0f) {
        arg2 -= 1.0f;
    }

    unk60.data[3][0] = arg1;
    unk60.data[3][1] = arg2;
    flags |= 0x2000;
}

void Material::UpdateCounter(void) {
    if (updateEnabled) {
        frameCounter++;
    }
}
