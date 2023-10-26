#ifndef SPLINE_H
#define SPLINE_H

#include "common/Vector.h"
#include "common/Matrix.h"

Vector Spline_GetPosition(Vector*, Vector*, Vector*, Vector*, float);
Vector Spline_GetVelocity(Vector*, Vector*, Vector*, Vector*, float);

// Related:
// https://decomp.me/scratch/PbMFN
// https://decomp.me/scratch/4PTkP

struct SplinePoint {
    Vector mPos;
    Vector unk10;
};

struct Spline {
    int mNumPoints; // Maximum number of points
    int nodeIndex; // Current Node Index
    SplinePoint* mpPoints;
    bool unkC; // seems to relate to normalizing?

    void Init(int numPoints, bool);
    void Reset(void);
    bool AddNode(Vector* pVec);
    Vector GetPosition(float time);
    Vector GetVelocity(float time);
    void Deinit(void);
    bool MergeEnds(void);
    void Smooth(void);
};

struct UniformSplinePoint {
    Vector mPos;
    Vector unk10;
    float unk20;
};

struct UniformSpline {
    int mNumPoints;
    int nodeIndex;
    float unk8;
    UniformSplinePoint* mpPoints;
    bool unk10; // seems to relate to normalizing?

    void Init(int numPoints, bool);
    void Reset(void);
    bool AddNode(Vector*);
    Vector GetPosition(float time);
    Vector GetVelocity(float time);
    void Deinit(void);
    bool MergeEnds(void);
    void RegulateSpeed(void);
};

#endif // SPLINE_H
