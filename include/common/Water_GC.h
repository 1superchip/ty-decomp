#ifndef COMMOM_WATERGC_H
#define COMMOM_WATERGC_H

#include "Dolphin/gx.h"

#define WATER_TEX_WIDTH (0x20)
#define WATER_TEX_HEIGHT (0x20)

extern char waterIndTexData[WATER_TEX_HEIGHT][WATER_TEX_WIDTH]; // 2D array?
extern GXTexObj waterIndTexObj;
extern bool bWaterUpdate;

void Water_InitModule(void);
void Water_DeinitModule(void);
void Water_Update(void);


#endif // COMMOM_WATERGC_H