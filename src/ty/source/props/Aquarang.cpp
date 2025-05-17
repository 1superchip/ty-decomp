#include "ty/props/Aquarang.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"

static StaticPropDescriptor aquarangPropDesc;
static ModuleInfo<AquarangProp> aquarangPropModule;

Material* AquarangProp::pGlow = NULL;

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
    particle.scale = 60.0f;
    particle.angle = 0.0f;
    randAngle = RandomFR(&gb.mRandSeed, 0.0f, (2 * PI));
    yaw = 0.0f;

    // ID of object determines which way it spins
    yawUpdateVal = uniqueID & 1 ? 0.05f : -0.05f;
}

void AquarangProp::Update(void) {
    if (!bCurrVisible) {
        return;
    }

    randAngle += 0.1f;

    if (randAngle > (2 * PI)) {
        randAngle -= (2 * PI);
    }

    particle.scale = 60.0f + 8.0f * _table_sinf(randAngle);

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
        case MSG_Hide:
            bCurrVisible = false;
            break;
        case MSG_Show:
            bCurrVisible = true;
            break;
    }
    
    GameObject::Message(pMsg);
}
