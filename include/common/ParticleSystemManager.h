#ifndef COMMON_PARTICLESYSTEMMANAGER
#define COMMON_PARTICLESYSTEMMANAGER

#include "common/ParticleSystem.h"
#include "common/PtrListDL.h"

/// @brief Container for ParticleSystem structs in a ParticleManager
/// Groups are found by the ParticleSystemType instance and ParticleSystemType->mpMat instance
struct ParticleSystemGroup {
    ParticleSystemType* pType;
    ParticleSystem* pSystem;
    ParticleSystemGroup* pNext;

    /// @brief Unlinks a ParticleSystem from this group's ParticleSystem list
    /// @param pSysToUnlink Pointer to instance of system to unlink
    /// @return Whether the pSysToUnlink was unlinked or not 
    bool UnlinkParticleSystem(ParticleSystem* pSysToUnlink) {
        ParticleSystem* pCurrSys = pSystem;
        ParticleSystem* pPrevSys = NULL;
        while (pCurrSys) {
            if (pSysToUnlink == pCurrSys) {
                // if the current system is the system to destroy
                // Unlink it from the list
                if (pPrevSys != NULL) {
                    // if the previous system is not null
                    // pSysToDestroy is not the first entry in the group's ParticleSystem list
                    // so unlink pSysToDestroy from the previous system
                    pPrevSys->mpNext = pCurrSys->mpNext;
                } else {
                    // Otherwise unlink it from the group
                    // Since it is the first entry in the system list
                    pSystem = pCurrSys->mpNext;
                }
                // if the system to unlink was found in this group's list
                // then it has been unlinked from the list at the point
                // return true saying the system was unlinked from this group
                return true;
            }
            // set the previous system to the current
            pPrevSys = pCurrSys;
            pCurrSys = pCurrSys->mpNext;
        }
        // the group's system list did not contain the target ParticleSystem instance
        return false;
    }

    void Draw(void) {
        ParticleSystem* pCurrSys = pSystem;
        if (pCurrSys) {
            pCurrSys->mpType->unk30 = 0;
            while (pCurrSys) {
                pCurrSys->Draw();
                pCurrSys = pCurrSys->mpNext;
            }
        }
    }
};

struct ParticleSystemManager {
    ParticleSystemGroup* mpGroups;
    PtrListDL<ParticleChunk> mChunkList;
    PtrListDL<ParticleSystem> mParticleSysList;
    bool bDrawDebug;

    // VU fields
    int vu_numParticles; // VU numParticles
    int vu_numVirtualParticles; // VU numVirtualParticles
    int vu_numChunks; // VU numChunks
    
    int unk1C;

    // CPU fields
    int cpu_numParticles;
    int cpu_numVirtualParticles;
    int cpu_numChunks;

    int unk2C;
    int numRejectedSystems;
    int unk34;
    uint minParticles;
    uint maxParticles;
    uint minChunks;
    uint maxChunks;
    float avgParticles;
    float avgChunks;

    // ParticleSystemType instance which this manager manages
    // Set to NULL to manage every type this manager has in groups
    ParticleSystemType* pManagedTypeInstance;

    static bool menuInit;

    void Init(int numParticleSystems, int);
    void Deinit(void);
    ParticleSystem* CreateParticleSystem(ParticleSystemType*);
    void DestroyParticleSystem(ParticleSystem*);
    void RemoveAll(void);
    void Update(void);
    void Draw(void);
    ParticleChunk* AllocParticleChunk(void);
    void FreeParticleChunk(ParticleChunk*);
    ParticleSystemGroup* FindGroup(ParticleSystemType*);
    void DrawDebug(float xPos, float* yPos);
};

extern ParticleSystemManager defaultParticleManager;

inline void ParticleSystemManager_Init(int r3, int r4) {
    defaultParticleManager.Init(r3, r4);
}

inline void ParticleSystemManager_Deinit(void) {
    defaultParticleManager.Deinit();
}

inline void ParticleSystemManager_Update(void) {
    defaultParticleManager.Update();
}

inline void ParticleSystemManager_Draw(void) {
    defaultParticleManager.Draw();
}

#endif // COMMON_PARTICLESYSTEMMANAGER
