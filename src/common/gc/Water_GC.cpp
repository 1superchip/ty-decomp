#include "common/Water_GC.h"
#include "common/StdMath.h"

#include "Dolphin/os.h"

#define WATER_TEX_WIDTH (0x20)
#define WATER_TEX_HEIGHT (0x20)

char waterIndTexData[WATER_TEX_HEIGHT][WATER_TEX_WIDTH]; // 2D array?
GXTexObj waterIndTexObj;

bool bWaterUpdate = true;

void Water_InitModule(void) {
    GXInitTexObj(&waterIndTexObj, waterIndTexData, 16, 16, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, 0);
    GXInitTexObjLOD(&waterIndTexObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
}

void Water_DeinitModule(void) {}

void Water_Update(void) {
    if (!bWaterUpdate) {
        return;
    }

    static u8 phaseU = 0;
    static u8 phaseV = 64;

    phaseU++;
    phaseV++;

    char* r28 = &waterIndTexData[0][0];
    char* r27 = &waterIndTexData[1][0];

    static float angle = 0.0f;

    angle += 0.1f;
    if (angle > 2 * PI) {
        angle -= 2 * PI;
    }

    union {
        float f;
        int i;
    } rngSeed;

    for (int r26 = 0; r26 < 0x10; r26 += 4) {
        for (int r25 = 0; r25 < 0x10; r25 += 4) {
            int* rand = &rngSeed.i;
            rngSeed.f = 0.32f;
            for (int i = r26; i < r26 + 4; i++) {
                for (int j = r25; j < r25 + 4; j++) {
                    float f16 = (float)i + 57.0f;
                    float f24 = (float)j + 30.0f;
                    rngSeed.f = (f24 * f16) * f16 * (0.2f + f24) * (0.8f + f16);
                    float randAngle = RandomFR(rand, 0.0f, 2 * PI);
                    rngSeed.f = (0.05f + f24) * (1.2f + f24) * (2.15f + f16) * (1.2f + f16);
                    float f15 = RandomFR(rand, 0.0f, 2 * PI);
                    f15 += angle;
                    int dat0 = 128.0f + (127.0f * _table_sinf(randAngle + angle));
                    int dat1 = 128.0f + (127.0f * _table_sinf(f15));
                    r28[0] = dat0;
                    r27[1] = dat1;
                    r28 += 2;
                    r27 += 2;
                }
            }
            
            r28 += 0x20;
            r27 += 0x20;
        }
    }

    DCFlushRange((uint*)&waterIndTexData, sizeof(waterIndTexData));
    bWaterUpdate = false;
}
