#include "types.h"
#include "common/MKAnimScript.h"
#include "common/KromeIni.h"
#include "common/PtrListDL.h"
#include "common/Str.h"
#include "common/FileSys.h"

extern "C" int strlen(char*);
extern "C" int strcmpi(char*, char*);
extern "C" int stricmp(char*, char*);
extern "C" char* strrchr(char*, char);
extern "C" int sscanf(const char*, const char*, ...);
extern "C" void strcpy(char*, char*);
extern "C" void memset(void*, int, int);

// TODO: Clean up fields

PtrListDL<MKAnimScriptTemplate> mkAnimScriptTemplates;

float MKAnimScript::advanceAmount = 0.5f;

void MKAnimScript_InitModule(void) {
    mkAnimScriptTemplates.Init(256, sizeof(MKAnimScriptTemplate));
}

void MKAnimScript_DeinitModule(void) {
    mkAnimScriptTemplates.Deinit();
}

static inline char* AddToStringTable(char* arg0, char* arg2) {
    int len = strlen(arg2);

    while (*arg0 != '\0') {
        int tmp_len = strlen(arg0);
        if ((tmp_len == len) && (strcmpi(arg0, arg2) == 0)) {
            return arg0;
        }
        arg0 += tmp_len + 1;
    }

    strcpy(arg0, arg2);
    arg0[len + 1] = '\0';
    return arg0;
}

// these inlines may be fake
static inline KromeIniLine* GetStartLine_Fake(KromeIni* pIni) {
    return pIni->GotoLine(NULL, NULL);
}
static inline void SplitLine(char* pString) {
    // line data would be "%s.mdl" or "%s.ase"
    // %s being any filename
    char* pExt = strrchr(pString, '.'); // get extension of file
    if ((pExt != NULL) && ((strcmpi(pExt, ".ase") == 0) || (strcmpi(pExt, ".mdl") != 0))) {
        *pExt = '\0';
    }
}
static inline void SplitLine2(char* pString) {
    // line data would be "%s.anm" or "%s.ase"
    // %s being any filename
    char* pExt = strrchr(pString, '.'); // get extension of file
    if ((pExt != NULL) && ((strcmpi(pExt, ".ase") == 0) || (strcmpi(pExt, ".anm") != 0))) {
        *pExt = '\0';
    }
}

void ParseBadFile(char* arg0, MKAnimScriptTemplate* pTemplate) {
    int badAnimCount = 0;
    int animEventCount = 0;
    int animRangeCount = 0;
    int stringSize = 1;

    KromeIni ini;
    ini.Init(arg0);

    KromeIniLine* pLine = ini.GotoLine(NULL, NULL);

    AnimEvent* pNextEvent;
    MKAnim* pCurAnim;

    while (pLine != NULL) {
        if (pLine->section != NULL) {
            stringSize += strlen(pLine->section) + 1;
            badAnimCount++;
        } else if (pLine->pFieldName != NULL) {
            if (strcmpi(pLine->pFieldName, "mesh") == 0) {
                stringSize += strlen(pLine->data) + 1;
            } else if (strcmpi(pLine->pFieldName, "skeleton") == 0) {
                stringSize += strlen(pLine->data) + 1;
            } else if (strcmpi(pLine->pFieldName, "anim") == 0) {
                stringSize += strlen(pLine->data) + 1;
                badAnimCount++;
            } else if ((*pLine->pFieldName >= '0') && (*pLine->pFieldName <= '9')) {
                animRangeCount++;
            } else if (strcmpi(pLine->pFieldName, "event") == 0) {
                char* pEventName = "";
                pLine->AsString(0, &pEventName);
                stringSize += strlen(pEventName) + 1;
                animEventCount++;
            }
        }
        
        pLine = ini.GetNextLine();
    }

    int templateSectionSize = sizeof(MKAnimTemplateSection) 
        + (badAnimCount * sizeof(MKAnim)) 
        + (animRangeCount * sizeof(AnimRange)) 
        + (animEventCount * sizeof(AnimEvent)) 
        + stringSize;
    
    MKAnimTemplateSection* pTemplateSection = (MKAnimTemplateSection*)Heap_MemAlloc(templateSectionSize);
    pTemplate->pSection = pTemplateSection;

    memset((void*)pTemplate->pSection, 0, templateSectionSize);

    AnimRange* pRanges = (AnimRange*)&pTemplate->pSection->anims[badAnimCount]; // get AnimRange structs (they are placed after MKAnim structs)

    pNextEvent = (AnimEvent*)&pRanges[animRangeCount];

    char* pStringTable = (char*)&pNextEvent[animEventCount];

    pStringTable[0] = pStringTable[1] = '\0';

    pCurAnim = NULL;

    strcpy(pTemplate->name, arg0);

    pLine = GetStartLine_Fake(&ini);
    while (pLine != NULL) {
        if (pLine->section != NULL) {
            MKAnimTemplateSection* pTemplateSection = pTemplate->pSection;
            pCurAnim = &pTemplateSection->anims[pTemplateSection->animCount++];
            pCurAnim->unk0 = AddToStringTable(pStringTable, pLine->section);
            pCurAnim->nmbrOfRanges = 0;
            pCurAnim->pAnimRanges = pRanges;
        } else if (pLine->pFieldName != NULL) {
            if (strcmpi(pLine->pFieldName, "mesh") == 0) {
                SplitLine(pLine->data);
                pTemplate->pSection->pMeshName = AddToStringTable(pStringTable, pLine->data);
            } else if (strcmpi(pLine->pFieldName, "skeleton") == 0) {
                SplitLine2(pLine->data);
                pTemplate->pSection->pAnimName = AddToStringTable(pStringTable, pLine->data);
            } else if (strcmpi(pLine->pFieldName, "anim") == 0) {
                MKAnimTemplateSection* pTemplateSection = pTemplate->pSection;
                pCurAnim = &pTemplateSection->anims[pTemplateSection->animCount++];
                pCurAnim->unk0 = AddToStringTable(pStringTable, pLine->data);
                pCurAnim->nmbrOfRanges = 0;
                pCurAnim->cycleType = 0;
                pCurAnim->unk6 = 0;
                pCurAnim->unk8 = 0;
                pCurAnim->unkB = 0;
                pCurAnim->pAnimRanges = pRanges;
            } else {
                int startFrame;
                int endFrame;
                int unk2_local; // speed
                if ((*pLine->pFieldName >= '0') && (*pLine->pFieldName <= '9')) {
                    // sscanf returns the number of variables filled
                    switch (sscanf(pLine->pFieldName, "%d-%d,%d", &startFrame, &endFrame, &unk2_local)) {
                        case 0: // if zero elements are filled, fallthrough entire switch setting default values
                            startFrame = 0;
                            // fallthrough
                        case 1:
                            endFrame = startFrame;
                            // fallthrough
                        case 2:
                            unk2_local = 1;
                            break;
                    }
                    pRanges->startFrame = startFrame;
                    pRanges->endFrame = endFrame;
                    pRanges->unk4 = unk2_local; // speed
                    pRanges->nmbrOfEvents = 0;
                    pRanges->pEvents = pNextEvent;
                    pRanges->unkC = 0;
                    pRanges++;
                    pCurAnim->nmbrOfRanges++;
                    pCurAnim->unk6 = (pCurAnim->unk6 + endFrame) - startFrame;
                } else if (strcmpi(pLine->pFieldName, "event") == 0) {
                    char* unk0 = "";
                    char* unk1 = "";
                    int startEventFrame;
                    int endEventFrame;
                    pLine->AsString(0, &unk0);
                    pLine->AsString(1, &unk1);
                    // sscanf returns the number of variables filled
                    switch (sscanf(unk1, "%d-%d", &startEventFrame, &endEventFrame)) {
                        case 0: // if zero elements are filled, fallthrough entire switch setting default values
                            startEventFrame = -1;
                            // fallthrough
                        case 1:
                            endEventFrame = startEventFrame;
                            // fallthrough
                        case 2:
                            break;
                    }
                    if (startEventFrame != -1) {
                        int i = 0;
                        for (; i < pCurAnim->nmbrOfRanges; i++) {
                            // find which animation range this event belongs to
                            if (pCurAnim->pAnimRanges[i].startFrame <= startEventFrame && pCurAnim->pAnimRanges[i].endFrame >= endEventFrame) {
                                break;
                            }
                        }
                        if (i < pCurAnim->nmbrOfRanges) {
                            // set event fields of range
                            pCurAnim->pAnimRanges[i].pEvents[pCurAnim->pAnimRanges[i].nmbrOfEvents].startEventFrame = startEventFrame;
                            pCurAnim->pAnimRanges[i].pEvents[pCurAnim->pAnimRanges[i].nmbrOfEvents].endEventFrame = endEventFrame;
                            pCurAnim->pAnimRanges[i].pEvents[pCurAnim->pAnimRanges[i].nmbrOfEvents].unk0_string = AddToStringTable(pStringTable, unk0);
                            pCurAnim->pAnimRanges[i].nmbrOfEvents++;
                            pNextEvent++;
                        }
                    }
                } else if (strcmpi(pLine->pFieldName, "cycle") == 0) {
                    char* pType = "";
                    pLine->AsString(0, &pType);
                    if (strcmpi(pType, "loop") == 0) {
                        pCurAnim->cycleType = CycleType_Loop;
                    } else if (strcmpi(pType, "rebound") == 0) {
                        pCurAnim->cycleType = CycleType_Rebound;
                    } else if (strcmpi(pType, "stop") == 0) {
                        pCurAnim->cycleType = CycleType_Stop;
                    } else {
                        pCurAnim->cycleType = CycleType_Stop; // Stop is default
                    }
                }
            }
        }
        pLine = ini.GetNextLine();
    }
    ini.Deinit();

    pTemplate->templateSize = (int)pStringTable - (int)pTemplate->pSection;

    while (((char*)pTemplate->pSection)[pTemplate->templateSize] != '\0' || ((char*)pTemplate->pSection + 1)[pTemplate->templateSize] != '\0') {
        pTemplate->templateSize++;
    }
    pTemplate->templateSize += 2;
}

void MKAnimScript::Init(char* pFilename) {
    MKAnimScriptTemplate* pFound;
    MKAnimScriptTemplate** pList = (MKAnimScriptTemplate**)mkAnimScriptTemplates.pMem;
    while (*pList != NULL) {
        if (stricmp((*pList)->name, pFilename) == 0) {
            break;
        }
        pList++;
    }
    pFound = *pList;
    if (pFound != NULL) {
        pTemplate = pFound;
        pTemplate->referenceCount++;
    } else {
        pTemplate = mkAnimScriptTemplates.GetNextEntry();
        int size;
        char* fileName = Str_Printf("%s.gas", pFilename);
        if (FileSys_Exists(fileName, &size)) {
            pTemplate->pSection = (MKAnimTemplateSection*)FileSys_Load(fileName, NULL, NULL, -1);
            pTemplate->templateSize = size;
            pTemplate->UnpackTemplate();
        } else {
            fileName = Str_Printf("%s.bad", pFilename);
            if (FileSys_Exists(fileName, &size)) {
                ParseBadFile(fileName, pTemplate);
            } else {
                return;
            }
        }
        strcpy(pTemplate->name, pFilename);
        pTemplate->referenceCount = 1;
    }
    nextAnim = NULL;
    currAnim = NULL;
    unk10 = 0.0f;
    unkC = 0.0f;
    unk14 = 0.0f;
    unk18 = 0;
    unk1A = 0;
    unk1E = 0;
    unk1C = 0;
}

// currently this does not inline Init(char*)
// defining it as inline matches
// if it is defined as inline, will it be stripped by the linker even though there are calls to the function outside of this file?
// https://decomp.me/scratch/jqJsO
/*void MKAnimScript::Init(MKAnimScript* pOther) {
    Init(pOther->pTemplate->name);
}*/
void MKAnimScript::Init(MKAnimScript* pOther) {
    char* otherName = pOther->pTemplate->name;
    MKAnimScriptTemplate* pFound;
    MKAnimScriptTemplate** pList = mkAnimScriptTemplates.GetMem();
    while (*pList != NULL) {
        if (stricmp((*pList)->name, otherName) == 0) {
            break;
        }
        pList++;
    }
    pFound = *pList;
    if (pFound != NULL) {
        pTemplate = pFound;
        pTemplate->referenceCount++;
    } else {
        pTemplate = mkAnimScriptTemplates.GetNextEntry();
        int size;
        char* fileName = Str_Printf("%s.gas", otherName);
        if (FileSys_Exists(fileName, &size) != false) {
            pTemplate->pSection = (MKAnimTemplateSection *)FileSys_Load(fileName, NULL, NULL, -1);
            pTemplate->templateSize = size;
            pTemplate->UnpackTemplate();
        } else {
            fileName = Str_Printf("%s.bad", otherName);
            if (FileSys_Exists(fileName, &size) == false) {
                return;
            }
            ParseBadFile(fileName, pTemplate);
        }
        strcpy(pTemplate->name, otherName);
        pTemplate->referenceCount = 1;
    }
    nextAnim = NULL;
    currAnim = NULL;
    unk10 = 0.0f;
    unkC = 0.0f;
    unk14 = 0.0f;
    unk18 = 0;
    unk1A = 0;
    unk1E = 0;
    unk1C = 0;
}

void MKAnimScript::Deinit(void) {
    if (--pTemplate->referenceCount == 0) {
        Heap_MemFree(pTemplate->pSection);
        mkAnimScriptTemplates.Destroy(pTemplate);
    }
}

MKAnim* MKAnimScript::GetAnim(char* pAnimName) {
    if (*pAnimName == '\0') {
        return NULL;
    }

    for (int i = 0; i < pTemplate->pSection->animCount; i++) {
        if (strcmpi(pAnimName, pTemplate->pSection->anims[i].unk0) == 0) {
            return &pTemplate->pSection->anims[i];
        }
    }

    return NULL;
}

MKAnim* MKAnimScript::GetAnim(int animNumber) {
    if (animNumber < pTemplate->pSection->animCount) {
        return &pTemplate->pSection->anims[animNumber];
    }

    return NULL;
}

bool MKAnimScript::Exists(char* pAnimName) {
    for (int i = 0; i < pTemplate->pSection->animCount; i++) {
        if (strcmpi(pAnimName, pTemplate->pSection->anims[i].unk0) == 0) {
            return true;
        }
    }

    return false;
}

char* MKAnimScript::GetMeshName(void) {
    return pTemplate->pSection->pMeshName;
}

char* MKAnimScript::GetAnimName(void) {
    return pTemplate->pSection->pAnimName;
}

void MKAnimScript::SetAnim(MKAnim* pMKAnim) {
    currAnim = pMKAnim;
    unkC = (float)currAnim->pAnimRanges->startFrame;
    unk1A = -1;
    unk1E = 0;
    s16 tmp = currAnim->pAnimRanges->unk4;
    unk14 = (tmp != 0) ? 1.0f / (float)tmp : 1.0f;
    unk1C = 0;
    nextAnim = NULL;
    unk18 = 0;
}

void MKAnimScript::TweenAnim(MKAnim* pToAnim, short arg2) {
    nextAnim = pToAnim;
    unk10 = (float)nextAnim->pAnimRanges->startFrame;
    unk1C = arg2;
    unk18 = 0;
}

void MKAnimScript::Animate(void) {
    unk1A = (short)unkC;
    if (unk14) {
        float tempC = unkC;
        float fVar1 = tempC + (unk14 * advanceAmount);
        AnimRange* pRange = &currAnim->pAnimRanges[unk1E];
        MKAnim* pAnim = currAnim;
        if (fVar1 < ((float)pRange->startFrame) || fVar1 > ((float)pRange->endFrame)) {
            if (unk14 > 0.0f && (unk1E + 1 < pAnim->nmbrOfRanges)) {
                AnimRange* pRange = &currAnim->pAnimRanges[++unk1E];
                float fVar5;
                fVar1 = (float)pRange->startFrame;
                if (pRange->unk4 != 0) {
                    fVar5 = 1.0f / currAnim->pAnimRanges[0].unk4;
                } else {
                    fVar5 = 1.0f;
                }
                unk14 = fVar5;
            } else if (unk14 < 0.0f && unk1E != NULL) {
                AnimRange* pRange = &currAnim->pAnimRanges[--unk1E];
                fVar1 = (float)pRange->startFrame;
                float fVar5;
                if (pRange->unk4 != 0) {
                    fVar5 = 1.0f / currAnim->pAnimRanges[0].unk4;
                } else {
                    fVar5 = 1.0f;
                }
                unk14 = fVar5;
            } else {
                switch (pAnim->cycleType) {
                case CycleType_Stop:
                    unk14 = 0.0f;
                    fVar1 = tempC;
                    break;
                case CycleType_Loop:
                    unk1E = 0;
                    AnimRange* pRange = &currAnim->pAnimRanges[0];
                    fVar1 = (float)pRange->startFrame;
                    int unk2 = currAnim->pAnimRanges[0].unk4;
                    float fVar5;
                    if (pRange->unk4 != 0) {
                        fVar5 = 1.0f / (float)currAnim->pAnimRanges[0].unk4;
                    } else {
                        fVar5 = 1.0f;
                    }
                    unk14 = fVar5;
                    unk18++;
                    break;
                case CycleType_Rebound:
                    unk14 = -unk14;
                    if (unk14 > 0.0f) {
                        unk18++;
                    }
                    break;
                }
            }
        }
        unkC = fVar1;
    }
}

void MKAnimScript::Apply(Animation* pAnimation) {
    if (unk1C != 0) {
        pAnimation->Tween(unk10, 1.0f / (float)unk1C);
        if (--unk1C == 0) {
            SetAnim(nextAnim);
        }
    } else {
        pAnimation->Tween(unkC, 1.0f);
    }
}

void MKAnimScript::ApplyNode(Animation* pAnimation, int nodeIndex) {
    if (unk1C != 0) {
        pAnimation->TweenNode(unk10, 1.0f / (float)unk1C, nodeIndex);
        if (--unk1C == 0) {
            SetAnim(nextAnim);
        }
    } else {
        pAnimation->TweenNode(unkC, 1.0f, nodeIndex);
    }
}

char* MKAnimScript::GetEventByName(char* pName) {
    for (int i = 0; i < pTemplate->pSection->animCount; i++) {
        for (int j = 0; j < pTemplate->pSection->anims[i].nmbrOfRanges; j++) {
            for (int z = 0; z < pTemplate->pSection->anims[i].pAnimRanges[j].nmbrOfEvents; z++) {
                if (strcmpi(pName, pTemplate->pSection->anims[i].pAnimRanges[j].pEvents[z].unk0_string) == 0) {
                    return pTemplate->pSection->anims[i].pAnimRanges[j].pEvents[z].unk0_string;
                }
            }
        }
    }

    return NULL;
}

char* MKAnimScript::GetEvent(int eventIdx) {
    int j = 0;
    s16 iUnkC = unkC;
    if (currAnim != NULL && iUnkC != unk1A && unk1C == 0) {
        AnimRange* pRange = &currAnim->pAnimRanges[unk1E];
        for (int i = 0; i < pRange->nmbrOfEvents; i++) {
            if (iUnkC >= pRange->pEvents[i].startEventFrame && iUnkC <= pRange->pEvents[i].endEventFrame && j++ == eventIdx) {
                return pRange->pEvents[i].unk0_string;
            }
        }
    }

    return NULL;
}

bool MKAnimScript::HasLooped(void) {
    return currAnim && (unkC == (float)currAnim->pAnimRanges[0].startFrame) &&
        (currAnim->pAnimRanges[currAnim->nmbrOfRanges - 1].endFrame == unk1A);
}

int MKAnimScript::UpdatesUntilFinished(void) {
    if (!unk14 || currAnim == NULL) {
        return 0;
    }
    float fVar3 = currAnim->pAnimRanges[unk1E].endFrame - unkC;
    int start = unk1E + 1;

    while (start < currAnim->nmbrOfRanges) {
        AnimRange* pRange = &currAnim->pAnimRanges[start];
        fVar3 += pRange->startFrame - pRange->endFrame;
        start++;
    }

    return fVar3 / advanceAmount;
}

int MKAnimScript::GetLength(void) {
    if (currAnim != NULL) {
        return currAnim->unk6;
    }

    return 0;
}

void MKAnimScript::GetStartAndEnd(MKAnim* pAnim, short* pStart, short* pEnd) {
    if (pStart != NULL) {
        *pStart = pAnim->pAnimRanges[0].startFrame;
    }

    if (pEnd != NULL) {
        *pEnd = pAnim->pAnimRanges[pAnim->nmbrOfRanges - 1].endFrame;
    }
}

void MKAnimScript::SetAnimKeepingPosition(MKAnim* pMKAnim) {
    float fVar3 = 0.0f;
    if (currAnim != NULL) {
        float fVar1 = 0.0f;
        float fVar2 = 0.0f;
        int i = 0;
        for (int index = 0; index < currAnim->nmbrOfRanges; index++) {
            if (i == unk1E) {
                fVar2 = fVar1 + (unkC - (float)currAnim->pAnimRanges[index].startFrame);
            }
            i++;
            fVar1 += currAnim->pAnimRanges[index].endFrame - currAnim->pAnimRanges[index].startFrame;
        }

        fVar3 = fVar2 / fVar1;
    }

    SetAnim(pMKAnim);
    if (fVar3 != 0.0f) {
        float fVar2 = fVar3 * GetLength();
        for (int i = 0; i < currAnim->nmbrOfRanges; i++) {
            float fVar1 = currAnim->pAnimRanges[i].endFrame - currAnim->pAnimRanges[i].startFrame;
            if (fVar1 < fVar2) {
                fVar2 -= fVar1;
            } else {
                unkC = fVar2 + (float)currAnim->pAnimRanges[i].startFrame;
                unk1E = (u8)i;
                return;
            }
        }
    }
}

float MKAnimScript::GetNormalPosition(void) {
    float fVar3 = 0.0f;
    if (currAnim != NULL) {
        float fVar1 = 0.0f;
        float fVar2 = 0.0f;
        int index = currAnim->nmbrOfRanges;
        int i = 0;

        for (int index = 0; index < currAnim->nmbrOfRanges; index++) {
            if (i == unk1E) {
                fVar2 = fVar1 + (unkC - (float)currAnim->pAnimRanges[index].startFrame);
            }
            i++;
            fVar1 += currAnim->pAnimRanges[index].endFrame - currAnim->pAnimRanges[index].startFrame;
        }

        if (fVar1 != 0.0f) {
            fVar3 = fVar2 / fVar1;
        }
    }

    return fVar3;
}

float MKAnimScript::GetFrameOfNormalPosition(float arg1, MKAnim* pAnim) {
    float fVar3 = (float)pAnim->unk6 * arg1;
    int index = 0;

    for (int i = 0; i < pAnim->nmbrOfRanges; i++) {
        AnimRange* pRange = &pAnim->pAnimRanges[i];
        float fVar1 = fVar3 + (float)pRange->startFrame;
        
        if (fVar1 < pRange->endFrame) {
            return pAnim->pAnimRanges[index].startFrame + fVar3;
        }

        index++;
        fVar3 -= (float)(pRange->endFrame - pRange->startFrame);
    }

    return pAnim->pAnimRanges[pAnim->nmbrOfRanges - 1].endFrame;
}

void MKAnimScript::SetAnimNormalised(MKAnim* pMKAnim, float normal) {
    SetAnim(pMKAnim);
    normal *= (float)currAnim->unk6;
    if (currAnim->nmbrOfRanges == 1) {
        unkC += normal;
    } else {
        for (int i = 0; i < currAnim->nmbrOfRanges; i++) {
            if (currAnim->pAnimRanges[i].startFrame + normal < (float)currAnim->pAnimRanges[i].endFrame) {
                unk1E = (s8)i;
                unkC = currAnim->pAnimRanges[i].startFrame + normal;
                return;
            }
            normal -= currAnim->pAnimRanges[i].endFrame - currAnim->pAnimRanges[i].startFrame;
        }
    }
}

static inline void Template_UpdateAddress(int* arg0, int baseAddress) {
    *arg0 += baseAddress;
}

void MKAnimScriptTemplate::UnpackTemplate(void) {
    Template_UpdateAddress((int*)&pSection->pMeshName, (int)pSection);
    Template_UpdateAddress((int*)&pSection->pAnimName, (int)pSection);
    for (int animIndex = 0; animIndex < pSection->animCount; animIndex++) {
        Template_UpdateAddress((int*)&pSection->anims[animIndex].unk0, (int)pSection);
        Template_UpdateAddress((int*)&pSection->anims[animIndex].pAnimRanges, (int)pSection);
        for (int rangeIndex = 0; rangeIndex < pSection->anims[animIndex].nmbrOfRanges; rangeIndex++) {
            Template_UpdateAddress((int*)&pSection->anims[animIndex].pAnimRanges[rangeIndex].pEvents, (int)pSection);
            Template_UpdateAddress((int*)&pSection->anims[animIndex].pAnimRanges[rangeIndex].unkC, (int)pSection);
            for (int eventIndex = 0; eventIndex < pSection->anims[animIndex].pAnimRanges[rangeIndex].nmbrOfEvents; eventIndex++) {
                Template_UpdateAddress((int*)&pSection->anims[animIndex].pAnimRanges[rangeIndex].pEvents[eventIndex].unk0_string, (int)pSection);
            }
        }
    }
}