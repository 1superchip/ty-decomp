#include "ty/windmill.h"
#include "ty/global.h"
#include "ty/RangeCheck.h"
#include "ty/tools.h"
#include "common/PtrListDL.h"

bool bWindmillsLoaded = false;

PtrListDL<WindmillStruct> Windmills;

int WindmillStruct::MaxWindmills = 0;

void WindMill_LoadResources(void) {

}

void Windmill_Init(void) {
    if (!bWindmillsLoaded) {
        bWindmillsLoaded = true;

        Windmills.Init(WindmillStruct::MaxWindmills, sizeof(WindmillStruct));
    }
}

void Windmill_Deinit(void) {
    if (bWindmillsLoaded) {
        WindmillStruct** ppWindMills = Windmills.GetMem();

        while (*ppWindMills) {
            Windmill_Delete(*ppWindMills);
            ppWindMills++;
        }
        
        WindmillStruct::MaxWindmills = 0;

        Windmills.Deinit();

        bWindmillsLoaded = false;
    }
}

WindmillStruct* Windmill_Add(Vector* pPos, Vector* pRot) {
    WindmillStruct* pWindMill = Windmills.GetNextEntry();
    if (pWindMill) {
        pWindMill->unk44 = pWindMill->pos = *pPos;
        pWindMill->unk54 = pWindMill->rot = *pRot;

        pWindMill->state = 1;

        pWindMill->updateCounter = pWindMill->drawCounter = 0;

        pWindMill->pModel = Model::Create("Prop_0025_WindMill", NULL);

        pWindMill->pModel->matrices[0].SetRotationPYR(pRot);
        pWindMill->pModel->matrices[0].SetTranslation(pPos);

        pWindMill->pModel->SetLocalToWorldDirty();

        pWindMill->collisionInfo.Init(true, 0, NULL);

        int index = -1;
        if (pWindMill->pModel->SubObjectExists("C_Collide", &index)) {
            Collision_AddStaticModel(pWindMill->pModel, &pWindMill->collisionInfo, index);
            pWindMill->pModel->EnableSubObject(index, false);
        }

        pWindMill->bladeIdx = pWindMill->pModel->GetSubObjectIndex("A_Blade");
        pWindMill->finIdx = pWindMill->pModel->GetSubObjectIndex("A_Fin");
    }

    return pWindMill;
}

void Windmill_Delete(WindmillStruct* bptr) {
    bptr->pModel->Destroy();

    Windmills.Destroy(bptr);
}

void Windmill_Update(void) {
    if (bWindmillsLoaded) {
        // Calculate zone every 8 game logic frames
        int shouldCalcZone = (gb.logicGameCount % 8) == 0 ? 1 : 0; // this may be (& 7)?

        WindmillStruct** ppWindMills = Windmills.GetMem();

        while (*ppWindMills) {
            WindmillStruct* pWindMill = *ppWindMills;

            pWindMill->updateCounter++;
            
            switch (pWindMill->state) {
                case 1:
                    pWindMill->Idle();
                    break;
            }

            if (shouldCalcZone) {
                pWindMill->zone = Range_WhichZone(pWindMill->GetModelPos(), &pWindMill->lodBlend);
            }

            ppWindMills++;
        }
    }
}

void Windmill_Draw(void) {
    if (bWindmillsLoaded) {
        WindmillStruct** ppWindMills = Windmills.GetMem();
        
        while (*ppWindMills) {
            WindmillStruct* pWindMill = *ppWindMills;

            pWindMill->drawCounter++;

            Matrix fin;
            fin.SetIdentity();
            fin.SetRotationYaw(pWindMill->yaw);
            Tools_ApplyRotationToSubObject(pWindMill->pModel, pWindMill->finIdx, &fin, 0);

            Matrix blade;
            blade.SetIdentity();
            blade.SetRotationRoll(pWindMill->roll);
            Tools_ApplyRotationToSubObject(pWindMill->pModel, pWindMill->bladeIdx, &blade, pWindMill->finIdx);

            pWindMill->unk8C = pWindMill->pModel->Draw(NULL);

            ppWindMills++;
        }
    }
}

// Stripped
void Windmill_Reset(void) {
    if (bWindmillsLoaded) {
        WindmillStruct** ppWindMills = Windmills.GetMem();

        while (*ppWindMills) {
            WindmillStruct* pWindMill = *ppWindMills;
            
            pWindMill->state = 1;
            pWindMill->updateCounter = pWindMill->drawCounter = 0;
            
            ppWindMills++;
        }
    }
}

void WindmillStruct::Idle(void) {
    if (updateCounter == 1) {
        roll = 0.0f;
        yaw = 0.0f;
    }

    roll -= PI / 64.0f;
    yaw = Tools_Wobble(gb.logicGameCount * 0.003f, 12345) * 0.5f;
}
