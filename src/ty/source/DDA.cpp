#include "types.h"
#include "ty/DDA.h"
#include "common/Heap.h"
#include "common/Timer.h"
#include "ty/global.h"

extern "C" void memset(void*, int, int);
extern "C" int ty[0x1178]; // from Ty.cpp

bool DDASession::bInitialised = false;
bool DDASession::bSessionStarted = false;
bool DDASession::bConvertToReadable = false;
static int pDDAMenu;
static int pDDADrawEnabled;
static int pDDADrawCameraEnabled;
static int pDDASaveEnabled;
static int pDDAConvertToReadable;

bool DDASession::bSaveEnabled = true;

DDASession dda;

void DDASession::Init(void) {
    bInitialised = true;
    pDDAMenu = 0;
    pDDADrawEnabled = 0;
    pDDADrawCameraEnabled = 0;
    pDDASaveEnabled = 0;
    pDDAConvertToReadable = 0;
	// 38 entries total
    unk10.Init(38, sizeof(DDAUnk18));
    unk14.Init(38, sizeof(DDAUnk14));
    dbgMsgTimer = 0;
    unk20 = 0;
    unk24[19] = 0;
    LoadStatsInfo();
}

void DDASession::Update(void) {
	if (bConvertToReadable != false) {
        ConvertStatsToReadable();
        bConvertToReadable = false;
	}
    DrawDebugMessage();
    DrawDebugStats();
}

void DDASession::DrawDebugMessage(void) {
	if (dbgMsgTimer > 0) {
        dbgMsgTimer--;
    }
}

void DDASession::DrawDebugStats(void) {
	return;
}

//https://decomp.me/scratch/wpnTU
void DDASession::StartSession(void) {
    TimerInfo startTime;
    bSessionStarted = true;
    currentCheckpoint = NULL;
    memset(this, 0, 0x10);
    unk0 = 0;
    levelNumber = ((int*)&gb.padding_0x2E0)[0x103]; // GetCurrentLevel inline
    Timer_GetSystemTime(&startTime);
    startDay = startTime.day;
    startMonth = startTime.month;
    startHour = startTime.hours;
    startMinutes = startTime.minutes;
    startSeconds = startTime.seconds;
    unk10.Reset();
    unk14.Reset();
}

void DDASession::EndSession(void) {
    TimerInfo endTime;
    if (currentCheckpoint != false) {
        EndCheckpoint();
    }
    Timer_GetSystemTime(&endTime);
    endHours = endTime.hours;
    endMinutes = endTime.minutes;
    endSeconds = endTime.seconds;
    SaveCurrentSession();
    bSessionStarted = false;
}

extern "C" void memset(void*, int, int);

void DDASession::NewCheckpoint(int arg1) {
    TimerInfo startTime; // checkpoint start time
    if (currentCheckpoint != NULL) {
        if (currentCheckpoint->checkpointNumber == arg1) {
            return; // return if this is the same checkpoint
        }
        EndCheckpoint();
    }
    if(unk10.IsFull()) {
        return;
    }
    // Get new checkpoint pointer
    currentCheckpoint = unk10.GetNextEntry();
    currentCheckpoint->unk0 = unk10.GetSize();
    currentCheckpoint->unk4 = 0;
    currentCheckpoint->checkpointNumber = arg1;
    currentCheckpoint->unk9 = ty[0x1178 / 4];
    Timer_GetSystemTime(&startTime);
    currentCheckpoint->checkpointStartHours = startTime.hours;
    currentCheckpoint->checkpointStartMinutes = startTime.minutes;
    currentCheckpoint->checkpointStartSeconds = startTime.seconds;
    currentCheckpoint->unk10 = 0;
    currentCheckpoint->opals = 0;
    currentCheckpoint->cogs = 0;
    currentCheckpoint->thunderEggs = 0;
    currentCheckpoint->bilbies = 0;
    currentCheckpoint->unk18 = 0;
    currentCheckpoint->unk1A = 0;
    currentCheckpoint->unk1C = 0;
    currentCheckpoint->unk1E = 0;
    currentCheckpoint->unk20 = 0;
    currentCheckpoint->unk22 = 0;
    currentCheckpoint->unk24 = 0;
    currentCheckpoint->unk26 = 0;
    currentCheckpoint->unk28 = 0;
    currentCheckpoint->damageCause = 0;
    currentCheckpoint->unk3C = 0;
    memset((char*)currentCheckpoint + 0x2c, 0, 0x10);
    unk24[19] = ty[0x1178 / 4];
}

void DDASession::EndCheckpoint(void) {
    TimerInfo endTime;
    Timer_GetSystemTime(&endTime);
    currentCheckpoint->checkpointEndHours = endTime.hours;
    currentCheckpoint->checkpointEndMinutes = endTime.minutes;
    currentCheckpoint->checkpointEndSeconds = endTime.seconds;
    currentCheckpoint = NULL;
}

extern "C" float* pHero;

void DDASession::StoreDeathInfo(void) {
    DDAUnk14* pInfo;
    TimerInfo deathTime;
    if (currentCheckpoint == NULL) {
        return;
    }
    if (unk14.IsFull()) {
        return;
    }
    pInfo = unk14.GetNextEntry();
    pInfo->unk0 = *(int*)currentCheckpoint;

    pInfo->deathPosX = (int)pHero[0x40 / 4];
    pInfo->deathPosY = (int)pHero[0x44 / 4];
    pInfo->deathPosZ = (int)pHero[0x48 / 4];
    pInfo->unk14 = currentCheckpoint->damageCause;
    pInfo->unk18 = 0;
    Timer_GetSystemTime(&deathTime);
    pInfo->deathTimeHour = deathTime.hours;
    pInfo->deathTimeMinutes = deathTime.minutes;
    pInfo->deathTimeSeconds = deathTime.seconds;
}

void DDASession::StoreDamageInfo(DDADamageCause damageCause) {
    if (currentCheckpoint == NULL) {
        return;
    }
    
    currentCheckpoint->damageCause = damageCause;
    switch (damageCause) {
        case 1:
            currentCheckpoint->unk20++;
            break;
        case 2:
            currentCheckpoint->unk22++;
            break;
        case 3:
            currentCheckpoint->unk24++;
            break;
        case 4:
            currentCheckpoint->unk26++;
            break;
        default:
            currentCheckpoint->unk28++;
            break;
    }
}

void DDASession::StorePickupInfo(DDAPickupType pickupType) {
    if (currentCheckpoint == NULL) {
        return;
    }
    switch (pickupType) {
        case Pickup_Gem:
            currentCheckpoint->opals++;
            break;
        case Pickup_Cog:
            currentCheckpoint->cogs++;
            break;
        case Pickup_ThunderEgg:
            currentCheckpoint->thunderEggs++;
            break;
        case Pickup_Bibly:
            currentCheckpoint->bilbies++;
            break;
    }
}

void DDASession::StoreEnemyDeathInfo(DDAEnemyDamageCause enemyDamageCause) {
    if (currentCheckpoint == NULL) {
        return;
    }

    switch (enemyDamageCause) {
        case 1:
            currentCheckpoint->unk18++;
            break;
        case 2:
            currentCheckpoint->unk1A++;
            break;
        case 3:
            currentCheckpoint->unk1C++;
            break;
        default:
            currentCheckpoint->unk1E++;
            break;
    }
}

void DDASession::StoreRangChanged(void) {
    if (currentCheckpoint == NULL) {
        return;
    }
    if (unk24[19] == ty[0x1178 / 4]) {
        return;
    }
    unk24[19] = ty[0x1178 / 4];
    currentCheckpoint->unk10++;
}

void DDASession::StoreCameraInfo(DDACameraAction cameraAction) {
    if (currentCheckpoint == NULL) {
        return;
    }
    switch (cameraAction) {
        case 0:
            currentCheckpoint->unk2C++;
            break;
        case 1:
            currentCheckpoint->unk2E++;
            break;
        case 2:
            currentCheckpoint->unk2D++;
            break;
        case 3:
            currentCheckpoint->unk2F++;
            break;
        case 4:
            currentCheckpoint->unk30++;
            break;
        case 5:
            currentCheckpoint->unk31++;
            break;
        case 6:
            currentCheckpoint->unk32++;
            break;
        case 7:
            currentCheckpoint->unk33++;
            break;
        case 8:
            currentCheckpoint->unk34++;
            break;
        case 9:
            currentCheckpoint->unk35++;
            break;
        case 10:
            currentCheckpoint->unk36++;
            break;
        case 11:
            currentCheckpoint->unk37++;
            break;
        case 12:
            currentCheckpoint->unk38++;
            break;
    }
}

int DDASession::DeathCount(void) {
    return unk14.GetSize();
}

void DDASession::LoadStatsInfo(void) {
	return;
}

void DDASession::SaveCurrentSession(void) {
    if (bSaveEnabled == false) {
        return;
    }
    if (gb.autoLevelSwitch == false) {
        return;
    }
}

void DDASession::ConvertStatsToReadable(void) {
	return;
}
