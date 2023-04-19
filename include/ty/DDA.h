#ifndef DDA_H
#define DDA_H

#include "types.h"
#include "common/PtrListDL.h"

enum DDADamageCause {
	
};

enum DDAPickupType {
	Pickup_Gem = 1,
	Pickup_Cog = 2,
	Pickup_ThunderEgg = 3,
	Pickup_Bibly = 4
};

enum DDAEnemyDamageCause {
	
};

enum DDACameraAction {
    
};

struct DDAUnk14 {
    int unk0;
    u8 deathTimeHour;
    u8 deathTimeMinutes;
    u8 deathTimeSeconds;
    u8 unk7;
    int deathPosX;
    int deathPosY;
    int deathPosZ;
    u8 unk14;
    u8 unk15;
    u8 unk16;
    u8 unk17;
    int unk18;
};

struct DDAUnk18 {
    int unk0;
    int unk4;
    u8 checkpointNumber;
    u8 unk9;
    u8 checkpointStartHours;
    u8 checkpointStartMinutes;
    u8 checkpointStartSeconds;
    u8 checkpointEndHours;
    u8 checkpointEndMinutes;
    u8 checkpointEndSeconds;
    u16 unk10;
    u16 opals;
    u8 cogs;
    u8 thunderEggs;
    u8 bilbies;
    u8 unk17;
    u16 unk18;
    u16 unk1A;
    u16 unk1C;
    u16 unk1E;
    u16 unk20;
    u16 unk22;
    u16 unk24;
    u16 unk26;
    u16 unk28;
    u8 damageCause;
    u8 unk2B;
    u8 unk2C;
    u8 unk2D;
    u8 unk2E;
    u8 unk2F;
    u8 unk30;
    u8 unk31;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 unk39;
    u8 unk3A;
    u8 unk3B;
    int unk3C;
};

// size of 0x74?
struct DDASession {
	int unk0;
    int levelNumber;
    u8 startDay;
    u8 startMonth;
    u8 startHour;
    u8 startMinutes;
    u8 startSeconds;
    u8 endHours;
    u8 endMinutes;
    u8 endSeconds;
    PtrListDL<DDAUnk18> unk10; // check this??
    PtrListDL<DDAUnk14> unk14; // death struct
    DDAUnk18* currentCheckpoint;
    int dbgMsgTimer;
    u8 unk20;
    int unk24[(0x74-0x24) / 4];
	
	void Init(void);
	void Update(void);
	void DrawDebugMessage(void);
	void DrawDebugStats(void);
	void StartSession(void);
	void EndSession(void);
	void NewCheckpoint(int);
	void EndCheckpoint(void);
	void StoreDeathInfo(void);
    void StoreDamageInfo(DDADamageCause);
    void StorePickupInfo(DDAPickupType);
	void StoreEnemyDeathInfo(DDAEnemyDamageCause);
    void StoreRangChanged(void);
    void StoreCameraInfo(DDACameraAction);
    int DeathCount(void);
	void LoadStatsInfo(void);
	void SaveCurrentSession(void);
	void ConvertStatsToReadable(void);
	
	static bool bInitialised;
	static bool bSessionStarted;
	static bool bConvertToReadable;
	static bool bSaveEnabled;
};

// structs of "DDACheckpointInfoList" / "DDADeathInfoList"

#endif // DDA_H