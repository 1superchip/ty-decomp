
#include "common/ParticleSystemManager.h"
#include "common/View.h"

ParticleSystemManager defaultParticleManager;

bool ParticleSystemManager::menuInit = false;

void ParticleSystemManager::Init(int numParticleSystems, int r5) {
    mChunkList.Init((r5 + 23) / 24, sizeof(ParticleChunk));
    mParticleSysList.Init(numParticleSystems, sizeof(ParticleSystem));
    mpGroups = NULL;
    bDrawDebug = false;

    vu_numParticles = 0;
    vu_numVirtualParticles = 0;
    vu_numChunks = 0;

    unk1C = 0;

    cpu_numParticles = 0;
    cpu_numVirtualParticles = 0;
    cpu_numChunks = 0;

    unk2C = 0;
    numRejectedSystems = 0;
    unk34 = 0.0f;
    minParticles = 0;
    minChunks = 0;
    maxChunks = 0;
    avgParticles = 0.0f;
    avgChunks = 0.0f;
    pManagedTypeInstance = NULL;
    if (!menuInit) {
        menuInit = true;
    }
}

void ParticleSystemManager::Deinit(void) {
    mChunkList.Deinit();
    mParticleSysList.Deinit();

    // Free all groups in the list
    while(mpGroups) {
        ParticleSystemGroup* pCurr = mpGroups;
        mpGroups = mpGroups->pNext;
        Heap_MemFree(pCurr);
    }
}

/// @brief Creates a ParticleSystem and a ParticleSystemGroup system if needed
/// @param pType ParticleSystemType instance for the particle
/// @return New Particle System, NULL if a system can't be created
ParticleSystem* ParticleSystemManager::CreateParticleSystem(ParticleSystemType* pType) {
    // If the ParticleSystem list is full, return NULL
    if (mParticleSysList.IsFull()) {
        return NULL;
    }
    // Find a group based on pType
    ParticleSystemGroup* pGroup = FindGroup(pType);
    if (pGroup == NULL) {
        // if the group is NULL, allocate a new group and add it to the list
        pGroup = (ParticleSystemGroup*)Heap_MemAlloc(sizeof(ParticleSystemGroup));
        if (pGroup == NULL) {
            // if the allocated group is NULL, return NULL
            return NULL;
        }
        // Link the new group to the list
        pGroup->pNext = mpGroups;
        mpGroups = pGroup;
        pGroup->pType = pType;
        pGroup->pSystem = NULL;
    }
    // Get the next ParticleSystem from the list
    ParticleSystem* pSys = mParticleSysList.GetNextEntry();
    if (pSys) {
        // if the system isn't NULL, link it to the group's list
        // and set its manager to this
        pSys->mpNext = pGroup->pSystem;
        pGroup->pSystem = pSys;
        pSys->pSystemManager = this;
    }
    
    return pSys;
}

/// @brief Unlinks a ParticleSystem from a ParticleSystemManager
/// @param pSysToDestroy ParticleSystem to unlink from the manager
void ParticleSystemManager::DestroyParticleSystem(ParticleSystem* pSysToDestroy) {
    ParticleSystemGroup* pGroup = mpGroups;
    // Iterate through all groups
    while (pGroup) {
        // Attempt to unlink the particle system from the current group
        bool bWasUnlinked = pGroup->UnlinkParticleSystem(pSysToDestroy);
        if (bWasUnlinked) {
            // if the system was unlinked from the group
            // remove it from the manager's list of particle systems
            mParticleSysList.Destroy(pSysToDestroy);
            return; // only remove it from one group
            // can it be owned by more than 1 group?
        }
        pGroup = pGroup->pNext;
    }
}

void ParticleSystemManager::RemoveAll(void) {
    // Deinit all ParticleSystems
    ParticleSystem** pSystemList = mParticleSysList.GetMem();
    while (*pSystemList) {
        (*pSystemList)->Deinit();
        pSystemList++;
    }
    // Free all groups
    ParticleSystemGroup* pCurrGroup = mpGroups;
    while (pCurrGroup) {
        ParticleSystemGroup* next = pCurrGroup->pNext;
        Heap_MemFree(pCurrGroup);
        pCurrGroup = next;
    }
    mpGroups = NULL;
    mChunkList.Reset();
    mParticleSysList.Reset();
}

void ParticleSystemManager::Update(void) {
    // Note: the type is addressed based not name based
    ParticleSystem* pSystem;
    if (pManagedTypeInstance == NULL) {
        // if the managed ParticleSystemType is NULL, update every group
        ParticleSystemGroup* pGroup = mpGroups;
        while (pGroup) {
            pSystem = pGroup->pSystem;
            while (pSystem) {
                pSystem->Update();
                pSystem = pSystem->mpNext;
            }
            pGroup = pGroup->pNext;
        }
    } else {
        // if the managed ParticleSystemType isn't NULL
        // iterate through every group and only update systems of groups
        // which have the same ParticleSystemType instance as this manager
        ParticleSystemGroup* pGroup = mpGroups;
        while (pGroup) {
            if (pManagedTypeInstance == pGroup->pType) {
                pSystem = pGroup->pSystem;
                while (pSystem) {
                    pSystem->Update();
                    pSystem = pSystem->mpNext;
                }
                return;
            }
            pGroup = pGroup->pNext;
        }
    }
}

void ParticleSystemManager::Draw(void) {
    vu_numParticles = 0;
    vu_numVirtualParticles = 0;
    vu_numChunks = 0;

    unk1C = 0;

    cpu_numParticles = 0;
    cpu_numVirtualParticles = 0;
    cpu_numChunks = 0;

    unk2C = 0;
    numRejectedSystems = 0;
    minParticles = -1;
    maxParticles = 0;
    minChunks = -1;
    maxChunks = 0;
    avgParticles = 0.0f;
    avgChunks = 0.0f;
    if (ParticleSystem::bDraw) {
        View::GetCurrent()->SetLocalToWorldMatrix(NULL);
        if (pManagedTypeInstance == NULL) {
            // if the managed type instance is NULL
            // update all groups
            ParticleSystemGroup* pCurrGroup = mpGroups;
            while (pCurrGroup) {
                pCurrGroup->Draw();
                pCurrGroup = pCurrGroup->pNext;
            }
        } else {
            ParticleSystemGroup* pCurrGroup = mpGroups;
            while (pCurrGroup) {
                // Only update the group which has this particlemanager's managed
                // type instance
                if (pManagedTypeInstance == pCurrGroup->pType) {
                    pCurrGroup->Draw();
                    break;
                }
                pCurrGroup = pCurrGroup->pNext;
            }
        }
    }

    if (bDrawDebug) {
        float yPos = 100.0f;
        DrawDebug(50.0f, &yPos);
    }
}

/// @brief Gets a new ParticleChunk from this Manager's list
/// @param  None
/// @return Pointer to a ParticleChunk, NULL if list is full
ParticleChunk* ParticleSystemManager::AllocParticleChunk(void) {
    ParticleChunk* pChunk = NULL;
    if (!mChunkList.IsFull()) {
        pChunk = mChunkList.GetNextEntry();
        if (pChunk) {
            pChunk->Init();
        }
    }
    return pChunk;
}

void ParticleSystemManager::FreeParticleChunk(ParticleChunk* pChunk) {
    mChunkList.Destroy(pChunk);
}

/// @brief Finds a ParticleSystemGroup by a ParticleSystemType instance
/// @param pType Instance of ParticleSystemType to filter by
/// @return NULL if no group was found, otherwise a pointer to a ParticleSystemGroup
ParticleSystemGroup* ParticleSystemManager::FindGroup(ParticleSystemType* pType) {
    ParticleSystemGroup* pGroup = mpGroups;
    while (pGroup) {
        if (pGroup->pType->mpMat == pType->mpMat && pGroup->pType == pType) {
            return pGroup;
        }
        pGroup = pGroup->pNext;
    }
    return NULL;
}

/// @brief Unused Debugging function for ParticleSystemManager
/// @param xPos X Position of Text 
/// @param yPos Pointer to Y Position of Text, the float at yPos will be updated
void ParticleSystemManager::DrawDebug(float xPos, float* yPos) {

}

/*
#include "common/Debug.h"
void ParticleSystemManager::DrawDebug(float xPos, float* yPos) {
    gpDebugFont->DrawText(
        Str_Printf("numChunks: %d", mChunkList.GetSize()),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("numSystems: %d", mParticleSysList.GetSize()),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("CPU -- numParticles: %d, numVirtualParticles: %d, numChunks: %d",
            unk20, numVirtualParticles, numChunks),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("VU -- numParticles: %d, numVirtualParticles: %d, numChunks: %d",
            unk14, unk18, unk1C),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("VU -- numCodeBytes: %d", unk34),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("numRejectedSystems: %d", numRejectedSystems),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("paused: %d", ParticleSystem::bPaused),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("minParticles: %d, maxParticles: %d, avgParticles: %g",
            minParticles, maxParticles, avgParticles),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
    gpDebugFont->DrawText(
        Str_Printf("minChunks: %d, maxChunks: %d, avgChunks: %g", 
            minChunks, maxChunks, avgChunks),
        xPos, *yPos, 1.0f, 1.0f, (FontJustify)0, 0x80ffffff
    );
    *yPos += 20.0f;
}
*/
