#include "ty/props/BunyipStone.h"
#include "ty/GameObjectManager.h"
#include "ty/bunyip.h"
#include "common/Str.h"

static StaticPropDescriptor bunyipStoneDesc;
static ModuleInfo<BunyipStone> bunyipStoneModule;

void BunyipStone_LoadResources(KromeIni* pIni) {
    bunyipStoneDesc.Init(&bunyipStoneModule, "Prop_0430_BunStnEarth", "BunStnEarth", 0, 0);
    bunyipStoneDesc.Load(pIni);
    objectManager.AddDescriptor(&bunyipStoneDesc);
}

void BunyipStone::Init(GameObjDesc* pDesc) {
    StaticProp::Init(pDesc);
    
    cooldownTimer = 0;
}

void BunyipStone::LoadDone(void) {
    StaticProp::LoadDone();

    for (int i = 0; i < ARRAY_SIZE(stoneMatrixIndices); i++) {
        stoneMatrixIndices[i] = pModel->GetSubObjectMatrixIndex(
            pModel->GetSubObjectIndex(Str_Printf("A_Stone%d", i + 1))
        );
    }

    pModel->pMatrices[stoneMatrixIndices[0]].Scale(0.7f);
    Reset();
}

void BunyipStone::Reset(void) {
    mBallRot1.Set(0.0f, 0.0f, 0.0f);
    mBallRot2.Set(0.0f, 0.0f, 0.0f);

    pModel->pMatrices[stoneMatrixIndices[0]].Row3()->y += 110.0f;

    pModel->pMatrices[stoneMatrixIndices[1]].SetRotationPYR(&mBallRot1);
    pModel->pMatrices[stoneMatrixIndices[1]].Scale(0.5f); // Make the rotating balls smaller

    pModel->pMatrices[stoneMatrixIndices[2]].SetRotationPYR(&mBallRot2);
    pModel->pMatrices[stoneMatrixIndices[2]].Scale(0.5f); // Make the rotating balls smaller

    centerBallYWobble = 0.0f;

    mBallState = BS_BALLSTATE_UPDATE;

    unk8C = 0.0f;
    unk90 = 0.0f;
    unk94 = 0.0f;
    unk98 = 0.0f;
    
    cooldownTimer = 0;
}

void BunyipStone::Update(void) {
    UpdateBalls();

    if (cooldownTimer > 0) {
        cooldownTimer--;
    } else {
        Vector dist;
        dist.Sub(pHero->GetPos(), GetPos());

        if (dist.MagSquared() < 2500.0f && Bunyip_Activate()) {
            // if the hero is in distance and the Bunyip was activated, set the timer
            // Cooldown lasts for ~120 seconds at 60 FPS (PAL/50 FPS → ~144 seconds)
            cooldownTimer = gDisplay.fps * 120.0f;
            mBallState = BS_BALLSTATE_HIDE; // Start hiding the balls in the stone
        }
    }
}

void BunyipStone::UpdateBalls(void) {
    switch (mBallState) {
        case BS_BALLSTATE_SHOW:
            // Raise the balls out of the stone
            if (pModel->pMatrices[stoneMatrixIndices[0]].Row3()->y < 110.0f) {
                pModel->pMatrices[stoneMatrixIndices[0]].Row3()->y += 1.0f;
                pModel->pMatrices[stoneMatrixIndices[1]].Row3()->y += 1.0f;
                pModel->pMatrices[stoneMatrixIndices[2]].Row3()->y += 1.0f;
            } else {
                centerBallYWobble = 0.0f;
                mBallState = BS_BALLSTATE_UPDATE;
            }

            OrbitBalls();
            break;
        case BS_BALLSTATE_UPDATE:
            centerBallYWobble += 0.031f;
            if (centerBallYWobble > 2 * PI) {
                centerBallYWobble -= 2 * PI;
            }

            // Wobble the center ball up and down
            pModel->pMatrices[stoneMatrixIndices[0]].Row3()->y = 110.0f + _table_sinf(centerBallYWobble) * 10.0f;
            
            OrbitBalls();
            break;
        case BS_BALLSTATE_HIDE:
            // Decreases the y position of each ball to hide them in the stone (stone deactivated)

            if (pModel->pMatrices[stoneMatrixIndices[0]].Row3()->y > 0.0f) {
                pModel->pMatrices[stoneMatrixIndices[0]].Row3()->y -= 1.0f;
            }

            if (pModel->pMatrices[stoneMatrixIndices[1]].Row3()->y > 0.0f) {
                pModel->pMatrices[stoneMatrixIndices[1]].Row3()->y -= 1.0f;
            }

            if (pModel->pMatrices[stoneMatrixIndices[2]].Row3()->y > 0.0f) {
                pModel->pMatrices[stoneMatrixIndices[2]].Row3()->y -= 1.0f;
            }

            if (cooldownTimer == 0) {
                // once the cooldown timer is 0, start increasing the height
                mBallState = BS_BALLSTATE_SHOW;
            }
            break;
    }
}

void BunyipStone::OrbitBalls(void) {
    unk8C += 0.041f;
    if (unk8C > 2 * PI) {
        unk8C -= 2 * PI;
    }

    unk90 += 0.032f;
    if (unk90 > 2 * PI) {
        unk90 -= 2 * PI;
    }

    mBallRot1.x += 0.01f;
    mBallRot1.y -= 0.02231f;
    mBallRot1.z += 0.0312f;

    pModel->pMatrices[stoneMatrixIndices[1]].SetRotationPYR(&mBallRot1);
    pModel->pMatrices[stoneMatrixIndices[1]].Scale(0.5f);

    pModel->pMatrices[stoneMatrixIndices[1]].Row3()->x = 
        pModel->pMatrices[stoneMatrixIndices[0]].Row3()->x + _table_sinf(unk8C) * 45.0f;
    
    pModel->pMatrices[stoneMatrixIndices[1]].Row3()->z = 
        pModel->pMatrices[stoneMatrixIndices[0]].Row3()->z + _table_cosf(unk8C) * 45.0f;
    
    pModel->pMatrices[stoneMatrixIndices[1]].Row3()->y = 
        pModel->pMatrices[stoneMatrixIndices[0]].Row3()->y + _table_sinf(unk90) * 45.0f;
    
    unk94 -= 0.0551f;
    if (unk94 < 0.0f) {
        unk94 += 2 * PI;
    }

    unk98 -= 0.046f;
    if (unk98 < 0.0f) {
        unk98 += 2 * PI;
    }

    mBallRot2.x -= 0.029f;
    mBallRot2.y += 0.0122f;
    mBallRot2.z += 0.001f;

    pModel->pMatrices[stoneMatrixIndices[2]].SetRotationPYR(&mBallRot2);
    pModel->pMatrices[stoneMatrixIndices[2]].Scale(0.5f);

    pModel->pMatrices[stoneMatrixIndices[2]].Row3()->x = 
        pModel->pMatrices[stoneMatrixIndices[0]].Row3()->x + _table_sinf(unk94) * 29.0f;
    
    pModel->pMatrices[stoneMatrixIndices[2]].Row3()->z = 
        pModel->pMatrices[stoneMatrixIndices[0]].Row3()->z + _table_cosf(unk94) * 29.0f;
    
    pModel->pMatrices[stoneMatrixIndices[2]].Row3()->y = 
        pModel->pMatrices[stoneMatrixIndices[0]].Row3()->y + _table_sinf(unk98) * 29.0f;
}
