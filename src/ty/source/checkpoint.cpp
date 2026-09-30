#include "ty/checkpoint.h"
#include "ty/Hero.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"

static MKAnimScript DunnyBAD;
static MKAnim* anims[3];

static bool bCheckpointsLoaded = false;

static LODDescriptor LODInfo;

static GameObjDesc checkpointDesc;
static ModuleInfo<CheckpointStruct> checkpointModule;

static BoundingVolume checkpointBoundVolume;

void Checkpoint_LoadResources(KromeIni* pIni) {
    if (!bCheckpointsLoaded) {
        DunnyBAD.Init("Prop_0018_Thunderbox");

        anims[0] = DunnyBAD.GetAnim("dormant");
        anims[1] = DunnyBAD.GetAnim("appear");
        anims[2] = DunnyBAD.GetAnim("dooropen");

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

    mAnimScript.Init(&DunnyBAD);

    pModel = Model::Create(
        DunnyBAD.GetMeshName(),
        DunnyBAD.GetAnimName()
    );

    pModel->renderType = 3;

    mLodManager.Init(pModel, 0, &LODInfo);

    unk48 = Model::Create("Prop_0092_DunnyRoll", NULL);
    unk48->renderType = 3;

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

}

void CheckpointStruct::Draw(void) {
    if (mState > 0) {
        mAnimScript.Apply(unk4C);
        unk48->Draw(NULL);

        mLodManager.Draw(pModel, detailLevel, unk1C, distSquared, IsInWater());
    }
}

void CheckpointStruct::Reset(void) {
    GameObject::Reset();
    SetState(CPS_1);
}

void CheckpointStruct::Update(void) {
    switch (mState) {
        case CPS_0:
            return;
        case CPS_1:
            Dormant();
            break;
        case CPS_2:
            Activating();
            break;
        case CPS_3:
            return;
        case CPS_4:
            SpringOpen();
            break;
        case CPS_5:
            return;
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
}

void CheckpointStruct::SpringOpen(void) {

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
            mAnimScript.SetAnim(anims[0]);
            break;
        case CPS_2:
            if (pHero->pLastCheckPoint && pHero->pLastCheckPoint != this) {
                pHero->pLastCheckPoint->SetState(CPS_1);
            }

            pHero->pLastCheckPoint = this;

            SoundBank_Play(SFX_CheckPointAppear, NULL, 0);

            mAnimScript.SetAnim(anims[1]);

            if (gb.mGameData.IsDirty()) {
                TyMemCard_AutoSaveGame();
            }
            break;
        case CPS_3:
            break;
        case CPS_4:
            mAnimScript.SetAnim(anims[2]);
            unk7C = 0;
            unk88 = 60;
            mCollisionInfo.Disable();
            if (cameraSource.MagSquared()) {
                GameCamera_SnapSource(&cameraSource);
            }
            break;
    }
}
