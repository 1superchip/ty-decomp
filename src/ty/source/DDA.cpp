#include "types.h"
#include "ty/DDA.h"
#include "common/Heap.h"
#include "common/Timer.h"

// from a different file
extern "C" union {
u8 y[0xe84];
u32 x[0x6f0];
} gb;

extern "C" void memset(void*, int, int);
extern "C" int ty[0x1178]; // from Ty.cpp

bool DDASession::bInitialised;
bool DDASession::bSessionStarted;
bool DDASession::bConvertToReadable;
static int pDDAMenu;
static int pDDADrawEnabled;
static int pDDADrawCameraEnabled;
static int pDDASaveEnabled;
static int pDDAConvertToReadable;

bool DDASession::bSaveEnabled = true;

DDASession dda;

// https://decomp.me/scratch/bJko0
void DDASession::Init(void) {
    bInitialised = true;
    pDDAMenu = 0;
    pDDADrawEnabled = 0;
    pDDADrawCameraEnabled = 0;
    pDDASaveEnabled = 0;
    pDDAConvertToReadable = 0;
    unk10.Init(0x26, sizeof(DDAUnk18));
    unk14.Init(0x26, sizeof(DDAUnk14));
    unk1C = 0;
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
	if (unk1C > 0) {
        unk1C--;
    }
}

void DDASession::DrawDebugStats(void) {
	return;
}

//https://decomp.me/scratch/wpnTU
void DDASession::StartSession(void) {
    TimerInfo startTime;
    bSessionStarted = true;
    currentCheckpoint = 0;
    memset(this, 0, 0x10);
    unk0 = 0;
    levelNumber = gb.x[0x6ec / 4];
    Timer_GetSystemTime(&startTime);
    startDay = startTime.day;
    startMonth = startTime.month;
    startHour = startTime.hours;
    startMinutes = startTime.minutes;
    startSeconds = startTime.seconds;
    while ((int*)*(int*)unk10.pMem != 0) {
        ((int*)unk10.pMem)++;
    }
    while ((int*)*(int*)unk14.pMem != 0) {
        ((int*)unk14.pMem)++;
    }
}

//https://decomp.me/scratch/K3LS5
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
    TimerInfo checkpointTime;
    if (currentCheckpoint != NULL) {
        if (currentCheckpoint->unk8 == arg1) {
            return;
        }
        EndCheckpoint();
    }
    if(unk10.IsFull() == true) {
        return;
    }
    currentCheckpoint = unk10.GetNextEntry();
    currentCheckpoint->unk0 = unk10.GetSize();
    currentCheckpoint->unk4 = 0;
    currentCheckpoint->unk8 = arg1;
    currentCheckpoint->unk9 = ty[0x1178 / 4];
    Timer_GetSystemTime(&checkpointTime);
    currentCheckpoint->checkpointStartHours = checkpointTime.hours;
    currentCheckpoint->checkpointStartMinutes = checkpointTime.minutes;
    currentCheckpoint->checkpointStartSeconds = checkpointTime.seconds;
    currentCheckpoint->unk10 = 0;
    currentCheckpoint->unk12 = 0;
    currentCheckpoint->unk14 = 0;
    currentCheckpoint->unk15 = 0;
    currentCheckpoint->unk16 = 0;
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

// https://decomp.me/scratch/Qyupo
// most likely PtrList inlines
extern "C" float* pHero;

void DDASession::StoreDeathInfo(void) {
    DDAUnk14* ptr;
    DDAUnk14* pInfo;
    TimerInfo deathTime;
    if (currentCheckpoint == NULL) {
        return;
    }
    if (unk14.IsFull() != false) {
        return;
    }
    pInfo = unk14.GetNextEntry();
    *(int*)pInfo = *(int*)currentCheckpoint;

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
    if (currentCheckpoint == 0) {
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
        case 1:
            currentCheckpoint->unk12++;
            break;
        case 2:
            currentCheckpoint->unk14++;
            break;
        case 3:
            currentCheckpoint->unk15++;
            break;
        case 4:
            currentCheckpoint->unk16++;
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
    if (gb.y[0xe84] == false) {
        return;
    }
}

void DDASession::ConvertStatsToReadable(void) {
	return;
}