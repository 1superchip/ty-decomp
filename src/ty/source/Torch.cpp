#include "ty/Torch.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"

static StaticPropDescriptor torchDesc;
static ModuleInfo<Torch> torchModule;

static int flameRefIndex = -1;
int pTorchPlayingSFX = 0;

static Vector lightCol = {1.0f, 1.0f, 1.0f, 0.0f};

void Torch_LoadResources(KromeIni* pIni) {
    torchDesc.Init(&torchModule, "prop_0011_torch", "Torch1", 1, 0);
    torchDesc.Load(pIni);
    objectManager.AddDescriptor(&torchDesc);
}

void Torch::Init(GameObjDesc* pDesc) {
    StaticProp::Init(pDesc);
    pTorchPlayingSFX = 0;
    flameRefIndex = -1;
    collide = false;
    mSoundHelper.Init();
}

void Particle_Fire_Init(ParticleSystem**, Vector*, BoundingVolume*, float, bool);

void Torch::LoadDone(void) {
    StaticProp::LoadDone();
    bFoundWater = false;
    mpWobbleTexMat = NULL;
    // Never set! Not sure what field this should have checked
    if (unkCC != 2) {
        mpWobbleTexMat = Material::Create("fx_091");
        CollisionResult cr;
        Vector top = {
            GetPos()->x,
            GetPos()->y + 50.0f,
            GetPos()->z
        };
        Vector bottom = {
            GetPos()->x,
            GetPos()->y - 50.0f,
            GetPos()->z
        };
        // Check for collisions with ground and if the collision occured against water
        if (Collision_RayCollide(&top, &bottom, &cr, COLLISION_MODE_POLY, 0) &&
            (cr.collisionFlags & ID_WATER_BLUE)) {
            bFoundWater = true;
            mWaterY = cr.pos.y;
            mWobbleTex.Init(7, 7);
        }
    }

    Particle_Fire_Init(&mpParticleSys0, GetPos(), pModel->GetModelVolume(), 2.0f, true);
    Particle_Fire_Init(&mpParticleSys1, GetPos(), pModel->GetModelVolume(), 2.0f, false);

    mDefaultScale = StaticProp::loadInfo.defaultScale;
    mDefaultRot = StaticProp::loadInfo.defaultRot;
    pLocalToWorld->GetRotationPYR(&mDefaultRot);
    Reset();
}

void Particle_DestroyASystem(ParticleSystem**, float);
void Torch::Deinit(void) {
    mSoundHelper.Deinit();

    if (mpWobbleTexMat) {
        mpWobbleTexMat->Destroy();
        mpWobbleTexMat = NULL;
    }

    Particle_DestroyASystem(&mpParticleSys0, 0.0f);
    Particle_DestroyASystem(&mpParticleSys1, 0.0f);

    if (bFoundWater) {
        mWobbleTex.Deinit();
    }

    StaticProp::Deinit();
}

/// @brief Resets Torch fields
/// @param  None
void Torch::Reset(void) {
    SetState(TORCH_IDLE, 1);
    
    mSoundHelper.Reset();
    mFrozenTimer = 0;

    mRot = mDefaultRot;

    Vector temp0 = *GetPos();
    CollisionResult cr;
    temp0.y += 50.0f;
    if (Tools_TestFloor(&temp0, &cr, 1000.0f, false)) {
        mFloorY = cr.pos.y;
    } else {
        mFloorY = GetPos()->y - 150.0f;
    }

    if (Tools_TestFloor(&temp0, &cr, -1000.0f, false)) {
        mCeilingY = cr.pos.y;
    } else {
        mCeilingY = GetPos()->y + 500.0f;
    }

    // Emits fire by default
    bEmitFire = true;

    unkA8 = 0.0f;

    if (flameRefIndex < 0) {
        flameRefIndex = pModel->GetRefPointIndex("R_Flame");
    }

    mFlameRefIndex = flameRefIndex;
    pModel->GetRefPointWorldPosition(mFlameRefIndex, &mFlamePos);
    mLightCol = lightCol;
    bEmitFire2 = true;
    unk10C = 500.0f;
}

/// @brief Updates the Torch Object
/// @param  None
void Torch::Update(void) {
    unkAC++;

    if (mFrozenTimer > 0) {
        // if mFrozenTimer is greater than 0
        // the Torch should not emit fire
        mFrozenTimer--;
        bEmitFire = false;
        bEmitFire2 = false;
    } else {
        // if mFrozenTimer is 0 or less than 0
        // the torch should emit fire again
        bEmitFire = true;
        bEmitFire2 = true;
    }

    float colorWobble = 1.0f + (Tools_Wobble((float)unkAC * 0.08f, 12345) * 0.2f);
    mLightCol.Scale(&lightCol, colorWobble);

    switch (mState) {
        case TORCH_IDLE:
            Idle();
            break;
        case TORCH_HIT:
            Hit();
            break;
    }

    mWobbleTex.IncrementUnk4();

    if (bEmitFire) {
        // Only update Ty's shadow if the torch flame is lit
        UpdateShadow();
    }

    bEmitFire2 = bEmitFire;
    mSoundHelper.Update(199, 0, bEmitFire, 0, &mFlamePos, distSquared, 0);
}

void Torch::Draw(void) {
    lodManager.Draw(pModel, detailLevel, unk1C, distSquared, IsInWater());

    pModel->GetRefPointWorldPosition(flameRefIndex, &mFlamePos);

    if (bFoundWater) {
        mWobbleTex.SetUpGrid(&mFlamePos, 100.0f, 200.0f, mWaterY);
        mWobbleTex.WobbleUVs(0.33f);

        if (mWaterY < View::GetCurrent()->mCamPos.y) {
            mWobbleTex.Draw(mpWobbleTexMat, false);
        }
    }
}

void Torch::Message(MKMessage* pMsg) {
    mSoundHelper.Message(pMsg);
    GameObject::Message(pMsg);
}

void Torch::Idle(void) {
    CheckForHit();
    EmitFire();
}

/// @brief Executes when the Torch is hit by a boomerang
/// @param  None
void Torch::Hit(void) {
    if (unkAC == 1) {
        unkB0 = 1;
        mRotSpeedSetting = 0;
        mRotInc = PI / 32.0f;
    }

    switch (unkB0) {
        case 1:
            mRot.x += mRotInc;
            mRot.z += mRotInc;
            
            if (mRot.x > mDefaultRot.x + mRotInc) {
                unkB0 = 2;
            }
            break;
        case 2:
            mRot.x -= mRotInc;
            mRot.z -= mRotInc;

            if (mRot.x < mDefaultRot.x - mRotInc) {
                unkB0 = 3;
            }
            break;
        case 3:
            mRot.x += mRotInc;
            mRot.z += mRotInc;

            if (mRot.x >= mDefaultRot.x) {
                mRotSpeedSetting++;
                if (mRotSpeedSetting == 2) {
                    mRotInc = PI / 64.0f;
                } else if (mRotSpeedSetting == 3) {
                    mRotInc = PI / 96.0f;
                }
                unkB0 = 1;
            }
            
            if (mRotSpeedSetting == 4) {
                unkB0 = 4;
            }
            break;
        case 4:
            SetState(TORCH_IDLE, 0);
            mRot = mDefaultRot;
            break;
    }
    
    pModel->SetRotation(&mRot);
    pModel->GetRefPointWorldPosition(mFlameRefIndex, &mFlamePos);
    EmitFire();
}

/// @brief Sets the state of a Torch, sets unkAC to 0
/// @param newState New State Value
/// @param alwaysSetState Always sets the state to the new state and unkAC to 0
void Torch::SetState(TorchState newState, int alwaysSetState) {
    if (mState != newState || alwaysSetState) {
        mState = newState;
        unkAC = 0;
    }
}

void Particle_Fire_Create(ParticleSystem**, Vector*, float, bool);

/// @brief Creates Fire Particles around the Torch Flame
/// @param None
void Torch::EmitFire(void) {
    if (bEmitFire) {
        Particle_Fire_Create(&mpParticleSys1, &mFlamePos, mDefaultScale.x * 2.0f, false);
        unkA8 += 0.2833f;
        while (unkA8 > 1.0f) {
            Vector sp8;
            unkA8 -= 1.0f;
            sp8.Set(
                ((RandomI(&gb.mRandSeed) % 100) * 10.0f) / 100.0f + (mFlamePos.x - 5.0f),
                ((RandomI(&gb.mRandSeed) % 100) * 10.0f) / 100.0f + (mFlamePos.y - 5.0f),
                ((RandomI(&gb.mRandSeed) % 100) * 10.0f) / 100.0f + (mFlamePos.z - 5.0f)
            );
            Particle_Fire_Create(&mpParticleSys0, &sp8, mDefaultScale.x, true);
        }
    }
}

// void DebugInfo_Sphere(char*, Vector*, float, int, Vector*);

/// @brief Adds a shadow to Ty when Ty is near a lit torch
/// @param  None
void Torch::UpdateShadow(void) {
    if (mFlamePos.IsInsideSphere(&ty.unk338, 500.0f)) {
        // DebugInfo_Sphere("-tomLowe", &mFlamePos, 500.0f, 1, 0);
        Vector particlePos = mFlamePos;
        particlePos.x += RandomFR(&gb.mRandSeed, -4.0f, 4.0f);
        particlePos.y += RandomFR(&gb.mRandSeed, -4.0f, 4.0f);
        particlePos.z += RandomFR(&gb.mRandSeed, -4.0f, 4.0f);
        float mag = Sqr<float>(Max<float>(ApproxMag(&mFlamePos, &ty.unk338), 10.0f));
        ty.AddShadowLight(&particlePos, (Sqr<float>(500.0f) / mag) - 1.0f);
    }
}

// void* Boomerang_CheckForHit(Model*, int, CollisionResult*);
// void* Boomerang_CheckForHitSphere(Vector*, float, bool);
void SoundBank_Play(int, Vector*, uint);

/// @brief Checks if the Torch is hit by a boomerang
/// @param  None
void Torch::CheckForHit(void) {
    CollisionResult cr;
    // unkCC is never set!
    // possibly should have been the field at 0xC8 which is the state field?
    if (unkCC != 2) {
        // Check if a boomerang collides with the (Dynamic) Torch Model
        void* pBoomerang = Boomerang_CheckForHit(pModel, -1, &cr);
        if (pBoomerang) {
            ((bool*)pBoomerang)[0x54] = true;
            // if the Torch was hit by a boomerang and 
            // the torch wasn't in the hit state
            // set the state to TORCH_HIT
            if (mState != TORCH_HIT) {
                mState = TORCH_HIT;
                unkAC = 0;
            }
            // Plays the torch model wobbling back-and-forth sound
            SoundBank_Play(303, GetPos(), 0);
        }

        // Check if a boomerang collides with a sphere at mFlamePos
        pBoomerang = Boomerang_CheckForHitSphere(&mFlamePos, 50.0f, false);
        if (pBoomerang) {
            // if the Boomerang was the Frostyrang
            // Set the torch frozen timer and play a sound if it was emitting fire
            if (((int*)pBoomerang)[0x50 / 0x4] == 1) {
                mFrozenTimer = 300;
                if (bEmitFire) {
                    SoundBank_Play(200, GetPos(), 0);
                    mSoundHelper.Stop(); // Stop playing the fire crackling sound
                }
            } else if (bEmitFire) {
                ((int*)pBoomerang)[0x4C / 0x4] = 0x1E;
            }
        }
    }
}
