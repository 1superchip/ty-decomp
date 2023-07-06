#ifndef COLLISIONOBJECT_H
#define COLLISIONOBJECT_H

#include "common/Collision.h"

struct TyCollisionInfo {
    bool bEnabled;
};

struct ColObjDescriptor {
    float radius;
    float yOffset;
    CollisionMode collisionMode;
    int velocityCalculation; // Velocity Calculation Setting
    int numResolveIterations; // Number of iterations to execute when resolving collisions
    char unk14;
    int ignoreMatID;
    float unk1C;
    float unk20;
    int unk24;
    float unk28;
    float unk2C;
    int unk30;
    float unk34;
    float unk38;
    int unk3C;
    int unk40;
};

#define COLOBJ_MAX_RESULTS (2)

struct ContextInfo {
    int numValid;
    CollisionResult results[COLOBJ_MAX_RESULTS];
    int velocitySetting;

    void ResetNumValid(void) {
        numValid = 0;
    }
    CollisionResult* LastResult(void) {
        if (numValid != 0) {
            return &results[numValid - 1];
        }
        return NULL;
    }
    bool HasEntry(void) {
        return numValid > 0;
    }
    void CopyResult(CollisionResult* pCr) {
        if (numValid < COLOBJ_MAX_RESULTS) {
            results[numValid] = *pCr;
            numValid++;
        }
    }
};

struct CollisionObject {
    ContextInfo contexts[3];
    Vector pos;
    ColObjDescriptor* pColObjDesc;
    u8 colObjCount;
    CollisionObject** ppColObjects;
    TyCollisionInfo* pTyColInfo;
    Vector unk1D0;
    Vector pos1;

    void Init(ColObjDescriptor* pDesc, Vector* pos, TyCollisionInfo* pDefColInfo, u8 _colObjCount, CollisionObject** collisionObjects);
    void Deinit(void);
    int Update(Vector*, float);
    bool Teleport(Vector*, float, int iterations);
    bool IsEnabled(void);
    void SetColObjList(u8 _colObjCount, CollisionObject** _ppColObjects);
    ContextInfo* StoreResult(CollisionResult* pCr, Vector*);
    void ResolveCollisions(Vector*, Vector*, int resolveType);
    void CalculateNewVelocity(Vector*, Vector*, float, ContextInfo* pCtxInfo, CollisionResult* pCr);
    bool CheckCollisions(Vector*, Vector*, CollisionResult* pCr);
    bool CheckObjectCollision(Vector*, Vector*, CollisionObject*, CollisionResult*);
};

#endif // COLLISIONOBJECT_H
