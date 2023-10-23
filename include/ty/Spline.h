#ifndef SPLINE_H
#define SPLINE_H

#include "common/Vector.h"
#include "common/Matrix.h"

Vector Spline_GetPosition(Vector*, Vector*, Vector*, Vector*, float);
Vector Spline_GetVelocity(Vector*, Vector*, Vector*, Vector*, float);

struct SplinePoint {
    Vector unk0;
    Vector unk10;
};

struct Spline {
    int mNumPoints;
    int nodeIndex;
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
    Vector unk0;
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
