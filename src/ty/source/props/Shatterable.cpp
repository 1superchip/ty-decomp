#include "ty/GameObjectManager.h"
#include "ty/props/Shatterable.h"
#include "ty/global.h"
#include "ty/soundbank.h"

extern bool gAssertBool;

ShatterStruct* Shatter_Add(Model*, float, float, int);

static Material* pSlideMat;

static ParticleSystemType slideType;

static ModuleInfo<ShatterableFX> shatterableFXModuleInfo;
static ShatterableDescriptor desc;

static ModuleInfo<Shatterable> moduleInfo;

static ParticleEnvelope slideEnvelope[3] = {
    {0.0f, 0.3f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f},
    {0.4f, 0.8f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 0.1f, 1.2f, 0.0f, 0.0f, 0.0f, 0.0f}
};

void Shatterable_LoadResources(KromeIni* pIni) {
    desc.Init(&moduleInfo, "", "", SHATTERABLE_SEARCHMASK | 1, 0);
    LoadDescriptors<ShatterableDescriptor>(pIni, "ShatterableProps", &desc);

    ShatterableFXPropDesc defaultSFXDesc;
    defaultSFXDesc.Init(&shatterableFXModuleInfo, "", "", 1, 0);
    LoadDescriptors<ShatterableFXPropDesc>(pIni, "ShatterableFX", &defaultSFXDesc);

    pSlideMat = Material::Create("fx_079");

    slideType.Init("Slide", pSlideMat, 1.0f, 10.0f, 10.0f, 1);

    slideType.SetEnvelope(ARRAY_SIZE(slideEnvelope), slideEnvelope);

    slideType.unk64 = 0.999f;
    slideType.yVel = -10.0f;
}

/// @brief 
/// @param pDesc Descriptor to check
/// @return 
bool Shatterable_IsMember(MKPropDescriptor* pDesc) {
    return pDesc->searchMask & SHATTERABLE_SEARCHMASK;
}

void ShatterableDescriptor::Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags) {
    StaticPropDescriptor::Init(pMod, pMdlName, pDescrName, _searchMask, _flags);

    upVel = 0.0f;
    power = 3.0f;
    spinRate = 0.1f;
    bounce = 0.7f;
    life = 120;
    gravity = 1.0f;
}

void ShatterableDescriptor::Load(KromeIni* pIni) {
    StaticPropDescriptor::Load(pIni);

    KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);

    if (pLine) {
        pLine = pIni->GotoLine(modelName, NULL);
        while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
            if (pLine->pFieldName) {
                gAssertBool = LoadLevel_LoadFloat(pLine, "upVel", &upVel) ||
                    LoadLevel_LoadFloat(pLine, "power", &power) || 
                    LoadLevel_LoadFloat(pLine, "bounce", &bounce) ||
                    LoadLevel_LoadFloat(pLine, "gravity", &gravity) ||
                    LoadLevel_LoadInt(pLine, "life", &life) ||
                    LoadLevel_LoadFloat(pLine, "spinRate", &spinRate);
            }

            pLine = pIni->GetLineWithLine(pLine);
        }
    }
}

void Shatterable::Init(GameObjDesc* pDesc) {
    StaticProp::Init(pDesc);

    shatter = Shatter_Add(
        pModel,
        GetDesc()->gravity,
        GetDesc()->bounce,
        GetDesc()->life
    );

    OnShatter.Init();
    unk5C = 1;
}

bool Shatterable::LoadLine(KromeIniLine* pLine) {
    bool bVisible = true;

    if (LoadLevel_LoadBool(pLine, "bVisible", &bVisible)) {
        unk5C = bVisible ? 1 : 3;
        return true;
    } else {
        return OnShatter.LoadLine(pLine, "OnShatter") || StaticProp::LoadLine(pLine);
    }
}

void Shatterable::LoadDone(void) {
    StaticProp::LoadDone();
    Reset();
}

void Shatterable::Reset(void) {
    unk58 = unk5C;

    collisionInfo.Init(true, 0, this);

    if (unk58 == 3) {
        collisionInfo.Disable();
    } else {
        collisionInfo.Enable();
    }
}

void Shatterable::Draw(void) {
    if (unk58 != 0 && unk58 != 3) {
        StaticProp::Draw();
    }
}

struct UnknownShatterMessage : MKMessage {
    Vector unk4;
    bool unk14;
};

void Shatterable::Message(MKMessage* pMsg) {
    Vector tempVel = {0.0f, GetDesc()->upVel, 0.0f};

    switch (pMsg->unk0) {
        case MSG_UNK_7:
            if (unk58 != 0) {
                UnknownShatterMessage* pUnkMsg = (UnknownShatterMessage*)pMsg;
                pUnkMsg->unk14 = true;

                tempVel.Add(&pUnkMsg->unk4);

                tempVel.y = GetDesc()->upVel;

                collisionInfo.Disable();

                shatter->Explode(&tempVel, GetDesc()->spinRate, GetDesc()->power);
                PlayShatterSound();
                unk58 = 0;
                OnShatter.Send();
            }
            break;
        case MSG_ExplosionMsg:
        case MSG_UNK_20:
            if (unk58 != 0) {
                collisionInfo.Disable();
                shatter->Explode(&tempVel, GetDesc()->spinRate, GetDesc()->power);
                PlayShatterSound();
                unk58 = 0;
                OnShatter.Send();
            }
            break;
        case MSG_Resolve:
            OnShatter.Resolve();
            break;
        case MSG_Activate:
        case MSG_Show:
            unk58 = 1;
            collisionInfo.Enable();
            break;
        case MSG_Deactivate:
        case MSG_Hide:
            unk58 = 3;
            collisionInfo.Disable();
            break;
        default:
            GameObject::Message(pMsg);
            break;
    }
}

extern "C" int stricmp(char*, char*);

void Shatterable::PlayShatterSound(void) {
    if (
        stricmp(GetDesc()->descrName, "WoodFenceMid1") == 0 ||
        stricmp(GetDesc()->descrName, "WoodFenceMid2") == 0 ||
        stricmp(GetDesc()->descrName, "WoodFenceEnd1") == 0 ||
        stricmp(GetDesc()->descrName, "WoodFenceEnd2") == 0
    ) {
        SoundBank_Play(0x151, GetPos(), 0);
    } else if (
        stricmp(GetDesc()->descrName, "Shed1") == 0 || 
        stricmp(GetDesc()->descrName, "Shack1") == 0
    ) {
        SoundBank_Play(0x152, GetPos(), 0);
    } else if (stricmp(GetDesc()->descrName, "RoadPost") == 0) {
        SoundBank_Play(0x239, GetPos(), 0);
    } else if (
        stricmp(GetDesc()->descrName, "EmuSign") == 0 || 
        stricmp(GetDesc()->descrName, "SpeedSign") == 0
    ) {
        SoundBank_Play(0x23A, GetPos(), 0);
    } else if (
        stricmp(GetDesc()->descrName, "LeafClump") == 0 || 
        stricmp(GetDesc()->descrName, "RiverLog") == 0 ||
        stricmp(GetDesc()->descrName, "RiverRock") == 0
    ) {
        SoundBank_Play(0x235, GetPos(), 0);
    } else if (
        stricmp(GetDesc()->descrName, "HayBale") == 0 || 
        stricmp(GetDesc()->descrName, "GrassTree1") == 0
    ) {
        SoundBank_Play(0x25D, GetPos(), 0);
    } else if (stricmp(GetDesc()->descrName, "WaterTrough") == 0) {
        SoundBank_Play(0x25E, GetPos(), 0);
    } else if (
        stricmp(GetDesc()->descrName, "BurningLog") == 0 || 
        stricmp(GetDesc()->descrName, "FlamingLog2") == 0
    ) {
        SoundBank_Play(0x260, GetPos(), 0);
    } else if (stricmp(GetDesc()->descrName, "CollIceWall") == 0) {
        SoundBank_Play(0x261, GetPos(), 0);
    }
}

void ShatterableFX::Init(GameObjDesc* pDesc) {
    Shatterable::Init(pDesc);
    
    pSystem = NULL;
}

void ShatterableFX::LoadDone(void) {
    Shatterable::LoadDone();

    switch (GetDesc()->effectFlags) {
        case 1:
            pSystem = ParticleSystem::Create(
                &slideType, 
                GetPos(), 
                pModel->GetModelVolume(),
                1.0f,
                1,
                NULL
            );
            break;
    }
}

void ShatterableFX::Deinit(void) {
    switch (GetDesc()->effectFlags) {
        case 1:
            if (pSystem) {
                pSystem->Destroy();
            }
            break;
    }

    StaticProp::Deinit();
}

void ShatterableFX::Reset(void) {
    Shatterable::Reset();

    switch (GetDesc()->effectFlags) {
        case 1:
            if (pSystem) {
                pSystem->DestroyAll();
            }
            break;
    }
}

Vector* GameCamera_GetDir(void);

void ShatterableFX::Update(void) {
    if (unk58 != 0 && unk58 != 3) {
        Shatterable::Update();

        switch (GetDesc()->effectFlags) {
            case 1:
                // Create a particle
                if (pSystem && RandomIR(&gb.mRandSeed, 0, 4) == 0) {
                    Particle* p = pSystem->CreateParticle();
                    if (p) {
                        p->mX = GetPos()->x - GameCamera_GetDir()->x * 60.0f;
                        p->mY = GetPos()->y - 10.0f;
                        p->mZ = GetPos()->z - GameCamera_GetDir()->z * 60.0f;

                        p->unk20 = RandomFR(&gb.mRandSeed, -100.0f, 100.0f);
                        p->unk24 = RandomFR(&gb.mRandSeed, 30.0f, 60.0f);
                        p->unk28 = RandomFR(&gb.mRandSeed, -100.0f, 100.0f);
                        p->mAngle = RandomFR(&gb.mRandSeed, -PI, PI);
                    }
                }
                break;
        }
    }
}
