#ifndef RENDERTEXTURE_H
#define RENDERTEXTURE_H

#include "common/View.h"
#include "common/KromeIni.h"

struct RenderTexture {
    Material* pRenderMat;
    View mView;
    int OverlayStage;
    bool bHasRenderTarget;

    void Deinit(void);
    void LoadLine(KromeIniLine* pLine);

    virtual void Render(void) = 0;
};

struct WaterSlideEffect : RenderTexture {
    Material* pUnderlay;
    Material* pOverlay;

    void Deinit(void);
    void LoadLine(KromeIniLine* pLine);
    void Update(void);

    virtual void Render(void);
};

struct CausticEffect : RenderTexture {
    float unkData[3];
    Material* pMat;

    void Deinit(void);
    void LoadLine(KromeIniLine* pLine);
    void LoadDone(void);
    void Update(void);

    virtual void Render(void);
};

struct LavaEffect : RenderTexture {
    Material* pMaterials[3];

    void Deinit(void);
    void LoadLine(KromeIniLine* pLine);
    void Update(void);

    virtual void Render(void);
};

void RenderTexture_LoadResources(void);
void RenderTexture_Init(void);
void RenderTexture_Deinit(void);

void RenderTexture_ResetWaterBlendValues(void);

void RenderTexture_RenderWater(void);

void RenderTexture_Update(void);

void RenderTexture_SetWaterLow(char*, float height);
void RenderTexture_SetWaterHigh(char*, float height);
void RenderTexture_SetWaterOutput(char* pOutputName);

void RenderTexture_WaterSlideLine(KromeIniLine*);
void RenderTexture_WaterSlideEnd(void);

void RenderTexture_CausticsLine(KromeIniLine*);
void RenderTexture_CausticsEnd(void);

void RenderTexture_LavaLine(KromeIniLine*);
void RenderTexture_LavaEnd(void);

void RenderTexture_RenderForOverlay(int);
void RenderTexture_ResetVRam(void);


#endif // RENDERTEXTURE_H
