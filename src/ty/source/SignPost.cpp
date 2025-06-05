#include "ty/SignPost.h"
#include "ty/ParticleEngine.h"
#include "ty/global.h"
#include "ty/GameObjectManager.h"

DetourData detourData;

static SignPostDesc signPostDesc;
static SignPostDesc smallPostDesc;
static SignPostDesc crocSignDesc;
static SignPostDesc detourDesc;
static SignPostDesc constructionDesc;
static SignPostDesc directionDesc;
static SignPostDesc direction2Desc;
static SignPostDesc billboardDesc;

static ModuleInfo<TaSignPost> taSignPostModule;
static ModuleInfo<SignPost> signPostModule;

static bool bIsHidden = false;

static char* buttonSubObject = "a_image_gcn";

void SignPost_LoadResources(KromeIni* pIni) {
    signPostDesc.Init(&signPostModule, "prop_0005_signpost", "SignPost", 1, 0);
    signPostDesc.Load(pIni);
    signPostDesc.type = SP_TYPE_0;
    signPostDesc.bDynamic = true;

    smallPostDesc.Init(&taSignPostModule, "Prop_0393_SignPost", "TASignPost", 1, 0);
    smallPostDesc.Load(pIni);
    smallPostDesc.type = SP_TYPE_1;

    crocSignDesc.Init(&signPostModule, "prop_0172_croc_sign", "CrocSign", 1, 0);
    crocSignDesc.Load(pIni);
    crocSignDesc.type = SP_TYPE_1;
    crocSignDesc.bDynamic = true;

    detourDesc.Init(&signPostModule, "prop_0014_detour_sign", "DetourSign", 1, 0);
    detourDesc.Load(pIni);
    detourDesc.type = SP_DETOUR;
    detourDesc.bDynamic = true;

    constructionDesc.Init(&signPostModule, "prop_0080_construct_sign", "ConstructionSign", 1, 0);
    constructionDesc.Load(pIni);
    constructionDesc.type = SP_CONSTRUCTION;
    constructionDesc.bDynamic = true;

    directionDesc.Init(&signPostModule, "prop_0124_arrow_01", "DirectionArrow", 1, 0);
    directionDesc.Load(pIni);
    directionDesc.type = SP_DIRECTION;
    directionDesc.bDynamic = true;

    direction2Desc.Init(&signPostModule, "prop_0400_arrow02", "DirectionArrow2", 1, 0);
    direction2Desc.Load(pIni);
    direction2Desc.type = SP_DIRECTION2;
    direction2Desc.bDynamic = false;

    billboardDesc.Init(&signPostModule, "prop_0581_billboard", "Billboard", 1, 0);
    billboardDesc.Load(pIni);
    billboardDesc.type = SP_BILLBOARD;
    billboardDesc.bDynamic = false;

    objectManager.AddDescriptor(&signPostDesc);
    objectManager.AddDescriptor(&smallPostDesc);
    objectManager.AddDescriptor(&crocSignDesc);
    objectManager.AddDescriptor(&detourDesc);
    objectManager.AddDescriptor(&constructionDesc);
    objectManager.AddDescriptor(&directionDesc);
    objectManager.AddDescriptor(&direction2Desc);
    objectManager.AddDescriptor(&billboardDesc);

    detourData.anim.Init("prop_0014_detour_sign");
}

void SignPost::Init(GameObjDesc* pDesc) {
    StaticProp::Init(pDesc);

    if (GetDesc()->type == SP_DETOUR) {
        unk84 = RandomI(&gb.mRandSeed) % 120;

        pModel->pAnimation->NodeExists("N_detour_light_01", &light1_index);
        pModel->pAnimation->NodeExists("N_detour_light_02", &light2_index);
    }

    if (GetDesc()->type == SP_TYPE_0) {
        GetDesc()->bUseGroundColor = true;
    } else {
        GetDesc()->bUseGroundColor = false;
    }

    mState = SPS_0;

    Reset();
}

void SignPost::LoadDone(void) {
    mDefaultRot = StaticProp::loadInfo.defaultRot;

    StaticProp::LoadDone();

    mRot = mDefaultRot;

    int refPointIndex = pModel->GetRefPointIndex("R_Center");
    pModel->GetRefPointWorldPosition(refPointIndex, &centrePos);

    pModel->matrices[0].SetRotationPYR(&mRot);

    if (GetDesc()->type == SP_TYPE_0) {
        int index;

        if (pModel->SubObjectExists("a_post", &index)) {
            pModel->EnableOnlySubObject(index, true);
        }

        if (pModel->SubObjectExists(buttonSubObject, &index)) {
            pModel->EnableSubObject(index, true);
        }
    } else if (GetDesc()->type == SP_BILLBOARD) {
        int index;

        if (pModel->SubObjectExists("a_billboard", &index)) {
            pModel->EnableOnlySubObject(index, true);
        }

        if (pModel->SubObjectExists(buttonSubObject, &index)) {
            pModel->EnableSubObject(index, true);
        }
    }
}

void SignPost::Reset(void) {
    GameObject::Reset();

    unk94 = 0;

    SetState(SPS_1);

    collisionInfo.Enable();

    unk95 = 0;

    bIsHidden = false;

    mRot = mDefaultRot;

    pModel->matrices[0].SetRotationPYR(&mRot);
}

void SignPost::Update(void) {
    if (bIsHidden) {
        if (!unk95) {
            collisionInfo.Disable();
            unk95 = true;
        }
    } else {
        if (unk95) {
            collisionInfo.Enable();
            unk95 = false;
        }

        unk8C++;

        GameObject::Update();

        switch (mState) {
            case SPS_1:
                Idle();
                break;
            case SPS_2:
                Hit();
                break;
        }

        if (pHero->IsTy()) {
            ty.mAutoTarget.Set(TP_1, NULL, NULL, &centrePos, pModel);

            if (GetDesc()->type == SP_DIRECTION2 && ty.mAutoTarget.unk1E8 == pModel) {
                float mag = ApproxMag(&ty.unk338, &centrePos);
                Vector look;
                look = *pModel->matrices[0].Row0();

                if (GetDesc()->type == SP_DIRECTION) {
                    look.Scale(-mag);
                } else {
                    look.Scale(mag);
                }

                Vector temp;
                temp.Add(&ty.unk338, &centrePos);
                temp.Scale(0.5f);
                look.Add(&temp);

                Vector toLookPos;
                toLookPos.Sub(&look, &ty.unk338);

                if (toLookPos.Dot(ty.pModel->matrices[0].Row2()) < 0.0f && (ty.unk1688 >= 100 && ty.unk1688 < 200)) {
                    ty.mAutoTarget.unk1F8 = look;
                }
            }
        }
        
    }
}

void SignPost::Draw(void) {
    if (mState == SPS_0 || bIsHidden) {
        return;
    }

    if (GetDesc()->type == SP_DETOUR) {
        Vector v;

        if (unk84 > 60) {
            pModel->pAnimation->GetNodeWorldPosition(light1_index, &v);
            particleManager->SpawnFastGlow(&v, 96.0f);
        } else {
            pModel->pAnimation->GetNodeWorldPosition(light2_index, &v);
            particleManager->SpawnFastGlow(&v, 96.0f);
        }
    }

    StaticProp::Draw();
}

bool Maurie_ComeOverHere(SignPost*);
void Maurie_TakeOff(SignPost*);

void SignPost::Idle(void) {
    switch (GetDesc()->type) {
        case SP_TYPE_1:
            break;
        case SP_TYPE_0:
            if (!unk94) {
                if (pHero->pos.IsInsideSphere(GetPos(), pHero->objectRadiusAdjustment + 1000.0f) && Maurie_ComeOverHere(this)) {
                    unk94 = true;
                }
            } else if (!pHero->pos.IsInsideSphere(GetPos(), pHero->objectRadiusAdjustment + 1000.0f)) {
                Maurie_TakeOff(this);
                unk94 = false;
            }
            break;
        case SP_DETOUR:
            unk84++;
            if (unk84 > 120) {
                unk84 = 0;
            }
            break;
    }
}

void IceBlock_TestCollisionByVolume(Model*, bool);
struct FootEffect {
    char padding[0x10];
    void Init(void);
    void Deinit(void);

    void Spawn(Vector*, Vector*, BoundingVolume*, float, int, float, Vector*, bool);
};
extern FootEffect gbFootEffects;

void SignPost::Hit(void) {
    if (unk8C == 1) {
        IceBlock_TestCollisionByVolume(pModel, true);
        mRotSetting = 1;
        unk88 = 0;

        mRotInc = PI / 32.0f;

        mRot = mDefaultRot;

        Vector tPos = *GetPos();
        tPos.y += 20.0f;

        Vector vel = {0.0f, 20.0f, 0.0f, 0.0f};

        gbFootEffects.Spawn(&tPos, &vel, pModel->GetModelVolume(), 2.0f, 0x10, 1.0f, GetPos(), false);

        SoundBank_Play(0x12F, GetPos(), 0);
    }

    switch (mRotSetting) {
        case 1:
            mRot.x += mRotInc;
            if (mRot.x > mDefaultRot.x + (PI / 32.0f)) {
                mRotSetting = 2;
            }
            break;
        case 2:
            mRot.x -= mRotInc;
            if (mRot.x < mDefaultRot.x - (PI / 32.0f)) {
                mRotSetting = 3;
            }
            break;
        case 3:
            mRot.x += mRotInc;
            if (mRot.x >= mDefaultRot.x) {
                unk88++;

                if (unk88 == 2) {
                    mRotInc = PI / 64.0f;
                }

                if (unk88 == 3) {
                    mRotInc = PI / 96.0f;
                }

                mRotSetting = 1;
            }

            if (unk88 == 4) {
                mRotSetting = 4;
            }
            break;
        case 4:
            SetState(SPS_1);
            mRot = mDefaultRot;
            break;
    }

    pModel->matrices[0].SetRotationPYR(&mRot);
}

void SignPost::SetState(SignPostState newState) {
    if (mState != newState) {
        mState = newState;

        unk8C = 0;
    }
}

void SignPost_HideAll(void) {
    bIsHidden = true;
}

void SignPost_ShowAll(void) {
    bIsHidden = false;
}

void SignPost::CheckForHit(void) {
    CollisionResult collisionResult;
    Boomerang* pRang = Boomerang_CheckForHit(pModel, -1, &collisionResult);

    if (pRang) {
        pRang->unk54 = true;
        SetState(SPS_2);
    }
}

void SignPost::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_BoomerangMsg:
            if (mState == SPS_2) {
                return;
            }

            BoomerangMessage* pRangMsg = (BoomerangMessage*)pMsg;
            if (pRangMsg->pBoomerang == NULL || pRangMsg->pBoomerang->mRangType != BR_Frostyrang) {
                // Don't enter the hit state if hit by the frostyrang
                SetState(SPS_2);
            }
            break;
        default:
            GameObject::Message(pMsg);
            break;
    }
}

void TaSignPost::LoadDone(void) {
    SignPost::LoadDone();

    collisionInfo.Disable();
}

void TaSignPost::Reset(void) {
    SignPost::Reset();

    collisionInfo.Disable();
}

void TaSignPost::Update(void) {

}

void TaSignPost::Draw(void) {
    if (gb.pDialogPlayer) {
        SignPost::Draw();
    }
}
