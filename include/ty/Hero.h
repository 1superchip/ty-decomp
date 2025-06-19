#ifndef HERO_H
#define HERO_H

#include "ty/GameObject.h"
#include "ty/Shadow.h"
#include "ty/effects/RainbowEffect.h"
#include "ty/TyHealth.h"

// Hero might be at least 0x104 bytes long as Ty and Bushpig both have a
// CheckpointStruct pointer at 0x100

struct SpecialPickupStruct;
struct CheckpointStruct;
struct WakeStruct;

struct OpalMagnetData {
    char padding_0x0[0x8];
    Material* unk8;
    char padding[0x30];

    void Init(void);
    void Deinit(void);
    void Reset(void);
    void Draw(void);
    void Activate(void);
    bool IsActive(void);
};

struct GlowParticleData {
    char padding[0x44];

    void Init(void);
    void Deinit(void);
    void Reset(void);
    void Update(void);
    void Activate(Vector, float);
};

struct TySounds {
    char padding[0xA4];

    void Init(void);
    void Reset(void);
    void UpdateSounds(void);
};

// Made up enum for type
enum HeroType {
    HERO_TY         = 0,
    HERO_BUSHPIG    = 1
};

struct Hero : GameObject {
    Model* pReflectionModel;
    Vector pos;
    Vector lastPos; // last safe position
    // fixed velocity vector
    // velocity vector which points towards where ty will go when on contact with collision
    Vector velocity;
    Vector diveVec;
    Shadow mShadow;
    CheckpointStruct* pLastCheckPoint;
    float radius;
    float objectRadiusAdjustment;
    HeroType mType;
    WakeStruct* pWake;
    int unkE8;
    RainbowEffect rainbowEffect;
    int invicibilityFrames;
    TyHealth tyHealth;
    OpalMagnetData opalMagnetData;
    GlowParticleData glowParticleData;
    TySounds tySounds;
    
    virtual bool IsBiting(void) = 0;
    virtual bool InWater(void) = 0;
    virtual bool IsClaiming(void) = 0;
    virtual void SetFindItem(Vector*, SpecialPickupStruct*) = 0;

    bool IsTy(void) {
        return mType == HERO_TY;
    }

    bool IsBushPig(void) {
        return mType == HERO_BUSHPIG;
    }
};

extern Hero* pHero;

#endif // HERO_H
