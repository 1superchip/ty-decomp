#include "ty/effects/ChronorangEffects.h"
#include "ty/global.h"

#include "ty/tytypes.h"

Material* Pollen::pPollenMat;
Material* SleepyDust::pSleepyDustMat;

ParticleSystemType Pollen::pollenType;
ParticleSystemType SleepyDust::sleepyDustType;

SleepyDust sleepyDust;

/// @brief Custom update function for the Pollen Particle System
/// @param pSys ParticleSystem to update
void Pollen_Update(ParticleSystem* pSys) {

    float f30 = pSys->GetAge() - pSys->mpType->unk1C;

    float xVel = pSys->mpType->xVel;
    float yVel = pSys->mpType->yVel + (1.0f / 6.0f);
    float zVel = pSys->mpType->zVel;

    if (!pSys->mpType->InfiniteParticles()) {
        pSys->DestroyAllParticlesCreatedBefore(f30);
    }

    ParticleChunk* pCurrChunk = pSys->GetChunks();
    while (pCurrChunk) {
        Particle* pParticle = &pCurrChunk->mChunkData[pCurrChunk->mDataIndex];
        do {
            pParticle->mX += pParticle->unk20;
            pParticle->mY += pParticle->unk24;
            pParticle->mZ += pParticle->unk28;

            pParticle->unk20 *= 0.98f;
            pParticle->unk24 *= 0.98f;
            pParticle->unk28 *= 0.98f;

            pParticle->mX += xVel;
            pParticle->mY += yVel;
            pParticle->mZ += zVel;

            pParticle->mColor.w *= 0.993f;
            
            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);
        
        pCurrChunk = pCurrChunk->GetNext();
    }
}

/// @brief Custom update function for the SleepyDust Particle System
/// @param pSys ParticleSystem to update
void SleepyDust_Update(ParticleSystem* pSys) {

}

/// @brief Loads resources for the Pollen and SleepyDust Particles
/// @param None
void ChronorangEffects_LoadResources(void) {
    Pollen::pPollenMat = Material::Create("FX_009");
    Pollen::pollenType.Init("Pollen", Pollen::pPollenMat, 4.0f, 3.0f, 3.0f, 1);
    Pollen::pollenType.updateFunc = Pollen_Update;
    
    SleepyDust::pSleepyDustMat = Material::Create("FX_009");
    SleepyDust::sleepyDustType.Init("SleepyDust", SleepyDust::pSleepyDustMat, 2.5f, 4.0f, 4.0f, 1);
    SleepyDust::sleepyDustType.updateFunc = SleepyDust_Update;

    sleepyDust.pSystem = NULL;

    SleepyDust::sleepyDustType.unk2C |= 2;
}

void Pollen::Init(Vector* pPos) {

    static BoundingVolume pPollenVol = {
        {-1000.0f, -1000.0f, -1000.0f},
        {2000.0f, 2000.0f, 2000.0f}
    };

    pSystem = ParticleSystem::Create(&Pollen::pollenType, pPos, &pPollenVol, 1.0f, 1, NULL);
}

void Pollen::Deinit(void) {
    if (pSystem) {
        pSystem->Destroy();
        pSystem = NULL;
    }
}

/// @brief Creates a new pollen particle
/// @param pPos 
/// @param arg2 
/// @param f1 
void Pollen::Spawn(Vector* pPos, Vector* arg2, float f1) {
    if (pSystem == NULL) {
        return;
    }

    Particle* p = pSystem->CreateParticle();
    if (p == NULL) {
        return;
    }

    p->unk20 = arg2->x + RandomFR(&gb.mRandSeed, -f1, f1);
    p->unk24 = arg2->y + RandomFR(&gb.mRandSeed, -f1, f1);
    p->unk28 = arg2->z + RandomFR(&gb.mRandSeed, -f1, f1);

    p->mX = pPos->x + p->unk20 * 7.0f;
    p->mY = pPos->y + p->unk24 * 7.0f;
    p->mZ = pPos->z + p->unk28 * 7.0f;
}

void Pollen::SetVel(Vector* pVel) {
    Pollen::pollenType.xVel = pVel->x;
    Pollen::pollenType.yVel = pVel->y;
    Pollen::pollenType.zVel = pVel->z;
}

void SleepyDust_Init(void) {
    sleepyDust.Init();
}

void SleepyDust::Init(void) {

    static Vector zero = {0.0f, 0.0f, 0.0f, 0.0f};

    static BoundingVolume sleepyDustVol = {
        {-100.0f, -100.0f, -100.0f},
        {200.0f, 200.0f, 200.0f}, 
    };

    pSystem = ParticleSystem::Create(
        &SleepyDust::sleepyDustType, &zero, &sleepyDustVol, 1.0f, NUM_DUST_ENTRIES, NULL
    );

    SleepyDust_Reset();

    if (pSystem) {
        for (int i = 0; i < 100; i++) {
            Particle* p = pSystem->CreateParticle();
            
            if (p) {
                Vector normal;
                Tools_RandomNormal(&normal);
                normal.Scale(RandomFR(&gb.mRandSeed, 30.0f, 60.0f));

                p->mX = normal.x;
                p->mY = normal.y;
                p->mZ = normal.z;
            }
        }
    }
}

void SleepyDust_Deinit(void) {
    if (sleepyDust.pSystem) {
        sleepyDust.pSystem->Destroy();
        sleepyDust.pSystem = NULL;
    }
}

void SleepyDust_Reset(void) {
    for (int i = 0; i < NUM_DUST_ENTRIES; i++) {
        sleepyDust.dustEntries[i].matrix.SetIdentity();
        sleepyDust.pSystem->pDynamicData[i].pMatrix = &sleepyDust.dustEntries[i].matrix;

        sleepyDust.dustEntries[i].unk0 = false;
        sleepyDust.dustEntries[i].unk1 = false;
    }
}

void SleepyDust_UpdateAll(void) {
    sleepyDust.Update();
}

void SleepyDust::Update(void) {
    if (pSystem == NULL) {
        return;
    }

    for (int i = 0; i < NUM_DUST_ENTRIES; i++) {
        if (dustEntries[i].unk0) {
            if (dustEntries[i].unk1) {
                dustEntries[i].matrix.Scale(0.92f);

                pSystem->pDynamicData[i].unk0 *= 0.9f;

                if (dustEntries[i].matrix.Row1()->MagSquared() < 0.05f) {
                    dustEntries[i].unk1 = false;
                    dustEntries[i].unk0 = false;
                }
            } else {
                pSystem->pDynamicData[i].unk0 = 1.0f - ((1.0f - pSystem->pDynamicData[i].unk0) * 0.94f);

                dustEntries[i].matrix = dustEntries[i].pModel->matrices[0];

                Vector v1;
                Vector v2;

                v1.ApplyMatrix(&dustEntries[i].pModel->GetBoundingVolume(-1)->v1, &dustEntries[i].matrix);
                v2.ApplyRotMatrix(&dustEntries[i].pModel->GetBoundingVolume(-1)->v2, &dustEntries[i].matrix);

                Vector sp8 = dustEntries[i].pModel->GetBoundingVolume(-1)->v2;

                sp8.x = sp8.y = sp8.z = 
                    (Abs<float>(sp8.x) + Abs<float>(sp8.y) + Abs<float>(sp8.z)) / 3.0f;

                dustEntries[i].matrix.Scale(&dustEntries[i].matrix, &sp8);
                dustEntries[i].matrix.Scale(0.01f);

                dustEntries[i].yaw += 0.02f;

                Matrix tmp;
                tmp.SetRotationYaw(dustEntries[i].yaw);
                dustEntries[i].matrix.Multiply3x3(&tmp, &dustEntries[i].matrix);

                v2.Scale(0.5f);

                dustEntries[i].matrix.Row3()->Add(&v1, &v2);

                dustEntries[i].unk1 = true;
            }

        } else {
            pSystem->pDynamicData[i].unk0 = 0.0f;
        }
    }
}

void SleepyDust_Update(Model* pModel) {
    for (int i = 0; i < NUM_DUST_ENTRIES; i++) {
        if (sleepyDust.dustEntries[i].unk0 && sleepyDust.dustEntries[i].pModel == pModel) {
            sleepyDust.dustEntries[i].unk1 = false;
        }
    }
}

void SleepyDust_AddNewUser(Model* pModel) {
    if (sleepyDust.pSystem == NULL) {
        return;
    }

    for (int i = 0; i < NUM_DUST_ENTRIES; i++) {
        if (sleepyDust.dustEntries[i].unk0 && sleepyDust.dustEntries[i].pModel == pModel) {
            return;
        }
    }

    for (int i = 0; i < NUM_DUST_ENTRIES; i++) {
        if (!sleepyDust.dustEntries[i].unk0) {
            sleepyDust.dustEntries[i].pModel = pModel;
            sleepyDust.dustEntries[i].unk1 = false;
            sleepyDust.dustEntries[i].unk0 = true;
            sleepyDust.dustEntries[i].yaw = 0.0f;
            break;
        }
    }
}

void SleepyDust_RemoveUser(Model* pModel) {
    if (sleepyDust.pSystem == NULL) {
        return;
    }

    for (int i = 0; i < NUM_DUST_ENTRIES; i++) {
        if (sleepyDust.dustEntries[i].unk0 && sleepyDust.dustEntries[i].pModel == pModel) {
            sleepyDust.dustEntries[i].unk1 = true;
        }
    }
}
