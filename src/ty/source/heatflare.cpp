#include "ty/heatflare.h"
#include "common/PtrListDL.h"
#include "common/Material.h"
#include "ty/global.h"

Material* pCaptureMaterial;
static PtrListDL<HeatFlareInfo> heatFlares;
static bool bHeatFlares = false;
static bool bScreenGrabbed = false;

void HeatFlare_LoadResources(void) {
    pCaptureMaterial = Material::Create("capture");
}

void HeatFlare_Init(void) {
    if (!bHeatFlares) {
        heatFlares.Init(200, sizeof(HeatFlareInfo));
        bHeatFlares = true;
    }
}

void HeatFlare_Deinit(void) {
    if (bHeatFlares) {
        heatFlares.Deinit();
        bHeatFlares = false;
    }
}

// Stripped function
HeatFlareInfo* HeatFlare_Add(Vector* pVec, float f1) {
    if (!bHeatFlares) {
        return NULL;
    }

    if (!heatFlares.IsFull()) {
        HeatFlareInfo* pFlare = heatFlares.GetNextEntry();

        pFlare->unk0 = *pVec;
        pFlare->unk0.y += 200.0f;
        pFlare->unk10 = f1;
        pFlare->unk14 = 0.0f;
        pFlare->unk18 = 1;

        return pFlare;
    } else {
        return NULL;
    }
}

// Stripped function
// Enables a HeatFlare?
// void HeatFlare_On(void* pVoidHeatFlare, int a1) {
//     if (bHeatFlares) {
//         HeatFlareInfo* pHeatFlare = (HeatFlareInfo*)pVoidHeatFlare;
//         pHeatFlare->unk18 = a1;
//     }
// }

void HeatFlare_Update(void) {
    bScreenGrabbed = false;
    if (bHeatFlares) {

    }
}

void HeatFlare_Draw(void) {
    if (!bHeatFlares) {
        return;
    }
}

void HeatFlare_Unlock(void) {
    return;
}
