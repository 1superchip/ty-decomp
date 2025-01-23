#ifndef COMMON_MKGRASS
#define COMMON_MKGRASS

#include "common/Model.h"

#define MAX_GRASS_ENTRIES (22)

struct GrassInfo {
    int clumpSize;
    int density;
    float minHeight;
    float maxHeight;
    float width;
    float movement;
    float animSpeed;
    float maxPushAway;
    float pushAwayRadius;
    int numTextures;
    float upVector;
    float textureAnimSpeed;
    float unk30;
    int unk34;
    float maxVisibleRadius;
    Material* pMat;
    char materialName[32];
};

void Grass_DrawGC(Model* pModel, u8* pStripData, int stripMaxOffset, int grassIndex, float f1, float f2);

void MKGrass_InitTypes(char* pName);
void MKGrass_DeinitTypes(void);

void MKGrass_Deinit(void);

void MKGrass_Update(void);
void MKGrass_Draw(void);

void MKGrass_Init(void);

void MKGrassGC_LoadTextures(char** ppTextureNames);
void MKGrassGC_UnloadTextures(void);

void MKGrass_AddModel(Model* pModel);

void MKGrass_SetPushAwayPos(Vector*, int pushAwayIndex);

extern Vector* pGrassPushAway;
extern float GrassGCMaxRadius;

#endif // COMMON_MKGRASS
