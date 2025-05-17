#ifndef PARTICLE_ENGINE_H
#define PARTICLE_ENGINE_H

#include "common/Vector.h"
#include "ty/StructList.h"

struct TyParticleManager {
    void LoadResources(void);
    void Init(void);
    void Deinit(void);

    void SpawnSpark(Vector* pPos);
    void SpawnAnts(Vector*, Vector*, Vector*);
};

#endif // PARTICLE_ENGINE_H
