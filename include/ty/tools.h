#ifndef TOOLS_H
#define TOOLS_H

#include "common/Vector.h"
#include "common/Model.h"
#include "common/Collision.h"
#include "common/MKAnimScript.h"
#include "common/View.h"
#include "common/Blitter.h"

inline float Tools_Vibrate(int val, float period, float f2) {
    // ASSERT(period)

    float temp = val % (int)period;
    if (temp <= (period * 0.2f)) {
        return (-temp * f2) / (period * 0.2f);
    } else {
        return ((temp - period) * f2) / (period * 0.8f);
    }
}

inline void Tools_ReverseMatrixXZ(Matrix* pMatrix) {
    pMatrix->Row0()->Inverse();
    pMatrix->Row2()->Inverse();
}

void Tools_ApplyDoppler(int, Vector*, Vector*, Vector*, Vector*);
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

void Tools_DrawOverlay(
    Material* pMaterial, 
    Vector* pColor, 
    float x, float y, 
    float uv0, float uv1, 
    float, float, 
    float, float, 
    float endX, float endY, 
    float uv2, float uv3, 
    float, float, 
    float, float
);

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

    bool CheckUnk4Flags(int value) {
        // this function takes a bool and has more logic with that
        return unk4 & (1 << value);
    }
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
void Tools_DropShadow_Add(float, Vector* pPos, Vector* pNormal, float alpha);
void Tools_DropShadow_Draw(void);
void Tools_DropShadow_Update(void);
void Tools_DrawDropShadow(Material* pMat, Vector* pColor /* Optional, pass NULL if unneeded */, float,
        float x, float y, float z, float, float, float, float angle, bool uvSetting);

struct ShadowInfo {
    Vector pos;
    Vector unk10;
    float unk20;
    float alpha;
    void Draw(void); // needed to be defined in tools.cpp so .rodata is ordered correctly
};

struct WayPointLoadInfo {
    int unk0;
    Vector vecs[2]; // need to figure out size of this structure
    bool LoadLine(KromeIniLine*, bool);
};

#define MAX_WAYPOINTS (16)

struct Tools_WayPoints {
    Vector vecs[MAX_WAYPOINTS];
    int unk100;
    int unk104;

    enum LoadMode {};
    
    void Init(void);
    void Reset(void);
    bool Load(WayPointLoadInfo* pLoadInfo, LoadMode loadMode);
    bool LoadLine(KromeIniLine* pLine, LoadMode loadMode);

    int GetIndex(int num) {
        if (num < 0) {
            return unk104 - 1;
        } else if (num >= unk104) {
            return 0;
        } else {
            return num;
        }
    }

    void Subtract1Index(void) {
        unk100 = GetIndex(unk100 - 1);
    }

    void Add1Index(void) {
        unk100 = GetIndex(unk100 + 1);
    }

    Vector* GetPrev(void) {
        return &vecs[GetIndex(unk100 - 1)];
    }

    Vector* GetNext(void) {
        return &vecs[GetIndex(unk100 + 1)];
    }

    Vector* GetCurrent(void) {
        return &vecs[unk100];
    }
};

int Tools_GetAnimationNode(Model*, char*);

enum Tools_SetNodeFlag {
    TOOLS_NODEFLAG_0 = 0,
    TOOLS_NODEFLAG_1 = 1,
    TOOLS_NODEFLAG_2 = 2,
};

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
    enum FadeMode {
        FADEMODE_0 = 0,
        FADEMODE_1 = 1,
        FADEMODE_2 = 2,
        FADEMODE_3 = 3,
        FADEMODE_4 = 4,
        FADEMODE_5 = 5,
        FADEMODE_6 = 6,
        FADEMODE_7 = 7,
        FADEMODE_8 = 8,
        FADEMODE_9 = 9,
        FADEMODE_10 = 10,
        FADEMODE_11 = 11,
    };

    enum FadeState {
        FADESTATE_0 = 0,
        FADESTATE_1 = 1,
        FADESTATE_2 = 2,
        FADESTATE_3 = 3
    };

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

template <typename T>
struct CircularQueue {
    int numMax;
    int unk4; // rear
    int unk8; // front
    T* data;

    void Init(int max) {
        // ASSERT(max > 0, "CircularQueue::Init: Queue size must be >= 1");
        data = (T*)Heap_MemAlloc(max * sizeof(T));
        // ASSERT(data, "CircularQueue::Init: Error allocating memory");
        numMax = max;
        unk4 = unk8 = 0;
    }

    void Deinit(void) {
        Heap_MemFree(data);

        numMax = 0;
        unk4 = unk8 = 0;
    }

    bool IsEmpty(void) {
        return unk4 == 0;
    }

    void Reset(void) {
        unk4 = unk8 = 0;
    }

    void UnkInline(void) {
        if (unk4 > 0) {
            unk8++;
            
            if (unk8 >= numMax) {
                unk8 -= numMax;
            }

            unk4--;
        }
    }

    T* Add(void) {
        if (unk4 == numMax) {
            UnkInline();
        }

        int newIndex = unk8 + unk4;
        if (newIndex >= numMax) {
            newIndex -= numMax;
        }

        unk4++;

        return &data[newIndex];
    }

    int GetUnk4(void) {
        return unk4;
    }

    T* Get(int index) {
        if (index < 0 || index >= unk4) {
            return NULL;
        }

        int newIndex = unk8 + index;
        if (newIndex >= numMax) {
            newIndex -= numMax;
        }

        return &data[newIndex];
    }

    T* GetCurr(void) {
        return unk4 > 0 ? &data[unk8] : NULL;
    }
};

#endif // TOOLS_H
