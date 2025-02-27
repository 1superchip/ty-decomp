
#include "ty/global.h"
#include "common/MKRumble.h"
#include "ty/tytypes.h"

const char VersionNumber[] = "190b";

const float FOV = PI / 3.0f;
const float PaddleDepth = 50.0f;

GlobalVar gb;

bool GameSubStateFSM::bInitialised = false;

dpActorInfoStruct actorInfo[NUM_DIALOGPLAYER_ACTORS] = {
    DIALOG_ACTORINFO_MACRO("", 0x0, 0),
    DIALOG_ACTORINFO_MACRO("Ty", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Maurie", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Shazza", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Rex", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Elle", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("FrillLizard", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Dennis", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("FatEmu", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Lenny", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Sheila", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("BunyipElder", 0x01000000, 3),
    DIALOG_ACTORINFO_MACRO("Julius", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Ken", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Farlapp", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Aurora", 0, 0),
    DIALOG_ACTORINFO_MACRO("Andy", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("RhinoRunner", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Bull", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("ShadowBat", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Fluffy", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Crikey", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("BossCass", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Neddy", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Sly", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Chockers", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("SynkerFrog", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("AuroraKid", 0, 0),
    DIALOG_ACTORINFO_MACRO("Bushpig", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Boonie", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Dubbo", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Elizabeth", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Gummy", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Katie", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Kiki", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Mim", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Snugs", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Shark", 0, 0),
    DIALOG_ACTORINFO_MACRO("BlueTongue", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("TreasureChest", 0, 0),
    DIALOG_ACTORINFO_MACRO("SnowAndy", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Leech", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Wallaby", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Bat", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("TrapdoorSpider", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Roach", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Bunyip", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Ringo", 0, 0),
    DIALOG_ACTORINFO_MACRO("RingoTentacle", 0, 0),
    DIALOG_ACTORINFO_MACRO("Speedboat", 0, 0),
    DIALOG_ACTORINFO_MACRO("SynkerFrillLizard", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("SynkerBlueTongue", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Skink", 0x01000000, 0),
    DIALOG_ACTORINFO_MACRO("Boomerang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Frostyrang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Flamerang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Kaboomarang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Doomarang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Megarang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Zoomerang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Infrarang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Zappyrang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Aquarang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Multirang", 0, 0),
    DIALOG_ACTORINFO_MACRO("Chronorang", 0, 0),
    DIALOG_ACTORINFO_MACRO("JuliusMachine", 0, 0),
    DIALOG_ACTORINFO_MACRO("ThundEggCollector", 0, 0),
    DIALOG_ACTORINFO_MACRO("Portal", 0, 3)
};

int actorInfoCount = NUM_DIALOGPLAYER_ACTORS;
float joystickDeadZone = 0.16f;

static GameSubStateFSM::State gameSubStates[20] = {
    {NULL, NULL, &GameSubStateFSM::InGameUpdate, &GameSubStateFSM::InGameDraw}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, &GameSubStateFSM::BushPig_Update, &GameSubStateFSM::BushPig_Draw}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL}, 
    {NULL, NULL, NULL, NULL}, 
    {
        // Minigame A2
        &GameSubStateFSM::MiniGame_A2_Init, 
        &GameSubStateFSM::MiniGame_A2_Deinit, 
        &GameSubStateFSM::MiniGame_A2_Update, 
        &GameSubStateFSM::MiniGame_A2_Draw,
    },
    {
        // Minigame B1
        &GameSubStateFSM::MiniGame_B1_Init, 
        &GameSubStateFSM::MiniGame_B1_Deinit, 
        &GameSubStateFSM::MiniGame_B1_Update, 
        &GameSubStateFSM::MiniGame_B1_Draw,
    },
    {
        // Minigame B3
        &GameSubStateFSM::MiniGame_B3_Init, 
        &GameSubStateFSM::MiniGame_B3_Deinit, 
        &GameSubStateFSM::MiniGame_B3_Update, 
        &GameSubStateFSM::MiniGame_B3_Draw,
    },
    {
        // Minigame C2
        &GameSubStateFSM::MiniGame_C2_Init, 
        &GameSubStateFSM::MiniGame_C2_Deinit, 
        &GameSubStateFSM::MiniGame_C2_Update, 
        &GameSubStateFSM::MiniGame_C2_Draw,
    },
    {
        // Minigame C3
        &GameSubStateFSM::MiniGame_C3_Init, 
        &GameSubStateFSM::MiniGame_C3_Deinit, 
        &GameSubStateFSM::MiniGame_C3_Update, 
        &GameSubStateFSM::MiniGame_C3_Draw,
    },
    // Minigame D1
    {NULL, NULL, NULL, NULL},
    {
        // Minigame E4
        &GameSubStateFSM::MiniGame_E4_Init, 
        &GameSubStateFSM::MiniGame_E4_Deinit, 
        &GameSubStateFSM::MiniGame_E4_Update, 
        &GameSubStateFSM::MiniGame_E4_Draw,
    },
    // 20
    // {}
};

void GameSubStateFSM::Init(GameSubState newState) {
    Init(gameSubStates, newState);
    bInitialised = true;
}

void GameSubStateFSM::Deinit(void) {
    if (bInitialised) {
        CallStateDeinit();
    }
    
    bInitialised = false;
}

void VibrateJoystick(float f1, float f2, float f3, char flags, float f4) {
    if (pGameSettings->unk6 && gb.pDialogPlayer == NULL) {
        MKRumble_Play(gb.mJoyPad1.mInputDeviceID, f1, f2, f3, flags, f4);
    }
}

void VibrateUpdate(void) {
    if (pGameSettings->unk6) {
        MKRumble_Update();
    }
}

void VibrateReset(void) {
    if (pGameSettings->unk6) {
        MKRumble_Reset();
    }
}

void GlobalVar::ResetLight(void) {
    for (int i = 0; i < 3; i++) {
        mDirectLight.SetLight(i, &level.mLights[i], &level.mLightColors[i]);
    }

    mDirectLight.SetAmbient(&level.ambientLight);
    
    if (View::GetCurrent()) {
        View::GetCurrent()->SetDirectLight(&mDirectLight);
    }
}

void LogicState::Init(GameState state) {
    gb.mLogicState.unk0 = 0;
    gb.mLogicState.unk4 = 0;
    gb.mLogicState.nextGameState = state;
    gb.mLogicState.unkC = 1;
}

void LogicState::Set(GameState newState) {
    if (newState != gb.mLogicState.unk0 || gb.mLogicState.nextGameState) {
        if (!DebugState(gb.mLogicState.nextGameState)) {
            DebugState((GameState)gb.mLogicState.unk0);
        }
        gb.mLogicState.nextGameState = newState;
    }
}

void LogicState::Update(void) {
    // set previous state?
    gb.mLogicState.unk4 = gb.mLogicState.unk0;
    // set new/current from the next
    gb.mLogicState.unk0 = gb.mLogicState.nextGameState;
    // set next state to NONE
    gb.mLogicState.nextGameState = STATE_NONE;
}

bool LogicState::DebugState(GameState state) {
    return state == (GameState)6 || state == (GameState)7 ||
        state == (GameState)10 || state == (GameState)11;
}

void LoadManuallyScrollingTextures(void) {
    gb.pWaterfallMat = Material::Find("waterfall_01");
    gb.pTyZ1Mat = Material::Find("ty_z1_001");
    gb.pTyZ1MatB = Material::Find("ty_z1_001b");
    gb.pTyA1Mat = Material::Find("TY_A1_022");
    gb.pTyA1OverlayMat = Material::Find("TY_A1_022_Overlay");
    gb.pWaterfallOverlay = Material::Find("Waterfall_Overlay_03");
    gb.unk778 = Material::Find("TY_A1_Env_004");
    gb.unk77C = Material::Find("TY_A1_Env_005");
    gb.unk780 = Material::Find("TY_A1_Env_006");
    gb.pSmokeTex = Material::Find("smoke");
    gb.unk784 = Material::Find("TY_A4_Env_002");
}

void ManuallyScrollTextures(void) {
    static int LastLogicGameCount = 0;
    if (!gb.bOnPauseScreen && LastLogicGameCount != gb.logicGameCount) {
        LastLogicGameCount = gb.logicGameCount;

        if (gb.pWaterfallMat) {
            gb.pWaterfallMat->ScrollUVOffset(0.0f, 1.0f / 60.0f);
        }

        if (gb.pTyZ1Mat) {
            gb.pTyZ1Mat->ScrollUVOffset(1.0f / 120.0f, 1.0f / 120.0f);
        }

        if (gb.pTyZ1MatB) {
            gb.pTyZ1MatB->ScrollUVOffset(0.0f, 1.0f / 2400.0f);
        }

        if (gb.pTyA1Mat) {
            gb.pTyA1Mat->ScrollUVOffset(0.0f, -(1.0f / 300.0f));
        }

        if (gb.pTyA1OverlayMat) {
            gb.pTyA1OverlayMat->SetMatrixX(0.5f);
            gb.pTyA1OverlayMat->ScrollUVOffset(0.0f, -0.0050000004f);
        }

        if (gb.pWaterfallOverlay) {
            gb.pWaterfallOverlay->SetMatrixX(0.5f);
            gb.pWaterfallOverlay->ScrollUVOffset(0.0f, 7.0f / 6000.0f);
        }

        if (gb.unk778) {
            gb.unk778->ScrollUVOffset(1.0f / 2400.0f, 1.0f / 6000.0f);
        }

        if (gb.unk77C) {
            gb.unk77C->ScrollUVOffset(3.0f / 8000.0f, 1.0f / 6000.0f);
        }

        if (gb.unk780) {
            gb.unk780->ScrollUVOffset(1.0f / 3000.0f, 1.0f / 6000.0f);
        }

        if (gb.pSmokeTex) {
            gb.pSmokeTex->SetMatrixX(0.0f);
            gb.pSmokeTex->ScrollUVOffset(0.0f, 1.0f / 1200.0f);
        }
        
        if (gb.unk784) {
            gb.unk784->ScrollUVOffset(0.0f, 1.0f / 2400.0f);
        }
    }
}

void FreeManuallyScrollingTexturePointers(void) {
    gb.pSmokeTex = NULL;
    gb.pZ1Mat_B = NULL;
    gb.pWaterfallMat = NULL;
    gb.pTyZ1Mat = NULL;
    gb.pTyZ1MatB = NULL;
    gb.pTyA1Mat = NULL;
    gb.pTyA1OverlayMat = NULL;
    gb.pWaterfallOverlay = NULL;
    gb.unk778 = NULL;
    gb.unk77C = NULL;
    gb.unk780 = NULL;
    gb.unk784 = NULL;
}
