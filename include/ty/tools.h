#ifndef TOOLS_H
#define TOOLS_H

#include "common/Vector.h"
#include "common/Model.h"
#include "common/Collision.h"
#include "common/MKAnimScript.h"
#include "common/View.h"
#include "common/Blitter.h"

void Tools_ApplyDoppler(Vector*, Vector*, Vector*, Vector*);
Model* Tools_GetGroundLightModel(Vector*, int* pSubObjectIndex, float maxDist);
bool Tools_CollideXZ(Vector*, Vector*, Vector*, Vector*, float, float, float, float);

// Copies pSrc to pDest and then sets the first '.' to '\0' in pDest
void Tools_StripExtension(char* pDest, const char* pSrc);

Vector Tools_GroundColor(CollisionResult* pCr);
float Tools_RandomGaussian(void);
void Tools_ApplyRotationToSubObject(Model* pModel, int, Matrix*, int);
void Tools_CollisionBounce(Vector*, Vector*, CollisionResult* pCr, float);
float Tools_SCurve(float);
void Tools_GetPYR(Matrix* pRotMatrix, float* pPitch, float* pYaw, float* pRoll);
void DebugInfo_Line3d(char*, Vector*, Vector*, int, bool);
float GetFloor(Vector*, float, CollisionResult* pCr);
float Tools_GetCollideHeight(Vector*, Vector*, bool*, float);

void Tools_DrawOverlay(Material*, Vector*, float, 
        float, float, float, float, float, float,
        float, float, float, float, float, float,
        float, float, float);
void Tools_RenderToTarget(Material*, float, float, float, float, float, float, float);

float Tools_Wobble(float, int);
int Round(float);
float GetPitch2Points(Vector*, Vector*);

/*
Approximates the magnitude of the vector (pVector1 - pVector) using Heron's Method
of approximating a square root
*/
float ApproxMag(Vector* pVector, Vector* pVector1);
float ApproxMag(float, float, float);
float ApproxMagXZ(Vector*, Vector*);
float ApproxMag(float, float);

float ExactMag(const Vector*, const Vector*);
float ExactMag(float, float, float);
float ExactMagXZ(Vector*, Vector*);
float ExactMag(float, float);

void Tools_Dampen(Vector*, Vector*, float);
float Tools_CriticalDamp(float, float, float, float);

bool Tools_BuildLTWMatrix(Matrix* m, Vector* forward, Vector*);
bool Tools_BuildMatrixFromFwd(Matrix* pM, Vector* pFwd, Vector* pUp);

bool Tools_TestFloor(Vector*, CollisionResult* pCr, float, bool bCollisionMode);
float Tools_GetFloor(const Vector&, CollisionResult* pCr, float, bool bCollisionMode, uint collisionFlags);
bool RayToSphere(Vector*, Vector*, Vector*, float radius, float, bool);
bool Tools_ClipSphereToDynamicModel(const Vector&, float, Vector*, Model* pModel, int subobjectIdx);
bool Tools_CapsuleTest(Vector*, Vector*, Vector*, float, bool);
bool Tools_RayToVertCyl(Vector*, Vector*, Vector*, float, float);
bool Tools_PlaneTest(Vector*, Vector*, Vector*, CollisionResult* pCr);
bool Tools_SweepSphereToPlane(Vector*, Vector*, float, Vector*, CollisionResult* pCr);
Vector Tools_MakePlane(Vector*, Vector*);
bool Tools_CapsuleTestMagSq(Vector*, Vector*, Vector*, float);

struct Tools_AnimEntry {
    char* pName;
    MKAnim* pAnim;
};

struct Tools_AnimDesc {
    Tools_AnimEntry* pEntries;
    int unk4;
    void Init(Tools_AnimEntry* _pAnims, int _count, MKAnimScript*);
};

struct Tools_AnimManager {
    Tools_AnimDesc* pDescriptor;
    void Init(Tools_AnimDesc*);
    MKAnim* GetAnim(int index) {
        return pDescriptor->pEntries[index].pAnim;
    }
};

struct Tools_AnimEvent {
    char* pName;
    char* pEventName;
};

struct Tools_AnimEventDesc {
    Tools_AnimEvent* pEvents;
    int mCount;
    void Init(Tools_AnimEvent*, int, MKAnimScript*);
};

struct Tools_AnimEventManager {
    Tools_AnimEventDesc* pDescriptor;
    int unk4;
    int unk8;
    void Init(Tools_AnimEventDesc*);
    void Update(MKAnimScript*);
};

int Tools_GetAnimationLength(MKAnimScript*);

struct Tools_ParticleRef {
    Vector refPointWorldPos;
    Vector targetWorldPos; // direction?
    int refPointIdx;
    int targetPointIdx;
    void Init(Model* pModel, char* pRefPointName);
    void Update(Model* pModel);
};

void Tools_DropShadow_Init(void);
void Tools_DropShadow_Deinit(void);
void Tools_DropShadow_Add(float, Vector*, Vector*, float);
void Tools_DropShadow_Draw(void);
void Tools_DropShadow_Update(void);
void Tools_DrawDropShadow(Material* pMat, Vector* pColor /* Optional, pass NULL if unneeded */, float,
        float x, float y, float z, float, float, float, float angle, bool uvSetting);

struct ShadowInfo {
    Vector unk0;
    Vector unk10;
    float unk20;
    float unk24;
    void Draw(void); // needed to be defined in tools.cpp so .rodata is ordered correctly
};

struct WayPointLoadInfo {
    int unk0;
    Vector vecs[2]; // need to figure out size of this structure
    bool LoadLine(KromeIniLine*, bool);
};

struct Tools_WayPoints {
    Vector vecs[16];
    int unk100;
    int unk104;
    enum LoadMode {};
    void Init(void);
    bool Load(WayPointLoadInfo*, LoadMode);
    bool LoadLine(KromeIniLine*, LoadMode);
};

int Tools_GetAnimationNode(Model*, char*);

enum Tools_SetNodeFlag {};

void Tools_SetNode(Animation*, int, Matrix*, Tools_SetNodeFlag);

void Tools_MatriceRotate(float, Vector*, Matrix*);

Vector* Tools_GetRefPointPos(Model* pModel, char*);

void Tools_ApplyFrictionAndGravity(Vector*, Vector*, Vector*, float);

// Tools_RandomRadialVectorXZ__FR6Vector
void Tools_RandomRadialVectorXZ(Vector&); // might be the wrong signature
Vector* Tools_RandomNormal(Vector*);

float Tools_TurnToAngle(float currentAngle, float maxAngle, float maxTurnAngle);
float Tools_SmoothToValue2(float, float, float, float, float);
float Tools_SmoothToAngle2(float, float, float, float, float);

Vector* Tools_RandomBox(Vector*, float);
void Tools_RandomVector(Vector* pResult, Vector* pMin, Vector* pMax);
int Tools_GetTimeInSeconds(void);

struct Tools_DynamicStringTable {
    char* pStringTable;
    int currentLen;
    int nmbrOfStrings;

    void Init(void);
    char* AppendString(char*); // return type?
    char* FindString(char*);
};

void Tools_EnableWideScreen(View* pView, bool bEnableWideScreen);

float Tools_CylinderTest(Vector*, Vector*, float radius, Vector* pTestPoint);
bool Tools_CapsuleTest(Vector*, Vector*, float, float, Vector*);

#define NUM_PADKEYS 16
struct PadKey {
    int unk0;
    u8 unk4;
};
int Tools_MapPadToKey(int);
void Tools_ProcessString(char*);

struct FaderObject {
    enum FadeMode {};
    enum FadeState {};

    FaderObject::FadeMode fadeMode;
    FaderObject::FadeState currFadeState;
    FaderObject::FadeState prevFadeState;
    int unkC;
    int unk10;
    int unk14;
    int unk18;

    void Reset(void);
    void Update(void);
    void Fade(FaderObject::FadeMode, float, float, float, bool);
    FaderObject::FadeState GetNextState(FaderObject::FadeState, FaderObject::FadeMode);
    float GetFadePercentage(void);
};


#endif // TOOLS_H
