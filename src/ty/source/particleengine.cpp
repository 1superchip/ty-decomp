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
    unk78 = Material::Create("fx_007");

    unk40 = Material::Create("fx_008");
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
    unk74 = Material::Create("fx_067");

    unk88 = Material::Create("fx_076");
    unk80 = Material::Create("fx_073");
    unk94 = Material::Create("fx_137");
    unk84 = Material::Create("fx_075");

    unk90 = Material::Create("fx_078");
    unk8C = Material::Create("fx_040");

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
}

void TyParticleManager::Deinit(void) {

}

void TyParticleManager::SpawnSpark(Vector* pPos) {

}

void TyParticleManager::SpawnAnts(Vector* p, Vector* p1, Vector* p2) {

}

void TyParticleManager::SpawnChomp(Vector* p, float f1) {

}

void TyParticleManager::SpawnExclamation(void) {

}

void TyParticleManager::StopExclamation(bool b) {

}

void TyParticleManager::SpawnGhost(Vector* p) {
    
}

void TyParticleManager::DrawPreWater(void) {
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);


}

void TyParticleManager::DrawPostWater(void) {
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);

    
}
