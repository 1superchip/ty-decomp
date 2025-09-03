#include "ty/TyHealth.h"
#include "ty/soundbank.h"

#include "ty/Ty.h"

void Hud_ShowLives(void);

void VibrateJoystick(float, float, float, char, float);

extern "C" int Sound_IsVoicePlaying(int);

HealthInfo TyHealth::heathInfo[] = {
    {2, 4, 1, 4, 0x10F, -1, -1, "paw_health"},
    {2, 8, 1, 8, 0x21, 0x22, 0x23, "paw_air"},
    {2, 4, 1, 8, -1, -1, -1, "paw_health"}
};

void TyHealth::Init(TyHealthType type) {
    pMaterial = Material::Create("fx_002a");

    bubbleList.Init(20);

    SetType(type);

    unk4 = 0;

    for (int i = 0; i < ARRAY_SIZE(heathInfo); i++) {
        heathInfo[i].unk20 = Material::Create(heathInfo[i].unk1C);
    }

    unk18 = -1;
    unk1C = 0;
    unk20 = 1.0f;
}

void TyHealth::Deinit(void) {
    bubbleList.Deinit();

    pMaterial->Destroy();

    for (int i = 0; i < ARRAY_SIZE(heathInfo); i++) {
        heathInfo[i].unk20->Destroy();
    }
}

bool TyHealth::Hurt(HurtType type) {
    unk4 = 15;

    if (ty.mFsm.GetState() == TY_AS_28 || ty.mFsm.GetState() == TY_AS_29) {
        SoundBank_Stop(&unk18);
    }

    if (type != HURT_TYPE_6 && mType == HEALTH_TYPE_1) {
        heathInfo[mType].unkC = Max<int>(heathInfo[mType].unkC - 2, 0);
    } else {
        heathInfo[mType].unkC = Max<int>(heathInfo[mType].unkC - 1, 0);
    }

    DoLoseFX();

    Hud_ShowLives();

    VibrateJoystick(0.0f, 0.6f, 0.27f, 0, 4.0f);

    return heathInfo[mType].unkC > 0;
}

void TyHealth::Gain(int r4) {
    if (unk18 > -1 && Sound_IsVoicePlaying(unk18)) {
        SoundBank_Stop(&unk18);
    }

    unk4 = 0;

    heathInfo[mType].unkC = Min<int>(heathInfo[mType].unkC + r4, heathInfo[mType].unk4 * heathInfo[mType].unk8);

    Hud_ShowLives();
}

void TyHealth::SetType(TyHealthType type) {
    mType = type;
    unk4 = 0;

    if (ty.mFsm.GetState() == TY_AS_28 || ty.mFsm.GetState() == TY_AS_29) {
        heathInfo[mType].unkC = 0;
    } else {
        heathInfo[mType].unkC = heathInfo[mType].unk8 * heathInfo[mType].unk4;
    }

    Hud_ShowLives();
}

void TyHealth::SetNumSymbols(int r4) {
    for (int i = 0; i < ARRAY_SIZE(heathInfo); i++) {
        heathInfo[i].unk8 = r4;
    }

    SetType(mType);
}

void TyHealth::DrawLooseFX(Vector vec) {

}

void TyHealth::DrawHudElement(Vector vec) {

}

void TyHealth::DoLoseFX(void) {
    if (heathInfo[mType].unk18 >= 0 && heathInfo[mType].unkC == 1) {
        if (unk18 == -1) {
            unk18 = SoundBank_Play(heathInfo[mType].unk18, NULL, 0);
        }
    } else if (heathInfo[mType].unk14 >= 0 && heathInfo[mType].unkC > 0) {
        SoundBank_Play(heathInfo[mType].unk14, NULL, 0);
    }
}
