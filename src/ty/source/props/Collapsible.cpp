#include "common/StdMath.h"
#include "ty/GameObjectManager.h"
#include "ty/props/Collapsible.h"

static ModuleInfo<Collapsible> moduleInfo;

void Collapsible_LoadResources(KromeIni *pIni) {
    StaticPropDescriptor desc;
    desc.Init(&moduleInfo, "", "", 1, 0);
    LoadDescriptors<StaticPropDescriptor>(pIni, "CollapsibleProps", &desc);
}

void Collapsible::LoadDone(void) {
    StaticProp::LoadDone();
    scale = StaticProp::loadInfo.defaultScale;
    defaultRot = StaticProp::loadInfo.defaultRot;
    // Set the default translation to the translation of the model
    defaultTrans = *GetPos();
}

void Collapsible::Update(void) {
    GameObject::Update();

    switch (state) {
        case COLLAPSIBLE_STATE_0:
            if (TyOn()) {
                unk5C = 30;
                state = COLLAPSIBLE_STATE_1;
            }

            break;
        case COLLAPSIBLE_STATE_1:
            if (--unk5C <= 0) {
                collisionInfo.Disable();

                if (TyOn()) {
                    ty.mContext.floor.bOn = false;
                    ty.mContext.floor.bValid = false;
                }

                shatter->pModel->matrices[0] = pModel->matrices[0];
                shatter->Fall();
                unk5C = 30;
                state = COLLAPSIBLE_STATE_2;
            } else {
                float yDelta = Tools_Vibrate(unk5C, 10.0f, ((unk5C - 30) / 30.0f) * 15.0f);
                pModel->matrices[0].Row3()->y = defaultTrans.y + yDelta;
            }
            break;
        case COLLAPSIBLE_STATE_2:
            if (--unk5C <= 0) {
                pModel->matrices[0].Row3()->y = defaultTrans.y;
                unk5C = 0;
                pModel->SetLocalToWorldDirty();
                state = COLLAPSIBLE_STATE_3;
            } else {
                pModel->matrices[0].Row3()->y =
                    (defaultTrans.y - 400.0f) + SmoothCenteredCurve(unk5C / 60.0f) * 400.0f;
                pModel->SetLocalToWorldDirty();
            }
            break;
        case COLLAPSIBLE_STATE_3:
            if (++unk5C >= 660) {
                unk5C = 0;
                Matrix scaleMatrix;
                scaleMatrix.SetTranslation(&defaultTrans);
                scaleMatrix.SetRotationPYR(&defaultRot);
                scaleMatrix.Scale(&scale);
                pModel->matrices[0].Scale(&scaleMatrix, 0.01f);
                state = COLLAPSIBLE_STATE_4;
            }
            break;
        case COLLAPSIBLE_STATE_4:
            if (++unk5C >= 30) {
                Reset();
            } else {
                Matrix scaleMatrix;
                scaleMatrix.SetTranslation(&defaultTrans);
                scaleMatrix.SetRotationPYR(&defaultRot);
                scaleMatrix.Scale(&scale);
                float angle = 3.9826f * (unk5C / 30.0f);
                pModel->matrices[0].Scale(&scaleMatrix, 0.6f * (1.0f - _table_cosf(angle)));
            }

            pModel->SetLocalToWorldDirty();
            break;
    }
}
