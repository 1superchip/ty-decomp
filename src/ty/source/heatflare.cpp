#include "ty/heatflare.h"
#include "common/PtrListDL.h"
#include "common/Material.h"

Material* pCaptureMaterial;
static PtrListDL<HeatFlare> heatFlares;
static bool bHeatFlares;
static bool bScreenGrabbed;

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

void HeatFlare_Update(void) {
	bScreenGrabbed = false;
}

void HeatFlare_Draw(void) {
	if (bHeatFlares) return;
}

void HeatFlare_Unlock(void) {
	return;
}