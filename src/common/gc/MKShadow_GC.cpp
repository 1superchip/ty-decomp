#include "common/MKShadow.h"
#include "common/System_GC.h"
#include "common/Heap.h"

static MKShadowDetail sd[MKSHADOW_DETAIL_COUNT] = {
    {256, 256, NULL, NULL, {}},
    {128, 128, NULL, NULL, {}},
    {64, 64, NULL, NULL, {}}
};

static uint* pHashTable = NULL;
static int* pEdgeTable = NULL;
static int nmbrOfEdges = 0;
static Vector* pWVerts = NULL;

static float metreScale = 100.0f;

static void MKShadow_ApplyAlphaTint(u8);
static void MKShadow_VolumeRenderStage1(void);
static void MKShadow_VolumeRenderStage2(void);

extern "C" void memset(void*, int, int);

void MKShadow_AddEdge(int a0, int a1) {
    uint hash = (a1 << 0x10) | a0;
    uint a0_hash = (a0 << 0x10) | a1;

    int i = (a0 * 4) & 1023;

    for (; pHashTable[i] != 0;) {
        if (pHashTable[i] == a0_hash) {
            pHashTable[i] = 0xFFFFFFFF;
            return;
        }
        i = (i + 1) & 1023;
    }

    i = (a1 * 4) & 1023;
    for (; pHashTable[i] != 0 && pHashTable[i] != 0xFFFFFFFF;) {
        i = (i + 1) & 1023;
    }
    pHashTable[i] = hash;
}

/// @brief Adds a new poly to this shadow
/// @param pVert0 Vertex 0 of the poly
/// @param pVert1 Vertex 1 of the poly
/// @param pVert2 Vertex 2 of the poly
/// @return Index of the new poly
int MKShadow_Animated::AddPoly(MKShadowVert* pVert0, MKShadowVert* pVert1, MKShadowVert* pVert2) {
    int v0_index = AddVert(pVert0);
    int v1_index = AddVert(pVert1);
    int v2_index = AddVert(pVert2);
    pPolys[numberOfPolys].vertex0_index = v0_index;
    pPolys[numberOfPolys].vertex1_index = v1_index;
    pPolys[numberOfPolys].vertex2_index = v2_index;
    int currPolyIndex = numberOfPolys; // return the current poly index
    numberOfPolys++; // increment the number of polys in this shadow
    return currPolyIndex;
}

/// @brief Adds a vertex to this shadow if not already in the shadow
/// @param pNewVert New Vertex to add
/// @return Index of the vertex to add
int MKShadow_Animated::AddVert(MKShadowVert* pNewVert) {
    // Search through all existing vertices
    for (int i = 0; i < numberOfVertices; i++) {
        if (pVerts[i].x == pNewVert->x && 
            pVerts[i].y == pNewVert->y &&
            pVerts[i].z == pNewVert->z &&
            pVerts[i].matrixIndex == pNewVert->matrixIndex) {
            // if the vertex exists, return the current index
            return i;
        }
    }
    // Add the new vertex to the array
    pVerts[numberOfVertices] = *pNewVert;
    int vertIndex = numberOfVertices;
    numberOfVertices++; // increment the number of vertices in this shadow
    return vertIndex; // return the index of the vertex added
}

void MKShadow_InitModule(void) {
    pWVerts = (Vector*)Heap_MemAlloc(sizeof(Vector) * 1024);
    pHashTable = (uint*)Heap_MemAlloc(sizeof(uint) * 1024);
    pEdgeTable = (int*)Heap_MemAlloc(sizeof(int) * 1024);
}

void MKShadow_DeinitModule(void) {
    for (int i = 0; i < ARRAY_SIZE(sd); i++) {
        sd[i].pMaterial0->Destroy();
        sd[i].pMaterial1->Destroy();
    }
    
    Heap_MemFree((void*)pWVerts);
    Heap_MemFree((void*)pHashTable);
    Heap_MemFree((void*)pEdgeTable);
}

/// @brief Unused function
/// @param scale New scale
void MKShadow_SetMetreScale(float scale) {
    metreScale = scale;
}

void MKShadow_DetectEdges(MKShadow* pShadow, Vector* pVec, Model* pModel, Vector* pVec1) {
    // Vertices after matrices have been applied
    static Vector tempVertices[1024];
    
    MKShadow_Animated* pShadowData = pShadow->mpAnimatedShadow;
    
    memset((void*)pHashTable, 0, sizeof(uint) * 1024);
    
    // Apply all matrices to the vertices
    for (int i = 0; i < pShadowData->numberOfVertices; i++) {
        tempVertices[i].ApplyMatrix((Vector*)&pShadowData->pVerts[i], &pModel->pMatrices[pShadowData->pVerts[i].matrixIndex]);
    }

    // Loop over all polys
    for (int i = 0; i < pShadowData->numberOfPolys; i++) {
        Vector sp28; // edge10?
        Vector sp18; // edge21?
        Vector sp8; // normal?

        MKShadowPoly* pPoly = &pShadowData->pPolys[i];

        // Vertex positions of this poly
        Vector* pVert0 = &tempVertices[pPoly->vertex0_index];
        Vector* pVert1 = &tempVertices[pPoly->vertex1_index];
        Vector* pVert2 = &tempVertices[pPoly->vertex2_index];

        // Calculate edges?
        sp28.Sub(pVert1, pVert0);
        sp18.Sub(pVert2, pVert1);

        // Calculate normal?
        sp8.Cross(&sp28, &sp18);
        
        Vector triCenter; // center of triangle
        Vector f;

        // Calculate triangle center
        triCenter.Add(pVert0, pVert1);
        triCenter.Add(pVert2);
        triCenter.Scale(1.0f / 3.0f);

        f.Sub(&triCenter, pVec1);
        if (f.Dot(&sp8) > 0.0f) {
            MKShadow_AddEdge(pPoly->vertex0_index, pPoly->vertex1_index);
            MKShadow_AddEdge(pPoly->vertex1_index, pPoly->vertex2_index);
            MKShadow_AddEdge(pPoly->vertex2_index, pPoly->vertex0_index);
            pWVerts[pPoly->vertex0_index] = *pVert0;
            pWVerts[pPoly->vertex1_index] = *pVert1;
            pWVerts[pPoly->vertex2_index] = *pVert2;
        }
    }

    nmbrOfEdges = 0;
    for (int i = 0; i < 1024; i++) {
        if (pHashTable[i] != 0 && pHashTable[i] != 0xFFFFFFFF) {
            pEdgeTable[nmbrOfEdges++] = pHashTable[i];
        }
    }
}

void MKShadow_Render(MKShadow* pShadow, Vector* pVec, Vector* pVec1, float f1, int, uint*, Vector*) {
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);

    Vector sp18;
    sp18.Sub(pVec, pVec1);
    sp18.Normalise();
    
    Vector sp3C;
    sp3C.Scale(&sp18, f1 + metreScale);
    
    Vector sp28;
    sp28 = *pVec;
    
    Vector sp30;
    sp30 = *pVec;
    
    sp3C.Add(&sp30);
    Vector sp8;

    if (nmbrOfEdges <= 0) {
        return;
    }
    sp8.Sub(&sp28, pVec1);

    float f14 = (1.0f / sp8.Magnitude()) * f1;

    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    Material::UseNone(-1);
    GXSetClipMode(GX_CLIP_DISABLE);
    MKShadow_VolumeRenderStage1();

    for (int i = 1; i < 3; i++) {
        if (i == 2) {
            MKShadow_VolumeRenderStage2();
        }

        for (int j = 0; j < nmbrOfEdges; j++) {

            Vector vert0;
            Vector vert0_tempScale;
            Vector vert1;
            Vector vert1_tempScale;
            Vector vert001;
            Vector vert111;
            Vector half0;
            Vector half1;

            vert0 = pWVerts[pEdgeTable[j] & 0xFFFF];
            vert0_tempScale.Sub(&vert0, pVec1);
            vert0_tempScale.Scale(f14);
            vert001.Add(&vert0_tempScale, &vert0);
            vert0_tempScale.Scale(0.5f);
            half0.Add(&vert0_tempScale, &vert0);
            
            vert1 = pWVerts[(pEdgeTable[j] >> 16) & 0xFFFF];
            vert1_tempScale.Sub(&vert1, pVec1);
            vert1_tempScale.Scale(f14);
            vert111.Add(&vert1_tempScale, &vert1);
            vert1_tempScale.Scale(0.5f);
            
            half1.Add(&vert1_tempScale, &vert1);
            
            if (gRenderState.fillState == 3) {
                GXBegin(GX_LINESTRIP, GX_VTXFMT1, 8);
            } else {
                GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 8);
            }

            GXPosition3f32(sp28.x, sp28.y, sp28.z);
            GXColor4u8(255, 255, 255, 4);

            GXPosition3f32(vert0.x, vert0.y, vert0.z);
            GXColor4u8(255, 255, 255, 4);

            GXPosition3f32(vert1.x, vert1.y, vert1.z);
            GXColor4u8(255, 255, 255, 4);

            GXPosition3f32(half0.x, half0.y, half0.z);
            GXColor4u8(255, 255, 255, 4);

            GXPosition3f32(half1.x, half1.y, half1.z);
            GXColor4u8(255, 255, 255, 4);

            GXPosition3f32(vert001.x, vert001.y, vert001.z);
            GXColor4u8(255, 255, 255, 4);

            GXPosition3f32(vert111.x, vert111.y, vert111.z);
            GXColor4u8(255, 255, 255, 4);

            GXPosition3f32(sp3C.x, sp3C.y, sp3C.z);
            GXColor4u8(255, 255, 255, 4);
        }
    }

    GXSetClipMode(GX_CLIP_ENABLE);
    GXSetCullMode(GX_CULL_NONE);
    Material::UseNone(-1);
}

void MKShadow_RenderStretchBlurEffect(MKShadow* pShadow, Vector* pVec, Vector* pVec1, float f1, int* pColor) {
    if (nmbrOfEdges <= 0) {
        return;
    }
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);
    Vector sp18;
    Vector sp8;
    sp18.Sub(pVec, pVec1);
    sp18.Normalise();
    sp8.Sub(pVec, pVec1);

    float f19 = (1.0f / sp8.Magnitude()) * f1;

    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    Material::UseNone(-1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_XOR);
    GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
    GXSetClipMode(GX_CLIP_DISABLE);

    for (int i = 0; i < nmbrOfEdges; i++) {
        
        Vector vert0;
        Vector vert01;
        Vector vert1;
        Vector vert11;
        
        vert0 = pWVerts[pEdgeTable[i] & 0xFFFF];
        vert01.Sub(&vert0, pVec1);
        vert01.Scale(f19);
        vert01.Add(&vert0);
        
        vert1 = pWVerts[(pEdgeTable[i] >> 16) & 0xFFFF];
        vert11.Sub(&vert1, pVec1);
        vert11.Scale(f19);
        vert11.Add(&vert1);
        
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
        
        GXPosition3f32(vert0.x, vert0.y, vert0.z);
        GXColor1u32(pColor[0]);
        
        GXPosition3f32(vert1.x, vert1.y, vert1.z);
        GXColor1u32(pColor[1]);
        
        GXPosition3f32(vert01.x, vert01.y, vert01.z);
        GXColor1u32(pColor[2]);
        
        GXPosition3f32(vert11.x, vert11.y, vert11.z);
        GXColor1u32(pColor[3]);
    }

    GXSetClipMode(GX_CLIP_ENABLE);
    Material::UseNone(-1);
}

void MKShadow_EndScene(void) {
    Material::UseNone(-1);
    GXSetColorUpdate(GX_DISABLE);
    GXSetBlendMode(GX_BM_LOGIC, GX_BL_ZERO, GX_BL_DSTALPHA, GX_LO_AND);
    GXSetDstAlpha(GX_DISABLE, 127);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    MKShadow_ApplyAlphaTint(128);
    GXSetColorUpdate(GX_ENABLE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTALPHA, GX_LO_XOR);
    GXSetDstAlpha(GX_ENABLE, 127);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    MKShadow_ApplyAlphaTint(255);
    Material::UseNone(-1);
}

void MKShadow_Animated::Build(char* pName) {

    Model* pModel = Model::Create(pName, NULL);
    ModelTemplate* pTemplate = pModel->pTemplate;
    
    pPolys = (MKShadowPoly*)Heap_MemAlloc(sizeof(MKShadowPoly) * MKSHADOW_POLY_COUNT);
    numberOfPolys = 0;

    pVerts = (MKShadowVert*)Heap_MemAlloc(sizeof(MKShadowVert) * MKSHADOW_POLY_COUNT);
    numberOfVertices = 0;
    
    for (int i = 0; i < pTemplate->pModelData->nmbrOfSubObjects; i++) {
        SubObject* pSub = &pTemplate->pModelData->pSubObjects[i];

        for (int j = 0; j < pSub->nmbrOfMaterials; j++) {
            SubObjectMaterial* pMat = &pSub->pMaterials[j];
            DisplayList* pDList = pMat->pStripData;

            while (pDList->primitive == GX_TRIANGLESTRIP &&
                    (u16*)pDList < (u16*)&((u8*)pMat->pStripData)[pMat->maxOffset * 16]) {
                u16 totalVertices = pDList->vertexCount;
                // skip over display list header
                (char*)pDList += sizeof(pDList->primitive) + sizeof(pDList->vertexCount);
                // now the display list pointer points to indices
                
                for (int k = 0; k < totalVertices - 2; k++) {
                    MKShadowVert verts[3];
                    u16* displayListIndices = (u16*)pDList;
                    Vertex* pModelVertices =
                        pModel->pTemplate->pModelData->pVertices;
                    int x = 0;
                    
                    for (int v = 0; v < 3; v++) {
                        verts[v].matrixIndex = pModelVertices[((u16*)pDList)[x]].matrix1;
                        verts[v].mNormal.Set(
                            pModelVertices[((u16*)pDList)[x + 1]].normal[0] / 64.0f,
                            pModelVertices[((u16*)pDList)[x + 1]].normal[1] / 64.0f,
                            pModelVertices[((u16*)pDList)[x + 1]].normal[2] / 64.0f
                        );
                        verts[v].x = pModelVertices[((u16*)pDList)[x]].pos[0];
                        verts[v].y = pModelVertices[((u16*)pDList)[x]].pos[1];
                        verts[v].z = pModelVertices[((u16*)pDList)[x]].pos[2];
                        x += 4;
                    }

                    Vector sp38;
                    sp38.Add(&verts[0].mNormal, &verts[1].mNormal);
                    sp38.Add(&verts[2].mNormal);

                    sp38.Normalise();

                    Vector computedNorm;

                    Vector diff0 = {
                        verts[2].x - verts[0].x,
                        verts[2].y - verts[0].y,
                        verts[2].z - verts[0].z
                    };

                    Vector diff1 = {
                        verts[1].x - verts[0].x,
                        verts[1].y - verts[0].y,
                        verts[1].z - verts[0].z
                    };

                    computedNorm.Cross(&diff0, &diff1);
                    computedNorm.Normalise();

                    if (computedNorm.Dot(&sp38) > 0.0f) {
                        AddPoly(&verts[0], &verts[1], &verts[2]);
                    } else {
                        AddPoly(&verts[2], &verts[1], &verts[0]);
                    }

                    (char*)pDList += 8;
                }

                (char*)pDList += 0x10;
            }
        }
    }
    
    pModel->Destroy();
}

/// @brief Creates a MKShadow object from a Model's name
/// @param pName Model name for Shadow
/// @return Pointer to new MKShadow object
MKShadow* MKShadow_CreateAnimatedFromModel(char* pName) {
    MKShadow* pShadow = (MKShadow*)Heap_MemAlloc(sizeof(MKShadow));
    MKShadow_Animated* pAnimated = (MKShadow_Animated*)Heap_MemAlloc(sizeof(MKShadow_Animated));
    pShadow->mType = MKSHADOW_ANIMATED;
    pShadow->mpAnimatedShadow = pAnimated;
    pShadow->mpAnimatedShadow->Build(pName);
    return pShadow;
}

/// @brief Destroys a MKShadow object
/// @param pShadow MKShadow to destroy
void MKShadow_Destroy(MKShadow* pShadow) {
    switch (pShadow->mType) {
        case MKSHADOW_ANIMATED:
            MKShadow_Animated* pAnimated = pShadow->mpAnimatedShadow;
            Heap_MemFree(pAnimated->pPolys);
            Heap_MemFree(pAnimated->pVerts);
            break;
        case MKSHADOW_STATIC:
            break;
    }
    Heap_MemFree(pShadow->mpAnimatedShadow);
    Heap_MemFree(pShadow);
}

static void MKShadow_ApplyAlphaTint(u8 color) {
    // Get old projection
    float projection[7];
    GXGetProjectionv((float*)&projection);

    // Create new projection
    float newProj[4][4];
    newProj[0][0] = 0.02f;
    newProj[0][1] = 0.0f;
    newProj[0][2] = 0.0f;
    newProj[0][3] = -1.0f;
    newProj[1][0] = 0.0f;
    newProj[1][1] = -0.02f;
    newProj[1][2] = 0.0f;
    newProj[1][3] = 1.0f;
    newProj[2][0] = 0.0f;
    newProj[2][1] = 0.0f;
    newProj[2][2] = 1.0f;
    newProj[2][3] = -1.0f;
    newProj[3][0] = 0.0f;
    newProj[3][1] = 0.0f;
    newProj[3][2] = 0.0f;
    newProj[3][3] = 1.0f;
    GXSetProjection(newProj, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(GX_PNMTX1);

    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);

    GXPosition3f32(0.0f, 0.0f, 0.0f);
    GXColor4u8(255, 255, 255, color);
    
    GXPosition3f32(0.0f, 100.0f, 0.0f);
    GXColor4u8(255, 255, 255, color);

    GXPosition3f32(100.0f, 0.0f, 0.0f);
    GXColor4u8(255, 255, 255, color);

    GXPosition3f32(100.0f, 100.0f, 0.0f);
    GXColor4u8(255, 255, 255, color);

    // Restore old projection
    GXSetProjectionv((float*)&projection);
    GXSetCurrentMtx(GX_PNMTX0);
}

static void MKShadow_VolumeRenderStage1(void) {
    GXSetCullMode(GX_CULL_FRONT);
    GXSetColorUpdate(GX_DISABLE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_XOR);
    GXSetDstAlpha(GX_DISABLE, 127);
    GXSetZMode(GX_TRUE, GX_GEQUAL, GX_FALSE);
}

static void MKShadow_VolumeRenderStage2(void) {
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_DISABLE);
    GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_XOR);
    GXSetDstAlpha(GX_DISABLE, 127);
    GXSetZMode(GX_TRUE, GX_GEQUAL, GX_FALSE);
}

void MKShadow_BeginScene(int) {

}

void MKShadow_CaptureZBuffer(void) {

}

void MKShadow_ClearBuffer(void) {

}

void MKShadow_BlitBuffer(int, int) {

}

static Vector MKShadow_GC_cpp_padding;
