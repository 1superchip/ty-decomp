#include "types.h"
#include "common/Vector.h"
#include "ty/BoundingRegion.h"

bool IsWithin(float x, float min, float max) {
    return (x >= min && x <= max);
}

bool BoundingRegion::IsPointInBoundaryRect(Vector* pPoint) {
    u32 isWithin = pPoint->x >= rect.minPoint[0]
        && pPoint->x < rect.maxPoint[0]
        && pPoint->z >= rect.minPoint[1]
        && pPoint->z < rect.maxPoint[1];
    return static_cast<bool>(isWithin);
}

void BoundingRegion::Init(PathSegment* pathSegment) {
	pPath = pathSegment;
	setPolyBoundingRect(&rect, pPath);
}

bool BoundingRegion::IsPointWithin(Vector* pPoint) {
    if ((IsPointInBoundaryRect(pPoint) & 1) == 0) {
        return false;
    }

    Vector max = {10000000.0f, pPoint->z, 0.0f, 0.0f};
    return getIntersectCount(pPoint, &max) & 1;
}

// inline here "ArePointsWithin"?
bool BoundingRegion::ArePointsWithinAndAdjacent(Vector* pPoint, Vector* pPoint1) {
    if (((IsPointInBoundaryRect(pPoint) & 1) == 0) || ((IsPointInBoundaryRect(pPoint1) & 1) == 0)) {
        return false;
    }
    
    return !getIntersectCount(pPoint, pPoint1);
}

bool BoundingRegion::setPolyBoundingRect(RectXZ *pRect, PathSegment* pSegment) {
    Vector* pLastVector = (pSegment->points + pSegment->numPoints) - 1;
    Vector* pPoints = pSegment->points;
    
    float xMin, xMax, zMin, zMax;
    zMin = 10000000.0f;
    xMin = 10000000.0f;
    zMax = -10000000.0f;
    xMax = -10000000.0f;
    
    // find min/max
    while (pPoints <= pLastVector) {
        if (pPoints->x < xMin) {
            xMin = pPoints->x;
        }

        if (pPoints->x > xMax) {
            xMax = pPoints->x;
        }

        if (pPoints->z < zMin) {
            zMin = pPoints->z;
        }

        if (pPoints->z > zMax) {
            zMax = pPoints->z;
        }
        
        pPoints++;
    }
    
    pRect->minPoint[0] = xMin;
    pRect->minPoint[1] = zMin;
    pRect->maxPoint[0] = xMax;
    pRect->maxPoint[1] = zMax;

    // check if points are valid
    if (pRect->minPoint[0] >= pRect->maxPoint[0] || pRect->minPoint[1] >= pRect->maxPoint[1]) {
        return false;
    }
    
    return true;
}

int BoundingRegion::getIntersectCount(Vector* pPoint, Vector* pPoint1) {
    int intersectCount = 0;
    int i;

    for (i = 0; i < pPath->numPoints - 1; i++) {
        if (isIntersect(pPoint, pPoint1, &pPath->points[i], &pPath->points[i + 1]) != false) {
            intersectCount++;
        }
    }

    if (isIntersect(pPoint, pPoint1, &pPath->points[pPath->numPoints - 1], &pPath->points[0]) != false) {
        intersectCount++;
    }
    
    return intersectCount;
}

bool BoundingRegion::isIntersect(Vector* pPoint, Vector* pPoint1, Vector* pPoint2, Vector* pPoint3) {
	/*
	// might be cleaner than current code?
	return isCounterClockWise(pPoint, pPoint1, pPoint2) != isCounterClockWise(pPoint, pPoint1, pPoint3) &&
            isCounterClockWise(pPoint2, pPoint3, pPoint) != isCounterClockWise(pPoint2, pPoint3, pPoint1);
	*/
    bool intersected = false;

    if (isCounterClockWise(pPoint, pPoint1, pPoint2) != isCounterClockWise(pPoint, pPoint1, pPoint3)) {
        if (isCounterClockWise(pPoint2, pPoint3, pPoint) != isCounterClockWise(pPoint2, pPoint3, pPoint1)) {
            intersected = true;
        }
    }

    return intersected;
}

bool BoundingRegion::isCounterClockWise(Vector* pPoint, Vector* pPoint1, Vector* pPoint2) {
    /*
	// might be cleaner than the current code?
	float p01x = (pPoint1->x - pPoint->x);
    float p02x = (pPoint2->x - pPoint->x);
    float p01z = (pPoint1->z - pPoint->z);
    float p02z = (pPoint2->z - pPoint->z);
    return p01x * p02z >
            p01z * p02x ? true : false;
			*/
    
    bool isCCW;
    if ((pPoint1->x - pPoint->x) * (pPoint2->z - pPoint->z) > (pPoint1->z - pPoint->z) * (pPoint2->x - pPoint->x)) {
        isCCW = true;
    } else {
        isCCW = false;
    }

    return isCCW;
}
