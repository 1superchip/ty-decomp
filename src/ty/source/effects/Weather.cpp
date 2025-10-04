#include "ty/effects/Weather.h"

MKParticleGen gParticleGen;

static WeatherInit* gpInit = NULL;
static int gNumTypes = 0;
static int gCurTypeMenu;
static bool gbPauseWeather = false;
static bool gbFixRain = false;

static int gCurType = -1;

static bool gbEnableWeather = true;

void Weather_Init(WeatherInit* pInit, int numTypes, char**) {
    gpInit = pInit;
    gNumTypes = numTypes;

    gCurType = -1;

    for (int i = 0; i < gNumTypes; i++) {
        if (gpInit[i].type & 4) {
            gpInit[i].genType.SetEnvelope(gpInit[i].genType.pEnvelopes, gpInit[i].genType.numEnvEntries);
        }
    }
}

void Weather_Deinit(void) {
    gCurType = -1;
}

void Weather_Update(void) {

}

void Weather_Draw(View* pView) {
    if (gbEnableWeather && gCurType >= 0) {
        static Vector origCameraPos = gParticleGen.mSrcPos;

        if (gpInit[gCurType].type & WEATHER_TYPE_4) {

        }

        if (gpInit[gCurType].type & WEATHER_TYPE_2) {
            gParticleGen.Draw(pView, !gbFixRain ? &origCameraPos : NULL, &origCameraPos);
        }

        if (gpInit[gCurType].type & WEATHER_TYPE_LIGHTNING) {
            
        }
    }
}

void Weather_InitType(int type) {

}

void Weather_SetType(int type) {
    if (gCurType != -1) {

    }

    gCurType = type;
    gCurTypeMenu = type;

    if (gpInit[type].type & 4) {

    }

    if (gpInit[type].type & 2) {
        gParticleGen.Init(&gpInit[type].genType, NULL);
    }

    if (gpInit[type].type & WEATHER_TYPE_LIGHTNING) {

    }
}

void Weather_DeinitType(void) {

}

int Weather_GetType(void) {
    return gCurType;
}

void Weather_Enable(bool bEnableWeather) {
    gbEnableWeather = bEnableWeather;
}
