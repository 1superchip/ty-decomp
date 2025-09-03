#include "ty/Ty.h"
#include "ty/global.h"
#include "ty/ParticleEngine.h"
#include "ty/bunyip.h"

static GameObjDesc tyDesc;
Ty ty;

static ModuleInfo<Ty> tyModuleInfo;
Hero* pHero;
static bool bInitialised = false;
static bool bResourcesLoaded = false;

bool TyFSM::SolidSurfaceState(int state) {
    if (
        state == TY_AS_3 || 
        state == TY_AS_5 ||
        state == TY_AS_35 ||
        state == TY_AS_32 ||
        state == TY_AS_34 ||
        state == TY_AS_28 ||
        state == TY_AS_33 ||
        state == TY_AS_44 ||
        state == TY_AS_36 ||
        state == TY_AS_46 ||
        state == TY_AS_2 ||
        state == TY_AS_4 ||
        state == TY_AS_40 ||
        state == TY_AS_41
    ) {
        return true;
    }
    
    return false;
}

bool TyFSM::BiteState(int state) {
    if (state == TY_AS_1) {
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
    if (state == TY_AS_5 || state == TY_AS_3 || state == TY_AS_2 || state == TY_AS_4) {
        return true;
    }

    return false;
}

bool TyFSM::SneakState(int state) {
    return state == TY_AS_2;
}

bool TyFSM::WaterSurfaceState(int state) {
    return state == TY_AS_14 || state == TY_AS_37 || state == TY_AS_15 || state == TY_AS_38 || state == TY_AS_21;
}

bool TyFSM::FirstPersonState(int state) {
    return state == TY_AS_45;
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

char* playerFileNames[] = {
    "act_01_ty",
    "act_01_tyshadow",
    "act_01_ty_01",
    "act_01_ty_02"
};

void Ty::LoadResources(void) {
    rangPropLeftAnimScript.Init("prop_0137_rangpropleft");
    rangPropRightAnimScript.Init("prop_0137_rangpropright");
    unk534.Init(playerFileNames[gb.unk100 * 4]);

    animScript.Init(&unk534);
    unk4EC.Init(&unk534);
    unk50C.Init(&unk534);

    unk52C = false;

    if (gb.level.GetCurrentLevel() == LN_OUTBACK_SAFARI) {
        pModel = Model::Create(unk534.GetMeshName(), NULL);
        // GetNodesAndSubObjects();
    } else {
        pModel = Model::Create(unk534.GetMeshName(), unk534.GetAnimName());

        pReflectionModel = Model::Create(playerFileNames[gb.unk100 * 4 + 1], NULL);
        pReflectionModel->pAnimation = pModel->GetAnimation();

        mShadow.Init(
            playerFileNames[gb.unk100 * 4 + 1],
            pModel,
            250.0f,
            NULL
        );

        pMatEyes = Material::Find(playerFileNames[gb.unk100 * 4 + 3]);
        pMatEyeballs = Material::Find("Act_01_TY_01a");
        // GetNodesAndSubObjects();
        // LoadAnimations();

        unk530 = 0.0f;
    }
}

void Ty::Init(void) {
    Ty::Init(&tyDesc);

    pLastCheckPoint = NULL;

    LoadResources();
}

void Ty::InitEvents(void) {

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

        mTyRainbowEffect.Deinit();
    }

    mBoomerangManager.Deinit();
    // FreeResources();
    GameObject::Deinit();
}

void Ty::Reset(void) {
    if (pHero->IsTy()) {
        ResetVars();
        SetAbsolutePosition(&pos, TY_AS_50, 1.0f, true);
        lastSafePos = pos;

        mTyRainbowEffect.Reset();

        tyHealth.SetType(HEALTH_TYPE_0);
        tySounds.Reset();
        opalMagnetData.Reset();
        glowParticleData.Reset();
    }
}

void Ty::ResetVars(void) {

}

void Ty::Update(void) {

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

void VibrateJoystick(float, float, float, char, float);

void Ty::Hurt(HurtType hurtType, DDADamageCause damageCause, bool, Vector* pVec, float f1) {
    static int flinch = 0;
    
    if (gb.disableTriggers || gb.infinitePie) {
        return;
    }

    if (invicibilityFrames > 0 || mFsm.GetStateEx() == TY_AS_28 || mFsm.GetStateEx() == TY_AS_29) {
        return;
    }

    EnableHead(TY_HEAD_0);

    if (hurtType == HURT_TYPE_1 || hurtType == HURT_TYPE_5) {
        VibrateJoystick(0.5f, 1.0f, (f1 / 45.0f) + 0.2f, 0, 4.0f);
    }

    dda.StoreDamageInfo(damageCause);

    if (ty.mMediumMachine.GetUnk0() == TY_MEDIUM_3 && mFsm.GetStateEx() != TY_AS_25) {
        for (int i = 0; i < 31; i++) {

        }

        SoundBank_Play(0x9, NULL, ID_NONE);
    }

    if (tyHealth.Hurt(hurtType)) {
        switch (hurtType) {
            case HURT_TYPE_0:
            case HURT_TYPE_3:
            case HURT_TYPE_6:
                flinch = (flinch + 1) % 3;
                if (unk4EC.Condition()) {
                    unk4EC.SetAnim(NULL);
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

// void Ty::SetToIdle(bool, TyMedium) {

// }

bool Ty::IsClaiming(void) {
    return mFsm.GetState() == TY_AS_33;
}

void AutoTargetStruct::Set(TargetPriority prio, Vector*, Vector*, Vector*, Model*) {

}

void AutoTargetStruct::SetNearestTargetEnemy(Vector*, Model*, Vector*) {

}

void AutoVisible::AddToList(Vector*, Model*) {

}

void AutoTargetStruct::Reset(void) {

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
    return mFsm.GetState() == TY_AS_1;
}

void TySounds::Init(void) {

}

void TySounds::Reset(void) {

}

void TySounds::UpdateSounds(void) {

}

void Ty::InitRangChange(void) {

}

void Ty::DeinitRangChange(void) {

}

void Ty::RangChange(void) {

}

void Ty::RangChangeTransition(void) {

}

void Ty::SwapRangs(char*) {

}

void Ty::InitTwirlRang(void) {

}

void Ty::TwirlRang(void) {

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
