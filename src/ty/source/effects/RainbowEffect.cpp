#include "ty/effects/RainbowEffect.h"

#ifdef TYTYPES_H
#error "tytypes.h included in RainbowEffect.cpp"
#endif

// FAKE! Prevent tytypes.h from being included in this file
#define TYTYPES_H

#include "ty/global.h"
#include "common/StdMath.h"

static const Vector RainbowEffect_RodataPadding[5] = {};

ParticleSystemType RainbowEffect::rainbowEffectType;
Material* RainbowEffect::pRainbowMat;

void RainbowEffect_Update(ParticleSystem* pSys) {

    float f31 = gDisplay.dt;

    float f29 = pSys->GetAge() - pSys->mpType->unk1C;
    
    float f30 = gDisplay.dt * pSys->mpType->yVel;

    if (!pSys->mpType->InfiniteParticles()) {
        pSys->DestroyAllParticlesCreatedBefore(f29);
    }

    ParticleChunk* pCurrChunk = pSys->GetChunks();
    while (pCurrChunk) {
        Particle* pParticle = pCurrChunk->GetParticle();
        do {
            pParticle->mY += f30;

            float diffAge = pSys->age - pParticle->unkC;
            if (diffAge > pSys->mpType->mpEnvelopes[pParticle->mEnvelopeIndex].unkC) {
                pParticle->mEnvelopeIndex++;
            }
            
            int prevEnvIndex = pParticle->mEnvelopeIndex - 1;
            float f4 = (diffAge - pSys->mpType->mpEnvelopes[prevEnvIndex].unkC) / 
                pSys->mpType->mpEnvelopes[prevEnvIndex].deltaAge;
            
            pParticle->mColor.w = pSys->mpType->mpEnvelopes[prevEnvIndex].unk4 + 
                (f4 * pSys->mpType->mpEnvelopes[prevEnvIndex].unk14);

            if ((gb.logicGameCount % 2) != 0) {
                pParticle->unk30 = Max<float>(pParticle->unk30 - (f31 * 25.0f), 0.11f);
                pParticle->unk30 += RandomFR(&gb.mRandSeed, 0.0f, 0.2f);
            }
            
            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);

        pCurrChunk = pCurrChunk->GetNext();
    }
}

void RainbowEffect_LoadResources(void) {

    #define NUM_RAINBOWEFFECT_ENVELOPES (4)

    static ParticleEnvelope envelope[NUM_RAINBOWEFFECT_ENVELOPES] = {
        {
            0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f
        },
        {
            0.1f, 0.9f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f
        },
        {
            0.5f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f
        },
        {
            1.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f
        }
    };

    RainbowEffect::pRainbowMat = Material::Create("fx_031");

    RainbowEffect::rainbowEffectType.Init(
        "Rainbow Effect", RainbowEffect::pRainbowMat,
        2.0f, 1.0f, 1.0f, 1
    );

    RainbowEffect::rainbowEffectType.SetEnvelope(NUM_RAINBOWEFFECT_ENVELOPES, envelope);
    RainbowEffect::rainbowEffectType.yVel = -6.0f;
    RainbowEffect::rainbowEffectType.updateFunc = RainbowEffect_Update;
}


void RainbowEffect::Init(Vector* pPos) {
    static BoundingVolume pRainbowVol = {
        {-300.0f, -300.0f, -300.0f}, 
        {600.0f, 600.0f, 600.0f}
    };

    pSys = ParticleSystem::Create(&rainbowEffectType, pPos, &pRainbowVol, 1.0f, 1, NULL);

    for (int i = 0; i < NUM_RAINBOWEFFECT_SUBS; i++) {
        subs[i].unk10[0] = RandomFR(&gb.mRandSeed, 0.0f, 0.5f);
        subs[i].unk10[1] = RandomFR(&gb.mRandSeed, 0.0f, 0.5f);
        subs[i].unk10[2] = RandomFR(&gb.mRandSeed, 0.0f, 0.5f);

        subs[i].particleColor[0] = RandomFR(&gb.mRandSeed, 0.0f, 0.5f);
        subs[i].particleColor[1] = RandomFR(&gb.mRandSeed, 0.0f, 0.5f);
        subs[i].particleColor[2] = RandomFR(&gb.mRandSeed, 0.0f, 0.5f);

        subs[i].unk8 = subs[i].unkC = 0;
        subs[i].unk4 = false;
    }

    for (int i = 0; i < NUM_RAINBOWEFFECT_SUBS; i++) {
        subs[i].spline.Init(6, false);
    }

    unk4 = 5.0f;
    unkC = 5.0f;
    unk8 = 1.0f;
    unk0 = 240.0f;
    unk10 = 1.0f;
}

void RainbowEffect::Deinit(void) {
    for (int i = 0; i < NUM_RAINBOWEFFECT_SUBS; i++) {
        subs[i].spline.Deinit();
    }
}

void RainbowEffect::Reset(void) {
    if (pSys) {
        pSys->DestroyAll();
    }
}

void RainbowEffect::Update(float f1) {
    Vector pp1;
    pp1.SetZero();

    if ((gb.logicGameCount % 2) != 0) {
        for (int i = 0; i < NUM_RAINBOWEFFECT_SUBS; i++) {
            subs[i].unk8++;
            if (!subs[i].unk4 || subs[i].unk8 == subs[i].unkC * 5) {
                subs[i].spline.Reset();

                if ((i % 2) == 0) {
                    Vector pos1 = {
                        RandomFR(&gb.mRandSeed, -25.0f, -35.0f),
                        0.0f,
                        RandomFR(&gb.mRandSeed, -25.0f, -35.0f),
                        0.0f
                    };
                    subs[i].spline.AddNode(&pos1);

                    Vector tmpPos;
                    tmpPos.Set(
                        RandomFR(&gb.mRandSeed, -25.0f, -35.0f),
                        RandomFR(&gb.mRandSeed, 0.0f, 50.0f),
                        RandomFR(&gb.mRandSeed, 25.0f, 35.0f)
                    );
                    subs[i].spline.AddNode(&tmpPos);

                    tmpPos.Set(
                        RandomFR(&gb.mRandSeed, 25.0f, 35.0f),
                        RandomFR(&gb.mRandSeed, 0.0f, 50.0f),
                        RandomFR(&gb.mRandSeed, 25.0f, 35.0f)
                    );
                    subs[i].spline.AddNode(&tmpPos);
                    
                    tmpPos.Set(
                        RandomFR(&gb.mRandSeed, 25.0f, 35.0f),
                        RandomFR(&gb.mRandSeed, 0.0f, 50.0f),
                        RandomFR(&gb.mRandSeed, -25.0f, -35.0f)
                    );
                    subs[i].spline.AddNode(&tmpPos);
                    subs[i].spline.AddNode(&pos1);
                } else {
                    Vector pos1 = {
                        RandomFR(&gb.mRandSeed, 25.0f, 35.0f),
                        0.0f,
                        RandomFR(&gb.mRandSeed, -25.0f, -35.0f),
                        0.0f
                    };
                    subs[i].spline.AddNode(&pos1);

                    Vector tmpPos;
                    tmpPos.Set(
                        RandomFR(&gb.mRandSeed, 25.0f, 35.0f),
                        RandomFR(&gb.mRandSeed, 0.0f, 50.0f),
                        RandomFR(&gb.mRandSeed, 25.0f, 35.0f)
                    );
                    subs[i].spline.AddNode(&tmpPos);

                    tmpPos.Set(
                        RandomFR(&gb.mRandSeed, -25.0f, -35.0f),
                        RandomFR(&gb.mRandSeed, 0.0f, 50.0f),
                        RandomFR(&gb.mRandSeed, 25.0f, 35.0f)
                    );
                    subs[i].spline.AddNode(&tmpPos);
                    
                    tmpPos.Set(
                        RandomFR(&gb.mRandSeed, -25.0f, -35.0f),
                        RandomFR(&gb.mRandSeed, 0.0f, 50.0f),
                        RandomFR(&gb.mRandSeed, -25.0f, -35.0f)
                    );
                    subs[i].spline.AddNode(&tmpPos);
                    subs[i].spline.AddNode(&pos1);
                }

                subs[i].unkC = RandomIR(&gb.mRandSeed, 5, 20);
                subs[i].unk8 = 0;
                subs[i].unk4 = true;
            }

            if (f1 > 60.0f) {
                subs[i].particleColor[0] += (subs[i].unk10[0] - subs[i].particleColor[0]) / 150.0f;
                subs[i].particleColor[1] += (subs[i].unk10[1] - subs[i].particleColor[1]) / 150.0f;
                subs[i].particleColor[2] += (subs[i].unk10[2] - subs[i].particleColor[2]) / 150.0f;

                if ((RandomI(&gb.mRandSeed) % 300) == 0) {
                    subs[i].unk10[0] = RandomFR(&gb.mRandSeed, 0.0f, 1.0f);
                    subs[i].unk10[1] = RandomFR(&gb.mRandSeed, 0.0f, 1.0f);
                    subs[i].unk10[2] = RandomFR(&gb.mRandSeed, 0.0f, 1.0f);

                    switch (RandomI(&gb.mRandSeed) % 3) {
                        case 0:
                            subs[i].unk10[0] = 1.0f;
                            break;
                        case 1:
                            subs[i].unk10[1] = 1.0f;
                            break;
                        case 2:
                            subs[i].unk10[2] = 1.0f;
                            break;
                    }
                }

            } else {
                subs[i].particleColor[0] *= 0.95f;
                subs[i].particleColor[1] *= 0.95f;
                subs[i].particleColor[2] *= 0.95f;
            }

            if (subs[i].spline.nodeIndex > 1) {
                pp1 = subs[i].spline.GetPosition(subs[i].unk8 / (subs[i].unkC * 5.0f));
            }

            pp1.Scale(unkC * unk10);
            pp1.Add(pSys->mpPos);

            pp1.y += unk10 * 40.0f;

            Vector color = {
                subs[i].particleColor[0],
                subs[i].particleColor[1],
                subs[i].particleColor[2],
                1.0f
            };

            Spawn(&pp1, &color);
        }

        // SmoothCenteredCurve(f1 / unk0)
        // unk0 is the x coordinate of the parabola with the points (0, 0), (unk0, 0) and the vertex at (unk0/2, 1)
        unkC = (SmoothCenteredCurve(f1 / unk0) * (unk8 - unk4)) + unk4;
    }
}

void RainbowEffect::Spawn(Vector* pPos, Vector* pColor) {
    if (pSys) {
        Particle* p = pSys->CreateParticle();
        if (p) {
            p->mX = pPos->x;
            p->mY = pPos->y;
            p->mZ = pPos->z;
            p->unk50 = unk10 * 60.0f;
            p->mColor = *pColor;
            p->unk30 = 1.0f;
            p->unk48 = 0.0f;
            p->unk4C = 1.0f;
        }
    }
}
