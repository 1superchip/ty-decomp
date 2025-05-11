#define KEEP_DATA_POOLING (1)

#include "types.h"
#include "ty/props/StaticProp.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"
#include "common/StdMath.h"
#include "common/Str.h"

struct TyParticleManager {
    char unk[0x44];
    void SpawnBridgeChunk(Vector*, Model*);
    void SpawnWaterRipple(Vector*, float);
    void SpawnLeafGrassDust(Vector*, Vector*, bool);
};

extern TyParticleManager* particleManager;

extern "C" void strcpy(char*, char*);
extern "C" int stricmp(char*, char*);
extern "C" void strncpy(char*, char*, int);

ModuleInfo<StaticProp> staticPropModuleInfo;
ModuleInfo<StaticFXProp> staticFXPropModuleInfo;

static float OrderStaticPropFloats(void) {
    return 1.0f;
}

StaticPropLoadInfo StaticProp::loadInfo = {
    {0.0f, 0.0f, 0.0f, 0.0f}, // Default Rotation
    {1.0f, 1.0f, 1.0f, 0.0f} // Default Scale
};

static const Vector unused_vec = {0.0f, 0.0f, 0.0f, 0.0f};

bool StaticFXProp::bTempVisible = false;

static bool PointInBoundingBox(Model* pModel, Vector* pPoint, float arg3) {
    Matrix inverseLTW;

    Vector scale3D = {1.0f, arg3, 1.0f, 0.0f};

    inverseLTW.Scale(&pModel->matrices[0], &scale3D);

    Vector matrixScaleSqr = {
        pModel->matrices[0].Row0()->MagSquared(),
        pModel->matrices[0].Row1()->MagSquared(),
        pModel->matrices[0].Row2()->MagSquared()
    };

    Vector invScale2;

    Vector localPt;

    invScale2.Set(
        1.0f / matrixScaleSqr.x, 
        1.0f / matrixScaleSqr.y, 
        1.0f / matrixScaleSqr.z, 
        1.0f
    );

    inverseLTW.Scale(pModel->matrices, &invScale2);
    inverseLTW.InverseSimple();

    localPt.ApplyMatrix(pPoint, &inverseLTW);

    BoundingVolume* pModelVolume = pModel->GetModelVolume();

    return ((localPt.x >= pModelVolume->v1.x) && localPt.x < pModelVolume->v1.x + pModelVolume->v2.x) &&
        ((localPt.y >= pModelVolume->v1.y) && localPt.y < pModelVolume->v1.y + pModelVolume->v2.y) &&
        ((localPt.z >= pModelVolume->v1.z) && localPt.z < pModelVolume->v1.z + pModelVolume->v2.z);
}

void StaticProp_LoadResources(KromeIni* pIni) {
    StaticPropDescriptor defaultDesc;
    defaultDesc.Init(&staticPropModuleInfo, "", "", 1, 0);
    LoadDescriptors<StaticPropDescriptor>(pIni, "StaticProps", &defaultDesc);
    StaticFXPropDesc defaultFXDesc;
    defaultFXDesc.Init(&staticFXPropModuleInfo, "", "", 1, 0);
    LoadDescriptors<StaticFXPropDesc>(pIni, "StaticFXProps", &defaultFXDesc);
}

void StaticPropDescriptor::Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags) {
    GameObjDesc::Init(pMod, pMdlName, pDescrName, _searchMask, _flags);
    strcpy(subObjectName, "C_Collide");
    bDynamic = false; // does not have dynamic collision by default
    bUseGroundColor = true; // uses ground color by default
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
                    strncpy(subObjectName, pString, sizeof(subObjectName));
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

    collisionInfo.Init(true, GetDesc()->collisionInfoFlags, this);

    lodManager.Init(pModel, 0, &GetDesc()->lodDesc);

    collide = true;
}

void StaticProp::Deinit(void) {
    GameObject::Deinit();
}

bool StaticProp::LoadLine(KromeIniLine* pLine) {
    return GameObject::LoadLine(pLine) || LoadLevel_LoadVector(pLine, "pos", pModel->matrices[0].Row3()) || 
        LoadLevel_LoadVector(pLine, "rot", &loadInfo.defaultRot) || 
        LoadLevel_LoadVector(pLine, "scale", &loadInfo.defaultScale) || 
        LoadLevel_LoadBool(pLine, "collide", &collide);
}

void StaticProp::LoadDone(void) {
    CollisionResult cr;

    pModel->matrices[0].SetTranslation(GetPos());
    pModel->matrices[0].SetRotationPYR(&loadInfo.defaultRot);
    pModel->matrices[0].Scale(&loadInfo.defaultScale);
    pModel->SetLocalToWorldDirty();

    if (pModel->pAnimation != NULL) {
        pModel->pAnimation->CalculateMatrices();
    }

    Vector start = {0.0f, 300.0f, 0.0f, 0.0f};
    start.Add(&start, GetPos());

    Vector end = {0.0f, -12300.0f, 0.0f, 0.0f};
    end.Add(&end, GetPos());

    if (GetDesc()->bUseGroundColor &&
        Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, 0x400)) {
        pModel->colour = Tools_GroundColor(&cr);
    }

    if (collide != false) {
        int index = -1;
        if (GetDesc()->bDynamic) {
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

    loadInfo.defaultRot.SetZero();
    loadInfo.defaultScale.Set(1.0f, 1.0f, 1.0f, 1.0f);

    objectManager.AddObject(this, pModel);
}

void StaticProp::Draw(void) {
    lodManager.Draw(pModel, detailLevel, unk1C, distSquared, IsInWater());
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
    return StaticProp::LoadLine(pLine) || LoadLevel_LoadBool(pLine, "bVisible", &bTempVisible);
}

void StaticFXProp::LoadDone(void) {
    mDefaultRot = StaticProp::loadInfo.defaultRot;
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

    if (Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, ~ID_WATER_BLUE /* ~ID_WATER_BLUE */)
            && (GetDesc()->effectFlags & FX_WaterRipple)) {
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
        StaticProp::Draw();
    }
}

void StaticFXProp::UpdateShake(void) {
    Vector tyShakePos = ty.pos;
    tyShakePos.y += gb.unk78C;

    if (gb.unk78C != 0.0f) {
        
        if (!TyOn() && !PointInBoundingBox(pModel, &tyShakePos, 1.2f)) {
            return;
        }

        if (Abs<float>(gb.unk78C) > 0.1f) {
            Vector tmp = unk58;
            tmp.y += gb.unk78C;
            pModel->SetPosition(&tmp);
            pModel->SetLocalToWorldDirty();
            if (b1 == false) {
                tmp.Set(
                    RandomFR(&gb.mRandSeed, -30.0f, 30.0f),
                    -5.0f,
                    RandomFR(&gb.mRandSeed, -30.0f, 30.0f)
                );
                tmp.Add(&ty.pos);
                particleManager->SpawnBridgeChunk(&tmp, pModel);
                b1 = true;
            }
        } else {
            pModel->SetPosition(&unk58);
            b1 = false;
        }
    }
}

void StaticFXProp::UpdateWaterRipple(void) {
    if (ty.pos.IsInsideSphere(&unk58, 800.0f) && bCollidesWithWater && (gb.logicGameCount > (uint)unk9C)) {
        unk9C = gb.logicGameCount + RandomIR(&gb.mRandSeed, 60, 180);
        Vector ripplePosition = {0.0f, 5.0f, 0.0f, 0.0f};
        ripplePosition.x += waterCollisionPos.x;
        ripplePosition.y += waterCollisionPos.y;
        ripplePosition.z += waterCollisionPos.z;
        particleManager->SpawnWaterRipple(&ripplePosition, RandomFR(&gb.mRandSeed, 80.0f, 120.0f));
    }
}

void StaticFXProp::UpdateDropLeaf(void) {
    int particleFlags = lodManager.pDescriptor->particleFlags;

    if (lodManager.TestLOD(particleFlags) && (gb.logicGameCount > (uint)unk9C)) {
        Vector temp = unk58;
        Vector vel = {0.0f, -10.0f, 0.0f, 0.0f};
        temp.y += RandomIR(&gb.mRandSeed, 700, 800);
        temp.x += RandomIR(&gb.mRandSeed, -1200, 1200);
        temp.z += RandomIR(&gb.mRandSeed, -1200, 1200);
        particleManager->SpawnLeafGrassDust(&temp, &vel, true);
        unk9C = gb.logicGameCount + RandomIR(&gb.mRandSeed, 800, 1200);
    }
}

void StaticFXProp::UpdateRotate(void) {
    autoRotation.Add(&GetDesc()->autoRotate);
    autoRotation.NormaliseRot();

    if (rotateSubObjIndex < 0) {
        Vector r = mDefaultRot;
        r.Add(&autoRotation);
        pModel->matrices[0].SetRotationPYR(&r);
        pModel->SetLocalToWorldDirty();
    } else {
        Matrix* subObjMatrix = &pModel->pMatrices[pModel->GetSubObjectMatrixIndex(rotateSubObjIndex)];
        Vector* subObjectOrigin = pModel->GetSubObjectOrigin(rotateSubObjIndex);
        Vector invOrigin;
        invOrigin.Inverse(subObjectOrigin);
        subObjMatrix->SetIdentity();
        subObjMatrix->Translate(&invOrigin);
        subObjMatrix->RotatePYR(&autoRotation);
        subObjMatrix->Translate(subObjectOrigin);
    }
}

void StaticFXProp::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Hide:
            Show(false);
            break;
        case MSG_Show:
            Show(true);
            break;
        default:
            GameObject::Message(pMsg);
            break;
    }
}

void StaticFXProp::Show(bool bShow) {
    bVisible = bShow;
    
    if (bVisible) {
        collisionInfo.Enable();
    } else {
        collisionInfo.Disable();
    }
}
