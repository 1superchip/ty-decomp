#include "types.h"
#include "common/Model.h"
#include "common/Material.h"
#include "ty/LensFlare.h"
#include "common/Blitter.h"

struct View;

// from another file
struct ZCheckRequest {
    Vector unk0;
    int unk10;
    View* unk14;
    int unk18;
    float unk1C;
    float unk20;
    float unk24;
    float unk28;
    float unk2C;
};
ZCheckRequest* System_CreateZRequest(void);
void System_DestroyZRequest(ZCheckRequest*);

static char* LF_TEXTURE_NAME = "fx_081";
static char* LF_SUNTEXTURE_NAME = "fx_083";

static Material* pMaterial;
static Material* pSunMaterial;
static ZCheckRequest* pZRequest;
static int wait;

static ZCheckRequest zRequestLast;

/*
// these are unused variables that would have been used by code that was either stripped or unimplemented
static Vector centre;
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
        pZRequest->unk1C = 1.0f;
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