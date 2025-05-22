#include "ty/ParticleEngine.h"
#include "ty/global.h"
#include "ty/Ty.h"

static bool bParticleResourcesLoaded = false;
static bool bParticleManagerInited = false;

static TyParticleManager actualParticleManager;

TyParticleManager* particleManager = &actualParticleManager;

void TyParticleManager::LoadResources(void) {
    bParticleResourcesLoaded = true;

    gb.pTensionMat = Material::Create("tension");
    gb.pShadowMat = Material::Create("shadow");
    gb.pBlackSquareMat = Material::Create("black");

    ty.rangTrailsMat = Material::Create("trail_03");

    unkD0[0] = Model::Create("prop_0142_leafparticle2", NULL);
    unkD0[0]->renderType = 3;

    unkD0[1] = Model::Create("prop_0140_grassparticle", NULL);
    unkD0[1]->renderType = 3;

    unkD0[2] = Model::Create("prop_0136_leafparticle", NULL);
    unkD0[2]->renderType = 3;

    pFeatherModel = Model::Create("Prop_0186_FeatherParticle", NULL);
    pFeatherModel->renderType = 3;

    pFeatherModel->matrices[0].Scale(0.7f);
    pFeatherModel->SetInverseScaleValue(0, 1.0f / 0.7f);

    pIceShardModels[0] = Model::Create("Prop_0064_IceShard1", NULL);
    pIceShardModels[1] = Model::Create("Prop_0065_IceShard2", NULL);

    pWoodModels[0] = Model::Create("Prop_0015_wood1", NULL);
    pWoodModels[0]->renderType = 3;

    pWoodModels[1] = Model::Create("Prop_0016_wood2", NULL);
    pWoodModels[1]->renderType = 3;

    pTMoundModels[0] = Model::Create("Prop_0024_TMoundChunk01", NULL);
    pTMoundModels[0]->renderType = 3;

    pTMoundModels[1] = Model::Create("Prop_0024_TMoundChunk02", NULL);
    pTMoundModels[1]->renderType = 3;

    pTMoundModels[2] = Model::Create("Prop_0024_TMoundChunk03", NULL);
    pTMoundModels[2]->renderType = 3;

    unk58 = Material::Create("fx_001");
    unk5C = Material::Create("fx_002");
    unk60 = Material::Create("fx_002b");

    pBilbyAtomMaterial = Material::Create("fx_007");

    pFireMaterial = Material::Create("fx_008");

    unk44 = Material::Create("fx_012");
    unk48 = Material::Create("fx_020");
    unk4C = Material::Create("fx_059");
    unk50 = Material::Create("fx_030");
    unk7C = Material::Create("fx_038");
    unk54 = Material::Create("fx_061");
    unk68 = Material::Create("fx_062");
    unk6C = Material::Create("fx_063");
    unk70 = Material::Create("fx_064");
    unk64 = Material::Create("fx_031");

    pExclamationMaterial = Material::Create("fx_067");

    unk88 = Material::Create("fx_076");
    unk80 = Material::Create("fx_073");
    unk94 = Material::Create("fx_137");
    unk84 = Material::Create("fx_075");

    unk90 = Material::Create("fx_078");
    pGooMaterial = Material::Create("fx_040");

    unk98[0] = Material::Create("fx_068");
    unk98[1] = Material::Create("fx_069");
    unk98[2] = Material::Create("fx_070");
    unk98[3] = Material::Create("fx_071");

    unkA8 = Material::Create("ty_footprint");
    unkAC = Material::Create("ty_footprint2");

    unkE0 = Model::Create("prop_0041_splash", "propanim_0041_splash");
    unkE0->renderType = 3;
}

void TyParticleManager::Init(void) {
    bParticleManagerInited = true;

    mFireData.Init(50);
    mFireParticles.Init(mFireData.GetCount());

    mTireDustData.Init(50);
    mTireDustParticles.Init(mTireDustData.GetCount());

    mBreathMistData.Init(5);
    mBreathMistParticles.Init(mBreathMistData.GetCount());

    mBilbyAtomParticles.Init(50);

    mRippleList.Init(100, sizeof(WaterRippleStruct));
    mGooList.Init(100, sizeof(GooStruct));

    mFastGlowParticles.Init(10);

    mLeafList.Init(16, sizeof(LeafGrassDustChunkStruct));
    mFeatherList.Init(20, sizeof(FeatherStruct));
}

void TyParticleManager::Deinit(void) {
    if (bParticleManagerInited) {
        bParticleManagerInited = false;

        mFireData.Deinit();
        mFastGlowParticles.Deinit();
        mTireDustData.Deinit();
        mBreathMistData.Deinit();

        mLeafList.Deinit();
        mFeatherList.Deinit();


        mBilbyAtomParticles.Deinit();

        mRippleList.Deinit();
        mGooList.Deinit();
        mFireParticles.Deinit();
        mTireDustParticles.Deinit();
        mBreathMistParticles.Deinit();
    }
}

void TyParticleManager::SpawnSpark(Vector* pPos) {

}

void TyParticleManager::SpawnAnts(Vector* p, Vector* p1, Vector* p2) {

}

void TyParticleManager::SpawnChomp(Vector* p, float f1) {

}

void TyParticleManager::SpawnExclamation(void) {
    unkE4 = 180.0f;

    unkEC = 0;

    pHeroPos = &pHero->pos;

    if (pHero->IsBushPig()) {
        unkHeroFloat = 320.0f;
    } else {
        unkHeroFloat = 120.0f;
    }

    exclamationParticle.scale = 16.0f;
    exclamationParticle.angle = 0.0f;
    exclamationParticle.color.Set(1.0f, 1.0f, 1.0f, 1.0f);

    unkF8 = false;
}

void TyParticleManager::StopExclamation(bool b) {
    if (unkE4 != 0.0f && b) {
        unkE4 = 59.0f;
        unkF8 = true;
    } else {
        unkE4 = 0.0f;
        unkF8 = true;
    }
}

void TyParticleManager::SpawnGhost(Vector* p) {
    
}

void TyParticleManager::SpawnGhostSmoke(Vector* p, int r5) {

}

void TyParticleManager::SpawnWaterSteam(Vector* p, float f1) {

}

void TyParticleManager::SpawnFastGlow(Vector* pPos, float scale) {
    Blitter_Particle* pParticle = mFastGlowParticles.GetNextEntry();

    if (pParticle) {
        pParticle->pos = *pPos;
        pParticle->scale = scale / 2.0f;
        pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
        pParticle->angle = 0.0f;
    }
}

/// @brief Spawns a Bilby Pickup Atom Particle
/// @param pPos Position of the particle
/// @param scale Size of the particle
/// @param color RGB color of the particle
void TyParticleManager::SpawnBilbyPickupAtom(Vector* pPos, float scale, int color) {
    Blitter_Particle* pParticle = mBilbyAtomParticles.GetNextEntry();

    if (pParticle) {
        pParticle->pos = *pPos;
        pParticle->scale = scale / 2.0f;

        float rgb = color / 255.0f;

        pParticle->color.Set(rgb, rgb, rgb, 1.0f);
        pParticle->angle = 0.0f;
    }
}

void TyParticleManager::SpawnTireDust(Vector* pPos, float scale, float alpha) {
    float* pTireDustData = mTireDustData.GetNextEntry();
    Blitter_Particle* pParticle = mTireDustParticles.GetNextEntry();

    if (pTireDustData == NULL || pParticle == NULL) {
        return;
    }

    pTireDustData[0] = 1.0f;

    pParticle->pos = *pPos;

    pParticle->scale = scale / 2.0f;

    float clampedAlpha = alpha;

    if (clampedAlpha < 0.0f) {
        clampedAlpha = 0.0f;
    }

    if (clampedAlpha > 1.0f) {
        clampedAlpha = 1.0f;
    }

    pParticle->color.Set(1.0f, 1.0f, 1.0f, clampedAlpha);

    pParticle->angle = 0.0f;
}

void TyParticleManager::SpawnBreathMist(Vector* pPos, Vector* pVel, float scale) {
    BreathMistStruct* pBreathMistData = mBreathMistData.GetNextEntry();
    Blitter_Particle* pParticle = mBreathMistParticles.GetNextEntry();

    if (pBreathMistData == NULL || pParticle == NULL) {
        return;
    }

    pBreathMistData->unk0 = 1.0f;
    pBreathMistData->vel = *pVel;

    pParticle->pos = *pPos;
    pParticle->scale = scale / 2.0f;
    pParticle->color.Set(1.0f, 1.0f, 1.0f, 138.0f / 255.0f);
    pParticle->angle = 0.0f;
}

void TyParticleManager::SpawnFireParticle(Vector* pPos, float scaleFactor) {
    FireStruct* pFireData = mFireData.GetNextEntry();
    Blitter_Particle* pParticle = mFireParticles.GetNextEntry();

    if (pFireData == NULL || pParticle == NULL) {
        return;
    }

    pFireData->unk0 = (((RandomI(&gb.mRandSeed) % 100) * 10) / 100.0f) + 1.0f;
    pFireData->unk4 = 1.0f;

    pParticle->pos.x = (pPos->x - 5.0f) + (((RandomI(&gb.mRandSeed) % 100) * 10.0f) / 100.0f);
    pParticle->pos.y = (pPos->y - 5.0f) + (((RandomI(&gb.mRandSeed) % 100) * 10.0f) / 100.0f);
    pParticle->pos.z = (pPos->z - 5.0f) + (((RandomI(&gb.mRandSeed) % 100) * 10.0f) / 100.0f);

    pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);

    pParticle->angle = 0.0f;

    pParticle->scale = ((RandomI(&gb.mRandSeed) % 100) * 10.0f) / 100.0f;
    pParticle->scale *= scaleFactor;
}

void TyParticleManager::SpawnIceShard(Vector* p, Vector* p1, float f1) {

}

void TyParticleManager::SpawnGoo(Vector* pPos, Vector* pColor) {
    if (mGooList.IsFull()) {
        return;
    }

    GooStruct* pGoo = mGooList.GetNextEntry();

    if (pGoo) {
        pGoo->mParticle.pos = *pPos;

        pGoo->mParticle.scale = RandomFR(&gb.mRandSeed, 15.0f, 30.0f);

        pGoo->mParticle.color = *pColor;

        pGoo->mParticle.angle = 0.0f;

        pGoo->unk40 = RandomFR(&gb.mRandSeed, -0.4f, 0.4f);

        pGoo->unk30.Set(
            RandomFR(&gb.mRandSeed, -3.0f, 3.0f),
            RandomFR(&gb.mRandSeed, 10.0f, 14.0f),
            RandomFR(&gb.mRandSeed, -3.0f, 3.0f)
        );

        pGoo->mParticle.unk24 = 0;

        pGoo->unk44 = 120;
    }
}

void TyParticleManager::SetCamera(Vector* p, Vector* p1) {
    unk20 = *p;
    unk30 = *p1;
}

void TyParticleManager::Update(void) {

}

void TyParticleManager::DrawPreWater(void) {
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);

    if (gb.pDialogPlayer == NULL) {
        WaterRippleStruct** ppRipples = mRippleList.GetMem();

        if (*ppRipples) {
            while (*ppRipples) {
                unk58->color = (*ppRipples)->unk1C;
                unk58->color.w = 1.0f;

                // Debug build:
                // unk58->color.Set(1.0f, 1.0f, 1.0f, 1.0f);

                Tools_DrawDropShadow(
                    unk58,
                    &(*ppRipples)->unk1C,
                    (*ppRipples)->unk4,
                    (*ppRipples)->x, (*ppRipples)->y, (*ppRipples)->z,
                    0.0f, 1.0f, 0.0f,
                    (*ppRipples)->angle,
                    false
                );

                ppRipples++;
            }

            View::GetCurrent()->SetLocalToWorldMatrix(NULL);
        }
    }
}

void TyParticleManager::DrawPostWater(void) {
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);

    Blitter_Particle* pFireParticles = mFireParticles.GetCurrEntry();
    if (pFireParticles) {
        pFireMaterial->Use();

        pFireParticles->Draw(mFireParticles.GetCount());
    }

    GooStruct** ppGoos = mGooList.GetMem();
    if (*ppGoos) {
        pGooMaterial->Use();

        while (*ppGoos) {
            (*ppGoos)->mParticle.DrawFrames(1, 1.0f / 4.0f);

            ppGoos++;
        }
    }
    
}

void TyParticleManager::SpawnElectricity(Vector* p, float f1) {

}
