#ifndef RAINBOW_EFFECT_H
#define RAINBOW_EFFECT_H

#include "common/ParticleSystem.h"
#include "ty/Spline.h"

void RainbowEffect_LoadResources(void);

void RainbowEffect_Update(ParticleSystem* pSys);

struct RainbowEffectSubStruct {
    Spline spline;
    bool unk4;
    int unk8;
    int unkC;
    float unk10[3];
    float particleColor[3]; // RGB
};

#define NUM_RAINBOWEFFECT_SUBS (4)

struct RainbowEffect {
    float unk0;
    float unk4;
    float unk8;
    float unkC;
    float unk10;
    ParticleSystem* pSys;
    RainbowEffectSubStruct subs[NUM_RAINBOWEFFECT_SUBS];

    static Material* pRainbowMat;
    static ParticleSystemType rainbowEffectType;

    void Init(Vector* pPos);
    void Deinit(void);
    void Reset(void);
    void Update(float f1);
    void Spawn(Vector* pPos, Vector* pColor);
};

#endif // RAINBOW_EFFECT_H
