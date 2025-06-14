#include "common/MKGrass.h"
#include "common/System_GC.h"
#include "common/KromeIni.h"

#define NUM_RANDOM_TIME_ENTRIES (32)

static float randomTimes[NUM_RANDOM_TIME_ENTRIES][3];

static GrassInfo GrassInfoGC[MAX_GRASS_ENTRIES];

static Vector* pGrassCameraPos;

static float GrassHeight;
static float GrassX;
static float GrassZ;
static float SwayX;
static float SwayZ;

static float GrassUOffset;

static bool bGrassEnabled;

Vector* pGrassPushAway;

static u16 GrassColor;

static float pushAwayRadius;
static float maxPushAway;

static float movement;

static bool bEnableAnimation = true;

float GCSinTable[256] = {
    0.0f, 0.02454099990427494f, 0.04906800016760826f, 0.07356499880552292f, 
    0.09801699966192245f, 0.12241099774837494f, 0.14673000574111938f, 0.17096200585365295f, 
    0.19508999586105347f, 0.21910099685192108f, 0.2429800033569336f, 0.2667129933834076f, 
    0.29028499126434326f, 0.3136819899082184f, 0.33689001202583313f, 0.3598949909210205f, 
    0.38268300890922546f, 0.4052410125732422f, 0.42755499482154846f, 0.44961100816726685f, 
    0.4713970124721527f, 0.49289798736572266f, 0.5141029953956604f, 0.5349979996681213f, 
    0.5555700063705444f, 0.5758079886436462f, 0.5956990122795105f, 0.615231990814209f, 
    0.6343929767608643f, 0.65317302942276f, 0.6715589761734009f, 0.6895409822463989f, 
    0.7071070075035095f, 0.7242469787597656f, 0.7409510016441345f, 0.7572090029716492f, 
    0.7730100154876709f, 0.788345992565155f, 0.8032079935073853f, 0.8175849914550781f, 
    0.8314700126647949f, 0.8448539972305298f, 0.8577290177345276f, 0.8700870275497437f, 
    0.8819209933280945f, 0.8932240009307861f, 0.9039890170097351f, 0.9142100214958191f, 
    0.9238799810409546f, 0.9329929947853088f, 0.9415439963340759f, 0.9495279788970947f, 
    0.9569399952888489f, 0.9637759923934937f, 0.9700310230255127f, 0.9757019877433777f, 
    0.9807850122451782f, 0.9852780103683472f, 0.9891769886016846f, 0.9924799799919128f, 
    0.9951850175857544f, 0.9972900152206421f, 0.9987949728965759f, 0.9996989965438843f, 
    1.0f, 0.9996989965438843f, 0.9987949728965759f, 0.9972900152206421f, 
    0.9951850175857544f, 0.9924799799919128f, 0.9891759753227234f, 0.9852780103683472f, 
    0.9807850122451782f, 0.9757019877433777f, 0.9700310230255127f, 0.9637759923934937f, 
    0.9569399952888489f, 0.9495279788970947f, 0.9415439963340759f, 0.9329929947853088f, 
    0.9238799810409546f, 0.9142100214958191f, 0.9039890170097351f, 0.8932240009307861f, 
    0.8819209933280945f, 0.8700870275497437f, 0.8577290177345276f, 0.8448539972305298f, 
    0.8314700126647949f, 0.8175849914550781f, 0.8032069802284241f, 0.788345992565155f, 
    0.7730100154876709f, 0.7572090029716492f, 0.7409510016441345f, 0.7242469787597656f, 
    0.7071070075035095f, 0.6895400285720825f, 0.6715589761734009f, 0.65317302942276f, 
    0.6343929767608643f, 0.6152309775352478f, 0.5956990122795105f, 0.5758079886436462f, 
    0.5555700063705444f, 0.5349969863891602f, 0.5141029953956604f, 0.49289798736572266f, 
    0.4713970124721527f, 0.44961100816726685f, 0.42755499482154846f, 0.4052410125732422f, 
    0.38268300890922546f, 0.3598949909210205f, 0.33689001202583313f, 0.3136819899082184f, 
    0.29028499126434326f, 0.2667129933834076f, 0.2429800033569336f, 0.21910099685192108f, 
    0.19508999586105347f, 0.17096200585365295f, 0.14673000574111938f, 0.12241099774837494f, 
    0.09801699966192245f, 0.07356400042772293f, 0.04906700178980827f, 0.02454099990427494f, 
    -0.0f, -0.02454099990427494f, -0.04906800016760826f, -0.07356499880552292f, 
    -0.09801699966192245f, -0.12241099774837494f, -0.14673100411891937f, -0.17096200585365295f, 
    -0.19508999586105347f, -0.21910099685192108f, -0.2429800033569336f, -0.2667129933834076f, 
    -0.29028499126434326f, -0.3136819899082184f, -0.33689001202583313f, -0.3598949909210205f, 
    -0.38268300890922546f, -0.4052410125732422f, -0.42755499482154846f, -0.44961199164390564f, 
    -0.4713970124721527f, -0.49289798736572266f, -0.5141029953956604f, -0.5349979996681213f, 
    -0.5555700063705444f, -0.5758079886436462f, -0.5956990122795105f, -0.615231990814209f, 
    -0.6343929767608643f, -0.65317302942276f, -0.6715589761734009f, -0.6895409822463989f, 
    -0.7071070075035095f, -0.7242469787597656f, -0.7409510016441345f, -0.7572090029716492f, 
    -0.7730100154876709f, -0.788345992565155f, -0.8032079935073853f, -0.8175849914550781f, 
    -0.8314700126647949f, -0.8448539972305298f, -0.8577290177345276f, -0.8700870275497437f, 
    -0.8819209933280945f, -0.8932240009307861f, -0.9039890170097351f, -0.9142100214958191f, 
    -0.9238799810409546f, -0.9329929947853088f, -0.9415439963340759f, -0.9495279788970947f, 
    -0.9569399952888489f, -0.9637759923934937f, -0.9700310230255127f, -0.9757019877433777f, 
    -0.9807850122451782f, -0.9852780103683472f, -0.9891769886016846f, -0.9924799799919128f, 
    -0.9951850175857544f, -0.9972900152206421f, -0.9987949728965759f, -0.9996989965438843f, 
    -1.0f, -0.9996989965438843f, -0.9987949728965759f, -0.9972900152206421f, 
    -0.9951850175857544f, -0.9924790263175964f, -0.9891769886016846f, -0.9852780103683472f, 
    -0.9807850122451782f, -0.9757019877433777f, -0.9700310230255127f, -0.9637759923934937f, 
    -0.9569399952888489f, -0.9495279788970947f, -0.9415439963340759f, -0.9329929947853088f, 
    -0.9238790273666382f, -0.9142100214958191f, -0.9039890170097351f, -0.8932240009307861f, 
    -0.8819209933280945f, -0.8700870275497437f, -0.8577290177345276f, -0.8448529839515686f, 
    -0.8314689993858337f, -0.8175849914550781f, -0.8032079935073853f, -0.788345992565155f, 
    -0.7730100154876709f, -0.7572090029716492f, -0.7409510016441345f, -0.7242469787597656f, 
    -0.7071070075035095f, -0.6895409822463989f, -0.6715589761734009f, -0.65317302942276f, 
    -0.6343929767608643f, -0.6152309775352478f, -0.5956990122795105f, -0.5758079886436462f, 
    -0.5555700063705444f, -0.5349979996681213f, -0.5141029953956604f, -0.49289798736572266f, 
    -0.4713970124721527f, -0.44961100816726685f, -0.42755499482154846f, -0.4052410125732422f, 
    -0.38268300890922546f, -0.3598949909210205f, -0.33689001202583313f, -0.3136819899082184f, 
    -0.29028400778770447f, -0.2667120099067688f, -0.2429800033569336f, -0.21910099685192108f, 
    -0.19508999586105347f, -0.17096200585365295f, -0.14673000574111938f, -0.12240999937057495f, 
    -0.09801699966192245f, -0.07356499880552292f, -0.04906800016760826f, -0.02454099990427494f
};

float GrassGCMaxRadius = 12000.0f;

extern "C" int stricmp(char*, char*);
extern "C" void memset(void*, int, size_t);
extern "C" int strcmpi(char*, char*);
extern "C" void strcpy(char*, const char*);

extern int gFrameCounter;

#define RGB2COLOR(r, g, b) (((((r & 0xf8)>>3)<<11) | (((g & 0xfc)>>2)<<5) | ((b & 0xfc)>>3)))

static inline float GetSin(int z, int x) {
    z = x + z;
    int y = gFrameCounter * 2;
    y += z;
    return GCSinTable[y & 255];
}

inline void DrawGrass(bool bAnimate, float posX, float posY, float posZ, float numTexturesScaled) {

    float sx = 0.0f;
    float sz = 0.0f;

    if (bAnimate) {
        float sin = GetSin(*(int*)&posZ, *(int*)&posX);
        sin *= movement;
        sx = sin * SwayX;
        sz = sin * SwayZ;
    }

    if (pGrassPushAway) {
        float pushDist;
        float dx;
        float dz;
        dx = posX - pGrassPushAway->x;
        dz = posZ - pGrassPushAway->z;
        pushDist = dx * dx + dz * dz;
        if (pushDist < pushAwayRadius) {
            float scale = maxPushAway * (1.0f - (pushDist / pushAwayRadius));
            dx *= scale;
            dz *= scale;
            sx += dx;
            sz += dz;
        }
    }

    GXWGFifo.f32 = (posX - GrassX) + sx;
    GXWGFifo.f32 = posY + GrassHeight;
    GXWGFifo.f32 = (posZ - GrassZ) + sz;
    GXColor1u16(GrassColor);
    GXTexCoord2f32(GrassUOffset, 0.0f);

    GXWGFifo.f32 = (posX + GrassX) + sx;
    GXWGFifo.f32 = posY + GrassHeight;
    GXWGFifo.f32 = (posZ + GrassZ) + sz;
    GXColor1u16(GrassColor);
    GXTexCoord2f32(GrassUOffset + 0.125f, 0.0f);

    GXWGFifo.f32 = (posX + GrassX);
    GXWGFifo.f32 = posY;
    GXWGFifo.f32 = (posZ + GrassZ);
    GXColor1u16(GrassColor);
    GXWGFifo.f32 = GrassUOffset + 0.125f;
    GXWGFifo.f32 = 1.0f;

    GXWGFifo.f32 = (posX - GrassX);
    GXWGFifo.f32 = posY;
    GXWGFifo.f32 = (posZ - GrassZ);
    GXColor1u16(GrassColor);
    GXTexCoord2f32(GrassUOffset, 1.0f);

    GrassUOffset += 0.125f;

    if (GrassUOffset >= numTexturesScaled) {
        GrassUOffset = 0.0f;
    }
}


void Grass_DrawGC(Model* pModel, u8* pStripData, int stripMaxOffset, int grassIndex, float f1, float f2) {
    if (!bGrassEnabled) {
        return;
    }

    GrassInfo* pGrass = &GrassInfoGC[grassIndex];

    if (f1 >= Min<float>(pGrass->maxVisibleRadius, GrassGCMaxRadius) || f2 < 0.0f) {
        return;
    }

    pGrass->pMat->Use();

    float vx = View::GetCurrent()->unk48.Row0()->x;
    GrassX = vx * pGrass->width * 100.0f;
    
    float vz = View::GetCurrent()->unk48.Row0()->z;
    GrassZ = vz * pGrass->width * 100.0f;

    SwayX = View::GetCurrent()->unk48.Row0()->x;
    SwayZ = View::GetCurrent()->unk48.Row0()->z;

    pGrassCameraPos = View::GetCurrent()->unk48.Row3();

    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    float f31 = pGrass->minHeight * 100.0f;
    float f30 = ((pGrass->maxHeight - pGrass->minHeight) * 100.0f) * (1.0f / 7.0f);

    pushAwayRadius = Sqr<float>(pGrass->pushAwayRadius);
    maxPushAway = pGrass->maxPushAway;
    movement = pGrass->movement;

    DisplayList* pStrip = (DisplayList*)pStripData;

    while (pStrip->primitive == GX_TRIANGLESTRIP && (u8*)pStrip < pStripData + stripMaxOffset) {
        u16 numVertices = pStrip->vertexCount;

        // Increment pStrip to start of index data
        (char*)pStrip += sizeof(pStrip->primitive) + sizeof(pStrip->vertexCount);

        for (int i = 0; i < numVertices - 2; i++) {
            Vertex* pModelVertices = pModel->pTemplate->pModelData->pVertices;
            
            u16 i0 = *(u16*)pStrip;
            u16 i1 = *(u16*)((u8*)pStrip + 8);
            u16 i2 = *(u16*)((u8*)pStrip + 16);

            (u8*)pStrip += 8; // go to next set of indices, now u16 at 0x0 is a vertexIndex
            
            Vector* pVertex = (Vector*)&pModelVertices[i0].pos; // vertexIndex
            Vector* pNextVertexPos = (Vector*)&pModelVertices[i1].pos; // next vertexIndex
            Vector* pNextVertexPos2 = (Vector*)&pModelVertices[i2].pos; // next vertexIndex


            float dist = (pVertex->x - pGrassCameraPos->x) * (pVertex->x - pGrassCameraPos->x) + 
                (pVertex->y - pGrassCameraPos->y) * (pVertex->y - pGrassCameraPos->y) +
                (pVertex->z - pGrassCameraPos->z) * (pVertex->z - pGrassCameraPos->z);

            float maxRadius = Min<float>(Sqr<float>(GrassGCMaxRadius), Sqr<float>(pGrass->maxVisibleRadius));

            if (dist >= maxRadius) {
                continue;
            }
            
            // bend the grass up and down based on the distance from the camera
            float grassHeightScale = pGrass->upVector * (1.0f - (dist / maxRadius));

            u8* r23 = pModelVertices[*(u16*)((u8*)pStrip - 4)].color;
            u8* r22 = pModelVertices[*(u16*)((u8*)pStrip + 4)].color;
            u8* r21 = pModelVertices[*(u16*)((u8*)pStrip + 0xC)].color;

            int count = pGrass->clumpSize;
            
            GrassUOffset =
                ((pGrass->numTextures - 1) & pGrass->unk34 + (*(int*)&pVertex->x ^ *(int*)&pVertex->z))
                * 0.125f;

            GXBegin(GX_QUADS, GX_VTXFMT2, count * 4);

            float* times = randomTimes[0];
            float numTexturesScaled = (pGrass->numTextures - 1) * 0.125f;

            for (int j = 0; j < count; j++) {
                GrassHeight = grassHeightScale * (f31 + ((float)(~j & (8 - 1)) * f30));
                
                float x = *times++;
                float y = *times++;
                float z = *times++;
                
                u16 r = r23[0] * x + r22[0] * y + r21[0] * z;
                u16 g = r23[1] * x + r22[1] * y + r21[1] * z;
                u16 b = r23[2] * x + r22[2] * y + r21[2] * z;

                GrassColor = RGB2COLOR(r, g, b);

                bool bAnimate = pGrass->animSpeed > 0.0f && bEnableAnimation;

                DrawGrass(
                    bAnimate,
                    x * pVertex->x + y * pNextVertexPos->x + z * pNextVertexPos2->x,
                    x * pVertex->y + y * pNextVertexPos->y + z * pNextVertexPos2->y,
                    x * pVertex->z + y * pNextVertexPos->z + z * pNextVertexPos2->z,
                    numTexturesScaled
                );
                
            }
        }
        (char*)pStrip += 0x10;
    }
}

void MKGrass_InitTypes(char* pName) {
    // should MKGrass_LoadIni(char*) be inlined in this version?
    KromeIni ini;
    GrassInfo* pGrass;
    KromeIniLine* pLine;

    ini.Init(pName);

    pLine = ini.GotoLine(NULL, NULL);

    memset(GrassInfoGC, 0, sizeof(GrassInfoGC));

    int i = -1;

    while (pLine != NULL) {
        if (pLine->section != NULL) {
            i++;
        } else if (pLine->pFieldName != NULL) {
            if (strcmpi(pLine->pFieldName, "material") == 0) {
                strcpy(GrassInfoGC[i].materialName, pLine->data);
                GrassInfoGC[i].pMat = NULL;
            } else if (strcmpi(pLine->pFieldName, "clumpSize") == 0) {
                pLine->AsInt(0, &GrassInfoGC[i].clumpSize);
            } else if (strcmpi(pLine->pFieldName, "density") == 0) {
                pLine->AsInt(0, &GrassInfoGC[i].density);
            } else if (strcmpi(pLine->pFieldName, "minHeight") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].minHeight);
            } else if (strcmpi(pLine->pFieldName, "maxHeight") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].maxHeight);
            } else if (strcmpi(pLine->pFieldName, "width") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].width);
            } else if (strcmpi(pLine->pFieldName, "movement") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].movement);
            } else if (strcmpi(pLine->pFieldName, "animSpeed") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].animSpeed);
            } else if (strcmpi(pLine->pFieldName, "maxPushAway") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].maxPushAway);
            } else if (strcmpi(pLine->pFieldName, "pushAwayRadius") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].pushAwayRadius);
            } else if (strcmpi(pLine->pFieldName, "numTextures") == 0) {
                pLine->AsInt(0, &GrassInfoGC[i].numTextures);
            } else if (strcmpi(pLine->pFieldName, "upVector") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].upVector);
            } else if (strcmpi(pLine->pFieldName, "textureAnimSpeed") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].textureAnimSpeed);
            } else if (strcmpi(pLine->pFieldName, "maxVisibleRadius") == 0) {
                pLine->AsFloat(0, &GrassInfoGC[i].maxVisibleRadius);
            }
        }

        pLine = ini.GetNextLine();
    }

    ini.Deinit();

    for (int i = 0; i < ARRAY_SIZE(GrassInfoGC); i++) {
        GrassInfoGC[i].clumpSize *= GrassInfoGC[i].density;
        GrassInfoGC[i].pushAwayRadius *= 100.0f;
        GrassInfoGC[i].maxVisibleRadius *= 100.0f;
        GrassInfoGC[i].movement *= 100.0f;

        if (GrassInfoGC[i].clumpSize > 32) {
            GrassInfoGC[i].clumpSize = 32;
        }
    }

    bGrassEnabled = true;

    int seed = 0;

    for (int i = 0; i < NUM_RANDOM_TIME_ENTRIES; i++) {
        float rand0 = RandomFR(&seed, 0.0f, 1.0f);
        float rand1 = RandomFR(&seed, 0.0f, 1.0f);
        float rand2 = RandomFR(&seed, 0.0f, 1.0f);

        float mult = 1.0f / (rand0 + rand1 + rand2);

        rand0 *= mult;
        rand1 *= mult;
        rand2 *= mult;

        randomTimes[i][0] = rand0;
        randomTimes[i][1] = rand1;
        randomTimes[i][2] = rand2;
    }
}

void MKGrass_DeinitTypes(void) {
    return;
}

void MKGrass_Deinit(void) {
    return;
}

void MKGrass_Update(void) {
    for (int i = 0; i < ARRAY_SIZE(GrassInfoGC); i++) {
        GrassInfoGC[i].unk30 += gDisplay.dt * GrassInfoGC[i].textureAnimSpeed;
        GrassInfoGC[i].unk34 = GrassInfoGC[i].unk30;
    }
}

void MKGrass_Draw(void) {
    return;
}

void MKGrass_Init(void) {
    return;
}

void MKGrassGC_LoadTextures(char** ppTextureNames) {
    char** textureNames = ppTextureNames;
    while (*textureNames) {
        for (int i = 0; i < ARRAY_SIZE(GrassInfoGC); i++) {
            if (stricmp(*textureNames, GrassInfoGC[i].materialName) == 0) {
                GrassInfoGC[i].pMat = Material::Create(*textureNames);
            }
        }

        textureNames++;
    }
}

void MKGrassGC_UnloadTextures(void) {
    for (int i = 0; i < ARRAY_SIZE(GrassInfoGC); i++) {
        if (GrassInfoGC[i].pMat) {
            GrassInfoGC[i].pMat->Destroy();
            GrassInfoGC[i].pMat = NULL;
        }
    }
}

void MKGrass_AddModel(Model* pModel) {
    return;
}

void MKGrass_SetPushAwayPos(Vector* pPos, int pushAwayIndex) {
    return;
}
