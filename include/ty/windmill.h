#ifndef WINDMILL_H
#define WINDMILL_H

#include "common/Collision.h"
#include "common/Model.h"

struct WindmillStruct {
    Model* pModel;
    Vector pos;
    Vector rot;
    char padding24[0x20];
    Vector unk44;
    Vector unk54;
    int state;
    int updateCounter;
    int drawCounter;
    float roll;
    float yaw;
    int bladeIdx;
    int finIdx;
    CollisionInfo collisionInfo;
    int unk8C;
    int zone;
    float lodBlend;
    int unk98;

    static int MaxWindmills;

    void Idle(void);

    Vector* GetModelPos(void) {
        return pModel->matrices[0].Row3();
    }
};

void WindMill_LoadResources(void);

void Windmill_Init(void);
void Windmill_Deinit(void);

WindmillStruct* Windmill_Add(Vector* pPos, Vector* pRot);
void Windmill_Delete(WindmillStruct* bptr);

void Windmill_Update(void);
void Windmill_Draw(void);

#endif // WINDMILL_H
