#include "ty/Ty.h"
#include "ty/global.h"
#include "ty/ParticleEngine.h"
#include "ty/bunyip.h"
#include "ty/main.h"
#include "ty/controlval.h"

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
        // GetNodesAndSubObjects();
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
        // GetNodesAndSubObjects();
        // LoadAnimations();

        StartAnimation(&animScript, biteUnchargeAnim, 0, false);
        StartAnimation(&unk4EC, biteUnchargeAnim, 0, true);

        unk11F8[0].Init(BOOMERANG_SIDE_LEFT);
        unk11F8[1].Init(BOOMERANG_SIDE_RIGHT);
    }
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
        // UpdateBoomerangs();
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

void AutoTargetStruct::Set(TargetPriority prio, Vector*, Vector*, Vector*, Model*) {

}

void AutoTargetStruct::SetNearestTargetEnemy(Vector*, Model*, Vector*) {

}

void AutoVisible::AddToList(Vector*, Model*) {

}

void AutoTargetStruct::Reset(void) {

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

}

void Ty::TwirlRang(void) {
    UpdateAnimation();

    if (animScript.Condition() && animScript.currAnim == unk778) {
        StartAnimation(&animScript, unk780, 5, false);
    }

    TryChangeState(animScript.Condition() && animScript.currAnim == unk780, TY_AS_35);
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
