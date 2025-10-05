#include "ty/effects/Rain.h"
#include "common/System_GC.h"
#include "common/Debug.h"
#include "ty/tytypes.h"

static bool gbDrawRainModel = false;
static bool gbDrawRainDebugInfo = false;

void Rain::Init(RainInit* pNewInit) {
    pInit = pNewInit;

    pModel = Model::Create(pNewInit->pModelName, NULL);
    pMaterial = Material::Create(pNewInit->pMaterialName);

    unkC = 1.0f;
}

void Rain::Deinit(void) {
    if (pMaterial) {
        pMaterial->Destroy();

        pMaterial = NULL;
    }

    if (pModel) {
        pModel->Destroy();
        
        pModel = NULL;
    }
}

void Rain::Update(void) {
    pMaterial->ScrollUVOffset(0.0f, pInit->materialScrollSpeed * gDisplay.dt);
}

void Rain::Draw(View* pView, bool r4) {
    if (gbDrawRainModel) {
        if (!r4) {
            pModel->SetPosition(pView->unk48.Row3());
        }

        pModel->colour = pInit->color;

        pModel->colour.w *= pInit->alpha * unkC;

        pModel->Draw(NULL);

        if (gbDrawRainDebugInfo) {
            DrawDebugInfo(pView);
        }
    }
}

void Rain::DrawDebugInfo(View* pView) {
    float y = 200.0f;

    gpDebugFont->DrawText(
        Str_Printf("Pos: %s", FormatVector(pModel->matrices[0].Row3())),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_RED
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("Cyl Box: %s", FormatVector(&pModel->GetBoundingVolume(-1)->v1)),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_RED
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("         %s", FormatVector(&pModel->GetBoundingVolume(-1)->v2)),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_RED
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("alphaScale: %g", pInit->alpha * unkC),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_RED
    );

    y += 20.0f;
}
