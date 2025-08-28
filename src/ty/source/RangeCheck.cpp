#include "types.h"
#include "ty/RangeCheck.h"
#include "ty/global.h"
#include "ty/tools.h"
#include "common/Str.h"
#include "common/Heap.h"
#include "ty/Ty.h"

Vector* GameCamera_GetPos(void);
Vector* GameCamera_GetDir(void);
void GameCamera_GetVectors(Vector* pCamPos, Vector* pTargetPoint, Vector* pCamDir);
extern "C" void memset(void*, int, int);
extern "C" int stricmp(char*, char*);
extern "C" void strncpy(char*, char*, int);
int strnicmp(char const*, char const*, int);
void Draw_AddPostDrawModel(Model*, float, bool);

static int nextAvailableLODEntryIndex;
static LODEntry* lodEntryPool = NULL;

static Vector cameraPos;
static Vector cameraVector; // camera direction

static int heroState = TY_AS_35;
static int maxLODEntries = 0x400;

void LOD_Deinit(void) {
    if (lodEntryPool != NULL) {
        Heap_MemFree(lodEntryPool);
    }
    lodEntryPool = NULL;
}

int Range_WhichZone(Vector* point, float* arg1) {
    // no vector here
    Vector diff;
    diff.x = point->x - cameraPos.x;
    diff.y = point->y - cameraPos.y;
    diff.z = point->z - cameraPos.z;
    float dist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
    float fVar1 = 0.0f;
    if (arg1 != NULL) {
        *arg1 = 1.0f;
    }

    int zoneId = 0;
    for (; zoneId < ARRAY_SIZE(gb.level.lodRanges); zoneId++) {
        float zone = gb.level.lodRanges[zoneId];
        if (dist < zone) {
            if (arg1 == NULL) {
                return zoneId;
            }
            
            *arg1 = (dist - fVar1) / (zone - fVar1);
            return zoneId;
        }
        
        fVar1 = zone;
    }

    return zoneId;
}

bool Range_IsVisible(Vector* point) {
    Vector camToObj;
    camToObj.Sub(point, &cameraPos);
    camToObj.Normalise();
    return camToObj.Dot(&cameraVector) >= 0.5f;
}

void Range_ModelSetAlpha(Model* pModel, int arg1, float arg2, float arg3, float arg4, float arg5, int arg6) {
    if (arg1 == arg6 - 1) {
        pModel->colour.w = Min<float>(2.0f * (1.0f - arg2), 1.0f);
    } else if (arg1 == 0) {
        Vector center = *pModel->matrices[0].Row3();
        Vector target;
        Vector source;
        center.y += arg3 / 2.0f;

        GameCamera_GetVectors(&source, &target, NULL);

        if (RayToSphere(&source, &target, &center, 10.0f + arg4, -1.0f, true) && ty.mFsm.GetStateEx() != TY_AS_45) {
            pModel->colour.w = Max<float>(arg5, pModel->colour.w - 0.05625f);
        } else {
            pModel->colour.w = Min<float>(1.0f, pModel->colour.w + 0.05625f);
        }
    } else {
        pModel->colour.w = Min<float>(1.0f, pModel->colour.w + 0.05625f);
    }
}

void Range_Update(void) {
    if (pHero->IsTy()) {
        heroState = ty.mFsm.GetStateEx();
    } else {
        heroState = 0;
    }

    cameraPos = *GameCamera_GetPos();
    cameraVector = *GameCamera_GetDir();
}

LODEntry* LODEntry_GetNextEntryFromPool(void) {
    if (lodEntryPool == NULL) {
        lodEntryPool = (LODEntry *)Heap_MemAlloc(maxLODEntries * sizeof(LODEntry));
        nextAvailableLODEntryIndex = 0;
    }
    memset((void *)&lodEntryPool[nextAvailableLODEntryIndex], 0, sizeof(LODEntry));
    return &lodEntryPool[nextAvailableLODEntryIndex++];
}

void LODEntry::Init(Model* pModel) {
    subObjectIndex = pModel->GetSubObjectIndex(name);
}

bool LODEntry::CheckFlags(int flags) {
    return flags & subObjectFlags;
}

void LODDescriptor::ResolveSubObjects(Model* pModel) {
    for (int i = 0; i < nmbrOfEntries; i++) {
        pEntries[i].Init(pModel);
    }
}

void LODDescriptor::Init(KromeIni* pIni, char* arg1) {
    char section[0x20];
    memset((void*)this, 0, sizeof(LODDescriptor));
    invisibleZone = -1;
    maxScissorDist = 1000.0f;
    flags |= 1;
    if (pIni != NULL && arg1 != NULL) {
        Tools_StripExtension(section, (const char*)arg1);
        KromeIniLine* pLine = pIni->GotoLine(section, NULL);
        ParseIni(pIni, pLine);
    }
}

void LODDescriptor::ReplicateLODData(int arg0, int arg1) {
    int shift = 1 << arg0;
    int bits = 0;
    while (arg0 < arg1) {
        bits |= (1 << arg0);
        arg0++;
    }

    for (int i = 0; i < nmbrOfEntries; i++) {
        pEntries[i].subObjectFlags |= (pEntries[i].CheckFlags(shift)) ? bits : 0;
    }

    shadowFlags |= (shadowFlags & shift) ? bits : 0;
    particleFlags |= (particleFlags & shift) ? bits : 0;
    soundFlags |= (soundFlags & shift) ? bits : 0;
    scissorFlags |= (scissorFlags & shift) ? bits : 0;
}

void Warn(KromeIni* pIni, char* string) {
    pIni->Warning(string);
}

void LODDescriptor::ParseIni(KromeIni* pIni, KromeIniLine* pLine) {
    int warning = -2;
    while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
        if (pLine->pFieldName != NULL) {
            char* pString = NULL;
            if (stricmp(pLine->pFieldName, "frag") == 0) {
                for (int i = 0; i < pLine->elementCount; i++) {
                    pLine->AsString(i, &pString);
                    GetEntryFromString(pString)->unk28 = 1;
                }
            } else if (stricmp(pLine->pFieldName, "lod") == 0) {
                int newLodIndex = 0;
                if (pLine->AsInt(0, &newLodIndex) != false) {
                    if ((newLodIndex < 0) || (newLodIndex >= 8)) {
                        Warn(pIni, Str_Printf("LODDescriptor::ParseIni: LOD index out of range (%d)", warning));
                        if (newLodIndex < 0) {
                            newLodIndex = 0;
                        } else if (newLodIndex >= 8) {
                            newLodIndex = 7;
                        }
                    }
                    ReplicateLODData(Max<int>(0, warning), newLodIndex);
                    warning = newLodIndex;
                }
                if (warning + 1 > invisibleZone) {
                    invisibleZone = warning + 1;
                }
            } else if (stricmp(pLine->pFieldName, "subobj") == 0) {
                if (warning < -1) {
                    pIni->Warning("LODDescriptor::ParseIni: No LOD range specified for subobj line");
                } else {
                    for (int i = 0; i < pLine->elementCount; i++) {
                        pLine->AsString(i, &pString);
                        GetEntryFromString(pString)->subObjectFlags |= 1 << warning;
                    }
                }
            } else if (stricmp(pLine->pFieldName, "effect") == 0) {
                if (warning < -1) {
                    pIni->Warning("LODDescriptor::ParseIni: No LOD range specified for effect line");
                } else {
                    pLine->AsString(0, &pString);
                    if (stricmp("particles", pString) == 0) {
                        particleFlags |= 1 << warning;
                    } else if (stricmp("shadow", pString) == 0) {
                        shadowFlags |= 1 << warning;
                    } else if (stricmp("sound", pString) == 0) {
                        soundFlags |= 1 << warning;
                    } else if (stricmp("scissor", pString) == 0) {
                        scissorFlags |= 1 << warning;
                    }
                }
            } else if (stricmp(pLine->pFieldName, "flags") == 0) {
                flags = 0;
                for (int i = 0; i < pLine->elementCount; i++) {
                    pLine->AsString(i, &pString);
                    if (stricmp("alpha", pString) == 0) {
                        flags |= LODFlags_Alpha;
                    } else if (stricmp("camerafade", pString) == 0) {
                        flags |= LODFlags_CameraFade;
                    } else if (stricmp("scissor", pString) == 0) {
                        flags |= LODFlags_Scissor;
                    } else if (stricmp("alphaProp", pString) == 0) {
                        flags |= LODFlags_AlphaProp;
                    }
                }
            } else if (stricmp(pLine->pFieldName, "radius") == 0) {
                pLine->AsFloat(0, &radius);
            } else if (stricmp(pLine->pFieldName, "maxScissorDist") == 0) {
                pLine->AsFloat(0, &maxScissorDist);
            } else if (stricmp(pLine->pFieldName, "height") == 0) {
                pLine->AsFloat(0, &height);
            } else if (stricmp(pLine->pFieldName, "minalpha") == 0) {
                pLine->AsFloat(0, &minalpha);
            } else if (stricmp(pLine->pFieldName, "invisibleZone") == 0) {
                pLine->AsInt(0, &invisibleZone);
            }
        }

        pLine = pIni->GetNextLine();
    }

    ReplicateLODData(Max<int>(0, warning), 8);
    invisibleZone = 8;
}

LODEntry* LODDescriptor::GetEntryFromString(char* name) {
    LODEntry* pFoundEntry = NULL;

    if (pEntries == NULL) {
        pEntries = LODEntry_GetNextEntryFromPool();
        nmbrOfEntries = 1;
        pFoundEntry = pEntries;
        strncpy(pFoundEntry->name, name, sizeof(pFoundEntry->name));
    } else {
        for (int i = 0; i < nmbrOfEntries; i++) {
            if (stricmp(pEntries[i].name, name) == 0) {
                pFoundEntry = &pEntries[i];
                break;
            }
        }
        
        if (pFoundEntry == NULL) {
            pFoundEntry = LODEntry_GetNextEntryFromPool();
            strncpy(pFoundEntry->name, name, sizeof(pFoundEntry->name));
            nmbrOfEntries++;
        }
    }

    return pFoundEntry;
}

void LODManager::Init(Model* pModel, int arg1, LODDescriptor* d) {
    int i;

    pDescriptor = d;
    subobjectEnableFlags = arg1;
    pDescriptor->ResolveSubObjects(pModel);
    
    for (i = 0; i < pModel->GetNmbrOfSubObjects(); i++) {
        if (strnicmp((const char*)pModel->GetSubObjectName(i), "f_", 2) == 0) {
            pModel->EnableSubObject(i, 0);
        }
    }

    for (i = 0; i < pDescriptor->nmbrOfEntries; i++) {
        pModel->EnableSubObject(
            pDescriptor->pEntries[i].subObjectIndex, 
            pDescriptor->pEntries[i].CheckFlags(1 << subobjectEnableFlags)
        );
    }
}

void LODManager::InternalUpdate(Model* pModel, int arg1, float arg2) {

    if (arg1 != subobjectEnableFlags) {
        subobjectEnableFlags = arg1;
        int shift = 1 << subobjectEnableFlags;

        for (int i = 0; i < pDescriptor->nmbrOfEntries; i++) {
            pModel->EnableSubObject(pDescriptor->pEntries[i].subObjectIndex, pDescriptor->pEntries[i].CheckFlags(shift));
        }

        if (pDescriptor->flags & LODFlags_Scissor) {
            int descScissorFlags = pDescriptor->scissorFlags;
            pModel->bScissoring = TestLOD(descScissorFlags);
        }
    }

    if (pDescriptor->flags & LODFlags_CameraFade) {
        if (subobjectEnableFlags == pDescriptor->invisibleZone - 1) {
            pModel->colour.w = arg2;
        } else if (subobjectEnableFlags <= 0 && pDescriptor->flags & LODFlags_CameraFade) {
            Vector center = *pModel->matrices[0].Row3();
            Vector target;
            Vector source;
            center.y += pDescriptor->height / 2.0f;
            GameCamera_GetVectors(&source, &target, NULL);
            if (RayToSphere(&source, &target, &center, 10.0f + pDescriptor->radius, -1.0f, true) && heroState != TY_AS_45) {
                pModel->colour.w = Max<float>(pDescriptor->minalpha, pModel->colour.w - 0.05625f);
            } else {
                pModel->colour.w = Min<float>(1.0f, pModel->colour.w + 0.05625f);
            }
        } else {
            pModel->colour.w = Min<float>(1.0f, pModel->colour.w + 0.05625f);
        }
    } else if (pDescriptor->flags & LODFlags_Alpha) {
        pModel->colour.w = arg2;
    }

}

bool LODManager::Draw(Model* pModel, int arg1, float arg2, float arg3, bool arg4) {
    bool ret;
    
    InternalUpdate(pModel, arg1, arg2);

    if ((pDescriptor->flags & LODFlags_Scissor) && pDescriptor->maxScissorDist > 0.0f) {
        pModel->bScissoring = arg3 < Sqr<float>(pDescriptor->maxScissorDist);
    }

    if (pModel->colour.w < 1.0f || pDescriptor->flags & LODFlags_AlphaProp) {
        if (subobjectEnableFlags <= 0 || (subobjectEnableFlags >= 7 || pDescriptor->flags & LODFlags_AlphaProp)) {
            Draw_AddPostDrawModel(pModel, arg3, arg4);
            return true;
        } else {
            ret = pModel->Draw(NULL);
        }
    } else {
        ret = pModel->Draw(NULL);
    }

    return ret;
}
