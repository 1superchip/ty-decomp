#ifndef COMMON_COLLISION
#define COMMON_COLLISION

#include "types.h"
#include "common/Vector.h"
#include "common/Model.h"
#include "common/PtrListDL.h"
#include "common/MKSceneManager.h"
#include "common/StdMath.h"

enum CollisionMode {};

struct CollisionNode
{
    CollisionNode* pPrev; // may actually be the next node pointer?
    CollisionNode* pNext; // may actually be the previous node pointer?
    
    void* PTR_0x8;
    void INIT()
    {
        PTR_0x8 = NULL;
        pPrev = pNext = this;
    }
    void LINK(CollisionNode* pNode, void* ptr)
    {
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
	Vector normal;
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
};

struct CollisionTriangle {
	int flags;
	int subObjectIdx;
	CollisionInfo* pCollisionInfo;
	float vertexPos0[3]; // position of vertex 0
	float vertexPos1[3]; // position of vertex 1
	float vertexPos2[3]; // position of vertex 2
	int unk30;
	int unk34;
	int unk38;
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
    Vector pos;
    Vector pos1;
    Vector dXYZ;
    Vector negDXYZ;
    Vector unk40;
    Vector unk50;
    float unk60;
    float radius;

    void Create(Vector* v1, Vector* v2, float sphereRadius) {
        pos = *v1;
        pos1 = *v2;
        radius = sphereRadius;
        dXYZ.Sub(v2, v1);
        unk60 = dXYZ.Normalise();
        negDXYZ.Scale(&dXYZ, -1.0f);
        unk40.x = Min<float>(v1->x - sphereRadius, v2->x - sphereRadius);
        unk40.y = Min<float>(v1->y - sphereRadius, v2->y - sphereRadius);
        unk40.z = Min<float>(v1->z - sphereRadius, v2->z - sphereRadius);
        unk50.x = Max<float>(v1->x + sphereRadius, v2->x + sphereRadius);
        unk50.y = Max<float>(v1->y + sphereRadius, v2->y + sphereRadius);
        unk50.z = Max<float>(v1->z + sphereRadius, v2->z + sphereRadius);
    }
};

void Collision_InitModule(void);
void Collision_DeinitModule(void);

void Collision_Init(int heapSize, float minX, float /* unused; */ minY, float minZ, 
		float width, float height, int tilesAcross, int tilesDown);
void Collision_Deinit(void);
void Collision_Update(void);
void Collision_Draw(void);

void Collision_AddStaticModel(Model*, CollisionInfo*, int subobject);
void Collision_AddDynamicModel(Model*, CollisionInfo*, int subobject);
void Collision_AddDynamicSubobject(Model*, int subobject, CollisionInfo*);
void Collision_DeleteDynamicModel(Model*);

int Collision_SphereCollide(Vector* pPos, float radius, CollisionResult* pCr, int flags, int maxCollisions);
bool Collision_SweepSphereCollideDynamicModel(SphereRay*, CollisionResult*, DynamicItem*);
bool Collision_RayCollideDynamicModel(Vector*, Vector*, CollisionResult*, Model*, int);
bool Collision_RayCollide(Vector*, Vector*, CollisionResult*, CollisionMode, int);
bool Collision_SweepSphereCollide(Vector* pVec, Vector* pVec1, float sphereRadius,
    CollisionResult* pCr, CollisionMode pMode, int arg3);

#endif // COMMON_COLLISION