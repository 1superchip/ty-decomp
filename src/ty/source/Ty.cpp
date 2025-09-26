#include "ty/Ty.h"
#include "ty/global.h"
#include "ty/ParticleEngine.h"
#include "ty/bunyip.h"
#include "ty/main.h"
#include "ty/props/WaterVolume.h"
#include "common/Str.h"

static GameObjDesc tyDesc;
Ty ty;

static ModuleInfo<Ty> tyModuleInfo;
Hero* pHero;
static bool bInitialised = false;
static bool bResourcesLoaded = false;

static TyMediumMachine::State mediumFSMStates[] = {
    {
        // MD_None
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }, 
    {
        // TY_MEDIUM_1
        &Ty::LandMediumInit,
        &Ty::LandMediumDeinit,
        &Ty::LandMediumUpdate
    },
    {
        // TY_MEDIUM_2
        &Ty::WaterMediumInit,
        &Ty::WaterMediumDeinit,
        &Ty::WaterMediumUpdate
    },
    {
        // TY_MEDIUM_3
        &Ty::UnderWaterMediumInit,
        &Ty::UnderWaterMediumDeinit,
        &Ty::UnderWaterMediumUpdate
    },
    {
        // TY_MEDIUM_4
        &Ty::AirMediumInit,
        &Ty::AirMediumDeinit,
        &Ty::AirMediumUpdate
    }
};

static TyFSM::State tyStates[] = {
    {},
    {

    },
};

void TyFSM::Update(Ty* pTy) {
    if (unk14 != AS_None) {
        if (unk10 != AS_None) {
            if (pStates[unk10].Deinit) {
                (pTy->*pStates[unk10].Deinit)();
            }
        }

        unk4 = unk0;
        unk0 = unk10;
        unk10 = unk14;
        unk14 = AS_None;

        if (pStates[unk10].medium != MD_None) {
            pTy->SetMedium(pStates[unk10].medium);
        }

        unk8 = gb.logicGameCount;
        unkC = 0;

        if (unk10 != AS_None) {
            if (pStates[unk10].Init) {
                (pTy->*pStates[unk10].Init)();
            }
        }
    }

    unkC++;

    if (unk10 != AS_None) {
        if (pStates[unk10].Active) {
            (pTy->*pStates[unk10].Active)();
        }
    }
}

bool TyFSM::SolidSurfaceState(int state) {
    if (
        state == TY_AS_3 || 
        state == TY_AS_5 ||
        state == TY_AS_35 ||
        state == TY_AS_32 ||
        state == TY_AS_34 ||
        state == TY_AS_28 ||
        state == AS_FindItem ||
        state == TY_AS_44 ||
        state == TY_AS_36 ||
        state == TY_AS_46 ||
        state == AS_Sneak ||
        state == TY_AS_4 ||
        state == TY_AS_40 ||
        state == TY_AS_41
    ) {
        return true;
    }
    
    return false;
}

bool TyFSM::BiteState(int state) {
    if (state == AS_Bite) {
        return true;
    }

    return false;
}

bool TyFSM::SwimmingState(int state) {
    if (
        state == TY_AS_14 ||
        state == TY_AS_15 ||
        state == TY_AS_38 ||
        state == TY_AS_16 ||
        state == TY_AS_17 ||
        state == TY_AS_39 ||
        state == TY_AS_18 ||
        state == TY_AS_49 ||
        state == TY_AS_19 ||
        state == TY_AS_20 ||
        state == TY_AS_29 ||
        state == TY_AS_22 ||
        state == TY_AS_24
    ) {
        return true;
    }

    return false;
}

bool TyFSM::IdleState(int state) {
    if (
        state == TY_AS_34 ||
        state == TY_AS_35 ||
        state == TY_AS_36 ||
        state == TY_AS_38 ||
        state == TY_AS_39 ||
        state == TY_AS_37 ||
        state == TY_AS_40
    ) {
        return true;
    }

    return false;
}

bool TyFSM::KnockBackState(int state) {
    if (state == TY_AS_32 || state == TY_AS_20 || state == TY_AS_20 || state == TY_AS_21) {
        return true;
    }

    return false;
}

bool TyFSM::MoveState(int state) {
    if (state == TY_AS_5 || state == TY_AS_3 || state == AS_Sneak || state == TY_AS_4) {
        return true;
    }

    return false;
}

bool TyFSM::SneakState(int state) {
    return state == AS_Sneak;
}

bool TyFSM::WaterSurfaceState(int state) {
    return state == TY_AS_14 || state == TY_AS_37 || state == TY_AS_15 || state == TY_AS_38 || state == TY_AS_21;
}

bool TyFSM::FirstPersonState(int state) {
    return state == AS_FirstPerson;
}

bool TyFSM::AirState(int state) {
    if (
        state == TY_AS_26 ||
        state == TY_AS_7 ||
        state == TY_AS_8 ||
        state == TY_AS_9 ||
        state == TY_AS_27
    ) {
        return true;
    }

    return false;
}

void Hud_BoomerangScroll(int, int);

void RangChangeData::PerformChange(void) {
    if (boomerangButton == 0) {
        return;
    }

    BoomerangType foundType = ty.mBoomerangManager.GetCurrentType();

    while (Abs<int>(boomerangButton) != 0) {
        foundType = boomerangButton > 0 ? GetNextRang(foundType) : GetPrevRang(foundType);

        if (foundType == BR_Aquarang) {
            // Disallow switching to the aquarang as it is only meant to be used underwater
            // and is forced to be used underwater
            foundType = boomerangButton > 0 ? GetNextRang(foundType) : GetPrevRang(foundType);
        }

        boomerangButton = boomerangButton + (boomerangButton > 0 ? -1 : 1);
    }

    if (ty.mBoomerangManager.GetCurrentType() != foundType) {
        ty.mBoomerangManager.SetType(foundType);
        
        Hud_BoomerangScroll(foundType, 0);
    }
}

void BoomerangAnimInfo::Init(BoomerangSide side) {
    unk8.Init(&ty.rangPropAnimScripts[side]);

    pModel = Model::Create(
        ty.rangPropAnimScripts[side].GetMeshName(),
        ty.rangPropAnimScripts[side].GetAnimName()
    );

    pModel->b5 = true;
}

void BoomerangAnimInfo::Deinit(void) {
    pModel->Destroy();
    unk8.Deinit();
}

void Ty_LoadResources(void) {
    tyDesc.Init(
        &tyModuleInfo,
        "Ty",
        "Ty",
        0,
        1
    );

    if (!bResourcesLoaded) {
        bResourcesLoaded = true;
    }
}

void Ty_Init(void) {
    if (!bInitialised) {
        pHero = &ty;

        ty.Init();

        bInitialised = true;
    }
}

void Ty_Deinit(void) {
    if (bInitialised) {
        ty.Deinit();

        bInitialised = false;
    }
}

void Ty_Update(void) {
    if (bInitialised) {
        ty.Update();
    }
}

void Ty_Draw(void) {
    if (bInitialised) {
        ty.Draw();
    }
}

void Ty::StartBlendAnimation(MKAnim* pAnim, bool r5) {
    unk530 = 1.0f;

    if (r5) {
        animScript.SetAnimKeepingPosition(pAnim);
    } else {
        animScript.SetAnim(pAnim);
    }
}

void Ty::StartAnimation(MKAnimScript* pAnimScript, MKAnim* pAnim, int r6, bool r7) {
    unk530 = 0.0f;
    
    if (pAnim) {
        if (r6 > 0) {
            pAnimScript->TweenAnim(pAnim, r6);
        } else {
            pAnimScript->SetAnim(pAnim);
        }
    }

    unkF98 = false;
}

void Ty::StartAnimIfNew(MKAnimScript* pAnimScript, MKAnim* pAnim, int r6, bool r7) {
    bool a = pAnimScript->currAnim == pAnim;
    bool b = pAnimScript->unk1C > 0 && pAnimScript->nextAnim == pAnim;

    if (!a && !b) {
        ty.StartAnimation(pAnimScript, pAnim, r6, r7);
    }
}

float Ty::GetSpeedFromJoyPad(float f1) {
    return Min<float>(ApproxMag(gb.mJoyPad1.GetUnk58(), gb.mJoyPad1.GetUnk5C()), 1.0f) * f1;
}

void Ty::UpdateHorzVel(float smoothing) {
    float xFactor = 0.0f;
    float zFactor = 0.0f;

    if (!IsJoyPadZero()) {
        xFactor = mRot.GetUnkC() * magnitude;
        zFactor = mRot.GetUnk10() * magnitude;
    }
    
    velocity.x = AdjustFloat(velocity.x, xFactor, 1.0f - smoothing);
    velocity.z = AdjustFloat(velocity.z, zFactor, 1.0f - smoothing);
}

extern "C" double atan2(double, double);

float GameCamera_GetMoveYaw(void);

void Ty::UpdateYaw(float scale) {
    if (!IsJoyPadZero()) {
        float moveYaw = GameCamera_GetMoveYaw();

        unk1414 = atan2(-gb.mJoyPad1.GetUnk58(), gb.mJoyPad1.GetUnk5C());
        unk1414 += moveYaw;
        
        unk1414 = NormaliseAngle(unk1414);

        float fVar3 = NormaliseAngle(-unk1414 - mRot.GetUnk4());

        if (fVar3 > PI) {
            fVar3 -= 2 * PI;
        }
        
        mRot.UnknownInline(ValidPIRange(mRot.GetUnk4() + fVar3 * scale));
    }
}

float Ty::GetYawFromJoy(void) {
    if (IsJoyPadZero()) {
        return ty.mRot.unk4;
    } else {
        float moveYaw = GameCamera_GetMoveYaw();
        return -NormaliseAngle(moveYaw + (float)atan2(-gb.mJoyPad1.GetUnk58(), gb.mJoyPad1.GetUnk5C()));
    }
}

struct PlayerFileNameStruct {
    char* unk0;
    char* unk4;
    char* unk8;
    char* unkC;
};

// char* playerFileNames[] = {
//     "act_01_ty",
//     "act_01_tyshadow",
//     "act_01_ty_01",
//     "act_01_ty_02"
// };

PlayerFileNameStruct playerFileNames[] = {
    {"act_01_ty", "act_01_tyshadow", "act_01_ty_01", "act_01_ty_02"}
};

void Ty::LoadResources(void) {
    rangPropAnimScripts[0].Init("prop_0137_rangpropleft");
    rangPropAnimScripts[1].Init("prop_0137_rangpropright");
    
    unk534.Init(playerFileNames[gb.unk100].unk0);

    animScript.Init(&unk534);
    unk4EC.Init(&unk534);
    unk50C.Init(&unk534);

    unk52C = false;

    if (gb.level.GetCurrentLevel() == LN_OUTBACK_SAFARI) {
        pModel = Model::Create(unk534.GetMeshName(), NULL);
        GetNodesAndSubObjects();
    } else {
        pModel = Model::Create(unk534.GetMeshName(), unk534.GetAnimName());

        pReflectionModel = Model::Create(playerFileNames[gb.unk100].unk4, NULL);
        pReflectionModel->pAnimation = pModel->GetAnimation();

        mShadow.Init(
            playerFileNames[gb.unk100].unk4,
            pModel,
            250.0f,
            NULL
        );

        pMatEyes = Material::Find(playerFileNames[gb.unk100].unkC);
        pMatEyeballs = Material::Find("Act_01_TY_01a");
        GetNodesAndSubObjects();
        LoadAnimations();

        StartAnimation(&animScript, biteUnchargeAnim, 0, false);
        StartAnimation(&unk4EC, biteUnchargeAnim, 0, true);

        unk11F8[0].Init(BOOMERANG_SIDE_LEFT);
        unk11F8[1].Init(BOOMERANG_SIDE_RIGHT);

        breathMist = 0;
    }

    actorInfo[1].pModel = pModel;
}

void Ty::FreeResources(void) {
    
    if (pReflectionModel) {
        pReflectionModel->pAnimation = NULL;
        pReflectionModel->Destroy();

        pReflectionModel = NULL;

        mShadow.Deinit();

        unk11F8[0].Deinit();
        unk11F8[1].Deinit();
    }

    animScript.Deinit();
    unk4EC.Deinit();
    unk50C.Deinit();

    unk52C = false;
    
    rangPropAnimScripts[0].Deinit();
    rangPropAnimScripts[1].Deinit();

    unk534.Deinit();
}

void Particle_FrostyBreath_Init(ParticleSystem**, Vector*, BoundingVolume*);
void Particle_Fire_Init(ParticleSystem**, Vector*, BoundingVolume*, float, bool);
void Particle_Ice_Init(ParticleSystem**, Vector*);

void Ty::Init(void) {
    Ty::Init(&tyDesc);

    pLastCheckPoint = NULL;

    LoadResources();

    if (gb.level.GetCurrentLevel() != LN_OUTBACK_SAFARI) {
        tyHealth.Init(HEALTH_TYPE_0);

        pLocalToWorld = &pModel->matrices[0];
        pGameObject = NULL;

        mMediumMachine.Init(mediumFSMStates, MD_None);

        mContext.Init(gb.mDataVal.jumpGravity, gb.mDataVal.jumpMaxGravity);

        mRangTrails[0].Init(16);
        mRangTrails[1].Init(16);

        mIceTrails[0].Init(16);
        mIceTrails[1].Init(16);

        mTyRainbowEffect.Init(&ty.pos);

        EnableHead(TYH_Normal);

        mWaterSlide.Init();
        tySounds.Init();
        tyBite.Init();

        InitEvents();

        StartAnimation(&animScript, biteUnchargeAnim, 0, false);

        mFsm.SetState(TY_AS_35, false);

        opalMagnetData.Init();
        glowParticleData.Init();

        switch (breathMist) {
            case 1:
                Particle_FrostyBreath_Init(&unkDC0[breathMist], pModel->matrices[0].Row3(), pModel->GetModelVolume());
                break;
        }

        Particle_Fire_Init(&pSystems[0], pModel->matrices[0].Row3(), pModel->GetModelVolume(), 2.0f, true);
        Particle_Fire_Init(&pSystems[1], pModel->matrices[0].Row3(), pModel->GetModelVolume(), 2.0f, false);
        Particle_Ice_Init(&pSystems[2], pModel->matrices[0].Row3());

        BoundingVolume volume = *(pModel->GetModelVolume());

        volume.v2.y += 500.0f;

        mBubble.Init(pModel->matrices[0].Row3(), &volume, 1.0f);

        mDustTrail.Init(&pos, gb.mDataVal.swimSpeedFast, 35.0f, 0.6f);

        if (gb.bE3) {
            gb.mGameData.SetLearntToSwim(true);
            gb.mGameData.SetLearntToDive(true);
            gb.mGameData.SetHasRang(BR_Aquarang, true);
        }
    }
}

void Ty::InitEvents(void) {
    MKAnimScript* pScript = &animScript;

    unk7B0 = pScript->GetEventByName("leftThrowBoomerang");
    unk7B4 = pScript->GetEventByName("rightThrowBoomerang");

    unk7B8 = pScript->GetEventByName("leftCatch");
    unk7BC = pScript->GetEventByName("rightCatch");

    unk7C0 = pScript->GetEventByName("lefthand");
    unk7C4 = pScript->GetEventByName("righthand");

    unk7C8 = pScript->GetEventByName("blink");

    unk7CC = pScript->GetEventByName("RangTyRightOff");
    unk7D0 = pScript->GetEventByName("RangTyRightOn");

    unk7D4 = pScript->GetEventByName("RangTyLeftOff");
    unk7D8 = pScript->GetEventByName("RangTyLeftOn");

    unk7DC = pScript->GetEventByName("RangTyRightSpecialIdle2Off");
    unk7E0 = pScript->GetEventByName("RangTyRightSpecialIdle2On");

    unk7E4 = pScript->GetEventByName("BiteHeadOn");
    unk7E8 = pScript->GetEventByName("BiteHeadOff");

    unk7EC = pScript->GetEventByName("leftToe");
    unk7F0 = pScript->GetEventByName("rightToe");

    unk7F4 = pScript->GetEventByName("leftFoot");
    unk7F8 = pScript->GetEventByName("rightFoot");

    unk800 = animScript.GetEventByName("TrailOff");
    unk7FC = animScript.GetEventByName("TrailOn");

    unk804 = animScript.GetEventByName("FliesOn");
    
    unk808 = animScript.GetEventByName("rangTwirlOn");
    unk80C = animScript.GetEventByName("rangTwirlOff");

    unk810 = animScript.GetEventByName("RangChange");

    unk814 = animScript.GetEventByName("KnockDown");

    unk818 = animScript.GetEventByName("TyLand");

    unk81C = animScript.GetEventByName("TyJump");

    unk820 = animScript.GetEventByName("ShowItem");
    unk824 = animScript.GetEventByName("HideItem");
}

void Ty::PostLoadInit(void) {
    BoomerangManagerInit bmInit = {
        Boomerang_pDescriptors,
        BR_Max,
        gb.mGameData.GetBoomerang(),
        true,
        &unk534,
        unk488,
        {unk3C0, unk3D4},
        {
            {"leftThrowBoomerang"},
            {"rightThrowBoomerang"},
            {"leftCatch"},
            {"rightCatch"}
        },
        {
            true,
            unk6EC,
            {unk6C4, unk6D8},
            {unk6CC, unk6E0}
        }
    };

    mBoomerangManager.Init(&bmInit);
}

void Ty::Deinit(void) {
    if (pHero->IsTy()) {
        mFsm.DeinitState(this);
        mMediumMachine.CallDeinit(this);

        ResetPitchAndRoll();
        mTyRainbowEffect.Deinit();
        tyBite.Deinit();

        mRangTrails[0].Deinit();
        mRangTrails[1].Deinit();

        mIceTrails[0].Deinit();
        mIceTrails[1].Deinit();

        mBubble.Deinit();

        mWaterSlide.Deinit();

        for (int i = 0; i < ARRAY_SIZE(pSystems); i++) {
            if (pSystems[i]) {
                pSystems[i]->Destroy();
            }

            pSystems[i] = NULL;
        }

        if (breathMist != 0) {
            if (unkDC0[breathMist]) {
                unkDC0[breathMist]->Destroy();
            }

            unkDC0[breathMist] = NULL;
        }

        breathMist = 0;

        pGameObject = NULL;

        mDustTrail.Deinit();

        tyHealth.Deinit();

        opalMagnetData.Deinit();
        glowParticleData.Deinit();
    }

    mBoomerangManager.Deinit();
    FreeResources();
    GameObject::Deinit();

    mHeadTurningInfo.mNodeOverride.pAnimation = NULL;
}

void Ty::Reset(void) {
    if (pHero->IsTy()) {
        ResetVars();
        SetAbsolutePosition(&pos, 50, 1.0f, true);
        lastSafePos = pos;

        mTyRainbowEffect.Reset();

        tyHealth.SetType(HEALTH_TYPE_0);
        tySounds.Reset();
        opalMagnetData.Reset();
        glowParticleData.Reset();
    }
}

void Ty::ResetVars(void) {
    gb.ResetLight();

    tyBite.Reset();

    mBoomerangManager.Reset();

    mAutoTarget.Reset();

    EnableHead(TYH_Normal);
}

void Ty::Update(void) {
    if (gb.pDialogPlayer) {
        if (GetMedium() == TY_MEDIUM_3) {
            ResetDrownTimer();
        }
    } else {
        // CommonPreLogicChecks();
        mFsm.Update(this);
        mMediumMachine.Update(this, false);
        // CommonPostLogicChecks();
        // UpdateLastSafePos();
        UpdateBoomerangs();
        // UpdateRangTrails();
        // glowParticleData.Draw();
        ty.mAutoTarget.Reset();
        // UseSpecialParticleEffect();
    }
}

void Ty::Draw(void) {
    mFsm.Draw(this);
}

void Ty::StartDeath(HurtType hurtType, bool r5) {
    gb.unk7AC = true;
    particleManager->StopExclamation(true);

    dda.StoreDeathInfo();

    if (InWater()) {
        mFsm.Set(TY_AS_29);
    } else {
        mFsm.Set(TY_AS_28);
    }
}

void Ty::Hurt(HurtType hurtType, DDADamageCause damageCause, bool, Vector* pVec, float f1) {
    static int flinch = 0;
    
    if (gb.disableTriggers || gb.infinitePie) {
        return;
    }

    if (invicibilityFrames > 0 || mFsm.GetStateEx() == TY_AS_28 || mFsm.GetStateEx() == TY_AS_29) {
        return;
    }

    EnableHead(TYH_Normal);

    if (hurtType == HURT_TYPE_1 || hurtType == HURT_TYPE_5) {
        VibrateJoystick(0.5f, 1.0f, (f1 / 45.0f) + 0.2f, 0, 4.0f);
    }

    dda.StoreDamageInfo(damageCause);

    if (ty.mMediumMachine.GetUnk0() == TY_MEDIUM_3 && mFsm.GetStateEx() != TY_AS_25) {
        for (int i = 0; i < 30; i++) {
            Vector tmp = {
                RandomFR(&gb.mRandSeed, -10.0f, 10.0f),
                RandomFR(&gb.mRandSeed, -10.0f, 10.0f),
                RandomFR(&gb.mRandSeed, -10.0f, 10.0f)
            };

            Vector spawnPos;

            spawnPos.Add(&unk324, &tmp);

            mBubble.Create(
                &spawnPos, 
                RandomFR(&gb.mRandSeed, 2.0f, 4.0f), 
                mContext.water.pos.y,
                4.5f,
                0.0f
            );
        }

        SoundBank_Play(SFX_TyWaterBubble, NULL, ID_NONE);
    }

    if (tyHealth.Hurt(hurtType)) {
        switch (hurtType) {
            case HURT_TYPE_0:
            case HURT_TYPE_3:
            case HURT_TYPE_6:
                flinch = (flinch + 1) % ARRAY_SIZE(flinchAnims);
                if (unk4EC.Condition()) {
                    StartAnimation(&unk4EC, flinchAnims[flinch], 0, false);
                }
                break;
            case HURT_TYPE_2:
                SetKnockBackFromPos(&pos, f1, KB_TYPE_2);
                break;
            case HURT_TYPE_1:
            case HURT_TYPE_4:
                SetKnockBackFromPos(pVec, f1, KB_TYPE_0);
                break;
            case HURT_TYPE_5:
                SetKnockBackFromDir(pVec, f1, KB_TYPE_0);
                break;
        }

        invicibilityFrames = 240; // 240 frames
    } else {
        StartDeath(hurtType, false);
    }
}

void Ty::SetToIdle(bool bResetVel, TyMedium newMedium) {
    if (
        mFsm.GetStateEx() == AS_FindItem || 
        mFsm.GetStateEx() == TY_AS_28 || 
        mFsm.GetStateEx() == TY_AS_29 ||
        mFsm.unk14 == AS_FindItem || 
        mFsm.unk14 == TY_AS_28 || 
        mFsm.unk14 == TY_AS_29
    ) {
        return;
    }

    TyMedium nextMedium = newMedium;
    if (nextMedium == MD_None) {
        nextMedium = GetMedium();
    }

    switch (nextMedium) {
        case TY_MEDIUM_1:
            mFsm.SetState(TY_AS_35, true);
            ty.mContext.water.bValid = false;
            break;
        case TY_MEDIUM_2:
            mFsm.SetState(TY_AS_38, true);
            break;
        case TY_MEDIUM_3:
            mFsm.SetState(TY_AS_39, true);
            break;
    }

    if (bResetVel) {
        velocity.SetZero();
        lastVelocity.SetZero();
        directVelocity.SetZero();
    }
}

bool Ty::IsClaiming(void) {
    return mFsm.GetState() == AS_FindItem;
}

void Ty::SetFindItem(Vector* pPos, SpecialPickupStruct* pPickup) {
    if (mFsm.GetStateEx() == AS_FindItem || mFsm.GetStateEx() == TY_AS_28 || mFsm.GetStateEx() == TY_AS_29) {
        pPickup->ScaleOut();
    } else {
        if (pBunyip) {
            pBunyip->SetState(BUNYIP_IDLE);
        }

        pSpecialPickup = pPickup;

        ty.SetAbsolutePosition(pPos, 20, 1.0f, true);

        if (mContext.water.bValid) {
            if (mContext.GetYDistanceToWater(&pos) > 85.0f) {
                // No check for if the player has unlocked the ability to swim
                SetMedium(TY_MEDIUM_3);
            }
        }

        mFsm.SetState(AS_FindItem, false);
    }
}

void GameCamera_SnapBehindHero(bool, bool);

/// @brief 
/// @param pPos 
/// @param pNewRot 
/// @return 
bool Ty::StableReposition(Vector* pPos, Vector* pNewRot) {
    if (SetAbsolutePosition(pPos, 20, 20.0f, true)) {
        if (pNewRot) {
            mRot.SetRotByVec(pNewRot);
        }

        TyMedium nextMedium = mContext.floor.bOn ? TY_MEDIUM_1 : TY_MEDIUM_4;

        if (mContext.water.bValid) {
            if (mContext.GetYDistanceToWater(&pos) > PaddleDepth) {
                nextMedium = TY_MEDIUM_2;
            }

            if (mContext.GetYDistanceToWater(&pos) > 85.0f) {
                nextMedium = TY_MEDIUM_3;
            }
        }

        SetToIdle(true, nextMedium);

        GameCamera_SnapBehindHero(true, false);

        return true;
    } else {
        return false;
    }
}

void Ty::SetBunyip(Bunyip* pNewBunyip) {
    pBunyip = pNewBunyip;

    if (mFsm.GetState() == AS_Sneak || mFsm.GetState() == TY_AS_3) {
        pBunyip->SetState(BUNYIP_STATE_3);
    } else {
        pBunyip->SetState(BUNYIP_STATE_4);
    }
}

void Ty::SetTwirlRangs(void) {
    if (mFsm.GetState() != AS_Doomerang) {
        mFsm.SetState(TY_AS_43, false);
    }
}

void Ty::UpdateLocalToWorldMatrix(void) {
    Vector translation;
    Matrix tmpMat;

    translation.Set(pos.x, pos.y + gb.unk78C + unk19E0, pos.z);

    pModel->matrices[0].SetIdentity();

    pModel->matrices[0].SetRotationPitch(mRot.GetUnk0());

    tmpMat.SetRotationRoll(mRot.GetUnk8());

    pModel->matrices[0].Multiply3x3(&tmpMat);

    tmpMat.SetRotationYaw(mRot.GetUnk4());

    pModel->matrices[0].Multiply3x3(&tmpMat);

    pModel->matrices[0].SetTranslation(&translation);

    pModel->SetLocalToWorldDirty();
}

bool Ty::FallMove(float f1, float f2, float f3) {
    magnitude = GetSpeedFromJoyPad(f1);

    UpdateYaw(f2);

    UpdateHorzVel(f3);

    velocity.y = lastVelocity.y;

    mContext.VelocityInline(&velocity, 0.0f);

    return velocity.MagSquared() > 0.0f;
}

void Ty::EnableHead(TyHeads head) {
    // ASSERT("head >= TYH_Normal && head < TYH_Max", Str_Printf("Ty::EnableHead: head %d out of range [%d..%d]", head, TYH_Normal, TYH_Max), ...);

    pModel->EnableSubObject(A_Head_idx,     head == TYH_Normal);
    pModel->EnableSubObject(A_BiteHead_idx, head == TYH_1);
}

void Ty::CheckForRangChange(void) {
    if (mFsm.GetStateEx() != AS_Doomerang) {
        if (gb.mLogicState.GetCurr() == STATE_10 || gb.mLogicState.GetCurr() == STATE_11) {
            return;
        }
        
        bool b = gb.mJoyPad1.IsNewlyPressed(3);
        bool b1 = gb.mJoyPad1.IsNewlyPressed(2);

        if (b || b1) {
            if (!InWater() && mBoomerangManager.GetCurrentType() != BR_Aquarang) {
                if (
                    ((mFsm.GetStateEx() != TY_AS_34 && mFsm.GetStateEx() != TY_AS_35 && mFsm.GetStateEx() != TY_AS_36) || (mBoomerangManager.HasFired() || !gb.mGameData.HasBothRangs())) 
                    && mFsm.GetStateEx() != TY_AS_42
                ) {
                    BoomerangType type;
                    if (b) {
                        type = GetNextRang(mBoomerangManager.GetNextType());
                    } else {
                        type = GetPrevRang(mBoomerangManager.GetNextType());
                    }

                    if (type == BR_Aquarang) {
                        type = b ? GetNextRang(type) : GetPrevRang(type);
                    }

                    mBoomerangManager.SetType(type);

                    Hud_BoomerangScroll(type, b);
                }
            }
        }
    }
}

void Hud_InitBoomerangs(void);

void Ty::SwitchToAquaRang(void) {
    if (mBoomerangManager.GetCurrentType() != BR_Aquarang) {
        mBoomerangType = mBoomerangManager.GetCurrentType();

        mBoomerangManager.SetType(BR_Aquarang);

        if (!gb.mGameData.CheckLearntToSwim()) {
            mBoomerangManager.Disable();
        }

        Hud_InitBoomerangs();
    }
}

void Ty::SwitchBackFromAquaRang(void) {
    mBoomerangManager.SetType(mBoomerangType);
    mBoomerangManager.Enable();
    mBoomerangManager.ShowAll();

    Hud_InitBoomerangs();
}

void Ty::StartRangSpecialAnimation(BoomerangSide side, MKAnim* pAnim) {
    if (pAnim) {
        unk11F8[side].unk4 = true;
        unk11F8[side].unk8.SetAnim(pAnim);

        mRangTrails[side].Reset();
    } else {
        mBoomerangManager.Show(side);
        unk11F8[side].unk4 = false;
    }
}

void Ty::UpdateRangSpecial(void) {
    for (int i = 0; i < ARRAY_SIZE(unk11F8); i++) {
        if (unk11F8[i].unk4) {
            unk11F8[i].unk8.Animate();

            unk11F8[i].unk8.Apply(unk11F8[i].pModel->pAnimation);

            if (unk11F8[i].unk8.Condition() || ty.mFsm.GetStateEx() != TY_AS_35) {
                unk11F8[i].unk4 = false;
                unk11F8[i].pModel->matrices[0].SetIdentity();
                unk11F8[i].pModel->colour.w = 1.0f;
            }
        }
    }
}

View* GameCamera_View(void);

void Ty::UpdateBoomerangs(void) {
    UpdateRangSpecial();

    Matrix left;
    left.SetIdentity();

    Matrix right;
    right.SetIdentity();

    if (mFsm.GetStateEx() == AS_FirstPerson) {
        right = GameCamera_View()->unk48;
        left = GameCamera_View()->unk48;
    } else {
        right = *pModel->pAnimation->GetNodeMatrix(unk474);
        left = *pModel->pAnimation->GetNodeMatrix(unk460);

        right.Row3()->Copy(&unk478);
        left.Row3()->Copy(&unk464);
    }

    if (unk11F8[BOOMERANG_SIDE_RIGHT].unk4) {
        right = *unk11F8[BOOMERANG_SIDE_RIGHT].pModel->pAnimation->GetNodeMatrix(
            unk11F8[BOOMERANG_SIDE_RIGHT].pModel->pAnimation->GetNodeIndex("RangPropRight")
        );
    }

    if (unk11F8[BOOMERANG_SIDE_LEFT].unk4) {
        left = *unk11F8[BOOMERANG_SIDE_LEFT].pModel->pAnimation->GetNodeMatrix(
            unk11F8[BOOMERANG_SIDE_LEFT].pModel->pAnimation->GetNodeIndex("RangPropLeft")
        );
    }

    Matrix d;
    d.SetIdentity();
    d.SetRotationYaw(PI / 2.0f);
    right.Multiply3x3(&d, &right);

    d.SetIdentity();
    d.SetRotationYaw(PI / 2.0f);
    d.RotateRoll(PI);
    left.Multiply3x3(&d, &left);

    mBoomerangManager.Update(&left, &right, &pModel->colour);
}

void Fly_ShowTyFlies(void);

void Ty::ProcessAnimationEvents(MKAnimScript* pAnimScript) {
    if (pAnimScript->unkC == pAnimScript->unk1A) {
        return;
    }
    
    int i = 0;
    char* pEventName;
    while (pEventName = pAnimScript->GetEvent(i++)) {
        mFsm.CallEvent(this, pEventName);

        if (pEventName == unk7C8) {
            // MakeHeroBlink();
        } else if (pEventName == unk7CC) {
            mBoomerangManager.Hide(BOOMERANG_SIDE_RIGHT);
        } else if (pEventName == unk7D0) {
            mBoomerangManager.Show(BOOMERANG_SIDE_RIGHT);
        } else if (pEventName == unk7D4) {
            mBoomerangManager.Hide(BOOMERANG_SIDE_LEFT);
        } else if (pEventName == unk7D8) {
            mBoomerangManager.Show(BOOMERANG_SIDE_LEFT);
        } else if (pEventName == unk7E0) {
            unk11F8[BOOMERANG_SIDE_RIGHT].pModel->matrices[0].SetIdentity();
            unk11F8[BOOMERANG_SIDE_RIGHT].pModel->matrices[0].CopyRotation(&pModel->matrices[0]);
            unk11F8[BOOMERANG_SIDE_RIGHT].pModel->matrices[0].SetTranslation(&unk3B0);
            StartRangSpecialAnimation(BOOMERANG_SIDE_RIGHT, unk758);
            SoundBank_Play(0x1BE, NULL, 0);
        } else if (pEventName == unk7DC) {
            StartRangSpecialAnimation(BOOMERANG_SIDE_RIGHT, NULL);
        } else if (pEventName == unk7FC) {
            unkF98 = true;
        } else if (pEventName == unk800) {
            unkF98 = false;
        } else if (pEventName == unk808) {
            tySounds.unk10 = SoundBank_Play(0x1BD, NULL, 0);
        } else if (pEventName == unk80C) {
            SoundBank_Stop(&tySounds.unk10);
        } else if (pEventName == unk804) {
            Fly_ShowTyFlies();
        } else if (pEventName == unk818) {
            SoundBank_Play(SFX_TyLand, NULL, mContext.floor.GetCollisionFlags());
        } else if (pEventName == unk81C) {
            SoundBank_Play(SFX_TyJump, NULL, mContext.floor.GetCollisionFlags());
        }
    }
}

void Ty::UpdateAnimation(void) {
    ProcessAnimationEvents(&animScript);
    ProcessAnimationEvents(&unk4EC);

    animScript.Animate();
    unk4EC.Animate();

    if (unk530 > 0.0f) {
        pModel->pAnimation->Tween(animScript.unkC, 1.0f - unk530);
        unk530 -= 0.1f;
    } else {
        animScript.Apply(pModel->pAnimation);
    }

    if (!unk4EC.Condition()) {
        unk4EC.ApplyNode(pModel->pAnimation, unk488);
    }

    mBoomerangManager.UpdateAnimation(pModel);

    if (unk52C) {
        unk50C.ApplyNode(pModel->pAnimation, unk4B0);
    }
}

void Ty::SetPitchAndRoll(float f1, float f2) {
    Vector localNode = *pModel->pAnimation->GetNodeOrigin(unk334);

    Matrix custom;
    custom.SetIdentity();

    localNode.Inverse();

    custom.Translate(&localNode);

    Vector vector = {1.0f, 0.0f, 0.0f, 0.0f};

    Tools_MatriceRotate(f1, &vector, &custom);

    vector.Set(0.0f, -1.0f, 0.0f);

    Tools_MatriceRotate(f2, &vector, &custom);

    localNode.Inverse();

    custom.Translate(&localNode);

    Tools_SetNode(pModel->pAnimation, unk334, &custom, TOOLS_NODEFLAG_1);
}

void Ty::ResetPitchAndRoll(void) {
    roll = 0.0f;

    Matrix custom;
    custom.SetIdentity();

    Tools_SetNode(pModel->pAnimation, unk334, &custom, TOOLS_NODEFLAG_1);
    Tools_SetNode(pModel->pAnimation, unk334, NULL, TOOLS_NODEFLAG_0);
}

Vector* GameCamera_GetDir(void);

void Ty::ThrowBoomerang(void) {
    if (pBunyip && !unk1114 && (gb.mJoyPad1.mNewButtonsPressedThisFrame & tyControl.activeControls[2])) {
        pBunyip->SetState(BUNYIP_PUNCH);
        return;
    }

    static int lookTimeOut = gb.logicGameCount;

    if (mBoomerangManager.GetCurrentType() != BR_Aquarang || GetMedium() == TY_MEDIUM_3) {
        bool b = mBoomerangManager.GetCurrentType() == BR_Doomerang && GetMedium() != TY_MEDIUM_1;

        if (!unk1114 && (gb.mJoyPad1.mNewButtonsPressedThisFrame & tyControl.activeControls[2]) && !b) {
            StartRangSpecialAnimation(BOOMERANG_SIDE_RIGHT, NULL);
            StartRangSpecialAnimation(BOOMERANG_SIDE_LEFT, NULL);

            SoundBank_Stop(&tySounds.unk10);

            Vector dir = *mRot.GetFrontVector();

            if (mFsm.GetStateEx() == TY_AS_6) {
                float angle = GetBreakAndTurnAngle();
                dir.x = _table_cosf(angle);
                dir.z = _table_sinf(angle);
            }

            if (GetMedium() == TY_MEDIUM_3) {
                dir.x *= _table_cosf(pitch);
                dir.z *= _table_cosf(pitch);
                dir.y = _table_sinf(pitch);
            } else {
                Vector end;
                end.Scale(&dir, 300.0f);
                end.Add(&pos);

                CollisionResult cr;

                // changed from 390.0f to 190.0f in the debug build
                if (Tools_TestFloor(end, 200.0f, &cr, 390.0f, false)) {
                    if (cr.pos.y > pos.y) {
                        dir.Sub(&cr.pos, &ty.pos);
                        dir.Normalise();
                    }
                }
            }

            if (mFsm.FirstPersonState()) {
                dir = *GameCamera_GetDir();
            }

            mAutoTarget.SetUnk208(2);

            // Fire both if kaboomerang
            if (mBoomerangManager.Fire(&dir, mFsm.FirstPersonState() ? NULL : mAutoTarget.GetTargetPos(), mBoomerangManager.GetCurrentType() == BR_Kaboomerang)) {
                lookTimeOut = gb.logicGameCount + (int)(gDisplay.fps * 0.8f);
            }
        }

        if ((int)gb.logicGameCount > lookTimeOut) {
            ty.mAutoTarget.SetUnk208(0);
        }

        unk1114 = gb.mJoyPad1.mNewButtonsPressedThisFrame & tyControl.activeControls[2];
    }
}

bool Ty::IsJoyPadZero(void) {
    return gb.mJoyPad1.GetUnk58() == 0.0f && gb.mJoyPad1.GetUnk5C() == 0.0f;
}

bool Ty::TryChangeState(bool r4, HeroActorState nState) {
    if (pBunyip) {
        if (gb.mJoyPad1.mNewButtonsPressedThisFrame & tyControl.activeControls[1]) {
            pBunyip->SetState(BUNYIP_ROAR);
        }

        if (!mFsm.MoveState(nState) && !mFsm.SwimmingState(nState) && nState != TY_AS_35 && nState != TY_AS_7 && nState != TY_AS_26) {
            return false;
        }

        if (pBunyip->GetState() == BUNYIP_PUNCH || pBunyip->GetState() == BUNYIP_ROAR) {
            if (nState != TY_AS_35 && nState != TY_AS_26) {
                nState = TY_AS_35;
            }
        }
    }

    if (r4 && ty.mFsm.GetStateEx() != nState) {
        ty.mFsm.SetState(nState, false);
        return true;
    }

    return false;
}

bool Ty::TryChangeState(int r4, HeroActorState nState) {
    return TryChangeState(r4 ? true : false, nState);
}

void Ty::GetNewNodePositions(void) {
    pModel->pAnimation->GetNodeWorldPosition(unk320, &unk324);
    pModel->pAnimation->GetNodeWorldPosition(unk334, &unk338);
    pModel->pAnimation->GetNodeWorldPosition(unk49C, &unk4A0);
    pModel->pAnimation->GetNodeWorldPosition(unk488, &unk48C);
    pModel->pAnimation->GetNodeWorldPosition(unk3C0, &unk3C4);
    pModel->pAnimation->GetNodeWorldPosition(unk3D4, &unk3D8);
    pModel->pAnimation->GetNodeWorldPosition(unk460, &unk464);
    pModel->pAnimation->GetNodeWorldPosition(unk474, &unk478);
    pModel->pAnimation->GetNodeWorldPosition(unk348, &unk34C);
    pModel->pAnimation->GetNodeWorldPosition(unk35C, &unk360);
    pModel->pAnimation->GetNodeWorldPosition(unk370, &unk374);
    pModel->pAnimation->GetNodeWorldPosition(unk384, &unk388);
    pModel->pAnimation->GetNodeWorldPosition(unk398, &unk39C);
    pModel->pAnimation->GetNodeWorldPosition(unk3AC, &unk3B0);
    pModel->pAnimation->GetNodeWorldPosition(unk3E8, &unk3EC);

    pModel->GetRefPointWorldPosition(unk410, &unk414);

    pModel->pAnimation->GetNodeWorldPosition(unk424, &unk428);

    pModel->GetRefPointWorldPosition(unk3FC, &unk400);
    pModel->GetRefPointWorldPosition(unk438, &unk43C);
    pModel->GetRefPointWorldPosition(unk44C, &unk450);

    int numMatrices = pModel->pAnimation->GetNmbrOfMatrices();
    for (int i = 0; i < numMatrices; i++) {
        pModel->pAnimation->pMatrices[i].Row0();
        pModel->pAnimation->pMatrices[i].Row1();
        pModel->pAnimation->pMatrices[i].Row2();
        pModel->pAnimation->pMatrices[i].Row3();
    }
}

void Ty::Pitch(float f1) {
    float smoothing = 0.15f;
    float ang = NormaliseAngle(f1 - pitch);
    if (ang > PI) {
        ang -= 2 * PI;
    }

    pitch += ang * smoothing;

    SetPitchAndRoll(pitch, 0.0f);
}

void AutoTargetStruct::Set(TargetPriority prio, Vector* r5, Vector* r6, Vector* r7, Model* pModel) {
    Vector tyPos = ty.pos;
    tyPos.y += ty.radius;

    if (prio >= TP_2) {
        if (r5 && ty.mBoomerangManager.GetCurrentType() == BR_Megarang) {
            unk74.AddToList(r5, pModel);
            unkD8.AddToList(r5, pModel);
        }
        
        if (r6 && ty.mFsm.GetStateEx() == AS_Bite) {
            unk13C.AddToList(r6, pModel);
        }
    }

    if (prio == TP_3) {
        SetNearestTargetEnemy(r7, pModel, &tyPos);
        unk54 = unk5C = -0.4f;
    }

    if (r5) {

    }

    if (r6) {

    }

    if (r7) {

    }

    if (unk208 == 2) {
        unk1E8 = unk1E0;
        unk1F8 = targetPos;
        targetPriority = unk1F0;
    } else if (unk208 == 1) {
        unk1E8 = unk1E4;
        unk1F8 = unk1C0;
        targetPriority = unk1EC;
    } else if (unk208 == 3) {
        unk1E8 = NULL;
        unk1F8.SetZero();
        targetPriority = 0;
    }
}

void AutoTargetStruct::SetNearestTargetEnemy(Vector*, Model*, Vector*) {

}

void AutoVisible::AddToList(Vector* pVec, Model* pModel) {
    if (pVec->IsInsideSphere(&unk54, 750.0f) && numItems < 10 && pModel) {
        unk0[numItems] = pModel->matrices[0].Row3();
        unk28[numItems] = pVec->y - pModel->matrices[0].Row3()->y;

        numItems++;
    }
}

void AutoTargetStruct::Reset(void) {
    for (int i = 0; i < ARRAY_SIZE(unk0); i++) {
        unk0[i].unk0 = 0.0f;
        unk0[i].unk4.SetZero();
        unk0[i].unk14 = 0;
    }

    targetPos.SetZero();
    unk1B0.SetZero();
    unk1C0.SetZero();
    unk1D0.SetZero();
    
    unk1E0 = NULL;
    unk1E4 = NULL;
    unk1E8 = NULL;
    unk1EC = 0;
    unk1F0 = 0;
    targetPriority = 0;

    unk1F8.SetZero();

    unk74.numItems = unkD8.numItems = unk13C.numItems = 0;

    unk48 = Sqr<float>(1000.0f);
    unk4C = 0.8f;

    if (ty.GetMedium() == TY_MEDIUM_4) {
        unk50 = Sqr<float>(400.0f) * 2.0f;
    } else {
        unk50 = Sqr<float>(300.0f);
    }

    unk54 = 0.0f;

    unk58 = Sqr<float>(800.0f);
    unk5C = 0.0f;
    unk60 = 300.0f;

    tyRot = *ty.mRot.GetFrontVector();

    if (ty.GetMedium() == TY_MEDIUM_3) {
        unk60 *= 20.0f;
        unk48 *= 2.0f;
        unk58 *= 2.0f;
        unk4C = 0.7f;

        Matrix rotation;
        rotation.SetRotationPitch(-ty.pitch);

        tyRot.ApplyRotMatrix(&rotation);
    }
}

void Ty::GetNodesAndSubObjects(void) {
    pModel->SetPosition(&pos);
    pModel->SetRotation(mRot.GetRotVector());
    
    pModel->RefPointExists("R_EYE", &unk410);
    pModel->RefPointExists("R_WING_L", &unk438);
    pModel->RefPointExists("R_WING_R", &unk44C);

    if (pModel->pAnimation) {
        pModel->pAnimation->NodeExists("Z_HEAD", &unk320);
        pModel->pAnimation->NodeExists("Z_ROOT", &unk334);
        pModel->pAnimation->NodeExists("Z_RANG_L", &unk460);
        pModel->pAnimation->NodeExists("Z_RANG_R", &unk474);
        pModel->pAnimation->NodeExists("Z_BICEP_L", &unk3C0);
        pModel->pAnimation->NodeExists("Z_BICEP_R", &unk3D4);
        pModel->pAnimation->NodeExists("Z_WAIST", &unk488);
        pModel->pAnimation->NodeExists("Z_PELVIS", &unk49C);
        pModel->pAnimation->NodeExists("Z_TOE_L", &unk348);
        pModel->pAnimation->NodeExists("Z_TOE_R", &unk35C);
        pModel->pAnimation->NodeExists("Z_HAND_L", &unk370);
        pModel->pAnimation->NodeExists("Z_HAND_R", &unk384);
        pModel->pAnimation->NodeExists("Z_GLOVE_L", &unk398);
        pModel->pAnimation->NodeExists("Z_GLOVE_R", &unk3AC);
        pModel->pAnimation->NodeExists("Z_NOSE", &unk424);
        pModel->RefPointExists("R_NOSE", &unk3FC);
        pModel->pAnimation->NodeExists("z_FaceMouth", &unk4B0);
        pModel->pAnimation->NodeExists("n_TyOffset", &unk3E8);
    }

    A_Head_idx = pModel->GetSubObjectIndex("A_Head");
    A_BiteHead_idx = pModel->GetSubObjectIndex("A_BiteHead");

    EnableHead(TYH_Normal);
}

void Ty::LoadAnimations(void) {
    pSneakAnim          = unk534.GetAnim("sneak");
    walkAnim            = unk534.GetAnim("walk");
    jogAnim             = unk534.GetAnim("jog");
    run00Anim           = unk534.GetAnim("run00");
    unk5C4      = unk534.GetAnim("hillSlideBelly");
    unk5C8      = unk534.GetAnim("hillSlideBum");
    dogPaddleAnim       = unk534.GetAnim("dogPaddle");
    surfaceSwimAnim     = unk534.GetAnim("surfaceSwim");
    unk5D4      = unk534.GetAnim("surfaceSwimIdle");
    swimSlowAnim        = unk534.GetAnim("swimSlow");
    swimFastAnim        = unk534.GetAnim("swimFast");
    swimIdleAnim        = unk534.GetAnim("swimIdle");
    unk5E4      = unk534.GetAnim("swimDiveOne");
    unk5E8      = unk534.GetAnim("swimDiveTwo");
    unk5EC      = NULL;
    drownGaspAnim       = unk534.GetAnim("DrownGasp");
    drownDeadAnim       = unk534.GetAnim("DrownDead");
    unk5F8      = unk534.GetAnim("DrownSettle");
    unk5FC      = unk534.GetAnim("bite1");
    unk600      = unk534.GetAnim("bite1a");
    unk604      = unk534.GetAnim("biteGrowl1");
    unk608      = unk534.GetAnim("bite2");
    unk60C      = unk534.GetAnim("biteGrowl2");
    unk610      = unk534.GetAnim("bite3");
    unk614      = unk534.GetAnim("biteGrowl3");
    unk618      = unk534.GetAnim("bite4");
    unk61C      = unk534.GetAnim("biteGrowl4");
    unk620      = unk534.GetAnim("biteCharge");
    unk624      = unk534.GetAnim("biteChargeLoop");
    unk628      = unk534.GetAnim("biteFly");
    unk62C      = unk534.GetAnim("biteFlyEnd");
    unk630      = unk534.GetAnim("fallDive1");
    unk634      = unk534.GetAnim("fallDiveCycle");
    unk638      = unk534.GetAnim("fallDive2");
    unk63C      = unk534.GetAnim("fallDive1_backwards");
    unk640      = unk534.GetAnim("fallDive1_Bomb");
    unk644      = unk534.GetAnim("fallDiveCycle_Bomb");
    unk648      = unk534.GetAnim("fallDive2_Bomb");
    unk65C      = unk534.GetAnim("biteStun");
    unk650      = unk534.GetAnim("biteDiveForward");
    unk658      = unk534.GetAnim("biteDiveHeadStuck");
    unk654      = unk534.GetAnim("biteDiveIdle");
    unk64C      = unk534.GetAnim("biteDiveRise");
    // unk664      = unk534.GetAnim("grab");
    // unk66c      = unk534.GetAnim(@3990);
    // unk674      = unk534.GetAnim("PullUp");
    // hasBothRangAnims[1] = unk534.GetAnim(@3992);
    // unk660      = unk534.GetAnim(@3993);
    // unk668      = unk534.GetAnim(@3994);
    // unk670      = unk534.GetAnim(@3995);
    // hasBothRangAnims[0] = unk534.GetAnim(@3996);
    // unk680      = unk534.GetAnim(@3997);
    biteUnchargeAnim    = unk534.GetAnim("relaxIdle");
    // unk688      = unk534.GetAnim(@3999);
    // unk68c      = unk534.GetAnim(@4000);
    // unk690      = unk534.GetAnim(@4001);
    // unk694      = unk534.GetAnim(@4002);
    // unk698      = unk534.GetAnim(@4003);
    // unk6a4      = unk534.GetAnim("jump");
    // unk6a8      = unk534.GetAnim(@4005);
    // unk6b8      = unk534.GetAnim(@4006);
    // unk6ac      = unk534.GetAnim(@4007);
    // unk6b0      = unk534.GetAnim(@4008);
    // unk6b4      = unk534.GetAnim(@4009);
    // unk6bc      = unk534.GetAnim("fall");
    // unk6c0      = unk534.GetAnim(@4011);
    // unk6c4      = unk534.GetAnim(@4012);
    // unk6c8      = unk534.GetAnim(@4013);
    // unk6cc      = unk534.GetAnim(@1046);
    // unk6d0      = unk534.GetAnim(@4014);
    // unk6d4      = unk534.GetAnim(@4015);
    // unk6d8      = unk534.GetAnim(@4016);
    // unk6dc      = unk534.GetAnim(@4017);
    // unk6e0      = unk534.GetAnim(@1047);
    // unk6e4      = unk534.GetAnim(@4018);
    // unk6e8      = unk534.GetAnim(@4019);
    // unk6ec      = unk534.GetAnim(@4012);
    splatLandAnim       = unk534.GetAnim("splatLand");
    getUpAnim           = unk534.GetAnim("getUp");
    // knockdownAnim       = unk534.GetAnim(@4022);
    // unk6fc      = unk534.GetAnim(@4023);
    // unk700      = unk534.GetAnim(@4024);
    // unk704      = unk534.GetAnim(@4025);
    // flinchAnims[0]      = unk534.GetAnim("flinch1");
    // flinchAnims[1]      = unk534.GetAnim("flinch2");
    // flinchAnims[2]      = unk534.GetAnim("flinch3");
    // unk714      = 0;
    // unk718      = 0;
    // unk71c      = unk534.GetAnim(@4029);
    // unk720      = unk534.GetAnim(@4030);
    // unk724      = nullptr;
    // unk72c      = unk534.GetAnim("death");
    // unk730      = 0;
    // unk734      = 0;
    // unk738      = 0;
    // unk73c      = unk534.GetAnim(@4032);
    // unk740      = unk534.GetAnim(@4033);
    // unk744      = unk534.GetAnim(@4034);
    // unk748      = unk534.GetAnim(@4035);
    // unk74c      = unk534.GetAnim(@4036);
    // unk750      = unk534.GetAnim(@4037);
    // unk754      = unk534.GetAnim(@4038);
    // edgeWobbleAnim      = unk534.GetAnim(@4039);
    // waterShakeAnim      = unk534.GetAnim(@4040);
    // afterDunnyAnim      = unk534.GetAnim(@4041);
    // unk758      = rangPropRightAnimScript.GetAnim(@4000);
    // skidAnim            = unk534.GetAnim("skid");
    // skidRecoverAnim     = unk534.GetAnim(@4043);
    // skidTurnAnim        = unk534.GetAnim(@4044);
    // unk768      = unk534.GetAnim(@4045);
    // bogusDiveAnim       = unk534.GetAnim(@4046);
    // lipsDrownAnim       = unk534.GetAnim(@4047);
    // sharkCageEnterAnim  = unk534.GetAnim(@4048);
    // rangChangeStartAnim = unk534.GetAnim(@4049);
    // unk77c      = unk534.GetAnim(@4050);
    // unk780      = unk534.GetAnim(@4051);
    unk784      = unk534.GetAnim("TyShootUp");

    for (int i = 0; i < ARRAY_SIZE(ClaimLandAnims); i++) {
        ClaimLandAnims[i] = unk534.GetAnim(Str_Printf("ClaimLand%d", i + 1));
        ClaimWaterAnims[i] = unk534.GetAnim(Str_Printf("ClaimWater%d", i + 1));
    }
}

void TyContext::WaterInfo::Update(Vector* p, Vector* p1) {
    Vector start = *(p->y > p1->y ? p : p1);
    Vector end = *(p->y > p1->y ? p1 : p);

    start.y += 300.0f;
    end.y -= 300.0f;

    CollisionResult cr;

    bValid = Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, ~ID_WATER_BLUE);
    if (bValid) {
        pos = cr.pos;
    }

    if (!bValid) {
        pos = *p;
        bValid = WaterVolume_IsWithin(p, &pos.y);
    }

    if (bValid) {
        bValid = p->y < pos.y;
    } else if (ty.GetMedium() == TY_MEDIUM_3) {
        bValid = true;
    }
}

void GameCamera_SetPlatformYawDelta(float);

void Ty::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_UpdateAttachment: {
            PlatformMoveMsg* pUpdateMsg = (PlatformMoveMsg*)pMsg;

            Vector v = mLedgePullUpData.unk4;

            v.ApplyMatrix(pUpdateMsg->pModelInverseMatrix);
            v.ApplyMatrix(pUpdateMsg->pModelMatrix);

            Vector deltaPos = {
                0.0f,
                0.0f,
                0.0f,
                1.0f
            };

            deltaPos.Sub(&v, &mLedgePullUpData.unk4);

            mLedgePullUpData.LedgeMoved(&deltaPos, pUpdateMsg->rot->y);

            SetAbsolutePosition(pUpdateMsg->trans, 20, 1.0f, GetMedium() == TY_MEDIUM_1);

            mRot.IncreaseUnk4(pUpdateMsg->rot->y);

            UpdateLocalToWorldMatrix();

            GameCamera_SetPlatformYawDelta(pUpdateMsg->rot->y);

            break;
        };
    }

    GameObject::Message(pMsg);
}

void Ty::ResetDrownTimer(void) {
    unk828 = gb.logicGameCount + (int)((gDisplay.fps * 180.0f) / tyHealth.GetHealthFieldUnk4());
}

void Ty::WaterMediumInit(void) {
    BoomerangManagerAnims swimAnims = {
        false,
        unk6C8,
        {unk6C8, unk6DC},
        {unk6D4, unk6E8}
    };

    mBoomerangManager.SetAnims(&swimAnims);

    if (pBunyip) {
        pBunyip->SetState(BUNYIP_DISAPPEAR);
    }
}

void Ty::WaterMediumUpdate(void) {
    
}

void Ty::WaterMediumDeinit(void) {
    
}

void Hud_ShowHealthMeter(bool);

void Ty::UnderWaterMediumInit(void) {
    BoomerangManagerAnims underWaterAnims = {
        true,
        unk6C8,
        {unk6C8, unk6DC},
        {unk6D0, unk6E4}
    };

    mBoomerangManager.SetAnims(&underWaterAnims);

    tyHealth.SetType(HEALTH_TYPE_1);

    SoundBank_PlayExclusiveAmbientSound(false);

    Hud_ShowHealthMeter(true);

    ResetDrownTimer();

    if (pBunyip) {
        pBunyip->SetState(BUNYIP_DISAPPEAR);
    }
}

void Ty::UnderWaterMediumUpdate(void) {
    if (gb.logicGameCount >= unk828) {
        ty.Hurt(HURT_TYPE_6, DDA_DAMAGE_3, false, NULL, 15.0f);
        ResetDrownTimer();
    }
}

void Ty::UnderWaterMediumDeinit(void) {
    Hud_ShowHealthMeter(false);

    tyHealth.SetType(HEALTH_TYPE_0);

    SoundBank_PlayExclusiveAmbientSound(true);
}

static int airStuckCount = 0;

void Ty::AirMediumInit(void) {
    unk167E = false;

    airStuckCount = 0;

    BoomerangManagerAnims bmAnims = {
        true,
        unk6EC,
        {unk6C4, unk6D8},
        {unk6CC, unk6E0}
    };

    mBoomerangManager.SetAnims(&bmAnims);
}

void Ty::AirMediumUpdate(void) {
    float distSq = SquareDistance(&pos, &lastPos);
    if (distSq < Sqr<float>(0.2f) && velocity.MagSquared() > Sqr<float>(4.0f)) {
        airStuckCount++;

        if (airStuckCount > 10) {
            airStuckCount = 0;

            velocity.SetZero();
            SetFakeFloor();
        }
    } else {
        airStuckCount = 0;
    }
}

void Ty::AirMediumDeinit(void) {
    unk167E = (mContext.floor.GetDiff(&pos) < 50.0f) && (mContext.floor.GetCollisionFlags() & 2);

    if (mContext.floor.bOn || mContext.floor.bUnderFeet) {
        unk141C = gb.logicGameCount;
    }
}

void Ty::LandMediumInit(void) {
    BoomerangManagerAnims boomAnims = {
        true,
        unk6EC,
        {unk6C4, unk6D8},
        {unk6CC, unk6E0}
    };

    mBoomerangManager.SetAnims(&boomAnims);
}

void Ty::LandMediumUpdate(void) {
    if (unk167E && !(mContext.floor.GetCollisionFlags() & 2)) {
        unk167E = false;
    }
}

void Ty::LandMediumDeinit(void) {
    
}

bool Ty::IsBiting(void) {
    return mFsm.GetState() == AS_Bite;
}

void TySounds::Init(void) {
    mFader1.Init(1.5f, 1.5f);
    mFader3.Init(1.5f, 1.5f);
    mFader2.Init(1.0f, 0.0f);

    mPhrasePlayer.Init();

    unk0 = -1;
    unk4 = -1;
    unk8 = -1;
    unkC = -1;
    unk10 = -1;
}

void TySounds::Reset(void) {
    mFader1.Reset();
    mFader2.Reset();
    mFader3.Reset();

    mPhrasePlayer.Deinit();

    SoundBank_Stop(&unk0);
    SoundBank_Stop(&unk4);
    SoundBank_Stop(&unk8);
    SoundBank_Stop(&unkC);
    SoundBank_Stop(&unk10);
}

void TySounds::UpdateSounds(void) {
    if (pHero->IsTy()) {
        bool b, b1, b2;
        int tyState = ty.mFsm.GetState();

        DialogPlayer*& p = (DialogPlayer*&)gb.pDialogPlayer;

        b = !p && (tyState == TY_AS_15);

        b1 = !p && (tyState == TY_AS_14 || tyState == TY_AS_37);

        b2 = !p && (
            (
                ((tyState == TY_AS_34 || tyState == TY_AS_35) && ty.mContext.floor.bOn) &&
                ty.directvel_to_velocity_interpolation == 20
            ) && ty.directVelocity.MagSquared() > 0.7f
        );

        // bool dialogOff = gb.pDialogPlayer == NULL;

        // b = dialogOff && (tyState == TY_AS_15);

        // b1 = dialogOff && (tyState == TY_AS_14 || tyState == TY_AS_37);

        // b2 = dialogOff && (
        //     (
        //         ((tyState == TY_AS_34 || tyState == TY_AS_35) && ty.mContext.floor.bOn) &&
        //         ty.directvel_to_velocity_interpolation == 20
        //     ) && ty.directVelocity.MagSquared() > 0.7f
        // );

        mFader1.Update(0x6, false, b, NULL, NULL, -1.0f, ID_NONE);
        mFader3.Update(0x25, false, b1, NULL, NULL, 0.0f, ID_WATER_BLUE);
        mFader2.Update(0x14C, false, b2, NULL, NULL, -1.0f, ID_NONE);
    }
}

void Ty::InitRangChange(void) {
    StartAnimation(&animScript, unk778, 5, false);

    mRangChangeData.boomerangButton = (gb.mJoyPad1.mButtonsPressed & tyControl.buttonVals[3]) ? 1 : -1;
    mRangChangeData.unk4 = false;
}

void Ty::DeinitRangChange(void) {
    mRangChangeData.PerformChange();
}

void Ty::RangChange(void) {
    UpdateAnimation();

    ThrowBoomerang();

    bool b = gb.mJoyPad1.IsNewlyPressed(3);
    bool b1 = gb.mJoyPad1.IsNewlyPressed(2);

    mRangChangeData.unk4 = mRangChangeData.unk4 || b1 || b;
    

    if (animScript.currAnim != unk778 && animScript.nextAnim != unk778) {
        mRangChangeData.boomerangButton += b;
        mRangChangeData.boomerangButton -= b1;
        mRangChangeData.PerformChange();
    }

    if (animScript.Condition() && (animScript.currAnim == unk778 || animScript.currAnim == unk77C)) {
        if (mRangChangeData.unk4) {
            mRangChangeData.unk4 = false;
            StartAnimation(&animScript, unk77C, 0, true);
        } else {
            StartAnimation(&animScript, unk780, 5, true);
        }
    }


    RangChangeTransition();
}

void Ty::RangChangeTransition(void) {
    if (animScript.Condition() || mBoomerangManager.HasFired()) {
        // TryChangeState()
    }
}

void Ty::SwapRangs(char* r4) {
    if (r4 == unk810) {
        mRangChangeData.PerformChange();
    }
}

void Ty::InitTwirlRang(void) {
    StartAnimation(&animScript, unk778, 5, true);
}

void Ty::TwirlRang(void) {
    UpdateAnimation();

    if (animScript.Condition() && animScript.currAnim == unk778) {
        StartAnimation(&animScript, unk780, 5, false);
    }

    bool b = animScript.Condition() && animScript.currAnim == unk780;

    TryChangeState(b, TY_AS_35);
}

bool Ty::IsAbleToGlide(void) {
    return mBoomerangManager.AreBothReady() &&
        gb.mGameData.HasBothRangs() &&
        !mFsm.SwimmingState(mFsm.unk4) &&
        mFsm.unk4 != TY_AS_51 &&
        (mContext.floor.GetDiff(&pos) > 200.0f || (mContext.floor.GetCollisionFlags() & 8) == 0);
}

void OpalMagnetData::Init(void) {
    unk8 = Material::Create("FX_100");
    Reset();
}

void OpalMagnetData::Deinit(void) {
    if (unk8) {
        unk8->Destroy();
    }

    unk8 = NULL;
}

void OpalMagnetData::Reset(void) {
    mEndTime = 0;
    unk4 = 0.0f;
}

void OpalMagnetData::Draw(void) {

}

void OpalMagnetData::Activate(void) {

}

bool OpalMagnetData::IsActive(void) {
    return mEndTime > gb.logicGameCount;
}

void Particle_DestroyASystem(ParticleSystem**, float);

void GlowParticleData::Init(void) {
    Reset();
}

void GlowParticleData::Deinit(void) {
    if (pSys) {
        Particle_DestroyASystem(&pSys, 0.0f);
    }

    pSys = NULL;
}

void GlowParticleData::Reset(void) {

}

void GlowParticleData::Update(void) {

}

void GlowParticleData::Activate(Vector, float) {

}
