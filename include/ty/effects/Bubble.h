#ifndef BUBBLE_H
#define BUBBLE_H

#include "common/ParticleSystem.h"

/// @brief Bubble particle structure
struct Bubble {
    ParticleSystem* mpSystem;

    static Material* pBubbleMat;
    static ParticleSystemType bubbleType;

    void Init(Vector* pPos, BoundingVolume* pVolume, float f1);
    void Deinit(void);
    void Create(Vector* pPos, float f1, float f2, float f3, float f4);
};

void Bubble_LoadResources(void);
void Bubble_CustomUpdate(ParticleSystem* pSys);

#endif // BUBBLE_H
