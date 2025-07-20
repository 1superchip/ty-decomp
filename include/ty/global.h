#ifndef GLOBAL_H
#define GLOBAL_H

#include "common/Input.h"
#include "common/Material.h"
#include "ty/GameData.h"
#include "ty/DataVal.h"
#include "common/DirectLight.h"
#include "common/Font.h"
#include "ty/ExtendedAnalogControl.h"

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

    GameSubState mCurrState;
    int mNewState; // rename?
    int mPrevPrevState; // rename?
    State* pStates;
    int unk10; // rename?

    static bool bInitialised;

    void Init(State* pFSMStates, GameSubState state) {
        pStates = pFSMStates;
        mCurrState = state;
        mPrevPrevState = -1;
        mNewState = -1;
        if (pStates[mCurrState].Init) {
            (this->*pStates[mCurrState].Init)();
        }
    }

    // This is CallStateDeinit?
    inline void CallStateDeinit(void) {
        if (mCurrState != GSS_NONE) {
            if (pStates[mCurrState].Deinit) {
                (this->*pStates[mCurrState].Deinit)();
            }
        }
        
        mCurrState = GSS_NONE;
    }

    GameSubState GetState(void) {
        return mCurrState;
    }

    void ChangeStateInternal(int nextState, bool bAlwaysChange) {
        if (bAlwaysChange || (mCurrState != nextState)) {
            mNewState = nextState;
        }
    }
    
    void ChangeState(int nextState) {
        unk10 = GetState();
        ChangeStateInternal(nextState, false);
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
    ExtendedAnalogControl mAnalogControl;

    int mButtonsPressed;
    int mPrevButtonsPressed;

    float unk38;
    float unk3C;
    float unk40;
    float unk44;
    float unk48;
    float unk4C;
    float unk50;
    float unk54;
    float unk58;
    float unk5C;
    float unk60;
    float unk64;
    float unk68;

    int mPrevNewButtons;
    int mNewButtonsPressedThisFrame;

    InputDevices mInputDeviceID;

    // inlines to get if buttons have been pressed or not

    ExtendedAnalogControl* GetAnalogControl(void) {
        return &mAnalogControl;
    }

    InputDevices GetDeviceID(void) {
        return mInputDeviceID;
    }

    void DeinitButtons(void) {
        mButtonsPressed = -1;
        mPrevButtonsPressed = -1;
    }

    void SetNewFrame(void) {
        mPrevButtonsPressed = mButtonsPressed;
        mNewButtonsPressedThisFrame = 0;
    }

    float GetUnk38(void) {
        return unk38;
    }

    float GetUnk3C(void) {
        return unk3C;
    }

    float GetUnk48(void) {
        return unk48;
    }

    float GetUnk4C(void) {
        return unk4C;
    }

    float GetUnk58(void) {
        return unk58;
    }

    float GetUnk5C(void) {
        return unk5C;
    }

    float GetUnk60(void) {
        return unk60;
    }

    float GetUnk64(void) {
        return unk64;
    }

    void Reset_Unk58_Unk5C(void) {
        unk58 = 0.0f;
        unk5C = 0.0f;
    }

    void ResetButtonsPressed(void) {
        mButtonsPressed = 0;
    }
};

enum GameState {
    STATE_NONE  = 0,
    STATE_1     = 1,
    STATE_2     = 2,
    STATE_3     = 3,
    STATE_4     = 4,
    STATE_5     = 5,
    STATE_6     = 6,
    STATE_7     = 7,
    STATE_8     = 8,
    STATE_9     = 9,
    STATE_10    = 10,
    STATE_11    = 11,
};

struct LogicState {
    GameState currState;
    GameState prevState;
    GameState nextGameState; // Official name
    int unkC;

    void Init(GameState);
    void Set(GameState);
    void Update(void);
    bool DebugState(GameState);

    GameState GetCurr(void) {
        return currState;
    }

    GameState GetPrev(void) {
        return prevState;
    }

    GameState GetNext(void) {
        return nextGameState;
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
    float farPlane;
    float waterFogPlane;
    float waterFarPlane;
    float envRotate;
    float envYRot;
    Vector quarterLight;
    Vector lightDir;
    Vector halfLight;
    Vector alphaLightMapColor;
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
    LevelNumber levelNumber;
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
    ZoneNumber GetZone(LevelNumber levelNr);
    bool ShowGameInfo(LevelNumber levelNr);
    bool ShowGemCount(LevelNumber levelNr);

    LevelNumber GetCurrentLevel(void) {
        return levelNumber;
    }

    void ChangeLevel(LevelNumber newLevel) {

    }
    
    void InitDefaults(void);
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
    uint logicGameCount; // Does not increment while paused. Reset when entering a level
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

    // Boolean setting for BushPig movement function
    // If true, uses BushPig::MoveBull2
    // Else, uses BushPig::MoveBull
    bool bUseBushPigMovement2;

    // Refpoint position of sun object in level model used for lensflare (unused)
    Vector sunOrigin;
    float sunMagnitude;
    bool unk7AC;
    bool unk7AD;
    bool unk7AE;

    Material* unk7B0; // pUnk860
    int unk7B4;

    Material* unk7B8; // pUnk868
    int unk7BC;

    View mView;
    // seems to contain unused fields such as an ObjectiveFinder struct
    char padding[0xD94 - (0x7C0 + sizeof(View))];
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

void DeinitializeLevel(void);
void Setup_PreloadLevel(void);
void Setup_LoadLevel(void);
void Setup_PostLoadLevel(void);
void PreInitializeLevel(void);
void InitializeLevel(void);
void InitializeGame(void);
void LogMaterialUsage(void);
char** Setup_GetGrassForLevel(void);

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

extern dpActorInfoStruct actorInfo[];

#define ACTOR_BUNYIP_ID (46)

#endif // GLOBAL_H
