#include "ty/FinishLine.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"
#include "ty/Hero.h"
#include "ty/GuideParticle.h"

void TimeTrial_StopTimer(void);

static MKAnimScript finishLineBAD;

static MKAnim* anims[3];

static GameObjDesc finishLineDesc;

static ModuleInfo<FinishLineStruct> finishLineModule;

void FinishLine_Show(void) {
    if (finishLineDesc.instanceCount > 0) {
        ((FinishLineStruct*)finishLineDesc.pInstances)->SetState(FLS_1);
    }
}

void FinishLine_Hide(void) {
    if (finishLineDesc.instanceCount > 0) {
        ((FinishLineStruct*)finishLineDesc.pInstances)->SetState(FLS_0);
    }
}

bool FinishLine_IsCrossedBy(Vector* pPos) {
    if (finishLineDesc.instanceCount > 0) {
        return ((FinishLineStruct*)finishLineDesc.pInstances)->IsCrossedBy(pPos);
    }

    return false;
}

void FinishLine_LoadResources(KromeIni* pIni) {
    finishLineBAD.Init("Prop_0088_FinishLine");

    anims[0] = finishLineBAD.GetAnim("Flappity01");
    anims[1] = finishLineBAD.GetAnim("Flappity02");
    anims[2] = finishLineBAD.GetAnim("Flappity03");

    finishLineDesc.Init(
        &finishLineModule,
        finishLineBAD.GetMeshName(),
        "FinishLine",
        1,
        0
    );

    objectManager.AddDescriptor(&finishLineDesc);
}

void FinishLineStruct::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);

    bOn = true;

    pModel = Model::Create(finishLineBAD.GetMeshName(), finishLineBAD.GetAnimName());

    pModel->renderType = 3;

    pModel->matrices[0].SetIdentity();

    mtx.SetIdentity();

    animScript.Init(&finishLineBAD);

    animScript.SetAnim(anims[0]);

    Reset();
}

void FinishLineStruct::Deinit(void) {
    bOn = false;
    GameObject::Deinit();
}

bool FinishLineStruct::LoadLine(KromeIniLine* pLine) {
    Vector temp;
    if (LoadLevel_LoadVector(pLine, "pos", &temp)) {
        pModel->SetPosition(&temp);
        return true;
    }

    if (LoadLevel_LoadVector(pLine, "rot", &temp)) {
        pModel->SetRotation(&temp);
        return true;
    }

    if (LoadLevel_LoadVector(pLine, "scale", &scale)) {
        return true;
    }

    return GameObject::LoadLine(pLine);
}

void FinishLineStruct::LoadDone(void) {
    objectManager.AddObject(this, pModel);

    mtx = pModel->matrices[0];

    mtx.Row3()->y -= 50.0f;

    Vector invScale = {1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z, 0.0f};

    mtx.Scale(&invScale);
    mtx.InverseSimple();
    pModel->matrices[0].Scale(&scale);
}

void FinishLineStruct::Update(void) {
    switch (state) {
        case FLS_1:
            Idle();
            break;
        case FLS_2:
            PostFinish();
            break;
        default:
            return;
    }

    animScript.Animate();

    if (animScript.HasLooped()) {
        animScript.SetAnim(anims[RandomI(&gb.mRandSeed) % (int)ARRAY_SIZE(anims)]);
    }
}

void FinishLineStruct::Draw(void) {
    if (bOn && state != FLS_0) {
        animScript.Apply(pModel->pAnimation);
        pModel->Draw(NULL);
    }
}

void FinishLineStruct::Reset(void) {
    GameObject::Reset();
    
    unkBC = true;
    SetState(FLS_0);
}

void FinishLineStruct::Idle(void) {
    if (unkBC) {
        Vector pos2 = pHero->velocity;
        pos2.Scale(-0.5f);

        pos2.Add(&pHero->pos);

        if (GuideParticle_HasPassedThoughAll() && (IsCrossedBy(&pHero->pos) || IsCrossedBy(&pos2))) {
            TimeTrial_StopTimer();
            SetState(FLS_2);
        }

    }
}

void FinishLineStruct::PostFinish(void) {

}

void FinishLineStruct::SetState(FinishLineState newState) {
    if (state != newState) {
        state = newState;
    }
}

bool FinishLineStruct::IsCrossedBy(Vector* pPos) {
    Vector locPos;

    locPos.ApplyMatrix(pPos, &mtx);

    return pModel->GetBoundingVolume(-1)->IsWithin(&locPos);
}

void FinishLineStruct::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Activate:
            unkBC = true;
            break;
        case MSG_Deactivate:
            unkBC = false;
            break;
    }

    GameObject::Message(pMsg);
}
