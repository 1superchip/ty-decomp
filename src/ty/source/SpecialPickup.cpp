#include "ty/SpecialPickup.h"
#include "ty/GameObjectManager.h"
#include "ty/RangeCheck.h"

static Material* pEggMat = NULL;
static Texture* pEggTex[ELEMENT_MAX] = {};

static Vector thunderEggColour[ELEMENT_MAX] = {
    {0.8f, 0.1f, 0.2f, 1.0f},
    {0.01f, 0.01f, 0.9f, 1.0f},
    {0.01f, 0.9f, 0.01f, 1.0f},
    {0.8f, 0.1f, 0.8f, 1.0f},
    {0.4f, 0.01f, 0.04f, 1.0f},
};

static bool bHideAll = false;

static LODDescriptor cogLodDesc = {};
static LODDescriptor thEggLodDesc = {};

static ModuleInfo<SpecialPickupStruct> thunderEggModule;
static GameObjDesc thunderEggDesc;

static ModuleInfo<SpecialPickupStruct> goldenCogModule;
static GameObjDesc goldenCogDesc;

void SpecialPickup_HideAll(void) {
    bHideAll = true;
}

void SpecialPickup_ShowAll(void) {
    bHideAll = false;
}

void SpecialPickup_LoadResources(KromeIni* pIni) {
    cogLodDesc.Init(pIni, "Prop_0085_GoldenCog");
    thEggLodDesc.Init(pIni, "Prop_0084_ThunderEgg");

    thunderEggDesc.Init(&thunderEggModule, "Prop_0084_ThunderEgg", "ThunderEgg", 1, 1);
    thunderEggDesc.maxUpdateDist = 9999999.0f;
    objectManager.AddDescriptor(&thunderEggDesc);

    goldenCogDesc.Init(&goldenCogModule, "Prop_0085_GoldenCog", "GoldenCog", 1, 1);
    objectManager.AddDescriptor(&goldenCogDesc);

    pEggMat = NULL;

    pEggTex[0] = NULL;
    pEggTex[1] = Texture::Create("Prop_Common_Page_Bluenv");
    pEggTex[2] = Texture::Create("Prop_Common_Page_GRenv");
    pEggTex[3] = NULL;
    pEggTex[4] = Texture::Create("Prop_Common_Page_Yenv");
}

SpecialPickupStruct* GetThunderEgg(ThunderEggType type) {
    if (!thunderEggDesc.pModule->pData->bUpdate) {
        return NULL;
    }

    DescriptorIterator it = thunderEggDesc.Begin();
    
    while (*it) {
        if (type == (static_cast<SpecialPickupStruct*>(*it))->subType) {
            return static_cast<SpecialPickupStruct*>(*it);
        }

        it++;
    }

    return NULL;
}

SpecialPickupStruct* GetGoldenCog(GoldenCogType type) {
    if (!goldenCogDesc.pModule->pData->bUpdate) {
        return NULL;
    }

    DescriptorIterator it = goldenCogDesc.Begin();
    
    while (*it) {
        if (type == (static_cast<SpecialPickupStruct*>(*it))->subType) {
            return static_cast<SpecialPickupStruct*>(*it);
        }

        it++;
    }

    return NULL;
}

void SpecialPickupStruct::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);

    if (pDesc == &thunderEggDesc) {
        type = SPT_ThunderEgg;
    } else if (pDesc == &goldenCogDesc) {
        type = SPT_GoldenCog;
    }

    SetState(SpecialPickupState_1, true);

    subState = 0;
    
    bInitiallyVisible = true;

    bHideAll = false;

    unk6C = 0.0f;
    unk74 = 0.0f;

    pos.SetZero();
    unk4C.SetZero();

    mEventMessage.Init();
    mRider.Init();

    unk70 = 1.0f;

    switch (type) {
        case SPT_ThunderEgg:
            pModel = Model::Create("Prop_0084_ThunderEgg", NULL);
            mLodManager.Init(pModel, 0, &thEggLodDesc);
            if (!pEggMat) {
                pEggMat = Material::Find("Prop_Common_Page_redEnv");
            }
            break;
        case SPT_GoldenCog:
            pModel = Model::Create("Prop_0085_GoldenCog", NULL);
            mLodManager.Init(pModel, 0, &cogLodDesc);
            break;
    }

    unk5D = false;
    unk5E = false;
}

void SpecialPickupStruct::Deinit(void) {
    pEggMat = NULL;
    GameObject::Deinit();
}

extern "C" int stricmp(char*, char*);

bool SpecialPickupStruct::LoadLine(KromeIniLine* pLine) {
    return mEventMessage.LoadLine(pLine, "OnCollected") ||
        LoadLevel_LoadInt(pLine, "type", &subType) ||
        LoadLevel_LoadVector(pLine, "pos", &pos) ||
        stricmp(pLine->pFieldName, "camera") == 0 || 
        LoadLevel_LoadVector(pLine, "cameraSource", &cameraSource) ||
        LoadLevel_LoadVector(pLine, "cameraTarget", &cameraTarget) ||
        LoadLevel_LoadBool(pLine, "bInitiallyVisible", &bInitiallyVisible) ||
        mRider.LoadLine(pLine) || 
        GameObject::LoadLine(pLine);
}

void SpecialPickupStruct::LoadDone(void) {
    Reset();
    objectManager.AddObject(this, pModel);
}

void SpecialPickupStruct::Message(MKMessage* pMsg) {

}

Vector* GameCamera_GetPos(void);
extern "C" double atan2(double, double);

void SpecialPickupStruct::Update(void) {
    if (bHideAll) {
        return;
    }

    switch (state) {
        case SpecialPickupState_0:
            return;
        case SpecialPickupState_1:
            Idle();
            break;
        case SpecialPickupState_4:
            Collecting();
            switch (subState) {
                case 6:
                    break;
                default:
                    return;
            }
            break;
        case SpecialPickupState_2:
            Thrown();
            break;
        case SpecialPickupState_3:
            Controlled();
            break;
        default:
            return;
    }

    float dz = GameCamera_GetPos()->z - pModel->matrices[0].Row3()->z;
    float dx = GameCamera_GetPos()->x - pModel->matrices[0].Row3()->x;

    float rot = atan2(
        dx, dz
    );

    pModel->matrices[0].SetIdentity();
    pModel->matrices[0].SetRotationYaw(unk6C);

    Matrix sp48;
    sp48.SetRotationPitch(PI / 32.0f);

    pModel->matrices[0].Multiply3x3(&sp48);

    sp48.SetRotationYaw(rot);
    pModel->matrices[0].Multiply3x3(&sp48);

    pModel->matrices[0].Scale(unk70);

    if (gb.logicGameCount % 2 == 0) {

    }

    if (pHero->IsTy() && type == SPT_GoldenCog) {
        if (state == SpecialPickupState_4) {
            UpdateShadow(unk70);
        } else {
            UpdateShadow(1.0f);
        }
    }
}

void SpecialPickupStruct::Draw(void) {
    if (bHideAll) {
        return;
    }

    if (unk5E) {
        unk1C *= 0.33f;
    }

    if (bDrawCheatLines) {
        Blitter_Line3D line;
        if (type == SPT_ThunderEgg) {
            line.color.Set(0.7f, 0.0f, 0.9f, 1.0f);
        } else {
            line.color.Set(0.9f, 0.9f, 0.0f, 1.0f);
        }

        line.color1 = line.color;

        line.point = line.point1 = *GetPos();
        line.point1.y += 10000.0f;

        line.DrawNoMat(1, 1.0f);
    }

    if (type == SPT_ThunderEgg) {
        pEggMat->SetTextureAlias(pEggTex[gb.level.GetElementType()]);
    }

    mLodManager.Draw(pModel, detailLevel, unk1C, distSquared, IsInWater());
}

void SpecialPickupStruct::Reset(void) {
    bHideAll = false;

    if (bInitiallyVisible) {
        state = SpecialPickupState_1;
    } else {
        state = SpecialPickupState_0;
    }

    unk5D = false;
    unk5E = false;
    unk70 = 1.0f;

    pModel->matrices[0].SetIdentity();
    pModel->matrices[0].SetRotationPYR(&unk4C);
    pModel->SetPosition(&pos);
    pModel->SetLocalToWorldDirty();

    mRider.Attach(this);

    camSrc = cameraSource;
    camTarget = cameraTarget;
    cameraDir.Sub(&camTarget, &camSrc);
    cameraDir.Normalise();

    unk124 = (float)atan2(
        camSrc.z - camTarget.z,
        camSrc.x - camTarget.x
    ) + (PI / 2.0f);
}

bool Dialog_IsLoading(void);
bool Dialog_IsPlaying(void);

void SpecialPickupStruct::Idle(void) {
    if (!Dialog_IsLoading() && !Dialog_IsPlaying() && GetPos()->IsInsideSphere(&pHero->pos, pHero->objectRadiusAdjustment + 60.0f)) {
        SetState(SpecialPickupState_4, false);
    }

    unk6C = NormaliseAngle(unk6C + (PI / 128.0f));

    if (pHero->IsTy()) {
        ty.mAutoTarget.Set(TP_2, NULL, NULL, GetPos(), pModel);
    }
}

void SpecialPickupStruct::Collecting(void) {

}

void SpecialPickupStruct::SetState(SpecialPickupState newState, bool) {

}

void SpecialPickupStruct::Thrown(void) {
    mQuadratic.Update((gDisplay.dt / unk78) * unk74);
}

void SpecialPickupStruct::Controlled(void) {

}

void SpecialPickupStruct::ScaleIn(void) {
    if (subState == 6) {
        subState = 2;
        unk70 = 0.001f;
        pModel->matrices[0].Scale(unk70);
    }
}

void SpecialPickupStruct::ScaleOut(void) {
    if (subState < 5) {
        subState = 5;
        unk70 = 1.0f;
    }
}

void SpecialPickupStruct::SetShowPos(Vector* pPos) {
    if (subState != 6) {
        GetPos()->Copy(pPos);
        GetPos()->y += unk74;
    }
}

void SpecialPickupStruct::UpdateShadow(float f1) {

}

bool SpecialPickupStruct::IsCollected(void) {
    return state == SPS_Collected;
}

void SpecialPickupStruct::SetCollected(bool r4) {
    if (r4) {
        state = SPS_Collected;
        unk5D = true;
    } else {
        state = SpecialPickupState_0;
        unk5D = false;
    }
}

void SpecialPickupStruct::SetTransparent(bool r4) {
    unk5E = r4;
    if (r4) {
        unk5D = false;
    }
}
