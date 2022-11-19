#include "types.h"
#include "common/Model.h"
#include "common/Heap.h"
#include "common/FileSys.h"
#include "common/Str.h"
#include "common/PtrList.h"

// from Str.cpp
extern bool gAssertBool;

// from a different file
struct MKDefaults {
	s8 padding[0x44];
	int modelInstanceCount;
	int modelTemplateCount;
	int padding1[13];
};
extern MKDefaults gMKDefaults;

bool moduleInitialised;
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
    modelInstances.Init(gMKDefaults.modelInstanceCount);
    modelTemplates.Init(gMKDefaults.modelTemplateCount);
    
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
    while (*--modelInstances.pPointers != NULL) {}
    if (modelInstances.pPointers != (Model**)&gEmptyPtrList) {
        Heap_MemFree(modelInstances.pPointers);
    }
    modelInstances.pPointers = NULL;
    while (*--modelTemplates.pPointers != NULL) {}
    if (modelTemplates.pPointers != (ModelTemplate**)&gEmptyPtrList) {
        Heap_MemFree(modelTemplates.pPointers);
    }
    modelTemplates.pPointers = NULL;
    moduleInitialised = false;
}

extern "C" void strcpy(char*, char*);
extern "C" char* strtok(char*, char*);
extern "C" void memset(void*, int, int);
extern "C" void strncpy(char*, char*, int);
extern "C" void DCStoreRange(uint*, int);
extern "C" int stricmp(char*, char*);

Model* Model::Create(char* pMeshName, char* pAnimName) {
    char mesh[0x20];
    char anim[0x20];
    int fileSize;
    Model* pModel;
    ModelTemplate* pModelTemplate;
    strcpy(mesh, pMeshName);
    strtok(mesh, ".");
    if (pAnimName != NULL) {
        strcpy(anim, pAnimName);
        strtok(anim, ".");
    }
    ModelTemplate** pTemplates = modelTemplates.pPointers;
    while (*pTemplates != NULL) {
        if (stricmp((*pTemplates)->name, mesh) == 0) {
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
        char* fileName = Str_Printf("%s%s", mesh, ".gmd");
        FileSys_Exists(fileName, &fileSize);
        pModelTemplate = (ModelTemplate*)Heap_MemAlloc(sizeof(ModelTemplate));
        memset(pModelTemplate, 0, sizeof(ModelTemplate));
        strncpy(pModelTemplate->name, mesh, 31);
        fileName = Str_Printf("%s%s", mesh, ".gmd");
        pModelTemplate->pModelData = (ModelData*)FileSys_Load(fileName, &fileSize, NULL, -1);
        pModelTemplate->templateDataSize = fileSize;
        Model_UnpackTemplate(pModelTemplate);
        DCStoreRange((uint*)pModelTemplate->pModelData, fileSize);
        pModelTemplate->referenceCount = 1;
        modelTemplates.AddEntry(pModelTemplate);
    }
    int modelSize = (sizeof(Matrix) * (pModelTemplate->pModelData->nmbrOfMatrices - 1)) + sizeof(Model) + (pModelTemplate->pModelData->nmbrOfMatrices * 4);
    Model* out = (Model*)Heap_MemAlloc(pModelTemplate->pModelData->nmbrOfSubObjects + modelSize);
    pModel = modelInstances.AddEntry(out);
    pModel->pTemplate = pModelTemplate;
    pModel->pMatrices = (Matrix*)&pModel->matrices;
    pModel->unkC = (float*)(pModel->pMatrices + pModelTemplate->pModelData->nmbrOfMatrices);
    pModel->subobjectData = (u8*)(pModel->unkC + pModelTemplate->pModelData->nmbrOfMatrices);
    if (pAnimName != NULL) {
        pModel->SetAnimation(Animation::Create(anim, pModel->pMatrices));
        pModel->flags.bits.bHasAnimation = 1;
    } else {
        pModel->pAnimation = NULL;
        pModel->flags.bits.bHasAnimation = 0;
    }
    pModel->flags.bits.b0 = 0;
    pModel->renderType = -1;
    pModel->flags.bits.b3 = 0;
    pModel->flags.bits.b4 = 0;
    pModel->flags.bits.b5 = 0;
    pModel->colour.Set(1.0f, 1.0f, 1.0f);
    pModel->colour.w = 1.0f;
    int matrixIdx = 0;
    while (matrixIdx < pModel->pTemplate->pModelData->nmbrOfMatrices) {
        pModel->pMatrices[matrixIdx].SetIdentity();
        pModel->unkC[matrixIdx] = 1.0f;
        matrixIdx++;
    }
    memset(pModel->subobjectData, 0, pModelTemplate->pModelData->nmbrOfSubObjects);
    return pModel;
}

void Model::Destroy(void) {
    flags.bits.b0 = 3;
    if (!((u8)flags.flagData >> 5 & 1)) {
        return;
    }
    if (pAnimation != NULL) {
        pAnimation->Destroy();
        pAnimation = NULL;
        flags.bits.bHasAnimation = 0;
    }
}

void Model::Purge(void) {
    Model **ppModels = modelInstances.pPointers;
    while (*ppModels != NULL) {
        Model *pModel = *ppModels;
        if (pModel->flags.bits.b0 != 0 && --pModel->flags.bits.b0 == 0) {
            if ((*ppModels)->pTemplate != NULL && --(*ppModels)->pTemplate->referenceCount == 0) {
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
            *ppModels = modelInstances.GetUnkEntry();
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

// https://decomp.me/scratch/kkyBc
// clean this up when Animation is decomped
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
    for(int i = 0; i < pTemplate->pModelData->nmbrOfAnimNodes; i++) {
        pAnim->frames[i].pVector = &pTemplate->pModelData->unk14[i];
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
    arg2 = !arg2;
    int data = subobjectData[subObjectIndex] & ~1;
    subobjectData[subObjectIndex] = data | arg2;
}

void Model::EnableOnlySubObject(int subObjectIndex, bool arg2) {
    int i = 0;
    while (i < pTemplate->pModelData->nmbrOfSubObjects) {
        subobjectData[i] &= ~1;
        if (!(subObjectIndex - i) != arg2) {
            subobjectData[i] |= 1;
        }
        i++;
    }
}

void Model::SetInverseScaleValue(int idx, float arg2) {
    if (idx == -1) {
        for(int i = 0; i < pTemplate->pModelData->nmbrOfMatrices; i++) {
            unkC[i] = arg2;
        }
        return;
    }
    unkC[idx] = arg2;
}

extern "C" int strcmpi(char*, char*);

bool Model::RefPointExists(char* pRefPointName, int* arg2) {
    for(int i = 0; i < pTemplate->pModelData->nmbrOfRefPoints; i++) {
        if (strcmpi(pTemplate->pModelData->pRefPoints[i].pName, pRefPointName) == 0) {
            if (arg2 != NULL) {
                *arg2 = i;
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
        pOut->ApplyMatrix(pOut, pMatrices);
        return;
    }
    pOut->ApplyMatrix(pOut, pAnimation->GetNodeMatrix(pTemplate->pModelData->pRefPoints[refPointIndex].matrix1 - 1));
}

bool Model::SubObjectExists(char* pSubObjectName, int* arg2) {
    for(int i = 0; i < pTemplate->pModelData->nmbrOfSubObjects; i++) {
        if (strcmpi(pTemplate->pModelData->pSubObjects[i].pName, pSubObjectName) == 0) {
            if (arg2 != NULL) {
                *arg2 = i;
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
    ModelTemplate** ppTemplates = modelTemplates.pPointers;
    if (*ppTemplates != NULL) {
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
    for(i = 0; i < pData->nmbrOfSubObjects; i++) {
        Ptr_Fixup(&pData->pSubObjects[i].pName, pData);
        Ptr_Fixup(&pData->pSubObjects[i].pOptions, pData);
        Ptr_Fixup((char**)&pData->pSubObjects[i].pMaterials, pData);
        for(int j = 0; j < pData->pSubObjects[i].nmbrOfMaterials; j++) {
            Ptr_Fixup(&pData->pSubObjects[i].pMaterials[j].pMaterialName, pData);
            Ptr_Fixup((char**)&pData->pSubObjects[i].pMaterials[j].pStripData, pData);
            pData->pSubObjects[i].pMaterials[j].pMaterial = Material::Create(pData->pSubObjects[i].pMaterials[j].pMaterialName);
        }
    }
    for(i = 0; i < pData->nmbrOfRefPoints; i++) {
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

BoundingVolume* Model::GetBoundingVolume(int subObjectIndex) {
    if (subObjectIndex < 0) {
        return &pTemplate->pModelData->volume;
    }
    return &pTemplate->pModelData->pSubObjects[subObjectIndex].volume;
}

char* Model::GetName(void){
	return pTemplate->name;
}

void Model::SetAlphaLightIntensity(int subObjectIndex, float intensity) {
    if (subObjectIndex == -1) {
        for(int i = 0; i < pTemplate->pModelData->nmbrOfSubObjects; i++) {
            pTemplate->pModelData->pSubObjects[i].alphaLightIntensity = intensity;
        }
        return;
    }
    pTemplate->pModelData->pSubObjects[subObjectIndex].alphaLightIntensity = intensity;
}

void Model::SetRenderTypeOverride(int renderTypeOverride) {
	renderType = renderTypeOverride;
}

Material* Model::GetSubObjectMaterial(int subObjectIndex, int materialIndex) {
	return pTemplate->pModelData->pSubObjects[subObjectIndex].pMaterials[materialIndex].pMaterial;
}

template <typename ModelTemplate>
void PtrList<ModelTemplate>::Destroy(ModelTemplate* pTemplate) {
    ModelTemplate** ptrs = pPointers;
    while (*ptrs != NULL) {
        if (*ptrs == pTemplate) {
            *ptrs = *pPointers++;
            return;
        }
        ptrs++;
    }
}