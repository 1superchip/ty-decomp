#ifndef FLAME_H
#define FLAME_H

#include "common/ParticleSystem.h"

void Flame_LoadResources(void);

/// @brief Flame particle structure
struct Flame {
    ParticleSystem* mpSystem;
    float unk4;

    static ParticleSystemType type;
    static ParticleEnvelope envelope[3];

    void Init(Vector* pPos, BoundingVolume* pVolume, float f1, float f2);
    void Deinit(void);
    void Create(Vector* pDir, float dirLen, Vector* pColor);
    void Reset(void);
};

void Flame_CustomUpdate(ParticleSystem* pSys);

#endif // FLAME_H
