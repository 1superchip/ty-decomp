#ifndef COMMON_ANIMATION
#define COMMON_ANIMATION

#include "types.h"
#include "common/Vector.h"
#include "common/Matrix.h"
//#include "common/GC_ByteFixup.h"

void Animation_InitModule(void);
void Animation_DeinitModule(void);

struct AnimDef {
    enum Type {};
    char* pName;
    Type type;
    short unk8;
    short unkA;
    short unkC;
    short unkE;
};

struct AnimationData {
    struct Node {
        struct KeyFrame {
            Vector* pos;
            Vector* rot;
            Vector* scale;
        };

        Vector origin;
        char* pName;
        int parent;
        int nmbrOfKeyFrames;
        KeyFrame* pKeyFrames;
    };
    
    uint id;
    int nmbrOfFrames;
    int nmbrOfNodes;
    Node* pNodes;
    int nmbrOfAnimDefs;
    AnimDef* pAnimDefs;
};

struct AnimationTemplate {
    char name[0x20];
    int referenceCount;
    AnimationData* pAnimData;
};

struct Animation {
    struct FrameInstance {
        Vector position;
        Vector rotation; // quaternion QuatRotation(?)
        Vector scale;
        Matrix unk30; // custom0/1/2/3
        Vector* pOrigin;
        bool matrixCalc     : 1;
        bool frameCalc      : 1; // Has frame been calculated
        bool useCustom      : 1;
        u8 disableAnim      : 1;
        float targetFrame;
        float targetWeight;
    };

    AnimationTemplate* pTemplate;
    Matrix* pMatrices;
    int unk8;
    int unkC;
    FrameInstance frames[1];
    
    static Animation* Create(char* pFilename, Matrix*);
    
    void Destroy(void);
    void Tween(float, float);
    void TweenNode(float, float, int);
    void SetLocalToWorldDirty(void);
    void SetNodeMatrix(int nodeIndex, Matrix* pMatrix, bool bDisableAnim);
    void CalculateMatrices(void);
    int GetNmbrOfNodes(void);
    bool NodeExists(char*, int*);
    int GetNodeIndex(char*);
    Matrix* GetNodeMatrix(int nodeIndex);
    Vector* GetNodeOrigin(int nodeNumber);
    void GetNodeWorldPosition(int index, Vector* pPos);
    void CalculateNodeMatrix(int);

    void Set(float arg0) {
        Tween(arg0, 1.0f);
    }
};

void Animation_CalculateFrame(Animation::FrameInstance*, AnimationData::Node*);
void Animation_InterpolateFrame(Animation::FrameInstance*, AnimationData::Node*, float);
void Animation_InterpolateFrameData(Animation::FrameInstance*, float, Vector*, Vector*, Vector*, Vector*, Vector*, Vector*);

void Animation_UnpackTemplate(AnimationData*);


#endif // COMMON_ANIMATION
