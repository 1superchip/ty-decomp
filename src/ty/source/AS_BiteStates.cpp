#include "ty/Ty.h"
#include "ty/global.h"
#include "ty/Kinematics.h"
#include "common/MKRumble.h"

extern "C" int Sound_IsVoicePlaying(int);

extern void Particle_DestroyASystem(ParticleSystem**, float);
void Particle_BiteGlow_Init(ParticleSystem**, Matrix*, int);
void Particle_BiteGlow_SetToExpand(bool);

void GameCamera_UseCurrentVector(bool);
void GameCamera_UseAutoCenter(bool, float);

struct EnemyObject {
    static bool bCanAttackHero;

    static void SetCanAttackHero(bool bCanAttack) {
        EnemyObject::bCanAttackHero = bCanAttack;
    }
};

static StateMachine<TyBite>::State biteStates[] = {
    {}
};

void ScrubTurkey_IncrementCatchAttemps(void);

void Ty::InitBite(void) {
    ResetPitchAndRoll();

    ty.pitch = 0.0f;

    tyBite.Reset();

    if (ty.GetMedium() == TY_MEDIUM_4) {
        tyBite.fsm.Init(biteStates, BS_3);
    } else if (gb.mGameData.CheckChargeBites()) {
        tyBite.fsm.Init(biteStates, BS_0);
    } else {
        tyBite.fsm.Init(biteStates, BS_1);
    }

    ty.mAutoTarget.SetUnk208(1);

    ScrubTurkey_IncrementCatchAttemps();
}

void Ty::Bite(void) {
    ty.SetTyCameraChecks(14);

    ty.UpdateAnimation();
    tyBite.fsm.Update(&tyBite, false);

    if (!tyBite.bCollisionHit) {
        tyBite.ApplyGravity();
    }

    tyBite.unkD0++;

    AirToWaterTransition();
}

void Ty::DeinitBite(void) {
    ResetPitchAndRoll();
    tyBite.Reset();
    tyBite.fsm.Deinit(&tyBite);
    EnableHead(TYH_Normal);
    ty.mAutoTarget.SetUnk208(0);
}

void Ty::EventBite(char* arg1) {
    if (arg1 == unk7E4) {
        EnableHead(TYH_1);
    } else if (arg1 == unk7E8) {
        EnableHead(TYH_Normal);
    }
}

void TyBite::Init(void) {
    for (int i = 0; i < ARRAY_SIZE(unk10); i++) {
        unk10[i].SetIdentity();
    }

    unk10[0].RotatePitch(PI / 4.0f);
    unk10[0].SetTranslation(&ty.unk338);

    unk10[1].RotateYaw(1.3f);
    unk10[1].Scale(0.2f);

    unk10[2].Scale(0.2f);

    Particle_BiteGlow_Init(&pSystem, unk10, ARRAY_SIZE(unk10));

    pMat = NULL;
    pMat = Material::Create("FX_100");
}

void TyBite::Deinit(void) {
    if (pSystem) {
        Particle_DestroyASystem(&pSystem, 0.0f);
        pSystem = NULL;
    }

    if (pMat) {
        pMat->Destroy();
    }
}

void TyBite::TurnTy(float ang) {
    float scalar = 1.0f / 5.0f;
    
    float diff = NormaliseAngle(ang - ty.mRot.GetUnk4());

    if (diff > PI) {
        diff -= 2.0f * PI;
    }

    ty.diveVec.y = diff * scalar;

    ty.mRot.UnknownInline(ty.mRot.GetUnk4() + ty.diveVec.y);
}

void TyBite::ApplyGravity(void) {
    Vector paddleVect = {
        ty.pos.x,
        ty.pos.y + PaddleDepth,
        ty.pos.z
    };

    if (ty.mContext.water.bValid) {
        if (ty.mContext.GetYDistanceToWater(&paddleVect) > 0.0f) {
            gravity = 0.0f;
            ty.velocity.y = 0.0f;
        }
    }

    if (ty.mContext.floor.bOn) {
        gravity = 0.0f;
    } else {
        gravity -= ty.mContext.jumpGravity / 2.0f;
        ty.velocity.y += gravity;
    }
}

bool TyBite::InAir(Vector* pPos) {
    Vector start = *pPos;
    start.y += 20.0f;

    Vector end = *pPos;
    end.y -= 100.0f;

    return !Collision_RayCollide(&start, &end, NULL, COLLISION_MODE_POLY, 0);
}

void SuperGlowExpand(bool arg0) {
    if (arg0) {
        ty.tyBite.unkF4 = Min<float>(ty.tyBite.unkF4 + 2.0f, 80.0f);
    } else {
        ty.tyBite.unkF4 = Max<float>(ty.tyBite.unkF4 - 2.0f, 0.0f);
    }
}

void TyBite::DrawSuperGlow(void) {
    if (unkF4) {
        View::GetCurrent()->SetLocalToWorldMatrix(NULL);

        if (pMat) {
            pMat->Use();
        }

        Tools_Wobble((float)gb.logicGameCount * 0.1f, 12345);

        Blitter_Particle particle;
        particle.Draw(1);
    }
}

void TyBite::InitBiteCharge(void) {
    ty.StartAnimation(&ty.animScript, ty.unk620, 10, false);
    
    targetDir = ty.velocity;
    speed = targetDir.Normalise();

    unkD0 = 0;
    superBiteCharge = 0;

    Particle_BiteGlow_SetToExpand(false);

    VibrateJoystick(1.0f, 1.0f, 9.0f, 0x40 | 0x2, 3.0f);

    EnemyObject::SetCanAttackHero(false);
}

void TyBite::BiteCharge(void) {
    GameCamera_UseAutoCenter(true, 0.01f);

    unk10[0].SetTranslation(&ty.unk338);
    unk10[1].SetTranslation(&ty.unk39C);
    unk10[2].SetTranslation(&ty.unk3B0);
}

enum CameraBiteEffectState {
    CBES_0 = 0,
    CBES_1 = 1,
    CBES_2 = 2,
    CBES_3 = 3,
};
void GameCamera_UseBiteEffect(CameraBiteEffectState, float, float);

void TyBite::DeinitBiteCharge(void) {
    GameCamera_UseAutoCenter(false, 0.01f);
    
    for (int i = 0; i < 2; i++) {
        ty.lights[i] = gb.level.mLights[i];
        ty.levelDataColours[i] = gb.level.mLightColors[i];
    }

    ty.ambientLight = gb.level.ambientLight;

    MKRumble_Stop(gb.mJoyPad1.GetDeviceID());

    GameCamera_UseBiteEffect(CBES_3, -8.0f, 0.0f);

    if (Sound_IsVoicePlaying(ty.tySounds.unk0) != 0) {
        SoundBank_Stop(&ty.tySounds.unk0);
    }

    SoundBank_Stop(&ty.tySounds.unk4);
    
    EnemyObject::SetCanAttackHero(true);
}

void TyBite::InitBiteUncharge(void) {
    ty.velocity.SetZero();

    Particle_BiteGlow_SetToExpand(true);

    ty.StartAnimation(&ty.animScript, ty.biteUnchargeAnim, 30, false);

    unkD0 = 0;
}

void TyBite::DeinitBiteUncharge(void) {
    unkF4 = 0.0f;
}

void Hud_ShowOpals(void);

void TyBite::InitSuperBite(void) {
    Hud_ShowOpals();

    ty.SetMedium(TY_MEDIUM_4);

    ty.tySounds.unk8 = SoundBank_Play(SFX_TyBiteRelease, NULL, 0);
}

void TyBite::SuperBite(void) {

}

void TyBite::DeinitSuperBite(void) {
    ty.SpeedLinesEnable(NULL);
    speed = 2.0f;
    unkF4 = 0.0f;
}

void TyBite::InitRolyPoly(void) {
    GameCamera_UseCurrentVector(true);

    ty.StartAnimation(&ty.animScript, ty.unk62C, 0, false);

    bCollisionHit = false;

    float animLength = Tools_GetAnimationLength(&ty.animScript);
    float sp8 = 0.0f;

    targetDir.Normalise();

    discreteAcceleration = Kin_GetDiscreteAcceleration(NULL, &speed, &sp8, &animLength);
}

void TyBite::RolyPoly(void) {

}

void TyBite::DeinitRolyPoly(void) {
    GameCamera_UseCurrentVector(false);

    ty.SpeedLinesEnable(NULL);

    unkF4 = 0.0f;
}

void TyBite::InitBite(void) {
    GameCamera_UseCurrentVector(true);

    Particle_BiteGlow_SetToExpand(true);

    ty.SetMedium(TY_MEDIUM_4);

    ty.SpeedLinesEnable(NULL);
    unk145 = false;
}

void TyBite::Bite(void) {

    if (
        (gb.mJoyPad1.mButtonsPressed & tyControl.activeControls[1]) && 
        !(gb.mJoyPad1.mPrevButtonsPressed & tyControl.activeControls[1])
    ) {
        unk145 = true;
    }

    Vector add = {
        0.0f,
        -(float)(unkD0 * unkD0) / 120.0f,
        0.0f
    };

    unk10[0].Translate(&add);
    unk10[1].Translate(&add);
    unk10[2].Translate(&add);

    TurnTy(unk13C);

    SuperGlowExpand(false);

    if (bCollisionHit) {
        ty.Pitch(unk140);
    }

    if (unkD0 == unk148) {
        if (targetedModel) {
            SoundBank_Play(SFX_TyBiteObject, NULL, 0);
        } else {
            SoundBank_Play(SFX_TyBiteObject, NULL, 0);
        }
    }

    if (unkE4 > 1 && unkE4 < 4 && unkD0 < 8.0f) {
        return;
    }

    if (targetedModel) {
        targetPos = *targetedModel->matrices[0].Row3();
        targetPos.y += tyRadius;
    }

    speed += discreteAcceleration;

    Vector tyCentre = ty.pos;
    tyCentre.y += ty.radius;

    Vector toTarget;
    toTarget.Sub(&targetPos, &tyCentre);

    Vector tDir;
    tDir.Normalise(&toTarget);

    tDir.Scale(40.0f);
    
    toTarget.Subtract(&tDir);

    toTarget.Scale(1.0f / targetDist);

    ty.velocity.Scale(&toTarget, speed);

    targetDist -= speed;

    if (ty.animScript.Condition()) {
        fsm.SetState(BS_2, false);
    } else if (!ty.mContext.floor.bOn && gravity < 0.0f) {
        if (ty.mContext.floor.GetDiff(&ty.pos) > 100.0f) {
            ty.velocity.y = 0.0f;
            ty.mFsm.SetState(TY_AS_26, false);
        }
    }
}

void TyBite::DeinitBite(void) {
    GameCamera_UseCurrentVector(false);

    superBiteCharge = 0;
    unkF4 = 0.0f;
}

void TyBite::InitBiteGrowl(void) {
    ty.velocity.SetZero();

    if (unkE4 == 1) {
        ty.StartAnimation(&ty.animScript, ty.unk604, 5, false);
    } else if (unkE4 == 2) {
        ty.StartAnimation(&ty.animScript, ty.unk60C, 5, false);
    } else if (unkE4 == 3) {
        if (bCollisionHit) {
            ty.StartAnimation(&ty.animScript, ty.unk61C, 5, false);
        } else {
            ty.StartAnimation(&ty.animScript, ty.unk614, 5, false);
        }
    } else {
        ty.StartAnimation(&ty.animScript, ty.unk61C, 5, false);
    }
}

void TyBite::DeinitBiteGrowl(void) {
    ty.ResetPitchAndRoll();
}

extern "C" double atan2(double, double);
extern "C" inline float atan2f(float y, float x) {
    return atan2(y, x);
}

void TyBite::InitDiveBite(void) {
    ty.StartAnimation(&ty.animScript, ty.unk650, 0, false);

    Vector tyPos = ty.pos;
    tyPos.y += ty.radius;
    
    tyRadius = 0.0f;

    if (ty.mAutoTarget.unk1E4) {
        targetPos = ty.mAutoTarget.unk1C0;

        targetedModel = ty.mAutoTarget.unk1E4;

        tyRadius = targetPos.y - targetedModel->matrices[0].Row3()->y;
    } else {
        targetPos.Scale(ty.pModel->matrices[0].Row2(), -400.0f);
        targetPos.y -= 400.0f;
        targetPos.Add(&tyPos);
        targetedModel = NULL;
    }

    targetDir.Sub(&targetPos, &tyPos);
    isTargeting = true;
    targetDist = targetDir.Normalise();

    unk13C = atan2f(targetDir.x, -targetDir.z);
    unk140 = atan2f(targetDir.y, sqrtf(Sqr<float>(targetDir.x) + Sqr<float>(targetDir.z)));

    float animLen = Tools_GetAnimationLength(&ty.animScript);

    unk148 = (int)animLen - 5;

    speed = ty.velocity.Dot(&targetDir);

    discreteAcceleration = Kin_GetDiscreteAcceleration(&targetDist, &speed, NULL, &animLen);

    unkD0 = 0;

    tyPosY = ty.pos.y;

    unk145 = false;
}

void TyBite::DeinitDiveBite(void) {

}

void TyBite::InitDiveBiteToFall(void) {
    ty.StartAnimation(&ty.animScript, ty.unk6C0, 5, false);
    ty.ResetPitchAndRoll();
    ty.EnableHead(TYH_Normal);
    ty.velocity.SetZero();
}

void TyBite::DeinitDiveBiteToFall(void) {
    ty.velocity.SetZero();
}

void TyBite::Reset(void) {
    unk13C = unk140 = 0.0f;

    ty.diveVec.SetZero();

    unkD0 = unkE4 = 0;

    gravity = 0.0f;

    superBiteCharge = 0;

    unk155 = false;

    unkF4 = 0.0f;

    bCollisionHit = false;
    unk145 = false;
}

bool TyBite::CheckBitten(Model* pModel) {
    if (targetedModel && pModel == targetedModel && (ty.mFsm.GetState() == AS_Bite || ty.mFsm.GetState() == TY_AS_18)) {
        unk155 = true;
        return true;
    }

    return false;
}

void TyBite::EnableAutoTarget(Model* pModel, bool r5) {
    if (targetedModel && pModel == targetedModel) {
        targetedModel = NULL;
    }
}

bool TyBite::IsTargeting(Model* pModel) {
    return ty.mFsm.GetState() == AS_Bite && pModel == targetedModel && isTargeting;
}
