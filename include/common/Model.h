#include "types.h"
#include "common/Vector.h" // this has Matrix in it for now
//#include "common/Matrix.h"

void Model_InitModule(void);
void Model_DeinitModule(void);

// figure out what file BoundingVolume is from
struct BoundingVolume {
	Vector v1;
	Vector v2;
};

// Material_GC.h
struct Material {
    char* pName;
	void Destroy(void);
	static Material* Create(char*);
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
	s16 unkC;
	s8 matrix1;
	s8 matrix2;
	s16 uv[2];
	s8 normal[3];
	s8 color[4];
};

struct SubObjectMaterial {
	union {
		char* pMaterialName;
		Material* pMaterial;
	};
	void* pStripData;
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
            u8 b3 : 1;
            u8 b4 : 1;
            u8 b5 : 1;
            u8 b6 : 1;
            u8 b7 : 1;
        } bits;
        u8 flagData;
    } flags;
	Vector colour;
	Matrix matrices[1]; // Model has at least 1 matrix in it, more are allocated
	
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
	
	void Draw(u16*); // from Model_GC.cpp
};

void Model_UnpackTemplate(ModelTemplate*);
