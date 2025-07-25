#ifndef COMMON_COLLISION
#define COMMON_COLLISION

#include "types.h"
#include "common/Vector.h"
#include "common/Model.h"
#include "common/PtrListDL.h"
#include "common/MKSceneManager.h"
#include "common/StdMath.h"

enum CollisionMode {
    COLLISION_MODE_ALL       = 0,
    COLLISION_MODE_DYNAMIC   = 1, // Dynamic?
    COLLISION_MODE_POLY      = 2, // Ground?
};

#define ID_WALL             (0x1)       // regular walls we can collide with
#define ID_ICE              (0x8)       // will give ty inertia. For ice and oil
#define ID_SAND             (0x10)      // sandy floor
#define ID_QUICKSAND        (0x80)      // quicksand
#define ID_boomerang_ignore (0x100)
#define ID_LAVA             (0x200)     // hot stuff. For lava and coals
#define ID_WATER_BLUE       (0x400)     // Blue water
#define ID_SNOW             (0x800)     // snow
#define ID_WOOD             (0x1000)    // bridge
#define ID_TUNNEL           (0x2000)    // a tunnel
#define ID_INVISCOLLIDE     (0x4000)    // invisible texture used for collision (like the vertical collision on the bridge)
#define ID_WATER_SLIDE      (0x20000)   // material ID for the water slide
#define ID_GRASS_THICK      (0x80000)   // Dense grass pattern
#define ID_ROCK             (0x100000)  // Rock ground

struct DynamicItem;

struct CollisionNode {
    CollisionNode* pPrev; // may actually be the next node pointer?
    CollisionNode* pNext; // may actually be the previous node pointer?
    DynamicItem* PTR_0x8;

    void INIT() {
        PTR_0x8 = NULL;
        pPrev = pNext = this;
    }

    void LINK(CollisionNode* pNode, DynamicItem* ptr) {
        pNode->PTR_0x8 = ptr;
        pNode->pNext = pNext;
        pNode->pPrev = this;

        pNext->pPrev = pNode;
        pNext = pNode;
    }

    void Remove(void) {
        pNext->pPrev = pPrev;
        pPrev->pNext = pNext;
    }
};

struct CollisionVertex {
    float pos[3];
    u8 color[4];
};

// Contains polygon vertex info
struct CollisionThing {
    CollisionVertex verts[3];
    float normal[3];
};

struct CollisionInfo {
    bool bEnabled;
    int flags;
    MKProp* pProp; // should this be MKProp* ?

    void Init(bool bEnable, int _collisionFlags, MKProp* _pProp) {
        bEnabled = bEnable;
        flags = _collisionFlags;
        pProp = _pProp;
    }

    void Enable(void) {
        bEnabled = true;
    }

    void Disable(void) {
        bEnabled = false;
    }
    
    bool TestFlags(uint testFlags) {
        return (flags & testFlags) != 0;
    }
};

struct CollisionTriangle {
    int flags;
    int subObjectIdx;
    CollisionInfo* pCollisionInfo;
};

struct Item {
    Item* next;
    CollisionThing* collisionThing;
    CollisionTriangle* pTriangle;
};

struct DynamicItem {
    Model* pModel;
    int idx;
    float unk8;
    CollisionInfo* pInfo;
    PtrListDL<CollisionNode> unk10; // "dynamic_links"
    Vector unk14;
    Vector unk24;
    Vector pos;
    int unk44;
    int unk48;
    int unk4C;
    int unk50;

    void Update(void);
    bool UpdateOverlap(void);
    void Unlink(void);
    void Link(void);

    void GotoEnd(void) {
        while (*unk10.GetMem() != NULL) {
            unk10.pMem++;
        }
    }

    // Gets the matrix of the dynamic item
    Matrix* GetMatrix(void) {
        if (idx >= 0) {
            return &pModel->pMatrices[pModel->GetSubObjectMatrixIndex(idx)];
        } else {
            return &pModel->matrices[0];
        }
    }

    // unlinks dynamic links and deinits the item's PtrListDL
    void Deinit(void) {
        Unlink();
        unk10.Deinit();
    }
};

/// @brief Contains result information about a collision
struct CollisionResult {
    Vector pos;
    Vector normal;
    Vector color;
    Model* pModel;
    int itemIdx;
    uint collisionFlags;
    CollisionInfo* pInfo;
    float unk40;
};

struct SphereRay {
    Vector mStart; // Start position
    Vector mEnd; // End position
    Vector mDir; // Direction of the ray
    Vector negDXYZ; // -mDir
    Vector mMinPos; // Minimum point of the SphereRay
    Vector mMaxPos; // Maximum point of the SphereRay
    float mLength; // Length of ray
    float radius;

    void Create(Vector* v1, Vector* v2, float sphereRadius) {
        mStart = *v1;
        mEnd = *v2;
        radius = sphereRadius;
        mDir.Sub(v2, v1);
        mLength = mDir.Normalise();
        negDXYZ.Scale(&mDir, -1.0f);
        mMinPos.x = Min<float>(v1->x - sphereRadius, v2->x - sphereRadius);
        mMinPos.y = Min<float>(v1->y - sphereRadius, v2->y - sphereRadius);
        mMinPos.z = Min<float>(v1->z - sphereRadius, v2->z - sphereRadius);
        mMaxPos.x = Max<float>(v1->x + sphereRadius, v2->x + sphereRadius);
        mMaxPos.y = Max<float>(v1->y + sphereRadius, v2->y + sphereRadius);
        mMaxPos.z = Max<float>(v1->z + sphereRadius, v2->z + sphereRadius);
    }
};

void Collision_InitModule(void);
void Collision_DeinitModule(void);

void Collision_Init(int heapSize, float minX, float /* unused; */ minY, float minZ, 
        float width, float height, int tilesAcross, int tilesDown);
void Collision_Deinit(void);
void Collision_Update(void);
void Collision_Draw(void);

void Collision_AddStaticModel(Model* pModel, CollisionInfo* pInfo, int subobject);
void Collision_AddDynamicModel(Model* pModel, CollisionInfo* pInfo, int subobject);
void Collision_AddDynamicSubobject(Model* pModel, int subobject, CollisionInfo* pInfo);
void Collision_DeleteDynamicModel(Model* pModel);

int Collision_SphereCollide(Vector* pPos, float radius, CollisionResult* pCr, int ignoreFlags, int maxCollisions);
bool Collision_SweepSphereCollideDynamicModel(SphereRay*, CollisionResult*, DynamicItem*);
bool Collision_RayCollideDynamicModel(Vector*, Vector*, CollisionResult* pCr, Model* pModel, int subobjectIdx);
bool Collision_RayCollide(Vector* pStart, Vector* pEnd, CollisionResult* pCr, CollisionMode mode, int ignoreFlags);
bool Collision_SweepSphereCollide(Vector* pStart, Vector* pEnd, float sphereRadius,
    CollisionResult* pCr, CollisionMode pMode, int ignoreFlags);

#endif // COMMON_COLLISION
