#include "ty/props/Platform.h"
#include "common/Str.h"
#include "common/Heap.h"
#include "common/StdMath.h"
#include "ty/GameObjectManager.h"

extern struct Ty {
    char padding[0x40];
    Vector pos;
    char padding2[0xD8 - 0x50];
    float radius;
    char unk[0x7F4];
    bool unk844;
    bool unk845;
    bool unk846;
    float unk848;
    float unk84C;
    char unk1[0x34];
    int unk884;
    
    void SetAbsolutePosition(Vector*, int, float, bool);
} ty;

//extern void LoadLevel_LoadInt(KromeIniLine*, char*, int*);

static ModuleInfo<Platform> platformModuleInfo;
static void* tempMem;
static void* pCurrMem = tempMem;

void Create_UpdateAttachMessage(PlatformMoveMsg* pMsg, Vector* trans, Vector* rot, Vector* def, Matrix* mat) {
    pMsg->unk0 = MKMSG_UpdateAttachment;
    pMsg->trans = trans;
    pMsg->rot = rot;
    pMsg->vec = def;
    pMsg->mat = mat;
}

PlatformUpdateInfo* TempAlloc(int size) {
    PlatformUpdateInfo* m = (PlatformUpdateInfo*)pCurrMem;
    pCurrMem = (void*)((char*)pCurrMem + size);
    return m;
}

void TempFree(void* mem) {
    pCurrMem = mem;
}

void Platform_LoadResources(KromeIni* pIni) {
    pCurrMem = tempMem = Heap_MemAlloc(0x1000);
    
    PlatformDesc defaultPlatformDesc;
    defaultPlatformDesc.Init(&platformModuleInfo, "", "", GOID_Platform, 1);
    LoadDescriptors<PlatformDesc>(pIni, "Platforms", &defaultPlatformDesc);
}

void PlatformDesc::Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags) {
    StaticPropDescriptor::Init(pMod, pMdlName, pDescrName, _searchMask | GOID_Platform, _flags);
    maxTilt = 0.0f;
    rotVel = 0.0f;
    maxShadowHeight = 1000.0f;
}

void PlatformDesc::Load(KromeIni* pIni) {
    StaticPropDescriptor::Load(pIni);
    KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);
    
    if (pLine != NULL) {
        lodDesc.Init(pIni, modelName);
        while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
            if (pLine->pFieldName != NULL) {
                gAssertBool = LoadLevel_LoadFloat(pLine, "maxTilt", &maxTilt) ||
                    LoadLevel_LoadFloat(pLine, "rotVel", &rotVel) ||
                    LoadLevel_LoadFloat(pLine, "maxShadowHeight", &maxShadowHeight);
            }
            pLine = pIni->GetLineWithLine(pLine);
        }
    }

    bDynamic = true;
    maxMag = 150.0f;
    radius = 20.0f;
}

void Platform::Init(GameObjDesc* pDesc) {
    StaticProp::Init(pDesc);
    GetDesc()->maxMag = pModel->GetModelVolume()->v1.Magnitude();
    numAttached = 0;
    mCurrRot.Set(0.0f, 0.0f, 0.0f);
    attachments.Init(32);
    rider.Init();

    unkB0.SetZero();
    unkC0.SetZero();
    unkA0.SetZero();

    unkD0 = false;

    if (GetDesc()->maxShadowHeight > 0.0f) {
        UpdateShadow();
    }

    unk58 = NULL;
}

void Platform::Deinit(void) {
    pModel->Destroy();
    pModel = NULL;
    attachments.Deinit();
    StaticProp::Deinit();
}

bool Platform::LoadLine(KromeIniLine* pLine) {
    return rider.LoadLine(pLine) || StaticProp::LoadLine(pLine);
}

void Platform::LoadDone(void) {
    scale = StaticProp::loadInfo.defaultScale;
    mDefaultRot = StaticProp::loadInfo.defaultRot;
    mCurrRot = mDefaultRot;
    StaticProp::LoadDone();
}

void Platform::Update(void) {
    BeginUpdate();
    UpdateTilt();
    EndUpdate();
}

void Platform::Reset(void) {
    // Reset Platform rotation
    mCurrRot = mDefaultRot;
    pModel->matrices[0].SetRotationPYR(&mCurrRot);
    unk58 = NULL;
}

void Platform::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case 1:
            rider.Resolve();
            rider.Attach(this);
            break;
        case MKMSG_UpdateAttachment:
            BeginUpdate();
            PlatformMoveMsg* updateMsg = (PlatformMoveMsg*)pMsg;
            GetPos()->Copy(updateMsg->trans);
            mCurrRot.Add(updateMsg->rot);
            EndUpdate();
            break;
        case MKMSG_AttachObject:
            Attach(reinterpret_cast<MKMessage_GOBJ*>(pMsg)->pObj);
            break;
        case MKMSG_DetachObject:
            Detach(reinterpret_cast<MKMessage_GOBJ*>(pMsg)->pObj);
            break;
    }
    GameObject::Message(pMsg);
}

void Platform::BeginUpdate(void) {
    Matrix mat;

    if (numAttached == 0) {
        return;
    }

    unk58 = TempAlloc(numAttached * sizeof(Vector) + sizeof(PlatformUpdateInfo));
    mat.Inverse(&pModel->matrices[0]);
    unk58->unk0 = mat;
    unk58->currRot = mCurrRot;
    unk58->unk50.Set(0.0f, 0.0f, 0.0f, 1.0f);

    int idx = 0;
    GameObject** iter = attachments.GetPointers();
    while (*iter != NULL) {
        unk58->unk60Vecs[idx].ApplyMatrix((*iter)->GetPos(), &mat);
        unk58->unk50.Add(&unk58->unk60Vecs[idx]);
        idx++;
        iter++;
    }

    if (unk58 != NULL) {
        unk58->unk50.Scale(1.0f / numAttached);
    }
}

void Platform::EndUpdate(void) {
    UpdateRotationMatrix();
    UpdateAttached();

    if (GetDesc()->maxShadowHeight > 0.0f) {
        UpdateShadow();
    }

    if (unk58 != NULL) {
        TempFree((void*)unk58);
    }

    unk58 = NULL;
}

void Platform::UpdateTilt(void) {
    if (GetDesc()->maxTilt > 0.0f) {
        if (unk58 != NULL) {
            if (unk58->unk50.MagSquared() > Sqr<float>(GetDesc()->radius)) {
                Vector lPos = unk58->unk50;
                lPos.y = 0.0f;
                lPos.ClampMagnitude(GetDesc()->maxMag);
                lPos.Scale(GetDesc()->maxTilt / GetDesc()->maxMag);
                mCurrRot.x = AdjustFloat(mCurrRot.x, -lPos.z, 0.1f);
                mCurrRot.z = AdjustFloat(mCurrRot.z, lPos.x, 0.1f);
                return;
            }
        }

        mCurrRot.x = AdjustFloat(mCurrRot.x, 0.0f, 0.1f);
        mCurrRot.z = AdjustFloat(mCurrRot.z, 0.0f, 0.1f);
    }
}

void Platform::UpdateRotationMatrix(void) {
    Matrix tmpMat;
    pModel->matrices[0].SetRotationPitch(mCurrRot.x);
    tmpMat.SetRotationRoll(mCurrRot.z);
    pModel->matrices[0].Multiply3x3(&tmpMat);
    tmpMat.SetRotationYaw(mCurrRot.y);
    pModel->matrices[0].Multiply3x3(&tmpMat);
    pModel->matrices[0].Scale(&scale);
}

void Platform::UpdateAttached(void) {
    PlatformMoveMsg msg;
    Vector deltaPyr;
    if (unk58 != NULL) {
        deltaPyr.Sub(&mCurrRot, &unk58->currRot);
        GameObject** iter = attachments.GetPointers();
        int idx = 0;
        // iterate through attached objects and send the Move Message to them
        while (*iter != NULL) {
            Vector* pVec = &unk58->unk60Vecs[idx];
            pVec->ApplyMatrix(&pModel->matrices[0]);
            Create_UpdateAttachMessage(&msg, &unk58->unk60Vecs[idx], &deltaPyr, (Vector*)unk58, &pModel->matrices[0]);
            (*iter)->Message(&msg);
            idx++;
            iter++;
        }
    }
}

void Platform::UpdateShadow(void) {
    Vector start = {
        pModel->matrices[0].Row3()->x, 
        pModel->matrices[0].Row3()->y - 10.0f,
        pModel->matrices[0].Row3()->z, 
        0.0f
    };

    Vector end = {
        start.x, 
        start.y - GetDesc()->maxShadowHeight,
        start.z, 
        0.0f
    };

    if (CompareVectors(&start, &unkA0) == false) {
        unkD0 = false;
        CollisionResult cr;
        if (Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, 0)) {
            unkC0 = cr.normal;
            unkB0 = cr.pos;
            unkA0 = start;
            unkD0 = true;
            if (GetDesc()->bUseGroundColor) {
                pModel->colour = Tools_GroundColor(&cr);
            }
        }
    }
    
    if (unkD0) {
        float mag = 2.0f * GetDesc()->maxMag;
        float div = 1.0f - (0.6f * Clamp<float>(0.0f, (pModel->matrices[0].data[3][1] - unkB0.y) / 1000.0f, 1.0f));
        Tools_DropShadow_Add(mag, &unkB0, &unkC0, unk1C * div);
    }
}

// Add a GameObject to a platform's attachments
// Executes when an object connects with a platform whether when standing or grabbing with boomerangs
void Platform::Attach(GameObject* pObj) {
    attachments.AddEntry(pObj);
    numAttached++;
}

// Remove a GameObject from a platform's attachments
void Platform::Detach(GameObject* pObj) {
    attachments.Destroy(pObj);
    numAttached--;
}

void Platform::PushTy(int subObjectIndex) {
    float tyRadius = ty.radius;

    Vector clippedPos;
    Vector testPos = {ty.pos.x, ty.pos.y + ty.radius, ty.pos.z, ty.pos.w};

    if (Tools_ClipSphereToDynamicModel(testPos, tyRadius, &clippedPos, pModel, subObjectIndex)) {
        clippedPos.y -= tyRadius;
        if (clippedPos.y > ty.pos.y) {
            clippedPos.y = ty.pos.y;
        }

        ty.SetAbsolutePosition(&clippedPos, 25, 1.0f, true);
    }
}

void PlatformRider::Init(void) {
    platformID = -1;
    pObject = NULL;
    unk8 = false;
}

bool PlatformRider::LoadLine(KromeIniLine* pLine) {
    return LoadLevel_LoadInt(pLine, "platformID", &platformID);
}

void PlatformRider::Resolve(void) {
    if (platformID > 0) {
        pObject = objectManager.GetObjectFromID(platformID);
    }
}

void PlatformRider::Attach(GameObject* pObj) {
    if (pObject != NULL && !unk8) {
        reinterpret_cast<Platform*>(pObject)->Attach(pObj);
        unk8 = true;
    }
}

void PlatformRider::Detach(GameObject* pObj) {
    if (pObject != NULL && unk8) {
        reinterpret_cast<Platform*>(pObject)->Detach(pObj);
        unk8 = false;
    }
}

void PlatformRider::UpdateDynamicAttach(CollisionResult* pCr, GameObject* pObj) {
    CollisionInfo* pInfo = pCr->pInfo;
    if (pInfo == NULL) {
        return;
    }
    if (pInfo->pProp && (pInfo->pProp->pDescriptor->searchMask & GOID_Platform) &&
            pInfo->pProp != pObject) {
        // if (pObject && unk8) {
        //     reinterpret_cast<Platform*>(pObject)->Detach(pObj);
        //     unk8 = false;
        // }
        Detach(pObj);
        pObject = (GameObject*)pInfo->pProp;
        Attach(pObj);
        // pObject = reinterpret_cast<GameObject*>(pInfo->pProp);
        // if (pObject != NULL && !unk8) {
        //     reinterpret_cast<Platform*>(pObject)->Attach(pObj);
        //     unk8 = true;
        // }
    }
}

void PlatformRider::ToLocal(Vector* pVec) {
    if (pObject != NULL && unk8) {
        Matrix invLtw;
        invLtw.InverseSimple(pObject->pLocalToWorld);
        pVec->ApplyMatrix(pVec, &invLtw);
    }
}

void PlatformRider::ToWorld(Vector* pVec) {
    if (pObject != NULL && unk8) {
        pVec->ApplyMatrix(pVec, pObject->pLocalToWorld);
    }
}

void PlatformRider::ToLocalDir(Vector* pVec) {
    if (pObject != NULL && unk8) {
        Matrix invLtw;
        invLtw.InverseSimple(pObject->pLocalToWorld);
        pVec->ApplyRotMatrix(pVec, &invLtw);
    }
}

void PlatformRider::ToWorldDir(Vector* pVec) {
    if (pObject != NULL && unk8) {
        pVec->ApplyRotMatrix(pVec, pObject->pLocalToWorld);
    }
}
