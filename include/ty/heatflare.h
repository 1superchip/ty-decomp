#ifndef HEATFLARE_H
#define HEATFLARE_H

struct HeatFlare {
	char padding[0x1C];
};

void HeatFlare_LoadResources(void);
void HeatFlare_Init(void);
void HeatFlare_Deinit(void);
void HeatFlare_Update(void);
void HeatFlare_Draw(void);
void HeatFlare_Unlock(void);

#endif // HEATFLARE_H