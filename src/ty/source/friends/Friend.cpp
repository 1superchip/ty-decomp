#include "ty/friends/friend.h"
#include "ty/GameObjectManager.h"
#include "ty/tools.h"
#include "ty/global.h"

extern Material* pShockMaterial;

static float FRIEND_AUTOTARGET_RANGE_SQ = Sqr<float>(2500.0f);

void Aurora_LoadResources(KromeIni*);
void AuroraKid_LoadResources(KromeIni*);
void BunyipElder_LoadResources(KromeIni*);
void Elle_LoadResources(KromeIni*);
void FarLapp_LoadResources(KromeIni*);
void KenOath_LoadResources(KromeIni*);
void Maurie_LoadResources(KromeIni*);
void Rex_LoadResources(KromeIni*);
void Shazza_LoadResources(KromeIni*);
void Julius_LoadResources(KromeIni*);
void Sheila_LoadResources(KromeIni*);
void Lenny_LoadResources(KromeIni*);
void C3_MiniGameActors_LoadResources(KromeIni*);

void Friend_LoadResources(KromeIni* pIni) {
    Aurora_LoadResources(pIni);
    AuroraKid_LoadResources(pIni);
    BunyipElder_LoadResources(pIni);
    Elle_LoadResources(pIni);
    FarLapp_LoadResources(pIni);
    KenOath_LoadResources(pIni);
    Maurie_LoadResources(pIni);
    Rex_LoadResources(pIni);
    Shazza_LoadResources(pIni);
    Julius_LoadResources(pIni);
    Sheila_LoadResources(pIni);
    Lenny_LoadResources(pIni);
    C3_MiniGameActors_LoadResources(pIni);
}

bool Friend::LoadLine(KromeIniLine* pLine) {
    bool tempB;
    if (LoadLevel_LoadBool(pLine, "bActive", &tempB)) {
        mDefFlags = (tempB) ? mDefFlags | FSF_Active :
            mDefFlags & ~FSF_Active;
        return true;
    }
    if (LoadLevel_LoadBool(pLine, "bVisible", &tempB)) {
        mDefFlags = (tempB) ? mDefFlags | FSF_Visible :
            mDefFlags & ~FSF_Visible;
        return true;
    }
    if (LoadLevel_LoadBool(pLine, "bEnabled", &tempB)) {
        mDefFlags = (tempB) ? mDefFlags | FSF_Enabled :
            mDefFlags & ~FSF_Enabled;
        return true;
    }
    return GameObject::LoadLine(pLine) || LoadLevel_LoadVector(pLine, "pos", &mPos) ||
        LoadLevel_LoadVector(pLine, "rot", &mRot) || mPlatformRider.LoadLine(pLine);
}

void Friend::LoadDone(void) {
    mStartPos = mPos;
    mSaveRot = mRot;
    Reset();
    Collision_AddDynamicModel(pModel, &mCollisionInfo, -1);
    objectManager.AddObject(this, pModel);
    if (unkActorIdx > 0) {
        actorInfo[unkActorIdx].pModel = pModel;
    }
    mDefFlags |= FSF_Unknown8;
    mFlags = mDefFlags;
    if (!(mFlags & FSF_Visible)) {
        mCollisionInfo.Disable();
    }
}

/// @brief Draws a Friend object
/// @param  None
void Friend::Draw(void) {
    // If bFlashSkeleton is true, set the default material to the Shock Material
    if (bFlashSkeleton) {
        SetDefaultMaterial_UseNone(pShockMaterial);
    }
    if ((mFlags & FSF_Visible) && !gb.unkFE && gb.pDialogPlayer == NULL) {
        mLodManager.Draw(pModel, detailLevel, unk1C, distSquared, GetDrawFlag());
    }
    // If bFlashSkeleton is true, set the default material to NULL
    if (bFlashSkeleton) {
        SetDefaultMaterial_UseNone(NULL);
    }
}

extern "C" int Sound_IsVoicePlaying(int);
extern int SoundBank_Play(int, Vector*, uint);
enum BoomerangSound {};
struct Boomerang {
    char padding[0x50];
    int rangType;
    void PlaySound(BoomerangSound, int);
};

// BoomerangMessage is a real structure
// Furnace::HandleRangMsg(BoomerangMessage*)
struct BoomerangMessage : MKMessage {
    Boomerang* pBoomerang;
    bool unk8;

    void SetUnk8(bool b) {
        unk8 = b;
    }
};

void Friend::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case 1:
            mPlatformRider.Resolve();
            mPlatformRider.Attach(this);
            break;
        case MKMSG_UpdateAttachment:
            PlatformMoveMsg* pMoveMsg = (PlatformMoveMsg*)pMsg;
            mPos = *pMoveMsg->trans;
            mRot.y += pMoveMsg->rot->y;
            pModel->matrices[0].SetRotationPYR(&mRot);
            pModel->matrices[0].SetTranslation(&mPos);
            break;
        case 8: // BoomerangMessage id
            // BoomerangMessage?
            BoomerangMessage* pBoomerangMsg = (BoomerangMessage*)pMsg;
            if (mFlags & FSF_Visible) {
                if (pBoomerangMsg->pBoomerang->rangType != 3) {
                    pBoomerangMsg->SetUnk8(true);
                }
                if (unk100 == -1 || Sound_IsVoicePlaying(unk100) == 0) {
                    unk100 = SoundBank_Play(GetDesc()->unkD4, NULL, 0);
                }
                if (pBoomerangMsg->pBoomerang->rangType == 8) {
                    // if a Zappyrang hit a Friend, set the flash timer to 40
                    // and have the boomerang play a sound
                    // When a friend is hit by the zappyrang, they will flash on and off
                    // showing a skeleton
                    pBoomerangMsg->pBoomerang->PlaySound((BoomerangSound)3, 0);
                    mFlashTimer = 40;
                }
            }
            break;
        case MKMSG_ACTIVATE:
            mFlags |= FSF_Active;
            break;
        case MKMSG_DEACTIVATE:
            mFlags &= ~FSF_Active;
            break;
        case MKMSG_SHOW:
            mFlags |= FSF_Visible;
            mCollisionInfo.Enable();
            break;
        case MKMSG_HIDE:
            mFlags &= ~FSF_Visible;
            mCollisionInfo.Disable();
            break;
        case MKMSG_ENABLE:
            mFlags |= FSF_Enabled;
            break;
        case MKMSG_DISABLE:
            mFlags &= ~FSF_Enabled;
            break;
        default:
            GameObject::Message(pMsg);
            break;
    }
}

void FriendDescriptor::Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName,
    int _searchMask, int _flags) {
    GameObjDesc::Init(pMod, pMdlName, pDescrName, _searchMask, _flags);
    unkD4 = 0;
}

void FriendDescriptor::Load(KromeIni* pIni) {
    GameObjDesc::Load(pIni);
    mAnimScript.Init(modelName);
    mLodDesc.Init(pIni, modelName);
}

void Friend::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    pModel = Model::Create(GetDesc()->mAnimScript.GetMeshName(),
        GetDesc()->mAnimScript.GetAnimName());
    mLodManager.Init(pModel, 0, &GetDesc()->mLodDesc);
    mCollisionInfo.Init(true, 2, this);
    mPlatformRider.Init();
    mFlags = FSF_DEFAULT_FLAGS;
    mDefFlags = FSF_DEFAULT_FLAGS;
    unkActorIdx = 0;
    mFlashTimer = 0;
}

/// @brief Deinits the object
/// @param  None
void Friend::Deinit(void) {
    mStateManager.Deinit(this);
    if (pModel && pModel->flags.bits.b4) {
        Collision_DeleteDynamicModel(pModel);
    }
    GameObject::Deinit();
}

/// @brief Resets fields
/// @param  None
void Friend::Reset(void) {
    mAnimScript.Init(&GetDesc()->mAnimScript);
    mFlags = mDefFlags;
    unk100 = -1;
    pModel->matrices[0].SetRotationPYR(&mRot);
    pModel->matrices[0].SetTranslation(&mPos);
    CollisionResult cr;
    Vector start = mPos;
    start.y += 10.0f;
    Vector end = mPos;
    end.y -= 1000.0f;
    mFlashTimer = 0;
    bFlashSkeleton = false;
    if (Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, ID_WATER_BLUE)) {
        mPos.y = cr.pos.y;
        pModel->colour = Tools_GroundColor(&cr);
        mCollisionNormal = cr.normal;
    } else {
        mCollisionNormal.Set(0.0f, 1.0f, 0.0f);
    }
    if (mFlags & FSF_Visible) {
        mCollisionInfo.Enable();
    } else {
        mCollisionInfo.Disable();
    }
}

void Friend::Update(void) {
    if (gb.pDialogPlayer || !(mFlags & FSF_Active)) {
        return;
    }
    PreUpdate();
    mStateManager.UnkFunc(this, false);
    PostUpdate();
    // If the flash timer isn't 0, decrement it and
    // set bFlashSkeleton to either true or false randomly
    if (mFlashTimer != 0) {
        mFlashTimer--;
        if (mFlashTimer % 2) {
            if (RandomIR(&gb.mRandSeed, 0, 2) != 0) {
                bFlashSkeleton = true;
            } else {
                bFlashSkeleton = false;
            }
        }
    } else {
        bFlashSkeleton = false;
    }
}

struct TyFSM {
    char padding[0x10];
    int state;
    bool SolidSurfaceState(int);
    int GetState(void) {
        return state;
    }

    // Not sure about this function
    bool ExternalSolidSurfaceState(void) {
        return SolidSurfaceState(GetState());
    }
};

enum TargetPriority {};

struct AutoTargetStruct {
    void Set(TargetPriority, Vector*, Vector*, Vector*, Model*);
};

struct Ty {
    char padding[0x40];
    Vector mPos;
    char padding1[0x9E8];
    TyFSM mFsm;
    char padding2[0xC5C];
    AutoTargetStruct mAutoTargetStruct;

    void SetBounceOffFromPos(Vector*, float, bool);
};
extern Ty ty;

void Friend::PreUpdate(void) {
    if (!(mFlags & FSF_Active)) {
        return;
    }
    mAnimScript.Animate();
    mTyDistSq = mPos.DistSq(&ty.mPos);
    Vector tyDir;
    Vector dot = {0.0f, 1.0f, 0.0f, 0.0f};
    tyDir.Sub(&ty.mPos, &mPos);
    float length = tyDir.Normalise();

    if (!(mFlags & FSF_Visible)) {
        return;
    }
    
    if (ty.mFsm.ExternalSolidSurfaceState()) {
        if (dot.Dot(&tyDir) > 0.7f) {
            tyDir.Scale(length);
            if (Abs<float>(tyDir.y) < mLodManager.pDescriptor->height) {
                tyDir.y = 0.0f;
                if (tyDir.MagSquared() < Sqr<float>(mLodManager.pDescriptor->radius * 1.4f)) {
                    ty.SetBounceOffFromPos(GetPos(), 10.0f, false);
                }
            }
        }
    }
}

void IceBlock_TestCollision(Vector*, float, bool, bool, bool);

void Friend::PostUpdate(void) {
    if (!(mFlags & FSF_Active)) {
        return;
    }
    pModel->matrices[0].SetRotationPYR(&mRot);
    pModel->matrices[0].SetTranslation(&mPos);
    mAnimScript.Apply(pModel->GetAnimation());
    if (mTyDistSq < FRIEND_AUTOTARGET_RANGE_SQ && (mFlags & FSF_Visible)) {
        Vector sp8 = {mPos.x, mPos.y + (GetDesc()->mLodDesc.height / 2.0f), mPos.z};
        ty.mAutoTargetStruct.Set((TargetPriority)3, NULL, NULL, &sp8, pModel);
    }
    IceBlock_TestCollision(GetPos(), mLodManager.pDescriptor->radius, true, false, false);
    if ((mFlags & FSF_Visible) && (mFlags & FSF_Unknown8)) {
        Tools_DropShadow_Add(GetDesc()->pVolume->v2.x, &mPos, &mCollisionNormal, 1.0f);
    }
}
