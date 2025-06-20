#include "ty/effects/Flame.h"
#include "ty/ParticleEngine.h"
#include "ty/global.h"

ParticleSystemType Flame::type;

ParticleEnvelope Flame::envelope[3] = {
    {
        0.0f, 0.3f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f
    },
    {
        0.4f, 0.8f, 8.0f, 0.0f, 0.0f, 0.0f, 0.0f
    },
    {
        1.0f, 0.0f, 1.2f, 0.0f, 0.0f, 0.0f, 0.0f
    }
};

/// @brief Loads resources for the Flame Particle
void Flame_LoadResources(void) {
    Flame::type.Init("Flame", particleManager->pFireMaterial, 1.0f, 5.0f, 5.0f, 1);
    Flame::type.SetEnvelope(ARRAY_SIZE(Flame::envelope), Flame::envelope);
    Flame::type.unk48 = 0.4f;
    Flame::type.unk64 = 0.995f;
    Flame::type.updateFunc = &Flame_CustomUpdate;
    Flame::type.unk2C |= 0x100 | 0x2;
}

/// @brief Initiates this Flame object
/// @param pPos Position of the ParticleSystem
/// @param pVolume Volume of ParticleSystem
/// @param f1 
/// @param f2 
void Flame::Init(Vector* pPos, BoundingVolume* pVolume, float f1, float f2) {
    mpSystem = ParticleSystem::Create(&Flame::type, pPos, pVolume, f1, 1, NULL);
    unk4 = f2;
}

/// @brief Destroys the ParticleSystem
void Flame::Deinit(void) {
    if (mpSystem) {
        mpSystem->Destroy();
    }
}

/// @brief Creates a Flame particle
/// @param pDir Direction of particle
/// @param dirLen Length of the direction
/// @param pColor Color of the particle, Optional (pass NULL for {1, 1, 1, 1})
void Flame::Create(Vector* pDir, float dirLen, Vector* pColor) {
    Particle* p = mpSystem->CreateParticle();
    
    if (p == NULL) {
        return;
    }

    float rand[3];
    rand[0] = RandomFR(&gb.mRandSeed, -0.1f, 0.1f);
    rand[1] = RandomFR(&gb.mRandSeed, -0.1f, 0.1f);
    rand[2] = RandomFR(&gb.mRandSeed, -0.1f, 0.1f);
    
    float dx, dy;
    
    Vector dir = *pDir;
    dir.Normalise();

    dx = dir.x;
    dy = dir.y;
    dir.x += (rand[0] * dir.y) + (rand[0] * dir.z);
    dir.y += (rand[1] * dx) + (rand[1] * dir.z);
    dir.z += (rand[2] * dx) + (rand[2] * dy);
    dir.Scale(Abs<float>(dirLen));

    p->mX = mpSystem->mpPos->x;
    p->mY = mpSystem->mpPos->y;
    p->mZ = mpSystem->mpPos->z;

    p->unk20 = dir.x;
    p->unk24 = dir.y;
    p->unk28 = dir.z;

    if (pColor) {
        p->mColor = *pColor;
    } else {
        p->mColor.Set(1.0f, 1.0f, 1.0f, 1.0f);
    }

    p->mAngle = RandomFR(&gb.mRandSeed, -PI, PI);
    p->unk48 = _table_sinf(p->mAngle);
    p->unk4C = _table_cosf(p->mAngle);
    p->SetUnk58ArrayByIndex(0, unk4);
}

/// @brief Destroys all ParticleChunks
void Flame::Reset(void) {
    mpSystem->DestroyAll();
}

/// @brief Custom update for the Flame Particle System
/// @param pSys ParticleSystem to update
void Flame_CustomUpdate(ParticleSystem* pSys) {
    float f31 = gDisplay.dt;
    float f30 = pSys->GetAge() - pSys->mpType->unk1C;
    float f28 = pSys->mpType->unk48;
    float f29 = pSys->mpType->unk64;

    if (!pSys->mpType->InfiniteParticles()) {
        pSys->DestroyAllParticlesCreatedBefore(f30);
    }
    
    ParticleChunk* pCurrChunk = pSys->GetChunks();
    while (pCurrChunk) {
        Particle* pParticle = pCurrChunk->GetParticle();
        do {
            pParticle->mX += pParticle->unk20 * f31;
            pParticle->unk20 *= f29;
            pParticle->mY += pParticle->unk24 * f31;
            pParticle->unk24 *= f29;
            pParticle->unk24 += f28;
            pParticle->mZ += pParticle->unk28 * f31;
            pParticle->unk28 *= f29;
            
            float f3 = pSys->GetAge() - pParticle->unkC;
            if (f3 > pSys->mpType->mpEnvelopes[pParticle->mEnvelopeIndex].unkC) {
                pParticle->mEnvelopeIndex++;
            }
            
            int prevEnvIndex = pParticle->mEnvelopeIndex - 1;
            float f4 = (f3 - pSys->mpType->mpEnvelopes[prevEnvIndex].unkC) / 
                pSys->mpType->mpEnvelopes[prevEnvIndex].deltaAge;
            
            pParticle->mColor.w = pSys->mpType->mpEnvelopes[prevEnvIndex].unk4 + 
                (f4 * pSys->mpType->mpEnvelopes[prevEnvIndex].unk14);

            if (pParticle->GetUnk58ArrayByIndex(0) > 1.0f) {
                pParticle->unk30 = pParticle->GetUnk58ArrayByIndex(0);
            } else {
                pParticle->unk30 = pSys->mpType->mpEnvelopes[prevEnvIndex].unk8 + 
                    (f4 * pSys->mpType->mpEnvelopes[prevEnvIndex].unk18);
            }
            
            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);

        pCurrChunk = pCurrChunk->GetNext();
    }
}
