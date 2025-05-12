#include "ty/GuideParticle.h"
#include "common/ParticleSystem.h"
#include "ty/GameObjectManager.h"

extern void Draw_AddPostDrawModel(Model*, float, bool);

static GameObjDesc guideParticleDesc;

ParticleSystemType guideParticleSysType;

static ModuleInfo<GuideParticle> guideParticleModule;

static Material* pSparkleMaterial;

int GuideParticle::gateTimeOutCounter = 0;

static int counter = 0;

static bool bHide = true;

void GuideParticle_CustomUpdate(ParticleSystem*);

void GuideParticle_LoadResources(KromeIni* pIni) {

    static ParticleEnvelope gemEnvelope[] = {
        {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {0.2f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {0.8f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    };

    guideParticleDesc.Init(
        &guideParticleModule,
        "Prop_0413_TimeAttackRing",
        "TimeAttackRing",
        1,
        0
    );

    guideParticleDesc.maxDrawDist = 20000.0f;
    guideParticleDesc.maxUpdateDist = 20000.0f;

    objectManager.AddDescriptor(&guideParticleDesc);

    pSparkleMaterial = Material::Create("fx_007");

    guideParticleSysType.Init("TA Ring Particles", pSparkleMaterial, 5.0f, 24.0f, 24.0f, 1);

    guideParticleSysType.unk2C = 2;
    guideParticleSysType.updateFunc = GuideParticle_CustomUpdate;

    guideParticleSysType.SetEnvelope(ARRAY_SIZE(gemEnvelope), gemEnvelope);

}

void GuideParticle_Show(void) {
    counter = 0;
    bHide = false;
}

void GuideParticle_Hide(void) {
    bHide = true;
}

bool GuideParticle_HasPassedThoughAll(void) {
    return counter == guideParticleDesc.unk74;
}

void GuideParticle::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);

    bHide = true;

    pModel = Model::Create(pDesc->modelName, NULL);

    unk3C = 1.0f;

    unk40.SetIdentity();

    unk84 = counter;
    counter++;
    
    unk80 = -1;
}

void GuideParticle::Deinit(void) {
    GameObject::Deinit();
}

static Vector scaleLoadVar = {1.0f, 1.0f, 1.0f, 1.0f};

bool GuideParticle::LoadLine(KromeIniLine* pLine) {

}

void GuideParticle::LoadDone(void) {
    pModel->matrices[0].Scale(&scaleLoadVar);

    unk40 = pModel->matrices[0];

    scaleLoadVar.Set(1.0f, 1.0f, 1.0f, 1.0f);

    Reset();

    objectManager.AddObject(this, pModel);
}

void GuideParticle::Reset(void) {
    unk88 = 2;

    pModel->colour.Set(0.5f, 0.5f, 0.5f, 0.5f);

    unk3C = 1.0f;

    GuideParticle::gateTimeOutCounter = 0;
}

void GuideParticle::Draw(void) {
    if (!bHide && unk88 != 0) {
        Draw_AddPostDrawModel(pModel, distSquared, IsInWater() ? true : false);
    }
}

void GuideParticle::UpdateModule(void) {

}
