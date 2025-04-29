#include "types.h"
#include "common/System_GC.h"
#include "common/Model.h"
#include "common/Heap.h"
#include "common/FileSys.h"
#include "common/Str.h"
#include "common/Animation.h"

#include "Dolphin/os.h"

bool moduleInitialised = false;
PtrList<Model> modelInstances;
PtrList<ModelTemplate> modelTemplates;
int pCounterModelsDrawnRejected;
int pCounterModelTrianglesDrawn;
int pCounterModelTrianglesInScene;
int pCounterModelSubObjectsRejected;
static int pDebugOptionDrawBoundingBoxes;
static int pDebugDave;
static int pModelMenu;

void Model_InitModule(void) {
    modelInstances.Init(gMKDefaults.numModelInstances);
    modelTemplates.Init(gMKDefaults.numModelTemplateInstances);
    
    pCounterModelTrianglesInScene = 0;
    pCounterModelsDrawnRejected = 0;
    pCounterModelTrianglesDrawn = 0;
    pCounterModelSubObjectsRejected = 0;
    pModelMenu = 0;
    pDebugOptionDrawBoundingBoxes = 0;
    pDebugDave = 0;
    moduleInitialised = true;
}

void Model_DeinitModule(void) {
    Model::Purge();
    Model::Purge();
    Model::Purge();
    
    ModelTemplate** ppTemplates = modelTemplates.pPointers;
    while (*ppTemplates != NULL) {
        ppTemplates++;
    }

    modelInstances.Deinit();
    modelTemplates.Deinit();
    moduleInitialised = false;
}

extern "C" void strcpy(char*, char*);
extern "C" char* strtok(char*, char*);
extern "C" void memset(void*, int, int);
extern "C" void strncpy(char*, char*, int);
extern "C" int stricmp(char*, char*);

Model* Model::Create(char* pMeshName, char* pAnimName) {
    char meshName[0x20];
    char animName[0x20];
    int size;
    Model* pModel;
    ModelTemplate* pModelTemplate;
    
    strcpy(meshName, pMeshName);
    strtok(meshName, ".");

    if (pAnimName != NULL) {
        strcpy(animName, pAnimName);
        strtok(animName, ".");
    }

    ModelTemplate** pTemplates = modelTemplates.pPointers;
    while (*pTemplates != NULL) {
        if (stricmp((*pTemplates)->name, meshName) == 0) {
            break;
        }
        pTemplates++;
    }

    ModelTemplate* pFoundTemplate = *pTemplates;
    if (pFoundTemplate != NULL) {
        // if this template isn't a new instance, increment the reference count
        pFoundTemplate->referenceCount++;
        pModelTemplate = pFoundTemplate;
    } else {
        char* fileName = Str_Printf("%s%s", meshName, ".gmd");
        FileSys_Exists(fileName, &size);
        pModelTemplate = (ModelTemplate*)Heap_MemAlloc(sizeof(ModelTemplate));
        memset(pModelTemplate, 0, sizeof(ModelTemplate));
        strncpy(pModelTemplate->name, meshName, sizeof(pModelTemplate->name) - 1);
        fileName = Str_Printf("%s%s", meshName, ".gmd");
        pModelTemplate->pModelData = (ModelData*)FileSys_Load(fileName, &size, NULL, -1);
        pModelTemplate->templateDataSize = size;
        Model_UnpackTemplate(pModelTemplate);
        DCStoreRange((uint*)pModelTemplate->pModelData, size);
        pModelTemplate->referenceCount = 1;
        modelTemplates.AddEntry(pModelTemplate);
    }
    
    int modelSize = (sizeof(Matrix) * (pModelTemplate->pModelData->nmbrOfMatrices - 1)) +
        sizeof(Model) + (pModelTemplate->pModelData->nmbrOfMatrices * sizeof(Matrix*));
    pModel = (Model*)Heap_MemAlloc(pModelTemplate->pModelData->nmbrOfSubObjects + modelSize);
    pModel = modelInstances.AddEntry(pModel);
    pModel->pTemplate = pModelTemplate;
    pModel->pMatrices = pModel->matrices;
    pModel->unkC = (float*)(pModel->pMatrices + pModelTemplate->pModelData->nmbrOfMatrices);
    pModel->subobjectData = (u8*)(pModel->unkC + pModelTemplate->pModelData->nmbrOfMatrices);

    if (pAnimName != NULL) {
        pModel->SetAnimation(Animation::Create(animName, pModel->pMatrices));
        pModel->bHasAnimation = true;
    } else {
        pModel->pAnimation = NULL;
        pModel->bHasAnimation = false;
    }

    pModel->toBeDestroyed = 0;
    pModel->renderType = -1;
    pModel->bScissoring = false;
    pModel->collisionTracking = false;
    pModel->b5 = 0;

    pModel->colour.Set(1.0f, 1.0f, 1.0f, 1.0f);

    for (int matrixIdx = 0; matrixIdx < pModel->pTemplate->pModelData->nmbrOfMatrices; matrixIdx++) {
        pModel->pMatrices[matrixIdx].SetIdentity();
        pModel->unkC[matrixIdx] = 1.0f;
    }

    memset(pModel->subobjectData, 0, pModelTemplate->pModelData->nmbrOfSubObjects);
    return pModel;
}

/// @brief Begins the process of freeing a model
void Model::Destroy(void) {
    toBeDestroyed = 3;

    if (bHasAnimation && pAnimation) {
        pAnimation->Destroy();
        pAnimation = NULL;
        bHasAnimation = false;
    }
}

/// @brief Attempts to free all models that are no longer active
void Model::Purge(void) {
    Model** ppModels = modelInstances.GetPointers();

    while (*ppModels) {
        Model* pModel = *ppModels;
        if (pModel->toBeDestroyed != 0 && --pModel->toBeDestroyed == 0) {
            if ((*ppModels)->pTemplate && --(*ppModels)->pTemplate->referenceCount == 0) {
                // Destroy and free the ModelTemplate if there are no more references to it

                // destroy all subobject materials
                for (int i = 0; i < (*ppModels)->pTemplate->pModelData->nmbrOfSubObjects; i++) {
                    for (int j = 0; j < (*ppModels)->pTemplate->pModelData->pSubObjects[i].nmbrOfMaterials; j++) {
                        (*ppModels)->pTemplate->pModelData->pSubObjects[i].pMaterials[j].pMaterial->Destroy();
                    }
                }

                modelTemplates.Destroy((*ppModels)->pTemplate);
                Heap_MemFree((*ppModels)->pTemplate->pModelData);
                Heap_MemFree((*ppModels)->pTemplate);
                (*ppModels)->pTemplate = NULL;
            }

            Heap_MemFree(*ppModels);
            modelInstances.UnknownInline(ppModels);
        }
        
        ppModels++;
    }
    
}

void Model::SetPosition(Vector* pPos) {
    pMatrices->SetTranslation(pPos);
    if (pAnimation != NULL) {
        pAnimation->SetLocalToWorldDirty();
    }
}

void Model::SetRotation(Vector* pRot) {
    pMatrices->SetRotationPYR(pRot);
    if (pAnimation != NULL) {
        pAnimation->SetLocalToWorldDirty();
    }
}

void Model::SetAnimation(Animation* pAnim) {
    pAnimation = pAnim;
    if (pTemplate->pModelData->nmbrOfAnimNodes == 0) {
        return;
    }

    if (pAnim == NULL) {
        return;
    }

    if (pTemplate->pModelData->unk14 == NULL) {
        return;
    }

    for (int i = 0; i < pTemplate->pModelData->nmbrOfAnimNodes; i++) {
        pAnim->frames[i].pOrigin = &pTemplate->pModelData->unk14[i];
    }
}

void Model::SetLocalToWorldDirty(void) {
    if (pAnimation != NULL) {
        pAnimation->SetLocalToWorldDirty();
    }
}

bool Model::IsSubObjectEnabled(int subObjectIndex) {
    return !(subobjectData[subObjectIndex] & 1);
}

void Model::EnableSubObject(int subObjectIndex, bool arg2) {
    subobjectData[subObjectIndex] = (subobjectData[subObjectIndex] & ~1) | (arg2 ? 0 : 1); // lowest bit determines if it's active
}

void Model::EnableOnlySubObject(int subObjectIndex, bool arg2) {
    for (int i = 0; i < pTemplate->pModelData->nmbrOfSubObjects; i++) {
        subobjectData[i] &= ~1;
        if ((i == subObjectIndex) != arg2) {
            subobjectData[i] |= 1;
        }
    }
}

void Model::SetInverseScaleValue(int idx, float arg2) {
    if (idx == -1) {
        for (int i = 0; i < pTemplate->pModelData->nmbrOfMatrices; i++) {
            unkC[i] = arg2;
        }
    } else {
        unkC[idx] = arg2;
    }
}

extern "C" int strcmpi(char*, char*);

// optional parameter pRefPointIdx
// pass NULL if the refpoint index is unneeded
// the value at pRefPointIdx is set to the index of the found refpoint
// returns true if the refpoint exists
// else returns false
bool Model::RefPointExists(char* pRefPointName, int* pRefPointIdx) {
    for (int i = 0; i < pTemplate->pModelData->nmbrOfRefPoints; i++) {
        if (strcmpi(pTemplate->pModelData->pRefPoints[i].pName, pRefPointName) == 0) {
            if (pRefPointIdx != NULL) {
                *pRefPointIdx = i;
            }

            return true;
        }
    }

    return false;
}

int Model::GetRefPointIndex(char* pRefPointName) {
    int ret;
    gAssertBool = RefPointExists(pRefPointName, &ret);
    return ret;
}

Vector* Model::GetRefPointOrigin(int refPointIndex) {
    return &pTemplate->pModelData->pRefPoints[refPointIndex].position;
}

void Model::GetRefPointWorldPosition(int refPointIndex, Vector* pOut) {
    *pOut = pTemplate->pModelData->pRefPoints[refPointIndex].position;
    
    if ((int)pTemplate->pModelData->pRefPoints[refPointIndex].matrix1 == 0) {
        pOut->ApplyMatrix(pMatrices);
        return;
    }

    pOut->ApplyMatrix(pAnimation->GetNodeMatrix(pTemplate->pModelData->pRefPoints[refPointIndex].matrix1 - 1));
}

// optional parameter pSubObjectIdx
// pass NULL if the subobject index is unneeded
// the value at pSubObjectIdx is set to the index of the found subobject
// returns true if the subobject exists
// else returns false
bool Model::SubObjectExists(char* pSubObjectName, int* pSubObjectIdx) {
    for (int i = 0; i < pTemplate->pModelData->nmbrOfSubObjects; i++) {
        if (strcmpi(pTemplate->pModelData->pSubObjects[i].pName, pSubObjectName) == 0) {
            if (pSubObjectIdx != NULL) {
                *pSubObjectIdx = i;
            }

            return true;
        }
    }

    return false;
}

int Model::GetSubObjectIndex(char* pSubObjectName) {
    int ret;
    gAssertBool = SubObjectExists(pSubObjectName, &ret);
    return ret;
}

int Model::GetSubObjectMatrixIndex(int subObjectIndex) {
    return pTemplate->pModelData->pSubObjects[subObjectIndex].matrixIndex;
}

Vector* Model::GetSubObjectOrigin(int subObjectIndex) {
    return &pTemplate->pModelData->pSubObjects[subObjectIndex].origin;
}

void Model::List(void) {
    if (!modelTemplates.IsEmpty()) {
        ModelTemplate** ppTemplates = modelTemplates.pPointers;
        while (*ppTemplates != NULL) {
            ppTemplates++;
        }
    }
}

void Ptr_Fixup(char** x, ModelData* pData) {
    if (*x != NULL) {
        *x = (char*)((int)pData + *x);
    }
}

void Model_UnpackTemplate(ModelTemplate* pTemplate) {
    ModelData* pData = pTemplate->pModelData;
    int i;

    Ptr_Fixup((char**)&pData->pSubObjects, pData);
    Ptr_Fixup((char**)&pData->pRefPoints, pData);
    Ptr_Fixup((char**)&pData->unk14, pData);
    Ptr_Fixup((char**)&pData->pVertices, pData);

    for (i = 0; i < pData->nmbrOfSubObjects; i++) {
        Ptr_Fixup(&pData->pSubObjects[i].pName, pData);
        Ptr_Fixup(&pData->pSubObjects[i].pOptions, pData);
        Ptr_Fixup((char**)&pData->pSubObjects[i].pMaterials, pData);
        
        for (int j = 0; j < pData->pSubObjects[i].nmbrOfMaterials; j++) {
            Ptr_Fixup(&pData->pSubObjects[i].pMaterials[j].pMaterialName, pData);
            Ptr_Fixup((char**)&pData->pSubObjects[i].pMaterials[j].pStripData, pData);
            pData->pSubObjects[i].pMaterials[j].pMaterial = Material::Create(pData->pSubObjects[i].pMaterials[j].pMaterialName);
        }
    }

    for (i = 0; i < pData->nmbrOfRefPoints; i++) {
        Ptr_Fixup(&pData->pRefPoints[i].pName, pData);
    }
}

void Model::GetCentre(Vector* pCentre) {
    ModelData* pData = pTemplate->pModelData;
    float cx = pData->volume.v1.x + 0.5f * pData->volume.v2.x;
    float cy = pData->volume.v1.y + 0.5f * pData->volume.v2.y;
    float cz = pData->volume.v1.z + 0.5f * pData->volume.v2.z;
    pCentre->Set(pMatrices[0].data[3][0] + cx, pMatrices[0].data[3][1] + cy, pMatrices[0].data[3][2] + cz);
}

int Model::GetNmbrOfMatrices(void) {
    return pTemplate->pModelData->nmbrOfMatrices;
}

int Model::GetNmbrOfSubObjects(void) {
    return pTemplate->pModelData->nmbrOfSubObjects;
}

char* Model::GetSubObjectName(int subObjectIndex) {
    return pTemplate->pModelData->pSubObjects[subObjectIndex].pName;
}

// returns the model's volume if subObjectIndex == -1
// else returns subobject volume
BoundingVolume* Model::GetBoundingVolume(int subObjectIndex) {
    if (subObjectIndex < 0) {
        return &pTemplate->pModelData->volume;
    } else {
        return &pTemplate->pModelData->pSubObjects[subObjectIndex].volume;
    }
}

char* Model::GetName(void){
    return pTemplate->name;
}

void Model::SetAlphaLightIntensity(int subObjectIndex, float intensity) {
    if (subObjectIndex == -1) {
        // set intensity for all subobjects if index is -1
        for (int i = 0; i < pTemplate->pModelData->nmbrOfSubObjects; i++) {
            pTemplate->pModelData->pSubObjects[i].alphaLightIntensity = intensity;
        }
    } else {
        pTemplate->pModelData->pSubObjects[subObjectIndex].alphaLightIntensity = intensity;
    }
}

void Model::SetRenderTypeOverride(int renderTypeOverride) {
    renderType = renderTypeOverride;
}

Material* Model::GetSubObjectMaterial(int subObjectIndex, int materialIndex) {
    return pTemplate->pModelData->pSubObjects[subObjectIndex].pMaterials[materialIndex].pMaterial;
}
