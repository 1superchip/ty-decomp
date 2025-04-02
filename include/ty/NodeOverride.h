#ifndef NODEOVERRIDE_H
#define NODEOVERRIDE_H

#include "common/Animation.h"
#include "common/Model.h"
#include "common/Matrix.h"

struct NodeOverride {
    // these could be int bitfields
    u8 b1 : 1;
    u8 b0 : 1;

    Vector unk4;
    Vector unk14;
    float unk24;
    float unk28;
    int nodeNr;
    Animation* pAnimation;
    Matrix* pLocalToWorldMatrix;
    Model* pModel;
    float unk3C;
    int state;
    int unk44;
    void (*HeadTurningCallback)(void);
};

void NodeOverride_Init(NodeOverride* pNodeOverride, Animation* pAnimation, Matrix* pLocalToWorldMatrix, int nodeNr);
void NodeOverride_SetTarget(NodeOverride* pNodeOverride, Vector* pVec, Model* pModel);
void NodeOverride_Update(NodeOverride* pNodeOverride);

#endif // NODEOVERRIDE_H
