#ifndef TYTYPES_H
#define TYTYPES_H

#include "common/Vector.h"

struct Rotation {
    float unk0;
    float unk4;
    float unk8;
    float unkC;
    float unk10;

    Vector* GetFrontVector(void) {
        static Vector v;
        v.Set(unkC, 0.0f, unk10);
        return &v;
    }
    Vector* GetRotVector(void) {
        static Vector v;
        v.Set(unk0, unk4, unk8);
        return &v;
    }
};

#endif // TYTYPES_H
