#include "ty/SpecialPickup.h"
#include "ty/GameObjectManager.h"
#include "ty/RangeCheck.h"
#include "ty/ParticleEngine.h"

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

    pEggTex[ELEMENT_FIRE] = NULL;
    pEggTex[ELEMENT_ICE] = Texture::Create("Prop_Common_Page_Bluenv");
    pEggTex[ELEMENT_AIR] = Texture::Create("Prop_Common_Page_GRenv");
    pEggTex[ELEMENT_RAINBOW] = NULL;
    pEggTex[ELEMENT_EARTH] = Texture::Create("Prop_Common_Page_Yenv");
}

SpecialPickupStruct* GetThunderEgg(ThunderEggType type) {
    if (!thunderEggDesc.pModule->pData->bUpdate) {
        return NULL;
    }

    DescriptorIterator it = thunderEggDesc.Begin();
    
    while (*it) {
        if ((static_cast<SpecialPickupStruct*>(*it))->subType == type) {
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
        if ((static_cast<SpecialPickupStruct*>(*it))->subType == type) {
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

    SetState(SPS_Idle, true);

    subState = 0;
    
    bInitiallyVisible = true;

    bHideAll = false;

    unk6C = 0.0f;
    unk74 = 0.0f;

    pos.SetZero();
    unk4C.SetZero();

    OnCollected.Init();
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

extern void Particle_DestroyASystem(ParticleSystem**, float);

void SpecialPickupStruct::Deinit(void) {
    if (pParticleSys) {
        Particle_DestroyASystem(&pParticleSys, 0.0f);
    }

    pParticleSys = NULL;

    pEggMat = NULL;
    GameObject::Deinit();
}

extern "C" int stricmp(char*, char*);

bool SpecialPickupStruct::LoadLine(KromeIniLine* pLine) {
    return OnCollected.LoadLine(pLine, "OnCollected") ||
        LoadLevel_LoadInt(pLine, "type", &subType) ||
        LoadLevel_LoadVector(pLine, "pos", &pos) ||
        stricmp(pLine->pFieldName, "camera") == 0 || 
        LoadLevel_LoadVector(pLine, "cameraSource", &cameraSource) ||
        LoadLevel_LoadVector(pLine, "cameraTarget", &cameraTarget) ||
        LoadLevel_LoadBool(pLine, "bInitiallyVisible", &bInitiallyVisible) ||
        mRider.LoadLine(pLine) || 
        GameObject::LoadLine(pLine);
}

extern void Particle_Special_Init(ParticleSystem**, Vector*, BoundingVolume*);
extern void Particle_Special_Create(ParticleSystem**, Vector*, Vector*, Vector*);

void TimeTrial_Cancel(bool);

void SpecialPickupStruct::LoadDone(void) {
    Reset();

    Particle_Special_Init(&pParticleSys, GetPos(), pModel->GetModelVolume()); // inlines?

    objectManager.AddObject(this, pModel);
}

extern "C" double atan2(double, double);

void SpecialPickupStruct::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Resolve:
            OnCollected.Resolve();
            mRider.Resolve();
            mRider.Attach(this);
            break;
        case MSG_Show:
            if (state != SPS_Collected) {
                SetState(SPS_Idle, false);
            }
            break;
        case MSG_Hide:
            if (state != SPS_Collected) {
                SetState(SPS_0, false);
            }
            break;
        case MSG_Enable:
            if (state < SPS_4) {
                particleManager->StopExclamation(true);

                if (
                    !camTarget.x && !camTarget.y && !camTarget.z &&
                    !camSrc.x && !camSrc.y && !camSrc.z
                ) {
                    *GetPos() = *pHero->GetPos();
                }

                SetState(SPS_4, false);
                TimeTrial_Cancel(false);
            }
            break;
        case MSG_UpdateAttachment: {
                PlatformMoveMsg* pMoveMsg = (PlatformMoveMsg*)pMsg;
                GetPos()->Copy(pMoveMsg->trans);
            }
            break;
        case MSG_SpecialPickup_53: {
            SpecialPickUpMessage* pPickUpMsg = (SpecialPickUpMessage*)pMsg;

            camSrc = pPickUpMsg->unk4;
            camSrc.y += 50.0f;
            camSrc.x += 50.0f;

            camTarget = pPickUpMsg->unk4;

            cameraDir.Sub(&camTarget, &camSrc);
            cameraDir.Normalise();

            unk124 = (float)atan2(
                camSrc.z - camTarget.z,
                camSrc.x - camTarget.x
            ) + (PI / 2.0f);

            *GetPos() = pPickUpMsg->unk14;

            Vector mid;
            mid.Sub(&pPickUpMsg->unk4, &pPickUpMsg->unk14);
            mid.Scale(0.5f);
            mid.Add(&pPickUpMsg->unk14);

            mid.y += pPickUpMsg->unk24;

            mQuadratic.SetPoints(&pPickUpMsg->unk14, &mid, &pPickUpMsg->unk4);

            subState = pPickUpMsg->subState;
            unk78 = pPickUpMsg->unk2C;
            unk74 = 0.0f;

            SoundBank_Play(0x1C2, NULL, 0);
            SetState(SPS_2, false);
            TimeTrial_Cancel(false);
            break;
        }
        case MSG_SpecialPickup_54: {
            SpecialPickUpMessage* pPickUpMsg = (SpecialPickUpMessage*)pMsg;

            unk70 = pPickUpMsg->unk30;
            pPickUpMsg->unk34 = &pModel->matrices[0];
            unk74 = pPickUpMsg->unk24;

            SetState(SPS_3, false);
            TimeTrial_Cancel(false);
            break;
        }
        default:
            GameObject::Message(pMsg);
            break;
    }
}

Vector* GameCamera_GetPos(void);

void SpecialPickupStruct::Update(void) {
    if (bHideAll) {
        return;
    }

    switch (state) {
        case SPS_0:
            return;
        case SPS_Idle:
            Idle();
            break;
        case SPS_4:
            Collecting();
            switch (subState) {
                case 6:
                    break;
                default:
                    return;
            }
            break;
        case SPS_2:
            Thrown();
            break;
        case SPS_3:
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

    Vector lPos = *pModel->matrices[0].Row3();

    pModel->matrices[0].SetIdentity();
    pModel->matrices[0].SetRotationYaw(unk6C);

    Matrix sp48;
    sp48.SetRotationPitch(PI / 32.0f);

    pModel->matrices[0].Multiply3x3(&sp48);

    sp48.SetRotationYaw(PI - rot);
    pModel->matrices[0].Multiply3x3(&sp48);

    pModel->matrices[0].Scale(unk70);

    pModel->matrices[0].Row3()->Copy(&lPos);

    if (gb.logicGameCount % 2 == 0) {

    }

    if (pHero->IsTy() && type == SPT_GoldenCog) {
        if (state == SPS_4) {
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

    if (state == SPS_0 || state == SPS_Collected || (state == SPS_4 && (subState == 1 || subState == 6))) {
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
        state = SPS_Idle;
    } else {
        state = SPS_0;
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
        SetState(SPS_4, false);
    }

    unk6C = NormaliseAngle(unk6C + (PI / 128.0f));

    if (pHero->IsTy()) {
        ty.mAutoTarget.Set(TP_2, NULL, NULL, GetPos(), pModel);
    }
}

extern "C" void Sound_MusicDuckVolume(int, int, int);

void SpecialPickupStruct::Collecting(void) {
    switch (subState) {
        case 1:
            if (Dialog_IsLoading() || Dialog_IsPlaying()) {
                return;
            }
            unk70 = 0.0f;
            unk74 = 30.0f;

            switch (type) {
                case SPT_ThunderEgg:
                    Sound_MusicDuckVolume(
                        0,
                        SoundBank_Play(SFX_TyCollectEgg, NULL, 0),
                        15
                    );
                    break;
                case SPT_GoldenCog:
                    Sound_MusicDuckVolume(
                        0,
                        SoundBank_Play(SFX_TyCollectCog, NULL, 0),
                        15
                    );
                    break;
            }
            break;
        case 2:
            unk70 += 0.07f;
            if (unk70 > 1.25f) {
                unk70 = 0.0f;
                subState = 3;
            }
            break;
        case 3:
            unk70 -= 0.07f;
            if (unk70 < 1.0f) {
                unk70 = 1.0f;
                subState = 4;
            }
            break;
        case 4:
            break;
        case 5:
            unk70 -= 0.17f;
            if (unk70 < 0.0f) {
                subState = 6;
                unk70 = 1.0f;
            }
            break;
        case 6:
            if (!pHero->IsClaiming()) {
                SetState(SPS_Collected, false);
            }
            break;
    }

    if (subState != 6) {
        unk74 += 0.2f;
        unk6C = NormaliseAngle(unk6C + (PI / 64.0f));
    }
}

void SpecialPickupStruct::SetState(SpecialPickupState newState, bool r5) {
    if (newState != state || r5) {
        state = newState;

        if (state == SPS_Collected) {
            OnCollected.Send();
            mRider.Detach(this);
            unk5D = true;
        } else if (state == SPS_4) {
            Vector setTo = *GetPos();
            if (!pHero->InWater()) {
                setTo.y = Tools_GetFloor(*GetPos(), NULL, 400.0f, true, ID_WATER_BLUE);
            }

            pHero->SetFindItem(&setTo, this);

            subState = 1;
        }
    }
}

void SpecialPickupStruct::Thrown(void) {
    mQuadratic.Update(unk74 * (gDisplay.dt / unk78));

    *pModel->matrices[0].Row3() = mQuadratic.pos;

    unk74 += 1.0f;

    if (unk74 * (gDisplay.dt / unk78) > 1.0f) {
        switch (subState) {
            case 0:
                SetState(SPS_Idle, false);
                break;
            case 1:
                SetState(SPS_0, false);
                break;
        }
    }
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

void SpecialPickUpMessage::Init(void) {
    unk4.SetZero();
    unk14.SetZero();
    unk24 = 0.0f;
    subState = 0;
    unk2C = 0.0f;
    unk30 = 1.0f;
    unk34 = 0;
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
        state = SPS_0;
        unk5D = false;
    }
}

void SpecialPickupStruct::SetTransparent(bool r4) {
    unk5E = r4;
    if (r4) {
        unk5D = false;
    }
}
