#include "ty/CollisionObject.h"
#include "ty/tools.h"

static Vector upVector = {0.0f, 1.0f, 0.0f, 0.0f};

void CollisionObject::Init(ColObjDescriptor* pDesc, Vector* pos, 
        TyCollisionInfo* pDefColInfo, u8 _colObjCount, CollisionObject** collisionObjects) {
    contexts[1].numValid = 0;
    contexts[0].numValid = 0;
    contexts[2].numValid = 0;
    pColObjDesc = pDesc;
    pTyColInfo = pDefColInfo;
    colObjCount = _colObjCount;
    ppColObjects = collisionObjects;
    Vector tmpStart = *pos;
    Teleport(&tmpStart, 0.5f, 50);
}

void CollisionObject::Deinit(void) {
    contexts[0].ResetNumValid();
    contexts[1].ResetNumValid();
    contexts[2].ResetNumValid();
    pColObjDesc = NULL;
    ppColObjects = NULL;
    colObjCount = NULL;
    pos.Set(0.0f, 0.0f, 0.0f, 1.0f);
}

int CollisionObject::Update(Vector* pVec, float f1) {
    unk1D0.Add(pVec);
    float magSq = unk1D0.MagSquared();
    if (magSq < Sqr<float>(f1)) {
        Vector tmpVel = *pVec;
        if (contexts[0].numValid > 0) {
            Vector testPos = pos;
            testPos.y += pColObjDesc->yOffset;
            CollisionResult cr = *contexts[0].LastResult();
            Vector* crPos = &contexts[0].LastResult()->pos;
            Vector plane = Tools_MakePlane(&contexts[0].LastResult()->normal, crPos);
            Vector end;
            end.Add(&tmpVel, &testPos);
            if (Tools_SweepSphereToPlane(&testPos, &end, pColObjDesc->radius, &plane, &cr)) {
                CalculateNewVelocity(&tmpVel, &tmpVel, cr.unk40, contexts, NULL);
            }
        }
        pos.Add(&tmpVel);
    } else {
        if (f1 > 0.0f) {
            Vector tmpVel = {0.0f, 0.0f, 0.0f, 1.0f};
            tmpVel.Sub(&pos, &pos1);
            pVec->Add(&tmpVel);
            pVec->y -= sqrtf(magSq) * 0.3f;
            pos = pos1;
        }
        unk1D0.SetZero();
        Vector testPos = pos;
        testPos.y += pColObjDesc->yOffset;
        ResolveCollisions(&testPos, pVec, pColObjDesc->numResolveIterations);
        testPos.y -= pColObjDesc->yOffset;
        pos = testPos;
        pos1 = pos;
    }

    bool condition = contexts[2].HasEntry() || contexts[0].HasEntry() || contexts[1].HasEntry();
    int ret = 8;
    if (condition) {
        ret = 7;
    }

    return ret;
}

bool CollisionObject::Teleport(Vector* pDestination, float f1, int iterations) {
    Vector tmp = *pDestination;
    tmp.y += pColObjDesc->yOffset;
    while (Collision_SphereCollide(&tmp, pColObjDesc->radius, NULL, pColObjDesc->ignoreMatID, NULL) && iterations != 0) {
        tmp.y += f1;
        iterations--;
        if (iterations <= 0) {
            return false;
        }
    }
    pos = tmp;
    pos.y -= pColObjDesc->yOffset;
    Vector vel = {0.0f, -f1, 0.0f, 0.0f};
    ResolveCollisions(&tmp, &vel, 1);
    if (contexts[0].numValid > 0) {
        tmp.y -= pColObjDesc->yOffset;
        pos = tmp;
    }
    *pDestination = pos;
    pos1 = pos;
    unk1D0.SetZero();
    return true;
}

// Checks whether this CollisionObject is enabled or not
bool CollisionObject::IsEnabled(void) {
    bool ret;
    if (pTyColInfo != NULL) {
        ret = pTyColInfo->bEnabled;
    } else {
        ret = true;
    }
    return ret;
}

void CollisionObject::SetColObjList(u8 _colObjCount, CollisionObject** _ppColObjects) {
    colObjCount = _colObjCount;
    ppColObjects = _ppColObjects;
}

ContextInfo* CollisionObject::StoreResult(CollisionResult* pCr, Vector* pVec) {
    ContextInfo* pCtxInfo = &contexts[1];
    Vector* pUp = &upVector;
    float upDot = pUp->Dot(&pCr->normal);
    float* pf = &pColObjDesc->unk28;
    if (upDot > pColObjDesc->unk1C) {
        pCtxInfo = &contexts[0];
        pf = &pColObjDesc->unk1C;
    }
    else if (upDot < pColObjDesc->unk34) {
        pCtxInfo = &contexts[2];
        pf = &pColObjDesc->unk34;
    }
    pCtxInfo->velocitySetting = pColObjDesc->velocityCalculation;
    if (pVec->Dot(&pCr->normal) < pf[1]) {
        pCtxInfo->velocitySetting = (int)((int*)pf)[2];
    }
    pCtxInfo->CopyResult(pCr);
    return pCtxInfo;
}

void CollisionObject::ResolveCollisions(Vector* pVec, Vector* pVec1, int arg3) {
    contexts[0].ResetNumValid();
    contexts[1].ResetNumValid();
    contexts[2].ResetNumValid();
    CollisionResult lastCr;
    CollisionResult cr;
    bool bHasLastCr = false;
    for(int i = 0; i < arg3; i++) {
        Vector rayNormal;
        Vector end;
        if (pVec1->MagSquared() < Sqr<float>(0.05f)) {
            return;
        }
        float len = rayNormal.Normalise(pVec1);
        end.Add(pVec, pVec1);
        if (!CheckCollisions(pVec, &end, &cr)) {
            pVec->Add(pVec1);
            return;
        }
        ContextInfo* pCtx = StoreResult(&cr, &rayNormal);
        float neg_dot = -rayNormal.Dot(&cr.normal);
        float dVar7 = Max<float>(neg_dot, 0.000001f);
        float dVar6 = Max<float>(0.0f, cr.unk40 - 0.05f / dVar7);
        if (dVar6 > 0.0f) {
            Vector move;
            move.Scale(&rayNormal, dVar6);
            pVec->Add(&move);
        }
        float f1 = len - dVar6;
        if (f1 < 0.05f) {
            return;
        }
        CalculateNewVelocity(pVec1, &rayNormal, f1, pCtx, bHasLastCr ? &lastCr : NULL);
        bHasLastCr = true;
        lastCr = cr;
    }
}

void CollisionObject::CalculateNewVelocity(Vector* pVec, Vector* pVec1, float f1,
        ContextInfo* pCtxInfo, CollisionResult* pCr) {
    CollisionResult* pCtxResult = &pCtxInfo->results[pCtxInfo->numValid - 1];
    switch (pCtxInfo->velocitySetting) {
        case 1:
            if (pCr != NULL && pCtxResult->normal.Dot(&pCr->normal) < 0.0f) {
                Vector dir;
                Vector away;
                dir.Cross(&pCr->normal, &pCtxResult->normal);
                pVec->Scale(pVec1, f1);
                pVec->Scale(&dir, pVec->Dot(&dir));
                away.Add(&pCr->normal, &pCtxResult->normal);
                away.Scale(0.1f);
                pVec->Add(&away);
            } else {
                Vector normalVel;
                pVec->Scale(pVec1, f1);
                float s = -pVec->Dot(&pCtxResult->normal);
                s += GetSign<float>(s) * 0.05f;
                normalVel.Scale(&pCtxResult->normal, s);
                pVec->Add(&normalVel);
            }
            break;
        case 2:
            pVec->SetZero();
            break;
        case 3:
            Vector normalVel;
            pVec->Scale(pVec1, f1);
            normalVel.Scale(&pCtxResult->normal, -pVec->Dot(&pCtxResult->normal) * 2.0f);
            pVec->Add(&normalVel);
            break;
    }
}

/// @brief Checks collisions for this CollisionObject
/// @param pVec 
/// @param pVec1 
/// @param pCr CollisionResult to use for the collision
/// @return Returns if a collision has occurred
bool CollisionObject::CheckCollisions(Vector* pVec, Vector* pVec1, CollisionResult* pCr) {
    if (ppColObjects) {
        // if ppColObjects isn't NULL, loop through all entries to check object collisions
        for(int i = 0; i < colObjCount; i++) {
            if (ppColObjects[i]->IsEnabled()) { // Only run the object collision on the current object, if it is enabled
                if (CheckObjectCollision(pVec, pVec1, ppColObjects[i], pCr)) {
                    return true; // return true if an object-to-object collision occurred
                }
            }
        }
    }
    // Check collisions with objects and polys
    if (Collision_SweepSphereCollide(pVec, pVec1, pColObjDesc->radius, pCr, pColObjDesc->collisionMode, pColObjDesc->ignoreMatID)) {
        return true;
    }
    return false;
}

/// @brief Checks for a collision between another CollisionObject
/// @param pVec 
/// @param pVec1 
/// @param pOtherObj Other CollisionObject to check collision against
/// @param pCr CollisionResult to store collision info
/// @return Whether a collision occurs between this and pOtherObj
bool CollisionObject::CheckObjectCollision(Vector* pVec, Vector* pVec1, CollisionObject* pOtherObj, CollisionResult* pCr) {
    Vector otherPos = pOtherObj->pos;
    otherPos.y += pOtherObj->pColObjDesc->yOffset;
    Vector rayDir;
    Vector sphereDir;
    rayDir.Sub(pVec1, pVec);
    sphereDir.Sub(&otherPos, pVec);
    if (rayDir.Dot(&sphereDir) < 0.0f) {
        return false;
    }
    if (RayToSphere(pVec, pVec1, &otherPos, pColObjDesc->radius + pOtherObj->pColObjDesc->radius, -1.0f, true)) {
        pCr->normal.Sub(pVec, &otherPos);
        if (pCr->normal.MagSquared() == 0.0f) {
            pCr->normal.Sub(pVec, pVec1);
        }
        pCr->normal.Normalise();
        pCr->unk40 = 0.0f;
        pCr->pos.Scale(&pCr->normal, -pOtherObj->pColObjDesc->radius);
        pCr->pos.Add(pVec);
        pCr->pInfo = (CollisionInfo*)pOtherObj->pTyColInfo;
        pCr->collisionFlags = 0;
        pCr->pModel = NULL;
        pCr->color.Set(1.0f, 1.0f, 1.0f, 1.0f);
        return true;
    }
    return false;
}
