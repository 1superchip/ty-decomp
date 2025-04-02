#include "ty/ExtendedAnalogControl.h"
#include "common/View.h"
#include "common/Vector.h"
#include "common/StdMath.h"

void ExtendedAnalogControl::Init(float f1, float f2) {
    unk20 = f1;
    unk24 = f2;

    unk28 = 0.0f;

    magnitudeDifference = 0.0f;

    magnitude = 0.0f;
}

void ExtendedAnalogControl::Update(float f1, float f2) {
    float newMag = sqrtf(Sqr<float>(f1) + Sqr<float>(f2));

    float temp_f1 = ((newMag - magnitude) + magnitudeDifference) * 0.5f;

    magnitudeDifference = newMag - magnitude;

    if (temp_f1 < 0.0f && magnitude > 1e-07f) {
        unk8 *= (newMag / magnitude);
    } else if (temp_f1 > 0.0f) {
        unk8 += unk24 * temp_f1 + temp_f1;
    }

    unk8 -= unk20;

    if (unk8 > newMag * unk24) {
        unk8 = newMag * unk24;
    } else if (unk8 < newMag) {
        unk8 = newMag;
    }

    if (unk8 > 1.0f) {
        if (unk28 > Sqr<float>(unk24 - 1.0f) / unk20) {
            unk8 = 1.0f;
        }
    }

    unk28 = Max<float>(0.0f, (unk28 + unk8) - 1.0f);

    if (newMag < 1e-06f) {
        unk0 = unk4 = 0.0f;
    } else {
        unk0 = (f1 * unk8) / newMag;
        unk4 = (f2 * unk8) / newMag;
    }

    magnitude = newMag; // set magnitude to new magnitude

    unkC = f1;
    unk10 = f2;

    float scale = (magnitude + 1.0f) / 2.0f;

    unk18 = f1 * scale;
    unk1C = f2 * scale;
}

