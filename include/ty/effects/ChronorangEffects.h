#ifndef CHRONORANG_EFFECTS_H
#define CHRONORANG_EFFECTS_H

#include "common/ParticleSystem.h"

void ChronorangEffects_LoadResources(void);

void Pollen_Update(ParticleSystem* pSys);
void SleepyDust_Update(ParticleSystem* pSys);

struct Pollen {
    ParticleSystem* pSystem;
    
    static Material* pPollenMat;
    static ParticleSystemType pollenType;

    void Init(Vector* pPos);
    void Deinit(void);
    void Spawn(Vector* pPos, Vector* arg2, float f1);
    void SetVel(Vector* pVel);
};

struct SleepyDustEntry {
    Matrix matrix;
    Model* pModel;
    bool unk0;
    bool unk1;
    float yaw;
};

// 10 dynamic particle entries
#define NUM_DUST_ENTRIES (10)

struct SleepyDust {
    ParticleSystem* pSystem;

    char padding0[0x284 - 0x4];

    SleepyDustEntry dustEntries[NUM_DUST_ENTRIES];

    static Material* pSleepyDustMat;
    static ParticleSystemType sleepyDustType;

    void Init(void);
    void Update(void);
};

void SleepyDust_Init(void);
void SleepyDust_Deinit(void);
void SleepyDust_Reset(void);
void SleepyDust_UpdateAll(void);
void SleepyDust_Update(Model* pModel);
void SleepyDust_AddNewUser(Model* pModel);
void SleepyDust_RemoveUser(Model* pModel);

#endif // CHRONORANG_EFFECTS_H
