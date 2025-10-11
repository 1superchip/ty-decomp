#include "ty/Shears.h"
#include "ty/RangeCheck.h"
#include "ty/Ty.h"
#include "ty/tools.h"
#include "ty/soundbank.h"
#include "common/MKAnimScript.h"
#include "common/PtrListDL.h"

static MKAnimScript shearsBad;

static bool bShearsLoaded = false;

static PtrListDL<ShearsStruct> Shears;

int shearsMax = 0;

static short startFrame = -1;
static short midFrame = -1;
static short endFrame = -1;

void Shears_LoadResources(void) {
    shearsBad.Init("prop_0081_shears");

    shearsBad.GetStartAndEnd(shearsBad.GetAnim("close"), &startFrame, &midFrame);
    shearsBad.GetStartAndEnd(shearsBad.GetAnim("open"), NULL, &endFrame);
}

void Shears_Init(void) {
    if (!bShearsLoaded) {
        bShearsLoaded = true;
        Shears.Init(shearsMax, sizeof(ShearsStruct));
    }
}

void Shears_Deinit(void) {
    if (bShearsLoaded) {
        ShearsStruct** ppShears = Shears.GetMem();
        while (*ppShears) {
            if ((*ppShears)->unkB8->collisionTracking == true) {
                Collision_DeleteDynamicModel((*ppShears)->unkB8);
            }

            (*ppShears)->unkB8->Destroy();
            (*ppShears)->unkBC->Destroy();

            ppShears++;
        }

        Shears.Deinit();

        bShearsLoaded = false;
    }
}

extern "C" char* sprintf(char*, ...);

ShearsStruct* Shears_Add(ShearsLoadInfo* pLoadInfo) {
    ShearsStruct* pShear = Shears.GetNextEntry();

    if (pShear) {
        pShear->Load(pLoadInfo);

        pShear->unkB8 = Model::Create(shearsBad.GetMeshName(), shearsBad.GetAnimName());
        pShear->unkB8->renderType = 3;

        pShear->unkBC = Model::Create("Prop_0081_ShearsColide", NULL);

        for (int i = 0; i < ARRAY_SIZE(pShear->blades); i++) {
            char buf[256];

            sprintf(buf, "R_Shears%02d", i + 1);

            pShear->blades[i].bRefPointExists = pShear->unkBC->RefPointExists(
                buf,
                &pShear->blades[i].unk4
            );

            pShear->blades[i].b1 = false;
        }

        pShear->unkB8->SetPosition(&pShear->unk78);
        pShear->unkB8->SetRotation(&pShear->unk88);

        pShear->unkBC->SetPosition(&pShear->unk78);
        pShear->unkBC->SetRotation(&pShear->unk88);

        pShear->mCollisionInfo.Init(false, 0, NULL);

        Collision_AddDynamicModel(pShear->unkB8, &pShear->mCollisionInfo, -1);
    }

    return pShear;
}

void Shears_Update(void) {
    if (bShearsLoaded) {
        ShearsStruct** ppShears = Shears.GetMem();
        while (*ppShears) {
            ShearsStruct* pShear = *ppShears;
            pShear->unkD4++;

            switch (pShear->unkD0) {
                case 1:
                    pShear->Logic();
                    break;
            }

            pShear->unkE4 = Range_WhichZone(&pShear->unk78, &pShear->unkE8);

            ppShears++;
        }
    }
}

void Shears_Draw(void) {
    if (bShearsLoaded) {
        ShearsStruct** ppShears = Shears.GetMem();
        while (*ppShears) {
            ShearsStruct* pShear = *ppShears;

            if (pShear->unkD0 > 0 && pShear->unkE4 > -1 && pShear->unkE4 < 2) {
                pShear->unkB8->pAnimation->Tween(pShear->unkE0, 1.0f);

                pShear->unkC0 = pShear->unkB8->Draw(NULL);

                if (pShear->unkC0 != 0) {
                    for (int i = 0; i < ARRAY_SIZE(pShear->blades); i++) {
                        if (!pShear->blades[i].bRefPointExists) {
                            pShear->blades[i].b1 = false;
                        } else {
                            pShear->unkBC->GetRefPointWorldPosition(pShear->blades[i].unk4, &pShear->blades[i].unk8);
                            pShear->blades[i].b1 = true;
                        }
                    }
                } else {
                    for (int i = 0; i < ARRAY_SIZE(pShear->blades); i++) {
                        pShear->blades[i].b1 = false;
                    }
                }
            }

            ppShears++;
        }
    }
}

void ShearsStruct::Logic(void) {
    if (unkD4 == 1) {
        unkD8 = 0;
        unkDC = unkC4;
        unkE0 = startFrame;
    }

    switch (unkD8) {
        case 0:
            unkDC--;
            if (unkDC <= 0) {
                unkD8 = 3;
                mCollisionInfo.Enable();
                SoundBank_Play(0xBD, &unk78, NULL);
            }
            break;
        case 3:
            unkE0++;
            if (unkE0 >= midFrame) {
                unkE0 = midFrame;
                unkD8 = 2;
                unkDC = unkCC;
            }
            CheckForTy();
            break;
        case 2:
            unkDC--;
            if (unkDC <= 0) {
                unkD8 = 4;
                SoundBank_Play(0xBE, &unk78, NULL);
            }
            CheckForTy();
            break;
        case 4:
            unkE0++;
            if (unkE0 >= endFrame - 60) {
                unkE0 = endFrame;
                unkD8 = 1;
                unkDC = unkC8;
                mCollisionInfo.Disable();
            }
            CheckForTy();
            break;
        case 1:
            unkDC--;
            if (unkDC <= 0) {
                unkE0 = startFrame;
                unkD8 = 3;
                mCollisionInfo.Enable();
                SoundBank_Play(0xBD, &unk78, NULL);
            }
            break;
    }
}

void ShearsStruct::CheckForTy(void) {
    for (int i = 0; i < ARRAY_SIZE(blades); i++) {
        if (blades[i].b1 && ApproxMag(&ty.pos, &blades[i].unk8) < 130.0f) {
            if (unkD8 == 4) {
                ty.SetKnockBackFromPos(&blades[i].unk8, 15.0f, KB_TYPE_0);
            } else {
                ty.Hurt(HURT_TYPE_1, DDA_DAMAGE_4, false, &unk78, 15.0f);
            }

            break;
        }
    }
}

void ShearsStruct::Load(ShearsLoadInfo* pLoadInfo) {
    unk98 = pLoadInfo->unk0;
    unkA8 = pLoadInfo->unk10;

    unkC4 = pLoadInfo->unk20;
    unkCC = pLoadInfo->unk24;
    unkC8 = pLoadInfo->unk28;

    Reset();
}

void ShearsStruct::Reset(void) {
    unk78 = unk98;
    unk88 = unkA8;

    unkD0 = 1;
    unkD4 = 0;

    unkE0 = startFrame;
}
