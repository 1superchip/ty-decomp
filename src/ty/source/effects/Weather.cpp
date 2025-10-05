#include "ty/effects/Weather.h"

#include "ty/tytypes.h"

MKParticleGen gParticleGen;

Rain gRain;

Lightning gLightning;

static WeatherInit* gpInit = NULL;

static int gNumTypes = 0;
static int gCurType = -1;
static int gCurTypeMenu = 0;

static bool gbEnableWeather = true;
static bool gbPauseWeather = false;

static bool gbFixRain = false;

void Weather_Init(WeatherInit* pInit, int numTypes, char**) {
    gpInit = pInit;
    gNumTypes = numTypes;

    gCurType = -1;

    for (int i = 0; i < gNumTypes; i++) {
        if (gpInit[i].type & WEATHER_TYPE_4) {
            gpInit[i].particleType.SetEnvelope(gpInit[i].particleType.pEnvelopes, gpInit[i].particleType.numEnvEntries);
        }
    }
}

void Weather_Deinit(void) {
    gCurType = -1;
}

void Weather_Update(void) {
    if (!gbEnableWeather || gCurType < 0) {
        return;
    }

    if (gbPauseWeather) {
        return;
    }

    if (gpInit[gCurType].type & WEATHER_TYPE_2) {
        gRain.Update();
    }

    if (gpInit[gCurType].type & WEATHER_TYPE_4) {
        gParticleGen.Update();
    }
    
    if (gpInit[gCurType].type & WEATHER_TYPE_LIGHTNING) {
        gLightning.Update();
    }
}

void Weather_Draw(View* pView) {
    if (!gbEnableWeather || gCurType < 0) {
        return;
    }

    static Vector origCameraPos = *pView->unk48.Row3();

    if (gpInit[gCurType].type & WEATHER_TYPE_2) {
        gRain.Draw(pView, gbFixRain);
    }

    if (gpInit[gCurType].type & WEATHER_TYPE_4) {
        Vector* p = gbFixRain ? NULL : pView->unk48.Row3();
        gParticleGen.Draw(pView, p, &origCameraPos);
    }

    if (gpInit[gCurType].type & WEATHER_TYPE_LIGHTNING) {
        gLightning.Draw(pView);
    }
}

void Weather_InitType(int type) {
    if (!gpInit[type].bInitialised) {
        gpInit[type].bInitialised = true;

        if ((gpInit[type].type & WEATHER_TYPE_4) && gpInit[type].particleType.primitiveType == MKParticleGenType::Type_2) {
            gpInit[type].pRainParticleMaterial = Material::Create(gpInit[type].particleType.pMatName);
        }
    }
}

void Weather_SetType(int type) {
    if (gCurType != -1) {
        if (gpInit[gCurType].type & WEATHER_TYPE_2) {
            gRain.Deinit();
        }

        if (gpInit[gCurType].type & WEATHER_TYPE_4) {
            gParticleGen.Deinit();
        }

        if (gpInit[gCurType].type & WEATHER_TYPE_LIGHTNING) {
            gLightning.Deinit();
        }
    }

    gCurType = type;
    gCurTypeMenu = type;

    if (gpInit[gCurType].type & WEATHER_TYPE_2) {
        gRain.Init(&gpInit[gCurType].rainInit);
    }

    if (gpInit[gCurType].type & WEATHER_TYPE_4) {
        gParticleGen.Init(&gpInit[gCurType].particleType, gpInit[gCurType].pRainParticleMaterial);
    }

    if (gpInit[gCurType].type & WEATHER_TYPE_LIGHTNING) {
        gLightning.Init(&gpInit[gCurType].lightningInit);
    }
}

void Weather_DeinitType(void) {
    if (gCurType != -1) {
        if (gpInit[gCurType].type & WEATHER_TYPE_2) {
            gRain.Deinit();
        }

        if (gpInit[gCurType].type & WEATHER_TYPE_4) {
            gParticleGen.Deinit();
        }

        if (gpInit[gCurType].type & WEATHER_TYPE_LIGHTNING) {
            gLightning.Deinit();
        }

        gCurType = -1;
    }

    for (int i = 0; i < gNumTypes; i++) {
        if (gpInit[i].bInitialised) {
            if (gpInit[i].pRainParticleMaterial) {
                gpInit[i].pRainParticleMaterial->Destroy();

                gpInit[i].pRainParticleMaterial = NULL;
            }

            gpInit[i].bInitialised = false;
        }
    }

    Weather_Enable(false);
}

int Weather_GetType(void) {
    return gCurType;
}

void Weather_Enable(bool bEnableWeather) {
    gbEnableWeather = bEnableWeather;
}
