#ifndef TYHEALTH_H
#define TYHEALTH_H

#include "common/Vector.h"

enum TyHealthType {

};

enum HurtType {
    HURT_TYPE_5 = 5
};

struct HealthInfo {
    char padding[0x24];
};

struct TyHealth {
    char padding[0x24];

    void Init(TyHealthType);
    void Deinit(void);
    bool Hurt(HurtType);
    void Gain(int);
    void SetHealthType(TyHealthType);
    void SetNumSymbols(int);
    void DrawLooseFX(Vector);
    void DrawHudElement(Vector);
    void DoLoseFX(void);

    static HealthInfo heathInfo[3];
};

#endif // TYHEALTH_H
