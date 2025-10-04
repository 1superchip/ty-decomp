#ifndef WEATHER_H
#define WEATHER_H

#include "common/View.h"
#include "common/MKParticleGen.h"

#define WEATHER_TYPE_LIGHTNING (1)
#define WEATHER_TYPE_2 (2)
#define WEATHER_TYPE_4 (4)

struct RainInit {
    float unk0;
    char* pModelName;
    Vector color;
    char* pMaterialName;
};

struct WeatherInit {
    int type;
    MKParticleGenType genType;
    char padding[0xEC - 0x80];
};

void Weather_Init(WeatherInit* pInit, int numTypes, char**);
void Weather_Deinit(void);
void Weather_Update(void);
void Weather_Draw(View*);

void Weather_InitType(int);
void Weather_SetType(int type);

void Weather_DeinitType(void);

int Weather_GetType(void);

void Weather_Enable(bool);

#endif // WEATHER_H
