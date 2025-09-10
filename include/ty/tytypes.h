#ifndef TYTYPES_H
#define TYTYPES_H

#include "ty/tools.h"
#include "common/Vector.h"
#include "common/Collision.h"

struct Rotation {
    float unk0;
    float unk4; // yaw?
    float unk8; // roll?
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

    float GetUnk0(void) {
        return unk0;
    }

    float GetUnk4(void) {
        return unk4;
    }

    float GetUnk8(void) {
        return unk8;
    }

    float GetUnkC(void) {
        return unkC;
    }

    float GetUnk10(void) {
        return unk10;
    }

    void UnknownInline(float f1) {
        unk4 = NormaliseAngle(f1);
        unkC = _table_cosf(unk4 - PI/2);
        unk10 = _table_sinf(unk4 - PI/2);
    }

    void SetUnk0(float val) {
        unk0 = val;
    }

    void SetUnk8(float val) {
        unk8 = val;
    }

    void SetRotByVec(Vector* pRot) {
        SetUnk0(pRot->x);
        UnknownInline(pRot->y);
        SetUnk8(pRot->z);
    }

    void IncreaseUnk4(float val) {
        UnknownInline(unk4 + val);
    }
};

#ifndef KEEP_DATA_POOLING

struct UnknownColorVectorStruct {
    Vector* GetColor(void) {
        static Vector colorVec = {0.0f, 0.0f, 0.0f, 0.0f};
        return &colorVec;
    }
};

#endif // KEEP_DATA_POOLING

#endif // TYTYPES_H
