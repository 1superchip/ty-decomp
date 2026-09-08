#include "ty/Reflection.h"
#include "ty/Ty.h"
#include "Dolphin/gx.h"

static float ref_oldViewport[6];

static Material* pRefMat;

static u32 ref_oldScissorX;
static u32 ref_oldScissorY;
static u32 ref_oldScissorWidth;
static u32 ref_oldScissorHeight;

extern "C" {
    void GXGetScissor(u32*, u32*, u32*, u32*);
    void GXGetViewportv(float* vp);
    void memcpy(void*, void*, int);
    double atan2(double, double);
    inline float atan2f(float y, float x) {
        return atan2(y, x);
    }
};

void InitRender(GXTexObj* pTexObj) {
    u16 width = GXGetTexObjWidth(pTexObj);
    u16 height = GXGetTexObjHeight(pTexObj);

    GXGetViewportv(ref_oldViewport);

    GXSetViewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);

    GXGetScissor(&ref_oldScissorX, &ref_oldScissorY, &ref_oldScissorWidth, &ref_oldScissorHeight);

    GXSetScissor(0, 0, width, height);

    Material::UseNone(-1);
}

inline void CopyTex(GXTexObj* pTexObj) {
    u16 width = GXGetTexObjWidth(pTexObj);
    u16 height = GXGetTexObjHeight(pTexObj);
    void* pData = GXGetTexObjData(pTexObj);

    Material::UseNone(-1);

    GXSetTexCopySrc(0, 0, width, height);
    GXSetTexCopyDst(width, height, GX_TF_RGB5A3, GX_FALSE);

    GXCopyTex(pData, GX_TRUE);
}

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

void ReflectionStruct::Render(void) {
    if (!ty.mContext.water.bValid || ty.mContext.water.pos.y - ty.pos.y >= 200.0f || unk314) {
        return;
    }

    pRefMat = mpRefMat;

    InitRender(&pRefMat->unk54->texObj);

    gRenderState.alpha = 255;
    
    View* pSavedView = View::GetCurrent();
    
    Vector tyPos = ty.pos;
    tyPos.y = ty.mContext.water.pos.y + 55.0f;
    
    // July 1st
    // Vector tyPos2 = tyPos;
    // tyPos2.y = (ty.mContext.water.pos.y * 2.0f) - tyPos2.y;
    
    Vector dir;
    dir.Sub(&tyPos, &pSavedView->mCamPos);

    Tools_EnableWideScreen(&mView, pGameSettings->unk5 == 1);
    mView.Use();
    mView.ClearBuffer(0, 0, 0, 0);

    Vector camPos = pSavedView->mCamPos;
    camPos.y = (ty.mContext.water.pos.y * 2.0f) - camPos.y;

    mView.SetCameraLookAt(&camPos, &tyPos);

    float fov = atan2f(unk308 * 0.5f, dir.Dot(&pSavedView->mFwdDir)) * 2.0f;

    mView.SetAspectRatio(1.0f, 0.85f);
    mView.SetProjection(fov, 30.0f, pSavedView->farZ);

    ty.LightTy();
    
    ty.pModel->pAnimation->CalculateMatrices();

    int numMatrices = ty.pReflectionModel->GetNmbrOfMatrices();

    memcpy(
        ty.pReflectionModel->pMatrices,
        ty.pModel->pMatrices,
        numMatrices * sizeof(Matrix)
    );

    ty.pReflectionModel->Draw(NULL);

    ty.mBoomerangManager.DrawReflection();

    gb.ResetLight();

    Tools_EnableWideScreen(pSavedView, pGameSettings->unk5 == 1);
    pSavedView->Use();

    mWobbleTex.SetUpGrid(&tyPos, unk310, unk308, ty.mContext.water.pos.y);
    mWobbleTex.WobbleUVs(0.33f);

    CopyTex(&pRefMat->unk54->texObj);

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
