#ifndef TYTYPES_H
#define TYTYPES_H

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
        // unk4 = NormaliseAngle(f1)
        // unkC = _table_cosf(unk4 - PI/2)
        // unk10 = _table_sinf(unk4 - PI/2)
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

struct UnknownColorVectorStruct {
    Vector* GetColor(void) {
        static Vector colorVec = {0.0f, 0.0f, 0.0f, 0.0f};
        return &colorVec;
    }
};

struct FloorInfo {
    bool bUnderFeet;
    bool bOn;
    bool bValid;
    CollisionResult res;

    float GetDiff(Vector* pPos) {
        return bValid ? pPos->y - res.pos.y : 10000.0f;
    }
};

struct TyContext {
    struct WaterInfo {
        bool bValid;
        Vector pos;
    };

    WaterInfo water;
    FloorInfo floor;
    CollisionResult results[4];
    int resultIndex;

    void Reset(void) {

    }

    float GetYDistanceToWater(Vector* p) {
        return water.pos.y - p->y;
    }

    // was this defined in the header?
    // void VelocityInline(Vector* pVelocity, float f1) {
    //     Vector g = {0.0f, -gb.mDataVal.jumpGravity, 0.0f, 0.0f};
    // }
};

// Ty inherits Hero
// Should be defined in Ty.h
// struct Ty : GameObject {
//     void* pReflectionModel;
//     Vector position;
//     Vector lastPosition;

//     // fixed velocity vector 
//     // which points towards where ty will go when on contact with collision
//     Vector velocity;

//     Vector diveVec;
// };

#endif // TYTYPES_H
