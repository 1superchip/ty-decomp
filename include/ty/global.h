#ifndef GLOBAL_H
#define GLOBAL_H


#include "common/Input.h"
#include "common/Material.h"
#include "ty/GameData.h"
#include "ty/DataVal.h"
#include "common/DirectLight.h"
#include "common/Font.h"

enum GameSubState {
    GSS_NONE        = -1,
    GSS_InGame      = 0,
    GSS_BushPig     = 2,
    GSS_MiniGame_A2 = 12,
    GSS_MiniGame_B1 = 13,
    GSS_MiniGame_B3 = 14,
    GSS_MiniGame_C2 = 15,
    GSS_MiniGame_C3 = 16,
    GSS_MiniGame_D1 = 17, // no code but the name of this state exists
    GSS_MiniGame_E4 = 18,
};

// does this use ImmediateFSM<T>?
struct GameSubStateFSM {

    typedef void(GameSubStateFSM::*InitFunc)(void);
    typedef void(GameSubStateFSM::*DeinitFunc)(void);
    typedef void(GameSubStateFSM::*UpdateFunc)(void);
    typedef void(GameSubStateFSM::*DrawFunc)(void);
    typedef void(GameSubStateFSM::*StateFunc5)(void);

    struct State {
        InitFunc    Init; // Init
        DeinitFunc  Deinit; // Deinit?
        UpdateFunc  Update; // Update
        DrawFunc    Draw; // Draw
        StateFunc5  func5; // Unused
    };

    // figure out proper names
    GameSubState prevState;
    int newState;
    int mPrevPrevState;
    State* pStates;
    int unk10;

    static bool bInitialised;

    void Init(State* pFSMStates, GameSubState state) {
        pStates = pFSMStates;
        prevState = state;
        mPrevPrevState = -1;
        newState = -1;
        if (pStates[prevState].Init) {
            (this->*pStates[prevState].Init)();
        }
    }

    // This is CallStateDeinit?
    inline void CallStateDeinit(void) {
        if (prevState != GSS_NONE) {
            if (pStates[prevState].Deinit) {
                (this->*pStates[prevState].Deinit)();
            }
        }
        prevState = GSS_NONE;
    }

    void Init(GameSubState newState);
    void Deinit(void);

    void InGameUpdate(void);
    void InGameDraw(void);

    void BushPig_Update(void);
    void BushPig_Draw(void);

    void MiniGame_A2_Init(void);
    void MiniGame_A2_Deinit(void);
    void MiniGame_A2_Update(void);
    void MiniGame_A2_Draw(void);

    void MiniGame_B1_Init(void);
    void MiniGame_B1_Deinit(void);
    void MiniGame_B1_Update(void);
    void MiniGame_B1_Draw(void);

    void MiniGame_B3_Init(void);
    void MiniGame_B3_Deinit(void);
    void MiniGame_B3_Update(void);
    void MiniGame_B3_Draw(void);

    void MiniGame_C2_Init(void);
    void MiniGame_C2_Deinit(void);
    void MiniGame_C2_Update(void);
    void MiniGame_C2_Draw(void);

    void MiniGame_C3_Init(void);
    void MiniGame_C3_Deinit(void);
    void MiniGame_C3_Update(void);
    void MiniGame_C3_Draw(void);

    void MiniGame_E4_Init(void);
    void MiniGame_E4_Deinit(void);
    void MiniGame_E4_Update(void);
    void MiniGame_E4_Draw(void);
};

void VibrateJoystick(float, float, float, char, float);
void VibrateUpdate(void);
void VibrateReset(void);
void LoadManuallyScrollingTextures(void);
void ManuallyScrollTextures(void);
void FreeManuallyScrollingTexturePointers(void);

struct JoyPad {
    char padding0x0[0x74];
    InputDevices mInputDeviceID;
};

enum GameState {
    STATE_NONE = 0,
};

struct LogicState {
    int unk0;
    int unk4;
    GameState nextGameState;
    int unkC;

    void Init(GameState);
    void Set(GameState);
    void Update(void);
    bool DebugState(GameState);

    int GetUnk0(void) {
        return unk0;
    }
};

enum ElementType;

struct LevelInfo {
    char* levelId;
    ElementType elementType;
    ZoneNumber zone;
    TalismanType talismanType;
    bool bShowGameInfo;
    bool bShowGemCount;
};

struct LevelLayer {
    char name[0x20];
    Model* pModel;
    char padding24[0x38 - 0x24];
};

struct LevelData {
    LevelLayer layers[10];
    int nmbrOfLayers;
    int bEnvCubeLocked;
    Model* pEnvCube;
    Model* pEnvCubeWater;
    int collisionTilesAcross;
    int collisionTilesDown;
    char waterMaterial[0x20];
    float fog[4];
    int bDisplayMist;
    int bDrawLensFlare;
    float waterFog[4];
    float maxDepth;
    float fogPlane_0;
    float fogPlane_1;
    float fogPlane;
    float waterFogPlane;
    float waterFarPlane;
    float envRotate;
    float envYRot;
    Vector quarterLight;
    Vector lightDir;
    Vector halfLight;
    float alphaLightMapColor[4];
    Vector mLights[3];
    Vector mLightColors[3];
    Vector ambientLight;
    Vector worldMin;
    Vector worldMax;
    float lodRanges[8];
    int collisionHeapSize[24]; // Collision heap size array for each level
    LevelNumber unk400;
    int newLevelNumber;
    int nmbrOfLoadLevels;
    int levelNumber;
    int lastLevelIdx;
    bool bBossEnabled;

    void Init(void);
    void Deinit(void);
    ElementType GetElementType(void);
    ElementType GetElementType(LevelNumber levelNr);
    TalismanType GetTalismanType(LevelNumber levelNr);
    char* GetName(char* pName);
    char* GetName(LevelNumber levelNr);
    char* GetID(void);
    char* GetID(LevelNumber levelNr);
    LevelNumber GetLevelNumber(char* pName);
    LevelNumber GetZoneFirstLevelNumber(ZoneNumber);
    void EnableBoss(bool);
    bool IsBossEnabled(void);
    bool ShowGameInfo(LevelNumber levelNr);
    bool ShowGemCount(LevelNumber levelNr);

    int GetCurrentLevel(void) {
        return levelNumber;
    }
    void ChangeLevel(LevelNumber newLevel) {

    }
};

struct DialogPlayer;

struct GlobalVar {
    Material* pTensionMat;
    uint unk4;
    int unk8;
    JoyPad mJoyPad1;
    JoyPad mJoyPad2;
    bool bLevelInitialised;
    bool b3DSound;
    bool unkFE;
    int unk100;
    // Game savedata structure
    GameData mGameData;
    // Game config structure
    DataVal mDataVal;
    bool devLevelWarp;
    bool disableFrontend;
    bool debug;
    bool disableTriggers;
    bool disableLensFlare;
    int mNumChargeBites;
    bool whackyness;
    bool infinitePie;
    bool fastLoad;
    bool skipVideo;
    bool disableEnemies;
    bool disableFriends;
    bool unk2B2;
    bool unk2B3;
    bool enableBoss;
    // Global random number seed
    int mRandSeed;
    LogicState mLogicState;
    GameSubStateFSM mGameFsm;
    LevelData level;
    bool bOnPauseScreen;
    bool unk6F9;
    char padding6FA[6];
    bool unk700;
    bool bE3;
    DialogPlayer* pDialogPlayer;
    char dialogNameBuf[32];
    Vector tyPos; // ty jump pos?
    Vector glideJumpPos;
    float tyYPos;
    Material* pShadowMat;
    Material* pBlackSquareMat;
    uint logicGameCount;
    Material* pSmokeTex;
    Material* pZ1Mat_B;
    Material* pWaterfallMat;
    Material* pTyZ1Mat;
    Material* pTyZ1MatB;
    Material* pTyA1Mat;
    Material* pTyA1OverlayMat;
    Material* pWaterfallOverlay;
    Material* unk778;
    Material* unk77C;
    Material* unk780;
    Material* unk784;
    Material* unk788;
    float unk78C;
    Font* pGlobalFont;
    bool unk794;
    // Refpoint position of sun object in level model used for lensflare (unused)
    Vector sunOrigin;
    float sunMagnitude;
    bool unk7AC;
    bool unk7AD;
    bool unk7AE;
    char padding[0xD94 - 0x7AF];
    DirectLight mDirectLight;
    bool autoLevelSwitch;
    bool unkE85;
    bool introOnly;
    int unkE88; // levelWaitTimer?
    int unkE8C; // some level counter
    int unkE90;
    bool unkE94;
    bool fastDialogs;

    void ResetLight(void);
};

extern GlobalVar gb;

extern const char VersionNumber[];

extern const float FOV;
extern const float PaddleDepth;

inline bool TestColInfoFlag(CollisionResult* pCr, uint testFlags) {
    if (pCr->pInfo) {
        return pCr->pInfo->TestFlags(testFlags);
    }
    return false;
}

#define NUM_DIALOGPLAYER_ACTORS (68)

// Dialog Player Actor Info Structure
struct dpActorInfoStruct {
    char* pName;
    Model* pModel;
    int unk8;
    int unkC;
    int unk10;
};

#define DIALOG_ACTORINFO_MACRO(name, field3, field4) {name, NULL, NULL, field3, field4}

#endif // GLOBAL_H
