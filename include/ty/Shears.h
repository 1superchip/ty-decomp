#ifndef SHEARS_H
#define SHEARS_H

#include "common/Model.h"
#include "common/Collision.h"

struct ShearsLoadInfo {
    Vector unk0;
    Vector unk10;

    int unk20;
    int unk24;
    int unk28;
};

struct ShearBlade {
    bool bRefPointExists : 1;
    bool b1 : 1;

    int unk4;
    Vector unk8;
};

struct ShearsStruct {
    ShearBlade blades[5];
    Vector unk78;
    Vector unk88;
    Vector unk98;
    Vector unkA8;
    Model* unkB8;
    Model* unkBC;
    int unkC0;
    int unkC4;
    int unkC8;
    int unkCC;
    int unkD0;
    int unkD4;
    int unkD8;
    int unkDC;
    int unkE0;
    int unkE4;
    float unkE8;
    int unkEC;
    CollisionInfo mCollisionInfo;
    
    void Logic(void);
    void CheckForTy(void);
    void Load(ShearsLoadInfo* pLoadInfo);
    void Reset(void);
};

void Shears_LoadResources(void);

void Shears_Init(void);
void Shears_Deinit(void);

ShearsStruct* Shears_Add(ShearsLoadInfo* pLoadInfo);

void Shears_Update(void);

void Shears_Draw(void);

#endif // SHEARS_H
