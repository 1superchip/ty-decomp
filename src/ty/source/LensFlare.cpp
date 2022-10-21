#include "types.h"
#include "common/Model.h" // has Material defined in it
#include "ty/LensFlare.h"

// from another file
struct ZCheckRequest {
    int padding[7];
    float unk1C;
	Vector padding1;
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

// clean this function up?
void LensFlare_Update(void) {
    if (wait <= 0) {
        __memcpy((void*)&zRequestLast, (void*)pZRequest, sizeof(*pZRequest) - 0x1C);
        zRequestLast.padding[5] = pZRequest->padding[5];
        zRequestLast.padding[6] = pZRequest->padding[6];
        zRequestLast.unk1C = pZRequest->unk1C;
        zRequestLast.padding1 = pZRequest->padding1;
        return;
    }
    wait--;
}

void LensFlare_Draw(void) {
	return;
}