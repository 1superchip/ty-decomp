#include "ty/checkpoint.h"
#include "ty/Hero.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"
#include "ty/Ty.h"

enum CheckpointAnim {
    CPA_DORMANT     = 0,
    CPA_APPEAR      = 1,
    CPA_DOOROPEN    = 2,
    CPA_COUNT
};

static MKAnimScript DunnyBAD;
static MKAnim* anims[CPA_COUNT];

static bool bCheckpointsLoaded = false;

static LODDescriptor LODInfo;

static GameObjDesc checkpointDesc;
static ModuleInfo<CheckpointStruct> checkpointModule;

static BoundingVolume checkpointBoundVolume;

void Checkpoint_LoadResources(KromeIni* pIni) {
    if (!bCheckpointsLoaded) {
        DunnyBAD.Init("Prop_0018_Thunderbox");

        anims[CPA_DORMANT]  = DunnyBAD.GetAnim("dormant");
        anims[CPA_APPEAR]   = DunnyBAD.GetAnim("appear");
        anims[CPA_DOOROPEN] = DunnyBAD.GetAnim("dooropen");

        LODInfo.Init(pIni, DunnyBAD.GetMeshName());

        checkpointDesc.Init(
            &checkpointModule,
            "Prop_0018_Thunderbox", "Restart",
            1, 0
        );

        checkpointDesc.Load(pIni);

        objectManager.AddDescriptor(&checkpointDesc);

        bCheckpointsLoaded = true;
    }
}

void CheckpointStruct::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);

    pos.SetZero();
    rot.SetZero();

    scale = 1.0f;

    mAnimScript.Init(&DunnyBAD);

    pModel = Model::Create(
        DunnyBAD.GetMeshName(),
        DunnyBAD.GetAnimName()
    );

    pModel->renderType = 3;

    mLodManager.Init(pModel, 0, &LODInfo);

    unk48 = Model::Create("Prop_0092_DunnyRoll", NULL);
    unk48->renderType = 3;

    unk4C = pModel->GetAnimation();

    cameraSource.SetZero();

    mCollisionInfo.Init(true, 2, this);

    Reset();
}

void CheckpointStruct::Deinit(void) {
    if (unk48) {
        unk48->Destroy();
        unk48 = NULL;
    }

    GameObject::Deinit();
}

bool CheckpointStruct::LoadLine(KromeIniLine* pLine) {
    return LoadLevel_LoadVector(pLine, "pos", &pos) ||
        LoadLevel_LoadVector(pLine, "rot", &rot) ||
        LoadLevel_LoadFloat(pLine, "scale", &scale) ||
        LoadLevel_LoadVector(pLine, "cameraSource", &cameraSource) ||
        GameObject::LoadLine(pLine);
}

void CheckpointStruct::LoadDone(void) {
    Vector tempPos = pos;
    tempPos.y += 65.0f;

    pModel->matrices[0].Scale(scale);
    pModel->matrices[0].RotatePYR(&rot);
    pModel->matrices[0].SetTranslation(&tempPos);

    unk48->matrices[0].Scale(scale);
    unk48->matrices[0].RotatePYR(&rot);
    unk48->matrices[0].SetTranslation(&pos);

    objectManager.AddObject(this, &pModel->matrices[0], NULL);

    checkpointBoundVolume.v1.Set(-200.0f, -600.0f, -200.0f, 0.0f);
    checkpointBoundVolume.v2.Set(400.0f, 1200.0f, 400.0f, 0.0f);

    pDescriptor->pVolume = &checkpointBoundVolume;

    CollisionResult cr;
    
    Vector start = {0.0f, 300.0f, 0.0f};
    start.Add(pModel->matrices[0].Row3());
    
    Vector end = {0.0f, -12300.0f, 0.0f};
    end.Add(pModel->matrices[0].Row3());

    if (Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, ID_WATER_BLUE)) {
        pModel->colour = Tools_GroundColor(&cr);
    }

    if (Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, ID_WATER_BLUE)) {
        unk48->colour = Tools_GroundColor(&cr);
    }

    Collision_AddDynamicModel(pModel, &mCollisionInfo, -1);
}

void CheckpointStruct::Draw(void) {
    if (mState > 0) {
        mAnimScript.Apply(unk4C);
        unk48->Draw(NULL);

        mLodManager.Draw(pModel, detailLevel, unk1C, distSquared, IsInWater());
    }
}

void CheckpointStruct::Reset(void) {
    mCollisionInfo.Enable();
    GameObject::Reset();
    SetState(CPS_1);
}

void CheckpointStruct::Update(void) {
    float oldRotY = rot.y;
    float oldScale = scale;

    switch (mState) {
        case CPS_1:
            Dormant();
            break;
        case CPS_2:
            Activating();
            break;
        case CPS_4:
            SpringOpen();
            break;
        default:
            return;
    }

    if (oldScale != scale || oldRotY != rot.y) {
        pos = *pModel->matrices[0].Row3();

        pModel->matrices[0].SetIdentity();
        pModel->matrices[0].Scale(scale);
        pModel->matrices[0].RotatePYR(&rot);
        pModel->matrices[0].SetTranslation(&pos);

        pModel->SetLocalToWorldDirty();
    }

    Vector lookPos = {pModel->matrices[0].Row3()->x, pModel->matrices[0].Row3()->y + 80.0f, pModel->matrices[0].Row3()->z};

    if (pHero->IsTy()) {
        ty.mAutoTarget.Set(TP_1, NULL, NULL, &lookPos, pModel);
    }
}

void CheckpointStruct::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Activate:
            if (mState == CPS_1) {
                SetState(CPS_2);
            }
            break;
        default:
            GameObject::Message(pMsg);
            break;
    }
}

void CheckpointStruct::Dormant(void) {
    mAnimScript.Animate();

    if (GetPos()->IsInsideSphere(&pHero->pos, 500.0f)) {
        SetState(CPS_2);
    }
}

void CheckpointStruct::Activating(void) {
    mAnimScript.Animate();

    if (mAnimScript.Condition()) {
        if (GetPos()->IsInsideSphere(&pHero->pos, 200.0f)) {
            return;
        }

        SetState(CPS_3);
    }
}

void CheckpointStruct::SpringOpen(void) {
    switch (unk7C) {
        case 0:
            unk88--;
            if (unk88 <= 0) {
                unk7C = 1;
            }
            break;
        case 1:
            mAnimScript.Animate();
            if (mAnimScript.Condition()) {
                if (GetPos()->IsInsideSphere(&pHero->pos, 200.0f)) {
                    return;
                }

                SetState(CPS_3);
                mCollisionInfo.Enable();
            }
            break;
    }
}

void GameCamera_SnapSource(Vector*);
void TyMemCard_AutoSaveGame(void);

void CheckpointStruct::SetState(CheckpointState nState) {
    if (mState == nState) {
        return;
    }

    mState = nState;

    switch (mState) {
        case CPS_1:
            mAnimScript.SetAnim(anims[CPA_DORMANT]);
            break;
        case CPS_2:
            if (pHero->pLastCheckPoint && pHero->pLastCheckPoint != this) {
                pHero->pLastCheckPoint->SetState(CPS_1);
            }

            pHero->pLastCheckPoint = this;

            SoundBank_Play(SFX_CheckPointAppear, NULL, 0);

            mAnimScript.SetAnim(anims[CPA_APPEAR]);

            if (gb.mGameData.IsDirty()) {
                TyMemCard_AutoSaveGame();
            }
            break;
        case CPS_3:
            break;
        case CPS_4:
            mAnimScript.SetAnim(anims[CPA_DOOROPEN]);
            unk7C = 0;
            unk88 = 60;
            mCollisionInfo.Disable();
            if (cameraSource.MagSquared()) {
                GameCamera_SnapSource(&cameraSource);
            }
            break;
    }
}
