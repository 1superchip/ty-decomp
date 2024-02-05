#include "types.h"
#include "common/Animation.h"
#include "common/StdMath.h"
#include "common/Heap.h"
#include "common/FileSys.h"
#include "common/Str.h"
#include "common/GC_ByteFixup.h"
#include "common/PtrList.h"

extern "C" void strcpy(char*, char*);
extern "C" char* strtok(char*, char*);
extern "C" void memset(void*, int, int);
extern "C" int stricmp(char*, char*);
extern "C" void strncpy(char*, char*, int);
extern "C" int strcmpi(char*, char*);

static PtrList<Animation> animInstances;
static PtrList<AnimationTemplate> animTemplates;

void Animation_InitModule(void) {
    animInstances.Init(512);
    animTemplates.Init(256);
}

void Animation_DeinitModule(void) {
    AnimationTemplate** ppTemplates = animTemplates.pPointers;
    while (*ppTemplates != NULL) {
        ppTemplates++;
    }
    animInstances.Deinit();
    animTemplates.Deinit();
}

void SlerpQuat(Vector* pVec, Vector* pVec1, float weight, Vector* pVec2) {
    float invWeight = 1.0f - weight;
    float lerpX;
    float lerpY;
    float lerpZ;
    float lerpW;
    if (pVec->QuatDot(pVec1) > 0.0f) {
        lerpX = pVec->x * invWeight + pVec1->x * weight;
        lerpY = pVec->y * invWeight + pVec1->y * weight;
        lerpZ = pVec->z * invWeight + pVec1->z * weight;
        lerpW = pVec->w * invWeight + pVec1->w * weight;
    } else {
        lerpX = -pVec->x * invWeight + pVec1->x * weight;
        lerpY = -pVec->y * invWeight + pVec1->y * weight;
        lerpZ = -pVec->z * invWeight + pVec1->z * weight;
        lerpW = -pVec->w * invWeight + pVec1->w * weight;
    }
    float sqrt = 1.0f / sqrtf(lerpX * lerpX + lerpY * lerpY + lerpZ * lerpZ + lerpW * lerpW);
    pVec2->Set(lerpX * sqrt, lerpY * sqrt, lerpZ * sqrt, lerpW * sqrt);
}

Animation* Animation::Create(char* pFilename, Matrix* pMatrix) {
    Animation* pAnim;
    char animName[64];
    strcpy(animName, pFilename);
    strtok(animName, ".");
    AnimationTemplate** ppTemplates = animTemplates.GetPointers();
    AnimationTemplate* pAnimTemplate;
    while (*ppTemplates != NULL) {
        if (stricmp((*ppTemplates)->name, animName) == 0) {
            break;
        }
        ppTemplates++;
    }
    if (*ppTemplates != NULL) {
        pAnimTemplate = *ppTemplates;
        pAnimTemplate->referenceCount++;
    } else {
        int size;
        FileSys_Exists(Str_Printf("%s.anm", animName), &size);
        pAnimTemplate = (AnimationTemplate*)Heap_MemAlloc(sizeof(AnimationTemplate));
        memset((void*)pAnimTemplate, 0, 8);
        strncpy(pAnimTemplate->name, animName, sizeof(pAnimTemplate->name) - 1);
        pAnimTemplate->pAnimData = (AnimationData*)FileSys_Load(Str_Printf("%s.anm", animName), &size, NULL, -1);
        Animation_UnpackTemplate(pAnimTemplate->pAnimData);
        pAnimTemplate->referenceCount = 1;
        animTemplates.AddEntry(pAnimTemplate);
    }
    int animSize = (pAnimTemplate->pAnimData->nmbrOfNodes - 1) * sizeof(Animation::FrameInstance);
	pAnim = (Animation*)Heap_MemAlloc(animSize + sizeof(Animation));
    animInstances.AddEntry(pAnim);
    pAnim->pTemplate = pAnimTemplate;
    pAnim->pMatrices = pMatrix;
    memset((void*)&pAnim->frames, 0, pAnimTemplate->pAnimData->nmbrOfNodes * sizeof(Animation::FrameInstance));
    for(int i = 0; i < pAnimTemplate->pAnimData->nmbrOfNodes; i++) {
        pAnim->frames[i].pOrigin = &pAnimTemplate->pAnimData->pNodes[i].origin;
        pAnim->frames[i].rotation.Set(0.0f, 0.0f, 0.0f, 1.0f);
        pAnim->frames[i].b0 = 0;
        pAnim->frames[i].b1 = 0;
        pAnim->frames[i].b2 = 0;
        pAnim->frames[i].b3 = 0;
        pAnim->frames[i].targetFrame = 0.0f;
        pAnim->frames[i].targetWeight = 0.0f;
    }
    pAnim->Set(0.0f);
    return pAnim;
}

void Animation::Destroy(void) {
    if (--pTemplate->referenceCount == 0) {
        animTemplates.Destroy(pTemplate);
        Heap_MemFree(pTemplate->pAnimData);
        Heap_MemFree(pTemplate);
    }
    animInstances.Destroy(this);
    Heap_MemFree(this);
}

void Animation::Tween(float frameNmbr, float arg2) {
    float fVar1 = Clamp<float>(0.0f, arg2, 1.0f);
    Animation::FrameInstance* pFrames = frames;
    AnimationData::Node* pNodes = pTemplate->pAnimData->pNodes;
    for(int i = 0; i < pTemplate->pAnimData->nmbrOfNodes; i++) {
        if (!pFrames[i].b1 && fVar1 != 1.0f) {
            Animation_CalculateFrame(&pFrames[i], &pNodes[i]);
        }
        pFrames[i].targetFrame = frameNmbr;
        pFrames[i].targetWeight = fVar1;
        pFrames[i].b1 = 0;
        pFrames[i].b0 = 0;
    }
}

void Animation::TweenNode(float frameNmbr, float weight, int arg3) {
    Animation::FrameInstance* pFrame = &frames[arg3];
    AnimationData::Node* pNode = &pTemplate->pAnimData->pNodes[arg3];
    if (!pFrame->b1 && pFrame->targetWeight != 1.0f) {
        Animation_CalculateFrame(pFrame, pNode);
    }
    pFrame->targetFrame = frameNmbr;
    pFrame->targetWeight = weight;
    pFrame->b1 = 0;
    pFrame->b0 = 0;
    for(int i = 0; i < pTemplate->pAnimData->nmbrOfNodes; i++) {
        if (pTemplate->pAnimData->pNodes[i].parent == arg3) {
            TweenNode(frameNmbr, weight, i);
        }
    }
}

void Animation::SetLocalToWorldDirty(void) {
    for(int i = 0; i < pTemplate->pAnimData->nmbrOfNodes; i++) {
        frames[i].b0 = 0;
    }
}

// arg4 sets disableAnim
void Animation::SetNodeMatrix(int nodeIndex, Matrix* pMatrix, bool arg4) {
    if (pMatrix != NULL) {
        frames[nodeIndex].unk30 = *pMatrix;
        frames[nodeIndex].b2 = 1;
        frames[nodeIndex].b3 = arg4;
        return;
    }
    frames[nodeIndex].b2 = 0;
    frames[nodeIndex].b3 = 0;
}

void Animation::CalculateMatrices(void) {
    for(int i = 0; i < pTemplate->pAnimData->nmbrOfNodes; i++) {
        CalculateNodeMatrix(i);
    }
}

int GetFrame(AnimationData::Node::KeyFrame* pKeyFrame) {
    return *(int*)&pKeyFrame->unk0->w;
}

void Animation_InterpolateFrame(Animation::FrameInstance* pFrame, AnimationData::Node* pNode, float frameNmbr) {
    int start = 0;
    int end = pNode->nmbrOfKeyFrames - 1;
    // binary search
    while (end - start > 1) {
        int mid = (start + end) / 2;
        AnimationData::Node::KeyFrame* pKeyFrame = &pNode->pKeyFrames[mid];
        if (GetFrame(pKeyFrame) > frameNmbr) {
            // if the current keyframe number is greater than the target frame
            // set the end index to the current index
            end = mid;
        } else {
            start = mid;
        }
    }
    AnimationData::Node::KeyFrame* pKeyFrame = &pNode->pKeyFrames[start];
    if (frameNmbr == (float)GetFrame(pKeyFrame)) {
        pFrame->position = *pKeyFrame->unk0;
        pFrame->rotation = *pNode->pKeyFrames[start].unk4;
        pFrame->scale = *pNode->pKeyFrames[start].unk8;
    } else {
        AnimationData::Node::KeyFrame* pKeyFrame2 = &pNode->pKeyFrames[end];
        if (frameNmbr == (float)GetFrame(pKeyFrame2)) {
            pFrame->position = *pKeyFrame2->unk0;
            pFrame->rotation = *pNode->pKeyFrames[end].unk4;
            pFrame->scale = *pNode->pKeyFrames[end].unk8;
        } else {
            float d = (frameNmbr - GetFrame(pKeyFrame)) / (float)(GetFrame(pKeyFrame2) - GetFrame(pKeyFrame));
            Animation_InterpolateFrameData(pFrame,  d, pKeyFrame->unk0, pKeyFrame->unk4, pKeyFrame->unk8, 
                pKeyFrame2->unk0, pKeyFrame2->unk4, pKeyFrame2->unk8);
        }
    }
    pFrame->position.w = frameNmbr;
}

void Animation_InterpolateFrameData(Animation::FrameInstance* pFrame, float arg1, Vector* pPos, Vector* pRot, 
    Vector* pScale, Vector* pPos1, Vector* pRot1, Vector* pScale1) {
    // Linearly interpolate position and scale
    pFrame->position.x = (pPos1->x - pPos->x) * arg1 + pPos->x;
    pFrame->position.y = (pPos1->y - pPos->y) * arg1 + pPos->y;
    pFrame->position.z = (pPos1->z - pPos->z) * arg1 + pPos->z;
    pFrame->scale.x = (pScale1->x - pScale->x) * arg1 + pScale->x;
    pFrame->scale.y = (pScale1->y - pScale->y) * arg1 + pScale->y;
    pFrame->scale.z = (pScale1->z - pScale->z) * arg1 + pScale->z;
    // Slerp rotation
    SlerpQuat(pRot, pRot1, arg1, &pFrame->rotation);
}

int Animation::GetNmbrOfNodes(void) {
	return pTemplate->pAnimData->nmbrOfNodes;
}

// optional parameter pNodeIndex
// pass NULL if the node index is unneeded
// the value at pNodeIndex is set to the index of the found node
// returns true if the node exists
// else returns false
bool Animation::NodeExists(char* pName, int* pNodeIndex) {
    for(int i = 0; i < pTemplate->pAnimData->nmbrOfNodes; i++) {
        if (strcmpi(pTemplate->pAnimData->pNodes[i].pName, pName) == 0) {
            if (pNodeIndex != NULL) {
                *pNodeIndex = i;
            }
            return true;
        }
    }
    return false;
}

// returns the index of the node that has the same name of pNodeName
// returns -1 if the node doesn't exist
int Animation::GetNodeIndex(char* pNodeName) {
    for(int i = 0; i < pTemplate->pAnimData->nmbrOfNodes; i++) {
        if (strcmpi(pTemplate->pAnimData->pNodes[i].pName, pNodeName) == 0) {
            return i;
        }
    }
    return -1;
}

Matrix* Animation::GetNodeMatrix(int nodeIndex) {
    if (!frames[nodeIndex].b0) {
        CalculateNodeMatrix(nodeIndex);
    }
    return &pMatrices[nodeIndex] + 1;
}

Vector* Animation::GetNodeOrigin(int nodeNumber) {
    return frames[nodeNumber].pOrigin;
}

void Animation::GetNodeWorldPosition(int index, Vector* pPos) {
    pPos->ApplyMatrix(frames[index].pOrigin, GetNodeMatrix(index)); // may not use GetNodeMatrix?
}

void Animation_CalculateFrame(Animation::FrameInstance* pFrame, AnimationData::Node* pNode) {
    Animation::FrameInstance localFrame;
    if (pFrame->targetWeight == 1.0f) {
        Animation_InterpolateFrame(pFrame, pNode, pFrame->targetFrame);
    } else {
        Animation_InterpolateFrame(&localFrame, pNode, pFrame->targetFrame);
        Animation_InterpolateFrameData(pFrame, pFrame->targetWeight, &pFrame->position, 
            &pFrame->rotation, &pFrame->scale, &localFrame.position, &localFrame.rotation, &localFrame.scale);
        pFrame->position.w = pFrame->targetFrame;
    }
    pFrame->b1 = 1;
    pFrame->b0 = 0;
}

#define SQRT2 1.41421356f

void Animation::CalculateNodeMatrix(int index) {
    AnimationData::Node* pNode = &pTemplate->pAnimData->pNodes[index];
    Animation::FrameInstance* pFrame = &frames[index];
    if (!pFrame->b1) {
        Animation_CalculateFrame(pFrame, pNode);
    }
    if (pNode->parent != -1 && !frames[pNode->parent].b0) {
        CalculateNodeMatrix(pNode->parent);
    }
    Matrix* r28 = &pMatrices[index] + 1;
    Matrix* parentMatrix = &pMatrices[pNode->parent] + 1;
    if (!pFrame->b3) {
        float sX = pFrame->scale.x;
        float sY = pFrame->scale.y;
        float sZ = pFrame->scale.z;
        
        float xSqrt2 = SQRT2 * pFrame->rotation.x;
        float ySqrt2 = SQRT2 * pFrame->rotation.y;
        float zSqrt2 = SQRT2 * pFrame->rotation.z;
        float wSqrt2 = SQRT2 * pFrame->rotation.w;
        
        float oX = pFrame->pOrigin->x;
        float oY = pFrame->pOrigin->y;
        float oZ = pFrame->pOrigin->z;
        
        #define M00 (1.0f - (ySqrt2 * ySqrt2 + zSqrt2 * zSqrt2))
        #define M01 (xSqrt2 * ySqrt2 - wSqrt2 * zSqrt2)
        #define M02 (xSqrt2 * zSqrt2 + wSqrt2 * ySqrt2)

        #define M10 (xSqrt2 * ySqrt2 + wSqrt2 * zSqrt2)
        #define M11 (1.0f - (xSqrt2 * xSqrt2 + zSqrt2 * zSqrt2))
        #define M12 (ySqrt2 * zSqrt2 - wSqrt2 * xSqrt2)

        #define M20 (xSqrt2 * zSqrt2 - wSqrt2 * ySqrt2)
        #define M21 (ySqrt2 * zSqrt2 + wSqrt2 * xSqrt2)
        #define M22 (1.0f - (xSqrt2 * xSqrt2 + ySqrt2 * ySqrt2))

        // no call to Row methods?
        r28->Row0()->Set(sX * M00, sX * M01, sX * M02, 0.0f);
        r28->Row1()->Set(sY * M10, sY * M11, sY * M12, 0.0f);
        r28->Row2()->Set(sZ * M20, sZ * M21, sZ * M22, 0.0f);

        r28->data[3][0] = (-oX * sX * M00) + (-oY * sY * M10) + (-oZ * sZ * M20) + oX;
        r28->data[3][1] = (-oX * sX * M01) + (-oY * sY * M11) + (-oZ * sZ * M21) + oY;
        r28->data[3][2] = (-oX * sX * M02) + (-oY * sY * M12) + (-oZ * sZ * M22) + oZ;
        r28->data[3][3] = 1.0f;
        
        if (pFrame->position.x != 0.0f || pFrame->position.y != 0.0f || pFrame->position.z != 0.0f)
        {
            if (pNode->parent != -1)
            {
                Vector vec;
                Vector* op1 = pFrame->pOrigin;
                Vector* parentOrigin = frames[pNode->parent].pOrigin;
                vec.x = op1->x - parentOrigin->x;
                vec.y = op1->y - parentOrigin->y;
                vec.z = op1->z - parentOrigin->z;

                vec.x = pFrame->position.x - vec.x;
                vec.y = pFrame->position.y - vec.y;
                vec.z = pFrame->position.z - vec.z;
                
                r28->Translate(&vec);
            } else {
                r28->Translate(&pFrame->position);
            }
        }
        if (pFrame->b2) {
            r28->Multiply(&pFrame->unk30, r28);
        }
        r28->Multiply(parentMatrix);
    } else if (pFrame->b2) {
        r28->Multiply(&pFrame->unk30, parentMatrix);
    } else {
        *r28 = *parentMatrix;
    }
    frames[index].b0 = 1;
}

// https://decomp.me/scratch/NzMve

void FixupVec(Vector& vec) {
    if ((((char*)&vec)[0] & 1) == (((char*)&vec)[3] & 1)) {
        ByteReverseVector(vec);
        ((char*)&vec)[3] ^= 1;
    }
}

/*inline void Fixup__(AnimationData* pData, int addr, int& i, int &j) {
    

    for(i = 0; i < pData->nmbrOfNodes; i++) {
        ByteReverseVector(pData->pNodes[i].origin);
        ByteReverse<int>(pData->pNodes[i].parent);
        ByteReverse<int>(pData->pNodes[i].nmbrOfKeyFrames);
        Fixup<char>(pData->pNodes[i].pName, addr);
        Fixup<AnimationData::Node::KeyFrame>(pData->pNodes[i].pKeyFrames, addr);

        for( j = 0; j < pData->pNodes[i].nmbrOfKeyFrames; j++) {
            Fixup<Vector>(pData->pNodes[i].pKeyFrames[j].unk0, addr);
            Fixup<Vector>(pData->pNodes[i].pKeyFrames[j].unk4, addr);
            Fixup<Vector>(pData->pNodes[i].pKeyFrames[j].unk8, addr);
        }
    }
}*/

void FixupAnimDefs(AnimationData* pData, int addr, int& i) {
    for(i = 0; i < pData->nmbrOfAnimDefs; i++) {
        Fixup<char>(pData->pAnimDefs[i].pName, addr);
        ByteReverse<AnimDef::Type>(pData->pAnimDefs[i].type);
        ByteReverse<short>(pData->pAnimDefs[i].unk8);
        ByteReverse<short>(pData->pAnimDefs[i].unkA);
        ByteReverse<short>(pData->pAnimDefs[i].unkC);
        ByteReverse<short>(pData->pAnimDefs[i].unkE);
    }
}

void FixupHeader(AnimationData* pData, int addr) {
    ByteReverse<uint>(pData->id); // swap endian of file magic bytes
    ByteReverse<int>(pData->nmbrOfFrames);
    ByteReverse<int>(pData->nmbrOfNodes);
    ByteReverse<int>(pData->nmbrOfAnimDefs);
    Fixup<AnimationData::Node>(pData->pNodes, addr);
    Fixup<AnimDef>(pData->pAnimDefs, addr);
}

void Animation_UnpackTemplate(AnimationData* pAnimData) {
    int addr = (int)pAnimData;
    AnimationData* pData = (AnimationData*)pAnimData;
    int i;
	int j;
    ByteReverse<uint>(pData->id); // swap endian of file magic bytes
    ByteReverse<int>(pData->nmbrOfFrames);
    ByteReverse<int>(pData->nmbrOfNodes);
    ByteReverse<int>(pData->nmbrOfAnimDefs);
    Fixup<AnimationData::Node>(pData->pNodes, addr);
    Fixup<AnimDef>(pData->pAnimDefs, addr);
    //FixupHeader(pData, addr);
    /*ByteReverse<uint>(pData->id); // swap endian of file magic bytes
    ByteReverse<int>(pData->nmbrOfFrames);
    ByteReverse<int>(pData->nmbrOfNodes);
    ByteReverse<int>(pData->nmbrOfAnimDefs);
    Fixup<AnimationData::Node>(pData->pNodes, addr);
    Fixup<AnimDef>(pData->pAnimDefs, addr);*/
//Fixup__(pData, addr, i, j);
    for(i = 0; i < pData->nmbrOfNodes; i++) {
        ByteReverseVector(pData->pNodes[i].origin);
        ByteReverse<int>(pData->pNodes[i].parent);
        ByteReverse<int>(pData->pNodes[i].nmbrOfKeyFrames);
        Fixup<char>(pData->pNodes[i].pName, addr);
        Fixup<AnimationData::Node::KeyFrame>(pData->pNodes[i].pKeyFrames, addr);

        for(int j = 0; j < pData->pNodes[i].nmbrOfKeyFrames; j++) {
            Fixup<Vector>(pData->pNodes[i].pKeyFrames[j].unk0, addr);
            Fixup<Vector>(pData->pNodes[i].pKeyFrames[j].unk4, addr);
            Fixup<Vector>(pData->pNodes[i].pKeyFrames[j].unk8, addr);
        }
    }

    for(i = 0; i < pData->nmbrOfNodes; i++) {
        for(int j = 0; j < pData->pNodes[i].nmbrOfKeyFrames; j++) {
            if (((char*)pData->pNodes[i].pKeyFrames[j].unk0)[3] & 1) {
                ((char*)pData->pNodes[i].pKeyFrames[j].unk0)[0] |= 1;
            } else {
                ((char*)pData->pNodes[i].pKeyFrames[j].unk0)[0] &= 0xFE;
            }
            if (((char*)pData->pNodes[i].pKeyFrames[j].unk4)[3] & 1) {
                ((char*)pData->pNodes[i].pKeyFrames[j].unk4)[0] |= 1;
            } else {
                ((char*)pData->pNodes[i].pKeyFrames[j].unk4)[0] &= 0xFE;
            }
            if (((char*)pData->pNodes[i].pKeyFrames[j].unk8)[3] & 1) {
                ((char*)pData->pNodes[i].pKeyFrames[j].unk8)[0] |= 1;
            } else {
                ((char*)pData->pNodes[i].pKeyFrames[j].unk8)[0] &= 0xFE;
            }
        }
    }

    for(i = 0; i < pData->nmbrOfNodes; i++) {
        for(int j = 0; j < pData->pNodes[i].nmbrOfKeyFrames; j++) {
            FixupVec(*pData->pNodes[i].pKeyFrames[j].unk0);
            FixupVec(*pData->pNodes[i].pKeyFrames[j].unk4);
            FixupVec(*pData->pNodes[i].pKeyFrames[j].unk8);
        }
    }
    
    for(i = 0; i < pData->nmbrOfNodes; i++) {
        for(int j = 0; j < pData->pNodes[i].nmbrOfKeyFrames; j++) {
            ((char*)pData->pNodes[i].pKeyFrames[j].unk0)[3] &= 0xFE;
            ((char*)pData->pNodes[i].pKeyFrames[j].unk4)[3] &= 0xFE;
            ((char*)pData->pNodes[i].pKeyFrames[j].unk8)[3] &= 0xFE;
        }
    }

    for(i = 0; i < pData->nmbrOfAnimDefs; i++) {
        Fixup<char>(pData->pAnimDefs[i].pName, addr);
        ByteReverse<AnimDef::Type>(pData->pAnimDefs[i].type);
        ByteReverse<short>(pData->pAnimDefs[i].unk8);
        ByteReverse<short>(pData->pAnimDefs[i].unkA);
        ByteReverse<short>(pData->pAnimDefs[i].unkC);
        ByteReverse<short>(pData->pAnimDefs[i].unkE);
    }
    //FixupAnimDefs(pAnimData, addr);
    //FixupAnimDefs(pAnimData, addr, i);
}

template <>
void Fixup<Vector>(Vector*& data, int baseAddress) {
	if (data != NULL) {
		ByteReverse<Vector*>(data);
		data = (Vector*)((int)data + baseAddress);
	}
}

template <>
void Fixup<AnimationData::Node::KeyFrame>(AnimationData::Node::KeyFrame*& data, int baseAddress) {
	if (data != NULL) {
		ByteReverse<AnimationData::Node::KeyFrame*>(data);
		data = (AnimationData::Node::KeyFrame*)((int)data + baseAddress);
	}
}

template <>
void Fixup<char>(char*& data, int baseAddress) {
	if (data != NULL) {
		ByteReverse<char*>(data);
		data = (char*)((int)data + baseAddress);
	}
}

template <>
void Fixup<AnimDef>(AnimDef*& data, int baseAddress) {
	if (data != NULL) {
		ByteReverse<AnimDef*>(data);
		data = (AnimDef*)((int)data + baseAddress);
	}
}

template <>
void Fixup<AnimationData::Node>(AnimationData::Node*& data, int baseAddress) {
	if (data != NULL) {
		ByteReverse<AnimationData::Node*>(data);
		data = (AnimationData::Node*)((int)data + baseAddress);
	}
}

// Macro to easily order these functions which are out of order (weak functions at end of TU)
#define ByteReverseFuncMacro(type) \
template <>\
__declspec(weak) void ByteReverse(type& start)\
{\
    char* buffer = (char*)(&start);\
    int size = sizeof(type);\
    \
    for(int i = 0; i < size / 2; i++) {\
        char tmp = buffer[i];\
        buffer[i] = buffer[size - i - 1];\
        buffer[size - i - 1] = tmp;\
    }\
}

ByteReverseFuncMacro(AnimDef::Type)
ByteReverseFuncMacro(short)
ByteReverseFuncMacro(Vector*)
ByteReverseFuncMacro(AnimationData::Node::KeyFrame*)
ByteReverseFuncMacro(char*)
ByteReverseFuncMacro(AnimDef*)
