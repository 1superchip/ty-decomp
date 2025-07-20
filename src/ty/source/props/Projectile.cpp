#include "ty/props/Projectile.h"
#include "ty/Kinematics.h"
#include "ty/global.h"
#include "ty/GameObjectManager.h"
#include "ty/Ty.h"

ModuleInfo<Projectile> projectileModule;

void Projectile::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    mPosDiff.SetZero();
    mRot.SetZero();
    unk6C = 0;
    unk70 = 0;
    pModel = Model::Create(pDesc->modelName, NULL);
    unk74 = false;
    unk75 = true;
    mState = PROJECTILE_STATE_0;
}

void Projectile::Deinit(void) {
    GameObject::Deinit();
    mState = PROJECTILE_STATE_0;
}

void Projectile::EndLife(void) {
    unk6C = 0;
    unk70 = 0;
    mPosDiff.SetZero();
    mRot.SetZero();
    SetState(PROJECTILE_STATE_0);
}

void Projectile::Message(MKMessage* pMsg) {
    GameObject::Message(pMsg);
}

void Projectile::Update(void) {
    unk6C++;
    
    if (mState != PROJECTILE_STATE_2) {
        return;
    }

    if (unk70 > 1 && gb.pDialogPlayer == NULL) {
        unk70--;
        unk5C = *GetPos();
        AddSpin();
        UpdatePhysics();
        CheckForHit();
    } else {
        EndLife();
    }
}

void Projectile::UpdatePhysics(void) {
    Vector* pPos = GetPos();
    pPos->x += mPosDiff.x;
    pPos->z += mPosDiff.z;
    pPos->y += mPosDiff.y + -GetDesc()->unk98 * 0.5f;
    mPosDiff.y -= GetDesc()->unk98;
}

void Projectile::AddSpin(void) {
    Matrix rotMatrix;
    rotMatrix.SetIdentity();
    rotMatrix.RotatePitch(mRot.x * mRot.w);
    rotMatrix.RotateYaw(mRot.y * mRot.w);
    rotMatrix.RotateRoll(mRot.z * mRot.w);
    mRot.w += 1.0f;
    rotMatrix.CopyTranslation(&pModel->matrices[0]);
    pModel->matrices[0] = rotMatrix;
}

void Projectile::CheckForHit(void) {
    Vector* pPos = pModel->matrices[0].Row3();
    Vector sp18 = ty.pos;
    sp18.y += 50.0f;

    if (!unk74 && sp18.IsInsideSphere(pPos, GetDesc()->unk8C)) {
        unk74 = true;
        SoundBank_Play(GetDesc()->hitSound, pPos, 0);
        ResolveHit();
        if (unk75) {
            DamageTy();
        } else if (!ty.mFsm.KnockBackState() && !ty.mFsm.BiteState()) {
            KnockBackTy();
        }

        return;
    }

    CollisionResult cr;
    if (Collision_RayCollide(&unk5C, pPos, &cr, COLLISION_MODE_ALL, ID_WATER_BLUE)) {
        // if a collision occured, resolve the hit
        if (ResolveHit()) {
            *pPos = unk5C;
            float dot = cr.normal.Dot(&mPosDiff);
            Vector tmp = cr.normal;
            tmp.Scale(cr.normal.Dot(&mPosDiff));
            mPosDiff.Subtract(&tmp);
            tmp.Scale(0.7f);
            mPosDiff.Subtract(&tmp);
            SoundBank_Play(GetDesc()->groundHitSound, pPos, 0);
        }
    }

}

void Projectile::DamageTy(void) {
    Vector Vn = mPosDiff;
    Vn.Normalise();
    ty.Hurt(HURT_TYPE_5, DDA_DAMAGE_1, false, &Vn, 15.0f);
}

void Projectile::KnockBackTy(void) {
    Vector Vn = mPosDiff;
    Vn.Normalise();
    ty.SetKnockBackFromDir(&Vn, 15.0f, KB_TYPE_0);
}

bool Projectile::ResolveHit(void) {
    return true;
}

void Projectile::Fire(Vector* pVec) {
    pModel->matrices[0].SetTranslation(pVec);
    SetState(PROJECTILE_STATE_2);

    unk70 = GetDesc()->unk84;

    mRot.Set(
        RandomFR(&gb.mRandSeed, -0.3f, 0.3f),
        RandomFR(&gb.mRandSeed, -0.2f, 0.2f),
        RandomFR(&gb.mRandSeed, -0.2f, 0.2f)
    );

    mRot.w = 0.0f;
    unk74 = false;
    unk6C = 0;
}

void Projectile::FireAtTarget(Vector* pVec1, Vector* pVec2) {
    CheckShotPossible(pVec1, pVec2);
    Fire(pVec1);
}

bool Projectile::CheckShotPossible(Vector* pVec, Vector* pVec1) {
    float horizDist = sqrtf(Dist2D(pVec, pVec1));
    float sp18;
    float sp14;
    float sp10;
    sp18 = pVec1->y - pVec->y;
    sp14 = horizDist / GetDesc()->unk88;
    float f29 = 1.0f / sp14;
    sp10 = -GetDesc()->unk98;
    mPosDiff.Sub(pVec1, pVec);
    mPosDiff.Scale(f29);
    mPosDiff.y = Kin_GetInitialVelocity(&sp18, NULL, &sp14, &sp10);

    if (mPosDiff.y > GetDesc()->unk88) {
        float f4 = Sqr<float>(GetDesc()->unk88) + ((sp10 * 2.0f) * sp18);
        if (f4 < 0.0f) {
            return false;
        }

        float f30 = sqrtf(f4);

        float f1 = Min<float>(
            (f30 - GetDesc()->unk88) / sp10, 
            (-GetDesc()->unk88 - f30) / sp10
        );

        sp14 = f1;
        f29 = 1.0f / f1;

        if (horizDist * f29 > GetDesc()->unk88 || sp14 < 0.0f) {
            float f1 = Max<float>(
                (f30 - GetDesc()->unk88) / sp10,
                (-GetDesc()->unk88 - f30) / sp10
            );

            sp14 = f1;
            f29 = 1.0f / f1;
            if (horizDist * f29 > GetDesc()->unk88 || sp14 < 0.0f) {
                return false;
            }
        }

        mPosDiff.Sub(pVec1, pVec);
        mPosDiff.Scale(f29);
        mPosDiff.y = GetDesc()->unk88;
    }
    
    return true;
}

void Projectile::SetState(ProjectileState newState) {
    if (mState == PROJECTILE_STATE_0 && newState != PROJECTILE_STATE_0) {
        objectManager.AddObject(this, pModel);
    } else if (mState != PROJECTILE_STATE_0 && newState == PROJECTILE_STATE_0) {
        objectManager.RemoveObject(this);
    }

    mState = newState;
}

void Projectile::SetPos(Vector* pNewPos) {
    pModel->SetPosition(pNewPos);
}
