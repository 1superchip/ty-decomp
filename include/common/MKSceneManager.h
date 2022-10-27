#include "types.h"
//#include "common/Vector.h" // has matrix in it currently
#include "common/Model.h"
#include "common/KromeIni.h"

#define MKPROP_TypeMask 3
#define MKPROP_Static 0
#define MKPROP_Dynamic 1
#define MKPROP_Global 2

// not exactly sure
// these are the values used in GameObject::Message
#define MKMSG_Reset -1
#define MKMSG_Deinit -2
struct MKMessage {
	int unk0;
};

struct MKProp;

struct MKPropDescriptor {
	char* pName;
	float maxUpdateDist;
	float maxDrawDist;
	uint searchMask;
	int drawLayer; // should be named "layer"
	uint flags;
	BoundingVolume* pVolume;
	// 24 and 28 = pInstances and pNext?
    int unk1C;
    float maxScissorDist;
    int unk24;
	MKProp* pNext;
	
	static void* pDrawListDescs;
};

struct MKProp {
	MKPropDescriptor* pDescriptor;
	Model* pModel;
	Matrix* pLocalToWorld;
	int flags;
	int unk10; // some count 
	float unk14;
	u16 detailLevel;
	u16 rejectionResult; // result from Model_TrivialRejectTest, named "visibility"
	float unk1C;
	float distSquared;
	MKProp* pNext;
	MKProp* pPrev;
	MKProp* pPrevUpdated;
	MKProp* pNextUpdated;
	MKProp* pNextOfThisType;
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Draw(void);
	virtual void Message(MKMessage*);
};

struct SMNode {
	BoundingVolume volume;
	// next/prev?
	SMNode* unk20;
	SMNode* unk24;
	MKProp* pProp;
	float drawDist;
};

struct SMTree {
	SMNode* pNodes;
	SMNode* pLastNode;
	int nmbrOfSubObjects; // leaves
	int unkC;
	int propCount; // nodes
	
	void Deinit(void);
	void PairUp(int, int);
	void LinkUp(void);
};

struct MKSceneManagerInit {
	char padding[0x33];
	float unk34;
	float unk38;
};

struct MKSceneManager {
	MKProp props[2];
	int propIdx;
	Matrix matrix7C;
	SMTree trees[12];
	Vector activePoint;
	MKProp* prop1BC;
	MKProp* prop1C0;
	bool bHasBeenOptimised;
	float updateDistMult;
    float updateDrawMult;
	
	static void InitModule(void);
	static void DeinitModule(void);
	void Init(MKSceneManagerInit*);
	void Deinit(void);
	void AddTerrainModel(Model*, int, float);
	void AddProp(MKProp*);
	void AddStaticProp(MKProp*, int);
    void AddDynamicProp(MKProp*, int);
    void AddGlobalProp(MKProp*, int);
	void Optimise(void);
	void MakeTerrainTree(void);
	void MakePropTree(void);
	static void CalcZoneVis(void);
	void DrawTerrain(int);
	void DrawRecursiveTerrain(SMNode*, int);
	void DrawRecursiveProps(SMNode*, int);
	void DrawStaticProps(int);
	void DrawDynamicProps(int);
	void DrawGlobalProps(int);
	void UpdateProps(void);
	void SendMessage(MKMessage*, uint, bool, Vector*, float);
	void SetActivePoint(Vector*);
	void UpdateProp(MKProp*, MKMessage*);
	void RemoveProp(MKProp*);
	int GetPropsInRange(MKProp**, int, Vector*, float, int, int, bool);
};

//static void SMDrawProp(void*, int, float, float);