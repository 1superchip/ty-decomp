#include "types.h"
#include "ty/GameObjectManager.h"
#include "ty/props/StaticProp.h"
#include "common/StdMath.h"

struct TyParticleManager {
    char unk[0x44];
    void SpawnBridgeChunk(Vector*, Model*);
    void SpawnWaterRipple(Vector*, float);
    void SpawnLeafGrassDust(Vector*, Vector*, bool);
};

extern TyParticleManager* particleManager;

extern struct Ty {
    char padding[0x40];
    Vector pos;
    char unk[0x7F4];
    bool unk844;
    bool unk845;
    bool unk846;
    float unk848;
    float unk84C;
    char unk1[0x34];
    int unk884;
} ty;

extern struct GlobalVar {
    int unk[483];
    float unk78C;
} gb;

struct CollisionResult {
    Vector pos;
    Vector normal;
    Vector color;
    Model* pModel;
    int itemIdx;
    int collisionFlags;
    CollisionInfo* pInfo;
    float unk40;
};

enum CollisionMode {

};

bool Collision_RayCollide(Vector*, Vector*, CollisionResult*, CollisionMode, int);
void Collision_AddStaticModel(Model*, CollisionInfo*, int);
void Collision_AddDynamicModel(Model*, CollisionInfo*, int);
void Collision_AddDynamicSubobject(Model*, int, CollisionInfo*);
Vector Tools_GroundColor(CollisionResult*);
extern "C" void strcpy(char*, char*);
extern "C" int stricmp(char*, char*);
extern "C" void strncpy(char*, char*, int);

extern GameObjectManager objectManager;

ModuleInfo<StaticProp> staticPropModuleInfo;
ModuleInfo<StaticFXProp> staticFXPropModuleInfo;

static float OrderStaticPropFloats(void) {
	return 1.0f;
}

Vector StaticProp::loadInfo[2] = {{0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}};

static const Vector unused_vec = {0.0f, 0.0f, 0.0f, 0.0f};

bool StaticFXProp::bTempVisible;

inline bool BoundingVolume_CheckPoint(BoundingVolume *volume, Vector *point) {
    bool isWithin = false;
    if ((point->x >= volume->v1.x) && point->x < volume->v1.x + volume->v2.x) {
        if ((point->y >= volume->v1.y) && point->y < volume->v1.y + volume->v2.y) {
            if ((point->z >= volume->v1.z) && point->z < volume->v1.z + volume->v2.z) {
                isWithin = true;
            }
        }
    }
    return isWithin;
}

inline bool PointInBoundingBox(Model* pModel, Vector* pPoint, float arg3) {
    Matrix mat;
    Vector matrixScale = {1.0f, 0.0f, 1.0f, 0.0f};
    matrixScale.y = arg3;
    mat.Scale(&pModel->matrices[0], &matrixScale);
    float fVar1 = pModel->matrices[0].Row0()->Dot(pModel->matrices[0].Row0());
    float fVar2 = pModel->matrices[0].data[1][0] * pModel->matrices[0].data[1][0] +
    pModel->matrices[0].data[1][1] * pModel->matrices[0].data[1][1] + 
    pModel->matrices[0].data[1][2] * pModel->matrices[0].data[1][2];
    float fVar3 = pModel->matrices[0].data[2][0] * pModel->matrices[0].data[2][0] +
    pModel->matrices[0].data[2][1] * pModel->matrices[0].data[2][1] + 
    pModel->matrices[0].data[2][2] * pModel->matrices[0].data[2][2];
    Vector inv = {fVar1, fVar2, fVar3, 0.0f};
    Vector vec;
    Vector local;
    vec.x = 1.0f / inv.x;
    vec.y = 1.0f / inv.y;
    vec.w = 1.0f;
    vec.z = 1.0f / inv.z;
    mat.Scale(pModel->matrices, &vec);
    mat.InverseSimple(&mat);
    local.ApplyMatrix(pPoint, &mat);
    BoundingVolume* pModelVolume = pModel->GetBoundingVolume(-1);
    if (BoundingVolume_CheckPoint(pModelVolume, &local) != false) {
        return true;
    }
    return false;
}

void StaticProp_LoadResources(KromeIni* pIni) {
    StaticPropDescriptor defaultDesc;
    defaultDesc.Init(&staticPropModuleInfo, "", "", 1, 0);
    LoadDescriptors<StaticPropDescriptor>(pIni, "StaticProps", &defaultDesc);
    StaticFXPropDesc defaultFXDesc;
    defaultFXDesc.Init(&staticFXPropModuleInfo, "", "", 1, 0);
    LoadDescriptors<StaticFXPropDesc>(pIni, "StaticFXProps", &defaultFXDesc);
}

void StaticPropDescriptor::Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int arg4, int arg5) {
    GameObjDesc::Init(pMod, pMdlName, pDescrName, arg4, arg5);
    strcpy(subObjectName, "C_Collide");
    bDynamic = false; // does not have dynamic collision by default
    bUseGroundColor = true;
    collisionInfoFlags = 0;
}

void StaticPropDescriptor::Load(KromeIni* pIni) {
    GameObjDesc::Load(pIni);
    KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);
    if (pLine != NULL) {
        lodDesc.Init(pIni, modelName);
        while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
            if (pLine->pFieldName != NULL) {
                char* pString = NULL;
                if (stricmp(pLine->pFieldName, "collide") == 0 && pLine->AsString(0, &pString) != false) {
                    if (stricmp("dynamic", pString) == 0) {
                        bDynamic = true;
                        if (pLine->elementCount > 1) {
                            pLine->AsString(1, &pString);
                        }
                    }
                    strncpy(subObjectName, pString, 0x20);
                }
                LoadLevel_LoadBool(pLine, "bUseGroundColor", &bUseGroundColor);
                NameFlagPair flagsTmp[3] = {{"Grabable", 1}, {"NoIce", 2}, {"JumpCamera", 4}};
                LoadLevel_LoadFlags(pLine, "collisionInfoFlags", flagsTmp, 3, &collisionInfoFlags);
            }
            pLine = pIni->GetLineWithLine(pLine);
        }
    }
}

void StaticProp::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    pModel = Model::Create(pDesc->modelName, NULL);
    pModel->renderType = 3;
    int collisionFlags = GetDesc()->collisionInfoFlags;
    collisionInfo.bEnabled = true;
    collisionInfo.flags = collisionFlags;
    collisionInfo.pProp = this;
    lodManager.Init(pModel, 0, &descr_cast<StaticPropDescriptor*>(pDescriptor)->lodDesc);
    collide = true;
}

void StaticProp::Deinit(void) {
	GameObject::Deinit();
}

bool StaticProp::LoadLine(KromeIniLine* pLine) {
    bool ret = false;
    if (GameObject::LoadLine(pLine) || LoadLevel_LoadVector(pLine, "pos", pModel->matrices[0].Row3()) || 
        LoadLevel_LoadVector(pLine, "rot", &loadInfo[0]) || 
        LoadLevel_LoadVector(pLine, "scale", &loadInfo[1]) || 
        LoadLevel_LoadBool(pLine, "collide", &collide)) {
        ret = true;
        }
    return ret;
}

void StaticProp::LoadDone(void) {
    CollisionResult cr;
    pModel->matrices[0].SetTranslation(pModel->matrices[0].Row3());
    pModel->matrices[0].SetRotationPYR(&loadInfo[0]);
    pModel->matrices[0].Scale(&pModel->matrices[0], &loadInfo[1]);
    pModel->SetLocalToWorldDirty();
    if (pModel->pAnimation != NULL) {
        pModel->pAnimation->CalculateMatrices();
    }
    Vector start = {0.0f, 300.0f, 0.0f, 0.0f};
    start.Add(&start, GetPos());
    Vector end = {0.0f, -12300.0f, 0.0f, 0.0f};
    end.Add(&end, GetPos());
    if (GetDesc()->bUseGroundColor != false && Collision_RayCollide(&start, &end, &cr, (CollisionMode)2, 0x400) != false) {
        pModel->colour = Tools_GroundColor(&cr);
    }
    if (collide != false) {
        int index = -1;
        if (GetDesc()->bDynamic != false) {
			// dynamic collision
            if (pModel->SubObjectExists(GetDesc()->subObjectName, &index)) {
                Collision_AddDynamicSubobject(pModel, index, &collisionInfo);
                pModel->EnableSubObject(index, false);
            } else {
                Collision_AddDynamicModel(pModel, &collisionInfo, -1);
            }
        } else {
            if (pModel->SubObjectExists(GetDesc()->subObjectName, &index)) {
                Collision_AddStaticModel(pModel, &collisionInfo, index);
                pModel->EnableSubObject(index, false);
            }
        }
    }
    loadInfo[0].SetZero();
    loadInfo[1].Set(1.0f, 1.0f, 1.0f, 1.0f);
    objectManager.AddObject(this, pModel);
}

void StaticProp::Draw(void) {
    lodManager.Draw(pModel, detailLevel, unk1C, distSquared, flags & 0x40000000);
}

void StaticFXProp::Init(GameObjDesc* pDesc) {
    StaticProp::Init(pDesc);
	// does not collide with water by default
    bCollidesWithWater = false;
    unk9C = 10;
    bVisible = true;
    bTempVisible = bVisible;
    autoRotation.SetZero();
    rotateSubObjIndex = -1;
}

bool StaticFXProp::LoadLine(KromeIniLine* pLine) {
    bool ret = false;
    if (GameObject::LoadLine(pLine) || LoadLevel_LoadVector(pLine, "pos", pModel->matrices[0].Row3()) || 
        LoadLevel_LoadVector(pLine, "rot", &loadInfo[0]) || 
        LoadLevel_LoadVector(pLine, "scale", &loadInfo[1]) || 
        LoadLevel_LoadBool(pLine, "collide", &collide) || LoadLevel_LoadBool(pLine, "bVisible", &bTempVisible)) {
        ret = true;
        }
    return ret;
}

void StaticFXProp::LoadDone(void) {
    rot = StaticProp::loadInfo[0];
    StaticProp::LoadDone();
    CollisionResult cr;
    
    bVisible = bTempVisible;
    bDefaultVisible = bVisible;
    Vector start = {0.0f, 300.0f, 0.0f, 0.0f};
    start.x += unk58.x;
    start.y += unk58.y;
    start.z += unk58.z;
    Vector end = {0.0f, -300.0f, 0.0f, 0.0f};
    end.x += unk58.x;
    end.y += unk58.y;
    end.z += unk58.z;
    if (Collision_RayCollide(&start, &end, &cr, (CollisionMode)2, -0x401) && (GetDesc()->effectFlags & FX_WaterRipple)) {
		// if collision against water occurs and this prop has Water Ripple effects
		// set collides with water and the position of the collision
        bCollidesWithWater = true;
        waterCollisionPos = cr.pos;
    }
    if ((GetDesc()->effectFlags & FX_Rotate) && GetDesc()->rotateSubObj[0] != '\0') {
		// if the rotation subobject name isn't empty and this prop has Rotate effects
		// get the index of the subobject that controls rotation
        rotateSubObjIndex = pModel->GetSubObjectIndex(GetDesc()->rotateSubObj);
    }
}

void StaticFXProp::Reset(void) {
    GameObject::Reset();
    Show(bDefaultVisible);
}

void StaticFXProp::Update(void) {
    if (GetDesc()->effectFlags & FX_WaterRipple) {
        UpdateWaterRipple();
    }
    if (GetDesc()->effectFlags & FX_Shake) {
        UpdateShake();
    }
    if (GetDesc()->effectFlags & FX_SpawnLeaf) {
        UpdateDropLeaf();
    }
    if (GetDesc()->effectFlags & FX_Rotate) {
        UpdateRotate();
    }
}

void StaticFXProp::Draw(void) {
    if (bVisible) {
        lodManager.Draw(pModel, detailLevel, unk1C, distSquared, flags & 0x40000000);
    }
}

void StaticFXProp::UpdateShake(void) {
    Vector tyPos = ty.pos;
    tyPos.y += gb.unk78C;
    if (gb.unk78C != 0.0f) {
        bool unk = false;
        if (ty.unk844 != false) {
            float diff;
            if (ty.unk846 != false) {
                diff = ty.pos.y - ty.unk84C;
            } else {
                diff = 10000.0f;
            }
            if (diff < 10.0f) {
                unk = true;
            }
        }
        if (((ty.unk845 != false || unk) && (int*)ty.unk884 == (int*)&collisionInfo) == false) {
            if (PointInBoundingBox(pModel, &tyPos, 1.2f) == false) {
                return;
            }
        }
        float unkFloat = (gb.unk78C < 0.0f) ? -gb.unk78C : gb.unk78C;
        if (unkFloat > 0.1f) {
            Vector vecPos = unk58;
            vecPos.y += gb.unk78C;
            pModel->SetPosition(&vecPos);
            pModel->SetLocalToWorldDirty();
            if (b1 == false) {
                float rand = RandomFR((int*)&gb.unk[174], -30.0f, 30.0f);
                vecPos.x = RandomFR((int*)&gb.unk[174], -30.0f, 30.0f);
                vecPos.y = -5.0f;
                vecPos.x = vecPos.x + ty.pos.x;
                vecPos.z = rand;
                vecPos.y = -5.0f + ty.pos.y;
                vecPos.z = vecPos.z + ty.pos.z;
                particleManager->SpawnBridgeChunk(&vecPos, pModel);
                b1 = true;
            }
        } else {
            pModel->SetPosition(&unk58);
            b1 = false;
        }
    }
}

void StaticFXProp::UpdateWaterRipple(void) {
    if (ty.pos.DistSq(&unk58) < 640000.0f && bCollidesWithWater && ((uint)gb.unk[469] > (uint)unk9C)) {
        unk9C = gb.unk[469] + RandomIR((int*)&gb.unk[174], 0x3c, 0xb4);
        Vector ripplePosition = {0.0f, 5.0f, 0.0f, 0.0f};
        ripplePosition.x += waterCollisionPos.x;
        ripplePosition.y += waterCollisionPos.y;
        ripplePosition.z += waterCollisionPos.z;
        particleManager->SpawnWaterRipple(&ripplePosition, RandomFR((int*)&gb.unk[174], 80.0f, 120.0f));
    }
}

void StaticFXProp::UpdateDropLeaf(void) {
    int particleFlags = lodManager.pDescriptor->particleFlags;
    if (lodManager.TestLOD(particleFlags) && ((uint)gb.unk[469] > (uint)unk9C)) {
        Vector vecPos = unk58;
        Vector offset = {0.0f, -10.0f, 0.0f, 0.0f};
        vecPos.y += RandomIR((int*)&gb.unk[174], 700, 800);
        vecPos.x += RandomIR((int*)&gb.unk[174], -1200, 1200);
        vecPos.z += RandomIR((int*)&gb.unk[174], -1200, 1200);
        particleManager->SpawnLeafGrassDust(&vecPos, &offset, true);
        unk9C = gb.unk[469] + RandomIR((int*)&gb.unk[174], 800, 1200);
    }
}

void StaticFXProp::UpdateRotate(void) {
    StaticFXPropDesc* pDesc = GetDesc();
    autoRotation.x += pDesc->autoRotate.x;
    autoRotation.y += pDesc->autoRotate.y;
    autoRotation.z += pDesc->autoRotate.z;
    autoRotation.NormaliseRot(&autoRotation);
    if (rotateSubObjIndex < 0) {
        Vector modelRot = rot;
        modelRot.x += autoRotation.x;
        modelRot.y += autoRotation.y;
        modelRot.z += autoRotation.z;
        pModel->matrices[0].SetRotationPYR(&modelRot);
        pModel->SetLocalToWorldDirty();
    } else {
        Matrix* subObjMatrix = &pModel->pMatrices[pModel->GetSubObjectMatrixIndex(rotateSubObjIndex)];
        Vector* subObjectOrigin = pModel->GetSubObjectOrigin(rotateSubObjIndex);
        Vector origin;
        origin.x = -1.0f * subObjectOrigin->x;
        origin.y = -1.0f * subObjectOrigin->y;
        origin.z = -1.0f * subObjectOrigin->z;
        subObjMatrix->SetIdentity();
        subObjMatrix->Translate(subObjMatrix, &origin);
        subObjMatrix->RotatePYR(subObjMatrix, &autoRotation);
        subObjMatrix->Translate(subObjMatrix, subObjectOrigin);
    }
}

void StaticFXProp::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case 15:
            Show(false);
            break;
        case 14:
            Show(true);
            break;
        default:
            GameObject::Message(pMsg);
    }
}

void StaticFXProp::Show(bool bShow) {
    bVisible = bShow;
    if (bVisible != false) {
        collisionInfo.bEnabled = true;
    } else {
        collisionInfo.bEnabled = false;
    }
}