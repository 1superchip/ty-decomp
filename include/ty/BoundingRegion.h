#ifndef BOUNDINGREGION_H
#define BOUNDINGREGION_H

#include "ty/Path.h"

struct RectXZ {
    float minPoint[2];
    float maxPoint[2];
};

struct BoundingRegion {
    PathSegment* pPath;
    RectXZ rect;
    
    void Init(PathSegment*);
    bool IsPointWithin(Vector*);
    bool IsPointInBoundaryRect(Vector*);
    bool ArePointsWithinAndAdjacent(Vector*, Vector*);
    bool setPolyBoundingRect(RectXZ*, PathSegment*); // return is never used?
    int getIntersectCount(Vector*, Vector*);
    bool isIntersect(Vector*, Vector*, Vector*, Vector*);
    bool isCounterClockWise(Vector*, Vector*, Vector*);
};

#endif // BOUNDINGREGION_H
