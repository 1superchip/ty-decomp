#include "ty/props/Aquarang.h"
#include "ty/GameObjectManager.h"

static StaticPropDescriptor aquarangPropDesc;
static ModuleInfo<AquarangProp> aquarangPropModule;

struct UnkLevelInfo {
    Model* pModel;
    char padding0[0x10];
    char padding1[0x24];
};

struct GlobalVar {
    char padding0[0x2B8];
    int randSeed;
    char padding1[0x44];
    UnkLevelInfo levels[8];
    char padding2[0x50];
    int nmbrOfGroundModels;
    char padding3[0xAC];
    Vector color;
    char padding4[0x17C];
    Material* pShadowMat;
};

extern GlobalVar gb;

Material* AquarangProp::pGlow;

void Aquarang_LoadResources(KromeIni* pIni) {
    aquarangPropDesc.Init(&aquarangPropModule, "Prop_0407_AquaRang", "Aquarang", 1, 0);
    aquarangPropDesc.Load(pIni);
    objectManager.AddDescriptor(&aquarangPropDesc);
    AquarangProp::pGlow = Material::Create("FX_100");
}

bool AquarangProp::LoadLine(KromeIniLine* pLine) {
    return StaticProp::LoadLine(pLine) || LoadLevel_LoadBool(pLine, "bVisible", &bVisible);
}

void AquarangProp::LoadDone(void) {
    StaticProp::LoadDone();
    Reset();
}

void AquarangProp::Reset(void) {
    GameObject::Reset();
    bCurrVisible = bVisible;
    particle.color.Set(1.0f, 1.0f, 1.0f, 1.0f);
    particle.unk20 = 60.0f;
    particle.angle = 0.0f;
    randAngle = RandomFR(&gb.randSeed, 0.0f, (2 * PI));
    yaw = 0.0f;
    yawUpdateVal = unk10 & 1 ? 0.05f : -0.05f;
}

void AquarangProp::Update(void) {
    if (!bCurrVisible) return;
    randAngle += 0.1f;
    if (randAngle > (2 * PI)) {
        randAngle -= (2 * PI);
    }
    particle.unk20 = 60.0f + 8.0f * _table_sinf(randAngle);
    particle.pos = *StaticProp::GetPos();
    yaw += yawUpdateVal;
    if (yaw > (2 * PI)) {
        yaw -= (2 * PI);
    } else if (yaw > 0.0f) {
        yaw += (2 * PI);
    }
    pModel->matrices[0].SetRotationYaw(yaw);
}

void AquarangProp::Draw(void) {
    if (bCurrVisible) {
        View::GetCurrent()->SetLocalToWorldMatrix(NULL);
        pGlow->Use();
        particle.Draw(1);
        StaticProp::Draw();
    }
}

void AquarangProp::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case 0xf:
            bCurrVisible = false;
            break;
        case 0xe:
            bCurrVisible = true;
            break;
    }
    GameObject::Message(pMsg);
}
