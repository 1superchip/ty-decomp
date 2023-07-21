#include "types.h"
#include "common/Texture.h"
#include "common/Heap.h"
#include "common/PtrListDL.h"
#include "common/Vector.h"
#include "common/Str.h"
#include "common/FileSys.h"

// // not from this file
// struct _GXRenderModeObj {
//     u32 viTVMode;
//     u16 fbWidth;
//     u16 efbHeight;
//     u16 xfbHeight;
//     u16 viXOrigin;
//     u16 viYOrigin;
//     u16 viWidth;
//     u16 viHeight;
//     uint xfbMode;
//     u8 field_rendering;
//     u8 aa;
//     u8 sample_pattern[2][12];
//     u8 vfilter[7];
// };
extern "C" {
	u32 VIGetTvFormat(void);
	_GXRenderModeObj* DEMOGetRenderModeObj(void);
	double fmod(double, double);
	void memcpy(void*, void*, int);
	void strcpy(char*, char*);
	void DCStoreRange(uint*, int);
	void memset(void*, int, int);
	int stricmp(char*, char*);
};


Vector Texture_Color;

bool Texture::initialised;
static PtrListDL<Texture> textures;
bool Texture_bColourKey;
bool Texture_IsAlias;

int Texture_filterType = 2;

void Texture::InitModule(void) {
    textures.Init(0x200, sizeof(Texture)); // max of 512 textures
    Texture_Color.x = Texture_Color.y = Texture_Color.z = 0.0f;
    initialised = true;
}

void Texture::DeinitModule(void) {
    textures.Deinit();
    initialised = false;
}

Texture *Texture::Create(char *pName) {
    int size;
    Texture *pTex = Texture::Find(pName);
    if (pTex != NULL) {
        pTex->referenceCount++;
    } else {
        pTex = textures.GetNextEntry();
        strcpy(pTex->name, Str_CopyString(pName, 0x1f));
        pTex->bMpegTarget = false;
        TexFile* texFile = (TexFile*)FileSys_Load(Str_Printf("%s.gtx", pName), &size, 0, -1);
        DCStoreRange((uint *)texFile, size);
        pTex->pFileData = texFile;
        pTex->width = texFile->width;
        pTex->height = texFile->height;
        switch (texFile->fmt) {
        case 0:
			// RGB5A3
            pTex->bTlut = false;
            memset((void *)&pTex->texObj, 0, sizeof(GXTexObj));
            GXInitTexObj(&pTex->texObj, (int *)texFile + 8, pTex->width, pTex->height, GX_TF_RGB5A3, GX_REPEAT, GX_REPEAT, 0);
            break;
        case 1:
			// Indexed
            pTex->bTlut = true;
            memset((void *)&pTex->tlutObj, 0, sizeof(GXTlutObj));
            if (Texture_bColourKey != false) {
                if (!(texFile->unk20 & 0x8000)) {
                    texFile->unk20 &= 0x0FFF;
                } else {
                    texFile->unk20 = (texFile->unk20 >> 1) & 0xFFF;
                }
            }
            GXInitTlutObj(&pTex->tlutObj, (void *)((int)texFile + 0x20), GX_TL_RGB5A3, 0x100); // file + 0x20 is lookup table, format 2, 256 entries
            memset((void *)&pTex->texObj, 0, sizeof(GXTexObj));
            GXInitTexObjCI(&pTex->texObj, (void *)((int)texFile + 0x220), pTex->width, pTex->height, GX_TF_C8, GX_REPEAT, GX_REPEAT, 0, 0);
            break;
        case 2:
			// CMPR
            pTex->bTlut = false;
            memset((void *)&pTex->texObj, 0, sizeof(GXTexObj));
            GXInitTexObj(&pTex->texObj, (int *)texFile + 8, pTex->width, pTex->height, GX_TF_CMPR, GX_REPEAT, GX_REPEAT, 1);
            int bits = pTex->width < pTex->height ? pTex->width : pTex->height;
            int i;
            for (i = 31; i >= 0; i--) {
                if (bits & (1 << i))
                        break;
            }
            float maxLod = (float)i - 3.0f;
            int minFilter;
            switch (Texture_filterType) {
            case 0:
            case 1:
                minFilter = 1; // GX_Linear
                break;
            case 2:
                minFilter = 5; // GX_LIN_MIP_LIN
                break;
            default:
                minFilter = 5; // GX_LIN_MIP_LIN
            }
            // GXInitTexObjLOD(&pTex->texObj, 0.0f, maxLod, -4.0f, minFilter, 1, 1, 0, 0);
            GXInitTexObjLOD(&pTex->texObj, (GXTexFilter)minFilter, GX_LINEAR, 0.0f, maxLod, -4.0f, 1, 0, (GXAnisotropy)0);
            if (Texture_filterType >= 2 && pTex->width > 64) {
                break;
            }
            int memSize = ((pTex->width * pTex->height) / 2) + 0x20;
            void *mem = Heap_MemAlloc(memSize); // allocate memory for header + image
            memcpy(mem, texFile, memSize);
            DCStoreRange((uint *)mem, memSize);
            pTex->pFileData = mem;
            GXInitTexObj(&pTex->texObj, (void *)((int)mem + 0x20), pTex->width, pTex->height, GX_TF_CMPR, GX_REPEAT, GX_REPEAT, 0);
            Heap_MemFree(texFile);
        }
        pTex->referenceCount = 1;
    }
    return pTex;
}

void Texture::Destroy(void) {
    if (--referenceCount != 0) {
        return;
    }
    if (pFileData != NULL) {
        Heap_MemFree(pFileData);
        pFileData = NULL;
    }
    textures.Destroy(this);
}

Texture* Texture::Find(char* pName) {
    Texture** list;
    char* str = Str_CopyString(pName, 0x1f);
    list = (Texture**)textures.pMem;
    while (*list != NULL) {
        if (stricmp((*list)->name, str) == 0) {
            return *list;
        }
        list++;
    }
    return NULL;
}

void Texture::Use(void) {
    if (bTlut != false) {
        GXLoadTlut(&tlutObj, GX_TLUT0);
    }
    GXLoadTexObj(&texObj, GX_TEXMAP0);
}

Texture* Texture::CreateRenderTarget(char* pName, int arg2, int arg3, int arg4) {
	return Texture::Create(pName);
}

Texture* Texture::CreateFromRawData(char* pName, void* pRawData, int format, int width, int height) {
    Texture* pTex = textures.GetNextEntry();
    strcpy(pTex->name, Str_CopyString(pName, 0x1f));
    pTex->bMpegTarget = false;
    pTex->pFileData = pRawData;
    pTex->width = width;
    pTex->height = height;
    switch(format) {
        case 0:
            pTex->bTlut = false;
            memset((void*)&pTex->texObj, 0, sizeof(GXTexObj));
            GXInitTexObj(&pTex->texObj, pRawData, pTex->width, pTex->height, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, 0);
            break;
        case 2:
            pTex->bTlut = false;
            memset((void*)&pTex->texObj, 0, sizeof(GXTexObj));
            GXInitTexObj(&pTex->texObj, pRawData, pTex->width, pTex->height, GX_TF_RGB565, GX_REPEAT, GX_REPEAT, 0);
            break;
    }
    pTex->referenceCount = 1;
    return pTex;
}

Texture* Texture::CreateMpegTarget(char* pName, void* arg2, int width, int height) {
    Texture* pTex;
    pTex = Texture::Find(pName);
    if (pTex != NULL) {
        pTex->referenceCount++;
    } else {
        pTex = textures.GetNextEntry();
        pTex->pFileData = NULL;
        strcpy(pTex->name, Str_CopyString(pName, 0x1f));
        pTex->width = width;
        pTex->height = height;
        pTex->referenceCount = 1;
        pTex->unk70 = 0;
        pTex->bMpegTarget = true;
    }
    return pTex;
}