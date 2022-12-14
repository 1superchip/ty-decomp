#include "types.h"
#include "common/MKSceneManager.h"
#include "common/View.h"
#include "common/Heap.h"
#include "common/StdMath.h"
#include "common/Model.h"

extern "C" void memset(void*, int, int);

static Vector occludeArray[160];
static u16 terrainSubObjects[1024];
static SMTree staticPropTree[4];
static MKProp staticPropArray[4];
static MKProp dynamicPropArray[4];
static MKProp globalPropArray[4];
Model* pTerrainModel[12];
float terrainDrawDist[12];

static int occlusionObjects;
MKPropDescriptor* MKPropDescriptor::pDrawListDescs;

void MKSceneManager::InitModule(void) {
	return;
}

void MKSceneManager::DeinitModule(void) {
	return;
}

void MKSceneManager::Init(MKSceneManagerInit* initInfo) {
    for(int i = 0; i < 12; i++) {
        pTerrainModel[i] = NULL;
    }
    for(int i = 0; i < 4; i++) {
        staticPropTree[i].pLastNode = NULL;
        trees[i].pLastNode = NULL;
        staticPropArray[i].pNext = &staticPropArray[i];
        dynamicPropArray[i].pNext = &dynamicPropArray[i];
        globalPropArray[i].pNext = &globalPropArray[i];
        staticPropArray[i].pPrev = &staticPropArray[i];
        dynamicPropArray[i].pPrev = &dynamicPropArray[i];
        globalPropArray[i].pPrev = &globalPropArray[i];
        staticPropArray[i].unk10 = 0;
        dynamicPropArray[i].unk10 = 0;
        globalPropArray[i].unk10 = 0;
    }
    props[0].pNextUpdated = &props[0];
    props[0].pPrevUpdated = &props[0];
    props[1].pNextUpdated = &props[1];
    props[1].pPrevUpdated = &props[1];
    propIdx = 0;
    bHasBeenOptimised = false;
    updateDistMult = initInfo->unk34;
    updateDrawMult = initInfo->unk38;
    matrix7C.SetIdentity();
}

void MKSceneManager::Deinit(void) {
    for(int i = 0; i < 4; i++) {
        staticPropTree[i].Deinit();
    }
    for(int i = 0; i < 12; i++) {
        trees[i].Deinit();
    }
}

void MKSceneManager::AddTerrainModel(Model* terrainModel, int layer, float drawDist) {
    pTerrainModel[layer] = terrainModel;
    terrainDrawDist[layer] = drawDist;
}

static void CalcBoundingBox(MKProp* pProp, BoundingVolume* pVolume) {
    Vector vecs[8];
    Vector* pVec = &vecs[1];
    MKPropDescriptor* pDesc = pProp->pDescriptor;
    
    vecs[0].x = pDesc->pVolume->v1.x;
    vecs[0].y = pDesc->pVolume->v1.y;
    vecs[0].z = pDesc->pVolume->v1.z;
    vecs[0].w = pDesc->pVolume->v1.w;
    
    vecs[1].x = pDesc->pVolume->v1.x;
    vecs[1].y = pDesc->pVolume->v1.y;
    vecs[1].z = pDesc->pVolume->v1.z;
    vecs[1].w = pDesc->pVolume->v1.w;

    vecs[1].x = vecs[0].x + pDesc->pVolume->v2.x;
    
    vecs[2].x = vecs[0].x;
    vecs[2].y = vecs[0].y;
    vecs[2].z = vecs[0].z;
    vecs[2].w = vecs[0].w;

    vecs[2].y = vecs[1].y + pDesc->pVolume->v2.y;

    vecs[3].x = vecs[0].x;
    vecs[3].y = vecs[0].y;
    vecs[3].z = vecs[0].z;
    vecs[3].w = vecs[0].w;

    vecs[3].z = vecs[0].z + pDesc->pVolume->v2.z;
    
    vecs[4].x = vecs[1].x;
    vecs[4].y = vecs[0].y;
    vecs[4].z = vecs[0].z;
    vecs[4].w = vecs[0].w;

    vecs[4].y = vecs[3].y + pDesc->pVolume->v2.y;
    
    vecs[5].x = pDesc->pVolume->v1.x + pDesc->pVolume->v2.x;
    vecs[5].y = vecs[0].y;
    vecs[5].w = pDesc->pVolume->v1.w;
    vecs[5].z = pDesc->pVolume->v1.z;
    
    vecs[5].z = vecs[2].z + pDesc->pVolume->v2.z;

    vecs[6].z = pDesc->pVolume->v1.z;
    vecs[7].z = vecs[2].z;

    vecs[6].x = vecs[0].x;
    vecs[6].y = vecs[2].y;
    vecs[6].w = vecs[0].w;
    vecs[6].z = vecs[2].z + pDesc->pVolume->v2.z;
    
    vecs[7].x = vecs[4].x;
    vecs[7].y = vecs[3].y + pDesc->pVolume->v2.y;
    vecs[7].w = vecs[2].w;
    vecs[7].z = vecs[2].z + pDesc->pVolume->v2.z;
    
    for(int i = 0; i < 8; i++) {
        vecs[i].ApplyMatrix(&vecs[i], pProp->pLocalToWorld);
    }

    Vector min;
    min.Set(vecs[0].x, vecs[0].y, vecs[0].z);
    Vector max;
    max.Set(vecs[0].x, vecs[0].y, vecs[0].z);
    for(int i = 1; i < 8; i++) {
        min.x = Min<float>(min.x, vecs[i].x);
        max.x = Max<float>(max.x, vecs[i].x);
        min.y = Min<float>(min.y, vecs[i].y);
        max.y = Max<float>(max.y, vecs[i].y);
        min.z = Min<float>(min.z, vecs[i].z);
        max.z = Max<float>(max.z, vecs[i].z);
    }
    pVolume->v1.x = min.x;
    pVolume->v1.y = min.y;
    pVolume->v1.z = min.z;
    pVolume->v2.x = max.x - min.x;
    pVolume->v2.y = max.y - min.y;
    pVolume->v2.z = max.z - min.z;
}


void MKSceneManager::AddProp(MKProp* pProp) {
    switch (pProp->pDescriptor->flags & MKPROP_TypeMask) {
        case MKPROP_Static:
            AddStaticProp(pProp, pProp->pDescriptor->drawLayer);
            break;
        case MKPROP_Dynamic:
            AddDynamicProp(pProp, pProp->pDescriptor->drawLayer);
            break;
        case MKPROP_Global:
            AddGlobalProp(pProp, pProp->pDescriptor->drawLayer);
            break;
    }
}

void MKSceneManager::AddStaticProp(MKProp* pProp, int propIdx) {
    pProp->pDescriptor->drawLayer = propIdx;
    pProp->pDescriptor->flags = pProp->pDescriptor->flags & ~MKPROP_TypeMask;
    MKPropDescriptor* pDesc = pProp->pDescriptor;
    pDesc->pNext = NULL;
    pDesc->unk24 = 0;
    if (pProp->pLocalToWorld == NULL || pProp->pDescriptor->pVolume == NULL) {
        AddGlobalProp(pProp, propIdx);
    } else {
        pProp->pNext = staticPropArray[propIdx].pNext;
        pProp->pPrev = &staticPropArray[propIdx];
        pProp->pNext->pPrev = pProp;
        pProp->pPrev->pNext = pProp;
        pProp->pPrevUpdated = NULL;
        pProp->pNextUpdated = NULL;
        staticPropArray[propIdx].unk10++;
    }
}

void MKSceneManager::AddDynamicProp(MKProp* pProp, int propIdx) {
    pProp->pDescriptor->drawLayer = propIdx;
	// clear all type bits and set the dynamic type bit
    pProp->pDescriptor->flags = (pProp->pDescriptor->flags & ~MKPROP_TypeMask) | MKPROP_Dynamic;
    MKPropDescriptor* pDesc = pProp->pDescriptor;
    pDesc->pNext = NULL;
    pDesc->unk24 = 0;
    pProp->pNext = dynamicPropArray[propIdx].pNext;
    pProp->pPrev = &dynamicPropArray[propIdx];
    pProp->pNext->pPrev = pProp;
    pProp->pPrev->pNext = pProp;
    pProp->pPrevUpdated = NULL;
    pProp->pNextUpdated = NULL;
    dynamicPropArray[propIdx].unk10++;
}

void MKSceneManager::AddGlobalProp(MKProp* pProp, int propIdx) {
    pProp->pDescriptor->drawLayer = propIdx;
     // clear all type bits and set the global type bit
    pProp->pDescriptor->flags = (pProp->pDescriptor->flags & ~MKPROP_TypeMask) | MKPROP_Global;
    MKPropDescriptor* pDesc = pProp->pDescriptor;
    pDesc->pNext = NULL;
    pDesc->unk24 = 0;
    pProp->pNext = globalPropArray[propIdx].pNext;
    pProp->pPrev = &globalPropArray[propIdx];
    pProp->pNext->pPrev = pProp;
    pProp->pPrev->pNext = pProp;
    pProp->pPrevUpdated = NULL;
    pProp->pNextUpdated = NULL;
    globalPropArray[propIdx].unk10++;
}

void MKSceneManager::Optimise(void) {
	MakeTerrainTree();
	MakePropTree();
	bHasBeenOptimised = true;
}

void SMTree::Deinit(void) {
	if (pNodes != NULL) {
		Heap_MemFree(pNodes);
		pNodes = NULL;
	}
}

static inline void Vector_Average(Vector* pOut, Vector* pIn, Vector* pIn2) {
    pOut->x = (pIn->x + 0.5f * pIn2->x);
    pOut->y = (pIn->y + 0.5f * pIn2->y);
    pOut->z = (pIn->z + 0.5f * pIn2->z);
}

static inline void Vector_Sub(Vector* pOut, Vector* pIn, Vector* pIn2) {
    pOut->x = (pIn2->x - pIn->x);
    pOut->y = (pIn2->y - pIn->y);
    pOut->z = (pIn2->z - pIn->z);
}

static inline void BoundingVolume_Center(BoundingVolume* volume, BoundingVolume* volume1, Vector* pOut) {
    pOut->x = (volume->v1.x + 0.5f * volume->v2.x);// - (volume1->v1.x + 0.5f * volume1->v2.x);
    pOut->y = (volume->v1.y + 0.5f * volume->v2.y);// - (volume1->v1.y + 0.5f * volume1->v2.y);
    pOut->z = (volume->v1.z + 0.5f * volume->v2.z);// - (volume1->v1.z + 0.5f * volume1->v2.z);
}

void SMTree::PairUp(int arg1, int arg2) {
    SMNode* pNode2;
    SMNode save1;
    SMNode* pSave;
    int idx = 0;
    while (idx < arg2 - 1) {
        int iVar8 = idx + 1;
        SMNode* pNode1 = &pNodes[arg1 + idx];
        SMNode* pClosest = pNode1;
        float centreX = pNode1->volume.v1.x + 0.5f * pNode1->volume.v2.x;
        while (iVar8 < arg2) {
            pNode2 = &pNodes[arg1 + iVar8];
            float centreX2 = pNode2->volume.v1.x + 0.5f * pNode2->volume.v2.x;
            if (centreX2 < centreX) {
                centreX = centreX2;
                pClosest = pNode2;
            }
            iVar8++;
        }
        pSave = NULL;
        iVar8 = idx + 1;
        save1 = *pNode1;
        *pNode1 = *pClosest;
        *pClosest = save1;
        float maxDist = 100000000000000.0f;
        Vector average;
        Vector_Average(&average, &pNode1->volume.v1, &pNode1->volume.v2);
        while (iVar8 < arg2) {
            pNode2 = &pNodes[arg1 + iVar8];
            Vector centre;
            Vector_Average(&centre, &pNode2->volume.v1, &pNode2->volume.v2);
            BoundingVolume_Center(&pNode2->volume, &pNode1->volume, &centre);
            Vector_Sub(&centre, &centre, &average);
            float mag = centre.MagSquared();
            if (mag < maxDist) {
                maxDist = mag;
                pSave = pNode2;
            }
            iVar8++;
        }
        idx += 2;
        save1 = *(pNode1 + 1);
        *(pNode1 + 1) = *pSave;
        *pSave = save1;
    }
}

void SMNode_ComputeVolume(SMNode* pThis, SMNode* pNode1, SMNode* pNode2) {
    float xMax = Max<float>(pNode2->volume.v1.x + pNode2->volume.v2.x, pNode1->volume.v1.x + pNode1->volume.v2.x);
    float yMax = Max<float>(pNode2->volume.v1.y + pNode2->volume.v2.y, pNode1->volume.v1.y + pNode1->volume.v2.y);
    float zMax = Max<float>(pNode2->volume.v1.z + pNode2->volume.v2.z, pNode1->volume.v1.z + pNode1->volume.v2.z);
    pThis->volume.v1.x = Min<float>(pNode2->volume.v1.x, pNode1->volume.v1.x);
    pThis->volume.v1.y = Min<float>(pNode2->volume.v1.y, pNode1->volume.v1.y);
    pThis->volume.v1.z = Min<float>(pNode2->volume.v1.z, pNode1->volume.v1.z);
    pThis->volume.v2.x = xMax - pThis->volume.v1.x;
    pThis->volume.v2.y = yMax - pThis->volume.v1.y;
    pThis->volume.v2.z = zMax - pThis->volume.v1.z;
}

inline void SMTree::LinkUpRow(int firstLeaf, int leafCount, int firstParent) {
    SMNode* pParent = &pNodes[firstParent];
    SMNode* node = &pNodes[firstLeaf];
    int i = 0;
    while (i < leafCount) {
        pParent->unk20[0] = node;
        pParent->unk20[1] = node + 1;
        SMNode_ComputeVolume(pParent, node, node + 1);
        pParent->drawDist = Max<float>(node->drawDist, (node + 1)->drawDist);
        node += 2;
        pParent++;
        i += 2;
    }
}

void SMTree::LinkUp(void) {
    if (pLastNode == NULL || nmbrOfSubObjects == 1) {
        return;
    }
    s32 subobjectCount = nmbrOfSubObjects;
    int depth = -1;
    int s = subobjectCount;
    while (s != 0) {
        s >>= 1;
        depth++;
    }
    if (subobjectCount & subobjectCount - 1) {
        PairUp(0, subobjectCount);
        LinkUpRow(0, propCount - (((1 << depth)) * 2 - 1), nmbrOfSubObjects);
    }
    while (depth != 0) {
        PairUp(propCount - (((1 << depth)) * 2 - 1), 1 << depth);
        LinkUpRow(propCount - (((1 << depth)) * 2 - 1), 1 << depth, propCount - (((1 << (depth - 1))) * 2 - 1));
        depth--;
    }
}

void MKSceneManager::MakeTerrainTree(void) {
    int index;
    SMNode* pNode;
    ModelData *pData;
    SubObject *pSubObject;
    for (int i = 0; i < 12; i++) {
        if (pTerrainModel[i] != NULL && pTerrainModel[i]->GetNmbrOfSubObjects() > 1) {
            int numSubObjects = pTerrainModel[i]->pTemplate->pModelData->nmbrOfSubObjects;
            if (numSubObjects == 0) {
                trees[i].pNodes = NULL;
                trees[i].pLastNode = NULL;
                trees[i].propCount = 0;
                trees[i].unkC = 0;
                trees[i].nmbrOfSubObjects = 0;
            } else {
                ModelData *pData = pTerrainModel[i]->pTemplate->pModelData;
                trees[i].nmbrOfSubObjects = numSubObjects;
                trees[i].unkC = numSubObjects - 1;
                trees[i].propCount = trees[i].unkC + numSubObjects;
                trees[i].pNodes = (SMNode *)Heap_MemAlloc(trees[i].propCount * sizeof(SMNode));
                trees[i].pLastNode = &trees[i].pNodes[trees[i].propCount - 1];
                memset((void *)trees[i].pNodes, 0, trees[i].propCount * sizeof(SMNode));
            }
            pNode = &trees[i].pNodes[0];
            pData = pTerrainModel[i]->pTemplate->pModelData;
            for (index = 0; index < pData->nmbrOfSubObjects; index++) {
                pSubObject = &pData->pSubObjects[index];
                pNode->pData = (void*)(index + 1);
                pNode->volume = pSubObject->volume;
                pNode->unk20[0] = NULL;
                pNode->unk20[1] = NULL;
                pNode->drawDist = terrainDrawDist[i];
                pNode++;
            }
            trees[i].LinkUp();
        } else {
            trees[i].pNodes = NULL;
            trees[i].pLastNode = NULL;
            trees[i].propCount = 0;
            trees[i].unkC = 0;
            trees[i].nmbrOfSubObjects = 0;
        }
    }
}

void MKSceneManager::MakePropTree(void) {
    for (int i = 0; i < 4; i++) {
        int count = staticPropArray[i].unk10;
        if (count == 0) {
            staticPropTree[i].pNodes = NULL;
            staticPropTree[i].pLastNode = NULL;
            staticPropTree[i].propCount = 0;
            staticPropTree[i].unkC = 0;
            staticPropTree[i].nmbrOfSubObjects = 0;
        } else {
            ModelData *pData = pTerrainModel[i]->pTemplate->pModelData;
            staticPropTree[i].nmbrOfSubObjects = count;
            staticPropTree[i].unkC = count - 1;
            int c = staticPropTree[i].unkC;
            staticPropTree[i].propCount = c + count;
            staticPropTree[i].pNodes = (SMNode *)Heap_MemAlloc(staticPropTree[i].propCount * sizeof(SMNode));
            staticPropTree[i].pLastNode = &staticPropTree[i].pNodes[staticPropTree[i].propCount - 1];
            memset((void *)staticPropTree[i].pNodes, 0, staticPropTree[i].propCount * sizeof(SMNode));
        }
        MKProp* next = staticPropArray[i].pNext;
        SMNode* pNode = staticPropTree[i].pNodes;
        if (next != NULL) {
            while (next != &staticPropArray[i]) {
                pNode->pData = (void*)next;
                CalcBoundingBox(next, &pNode->volume);
                pNode->unk20[0] = NULL;
                pNode->unk20[1] = NULL;
                pNode->drawDist = next->pDescriptor->maxDrawDist;
                next = next->pNext;
                pNode++;
            }
            staticPropTree[i].LinkUp();
        }
    }
}

static inline void Subtract(Vector* diff, Vector* arg1, Vector* arg0) {
    diff->x = arg1->x - arg0->x;
    diff->y = arg1->y - arg0->y;
    diff->z = arg1->z - arg0->z;
}

static inline void Multiply(Vector* vec, float t) {
    vec->x = t * vec->x;
    vec->y = t * vec->y;
    vec->z = t * vec->z;
}

void MKSceneManager::CalcZoneVis(void) {
    Vector vec;
    Vector vec1;
    Vector vec2;
    Vector vec3;
    Vector vec4;
    Vector vec5;
    Vector vec6;
    Vector vec7;
    View* pCurrView = View::GetCurrent();
    Vector* pVec2 = &vec2;
    Vector* pVec1 = &vec1;
    Vector* pVec = &vec;
    Vector* pVec6 = &vec6;
    Vector* pVec5 = &vec5;
    Vector* pVec4 = &vec4;
    int objectIdx = 0;

    while (objectIdx < occlusionObjects) {
        Vector* pOccludeVector = &occludeArray[objectIdx * 10];
        Subtract(&vec3, pOccludeVector + 1, pCurrView->unk48.Row3());
        Subtract(&vec2, pOccludeVector + 2, pCurrView->unk48.Row3());
        Subtract(&vec1, pOccludeVector + 3, pCurrView->unk48.Row3());
        Subtract(pVec, pOccludeVector + 4, pCurrView->unk48.Row3());
        Subtract(&vec7, pOccludeVector + 3, pOccludeVector + 1);
        Subtract(&vec6, pOccludeVector + 1, pOccludeVector + 2);
        Subtract(&vec5, pOccludeVector + 4, pOccludeVector + 3);
        Subtract(&vec4, pOccludeVector + 2, pOccludeVector + 4);
        (pOccludeVector + 5)->Cross(&vec3, &vec7);
        (pOccludeVector + 6)->Cross(pVec2, pVec6);
        (pOccludeVector + 7)->Cross(pVec1, pVec5);
        (pOccludeVector + 8)->Cross(pVec, pVec4);
        float dot = vec3.Dot(pOccludeVector);
        if (dot > 0.0f) {
            // operators?
            Multiply((pOccludeVector + 5), -1.0f);
            Multiply((pOccludeVector + 6), -1.0f); // Inverse
            Multiply((pOccludeVector + 7), -1.0f);
            Multiply((pOccludeVector + 8), -1.0f);
            (pOccludeVector + 9)->x = pOccludeVector->x;
            (pOccludeVector + 9)->y = pOccludeVector->y;
            (pOccludeVector + 9)->z = pOccludeVector->z;
            (pOccludeVector + 9)->w = pOccludeVector->w;
        } else {
            (pOccludeVector + 9)->x = pOccludeVector->x * -1.0f;
            (pOccludeVector + 9)->y = pOccludeVector->y * -1.0f;
            (pOccludeVector + 9)->z = pOccludeVector->z * -1.0f;
        }
        objectIdx++;
    }
}

void MKSceneManager::DrawTerrain(int arg1) {
    View::pCurrentView->SetLocalToWorldMatrix(NULL);
    if (trees[arg1].pLastNode != NULL) {
        terrainSubObjects[0] = 0;
        DrawRecursiveTerrain(trees[arg1].pLastNode, 0);
        terrainSubObjects[terrainSubObjects[0] + 1] = -1;
        pTerrainModel[arg1]->Draw(terrainSubObjects);
        return;
    }
    if (pTerrainModel[arg1] != NULL) {
        pTerrainModel[arg1]->Draw(NULL);
    }
}

void MKSceneManager::DrawRecursiveTerrain(SMNode *node, int arg2) {
    s16 test = 0;
    if (!(arg2 & 1)) {
        test = Model_TrivialRejectTest(&node->volume, &View::GetCurrent()->unk1C8);
        if (test == 0) {
            return;
        }
        if (test == 1) {
            arg2 |= 1;
        }
    }
    if (node->pData != NULL) {
        u16 object = terrainSubObjects[0];
        object = object + 1;
        terrainSubObjects[0] = object;
        terrainSubObjects[object] = ((int)node->pData) - 1 | ((test == 1) ? 0x8000 : 0);
        return;
    }
    if (node->unk20[0] != NULL) {
        DrawRecursiveTerrain(node->unk20[0], arg2);
    }
    if (node->unk20[1] != NULL) {
        DrawRecursiveTerrain(node->unk20[1], arg2);
    }
}

static void SMDrawProp(void* pData, int arg1, float distSq, float arg3);

void MKSceneManager::DrawRecursiveProps(SMNode *node, int arg2) {
    s16 test = 0;
    float distSq = 0.0f;
    MKProp *data = (MKProp *)node->pData;
    float fVar19 = 0.0f;
    if (node->pData != NULL) {
        Vector *pLTWTrans = data->pLocalToWorld->Row3();
        Vector *pViewTrans = View::GetCurrent()->unk48.Row3();
        distSq = Sqr<float>(pLTWTrans->x - pViewTrans->x) + Sqr<float>(pLTWTrans->y - pViewTrans->y) + Sqr<float>(pLTWTrans->z - pViewTrans->z);
        fVar19 = Sqr<float>(Max<float>(updateDistMult, data->pDescriptor->maxDrawDist * updateDrawMult));
        if (distSq >= fVar19) {
            return;
        }
    }
    if (!(arg2 & 1)) {
        test = Model_TrivialRejectTest(&node->volume, &View::GetCurrent()->unk1C8);
        if (test == 0) {
            return;
        }
        if (test == 1) {
            arg2 |= 1;
        }
    }
    if (node->pData != NULL) {
        SMDrawProp(node->pData, arg2, distSq, fVar19);
    } else {
        if (node->unk20[0] != NULL) {
            DrawRecursiveProps(node->unk20[0], arg2);
        }
        if (node->unk20[1] != NULL) {
            DrawRecursiveProps(node->unk20[1], arg2);
        }
    }
}

void MKSceneManager::DrawStaticProps(int arg1) {
    MKPropDescriptor::pDrawListDescs = NULL;
    MKProp* next;
    MKPropDescriptor* pDrawDescs = MKPropDescriptor::pDrawListDescs;
    if (staticPropTree[arg1].pLastNode != NULL) {
        DrawRecursiveProps(staticPropTree[arg1].pLastNode, 0);
    }
    pDrawDescs = MKPropDescriptor::pDrawListDescs;
    while (pDrawDescs != NULL) {
        next = pDrawDescs->pNext;
        while (next != NULL) {
            next->Draw();
            next = next->pNextOfThisType;
        }
        pDrawDescs->pNext = NULL;
        pDrawDescs = (MKPropDescriptor*)pDrawDescs->unk24;
    }
    MKPropDescriptor::pDrawListDescs = NULL;
}

static void SMDrawProp(void* pData, int rejectResult, float distSq, float arg3) {
    MKProp* pPropData = (MKProp*)pData;
    MKPropDescriptor* pDesc = pPropData->pDescriptor;
    float fVar2 = distSq / arg3;
    fVar2 = 8.0f * (1.0f - Sqr<float>(1.0f - fVar2));
    int detail_level = (int)fVar2;
	int _detail = (detail_level <= 8) ? detail_level : 8;
    pPropData->detailLevel = (char)_detail;
    if (pPropData->detailLevel != 8) {
        pPropData->unk14 = fVar2 - detail_level;
        pPropData->distSquared = distSq;
        pPropData->rejectionResult = rejectResult;
        float fVar3 = 1.0f;
        if (pPropData->detailLevel == 7) {
            fVar3 = fVar3 - pPropData->unk14;
        }
        pPropData->unk1C = fVar3;
        if (pDesc->pNext == NULL) {
            pDesc->unk24 = (int)MKPropDescriptor::pDrawListDescs;
            MKPropDescriptor::pDrawListDescs = pDesc;
        }
        pPropData->pNextOfThisType = pDesc->pNext;
        pDesc->pNext = pPropData;
    }
}

void MKSceneManager::DrawDynamicProps(int arg1) {
    MKPropDescriptor::pDrawListDescs = NULL;
    float fVar3;
    float fVar2;
    Matrix matrix;
    MKPropDescriptor *pDrawDescs;
    MKProp *next = dynamicPropArray[arg1].pNext;
    while (next != &dynamicPropArray[arg1]) {
        matrix.Multiply4x4(next->pLocalToWorld, &View::GetCurrent()->unk1C8);
        MKPropDescriptor *pDesc = next->pDescriptor;
        s16 rejectResult = Model_TrivialRejectTest(pDesc->pVolume, &matrix);
        float fVar19 = 0.0f;
        float distSq = 0.0f;
        pDesc = next->pDescriptor;
        if (rejectResult != 0) {
            Vector *pLTWTrans = next->pLocalToWorld->Row3();
            Vector *pViewTrans = View::GetCurrent()->unk48.Row3();
            distSq = Sqr<float>(pLTWTrans->x - pViewTrans->x) + Sqr<float>(pLTWTrans->y - pViewTrans->y) + Sqr<float>(pLTWTrans->z - pViewTrans->z);
            fVar19 = Sqr<float>(Max<float>(updateDistMult, pDesc->maxDrawDist * updateDrawMult));
            if (distSq < fVar19) {
                SMDrawProp((int*)next, rejectResult, distSq, fVar19);
            }
        }
        next = next->pNext;
    }
    pDrawDescs = MKPropDescriptor::pDrawListDescs;
    while (pDrawDescs != NULL) {
        next = pDrawDescs->pNext;
        while (next != NULL) {
            next->Draw();
            next = next->pNextOfThisType;
        }
        pDrawDescs->pNext = NULL;
        pDrawDescs = (MKPropDescriptor *)pDrawDescs->unk24;
    }
    MKPropDescriptor::pDrawListDescs = NULL;
}

void MKSceneManager::DrawGlobalProps(int arg1) {
    MKPropDescriptor* pDesc;
    MKPropDescriptor::pDrawListDescs = NULL;
    MKProp* next = globalPropArray[arg1].pNext;
    while (next != &globalPropArray[arg1]) {
        pDesc = next->pDescriptor;
        if (pDesc->pNext == NULL) {
            pDesc->unk24 = (int)MKPropDescriptor::pDrawListDescs;
            MKPropDescriptor::pDrawListDescs = pDesc;
        }
        next->pNextOfThisType = pDesc->pNext;
        pDesc->pNext = next;
        next = next->pNext;
    }
    MKPropDescriptor* pDrawDescs = MKPropDescriptor::pDrawListDescs;
    pDrawDescs = MKPropDescriptor::pDrawListDescs;
    while (pDrawDescs != NULL) {
        next = pDrawDescs->pNext;
        while (next != NULL) {
            next->Draw();
            next = next->pNextOfThisType;
        }
        pDrawDescs->pNext = NULL;
        pDrawDescs = (MKPropDescriptor*)pDrawDescs->unk24;
    }
    MKPropDescriptor::pDrawListDescs = NULL;
}

void MKSceneManager::UpdateProps(void) {
    int idx;
    SMNode* node;
    MKMessage message;
    message.unk0 = -3;
    for (int i = 0; i < 4; i++) {
        idx = 0;
        node = staticPropTree[i].pNodes;
        while (idx < staticPropTree[i].propCount) {
            MKProp *data = (MKProp *)node->pData;
            node++;
            if (data != NULL) {
                Vector *pLTWTrans = data->pLocalToWorld->Row3();
                Vector *pActivePoint = &activePoint;
                float dist = Sqr<float>(pLTWTrans->x - pActivePoint->x) + Sqr<float>(pLTWTrans->y - pActivePoint->y) + Sqr<float>(pLTWTrans->z - pActivePoint->z);
                data->distSquared = dist; // set distSquared to the distance of the prop from the active point
                if (data->distSquared < data->pDescriptor->maxUpdateDist * data->pDescriptor->maxUpdateDist) {
                    // if the dist is less than the prop's max update distance, update the prop
                    UpdateProp(data, &message);
                }
            }
            idx++;
        }
        MKProp *dynamicProp = dynamicPropArray[i].pNext;
        while (dynamicProp != &dynamicPropArray[i]) {
            Vector *pLTWTrans = dynamicProp->pLocalToWorld->Row3();
            Vector *pActivePoint = &activePoint;
            float dist = Sqr<float>(pLTWTrans->x - pActivePoint->x) + Sqr<float>(pLTWTrans->y - pActivePoint->y) + Sqr<float>(pLTWTrans->z - pActivePoint->z);
            dynamicProp->distSquared = dist;
            if (dynamicProp->distSquared < dynamicProp->pDescriptor->maxUpdateDist * dynamicProp->pDescriptor->maxUpdateDist) {
                // if the dist is less than the prop's max update distance, update the prop
                prop1BC = dynamicProp->pNext;
                UpdateProp(dynamicProp, &message);
                dynamicProp = prop1BC;
            } else {
                dynamicProp = dynamicProp->pNext;
            }
        }
        MKProp *pGlobalProp = globalPropArray[i].pNext;
        while (pGlobalProp != &globalPropArray[i]) {
            prop1BC = pGlobalProp->pNext;
            UpdateProp(pGlobalProp, &message);
            pGlobalProp = prop1BC;
        }
    }
    int propIndex = 1 - propIdx;
    propIdx = propIndex;
    MKProp *lastProp = &props[propIdx];
    message.unk0 = -4;
    MKProp *pUpdateProp = lastProp->pNextUpdated;
    while (pUpdateProp != lastProp) {
        MKProp *nextProp = pUpdateProp->pNextUpdated;
        pUpdateProp->pNextUpdated->pPrevUpdated = pUpdateProp->pPrevUpdated;
        pUpdateProp->pPrevUpdated->pNextUpdated = pUpdateProp->pNextUpdated;
        pUpdateProp->pNextUpdated = NULL;
        pUpdateProp->pPrevUpdated = NULL;
        pUpdateProp->Message(&message);
        pUpdateProp = nextProp;
    }
}

void MKSceneManager::SendMessage(MKMessage *pMessage, uint mask, bool arg2, Vector *pPos, float radius) {
    int i;
    int index;
    if (arg2 != false) {
        float radiusSq = radius * radius;
        for (i = 0; i < 4; i++) {
            SMNode *node = staticPropTree[i].pNodes;
            index = 0;
            while (index < staticPropTree[i].propCount) {
                MKProp *currProp = (MKProp *)node->pData;
                node++;
                if (currProp != NULL && (mask == 0 || mask & currProp->pDescriptor->searchMask)) {
                    if (pPos != NULL) {
                        Vector *pLTWTrans = currProp->pLocalToWorld->Row3();
                        float distSq = Sqr<float>(pLTWTrans->x - pPos->x) + Sqr<float>(pLTWTrans->y - pPos->y) + Sqr<float>(pLTWTrans->z - pPos->z);
                        if (distSq < radiusSq) {
                            currProp->Message(pMessage);
                        }
                    } else {
                        currProp->Message(pMessage);
                    }
                }
                index++;
            }
        }
    }
    float radiusSq = radius * radius;
    for (i = 0; i < 4; i++) {
        MKProp *dynamicProp = dynamicPropArray[i].pNext;
        while (dynamicProp != &dynamicPropArray[i]) {
            prop1C0 = dynamicProp->pNext;
            if (mask == 0 || mask & dynamicProp->pDescriptor->searchMask) { // if the mask is equal to zero, send a message
                if (pPos != NULL) {
                    Vector *pLTWTrans = dynamicProp->pLocalToWorld->Row3();
                    float distSq = Sqr<float>(pLTWTrans->x - pPos->x) + Sqr<float>(pLTWTrans->y - pPos->y) + Sqr<float>(pLTWTrans->z - pPos->z);
                    if (distSq < radiusSq) {
                        dynamicProp->Message(pMessage);
                    }
                } else {
                    dynamicProp->Message(pMessage);
                }
            }
            dynamicProp = prop1C0;
        }
    }
    for (i = 0; i < 4; i++) {
        MKProp *globalProp = globalPropArray[i].pNext;
        while (globalProp != &globalPropArray[i]) {
            prop1C0 = globalProp->pNext;
            if (mask == 0 || mask & globalProp->pDescriptor->searchMask) {
                globalProp->Message(pMessage);
            }
            globalProp = prop1C0;
        }
    }
}

void MKSceneManager::SetActivePoint(Vector* pPoint) {
    activePoint = *pPoint;
}

void MKSceneManager::UpdateProp(MKProp* pProp, MKMessage* pMessage) {
    MKProp* nextUpdated = pProp->pNextUpdated;
    if (nextUpdated == NULL) {
        pProp->Message(pMessage);
    } else {
        nextUpdated->pPrevUpdated = pProp->pPrevUpdated;
        pProp->pPrevUpdated->pNextUpdated = pProp->pNextUpdated;
    }
    MKProp* prop = &props[propIdx];
    pProp->pNextUpdated = prop->pNextUpdated;
    pProp->pPrevUpdated = prop;
    pProp->pNextUpdated->pPrevUpdated = pProp;
    prop = pProp->pPrevUpdated;
    prop->pNextUpdated = pProp;

    pProp->Update();
}

void MKSceneManager::RemoveProp(MKProp* pProp) {
    MKMessage message;
    if (prop1BC == pProp) {
        prop1BC = pProp->pNext;
    }
    if (prop1C0 == pProp) {
        prop1C0 = pProp->pNext;
    }
    pProp->pNext->pPrev = pProp->pPrev;
    pProp->pPrev->pNext = pProp->pNext;
    pProp->pPrev = NULL;
    pProp->pNext = NULL;
    if (pProp->pNextUpdated != NULL || pProp->pPrevUpdated != NULL) {
        message.unk0 = -4; // analyze these ids
        pProp->Message(&message);
        pProp->pNextUpdated->pPrevUpdated = pProp->pPrevUpdated;
        pProp->pPrevUpdated->pNextUpdated = pProp->pNextUpdated;
        pProp->pPrevUpdated = NULL;
        pProp->pNextUpdated = NULL;
    }
}

int MKSceneManager::GetPropsInRange(MKProp** ppProps, int maxCount, Vector* pTestPt, float radius, int searchMask, int param_7, bool bIncludeStatic) {
    int ret = 0;
    int iVar7 = 0;
    MKProp** props;
    MKProp* prop;
    for (int i = 0; i < 4; i++, iVar7++) {
        if (param_7 == -1 || param_7 == iVar7) {
            props = ppProps;
            for (prop = dynamicPropArray[i].pNext; prop != &dynamicPropArray[i] && ret < maxCount; prop = prop->pNext) {
                if (searchMask == -1 || prop->pDescriptor->searchMask & searchMask) {
                    Vector *pLTWTrans = prop->pLocalToWorld->Row3();
                    float distSq = Sqr<float>(pLTWTrans->x - pTestPt->x) + Sqr<float>(pLTWTrans->y - pTestPt->y) + Sqr<float>(pLTWTrans->z - pTestPt->z);
                    if (distSq < radius * radius) {
                        props[ret] = prop;
                        ret++;
                    }
                }
            }
        }
    }
    return ret;
}

bool MKProp::LoadLine(KromeIniLine* pLine) {
	return false;
}

void MKProp::LoadDone(void) {
	return;
}

void MKProp::Reset(void) {
	return;
}

void MKProp::Update(void) {
	return;
}

void MKProp::Draw(void) {
	return;
}

void MKProp::Message(MKMessage* pMsg) {
	return;
}