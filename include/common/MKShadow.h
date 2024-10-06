#ifndef COMMON_MKSHADOW
#define COMMON_MKSHADOW

#include "common/Model.h"
#include "common/View.h"

#define MKSHADOW_POLY_COUNT (1024)
#define MKSHADOW_VERTEX_COUNT (MKSHADOW_POLY_COUNT * 3)

#define MKSHADOW_DETAIL_COUNT (3)

struct MKShadowDetail {
    int unk0;
    int unk4;
    Material* pMaterial0;
    Material* pMaterial1;
    View detailView;
};

/// @brief Contains information about individual vertices in an MKShadow
struct MKShadowVert {
    float x;
    float y;
    float z;
    int matrixIndex;
    Vector mNormal;
};

/// @brief Contains vertex indices for each polygon in an MKShadow
struct MKShadowPoly {
    int vertex0_index;
    int vertex1_index;
    int vertex2_index;
};

struct MKShadow_Animated {
    MKShadowPoly* pPolys;
    int numberOfPolys;
    MKShadowVert* pVerts;
    int numberOfVertices;

    int AddVert(MKShadowVert* pNewVert);
    int AddPoly(MKShadowVert* pVert0, MKShadowVert* pVert1, MKShadowVert* pVert2);
    void Build(char*);
};

enum MKShadow_Type {
    MKSHADOW_ANIMATED   = 0,
    MKSHADOW_STATIC     = 1
};

struct MKShadow {
    MKShadow_Type mType;
    MKShadow_Animated* mpAnimatedShadow;
};

void MKShadow_InitModule(void);
void MKShadow_DeinitModule(void);
void MKShadow_DetectEdges(MKShadow*, Vector*, Model*, Vector*);
void MKShadow_Render(MKShadow*, Vector*, Vector*, float, int, uint*, Vector*);
void MKShadow_RenderStretchBlurEffect(MKShadow*, Vector*, Vector*, float, int*);
void MKShadow_EndScene(void);
MKShadow* MKShadow_CreateAnimatedFromModel(char* pName);
void MKShadow_Destroy(MKShadow*);
void MKShadow_BeginScene(int);
void MKShadow_CaptureZBuffer(void);
void MKShadow_ClearBuffer(void);
void MKShadow_BlitBuffer(int, int);

#endif // COMMON_MKSHADOW
