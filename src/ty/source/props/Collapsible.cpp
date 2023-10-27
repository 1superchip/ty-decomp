#include "common/StdMath.h"
#include "ty/GameObjectManager.h"
#include "ty/props/Collapsible.h"

static ModuleInfo<Collapsible> moduleInfo;

extern struct Ty {
    char padding[0x40];
    Vector pos;
    char unk[0x7F4];
    bool unk844;
    bool unk845;
    bool unk846;
    float unk848;
    float unk84C;
    char unk1[0x34];
    int unk884;

    void SetAbsolutePosition(Vector *, int, float, bool);
} ty;

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
    defaultTrans = *pModel->matrices[0].Row3();
}

void Collapsible::Update(void) {
    GameObject::Update();
    bool unk;
    switch (state) {
    case 0:
        unk = false;
        if (ty.unk844 != false) {
            float diff;
            if (ty.unk846 != false) {
                diff = ty.pos.y - ty.unk84C;
            } else {
                diff = 10000.0f;
            }
            if (diff < 10.0f) {
                unk = true;
            }
        }
        if (((ty.unk845 != false || unk) && (int *)ty.unk884 == (int *)&collisionInfo) != false) {
            unk5C = 0x1e;
            state = 1;
        }
        break;
    case 1:
        if (--unk5C <= 0) {
            collisionInfo.bEnabled = false;
            unk = false;
            if (ty.unk844 != false) {
                float diff;
                if (ty.unk846 != false) {
                    diff = ty.pos.y - ty.unk84C;
                } else {
                    diff = 10000.0f;
                }
                if (diff < 10.0f) {
                    unk = true;
                }
            }
            if (((ty.unk845 != false || unk) && (int *)ty.unk884 == (int *)&collisionInfo) !=
                false) {
                ty.unk845 = false;
                ty.unk846 = false;
            }
            shatter->pModel->matrices[0] = pModel->matrices[0];
            shatter->Fall();
            unk5C = 0x1e;
            state = 2;
        } else {
            float f = 15.0f * ((float)(unk5C - 30) / 30.0f);
            float f2 = (float)(unk5C % 10);
            float f3;
            if (f2 <= 2.0f) {
                f3 = (-f2 * f) / 2.0f;
            } else {
                f3 = (f * (f2 - 10.0f)) / 8.0f;
            }
            pModel->matrices[0].Row3()->y = defaultTrans.y + f3;
        }
        break;
    case 2:
        if (--unk5C <= 0) {
            pModel->matrices[0].Row3()->y = defaultTrans.y;
            unk5C = 0;
            pModel->SetLocalToWorldDirty();
            state = 3;
        } else {
            float f = (float)unk5C / 60.0f;
            pModel->matrices[0].Row3()->y =
                (defaultTrans.y - 400.0f) + (1.0f - (4.0f * Sqr<float>(f - 0.5f))) * 400.0f;
            pModel->SetLocalToWorldDirty();
        }
        break;
    case 3:
        if (++unk5C >= 0x294) {
            unk5C = 0;
            Matrix scaleMatrix;
            scaleMatrix.SetTranslation(&defaultTrans);
            scaleMatrix.SetRotationPYR(&defaultRot);
            scaleMatrix.Scale(&scaleMatrix, &scale);
            pModel->matrices[0].Scale(&scaleMatrix, 0.01f);
            state = 4;
        }
        break;
    case 4:
        if (++unk5C >= 0x1E) {
            Reset();
        } else {
            Matrix scaleMatrix;
            scaleMatrix.SetTranslation(&defaultTrans);
            scaleMatrix.SetRotationPYR(&defaultRot);
            scaleMatrix.Scale(&scaleMatrix, &scale);
            float angle = 3.9826f * ((float)unk5C / 30.0f);
            pModel->matrices[0].Scale(&scaleMatrix, 0.6f * (1.0f - _table_cosf(angle)));
        }
        pModel->SetLocalToWorldDirty();
    }
}