#include "ty/GuideParticle.h"
#include "ty/global.h"
#include "ty/Hero.h"
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
    GuideParticle* pObjs = (GuideParticle*)guideParticleDesc.pInstances;

    for (int i = 0; i < guideParticleDesc.unk74; i++, pObjs++) {
        pObjs->Reset();
    }

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
    
    gateOrderIndex = -1;
}

void GuideParticle::Deinit(void) {
    GameObject::Deinit();
}

static Vector scaleLoadVar = {1.0f, 1.0f, 1.0f, 1.0f};

bool GuideParticle::LoadLine(KromeIniLine* pLine) {
    Vector temp;
    if (LoadLevel_LoadVector(pLine, "pos", &temp)) {
        pModel->matrices[0].SetTranslation(&temp);
        return true;
    }

    if (LoadLevel_LoadVector(pLine, "rot", &temp)) {
        pModel->matrices[0].SetRotationPYR(&temp);
        return true;
    }

    return LoadLevel_LoadVector(pLine, "scale", &scaleLoadVar) ||
        LoadLevel_LoadInt(pLine, "index", &gateOrderIndex) ||
        GameObject::LoadLine(pLine);
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

void GuideParticle::Update(void) {
    float roll = ((gb.logicGameCount % (uint)(gDisplay.displayFreq * 8.0f)) * 0.125f) * gDisplay.frameTime * PI * 2.0f;

    Matrix m;

    m.SetRotationRoll(roll);

    pModel->matrices[0].Multiply3x3(&m, &unk40);

    if (bHide) {
        return;
    }

    switch (unk88) {
        case 1:
            unk3C *= 1.04f;
            pModel->matrices[0].Scale(&unk40, unk3C);
            pModel->colour.Set(2.0f - unk3C, 2.0f - unk3C, 2.0f - unk3C, 2.0f - unk3C);
            if (unk3C > 2.0f) {
                unk3C = 1.0f;
                unk88 = 0;
            }
            break;
        case 2:
            if (gateOrderIndex == counter) {
                pModel->colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
                unk88 = 3;
            }
            break;
        case 3:
            Vector locHeroPos = *pHero->GetPos();
            locHeroPos.y += pHero->radius;

            Matrix invLTW;
            invLTW.Inverse(pLocalToWorld);

            locHeroPos.ApplyMatrix(&invLTW);

            // Check if hero is within the bounds of this object
            if (Sqr<float>(locHeroPos.x) + Sqr<float>(locHeroPos.y) <= pHero->radius * pHero->radius + 32500.0f 
                    && locHeroPos.z <= 50.0f && locHeroPos.z >= -50.0f) {
                SoundBank_Play(0xB7, NULL, 0);
                unk88 = 1;
                gateTimeOutCounter = 0;
                counter++;
            }
            break;
        case 0:
            break;
    }
}

void GuideParticle::Draw(void) {
    if (!bHide && unk88 != 0) {
        Draw_AddPostDrawModel(pModel, distSquared, IsInWater() ? true : false);
    }
}

float TimeTrial_GetTimer(void);
void TimeTrial_StopTimeAttack(void);

void GuideParticle::UpdateModule(void) {
    if (TimeTrial_GetTimer() <= 0.0f) {
        return;
    }

    int d = gDisplay.displayFreq * 15.0f;

    gateTimeOutCounter++;

    if (gateTimeOutCounter > d) {
        TimeTrial_StopTimeAttack();
    }
}

void GuideParticle_CustomUpdate(ParticleSystem* pSys) {
    ParticleChunk* pCurrChunk = pSys->GetChunks();
    while (pCurrChunk) {
        Particle* pParticle = pCurrChunk->GetParticle();
        do {
            pParticle->mAngle += pParticle->unk3C;
            pParticle->mAngle = pParticle->mAngle - ((int)(pParticle->mAngle * (1.0f / (2*PI))) * (2 * PI));

            pParticle->unk48 = _table_sinf(pParticle->mAngle * 4.0f);
            pParticle->unk4C = _table_cosf(pParticle->mAngle * 4.0f);

            pParticle->mX = pParticle->unk50 * (_table_sinf(pParticle->mAngle) * 150.0f);
            pParticle->mY = pParticle->unk50 * (_table_cosf(pParticle->mAngle) * 150.0f);
            pParticle->mZ = 0.0f;

            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);
        pCurrChunk = pCurrChunk->GetNext();
    }
}
