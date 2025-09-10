
#include "ty/global.h"
#include "common/Str.h"
#include "common/Model.h"
#include "ty/DDA.h"
#include "ty/GameObjectManager.h"
#include "ty/controlval.h"
#include "common/ParticleSystemManager.h"
#include "common/MKRumble.h"
#include "common/Translation.h"
#include "ty/heatflare.h"
#include "ty/LensFlare.h"
#include "ty/Mist.h"
#include "ty/props/WeatherProp.h"
#include "ty/tytypes.h"
#include "common/MKGrass.h"
#include "ty/Shadow.h"
#include "ty/Ty.h"
#include "ty/boomerang.h"
#include "ty/effects/Explosion.h"
#include "ty/RenderTexture.h"
#include "ty/Path.h"
#include "ty/ParticleEngine.h"

MKSceneManager gSceneManager;

int gLevelLoadStartTime = 0;

LevelInfo levelInfo[TOTAL_LEVEL_MAX] = {
    {"z1", ELEMENT_RAINBOW, ZN_0, TALISMAN_TYPE_0, false, true}, // Rainbow Cliffs
    {"z2", ELEMENT_RAINBOW, ZN_0, TALISMAN_TYPE_0, false, true}, 
    {"z3", ELEMENT_RAINBOW, ZN_0, TALISMAN_TYPE_0, false, false}, 
    {"z4", ELEMENT_RAINBOW, ZN_0, TALISMAN_TYPE_0, false, false}, 

    {"a1", ELEMENT_FIRE, ZN_1, TALISMAN_TYPE_0, true, true}, // Two Up
    {"a2", ELEMENT_FIRE, ZN_1, TALISMAN_TYPE_0, true, true}, // Walk in the Park
    {"a3", ELEMENT_FIRE, ZN_1, TALISMAN_TYPE_0, true, true}, // Ship Rex
    {"a4", ELEMENT_FIRE, ZN_1, TALISMAN_TYPE_0, false, false}, // Bull's Pen

    {"b1", ELEMENT_ICE, ZN_2, TALISMAN_TYPE_1, true, true}, // Bridge on the River Ty
    {"b2", ELEMENT_ICE, ZN_2, TALISMAN_TYPE_1, true, true}, // Snow Worries
    {"b3", ELEMENT_ICE, ZN_2, TALISMAN_TYPE_1, true, true}, // Outback Safari
    {"b4", ELEMENT_ICE, ZN_2, TALISMAN_TYPE_1, false, false}, //

    {"c1", ELEMENT_AIR, ZN_3, TALISMAN_TYPE_2, true, true}, // Lyre Lyre Pants on Fire
    {"c2", ELEMENT_AIR, ZN_3, TALISMAN_TYPE_2, true, true}, // Beyond the Black Stump
    {"c3", ELEMENT_AIR, ZN_3, TALISMAN_TYPE_2, true, true}, // Rex Marks The Spot
    {"c4", ELEMENT_AIR, ZN_3, TALISMAN_TYPE_2, false, false}, // Fluffy's Fjord

    {"d1", ELEMENT_AIR, ZN_5, TALISMAN_TYPE_3, false, false}, 
    {"d2", ELEMENT_AIR, ZN_5, TALISMAN_TYPE_3, false, false}, // Cass' Crest
    {"d3", ELEMENT_AIR, ZN_4, TALISMAN_TYPE_3, false, false}, 
    {"d4", ELEMENT_ICE, ZN_2, TALISMAN_TYPE_1, false, false}, // Crikey's Cove

    {"e1", ELEMENT_EARTH, ZN_5, TALISMAN_TYPE_3, false, false}, // Cass' Pass
    {"e2", ELEMENT_EARTH, ZN_5, TALISMAN_TYPE_4, false, false}, // Bonus World (Day)
    {"e3", ELEMENT_EARTH, ZN_5, TALISMAN_TYPE_4, false, false}, // Bonus World (Night)
    {"e4", ELEMENT_EARTH, ZN_5, TALISMAN_TYPE_4, false, false}, // Final Battle
};

void LevelData::InitDefaults(void) {
    lastLevelIdx = 0;
    nmbrOfLoadLevels = 0;

    mLights[0].Set(0.0f, 1.0f, 0.0f);
    mLightColors[0].Set(0.1f, 0.1f, 0.2f);

    mLights[1].Set(-0.2f, -1.0f, 0.7f);
    mLightColors[1].Set(0.65f, 0.65f, 0.65f);

    mLights[2].Set(-0.1f, -1.0f, -0.5f);
    mLightColors[2].Set(0.65f, 0.65f, 0.65f);
    
    ambientLight.Set(0.2f, 0.2f, 0.2f);
}

void LevelData::Init(void) {
    envRotate = 0.0f;
    envYRot = 0.0f;
    lightDir.x = 0.0f;
    lightDir.y = 0.0f;
    lightDir.z = 0.0f;
    fogPlane_0 = 0.0f;
    fogPlane_1 = 0.0f;
    farPlane = 25000.0f;
    lodRanges[0] = 4000000.0f;
    lodRanges[1] = 9000000.0f;
    lodRanges[2] = 1.6e+07f;
    lodRanges[3] = 3.6e+07f;
    lodRanges[4] = 8.1e+07f;
    lodRanges[5] = 1.96e+08f;
    lodRanges[6] = 4.0e+08f;
    lodRanges[7] = 9.0e+08f;
    nmbrOfLayers = 0;
    nmbrOfLoadLevels++;
}

void LevelData::Deinit(void) {
    if (pEnvCube) {
        pEnvCube->Destroy();
        pEnvCube = NULL;
    }

    if (pEnvCubeWater) {
        pEnvCubeWater->Destroy();
        pEnvCubeWater = NULL;
    }

    for (int i = 0; i < nmbrOfLayers; i++) {
        if (layers[i].pModel) {
            layers[i].pModel->Destroy();
        }

        layers[i].pModel = NULL;
    }

    nmbrOfLayers = 0;
}

ElementType LevelData::GetElementType(void) {
    return levelInfo[levelNumber].elementType;
}

ElementType LevelData::GetElementType(LevelNumber levelNr) {
    return levelInfo[levelNr].elementType;
}

TalismanType LevelData::GetTalismanType(LevelNumber levelNr) {
    return levelInfo[levelNr].talismanType;
}

char* LevelData::GetName(char* pName) {
    return GetName(GetLevelNumber(pName));
}

char* LevelData::GetName(LevelNumber levelNr) {
    return gpTranslation_StringArray[levelNr + 2];
}

char* LevelData::GetID(void) {
    return levelInfo[levelNumber].levelId;
}

char* LevelData::GetID(LevelNumber levelNr) {
    return levelInfo[levelNr].levelId;
}

extern "C" int stricmp(char*, char*);

LevelNumber LevelData::GetLevelNumber(char* pName) {
    for (int i = 0; i < TOTAL_LEVEL_MAX; i++) {
        if (stricmp(pName, levelInfo[i].levelId) == 0) {
            return (LevelNumber)i;
        }
    }

    return LN_RAINBOW_CLIFFS;
}

LevelNumber LevelData::GetZoneFirstLevelNumber(ZoneNumber zoneNum) {
    for (int i = 0; i < TOTAL_LEVEL_MAX; i++) {
        if (levelInfo[i].zone == zoneNum) {
            return (LevelNumber)i;
        }
    }

    return LN_3;
}

void Portal_HideAll(void);
void Portal_ShowAll(void);

void LevelData::EnableBoss(bool bEnable) {
    bBossEnabled = bEnable;
    if (bBossEnabled) {
        Portal_HideAll();
    } else {
        Portal_ShowAll();
    }
}

bool LevelData::IsBossEnabled(void) {
    return bBossEnabled;
}

ZoneNumber LevelData::GetZone(LevelNumber levelNr) {
    return levelInfo[levelNr].zone;
}

bool LevelData::ShowGameInfo(LevelNumber levelNr) {
    return levelInfo[levelNr].bShowGameInfo;
}

bool LevelData::ShowGemCount(LevelNumber levelNr) {
    return levelInfo[levelNr].bShowGemCount;
}

void BushPig_Deinit(void);
void GameCamera_Deinit(void);
void MiniGame_Deinit(void);
void Weather_DeinitType(void);
void BonusPickup_Deinit(void);
void PauseScreen_DeinitLevel(void);
void Fly_DeinitTyFlies(void);
void Critters_DeInit(void);
void Shears_Deinit(void);
void StaticSpikes_Deinit(void);
void SpikeyIce_Deinit(void);
void BarbedWire_Deinit(void);
void Bilby_Deinit(void);
void Spider_Deinit(void);
void Reeds_Deinit(void);
void IceBlock_Deinit(void);
void Windmill_Deinit(void);
void Waterfall_Deinit(void);
void Wake_Deinit(void);
void Dialog_Deinit(void);
void LetterBox_Deinit(void);
void WaterSlide_Deinit(void);
void Shatter_Deinit(void);
void Enemies_DeinitBoundingRegions(void);

struct FootEffect {
    char padding[0x10];
    void Init(void);
    void Deinit(void);
};
extern FootEffect gbFootEffects;

void DeinitializeLevel(void) {
    gb.mGameData.SynchroniseExitLevel();

    if (DDASession::bSessionStarted) {
        dda.EndSession();
    }

    MKRumble_Reset();
    Shadow_Deinit();
    Ty_Deinit();
    BushPig_Deinit();
    gb.mGameFsm.Deinit();
    MiniGame_Deinit();
    Weather_DeinitType();
    Gem_ParticleSystem_Deinit();
    Gem_DeleteList();
    BonusPickup_Deinit();
    PauseScreen_DeinitLevel();
    Fly_DeinitTyFlies();
    objectManager.DeinitLevel();
    HeatFlare_Deinit();
    Critters_DeInit();
    Shears_Deinit();
    StaticSpikes_Deinit();
    SpikeyIce_Deinit();
    BarbedWire_Deinit();
    Bilby_Deinit();
    Spider_Deinit();
    Reeds_Deinit();
    IceBlock_Deinit();
    Boomerang_Deinit();
    Windmill_Deinit();
    Tools_DropShadow_Deinit();
    MKGrass_Deinit();
    MKGrassGC_UnloadTextures();
    LensFlare_Deinit();
    Waterfall_Deinit();
    gbFootEffects.Deinit();
    RenderTexture_Deinit();
    Wake_Deinit();
    Dialog_Deinit();
    LetterBox_Deinit();
    Collision_Deinit();
    WaterSlide_Deinit();
    particleManager->Deinit();
    Shatter_Deinit();
    Mist_Deinit();
    Explosion_Deinit();
    Path_Manager_Deinit();
    Enemies_DeinitBoundingRegions();
    SoundBank_Deinit();
    gb.level.EnableBoss(false);
    FreeManuallyScrollingTexturePointers();
    gb.level.Deinit();
    GameCamera_Deinit();

    if (gb.unk4) {
        gb.unk4 = 0;
    }

    defaultParticleManager.RemoveAll();
    gb.mGameData.StopTime();
}

void Setup_PreloadLevel(void) {

    KromeIni ini;

    Vector worldMin = {0.0f, 0.0f, 0.0f, 0.0f};
    Vector worldMax = {0.0f, 0.0f, 0.0f, 0.0f};

    gb.level.collisionHeapSize[gb.level.GetCurrentLevel()] = 0x300000; // 3 MiB

    // read the collision heap size of the current level from collision.ini
    ini.Init("collision.ini");

    KromeIniLine* pLine = ini.GotoLine(NULL, NULL);

    while (pLine) {
        if (pLine->pFieldName) {
            if (stricmp(pLine->pFieldName, gb.level.GetID()) == 0) {
                pLine->AsInt(0, &gb.level.collisionHeapSize[gb.level.GetCurrentLevel()]);
                if (gb.level.collisionHeapSize[gb.level.GetCurrentLevel()] > 0x300000) {
                    // Max at 3 MiB
                    gb.level.collisionHeapSize[gb.level.GetCurrentLevel()] = 0x300000;
                }
                break;
            }
        } else if (pLine->comment == NULL || pLine->section != NULL) {
            break;
        }
        
        pLine = ini.GetNextLine();
    }
    
    ini.Deinit();

    // get the world minimum and maximum from the level's lv2 file
    ini.Init(Str_Printf("%s.lv2", gb.level.GetID()));
    ini.GotoLine("setup", NULL);

    pLine = ini.GetNextLine();
    pLine = ini.GetNextLine();

    while (pLine) {
        if (pLine->pFieldName) {
            if (stricmp(pLine->pFieldName, "worldMinMax") == 0) {
                pLine->AsFloat(0, &worldMin.x);
                pLine->AsFloat(1, &worldMin.y);
                pLine->AsFloat(2, &worldMin.z);
                pLine->AsFloat(3, &worldMax.x);
                pLine->AsFloat(4, &worldMax.y);
                pLine->AsFloat(5, &worldMax.z);
            }
        } else if (pLine->comment == NULL || pLine->section != NULL) {
            break;
        }
        pLine = ini.GetNextLine();
    }

    ini.Deinit();

    float minX = Min<float>(worldMin.x, worldMax.x);
    float minY = Min<float>(worldMin.y, worldMax.y);
    float minZ = Min<float>(worldMin.z, worldMax.z);
    float maxX = Max<float>(worldMin.x, worldMax.x);
    float maxY = Max<float>(worldMin.y, worldMax.y);
    float maxZ = Max<float>(worldMin.z, worldMax.z);

    float width = Max<float>(maxX - minX, Max<float>(maxY - minY, maxZ - minZ));

    gb.level.collisionTilesAcross = 128;
    gb.level.collisionTilesDown = 128;

    Collision_Init(
        gb.level.collisionHeapSize[gb.level.GetCurrentLevel()],
        minX, minY, minZ,
        width, width, 
        gb.level.collisionTilesAcross, gb.level.collisionTilesDown
    );

    Model* pGround = Model::Create(Str_Printf("room_%s_01", gb.level.GetID()), NULL);
    Collision_AddStaticModel(pGround, NULL, -1);
    pGround->Destroy();

    Model::Purge();
    Model::Purge();
    Model::Purge();
}

extern void Weather_Enable(bool);
extern void Shatter_Init(void);
extern void IceBlock_Init(void);
extern void Critters_Init(void);
extern void Fly_InitTyFlies(void);
extern void Dialog_Init(void);
extern void BushPig_Init(void);
extern void Level_Load(char*);
extern void Waterfall_Init(void);
extern void WaterSlide_Init(void);
extern void Enemies_InitBoundingRegions(void);
extern void MiniGame_Init(void);
extern int Main_GetCameraDistanceTypeForLevel(void);
extern void PauseScreen_InitLevel(void);
extern void EnableTechnoRangs(void);
extern void EnableElementalRangs(void);
extern void EnableChangeLevel(bool);

extern Vector* pCameraGrassPoint;
extern bool bEnableGallery;
extern bool bEnableMovies;

struct GameCameraHeroInfo;
extern GameCameraHeroInfo* Main_UpdateGlobalGameCameraHeroInfo(void);

extern void GameCamera_Reset(GameCameraHeroInfo*, int);

extern void Hud_Init(Ty*);

void Setup_LoadLevel(void) {
    gb.level.Init();

    gb.mGameData.SetMusicDucked(false);

    Shadow_Init();
    InitializeLevel();
    Weather_Enable(false);

    MKGrass_Init();
    MKGrassGC_LoadTextures(Setup_GetGrassForLevel());
    GrassGCMaxRadius = gb.level.GetCurrentLevel() == LN_LYRE_LYRE ? 5000.0f : 12000.0f;
    MKGrass_SetPushAwayPos(&ty.pos, 0);
    MKGrass_SetPushAwayPos(pCameraGrassPoint, 2);
    
    particleManager->Init();
    Shatter_Init();
    HeatFlare_Init();
    Tools_DropShadow_Init();
    IceBlock_Init();
    Critters_Init();
    Fly_InitTyFlies();
    Boomerang_Init();
    LensFlare_Init();
    Dialog_Init();

    gb.mGameData.SetCurrentZone(gb.level.GetZone(gb.level.GetCurrentLevel()));
    gb.mGameData.SetCurrentLevel(gb.level.GetCurrentLevel());

    PauseScreen_InitLevel();
    objectManager.InitLevel();

    static int modelCount = 0;

    if (modelCount != 0) {
        if (modelInstances.GetLength() != modelCount) {
            Model::List();
        }
    }
    
    modelCount = modelInstances.GetLength();

    Ty_Init();

    Level_Load(gb.level.GetID());

    if (System_GetCommandLineParameter("-LogMem") != NULL) {
        LogMaterialUsage();
        Model::List();
    }

    // Initiate Game FSM
    gb.mGameFsm.Init(GSS_InGame);

    switch (gb.level.GetCurrentLevel()) {
        case LN_OUTBACK_SAFARI:
            // if current level == 10, change the game fsm state to GSS_BushPig
            // and call BushPig_Init
            gb.mGameFsm.ChangeState(GSS_BushPig);
            BushPig_Init();
            break;
    }

    gb.mGameData.SynchroniseEnterLevel();

    ty.PostLoadInit();

    Gem_ParticleSystem_Init();

    GameCamera_Reset(
        Main_UpdateGlobalGameCameraHeroInfo(),
        Main_GetCameraDistanceTypeForLevel()
    );

    Hud_Init(&ty);
    gbFootEffects.Init();

    Waterfall_Init();
    WaterSlide_Init();
    RenderTexture_Init();
    WeatherProp_Init();
    Path_Manager_Init();
    Mist_Init();
    Enemies_InitBoundingRegions();
    MiniGame_Init();

    if (gb.mGameData.pSaveData->bCheatsEnabled) {
        gb.infinitePie = true;
        EnableTechnoRangs();
        EnableElementalRangs();
        EnableChangeLevel(true);
        bDrawCheatLines = true;
        bEnableGallery = true;
        bEnableMovies = true;
    }
}

void Setup_PostLoadLevel(void) {
    SoundBank_PlayExclusiveAmbientSound(true);

    pHero->Reset();

    // Send some init/start? message to all props
    MKMessage msg = {MSG_UNK_2};
    objectManager.SendMessageToAll(&msg, 0);

    LoadManuallyScrollingTextures();

    gb.unkE88 = 15;

    gb.mGameData.StartTime();
}

struct ScreenFaderObject {
    FaderObject mFader;

    void Init(Vector*, float, float);
};

struct FrontEndRes {
    char padding[0x404];
    ScreenFaderObject mFrontEndFader;
};

extern FrontEndRes gFERes;

void PreInitializeLevel(int r3) {
    gb.unk700 = true;
    gb.unk7AC = true;
    gb.level.unk400 = (LevelNumber)r3;
    gFERes.mFrontEndFader.Init(NULL, 640.0f, 512.0f);
    gFERes.mFrontEndFader.mFader.Fade(FaderObject::FADEMODE_5, 0.0f, 0.5f, 0.0f, false);
    gLevelLoadStartTime = Tools_GetTimeInSeconds();
}

void InitializeLevel(void) {
    gb.logicGameCount = 0;
    gb.unk700 = false;
    gb.bOnPauseScreen = false;
    gb.pDialogPlayer = NULL;
    gb.unk7AC = false;
    gb.unk7AE = false;
}

extern View* GameCamera_View();
extern "C" int sscanf(char*, char*, ...);

void InitializeGame(void) {
    gb.unk700 = false;
    
    gb.bLevelInitialised = false;
    gb.level.InitDefaults();
    gb.bLevelInitialised = true;

    gb.b3DSound = false;
    gb.unkFE = false;
    gb.mDataVal.Load();

    GameCamera_View()->SetProjection(FOV, GameCamera_View()->unk2C0, GameCamera_View()->unk2BC);

    GameData_New();
    gb.mGameData.Init();

    gb.mRandSeed = Tools_GetTimeInSeconds();

    char* pControlParam = System_GetCommandLineParameter("-controlConfig");
    if (pControlParam) {
        int configVal;
        char config = '\0';
        sscanf(pControlParam, "%s", &config);
        switch (config) {
            case '0':
            case 'A':
            case 'a':
                configVal = 0;
                break;
            case '1':
            case 'B':
            case 'b':
                configVal = 1;
                break;
            case '2':
            case 'C':
            case 'c':
                configVal = 2;
                break;
            case '3':
            case 'D':
            case 'd':
                configVal = 3;
                break;
            default:
                configVal = 0;
                break;
        }
        pGameSettings->controlConfig = configVal;
    }

    tyControl.Init(pGameSettings->controlConfig);
    if (System_GetCommandLineParameter("-tydev") != NULL) {
        gb.mGameData.SetLearntToSwim(true);
        gb.mGameData.SetLearntToDive(true);
        gb.mGameData.SetHasRang(BR_Flamerang, true);
        gb.mGameData.SetHasRang(BR_Frostyrang, true);
        gb.mGameData.SetBothRangs(true);
    } else {
        gb.mGameData.SetLearntToSwim(false);
        gb.mGameData.SetLearntToDive(false);
    }
}

void LogMaterialUsage(void) {
    Material** ppMaterials = Material::GetMaterialList();
    while (ppMaterials && *ppMaterials) {
        ppMaterials++;
    }
}

char* z1_grass[] = {"fx_099", "fx_000", NULL};

char* a1_grass[] = {"fx_000", "fx_085", NULL};
char* a2_grass[] = {"fx_102", "fx_134", "FX_103", NULL};
char* a3_grass[] = {"fx_000", "fx_097", NULL};
char* a4_grass[] = {"fx_099", NULL};

char* b1_grass[] = {"fx_000", "fx_085", NULL};
char* b2_grass[] = {"fx_099", NULL};
char* b3_grass[] = {"fx_140", NULL};

char* c1_grass[] = {"fx_134", "fx_085", "FX_103", "fx_102", NULL};

char* c2_grass[] = {"FX_120", "fx_099", "fx_136", NULL};
char* c3_grass[] = {"fx_000", "fx_097", NULL};
char* c4_grass[] = {NULL};

char* d1_grass[] = {"fx_000", NULL};
char* d2_grass[] = {NULL};
char* d4_grass[] = {"fx_000", NULL};

char* e1_grass[] = {"fx_134", "fx_085", NULL};
char* e2_grass[] = {"fx_099", "fx_000", "fx_102", "FX_103", NULL};
char* e3_grass[] = {"fx_099", "fx_102", "FX_103", "fx_000", NULL};
char* e4_grass[] = {NULL};

char* no_grass[] = {NULL};

char** Setup_GetGrassForLevel(void) {
    switch (gb.level.GetCurrentLevel()) {
        case LN_RAINBOW_CLIFFS:
            return z1_grass;
        case LN_TWO_UP:
            return a1_grass;
        case LN_WALK_IN_THE_PARK:
            return a2_grass;
        case LN_SHIP_REX:
            return a3_grass;
        case LN_BULLS_PEN:
            return a4_grass;
        case LN_BRIDGE_RIVER_TY:
            return b1_grass;
        case LN_SNOW_WORRIES:
            return b2_grass;
        case LN_OUTBACK_SAFARI:
            return b3_grass;
        case LN_LYRE_LYRE:
            return c1_grass;
        case LN_BLACK_STUMP:
            return c2_grass;
        case LN_REX_MARKS_SPOT:
            return c3_grass;
        case LN_FLUFFYS_FJORD:
            return c4_grass;
        case LN_16:
            return d1_grass;
        case LN_CASS_CREST:
            return d2_grass;
        case LN_CRIKEYS_COVE:
            return d4_grass;
        case LN_CASS_PASS:
            return e1_grass;
        case LN_BONUS_WORLD_DAY:
            return e2_grass;
        case LN_BONUS_WORLD_NIGHT:
            return e3_grass;
        case LN_FINAL_BATTLE:
            return e4_grass;
        default:
            return no_grass;
    }
}
