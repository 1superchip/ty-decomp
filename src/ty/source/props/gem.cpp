// Opal object source file

#include "ty/props/gem.h"
#include "ty/tytypes.h"
#include "ty/global.h"
#include "ty/DDA.h"
#include "ty/tools.h"
#include "ty/GameObjectManager.h"

// EXTERNS

struct OpalMagnetData {
    bool IsActive(void);
};
struct Hero : GameObject {
    int unk3C;
    Vector mPos;
    char padding1[0xD8 - (sizeof(Vector) + sizeof(GameObject))];
    float objectAdjustmentRadius;
    int type;
    char padding2[0x1FC - 0xE4];
    OpalMagnetData mMagnetData;

    bool IsTy(void) {
        return type == 0;
    }
    bool IsBushPig(void) {
        return type == 1;
    }
};

extern Hero* pHero;

struct TyFSM {
    char padding[0x10];
    int state;
    // https://decomp.me/scratch/wZJsi
    bool BiteState(int);

    bool BiteState(void) {
        return BiteState(state);
    }
};

struct Ty {
    char padding[0xa38];
    TyFSM fsm;
    char padding1[0x1250-(0xa38 + sizeof(TyFSM))];
    int medium;
    char padding2[0x1338-0x1254];
    int superbitecharge;
    char padding3[0x1a90-0x133c];
    int unk1A90;
    
    int GetMedium(void) {
        return medium;
    }
};
extern Ty ty;

// END EXTERNS

bool bDrawCheatLines = false;
Gem** Gem::gemPtrList = NULL;
int Gem::totalGems = 0;
static int bHideAll = false;
static ModuleInfo<Gem> moduleInfo;
static ParticleSystem* pSystem = NULL;
static int numDynamicData = 0;
static ElementType gemElement = ELEMENT_FIRE;
static int counter = 0;

StructList<Gem> spawnedGemList;
static GameObjDesc opalDesc;
static ParticleSystemType gemType[MAX_GEM_ELEMENTS];
static StructList<GemModelDrawData> modelDraw;
static StructList<GemPickupData> pickupData;
static StructList<Blitter_Particle> pickupDraw;

// TODO: Correct data ordering without externs and variables for literals
extern ElementInfo elementInfo[MAX_GEM_ELEMENTS];
extern GemParticleSysInfo gemPartSysInfo[MAX_GEM_ELEMENTS];

void Gem_DrawModel(Vector* pPos, Vector* pScale, ElementType type) {
    if (type < ELEMENT_FIRE || type >= MAX_GEM_ELEMENTS) {
        // if the type isn't valid, use the gemElement
        type = gemElement;
    }
    Model* pGemModel = elementInfo[type].pModel;
    pGemModel->matrices[0].CopyRotation(&View::GetCurrent()->unk48);
    pGemModel->matrices[0].SetTranslation(pPos);
    pGemModel->matrices[0].Scale(pScale);
    pGemModel->colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
    pGemModel->Draw(NULL);
}

void Gem_SetElement(ElementType newType) {
    gemElement = newType;
}

// fake symbols to disable pooling of .data in Gem_LoadResources
// and keep the correct .data order
__declspec(section ".sdata") extern char opal_str[];
extern char Prop_0270_FireOpal_str[];
extern char GemParticles_str[];

void Gem_LoadResources(KromeIni* pIni) {
    opalDesc.Init(&moduleInfo, Prop_0270_FireOpal_str, opal_str, 0, 1);
    opalDesc.Load(pIni);
    objectManager.AddDescriptor(&opalDesc);
    spawnedGemList.Init(GEMS_MAXOPALS);
    for(int i = 0; i < MAX_GEM_ELEMENTS; i++) {
        elementInfo[i].pMaterial = Material::Create(elementInfo[i].pMaterialName);
        elementInfo[i].pParticleMaterial = Material::Create(elementInfo[i].pMaterialName1);
        elementInfo[i].pModel = Model::Create(elementInfo[i].pModelName, NULL);
        elementInfo[i].pModel->renderType = 3;
        gemType[i].Init(GemParticles_str, elementInfo[i].pParticleMaterial,
            gemPartSysInfo[i].unk0, gemPartSysInfo[i].unk4, gemPartSysInfo[i].unk8, 1);
        if (gemPartSysInfo[i].updateFunc != NULL) {
            gemType[i].updateFunc = gemPartSysInfo[i].updateFunc;
        }
        gemType[i].SetEnvelope(4, gemPartSysInfo[i].envelopes);
        gemType[i].SetDistances(1000.0f, 2500.0f, 3000.0f);
    }
    // Allocate memory for opal pointer list
    Gem::gemPtrList = (Gem**)Heap_MemAlloc(sizeof(Gem*) * GEMS_MAXOPALS);
    Gem_PickupParticle_LoadResources();
}

// Data is defined here to force the previous function to not pool data
static ElementInfo elementInfo[MAX_GEM_ELEMENTS] = {
    {"prop_0270_fireopal", "fx_105", "fx_005", NULL, NULL, NULL},
    {"prop_0380_IceOpal", "fx_116", "fx_073", NULL, NULL, NULL},
    {"prop_0382_AirOpal", "fx_118", "fx_129", NULL, NULL, NULL},
    {"prop_0218_rainbowscale", "fx_090", "fx_128", NULL, NULL, NULL},
    {"prop_0381_EarthOpal", "fx_117", "fx_127", NULL, NULL, NULL}
};

static GemParticleSysInfo gemPartSysInfo[MAX_GEM_ELEMENTS] = {
    {
        4.0f, 2.0f, 2.0f,
        {
            {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.2f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.8f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        },
        &Gem_FireCustomUpdate
    },
    {
        4.0f, 1.5f, 1.5f,
        {
            {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.2f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.8f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        },
        NULL
    },
    {
        4.0f, 2.0f, 2.0f,
        {
            {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.2f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.8f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        },
        &Gem_FireCustomUpdate
    },
    {
        4.0f, 2.0f, 2.0f,
        {
            {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.2f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.8f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        },
        &Gem_FireCustomUpdate
    },
    {
        4.0f, 1.5f, 1.5f,
        {
            {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.2f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {0.8f, 0.7f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        },
        NULL
    }
};

char Prop_0270_FireOpal_str[] = "Prop_0270_FireOpal";
char GemParticles_str[] = "Gem Particles";
__declspec(section ".sdata") char opal_str[] = "Opal";


/// @brief Creates a new gem object
/// @param type Unused parameter
/// @param pPos Position of the gem
/// @param r5 
/// @return Pointer to gem object
Gem* Gem_Add(GemType type, Vector* pPos, Vector* r5) {
    Gem* pNewGem = spawnedGemList.GetNextEntry();
    opalDesc.pModule->ConstructObject(pNewGem); // construct the new Gem object
    pNewGem->Init(&opalDesc);
    pNewGem->pos = *pPos;
    pNewGem->unk94 = pNewGem->pos;
    pNewGem->mCollected = false;
    pNewGem->unk6C = 0;
    pNewGem->unkF6b0 = 0;
    pNewGem->mParticle.pos = pNewGem->pos;
    pNewGem->mParticle.angle = 0.0f;
    pNewGem->SetState((GemState)2);
    pNewGem->unkB4.SetIdentity();
    pNewGem->unkB4.SetTranslation(&pNewGem->pos);
    objectManager.AddObject(pNewGem, &pNewGem->unkB4,
        elementInfo[gemElement].pModel->GetModelVolume());
    if (r5) {
        pNewGem->unkF6b0 = 1;
        pNewGem->unk94 = *r5;
        pNewGem->SpawnStatic();
        pNewGem->SetState((GemState)0);
    } else {
        pNewGem->SetState((GemState)2);
    }
    pNewGem->Reset();
    pNewGem->CalcShadowPos();
    // Add the new gem to the list
    Gem::gemPtrList[Gem::totalGems++] = pNewGem;
    return pNewGem;
}

/// @brief Returns the number of existing opals
/// @param  None
/// @return Number of gems created
int Gem_GetCount(void) {
    return Gem::totalGems;
}

extern "C" void memset(void*, int, int);

/// @brief Deinits all spawned opals
/// @param  None
void Gem_DeleteList(void) {
    Gem* pGem = spawnedGemList.GetCurrEntry();
    while (pGem) {
        pGem->Deinit();
        pGem = spawnedGemList.GetNextEntryWithEntry(pGem);
    }
    spawnedGemList.UnknownSetPointer();
    // Set all gem pointers to NULL
    memset((void*)Gem::gemPtrList, 0, sizeof(Gem*) * GEMS_MAXOPALS);
    Gem::totalGems = 0; // set total spawned gems to 0
}

bool Gem::LoadLine(KromeIniLine* pLine) {
    return GameObject::LoadLine(pLine) || LoadLevel_LoadVector(pLine, "pos", &pos);
}

void Gem::LoadDone(void) {
    unk94 = pos;
    mCollected = false;
    unk6C = 0;
    unkF6b0 = 0;
    mParticle.pos = pos;
    mParticle.angle = 0.0f;
    SetState((GemState)2);
    CalcShadowPos();
    unkB4.SetIdentity();
    unkB4.SetTranslation(&pos);
    objectManager.AddObject(this, &unkB4,
        elementInfo[gemElement].pModel->GetModelVolume());
    // add this gem to the pointer list and increment the total gem count
    Gem::gemPtrList[Gem::totalGems++] = this;
    Reset();
}

void Gem::Init(GameObjDesc* pDesc) {
    GameObject::Init(pDesc);
    pos.Set(0.0f, 0.0f, 0.0f, 1.0f);
    bHideAll = false;
}

void Gem::Deinit(void) {
    pModel = NULL;
    GameObject::Deinit();
}

void Gem::Update(void) {
    // if bHideAll, don't update opals
    if (bHideAll || mState == (GemState)0) {
        return;
    }
    yOffsetAngle += 0.081f;
    if (yOffsetAngle >= (2 * PI)) {
        yOffsetAngle -= (2 * PI);
    }
    switch (mState) {
        case 2:
            Idle();
            break;
        case 3:
            Magnetised();
            break;
        case 4:
            Collecting();
            break;
        case 1:
            Spawning();
            break;
    }
    mParticle.pos.w = 1.0f;
    unkB4.SetTranslation(&mParticle.pos);
    if (mState < (GemState)5 && unkF6b2) {
        Vector tempPos;
        tempPos.x = mParticle.pos.x;
        tempPos.y = unkFC;
        tempPos.z = mParticle.pos.z;
        Tools_DropShadow_Add(
            40.0f, &tempPos, &mCollisionNormal,
            1.0f - Clamp<float>(0.0f, (mParticle.pos.y - unkFC) / 100.0f, 1.0f) * 0.5f
        );
    }
    if (mState < (GemState)4) {
        pSystem->pDynamicData[pSystem->mNumDyn].pMatrix = &unkB4;
        pSystem->mNumDyn++;
    }
}

extern void Draw_AddPostDrawElement(void*, void (*)(void*), float, bool);

void Gem::Draw(void) {
    if (bHideAll || mState == (GemState)0) {
        return;
    }
    if (mState >= (GemState)4) {
        return;
    }
    if (bDrawCheatLines) {
        Blitter_Line3D line;
        line.color.Set(0.0f, 1.0f, 0.0f, 1.0f);
        line.color1.Set(0.0f, 1.0f, 0.0f, 1.0f);
        line.point1 = mParticle.pos;
        line.point = mParticle.pos;
        line.point1.y += 10000.0f;
        line.DrawNoMat(1, 1.0f);
    }
    unkB4.CopyRotation(&View::GetCurrent()->unk48);
    elementInfo[gemElement].pModel->matrices[0] = unkB4;
    float draw = Sqr<float>(GetDesc()->maxDrawDist * 0.5f);
    if (mCollected) {
        unk1C *= 0.1f;
    }
    elementInfo[gemElement].pModel->colour.Set(1.0f, 1.0f, 1.0f, unk1C);
    if (distSquared < draw) {
        Draw_AddPostDrawElement((void*)this, &PostDraw, distSquared, GetDrawFlag() ? true : false);
    } else {
        View::GetCurrent()->SetLocalToWorldMatrix(NULL);
        mParticle.color.w = unk1C;
        elementInfo[gemElement].pMaterial->Use();
        mParticle.Draw(1);
    }
}

void Gem::PostDraw(void* pObj) {
    Gem* pGem = (Gem*)pObj;
    elementInfo[gemElement].pModel->matrices[0] = pGem->unkB4;
    elementInfo[gemElement].pModel->colour.Set(1.0f, 1.0f, 1.0f, pGem->unk1C);
    elementInfo[gemElement].pModel->Draw(NULL);
}

void Gem::Reset(void) {
    yOffsetAngle = RandomFR(&gb.mRandSeed, 0.0f, 2 * PI);
    bHideAll = false;
    if (unkF6b0) {
        SetState((GemState)0);
        mLerpTime = 0.0f;
        return;
    }
    SetState((GemState)2);
    mParticle.pos = pos;
}

void Gem_HideAll(void) {
    bHideAll = true;
}

void Gem_ShowAll(void) {
    bHideAll = false;
}

void Gem::Spawn(void) {
    if (mState != (GemState)5) {
        SetState((GemState)1);
    }
}

bool Gem::UpdateCollection(float f1) {
    bool bCollect = false;
    if (unkF4 > 0) {
        unkF4--;
        return false;
    }
    if (f1 <= Sqr<float>(pHero->objectAdjustmentRadius)) {
        bCollect = true;
    }
    if (bCollect) {
        Collect();
        return true;
    }
    return false;
}

float Gem::GetMagneticRangeSqr(void) {
    // default magnetic range is (hero->objectRadius + 100)^2
    float magneticRadius = Sqr<float>(pHero->objectAdjustmentRadius + 100.0f);
    if (pHero->mMagnetData.IsActive()) {
        // if the opal magnet is active
        // adjust the radius based on how many opals are collected
        int collectedGemCount = gb.mGameData.GetLevelCollectedGemCount();
        int newRadius = 500;
        if (collectedGemCount >= 270) {
            // if there are less than or equal to 30 opals left to collect
            newRadius += 1000; // 1500
        }
        if (collectedGemCount >= 285) {
            // if there are less than or equal to 15 opals left to collect
            newRadius += 1500; // 3000
        }
        if (collectedGemCount >= 295) {
            // if there are less than or equal to 5 opals left to collect
            newRadius += 3000; // 6000
        }
        if (collectedGemCount >= 298) {
            // if there are less than or equal to 2 opals left to collect
            newRadius += 6000; // 12000
        }
        if (collectedGemCount >= 299) {
            // if there are less than or equal to 1 opals left to collect
            newRadius += 24000; // 36000
        }
        magneticRadius = Sqr<int>(newRadius);
    }
    if (pHero->IsTy()) {
        if (ty.fsm.BiteState()) {
            switch (ty.GetMedium()) {
                case 0:
                case 1:
                case 2:
                case 5:
                case 6:
                    magneticRadius += (ty.superbitecharge / 150.0f) * (500000.0f - magneticRadius);
                    break;
            }
        } else {
            if (ty.unk1A90 == 3 && !pHero->mMagnetData.IsActive()) {
                magneticRadius *= 2.1f;
            }
        }
    }
    return magneticRadius;
}

bool Gem::CheckMagnetism(float f1) {
    if ((unkF4 > 0 && !pHero->IsBushPig())
        || (f1 > 1000000.0f && !pHero->mMagnetData.IsActive())) {
        return false;
    }
    if (f1 <= GetMagneticRangeSqr()) {
        SetState((GemState)3);
        return true;
    }
    return false;
}

void Gem::SpawnStatic(void) {
    Vector spC;
    spC.Sub(&unk94, &pos);
    float mag = spC.Magnitude();
    unk84[0] = RandomFR(&gb.mRandSeed, 0.2f, 0.4f);
    unk84[0] = unk84[0] / sqrtf(mag);
    unk84[1] = mag * 0.8f;
    if (unk84[1] < 200.0f) {
        unk84[1] = 200.0f;
    }
    unk84[2] = spC.y / spC.x;
    mLerpTime = 0.0f;
}

// Gem::SpawnDynamic went unused and used the float literals 2.0f and 50.0f
float Gem_SpawnDynamic(void) {
    volatile float x = 2.0f;
    return 50.0f;
}

void Gem::Idle(void) {
    mParticle.pos.y = unk94.y + _table_sinf(yOffsetAngle) * 5.0f;
    float heroDist = pHero->mPos.DistSq(&mParticle.pos);
    if (!UpdateCollection(heroDist)) {
        CheckMagnetism(heroDist);
    }
}

void Gem::Magnetised(void) {
    float f31 = 0.9f;
    if (pHero->mMagnetData.IsActive()) {
        f31 = 0.15f;
    } else if (pHero->IsTy()) {
        if (ty.fsm.BiteState()) {
            switch (ty.medium) {
                case 0:
                case 1:
                case 2:
                case 5:
                case 6:
                    f31 = 0.25f;
                    break;
            }
        } else if (ty.unk1A90 == 3) {
            f31 = 0.9f;
        }
    }
    Vector dir; // direction to hero
    Vector heroPos = pHero->mPos;
    heroPos.y += 50.0f;
    dir.Sub(&heroPos, &mParticle.pos);
    float mag = dir.Normalise(); // normalize direction
    if (!UpdateCollection(Sqr<float>(mag))) {
        if (mLerpTime >= 50.0f) {
            Collect();
        } else {
            dir.Scale(mLerpTime);
            mParticle.pos.Add(&dir);
            mLerpTime += (mag * 0.01f) * f31;
            if ((gb.logicGameCount % 5) == 0) {
                CalcShadowPos();
            }
        }
    }
}

void Gem::Collecting(void) {
    mParticle.unk20 *= 0.7f;
    if (mParticle.unk20 <= 0.1f) {
        SetState((GemState)5);
    }
}

void Gem::Spawning(void) {
    float dist = pHero->mPos.DistSq(&mParticle.pos);
    if (UpdateCollection(dist) || CheckMagnetism(dist)) {
        return;
    }
    if (mLerpTime >= 1.0f) {
        mParticle.pos = unk94;
        CalcShadowPos();
        SetState((GemState)2);
    } else {
        mParticle.pos.InterpolateLinear(&pos, &unk94, mLerpTime);
        mParticle.pos.y += unk84[1] * (1.0f - SqrXMinHalf(mLerpTime) * 4.0f);
        mLerpTime += unk84[0];
    }
}

void Gem::SetState(GemState newState) {
    mState = newState;
    switch (mState) {
        case (GemState)0:
            mParticle.color.Set(0.0f, 0.0f, 0.0f, 0.0f);
            mParticle.unk20 = 0.0f;
            break;
        case (GemState)2:
            unkF4 = gDisplay.displayFreq * 0.5f;
            mParticle.color.Set(
                1.0f, 1.0f, 1.0f, mCollected ? 0.15f : 1.0f
            );
            mParticle.unk20 = 20.0f;
            break;
        case (GemState)3:
            mLerpTime = 0.0f;
            break;
        case (GemState)1:
            unkF4 = gDisplay.displayFreq * 0.5f;
            mParticle.pos = pos;
            mParticle.color.Set(
                1.0f, 1.0f, 1.0f, mCollected ? 0.15f : 1.0f
            );
            mParticle.unk20 = 20.0f;
            break;
    }
}

extern void SoundBank_Play(int, Vector*, unsigned int);

void Gem::Collect(void) {
    SetState((GemState)4);
    Gem_PickupParticle_SpawnParticles(&mParticle.pos);
    gb.mGameData.CollectGem(mCollected);
    dda.StorePickupInfo(Pickup_Gem);
    SoundBank_Play(0xB7, NULL, 0);
}

void Gem::CalcShadowPos(void) {
    Vector start = mParticle.pos;
    Vector end = {mParticle.pos.x, mParticle.pos.y - 200.0f, mParticle.pos.z};
    CollisionResult cr;
    // Check for ground underneath the opal
    unkF6b2 = Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, 0);
    if (unkF6b2) {
        // if ground, set unkFC and copy the collision normal
        unkFC = cr.pos.y + 1.0f;
        mCollisionNormal = cr.normal;
    }
}

void Gem_ParticleSystem_Init(void) {
    static Vector pos = {0.0f, 0.0f, 0.0f, 0.0f};
    static BoundingVolume volume = {
        {-30.0f, -30.0f, -30.0f, 0.0f}, {30.0f, 30.0f, 30.0f, 0.0f}
    };
    if (Gem::totalGems <= 0) {
        return;
    }
    pSystem = ParticleSystem::Create(&gemType[gemElement],
        &pos, &volume, 1.0f, Gem::totalGems, NULL);
    if (pSystem == NULL) {
        return;
    }
    // Set the matrix of each DynamicData entry in the Particle System to the corresponding
    // gem matrix
    for(int i = 0; i < Gem::totalGems; i++) {
        pSystem->pDynamicData[i].pMatrix = &Gem::gemPtrList[i]->unkB4;
    }
    numDynamicData = pSystem->mNumDyn;
    pSystem->mNumDyn = 0;
}

void Gem_ParticleSystem_Deinit(void) {
    if (pSystem) {
        pSystem->mNumDyn = numDynamicData;
        pSystem->Destroy();
        pSystem = NULL;
    }
}

void Gem_ParticleSystem_Update(void) {
    Particle* p;
    if (pSystem == NULL || bHideAll) {
        return;
    }
    counter++;
    if ((counter % 20) != 0) {
        return;
    }
    switch (gemElement) {
        case ELEMENT_FIRE:
        case ELEMENT_AIR:
        case ELEMENT_RAINBOW:
            p = pSystem->CreateParticle();
            if (p) {
                p->mX = 0.0f;
                p->mY = 0.0f;
                p->mZ = 0.0f;
                p->unk20 = 0.0f;
                p->unk24 = 0.0f;
                p->unk28 = 7.5f;
                p->unk50 = 6.0f;
                ((char*)&p->unk58)[1] = 1;
                ((char*)&p->unk58)[0] = RandomIR(&gb.mRandSeed, 1, 3);
                p->unk3C = 0.0f;
                p->mAngle = RandomFR(&gb.mRandSeed, 0.0f, PI);
            }
            break;
        case ELEMENT_ICE:
        case ELEMENT_EARTH:
            p = pSystem->CreateParticle();
            if (p) {
                p->mX = RandomFR(&gb.mRandSeed, -10.0f, 10.0f);
                p->mY = RandomFR(&gb.mRandSeed, -5.0f, 10.0f);
                p->mZ = RandomFR(&gb.mRandSeed, 0.0f, 10.0f);
                p->unk20 = 0.0f;
                p->unk24 = RandomFR(&gb.mRandSeed, -10.0f, -5.0f);
                p->unk28 = 0.0f;
                p->unk3C = 0.0f;
                p->mAngle = RandomFR(&gb.mRandSeed, -PI, PI);
            }
            break;
    }
}

void Gem_FireCustomUpdate(ParticleSystem* pSys) {
    float f30 = pSys->age - pSys->mpType->unk1C;

    ParticleChunk** ppChunks = &pSys->mpChunks;
    do {
        ParticleChunk* pCurrChunk = *ppChunks;
        Particle* pParticle = &pCurrChunk->mChunkData[pCurrChunk->mDataIndex];
        do {
            if (f30 >= pParticle->unkC) {
                pSys->DestroyParticle(pParticle, ppChunks);
            }
            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);
        if (*ppChunks == pCurrChunk) {
            ppChunks = &(*ppChunks)->mpNext;
        }
    } while (*ppChunks);
    
    ppChunks = &pSys->mpChunks;
    while (*ppChunks) {
        ParticleChunk* pCurrChunk = *ppChunks;
        Particle* pParticle = &pCurrChunk->mChunkData[pCurrChunk->mDataIndex];
        do {
            Matrix sp28;
            pParticle->unk3C += 0.05f;
            if (pParticle->unk3C > (2.0f * PI)) {
                pParticle->unk3C -= (2.0f * PI);
            }
            Vector sp18 = {0.0f, pParticle->unk3C, pParticle->mAngle};
            Vector sp8 = {pParticle->unk20, pParticle->unk24, pParticle->unk28};
            sp28.SetRotationPYR(&sp18);
            sp8.ApplyRotMatrix(&sp28);
            pParticle->mX = sp8.x;
            pParticle->mY = sp8.y;
            pParticle->mZ = sp8.z;
            float delta = pSys->age - pParticle->unkC;
            if (delta > pSys->mpType->mpEnvelopes[pParticle->mEnvelopeIndex].unkC) {
                pParticle->mEnvelopeIndex++;
            }
            ParticleEnvelope* pEnv = &pSys->mpType->mpEnvelopes[pParticle->mEnvelopeIndex - 1];
            float delta2 = ((delta - pEnv->unkC) / pEnv->deltaAge);
            // delta = pEnv->unk14 * ((delta - pEnv->unkC) / pEnv->deltaAge);
            pParticle->mColor.w = (delta2 * pEnv->unk14) + pEnv->unk4;
            pParticle->unk50 += (((char*)&pParticle->unk58[0])[1]);
            if (pParticle->unk50 > 20.0f) {
                ((char*)&pParticle->unk58[0])[1] = 0xFF;
            } else if (pParticle->unk50 < 6.0f) {
                ((char*)&pParticle->unk58[0])[1] = 1;
            }
            pParticle->unk30 = ((((char*)&pParticle->unk58[0])[0]) * 0.1f) * (pParticle->unk50 * 0.05f);
            pParticle->unk48 = 0.0f;
            pParticle->unk4C = 1.0f;
            pParticle++;
        } while (pParticle < &pCurrChunk->mChunkData[24]);
        ppChunks = &(*ppChunks)->mpNext;
    }
}

void Gem_PickupParticle_LoadResources(void) {
    modelDraw.Init(15);
    pickupData.Init(75);
    pickupDraw.Init(75);
}

void Gem_PickupParticle_SpawnParticles(Vector* pVector) {
    Vector xDir;
    xDir = *pHero->pModel->matrices[0].Row0();
    xDir.y = 0.0f;
    xDir.Normalise();
    Vector zDir;
    zDir = *pHero->pModel->matrices[0].Row2();
    zDir.y = 0.0f;
    zDir.Normalise();
    zDir.Scale(-5.5f);
    for(int i = 0; i < 5; i++) {
        Blitter_Particle* pBlitterParticle = pickupDraw.GetNextEntry();
        GemPickupData* pPickupData = pickupData.GetNextEntry();
        if (pBlitterParticle == NULL) {
            break;
        }
        pPickupData->unk0 = xDir;
        pPickupData->unk0.Scale(RandomFR(&gb.mRandSeed, -1.8f, 1.8f));
        pPickupData->unk0.y = RandomFR(&gb.mRandSeed, 1.7f, 2.0f);
        pPickupData->unk0.Add(&zDir);
        pPickupData->unk10 = 0.5f;
        pBlitterParticle->pos = *pVector;
        pBlitterParticle->color.Set(1.0f, 1.0f, 1.0f, 0.8f);
        pBlitterParticle->unk20 = 12.0f;
        pBlitterParticle->angle = 0.0f;
    }
    GemModelDrawData* pModelData = modelDraw.GetNextEntry();
    if (pModelData) {
        pModelData->unk24 = 1.0f;
        pModelData->unk10 = false;
        pModelData->unk0 = *pVector;
        pModelData->unk0.w = 1.0f;
        pModelData->unk28 = 0.0f;
        for(int i = 0; i < NUM_GEMMODELDATA_IMAGES; i++) {
            pModelData->unk180_array[i][1] = RandomFR(&gb.mRandSeed, 0.0f, 2.0f * PI);
            pModelData->unk180_array[i][0] = RandomFR(&gb.mRandSeed, 5.0f, 15.0f);
            pModelData->imgs[i].unk40.x = 1.0f;
            pModelData->imgs[i].unk40.y = 1.0f;
            pModelData->imgs[i].unk40.z = 1.0f;
            pModelData->imgs[i].unk40.w = 1.0f;

            pModelData->imgs[i].z = 0.0f;

            pModelData->imgs[i].color.Set(1.0f, 1.0f, 1.0f, 1.0f);

            pModelData->imgs[i].uv0 = 0.0f;
            pModelData->imgs[i].uv2 = 1.0f;
            pModelData->imgs[i].uv1 = 0.0f;
            pModelData->imgs[i].uv3 = 1.0f;

            pModelData->imgs[i].startX = -10.0f;
            pModelData->imgs[i].endX = -10.0f;
            pModelData->imgs[i].startY = -10.0f;
            pModelData->imgs[i].endY = -10.0f;
        }
    }
}

void Gem_PickupParticle_Update(void) {

    if (pSystem) {
        pSystem->mNumDyn = 0;
    }

    GemModelDrawData* pModelData = modelDraw.GetCurrEntry();

    while(pModelData) {
        pModelData->unk24 -= gDisplay.updateFreq;
        if (pModelData->unk24 <= 0.0f) {
            modelDraw.CopyEntry(pModelData);
        } else if (pModelData->unk10) {
            pModelData->unk1C = (575.0f - pModelData->unk14) * 0.1f;
            pModelData->unk20 = (72.0f - pModelData->unk18) * 0.1f;
            pModelData->unk14 += pModelData->unk1C;
            pModelData->unk18 += pModelData->unk20;
            if (pModelData->unk28 < 0.1f) {
                pModelData->unk28 += 0.002f;
            }
            for(int i = 0; i < NUM_GEMMODELDATA_IMAGES; i++) {
                if (pGameSettings->unk5 == 1) {
                    pModelData->imgs[i].startX = (pModelData->unk14 - 11.0f) + 
                        (pModelData->unk180_array[i][0] * _table_sinf(pModelData->unk180_array[i][1])) + 60.0f;
                    pModelData->imgs[i].endX = pModelData->imgs[i].startX + 22.0f + 60.0f;
                } else {
                    pModelData->imgs[i].startX = (pModelData->unk14 - 11.0f) + 
                        pModelData->unk180_array[i][0] * _table_sinf(pModelData->unk180_array[i][1]);
                    pModelData->imgs[i].endX = pModelData->imgs[i].startX + 22.0f;
                }
                pModelData->imgs[i].startY = (pModelData->unk18 - 11.0f) + 
                    pModelData->unk180_array[i][0] * _table_cosf(pModelData->unk180_array[i][1]);
                pModelData->imgs[i].endY = pModelData->imgs[i].startY + 22.0f;
                pModelData->unk180_array[i][1] += 0.15f;
                if (pModelData->unk24 < 0.4f) {
                    pModelData->imgs[i].color.w -= 1.0f / (gDisplay.displayFreq * 0.25f);
                    pModelData->unk180_array[i][0] += 1.5f;
                }
            }
        }
        pModelData = modelDraw.GetNextEntryWithEntry(pModelData);
    }

    Blitter_Particle* pBlitParticle = pickupDraw.GetCurrEntry();
    GemPickupData* pPickupData = pickupData.GetCurrEntry();
    while(pPickupData) {
        pPickupData->unk10 -= gDisplay.updateFreq;
        if (pPickupData->unk10 <= 0.0f) {
            pickupData.CopyEntry(pPickupData);
            pickupDraw.CopyEntry(pBlitParticle);
        } else {
            pBlitParticle->pos.Add(&pPickupData->unk0);
            if (pPickupData->unk10 < 0.2f) {
                pBlitParticle->color.w -= gDisplay.updateFreq * 4.0f;
            }
        }
        pPickupData = pickupData.GetNextEntryWithEntry(pPickupData);
        pBlitParticle = pickupDraw.GetNextEntryWithEntry(pBlitParticle);
    }
}

void GameCamera_Use(bool);

void Gem_PickupParticle_Draw(void) {
    if (gb.unk704) {
        return;
    }
    GameCamera_Use(true);
    float f29 = View::GetCurrent()->unk2C0;
    View::GetCurrent()->unk2C0 = 30.0f;
    GemModelDrawData* pModelData = modelDraw.GetCurrEntry();
    while(pModelData) {
        if (!pModelData->unk10) {
            pModelData->unk10 = true;
            View::GetCurrent()->SetLocalToWorldMatrix(NULL);
            IntVector screen;
            pModelData->unk28 = View::GetCurrent()->TransformPoint(&screen, &pModelData->unk0);
            pModelData->unk28 = Clamp<float>(0.05f, pModelData->unk28 * 27.0f, 0.1f);
            pModelData->unk14 = screen.x;
            pModelData->unk18 = screen.y;
        }
        pModelData = modelDraw.GetNextEntryWithEntry(pModelData);
    }
    pModelData = modelDraw.GetCurrEntry();
    while (pModelData != NULL) {
        Vector scale = {
            pModelData->unk28,
            pModelData->unk28,
            pModelData->unk28,
            0.0f
        };
        View::GetCurrent()->TransformPoint2Dto3D(
            pModelData->unk14, pModelData->unk18,
            10.0f + View::GetCurrent()->unk2C0, &pModelData->unk0
        );
        Gem_DrawModel(&pModelData->unk0, &scale, gemElement);
        View::GetCurrent()->SetLocalToWorldMatrix(NULL);
        elementInfo[gemElement].pParticleMaterial->Use();
        pModelData->imgs[0].Draw(NUM_GEMMODELDATA_IMAGES);

        pModelData = modelDraw.GetNextEntryWithEntry(pModelData);
    }
    Blitter_Particle* pBlitterParticle = pickupDraw.GetCurrEntry();
    if (pBlitterParticle) {
        View::GetCurrent()->SetLocalToWorldMatrix(NULL);
        elementInfo[gemElement].pParticleMaterial->Use();
        pBlitterParticle->Draw(pickupDraw.GetCount());
    }
    View::GetCurrent()->unk2C0 = f29;
}

Material* Gem_GetParticleMaterial(ElementType type) {
    return elementInfo[type].pParticleMaterial;
}
