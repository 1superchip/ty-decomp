
#include "ty/main.h"
#include "ty/Ty.h"
#include "ty/global.h"
#include "ty/GameObjectManager.h"
#include "ty/heatflare.h"
#include "ty/LensFlare.h"
#include "ty/RenderTexture.h"
#include "ty/friends/friend.h"
#include "ty/ParticleEngine.h"
#include "ty/frontend/FrontEnd.h"
#include "ty/Mist.h"

#include "ty/props/AnimatingProp.h"
#include "ty/props/Shatterable.h"
#include "ty/props/WeatherProp.h"

#include "ty/boomerang.h"
#include "ty/Torch.h"
#include "ty/bunyip.h"
#include "ty/Script.h"
#include "ty/SignPost.h"
#include "ty/FinishLine.h"
#include "ty/effects/Weather.h"

#include "common/FileSys.h"
#include "common/ParticleSystemManager.h"
#include "common/MKPackage.h"
#include "common/Translation.h"
#include "common/MKGrass.h"

void GameCamera_InitModule(unsigned int*);

void GameCamera_Update(GameCameraHeroInfo*);

static void Main_CheckInputs(void);

int MaxMemoryUsed[TOTAL_LEVEL_MAX] = {};

int debugCollision = 0;

void Main_InitGameSystems(void) {
    ParticleSystemManager_Init(300, 2500);

    Main_InitStaticModules();
    Main_LoadStaticResources();

    dda.Init();

    InitializeGame();

    Weather_Enable(false);
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

    Boomerang_LoadResources(&ini);

    Bunyip_LoadResources(&ini);
    StaticProp_LoadResources(&ini);
    Script_LoadResources(&ini);
    WeatherProp_LoadResources(&ini);

    Gem_LoadResources(&ini);

    SignPost_LoadResources(&ini);

    FinishLine_LoadResources(&ini);
    Torch_LoadResources(&ini);

    Shatterable_LoadResources(&ini);

    Platform_LoadResources(&ini);
    AnimatingProp_LoadResources(&ini);

    ini.Deinit();

    Hud_Init(&ty);
    
    MKPackage_Free();
}

void FrontEnd_GoToScreen(tagFrontEndScreen);

bool Main_DoVideo(void) {
    switch (gb.level.GetPreviousLevel()) {
        case LN_RAINBOW_CLIFFS:
            switch (gb.level.GetCurrentLevel()) {
                case LN_BRIDGE_RIVER_TY:
                case LN_SNOW_WORRIES:
                case LN_OUTBACK_SAFARI:
                    if (!gb.mGameData.HasLevelBeenEntered(LN_BRIDGE_RIVER_TY) &&
                        !gb.mGameData.HasLevelBeenEntered(LN_SNOW_WORRIES) &&
                        !gb.mGameData.HasLevelBeenEntered(LN_OUTBACK_SAFARI) &&
                        !gb.mGameData.CheckVideoFlag(0x4)
                    ) {
                        FrontEnd_GoToScreen((tagFrontEndScreen)0x13);
                        return true;
                    }
                    break;
                case LN_11:
                    break;
                case LN_LYRE_LYRE:
                case LN_BLACK_STUMP:
                case LN_REX_MARKS_SPOT:
                    if (!gb.mGameData.HasLevelBeenEntered(LN_LYRE_LYRE) &&
                        !gb.mGameData.HasLevelBeenEntered(LN_BLACK_STUMP) &&
                        !gb.mGameData.HasLevelBeenEntered(LN_REX_MARKS_SPOT) &&
                        !gb.mGameData.CheckVideoFlag(0x8)
                    ) {
                        FrontEnd_GoToScreen((tagFrontEndScreen)0x14);
                        gb.mGameData.SetVideoFlag(0x8);
                        return true;
                    }
                    break;
                case LN_CASS_PASS:
                    if (!gb.mGameData.HasLevelBeenEntered(LN_CASS_PASS) && !gb.mGameData.CheckVideoFlag(0x10)) {
                        FrontEnd_GoToScreen((tagFrontEndScreen)0x15);
                        gb.mGameData.SetVideoFlag(0x10);
                        return true;
                    }
                    break;
            }
            break;
        case LN_TWO_UP:
            if (gb.mGameData.pSaveData->tyAttributes.bBothRangs && !gb.mGameData.CheckVideoFlag(0x2)) {
                FrontEnd_GoToScreen((tagFrontEndScreen)0x12);
                gb.mGameData.SetVideoFlag(0x2);
                return true;
            }
            break;
        case LN_BULLS_PEN:
            if (gb.mGameData.HasTalisman(TALISMAN_TYPE_0) && !gb.mGameData.CheckVideoFlag(0x20)) {
                FrontEnd_GoToScreen((tagFrontEndScreen)0x16);
                gb.mGameData.SetVideoFlag(0x20);
                return true;
            }
            break;
        case LN_FLUFFYS_FJORD:
            if (gb.mGameData.HasTalisman(TALISMAN_TYPE_2) && !gb.mGameData.CheckVideoFlag(0x40)) {
                FrontEnd_GoToScreen((tagFrontEndScreen)0x17);
                gb.mGameData.SetVideoFlag(0x40);
                return true;
            }
            break;
        case LN_CRIKEYS_COVE:
            if (gb.mGameData.HasTalisman(TALISMAN_TYPE_1) && !gb.mGameData.CheckVideoFlag(0x80)) {
                FrontEnd_GoToScreen((tagFrontEndScreen)0x18);
                gb.mGameData.SetVideoFlag(0x80);
                return true;
            }
            break;
    }

    return false;
}

Vector* GameCamera_GetPos(void);

void TyMemCard_LoadResources(void);
bool TyMemCard_IsAutoSaving(void);
int TyMemCard_Update(void);
bool TyMemCard_WasCardRemoved(void);

enum PauseScreen_State {};
void PauseScreen_Init(PauseScreen_State);
void PauseScreen_Update(void);
void PauseMap_UpdateHeroPos(void);

void BonusPickupParticle_Update(void);

void Shatter_Update(void);
void Hud_Update(void);
void Dialog_Update(void);
void Particle_Update(void);

extern int dialogWaitHackCounter;

void LogicGame(void) {
    gSceneManager.SetActivePoint(GameCamera_GetPos());

    if (TyMemCard_IsAutoSaving()) {
        switch (TyMemCard_Update()) {
            case 4:
                gb.bOnPauseScreen = true;
                PauseScreen_Init((PauseScreen_State)0xD);
                break;
            case 5:
                gb.bOnPauseScreen = true;
                PauseScreen_Init((PauseScreen_State)0xE);
                break;
            case 6:
                gb.bOnPauseScreen = true;
                PauseScreen_Init((PauseScreen_State)0xF);
                break;
        }
    }

    if (gb.unk700) {
        SoundBank_SetVolume(gFERes.mFader.GetFadePercentage(), SOUND_VOLUME_SET | MUSIC_VOLUME_SET);

        if (!gFERes.mFader.Update()) {
            Main_ChangeLevel(gb.level.unk400);
            gb.unk700 = false;
        }

        return;
    }

    if (!gb.bOnPauseScreen && !TyMemCard_IsAutoSaving() && TyMemCard_WasCardRemoved()) {
        gb.bOnPauseScreen = true;

        PauseScreen_Init((PauseScreen_State)0xD);
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

    ty.unkDE0.SetZero();

    Range_Update();

    dda.Update();
    Weather_Update();
    Gem_PickupParticle_Update();
    BonusPickupParticle_Update();

    ParticleSystemManager_Update();

    ManuallyScrollTextures();

    HeatFlare_Update();

    Tools_DropShadow_Update();

    if ((gb.logicGameCount % 3) == 0) {
        float f = gb.unk78C; // ???
        if (Abs<float>(gb.unk78C) < 0.1f) {
            gb.unk78C = 0.0f;
        } else {
            gb.unk78C = f * -0.8f;
        }
    }

    if (gb.unk7AC) {
        gb.mJoyPad1.mButtonsPressed             &= (tyControl.buttonVals[1] | tyControl.buttonVals[0]);
        gb.mJoyPad1.mNewButtonsPressedThisFrame &= (tyControl.buttonVals[1] | tyControl.buttonVals[0]);

        gb.mJoyPad1.Reset_Unk58_Unk5C();
        gb.mJoyPad1.GetAnalogControl()->Reset();
    }

    Shatter_Update();

    gSceneManager.UpdateProps();
    objectManager.UpdateModules();

    Dialog_Update();

    if (dialogWaitHackCounter > -1) {
        return;
    }

    Mist_Update();
    Gem_ParticleSystem_Update();
    MKGrass_Update();
    Explosion_Update();
    SleepyDust_UpdateAll();
    RenderTexture_Update();

    gb.mGameFsm.Update();

    VibrateUpdate();
    Particle_Update();

    particleManager->SetCamera(&ty.pos, &ty.pos);
    particleManager->Update();

    LensFlare_Update();

    GameCamera_Update(Main_UpdateGlobalGameCameraHeroInfo());

    Hud_Update();

    Collision_Update();
}

void GameSubStateFSM::InGameUpdate(void) {
    Ty_Update();
}

void GameSubStateFSM::InGameDraw(void) {
    Boomerang_Draw();
    Ty_Draw();

    if (!gb.pDialogPlayer) {
        
    }
}

extern "C" int sscanf(const char*, const char*, ...);

void ProcessCommandLine(void) {
    if (System_GetCommandLineParameter("-e3")) {
        gb.bE3 = true;
    }

    if (System_GetCommandLineParameter("-skipvideo")) {
        gb.skipVideo = true;
    } else {
        gb.skipVideo = false;
    }
    
    if (System_GetCommandLineParameter("-debug")) {
        gb.debug = true;
    } else {
        gb.debug = false;
    }
    
    if (System_GetCommandLineParameter("-devLevelWarp")) {
        gb.devLevelWarp = true;
    } else {
        gb.devLevelWarp = false;
    }
    
    if (System_GetCommandLineParameter("-disableFrontend")) {
        gb.disableFrontend = true;
        gb.mJoyPad1.SetInputDevice(CHAN_0);
    } else {
        gb.disableFrontend = false;
    }
    
    if (System_GetCommandLineParameter("-disableLensFlare")) {
        gb.disableLensFlare = true;
    } else {
        gb.disableLensFlare = false;
    }

    gb.disableEnemies   = System_GetCommandLineParameter("-disableEnemies") != NULL;
    gb.disableFriends   = System_GetCommandLineParameter("-disableFriends") != NULL;
    gb.disableTriggers  = System_GetCommandLineParameter("-disabletriggers") != NULL;
    gb.enableBoss       = System_GetCommandLineParameter("-enableBoss") != NULL;
    
    if (System_GetCommandLineParameter("-fastLoad")) {
        gb.fastLoad = true;
        debugCollision = 1;
    } else {
        gb.fastLoad = false;
    }
    
    char* pLevelArg = System_GetCommandLineParameter("-level");
    if (pLevelArg) {
        gb.level.newLevelNumber = gb.level.GetLevelNumber(pLevelArg);
    } else {
        gb.level.newLevelNumber = gb.bE3 ? LN_1 : LN_RAINBOW_CLIFFS;
    }

    gb.level.ChangeLevel(gb.level.newLevelNumber);

    gb.b3DSound = System_GetCommandLineParameter("-3dsound") != NULL;
    
    char* pChargeBiteArg = System_GetCommandLineParameter("-chargeBites");
    int numBites = 0;
    if (pChargeBiteArg) {
        sscanf(pChargeBiteArg, "%d", &numBites);
    }

    gb.mNumChargeBites = numBites;

    gb.whackyness   = System_GetCommandLineParameter("-whackyness") != NULL;
    gb.infinitePie  = System_GetCommandLineParameter("-infinitePie") != NULL;
    
    if (System_GetCommandLineParameter("-tydev")) {
        gb.mNumChargeBites = 100;
        gb.devLevelWarp = true;
        gb.disableFrontend = true;
        gb.infinitePie = true;
        gb.disableLensFlare = true;
    }
    
    if (System_GetCommandLineParameter("-eadev")) {
        gb.mNumChargeBites = 100;
        gb.devLevelWarp = true;
        gb.disableLensFlare = true;
        gb.disableFrontend = true;
        gb.infinitePie = true;
    }

    if (System_GetCommandLineParameter("-autoLevelSwitch")) {
        gb.autoLevelSwitch = true;
        gb.unkE85 = false;
        gb.introOnly = false;
        gb.unkE88 = 15;
        gb.unkE8C = 0;
        gb.unkE90 = 99;
        gb.fastDialogs = false;
    } else {
        gb.autoLevelSwitch = false;
    }

    if (System_GetCommandLineParameter("-introOnly")) {
        gb.introOnly = true;
    }

    if (System_GetCommandLineParameter("-fastDialogs")) {
        gb.fastDialogs = true;
    }
}

void Game_InitSystem(void) {
    if (System_GetCommandLineParameter("-cameraDebug")) {
        gMKDefaults.debugParticle_count = 3000;
        gMKDefaults.debugSphere_count = 3000;
    }

    gMKDefaults.unk8 += 0x40000;

    gpBuildVersion = (char*)&VersionNumber;

    gLevelLoadStartTime = Tools_GetTimeInSeconds();

    gMKDefaults.grassPtrListDLCount = 32;

    gMKDefaults.lockTo30 = true;

    gMKDefaults.screenshot = System_GetCommandLineParameter("-screenshot") != NULL;
}

bool Dialog_IsLoading(void);
bool Dialog_IsPlaying(void);

bool Dialog_Play(int, bool);

extern "C" void printf(char*, ...);

static LevelNumber ln[] = {
    LN_RAINBOW_CLIFFS, 

    LN_TWO_UP, 
    LN_WALK_IN_THE_PARK, 
    LN_SHIP_REX, 
    LN_BULLS_PEN, 

    LN_BRIDGE_RIVER_TY, 
    LN_SNOW_WORRIES, 
    LN_OUTBACK_SAFARI, 

    LN_LYRE_LYRE, 
    LN_BLACK_STUMP, 
    LN_REX_MARKS_SPOT, 
    LN_FLUFFYS_FJORD, 

    LN_16, 
    LN_CASS_CREST, 
    LN_CRIKEYS_COVE, 

    LN_CASS_PASS, 
    LN_BONUS_WORLD_DAY, 
    LN_BONUS_WORLD_NIGHT, 
    LN_FINAL_BATTLE
};

static int lnCount = ARRAY_SIZE(ln);

void Main_AutoLevelSwitch_Update(void) {
    static int modelInstanceCount = 0;

    if (!gb.bOnPauseScreen) {
        switch (gb.mLogicState.GetCurr()) {
            case STATE_5:
                MaxMemoryUsed[gb.level.GetCurrentLevel()] = Max<int>(Heap_MemoryUsed(), MaxMemoryUsed[gb.level.GetCurrentLevel()]);

                if (!gb.autoLevelSwitch || Dialog_IsLoading() || Dialog_IsPlaying() || (--gb.unkE88 >= 0)) {
                    break;
                }

                gb.unkE88 = 15;

                if (!gb.introOnly && gb.unkE90 >= 0) {
                    bool r29 = false;
                    int i = gb.unkE90 - 1;
                    while (i >= 0) {
                        if (modelInstanceCount != 0 && modelInstanceCount != modelInstances.GetLength()) {
                            printf(
                                "Model Instances Error (Was %d, Now %d) D %d, L %d\n", 
                                modelInstanceCount, 
                                modelInstances.GetLength(),
                                i,
                                ln[gb.unkE8C]
                            );
                        }

                        modelInstanceCount = modelInstances.GetLength();

                        if (Dialog_Play(i, false)) {
                            gb.unkE90 = i;
                            gb.unkE94 = true;
                            r29 = true;
                            break;
                        }

                        modelInstanceCount = modelInstances.GetLength();
                        i--;
                    }

                    if (!r29) {
                        gb.unkE90 = -1;
                    }

                    break;
                }


                for (gb.unkE8C = 0; gb.unkE8C < lnCount; gb.unkE8C++) {
                    if (ln[gb.unkE8C] == gb.level.GetCurrentLevel()) {
                        break;
                    }
                }

                gb.unkE8C++;

                if (gb.unkE8C >= lnCount) {
                    if (System_GetCommandLineParameter("-outputPackages")) {
                        gb.autoLevelSwitch = false;

                        if (System_GetCommandLineParameter("-outputOrder")) {
                            FileSys_OutputFileOrder();
                        }
                    }

                    gb.unkE8C = 0;

                    if (System_GetCommandLineParameter("-autoLanguageSwitch")) {
                        TranslationLanguage lang = (TranslationLanguage)((int)Translation_GetLanguage() + 1);

                        while (lang < LANGUAGE_NMBR_OF_LANGUAGES && (lang == Language_American && !Translation_IsLanguageAvailable(lang))) {
                            lang = (TranslationLanguage)((int)lang + 1);
                        }

                        if (lang < LANGUAGE_NMBR_OF_LANGUAGES) {
                            Translation_SetLanguage(lang);
                            gb.autoLevelSwitch = true;
                        } else {
                            gb.autoLevelSwitch = false;
                        }
                    }
                }

                modelInstanceCount = 0;
                gb.unkE90 = 99;
                Main_ChangeLevel(ln[gb.unkE8C]);
                break;
        }
    }
}

void LoadResources_PreLoadGraphics(void);

void Game_Init(void) {
    gb.bE3 = false;

    gb.mJoyPad1.SetInputDevice(CHAN_0);

    gb.unk6F9 = false;

    GameData_Init();
    gb.mGameData.Init();

    Translation_SetLanguage(Translation_GetDefaultLanguage());

    objectManager.Init();

    System_SetDebugCameraSpeedMultiplier(100.0f);

    gb.level.Init();

    ProcessCommandLine();

    SoundBank_LoadResources();

    FrontEnd_LoadResources();
    TyMemCard_LoadResources();
    LoadResources_PreLoadGraphics();

    MKPackage_Load("frontend", false);

    if (gDisplay.region == 1) {
        gb.mLogicState.Init(STATE_3);
    } else {
        gb.mLogicState.Init(STATE_2);
    }
}

void LanguageSelect_Init(void);
void LanguageSelect_Update(void);
void LanguageSelect_Deinit(void);

void TyMemCard_SetAppInfo(void);

void BootTest_Init(void);
void BootTest_Update(void);

void LoadResources_Init(void);
void LoadResources_Update(void);
void LoadResources_Deinit(void);

void FrontEnd_Init(void);
void FrontEnd_Update(void);
void FrontEnd_Deinit(void);

void FrontEnd_LoadLevel_Init(void);
void FrontEnd_LoadLevel_Update(void);
void FrontEnd_LoadLevel_Deinit(void);

void Copyright_Update(void);

int Game_Update(void) {
    View::pCurrentView = NULL;

    Main_AutoLevelSwitch_Update();
    Main_CheckInputs();

    if (gb.mLogicState.unkC != 0) {
        gb.mLogicState.unkC--;

        if (gb.mLogicState.unkC != 0) {
            return false;
        }

        gb.mLogicState.Update();

        switch (gb.mLogicState.GetCurr()) {
            case STATE_NONE:
                break;
            case STATE_1:
                break;
            case STATE_2:
                LanguageSelect_Init();
                break;
            case STATE_3:
                TyMemCard_SetAppInfo();
                BootTest_Init();
                break;
            case STATE_4:
                LoadResources_Init();
                break;
            case STATE_5:
                break;
            case STATE_6:
                break;
            case STATE_7:
                break;
            case STATE_8:
                FrontEnd_Init();
                break;
            case STATE_9:
                FrontEnd_LoadLevel_Init();
                break;
        }
    }

    if (dialogWaitHackCounter == -1) {
        switch (gb.mLogicState.GetCurr()) {
            case STATE_5:
                LogicGame();
                break;
            case STATE_6:
            case STATE_7:
                if (gb.mLogicState.GetCurr() != STATE_7) {
                    GameCamera_Update(Main_UpdateGlobalGameCameraHeroInfo());
                }

                if (Input_GetButtonState(gb.mJoyPad1.mInputDeviceID, 0xA, NULL) != 0) {
                    static uint FirstFrame = 0;

                    if ((Input_WasButtonPressed(gb.mJoyPad1.mInputDeviceID, 0xA, NULL) & 0xFF) != 0) {
                        LogicGame();

                        FirstFrame = gFrameCounter;
                    } else if ((gFrameCounter - FirstFrame > 10) && (gFrameCounter % 10) == 0) {
                        LogicGame();
                    }
                }
                break;
            case STATE_4:
                LoadResources_Update();
                break;
            case STATE_2:
                LanguageSelect_Update();
                break;
            case STATE_3:
                BootTest_Update();
                break;
            case STATE_1:
                Copyright_Update();
                break;
            case STATE_8:
                FrontEnd_Update();
                break;
            case STATE_9:
                FrontEnd_LoadLevel_Update();
                break;
            case STATE_10:
                LogicGame();
                break;
            case STATE_11:
                GameCamera_Update(Main_UpdateGlobalGameCameraHeroInfo());
                break;
        }

        if (gb.mLogicState.nextGameState != STATE_NONE) {
            switch (gb.mLogicState.GetCurr()) {
                case STATE_4:
                    LoadResources_Deinit();
                    gb.mLogicState.unkC = 12;
                    break;
                case STATE_2:
                    LanguageSelect_Deinit();
                    gb.mLogicState.unkC = 1;
                    break;
                case STATE_8:
                    FrontEnd_Deinit();
                    gb.mLogicState.unkC = 12;
                    break;
                case STATE_9:
                    FrontEnd_LoadLevel_Deinit();
                    gb.mLogicState.unkC = 1;
                    break;
                case STATE_5:
                    if (gb.mLogicState.nextGameState == STATE_8 || gb.mLogicState.nextGameState == STATE_9) {
                        DeinitializeLevel();
                        gb.mLogicState.unkC = 12;
                    } else {
                        gb.mLogicState.unkC = 1;
                    }
                    break;
                default:
                    gb.mLogicState.unkC = 1;
                    break;
            }
        }
    }

    SoundBank_Update();

    return false;
}

void Game_Draw(void) {

}

void Game_Deinit(void) {
    ParticleSystemManager_Deinit();
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

float Main_GetValidJoypadAxisValue(int val) {
    float m;

    if (val < 128) {
        m = (127.0f - val) * -1.0f;
    } else {
        m = val - 127.0f;
    }

    m /= 127.0f;

    float scalar = (1.0f / (1.0f - joystickDeadZone));

    if (m < -joystickDeadZone) {
        m = (m + joystickDeadZone) * scalar;
    } else if (m > joystickDeadZone) {
        m = (m - joystickDeadZone) * scalar;
    } else {
        m = 0.0f;
    }

    if (m < -1.0f) {
        m = -1.0f;
    }
    
    if (m > 1.0f) {
        m = 1.0f;
    }

    return m;
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

    int r29 = Input_GetButtonState(pJoyPad->mInputDeviceID, 0x11, NULL);
    int r30 = Input_GetButtonState(pJoyPad->mInputDeviceID, 0x10, NULL);
    int r28 = Input_GetButtonState(pJoyPad->mInputDeviceID, 0x13, NULL);
    int r3 = Input_GetButtonState(pJoyPad->mInputDeviceID, 0x12, NULL);

    if (r29 == 0 && r30 == 0 && r28 == 0 && r3 == 0) {
        r29 = r30 = r28 = r3 = 127;
    }

    pJoyPad->unk38 = Main_GetValidJoypadAxisValue(r29);
    pJoyPad->unk3C = Main_GetValidJoypadAxisValue(r30);
    pJoyPad->unk48 = Main_GetValidJoypadAxisValue(r28);
    pJoyPad->unk4C = Main_GetValidJoypadAxisValue(r3);

    pJoyPad->unk60 = pJoyPad->unk58;
    pJoyPad->unk64 = pJoyPad->unk5C;

    pJoyPad->unk58 = pJoyPad->unk5C = 0.0f;

    float tmp38 = pJoyPad->unk38;
    float tmp3C = pJoyPad->unk3C;

    pJoyPad->unk58 = tmp38;
    pJoyPad->unk5C = tmp3C;

    pJoyPad->mAnalogControl.Update(pJoyPad->unk58, pJoyPad->unk5C);

    pJoyPad->unk68 = Max<float>(Abs<float>(pJoyPad->unk40 - pJoyPad->unk38), Abs<float>(pJoyPad->unk44 - pJoyPad->unk3C));
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
    GameState unk8;
    void (*pFunc)(void);
    int buttons[20];
};

static CheatEntry cheats[] = {
    {0, 10, STATE_5, ToggleInfiniteHealth, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x1000, 0x2000, 0x2000}},
    {0, 10, STATE_5, EnableTechnoRangs, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x1000, 0x8000, 0x1000, 0x8000}},
    {0, 10, STATE_5, EnableElementalRangs, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x8000, 0x8000, 0x1000, 0x8000}},
    {0, 12, STATE_5, DrawCheatLines, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x8000, 0x8000, 0x2000, 0x200, 0x200}},
    {0, 12, STATE_5, ToggleChangeLevel, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x2000, 0x1000, 0x2000, 0x2000, 0x1000}},
    {0, 12, STATE_5, ToggleBonusWorlds, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x2000, 0x2000, 0x4000, 0x8000, 0x4000, 0x8000}},
    {0, 12, STATE_5, ToggleGallery, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x4000, 0x4000, 0x200, 0x8000, 0x200, 0x8000}},
    {0, 12, STATE_5, ToggleMovies, {0x400, 0x800, 0x400, 0x800, 0x1000, 0x1000, 0x4000, 0x4000, 0x2000, 0x200, 0x2000, 0x200}},
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

void Main_CheckShortcutKeys(void) {
    if (gb.bE3) {
        static int timeout = (int)gDisplay.fps * 60;

        if (gb.mLogicState.GetCurr() == STATE_5) {
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

    if (gb.mLogicState.GetCurr() == STATE_10 || gb.mLogicState.GetCurr() == STATE_11) {
        gb.mJoyPad1.SetInputDevice(CHAN_0);
    }
    
    gb.mJoyPad2.SetInputDevice(CHAN_1);

    Main_CheckJoypad(&gb.mJoyPad1);
    Main_CheckJoypadButtons(&gb.mJoyPad1);

    Main_CheckJoypad(&gb.mJoyPad2);
    Main_CheckJoypadButtons(&gb.mJoyPad2);
}

void GameCamera_UseRobotEffect(bool);

bool Bull_IsActive(void);

int Main_GetCameraDistanceTypeForLevel(void) {
    GameCamera_UseRobotEffect(false);

    int distanceType = 0;

    switch (gb.level.GetCurrentLevel()) {
        case LN_BULLS_PEN:
            if (Bull_IsActive()) {
                distanceType = 1;
            } else {
                distanceType = 0;
            }
            break;
        case LN_OUTBACK_SAFARI:
            distanceType = 2;
            break;
        case LN_FINAL_BATTLE:
            GameCamera_UseRobotEffect(true);
            break;
        default:
            distanceType = 0;
            break;
    }

    return distanceType;
}

GameCameraHeroInfo* Main_UpdateGlobalGameCameraHeroInfo(void) {

}

extern "C" void Sound_MusicStop(void);

void Main_ChangeLevel(LevelNumber newLevel) {
    Sound_MusicStop();

    gb.level.ChangeLevel(newLevel);

    if (!Main_DoVideo()) {
        gb.mLogicState.Set(STATE_9);
    }
}
