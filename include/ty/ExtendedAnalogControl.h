#ifndef EXTENDEDANALOGCONTROL_H
#define EXTENDEDANALOGCONTROL_H

#include "types.h"

struct ExtendedAnalogControl {
    float unk0; // normalized
    float unk4; // normalized

    float unk8;

    float unkC;
    float unk10;

    float magnitude;

    float unk18;
    float unk1C;

    float unk20;
    float unk24;

    float unk28;
    float magnitudeDifference;

    void Init(float, float);
    void Update(float, float);
    void Draw(void);

    void Reset(void) {
        unk18 = unk1C               = 0.0f;
        unk28 = magnitudeDifference = 0.0f;
        unk10 = magnitude           = 0.0f;
        unk0  = unk4 = unk8 = unkC  = 0.0f;
    }
};

#endif // EXTENDEDANALOGCONTROL_H
