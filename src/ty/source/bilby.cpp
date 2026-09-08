#include "ty/bilby.h"
#include "ty/DDA.h"
#include "ty/global.h"
#include "ty/Ty.h"
#include "common/PtrListDL.h"
#include "ty/ParticleEngine.h"
#include "ty/bunyip.h"
#include "ty/SpecialPickup.h"

bool bBilbiesLoaded = false;
PtrListDL<BilbyStruct> bilbies;
int bilbiesMax;

MKAnim* bilbyAnims[BT_MAX][2];
MKAnimScript bilbyBad[BT_MAX];

static bool bVisible = true;

void Bilby_LoadResources(void) {
    bilbyBad[BT_0].Init("act_04_bilby");
    bilbyBad[BT_1].Init("act_26_BilbyMum");
    bilbyBad[BT_2].Init("act_49_BilbyBoy");
    bilbyBad[BT_3].Init("act_50_Bilbygirl");
    bilbyBad[BT_4].Init("act_43_BilbyGrandma");

    bilbyAnims[BT_0][0] = bilbyBad[BT_0].GetAnim("idle01");
    bilbyAnims[BT_0][1] = bilbyBad[BT_0].GetAnim("dance");
    bilbyAnims[BT_1][0] = bilbyBad[BT_1].GetAnim("idle01");
    bilbyAnims[BT_1][1] = bilbyBad[BT_1].GetAnim("dance");
    bilbyAnims[BT_2][0] = bilbyBad[BT_2].GetAnim("idle01");
    bilbyAnims[BT_2][1] = bilbyBad[BT_2].GetAnim("dance");
    bilbyAnims[BT_3][0] = bilbyBad[BT_3].GetAnim("idle01");
    bilbyAnims[BT_3][1] = bilbyBad[BT_3].GetAnim("dance");
    bilbyAnims[BT_4][0] = bilbyBad[BT_4].GetAnim("idle01");
    bilbyAnims[BT_4][1] = bilbyBad[BT_4].GetAnim("dance");
}

void Bilby_Init(void) {
    if (!bBilbiesLoaded) {
        bVisible = true;
        
        bilbies.Init(bilbiesMax, sizeof(BilbyStruct));

        bBilbiesLoaded = true;
    }
}

void Bilby_Deinit(void) {
    if (!bBilbiesLoaded) {
        return;
    }

    BilbyStruct** pBilbies = bilbies.GetMem();
    while (*pBilbies) {
        BilbyStruct* pCurr = *pBilbies;

        if (pCurr->pModel2) {
            pCurr->pModel2->Destroy();
        }

        if (pCurr->pModel) {
            pCurr->pModel->Destroy();
        }

        pBilbies++;
    }

    bilbies.Deinit();

    bBilbiesLoaded = false;
}

void Bilby_Add(BilbyLoadInfo* loadInfo) {
    if (bBilbiesLoaded) {
        BilbyStruct* pNewBilby = bilbies.GetNextEntry();
        if (pNewBilby) {
            pNewBilby->bInitialised = false;
            pNewBilby->Init(loadInfo);

            pNewBilby->mCollisionInfo.Init(true, 0, NULL);

            Collision_AddDynamicModel(pNewBilby->pModel2, &pNewBilby->mCollisionInfo, -1);
        }
    }
}

void Bilby_Update(void) {
    if (!bBilbiesLoaded || !bVisible) {
        return;
    }

    BilbyStruct** pBilbies = bilbies.GetMem();
    while (*pBilbies) {
        BilbyStruct* pCurr = *pBilbies;

        pCurr->unk38++;

        switch (pCurr->mState) {
            case BS_1:
                pCurr->Idle();
                break;
            case BS_2:
                pCurr->Rescued();
                break;
        }

        pBilbies++;
    }
}

void Bilby_Draw(void) {
    if (!bBilbiesLoaded || !bVisible) {
        return;
    }

    BilbyStruct** pBilbies = bilbies.GetMem();
    while (*pBilbies) {
        BilbyStruct* pCurr = *pBilbies;

        if (pCurr->mState > BS_0) {
            pCurr->mAnimScript.Apply(pCurr->pModel->GetAnimation());
            pCurr->unk124 = pCurr->pModel->Draw(NULL);
            if (pCurr->unk31) {
                pCurr->pModel2->colour = pCurr->pModel->colour;

                if (bDrawCheatLines) {
                    Blitter_Line3D line;
                    line.color.Set(0.8f, 0.8f, 0.8f, 1.0f);
                    line.color1 = line.color;

                    line.point = line.point1 = *pCurr->pModel2->matrices[0].Row3();
                    line.point1.y += 10000.0f;

                    line.DrawNoMat(1, 1.0f);
                }

                pCurr->pModel2->Draw(NULL);
            }

            if (pCurr->mState == BS_2 && !gb.bOnPauseScreen) {
                for (int i = 0; i < ARRAY_SIZE(pCurr->unk84); i++) {
                    Vector pos;

                    pos = pCurr->unk84[i].unk0;
                    pos.y += 5.0f;

                    pos.x += _table_sinf(pCurr->unk84[i].unk10.y) * pCurr->unk84[i].unk10.x;
                    pos.z += _table_cosf(pCurr->unk84[i].unk10.y) * pCurr->unk84[i].unk10.x;

                    particleManager->SpawnBilbyPickupAtom(
                        &pos,
                        ((RandomI(&gb.mRandSeed) % 100) * 50.0f) / 100.0f + (pCurr->unk84[i].unk10.x * 0.3f),
                        pCurr->unk40
                    );
                }
            }
        }

        pBilbies++;
    }
}

void Bilby_HideAll(void) {
    bVisible = false;

    BilbyStruct** pBilbies = bilbies.GetMem();
    while (*pBilbies) {
        (*pBilbies)->mCollisionInfo.bEnabled = false;
        pBilbies++;
    }
}

void Bilby_ShowAll(void) {
    bVisible = true;

    BilbyStruct** pBilbies = bilbies.GetMem();
    while (*pBilbies) {
        if (!(*pBilbies)->unk32) {
            (*pBilbies)->mCollisionInfo.bEnabled = true;
        }

        pBilbies++;
    }
}

void BilbyStruct::Init(BilbyLoadInfo* loadInfo) {
    mType = loadInfo->type;

    pModel = Model::Create(
        bilbyBad[mType].GetMeshName(), bilbyBad[mType].GetAnimName()
    );

    unk30 = false;
    mState = BS_1;
    unk38 = 0;
    unk31 = true;

    mPos = loadInfo->pos;
    mRot = loadInfo->rot;
    unk2C = 1.0f;

    unk32 = 0;

    unk44 = -1;
    mSoundHelper.Init();
    mNextTime = 0;

    unk12C = bilbyAnims[mType];

    mAnimScript.Init(&bilbyBad[mType]);

    pModel->matrices[0].SetIdentity();
    pModel->matrices[0].SetRotationPYR(&mRot);
    pModel->matrices[0].Translate(&mPos);
    pModel->SetLocalToWorldDirty();

    pModel2 = Model::Create("prop_0044_cage", NULL);
    pModel2->matrices[0] = pModel->matrices[0];
    pModel2->SetLocalToWorldDirty();

    int subobjectIndex = pModel2->GetSubObjectIndex("C_Collide");
    pModel2->EnableSubObject(
        subobjectIndex, false
    );

    pShatter = Shatter_Add(pModel2, 1.0f, 0.7f, 120);

    Vector color;

    Vector start = {mPos.x, mPos.y + 20.0f, mPos.z};
    Vector end = {mPos.x, mPos.y - 100.0f, mPos.z};
    CollisionResult cr;

    if (Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, 0x0)) {
        Vector color = Tools_GroundColor(&cr);
        pModel->colour.Copy(&color);
        pModel2->colour.Copy(&color);
    }

    bInitialised = true;
}

extern "C" int Sound_IsVoicePlaying(int);

void BilbyStruct::Idle(void) {
    if (unk38 == 1) {
        mAnimScript.SetAnim(unk12C[0]);
    }

    mAnimScript.Animate();

    if (mAnimScript.Condition()) {
        mAnimScript.SetAnim(unk12C[0]);
    }

    if (gb.logicGameCount > mNextTime && (mSoundHelper.unk0 <= -1 || !Sound_IsVoicePlaying(mSoundHelper.unk0))) {
        Vector dist;
        dist.Sub(GetPos(), pHero->GetPos());
        mSoundHelper.Stop();

        mSoundHelper.Update(mType + 0x1FE, false, true, NULL, GetPos(), dist.MagSquared(), 0);

        mNextTime = gb.logicGameCount + (uint)(gDisplay.fps * RandomIR(&gb.mRandSeed, 7, 15));
    }

    if (pHero->IsBushPig()) {
        if (GetPos()->IsInsideSphere(&pHero->pos, pHero->objectRadiusAdjustment)) {
            gb.mGameData.RescueBilby(mType);
            SetState(BS_2);
        }
    } else {
        Vector centrePos = {
            GetPos()->x,
            GetPos()->y + 40.0f,
            GetPos()->z,
            0.0f
        };

        Vector lookPos = {
            GetPos()->x,
            GetPos()->y + 53.333333f,
            GetPos()->z,
            0.0f
        };

        float radius = pModel->GetModelVolume()->v2.y * 0.5f;

        if (ty.tyBite.HasBitten(pModel2, &centrePos, 0.0f) || (Bunyip_On() && ty.pBunyip->HasPunched(&centrePos, radius))) {
            for (int i = 0; i < 3; i++) {
                Vector t;
                t.Set(
                    GetPos()->x + -50.0f + (RandomI(&gb.mRandSeed) % 100),
                    (GetPos()->y + 130.0f) + -50.0f + (RandomI(&gb.mRandSeed) % 100),
                    GetPos()->z + -50.0f + (RandomI(&gb.mRandSeed) % 100)
                );

                particleManager->SpawnChomp(&t, 20.0f + (RandomI(&gb.mRandSeed) % 30));
            }

            VibrateJoystick(0.8f, 1.0f, 0.17f, 0, 4.0f);
            SetState(BS_2);
            gb.mGameData.RescueBilby(mType);
        } else {
            ty.mAutoTarget.Set(TP_2, NULL, &centrePos, &lookPos, pModel2);
        }

        Boomerang* pBoomerang = Boomerang_CheckForHit(GetPos(), 80.0f, 100.0f, 0.0f);
        if (pBoomerang && unk124) {
            pBoomerang->HitWorld(&pBoomerang->mPos, 0);
        }
    }
}

void BilbyStruct::Rescued(void) {
    unk32 = 1;

    bool r27 = true;

    for (int i = 0; i < BT_MAX; i++) {
        if (!Bilby_Rescued((BilbyType)i)) {
            r27 = false;
            break;
        }
    }

    if (unk38 == 1) {
        if (r27) {
            unk48 = gb.logicGameCount;

            unk3A = 1;
            unk3E = 105;
        } else {
            unk48 = gb.logicGameCount + (int)(gDisplay.fps * 2.0f);

            unk3A = unk3E = 0;

            mAnimScript.SetAnim(unk12C[1]);
        }

        mSoundHelper.Stop();

        SoundBank_Play(0x8D, NULL, 0);

        pShatter->Explode(NULL, 0.1f, 4.0f);

        unk31 = false;
        dda.StorePickupInfo(Pickup_Bilby);

        for (int i = 0; i < ARRAY_SIZE(unk84); i++) {
            Vector t;
            t.Set(
                GetPos()->x + -50.0f + (RandomI(&gb.mRandSeed) % 100),
                (GetPos()->y + 130.0f) + -50.0f + (RandomI(&gb.mRandSeed) % 100),
                GetPos()->z + -50.0f + (RandomI(&gb.mRandSeed) % 100)
            );
        }
    }
}

Vector* BilbyStruct::GetPos(void) {
    return pModel->matrices[0].Row3();
}

void BilbyStruct::SetState(BilbyState newState) {
    if (mState != newState) {
        mState = newState;
    }

    unk38 = 0;
}

enum BonusPickupType {
    BonusPickupType_0 = 0
};
void BonusPickup_Spawn(Vector*, BonusPickupType, GameObject*);

void Bilby_SetRescued(BilbyType type, bool r4) {
    if (!bBilbiesLoaded) {
        return;
    }

    BilbyStruct** pBilbies = bilbies.GetMem();
    while (*pBilbies) {
        if ((*pBilbies)->mType == type) {
            (*pBilbies)->unk32 = true;
            (*pBilbies)->unk31 = false;
            (*pBilbies)->mCollisionInfo.Disable();

            (*pBilbies)->SetState(BS_0);

            if (r4) {
                SpecialPickupStruct* pEgg = GetThunderEgg(ThunderEggType_1);
                return;
            } else {
                BonusPickup_Spawn((*pBilbies)->pModel->matrices[0].Row3(), BonusPickupType_0, NULL);
                return;
            }
        }

        pBilbies++;
    }
}

bool Bilby_Rescued(BilbyType type) {
    if (!bBilbiesLoaded) {
        return false;
    }

    BilbyStruct** pBilbies = bilbies.GetMem();
    while (*pBilbies) {
        if ((*pBilbies)->mType == type) {
            return (*pBilbies)->unk32;
        }

        pBilbies++;
    }

    return false;
}

void Bilby_EnableCheatLines(bool r3) {
    bDrawCheatLines = r3;
}
