#include "ty/effects/Weather.h"

char* pWeatherTypeStrs[] = {
    "none",
    "lightning",
    "stormy",
    "rain",
    "snow",
    "plankton",
    "foggy",
    NULL
};

char* pLightningModelNames[] = {
    "FX05_Lightning",
    "FX05_Lightning_02",
    "FX05_Lightning_03",
    "FX05_Lightning_04",
    "FX05_Lightning_05",
};

WeatherInit gWeatherInit[NUM_WEATHER_TYPES] = {
    {
        0,
        { // MKParticleGenType
            1.0f,
            500.0f,
            MKParticleGenType::Type_Line,
            1.0f,
            3.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            -300.0f,
            0.0f,
            1000, // numParticles
            NULL,
            defaultPGTEnvelope,
            ARRAY_SIZE(defaultPGTEnvelope),
            1.0f,
            9.9999998E+10f,
            -9.9999998E+10f
        },
        { // RainInit
            1.0f,
            "FX00_RainDome",
            {1.0f, 1.0f, 1.0f, 0.5f},
            "FX00_TX00",
            0.9f
        },
        { // LightningInit
            pLightningModelNames,
            ARRAY_SIZE(pLightningModelNames),
            6, // numFlashes
            0.1f, // minSpawnDelay
            1.0f, // maxSpawnDelay
            {0.7f, 1.0f, 0.7f, 1.0f}, // minColor
            {1.0f, 1.0f, 1.0f, 1.0f}, // maxColor
            10000.0f, // innerRadius
            50000.0f, // outerRadius
            0.5f, // numSecsFlashOff
            0.5f, // numSecsFlashOn
        },
        false,
        NULL
    },
    {
        WEATHER_TYPE_LIGHTNING,
        {},
        {},
        { // LightningInit
            pLightningModelNames,
            ARRAY_SIZE(pLightningModelNames),
            10, // numFlashes
            1.0f, // minSpawnDelay
            3.0f, // maxSpawnDelay
            {0.7f, 1.0f, 0.7f, 1.0f}, // minColor
            {1.0f, 1.0f, 1.0f, 1.0f}, // maxColor
            5000.0f, // innerRadius
            9000.0f, // outerRadius
            0.5f, // numSecsFlashOff
            0.5f, // numSecsFlashOn
        },
        false,
        NULL
    },
    {
        WEATHER_TYPE_LIGHTNING | WEATHER_TYPE_2 | WEATHER_TYPE_4,
        { // MKParticleGenType
            1.0f,
            500.0f,
            MKParticleGenType::Type_Line,
            1.0f,
            3.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            -300.0f,
            0.0f,
            1000, // numParticles
            NULL,
            defaultPGTEnvelope,
            ARRAY_SIZE(defaultPGTEnvelope),
            1.0f,
            9.9999998E+10f,
            -9.9999998E+10f
        },
        {
            1.0f,
            "FX00_RainDome",
            {1.0f, 1.0f, 1.0f, 0.5f},
            "FX00_TX00",
            0.9f
        },
        { // LightningInit
            pLightningModelNames,
            ARRAY_SIZE(pLightningModelNames),
            6, // numFlashes
            0.1f, // minSpawnDelay
            1.0f, // maxSpawnDelay
            {0.7f, 1.0f, 0.7f, 1.0f}, // minColor
            {1.0f, 1.0f, 1.0f, 1.0f}, // maxColor
            10000.0f, // innerRadius
            50000.0f, // outerRadius
            0.5f, // numSecsFlashOff
            0.5f, // numSecsFlashOn
        },
        false,
        NULL
    },
    {
        WEATHER_TYPE_4,
        { // MKParticleGenType
            1.0f,
            900.0f,
            MKParticleGenType::Type_Line,
            1.0f,
            15.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            0.0f,
            -950.0f,
            0.0f,
            500, // numParticles
            NULL,
            defaultPGTEnvelope,
            ARRAY_SIZE(defaultPGTEnvelope),
            1.0f,
            9.9999998E+10f,
            -9.9999998E+10f
        },
        {},
        {},
        false,
        NULL
    },
    {
        WEATHER_TYPE_4,
        { // MKParticleGenType
            1.0f,
            500.0f,
            MKParticleGenType::Type_2,
            2.5f,
            2.5f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            -100.0f,
            0.0f,
            500, // numParticles
            "fx_073",
            wobbleEnv,
            ARRAY_SIZE(wobbleEnv),
            4.0f,
            9.9999998E+10f,
            -9.9999998E+10f
        },
        {},
        {},
        false,
        NULL
    },
    {
        WEATHER_TYPE_4,
        { // MKParticleGenType
            1.0f,
            500.0f,
            MKParticleGenType::Type_2,
            2.5f,
            2.5f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            500, // numParticles
            "fx_108",
            wobbleEnv,
            ARRAY_SIZE(wobbleEnv),
            4.0f,
            0.0f,
            -9.9999998E+10f
        },
        {},
        {},
        false,
        NULL
    },
    {
        WEATHER_TYPE_4,
        { // MKParticleGenType
            1.0f,
            1000.0f,
            MKParticleGenType::Type_2,
            50.0f,
            50.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            0.0f,
            0.0f,
            0.0f,
            3500, // numParticles
            "fog_01",
            defaultPGTEnvelope,
            ARRAY_SIZE(defaultPGTEnvelope),
            4.0f,
            -1420.0f,
            -9.9999998E+10f
        },
        {},
        {},
        false,
        NULL
    }
};
