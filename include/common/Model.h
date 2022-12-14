#ifndef COMMON_MODEL
#define COMMON_MODEL

#include "types.h"
#include "common/Vector.h"
#include "common/Matrix.h"
#include "common/Material.h"

void Model_InitModule(void);
void Model_DeinitModule(void);

// figure out what file BoundingVolume is from
struct BoundingVolume {
	Vector v1;
	Vector v2;
};

// Animation.h
// correct this
struct FrameInstance {
    Vector* pVector;
    char padding[0x7C];
};

struct Animation {
	void* pTemplate;
    Matrix* pMatrix;
    char padding[0x78];
	
	void SetLocalToWorldDirty(void);
	void Destroy(void);
	void CalculateMatrices(void);
	static Animation* Create(char*, Matrix*);
	Matrix* GetNodeMatrix(int);
	
    FrameInstance frames[];
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
	float* unkC;
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
	void SetPosition(Vector*);
	void SetRotation(Vector*);
	void SetAnimation(Animation*);
	void SetLocalToWorldDirty(void);
	bool IsSubObjectEnabled(int);
	void EnableSubObject(int, bool);
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
	void GetCentre(Vector*);
	int GetNmbrOfMatrices(void);
	int GetNmbrOfSubObjects(void);
	char* GetSubObjectName(int);
	BoundingVolume* GetBoundingVolume(int);
	char* GetName(void);
	void SetAlphaLightIntensity(int, float);
	void SetRenderTypeOverride(int);
	Material* GetSubObjectMaterial(int, int);
	
	int Draw(u16*);
    ModelExplorer_GC* Explore(int*, int*, int*);
    int ExploreNextFace(ModelExplorer*);
    int ExploreNextMaterial(ModelExplorer*);
    int ExploreNextSubObject(ModelExplorer*);
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