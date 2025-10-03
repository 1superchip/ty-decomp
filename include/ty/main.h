#ifndef MAIN_H
#define MAIN_H

#include "ty/global.h"

void Main_InitGameSystems(void);
void Main_InitStaticModules(void);
void Main_LoadStaticResources(void);

bool Main_DoVideo(void);

void LogicGame(void);

void ProcessCommandLine(void);

void Game_InitSystem(void);

void Main_AutoLevelSwitch_Update(void);

void Game_Init(void);
int Game_Update(void);
void Game_Draw(void);
void Game_Deinit(void);

float ValidPIRange(float);

struct GameCameraHeroInfo;
GameCameraHeroInfo* Main_UpdateGlobalGameCameraHeroInfo(void);

void Main_ChangeLevel(LevelNumber);

#endif // MAIN_H
