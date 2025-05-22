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

};

struct GhostStruct {

};

struct GhostSmokeStruct {

};

struct ShockGlowStruct {

};

struct SnowFlakeStruct {

};

struct AntStruct {

};

struct SparkStruct {

};

struct FeatherStruct {
    void Update(void);
    void Draw(void); // Defined in particleengine.cpp
};

struct LeafGrassDustChunkStruct {
    void Update(void);
    void Draw(void); // Defined in particleengine.cpp
};

struct TyParticleManager {
    Model* pWoodModels[2];

    char unk8[12];

    Model* pTMoundModels[3];

    Vector unk20;
    Vector unk30;

    Material* unk40;
    Material* unk44;
    Material* unk48;
    Material* unk4C;
    Material* unk50;
    Material* unk54;
    Material* unk58;
    Material* unk5C;
    Material* unk60;
    Material* unk64;
    Material* unk68;
    Material* unk6C;
    Material* unk70;
    Material* unk74;
    Material* unk78;
    Material* unk7C;
    Material* unk80;
    Material* unk84;
    Material* unk88;
    Material* unk8C;
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
    void SpawnFastGlow(Vector*, float);
    void SpawnBilbyPickupAtom(Vector*, float, int);
    void SpawnTireDust(Vector*, float, float);
    void SpawnBreathMist(Vector*, Vector*, float);
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

    void SpawnFireParticle(Vector*, float);

    void SpawnIceShard(Vector*, Vector*, float);

    void SpawnGoo(Vector*, Vector*);

    void SpawnFootPrint(FootPrintTypes, Vector*, Vector*, Rotation*);

    void SetCamera(Vector*, Vector*);

    void Update(void);

    void DrawPreWater(void);
    void DrawPostWater(void);
    
    void SpawnElectricity(Vector*, float);
};

extern TyParticleManager* particleManager;

#endif // PARTICLE_ENGINE_H
