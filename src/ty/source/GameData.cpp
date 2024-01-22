#include "types.h"
#include "ty/global.h"
#include "ty/GameData.h"
#include "common/Heap.h"
#include "common/StdMath.h"
#include "ty/props/gem.h"
#include "common/View.h"

// EXTERNS
void Hud_SetGems(int);
void Hud_ShowCogs(void);
struct SpecialPickupStruct {
    int unk0[2];
    void SetTransparent(bool);
    void SetCollected(bool);
};
SpecialPickupStruct* GetThunderEgg(ThunderEggType);
SpecialPickupStruct* GetGoldenCog(GoldenCogType);
void Basket_PlaceUnused(Vector*);
void Stopwatch_Enable(bool);
void Stopwatch_Show(void);
void Hud_SetGems(int);
void SoundBank_Play(int, Vector*, uint);
void Hud_ShowOpals(void);
void Crate_CheckGems(void);
void TyMemCard_AutoSaveGame(void);
void Bilby_SetRescued(BilbyType, bool);
extern "C" void Sound_SetSystemAudioMode(int);
extern "C" int Sound_GetSystemAudioMode(void);
void SoundBank_SetVolume(float, int);
View* GameCamera_View(void);
void Hud_ShowLives(void);
void Hud_ShowBilbies(void);
struct TyHealth {
    char unk[0x10];
    void SetNumSymbols(int);
};
extern struct Ty {
    char unk[0x1d8];
    TyHealth health;
} ty;
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
    gameData->zoneInfo[0].bUnlocked = true;
    gameData->zoneInfo[1].bUnlocked = true;
    gameData->levelAB0 = 0;
    gameData->levelAB4 = 0;
    gameData->currentZone = 0;
    gameData->currentRang = 0;
    gameData->tyAttributes.bHasRangs[0] = true; // Boomerang
    gameData->levelAB4 = 0x18;
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
    gameData->prevLevelNumber1 = gameData->levelAB4;
}

void GameData::Init(void) {
    pSaveData = gameData;
    memset(&time, 0, sizeof(TimerInfo));
    time.year = -1;
    pSaveData->levelAB4 = 0x18;
}

void GameData::SynchroniseEnterLevel(void) {
    numChargeBites = 0;
    numChargeBites = gb.mNumChargeBites * 100;
    for(int i = 0; i < Total_ThunderEggs; i++) {
        SpecialPickupStruct* pEgg = GetThunderEgg((ThunderEggType)i);
        if (pSaveData->levels[pSaveData->levelAB0].thunderEggs[i] && pEgg) {
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
    
    for(int i = 0; i < Total_Cogs; i++) {
        SpecialPickupStruct* pCog = GetGoldenCog((GoldenCogType)i);
        // if cog for index is collected and Cog pointer isn't NULL
        // Set the cog collected and place a Picnic Basket
        if (pSaveData->levels[pSaveData->levelAB0].cogs[i] && pCog) {
            pCog->SetCollected(true);
            char* unkPtr = (char*)pCog->unk0[1];
            Basket_PlaceUnused((Vector*)((char*)unkPtr + 0x5C));
        }
    }
    
    for(int i = 0; i < Total_Bilbies; i++) {
        bool rescued = ((pSaveData->levels[pSaveData->levelAB0].bilbies[i] & 2) && (GetFreeBilbyCount() == 5))
            && !pSaveData->levels[pSaveData->levelAB0].thunderEggs[1];
        if (pSaveData->levels[pSaveData->levelAB0].bilbies[i] & 1) {
            Bilby_SetRescued((BilbyType)i, rescued);
        }
    }

    if (!pSaveData->levels[pSaveData->levelAB0].thunderEggs[0]) {
        // if Opal ThunderEgg isn't collected, synchronize opals that have already been collected
        SetCollectedGems();
    } else {
        numCollectedGems = 0;
        Hud_SetGems(numCollectedGems);
    }
    Crate_CheckGems();
    SetHasExtraHealth(pSaveData->tyAttributes.bHasExtraHealth);
    pSaveData->lives = (pSaveData->lives >= 5) ? pSaveData->lives : 5;
    if (pSaveData->levelAB4 != 0x18) {
        TyMemCard_AutoSaveGame();
    }
    SetHasRang((BoomerangType)4, false);
    SoundBank_SetVolume(1.0f, 3);
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
    gameData->prevLevelNumber1 = gameData->levelAB0;
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
        pSaveData->levelAB4 = pSaveData->levelAB0;
        pSaveData->levelAB0 = level;
        pSaveData->levels[pSaveData->levelAB0].nmbrOfTimesEntered++;
    }
}

void GameData::SetCurrentZone(ZoneNumber zone) {
    if (pSaveData) {
        pSaveData->currentZone = zone;
    }
}

void GameData::SetLearntToSwim(bool learntToSwim) {
	pSaveData->tyAttributes.bLearntToSwim = learntToSwim;
    bIsDirty = true;
}

void GameData::SetLearntToDive(bool learntToDive) {
	pSaveData->tyAttributes.bLearntToDive = learntToDive;
    bIsDirty = true;
}

void GameData::SetBothRangs(bool bHasBothRangs) {
	pSaveData->tyAttributes.bBothRangs = bHasBothRangs;
    bIsDirty = true;
}

void GameData::SetHasExtraHealth(bool bExtraHealth) {
    pSaveData->tyAttributes.bHasExtraHealth = bExtraHealth;
    if (bExtraHealth) {
        ty.health.SetNumSymbols(2);
    } else {
        ty.health.SetNumSymbols(1);
    }
    bIsDirty = true;
}

void GameData::SetHasRang(BoomerangType type, bool hasRang) {
    pSaveData->tyAttributes.bHasRangs[type] = hasRang;
    bIsDirty = true;
}

void GameData::SetBossDefeated(ZoneNumber zone, bool bossDefeated) {
    pSaveData->zoneInfo[zone].bZoneCompleted = bossDefeated;
    SetDataDirty(true);
    TalismanType index = gb.level.GetTalismanType((LevelNumber)pSaveData->levelAB0);
    SetHasTalisman(index, true);
    index = gb.level.GetTalismanType((LevelNumber)pSaveData->levelAB0);
    SetHasPlacedTalisman(index, true);
}

void GameData::SetZoneUnlocked(ZoneNumber zone, bool unlocked) {
    pSaveData->zoneInfo[zone].bUnlocked = unlocked;
    bIsDirty = true;
}

void GameData::SetZoneTalismanLocated(ZoneNumber zone, bool bLocated) {
    pSaveData->zoneInfo[zone].unk2 = bLocated;
    bIsDirty = true;
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
    u8* pGems = pSaveData->levels[pSaveData->levelAB0].gemArray;
    int totalGems = Min<int>(Gem::totalGems, GEMS_MAXOPALS);
    for(int i = 0; i < totalGems; i++) {
        if (CheckArrayByBitIndex(pGems, i) && Gem::gemPtrList[i]) {
            Gem::gemPtrList[i]->mCollected = true;
            Gem::gemPtrList[i]->SetState((GemState)5);
            gemCount++;
        }
    }
    numCollectedGems = gemCount;
    Hud_SetGems(numCollectedGems);
}

// Synchronizes the savedata for already collected gems in the current level
void GameData::GetCollectedGems(void) {
    u8* pGems = pSaveData->levels[pSaveData->levelAB0].gemArray;
    memset(pGems, 0, sizeof(pSaveData->levels[pSaveData->levelAB0].gemArray));
    int totalGems = Min<int>(Gem::totalGems, GEMS_MAXOPALS);
    for(int i = 0; i < totalGems; i++) {
        Gem* pGem = Gem::gemPtrList[i];
        if (pGem->mState == 5 || pGem->mState == 4 || pGem->mCollected) {
            pGems[i >> 3] |= 1 << (i & 7);
        }
    }
}

int GameData::GetGameCompletePercent(void) {
    int total = GetTotalGoldenCogCount() + GetTotalThunderEggCount() + GetTotalTalismanCount();
    total = (total * 100) / 167;
    return (total > 100) ? 100 : total;
}

/// @brief Gets the number of cogs collected within a single level
/// @param level Level number to calculate cog count
/// @return Total cogs collected in the level of the level parameter
int GameData::GetGoldenCogCount(LevelNumber level) {
    int count = 0;
    for(int i = 0; i < Total_Cogs; i++) {
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
        for(int j = 0; j < Total_Cogs; j++) {
            if (pSaveData->levels[i].cogs[j]) {
                count++;
            }
        }
    }
    return count;
}

int GameData::GetTotalTalismanCount(void) {
    int count = 0;
    for(int i = 0; i < Total_Talismans; i++) {
        if (pSaveData->bHasTalismans[i] != false) {
            count++;
        }
    }
    return count;
}

int GameData::GetCollectedGemCount(LevelNumber level) {
    if (level == pSaveData->levelAB0) {
        return numCollectedGems;
    }
    int count = 0;
    u8* gemArray = pSaveData->levels[level].gemArray;
    for(int i = 0; i < GEMS_MAXOPALS; i++) {
        if (CheckArrayByBitIndex(gemArray, i) != false) {
            count++;
        }
    }
    return count;
}

int GameData::GetFreeBilbyCount(void) {
    int count = 0;
    for(int i = 0; i < Total_Bilbies; i++) {
        if (pSaveData->levels[pSaveData->levelAB0].bilbies[i] & 1) {
            count++;
        }
    }
    return count++;
}

int GameData::GetTotalFreeBilbyCount(void) {
    int count = 0;
    for (u32 i = 0; i < Total_Levels; i++) {
        for(int j = 0; j < Total_Bilbies; j++) {
            if (IsBilbyFree(i, j)) {
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
    for(int i = 0; i < TOTAL_GALLERYIMAGES; i++) {
        if (CheckArrayByBitIndex(galleryArray, i)) {
            count++;
        }
    }
    return count;
}

void GameData::CollectCog(GoldenCogType cogType) {
    pSaveData->levels[pSaveData->levelAB0].cogs[cogType] = true;
    bIsDirty = true;
    Hud_ShowCogs();
    bIsDirty = true;
}

/// @brief Counts the number of collected thundereggs of a certain type
/// @param type Element type of level to consider
/// @return Number of thundereggs collected
int GameData::GetThunderEggCount(ElementType type) {
    int count = 0;
    for(int i = 0; i < Total_Levels; i++) {
        if (gb.level.GetElementType((LevelNumber)i) == type) {
            for(int j = 0; j < Total_ThunderEggs; j++) {
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
    for(int i = 0; i < Total_Levels; i++) {
        for(int j = 0; j < Total_ThunderEggs; j++) {
            if (pSaveData->levels[i].thunderEggs[j]) {
                count++;
            }
        }
    }
    return count;
}

void GameData::CollectThunderEgg(ThunderEggType type) {
    pSaveData->levels[pSaveData->levelAB0].thunderEggs[type] = true;
    bIsDirty = true;
    bIsDirty = true;
}

void GameData::SetHasPlacedTalisman(TalismanType type, bool bPlaced) {
    pSaveData->bHasPlacedTalisman[type] = bPlaced;
}

void GameData::UnlockZone(ZoneNumber zone) {
    SetZoneUnlocked(zone, true);
    bIsDirty = true;
}

void GameData::SetTimeAttackBest(int bestTime) {
	pSaveData->levels[pSaveData->levelAB0].bestTimeAttack = bestTime;
	bIsDirty = true;
}

void GameData::SetTimeAttackLastTime(int lastTime) {
	pSaveData->levels[pSaveData->levelAB0].lastTimeAttackTime = lastTime;
	bIsDirty = true;
}

int GameData::GetTimeAttackBest(LevelNumber level) {
	return pSaveData->levels[level].bestTimeAttack;
}

int GameData::GetTimeAttackLastTime(LevelNumber level) {
	return pSaveData->levels[level].lastTimeAttackTime;
}

void GameData::RescueBilby(BilbyType type) {
    pSaveData->levels[pSaveData->levelAB0].bilbies[type] |= 1;
    if (GetFreeBilbyCount() == 5) {
        pSaveData->levels[pSaveData->levelAB0].bilbies[type] |= 2;
    }
    Hud_ShowBilbies();
    bIsDirty = true;
}

void GameData::AddLife(void) {
    pSaveData->lives++;
    if (pSaveData->lives > 99) {
        pSaveData->lives = 99;
    }
    bIsDirty = true;
    Hud_ShowLives();
    bIsDirty = true;
}

void GameData::LoseLife(void) {
    if (!gb.bE3) {
        pSaveData->lives--;
        if (pSaveData->lives > 99) {
            pSaveData->lives = 99;
        }
        bIsDirty = true;
    }
    Hud_ShowLives();
    bIsDirty = true;
}

void GameData::RestoreLives(void) {
    pSaveData->lives = 5;
    if (pSaveData->lives > 99) {
        pSaveData->lives = 99;
    }
    bIsDirty = true;
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
    for(int i = 0; i < Total_Triggers; i++) {
        if (pSaveData->levels[pSaveData->levelAB0].triggers[i] == uniqueID) {
            return true;
        }
    }
    return false;
}

void GameData::SetHasBeenTriggered(uint uniqueID) {
    for(int i = 0; i < Total_Triggers; i++) {
        if (pSaveData->levels[pSaveData->levelAB0].triggers[i] == 0) {
            pSaveData->levels[pSaveData->levelAB0].triggers[i] = uniqueID;
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
