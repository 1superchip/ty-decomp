
#include "ty/main.h"
#include "ty/Ty.h"
#include "ty/global.h"
#include "ty/GameObjectManager.h"
#include "ty/heatflare.h"
#include "ty/RenderTexture.h"
#include "ty/friends/friend.h"
#include "ty/ParticleEngine.h"
#include "ty/frontend/FrontEnd.h"
#include "common/ParticleSystemManager.h"
#include "common/MKPackage.h"
#include "common/Translation.h"
#include "common/MKGrass.h"

void Weather_Deinit(void);
void GameCamera_InitModule(unsigned int*);

static void Main_CheckInputs(void);

int MaxMemoryUsed[TOTAL_LEVEL_MAX] = {};

void Main_InitGameSystems(void) {
    defaultParticleManager.Init(300, 2500);
    Main_InitStaticModules();
    Main_LoadStaticResources();

    dda.Init();

    InitializeGame();
}

void Main_InitStaticModules(void) {
    gb.mJoyPad1.mAnalogControl.Init(0.5f, 5.0f);

    GameCamera_InitModule(&gb.logicGameCount);
}

extern void Hud_Init(Ty*);

void Main_LoadStaticResources(void) {
    while (!MKPackage_IsLoaded(NULL)) {}

    gb.unk8 = 75;
    gb.unk4 = 0;

    KromeIni ini;

    ini.Init("global.model");

    gb.pGlobalFont = Font::Create("Font_Hud_256");

    HeatFlare_LoadResources();
    RenderTexture_LoadResources();
    Ty_LoadResources();

    Friend_LoadResources(&ini);

    memset((void*)particleManager, 0, sizeof(*particleManager));

    particleManager->LoadResources();

    RainbowEffect_LoadResources();
    ChronorangEffects_LoadResources();

    ini.Deinit();

    Hud_Init(&ty);
    
    MKPackage_Free();
}

bool Main_DoVideo(void) {

}

Vector* GameCamera_GetPos(void);

bool TyMemCard_IsAutoSaving(void);
int TyMemCard_Update(void);

enum PauseScreen_State {};
void PauseScreen_Init(PauseScreen_State);
void PauseScreen_Update(void);
void PauseMap_UpdateHeroPos(void);

void BonusPickupParticle_Update(void);

void Shatter_Update(void);

void LogicGame(void) {
    gSceneManager.SetActivePoint(GameCamera_GetPos());

    if (!TyMemCard_IsAutoSaving()) {
        switch (TyMemCard_Update()) {
            case 4:
                PauseScreen_Init((PauseScreen_State)0xD);
                break;
            case 5:
                PauseScreen_Init((PauseScreen_State)0xE);
                break;
            case 6:
                PauseScreen_Init((PauseScreen_State)0xE);
                break;
        }
    }

    if (gb.unk700) {
        gb.unk700 = false;

        return;
    }

    if (!gb.bOnPauseScreen && gb.mLogicState.GetCurr() == STATE_5) {
        if ((gb.mJoyPad1.mNewButtonsPressedThisFrame & tyControl.buttonVals[1]) && !TyMemCard_IsAutoSaving()) {
            if (gb.level.GetCurrentLevel() != LN_16) {
                gb.bOnPauseScreen = true;
                PauseScreen_Init((PauseScreen_State)0x4);
            }
        }
    }

    if (!gb.bOnPauseScreen && gb.mLogicState.GetCurr() == STATE_5) {
        if ((gb.mJoyPad1.mNewButtonsPressedThisFrame & tyControl.buttonVals[0]) && !TyMemCard_IsAutoSaving()) {
            gb.bOnPauseScreen = true;
            PauseScreen_Init((PauseScreen_State)0x3);
        }
    }

    if (gb.bOnPauseScreen) {
        PauseScreen_Update();
        return;
    } else {
        PauseMap_UpdateHeroPos();
    }

    gb.logicGameCount++;

    Range_Update();

    dda.Update();
    Gem_PickupParticle_Update();
    BonusPickupParticle_Update();

    defaultParticleManager.Update();

    ManuallyScrollTextures();

    HeatFlare_Update();

    Tools_DropShadow_Update();

    if (gb.logicGameCount % 10) {

    }

    Shatter_Update();

    gSceneManager.UpdateProps();
    objectManager.UpdateModules();

    gb.mGameFsm.CallStateDeinit();
}

void ProcessCommandLine(void) {

}

void Game_InitSystem(void) {
    if (System_GetCommandLineParameter("-cameraDebug")) {
        // check?
        gMKDefaults.debugParticle_count = 3000;
        gMKDefaults.debugSphere_count = 3000;
    }

    gMKDefaults.unk8 += 0x40000;

    gpBuildVersion = (char*)&VersionNumber;

    Tools_GetTimeInSeconds();

    gMKDefaults.screenshot = System_GetCommandLineParameter("-screenshot") != NULL;
}

void Main_AutoLevelSwitch_Update(void) {
    static int modelInstanceCount = 0;
}

void Game_Init(void) {
    GameData_Init();
    gb.mGameData.Init();

    Translation_SetLanguage(Translation_GetDefaultLanguage());

    objectManager.Init();

    System_SetDebugCameraSpeedMultiplier(100.0f);

    gb.level.Init();

    ProcessCommandLine();

    SoundBank_LoadResources();

    MKPackage_Load("frontend", false);

    if (gDisplay.region == 1) {
        gb.mLogicState.Init(STATE_3);
    } else {
        gb.mLogicState.Init(STATE_2);
    }
}

int Game_Update(void) {
    View::pCurrentView = NULL;

    Main_AutoLevelSwitch_Update();
    Main_CheckInputs();

    return false;
}

void Game_Draw(void) {

}

void Game_Deinit(void) {
    defaultParticleManager.Deinit();
    LOD_Deinit();
    Weather_Deinit();
    objectManager.Deinit();
    MKGrass_Deinit();
    Collision_Deinit();
}

float ValidPIRange(float f1) {
    if (f1 < 0.0f) {
        return f1 + (2 * PI);
    } else if (f1 > (2 * PI)) {
        return f1 - (2 * PI);
    } else {
        return f1;
    }
}

static void Main_CheckJoypadButtons(JoyPad* pJoyPad) {
    if ((pJoyPad->mButtonsPressed & 0x1000) && !(pJoyPad->mPrevButtonsPressed & 0x1000)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x1000;
    }

    if ((pJoyPad->mButtonsPressed & 0x2000) && !(pJoyPad->mPrevButtonsPressed & 0x2000)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x2000;
    }

    if ((pJoyPad->mButtonsPressed & 0x4000) && !(pJoyPad->mPrevButtonsPressed & 0x4000)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x4000;
    }

    if ((pJoyPad->mButtonsPressed & 0x8000) && !(pJoyPad->mPrevButtonsPressed & 0x8000)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x8000;
    }

    if ((pJoyPad->mButtonsPressed & 0x400) && !(pJoyPad->mPrevButtonsPressed & 0x400)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x400;
    }

    if ((pJoyPad->mButtonsPressed & 0x800) && !(pJoyPad->mPrevButtonsPressed & 0x800)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x800;
    }

    if ((pJoyPad->mButtonsPressed & 0x100) && !(pJoyPad->mPrevButtonsPressed & 0x100)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x100;
    }

    if ((pJoyPad->mButtonsPressed & 0x200) && !(pJoyPad->mPrevButtonsPressed & 0x200)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x200;
    }

    if ((pJoyPad->mButtonsPressed & 0x2) && !(pJoyPad->mPrevButtonsPressed & 0x2)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x2;
    }

    if ((pJoyPad->mButtonsPressed & 0x4) && !(pJoyPad->mPrevButtonsPressed & 0x4)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x4;
    }

    if ((pJoyPad->mButtonsPressed & 0x1) && !(pJoyPad->mPrevButtonsPressed & 0x1)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x1;
    }

    if ((pJoyPad->mButtonsPressed & 0x8) && !(pJoyPad->mPrevButtonsPressed & 0x8)) {
        pJoyPad->mNewButtonsPressedThisFrame += 0x8;
    }
}

static void Main_CheckJoypad(JoyPad* pJoyPad) {
    pJoyPad->mPrevButtonsPressed = pJoyPad->mButtonsPressed;
    pJoyPad->unk40 = pJoyPad->unk38;
    pJoyPad->unk44 = pJoyPad->unk3C;
    pJoyPad->unk50 = pJoyPad->unk48;
    pJoyPad->unk54 = pJoyPad->unk4C;

    pJoyPad->mPrevNewButtons = pJoyPad->mNewButtonsPressedThisFrame;
    pJoyPad->mNewButtonsPressedThisFrame = 0;
    pJoyPad->mButtonsPressed = 0;
    pJoyPad->mButtonsPressed = 0;

    for (int i = 0; i < 16; i++) {
        if (Input_GetButtonState(pJoyPad->mInputDeviceID, i, NULL) != 0) {
            pJoyPad->mButtonsPressed |= 1 << i;
        }
    }
}

void EnableTechnoRangs(void) {
    gb.mGameData.SetHasRang(BR_Megarang, true);
    gb.mGameData.SetHasRang(BR_Kaboomerang, true);
    gb.mGameData.SetHasRang(BR_Infrarang, true);
    gb.mGameData.SetHasRang(BR_Zoomerang, true);
    gb.mGameData.SetHasRang(BR_Doomerang, true);
    gb.mGameData.SetHasRang(BR_Multirang, true);
    gb.mGameData.SetHasRang(BR_Chronorang, true);

    ty.mBoomerangManager.SetHasBoth(true);

    gb.mGameData.SetBothRangs(true);
}

void EnableElementalRangs(void) {
    gb.mGameData.SetHasRang(BR_Aquarang, true);
    gb.mGameData.SetHasRang(BR_Frostyrang, true);
    gb.mGameData.SetHasRang(BR_Flamerang, true);
    gb.mGameData.SetHasRang(BR_Zappyrang, true);

    ty.mBoomerangManager.SetHasBoth(true);

    gb.mGameData.SetBothRangs(true);

    gb.mGameData.SetLearntToSwim(true);
    gb.mGameData.SetLearntToDive(true);
}

void SpecialPickup_EnableCheatLines(bool);
void Bilby_EnableCheatLines(bool);

static void DrawCheatLines(void) {
    bDrawCheatLines = !bDrawCheatLines;

    SpecialPickup_EnableCheatLines(bDrawCheatLines);
    Bilby_EnableCheatLines(bDrawCheatLines);
}

static void ToggleInfiniteHealth(void) {
    gb.infinitePie = !gb.infinitePie;
}

extern void Portal_ShowBonusWorlds(void);

static void ToggleBonusWorlds(void) {
    Portal_ShowBonusWorlds();
}

extern bool bEnableGallery;
extern bool bEnableMovies;

static void ToggleGallery(void) {
    bEnableGallery = !bEnableGallery;
}

static void ToggleMovies(void) {
    bEnableMovies = !bEnableMovies;
}

extern bool bShowAttractTimeout;
extern int secondsBeforeTimeout;

extern void ToggleChangeLevel(void);

struct CheatEntry {
    int buttonIdx;
    int numButtons;
    int unk8;
    void (*pFunc)(void);
    int buttons[20];
};

static CheatEntry cheats[] = {
    {0, 10, 5, ToggleInfiniteHealth, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x1000, 0x2000, 0x2000}},
    {0, 10, 5, EnableTechnoRangs, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x8000, 0x1000, 0x8000}},
    {0, 10, 5, EnableElementalRangs, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x8000, 0x8000, 0x1000, 0x8000}},
    {0, 12, 5, DrawCheatLines, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x8000, 0x8000, 0x2000, 0x200, 0x200}},
    {0, 12, 5, ToggleChangeLevel, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x2000, 0x1000, 0x2000, 0x2000, 0x1000}},
    {0, 12, 5, ToggleBonusWorlds, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x2000, 0x4000, 0x8000, 0x4000, 0x8000}},
    {0, 12, 5, ToggleGallery, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x4000, 0x4000, 0x200, 0x8000, 0x200, 0x8000}},
    {0, 12, 5, ToggleMovies, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x4000, 0x4000, 0x2000, 0x200, 0x2000, 0x200}},
};

// Macro works (they do create rodata symbols that would get stripped in this file)

// // Macro to easily define a cheat entry (logic state, function, var args of button masks)
// #define CHEAT_ENTRY(logicState, cheatFunc, ...) {0, ARRAY_SIZE(((int[]){__VA_ARGS__})), (logicState), (cheatFunc), {__VA_ARGS__}}

// // {0, 10, 5, ToggleInfiniteHealth, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x1000, 0x2000, 0x2000}},
// static CheatEntry cheats[] = {
//     CHEAT_ENTRY(5, ToggleInfiniteHealth, 0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x1000, 0x2000, 0x2000),
//     {0, 10, 5, EnableTechnoRangs, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x8000, 0x1000, 0x8000}},
//     {0, 10, 5, EnableElementalRangs, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x8000, 0x8000, 0x1000, 0x8000}},
//     {0, 12, 5, DrawCheatLines, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x8000, 0x8000, 0x2000, 0x200, 0x200}},
//     {0, 12, 5, ToggleChangeLevel, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x2000, 0x1000, 0x2000, 0x2000, 0x1000}},
//     {0, 12, 5, ToggleBonusWorlds, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x2000, 0x4000, 0x8000, 0x4000, 0x8000}},
//     {0, 12, 5, ToggleGallery, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x4000, 0x4000, 0x200, 0x8000, 0x200, 0x8000}},
//     {0, 12, 5, ToggleMovies, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x4000, 0x4000, 0x2000, 0x200, 0x2000, 0x200}},
// };

// #define CHEAT_BUTTONS(...) {__VA_ARGS__}

// // Macro to easily define a cheat entry (logic state, function, var args of button masks)
// #define CHEAT_ENTRY(logicState, cheatFunc, buttons) {0, ARRAY_SIZE(((int[])buttons)), (logicState), (cheatFunc), buttons}
// static CheatEntry cheats[] = {
//     CHEAT_ENTRY(5, ToggleInfiniteHealth, CHEAT_BUTTONS(0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x1000, 0x2000, 0x2000)),
//     CHEAT_ENTRY(5, EnableTechnoRangs, CHEAT_BUTTONS(0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x8000, 0x1000, 0x8000)),
//     CHEAT_ENTRY(5, EnableElementalRangs, CHEAT_BUTTONS(0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x8000, 0x8000, 0x1000, 0x8000)),
//     CHEAT_ENTRY(5, DrawCheatLines, CHEAT_BUTTONS(0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x8000, 0x8000, 0x2000, 0x200, 0x200)),
//     CHEAT_ENTRY(5, ToggleChangeLevel, CHEAT_BUTTONS(0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x2000, 0x1000, 0x2000, 0x2000, 0x1000)),
//     CHEAT_ENTRY(5, ToggleBonusWorlds, CHEAT_BUTTONS(0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x2000, 0x4000, 0x8000, 0x4000, 0x8000)),
//     CHEAT_ENTRY(5, ToggleGallery, CHEAT_BUTTONS(0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x4000, 0x4000, 0x200, 0x8000, 0x200, 0x8000)),
//     CHEAT_ENTRY(5, ToggleMovies, CHEAT_BUTTONS(0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x4000, 0x4000, 0x2000, 0x200, 0x2000, 0x200)),
// };

static int cheatCount = ARRAY_SIZE(cheats);

void FrontEnd_GoToScreen(tagFrontEndScreen);

void Main_CheckShortcutKeys(void) {
    if (gb.bE3) {
        static int timeout = (int)gDisplay.fps * 60;

        if (gb.mLogicState.currState == STATE_5) {
            if (
                gb.mJoyPad1.mButtonsPressed != gb.mJoyPad1.mPrevButtonsPressed || 
                gb.mJoyPad1.unk38 != 0.0f || 
                gb.mJoyPad1.unk48 != 0.0f || 
                gb.mJoyPad1.unk3C != 0.0f || 
                gb.mJoyPad1.unk4C != 0.0f
            ) {
                timeout = (int)gDisplay.fps * 60;
            }

            if (gb.pDialogPlayer) {
                timeout = (int)gDisplay.fps * 60;
            }

            if (!gb.unk7AC) {
                timeout--;
            }

            bShowAttractTimeout = timeout <= ((int)gDisplay.fps * 10);

            if (bShowAttractTimeout) {
                secondsBeforeTimeout = timeout / (int)gDisplay.fps;
            }

            if (timeout <= 0) {
                timeout = (int)gDisplay.fps * 60;
                FrontEnd_GoToScreen((tagFrontEndScreen)0);
            }
        }
    }

    if (gb.mJoyPad1.mButtonsPressed != gb.mJoyPad1.mPrevButtonsPressed) {
        for (int i = 0; i < cheatCount; i++) {
            if (cheats[i].unk8 == gb.mLogicState.GetCurr()) {
                int button = cheats[i].buttons[cheats[i].buttonIdx];

                int pressed = gb.mJoyPad1.mButtonsPressed;
                pressed &= button;

                if (!pressed || (gb.mJoyPad1.mPrevButtonsPressed & button)) {
                    if (!pressed && (gb.mJoyPad1.mPrevButtonsPressed & button)) {
                        cheats[i].buttonIdx++;

                        if (cheats[i].buttonIdx >= cheats[i].numButtons) {
                            SoundBank_Play(0x1BB, NULL, 0);

                            if (cheats[i].pFunc) {
                                cheats[i].pFunc();
                            }

                            cheats[i].buttonIdx = 0; // reset the input sequence
                        }
                    } else {
                        cheats[i].buttonIdx = 0; // reset the input sequence
                    }
                }
            }
        }
    }
}

static void Main_CheckInputs(void) {
    Main_CheckShortcutKeys();

    if (gb.mLogicState.currState == STATE_10 || gb.mLogicState.currState == STATE_11) {
        gb.mJoyPad1.SetNewFrame();

        Main_CheckJoypad(&gb.mJoyPad1);
        Main_CheckJoypadButtons(&gb.mJoyPad1);

        Main_CheckJoypad(&gb.mJoyPad2);
        Main_CheckJoypadButtons(&gb.mJoyPad2);
    }
}

int Main_GetCameraDistanceTypeForLevel(void) {

}

void Main_UpdateGlobalGameCameraHeroInfo(void) {

}

extern "C" void Sound_MusicStop(void);

void Main_ChangeLevel(LevelNumber newLevel) {
    Sound_MusicStop();

    gb.level.ChangeLevel(newLevel);

    if (!Main_DoVideo()) {
        gb.mLogicState.Set(STATE_9);
    }
}
