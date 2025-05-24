#ifndef PARTICLE_ENGINE_H
#define PARTICLE_ENGINE_H

#include "common/Model.h"
#include "common/Vector.h"
#include "common/Blitter.h"
#include "common/PtrListDL.h"
#include "ty/StructList.h"
#include "ty/tytypes.h"

enum FootPrintTypes {

};

struct WaterSteamStruct {
    char padding[0x10];
};

struct GhostStruct {
    float unk0;
    int unk4;
    int unk8;
    int unkC;
    int unk10;
    float unk14;
    float unk18;
    float unk1C;
};

struct GhostSmokeStruct {
    float unk0;
    char padding4[0xC];
    int unk10;
};

struct ChompStruct {
    float unk0;
    int unk4;
    int unk8;
    int unkC;
};

struct ShockGlowStruct {
    float unk0;
    int unk4;
    int unk8;
    int unkC;
};

struct SnowFlakeStruct {
    char padding[0x2C];
};

struct AntStruct {
    char padding[0x28];
};

struct SparkStruct {
    char padding[0x24];
};

struct FeatherStruct {
    float time;
    Vector pos;
    Vector unk14;
    Vector rot;
    Vector unk34;

    bool Update(void) {
        rot.Add(&unk34);

        pos.Add(&unk14);
        pos.y -= 0.6f;

        unk14.Scale(0.86f);

        time -= 1.0f / 60.0f;
        return time > 0.0f;
    }

    void Draw(void); // Defined in particleengine.cpp
};

struct LeafGrassDustChunkStruct {
    float time;
    int modelIndex;
    Vector pos;
    Vector unk18;
    Vector rot;
    Vector unk38;

    bool Update(void) {
        rot.Add(&unk38);

        pos.Add(&unk18);

        unk18.y = Clamp<float>(-5.0f, unk18.y - 5.0f, 10.0f);

        unk18.Scale(0.6f);

        time -= 1.0f / 60.0f;
        return time > 0.0f;
    }

    void Draw(void); // Defined in particleengine.cpp
};

// Start of unofficial

struct GooStruct {
    Blitter_Particle mParticle;

    Vector unk30;

    float unk40;
    int unk44;
};

struct WaterRippleStruct {
    float unk0;
    float unk4;
    float angle;
    float x;
    float y;
    float z;
    int unk18;
    Vector unk1C;
    char unk2C;
};

struct FireStruct {
    int unk0;
    float unk4;
};

struct BreathMistStruct {
    float unk0;
    Vector vel;
};

// End of unoffical

struct TyParticleManager {
    Model* pWoodModels[2];

    char unk8[12];

    Model* pTMoundModels[3];

    Vector unk20;
    Vector unk30;

    Material* pFireMaterial;
    Material* unk44;
    Material* unk48;
    Material* pDustMaterial;
    Material* pShockGlowMaterial;
    Material* unk54;
    Material* unk58;
    Material* unk5C;
    Material* unk60;
    Material* unk64;
    Material* pChompMaterial;
    Material* pGhostMaterial; // Ghost Skull
    Material* pGhostSmokeMaterial; // Ghost Smoke

    // Exclamation Mark
    Material* pExclamationMaterial;

    Material* pBilbyAtomMaterial;

    Material* unk7C;
    Material* unk80;
    Material* unk84;
    Material* unk88;
    Material* pGooMaterial;
    Material* unk90;
    Material* unk94;

    Material* unk98[4];

    Material* unkA8;
    Material* unkAC;

    char unkB0[0x18];

    Model* pIceShardModels[2];

    Model* unkD0[3];

    Model* pFeatherModel;

    Model* unkE0;

    float unkE4;

    char unkE8[4];

    int unkEC;

    Vector* pHeroPos;

    float unkHeroFloat; // Based on active hero

    bool unkF8;

    Blitter_Particle exclamationParticle;

    PtrListDL<GooStruct> mGooList;
    PtrListDL<WaterRippleStruct> mRippleList;

    StructList<Blitter_Particle> mFastGlowParticles;

    StructList<Blitter_Particle> mBilbyAtomParticles;

    StructList<FireStruct> mFireData;
    StructList<Blitter_Particle> mFireParticles;

    StructList<float> mTireDustData; // Custom type?
    StructList<Blitter_Particle> mTireDustParticles;

    StructList<BreathMistStruct> mBreathMistData;
    StructList<Blitter_Particle> mBreathMistParticles;

    StructList<ShockGlowStruct> mShockGlowData;
    StructList<Blitter_Particle> mShockGlowParticles;

    StructList<ChompStruct> mChompData;
    StructList<Blitter_Particle> mChompParticles;

    StructList<GhostStruct> mGhostData;
    StructList<Blitter_Particle> mGhostParticles;

    StructList<GhostSmokeStruct> mGhostSmokeData;
    StructList<Blitter_Particle> mGhostSmokeParticles;

    PtrListDL<LeafGrassDustChunkStruct> mLeafList;
    PtrListDL<FeatherStruct> mFeatherList;
    
    void LoadResources(void);
    void Init(void);

    void FreeResources(void);
    void Deinit(void);

    void SpawnSpark(Vector* pPos);
    void SpawnAnts(Vector*, Vector*, Vector*);
    void SpawnChomp(Vector*, float);

    void SpawnExclamation(void);
    void StopExclamation(bool);

    void SpawnGhost(Vector*);
    void SpawnGhostSmoke(Vector*, int);

    void SpawnWaterSteam(Vector*, float);
    void SpawnFastGlow(Vector* pPos, float scale);
    void SpawnBilbyPickupAtom(Vector* pPos, float scale, int color);
    void SpawnTireDust(Vector* pPos, float scale, float alpha);
    void SpawnBreathMist(Vector* pPos, Vector* pVel, float scale);
    void SpawnBridgeChunk(Vector*, Model*);
    void SpawnAntHillChunk(Vector*, Vector*);
    void SpawnLeafGrassDust(Vector*, Vector*, bool);
    void SpawnFeather(Vector*, Vector*);
    void SpawnShockGlow(Vector*, float);

    void SpawnWaterRipple(Vector*, float);
    void SpawnWaterDropGreen(Vector*, Vector*, float);
    void SpawnWaterDropBlue(Vector*, Vector*, float);

    void SpawnBigSplash(Vector*, bool, float, bool, float, int);

    void SpawnSnowFlake(Vector*, Vector*, float, float, int);

    void SpawnFireParticle(Vector* pPos, float scaleFactor);

    void SpawnIceShard(Vector*, Vector*, float);

    void SpawnGoo(Vector* pPos, Vector* pColor);

    void SpawnFootPrint(FootPrintTypes, Vector*, Vector*, Rotation*);

    void SetCamera(Vector*, Vector*);

    void Update(void);

    void DrawPreWater(void);
    void DrawPostWater(void);
    
    void SpawnElectricity(Vector*, float);
};

extern TyParticleManager* particleManager;

#endif // PARTICLE_ENGINE_H
