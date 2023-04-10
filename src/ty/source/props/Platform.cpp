#include "ty/props/Platform.h"
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

extern GameObjectManager objectManager;
extern bool gAssertBool;
Vector Tools_GroundColor(CollisionResult*);
void Tools_DropShadow_Add(float, Vector*, Vector*, float);
bool Tools_ClipSphereToDynamicModel(const Vector&, float, Vector*, Model*, int);
//extern void LoadLevel_LoadInt(KromeIniLine*, char*, int*);

static ModuleInfo<Platform> platformModuleInfo;
static Vector* tempMem;
static Vector* pCurrMem = tempMem;

void Create_UpdateAttachMessage(PlatformMoveMsg* pMsg, Vector* trans, Vector* rot, Vector* def, Matrix* mat) {
    pMsg->unk0 = 3;
    pMsg->trans = trans;
    pMsg->rot = rot;
    pMsg->vec = def;
    pMsg->mat = mat;
}

Vector* TempAlloc(int size) {
    Vector* m = pCurrMem;
    pCurrMem = (Vector*)((int)pCurrMem + size);
    return m;
}

void Platform_LoadResources(KromeIni* pIni) {
	pCurrMem = tempMem = (Vector*)Heap_MemAlloc(0x1000);
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
	SetMaxMag();
	numAttached = 0;
	unk7C.x = 0.0f;
	unk7C.y = 0.0f;
	unk7C.z = 0.0f;
	attachments.Init(0x20);
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
	unk6C = StaticProp::loadInfo[1];
	unk5C = StaticProp::loadInfo[0];
	unk7C = unk5C;
	StaticProp::LoadDone();
}

void Platform::Update(void) {
	BeginUpdate();
	UpdateTilt();
	EndUpdate();
}

void Platform::Reset(void) {
	unk7C = unk5C;
	pModel->matrices[0].SetRotationPYR(&unk7C);
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
			pModel->matrices[0].Row3()->Copy(updateMsg->trans);
			Vector* rotUpdate = updateMsg->rot;
			unk7C.x += rotUpdate->x;
			unk7C.y += rotUpdate->y;
			unk7C.z += rotUpdate->z;
			EndUpdate();
			break;
		case MKMSG_AttachObject:
			Attach(reinterpret_cast<MKMessage_GOBJ*>(pMsg)->pObj);
			break;
		case MKMSG_DetachObject:
			Detach(reinterpret_cast<MKMessage_GOBJ*>(pMsg)->pObj);
	}
	GameObject::Message(pMsg);
}

void Platform::BeginUpdate(void) {
	Matrix mat;
	if (numAttached == 0) {
		return;
	}
    unk58 = TempAlloc(numAttached * sizeof(Vector) + 0x70);
	mat.Inverse(&pModel->matrices[0]);
	*(Matrix*)unk58 = mat;
	unk58[4] = unk7C;
	unk58[5].Set(0.0f, 0.0f, 0.0f, 1.0f);
	int idx = 0;
	GameObject** iter = attachments.GetPointers();
	while (*iter != NULL) {
		(&unk58[idx] + 6)->ApplyMatrix((*iter)->pLocalToWorld->Row3(), &mat);
		unk58[5].Add(&unk58[idx] + 6);
		idx++;
		iter++;
	}
	if (unk58 != NULL) {
		unk58[5].CMultiply(&unk58[5], 1.0f / numAttached);
	}
}

void Platform::EndUpdate(void) {
	UpdateRotationMatrix();
	UpdateAttached();
	if (GetDesc()->maxShadowHeight > 0.0f) {
		UpdateShadow();
	}
	if (unk58 != NULL) {
		pCurrMem = unk58;
	}
	unk58 = NULL;
}

void Platform::UpdateTilt(void) {
	if (GetDesc()->maxTilt > 0.0f) {
		if (unk58 != NULL) {
			//Vector* vecs = (Vector*)unk58;
			if (unk58[5].MagSquared() > Sqr<float>(GetDesc()->radius)) {
				Vector tmp = unk58[5];
				tmp.y = 0.0f;
				tmp.ClampMagnitude(&tmp, GetDesc()->maxMag);
				tmp.CMultiply(&tmp, GetDesc()->maxTilt / GetDesc()->maxMag);
				unk7C.x = AdjustFloat(unk7C.x, -tmp.z, 0.1f);
				unk7C.z = AdjustFloat(unk7C.z, tmp.x, 0.1f);
				return;
			}
		}
		unk7C.x = AdjustFloat(unk7C.x, 0.0f, 0.1f);
		unk7C.z = AdjustFloat(unk7C.z, 0.0f, 0.1f);
	}
}

void Platform::UpdateRotationMatrix(void) {
	Matrix tmpMat;
	pModel->matrices[0].SetRotationPitch(unk7C.x);
	tmpMat.SetRotationRoll(unk7C.z);
	pModel->matrices[0].Multiply3x3(&pModel->matrices[0], &tmpMat);
	tmpMat.SetRotationYaw(unk7C.y);
	pModel->matrices[0].Multiply3x3(&pModel->matrices[0], &tmpMat);
	pModel->matrices[0].Scale(&pModel->matrices[0], &unk6C);
}

void Platform::UpdateAttached(void) {
	PlatformMoveMsg msg;
	Vector deltaPyr;
	if (unk58 != NULL) {
    	deltaPyr.Sub(&unk7C, &unk58[4]);
	    GameObject** iter = attachments.GetPointers();
    	int idx = 0;
		// iterate through attached objects and send the Move Message to them
	    while (*iter != NULL) {
	    	Vector* pVec = &unk58[idx] + 6;
    		pVec->ApplyMatrix(&pModel->matrices[0]);
            Create_UpdateAttachMessage(&msg, &unk58[idx] + 6, &deltaPyr, unk58, &pModel->matrices[0]);
    		(*iter)->Message(&msg);
	    	idx++;
    		iter++;
	    }
    }
}

void Platform::UpdateShadow(void) {
	Vector vec = {pModel->matrices[0].Row3()->x, pModel->matrices[0].Row3()->y - 10.0f,
		pModel->matrices[0].Row3()->z, 0.0f};
	Vector vec1 = {vec.x, vec.y - GetDesc()->maxShadowHeight, vec.z, 0.0f};
	if (CompareVectors(&vec, &unkA0) == false) {
		unkD0 = false;
		CollisionResult cr;
		if (Collision_RayCollide(&vec, &vec1, &cr, (CollisionMode)2, 0)) {
			unkC0 = cr.normal;
			unkB0 = cr.pos;
			unkA0 = vec;
			unkD0 = true;
			if (GetDesc()->bUseGroundColor) {
				pModel->colour = Tools_GroundColor(&cr);
			}
		}
	}
	if (unkD0) {
        float mag = 2.0f * GetDesc()->maxMag;
		float div = 1.0f - (0.6f * Clamp<float>((pModel->matrices[0].data[3][1] - unkB0.y) / 1000.0f, 0.0f, 1.0f));
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

void Platform::PushTy(int arg1) {
	float tyRadius = ty.radius;
	Vector vec;
	Vector tyPos = {ty.pos.x, ty.pos.y + ty.radius, ty.pos.z, ty.pos.w};
	if (Tools_ClipSphereToDynamicModel(tyPos, tyRadius, &vec, pModel, arg1)) {
		vec.y -= tyRadius;
		if (vec.y > ty.pos.y) {
			vec.y = ty.pos.y;
		}
		ty.SetAbsolutePosition(&vec, 0x19, 1.0f, true);
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
    if (pInfo->pProp && (pInfo->pProp->pDescriptor->searchMask & 0x40) &&
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