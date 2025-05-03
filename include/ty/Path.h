#ifndef PATH_H
#define PATH_H

#include "types.h"
#include "common/Vector.h"

#define MAX_NEXUS_JOINS (16)

struct PathLoadInfo {
    int numPoints;
    Vector points[16];
    int type;
};

struct PathSegment {
    void Init(void);
    int Find(PathSegment* segment, bool, int maxSearchDepth, int*, int*);
    void CalcLengths(void);

    Vector* GetFirstPoint(void) {
        return &points[0];
    }

    Vector* GetLastPoint(void) {
        return &points[numPoints] - 1;
    }

    u8 numPoints;
    s8 pathId;
    char unk2;
    PathSegment* unk4;
    PathSegment* unk8;
    u8 unkC;
    u8 unkD;

    Vector points[0];
};

struct PathNodeNexus {
    PathSegment* pSegment;
    u8 unk4;

    void Merge(PathNodeNexus*);

    Vector* GetPos(void);

    void GetStartNode(PathSegment* pStartSegment) {
        unk4 = 1;
        pSegment = pStartSegment;
    }

    void GetNextNode(void) {
        if (unk4 != 0) {
            unk4 = pSegment->unkC;
            pSegment = pSegment->unk4;
        } else {
            unk4 = pSegment->unkD;
            pSegment = pSegment->unk8;
        }
    }
    
    void GetEndNode(PathSegment* pEndSegment) {
        unk4 = 0;
        pSegment = pEndSegment;
    }
};

struct PathManager {
    PathSegment* pCurrentSegment;
    int unk4;
    int unk8;
    s8 pathId;
    bool unkD;
    char unkE;

    void Init(s8);
    void Init(s8, Vector*, float);

    void SetCurrentPath(PathSegment*, int);

    Vector* GetNearestPoint(Vector*, float);
    Vector* GetNearestNode(Vector*, float);

    int NextPoint(bool);
    int PrevPoint(bool);

    Vector* GetNextNearestNode(Vector*, PathSegment*, float);

    Vector* GetPoint(void) {
        return &pCurrentSegment->points[unk4];
    }
};

void ResolveConnections(PathSegment* pHeader);

#endif // PATH_H
