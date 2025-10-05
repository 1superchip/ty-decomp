#ifndef WEATHER_H
#define WEATHER_H

#include "common/View.h"
#include "common/MKParticleGen.h"

#include "ty/effects/Rain.h"
#include "ty/effects/Lightning.h"

#define WEATHER_TYPE_LIGHTNING (1)
#define WEATHER_TYPE_2 (2)
#define WEATHER_TYPE_4 (4)

struct WeatherInit {
    int type;
    MKParticleGenType particleType;
    RainInit rainInit;
    LightningInit lightningInit;
    bool bInitialised;
    Material* pRainParticleMaterial;
};

void Weather_Init(WeatherInit* pInit, int numTypes, char**);
void Weather_Deinit(void);
void Weather_Update(void);
void Weather_Draw(View* pView);

void Weather_InitType(int type);
void Weather_SetType(int type);

void Weather_DeinitType(void);

int Weather_GetType(void);

void Weather_Enable(bool bEnableWeather);

// Use an enum for names
#define NUM_WEATHER_TYPES (7)

extern WeatherInit gWeatherInit[];
extern char* pWeatherTypeStrs[];

#endif // WEATHER_H
