#ifndef TYHEALTH_H
#define TYHEALTH_H

#include "ty/StructList.h"
#include "common/Blitter.h"
#include "common/Material.h"
#include "common/Vector.h"

enum TyHealthType {
    HEALTH_TYPE_0 = 0,
    HEALTH_TYPE_1 = 1,
    HEALTH_TYPE_2 = 2,
};

enum HurtType {
    HURT_TYPE_0 = 0,
    HURT_TYPE_1 = 1,
    HURT_TYPE_2 = 2,
    HURT_TYPE_3 = 3,
    HURT_TYPE_4 = 4,
    HURT_TYPE_5 = 5,
    HURT_TYPE_6 = 6
};

struct HealthInfo {
    int unk0;
    int unk4;
    int unk8;
    int unkC;
    int unk10;
    int unk14; // sound id
    int unk18; // sound id
    char* unk1C;
    Material* unk20;
};

struct TyHealth {
    TyHealthType mType;
    int unk4;
    StructList<Blitter_Image> bubbleList;
    Material* pMaterial;
    int unk18; // sound id
    char unk1C;
    float unk20;

    void Init(TyHealthType);
    void Deinit(void);
    bool Hurt(HurtType);
    void Gain(int);
    void SetType(TyHealthType);
    void SetNumSymbols(int);
    void DrawLooseFX(Vector);
    void DrawHudElement(Vector);
    void DoLoseFX(void);

    float GetHealthFieldUnk4(void) {
        return heathInfo[mType].unk4;
    }

    static HealthInfo heathInfo[];
};

#endif // TYHEALTH_H
