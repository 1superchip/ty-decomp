#include "ty/heatflare.h"
#include "common/PtrListDL.h"
#include "common/Material.h"

Material* pCaptureMaterial;
static PtrListDL<HeatFlare> heatFlares;
static bool bHeatFlares = false;
static bool bScreenGrabbed = false;

void HeatFlare_LoadResources(void) {
    pCaptureMaterial = Material::Create("capture");
}

void HeatFlare_Init(void) {
    if (bHeatFlares == false) {
        heatFlares.Init(200, sizeof(HeatFlare));
        bHeatFlares = true;
    }
}

void HeatFlare_Deinit(void) {
    if (bHeatFlares != false) {
        heatFlares.Deinit();
        bHeatFlares = false;
    }
}

/*
// Stripped function
HeatFlare* HeatFlare_Add(Vector* pVec, float f1) {
    if (!bHeatFlares) {
        return NULL;
    }
    if (!heatFlares.IsFull()) {
        HeatFlare* pFlare = heatFlares.GetNextEntry();
        pFlare->unk0 = *pVec;
        pFlare->unk0.y += 200.0f;
        pFlare->unk10 = f1;
        pFlare->unk14 = 0.0f;
        pFlare->unk18 = true;
        return pFlare;
    }
    return NULL;
}
*/

/*
// Stripped function
// Enables a HeatFlare?
void HeatFlare_On(void* pVoidHeatFlare, int a1) {
    if (bHeatFlares) {
        HeatFlare* pHeatFlare = (HeatFlare*)pVoidHeatFlare;
        pHeatFlare->unk18 = a1;
    }
}
*/

void HeatFlare_Update(void) {
    bScreenGrabbed = false;
}

void HeatFlare_Draw(void) {
    if (bHeatFlares) return;
}

void HeatFlare_Unlock(void) {
    return;
}
