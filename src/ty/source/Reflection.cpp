#include "ty/Reflection.h"
#include "ty/Ty.h"
#include "Dolphin/gx.h"

static float ref_oldViewport[6];

static Material* pRefMat;

static u32 ref_oldScissorX;
static u32 ref_oldScissorY;
static u32 ref_oldScissorWidth;
static u32 ref_oldScissorHeight;

void ReflectionStruct::Init(void) {
    unk308 = unk310 = 100.0f;
    unk30C = 40.0f;

    mpRefMat = Material::Create("tyreflection");

    mWobbleTex.Init(7, 7);
}

void ReflectionStruct::Deinit(void) {
    mWobbleTex.Deinit();
    mpRefMat = NULL;
}

extern "C" {
    void GXGetScissor(u32*, u32*, u32*, u32*);
    void GXGetViewportv(float* vp);
    void memcpy(void*, void*, int);
};

void ReflectionStruct::Render(void) {
    if (!ty.mContext.water.bValid || ty.mContext.water.pos.y - ty.pos.y >= 200.0f || unk314) {
        return;
    }

    pRefMat = mpRefMat;

    GXTexObj* pTexObj = &pRefMat->unk54->texObj;

    u16 width = GXGetTexObjWidth(pTexObj);
    u16 height = GXGetTexObjHeight(pTexObj);

    GXGetViewportv(ref_oldViewport);

    GXSetViewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);

    GXGetScissor(&ref_oldScissorX, &ref_oldScissorY, &ref_oldScissorWidth, &ref_oldScissorHeight);

    GXSetScissor(0, 0, width, height);

    Material::UseNone(-1);

    View* pSavedView = View::pCurrentView;

    Vector tyPos = ty.pos;
    tyPos.y += 55.0f;

    Vector dir;
    dir.Sub(&pSavedView->mCamPos, &tyPos);

    gRenderState.alpha = 255;


    Tools_EnableWideScreen(pSavedView, pGameSettings->unk5 == 1);
    mView.Use();
    mView.ClearBuffer(0, 0, 0, 0);

    Vector camPos = pSavedView->mCamPos;
    camPos.y = (ty.mContext.water.pos.y * 2.0f) - camPos.y;

    mView.SetCameraLookAt(&camPos, &dir);

    mView.SetAspectRatio(1.0f, 0.85f);
    
    ty.pModel->pAnimation->CalculateMatrices();

    memcpy(
        ty.pReflectionModel->pMatrices,
        ty.pModel->pMatrices,
        ty.pReflectionModel->GetNmbrOfMatrices() * sizeof(Matrix)
    );

    ty.pReflectionModel->Draw(NULL);

    ty.mBoomerangManager.DrawReflection();

    gb.ResetLight();

    Tools_EnableWideScreen(pSavedView, pGameSettings->unk5 == 1);
    pSavedView->Use();

    Material::UseNone(-1);

    GXSetViewport(
        ref_oldViewport[0], ref_oldViewport[1],
        ref_oldViewport[2], ref_oldViewport[3],
        ref_oldViewport[4], ref_oldViewport[5]
    );
    GXSetScissor(ref_oldScissorX, ref_oldScissorY, ref_oldScissorWidth, ref_oldScissorHeight);

    Material::UseNone(-1);

    gRenderState.alpha = -1;
}

void ReflectionStruct::Draw(void) {
    if (!ty.mContext.water.bValid || ty.mContext.water.pos.y - ty.pos.y >= 200.0f || unk314) {
        return;
    }

    mpRefMat->blendMode = Blend_Alpha;
    mpRefMat->flags |= 0x10;

    mWobbleTex.Draw(mpRefMat, true);

    mpRefMat->blendMode = Blend_Blend;
    mpRefMat->flags &= ~0x10;
}

View* GameCamera_View(void);

void ReflectionStruct::Update(void) {
    mWobbleTex.unk4 += 1.0f;

    if (GameCamera_View()->unk48.Row3()->y < ty.mContext.water.pos.y) {
        unk314 = true;
        unk308 = -100.0f;
    } else {
        unk314 = false;
        unk308 = 100.0f;
    }
}
