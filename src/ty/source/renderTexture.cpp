#include "ty/RenderTexture.h"
#include "ty/global.h"
#include "ty/Hero.h"

extern "C" int stricmp(char*, char*);

static float scroll[3];

static View waterView;

static WaterSlideEffect waterslideEffect;
static CausticEffect causticEffect;
static LavaEffect lavaEffect;

Material* pWaterMat = NULL;
static Material* pWaterLow = NULL;
static Material* pWaterHigh = NULL;
static Material* pWaterOutput = NULL;

static float waterLowHeight = 0.0f;
static float waterHighHeight = 0.0f;

void RenderTexture_LoadResources(void) {
    waterslideEffect.bHasRenderTarget = false;
    causticEffect.bHasRenderTarget = false;
    lavaEffect.bHasRenderTarget = false;
}

void RenderTexture_Init(void) {
    for (int i = 0; i < ARRAY_SIZE(scroll); i++) {
        scroll[i] = 0.0f;
    }

    pWaterMat = NULL;

    if (gb.level.waterMaterial) {
        pWaterMat = Material::Create(gb.level.waterMaterial);
    }
}

void RenderTexture_Deinit(void) {
    if (pWaterMat) {
        pWaterMat->Destroy();
        pWaterMat = NULL;
    }

    RenderTexture_ResetWaterBlendValues();

    if (waterslideEffect.bHasRenderTarget) {
        waterslideEffect.Deinit();
    }

    if (causticEffect.bHasRenderTarget) {
        causticEffect.Deinit();
    }

    if (lavaEffect.bHasRenderTarget) {
        lavaEffect.Deinit();
    }
}

void RenderTexture_RenderWater(void) {

}

void RenderTexture_Update(void) {
    fmod_inline(scroll[0], 0.002f);
    fmod_inline(scroll[1], 0.002025f);
    fmod_inline(scroll[2], 0.00125f);
}

void RenderTexture_ResetWaterBlendValues(void) {
    if (pWaterLow) {
        pWaterLow->Destroy();
        pWaterLow = NULL;
    }

    if (pWaterHigh) {
        pWaterHigh->Destroy();
        pWaterHigh = NULL;
    }
    
    if (pWaterOutput) {
        pWaterOutput->Destroy();
        pWaterOutput = NULL;
    }

    waterLowHeight = waterHighHeight = 0.0f;
}

void RenderTexture_SetWaterLow(char* name, float height) {
    pWaterLow = Material::Create(name);
    waterLowHeight = height;
}

void RenderTexture_SetWaterHigh(char* name, float height) {
    pWaterHigh = Material::Create(name);
    waterHighHeight = height;
}

void RenderTexture_SetWaterOutput(char* pOutputName) {
    pWaterOutput = Material::CreateRenderTarget(pOutputName, 128, 128, 1);
}

void RenderTexture_WaterSlideLine(KromeIniLine* pLine) {
    waterslideEffect.LoadLine(pLine);
}

void RenderTexture_WaterSlideEnd(void) {

}

void RenderTexture_CausticsLine(KromeIniLine* pLine) {
    causticEffect.LoadLine(pLine);
}

void RenderTexture_CausticsEnd(void) {
    causticEffect.LoadDone();
}

void RenderTexture_LavaLine(KromeIniLine* pLine) {
    lavaEffect.LoadLine(pLine);
}

void RenderTexture_LavaEnd(void) {
    
}

void RenderTexture::LoadLine(KromeIniLine* pLine) {
    if (stricmp(pLine->pFieldName, "RenderTexture") == 0) {
        bHasRenderTarget = true;
    }
    
    if (stricmp(pLine->pFieldName, "OverlayStage") == 0) {
        pLine->AsInt(0, &OverlayStage);
    }
}

void RenderTexture::Deinit(void) {
    if (pRenderMat) {
        pRenderMat->Destroy();
        pRenderMat = NULL;
    }
    
    bHasRenderTarget = false;
}

void WaterSlideEffect::LoadLine(KromeIniLine* pLine) {
    char* pString;
    
    if (stricmp(pLine->pFieldName, "Underlay") == 0) {
        pLine->AsString(0, &pString);

        pUnderlay = Material::Create(pString);
    } else if (stricmp(pLine->pFieldName, "Overlay") == 0) {
        pLine->AsString(0, &pString);

        pOverlay = Material::Create(pString);
    }
    
    this->RenderTexture::LoadLine(pLine);
}

void WaterSlideEffect::Deinit(void) {
    if (pUnderlay) {
        pUnderlay->Destroy();
    }
    
    if (pOverlay) {
        pOverlay->Destroy();
    }

    pUnderlay = pOverlay = NULL;
    
    this->RenderTexture::Deinit();
}

void WaterSlideEffect::Render(void) {

}

void WaterSlideEffect::Update(void) {
    
}

void CausticEffect::LoadLine(KromeIniLine* pLine) {
    if (stricmp(pLine->pFieldName, "Material") == 0) {
        char* pString;
        pLine->AsString(0, &pString);

        pMat = Material::Create(pString);
    }
    
    this->RenderTexture::LoadLine(pLine);
}

void CausticEffect::LoadDone(void) {
    for (int i = 0; i < 3; i++) {
        unkData[i] = 0.0f;
    }
}

void CausticEffect::Deinit(void) {
    if (pMat) {
        pMat->Destroy();
    }
    
    pMat = NULL;
    
    this->RenderTexture::Deinit();
}

void CausticEffect::Render(void) {
    Tools_RenderToTarget(pMat, 0.0f, unkData[0], 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    Tools_RenderToTarget(pMat, unkData[1], 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f);
    Tools_RenderToTarget(pMat, unkData[2], unkData[3], 0.33f, 0.0f, 1.0f, 1.0f, 0.0f);
}

void CausticEffect::Update(void) {
    fmod_inline(unkData[0], 0.002f);
    fmod_inline(unkData[1], 0.002025f);
    fmod_inline(unkData[2], 0.00125f);
}

void LavaEffect::LoadLine(KromeIniLine* pLine) {
    char* pString;
    
    if (stricmp(pLine->pFieldName, "Material1") == 0) {
        pLine->AsString(0, &pString);

        pMaterials[0] = Material::Create(pString);
    }
    
    if (stricmp(pLine->pFieldName, "Material2") == 0) {
        pLine->AsString(0, &pString);

        pMaterials[1] = Material::Create(pString);
    }
    
    if (stricmp(pLine->pFieldName, "Material3") == 0) {
        pLine->AsString(0, &pString);

        pMaterials[2] = Material::Create(pString);
    }
    
    this->RenderTexture::LoadLine(pLine);
}

void LavaEffect::Deinit(void) {
    for (int i = 0; i < 3; i++) {
        if (pMaterials[i]) {
            pMaterials[i]->Destroy();
        }

        pMaterials[i] = NULL;
    }
    
    this->RenderTexture::Deinit();
}

void LavaEffect::Render(void) {
    Tools_RenderToTarget(pMaterials[0], 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    Tools_RenderToTarget(pMaterials[1], 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    Tools_RenderToTarget(pMaterials[2], 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
}

void LavaEffect::Update(void) {
    
}

void RenderTexture_RenderForOverlay(int) {

}

void RenderTexture_ResetVRam(void) {

}
