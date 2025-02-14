#ifndef TYTYPES_H
#define TYTYPES_H

#include "common/Vector.h"
#include "ty/GameObject.h"

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
};

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
