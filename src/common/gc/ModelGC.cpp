#include "types.h"
#include "common/System_GC.h"
#include "common/Model.h"
#include "common/View.h"
#include "common/Heap.h"
#include "Dolphin/gx.h"

extern "C" void DCStoreRange(uint*, int);
extern GXColor Material_MixedColor;
void Grass_DrawGC(Model*, u8*, int, int, float, float);

static inline void Vector_ApplyMatrix(Vector* pOut, Vector* pVector, Matrix* pMatrix) {
    float temp_f10;
    float temp_f11;
    float temp_f13;

    temp_f10 = pVector->x;
    temp_f11 = pVector->y;
    temp_f13 = pVector->z;
    float mx = pMatrix->data[3][0] + ((temp_f13 * pMatrix->data[2][0]) + ((temp_f10 * pMatrix->data[0][0]) + (temp_f11 * pMatrix->data[1][0])));
    float my = pMatrix->data[3][1] + ((temp_f13 * pMatrix->data[2][1]) + ((temp_f10 * pMatrix->data[0][1]) + (temp_f11 * pMatrix->data[1][1])));
    float mz = pMatrix->data[3][2] + ((temp_f13 * pMatrix->data[2][2]) + ((temp_f10 * pMatrix->data[0][2]) + (temp_f11 * pMatrix->data[1][2])));

    pOut->x = mx;
    pOut->y = my;
    pOut->z = mz;
}

static EffectDat effectData[256];

int Model::disableTrivialRejection;
extern float Model_TrivialRejectTestMinW;
extern float Model_TrivialRejectTestMaxW;

// https://decomp.me/scratch/OReIW
// https://decomp.me/scratch/hBQXJ


int Model::Draw(u16* pSubObjs) {
    static float vertexBuffer[65536];
    static char normalBuffer[65536];
    Matrix mat0;
    Matrix mat1;
    View* currView = View::pCurrentView;
    int ret = 0;
    int r25;
    Matrix localMat;
    if (pSubObjs || flags.bits.b5 || disableTrivialRejection) {
        ret = true;
    } else {
        mat0.Multiply4x4(&matrices[0], &currView->unk1C8);
        ret = Model_TrivialRejectTest(&pTemplate->pModelData->volume, &mat0);
    }
    if (ret == 0) {
        return ret;
    }
    if (gRenderState.fillState == 2) {
        Material::UseNone(-1);
    }
    GXColor color = *(GXColor*)&currView->fogColour;
    GXSetFog(GX_FOG_PERSP_LIN, currView->closeFogPlane,
        (currView->farFogPlane - currView->closeFogPlane) * 2.0f + currView->closeFogPlane,
        currView->unk2C0, currView->unk2BC, (GXColor&)color);
    Vertex* pVerts = pTemplate->pModelData->pVertices;
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxDesc(GX_VA_NRM, GX_INDEX16);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetArray(GX_VA_CLR0, (void*)&pVerts->color, sizeof(Vertex));
    GXSetArray(GX_VA_TEX0, (void*)&pVerts->uv, sizeof(Vertex));
    if (pAnimation != NULL) {
        static int bufferOffset = 0;
        pAnimation->CalculateMatrices();
        int vertexCount = pTemplate->pModelData->vertexCount;
        r25 = vertexCount * 3;
        if (bufferOffset + r25 >= 65504) {
            bufferOffset = 0;
        }
        float* vertexBuf = &vertexBuffer[bufferOffset];
        char* normalBuf = &normalBuffer[bufferOffset];
        Vertex* verts = pVerts;
        int i = 0;
        while (vertexCount > 0) {
            Matrix* pVertMatrix = &pMatrices[pVerts[i].matrix1];
            Vector vertexPos;
            vertexPos.x = pVerts[i].pos[0];
            vertexPos.y = pVerts[i].pos[1];
            vertexPos.z = pVerts[i].pos[2];
            float m00 = pVertMatrix->data[0][0];
            float m01 = pVertMatrix->data[0][1];
            float m02 = pVertMatrix->data[0][2];
            float m10 = pVertMatrix->data[1][0];
            float m11 = pVertMatrix->data[1][1];
            float m12 = pVertMatrix->data[1][2];
            float m20 = pVertMatrix->data[2][0];
            float m21 = pVertMatrix->data[2][1];
            float m22 = pVertMatrix->data[2][2];
            vertexBuf[0] = (vertexPos.x * m00) + (vertexPos.y * m10) + (vertexPos.z * m20) + pVertMatrix->data[3][0];
            vertexBuf[1] = (vertexPos.x * m01) + (vertexPos.y * m11) + (vertexPos.z * m21) + pVertMatrix->data[3][1];
            vertexBuf[2] = (vertexPos.x * m02) + (vertexPos.y * m12) + (vertexPos.z * m22) + pVertMatrix->data[3][2];
            
            float nx = pVerts[i].normal[0] * (1.0f / 64.0f);
            float ny = pVerts[i].normal[1] * (1.0f / 64.0f);
            float nz = pVerts[i].normal[2] * (1.0f / 64.0f);
            Vector transformedNormal;
            // apply rotation
            float tnx = (nx * m00) + (ny * m10) + (nz * m20);
            float tny = (nx * m01) + (ny * m11) + (nz * m21);
            float tnz = (nx * m02) + (ny * m12) + (nz * m22);
            if (pVerts[i].weight != 4096) { // if weight doesn't equal 4096, apply it
                float weight = (float)pVerts[i].weight * (1.0f / 4095.99992f);
                Matrix* pMatrix = &pMatrices[pVerts[i].matrix2];
                float m00 = pMatrix->data[0][0];
                float m01 = pMatrix->data[0][1];
                float m02 = pMatrix->data[0][2];
                float m10 = pMatrix->data[1][0];
                float m11 = pMatrix->data[1][1];
                float m12 = pMatrix->data[1][2];
                float m20 = pMatrix->data[2][0];
                float m21 = pMatrix->data[2][1];
                float m22 = pMatrix->data[2][2];
                float m30 = pMatrix->data[3][0];
                float m31 = pMatrix->data[3][1];
                float m32 = pMatrix->data[3][2];
                float inv = 1.0f - weight;
                // apply weight to vertex position
                Vector av;
                Vector_ApplyMatrix(&av, &vertexPos, pMatrix);
                vertexBuf[0] = (weight * vertexBuf[0]) + inv * av.x;
                vertexBuf[1] = (weight * vertexBuf[1]) + inv * av.y;
                vertexBuf[2] = (weight * vertexBuf[2]) + inv * av.z;
                tnx = (weight * tnx) + (inv * ((nz * pMatrix->data[2][0]) + ((nx * pMatrix->data[0][0]) + (ny * pMatrix->data[1][0]))));
                tny = (weight * tny) + (inv * ((nz * pMatrix->data[2][1]) + ((nx * pMatrix->data[0][1]) + (ny * pMatrix->data[1][1]))));
                tnz = (weight * tnz) + (inv * ((nz * pMatrix->data[2][2]) + ((nx * pMatrix->data[0][2]) + (ny * pMatrix->data[1][2]))));
            }
            float mult = unkC[pVerts[i].matrix1];
            tnx *= mult;
            tny *= mult;
            tnz *= mult;
            normalBuf[0] = tnx * 64.0f;
            normalBuf[1] = tny * 64.0f;
            normalBuf[2] = tnz * 64.0f;
            i++;
            normalBuf += 3;
            vertexBuf += 3;
            vertexCount--;
        }
        GXSetArray(GX_VA_POS, (void*)&vertexBuffer[bufferOffset], sizeof(float) * 3);
        GXSetArray(GX_VA_NRM, (void*)&normalBuffer[bufferOffset], sizeof(char) * 3);
        DCStoreRange((uint*)&normalBuffer[bufferOffset], r25);
        DCStoreRange((uint*)&vertexBuffer[bufferOffset], r25 * sizeof(float));
        Matrix invMatrix = View::GetCurrent()->unkC8;
        if (View::GetCurrent()->unk288) {
            invMatrix.Multiply(&invMatrix, &View::GetCurrent()->unk248);
        }
        invMatrix.data[0][2] *= -1.0f;
        invMatrix.data[1][2] *= -1.0f;
        invMatrix.data[2][2] *= -1.0f;
        invMatrix.data[3][2] *= -1.0f;
        invMatrix.Transpose(&invMatrix);
        GXLoadPosMtxImm(invMatrix.data, GX_PNMTX0);
        GXLoadNrmMtxImm(invMatrix.data, GX_PNMTX0);
        bufferOffset = bufferOffset + r25;
        bufferOffset = (bufferOffset + 0x20) & ~0x1f;
    } else {
        Matrix mat1;
        View* pView = View::GetCurrent();
        if ((&matrices[0]) != NULL) {
            mat1.Multiply(&matrices[0], &pView->unkC8);
        } else {
            mat1 = pView->unkC8;
        }
        if (pView->unk288) {
            mat1.Multiply(&mat1, &pView->unk248);
        }
        mat1.data[0][2] *= -1.0f;
        mat1.data[1][2] *= -1.0f;
        mat1.data[2][2] *= -1.0f;
        mat1.data[3][2] *= -1.0f;
        mat1.Transpose(&mat1);
        GXLoadPosMtxImm(mat1.data, GX_PNMTX0);
        GXLoadNrmMtxImm(mat1.data, GX_PNMTX0);
        GXSetArray(GX_VA_POS, (void*)&pVerts->pos, sizeof(Vertex));
        GXSetArray(GX_VA_NRM, (void*)&pVerts->normal, sizeof(Vertex));
    }
    Matrix mat3;
    Matrix mat2;
    mat2.SetIdentity();
    mat2.Multiply3x3(&matrices[0], &View::GetCurrent()->unkC8);
    mat3.Row0()->Set(0.5f, 0.0f, 0.0f, 0.0f);
    mat3.data[1][0] = 0.0f;
    mat3.data[1][1] = -0.5f;
    mat3.data[1][2] = 0.0f;
    mat3.data[1][3] = 0.0f;
    mat3.data[2][0] = 0.0f;
    mat3.data[2][1] = 0.0f;
    mat3.data[2][2] = 1.0f;
    mat3.data[2][3] = 0.0f;
    mat3.data[3][0] = 0.5f;
    mat3.data[3][1] = 0.5f;
    mat3.data[3][2] = 0.0f;
    mat3.data[3][3] = 1.0f;
    mat2.Multiply(&mat2, &mat3);
    mat2.Transpose(&mat2);
    GXLoadTexMtxImm(mat2.data, 0x39, GX_MTX2x4);
    GXColor chanColor = (GXColor){255, 255, 255, 255};
    int matrixIdx = 0;
    GXSetChanMatColor(GX_COLOR0A0, chanColor);
    int effectIdx = 0;
    int size = (pSubObjs != NULL) ? *pSubObjs++ : pTemplate->pModelData->nmbrOfSubObjects; // get number of subobjects to run through
    while (size-- != 0) {
        EffectDat* pEffect = effectData;
        int subobjectIndex;
        if (pSubObjs != NULL) {
            subobjectIndex = *pSubObjs++ & 0x7FFF; // if subObject array isn't null, get the current index
        } else {
            subobjectIndex = pTemplate->pModelData->nmbrOfSubObjects - (size + 1);
        }
        SubObject* pSubObj = &pTemplate->pModelData->pSubObjects[subobjectIndex];
        if (!(subobjectData[subobjectIndex] & 1)) {
            if (pAnimation == NULL && (pSubObj->matrixIndex != matrixIdx)) {
                matrixIdx = pSubObj->matrixIndex;
                if (matrixIdx > 0) {
                    Matrix* pMatrix = &pMatrices[pSubObj->matrixIndex];
                    localMat.Multiply(pMatrix, &matrices[0]);
                    Matrix localMatrix;
                    View* pView = View::GetCurrent();
                    localMatrix.Multiply(&localMat, &pView->unkC8);
                    if (pView->unk288) {
                        localMatrix.Multiply(&localMatrix, &pView->unk248);
                    }
                    localMatrix.data[0][2] *= -1.0f;
                    localMatrix.data[1][2] *= -1.0f;
                    localMatrix.data[2][2] *= -1.0f;
                    localMatrix.data[3][2] *= -1.0f;
                    localMatrix.Transpose(&localMatrix);
                    GXLoadPosMtxImm(localMatrix.data, GX_PNMTX0);
                    GXLoadNrmMtxImm(localMatrix.data, GX_PNMTX0);
                } else {
                    Matrix mat1;
                    View* pView = View::GetCurrent();
                    if ((&matrices[0]) != NULL) {
                        mat1.Multiply(&matrices[0], &pView->unkC8);
                    } else {
                        mat1 = pView->unkC8;
                    }
                    if (pView->unk288) {
                        mat1.Multiply(&mat1, &pView->unk248);
                    }
                    mat1.data[0][2] *= -1.0f;
                    mat1.data[1][2] *= -1.0f;
                    mat1.data[2][2] *= -1.0f;
                    mat1.data[3][2] *= -1.0f;
                    mat1.Transpose(&mat1);
                    GXLoadPosMtxImm(mat1.data, GX_PNMTX0);
                    GXLoadNrmMtxImm(mat1.data, GX_PNMTX0);
                }
            }
            EffectDat* effect = pEffect;
			// iterate through all subobject materials
            for(int i = 0; i < pSubObj->nmbrOfMaterials; i++) {
                SubObjectMaterial* pObjMaterial = &pSubObj->pMaterials[i];
                Material* pDefMat = gRenderState.pDefaultMaterial;
                Material* pMat;
                if (pDefMat != NULL) {
                    pMat = pDefMat;
                } else {
                    pMat = pObjMaterial->pMaterial;
                }
				// if material isn't invisible, call display lists of the material
                if (!(pMat->flags & Flag_Invisible)) {
                    switch (pMat->grass) {
                        case 0:
                            break;
                        case 1:
                            void* pStrip = pObjMaterial->pStripData;
                            effectData[effectIdx].pStripData = pStrip;
                            effectData[effectIdx].maxOffset = pObjMaterial->maxOffset * 16;
                            effectData[effectIdx].matEffect = pMat->effect;
                            effectData[effectIdx].minW = Model_TrivialRejectTestMinW;
                            effectData[effectIdx].maxW = Model_TrivialRejectTestMaxW;
                            effectIdx++;
                            break;
                        }
						// iterate through all material overlays of the current material
                        while (pMat != NULL) {
                            int red = (int)(colour.x * pMat->color.x * 255.0f);
                            int green = (int)(colour.y * pMat->color.y * 255.0f);
                            int blue = (int)(colour.z * pMat->color.z * 255.0f);
                            int alpha = (int)(colour.w * pMat->color.w * 255.0f);
                            if (red < 0) {
                                red = 0;
                            } else if (red > 0xff) {
                                red = 0xff;
                            }
                            if (green < 0) {
                                green = 0;
                            } else if (green > 0xff) {
                                green = 0xff;
                            }
                            if (blue < 0) {
                                blue = 0;
                            } else if (blue > 0xff) {
                                blue = 0xff;
                            }
                            if (alpha < 0) {
                                alpha = 0;
                            } else if (alpha > 0xff) {
                                alpha = 0xff;
                            }
                            *(int*)&Material_MixedColor = (red << 24) | (green << 16) | (blue << 8) | alpha;
                            int r15;
                            if (renderType == -1) {
                                r15 = pMat->type;
                            } else if (renderType == 3 && pMat->type == 2) {
                                r15 = 2;
                            } else {
                                r15 = renderType;
                            }
                            if (gRenderState.fillState != 2) {
                                pMat->Use();
                                GXSetChanCtrl(GX_COLOR0A0, (r15 == false ? 1 : 0), GX_SRC_REG, GX_SRC_VTX, 7, GX_DF_CLAMP, GX_AF_NONE);
                                if (renderType == 7) {
                                    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
                                    GXSetNumTevStages(2);
    
                                    float red = colour.x * pSubObj->alphaLightIntensity * 255.0f;
                                    red = (red < 0.0f) ? 0.0f : (red > 255.0f) ? 255.0f : red;
                                    float green = colour.y * pSubObj->alphaLightIntensity * 255.0f;
                                    green = (green < 0.0f) ? 0.0f : (green > 255.0f) ? 255.0f : green;
                                    float blue = colour.z * pSubObj->alphaLightIntensity * 255.0f;
                                    blue = (blue < 0.0f) ? 0.0f : (blue > 255.0f) ? 255.0f : blue;

                                    GXColor tevColor = (GXColor){0, 0, 0, 255};

                                    tevColor.r = red;
                                    tevColor.g = green;
                                    tevColor.b = blue;
                                    GXSetTevColor(GX_TEVREG0, tevColor);
                                    // setup Tev stage 0
                                    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_C0, GX_CC_ZERO, GX_CC_RASA, GX_CC_RASC);
                                    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
                                    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
                                    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
                                    // setup Tev stage 1
                                    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
                                    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
                                    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
                                    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    
                                    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
                                }
                            }
                            GXCallDisplayList((void*)pObjMaterial->pStripData, pObjMaterial->maxOffset * 16);
                            Material* pTmp = (pMat != pMat->pOverlayMat) ? pMat->pOverlayMat : NULL;
                            pMat = pTmp;
                        }
                }
            }
        }
    }
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    *(int*)&Material_MixedColor = -1;
    if (effectIdx != 0) {
        while (effectIdx-- > 0) {
            EffectDat* pEffect = &effectData[effectIdx];
            Grass_DrawGC(this, (u8*)pEffect->pStripData, pEffect->maxOffset,
                pEffect->matEffect, pEffect->minW, pEffect->maxW);
        }
    }
    GXSetFog(GX_FOG_NONE, 0.0f, 0.0f, 0.0f, 0.0f, *(GXColor*)&currView->fogColour);
    return ret;
}

// these floats are placed in .sbss after a static function variable in Model::Draw
float Model_TrivialRejectTestMinW;
float Model_TrivialRejectTestMaxW;

int Model_TrivialRejectTest(BoundingVolume* pVolume, Matrix* pMatrix) {
    float corner[2][4];
    corner[0][0] = pVolume->v1.x;
    corner[0][1] = pVolume->v1.y;
    corner[0][2] = pVolume->v1.z;
    corner[1][0] = pVolume->v1.x + pVolume->v2.x;
    corner[1][1] = pVolume->v1.y + pVolume->v2.y;
    corner[1][2] = pVolume->v1.z + pVolume->v2.z;
    int i;
    int cond = 0x3f;
    int bits = 0;
    float diff = View::GetCurrent()->unk2BC - View::GetCurrent()->unk2C0;
    float min = 1e+06f;
    float max = 0.0f;

    // @bug shouldn't these be loading data[x][2] not data[x][3]?
    float m02 = pMatrix->data[0][3];
    float m12 = pMatrix->data[1][3];
    float m22 = pMatrix->data[2][3];
    float m32 = pMatrix->data[3][3];

    float m00 = pMatrix->data[0][0];
    float m10 = pMatrix->data[1][0];
    float m20 = pMatrix->data[2][0];
    float m30 = pMatrix->data[3][0];
    float m01 = pMatrix->data[0][1];
    float m11 = pMatrix->data[1][1];
    float m21 = pMatrix->data[2][1];
    float m31 = pMatrix->data[3][1];
    for (i = 0; i < 8; i++) {
        float tx = corner[i & 1][0];
        float ty = corner[(i >> 1) & 1][1];
        float tz = corner[(i >> 2) & 1][2];

        float z = tx * m02 + ty * m12 + tz * m22 + m32;
        
        float x = tx * m00 + ty * m10 + tz * m20 + m30;
        float y = tx * m01 + ty * m11 + tz * m21 + m31;
        
        int c = z < 0.0f;
        c |= (z > diff) << 1;
        c |= (x < -z) << 2;
        c |= (y < -z) << 3;
        c |= (x > z) << 4;
        c |= (y > z) << 5;
        
        bits |= c;
        cond &= c;
        
        if (z < min) {
            min = z;
        }
        if (z > max) {
            max = z;
        }
    }

    if (cond != 0) {
        return 0;
    }
    return (bits != 0) ? -1 : 1;
}

// optional parameters to get number of vertices, triangles, and strips in the model
// pass NULL if these values are not needed
ModelExplorer_GC* Model::Explore(int* pVertCount, int* pTriangleCount, int* pStripCount) {
    ModelExplorer_GC* pExplorer = (ModelExplorer_GC*)Heap_MemAlloc(sizeof(ModelExplorer_GC));
    pExplorer->pModel = this;
    pExplorer->subObjectIdx = 0;
    pExplorer->materialIdx = 0;
    pExplorer->PrimeMaterial();
    if (pVertCount || pTriangleCount || pStripCount) {
        int numVertices = pTemplate->pModelData->vertexCount;
        int triangleCount = 0;
        int stripCount = 0;
        for(int i = 0; i < pTemplate->pModelData->nmbrOfSubObjects; i++) {
            SubObject* pSubobj = &pTemplate->pModelData->pSubObjects[i];
            for(int j = 0; j < pSubobj->nmbrOfMaterials; j++) {
                SubObjectMaterial* pObjMaterial = &pSubobj->pMaterials[j];
                u8* pStrip = (u8*)pObjMaterial->pStripData;
                for(int stripIdx = 0; stripIdx < pObjMaterial->nmbrOfStrips; stripIdx++) {
					// code does not check primitive
					// not designed to support anything but triangle strips within display lists
                    DisplayList *pDL = (DisplayList*)pStrip;
                    u16 num = pDL->vertexCount;
                    triangleCount += num - 1;
                    ++pStrip += (num * 8) + 1;
                    triangleCount--;
                    pStrip++;
                    stripCount++;
                }
            }
        }
        if (pVertCount != NULL) {
            *pVertCount = numVertices;
        }
        if (pTriangleCount != NULL) {
            *pTriangleCount = triangleCount;
        }
        if (pStripCount != NULL) {
            *pStripCount = stripCount;
        }
    }

    return pExplorer;
}


void ModelExplorer_GC::PrimeMaterial(void) {
    stripNum = 0;
    pStripData = pModel->pTemplate->pModelData->pSubObjects[subObjectIdx].pMaterials[materialIdx].pStripData;
    pMaterial = pModel->pTemplate->pModelData->pSubObjects[subObjectIdx].pMaterials[materialIdx].pMaterial;
    PrimeStrip();
}

void ModelExplorer_GC::PrimeStrip(void) {
    vertexCount = pStripData->vertexCount;
    currentVertex = 0;
    vertexIndices = &pStripData->vertexIndex;
    normalIndices = &pStripData->normalIndex;
    colorIndices = &pStripData->colorIndex;
    uvIndices = &pStripData->uvIndex;
    BuildVertex(0);
    BuildVertex(1);
    BuildVertex(2);
    unk8 = 0;
    triangleCount = vertexCount - 2; // triangle strip
    unk10 = 0;
}

void ModelExplorer_GC::BuildVertex(int vtxIndex) {
    Vertex* pVertices = pModel->pTemplate->pModelData->pVertices;
    vertices[vtxIndex].pos.x = pVertices[*vertexIndices].pos[0];
    vertices[vtxIndex].pos.y = pVertices[*vertexIndices].pos[1];
    vertices[vtxIndex].pos.z = pVertices[*vertexIndices].pos[2];
    vertices[vtxIndex].normal[0] = pVertices[*normalIndices].normal[2] / 64.0f; // bug? should use normal[0]?
    vertices[vtxIndex].normal[1] = pVertices[*normalIndices].normal[1] / 64.0f;
    vertices[vtxIndex].normal[2] = pVertices[*normalIndices].normal[2] / 64.0f;
    vertices[vtxIndex].color.x = pVertices[*colorIndices].color[0] / 255.0f;
    vertices[vtxIndex].color.y = pVertices[*colorIndices].color[1] / 255.0f;
    vertices[vtxIndex].color.z = pVertices[*colorIndices].color[2] / 255.0f;
    vertices[vtxIndex].color.w = pVertices[*colorIndices].color[3] / 255.0f;
    vertices[vtxIndex].uv[0] = pVertices[*uvIndices].uv[0];
    vertices[vtxIndex].uv[1] = pVertices[*uvIndices].uv[1];

    if (pModel->pTemplate->pModelData->pSubObjects[subObjectIdx].type == 0) {
        vertices[vtxIndex].subObjectMatrixIdx = pModel->GetSubObjectMatrixIndex(subObjectIdx);
        vertices[vtxIndex].weights[0] = 1.0f;
        vertices[vtxIndex].matrix2 = 0;
        vertices[vtxIndex].weights[1] = 0.0f;
    } else {
        vertices[vtxIndex].subObjectMatrixIdx = pVertices[*vertexIndices].matrix1;
        vertices[vtxIndex].matrix2 = pVertices[*vertexIndices].matrix2;
        vertices[vtxIndex].weights[0] = (float)pVertices[*vertexIndices].weight / 4096.0f;
        vertices[vtxIndex].weights[1] = 1.0f - vertices[vtxIndex].weights[0];
    }
    vertexIndices += 4;
    normalIndices += 4;
    uvIndices += 4;
    colorIndices += 4;
    currentVertex++;
}

bool Model::ExploreNextFace(ModelExplorer* pExplorer) {
    if (pExplorer->currentVertex < pExplorer->vertexCount) {
        ModelExplorer_GC* explorerGC = (ModelExplorer_GC*)pExplorer;
        explorerGC->BuildVertex(pExplorer->currentVertex % 3);
        pExplorer->unk10 = pExplorer->unk10 == 0;
        pExplorer->unk8++;
    } else {
        int stripCount = 
        pTemplate->pModelData->pSubObjects[pExplorer->subObjectIdx].pMaterials[pExplorer->materialIdx].nmbrOfStrips;
        if (++pExplorer->stripNum < stripCount) {
            ModelExplorer_GC* explorerGC = (ModelExplorer_GC*)pExplorer;
            u8* pStrip = (u8*)pExplorer->pStripData;
            // get address of next strip
            pExplorer->pStripData = (DisplayList*)&pStrip[(pExplorer->vertexCount * 8) + 3];
            explorerGC->PrimeStrip();
        } else {
            return ExploreNextMaterial(pExplorer);
        }
    }
    return true;
}

bool Model::ExploreNextMaterial(ModelExplorer* pExplorer) {
	// no inheritance?
    ModelExplorer_GC* explorerGC = (ModelExplorer_GC*)pExplorer;
    int matCount = pTemplate->pModelData->pSubObjects[pExplorer->subObjectIdx].nmbrOfMaterials;
    if (++pExplorer->materialIdx < matCount) {
        explorerGC->PrimeMaterial();
    } else {
		return ExploreNextSubObject(pExplorer);
    }
    return 1;
}

bool Model::ExploreNextSubObject(ModelExplorer* pExplorer) {
    if (++pExplorer->subObjectIdx < pTemplate->pModelData->nmbrOfSubObjects) {
        ModelExplorer_GC* explorerGC = (ModelExplorer_GC*)pExplorer;
        pExplorer->materialIdx = 0;
        explorerGC->PrimeMaterial();
    } else {
        return 0;
    }
    return 1;
}

void Model::ExploreClose(ModelExplorer* pExplorer) {
    Heap_MemFree(pExplorer);
}
