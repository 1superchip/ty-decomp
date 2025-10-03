#include "types.h"
#include "ty/global.h"
#include "ty/GameData.h"
#include "common/Heap.h"
#include "common/StdMath.h"
#include "ty/props/gem.h"
#include "common/View.h"
#include "ty/Ty.h"

// EXTERNS
void Hud_SetGems(int);
void Hud_ShowCogs(void);
SpecialPickupStruct* GetThunderEgg(ThunderEggType);
SpecialPickupStruct* GetGoldenCog(GoldenCogType);
void Basket_PlaceUnused(Vector*);
void Stopwatch_Enable(bool);
void Stopwatch_Show(void);
void Hud_SetGems(int);
void Hud_ShowOpals(void);
void Crate_CheckGems(void);
void TyMemCard_AutoSaveGame(void);
void Bilby_SetRescued(BilbyType, bool);
extern "C" void Sound_SetSystemAudioMode(int);
extern "C" int Sound_GetSystemAudioMode(void);
View* GameCamera_View(void);
void Hud_ShowLives(void);
void Hud_ShowBilbies(void);
extern "C" void memset(void*, int, int);
// End EXTERNS

static const int GAMEDATA_VERSION = 0x60000 |
    VersionNumber[0] | VersionNumber[1] << 1 | VersionNumber[2] << 2 | VersionNumber[3] << 3;
static SaveData* gameData;
GameDataSettings* pGameSettings;

void GameData_Init(void) {
    SaveData* pGameData = (SaveData*)Heap_MemAlloc(sizeof(SaveData));
    gameData = pGameData;
    pGameSettings = &pGameData->settings;
    GameData_New();
}

void GameData_New(void) {
    memset(gameData, 0, sizeof(SaveData));

    gameData->zoneInfo[ZN_0].bUnlocked = true;
    gameData->zoneInfo[ZN_1].bUnlocked = true;

    gameData->currentLevel = LN_RAINBOW_CLIFFS;
    gameData->previousLevel = LN_RAINBOW_CLIFFS;

    gameData->currentZone = ZN_0;

    gameData->currentRang = BR_Standard;

    gameData->tyAttributes.bHasRangs[BR_Standard] = true;

    gameData->previousLevel = TOTAL_LEVEL_MAX;

    gameData->unkADA[0] = false;

    gameData->settings.musicVolume = 7;
    gameData->settings.soundVolume = 10;
    gameData->settings.unk2 = 1;
    gameData->settings.unk3 = 0;
    gameData->settings.unk4 = 0;
    gameData->settings.unk5 = 0;
    gameData->settings.controlConfig = 0;
    gameData->settings.unk6 = 1;
    gameData->settings.unk7 = 1;
    gameData->settings.unk9 = 1;

    gameData->size = sizeof(SaveData);

    gameData->version = GAMEDATA_VERSION;

    gameData->prevLevelNumber1 = gameData->previousLevel;
}

void GameData::Init(void) {
    pSaveData = gameData;
    memset(&time, 0, sizeof(TimerInfo));
    time.year = -1;
    pSaveData->previousLevel = TOTAL_LEVEL_MAX;
}

void GameData::SynchroniseEnterLevel(void) {
    SetChargeBites(0);
    SetChargeBites(gb.mNumChargeBites);
    
    for (int i = 0; i < Total_ThunderEggs; i++) {
        SpecialPickupStruct* pEgg = GetThunderEgg((ThunderEggType)i);
        if (pSaveData->levels[pSaveData->currentLevel].thunderEggs[i] && pEgg) {
            // Set collected thunder eggs transparent
            pEgg->SetTransparent(true);
            // Hardcoded index for Time Attack ThunderEgg
            if (i == 3) {
                // If the index is 3, enable stopwatch
                Stopwatch_Enable(true);
                Stopwatch_Show();
            }
        }
    }
    
    for (int i = 0; i < Total_Cogs; i++) {
        SpecialPickupStruct* pCog = GetGoldenCog((GoldenCogType)i);
        // if cog for index is collected and Cog pointer isn't NULL
        // Set the cog collected and place a Picnic Basket
        if (pSaveData->levels[pSaveData->currentLevel].cogs[i] && pCog) {
            pCog->SetCollected(true);
            char* unkPtr = (char*)pCog->unk0[1];
            Basket_PlaceUnused((Vector*)((char*)unkPtr + 0x5C));
        }
    }
    
    for (int i = 0; i < Total_Bilbies; i++) {
        bool rescued = ((pSaveData->levels[pSaveData->currentLevel].bilbies[i] & 2) && (GetFreeBilbyCount() == 5))
            && !pSaveData->levels[pSaveData->currentLevel].thunderEggs[1];
        if (pSaveData->levels[pSaveData->currentLevel].bilbies[i] & 1) {
            Bilby_SetRescued((BilbyType)i, rescued);
        }
    }

    if (!pSaveData->levels[pSaveData->currentLevel].thunderEggs[0]) {
        // if Opal ThunderEgg isn't collected, synchronize opals that have already been collected
        SetCollectedGems();
    } else {
        numCollectedGems = 0;
        Hud_SetGems(numCollectedGems);
    }

    Crate_CheckGems();

    SetHasExtraHealth(pSaveData->tyAttributes.bHasExtraHealth);

    pSaveData->lives = Max<s16>(5, pSaveData->lives);

    if (pSaveData->previousLevel != TOTAL_LEVEL_MAX) {
        TyMemCard_AutoSaveGame();
    }

    SetHasRang(BR_Doomerang, false);
    SoundBank_SetVolume(1.0f, SOUND_VOLUME_SET | MUSIC_VOLUME_SET);
    Tools_EnableWideScreen(GameCamera_View(), (pGameSettings->unk5 == 1) ? true : false);
    float char3 = (float)pGameSettings->unk3;
    float char4 = (float)pGameSettings->unk4;
    System_SetScreenOffset(char3 / 16.0f, char4 / 16.0f);
    if (Sound_GetSystemAudioMode()) {
        Sound_SetSystemAudioMode(pGameSettings->unk2 ? 1 : 2);
    }
}

void GameData::SynchroniseExitLevel(void) {
    GetCollectedGems(); // Synchronize savedata with collected gems in the level 
}

SaveData* GameData::GetSaveData(void) {
    unk0 = true;
    StopTime();
    GetCollectedGems();
    gameData->prevLevelNumber1 = gameData->currentLevel;
    gameData->completePercent = GetGameCompletePercent();
    gameData->thunderEggCount = GetTotalThunderEggCount();
    return pSaveData;
}

void GameData::ReleaseSaveData(void) {
    unk0 = false;
    SetDirty(false);
    StartTime();
}

void GameData::SetDirty(bool dirty) {
    bIsDirty = dirty;
}

void GameData::SetCurrentLevel(LevelNumber level) {
    if (pSaveData) {
        pSaveData->previousLevel = pSaveData->currentLevel;
        pSaveData->currentLevel = level;
        pSaveData->levels[pSaveData->currentLevel].nmbrOfTimesEntered++;
    }
}

void GameData::SetCurrentZone(ZoneNumber zone) {
    if (pSaveData) {
        pSaveData->currentZone = zone;
    }
}

void GameData::SetLearntToSwim(bool learntToSwim) {
    pSaveData->tyAttributes.bLearntToSwim = learntToSwim;
    SetDataDirty(true);
}

void GameData::SetLearntToDive(bool learntToDive) {
    pSaveData->tyAttributes.bLearntToDive = learntToDive;
    SetDataDirty(true);
}

void GameData::SetBothRangs(bool bHasBothRangs) {
    pSaveData->tyAttributes.bBothRangs = bHasBothRangs;
    SetDataDirty(true);
}

void GameData::SetHasExtraHealth(bool bExtraHealth) {
    pSaveData->tyAttributes.bHasExtraHealth = bExtraHealth;

    if (bExtraHealth) {
        ty.tyHealth.SetNumSymbols(2);
    } else {
        ty.tyHealth.SetNumSymbols(1);
    }

    bIsDirty = true;
}

void GameData::SetHasRang(BoomerangType type, bool hasRang) {
    pSaveData->tyAttributes.bHasRangs[type] = hasRang;
    SetDataDirty(true);
}

// SetHasThunderEgg, SetHasGoldenCog, SetHasTalisman may have been placed here

void GameData::SetBossDefeated(ZoneNumber zone, bool bossDefeated) {
    pSaveData->zoneInfo[zone].bZoneCompleted = bossDefeated;
    
    SetDataDirty(true);

    TalismanType index = gb.level.GetTalismanType(pSaveData->currentLevel);
    SetHasTalisman(index, true);

    index = gb.level.GetTalismanType(pSaveData->currentLevel);
    SetHasPlacedTalisman(index, true);
}

void GameData::SetZoneUnlocked(ZoneNumber zone, bool unlocked) {
    pSaveData->zoneInfo[zone].bUnlocked = unlocked;
    SetDataDirty(true);
}

void GameData::SetZoneTalismanLocated(ZoneNumber zone, bool bLocated) {
    pSaveData->zoneInfo[zone].unk2 = bLocated;
    SetDataDirty(true);
}

void GameData::ReceiveOpalLifeup(void) {
    bOpalLifeup = true;
}

void GameData::CollectGem(bool arg0) {
    if (!arg0) {
        numCollectedGems++;
    }

    numChargeBites++;

    if (numCollectedGems == GEMS_MAXOPALS) {
        // play a sound when all opals have been collected
        SoundBank_Play(0x1bb, NULL, 0);
    }

    Hud_ShowOpals();
}

// Synchronizes opals for already collected gems in the savedata
void GameData::SetCollectedGems(void) {
    int gemCount = 0;
    u8* pGems = pSaveData->levels[pSaveData->currentLevel].gemArray;
    int totalGems = Min<int>(Gem::totalGems, GEMS_MAXOPALS);

    for (int i = 0; i < totalGems; i++) {
        if (CheckArrayByBitIndex(pGems, i) && Gem::gemPtrList[i]) {
            Gem::gemPtrList[i]->mCollected = true;
            Gem::gemPtrList[i]->SetState(GEMSTATE_5);
            gemCount++;
        }
    }

    numCollectedGems = gemCount;
    Hud_SetGems(numCollectedGems);
}

// Synchronizes the savedata for already collected gems in the current level
void GameData::GetCollectedGems(void) {
    u8* pGems = pSaveData->levels[pSaveData->currentLevel].gemArray;

    memset(pGems, 0, sizeof(pSaveData->levels[pSaveData->currentLevel].gemArray));

    int totalGems = Min<int>(Gem::totalGems, GEMS_MAXOPALS);

    for (int i = 0; i < totalGems; i++) {
        Gem* pGem = Gem::gemPtrList[i];
        if (pGem->mState == GEMSTATE_5 || pGem->mState == GEMSTATE_4 || pGem->mCollected) {
            pGems[i >> 3] |= 1 << (i & 7);
        }
    }

}

int GameData::GetGameCompletePercent(void) {
    int total = GetTotalGoldenCogCount() + GetTotalThunderEggCount() + GetTotalTalismanCount();
    return Min<int>(100, (total * 100) / 167);
}

/// @brief Gets the number of cogs collected within a single level
/// @param level Level number to calculate cog count
/// @return Total cogs collected in the level of the level parameter
int GameData::GetGoldenCogCount(LevelNumber level) {
    int count = 0;

    for (int i = 0; i < Total_Cogs; i++) {
        if (pSaveData->levels[level].cogs[i]) {
            count++;
        }
    }

    return count;
}

/// @brief Counts the number of total collected cogs
/// @param  None
/// @return Total cogs collected in all levels
int GameData::GetTotalGoldenCogCount(void) {
    int count = 0;

    for (int i = 0; i < Total_Levels; i++) {
        for (int j = 0; j < Total_Cogs; j++) {
            if (pSaveData->levels[i].cogs[j]) {
                count++;
            }
        }
    }

    return count;
}

int GameData::GetTotalTalismanCount(void) {
    int count = 0;

    for (int i = 0; i < Total_Talismans; i++) {
        if (pSaveData->bHasTalismans[i] != false) {
            count++;
        }
    }

    return count;
}

int GameData::GetCollectedGemCount(LevelNumber level) {
    if (level == pSaveData->currentLevel) {
        return numCollectedGems;
    }

    int count = 0;
    u8* gemArray = pSaveData->levels[level].gemArray;

    for (int i = 0; i < GEMS_MAXOPALS; i++) {
        if (CheckArrayByBitIndex(gemArray, i) != false) {
            count++;
        }
    }

    return count;
}

int GameData::GetFreeBilbyCount(void) {
    int count = 0;

    for (int i = 0; i < Total_Bilbies; i++) {
        if (pSaveData->levels[pSaveData->currentLevel].bilbies[i] & 1) {
            count++;
        }
    }

    return count++;
}

int GameData::GetTotalFreeBilbyCount(void) {
    int count = 0;

    for (u32 i = 0; i < Total_Levels; i++) {
        for (int j = 0; j < Total_Bilbies; j++) {
            if (IsBilbyFree((LevelNumber)i, j)) {
                count++;
            }
        }
    }

    return count;
}

bool GameData::SetHasGalleryImage(int nIndex) {
    // (nIndex >> 3) computes what byte should be accessed (nIndex / 8)
    // (1 << (nIndex & 7)) computes bit index for bit
    pSaveData->galleryImages[nIndex >> 3] |= (1 << (nIndex & 7));
    return false;
}

int GameData::GetHasGalleryImage(int nIndex) {
    // (nIndex >> 3) computes what byte should be accessed (nIndex / 8)
    // (1 << (nIndex & 7)) computes bit index for bit
    return (1 << (nIndex & 7)) & pSaveData->galleryImages[nIndex >> 3];
}

int GameData::GetTotalGalleryCount(void) {
    int count = 0;
    u8* galleryArray = pSaveData->galleryImages;

    for (int i = 0; i < TOTAL_GALLERYIMAGES; i++) {
        if (CheckArrayByBitIndex(galleryArray, i)) {
            count++;
        }
    }

    return count;
}

void GameData::CollectCog(GoldenCogType cogType) {
    SetHasGoldenCog(cogType, true);
    Hud_ShowCogs();
    SetDataDirty(true);
}

/// @brief Counts the number of collected thundereggs of a certain type
/// @param type Element type of level to consider
/// @return Number of thundereggs collected
int GameData::GetThunderEggCount(ElementType type) {
    int count = 0;

    for (int i = 0; i < Total_Levels; i++) {
        if (gb.level.GetElementType((LevelNumber)i) == type) {
            for (int j = 0; j < Total_ThunderEggs; j++) {
                if (pSaveData->levels[i].thunderEggs[j]) {
                    count++;
                }
            }
        }
    }

    return count;
}

/// @brief Counts the number of collected thundereggs
/// @param  None
/// @return Number of thundereggs collected
int GameData::GetTotalThunderEggCount(void) {
    int count = 0;

    for (int i = 0; i < Total_Levels; i++) {
        for (int j = 0; j < Total_ThunderEggs; j++) {
            if (pSaveData->levels[i].thunderEggs[j]) {
                count++;
            }
        }
    }
    
    return count;
}

void GameData::CollectThunderEgg(ThunderEggType type) {
    SetHasThunderEgg(type, true);
    SetDataDirty(true);
}

void GameData::SetHasPlacedTalisman(TalismanType type, bool bPlaced) {
    pSaveData->bHasPlacedTalisman[type] = bPlaced;
}

void GameData::UnlockZone(ZoneNumber zone) {
    SetZoneUnlocked(zone, true);
    SetDataDirty(true);
}

void GameData::SetTimeAttackBest(int bestTime) {
    pSaveData->levels[pSaveData->currentLevel].bestTimeAttack = bestTime;
    SetDataDirty(true);
}

void GameData::SetTimeAttackLastTime(int lastTime) {
    pSaveData->levels[pSaveData->currentLevel].lastTimeAttackTime = lastTime;
    SetDataDirty(true);
}

int GameData::GetTimeAttackBest(LevelNumber level) {
    return pSaveData->levels[level].bestTimeAttack;
}

int GameData::GetTimeAttackLastTime(LevelNumber level) {
    return pSaveData->levels[level].lastTimeAttackTime;
}

void GameData::RescueBilby(BilbyType type) {
    pSaveData->levels[pSaveData->currentLevel].bilbies[type] |= 1;

    if (GetFreeBilbyCount() == 5) {
        pSaveData->levels[pSaveData->currentLevel].bilbies[type] |= 2;
    }

    Hud_ShowBilbies();
    bIsDirty = true;
}

void GameData::AddLife(void) {
    SetLives(GetLives() + 1);
    Hud_ShowLives();
    SetDataDirty(true);
}

void GameData::LoseLife(void) {
    if (!gb.bE3) {
        SetLives(GetLives() - 1);
    }

    Hud_ShowLives();
    SetDataDirty(true);
}

void GameData::RestoreLives(void) {
    SetLives(5);
}

void GameData::AddChargeBite(void) {
    numChargeBites += 100;
}

void GameData::StartTime(void) {
    Timer_GetSystemTime(&time);
}

void GameData::StopTime(void) {
    TimerInfo timeDiff;
    TimerInfo currTime;

    if (time.year != -1) {
        Timer_GetSystemTime(&currTime);
        Timer_GetDifference(&timeDiff, &time, &currTime);
        pSaveData->totalPlayTime += Timer_GetDHMSInSeconds(&timeDiff);
    }
}

int GameData::GetTotalTime(void) {
    TimerInfo timeDiff;
    TimerInfo currTime;
    if (!gb.bOnPauseScreen) {
        Timer_GetSystemTime(&currTime);
        Timer_GetDifference(&timeDiff, &time, &currTime);
        pSaveData->totalPlayTime += Timer_GetDHMSInSeconds(&timeDiff);
        time = currTime;
    }

    return pSaveData->totalPlayTime;
}

bool GameData::GetHasBeenTriggered(uint uniqueID) {
    for (int i = 0; i < Total_Triggers; i++) {
        if (pSaveData->levels[pSaveData->currentLevel].triggers[i] == uniqueID) {
            return true;
        }
    }

    return false;
}

void GameData::SetHasBeenTriggered(uint uniqueID) {
    for (int i = 0; i < Total_Triggers; i++) {
        if (pSaveData->levels[pSaveData->currentLevel].triggers[i] == 0) {
            pSaveData->levels[pSaveData->currentLevel].triggers[i] = uniqueID;
            return;
        }
    }
}

void GameData::SetCurrentRang(BoomerangType type) {
    pSaveData->currentRang = type;
    bIsDirty = true;
}

// Seperate function to reorder floats
static float GetFloatMusicVolume(u8 volume) {
    return (float)volume;
}

float GameData::GetMusicVolume(void) {
    float volume = GetFloatMusicVolume(pSaveData->settings.musicVolume);
    float div = bMusicDucked ? 20.0f : 10.0f;
    return volume / div;
}

float GameData::GetSoundVolume(void) {
    return pSaveData->settings.soundVolume / 10.0f;
}

void GameData::SetMusicDucked(bool bDucked) {
    bMusicDucked = bDucked;
}
