#include "ty/props/Talisman.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"

extern void Particle_Special_Create(ParticleSystem**, Vector*, Vector*, Vector*);
extern void Particle_Special_Init(ParticleSystem**, Vector*, BoundingVolume*);
extern void Particle_DestroyASystem(ParticleSystem**, float);

// Data for each talisman type
static TalismanInfo talismanData[TALISMAN_COUNT] = {
    {"prop_0525_DingoTalisman", "DingoTalisman"},
    {"prop_0174_Frog_talisman", "Frog_Talisman"},
    {"prop_0526_CockyTalisman", "CockyTalisman"},
    {"prop_0527_PlatyTalisman", "PlatyTalisman"},
    {"prop_0522_TigerTalisman", "TigerTalisman"}
};

// Descriptors for each type of Talisman
static StaticPropDescriptor TalismanDesc[TALISMAN_COUNT];
static ModuleInfo<Talisman> TalismanModule;

// Loads the descriptor for each Talisman
void Talisman_LoadResources(KromeIni* pIni) {
    for (int i = 0; i < ARRAY_SIZE(talismanData); i++) {
        TalismanDesc[i].Init(&TalismanModule, talismanData[i].pModelName, talismanData[i].pDescrName, 1, 1);
        TalismanDesc[i].Load(pIni);
        objectManager.AddDescriptor(&TalismanDesc[i]);
    }
}

void Talisman::Init(GameObjDesc* pDesc) {
    StaticProp::Init(pDesc);
    Particle_Special_Init(&pParticleSystem, pModel->matrices[0].Row3(), pModel->GetModelVolume());
    bVisible = true; // Set Talisman default visibility to true
}

void Talisman::Deinit(void) {
    if (pParticleSystem) {
        Particle_DestroyASystem(&pParticleSystem, 0.0f);
    }

    pParticleSystem = NULL;
    StaticProp::Deinit();
}

bool Talisman::LoadLine(KromeIniLine* pLine) {
    return StaticProp::LoadLine(pLine) || LoadLevel_LoadBool(pLine, "bVisible", &bVisible);
}

void Talisman::LoadDone(void) {
    StaticProp::LoadDone();
    Reset();
}

void Talisman::Reset(void) {
    GameObject::Reset();
    bCurrentVisible = bVisible; // reset Talisman visibility
}

void Talisman::Update(void) {
    if (!bCurrentVisible) {
        return;
    }
    
    if ((gb.logicGameCount % 4) == 1) {
        Vector vec; // particlePos?
        Vector vec1; // vel?

        Vector colour = {1.0f, 1.0f, 0.0f, 0.0f};

        float randomAngle = ((RandomI(&gb.mRandSeed) % 100) * (2 * PI)) / 100.0f;

        float rand1 = ((RandomI(&gb.mRandSeed) % 100) * 50.0f) / 100.0f;

        vec.Set(_table_sinf(randomAngle) * rand1, RandomI(&gb.mRandSeed) % 5, _table_cosf(randomAngle) * rand1);
        vec1 = vec;
        vec1.Normalise();
        vec1.Scale(15.0f);
        vec1.y = RandomFR(&gb.mRandSeed, 30.0f, 44.0f);
        vec.Add(pModel->matrices[0].Row3());
        
        pParticleSystem->scale = 1.0f;

        Particle_Special_Create(&pParticleSystem, &vec, &vec1, &colour);
    }
}

void Talisman::Draw(void) {
    if (bCurrentVisible) {
        StaticProp::Draw();
    }
}

void Talisman::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Activate:
        case MSG_Show:
            bCurrentVisible = true;
            break;
        case MSG_Deactivate:
        case MSG_Hide:
            bCurrentVisible = false;
            break;
    }
    
    GameObject::Message(pMsg);
}
