#include "ty/heatflare.h"
#include "common/PtrListDL.h"
#include "common/Model.h" // material is currently in this

// .sbss
Material* pCaptureMaterial;
static PtrListDL<HeatFlare> heatFlares;
static bool bHeatFlares;
static bool bScreenGrabbed;

void HeatFlare_LoadResources(void) {
	pCaptureMaterial = Material::Create("capture");
}

// https://decomp.me/scratch/Yr0Jr
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