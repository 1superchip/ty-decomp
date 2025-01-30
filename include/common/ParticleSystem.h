#ifndef COMMON_PARTICLESYSTEM
#define COMMON_PARTICLESYSTEM

#include "common/Model.h"
#include "common/Material.h"
#include "common/System_GC.h"

struct ParticleSystem;
struct ParticleSystemManager;

struct BaseParticleSystemType {
    void (*updateFunc)(ParticleSystem*);
    char* pName;
    float unk8;
    float unkC;
    Material* mpMat;
    int unk14; // mNumTextures?
    float unk18;
    float unk1C;
    float unk20;
    float unk24;
    float unk28;
    int unk2C; // flags?
    bool unk30;
    float unk34;
    float unk38;
    float unk3C;
    float unk40; // cameraDistThreshold?

    void Init(char* pName, Material* pMat, float, float, float, int);
    void SetDistances(float, float, float);

    bool InfiniteParticles(void) {
        return unk2C & 1;
    }
};

struct ParticleEnvelope {
    float age;
    float unk4;
    float unk8;
    float unkC;
    float deltaAge;
    float unk14;
    float unk18;
};

struct SimpleParticleSystemType : BaseParticleSystemType {
    float unk44;
    float unk48;
    float unk4C;
    int mNumEnvelopes; // numEnvelopeNodes
    ParticleEnvelope* mpEnvelopes;

    void Init(char* pName, Material* pMat, float, float, float, int);
    void SetEnvelope(int numEnvelopes, ParticleEnvelope* pEnvelopes);
    static void Update(ParticleSystem* pSys);
    void CalculateEnvelope(void);
};

struct ParticleSystemType : SimpleParticleSystemType {
    float unk58;
    float unk5C;
    float unk60;
    float unk64;
    float xVel;
    float yVel;
    float zVel;
    
    void Init(char* pName, Material* pMat, float, float, float, int);
    static void Update(ParticleSystem* pSys);
};

struct Particle {
    float mX;
    float mY;
    float mZ;
    float unkC;
    Vector mColor;
    float unk20; // xDir
    float unk24; // yDir
    float unk28; // zDir
    int mEnvelopeIndex;
    float unk30;
    float unk34;
    float mAngle;
    float unk3C;
    float unk40;
    float unk44;
    float unk48;
    float unk4C;
    float unk50;
    float unk54;
    // unk58
    union {
        float floatData[2];
        char charData[2];
    };

    void SetUnk58ArrayByIndex(int index, float f1) {
        floatData[index] = f1;
    }

    float GetUnk58ArrayByIndex(int index) {
        return floatData[index];
    }
};

#define MAX_PARTICLES (24)

/// @brief Container for Particle structs
struct ParticleChunk {
    Particle mChunkData[MAX_PARTICLES];
    int mDataIndex;
    ParticleChunk* mpNext;

    void Init(void) {
        mDataIndex = 24;
        mpNext = NULL;
    }

    bool IsFull(void) {
        return mDataIndex == 0;
    }

    Particle* GetParticleData(void) {
        if (mDataIndex == 0) {
            return NULL;
        }
        return &mChunkData[--mDataIndex];
    }

    bool IsOwnerOf(Particle* p) {
        return p >= &mChunkData[0] && p < &mChunkData[mDataIndex];
    }

    void FreeParticle(Particle* p) {
        *p = mChunkData[mDataIndex++];
    }

    bool IsEmpty(void) {
        return mDataIndex == 24;
    }
};

// Linked List Structure
struct ParticleSystem {

    struct DynamicData {
        float unk0;
        float unk4;
        Matrix* pMatrix;
        float unkC;
        bool unk10;
    };

    ParticleSystemType* mpType;
    DynamicData* pDynamicData;
    int mNumDyn;
    Vector* mpPos; // Position
    BoundingVolume mVolume; // Volume
    float unk30;
    float scale;
    float unk38;
    float unk3C;
    int unk40;
    float uOffset;
    float vOffset;
    float textureIndexOffset;
    ParticleSystemManager* pSystemManager;
    DynamicData dynamicData;
    ParticleChunk* mpChunks;
    int numParticleChunks;
    int numLiveParticles;
    bool unk74; // related to this system being live?
    bool unk75;
    float unk78;
    bool isDying;
    float deathTimer; // Counts down and when less than or equal to zero, deinits this system
    float invDeathTimer;
    float age;
    ParticleSystem* mpNext;
    int mUniqueId; // Unique id of this system

    static Matrix identityMatrix;
    static bool bDrawBoundingBox;
    static bool bDrawPos;
    static bool bPaused;
    static int nextUniqueId;
    static bool bDraw;

    static ParticleSystem* Create(ParticleSystemType* _pType, Vector* _pPos,
        BoundingVolume* pVolume, float volumeScale, int numDyn, ParticleSystemManager* pManager);
    void Destroy(void);
    void Destroy(float);
    void Init(ParticleSystemType* _pType, Vector* pPos, BoundingVolume* pVolume,
        float volumeScale, int numDyn);
    void Deinit(void);
    Particle* AllocateParticle(void);
    Particle* CreateParticle(void);
    void DestroyParticle(Particle*, ParticleChunk**);
    void DestroyAll(void);
    void Update(void);
    void Draw(void);
    void DrawCPU(void);
    void DrawCPUChunk(ParticleSystem::DynamicData*, ParticleChunk*);
    bool IsVisible(ParticleSystem::DynamicData*);
    bool CheckLiveness(void);

    void DestroyAllParticlesCreatedBefore(float f1) {
        ParticleChunk** ppChunks = &mpChunks;
        do {
            ParticleChunk* pCurrChunk = *ppChunks;
            Particle* pParticle = &pCurrChunk->mChunkData[pCurrChunk->mDataIndex];
            do {
                if (f1 >= pParticle->unkC) {
                    DestroyParticle(pParticle, ppChunks);
                }
                pParticle++;
            } while (pParticle < &pCurrChunk->mChunkData[24]);
            if (*ppChunks == pCurrChunk) {
                ppChunks = &(*ppChunks)->mpNext;
            }
        } while (*ppChunks);
    }

    float GetAge(void) {
        return age;
    }
};

#endif // COMMON_PARTICLESYSTEM
