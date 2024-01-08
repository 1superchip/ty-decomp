#ifndef GLOBAL_H
#define GLOBAL_H

#include "common/Material.h"
#include "ty/GameData.h"
#include "ty/DataVal.h"
#include "common/DirectLight.h"
#include "ty/ImmediateFSM.h"

struct GameSubStateFSM {
    ImmediateFSM<GameSubStateFSM> fsm;

    static bool bInitialised;
};

struct JoyPad {
    char padding[0x78];
};

enum GameState {};

struct LogicState {
    int unk0;
};

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
    GameData mGameData;
    DataVal mDataVal;
    bool devLevelWarp;
    bool disableFrontend;
    bool debug;
    bool disableTriggers;
    bool disableLensFlare;
    int mNumChargeBits;
    bool whackyness;
    bool infinitePie;
    bool fastLoad;
    bool skipVideo;
    bool disableEnemies;
    bool disableFriends;
    bool unk2B2;
    bool unk2B3;
    bool enableBoss;
    int mRandSeed;
    LogicState mLogicState;
    int lastLogicState;
    GameState gameState;
    int unk2C8;
    GameSubStateFSM mGameFsm;
    int unk2DC;
    char padding_0x2E0[0x6F8 - 0x2E0];
    bool bOnPauseScreen;
    bool boot_controllerStatusTest;
    char padding_0x6FA[0x6];
    bool unk700;
    uint unk704;
    char dialogNameBuf[32];
    Vector tyPos;
    Vector glideJumpPos;
    float tyPosY;
    Material* pShadowMat;
    Material* pOverlayMat;
    uint logicGameCount;
    char padding[0xD94 - 0x758];
    DirectLight mDirectLight;
    bool autoLevelSwitch;
    bool unkE85;
    bool introOnly;
    int unkE88;
    int unkE8C;
    bool unkE94;
    bool fastDialogs;
};

extern GlobalVar gb;


#endif // GLOBAL_H
