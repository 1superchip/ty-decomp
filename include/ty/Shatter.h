#ifndef SHATTER_H
#define SHATTER_H

#include "common/PtrListDL.h"
#include "common/Model.h"

struct PieceStruct {

};

struct ShatterStruct {
    char padding[0x24];
    Model* pModel;
    char padding1[0x58 - 0x28];

    void Update(void);
    void Draw(void);
    void SetFragInfo(void);
    void Explode(Vector*, float, float);
    void Fall(void);
    void SetHitPlane(PieceStruct*);
    float GetRadius(PieceStruct*);
    void SetInitialGroundPlane(void);
    void Reset(void);
    void Delete(void);
};

void Shatter_Init(void);
void Shatter_Deinit(void);

ShatterStruct* Shatter_Add(Model*, float, float, int);

void Shatter_Update(void);
void Shatter_Draw(void);

#endif // SHATTER_H
