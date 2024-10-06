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
    LN_First = 0,
    TOTAL_LEVEL_MAX = 0x18
};
enum ZoneNumber {};
enum GoldenCogType {};
enum ThunderEggType {};
enum TalismanType {};
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
    bool bHasRangs[12];
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
    ZoneStruct zoneInfo[6];
    int levelAB0;
    int levelAB4;
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
    int numChargeBites;
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
    void SetCurrentLevel(LevelNumber);
    void SetCurrentZone(ZoneNumber);
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

    bool IsBilbyFree(int level, int type) {
        return pSaveData->levels[level].bilbies[type] & 1;
    }
    void SetDataDirty(bool bSet) {
        bIsDirty = bSet;
    }
    void SetHasTalisman(int zone, bool bHas) {
        pSaveData->bHasTalismans[zone] = bHas;
        SetDataDirty(true);
    }
    bool CheckLearntToSwim(void) {
        return pSaveData->tyAttributes.bLearntToSwim;
    }
    bool HasBothRangs(void) {
        return pSaveData->tyAttributes.bBothRangs;
    }
    bool IsThunderEggCollected(int eggIdx, int level) {
        return pSaveData->levels[level].thunderEggs[eggIdx];
    }
    bool CheckCurrentLevelThunderEgg(int eggIdx) {
        return IsThunderEggCollected(eggIdx, pSaveData->levelAB0);
    }
    bool IsZoneCompleted(int zoneIdx) {
        return pSaveData->zoneInfo[zoneIdx].bZoneCompleted;
    }
    int GetCurrentLevel(void) {
        return pSaveData->levelAB0;
    }
    bool HasLevelBeenEntered(int level) {
        return pSaveData->levels[level].nmbrOfTimesEntered <= 1;
    }
    bool HasBoomerang(BoomerangType index) {
        return pSaveData->tyAttributes.bHasRangs[index];
    }
    bool GetLevelEnterCount(int level) {
        return pSaveData->levels[level].nmbrOfTimesEntered;
    }
    bool CheckZone_Unk0(int zoneIdx) {
        return pSaveData->zoneInfo[zoneIdx].bUnlocked;
    }
    int GetLevelCollectedGemCount(void) {
        return GetCollectedGemCount((LevelNumber)pSaveData->levelAB0);
    }
};
void GameData_Init(void);
void GameData_New(void);

inline int CheckArrayByBitIndex(u8* pArray, int bitIndex) {
    return pArray[bitIndex >> 3] & (1 << (bitIndex & 7));
}

#endif // GAMEDATA_H
