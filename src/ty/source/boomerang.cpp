#include "ty/boomerang.h"
#include "ty/BoomerangManager.h"
#include "ty/effects/ChronorangEffects.h"
#include "ty/Shadow.h"
#include "ty/tools.h"
#include "ty/global.h"
#include "ty/GameObjectManager.h"
#include "common/Translation.h"
#include "common/MKGrass.h"
#include "ty/soundbank.h"
#include "ty/ty.h"

extern bool gAssertBool;
extern "C" void strcpy(char*, char*);

static Doomerang* pInAirDoomerang = NULL;
static ModuleInfo<EndGameObjective> endGameObjectiveModuleInfo;
static GameObjDesc endGameObjectiveDesc;

static BoomerangType boomerangChangeOrder[NUM_BOOMERANGS] = {
    BR_Standard, 
    BR_Flamerang, 
    BR_Frostyrang, 
    BR_Zappyrang, 
    BR_Zoomerang, 
    BR_Multirang, 
    BR_Infrarang, 
    BR_Megarang, 
    BR_Kaboomerang, 
    BR_Chronorang, 
    BR_Doomerang, 
    BR_Aquarang,
};

static char* BOOMERANG_DEFAULT_TRAIL_MAT = "RANGTRAIL_01";

static Shadow boomerangShadow[NUM_BOOMERANGS];

static BoundingVolume rangVol = {
    {-3000.0f, -3000.0f, -3000.0f, 0.0f},
    {6000.0f, 6000.0f, 6000.0f, 0.0f}
};

static BoomerangStaticInfo boomerangInfo[NUM_BOOMERANGS] = {
    {
        "prop_0484_rang_01",
        0x1E,
        0x2C,
        {0x14, 0x15, 0x16, 0x171}
    },
    {
        "prop_0485_rang_02",
        0x21,
        0x2F,
        {0x172, 0x173, 0x174, 0x175}
    },
    {
        "prop_0486_rang_03",
        0x20,
        0x2E,
        {0x176, 0x177, 0x1C, 0x178}
    },
    {
        "prop_0538_rang_23",
        0x28,
        0x36,
        {0x17D, 0x17E, 0x17F, 0x180}
    },
    {
        "prop_0487_rang_06",
        0x2A,
        0x38,
        {0x181, 0x182, 0x183, 0x184}
    },
    {
        "prop_0539_rang_22",
        0x27,
        0x35,
        {0x185, 0x186, 0x187, 0x188}
    },
    {
        "prop_0489_rang_08",
        0x24,
        0x32,
        {0x189, 0x18A, 0x18B, 0x18C}
    },
    {
        "prop_0492_rang_11",
        0x26,
        0x34,
        {0x18D, 0x18E, 0x18F, 0x190}
    },
    {
        "prop_0493_rang_20",
        0x23,
        0x31,
        {0x192, 0x193, 0x194, 0x195}
    },
    {
        "prop_0488_rang_07",
        0x1F,
        0x2D,
        {0x17, 0x18, 0x19, 0x196}
    },
    {
        "prop_0491_rang_10",
        0x25,
        0x33,
        {0x197, 0x198, 0x199, 0x19A}
    },
    {
        "prop_0537_rang_21",
        0x29,
        0x37,
        {0x14, 0x15, 0x16, 0x268}
    },
};

BoomerangDesc* Boomerang_pDescriptors[NUM_BOOMERANGS];
static DoomerangDesc boomerangDescriptors[NUM_BOOMERANGS];

PtrList<Boomerang> boomerangs;

static ModuleInfo<Boomerang> boomerangModule;
static ModuleInfo<Frostyrang> frostyrangModule;
static ModuleInfo<Flamerang> flamerangModule;
static ModuleInfo<Kaboomerang> kaboomerangModule;

static ModuleInfo<Doomerang> doomerangModule;
static ModuleInfo<Megarang> megarangModule;
static ModuleInfo<Zappyrang> zappyrangModule;
static ModuleInfo<Aquarang> aquarangModule;

static ModuleInfo<Chronorang> chronorangModule;

static ModuleInfoBase* pBoomerangModules[NUM_BOOMERANGS] = {
    &boomerangModule, // Boomerang
    &frostyrangModule,
    &flamerangModule,
    &kaboomerangModule,
    &doomerangModule,
    &megarangModule,
    &boomerangModule, // Zoomerang
    &boomerangModule, // Infrarang
    &zappyrangModule,
    &aquarangModule,
    &boomerangModule, // Mutirang
    &chronorangModule,
};

void GameCamera_SnapDoomarangCamera(Vector*, Vector*, Vector*, float);

void BoomerangDesc::Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, BoomerangType type, BoomerangStaticInfo* pInfo) {
    GameObjDesc::Init(pMod, pMdlName, pDescrName, 1, 1);
    strcpy(trailMatName, BOOMERANG_DEFAULT_TRAIL_MAT);
    mType = type;
    mpStaticInfo = pInfo;
    ammoCount = 1;
    ammoCount = 1;
    unkA8 = 2;
    unkAC = 2;
    range = 1500.0f;
    flightTime = 1.5f;
}

void BoomerangDesc::Load(KromeIni* pIni) {
    GameObjDesc::Load(pIni);

    KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);

    while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
        if (pLine->pFieldName) {
            gAssertBool = LoadLevel_LoadFloat(pLine, "range", &range) ||
                LoadLevel_LoadFloat(pLine, "flightTime", &flightTime) ||
                LoadLevel_LoadString(pLine, "trailMatName", trailMatName, sizeof(trailMatName), 0) ||
                LoadLevel_LoadInt(pLine, "ammoCount", &ammoCount);
        }

        pLine = pIni->GetLineWithLine(pLine);
    }
}

void DoomerangDesc::Load(KromeIni* pIni) {
    BoomerangDesc::Load(pIni);

    KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);

    pitchSpeed = turnSpeed = 25.0f;
    speed = 10.0f;
    
    while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
        if (pLine->pFieldName) {
            gAssertBool = LoadLevel_LoadFloat(pLine, "pitchSpeed", &pitchSpeed) ||
                LoadLevel_LoadFloat(pLine, "turnSpeed", &turnSpeed) ||
                LoadLevel_LoadFloat(pLine, "speed", &speed);
        }
        
        pLine = pIni->GetLineWithLine(pLine);
    }
}

extern void Particle_Fire_Init(ParticleSystem**, Vector*, BoundingVolume*, float, bool);
extern "C" void strcat(char*, char*);

void Boomerang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    mSpline.Init(8, false);

    mpWeapon = pWeapon;
    
    GameObject::Init(pDesc);

    pModel = Model::Create(pDesc->modelName, NULL);

    mRangType = GetDesc()->mType;

    attachIdx = pModel->GetRefPointIndex("R_RangAttach");
    subObjectIndex = pModel->GetSubObjectIndex("a_motion");

    rang01Idx = pModel->GetRefPointIndex("r_rang01");
    rang02Idx = pModel->GetRefPointIndex("r_rang02");

    Vector rang1Pos;
    Vector rang2Pos;

    pModel->GetRefPointWorldPosition(rang01Idx, &rang1Pos);
    pModel->GetRefPointWorldPosition(rang02Idx, &rang2Pos);
    mag = ApproxMag(&rang1Pos, &rang2Pos);
    
    mTrail.Init(GetDesc()->trailMatName, 0.5f);

    unkA8.unk0 = -1;

    pModelUnkF4 = NULL;

    char shadowName[32];
    strcpy(shadowName, pDesc->modelName);
    strcat(shadowName, "shadow");
    pModelUnkF4 = Model::Create(shadowName, NULL);

    unk56 = true;
    bParticlesEnabled = true;
    unk98 = PI / 6.0f; // 30 degrees
    unk9C = 0.8f;

    Particle_Fire_Init(&pFireSys, &mPos, &rangVol, 2.0f, true);

    Reset();
    unk94 = 0x35;

    if (mRangType == BR_Zoomerang) {
        unk56 = false;
    }
}

extern void Particle_DestroyASystem(ParticleSystem**, float);

void Boomerang::Deinit(void) {
    GameObject::Deinit();
    mSpline.Deinit();
    mTrail.Deinit();

    if (pFireSys) {
        Particle_DestroyASystem(&pFireSys, 0.0f);
    }
    
    pFireSys = NULL;

    if (pModelUnkF4) {
        pModelUnkF4->Destroy();
        pModelUnkF4 = NULL;
    }

    StopSounds();
}

void Boomerang::Update(void) {
    if (mpWeapon->mSide == BOOMERANG_SIDE_LEFT) {
        pModel->matrices[0].Row1()->Inverse();
    }

    switch (unk6C) {
        case BOOMERANG_STATE_0:
            UpdateIdle();
            break;
        case BOOMERANG_STATE_1:
            UpdateLoaded();
            if (bParticlesEnabled) {
                UpdateParticleEffect();
            }
            UpdateColor();
            break;
        case BOOMERANG_STATE_2:
            UpdateFired();
            UpdateTrail();
            UpdateParticleEffect();
            HandleFire();
            UpdateColor();
            break;
    }

    pModel->matrices[0].SetTranslation(&mPos);

    if (mpWeapon->mSide == BOOMERANG_SIDE_LEFT) {
        pModel->matrices[0].Row1()->Inverse();
    }

    if (unk6C != BOOMERANG_STATE_0) {
        if (mpWeapon->mSide == BOOMERANG_SIDE_LEFT) {
            MKGrass_SetPushAwayPos(&mPos, 1);
        } else {
            MKGrass_SetPushAwayPos(&mPos, 2);
        }
    }
}

void Boomerang::Draw(void) {
    if (gb.pDialogPlayer != NULL || unk6C == BOOMERANG_STATE_0) {
        return;
    }
    
    if (pModel) {
        pModel->Draw(NULL);
    }
    
    if (unk6C == BOOMERANG_STATE_2 && !unk89) {
        mTrail.Draw();
    }
}

void Boomerang::DrawReflection(void) {
    if (unk6C != BOOMERANG_STATE_0 && pModelUnkF4) {
        pModelUnkF4->matrices[0] = pModel->matrices[0];

        pModelUnkF4->Draw(NULL);
    }
}

void Boomerang::DrawShadow(Vector* pVec) {
    if (unk6C == BOOMERANG_STATE_0 || (mRangType == BR_Doomerang && unk89)) {
        return;
    }

    boomerangShadow[mRangType].pModel = pModel;
    boomerangShadow[mRangType].Draw(pVec, false);
}

void Boomerang::Reset(void) {
    if (boomerangs.FindEntry(this)) {
        boomerangs.Destroy(this);
    }

    mSpline.Reset();
    
    time = 0.0f;

    unk84 = gDisplay.fps * GetDesc()->flightTime;

    unk88 = false;
    unk89 = false;
    unk54 = false;
    
    unk6C = BOOMERANG_STATE_0;

    mTrail.Reset();

    mFireTimer = 0;

    unkB0 = 1.0f;

    pModel->EnableSubObject(subObjectIndex, false);

    StopSounds();
}

void Boomerang::UpdateColor(void) {
    pModel->colour.x = mpWeapon->colour.x;
    pModel->colour.y = mpWeapon->colour.y;
    pModel->colour.z = mpWeapon->colour.z;

    if (pModelUnkF4) {
        pModelUnkF4->colour.x = mpWeapon->colour.x;
        pModelUnkF4->colour.y = mpWeapon->colour.y;
        pModelUnkF4->colour.z = mpWeapon->colour.z;
    }
}

void Boomerang::Load(void) {
    unk6C = BOOMERANG_STATE_1;
    InitLoaded();
}

void Boomerang::Fire(Vector* pVec1, Vector* pVec2) {
    int i;

    unk84 = GetDesc()->flightTime * gDisplay.fps;

    Vector up = {0.0f, 1.0f, 0.0f, 0.0f};

    Vector dir;

    if (pVec2) {
        dir.Sub(GetCatchPos(), pVec2);
    } else {
        dir.Inverse(pVec1);
    }

    Matrix rotation;

    Tools_BuildMatrixFromFwd(&rotation, &dir, &up);
    
    *rotation.Row3() = mOldPos = mPos = *GetCatchPos();

    Vector splinePoints[4];

    if (mpWeapon->mSide == BOOMERANG_SIDE_LEFT) {
        splinePoints[0].Set(0.0f, 0.0f, 0.0f);
        splinePoints[1].Set(-0.1333f, 0.0f, 1.0f);
        splinePoints[2].Set(-0.66f, 0.0666f, 0.2f);
        splinePoints[3].Set(0.0f, 0.0f, 0.0f);
    } else {
        splinePoints[0].Set(0.0f, 0.0f, 0.0f);
        splinePoints[1].Set(0.1333f, 0.0f, 1.0f);
        splinePoints[2].Set(0.66, 0.0666f, 0.2f);
        splinePoints[3].Set(0.0f, 0.0f, 0.0f);
    }

    mSpline.Reset();

    for (i = 0; i < 4; i++) {
        splinePoints[i].Scale(GetDesc()->range);
        mSpline.AddNode(&splinePoints[i]);
    }

    mSpline.Smooth();

    mSpline.mpPoints[1].unk10.y = 0.0f;

    if (pVec2 && dir.MagSquared() < Sqr<float>(GetDesc()->range)) {
        DoCurvedTargeting(dir.Magnitude());
    }

    for (i = 0; i < mSpline.nodeIndex; i++) {
        mSpline.mpPoints[i].mPos.ApplyMatrix(&rotation);
        mSpline.mpPoints[i].unk10.ApplyRotMatrix(&rotation);
    }

    boomerangs.AddEntry(this);

    unk6C = BOOMERANG_STATE_2;

    InitFired();
}

void Boomerang::Deactivate(void) {
    Reset();

    if (boomerangs.FindEntry(this)) {
        boomerangs.Destroy(this);
    }

    unk6C = BOOMERANG_STATE_0;
    InitIdle();
}

void Boomerang::InitFired(void) {
    pModel->EnableSubObject(subObjectIndex, true);
    unkA8.unk0 = PlaySound(BR_SOUND_0, 0);
}

extern "C" void Sound_Update3d(int, int, Vector*);

/// @brief Updates the boomerang in flight
/// @param None
void Boomerang::UpdateFired(void) {
    if (unkA8.unk0 != -1) {
        Sound_Update3d(unkA8.unk0, -1, &mPos);
    }

    time += 1.0f / unk84;
    UpdateSplineEndPosition();

    mOldPos = mPos;
    mPos.x += 0.01f;

    mPos = mSpline.GetPosition(Min<float>(time, 1.0f));

    if (!unk89) {

        // Check if a collision occurs
        CollisionResult cr;
        if (!unk54 && Collision_RayCollide(&mOldPos, &mPos, &cr, COLLISION_MODE_ALL, 0x8000 | 0x4000 | 0x100)) {

            if (!SendHitMessage(&cr)) {
                HitWorld(&cr.pos, cr.collisionFlags);
            }

            unk54 = true;
            CollideWithEnvironment(&cr);

            mPos = cr.pos;
        }

        if (unk54) {
            DoDeflection();
        }
    }

    SetOrientation(unk98, unk9C, 0.5f);
    
    ScaleRang();

    if ((1.0f - time) * unk84 <= mpWeapon->unkA8 && !unk88) {
        unk88 = true;
        PlaySound(BR_SOUND_1, 0);
        mpWeapon->StartCatch(this);
    }
}

void Boomerang::DoDeflection(void) {
    StopSounds();
    unk89 = true;
    mSpline.Reset();

    Vector halfWay;
    halfWay.Add(GetCatchPos(), &mPos);
    halfWay.Scale(0.5f);
    halfWay.y += 700.0f;

    mSpline.AddNode(&mPos);
    mSpline.AddNode(&halfWay);
    mSpline.AddNode(GetCatchPos());

    unk84 = 48.0f;
    time = 0.0f;
}

void Particle_Fire_Create(ParticleSystem**, Vector*, float, bool);

void Boomerang::HandleFire(void) {
    if (mFireTimer == 0) {
        return;
    }
    
    Vector rndFirePos;

    for (int i = 0; i < 3; i++) {
        float randAngle = RandomFR(&gb.mRandSeed, 0.0f, 2.0f * PI);
        float scale     = RandomFR(&gb.mRandSeed, 0.0f, 2.0f);

        rndFirePos.Set(
            scale * _table_sinf(randAngle),
            RandomI(&gb.mRandSeed) % 5,
            scale * _table_cosf(randAngle)
        );

        rndFirePos.Add(&mPos);

        Particle_Fire_Create(&pFireSys, &rndFirePos, 1.0f, true);
    }

    mFireTimer--;
}

void Boomerang::SetOrientation(float f1, float f2, float f3) {

    pModel->matrices[0].SetRotationYaw(f3 * gb.logicGameCount);

    pModel->matrices[0].RotateRoll(((mpWeapon->mSide == BOOMERANG_SIDE_LEFT) ? f1 : -f1) + -PI2);

    Vector dir;
    dir.Sub(&mOldPos, &mPos);

    Vector up = {0.0f, 1.0f, 0.0f, 0.0f};

    Matrix rotation;
    rotation.SetIdentity();

    Tools_BuildMatrixFromFwd(&rotation, &dir, &up);
    pModel->matrices[0].Multiply3x3(&rotation);

    OrientTowardsCamera(f2);
}

Vector* GameCamera_GetPos(void);

void Boomerang::OrientTowardsCamera(float weight) {
    Matrix rotation;
    Vector toRang;
    Vector cross;
    QuatRotation quat;

    // ASSERT(weight <= PI/2.f, "Boomerang::OrientTowardsCamera: exceeded maximum weight of 1.57", ...)

    toRang.Sub(&mPos, GameCamera_GetPos());
    toRang.Normalise();

    float f30 = pModel->matrices[0].Row1()->Dot(&toRang);

    if (Abs<float>(f30) > 0.9f) {
        return;
    }

    cross.Cross(pModel->matrices[0].Row1(), &toRang);
    cross.Normalise();

    quat.ConvertNormal(&cross, f30 > 0.0f ? (1.0f - f30) * weight : (-1.0f - f30) * weight);
    rotation.SetRotationQ(&quat);

    pModel->matrices[0].Multiply3x3(&rotation);
}

void Boomerang::InitIdle(void) {
    pModel->EnableSubObject(subObjectIndex, false);
}

void Boomerang::UpdateIdle(void) {

}

void Boomerang::InitLoaded(void) {

}

void Boomerang::UpdateLoaded(void) {

    pModel->matrices[0] = mpWeapon->catchMatrix;

    Vector attachPoint;
    pModel->GetRefPointWorldPosition(attachIdx, &attachPoint);

    attachPoint.Sub(pModel->matrices[0].Row3(), &attachPoint);

    pModel->matrices[0].Translate(&attachPoint);

    mPos = *pModel->matrices[0].Row3();
}

/// @brief Sends a boomerang hit message to the object that is in pCr
/// @param pCr CollisionResult that should be used to send the hit message
/// @return 
bool Boomerang::SendHitMessage(CollisionResult* pCr) {
    if (pCr && pCr->pInfo) {
        BoomerangMessage msg;

        MKProp* pProp = pCr->pInfo->pProp;

        msg.unk0 = MSG_BoomerangMsg;
        msg.pBoomerang = this;
        msg.SetUnk8(false);

        if (pProp) {
            pProp->Message(&msg);
            return msg.unk8;
        }
    }

    return false;
}

void Boomerang::UpdateSplineEndPosition(void) {
    mSpline.mpPoints[mSpline.nodeIndex - 1].mPos = *GetCatchPos();

    Vector newVel;

    newVel.Sub(&mSpline.mpPoints[mSpline.nodeIndex - 2].mPos, GetCatchPos());
    newVel.Scale(3.0f);
    newVel.Add(&mSpline.mpPoints[mSpline.nodeIndex - 2].unk10);
    newVel.Scale(-0.5f);

    mSpline.mpPoints[mSpline.nodeIndex - 1].unk10 = newVel;
}

Vector* Boomerang::GetCatchPos(void) {
    return mpWeapon->catchMatrix.Row3();
}

extern "C" double atan2(double, double);

void Boomerang::DoCurvedTargeting(float f1) {
    float range = GetDesc()->range;

    float f14 = mSpline.mpPoints[0].unk10.z;
    float f18 = mSpline.mpPoints[1].unk10.z;

    float val = 0.5f;
    for (int i = 0; i < 5; i++) {
        val = ((f1 - ((f14 * -2.0f + range * 3.0f) - f18) * val * val) -
            (((f14 + f18) - range * 2.0f) * val * val * val)) / f14;
    }

    if (val > 1.0f) {
        val = 1.0f;
    }

    Matrix rot;
    
    rot.SetRotationYaw(atan2(mSpline.GetPosition(val / 3.0f).x, f1));
    
    for (int i = 0; i < mSpline.nodeIndex; i++) {
        mSpline.mpPoints[i].mPos.ApplyRotMatrix(&rot);
        mSpline.mpPoints[i].unk10.ApplyRotMatrix(&rot);
    }
}

void Boomerang::UpdateTrail(void) {
    if (unk89) {
        return;
    }

    Vector vel;
    Vector cross;
    Vector top;
    Vector bottom;

    vel.Sub(&mPos, &mOldPos);

    cross.Cross(&vel, pModel->matrices[0].Row1());
    cross.Normalise();
    cross.Scale((unkB0 * mag) / 2.0f);
    
    top.Add(&mPos, &cross);

    bottom.Sub(&mPos, &cross);

    mTrail.Update(&top, &bottom);
}

void Boomerang::StopSounds(void) {
    unkA8.Stop();
}

void Doomerang::StopSounds(void) {
    Boomerang::StopSounds();
    mSoundHelper.Stop();
}

int Boomerang::PlaySound(BoomerangSound sound, int collisionFlags) {
    return SoundBank_Play(
        GetDesc()->mpStaticInfo->sounds[sound], 
        sound == BR_SOUND_1 ? NULL : &mPos, 
        collisionFlags
    );
}

void Boomerang::ScaleRang(void) {
    if (!unk56) {
        return;
    }

    float temp = ApproxMag(GameCamera_GetPos(), &mPos);
    float temp1 = ApproxMag(GameCamera_GetPos(), GetCatchPos());

    float m = Max<float>(0.0f, temp - temp1);
    unkB0 = m * 0.0004f + 1.0f;
    pModel->matrices[0].Scale(unkB0);
}

struct TyParticleManager {
    char unk[0x44];
    void SpawnBridgeChunk(Vector*, Model*);
    void SpawnWaterRipple(Vector*, float);
    void SpawnLeafGrassDust(Vector*, Vector*, bool);
    void SpawnBigSplash(Vector*, bool, float, bool, float, int);
    void SpawnSpark(Vector*);
    void SpawnWaterSteam(Vector*, float);
    void SpawnElectricity(Vector*, float);
    void SpawnShockGlow(Vector*, float);
};

extern TyParticleManager* particleManager;

/// @brief 
/// @param pPos Position the collision occurred
/// @param collisionFlags Collision flags of the collision
void Boomerang::HitWorld(Vector* pPos, int collisionFlags) {
    unk54 = true;

    if (unk89) {
        return;
    }

    PlaySound(BR_SOUND_2, collisionFlags);

    if (collisionFlags & ID_WATER_BLUE) {
        particleManager->SpawnBigSplash(pPos, true, 0.3f, true, 1.7f, 6);
    } else {
        particleManager->SpawnSpark(pPos);
    }
}

Boomerang* Boomerang_CheckForHitSphere(Vector* pVec, float radius, bool r4) {
    Boomerang** ppBoomerangs = boomerangs.GetPointers();

    while (*ppBoomerangs) {
        if (r4 || !(*ppBoomerangs)->unk54) {
            if (RayToSphere(&(*ppBoomerangs)->mOldPos, &(*ppBoomerangs)->mPos, pVec, radius + 8.0f, -1.0f, true)) {
                return *ppBoomerangs;
            }
        }

        ppBoomerangs++;
    }

    return NULL;
}

Boomerang* Boomerang_CheckForHit(Model* pModel, int subobjectIdx, CollisionResult* pCr) {
    Boomerang** ppBoomerangs = boomerangs.GetPointers();

    while (*ppBoomerangs) {
        if (!(*ppBoomerangs)->unk54) {
            if (Collision_RayCollideDynamicModel(
                    &(*ppBoomerangs)->mOldPos, &(*ppBoomerangs)->mPos, pCr, pModel, subobjectIdx)) {
                return *ppBoomerangs;
            }
        }

        ppBoomerangs++;
    }

    return NULL;
}

Boomerang* Boomerang_CheckForHit(Vector* p, float f1, float f2, float f3) {
    Boomerang** ppBoomerangs = boomerangs.GetPointers();

    while (*ppBoomerangs) {
        if (!(*ppBoomerangs)->unk54) {

            Vector sp8 = *p;
            sp8.y -= f3 + 8.0f;
            
            if (Tools_RayToVertCyl(
                    &(*ppBoomerangs)->mOldPos, &(*ppBoomerangs)->mPos, &sp8, f2 + 8.0f, f1 + 16.0f)) {
                return *ppBoomerangs;
            }
        }

        ppBoomerangs++;
    }

    return NULL;
}

/// @brief Finds the closest boomerang to pPoint
/// @param pPoint Position to test for closest boomerang
/// @return Pointer to closest boomerang, may be NULL
Boomerang* Boomerang_FindClosest(Vector* pPoint) {
    
    Boomerang* pClosestRang = NULL;
    float closestDist = 1e+08f;

    Boomerang** ppBoomerangs = boomerangs.GetPointers();

    while (*ppBoomerangs) {
        if (!(*ppBoomerangs)->unk54) {
            float dist = SquareDistance(&(*ppBoomerangs)->mPos, pPoint);
            if (dist < closestDist) {
                pClosestRang = *ppBoomerangs;
                closestDist = dist;
            }
        }
        ppBoomerangs++;
    }
    
    return pClosestRang;
}

void Boomerang_LoadResources(KromeIni* pIni) {
    boomerangs.Init(50);

    for (int i = 0; i < NUM_BOOMERANGS; i++) {

        boomerangDescriptors[i].Init(
            pBoomerangModules[i], 
            boomerangInfo[i].pModelName, 
            boomerangInfo[i].pModelName, 
            (BoomerangType)i, 
            &boomerangInfo[i]
        );

        boomerangDescriptors[i].Load(pIni);

        char shadowName[32];
        strcpy(shadowName, boomerangDescriptors[i].modelName);
        strcat(shadowName, "shadow");
        boomerangShadow[i].Init(shadowName, NULL, 250.0f, NULL);

        Boomerang_pDescriptors[i] = &boomerangDescriptors[i];
    }

    endGameObjectiveDesc.Init(
        &endGameObjectiveModuleInfo,
        "EndGameObjective",
        "EndGameObjective",
        0x200,
        0
    );
    
    objectManager.AddDescriptor(&endGameObjectiveDesc);
}

void Boomerang_Init(void) {
    boomerangs.Reset();
    SleepyDust_Init();
}

void Boomerang_Deinit(void) {
    boomerangs.Reset();
    SleepyDust_Deinit();
}

void Boomerang_Reset(void) {
    boomerangs.Reset();
    SleepyDust_Reset();
}

void Boomerang_Draw(void) {
    Boomerang** ppBoomerangs = boomerangs.GetPointers();
    while (*ppBoomerangs) {
        (*ppBoomerangs)->Draw();
        ppBoomerangs++;
    }
}

/// @brief Gets the next valid boomerang to change to
/// @param type Current boomerang type to change from
/// @return Type of boomerang
BoomerangType GetNextRang(BoomerangType type) {
    int i;

    // find the index of the current type within the change order
    for (i = 0; i < NUM_BOOMERANGS; i++) {
        if (boomerangChangeOrder[i] == type) {
            break;
        }
    }

    bool bFoundValidRang = false;
    while (!bFoundValidRang) {
        // loop until a valid boomerang is found
        i++; // don't test the current type, the first iteration
        if (i >= NUM_BOOMERANGS) {
            i -= NUM_BOOMERANGS;
        }
        
        type = boomerangChangeOrder[i];

        // Prevent switching the the doomerang outside of the final battle
        if (type != BR_Doomerang || gb.level.GetCurrentLevel() == LN_FINAL_BATTLE) {
            if (gb.mGameData.HasBoomerang(type)) {
                // Only allow changing to boomerangs which have been unlocked in the save file
                bFoundValidRang = true;
            }
        }
    }

    return type;
}

/// @brief Gets the next previous valid boomerang to change to
/// @param type Current boomerang type to change from
/// @return Type of boomerang
BoomerangType GetPrevRang(BoomerangType type) {
    int i;

    // find the index of the current type within the change order
    for (i = 0; i < NUM_BOOMERANGS; i++) {
        if (boomerangChangeOrder[i] == type) {
            break;
        }
    }

    bool bFoundValidRang = false;
    while (!bFoundValidRang) {
        // loop until a valid boomerang is found
        i--; // don't test the current type in the first iteration
        if (i < 0) {
            i += NUM_BOOMERANGS;
        }

        type = boomerangChangeOrder[i];
        
        // Prevent switching to the doomerang outside of the final battle
        if (type != BR_Doomerang || gb.level.GetCurrentLevel() == LN_FINAL_BATTLE) {
            if (gb.mGameData.HasBoomerang(type)) {
                // Only allow changing to boomerangs which have been unlocked in the save file
                bFoundValidRang = true;
            }
        }
    }

    return type;
}

char* Boomerang_GetName(BoomerangType type) {
    // ASSERT(
    //     type >= 0 && type < BR_Max,
    //     "This is not an actual boomerang type!!",
    //     "Source\\boomerang.cpp",
    //     1373,
    // );

    return gpTranslation_StringArray[boomerangInfo[type].nameTranslationId];
}

char* Boomerang_GetModelName(BoomerangType type) {
    // ASSERT(
    //     type >= 0 && type < BR_Max,
    //     "This is not an actual boomerang type!!",
    //     "Source\\boomerang.cpp",
    //     1389,
    // );

    return boomerangInfo[type].pModelName;
}

char* Boomerang_GetDescription(BoomerangType type) {
    // ASSERT(
    //     type >= 0 && type < BR_Max,
    //     "This is not an actual boomerang type!!",
    //     "Source\\boomerang.cpp",
    //     1404
    // );

    return gpTranslation_StringArray[boomerangInfo[type].descriptionTranslationId];
}

void Flamerang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    Boomerang::Init(pDesc, pWeapon);

    Particle_Fire_Init(&mpFlameSys, &mPos, &rangVol, 2.0f, false);

    unk94 = 0x37;
    bSpawnFlameParticles = false;

    mFlame.Init(&mPos, &rangVol, 2.0f, 25.0f);
}

void Flamerang::Deinit(void) {
    if (mpFlameSys) {
        Particle_DestroyASystem(&mpFlameSys, 0.0f);
    }

    mpFlameSys = NULL;
    bSpawnFlameParticles = false;

    mFlame.Deinit();

    Boomerang::Deinit();
}

void Particle_Fire_CreateExplosion(Vector*, float);

/// @brief 
/// @param pPos Position the collision occurred
/// @param collisionFlags Collision flags of the collision
void Flamerang::HitWorld(Vector* pPos, int collisionFlags) {
    unk54 = true;

    if (unk89) {
        return;
    }

    PlaySound(BR_SOUND_2, collisionFlags);

    if (collisionFlags & ID_WATER_BLUE) {
        particleManager->SpawnWaterSteam(pPos, 30.0f);
        particleManager->SpawnBigSplash(pPos, true, 0.1f, true, 1.0f, 2);
        SoundBank_Play(30, pPos, 0);
    } else {
        Particle_Fire_CreateExplosion(pPos, 2.0f);
    }
}

void Flamerang::ModifyRangeInFlight(float f1) {
    if (time > 0.5f) {
        return;
    }

    Vector dir;

    dir.Sub(&mSpline.mpPoints[1].mPos, &mSpline.mpPoints[0].mPos);
    dir.Normalise();
    dir.Scale(f1);

    mSpline.mpPoints[1].mPos.Add(&mSpline.mpPoints[0].mPos, &dir);
    mSpline.mpPoints[0].mPos = mPos;

    unk84 *= 1.0f - time;
    time = 0.0f;
}

void Flamerang::UpdateParticleEffect(void) {
    Vector endPos = mPos;

    if (unk6C == BOOMERANG_STATE_1) {
        pModel->GetRefPointWorldPosition(rang01Idx, &endPos);

        Particle_Fire_Create(&mpFlameSys, &endPos, 0.5f, false);

        if (RandomIR(&gb.mRandSeed, 0, 4) == 0) {
            Particle_Fire_Create(&pFireSys, &endPos, 0.5f, true);
        }
    } else if ((gb.logicGameCount & 1) != 0 && !unk89) {
        Particle_Fire_Create(&mpFlameSys, &mPos, 2.0f, false);

        Vector ttt;

        ttt = mPos;

        ttt.x -= RandomFR(&gb.mRandSeed, 10.0f, 30.0f);
        ttt.y -= RandomFR(&gb.mRandSeed, 10.0f, 30.0f);
        ttt.z -= RandomFR(&gb.mRandSeed, 10.0f, 30.0f);

        Particle_Fire_Create(&pFireSys, &ttt, 1.0f, true);
    }

    if (mpWeapon->mSide == BOOMERANG_SIDE_LEFT) {
        ty.rangLight0 = endPos;
    } else {
        ty.rangLight1 = endPos;
    }

    if (bSpawnFlameParticles) {
        Vector temp = {0.0f, 0.0f, 0.0f, 0.0f};
        mFlame.Create(&temp, 0.0f, NULL);
    }
}

void Flamerang::Deactivate(void) {
    Boomerang::Deactivate();
    bSpawnFlameParticles = false;
}

void Particle_Ice_Init(ParticleSystem**, Vector*);

void Frostyrang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    Boomerang::Init(pDesc, pWeapon);

    Particle_Ice_Init(&mpParticleSys, &mPos);

    unk94 = 0x36;
}

void Frostyrang::Deinit(void) {
    if (mpParticleSys) {
        Particle_DestroyASystem(&mpParticleSys, 0.0f);
    }

    mpParticleSys = NULL;

    Boomerang::Deinit();
}

/// @brief 
/// @param pPos Position the collision occurred
/// @param collisionFlags Collision flags of the collision
void Frostyrang::HitWorld(Vector* pPos, int collisionFlags) {
    unk54 = true;

    if (unk89) {
        return;
    }

    PlaySound(BR_SOUND_2, collisionFlags);

    if (collisionFlags & 0x200) {
        particleManager->SpawnWaterSteam(pPos, 30.0f);
        SoundBank_Play(0x1E, pPos, 0);
    } else if ((collisionFlags & ID_WATER_BLUE) == 0) {
        particleManager->SpawnSpark(pPos);
    }
}

void Particle_Ice_Create(ParticleSystem**, Vector*, Vector*, float, float);

void Frostyrang::UpdateParticleEffect(void) {
    Vector endPos;

    if (unk6C == BOOMERANG_STATE_1) {
        pModel->GetRefPointWorldPosition(rang01Idx, &endPos);

        if (RandomIR(&gb.mRandSeed, 0, 9) == 0) {
            Vector vel;
            vel.Set(
                RandomFR(&gb.mRandSeed, -0.3f, 0.3f),
                0.0f,
                RandomFR(&gb.mRandSeed, -0.3f, 0.3f)
            );

            Particle_Ice_Create(&mpParticleSys, &endPos, &vel, RandomFR(&gb.mRandSeed, 6.0f, 14.0f), 1.8f);
        }

        // if (mpWeapon->mSide == BOOMERANG_SIDE_LEFT) {
        //     ty.rangLight0 = endPos;
        // } else {
        //     ty.rangLight1 = endPos;
        // }
    } else if ((gb.logicGameCount & 1) != 0 && !unk89) {
        Vector npos;

        npos = mPos;

        npos.x += RandomFR(&gb.mRandSeed, -20.0f, 20.0f);
        npos.y += RandomFR(&gb.mRandSeed, -20.0f, 20.0f);
        npos.z += RandomFR(&gb.mRandSeed, -20.0f, 20.0f);

        Vector nvel;
        nvel.SetZero();

        Particle_Ice_Create(&mpParticleSys, &npos, &nvel, RandomFR(&gb.mRandSeed, 8.0f, 16.0f), 1.8f);
    }
}

struct IceBlockStruct;
void IceBlock_Add(Vector*, Vector*, CollisionResult*, IceBlockStruct**);

void Frostyrang::CollideWithEnvironment(CollisionResult* pCr) {
    if (!TestColInfoFlag(pCr, 2) && !(pCr->collisionFlags & 0x200)) {
        IceBlock_Add(&pCr->pos, &pCr->normal, pCr, NULL);
    }
}

static Kaboomerang* pOtherKaboomerang = NULL;
static Matrix kMatrix;
static float kAngle = 0.0f;

void Kaboomerang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    Boomerang::Init(pDesc, pWeapon);

    mExplosion.Init(&mPos, false, 1.0f);

    unk94 = 0x38;
}

void Kaboomerang::Deinit(void) {
    mExplosion.Deinit();

    Boomerang::Deinit();
}

void Kaboomerang::Fire(Vector* pVec1, Vector* pVec2) {
    Vector dir;

    if (pVec2) {
        dir.Sub(GetCatchPos(), pVec2);
    } else {
        dir.Inverse(pVec1);
    }

    mKaboomerangFlightTime = 0.0f;

    mPos = mOldPos = *GetCatchPos();

    boomerangs.AddEntry(this);

    kAngle = -PI2;

    if (pOtherKaboomerang) {
        mpOtherKaboomerang = pOtherKaboomerang;
        mpOtherKaboomerang->mpOtherKaboomerang = this;

        pOtherKaboomerang = NULL;

        mCentrePos.Add(&mPos, &mpOtherKaboomerang->mPos);
        mCentrePos.Scale(0.5f);

        mpOtherKaboomerang->mCentrePos = mCentrePos;
        
        Tools_BuildMatrixFromFwd(&kMatrix, &dir, &gYAxis);

        kMatrix.SetTranslation(&mCentrePos);
        mpOtherKaboomerang->boomerangCatchDistance = 
            boomerangCatchDistance = ApproxMag(&mCentrePos, &mPos);
    } else {
        pOtherKaboomerang = this;
    }

    unk6C = BOOMERANG_STATE_2;
    InitFired();
}

/// @brief Updates the boomerang in flight
/// @param None
void Kaboomerang::UpdateFired(void) {
    if (unkA8.unk0 != -1) {
        Sound_Update3d(unkA8.unk0, -1, &mPos);
    }

    // Both kaboomerangs move with each other 
    // and rotate around their center position

    float radius = (boomerangCatchDistance * (1.0f - mKaboomerangFlightTime));
    radius += SmoothCenteredCurve(mKaboomerangFlightTime) * 100.0f;
    
    kAngle += 0.06f;

    // offset the rotation of the right kaboomerang by 126 degrees
    float angle = mpWeapon->mSide == BOOMERANG_SIDE_LEFT ? kAngle : kAngle + (126.0f * (PI / 180.0f));

    // operator order ps2?
    Vector localPos = {
        _table_sinf(angle) * radius,
        _table_cosf(angle) * (radius * 0.5f),
        mKaboomerangFlightTime * GetDesc()->range
    };

    mOldPos = mPos;
    mPos.ApplyMatrix(&localPos, &kMatrix);

    Vector oldCentrePos = mCentrePos;

    mCentrePos.Set(
        0.0f, 0.0f, GetDesc()->range * mKaboomerangFlightTime // operation order ps2?
    );

    mCentrePos.ApplyMatrix(&kMatrix);

    if (!unk88) {
        CollisionResult cr;
        bool b = false;
        if (Collision_RayCollide(&oldCentrePos, &mCentrePos, &cr, COLLISION_MODE_ALL, 0x8000 | 0x4000 | 0x100)) {
            b = SendHitMessage(&cr);
            unk54 = true;
            mPos = cr.pos;
        }

        mKaboomerangFlightTime += 1.0f / unk84;

        if (unk54 || mKaboomerangFlightTime > 1.0f) {
            
            // Send explosion message to all props near this
            ExplosionMessage msg;
            msg.Init(&mPos);
            objectManager.SendMessage(&msg, 0, &mPos, 1000.0f, false);

            if (!b) {
                PlaySound(BR_SOUND_3, 0);
            }

            mExplosion.Explode(&mPos, 0.0f);
            unk88 = true;
            mpOtherKaboomerang->unk88 = true;
            mpOtherKaboomerang->mpWeapon->StartCatch(mpOtherKaboomerang);
            mpWeapon->StartCatch(this);
        }
    }

    SetOrientation(unk98, 0.0f, 0.5f);
    ScaleRang();
}

void Zappyrang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    Boomerang::Init(pDesc, pWeapon);

    unk94 = 0x3D;
}

void Zappyrang::CollideWithEnvironment(CollisionResult* pCr) {
    CollisionInfo* pInfo = pCr->pInfo;

    if (pInfo && pInfo->pProp) {
        MKMessage msg = {MSG_UNK_20};
        pInfo->pProp->Message(&msg);
    }
}

void Zappyrang::UpdateParticleEffect(void) {
    pModel->GetRefPointWorldPosition(rang01Idx, &unkFC);

    if ((gb.logicGameCount % 10) == 0) {
        particleManager->SpawnElectricity(&unkFC, 7.0f);
    }
}

void Aquarang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    Boomerang::Init(pDesc, pWeapon);

    mBubble.Init(&mPos, &rangVol, 1.0f);
}
void Aquarang::InitFired(void) {
    pModel->EnableSubObject(subObjectIndex, true);
    PlaySound(BR_SOUND_0, 0);
}

/// @brief Updates the boomerang in flight
/// @param None
void Aquarang::UpdateFired(void) {
    if (unkA8.unk0 != -1) {
        Sound_Update3d(unkA8.unk0, -1, &mPos);
    }

    time += 1.0f / unk84;

    UpdateSplineEndPosition();

    mOldPos = mPos;

    mPos = mSpline.GetPosition(Min<float>(time, 1.0f));

    if (!unk89) {
        CollisionResult cr;
        cr.normal.SetZero();

        if (!unk54 && Collision_RayCollide(&mOldPos, &mPos, &cr, COLLISION_MODE_ALL, 0x8000 | 0x4000 | 0x100)) {
            if (!SendHitMessage(&cr)) {
                HitWorld(&cr.pos, cr.collisionFlags);
            }

            unk54 = true;
            CollideWithEnvironment(&cr);
            mPos = cr.pos;
        }

        if (unk54) {
            unk89 = true;

            Vector temp;
            temp.Sub(&mPos, &mOldPos);

            if (!cr.normal.x && !cr.normal.y && !cr.normal.z) {
                cr.normal.Inverse(&temp);
                cr.normal.Normalise();
                cr.normal.Scale(0.7f);
                cr.normal.y = 0.7f;
            }

            Vector deflectPos;
            deflectPos = cr.normal;
            deflectPos.Scale(temp.Dot(&cr.normal) * 2.0f);

            temp.Subtract(&deflectPos);
            temp.Scale(15.0f);

            deflectPos.Add(&temp, &mPos);

            temp = cr.normal;
            temp.Scale(15.0f);
            temp.Add(&mPos);

            if (Collision_RayCollide(&temp, &deflectPos, &cr, COLLISION_MODE_ALL, 0x8000 | 0x4000 | 0x100)) {
                deflectPos = cr.normal;
                deflectPos.Scale(10.0f);
                deflectPos.Add(&cr.pos);
            }

            Vector returnPos;

            if (mpWeapon->mSide == BOOMERANG_SIDE_RIGHT) {
                returnPos.Scale(ty.pModel->matrices[0].Row0(), -350.0f);
                returnPos.Add(GetCatchPos());
            } else {
                returnPos.Scale(ty.pModel->matrices[0].Row0(), 350.0f);
                returnPos.Add(GetCatchPos());
            }

            temp.Scale(ty.pModel->matrices[0].Row2(), 100.0f);
            returnPos.Add(&temp);

            mSpline.Reset();
            mSpline.AddNode(&mPos);
            mSpline.AddNode(&deflectPos);
            mSpline.AddNode(&returnPos);
            mSpline.AddNode(GetCatchPos());

            unk84 = 48.0f;
            time = 0.0f;
        }
    }

    SetOrientation(unk98, unk9C, 0.5f);

    ScaleRang();

    if ((1.0f - time) * unk84 <= mpWeapon->unkA8 && !unk88) {
        unk88 = true;
        PlaySound(BR_SOUND_1, 0);
        mpWeapon->StartCatch(this);
    }
}

void Aquarang::UpdateParticleEffect(void) {
    if ((gb.logicGameCount & 1) == 0 && unk6C != BOOMERANG_STATE_1) {
        Vector ttt = mPos;

        ttt.x += RandomFR(&gb.mRandSeed, -15.0f, 15.0f);
        ttt.y += RandomFR(&gb.mRandSeed, -15.0f, 15.0f);
        ttt.z += RandomFR(&gb.mRandSeed, -15.0f, 15.0f);

        mBubble.Create(&ttt, RandomFR(&gb.mRandSeed, 2.0f, 10.0f), ty.mContext.water.pos.y, 1.0f, 0.0f);
    }
}

void Aquarang::Deinit(void) {
    mBubble.Deinit();

    Boomerang::Deinit();
}

void Megarang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    Boomerang::Init(pDesc, pWeapon);

    unk94 = 0x3A;
}

void Megarang::UpdateLoaded(void) {
    Boomerang::UpdateLoaded();

    if (mpWeapon->mSide == BOOMERANG_SIDE_LEFT) {
        ty.mAutoTarget.unk74.unk54.Scale(&ty.mAutoTarget.tyRot, 750.0f);
        ty.mAutoTarget.unk74.unk54.Add(&ty.pos);
        ty.mAutoTarget.unk74.unk54.y += ty.radius;
    } else {
        ty.mAutoTarget.unkD8.unk54.Scale(&ty.mAutoTarget.tyRot, 750.0f);
        ty.mAutoTarget.unkD8.unk54.Add(&ty.pos);
        ty.mAutoTarget.unkD8.unk54.y += ty.radius;
    }
}

void Megarang::Fire(Vector* pVec1, Vector* pVec2) {

    AutoVisible vis = mpWeapon->mSide == BOOMERANG_SIDE_LEFT ?
        ty.mAutoTarget.unk74 : ty.mAutoTarget.unkD8;

    if (vis.numItems != 0) {
        int i;

        int closestIndex = -1;
        
        if (pVec2) {
            for (i = 0; i < vis.numItems; i++) {
                Vector target = *vis.unk0[i];
                target.y += vis.unk28[i];
                if (target.IsInsideSphere(pVec2, 20.0f)) {
                    closestIndex = i;
                    break;
                }
            }
        }

        if (closestIndex == -1) {
            float closestDist = Sqr<float>(10000.0f);

            for (i = 0; i < vis.numItems; i++) {
                float newDist = SquareDistance(vis.unk0[i], &vis.unk54);
                if (newDist < closestDist) {
                    closestDist = newDist;
                    closestIndex = i;
                }
            }
        }

        unk100 = vis.unk28[closestIndex];
        unk108 = vis.unk0[closestIndex];

        mSpline.Reset();
        mSpline.AddNode(GetCatchPos());

        mOldPos = mPos = *GetCatchPos();

        Vector sp98 = *unk108;
        sp98.y += unk100;

        mSpline.AddNode(&sp98);

        unk84 = (unk104 * 0.5f) * ApproxMag(GetCatchPos(), &sp98);

        if (pVec2 == NULL) {
            mSpline.mpPoints[0].unk10.Scale(pVec1, 750.0f);
        }

        UpdateEndPosition();

        boomerangs.AddEntry(this);

        unk6C = BOOMERANG_STATE_2;

        InitFired();
    } else {
        Boomerang::Fire(pVec1, pVec2);
        unk108 = NULL;
    }
}

View* GameCamera_View(void);

void Megarang::UpdateFired(void) {

    AutoVisible vis = mpWeapon->mSide == BOOMERANG_SIDE_LEFT ?
        ty.mAutoTarget.unk74 : ty.mAutoTarget.unkD8;
    
    if (unkA8.unk0 != -1) {
        Sound_Update3d(unkA8.unk0, -1, &mPos);
    }

    time += 1.0f / unk84;

    if (time > 1.0f) {
        time = 1.0f;
        unkFC = 0;
        unk54 = true;
    }

    mOldPos = mPos;

    mPos = mSpline.GetPosition(time);

    unkFC--;

    if (unkFC < 0 && !unk89) {
        CollisionResult cr;

        bool r31 = false;

        if (!unk54 && Collision_RayCollide(&mOldPos, &mPos, &cr, COLLISION_MODE_ALL, 0x8000 | 0x4000 | 0x100)) {
            if (!SendHitMessage(&cr)) {
                HitWorld(&cr.pos, cr.collisionFlags);
                r31 = true;
            }

            unk54 = true;
            mPos = cr.pos;
            CollideWithEnvironment(&cr);
        }

        if (unk54) {
            if (unk108) {
                Vector glowPos = *unk108;
                glowPos.y += unk100;
                Vector tmp = *GameCamera_View()->unk48.Row2();
                tmp.Scale(-50.0f);
                particleManager->SpawnShockGlow(&glowPos, 128.0f);
            }

            unk10C++;
            if (!r31) {
                unkFC = 8;
            }

            bool found = false;
            Vector* pFoundVec = NULL;

            for (int i = 0; i < vis.numItems; i++) {
                if (vis.unk0[i] != unk108) {
                    found = true;
                    unk100 = vis.unk28[i];
                    pFoundVec = vis.unk0[i];

                    if (mpWeapon->mSide == BOOMERANG_SIDE_LEFT) {
                        break;
                    }
                }
            }

            unk108 = pFoundVec;

            if (!found || unk10C >= 10) {
                DoDeflection();
                unk108 = NULL;
            } else {
                mSpline.Reset();
                mSpline.AddNode(&mPos);
                mPos = mOldPos;

                Vector target = *unk108;
                target.y += unk100;

                mSpline.AddNode(&target);

                float mag = ApproxMag(&target, &mOldPos);

                if (r31) {
                    Vector vel = mSpline.GetVelocity(Min<float>(time, 1.0f));

                    float dot = cr.normal.Dot(&vel);
                    cr.normal.Scale(dot * -2.0f);

                    vel.Add(&cr.normal);
                    mSpline.mpPoints[0].unk10 = vel;
                } else {
                    mSpline.mpPoints[0].unk10.y += mag * 0.5f;
                }

                unk84 = unk104 * mag;
                time = 0.0f;
            }
        }

    }

    if (unkFC <= 0) {
        unk54 = false;
    }

    UpdateEndPosition();

    SetOrientation(unk98, unk9C, 0.5f);

    ScaleRang();

    if (!unk108 && (1.0f - time) * unk84 <= mpWeapon->unkA8 && !unk88) {
        unk88 = true;
        PlaySound(BR_SOUND_1, 0);
        mpWeapon->StartCatch(this);
    }
}

void Megarang::Reset(void) {
    Boomerang::Reset();

    unk10C = 0;
    unk108 = NULL;
    unk104 = 0.05f;
    unkFC = 0;
}

void Megarang::UpdateEndPosition(void) {
    int prevIndex = mSpline.nodeIndex - 1;

    if (unk108) {
        mSpline.mpPoints[prevIndex].mPos = *unk108;
        mSpline.mpPoints[prevIndex].mPos.y += unk100;
    } else {
        mSpline.mpPoints[prevIndex].mPos = *GetCatchPos();
    }

    mSpline.mpPoints[prevIndex].unk10.Sub(
        &mSpline.mpPoints[prevIndex].mPos,
        &mSpline.mpPoints[prevIndex - 1].mPos
    );

    mSpline.mpPoints[prevIndex].unk10.Scale(3.0f);

    mSpline.mpPoints[prevIndex].unk10.Subtract(
        &mSpline.mpPoints[prevIndex - 1].unk10
    );

    mSpline.mpPoints[prevIndex].unk10.Scale(0.5f);
}

void Megarang::HitWorld(Vector* pPos, int collisionFlags) {
    unk54 = true;

    if (!unk89 && unkFC <= 0) {
        PlaySound(BR_SOUND_2, collisionFlags);

        if (collisionFlags & 0x400) {
            particleManager->SpawnBigSplash(pPos, true, 0.3f, true, 1.7f, 6);
        } else {
            particleManager->SpawnSpark(pPos);
        }
    }
}

static float speedScale = 1.0f;
static int numHits = 0;

void Doomerang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    mSoundHelper.Init();

    Boomerang::Init(pDesc, pWeapon);

    mExplosion.Init(&mPos, false, 0.1f);

    numHits = 0;
    speedScale = 1.0f;

    unkFC.SetZero();
    unk10C.SetZero();
    unk11C.SetZero();

    int refPointIndex1;
    int refPointIndex2;
    int refPointIndex3;

    if (gb.level.GetCurrentLevel() == LN_FINAL_BATTLE) {
        for (int i = 0; i < gb.level.nmbrOfLayers; i++) {
            Model* pLayerModel = gb.level.layers[i].pModel;
            if (pLayerModel) {
                if (pLayerModel->RefPointExists("R_Doomerang1", &refPointIndex1)) {
                    pLayerModel->GetRefPointWorldPosition(refPointIndex1, &unkFC);
                }

                if (pLayerModel->RefPointExists("R_Doomerang2", &refPointIndex2)) {
                    pLayerModel->GetRefPointWorldPosition(refPointIndex2, &unk10C);
                }

                if (pLayerModel->RefPointExists("R_Doomerang3", &refPointIndex3)) {
                    pLayerModel->GetRefPointWorldPosition(refPointIndex3, &unk11C);
                }
            }
        }
    }

    unk538 = false;
}

void Doomerang::Deinit(void) {
    mExplosion.Deinit();

    Boomerang::Deinit();

    pInAirDoomerang = NULL;

    mSoundHelper.Deinit();
}

void Doomerang::Fire(Vector* pVel, Vector* p1) {
    unk130 = 0.0f;
    unk84 = 2000000.0f;

    velocity = *pVel;

    if (p1) {
        velocity.Sub(p1, GetCatchPos());
    }

    velocity.Normalise();

    unk12C = atan2(velocity.y, sqrtf(Sqr<float>(velocity.x) + Sqr<float>(velocity.z)));
    velocity.Scale(GetDesc()->speed * speedScale);

    mOldPos = mPos = *GetCatchPos();

    if (ty.mFsm.GetStateEx() != 0x1C) {
        ty.mFsm.SetState((HeroActorState)0x32, false);

        boomerangs.AddEntry(this);

        unk6C = BOOMERANG_STATE_2;

        InitFired();
    }
}

void Doomerang::InitFired(void) {
    pModel->EnableSubObject(subObjectIndex, true);

    unkA8.unk0 = SoundBank_Play(0x1A, NULL, 0);

    pInAirDoomerang = this;

    boomerangShadow[mRangType].unk4 *= 4.0f;

    unk538 = false;

    unk94 = 0x39;

    unk539 = false;
}

void GameCamera_UseDoomarangCamera(bool, Vector*, Vector*, Vector*, float);

/// @brief Updates the boomerang in flight
/// @param None
void Doomerang::UpdateFired(void) {
    if (!unk538 && !unk89) {
        CheckForEnteringRegions();
    }

    if (!unk538) {
        unk130 += 1.0f;

        float f31 = 0.0f;

        if (!unk89) {
            if (unkA8.unk0 <= -1 && gb.pDialogPlayer == NULL) {
                unkA8.unk0 = SoundBank_Play(26, NULL, 0);
            }

            float old12C = unk12C;
            f31 = gb.mJoyPad1.GetUnk58() * 0.5f;

            unk12C = Clamp<float>(
                -1.3f, 
                (gb.mJoyPad1.GetUnk5C() * 0.001f) * GetDesc()->pitchSpeed * speedScale + unk12C, // some diff here in aug 8
                // unk12C + GetDesc()->pitchSpeed * speedScale * (gb.mJoyPad1.GetUnk5C() * 0.001f),
                1.3f
            );

            Matrix mat;
            mat.SetIdentity();
            mat.RotatePitch(unk12C - old12C);
            mat.RotateYaw(gb.mJoyPad1.GetUnk58() * -0.001f * GetDesc()->turnSpeed * speedScale);

            Vector up = {0.0f, 1.0f, 0.0f};

            velocity.Inverse();

            Matrix m;
            Tools_BuildMatrixFromFwd(&m, &velocity, &up);

            mat.Multiply3x3(&m);

            velocity.Set(0.0f, 0.0f, GetDesc()->speed * speedScale);
            velocity.ApplyRotMatrix(&mat);

            pModel->matrices[0].SetIdentity();
            pModel->matrices[0].SetRotationYaw(-unk130 * 0.3f);
            pModel->matrices[0].RotateRoll(f31);
            pModel->matrices[0].Multiply3x3(&m);

            mOldPos = mPos;
            mPos.Add(&velocity);

            CollisionResult cr;

            if (Collision_SweepSphereCollide(
                    &mOldPos, &mPos, 45.0f, &cr, COLLISION_MODE_ALL, 0x8000 | 0x4000 | 0x100)) {
                
                VibrateJoystick(1.0f, 0.0f, gDisplay.dt, 0, 4.0f);
                if ((gb.logicGameCount % 3) == 0) {
                    Vector box = mPos;
                    particleManager->SpawnSpark(Tools_RandomBox(&box, 10.0f));
                }

                mSoundHelper.Update(0x23B, false, true, NULL, NULL, -1.0f, 0);
            } else {
                mSoundHelper.Update(0x23B, false, false, NULL, NULL, -1.0f, 0);
            }

            if (Collision_RayCollide(&mOldPos, &mPos, &cr, COLLISION_MODE_ALL, 0x8000 | 0x4000 | 0x100)) {
                mSoundHelper.Stop();

                if ((cr.collisionFlags & 0x8000000) != 0 || (cr.collisionFlags & 0x20) != 0) {
                    mpWeapon->StartCatch(this);
                    unk88 = true;
                } else {
                    SendHitMessage(&cr);
                    unk89 = true;

                    mExplosion.Explode(&mPos, 0.0f);

                    unk134 = unk130;

                    pModel->colour.w = 0.0f;

                    unkA8.Stop();

                    SoundBank_Play(0x7A, NULL, 0);
                }
            }
        }

        if (unk130 > gDisplay.fps * 0.25f && !unk88) {
            GameCamera_UseDoomarangCamera(
                true, 
                &mPos, &velocity, 
                pModel->matrices[0].Row1(),
                f31
            );
        }

        if (!unk88 && unk89 && (
                (unk130 - unk134 > gDisplay.fps * 1.3f) || (unk130 < gDisplay.fps * 0.45f)
            )) {
            if (unk539 && ++numHits > 6 && speedScale > 0.8f) {
                speedScale *= 0.97f;
            }
            unk88 = true;
            mpWeapon->StartCatch(this);
            unk539 = false;
        }
    }
}

// Don't have a symbol for this but it is inlined in
// EndGameObjective::Message
void Doomerang::ResetEndGame(void) {
    mPos = mOldPos = unk10C;
    
    unk12C = 0.0f;

    velocity.Set(0.0f, 0.0f, -GetDesc()->speed);

    mTrail.Reset();

    Vector up = {0.0f, 1.0f, 0.0f, 0.0f};
    GameCamera_SnapDoomarangCamera(
        &mPos,
        &velocity,
        &up,
        0.0f
    );

    unk538 = false;
    unk539 = true;
}

void Doomerang::UpdateParticleEffect(void) {

}

void Doomerang::CheckForEnteringRegions(void) {
    if (mPos.IsInsideSphere(&unkFC, 170.0f)) {
        DescriptorIterator it = endGameObjectiveDesc.Begin();

        while (*it) {
            static_cast<EndGameObjective*>(*it)->OnStart.Send();

            it++;
        }

        unk538 = true;

        return;
    }
    
    if (!mPos.IsInsideSphere(&unk11C, 300.0f)) {
        return;
    }

    unk538 = true;

    DescriptorIterator it = endGameObjectiveDesc.Begin();

    while (*it) {
        static_cast<EndGameObjective*>(*it)->OnSuccess.Send();

        it++;
    }

    gb.mGameData.SetBossDefeated(ZN_5, true);
}

void GameCamera_UseDoomarangCamera(bool, Vector*, Vector*, Vector*, float);

void Doomerang::Deactivate(void) {
    pModel->EnableSubObject(subObjectIndex, false);

    ty.mFsm.SetState((HeroActorState)0x23, false);

    pModel->colour.w = 1.0f;
    
    boomerangShadow[mRangType].unk4 /= 4.0f;
    
    pInAirDoomerang = NULL;

    GameCamera_UseDoomarangCamera(false, NULL, NULL, NULL, 0.0f);

    mSoundHelper.Stop();

    Boomerang::Deactivate();
}

Doomerang* Boomerang_GetInAirDoomerang(void) {
    return pInAirDoomerang;
}

void Chronorang::Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon) {
    Boomerang::Init(pDesc, pWeapon);

    mPollen.Init(pModel->matrices[0].Row3());

    unk94 = 0x40;

    unkFC.SetZero();
    mVel.SetZero();
}

void Chronorang::Deinit(void) {
    mPollen.Deinit();

    Boomerang::Deinit();
}

void Chronorang::UpdateParticleEffect(void) {
    Vector endPos;
    Vector to;

    pModel->GetRefPointWorldPosition(rang01Idx, &endPos);

    to.Sub(&endPos, &unkFC);
    to.Scale(0.0f);

    Vector v = ty.velocity;
    v.Scale(0.3f);

    mVel.InterpolateLinear(&mVel, &v, 0.02f);

    mPollen.SetVel(&mVel);

    if (unk6C == BOOMERANG_STATE_1) {
        if ((gb.logicGameCount % 5) == 0) {
            mPollen.Spawn(&endPos, &to, 0.5f);
        }
    } else if ((gb.logicGameCount % 5) == 0) {
        mPollen.Spawn(&mPos, &to, 1.0f);
    }

    unkFC = endPos;
}

inline void EndGameObjective::Message(MKMessage* pMsg) {
    if (pMsg->unk0 == MSG_Activate) {
        /* ASSERT(
                pInAirDoomerang,
                "This can\'t happen! activate end game objective without doomerang in air",
                "Source\\boomerang.cpp",
                42 // line number
            );
        */
        if (pInAirDoomerang) {
            pInAirDoomerang->ResetEndGame();
        }
    } else {
        LevelObjective::Message(pMsg);
    }
}
