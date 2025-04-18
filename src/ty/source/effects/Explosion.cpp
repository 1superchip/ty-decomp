#include "ty/effects/Explosion.h"
#include "ty/global.h"
#include "ty/tytypes.h"
#include "ty/tools.h"
#include "common/Blitter.h"
#include "common/Heap.h"

extern void Particle_Fire_Init(ParticleSystem**, Vector*, BoundingVolume*, float, bool);
void Particle_Fire_Create(ParticleSystem**, Vector*, float, bool);
extern Vector* GameCamera_GetPos(void);
extern void Draw_AddPostDrawElement(void*, void (*)(void*), float, bool);

extern bool gAssertBool;

StructList<Explosion*> activeSet;

static void Explosion_PostDrawFunc(void* pExplosion) {
    static_cast<Explosion*>(pExplosion)->Draw();
}

void Explosion_LoadResources(void) {
    activeSet.Init(20);
}

void Explosion::Init(Vector* pPos, bool r5, float f1) {
    int i;

    static BoundingVolume pExplosionVol = {
        {-400.0f, -400.0f, -400.0f},
        { 800.0f,  800.0f,  800.0f}
    };

    unk3EC = true;
    unk20 = r5;

    mBubble.Init(pPos, &pExplosionVol, 1.0f);
    pMaterial = Material::Create("fx_131");

    for (i = 0; i < NUM_CLOUDS; i++) {
        mClouds[i].pModel = NULL;
        mClouds[i].pModel = Model::Create("Prop_0535_Explosion", NULL);
        mClouds[i].pModel->renderType = 3;
    }

    pExploRing = NULL;
    pExploRing = Model::Create("Prop_0551_ExploRing", NULL);

    for (i = 0; i < NUM_TENDRILS; i++) {
        mTendrils[i].pModel = NULL;
        mTendrils[i].pModel = Model::Create("Prop_0544_ExploTend", NULL);
        mTendrils[i].pModel->renderType = 3;
        mTendrils[i].refPointIndex = -1;
        gAssertBool = mTendrils[i].pModel->RefPointExists("R_Point", &mTendrils[i].refPointIndex);
    }

    unk3C8 = f1;

    unk21 = false;

    static BoundingVolume vol = {
        {-1000.0f, -1000.0f, -1000.0f, 1.0f},
        { 2000.0f,  2000.0f,  2000.0f, 1.0f}
    };

    Particle_Fire_Init(&mpSystem, &mPos, &vol, 12.0f, true);
}

void Explosion::Deinit(void) {
    if (mpSystem) {
        mpSystem->Destroy();
    }

    mpSystem = NULL;

    if (pMaterial) {
        pMaterial->Destroy();
    }

    pMaterial = NULL;

    for (int i = 0; i < NUM_CLOUDS; i++) {
        if (mClouds[i].pModel) {
            mClouds[i].pModel->Destroy();
        }

        mClouds[i].pModel = NULL;
    }

    if (pExploRing) {
        pExploRing->Destroy();
    }

    pExploRing = NULL;

    for (int i = 0; i < NUM_TENDRILS; i++) {
        if (mTendrils[i].pModel) {
            mTendrils[i].pModel->Destroy();
        }

        mTendrils[i].pModel = NULL;
    }

    mBubble.Deinit();

    Explosion** pExplosions = activeSet.GetCurrEntry();
    while (pExplosions) {
        if (*pExplosions == this) {
            activeSet.CopyEntry(pExplosions);
            break;
        }

        pExplosions = activeSet.GetNextEntryWithEntry(pExplosions);
    }
}

extern "C" double atan2(double, double);
void VibrateJoystick(float, float, float, char, float);
void GameCamera_Shake(Vector*);

void Explosion::Explode(Vector* pPos, float f1) {

    Explosion** ppExplosions = activeSet.GetNextEntry();

    if (ppExplosions) {
        int i;

        *ppExplosions = this;

        unk21 = true;
        mPos = *pPos;
        unk3D0 = 50.0f;
        mParticleColor = 0.0f;

        for (i = 0; i < NUM_CLOUDS; i++) {
            mClouds[i].Setup(&mPos, unk3C8);
        }

        mClouds[0].unk30 = 0.5f;
        mClouds[0].unk20 = mPos;
        mClouds[0].unk34 = 1.0f;
        mClouds[0].unk10.Scale(0.5f);

        unk3C4 = RandomIR(&gb.mRandSeed, 5, 9);

        if (unk3C8 <= 0.5f) {
            unk3C4 = 0;
        }

        for (i = 0; i < unk3C4; i++) {
            float min = (i / (float)unk3C4) * PI * 2.0f;
            float max = ((i + 1) / (float)unk3C4) * PI * 2.0f;
            mTendrils[i].Setup(RandomFR(&gb.mRandSeed, min, max), &mPos);
        }

        unk8 = f1;
        unk24 = 0.0f;
        unk3D8 = 0.1f;

        mRingRot.Set(0.0f, 0.0f, 0.0f);

        Vector toExp;
        toExp.Sub(&mPos, GameCamera_GetPos());
        float mag = toExp.Normalise();
        
        mRingRot.y = atan2(toExp.x, -toExp.z);

        if (toExp.y < 0.1f && toExp.y >= 0.0f) {
            mRingRot.x = 0.1f;
        }

        if (toExp.y > -0.1f && toExp.y < 0.0f) {
            mRingRot.x = -0.1f;
        }

        pExploRing->SetRotation(&mRingRot);
        pExploRing->matrices[0].Scale(unk3D8 * unk3C8);

        Vector p = *pPos;
        p.y += unk3C8 * 100.0f;

        float x = 1.0f;
        if (mag > 800.0f) {
            x = Max<float>(0.0f, 1.0f - ((mag - 800.0f) / 2000.0f));
        }

        VibrateJoystick(x, x, 1.3f, 0x41, 4.0f);

        Vector shake;
        shake.Set(0.0f, 20.0f, 0.0f);
        GameCamera_Shake(&shake);
    }
}

void Explosion_Update(void) {
    Explosion** pExplosions = activeSet.GetCurrEntry();
    while (pExplosions) {

        (*pExplosions)->Update();

        if ((*pExplosions)->unk24 > 120.0f) {
            (*pExplosions)->unk21 = false;
            activeSet.CopyEntry(pExplosions);
        }

        pExplosions = activeSet.GetNextEntryWithEntry(pExplosions);
    }
}

void Explosion_Deinit(void) {
    activeSet.UnknownSetPointer();
}

void Explosion::Update(void) {
    unk24 += 1.0f;

    if (unk24 <= 10.0f) {
        unk3D0 = SmoothCenteredCurve((unk24 + 6.0f) / 30.0f) * 1500.0f;
        mParticleColor = SmoothCenteredCurve((unk24 + 6.0f) / 30.0f);
    } else {
        unk3D0 *= 0.98f;
        mParticleColor *= 0.95f;
    }

    if (unk24 <= 6.0f) {
        for (int i = 0; i < NUM_CLOUDS; i++) {
            mClouds[i].pModel->colour.w = (unk24 + 2.0f) / 8.0f;
        }

        pExploRing->colour.x = pExploRing->colour.y = pExploRing->colour.z = 
            Clamp<float>(0.0f, (unk3C8 - 0.5f) * (unk24 + 2.0f) / 4.0f, 1.0f);

        for (int i = 0; i < unk3C4; i++) {
            mTendrils[i].pModel->colour.w = mClouds[0].pModel->colour.w;
        }
    }

    if (unk24 > 60.0f) {
        for (int i = 0; i < NUM_CLOUDS; i++) {
            mClouds[i].pModel->colour.w *= 0.8f;
        }

        for (int i = 0; i < unk3C4; i++) {
            mTendrils[i].pModel->colour.w = mClouds[0].pModel->colour.w;
        }
    }

    if (unk24 > 50.0f) {
        pExploRing->colour.x = pExploRing->colour.y = pExploRing->colour.z = pExploRing->colour.x * 0.95f;
    }

    float val = 1.5f;
    if (unk24 > 10.0f) {
        val = 0.2f - (0.2f - mClouds[0].pModel->colour.x) * 0.9f;
    }

    for (int i = 0; i < NUM_CLOUDS; i++) {
        mClouds[i].Update(unk3C8, val);
    }

    if (unk24 > 1.0f) {
        unk3D8 = 4.0f - (4.0f - unk3D8) * 0.95f;
    }

    pExploRing->SetRotation(&mRingRot);
    pExploRing->matrices[0].Scale(unk3C8 * unk3D8);
    pExploRing->SetPosition(&mPos);
    pExploRing->matrices[0].Row3()->y += 5.0f;

    float dVar13 = unk3C8 * 3.0f * Clamp<float>(0.0f, 1.0f - (unk24 - 30.0f) * 0.05f, 1.0f);

    for (int i = 0; i < unk3C4; i++) {
        if (unk24 > mTendrils[i].unk30) {
            mTendrils[i].Update(unk3C8, &mPos, val);

            if (((int)unk24 % 4) == (i % 4)) {
                Particle_Fire_Create(&mpSystem, &mTendrils[i].unk0, dVar13, true);
            }

            if (unk20 && unk24 < 50.0f) {
                if (((int)unk24 % 4) == ((i + 2) % 4)) {
                    mBubble.Create(&mTendrils[i].unk0, 8.0f, unk8, 60.0f, 0.0f);
                }
            }
        }
    }

    if (unk20 && unk24 < 60.0f) {
        Vector rand = mPos;

        rand.y += unk3C8 * 100.0f;

        mBubble.Create(Tools_RandomBox(&rand, unk3C8 * 150.0f), 8.0f, unk8, 60.0f, 0.0f);
    }
}

void Explosion_Draw(void) {
    Explosion** pExplosions = activeSet.GetCurrEntry();
    while (pExplosions) {
        
        Draw_AddPostDrawElement(
            *pExplosions, 
            Explosion_PostDrawFunc,
            GameCamera_GetPos()->DistSq(&(*pExplosions)->mPos),
            (*pExplosions)->unk20
        );

        pExplosions = activeSet.GetNextEntryWithEntry(pExplosions);
    }
}

void Explosion::Draw(void) {
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);

    if (pMaterial) {
        pMaterial->Use();
    }

    Blitter_Particle part;
    part.pos = mPos;
    part.color.Set(
        mParticleColor, mParticleColor, mParticleColor, 1.0f
    );
    part.angle = 0.0f;
    part.unk20 = unk3D0 * unk3C8;

    part.Draw(1);

    for (int i = 0; i < unk3C4; i++) {
        mTendrils[i].pModel->Draw(NULL);
    }

    for (int i = 0; i < NUM_CLOUDS; i++) {
        mClouds[i].pModel->Draw(NULL);
    }

    if (unk3EC) {
        pExploRing->Draw(NULL);
    }
}

void Tendril::Setup(float yaw, Vector* pPos) {
    mRoll = RandomFR(&gb.mRandSeed, 0.0f, 2.0f * PI);
    unk34 = RandomFR(&gb.mRandSeed, 0.1f, 0.2f);

    if (RandomIR(&gb.mRandSeed, 0, 2) != 0) {
        unk34 = -unk34;
    }

    mPitch = Tools_RandomGaussian();
    mYaw = yaw;

    pModel->matrices[0].SetRotationRoll(mRoll);
    pModel->matrices[0].RotatePitch(mPitch);
    pModel->matrices[0].RotateYaw(mYaw);

    pModel->matrices[0].Scale(0.01f);
    pModel->SetPosition(pPos);

    pModel->colour.Set(1.5f, 1.5f, 1.5f, 0.5f);

    unk24 = RandomFR(&gb.mRandSeed, -0.2f, 0.3f);
    unk28 = RandomFR(&gb.mRandSeed, 0.93f, 0.96f);
    unk2C = RandomFR(&gb.mRandSeed, 1.0f, 1.6f);
    unk30 = RandomFR(&gb.mRandSeed, 0.0f, 10.0f);
}

void Tendril::Update(float f1, Vector* pPos, float f2) {
    unk24 = unk2C - unk28 * (unk2C - unk24);
    mRoll += unk34;
    unk34 *= 0.96f;

    pModel->matrices[0].SetRotationRoll(mRoll);
    pModel->matrices[0].RotatePitch(mPitch);
    pModel->matrices[0].RotateYaw(mYaw);

    pModel->matrices[0].Scale(f1 * unk24);

    pModel->SetInverseScaleValue(-1, 1.0f / (f1 * unk24));

    pModel->SetPosition(pPos);

    pModel->colour.x = pModel->colour.y = pModel->colour.z = f2;

    if (refPointIndex != -1) {
        pModel->GetRefPointWorldPosition(refPointIndex, &unk0);
    }
}

void Cloud::Setup(Vector* p, float f1) {
    unk20 = *p;
    unk0.SetZero();
    unk10.SetZero();

    Tools_RandomBox(&unk10, 1.0f / 40.0f);
    Tools_RandomBox(&unk0, PI);

    pModel->matrices[0].Scale(0.0f);

    unk20.x += GetRandom(f1 * 80.0f, f1 * 130.0f);
    unk20.y += GetRandom(f1 * 80.0f, f1 * 130.0f);
    unk20.z += GetRandom(f1 * 80.0f, f1 * 130.0f);

    pModel->SetPosition(&unk20);

    pModel->colour.Set(1.5f, 1.5f, 1.5f, 0.5f);

    unk34 = RandomFR(&gb.mRandSeed, 0.5f, 0.8f);
    unk30 = RandomFR(&gb.mRandSeed, 0.2f, 0.3f);
}

float Cloud::GetRandom(float min, float max) {
    float val = RandomFR(&gb.mRandSeed, min, max);

    if (RandomIR(&gb.mRandSeed, 0, 2) != 0) {
        val = -val;
    }

    return val;
}

void Cloud::Update(float f1, float f2) {

    unk30 = unk34 - (unk34 - unk30) * 0.95f;

    unk0.Add(&unk10);

    unk10.Scale(0.96f);

    pModel->matrices[0].SetRotationPYR(&unk0);

    pModel->matrices[0].Scale(f1 * unk30);

    pModel->SetInverseScaleValue(-1, 1.0f / (f1 * unk30));

    pModel->colour.x = pModel->colour.y = pModel->colour.z = f2;

    pModel->SetPosition(&unk20);
}
