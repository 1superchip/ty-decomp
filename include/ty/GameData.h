#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "common/Timer.h"
#include "ty/props/gem.h"
#include "ty/boomerang.h"

#define Total_Bilbies 		(5)
#define Total_ThunderEggs	(8)
#define Total_Cogs			(10)
#define Total_Triggers 		(20)
#define Total_Levels 		(0x18)
#define Total_Talismans 	(5)
#define TOTAL_GALLERYIMAGES	(512)

enum LevelNumber {
    LN_First            = 0, 

    LN_RAINBOW_CLIFFS   = 0,  // z1
    LN_1                = 1,  // z2
    LN_2                = 2,  // z3
    LN_3                = 3,  // z4

    LN_TWO_UP           = 4,  // a1
    LN_WALK_IN_THE_PARK = 5,  // a2
    LN_SHIP_REX         = 6,  // a3
    LN_BULLS_PEN        = 7,  // a4

    LN_BRIDGE_RIVER_TY  = 8,  // b1
    LN_SNOW_WORRIES     = 9,  // b2
    LN_OUTBACK_SAFARI   = 10, // b3
    LN_11               = 11, // b4

    LN_LYRE_LYRE        = 12, // c1
    LN_BLACK_STUMP      = 13, // c2
    LN_REX_MARKS_SPOT   = 14, // c3
    LN_FLUFFYS_FJORD    = 15, // c4

    LN_16               = 16, // d1
    LN_CASS_CREST       = 17, // d2
    LN_18               = 18, // d3
    LN_CRIKEYS_COVE     = 19, // d4

    LN_CASS_PASS        = 20, // e1
    LN_BONUS_WORLD_DAY  = 21, // e2
    LN_BONUS_WORLD_NIGHT= 22, // e3
    LN_FINAL_BATTLE     = 23, // e4

    TOTAL_LEVEL_MAX     = 24,
};

enum ZoneNumber {
    ZN_0        = 0,
    ZN_1        = 1,
    ZN_2        = 2,
    ZN_3        = 3,
    ZN_4        = 4,
    ZN_5        = 5,
    TOTAL_ZONES = 6
};

enum GoldenCogType {};
enum ThunderEggType {};

enum TalismanType {
    TALISMAN_TYPE_0 = 0,
    TALISMAN_TYPE_1 = 1,
    TALISMAN_TYPE_2 = 2,
    TALISMAN_TYPE_3 = 3,
    TALISMAN_TYPE_4 = 4,
    TALISMAN_TYPE_5 = 5
};

enum BilbyType {};

struct GameDataSettings {
    u8 musicVolume;
    u8 soundVolume;
    u8 unk2;
    s8 unk3;
    s8 unk4;
    u8 unk5;
    bool unk6;
    char unk7;
    u8 controlConfig;
    char unk9;
    char unkA;
    char unkB;
};

extern GameDataSettings* pGameSettings;

struct Attributes {
    bool bLearntToSwim;
    bool bLearntToDive;
    bool bBothRangs;
    bool bHasExtraHealth;
    bool bHasRangs[BR_Max];
};

#define CHAR_BIT (8)
#define BITS_TO_CHARS(bits)   ((((bits) - 1) / CHAR_BIT) + 1)

struct SaveLevelEntry {
    u8 nmbrOfTimesEntered;
    u8 gemArray[BITS_TO_CHARS(GEMS_MAXOPALS)];
    u8 unk27;
    bool thunderEggs[Total_ThunderEggs];
    bool cogs[Total_Cogs];
    u8 bilbies[Total_Bilbies];
    int bestTimeAttack;
    int lastTimeAttackTime;
    u16 triggers[Total_Triggers];
};

struct ZoneStruct {
    bool bUnlocked;
    u8 bZoneCompleted;
    u8 unk2;
};

struct SaveData {
    int size;
    int version;
    int prevLevelNumber1;
    u8 completePercent;
    u8 thunderEggCount;
    u8 unkE;
    bool bCheatsEnabled;
    GameDataSettings settings;
    SaveLevelEntry levels[Total_Levels];
    ZoneStruct zoneInfo[TOTAL_ZONES];
    LevelNumber currentLevel;
    LevelNumber previousLevel;
    int currentZone;
    int currentRang;
    Attributes tyAttributes;
    bool bHasTalismans[Total_Talismans];
    bool bHasPlacedTalisman[Total_Talismans];
    char unkADA[2];
    s16 lives;
    u8 galleryImages[TOTAL_GALLERYIMAGES / 8];
    int totalPlayTime;
};

struct GameData {
    bool unk0;
    bool bIsDirty;
    int numCollectedGems;
    int numChargeBites; // Opal count used for charged bites (super bites)
    bool bOpalLifeup;
    TimerInfo time;
    SaveData* pSaveData;
    bool bMusicDucked;
    
    void Init(void);
    void SynchroniseEnterLevel(void);
    void SynchroniseExitLevel(void);
    SaveData* GetSaveData(void);
    void ReleaseSaveData(void);
    void SetDirty(bool);
    void SetCurrentLevel(LevelNumber level);
    void SetCurrentZone(ZoneNumber zone);
    void SetLearntToSwim(bool);
    void SetLearntToDive(bool);
    void SetBothRangs(bool);
    void SetHasExtraHealth(bool);
    void SetHasRang(BoomerangType, bool);
    void SetBossDefeated(ZoneNumber, bool);
    void SetZoneUnlocked(ZoneNumber, bool);
    void SetZoneTalismanLocated(ZoneNumber, bool);
    void ReceiveOpalLifeup(void);
    void CollectGem(bool);
    void SetCollectedGems(void);
    void GetCollectedGems(void);
    int GetGameCompletePercent(void);
    int GetGoldenCogCount(LevelNumber);
    int GetTotalGoldenCogCount(void);
    int GetTotalTalismanCount(void);
    int GetCollectedGemCount(LevelNumber);
    int GetFreeBilbyCount(void);
    int GetTotalFreeBilbyCount(void);
    bool SetHasGalleryImage(int);
    int GetHasGalleryImage(int); // return type?
    int GetTotalGalleryCount(void);
    void CollectCog(GoldenCogType);
    int GetThunderEggCount(ElementType);
    int GetTotalThunderEggCount(void);
    void CollectThunderEgg(ThunderEggType);
    void SetHasPlacedTalisman(TalismanType, bool);
    void UnlockZone(ZoneNumber);
    void SetTimeAttackBest(int);
    void SetTimeAttackLastTime(int);
    int GetTimeAttackBest(LevelNumber);
    int GetTimeAttackLastTime(LevelNumber);
    void RescueBilby(BilbyType);
    void AddLife(void);
    void LoseLife(void);
    void RestoreLives(void);
    void AddChargeBite(void);
    void StartTime(void);
    void StopTime(void);
    int GetTotalTime(void);
    bool GetHasBeenTriggered(uint); // return type?
    void SetHasBeenTriggered(uint);
    void SetCurrentRang(BoomerangType);
    float GetMusicVolume(void);
    float GetSoundVolume(void);
    void SetMusicDucked(bool);

    bool IsBilbyFree(LevelNumber level, int type) {
        return pSaveData->levels[level].bilbies[type] & 1;
    }

    void SetDataDirty(bool bSet) {
        bIsDirty = bSet;
    }

    void SetHasTalisman(TalismanType type, bool bHas) {
        // Most likely defined in GameData.cpp
        pSaveData->bHasTalismans[type] = bHas;
        SetDataDirty(true);
    }

    bool CheckLearntToSwim(void) {
        return pSaveData->tyAttributes.bLearntToSwim;
    }

    bool HasBothRangs(void) {
        return pSaveData->tyAttributes.bBothRangs;
    }

    bool IsThunderEggCollected(int eggIdx, LevelNumber level) {
        return pSaveData->levels[level].thunderEggs[eggIdx];
    }

    bool CheckCurrentLevelThunderEgg(int eggIdx) {
        return IsThunderEggCollected(eggIdx, pSaveData->currentLevel);
    }

    bool IsZoneCompleted(ZoneNumber zoneIdx) {
        return pSaveData->zoneInfo[zoneIdx].bZoneCompleted;
    }

    LevelNumber GetCurrentLevel(void) {
        return pSaveData->currentLevel;
    }

    bool HasLevelBeenEntered(LevelNumber level) {
        return pSaveData->levels[level].nmbrOfTimesEntered > 1;
    }

    bool HasBoomerang(BoomerangType index) {
        return pSaveData->tyAttributes.bHasRangs[index];
    }

    BoomerangType GetBoomerang(void) {
        return (BoomerangType)pSaveData->currentRang;
    }

    bool GetLevelEnterCount(LevelNumber level) {
        return pSaveData->levels[level].nmbrOfTimesEntered;
    }

    bool CheckZone_Unk0(ZoneNumber zoneIdx) {
        return pSaveData->zoneInfo[zoneIdx].bUnlocked;
    }

    int GetLevelCollectedGemCount(void) {
        return GetCollectedGemCount(pSaveData->currentLevel);
    }

    void SetHasGoldenCog(GoldenCogType cogType, bool bHasCog) {
        // Most likely defined in GameData.cpp
        pSaveData->levels[pSaveData->currentLevel].cogs[cogType] = bHasCog;
        SetDataDirty(true);
    }

    void SetHasThunderEgg(ThunderEggType eggType, bool bHasEgg) {
        // Most likely defined in GameData.cpp
        pSaveData->levels[pSaveData->currentLevel].thunderEggs[eggType] = bHasEgg;
        SetDataDirty(true);
    }

    int GetLives(void) {
        return pSaveData->lives;
    }

    void SetLives(int numLives) {
        pSaveData->lives = numLives;

        if (pSaveData->lives > 99) {
            pSaveData->lives = 99;
        }

        SetDataDirty(true);
    }

    void SetChargeBites(int count) {
        // Set the charge bite opal count to (count * 100)
        numChargeBites = count * 100;
    }
};

void GameData_Init(void);
void GameData_New(void);

inline int CheckArrayByBitIndex(u8* pArray, int bitIndex) {
    return pArray[bitIndex >> 3] & (1 << (bitIndex & 7));
}

#endif // GAMEDATA_H
