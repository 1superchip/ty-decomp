#ifndef COMMON_MODEL
#define COMMON_MODEL

#include "types.h"
#include "common/Vector.h"
#include "common/Matrix.h"
#include "common/Material.h"
#include "common/Animation.h"

void Model_InitModule(void);
void Model_DeinitModule(void);

// figure out what file BoundingVolume is from
struct BoundingVolume {
	Vector v1;
	Vector v2;

	// might not be a struct method?
	void GetCentre(Vector* pCentre) {
		pCentre->x = v1.x + (v2.x * 0.5f);
		pCentre->y = v1.y + (v2.y * 0.5f);
		pCentre->z = v1.z + (v2.z * 0.5f);
	}
};

struct RefPoint {
	Vector position;
	char* pName;
	s16 matrix1;
	s16 matrix2;
	float weight1;
	float weight2;
};

struct Vertex {
	float pos[3];
	s16 weight;
	u8 matrix1;
	u8 matrix2;
	s16 uv[2];
	s8 normal[3];
	u8 color[4];
};

#pragma pack(push, 1)
struct DisplayList {
    u8 primitive;
    u16 vertexCount;
    u16 vertexIndex;
    u16 normalIndex;
    u16 colorIndex;
    u16 uvIndex;
};
#pragma pack(pop)

struct SubObjectMaterial {
	union {
		char* pMaterialName;
		Material* pMaterial;
	};
	DisplayList* pStripData;
	u16 maxOffset;
	int nmbrOfStrips;
};

struct SubObject {
	BoundingVolume volume;
	Vector origin;
	char* pName;
	char* pOptions;
	int nmbrOfFaces;
	int type;
	s16 matrixIndex;
	s16 nmbrOfMaterials;
	SubObjectMaterial* pMaterials;
	float alphaLightIntensity;
	char unk4C[4];
};

struct ModelData {
	char id[4];
	s16 nmbrOfMatrices;
	s16 nmbrOfSubObjects;
	s16 nmbrOfRefPoints;
	s16 nmbrOfAnimNodes;
	SubObject* pSubObjects;
	RefPoint* pRefPoints;
	Vector* unk14; // animation data
	Vertex* pVertices;
	int vertexCount;
	BoundingVolume volume;
};

struct ModelTemplate {
	char name[32];
	int referenceCount;
	int templateDataSize;
	int unk28;
	ModelData* pModelData;
};

struct ModelExplorer_GC;
struct ModelExplorer;

struct Model {
	ModelTemplate* pTemplate;
	Animation* pAnimation;
	Matrix* pMatrices;
	float* unkC; // inverse scale values
	u8* subobjectData;
	int renderType;
	union
    {
        struct
        {
            u8 b0 : 2;
            u8 bHasAnimation : 1;
            bool b3 : 1; // "bScissoring"?
            u8 b4 : 1;
            u8 b5 : 1;
            u8 b6 : 1;
            u8 b7 : 1;
        } bits;
        u8 flagData;
    } flags;
	Vector colour;
	Matrix matrices[1]; // Model has at least 1 matrix in it, more are allocated
	static int disableTrivialRejection;
	
	static Model* Create(char*, char*);
	void Destroy(void);
	static void Purge(void);
	void SetPosition(Vector* pPos);
	void SetRotation(Vector*);
	void SetAnimation(Animation*);
	void SetLocalToWorldDirty(void);
	bool IsSubObjectEnabled(int);
	void EnableSubObject(int subObjectIndex, bool);
	void EnableOnlySubObject(int, bool);
	void SetInverseScaleValue(int, float);
	bool RefPointExists(char*, int*);
	int GetRefPointIndex(char*);
	Vector* GetRefPointOrigin(int);
	void GetRefPointWorldPosition(int, Vector*);
	bool SubObjectExists(char*, int*);
	int GetSubObjectIndex(char*);
	int GetSubObjectMatrixIndex(int);
	Vector* GetSubObjectOrigin(int);
	static void List(void);
	void GetCentre(Vector* pCentre);
	int GetNmbrOfMatrices(void);
	int GetNmbrOfSubObjects(void);
	char* GetSubObjectName(int subObjectIndex);
	BoundingVolume* GetBoundingVolume(int subObjectIndex);

	// Returns the volume of the model
	BoundingVolume* GetModelVolume(void) {
		return GetBoundingVolume(-1);
	}
	char* GetName(void);
	void SetAlphaLightIntensity(int, float);
	void SetRenderTypeOverride(int);
	Material* GetSubObjectMaterial(int, int);
	Animation* GetAnimation(void) {
		return pAnimation;
	}
	
	int Draw(u16*);
    ModelExplorer_GC* Explore(int* pVertexCount, int* pTriangleCount, int* pStripCount);
    bool ExploreNextFace(ModelExplorer*);
    bool ExploreNextMaterial(ModelExplorer*);
    bool ExploreNextSubObject(ModelExplorer*);
	void ExploreClose(ModelExplorer*);
};

struct ModelExplorerVertex {
    Vector pos;
    float normal[3];
    int unk3;
    Vector color;
    float uv[2];
    int subObjectMatrixIdx;
    int matrix2;
    float weights[2];
};

struct ModelExplorer {
    Material* pMaterial;
    int subObjectIdx;
    int unk8;
    int triangleCount;
    int unk10;
    ModelExplorerVertex vertices[3];
    Model* pModel;
    DisplayList* pStripData;
    int materialIdx;
    int stripNum;
    int currentVertex;
    int vertexCount;
};

struct ModelExplorer_GC {
    Material* pMaterial;
    int subObjectIdx;
    int unk8;
    int triangleCount;
    int unk10;
    ModelExplorerVertex vertices[3];
    Model* pModel;
    DisplayList* pStripData;
    int materialIdx;
    int stripNum;
    int currentVertex;
    uint vertexCount;
    u16* vertexIndices;
    u16* normalIndices;
    u16* uvIndices;
    u16* colorIndices;

    void PrimeMaterial(void);
    void PrimeStrip(void);
    void BuildVertex(int);
};

void Model_UnpackTemplate(ModelTemplate*);
int Model_TrivialRejectTest(BoundingVolume*, Matrix*);

struct EffectDat {
    void* pStripData;
    int maxOffset;
    int matEffect;
    float minW;
    float maxW;
};

#endif // COMMON_MODEL
