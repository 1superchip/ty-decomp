#include "ty/effects/Lightning.h"
#include "ty/tytypes.h"
#include "common/Debug.h"
#include "common/System_GC.h"

static bool gbDrawLightning = true;

static bool gbLightningInOrder = false;
static bool gbFixLightning = false;
static bool gbDrawLightningDebugInfo = false;
static bool gbDebugModeNextLightning = false;

void Lightning::Init(LightningInit* pNewInit) {
    pInit = pNewInit;

    bActive = false;

    minFramesDelay  = pNewInit->minSpawnDelay      * gDisplay.fps;
    maxFramesDelay  = pNewInit->maxSpawnDelay      * gDisplay.fps;
    numFramesOff    = pNewInit->numSecsFlashOff    * gDisplay.fps;
    numFramesOn     = pNewInit->numSecsFlashOn     * gDisplay.fps;

    basePos.Set(0.0f, 0.0f, 0.0f);

    ppModels = (Model**)Heap_MemAlloc(pNewInit->numModels * sizeof(Model*));

    for (int i = 0; i < pNewInit->numModels; i++) {
        ppModels[i] = Model::Create(pNewInit->modelNames[i], NULL);
    }

    ResetActivation();

    height = 0.0f;

    lightningPos.Set(0.0f, 0.0f, 0.0f);
}

void Lightning::Deinit(void) {
    if (ppModels) {
        for (int i = 0; i < pInit->numModels; i++) {
            if (ppModels[i]) {
                ppModels[i]->Destroy();
            }
        }

        Heap_MemFree(ppModels);
        ppModels = NULL;
    }
}

void Lightning::Update(void) {
    if (gFrameCounter >= nextActivationChange) {
        ChangeActivation();
    }
}

void Lightning::Draw(View* pView) {
    if (gbDrawLightning) {
        if (bActive) {
            if (!gbFixLightning) {
                basePos = *pView->unk48.Row3();
            }

            ppModels[randNum]->Draw(NULL);

            if (randNum1 > 0) {
                ppModels[randNum1]->Draw(NULL);
            }
        }

        if (gbDrawLightningDebugInfo) {
            DrawDebugInfo(pView);
        }
    }
}

void Lightning::DrawDebugInfo(View* pView) {
    float y = 200.0f;

    gpDebugFont->DrawText(
        Str_Printf(
            "numModels: %d, numFlashes: %d, minSpawnDelay: %g, maxSpawnDelay: %g", 
            pInit->numModels, pInit->numFlashes, pInit->minSpawnDelay, pInit->maxSpawnDelay
        ),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("minColor: %s, maxColor: %s", PrintVectorW(&pInit->minColor), PrintVectorW(&pInit->maxColor)),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("innerRadius: %g, outerRadius: %g", pInit->innerRadius, pInit->outerRadius),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("numSecsFlashOn: %d, numSecsFlashOff: %d", pInit->numSecsFlashOn, pInit->numSecsFlashOff),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("bActive: %d, nextActivationChange: %g", bActive, nextActivationChange),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("minFramesDelay: %d, maxFramesDelay: %d", minFramesDelay, maxFramesDelay),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("numFlashes: %d, numFramesOff: %d, numFramesOn: %d", numFlashes, numFramesOff, numFramesOn),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("height: %g, basePos: %s", height, FormatVector(&basePos)),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("lightningPos: %s", FormatVector(&lightningPos)),
        10.0f, y,
        1.5f, 1.5f,
        FONT_JUSTIFY_0,
        COLOR_WHITE
    );

    y += 20.0f;

    if (randNum != -1) {
        gpDebugFont->DrawText(
            Str_Printf("cur model 1: %s", pInit->modelNames[randNum]),
            10.0f, y,
            1.5f, 1.5f,
            FONT_JUSTIFY_0,
            COLOR_WHITE
        );
    }
    
    y += 20.0f;

    if (randNum1 != -1) {
        gpDebugFont->DrawText(
            Str_Printf("cur model 2: %s", pInit->modelNames[randNum1]),
            10.0f, y,
            1.5f, 1.5f,
            FONT_JUSTIFY_0,
            COLOR_WHITE
        );
    }

    y += 20.0f;
}

void Lightning::ResetActivation(void) {
    numFlashes = 0;

    static int seed = 0;

    nextActivationChange = gFrameCounter + RandomIR(&seed, minFramesDelay, maxFramesDelay);

    CalcNextSpawnPoint();
}

void Lightning::ChangeActivation(void) {
    bActive = !bActive;

    if (!bActive) {
        numFlashes++;

        if (numFlashes >= pInit->numFlashes) {
            ResetActivation();
        } else {
            nextActivationChange = gFrameCounter + numFramesOn;
        }
    } else {
        if (gbDebugModeNextLightning) {
            System_EnableDebugMode(true);
        }

        nextActivationChange = gFrameCounter + numFramesOff;
    }
}

void Lightning::CalcNextSpawnPoint(void) {
    static int seed = 0;

    if (gbLightningInOrder) {
        if (randNum >= pInit->numModels - 1) {
            randNum = 0;
        } else {
            randNum++;
        }

        randNum1 = -1;
    } else {
        randNum = RandomIR(&seed, 0, pInit->numModels - 1);
        randNum1 = RandomIR(&seed, -8, pInit->numModels - 1);
    }

    if (gbFixLightning) {
        lightningPos.Set(0.0f, 0.0f, 0.0f);
    } else {
        float radiusDifference = pInit->outerRadius - pInit->innerRadius;

        lightningPos.x = basePos.x + pInit->innerRadius + RandomFR(&seed, -radiusDifference, radiusDifference);
        lightningPos.y = height;
        lightningPos.z = basePos.z + pInit->innerRadius + RandomFR(&seed, -radiusDifference, radiusDifference);
    }

    ppModels[randNum]->SetPosition(&lightningPos);
    if (randNum1 >= 0) {
        ppModels[randNum1]->SetPosition(&lightningPos);
    }

    ppModels[randNum]->colour.x = RandomFR(&seed, pInit->minColor.x, pInit->maxColor.x);
    ppModels[randNum]->colour.y = RandomFR(&seed, pInit->minColor.y, pInit->maxColor.y);
    ppModels[randNum]->colour.z = RandomFR(&seed, pInit->minColor.z, pInit->maxColor.z);
    if (randNum1 >= 0) {
        ppModels[randNum1]->colour.x = RandomFR(&seed, pInit->minColor.x, pInit->maxColor.x);
        ppModels[randNum1]->colour.y = RandomFR(&seed, pInit->minColor.y, pInit->maxColor.y);
        ppModels[randNum1]->colour.z = RandomFR(&seed, pInit->minColor.z, pInit->maxColor.z);
    }
}
