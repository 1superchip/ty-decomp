#include "types.h"
#include "ty/DDA.h"
#include "common/Heap.h"
#include "common/Timer.h"
#include "ty/global.h"
#include "ty/Ty.h"

extern "C" void memset(void*, int, int);

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
    checkpointList.Init(38, sizeof(DDACheckpointInfo));
    deathList.Init(38, sizeof(DDADeathInfo));
    dbgMsgTimer = 0;
    unk20 = 0;
    unk = 0;
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
    levelNumber = gb.level.GetCurrentLevel();
    Timer_GetSystemTime(&startTime);
    startDay = startTime.day;
    startMonth = startTime.month;
    startHour = startTime.hours;
    startMinutes = startTime.minutes;
    startSeconds = startTime.seconds;
    checkpointList.Reset();
    deathList.Reset();
}

void DDASession::EndSession(void) {
    TimerInfo endTime;

    if (currentCheckpoint) {
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

    if (checkpointList.IsFull()) {
        return;
    }

    // Get new checkpoint pointer
    currentCheckpoint = checkpointList.GetNextEntry();
    currentCheckpoint->unk0 = checkpointList.GetSize();
    currentCheckpoint->unk4 = 0;
    currentCheckpoint->checkpointNumber = arg1;
    currentCheckpoint->unk9 = ty.mBoomerangManager.GetCurrentType();
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

    memset(&currentCheckpoint->cameraInfo, 0, sizeof(DDACameraInfo));

    unk = ty.mBoomerangManager.GetCurrentType();
}

void DDASession::EndCheckpoint(void) {
    TimerInfo endTime;
    Timer_GetSystemTime(&endTime);
    currentCheckpoint->checkpointEndHours = endTime.hours;
    currentCheckpoint->checkpointEndMinutes = endTime.minutes;
    currentCheckpoint->checkpointEndSeconds = endTime.seconds;
    currentCheckpoint = NULL;
}

void DDASession::StoreDeathInfo(void) {
    DDADeathInfo* pInfo;
    TimerInfo deathTime;

    if (currentCheckpoint == NULL) {
        return;
    }

    if (deathList.IsFull()) {
        return;
    }

    pInfo = deathList.GetNextEntry();
    pInfo->unk0 = currentCheckpoint->unk0;

    pInfo->deathPosX = pHero->pos.x;
    pInfo->deathPosY = pHero->pos.y;
    pInfo->deathPosZ = pHero->pos.z;
    pInfo->damageCause = currentCheckpoint->damageCause;
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
        case DDA_DAMAGE_1:
            currentCheckpoint->unk20++;
            break;
        case DDA_DAMAGE_2:
            currentCheckpoint->unk22++;
            break;
        case DDA_DAMAGE_3:
            currentCheckpoint->unk24++;
            break;
        case DDA_DAMAGE_4:
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
        case DDA_ENEMY_DAMAGE_1:
            currentCheckpoint->unk18++;
            break;
        case DDA_ENEMY_DAMAGE_2:
            currentCheckpoint->unk1A++;
            break;
        case DDA_ENEMY_DAMAGE_3:
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

    if (unk == ty.mBoomerangManager.GetCurrentType()) {
        return;
    }

    unk = ty.mBoomerangManager.GetCurrentType();

    currentCheckpoint->unk10++;
}

void DDASession::StoreCameraInfo(DDACameraAction cameraAction) {
    if (currentCheckpoint == NULL) {
        return;
    }

    switch (cameraAction) {
        case 0:
            currentCheckpoint->cameraInfo.unk2C++;
            break;
        case 1:
            currentCheckpoint->cameraInfo.unk2E++;
            break;
        case 2:
            currentCheckpoint->cameraInfo.unk2D++;
            break;
        case 3:
            currentCheckpoint->cameraInfo.unk2F++;
            break;
        case 4:
            currentCheckpoint->cameraInfo.unk30++;
            break;
        case 5:
            currentCheckpoint->cameraInfo.unk31++;
            break;
        case 6:
            currentCheckpoint->cameraInfo.unk32++;
            break;
        case 7:
            currentCheckpoint->cameraInfo.unk33++;
            break;
        case 8:
            currentCheckpoint->cameraInfo.unk34++;
            break;
        case 9:
            currentCheckpoint->cameraInfo.unk35++;
            break;
        case 10:
            currentCheckpoint->cameraInfo.unk36++;
            break;
        case 11:
            currentCheckpoint->cameraInfo.unk37++;
            break;
        case 12:
            currentCheckpoint->cameraInfo.unk38++;
            break;
    }
}

int DDASession::DeathCount(void) {
    return deathList.GetSize();
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
