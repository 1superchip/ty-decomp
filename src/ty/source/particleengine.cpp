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
    pFastGlowMaterial = Material::Create("fx_020");
    pDustMaterial = Material::Create("fx_059");
    pShockGlowMaterial = Material::Create("fx_030");
    unk7C = Material::Create("fx_038");
    unk54 = Material::Create("fx_061");
    pChompMaterial = Material::Create("fx_062");
    pGhostMaterial = Material::Create("fx_063");
    pGhostSmokeMaterial = Material::Create("fx_064");
    unk64 = Material::Create("fx_031");

    pExclamationMaterial = Material::Create("fx_067");

    unk88 = Material::Create("fx_076");
    unk80 = Material::Create("fx_073");
    unk94 = Material::Create("fx_137");
    unk84 = Material::Create("fx_075");

    unk90 = Material::Create("fx_078");
    pGooMaterial = Material::Create("fx_040");

    pWaterWashMaterials[0] = Material::Create("fx_068");
    pWaterWashMaterials[1] = Material::Create("fx_069");
    pWaterWashMaterials[2] = Material::Create("fx_070");
    pWaterWashMaterials[3] = Material::Create("fx_071");

    unkA8 = Material::Create("ty_footprint");
    unkAC = Material::Create("ty_footprint2");

    unkE0 = Model::Create("prop_0041_splash", "propanim_0041_splash");
    unkE0->renderType = 3;
}

void TyParticleManager::Init(void) {
    bParticleManagerInited = true;

    mFireData.Init(50);
    mFireParticles.Init(mFireData.capacity());

    mTireDustData.Init(50);
    mTireDustParticles.Init(mTireDustData.capacity());

    mBreathMistData.Init(5);
    mBreathMistParticles.Init(mBreathMistData.capacity());

    mShockGlowData.Init(20);
    mShockGlowParticles.Init(mShockGlowData.capacity());

    mWaterDropGreenData.Init(50);
    mWaterDropGreenParticles.Init(mWaterDropGreenData.capacity());

    mWaterDropBlueData.Init(50);
    mWaterDropBlueParticles.Init(mWaterDropBlueData.capacity());

    mShockGlowData.Init(20);
    mShockGlowParticles.Init(mShockGlowData.capacity());

    mChompData.Init(20);
    mChompParticles.Init(mChompData.capacity());

    mGhostData.Init(5);
    mGhostParticles.Init(mGhostData.capacity());

    mGhostSmokeData.Init(30);
    mGhostSmokeParticles.Init(mGhostSmokeData.capacity());

    mWaterSteamData.Init(30);
    mWaterSteamParticles.Init(mWaterSteamData.capacity());

    unkE4 = 0.0f;

    mSparkData.Init(150);
    mSparkParticles.Init(mSparkData.capacity());

    mAntData.Init(50);
    mAntParticles.Init(mAntData.capacity());

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
        mShockGlowData.Deinit();
        //
        mWaterDropGreenData.Deinit();
        mWaterDropBlueData.Deinit();
        mChompData.Deinit();
        mGhostData.Deinit();
        mGhostSmokeData.Deinit();
        mWaterSteamData.Deinit();
        //
        mSparkData.Deinit();
        mAntData.Deinit();

        mBilbyAtomParticles.Deinit();

        mRippleList.Deinit();
        mGooList.Deinit();

        mFireParticles.Deinit();
        mTireDustParticles.Deinit();
        mBreathMistParticles.Deinit();
        mShockGlowParticles.Deinit();
        mWaterDropGreenParticles.Deinit();
        mWaterDropBlueParticles.Deinit();
        mChompParticles.Deinit();
        mGhostParticles.Deinit();
        mGhostSmokeParticles.Deinit();
        mWaterSteamParticles.Deinit();
        mSparkParticles.Deinit();
        mAntParticles.Deinit();
        //
        //
        //
        //
        //
        //
        //
    }
}

void TyParticleManager::SpawnSpark(Vector* pPos) {
    for (int i = 0; i < 10; i++) {
        SparkStruct* pSparkStruct = mSparkData.GetNextEntry();
        Blitter_Particle* pSparkParticle = mSparkParticles.GetNextEntry();

        if (pSparkStruct == NULL || pSparkParticle == NULL) {
            return;
        }

        pSparkStruct->unk10 = 1.0f;

        Vector vv;

        vv.Set(
            -3.0f + ((RandomI(&gb.mRandSeed) % 100) * 6.0f) / 100.0f,
            ((RandomI(&gb.mRandSeed) % 100) * 6.0f) / 100.0f,
            -3.0f + ((RandomI(&gb.mRandSeed) % 100) * 6.0f) / 100.0f
        );

        pSparkStruct->unk0 = vv;

        pSparkStruct->unk18 = pSparkStruct->unk1C = pSparkStruct->unk20 = 100 + (RandomI(&gb.mRandSeed) % 155);

        pSparkParticle->pos = *pPos;
        pSparkParticle->scale = 4.0f;
        pSparkParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
        pSparkParticle->angle = 0.0f;
    }
}

void TyParticleManager::SpawnAnts(Vector* p, Vector* p1, Vector* p2) {
    for (int i = 0; i < 3; i++) {
        AntStruct* pAntStruct = mAntData.GetNextEntry();
        Blitter_Particle* pParticle = mAntParticles.GetNextEntry();

        if (pAntStruct == NULL || pParticle == NULL) {
            return;
        }

        pAntStruct->unk10 = 1.0f;

        Vector vv;

        vv.Set(
            ((RandomI(&gb.mRandSeed) % 100) * 6.0f) / 100.0f + -3.0f,
            ((RandomI(&gb.mRandSeed) % 100) * 6.0f) / 100.0f,
            ((RandomI(&gb.mRandSeed) % 100) * 6.0f) / 100.0f + -3.0f
        );

        pAntStruct->unk0 = vv;

        pAntStruct->unk1C = pAntStruct->unk20 = pAntStruct->unk24 =(RandomI(&gb.mRandSeed) % 155) + 100;

        pAntStruct->unk18 = RandomFR(&gb.mRandSeed, -100.0f, 100.0f);
        pAntStruct->unk18 *= 0.001f;

        pParticle->pos = *p;
        pParticle->scale = 4.0f;
        pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
        pParticle->angle = (RandomFR(&gb.mRandSeed, 0.0f, 100.0f) * 6.28f) * 0.01f;
    }
}

void TyParticleManager::SpawnChomp(Vector* pPos, float f1) {
    ChompStruct* pChompStruct = mChompData.GetNextEntry();
    Blitter_Particle* pParticle = mChompParticles.GetNextEntry();

    if (pChompStruct == NULL || pParticle == NULL) {
        return;
    }

    pChompStruct->unk0 = 1.0f;
    pChompStruct->unk4 = pChompStruct->unk8 = pChompStruct->unkC = 255;

    pParticle->pos = *pPos;
    pParticle->scale = f1 / 2.0f;
    pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    pParticle->angle = 0.0f;
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

void TyParticleManager::SpawnGhost(Vector* pPos) {
    GhostStruct* pGhostData = mGhostData.GetNextEntry();
    Blitter_Particle* pParticle = mGhostParticles.GetNextEntry();

    if (pGhostData == NULL || pParticle == NULL) {
        return;
    }

    pGhostData->unk0 = 1.0f;
    pGhostData->unk4 = pGhostData->unk8 = pGhostData->unkC = 255;

    pGhostData->unk14 = 0.0f;
    pGhostData->unk18 = 75.0f;
    pGhostData->unk1C = 75.0f;

    pGhostData->unk10 = 0;

    pParticle->pos = *pPos;
    pParticle->scale = 75.0f;
    pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    pParticle->angle = 0.0f;
}

void TyParticleManager::SpawnGhostSmoke(Vector* pPos, int r5) {
    GhostSmokeStruct* pGhostSmokeData = mGhostSmokeData.GetNextEntry();
    Blitter_Particle* pParticle = mGhostSmokeParticles.GetNextEntry();

    if (pGhostSmokeData == NULL || pParticle == NULL) {
        return;
    }

    pGhostSmokeData->unk0 = 4.0f;
    pGhostSmokeData->unk10 = r5;

    pParticle->pos = *pPos;
    pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    pParticle->angle = 0.0f;

    switch (r5) {
        case 1:
            pParticle->scale = 25.0f;
            break;
        case 2:
            pParticle->scale = 25.0f;
            break;
    }
}

void TyParticleManager::SpawnWaterSteam(Vector* pPos, float scale) {
    WaterSteamStruct* pWaterSteamData = mWaterSteamData.GetNextEntry();
    Blitter_Particle* pParticle = mWaterSteamParticles.GetNextEntry();

    if (pWaterSteamData == NULL || pParticle == NULL) {
        return;
    }

    pWaterSteamData->unk0 = 4.0f;

    pParticle->pos = *pPos;
    pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    pParticle->angle = 0.0f;

    pParticle->scale = scale;
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
    TireDustStruct* pTireDustData = mTireDustData.GetNextEntry();
    Blitter_Particle* pParticle = mTireDustParticles.GetNextEntry();

    if (pTireDustData == NULL || pParticle == NULL) {
        return;
    }

    pTireDustData->unk0 = 1.0f;

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

void TyParticleManager::SpawnBridgeChunk(Vector*, Model* pModel) {
    Vector vel;
    vel.Set(0.0f, 20.0f, 0.0f);

    pModel->GetModelVolume();
}

void TyParticleManager::SpawnAntHillChunk(Vector*, Vector*) {

}

void TyParticleManager::SpawnLeafGrassDust(Vector*, Vector*, bool) {

}

void TyParticleManager::SpawnFeather(Vector*, Vector*) {
    
}

void TyParticleManager::SpawnShockGlow(Vector* pPos, float scale) {
    ShockGlowStruct* pShockGlowData = mShockGlowData.GetNextEntry();
    Blitter_Particle* pParticle = mShockGlowParticles.GetNextEntry();

    if (pShockGlowData == NULL || pParticle == NULL) {
        return;
    }

    pShockGlowData->unk4 = pShockGlowData->unk8 = pShockGlowData->unkC = 255;
    pShockGlowData->unk0 = 1.0f;

    pParticle->pos = *pPos;
    pParticle->scale = scale / 2.0f;
    pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    pParticle->angle = 0.0f;
}

void TyParticleManager::SpawnWaterRipple(Vector* pPos, float f1) {
    if (mRippleList.IsFull()) {
        return;
    }

    WaterRippleStruct* pRipple = mRippleList.GetNextEntry();
    if (pRipple) {
        pRipple->x = pPos->x;
        pRipple->y = pPos->y;
        pRipple->z = pPos->z;

        pRipple->unk4 = f1 / 2.0f;

        pRipple->angle = ((RandomI(&gb.mRandSeed) % 1000) * (2 * PI)) / 1000;

        pRipple->unk0 = 4.0f;

        pRipple->unk1C.Set(0.0f, 0.0f, 0.0f, 1.0f);

        pRipple->unk2C = 0;
    }
}

void TyParticleManager::SpawnWaterDropGreen(Vector* pPos, Vector* r5, float scale) {
    WaterDropGreenStruct* pWaterDropGreenData = mWaterDropGreenData.GetNextEntry();
    Blitter_Particle* pParticle = mWaterDropGreenParticles.GetNextEntry();

    if (pWaterDropGreenData == NULL || pParticle == NULL) {
        return;
    }

    pWaterDropGreenData->unk0 = 4.0f;
    pWaterDropGreenData->unk8 = *r5;
    pWaterDropGreenData->unk4 = pPos->y;

    pParticle->pos = *pPos;
    pParticle->scale = scale * 2.5f;
    pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    pParticle->angle = 0.0f;
}

void TyParticleManager::SpawnWaterDropBlue(Vector* pPos, Vector* r5, float scale) {
    WaterDropBlueStruct* pWaterDropBlueData = mWaterDropBlueData.GetNextEntry();
    Blitter_Particle* pParticle = mWaterDropBlueParticles.GetNextEntry();

    if (pWaterDropBlueData == NULL || pParticle == NULL) {
        return;
    }

    pWaterDropBlueData->unk0 = 4.0f;
    pWaterDropBlueData->unk8 = *r5;
    pWaterDropBlueData->unk4 = pPos->y;

    pParticle->pos = *pPos;
    pParticle->scale = scale * 2.5f;
    pParticle->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    pParticle->angle = 0.0f;
}

void TyParticleManager::SpawnBigSplash(Vector*, bool, float, bool, float, int) {

}

void TyParticleManager::SpawnSnowFlake(Vector*, Vector*, float, float, int) {

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

    pParticle->scale = ((RandomI(&gb.mRandSeed) % 100) * 50.0f) / 100.0f;
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

void TyParticleManager::SpawnFootPrint(FootPrintTypes, Vector*, Vector*, Rotation*, Vector*) {

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

    Blitter_Particle* pParticles = mFireParticles.GetCurrEntry();
    if (pParticles) {
        pFireMaterial->Use();

        pParticles->Draw(mFireParticles.size());
    }

    pParticles = mChompParticles.GetCurrEntry();
    if (pParticles) {
        pChompMaterial->Use();

        pParticles->Draw(mChompParticles.size());

        mChompParticles.UnknownSetPointer();
    }

    if (unkE4) {
        pParticles = &exclamationParticle;
        pExclamationMaterial->Use();

        pParticles->Draw(1);
    }

    pParticles = mFastGlowParticles.GetCurrEntry();
    if (pParticles) {
        pFastGlowMaterial->Use();

        pParticles->Draw(mFastGlowParticles.size());

        mFastGlowParticles.UnknownSetPointer();
    }

    pParticles = mTireDustParticles.GetCurrEntry();
    if (pParticles) {
        pDustMaterial->Use();

        pParticles->Draw(mTireDustParticles.size());
    }

    pParticles = mBreathMistParticles.GetCurrEntry();
    if (pParticles) {
        pDustMaterial->Use();

        pParticles->Draw(mBreathMistParticles.size());
    }

    pParticles = mGhostParticles.GetCurrEntry();
    if (pParticles) {
        pGhostMaterial->Use();

        pParticles->Draw(mGhostParticles.size());
    }

    pParticles = mGhostSmokeParticles.GetCurrEntry();
    if (pParticles) {
        pGhostSmokeMaterial->Use();

        pParticles->Draw(mGhostSmokeParticles.size());
    }

    pParticles = mBilbyAtomParticles.GetCurrEntry();
    if (pParticles) {
        pBilbyAtomMaterial->Use();

        pParticles->Draw(mBilbyAtomParticles.size());
        mBilbyAtomParticles.UnknownSetPointer();
    }

    LeafGrassDustChunkStruct** ppLeafChunks = mLeafList.GetMem();
    while (*ppLeafChunks) {
        (*ppLeafChunks)->Draw();

        ppLeafChunks++;
    }

    FeatherStruct** ppFeathers = mFeatherList.GetMem();
    while (*ppFeathers) {
        (*ppFeathers)->Draw();

        ppFeathers++;
    }

    View::GetCurrent()->SetLocalToWorldMatrix(NULL);

    pParticles = mShockGlowParticles.GetCurrEntry();
    if (pParticles) {
        pShockGlowMaterial->Use();

        pParticles->Draw(mShockGlowParticles.size());
    }

    for (int i = 0; i < ARRAY_SIZE(mWaterWashParticles); i++) {
        pParticles = mWaterWashParticles[i].GetCurrEntry();
        if (pParticles) {
            pWaterWashMaterials[i]->Use();

            pParticles->Draw(mWaterWashParticles[i].size());
        }
    }

    pParticles = mWaterDropGreenParticles.GetCurrEntry();
    if (pParticles) {
        pShockGlowMaterial->Use();

        pParticles->Draw(mWaterDropGreenParticles.size());
    }

    pParticles = mWaterDropBlueParticles.GetCurrEntry();
    if (pParticles) {
        pShockGlowMaterial->Use();

        pParticles->Draw(mWaterDropBlueParticles.size());
    }

    pParticles = mSparkParticles.GetCurrEntry();
    if (pParticles) {
        pShockGlowMaterial->Use();

        pParticles->Draw(mSparkParticles.size());
    }

    pParticles = mAntParticles.GetCurrEntry();
    if (pParticles) {
        pShockGlowMaterial->Use();

        pParticles->Draw(mAntParticles.size());
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
