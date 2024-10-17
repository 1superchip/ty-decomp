#ifndef COMMON_MKSCENEMANAGER
#define COMMON_MKSCENEMANAGER

#include "types.h"
#include "common/Vector.h"
#include "common/Matrix.h"
#include "common/KromeIni.h"
#include "common/Model.h"

#define NUM_TERRAIN_MODELS (12)
// #define MAX_TERRAIN_LAYERS (12)

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
    int drawLayer; // should be named layer?
    uint flags;
    BoundingVolume* pVolume;
    int unk1C;
    float maxScissorDist;
    MKPropDescriptor* pNext;
    MKProp* pProps; // MKProps of this type
    
    static MKPropDescriptor* pDrawListDescs;
};

struct MKProp {
    MKPropDescriptor* pDescriptor;
    Model* pModel;
    Matrix* pLocalToWorld;
    int flags;
    int uniqueID;
    float unk14;
    u16 detailLevel;
    u16 rejectionResult;
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

    bool GetDrawFlag(void) {
        return flags & 0x40000000;
    }
    MKPropDescriptor* GetDesc(void) {
        return pDescriptor;
    }
};

struct SMNode {
    BoundingVolume volume;
    // next/prev?
    SMNode* unk20[2];
    void* pData;
    float drawDist;
};

struct SMTree {
    SMNode* pNodes;
    SMNode* pLastNode;
    int nmbrOfSubObjects;
    int unkC;
    int propCount;
    
    void Init(int);
    void Deinit(void);
    void PairUp(int, int);
    void LinkUp(void);
    void LinkUpRow(int firstLeaf, int leafCount, int);
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
    void SendMessage(MKMessage* pMessage, uint mask, bool bIncludeStatic, Vector* pPos, float radius);
    void SetActivePoint(Vector*);
    void UpdateProp(MKProp*, MKMessage*);
    void RemoveProp(MKProp*);
    int GetPropsInRange(
        MKProp** ppProps, int maxCount, 
        Vector* pTestPt, float radius, 
        int searchMask, int param_7, bool bIncludeStatic
    );
};

extern MKSceneManager gSceneManager;

#endif // COMMON_MKSCENEMANAGER
