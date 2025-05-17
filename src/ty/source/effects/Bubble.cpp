#include "ty/effects/Bubble.h"
#include "ty/global.h"

ParticleSystemType Bubble::bubbleType;

Material* Bubble::pBubbleMat = NULL;

/// @brief Loads resources for the Bubble Particle
void Bubble_LoadResources(void) {
    Bubble::pBubbleMat = Material::Create("fx_002a");
    Bubble::bubbleType.Init("Bubble", Bubble::pBubbleMat, 2.0f, 1.0f, 1.0f, 1);
    Bubble::bubbleType.updateFunc = Bubble_CustomUpdate;
}

/// @brief Initiates this Bubble object by creating a ParticleSystem
/// @param pPos Position of the ParticleSystem
/// @param pVolume Volume of the ParticleSystem
/// @param f1 
void Bubble::Init(Vector* pPos, BoundingVolume* pVolume, float f1) {
    mpSystem = ParticleSystem::Create(&Bubble::bubbleType, pPos, pVolume, f1, 1, NULL);
}

/// @brief Destroys the ParticleSystem
void Bubble::Deinit(void) {
    if (mpSystem) {
        mpSystem->Destroy();
    }
}

void Bubble::Create(Vector* pPos, float f1, float f2, float f3, float f4) {
    Particle* p = mpSystem->CreateParticle();
    if (p) {
        p->mX = pPos->x;
        p->mY = pPos->y;
        p->mZ = pPos->z;
        p->unk30 = f1;
        p->unkC = RandomFR(&gb.mRandSeed, 0.0f, 1.0f);
        p->unk4C = 1.0f;
        p->unk48 = 0.0f;
        p->SetUnk58ArrayByIndex(0, f2 - f1);
        p->SetUnk58ArrayByIndex(1, f3);

        if (f4 <= 0.0f) {
            p->unk24 = Clamp<float>(1.0f, f1 * 0.3f, 15.0f) + RandomFR(&gb.mRandSeed, 0.0f, f1 * 0.1f);
        } else {
            p->unk24 = f4;
        }
    }
}

/// @brief Custom update function for the Bubble Particle System
/// @param pSys ParticleSystem to update
void Bubble_CustomUpdate(ParticleSystem* pSys) {
    ParticleChunk** ppChunks;

    if (!(pSys->mpType->unk2C & 1)) {
        // Test all particles to determine if they should be destroyed
        ppChunks = &pSys->mpChunks;
        do {
            ParticleChunk* pCurrChunk = *ppChunks;
            Particle* pParticle = &pCurrChunk->mChunkData[pCurrChunk->mDataIndex];
            do {
                
                pParticle->SetUnk58ArrayByIndex(
                    1, 
                    pParticle->GetUnk58ArrayByIndex(1) - gDisplay.dt
                );

                if (pParticle->GetUnk58ArrayByIndex(1) <= 0.0f || 
                    pParticle->mY > pParticle->GetUnk58ArrayByIndex(0)) {
                    pSys->DestroyParticle(pParticle, ppChunks);
                }

                pParticle++;
            } while (pParticle < &pCurrChunk->mChunkData[24]);

            if (*ppChunks == pCurrChunk) {
                ppChunks = &(*ppChunks)->mpNext;
            }
        } while (*ppChunks);
    }

    // Update the position of all particles 
    ppChunks = &pSys->mpChunks;
    while (*ppChunks) {
        ParticleChunk* pCurrChunk = *ppChunks;
        Particle* pParticle = &pCurrChunk->mChunkData[pCurrChunk->mDataIndex];
        do {
            pParticle->mX += _table_sinf((pParticle->GetUnk58ArrayByIndex(1) + pParticle->unkC) * 10.0f) * 0.3f;
            pParticle->mY += pParticle->unk24;
            pParticle->mZ += _table_sinf((pParticle->GetUnk58ArrayByIndex(1) + pParticle->unkC) * 10.0f) * 0.3f;
            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);
        ppChunks = &(*ppChunks)->mpNext;
    }
}
