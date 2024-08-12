#include "ty/bunyip.h"
#include "ty/global.h"
#include "ty/GameObjectManager.h"

static AnimatingPropDesc bunyipDesc;
static ModuleInfo<Bunyip> bunyipModule;

static bool bIsLastEventGroundHit = false;

static Tools_AnimEntry animEntries[11] = {
    {"breathe", NULL},
    {"idle01a", NULL},
    {"idle01b", NULL},
    {"idle02", NULL},
    {"walk01", NULL},
    {"run01", NULL},
    {"roar01", NULL},
    {"hit01", NULL},
    {"hit02", NULL},
    {"hit03", NULL},
    {"appear", NULL},
};

static Tools_AnimEvent animEvents[4] = {
    {"hit", NULL},
    {"stun", NULL},
    {"thud", NULL},
    {"footThud", NULL},
};

#pragma pool_data off
void Bunyip_LoadResources(KromeIni* pIni) {

    bunyipDesc.Init(
        &bunyipModule, 
        "act_63_bunyip", "Bunyip",
        1, 1,
        animEntries, 11,
        animEvents, 4,
        false, 
        GameObjFlags_All
    );

    bunyipDesc.Load(pIni);

    objectManager.AddDescriptor(&bunyipDesc);
}
#pragma pool_data reset

void Bunyip::Init(GameObjDesc* pDesc) {
    AnimatingProp::Init(pDesc);
    pModel->renderType = 2;
    mSoundHelper.Init();
    actorInfo[ACTOR_BUNYIP_ID].pModel = pModel;
}

void Bunyip::LoadDone(void) {
    AnimatingProp::LoadDone();
    mDefaultPos = *GetPos();
    Reset();
}

void Bunyip::Reset(void) {
    AnimatingProp::Reset();
    *GetPos() = mDefaultPos;
    mRot.SetZero();
    mScale = 1.0f;
    unkD9 = false;
    unkDA = false;
    mState = (BunyipState)0;
    bunyipLifeSpan = 0;
    mSoundHelper.Reset();
}

void Bunyip::Update(void) {

    mSoundHelper.Update(
        0x95, false, StateInline() && gb.pDialogPlayer == NULL, 
        NULL, NULL, -1.0f, 0
    );

    if (mState == (BunyipState)0 || gb.pDialogPlayer) {
        return;
    }

    mAnimScript.Animate();
    mAnimScript.Apply(pModel->pAnimation);
    mAnimEventManager.Update(&mAnimScript);

    if (unkC4 != 0) {
        unkC4--;
    }

    if (bunyipLifeSpan != 0) {
        bunyipLifeSpan--;
    } else {
        SetState((BunyipState)8);
    }

    switch (mState) {
        case BUNYIP_APPEAR:
            Appear();
            break;
        case BUNYIP_IDLE:
            Idle();
            break;
        case BUNYIP_ROAR:
            Roar();
            break;
        case BUNYIP_PUNCH:
            PunchCombo();
            break;
        case 8:
            Disappear();
            break;
    }
    
    CheckEvents();
}

void Bunyip::Draw(void) {
    if (mState == 0 || gb.pDialogPlayer) {
        return;
    }

    AnimatingProp::Draw();
}

void Bunyip::Message(MKMessage* pMsg) {
    mSoundHelper.Message(pMsg);
    AnimatingProp::Message(pMsg);
}

void Bunyip::Appear(void) {
    UpdatePos(GetPos(), &mRot);

    mRot.y += 0.02f;
    if (mRot.y > 2 * PI) {
        mRot.y -= 2 * PI;
    }

    if (mAnimScript.Condition()) {
        SetState(BUNYIP_IDLE);
    }
}

void Bunyip::Idle(void) {
    if (mAnimScript.Condition()) {
        // Set a random animation
        switch (RandomI(&gb.mRandSeed) % 50) {
            case 0:
            case 1:
            case 2:
            case 3:
                mAnimScript.SetAnim(mAnimManager.pDescriptor->pEntries[1].pAnim);
                break;
            case 4:
            case 5:
            case 6:
            case 7:
                mAnimScript.SetAnim(mAnimManager.pDescriptor->pEntries[2].pAnim);
                break;
            case 8:
                mAnimScript.SetAnim(mAnimManager.pDescriptor->pEntries[3].pAnim);
                break;
            default:
                mAnimScript.SetAnim(mAnimManager.pDescriptor->pEntries[0].pAnim);
                break;
        }
    }
}

void Bunyip::Roar(void) {
    if (mAnimScript.Condition()) {
        unkD9 = true; // roar flag?
        SetState(BUNYIP_IDLE);
    }
}

void Bunyip::PunchCombo(void) {
    pModel->pAnimation->GetNodeWorldPosition(mHandIndex, &unkB0);
    if (mAnimScript.Condition()) {
        unkD9 = true;
        SetState(BUNYIP_IDLE);
    }
}

extern "C" void Sound_SetVolume(int, char, char);

void Bunyip::Disappear(void) {
    Sound_SetVolume(mSoundHelper.unk0, mScale * 255.0f, mScale * 255.0f);
    mScale -= (1.0f / gDisplay.displayFreq);
    if (mScale < 0.0f) {
        mScale = 0.0f;
        unkD9 = true;
        SetState((BunyipState)0);
    }
    UpdatePos(GetPos(), &mRot);
}

void SoundBank_PauseMusic(bool, float);
void SoundBank_Play(int, Vector*, uint);

void Bunyip::SetState(BunyipState newState) {
    if (mState == newState) {
        return;
    }
    if ((mState == (BunyipState)6 || mState == (BunyipState)5) && !unkD9) {
        return;
    }

    unkD9 = false;
    mState = newState;

    switch (mState) {
        case 0:
            mSoundHelper.Stop();
            break;
        case BUNYIP_APPEAR:
            mScale = 1.0f;
            GetPos()->y += 75.0f;
            mAnimScript.SetAnim(mAnimManager.GetAnim(10));
            unkD9 = true;
            bunyipLifeSpan = gDisplay.displayFreq * (float)gb.mDataVal.bunyipLifeSpan;
            SoundBank_PauseMusic(true, 0.5f);
            SoundBank_Play(0x91, NULL, 0);
            break;
        case BUNYIP_IDLE:
            mScale = 1.0f;
            mAnimScript.TweenAnim(mAnimManager.GetAnim(0), 5);
            break;
        case 3:
            mAnimScript.TweenAnim(mAnimManager.GetAnim(4), 5);
            break;
        case 4:
            if (mAnimScript.currAnim != mAnimManager.GetAnim(5)) {
                mAnimScript.TweenAnim(mAnimManager.GetAnim(5), 5);
            }
            break;
        case BUNYIP_ROAR:
            mAnimScript.TweenAnim(mAnimManager.GetAnim(6), 5);
            break;
        case BUNYIP_PUNCH:
            if (unkC4 > 0) {
                unkDB++;
                if (unkDB > 2) {
                    unkDB = 0;
                }
            } else {
                unkDB = 0;
            }
            
            unkC4 = 120;
            mAnimScript.SetAnim(mAnimManager.GetAnim(unkDB + 7));
            bIsLastEventGroundHit = unkDB == 0;

            if (unkDB == 2) {
                mHandIndex = pModel->pAnimation->GetNodeIndex("Z_Hand_L");
            } else {
                mHandIndex = pModel->pAnimation->GetNodeIndex("Z_Hand_R");
            }

            pModel->pAnimation->GetNodeWorldPosition(mHandIndex, &unkB0);
            break;
        case 7:
            mAnimScript.SetAnim(mAnimManager.GetAnim(5));
            break;
        case 8:
            SoundBank_PauseMusic(false, 1.0f);
            mAnimScript.TweenAnim(mAnimManager.GetAnim(0), 5);
            break;
    }
}

void Bunyip::UpdatePos(Vector* pPos, Vector* pRot) {
    mRot = *pRot;
    pModel->matrices[0].SetRotationPYR(&mRot);
    pModel->matrices[0].Scale(mScale);
    pModel->SetInverseScaleValue(-1, 1.0f / mScale);
    *GetPos() = *pPos;
}

void GameCamera_Shake(Vector*);

void Bunyip::CheckEvents(void) {
    unkE4 = 5;

    for(int i = 0; mAnimScript.GetEvent(i++) != NULL;) {
        if (mAnimEventManager.CheckUnk4Flags(1)) {
            Vector shake = {0.0f, 15.0f, 0.0f, 0.0f};
            unkE4 = 0;
            GameCamera_Shake(&shake);
            VibrateJoystick(0.6f, 0.6f, 0.17f, 0, 4.0f);
            if (mAnimScript.currAnim != mAnimManager.pDescriptor->pEntries[9].pAnim) {
                SoundBank_Play(0x94, NULL, 0);
            }
            unkDA = !unkDA;
        } else if (mAnimEventManager.CheckUnk4Flags(2)) {
            SoundBank_Play(0x21c, NULL, 0);
            unkE4 = 1;
        } else if (mAnimEventManager.CheckUnk4Flags(4)) {
            SoundBank_Play(0x92, NULL, 0);
            VibrateJoystick(0.4f, 0.0f, 0.17f, 0, 4.0f);
        } else if (mAnimEventManager.CheckUnk4Flags(8)) {
            SoundBank_Play(0x93, NULL, 0);
            VibrateJoystick(0.4f, 0.0f, 0.17f, 0, 4.0f);
        }
    }
}

bool Bunyip::HasPunched(Vector* pPoint, float radius) {
    if (mState == (BunyipState)6 && unkDA && pPoint->IsInsideSphere(&unkB0, radius + 300.0f)) {
        unkD8 = true;
        return true;
    }
    return false;
}

bool Bunyip_On(void) {
    DescriptorIterator it = bunyipDesc.Begin();
    while (it.GetPointers()) {
        Bunyip* pBunyip = static_cast<Bunyip*>(it.GetPointers());

        if (pBunyip->StateInline()) {
            return true;
        }

        it.UpdatePointers();
    }
    return false;
}

struct Hero {
    char padding[0xE0];
    int heroType;

    bool IsTy(void) {
        return heroType == 0;
    }
};

extern Hero* pHero;

struct Ty {
    char padding[0x30];
    void SetBunyip(Bunyip*);
};
extern Ty ty;

bool Bunyip_Activate(void) {
    DescriptorIterator it = bunyipDesc.Begin();
    if (it.GetPointers()) {
        if (static_cast<Bunyip*>(it.GetPointers())->mState == 0 && pHero->IsTy()) {
            static_cast<Bunyip*>(it.GetPointers())->SetState(BUNYIP_APPEAR);

            ty.SetBunyip(static_cast<Bunyip*>(it.GetPointers()));
            return true;
        }

    }
    return false;
}

int Bunyip_GetLastEvent(void) {
    DescriptorIterator it = bunyipDesc.Begin();
    while (it.GetPointers()) {

        if (static_cast<Bunyip*>(it.GetPointers())->StateInline()) {
            return static_cast<Bunyip*>(it.GetPointers())->unkE4;
        }

        it.UpdatePointers();
    }
    return 5;
}

bool Bunyip_IsLastEventGroundHit(void) {
    DescriptorIterator it = bunyipDesc.Begin();
    while (it.GetPointers()) {

        if (static_cast<Bunyip*>(it.GetPointers())->StateInline()) {
            return static_cast<Bunyip*>(it.GetPointers())->unkE4 == 0;
        }

        it.UpdatePointers();
    }
    return false;
}
