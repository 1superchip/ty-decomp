#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "common/Vector.h"
#include "common/Model.h"
#include "common/ParticleSystem.h"
#include "ty/effects/Bubble.h"
#include "ty/StructList.h"

void Explosion_LoadResources(void);
void Explosion_Update(void);
void Explosion_Deinit(void);
void Explosion_Draw(void);

struct Tendril {
    Vector unk0;
    int refPointIndex;
    Model* pModel;
    float unk18;
    float unk1C;
    float unk20;
    float unk24;
    float unk28;
    float unk2C;
    float unk30;
    float unk34;

    void Setup(float, Vector*);
    void Update(float, Vector*, float);
};

struct Cloud {
    Vector unk0;
    Vector unk10;
    Vector unk20;
    float unk30;
    float unk34;
    Model* pModel;

    void Setup(Vector*, float);
    float GetRandom(float min, float max);
    void Update(float, float);
};

#define NUM_CLOUDS      (6)
#define NUM_TENDRILS    (10)

struct Explosion {
    ParticleSystem* mpSystem;
    Material* pMaterial;
    float unk8;
    Vector mPos;
    Bubble mBubble;
    bool unk20;
    bool unk21;
    float unk24;
    Cloud mClouds[NUM_CLOUDS];
    Model* pExploRing;
    Tendril mTendrils[NUM_TENDRILS];
    int unk3C4;
    float unk3C8;
    float unk3CC;
    float unk3D0;
    float mParticleColor;
    float unk3D8;
    Vector unk3DC;
    bool unk3EC;

    void Init(Vector* pPos, bool, float);
    void Deinit(void);
    void Explode(Vector*, float);
    void Update(void);
    void Draw(void);
};

extern StructList<Explosion*> activeSet;

#endif // EXPLOSION_H
