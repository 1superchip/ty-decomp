#include "ty/Shatter.h"
#include "ty/global.h"

static bool bIsInitialised = false;

PtrListDL<ShatterStruct> shatterObjects;
StructList<ShatterStruct*> liveObjects;

void Shatter_Init(void) {
    if (!bIsInitialised) {
        if (gb.level.GetCurrentLevel() == LN_OUTBACK_SAFARI) {
            shatterObjects.Init(950, sizeof(ShatterStruct));
        } else {
            shatterObjects.Init(400, sizeof(ShatterStruct));
        }

        liveObjects.Init(60);

        bIsInitialised = true;
    }
}

void Shatter_Deinit(void) {
    if (bIsInitialised) {

        ShatterStruct** ppShatters = shatterObjects.GetMem();
        while (*ppShatters) {
            (*ppShatters)->Delete();
            ppShatters++;
        }

        shatterObjects.Deinit();
        liveObjects.Deinit();

        bIsInitialised = false;
    }
}

ShatterStruct* Shatter_Add(Model*, float, float, int) {
    if (!bIsInitialised) {
        return NULL;
    }


}

