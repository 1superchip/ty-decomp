#include "ty/NodeOverride.h"

void NodeOverride_Init(NodeOverride* pNodeOverride, Animation* pAnimation, Matrix* pLocalToWorldMatrix, int nodeNr) {
    pNodeOverride->b1 = 1;
    pNodeOverride->b0 = 0;

    pNodeOverride->nodeNr = nodeNr;

    pNodeOverride->unk24 = 0.0f;
    
    pNodeOverride->unk4.SetZero();

    pNodeOverride->pModel = NULL;

    pNodeOverride->pAnimation = pAnimation;
    pNodeOverride->pLocalToWorldMatrix = pLocalToWorldMatrix;

    pNodeOverride->unk14 = *pLocalToWorldMatrix->Row2();

    pNodeOverride->unk28 = 0.25f;
    pNodeOverride->unk3C = -1.0f;
    pNodeOverride->state = 1;

    pNodeOverride->HeadTurningCallback = NULL;
}

void NodeOverride_SetTarget(NodeOverride* pNodeOverride, Vector* pVec, Model* pModel) {
    bool bDifferentModel = pNodeOverride->pModel != pModel;

    if (pVec) {
        pNodeOverride->unk4 = *pVec;
    }

    pNodeOverride->pModel = pModel;

    if (bDifferentModel && pNodeOverride->HeadTurningCallback) {
        pNodeOverride->unk44 = 0;
        pNodeOverride->HeadTurningCallback();
    }
}

void NodeOverride_SetState(NodeOverride* pNodeOverride, Vector* pVec) {
    Matrix* pNodeMatrix = pNodeOverride->pAnimation->GetNodeMatrix(pNodeOverride->nodeNr);

    float dot = pNodeMatrix->Row2()->Dot(pVec);
    if (pNodeOverride->pModel) {
        if (dot > 0.97f) {
            pNodeOverride->state = 3;
        } else {
            pNodeOverride->state = 4;
        }
    } else if (dot > 0.97f) {
        pNodeOverride->state = 1;
    } else if (pNodeOverride->state != 1) {
        pNodeOverride->state = 2;
    }
}

static bool NodeOverride_ClampTargetRay(NodeOverride* pNodeOverride, Vector* pVec) {
    if (pNodeOverride->unk3C == -1.0f) {
        return false;
    }

    Vector localDir = *pNodeOverride->pLocalToWorldMatrix->Row2();

    float f27 = localDir.Dot(pVec);
    if (f27 > pNodeOverride->unk3C) {
        return false;
    }

    Vector perp = {-localDir.z, 0.0f, localDir.x};
    float f26 = pVec->Dot(&perp);
    
    Vector v;
    
    Vector left;
    Vector right;
    Vector front;
    Vector back;
    
    Vector leftClamp;
    Vector rightClamp;
    
    Vector worldPos;
    
    pNodeOverride->pAnimation->GetNodeWorldPosition(pNodeOverride->nodeNr, &worldPos);

    front = localDir;
    front.Scale(-100.0f);
    front.Add(&worldPos);

    back = localDir;
    back.Scale(100.0f);
    back.Add(&worldPos);

    right = perp;
    right.Scale(100.0f);
    right.Add(&worldPos);

    left = perp;
    left.Scale(-100.0f);
    left.Add(&worldPos);

    
    if (f27 > 0.0f) {
        leftClamp.Sub(&front, &left);
        rightClamp.Sub(&front, &right);
    } else {
        leftClamp.Sub(&left, &back);
        rightClamp.Sub(&right, &back);
    }

    leftClamp.Scale(pNodeOverride->unk3C);
    leftClamp.Add(&left);
    
    rightClamp.Scale(pNodeOverride->unk3C);
    rightClamp.Add(&right);
    
    if (f26 > 0.0f) {
        v.Sub(&worldPos, &leftClamp);
        v.Normalise();
        *pVec = v;
    } else {
        v.Sub(&worldPos, &rightClamp);
        v.Normalise();
        *pVec = v;
    }

    return true;
}

void NodeOverride_CreateCustomMatrix(NodeOverride* pNodeOverride, Vector* pVec, Matrix* pMatrix) {
    Matrix inverseLTW;
    Matrix tempRotationMatrix;
    Vector nodeOrigin;
    Vector nodeOriginInverse;
    
    nodeOrigin = *pNodeOverride->pAnimation->GetNodeOrigin(pNodeOverride->nodeNr);

    nodeOriginInverse.Inverse(&nodeOrigin);

    inverseLTW.SetIdentity();
    inverseLTW.InverseSimple(pNodeOverride->pLocalToWorldMatrix);

    pVec->ApplyRotMatrix(&inverseLTW);

    pVec->Subtract(&pNodeOverride->unk14);
    pVec->Scale(pNodeOverride->unk28);
    pVec->Add(&pNodeOverride->unk14);
    pVec->Normalise();

    tempRotationMatrix.SetIdentity();

    *tempRotationMatrix.Row2() = *pVec;
    tempRotationMatrix.Row2()->w = 0.0f;
    
    tempRotationMatrix.Row1()->Set(0.0f, 1.0f, 0.0f);

    tempRotationMatrix.Row0()->Cross(tempRotationMatrix.Row2(), tempRotationMatrix.Row1());
    tempRotationMatrix.Row0()->Normalise();
    
    tempRotationMatrix.Row1()->Cross(tempRotationMatrix.Row0(), tempRotationMatrix.Row2());

    pMatrix->SetIdentity();

    pMatrix->Translate(&nodeOriginInverse);
    pMatrix->Multiply4x4(&tempRotationMatrix);
    pMatrix->Translate(&nodeOrigin);
}

void NodeOverride_Update(NodeOverride* pNodeOverride) {
    Matrix customMatrix;
    Vector targetRay;
    Vector idealTargetRay;

    pNodeOverride->unk44++;
    
    if (pNodeOverride->pModel && pNodeOverride->b1 != 0) {
        Vector targetWithOffset;
        Vector position;
        pNodeOverride->pAnimation->GetNodeWorldPosition(pNodeOverride->nodeNr, &position);

        targetWithOffset.Set(
            pNodeOverride->unk4.x, 
            pNodeOverride->unk4.y + pNodeOverride->unk24, 
            pNodeOverride->unk4.z
        );

        targetRay.Sub(&position, &targetWithOffset);
    } else {
        targetRay = *pNodeOverride->pLocalToWorldMatrix->Row2();
    }

    targetRay.Normalise();

    idealTargetRay = targetRay;

    NodeOverride_ClampTargetRay(pNodeOverride, &targetRay);
    NodeOverride_CreateCustomMatrix(pNodeOverride, &targetRay, &customMatrix);
    NodeOverride_SetState(pNodeOverride, &idealTargetRay);

    pNodeOverride->unk14 = targetRay;

    if (pNodeOverride->state == 1) {
        pNodeOverride->pAnimation->SetNodeMatrix(pNodeOverride->nodeNr, NULL, false);
    } else {
        pNodeOverride->pAnimation->SetNodeMatrix(pNodeOverride->nodeNr, &customMatrix, pNodeOverride->b0);
    }
}
