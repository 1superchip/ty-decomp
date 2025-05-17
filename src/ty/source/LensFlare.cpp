#include "types.h"
#include "common/Model.h"
#include "common/Material.h"
#include "ty/LensFlare.h"
#include "common/System_GC.h"
#include "common/Blitter.h"

// https://decomp.me/scratch/DcZyE

static char* LF_TEXTURE_NAME = "fx_081";
static char* LF_SUNTEXTURE_NAME = "fx_083";

static Material* pMaterial;
static Material* pSunMaterial;
static ZCheckRequest* pZRequest;
static int wait;

static ZCheckRequest zRequestLast;

/*
// these are unused variables that would have been used by code that was either stripped or unimplemented
static Vector centre = {320.0f, 256.0f, 0.0f, 0.0f};
float targetBlend = 1.0f;
static int flares[7] = { 0, 1, 2, 3, 2, 1, 0 }; // i think this are actually 7 not 8
static Blitter_Image lensFlareImages[4];
static Blitter_Image sunImage;
static float scales[7] = {
    0.5f, 1.7f, 1.0f, 1.0f, 1.3f, 2.0f, 3.0f
};
*/

void LensFlare_Init(void) {
    wait = 0;
    if (pMaterial == NULL) {
        pMaterial = Material::Create(LF_TEXTURE_NAME);
        pSunMaterial = Material::Create(LF_SUNTEXTURE_NAME);
        pZRequest = System_CreateZRequest();
        pZRequest->depth = 1.0f;
    }
}

void LensFlare_Deinit(void) {
    if (pMaterial != NULL) {
        pSunMaterial->Destroy();
        pSunMaterial = NULL;
        pMaterial->Destroy();
        pMaterial = NULL;
        System_DestroyZRequest(pZRequest);
        pZRequest = NULL;
    }
}

void LensFlare_Update(void) {
    if (wait <= 0) {
        zRequestLast = *pZRequest;
        return;
    }
    wait--;
}

void LensFlare_Draw(void) {
    return;
}
