#include "ty/Path.h"
#include "ty/global.h"
#include "common/Heap.h"

extern "C" void memset(void*, int, int);
extern "C" void memcpy(void*, void*, int);

static bool bPathLoaded = false;
static bool bPathManagerLoaded = false;

int pathCount = 0;

static PathSegment* pSegmentData = NULL;
static PathSegment* pTerminator = NULL;

void PathSegment::Init(void) {
    numPoints = 0;
    pathId = -1;
    unk2 = 0;
    unk4 = this;
    unk8 = this;
    unkC = 1;
    unkD = 0;
}

void PathSegment::CalcLengths(void) {
    points[0].w = 0.0f;

    for (int i = 1; i < numPoints; i++) {
        points[i].w = sqrtf(
            Sqr<float>(points[i].x - points[i - 1].x) +
            Sqr<float>(points[i].y - points[i - 1].y) +
            Sqr<float>(points[i].z - points[i - 1].z)
        );
        
        points[0].w += points[i].w;
    }
}

int PathSegment::Find(PathSegment* segment, bool r5, int maxSearchDepth, int* r7, int* r8) {
    if (this == segment) {
        if (r7) {
            *r7 = 0;
        }

        return -1;
    }

    if (maxSearchDepth != 0) {
        int r26 = 1;

        PathNodeNexus start;
        
        if (r5) {
            start.GetStartNode(this);
        } else {
            start.GetEndNode(this);
        }

        PathNodeNexus current;
        current = start;

        current.GetNextNode();

        while (current.pSegment != start.pSegment) {
            if (current.pSegment->Find(segment, !current.unk4, maxSearchDepth - 1, r7, r8 ? r8 + 1 : NULL)) {
                if (r7) {
                    r7[0]++;
                }

                if (r8) {
                    *r8 = r26;
                }

                return r26;
            }

            current.GetNextNode();
            r26++;
        }
    }

    return 0;
}

void ResolveConnections(PathSegment* pHeader) {
    while (pHeader->numPoints != 0) {
        pHeader->CalcLengths();

        PathSegment* pSeg = (PathSegment*)((char*)pHeader + sizeof(*pHeader) + (size_t)pHeader->numPoints * sizeof(Vector));
        while (pSeg->numPoints != 0) {
            if (pHeader->pathId == pSeg->pathId) {
                if (pHeader->GetFirstPoint()->IsInsideSphere(pSeg->GetLastPoint(), 250.0f)) {
                    PathNodeNexus nexus;
                    PathNodeNexus ts_nexus;
                    
                    nexus.GetStartNode(pHeader);
                    ts_nexus.GetEndNode(pSeg);
                    nexus.Merge(&ts_nexus);
                    break;
                } else if (pHeader->GetFirstPoint()->IsInsideSphere(pSeg->GetFirstPoint(), 250.0f)) {
                    PathNodeNexus nexus;
                    PathNodeNexus ts_nexus;
                    
                    nexus.GetStartNode(pHeader);
                    ts_nexus.GetStartNode(pSeg);
                    nexus.Merge(&ts_nexus);
                    break;
                }
            }
            
            pSeg = (PathSegment*)((char*)pSeg + sizeof(*pSeg) + pSeg->numPoints * sizeof(Vector));
        }
        
        pSeg = (PathSegment*)(&pHeader->points[pHeader->numPoints]);
        while (pSeg->numPoints != 0) {
            if (pHeader->pathId == pSeg->pathId) {
                if (pHeader->GetLastPoint()->IsInsideSphere(pSeg->GetFirstPoint(), 250.0f)) {
                    PathNodeNexus nexus;
                    PathNodeNexus ts_nexus;
                    
                    nexus.GetEndNode(pHeader);
                    ts_nexus.GetStartNode(pSeg);
                    nexus.Merge(&ts_nexus);
                    break;
                } else if (pHeader->GetLastPoint()->IsInsideSphere(pSeg->GetLastPoint(), 250.0f)) {
                    PathNodeNexus nexus;
                    PathNodeNexus ts_nexus;
                    
                    nexus.GetEndNode(pHeader);
                    ts_nexus.GetEndNode(pSeg);
                    nexus.Merge(&ts_nexus);
                    break;
                }
            }
            
            pSeg = (PathSegment*)((char*)pSeg + sizeof(*pSeg) + pSeg->numPoints * sizeof(Vector));
        }
        

        pHeader = (PathSegment*)((char*)pHeader + sizeof(*pHeader) + pHeader->numPoints * sizeof(Vector));
    }
}

void PathNodeNexus::Merge(PathNodeNexus* pOtherNexus) {

    PathNodeNexus thisTarget;
    PathNodeNexus thatTarget;
    
    thisTarget = *this;

    thisTarget.GetNextNode();

    while (thisTarget.pSegment != pSegment) {
        if (thisTarget.pSegment == pOtherNexus->pSegment) {
            return;
        }
        
        thisTarget.GetNextNode();
    }

    thisTarget.GetNextNode();
    
    thatTarget = *pOtherNexus;

    thatTarget.GetNextNode();
    
    if (unk4) {
        pSegment->unkC = thatTarget.unk4;
        pSegment->unk4 = thatTarget.pSegment;
    } else {
        pSegment->unkD = thatTarget.unk4;
        pSegment->unk8 = thatTarget.pSegment;
    }

    if (pOtherNexus->unk4) {
        pOtherNexus->pSegment->unkC = thisTarget.unk4;
        pOtherNexus->pSegment->unk4 = thisTarget.pSegment;
    } else {
        pOtherNexus->pSegment->unkD = thisTarget.unk4;
        pOtherNexus->pSegment->unk8 = thisTarget.pSegment;
    }
}

void Path_Init(void) {
    if (!bPathLoaded) {
        bPathLoaded = true;

        pSegmentData = (PathSegment*)Heap_MemAlloc(
            (pathCount * 0x100) + ((pathCount + 1) * sizeof(PathSegment))
        );
        
        pTerminator = pSegmentData;

        pSegmentData->Init();
    }
}

void Path_Load(PathLoadInfo* pLoadInfo) {
    pTerminator->pathId = pLoadInfo->type;
    pTerminator->numPoints = pLoadInfo->numPoints;
    
    memcpy((void*)pTerminator->points, (void*)pLoadInfo->points, pTerminator->numPoints * sizeof(Vector));

    pTerminator = (PathSegment*)((char*)pTerminator + sizeof(*pTerminator) + pTerminator->numPoints * sizeof(Vector));

    pTerminator->Init();
}

void Path_Manager_Init(void) {
    if (bPathLoaded && !bPathManagerLoaded) {
        bPathManagerLoaded = true;

        ResolveConnections(pSegmentData);
    } 
}

void Path_Manager_Deinit(void) {
    if (bPathLoaded) {
        bPathLoaded = false;

        Heap_MemFree(pSegmentData);
    }

    bPathManagerLoaded = false;
}

void* Path_GetTerminatorSegment(void) {
    return bPathLoaded ? pTerminator : NULL;
}

void* Path_GetFirstSegment(void) {
    return bPathLoaded ? pSegmentData : NULL;
}

void PathManager::Init(s8 r4) {
    memset((void*)this, 0, sizeof(*this));

    unk8 = r4;
    pathId = r4;

    if (bPathManagerLoaded) {
        PathSegment* pSeg = pSegmentData;

        while (true) {
            if (pSeg->pathId == pathId) {
                pCurrentSegment = pSeg;
                return;
            }

            pSeg = (PathSegment*)((char*)pSeg + sizeof(*pSeg) + pSeg->numPoints * sizeof(Vector));
        }
    } else {
        pCurrentSegment = NULL;
    }
}

void PathManager::Init(s8 r4, Vector* r5, float f1) {
    Init(r4);

    GetNearestNode(r5, f1);
}

void PathManager::SetCurrentPath(PathSegment* pSeg, int r5) {
    pCurrentSegment = pSeg;

    unk4 = r5 < pCurrentSegment->numPoints ? r5 : pCurrentSegment->numPoints - 1;
}

Vector* PathManager::GetNearestPoint(Vector* r4, float closestDist) {
    PathSegment* pClosestPath = NULL;

    int r8 = -1;

    PathSegment* pSeg = pSegmentData;

    while (pSeg->numPoints != 0) {
        if (pSeg->pathId == pathId) {
            for (int i = 0; i < pSeg->numPoints; i++) {
                float distSq = pSeg->points[i].DistSq(r4);
                if (distSq < closestDist) {
                    closestDist = distSq;
                    pClosestPath = pSeg;
                    r8 = i;
                }
            }
        }
        
        pSeg = (PathSegment*)((char*)pSeg + sizeof(*pSeg) + pSeg->numPoints * sizeof(Vector));
    }

    if (pClosestPath == NULL) {
        return NULL;
    }

    SetCurrentPath(pClosestPath, r8);
    return GetPoint();
}

Vector* PathManager::GetNearestNode(Vector* r4, float closestDist) {

    PathSegment* pClosestPath = NULL;
    u8 r6 = true;
    
    PathSegment* pSeg = pSegmentData;

    while (pSeg->numPoints != 0) {
        if (pSeg->pathId == pathId) {
            float distSq = pSeg->GetFirstPoint()->DistSq(r4);
            if (distSq < closestDist) {
                pClosestPath = pSeg;
                closestDist = distSq;
                r6 = true;
            }

            if (unkE & 1) {
                float distSq = pSeg->GetLastPoint()->DistSq(r4);
                if (distSq < closestDist) {
                    pClosestPath = pSeg;
                    closestDist = distSq;
                    r6 = false;
                }
            }
        }
        
        pSeg = (PathSegment*)((char*)pSeg + sizeof(*pSeg) + pSeg->numPoints * sizeof(Vector));
    }

    if (pClosestPath == NULL) {
        return NULL;
    }

    PathSegment* pOption[16];
    u8 pOptionIsStart[16];

    PathNodeNexus nexus;
    nexus.pSegment = pClosestPath;
    nexus.unk4 = r6;
    
    int options = 0;

    do {
        nexus.GetNextNode();

        if ((unkE & 1) != 0 || nexus.unk4) {
            pOption[options] = nexus.pSegment;
            pOptionIsStart[options] = nexus.unk4;
            options++;
        }
    } while (nexus.pSegment != pClosestPath);

    int rand = 0;
    if (options > 1) {
        rand = RandomIR(&gb.mRandSeed, 0, options);
    }

    PathSegment* pSeg2 = pOption[rand];
    u8 bIsStart = pOptionIsStart[rand];

    if ((unkE & 1) && !bIsStart) {
        unkD = 1;
    } else {
        unkD = 0;
    }

    SetCurrentPath(pSeg2, bIsStart ? 0 : 16);

    return GetPoint();
}

int PathManager::NextPoint(bool r4) {
    int ret;

    if (unkD) {
        unkD = false;
        ret = PrevPoint(r4);

        unkD = !unkD;
    } else {
        unk4++;

        if (unk4 < pCurrentSegment->numPoints - 1) {
            return 0;
        } else if (unk4 == pCurrentSegment->numPoints - 1) {
            PathSegment* pFoundSeg = NULL;
            PathNodeNexus nexus;

            nexus.GetEndNode(pCurrentSegment);

            while (true) {
                nexus.GetNextNode();

                if (nexus.pSegment == pCurrentSegment) {
                    if (pFoundSeg) {
                        return 1;
                    } else {
                        return 3;
                    }
                }

                if ((unkE & 1) || nexus.unk4) {
                    if (pFoundSeg) {
                        return 5;
                    }
                    pFoundSeg = nexus.pSegment;
                }
            }
        } else {
            PathSegment* pOption[16];
            u8 pOptionIsStart[16];

            int options = 0;
            
            PathSegment* pFoundSeg = NULL;
            PathNodeNexus nexus;

            nexus.GetEndNode(pCurrentSegment);
            
            while (true) {
                nexus.GetNextNode();

                if (nexus.pSegment == pCurrentSegment) {
                    break;
                }

                if ((unkE & 1) || nexus.unk4) {
                    pOption[options] = nexus.pSegment;
                    pOptionIsStart[options] = nexus.unk4;

                    options++;
                    
                    pFoundSeg = nexus.pSegment;
                }
            }

            if (options == 0) {
                if (unkE & 2) {
                    unkD = 1;
                    return NextPoint(r4);
                } else {
                    unk4 = pCurrentSegment->numPoints - 1;
                    return 3;
                }
            } else {
                int rand = 0;
                if (options > 1) {
                    rand = RandomIR(&gb.mRandSeed, 0, options);
                }

                pCurrentSegment = pOption[rand];
                if (pOptionIsStart[rand] != 0) {
                    unk4 = 0;
                } else {
                    unk4 = pCurrentSegment->numPoints - 1;
                    unkD = true;
                }

                if (r4) {
                    return NextPoint(r4);
                } else {
                    return 2;
                }
            }
        }
    }

    return ret;
}

int PathManager::PrevPoint(bool r4) {
    int ret;

    if (unkD) {
        unkD = false;
        ret = NextPoint(r4);

        unkD = !unkD;
    } else {
        unk4--;

        if (unk4 > 0) {
            return 0;
        } else if (unk4 == 0) {
            PathSegment* pFoundSeg = NULL;
            PathNodeNexus nexus;

            nexus.GetStartNode(pCurrentSegment);

            while (true) {
                nexus.GetNextNode();

                if (nexus.pSegment == pCurrentSegment) {
                    if (pFoundSeg) {
                        return 2;
                    } else {
                        return 4;
                    }
                }

                if ((unkE & 1) || !nexus.unk4) {
                    if (pFoundSeg) {
                        return 5;
                    }
                    pFoundSeg = nexus.pSegment;
                }
            }
        } else {
            PathSegment* pOption[16];
            u8 pOptionIsStart[16];

            int options = 0;
            
            PathSegment* pFoundSeg = NULL;
            PathNodeNexus nexus;

            nexus.GetStartNode(pCurrentSegment);
            
            while (true) {
                nexus.GetNextNode();

                if (nexus.pSegment == pCurrentSegment) {
                    break;
                }

                if ((unkE & 1) || !nexus.unk4) {
                    pOption[options] = nexus.pSegment;
                    pOptionIsStart[options] = nexus.unk4;

                    options++;
                    
                    pFoundSeg = nexus.pSegment;
                }
            }

            if (options == 0) {
                if (unkE & 2) {
                    unkD = 1;
                    return PrevPoint(r4);
                } else {
                    unk4 = 0;
                    return 4;
                }
            } else {
                int rand = 0;
                if (options > 1) {
                    rand = RandomIR(&gb.mRandSeed, 0, options);
                }

                pCurrentSegment = pOption[rand];
                if (pOptionIsStart[rand] != 0) {
                    unk4 = 0;
                    unkD = true;
                } else {
                    unk4 = pCurrentSegment->numPoints - 1;
                }

                if (r4) {
                    return PrevPoint(r4);
                } else {
                    return 1;
                }
            }
        }
    }

    return ret;
}

bool Path_Exists(s8 id) {
    PathSegment* pSeg = (PathSegment*)pSegmentData;

    while (pSeg->numPoints != 0) {
        if (pSeg->pathId == id) {
            return true;
        }

        pSeg = (PathSegment*)((char*)pSeg + sizeof(*pSeg) + pSeg->numPoints * sizeof(Vector));
    }

    return false;
}

// const pointer may be fake (need it to match the loads)
Vector* PathManager::GetNextNearestNode(Vector* const r4, PathSegment* r5, float closestDist) {
    
    float f9 = r5->GetFirstPoint()->DistSq(r4) + 1.0f;
    PathSegment* pClosestPath = NULL;
    u8 r6 = true;
    
    PathSegment* pSeg = pSegmentData;

    while (pSeg->numPoints != 0) {
        if (pSeg->pathId == pathId) {
            float distSq = pSeg->GetFirstPoint()->DistSq(r4);
            if (distSq < closestDist && distSq > f9) {
                pClosestPath = pSeg;
                closestDist = distSq;
                r6 = true;
            }

            if (unkE & 1) {
                float distSq = pSeg->GetLastPoint()->DistSq(r4);
                if (distSq < closestDist) {
                    pClosestPath = pSeg;
                    closestDist = distSq;
                    r6 = false;
                }
            }
        }
        
        pSeg = (PathSegment*)((char*)pSeg + sizeof(*pSeg) + pSeg->numPoints * sizeof(Vector));
    }

    if (pClosestPath == NULL) {
        return NULL;
    }

    PathSegment* pOption[16];
    u8 pOptionIsStart[16];

    PathNodeNexus nexus;
    nexus.pSegment = pClosestPath;
    nexus.unk4 = r6;
    
    int options = 0;

    do {
        nexus.GetNextNode();

        if ((unkE & 1) != 0 || nexus.unk4) {
            pOption[options] = nexus.pSegment;
            pOptionIsStart[options] = nexus.unk4;
            options++;
        }
    } while (nexus.pSegment != pClosestPath);

    int rand = 0;
    if (options > 1) {
        rand = RandomIR(&gb.mRandSeed, 0, options);
    }

    PathSegment* pSeg2 = pOption[rand];
    u8 bIsStart = pOptionIsStart[rand];

    if ((unkE & 1) && !bIsStart) {
        unkD = 1;
    } else {
        unkD = 0;
    }

    SetCurrentPath(pSeg2, bIsStart ? 0 : 16);

    return GetPoint();
}
