
#include "common/ParticleSystemManager.h"
#include "common/System_GC.h"
#include "common/Heap.h"

Matrix ParticleSystem::identityMatrix = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

static ParticleEnvelope defaultEnvelope[2] = {
    {
        0.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    }, 
    {
        1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    }
};

bool ParticleSystem::bDrawBoundingBox = false;
bool ParticleSystem::bDrawPos = false;
bool ParticleSystem::bPaused = false;
int ParticleSystem::nextUniqueId = 0;

bool ParticleSystem::bDraw = true;
static int numDefaultEnvelopeNodes = 2;

void BaseParticleSystemType::Init(char* _pName, Material* pMat, float f1, float f2, float f3, int r6) {
    updateFunc = NULL;
    pName = _pName;
    unk8 = f2;
    unkC = f3;
    mpMat = pMat;
    unk14 = r6; // mNumTextures?
    unk18 = 1.0f / (float)r6;
    unk1C = f1;
    SetDistances(2000.0f, 5000.0f, 6000.0f);
    unk2C = 0;
    unk30 = 0;
}

void BaseParticleSystemType::SetDistances(float f1, float f2, float f3) {
    unk20 = f1;
    unk24 = f2;
    unk28 = f3;
    unk38 = unk20 * unk20;
    unk34 = unk24 * unk24;
    unk3C = 1.0f / unk34;
    unk40 = unk28 * unk28;
}

ParticleSystem* ParticleSystem::Create(ParticleSystemType* _pType, Vector* _pPos,
        BoundingVolume* pVolume, float volumeScale, int numDyn, ParticleSystemManager* pManager) {
    
    if (pManager == NULL) {
        pManager = &defaultParticleManager;
    }

    ParticleSystem* pSys = pManager->CreateParticleSystem(_pType);
    if (pSys) {
        pSys->Init(_pType, _pPos, pVolume, volumeScale, numDyn);
    }

    return pSys;
}

void ParticleSystem::Destroy(void) {
    Deinit();
    pSystemManager->DestroyParticleSystem(this);
}

void ParticleSystem::Destroy(float f1) {
    isDying = true;
    deathTimer = f1;
    invDeathTimer = 1.0f;
}

void ParticleSystem::Init(ParticleSystemType* _pType, Vector* pPos, BoundingVolume* pVolume,
        float volumeScale, int numDyn) {
    
    mUniqueId = ParticleSystem::nextUniqueId;
    ParticleSystem::nextUniqueId++;
    
    mpType = _pType;
    mpPos = pPos;
    
    if (pVolume) {
        mVolume = *pVolume;
        mVolume.v1.Scale(volumeScale);
        mVolume.v2.Scale(volumeScale);
    }

    mNumDyn = numDyn;

    if (mNumDyn == 1) {
        pDynamicData = &dynamicData;
    } else {
        pDynamicData = (ParticleSystem::DynamicData*)
            Heap_MemAlloc(mNumDyn * sizeof(ParticleSystem::DynamicData));
    }

    for (int i = 0; i < mNumDyn; i++) {
        pDynamicData[i].unk0 = 1.0f;
        pDynamicData[i].unk4 = 1.0f;
        pDynamicData[i].unk10 = false;
        pDynamicData[i].pMatrix = &ParticleSystem::identityMatrix;
        pDynamicData[i].unkC = 0.0f;
    }

    unk78 = -10.0f;

    if (_pType->unk2C & 2) {
        unk74 = true;
    } else {
        unk74 = false;
    }

    unk75 = false;
    age = 0.0f;
    unk30 = 1.0f;
    scale = 1.0f;
    unk38 = -1E+20f;
    unk3C = 0.0f;
    uOffset = 0.0f;
    vOffset = 0.0f;
    textureIndexOffset = 0.0f;
    isDying = false;
    deathTimer = 1.0f;
    invDeathTimer = 1.0f;
    numParticleChunks = 0;
    numLiveParticles = 0;
    mpChunks = NULL;
    unk40 = 0;
    _pType->unk38 = _pType->unk20 * _pType->unk20;
    _pType->unk34 = _pType->unk24 * _pType->unk24;
    _pType->unk3C = 1.0f / _pType->unk34;
    _pType->unk40 = _pType->unk28 * _pType->unk28;
}

void ParticleSystem::Deinit(void) {
    DestroyAll();
    if (mNumDyn > 1) {
        Heap_MemFree((void*)pDynamicData);
    }
}

/// @brief 
/// @param  None
/// @return Pointer to new Particle, NULL if none could be allocated
Particle* ParticleSystem::AllocateParticle(void) {
    // Iterate over the linked link of mpChunks first
    ParticleChunk* pChunk = mpChunks;
    while (pChunk) {
        if (!pChunk->IsFull()) {
            return pChunk->GetParticleData();
        }
        pChunk = pChunk->mpNext;
    }

    // If mpChunks is NULL or every check is full in the list, allocate a new ParticleChunk
    ParticleChunk* pNewChunk = pSystemManager->AllocParticleChunk();
    if (pNewChunk) {
        // if the new chunk isn't NULL, increment the number
        // of particle chunks and add it to the linked list of chunks in this system
        numParticleChunks++;
        pNewChunk->mpNext = mpChunks;
        mpChunks = pNewChunk;
        return pNewChunk->GetParticleData();
    }
    return (Particle*)NULL;
}

extern "C" void memset(void*, int, int);

/// @brief Allocates a particle from this system
/// @param  None
/// @return Pointer to new Particle struct, NULL if particle can't be created
Particle* ParticleSystem::CreateParticle(void) {
    if (!unk74) {
        return NULL;
    }

    if (bPaused) {
        return NULL;
    }

    if (!(mpType->unk2C & 0x100) && unk75) {
        return NULL;
    }

    Particle* p = AllocateParticle();
    if (p) {
        memset((void*)p, 0, sizeof(Particle));
        p->unkC = age;
        p->unk34 = mpType->unk14 - textureIndexOffset;
        p->unk30 = mpType->mpEnvelopes->unk8;
        p->mColor.Set(1.0f, 1.0f, 1.0f, mpType->mpEnvelopes->unk4);
        p->mEnvelopeIndex = 1;
        p->mX = mpPos->x;
        p->mY = mpPos->y;
        p->mZ = mpPos->z;
        p->unk50 = 1.0f;
        p->unk54 = 1.0f;
        p->unk4C = 1.0f;
        numLiveParticles++;
    }

    return p;
}

void ParticleSystem::DestroyParticle(Particle* pParticle, ParticleChunk** ppChunk) {
    (*ppChunk)->FreeParticle(pParticle);
    numLiveParticles--;
    if ((*ppChunk)->IsEmpty()) {
        numParticleChunks--;
        pSystemManager->FreeParticleChunk(*ppChunk);
        (*ppChunk) = (*ppChunk)->mpNext;
    }
}

void ParticleSystem::DestroyAll(void) {
    ParticleChunk* pCurrChunk = mpChunks;
    while (pCurrChunk) {
        pCurrChunk->mDataIndex = 24;
        pSystemManager->FreeParticleChunk(pCurrChunk);
        pCurrChunk = pCurrChunk->mpNext;
    }

    mpChunks = NULL;
    numLiveParticles = 0;
    numParticleChunks = 0;
}

bool ParticleSystem::CheckLiveness(void) {
    if (mpType->unk2C & 0x2) {
        unk74 = true;
        return true;
    }

    if (unk74) {
        if (unk78 < age - 1.0f) {
            DestroyAll();
            unk74 = false;
        }
    } else if (unk78 == age) {
        unk74 = true;
    }

    return unk74;
}

void ParticleSystem::Update(void) {
    float f31 = gDisplay.frameTime;
    
    if (bPaused) {
        return;
    }

    if (isDying) {
        deathTimer -= f31;
        if (deathTimer <= 0.0f) {
            Deinit();
            pSystemManager->DestroyParticleSystem(this);
            return;
        }

        invDeathTimer = invDeathTimer * (deathTimer / (deathTimer + f31));
    }

    if (!CheckLiveness()) {
        age += f31;
        return;
    }

    if ((mpType->unk2C & 0x100) || !unk75) {
        age += f31;
        if (numLiveParticles != 0) {
            mpType->updateFunc(this);
        }
    }
}

void ParticleSystem::Draw(void) {
    unk75 = true;
    bool bAnyVisible = false;
    
    for (int i = 0; i < mNumDyn; i++) {
        if (IsVisible(&pDynamicData[i])) {
            bAnyVisible = true;
        }
    }

    if (bAnyVisible) {
        if (!mpType->unk30) {
            mpType->unk30 = true;
            mpType->mpMat->Use();
        }

        DrawCPU();
    }
    
    if (!unk75 && (bDrawBoundingBox || bDrawPos)) {
        if (bDrawBoundingBox) {
            for (int i = 0; i < mNumDyn; i++) {
                if (pDynamicData[i].unkC > 0.0f && !pDynamicData[i].unk10) {
                    BoundingVolume localVol = mVolume;
                    localVol.v1.Add(mpPos);
                    localVol.v2.Add(&localVol.v1);
                    /// @bug Shouldn't this be comparing the indexed struct's matrix?
                    // Should this be (pDynamicData[i].pMatrix != &ParticleSystem::identityMatrix)?
                    if (pDynamicData->pMatrix != &ParticleSystem::identityMatrix) {
                        localVol.v1.ApplyMatrix(pDynamicData[i].pMatrix);
                        localVol.v2.ApplyMatrix(pDynamicData[i].pMatrix);
                    }
                }
            }
        }

        if (bDrawPos) {
            for (int i = 0; i < mNumDyn; i++) {
                if (pDynamicData[i].unkC > 0.0f && !pDynamicData[i].unk10) {
                    Vector dataPos = *mpPos;
                    /// @bug Shouldn't this be comparing the indexed struct's matrix?
                    // Should this be (pDynamicData[i].pMatrix != &ParticleSystem::identityMatrix)?
                    if (pDynamicData->pMatrix != &ParticleSystem::identityMatrix) {
                        dataPos.ApplyMatrix(pDynamicData[i].pMatrix);
                    }
                }
            }
        }

    }
}

// Not sure if these functions should be declared as inline or just defined in the header
// They have to be weak and it is probably easier to declare them as inline

inline void ParticleSystem::DrawCPU(void) {
    for (int i = 0; i < mNumDyn; i++) {
        if (!pDynamicData[i].unk10 && !(pDynamicData[i].unkC <= 0.0f) && 
            pDynamicData[i].unk0 != 0.0f) {
            ParticleChunk* pCurrChunk = mpChunks;
            while (pCurrChunk) {
                DrawCPUChunk(&pDynamicData[i], pCurrChunk);
                pCurrChunk = pCurrChunk->mpNext;
            }
        }
    }
    
    // Update System Manager debug fields
    pSystemManager->vu_numParticles += numLiveParticles;
    pSystemManager->vu_numChunks += numParticleChunks;
    pSystemManager->vu_numVirtualParticles += numLiveParticles * mNumDyn;
    pSystemManager->unk1C += numParticleChunks * mNumDyn;

    pSystemManager->minParticles = pSystemManager->minParticles < numLiveParticles ?
        pSystemManager->minParticles : numLiveParticles;
    
    pSystemManager->maxParticles = pSystemManager->maxParticles > numLiveParticles ?
        pSystemManager->maxParticles : numLiveParticles;
    
    pSystemManager->minChunks = pSystemManager->minChunks < numParticleChunks ?
        pSystemManager->minChunks : numParticleChunks;
    
    pSystemManager->maxChunks = pSystemManager->maxChunks > numParticleChunks ?
        pSystemManager->maxChunks : numParticleChunks;
    
    pSystemManager->avgParticles = (pSystemManager->avgParticles + numLiveParticles) * 0.5f;

    pSystemManager->avgChunks = (pSystemManager->avgChunks + numParticleChunks) * 0.5f;
}

inline void ParticleSystem::DrawCPUChunk(ParticleSystem::DynamicData* pDynamic, ParticleChunk* pChunk) {
    int r26 = mpType->unk14 - 1;
    View* pCurrView = View::GetCurrent();
    Vector* pRow0 = pCurrView->unk48.Row0();
    Vector* pRow1 = pCurrView->unk48.Row1();
    float f29, f28, f27, f26;
    float alpha0 = unk30 * (invDeathTimer * (pDynamic->unk0 * pDynamic->unkC));
    float f24 = pDynamic->unk4 * scale;
    
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);

    for (int i = pChunk->mDataIndex; i < MAX_PARTICLES; i++) {
        float f11 = ((int)(pChunk->mChunkData[i].unk34 + textureIndexOffset) & r26);
        f29 = (pChunk->mChunkData[i].unk40 + uOffset) + f11 * mpType->unk18;
        f28 = pChunk->mChunkData[i].unk44 + vOffset;
        f27 = f29 + mpType->unk18;
        f26 = f28 + 1.0f;
        
        float particle48 = pChunk->mChunkData[i].unk48;
        float particle4C = pChunk->mChunkData[i].unk4C;
        
        float fVar13 = (pRow1->x * particle4C - pRow0->x * particle48) * mpType->unkC;
        float fVar14 = (pRow1->y * particle4C - pRow0->y * particle48) * mpType->unkC;
        float fVar15 = (pRow1->z * particle4C - pRow0->z * particle48) * mpType->unkC;
        float fVar16 = (pRow1->x * particle48 + pRow0->x * particle4C) * mpType->unk8;
        float fVar17 = (pRow1->y * particle48 + pRow0->y * particle4C) * mpType->unk8;
        float fVar18 = (pRow1->z * particle48 + pRow0->z * particle4C) * mpType->unk8;
        
        float f0 = (pChunk->mChunkData[i].unk30 * pChunk->mChunkData[i].unk50) * f24;
        float chunkAlpha = pChunk->mChunkData[i].unk54 * pChunk->mChunkData[i].mColor.w;
        
        float f23 = (fVar13 - fVar16) * f0;
        float f22 = (fVar14 - fVar17) * f0;
        float f21 = (fVar15 - fVar18) * f0;
        float f20 = (fVar13 + fVar16) * f0;
        float f19 = (fVar14 + fVar17) * f0;
        float f18 = (fVar15 + fVar18) * f0;
        
        Vector pos = {
            pChunk->mChunkData[i].mX,
            pChunk->mChunkData[i].mY,
            pChunk->mChunkData[i].mZ,
        };

        float alpha = alpha0 * chunkAlpha;
        if (pDynamic->pMatrix != &ParticleSystem::identityMatrix) {
            pos.ApplyMatrix(pDynamic->pMatrix);
        }
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
        int r, g, b, a;

        if (mpType->mpMat->blendMode == Blend_Additive ||
            mpType->mpMat->blendMode == Blend_Subtractive) {
            r = (pChunk->mChunkData[i].mColor.x * alpha) * 255.0f;
            g = (pChunk->mChunkData[i].mColor.y * alpha) * 255.0f;
            b = (pChunk->mChunkData[i].mColor.z * alpha) * 255.0f;
            a = 255;
        } else {
            r = pChunk->mChunkData[i].mColor.x * 255.0f;
            g = pChunk->mChunkData[i].mColor.y * 255.0f;
            b = pChunk->mChunkData[i].mColor.z * 255.0f;
            a = alpha * 255.0f;
        }
        
        GXPosition3f32(pos.x + f23, pos.y + f22, pos.z + f21);
        GXColor4u8(r, g, b, a);
        GXTexCoord2f32(f29, f28);

        GXPosition3f32(pos.x - f20, pos.y - f19, pos.z - f18);
        GXColor4u8(r, g, b, a);
        GXTexCoord2f32(f29, f26);

        GXPosition3f32(pos.x + f20, pos.y + f19, pos.z + f18);
        GXColor4u8(r, g, b, a);
        GXTexCoord2f32(f27, f28);

        GXPosition3f32(pos.x - f23, pos.y - f22, pos.z - f21);
        GXColor4u8(r, g, b, a);
        GXTexCoord2f32(f27, f26);
    }
}

inline bool ParticleSystem::IsVisible(ParticleSystem::DynamicData* pDynamicData) {
    
    if ((mpType->unk2C & 4) && (mpType->unk2C & 8)) {
        pDynamicData->unkC = 1.0f;
        unk78 = age;
        pDynamicData->unk10 = false;
        unk75 = false;
        return true;
    }
    
    Vector dist;
    dist.x = (mpPos->x + pDynamicData->pMatrix->data[3][0])
        - View::GetCurrent()->unk48.data[3][0];
    dist.y = (mpPos->y + pDynamicData->pMatrix->data[3][1])
        - View::GetCurrent()->unk48.data[3][1];
    dist.z = (mpPos->z + pDynamicData->pMatrix->data[3][2])
        - View::GetCurrent()->unk48.data[3][2];
    float magSq = dist.x * dist.x + dist.y * dist.y + dist.z * dist.z;
    
    if (magSq < mpType->unk40) {
        unk78 = age;
    }

    float f2 = magSq - mpType->unk38 > 0.0f ? magSq - mpType->unk38 : 0.0f;
    pDynamicData->unkC = 1.0f - (mpType->unk3C * f2);
    if (!(mpType->unk2C & 4) && pDynamicData->unkC < 0.0f) {
        pSystemManager->numRejectedSystems++;
        pDynamicData->unk10 = true;
        unk75 = false;
        return false;
    }

    pDynamicData->unkC = pDynamicData->unkC > 0.0f ? pDynamicData->unkC : 0.0f;
    if (numLiveParticles == 0 || pDynamicData->unk0 == 0.0f) {
        pSystemManager->numRejectedSystems++;
        pDynamicData->unk10 = false;
        unk75 = false;
        return false;
    }

    if (!(mpType->unk2C & 4) && magSq > mpType->unk34) {
        pSystemManager->numRejectedSystems++;
        pDynamicData->unk10 = false;
        unk75 = false;
        return false;
    }

    if (!(mpType->unk2C & 8)) {
        BoundingVolume localVol = mVolume;
        localVol.v1.Add(mpPos);
        // Apply Translation if the DynamicData isn't using the
        // ParticleSystem identity matrix
        if (pDynamicData->pMatrix != &identityMatrix) {
            localVol.v1.Add(pDynamicData->pMatrix->Row3());
        }
        if (Model_TrivialRejectTest(&localVol, &View::GetCurrent()->unk148) == 0) {
            pSystemManager->numRejectedSystems++;
            pDynamicData->unk10 = true;
            return false;
        }
    }
    
    pDynamicData->unk10 = false;
    unk75 = false;
    return true;
}

void ParticleSystemType::Init(char* _pName, Material* pMat, float f1, float f2, float f3, int r6) {
    SimpleParticleSystemType::Init(_pName, pMat, f1, f2, f3, r6);
    updateFunc = ParticleSystemType::Update;
    unk58 = 0.0f;
    unk5C = 0.0f;
    unk60 = 1.0f;
    unk64 = 0.9999f;
    xVel = 0.0f;
    yVel = 0.0f;
    zVel = 0.0f;
}

void ParticleSystemType::Update(ParticleSystem* pSys) {
    float f31 = gDisplay.frameTime;
    
    pSys->uOffset += f31 * pSys->mpType->unk58;
    pSys->vOffset += f31 * pSys->mpType->unk5C;
    if (pSys->uOffset > 1.0f) {
        pSys->uOffset -= 1.0f;
    }
    if (pSys->vOffset > 1.0f) {
        pSys->vOffset -= 1.0f;
    }

    pSys->textureIndexOffset += f31 * pSys->mpType->unk60;
    if (pSys->textureIndexOffset > (float)pSys->mpType->unk14) {
        pSys->textureIndexOffset -= (float)pSys->mpType->unk14;
    }
    
    float f30 = pSys->age - pSys->mpType->unk1C;
    float f29 = f31 * pSys->mpType->xVel;
    float f28 = f31 * pSys->mpType->yVel;
    float f27 = f31 * pSys->mpType->zVel;
    
    float f26 = f31 * pSys->mpType->unk44;
    float f25 = f31 * pSys->mpType->unk48;
    float f24 = f31 * pSys->mpType->unk4C;

    float f23 = pSys->mpType->unk64;
    
    if (!pSys->mpType->InfiniteParticles()) {
        pSys->DestroyAllParticlesCreatedBefore(f30);
    }

    ParticleChunk* pCurrChunk = pSys->GetChunks();
    while (pCurrChunk) {
        Particle* pParticle = &pCurrChunk->mChunkData[pCurrChunk->mDataIndex];
        do {
            pParticle->mX += f29;
            pParticle->mX += pParticle->unk20 * f31;
            pParticle->unk20 *= f23;
            pParticle->unk20 += f26;
            
            pParticle->mY += f28;
            pParticle->mY += pParticle->unk24 * f31;
            pParticle->unk24 *= f23;
            pParticle->unk24 += f25;
            
            pParticle->mZ += f27;
            pParticle->mZ += pParticle->unk28 * f31;
            pParticle->unk28 *= f23;
            pParticle->unk28 += f24;

            pParticle->mAngle += f31 * pParticle->unk3C;

            if (pParticle->mY <= pSys->unk38) {
                if (pParticle->mY >= pSys->unk38 - pSys->unk3C) {
                    pParticle->mY = pSys->unk38;
                    pParticle->unk20 = 0.0f;
                    pParticle->unk24 = 0.0f;
                    pParticle->unk28 = 0.0f;
                } else {
                    pParticle->mY = pSys->unk38 - pParticle->mY + pSys->unk38;
                    pParticle->unk24 = (float)__fabs(pParticle->unk24) * 0.5f;
                }
            }
            
            float f3 = pSys->age - pParticle->unkC;
            if (f3 > pSys->mpType->mpEnvelopes[pParticle->mEnvelopeIndex].unkC) {
                pParticle->mEnvelopeIndex++;
            }
            
            int prevEnvelopeIndex = pParticle->mEnvelopeIndex - 1;
            float f4 = (f3 - pSys->mpType->mpEnvelopes[prevEnvelopeIndex].unkC) / 
                pSys->mpType->mpEnvelopes[prevEnvelopeIndex].deltaAge;
            
            pParticle->mColor.w = pSys->mpType->mpEnvelopes[prevEnvelopeIndex].unk4 + 
                (f4 * pSys->mpType->mpEnvelopes[prevEnvelopeIndex].unk14);
            
            pParticle->unk30 = pSys->mpType->mpEnvelopes[prevEnvelopeIndex].unk8 + 
                (f4 * pSys->mpType->mpEnvelopes[prevEnvelopeIndex].unk18);
            
            pParticle->unk48 = _table_sinf(pParticle->mAngle);
            pParticle->unk4C = _table_cosf(pParticle->mAngle);
            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);

        pCurrChunk = pCurrChunk->GetNext();
    }
}

void SimpleParticleSystemType::Init(char* _pName, Material* pMat, float f1, float f2, float f3, int r6) {
    BaseParticleSystemType::Init(_pName, pMat, f1, f2, f3, r6);
    updateFunc = SimpleParticleSystemType::Update;
    unk44 = 0.0f;
    unk48 = 0.0f;
    unk4C = 0.0f;
    mpEnvelopes = defaultEnvelope;
    mNumEnvelopes = numDefaultEnvelopeNodes;
    static bool bDefaultInit = false;
    if (!bDefaultInit) {
        bDefaultInit = true;
        CalculateEnvelope();
    }
}

void SimpleParticleSystemType::SetEnvelope(int numEnvelopes, ParticleEnvelope* pEnvelopes) {
    mNumEnvelopes = numEnvelopes;
    mpEnvelopes = pEnvelopes;
    CalculateEnvelope();
}

void SimpleParticleSystemType::CalculateEnvelope(void) {
    for (int i = 0; i < mNumEnvelopes; i++) {
        mpEnvelopes[i].unkC = mpEnvelopes[i].age * unk1C;
    }

    int max = mNumEnvelopes - 1;
    for (int i = 0; i < max; i++) {
        int nextEnvIndex = i + 1;
        mpEnvelopes[i].deltaAge = mpEnvelopes[nextEnvIndex].unkC - mpEnvelopes[i].unkC;
        mpEnvelopes[i].unk14 = mpEnvelopes[nextEnvIndex].unk4 - mpEnvelopes[i].unk4;
        mpEnvelopes[i].unk18 = mpEnvelopes[nextEnvIndex].unk8 - mpEnvelopes[i].unk8;
    }

    mpEnvelopes[max].deltaAge = 0.0f;
    mpEnvelopes[max].unk14 = 0.0f;
    mpEnvelopes[max].unk18 = 0.0f;
}

void SimpleParticleSystemType::Update(ParticleSystem* pSys) {

    float f31 = gDisplay.frameTime;
    float f30 = pSys->age - pSys->mpType->unk1C;
    float f29 = f31 * pSys->mpType->unk44;
    float f28 = f31 * pSys->mpType->unk48;
    float f27 = f31 * pSys->mpType->unk4C;

    if (!pSys->mpType->InfiniteParticles()) {
        pSys->DestroyAllParticlesCreatedBefore(f30);
    }

    ParticleChunk* pCurrChunk = pSys->GetChunks();
    while (pCurrChunk) {
        Particle* pParticle = &pCurrChunk->mChunkData[pCurrChunk->mDataIndex];
        do {
            pParticle->mX += pParticle->unk20 * f31;
            pParticle->unk20 += f29;
            pParticle->mY += pParticle->unk24 * f31;
            pParticle->unk24 += f28;
            pParticle->mZ += pParticle->unk28 * f31;
            pParticle->unk28 += f27;
            
            float f3 = pSys->age - pParticle->unkC;
            if (f3 > pSys->mpType->mpEnvelopes[pParticle->mEnvelopeIndex].unkC) {
                pParticle->mEnvelopeIndex++;
            }
            
            int prevEnvIndex = pParticle->mEnvelopeIndex - 1;
            float f4 = (f3 - pSys->mpType->mpEnvelopes[prevEnvIndex].unkC) / 
                pSys->mpType->mpEnvelopes[prevEnvIndex].deltaAge;
            
            pParticle->mColor.w = pSys->mpType->mpEnvelopes[prevEnvIndex].unk4 + 
                (f4 * pSys->mpType->mpEnvelopes[prevEnvIndex].unk14);
            
            pParticle->unk30 = pSys->mpType->mpEnvelopes[prevEnvIndex].unk8 + 
                (f4 * pSys->mpType->mpEnvelopes[prevEnvIndex].unk18);
            
            pParticle->unk48 = 0.0f;
            pParticle->unk4C = 1.0f;
            
            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);
        
        pCurrChunk = pCurrChunk->GetNext();
    }
}

/*
// Unused stripped debugging function:
// PrintVector is defined in Debug.h
void ParticleSystem::DrawDebug(float xPos, float* yPos) {
    gpDebugFont->DrawText(
        Str_Printf("pos: %s", PrintVector(mpPos)),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("volume: %s", PrintVector(&mVolume.v1)),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("        %s", PrintVector(&mVolume.v2)),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("numParticleChunks: %d", numParticleChunks),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("numLivePartices: %d", numLiveParticles),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("uOffset: %g, vOffset: %g", uOffset, vOffset),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("textureIndexOffset: %g", textureIndexOffset),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("isDying: %d", isDying),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("deathTimer: %d", deathTimer),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("invDeathTimer: %d", invDeathTimer),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("age: %g", age),
        xPos, *yPos, 1.5f, 1.5f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
}
*/
