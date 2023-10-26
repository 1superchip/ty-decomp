#ifndef HEATFLARE_H
#define HEATFLARE_H

#include "common/Vector.h"

// possibly called "HeatFlareInfo" based on assertions?
struct HeatFlare {
    Vector unk0;
    float unk10;
    float unk14;
    int unk18;
};

void HeatFlare_LoadResources(void);
void HeatFlare_Init(void);
void HeatFlare_Deinit(void);
void HeatFlare_Update(void);
void HeatFlare_Draw(void);
void HeatFlare_Unlock(void);

#endif // HEATFLARE_H
