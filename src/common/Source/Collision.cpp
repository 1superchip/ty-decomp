#include "common/Collision.h"

CollisionResult lastCollision;

static bool bCollisionInit = false;
static bool bFound = false;
int collisionHeapSize = 0;
void* pCollisionHeap = NULL;
int collisionHeapIndex = 0;
static int collisionPolysAdded = 0;

static PtrListDL<DynamicItem> dynamicModels;

bool Collision_BInteriorPoint = false;
static Item** Collision_Grid = NULL;

static int Collision_MinX = 0;
static int Collision_MaxX = 0;
static int Collision_MinZ = 0;
static int Collision_MaxZ = 0;
static int Collision_TilesAcross = 0;
static int Collision_TilesDown = 0;
static int Collision_TileWidth = 0;
static int Collision_TileHeight = 0;
static float Collision_DynGridTileSizeX = 0;
static float Collision_DynGridTileSizeZ = 0;

#define NUM_DYNGRID_ENTRIES (1024)

static CollisionNode dynGrid[NUM_DYNGRID_ENTRIES];

#define EPSILON 1e-05f

void* CollisionHeap_Update(int size, int alignment) {
    if (alignment == 0) {
        // 32 bit alignment by default
        alignment = 4;
    }

    collisionHeapIndex += (alignment - 1);
    collisionHeapIndex &= ~(alignment - 1);
    void* pEntry = (u8*)pCollisionHeap + collisionHeapIndex;
    collisionHeapIndex += size;
    return pEntry;
}

void Collision_FindSphereDynamicGrid(Vector* pStartVector, Vector* pEndVector, 
    int* pStartX, int* pStartZ, int* pLengthX, int* pLengthZ) {

    int startX;
    int startZ;
    int endX;
    int endZ;
    
    startX = (pStartVector->x - Collision_MinX) / Collision_DynGridTileSizeX;
    startZ = (pStartVector->z - Collision_MinZ) / Collision_DynGridTileSizeZ;
    endX = (pEndVector->x - Collision_MinX) / Collision_DynGridTileSizeX;
    endZ = (pEndVector->z - Collision_MinZ) / Collision_DynGridTileSizeZ;

    *pStartX = Clamp<int>(0, startX, 31);
    *pStartZ = Clamp<int>(0, startZ, 31);
    *pLengthX = Clamp<int>(0, endX, 31) - *pStartX + 1;
    *pLengthZ = Clamp<int>(0, endZ, 31) - *pStartZ + 1;
}

int CalcNumDynamicLinks(float vecMag) {
    int div = vecMag / Min<float>(Collision_DynGridTileSizeX, Collision_DynGridTileSizeZ);
    return Sqr<int>(div + 2);
}

/// @brief Checks if the ray formed by pEnd and pStart lies on the front side of the plane formed by pVert0 and pVert1
/// @param pEnd End of the ray
/// @param pStart Start of the ray
/// @param pVert0 First vertex
/// @param pVert1 Second vertex
/// @return True if the ray lies on the front side of the plane (or on the plane), false otherwise
bool IsPointOnFrontSide(Vector* pEnd, Vector* pStart, Vector* pVert0, Vector* pVert1) {
    Vector v1;
    Vector v2;
    Vector v3;
    Vector v4;

    // Vector_Cross

    // Edge 0
    v1.x = pVert0->x - pStart->x;
    v1.y = pVert0->y - pStart->y;
    v1.z = pVert0->z - pStart->z;

    // Edge 1
    v2.x = pVert1->x - pStart->x;
    v2.y = pVert1->y - pStart->y;
    v2.z = pVert1->z - pStart->z;

    // Compute normal
    v3.x = v1.y * v2.z - v1.z * v2.y;
    v3.y = v1.z * v2.x - v1.x * v2.z;
    v3.z = v1.x * v2.y - v1.y * v2.x;

    // direction
    v4.x = pEnd->x - pStart->x;
    v4.y = pEnd->y - pStart->y;
    v4.z = pEnd->z - pStart->z;

    return v3.x * v4.x + v3.y * v4.y + v3.z * v4.z >= 0.0f;
}

static void CalcAAB(BoundingVolume* pVolume, Matrix* pMatrix, Vector* pMin, Vector* pMax) {
    Vector vecs[8];
    Vector* pLocalVec = &vecs[1];
    
    vecs[0].x = pVolume->v1.x;
    vecs[0].y = pVolume->v1.y;
    vecs[0].z = pVolume->v1.z;
    vecs[0].w = pVolume->v1.w;
    
    vecs[1].x = pVolume->v1.x;
    vecs[1].y = pVolume->v1.y;
    vecs[1].z = pVolume->v1.z;
    vecs[1].w = pVolume->v1.w;

    vecs[1].x = vecs[0].x + pVolume->v2.x;
    
    vecs[2].x = vecs[0].x;
    vecs[2].y = vecs[0].y;
    vecs[2].z = vecs[0].z;
    vecs[2].w = vecs[0].w;

    vecs[2].y = vecs[1].y + pVolume->v2.y;

    vecs[3].x = vecs[0].x;
    vecs[3].y = vecs[0].y;
    vecs[3].z = vecs[0].z;
    vecs[3].w = vecs[0].w;

    vecs[3].z = vecs[0].z + pVolume->v2.z;
    
    vecs[4].x = vecs[1].x;
    vecs[4].y = vecs[0].y;
    vecs[4].z = vecs[0].z;
    vecs[4].w = vecs[0].w;

    vecs[4].y = vecs[3].y + pVolume->v2.y;
    
    vecs[5].x = pVolume->v1.x + pVolume->v2.x;
    vecs[5].y = vecs[0].y;
    vecs[5].w = pVolume->v1.w;
    vecs[5].z = pVolume->v1.z;
    
    vecs[5].z = vecs[2].z + pVolume->v2.z;

    vecs[6].z = pVolume->v1.z;
    vecs[7].z = vecs[2].z;

    vecs[6].x = vecs[0].x;
    vecs[6].y = vecs[2].y;
    vecs[6].w = vecs[0].w;
    vecs[6].z = vecs[2].z + pVolume->v2.z;
    
    vecs[7].x = vecs[4].x;
    vecs[7].y = vecs[3].y + pVolume->v2.y;
    vecs[7].w = vecs[2].w;
    vecs[7].z = vecs[2].z + pVolume->v2.z;
    
    for (int i = 0; i < 8; i++) {
        vecs[i].ApplyMatrix(&vecs[i], pMatrix);
    }

    pMin->x = vecs[0].x;
    pMin->y = vecs[0].y;
    pMin->z = vecs[0].z;
    pMin->w = vecs[0].w;
    pMax->x = vecs[0].x;
    pMax->y = vecs[0].y;
    pMax->z = vecs[0].z;
    pMax->w = vecs[0].w;

    for (int i = 1; i < 8; i++) {
        pMin->x = Min<float>(pMin->x, vecs[i].x);
        pMax->x = Max<float>(pMax->x, vecs[i].x);
        pMin->y = Min<float>(pMin->y, vecs[i].y);
        pMax->y = Max<float>(pMax->y, vecs[i].y);
        pMin->z = Min<float>(pMin->z, vecs[i].z);
        pMax->z = Max<float>(pMax->z, vecs[i].z);
    }
}

void InterpolateVertexColor(Vector* pVec, Vector* pVec1, Vector* pos0, Vector* color0,
            Vector* pos1, Vector* color1, Vector* pos2, Vector* color2) {
    Vector side1;
    Vector side2;
    Vector cProduct;
    Vector toPoint;
    Vector cProduct2;

    side1.Sub(pos1, pos0);
    side2.Sub(pos2, pos0);

    cProduct.Cross(&side1, &side2);

    toPoint.Sub(pVec, pos0);
    cProduct2.Cross(&cProduct, &toPoint);

    float dot = cProduct2.Dot(&side1);
    float dot1 = cProduct2.Dot(&side2);

    float y = dot - dot1 ? dot / (dot - dot1) : 0.0f;
    y = Clamp<float>(0.0f, y, 1.0f);

    pVec1->InterpolateLinear(color1, color2, y);

    Vector midPoint;
    midPoint.InterpolateLinear(pos1, pos2, y);
    midPoint.Subtract(pos0);

    float t = midPoint.Dot(&toPoint);
    float mag = midPoint.MagSquared();

    float x = mag ? t / mag : 0.0f;
    x = Clamp<float>(0.0f, x, 1.0f);

    pVec1->InterpolateLinear(color0, pVec1, x);
}

void ConvertRGBA(Vector* pOut, u8* pColors) {
    pOut->Set(pColors[0] / 255.0f, pColors[1] / 255.0f, pColors[2] / 255.0f, 1.0f);
}

void GetVertexColorFromPoly(Vector* vec1, Vector* vec2, Item* pItem) {
    Vector color0;
    Vector color1;
    Vector color2;

    ConvertRGBA(&color0, pItem->collisionThing->verts[0].color);
    ConvertRGBA(&color1, pItem->collisionThing->verts[1].color);
    ConvertRGBA(&color2, pItem->collisionThing->verts[2].color);

    InterpolateVertexColor(
        vec2, vec1, 
        (Vector*)pItem->collisionThing->verts[0].pos, &color0, 
        (Vector*)pItem->collisionThing->verts[1].pos, &color1, 
        (Vector*)pItem->collisionThing->verts[2].pos, &color2
    );
}

// pA and pB are line endpoints
// pVec is the vector to project on line AB
// pProj is the projected point if the point is not projected on pA or pB
Vector* PROJECT_TO_LINE_SEGMENT(Vector *pVec, Vector *pA, Vector *pB, Vector* pProj)
{
    Vector diff, dispAB;
    diff.Sub(pVec, pA);
    dispAB.Sub(pB, pA);
    float distSqAB = Max<float>(dispAB.MagSquared(), EPSILON);
    float dot = diff.Dot(&dispAB);
    if (dot < 0.0f) return pA;
    if (dot > distSqAB) return pB;
    float div = dot / distSqAB;
    pProj->Set(div * dispAB.x, div * dispAB.y, div * dispAB.z);
    pProj->Add(pA);
    return pProj;
}

float CylTest_CapsFirst(Vector* pVec, Vector* pVec1, float param_3, float param_4, Vector* pVec2) {
    float x0 = pVec1->x - pVec->x;
    float y0 = pVec1->y - pVec->y;
    float z0 = pVec1->z - pVec->z;
    float x1 = pVec2->x - pVec->x;
    float y1 = pVec2->y - pVec->y;
    float z1 = pVec2->z - pVec->z;
    float local_3C = (x1 * x0) + (y1 * y0) + (z1 * z0);
    if (local_3C < 0.0f || local_3C > param_3) {
        return -1.0f;
    }
    float t = (x1 * x1) + (y1 * y1) + (z1 * z1) - (local_3C * local_3C) / (param_3);
    if (t > param_4) {
        return -1.0f;
    }
    return t;
}

void CollisionCpp_OrderFloats(void) {
    volatile float x = -1e+12f;
}

float GetDiv(Vector* pos, Vector* rayPos, Vector* nDXYZ, float radius) {
    Vector diff;
    diff.Sub(rayPos, pos);
    float nDot = diff.Dot(nDXYZ);
    float h = Sqr<float>(radius) - (diff.MagSquared() - (nDot * nDot));
    if (h < 0.0f) {
        return -1.0f;
    }
    return nDot - sqrtf(h);
}

Vector* NEAREST_POINT(Vector *pVec, Vector *pA, Vector *pB) {
    Vector tmp;
    Vector tmp2;
    tmp.Sub(pVec, pA);
    tmp2.Sub(pVec, pB);
    float distSqA = tmp.MagSquared();
    float distSqB = tmp2.MagSquared();

    return (distSqA < distSqB) ? pA : pB;
}

static void NearestPointOnTriEdge(Vector* pVec, Vector* pA, Vector* pB, Vector* pC) {
    Vector onAB;
    Vector onBC;
    Vector onCA;
    Vector* pNearestOnAB = PROJECT_TO_LINE_SEGMENT(pVec, pA, pB, &onAB);
    Vector* pNearestOnBC = PROJECT_TO_LINE_SEGMENT(pVec, pB, pC, &onBC);
    Vector* pNearestOnCA = PROJECT_TO_LINE_SEGMENT(pVec, pC, pA, &onCA);
    Vector* pNearest = NEAREST_POINT(pVec, pNearestOnAB, NEAREST_POINT(pVec, pNearestOnBC, pNearestOnCA));
    
    pVec->Copy(pNearest);
}

static void NearestPointOnQuadEdge(Vector* pVec, Vector* pA, Vector* pB, Vector* pC, Vector* pD) {
    Vector onAB;
    Vector onBC;
    Vector onCD;
    Vector onDA;
    Vector* pNearestOnAB = PROJECT_TO_LINE_SEGMENT(pVec, pA, pB, &onAB);
    Vector* pNearestOnBC = PROJECT_TO_LINE_SEGMENT(pVec, pB, pC, &onBC);
    Vector* pNearestOnCD = PROJECT_TO_LINE_SEGMENT(pVec, pC, pD, &onCD);
    Vector* pNearestOnDA = PROJECT_TO_LINE_SEGMENT(pVec, pD, pA, &onDA);

    Vector* pNearest = NEAREST_POINT(pVec, pNearestOnAB, NEAREST_POINT(pVec, pNearestOnBC, 
            NEAREST_POINT(pVec, pNearestOnCD, pNearestOnDA)));
    
    pVec->Copy(pNearest);
}

static void NearestPointOnPolyEdge(Vector* pVec, Vector* pVec1, int* indices, int numVerts) {
    if (numVerts == 3) {
        NearestPointOnTriEdge(pVec, &pVec1[indices[0]], &pVec1[indices[1]], &pVec1[indices[2]]);
    } else if (numVerts == 4) {
        NearestPointOnQuadEdge(pVec, &pVec1[indices[0]], &pVec1[indices[1]], &pVec1[indices[2]], &pVec1[indices[3]]);
    }
}

// checks if the ray formed by pVec and pVec1 intersects the polygon with nmbrOfVertices vertices of pVec2
static bool PointInPoly(Vector* pVec, Vector* pVec1, Vector* pVec2, int* indices, int nmbrOfVertices) {
    for (int i = 0; i < nmbrOfVertices - 1; i++) {
        if (IsPointOnFrontSide(pVec, pVec1, &pVec2[indices[i]], &pVec2[indices[i + 1]])) {
            return false;
        }
    }
    
    return !IsPointOnFrontSide(pVec, pVec1, &pVec2[indices[nmbrOfVertices - 1]], &pVec2[indices[0]]);
}

float SubDot(Vector* vec, Vector* vec1, Vector* vec2) {
    Vector tmp;
    tmp.Sub(vec, vec1);
    return tmp.Dot(vec2);
}

static bool SweepSphereToPoly(SphereRay* pRay, Vector* pVectors, int* indices, int numVerts, Vector* arg5, Vector* arg6, CollisionResult* pResult) {
    Vector hitPoint = pRay->mStart;
    Vector hitNormal = *arg5;

    float min;
    if (arg6->Dot(&pRay->mDir) < 0.0f) {
        return false;
    }
    
    float d = SubDot(&pRay->mStart, &pVectors[indices[0]], arg5);
    if (d > pRay->mLength + pRay->radius) {
        return false;
    }

    if (d <= pRay->radius) {
        hitPoint.Scale(arg6, d);
        min = 0.0f;
        hitPoint.Add(&pRay->mStart);
    } else {
        Vector sP;
        sP.Scale(arg6, pRay->radius);
        sP.Add(&sP, &pRay->mStart);
        
        float t;
        if (arg5->Dot(&pRay->mDir) == 0.0f) {
            t = -1e+12f;
        } else {
            t = -(arg5->x * (sP.x - pVectors[indices[0]].x) + 
                arg5->y * (sP.y - pVectors[indices[0]].y) + 
                arg5->z * (sP.z - pVectors[indices[0]].z)) / arg5->Dot(&pRay->mDir);
        }

        min = t;
        if (t <= 0.0f || t > pRay->mLength) {
            return false;
        }

        hitPoint.Scale(&pRay->mDir, min);
        hitPoint.Add(&sP);
    }

    if (!PointInPoly(&hitPoint, &pRay->mStart, pVectors, indices, numVerts)) {
        // point isn't in the polygon
        NearestPointOnPolyEdge(&hitPoint, pVectors, indices, numVerts);
        // pos is now the coordinate on the polygon's edge 
        if (hitPoint.IsInsideSphere(&pRay->mStart, pRay->radius)) {
            // if the position on the polygon's edge is in the SphereRay's sphere
            Collision_BInteriorPoint = true;
            min = 0.0f;
            hitNormal.Sub(&pRay->mStart, &hitPoint);
            if (hitNormal.Dot(&pRay->mDir) > 0.0f) {
                return false;
            }
        } else {
            // position is in polygon
            min = GetDiv(&hitPoint, &pRay->mStart, &pRay->negDXYZ, pRay->radius);
            if (min <= 0.0f || min > pRay->mLength) {
                return false;
            }

            Vector tmp;
            tmp.Scale(&pRay->negDXYZ, min);
            tmp.Add(&tmp, &hitPoint);
            hitNormal.Sub(&pRay->mStart, &tmp);
        }
    } else {
        Collision_BInteriorPoint = 0.0f == min;
    }

    pResult->normal = hitNormal;
    pResult->pos = hitPoint;
    pResult->unk40 = min;

    return true;
}

static bool SweepSphereToTri(SphereRay* pRay, Vector* pVert0, Vector* pVert1, 
            Vector* pVert2, Vector* pVec3, Vector* pVec4, CollisionResult* pResult) {
    Vector hitPoint = pRay->mStart;
    Vector hitNormal = *pVec3;
    float min;

    if (pVec4->Dot(&pRay->mDir) < 0.0f) {
        return false;
    }

    float d = SubDot(&pRay->mStart, pVert0, pVec3);
    if (d > pRay->mLength + pRay->radius) {
        return false;
    }

    if (d <= pRay->radius) {
        hitPoint.Scale(pVec4, d);
        min = 0.0f;
        hitPoint.Add(&pRay->mStart);
    } else {
        Vector tmp;
        tmp.Scale(pVec4, pRay->radius);
        tmp.Add(&tmp, &pRay->mStart);
        float t;
        if (pVec3->Dot(&pRay->mDir) == 0.0f) {
            t = -1e+12f;
        } else {
            t = -(pVec3->x * (tmp.x - pVert0->x) + 
                pVec3->y * (tmp.y - pVert0->y) + 
                pVec3->z * (tmp.z - pVert0->z)) / pVec3->Dot(&pRay->mDir);
        }
        min = t;
        if (t <= 0.0f || t > pRay->mLength) {
            return false;
        }
        hitPoint.Scale(&pRay->mDir, min);
        hitPoint.Add(&tmp);
    }

    if (!IsPointOnFrontSide(&hitPoint, &pRay->mStart, pVert1, pVert0) || 
        !IsPointOnFrontSide(&hitPoint, &pRay->mStart, pVert2, pVert1) || 
        !IsPointOnFrontSide(&hitPoint, &pRay->mStart, pVert0, pVert2)) {
        // point isn't in the triangle
        NearestPointOnTriEdge(&hitPoint, pVert0, pVert1, pVert2);
        // pos is now the coordinate on the polygon's edge 
        if (hitPoint.IsInsideSphere(&pRay->mStart, pRay->radius)) {
            // if the position on the polygon's edge is in the SphereRay's sphere
            Collision_BInteriorPoint = true;
            min = 0.0f;
            hitNormal.Sub(&pRay->mStart, &hitPoint);
            if (hitNormal.Dot(&pRay->mDir) > 0.0f) {
                return false;
            }
        } else {
            // position is in polygon
            min = GetDiv(&hitPoint, &pRay->mStart, &pRay->negDXYZ, pRay->radius);
            if (min <= 0.0f || min > pRay->mLength) {
                return false;
            }
            Vector tmp;
            tmp.Scale(&pRay->negDXYZ, min);
            tmp.Add(&tmp, &hitPoint);
            hitNormal.x = pRay->mStart.x - tmp.x;
            hitNormal.y = pRay->mStart.y - tmp.y;
            hitNormal.z = pRay->mStart.z - tmp.z;
        }
    } else {
        Collision_BInteriorPoint = 0.0f == min;
    }

    pResult->normal = hitNormal;
    pResult->pos = hitPoint;
    pResult->unk40 = min;

    return true;
}

void FindGridBoundaries(Vector* pMin, Vector* pMax, int* minX, int* minZ, int* maxX, int* maxZ) {
    // Find the minimum and maximum tiles
    int startX = (Min<float>(pMin->x, pMax->x) - Collision_MinX) / Collision_TileWidth;
    int startZ = (Min<float>(pMin->z, pMax->z) - Collision_MinZ) / Collision_TileHeight;

    int endX = (Max<float>(pMin->x, pMax->x) - Collision_MinX) / Collision_TileWidth;
    int endZ = (Max<float>(pMin->z, pMax->z) - Collision_MinZ) / Collision_TileHeight;

    *minX = Max<int>(startX, 0);
    *minZ = Max<int>(startZ, 0);
    *maxX = Min<int>(endX, Collision_TilesAcross - 1);
    *maxZ = Min<int>(endZ, Collision_TilesDown - 1);
}

void FindGridBoundariesSphere(Vector* pPos, float radius, int* minX, int* minZ, int* maxX, int* maxZ) {
    int startX = ((pPos->x - radius) - Collision_MinX) / Collision_TileWidth;
    int startZ = ((pPos->z - radius) - Collision_MinZ) / Collision_TileHeight;
    int endX = ((pPos->x + radius) - Collision_MinX) / Collision_TileWidth;
    int endZ = ((pPos->z + radius) - Collision_MinZ) / Collision_TileHeight;
    *minX = Max<int>(startX, 0);
    *minZ = Max<int>(startZ, 0);
    *maxX = Min<int>(endX, Collision_TilesAcross - 1);
    *maxZ = Min<int>(endZ, Collision_TilesDown - 1);
}

void Collision_InitModule(void) {}

void Collision_DeinitModule(void) {}

static void StoreSphereResult(Item* pItem, Vector* pVec, CollisionResult* pResult, float arg4) {
    lastCollision.pos = *(Vector*)&pItem->collisionThing->verts[0].pos;

    lastCollision.normal = *(Vector*)&pItem->collisionThing->normal;
    if (arg4 < 0.0f) {
        lastCollision.normal.Scale(-1.0f);
    }

    GetVertexColorFromPoly(&lastCollision.color, &lastCollision.pos, pItem);
    lastCollision.collisionFlags = pItem->pTriangle->flags;
    lastCollision.itemIdx = pItem->pTriangle->subObjectIdx;
    lastCollision.pModel = NULL;
    lastCollision.pInfo = pItem->pTriangle->pCollisionInfo;

    if (pResult != NULL) {
        *pResult = lastCollision;
    }
}

// pCr is a pointer to an array of CollisionResults of length maxCollisions (maxCollisions - 1)?
// returns the number of collisions that occurs
int Collision_SphereCollide(Vector* pPos, float radius, CollisionResult* pCr, int flags, int maxCollisions) {
    int minx;
    int minz;
    int maxx;
    int maxz;

    int numCollisions = 0;
    
    FindGridBoundariesSphere(pPos, radius, &minx, &minz, &maxx, &maxz);

    Item* pItem;

    for (int j = minz; j <= maxz; j++) {
        for (int i = minx; i <= maxx; i++) {
            pItem = Collision_Grid[j * Collision_TilesAcross + i];

            while (pItem != NULL) {
                if ((pItem->pTriangle->pCollisionInfo == NULL || pItem->pTriangle->pCollisionInfo->bEnabled) &&
                    (pItem->pTriangle->flags & flags) == 0) {
                    // need radiusSq
                    Vector* triVert0 = (Vector*)&pItem->collisionThing->verts[0];
                    Vector* triVert1 = (Vector*)&pItem->collisionThing->verts[1];
                    Vector* triVert2 = (Vector*)&pItem->collisionThing->verts[2];
                    float fVar19 =
                        SubDot(pPos, (Vector*)&pItem->collisionThing->verts[0], (Vector*)&pItem->collisionThing->normal);
                    float fVar0 = Abs<float>(fVar19);
                    if (!(fVar0 > radius)) {
                        Vector v1;
                        Vector v2;
                        Vector v3;
                        v1.Sub(triVert0, pPos);
                        if (v1.MagSquared() <= radius * radius) {
                            numCollisions++;
                            StoreSphereResult(pItem, pPos, pCr, fVar19);
                            maxCollisions--;
                            if (maxCollisions <= 0) {
                                goto end;
                            }
                            if (pCr) {
                                pCr++;
                            }
                        } else {
                            v1.Sub(triVert1, pPos);
                            if (v1.MagSquared() <= radius * radius) {
                                numCollisions++;
                                StoreSphereResult(pItem, pPos, pCr, fVar19);
                                maxCollisions--;
                                if (maxCollisions <= 0) {
                                    goto end;
                                }
                                if (pCr) {
                                    pCr++;
                                }
                            } else {
                                v1.Sub(triVert2, pPos);
                                if (v1.MagSquared() <= radius * radius) {
                                    numCollisions++;
                                    StoreSphereResult(pItem, pPos, pCr, fVar19);
                                    maxCollisions--;
                                    if (maxCollisions <= 0) {
                                        goto end;
                                    }
                                    if (pCr) {
                                        pCr++;
                                    }
                                } else {
                                    v1.Sub(triVert1, triVert0);
                                    if (CylTest_CapsFirst(triVert0, triVert1, v1.MagSquared(), radius * radius,
                                                          pPos) != -1.0f) {
                                        numCollisions++;
                                        StoreSphereResult(pItem, pPos, pCr, fVar19);
                                        maxCollisions--;
                                        if (maxCollisions <= 0) {
                                            goto end;
                                        }
                                        if (pCr) {
                                            pCr++;
                                        }
                                    } else {
                                        v1.Sub(triVert2, triVert1);
                                        if (CylTest_CapsFirst(triVert1, triVert2, v1.MagSquared(), radius * radius,
                                                              pPos) != -1.0f) {
                                            numCollisions++;
                                            StoreSphereResult(pItem, pPos, pCr, fVar19);
                                            maxCollisions--;
                                            if (maxCollisions <= 0) {
                                                goto end;
                                            }
                                            if (pCr) {
                                                pCr++;
                                            }
                                        } else {
                                            v1.Sub(triVert0, triVert2);
                                            if (CylTest_CapsFirst(triVert2, triVert0, v1.MagSquared(),
                                                                  radius * radius, pPos) != -1.0f) {

                                                numCollisions++;
                                                StoreSphereResult(pItem, pPos, pCr, fVar19);
                                                maxCollisions--;
                                                if (maxCollisions <= 0) {
                                                    goto end;
                                                }
                                                if (pCr) {
                                                    pCr++;
                                                }
                                            } else {
                                                v1.Sub(triVert1, triVert0);
                                                v2.Cross(&v1, (Vector*)&pItem->collisionThing->normal);
                                                v3.Sub(pPos, triVert0);
                                                if (v3.Dot(&v2) >= 0.0f) {

                                                    v1.Sub(triVert2, triVert1);
                                                    v2.Cross(&v1, (Vector*)&pItem->collisionThing->normal);

                                                    v3.Sub(pPos, triVert1);

                                                    if (v3.Dot(&v2) >= 0.0f) {

                                                        v1.Sub(triVert0, triVert2);
                                                        v2.Cross(&v1, (Vector*)&pItem->collisionThing->normal);
                                                        v3.Sub(pPos, triVert2);

                                                        if (v3.Dot(&v2) >= 0.0f) {
                                                            numCollisions++;
                                                            StoreSphereResult(pItem, pPos, pCr, fVar19);
                                                            maxCollisions--;
                                                            if (maxCollisions <= 0) {
                                                                goto end;
                                                            }
                                                            if (pCr) {
                                                                pCr++;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                pItem = pItem->next;
            }
        }
    }
end:
    return numCollisions;
}

/// @brief Checks if pPoint is on or in front of the plane formed by pItem
/// @param pPoint Point to check
/// @param pItem Item that forms the plane
/// @return True if pPoint is on or in front of the plane formed by pItem, otherwise false
bool InFrontOfItem(Vector* pPoint, Item* pItem) {
    Vector v;
    v.x = pPoint->x - pItem->collisionThing->verts[0].pos[0];
    v.y = pPoint->y - pItem->collisionThing->verts[0].pos[1];
    v.z = pPoint->z - pItem->collisionThing->verts[0].pos[2];
    return v.x * pItem->collisionThing->normal[0] +
        v.y * pItem->collisionThing->normal[1] +
        v.z * pItem->collisionThing->normal[2] >= 0.0f;
}

bool CheckItemSphereRay(Item* pItem, Vector* pVec, Vector* pVec1) {
    CollisionThing* pThing = pItem->collisionThing;
    bool ret = false;
    if ((((pThing->verts[0].pos[1] < pVec->y && pThing->verts[1].pos[1] < pVec->y &&
        pItem->collisionThing->verts[2].pos[1] < pVec->y) || (pThing->verts[0].pos[1] > pVec1->y &&
        pItem->collisionThing->verts[1].pos[1] > pVec1->y && pThing->verts[2].pos[1] > pVec1->y)) ||
        ((pItem->collisionThing->verts[0].pos[0] < pVec->x && pThing->verts[1].pos[0] < pVec->x &&
        pItem->collisionThing->verts[2].pos[0] < pVec->x) || (pThing->verts[0].pos[0] > pVec1->x &&
        pItem->collisionThing->verts[1].pos[0] > pVec1->x && pThing->verts[2].pos[0] > pVec1->x)) ||
        ((pItem->collisionThing->verts[0].pos[2] < pVec->z && pThing->verts[1].pos[2] < pVec->z &&
        pItem->collisionThing->verts[2].pos[2] < pVec->z) || (pThing->verts[0].pos[2] > pVec1->z &&
        pItem->collisionThing->verts[1].pos[2] > pVec1->z && pThing->verts[2].pos[2] > pVec1->z)))) {
        ret = true;
    }
    return ret;
}

static bool Collision_PolyCollide(Vector* pStart, Vector* pEnd, Vector* pDir, CollisionResult* pCr, int flags) {
    int minx;
    int minz;
    int maxx;
    int maxz;
    FindGridBoundaries(pStart, pEnd, &minx, &minz, &maxx, &maxz);

    bFound = false;

    for (int j = minz; j <= maxz; j++) {
        for (int i = minx; i <= maxx; i++) {
            Item* pItem = Collision_Grid[j * Collision_TilesAcross + i];
            while (pItem != NULL) {
                if ((pItem->pTriangle->pCollisionInfo == NULL ||
                    pItem->pTriangle->pCollisionInfo->bEnabled) &&
                    (pItem->pTriangle->flags & flags) == 0) {

                    bool bEndInFront = InFrontOfItem(pEnd, pItem);
                    bool bStartInFront = InFrontOfItem(pStart, pItem);

                    if (bStartInFront != bEndInFront) {
                        // start and end positions are on opposite sides of the plane
                        // ray from start to end may intersect the poly

                        Vector* pVert0 = (Vector*)&pItem->collisionThing->verts[0].pos;
                        Vector* pVert1 = (Vector*)&pItem->collisionThing->verts[1].pos;
                        Vector* pVert2 = (Vector*)&pItem->collisionThing->verts[2].pos;
                        Vector surfaceNormal = *(Vector*)&pItem->collisionThing->normal;

                        if (bEndInFront) {
                            Swap<Vector*>(pVert1, pVert2);
                            surfaceNormal.Inverse();
                        }

                        if (IsPointOnFrontSide(pEnd, pStart, pVert1, pVert0) &&
                            IsPointOnFrontSide(pEnd, pStart, pVert2, pVert1) &&
                            IsPointOnFrontSide(pEnd, pStart, pVert0, pVert2)) {
                            // ray is intersecting the polygon
                            float d = surfaceNormal.Dot(pDir);
                            if (d < 0.0f) {
                                Vector a;
                                a.Sub(pStart, pVert0);
                                // finalPos is the intersection point of the ray and polygon
                                // finalPos may be called "a"?
                                Vector newRayNormal;
                                float scale = -surfaceNormal.Dot(&a) / d;
                                newRayNormal.Scale(pDir, scale);
                                newRayNormal.Add(pStart);

                                // set fields of the collision result
                                lastCollision.pos = newRayNormal;
                                lastCollision.normal = surfaceNormal;
                                GetVertexColorFromPoly(&lastCollision.color, &lastCollision.pos, pItem);
                                lastCollision.collisionFlags = pItem->pTriangle->flags;
                                lastCollision.itemIdx = pItem->pTriangle->subObjectIdx;
                                lastCollision.pModel = NULL;
                                lastCollision.pInfo = pItem->pTriangle->pCollisionInfo;
                                lastCollision.unk40 = scale;

                                if (pCr) {
                                    // if a CollisionResult was passed, copy the result to it
                                    *pCr = lastCollision;
                                }
                                
                                bFound = true;
                                pEnd = &lastCollision.pos; // set the new end position to the position of the collision
                            }
                        }
                    }
                }
                pItem = pItem->next;
            }
        }
    }
    return bFound;
}

static void Collision_PolySweepSphereCollide(SphereRay* pRay, CollisionResult* pCr, int flags) {
    int minx;
    int minz;
    int maxx;
    int maxz;
    FindGridBoundaries(&pRay->mMinPos, &pRay->mMaxPos, &minx, &minz, &maxx, &maxz);

    for (int j = minz; j <= maxz; j++) {
        for (int i = minx; i <= maxx; i++) {
            float local_60 = i * (float)Collision_TileWidth + Collision_MinX;
            float local_64 = j * (float)Collision_TileHeight + Collision_MinZ;
            Item* pItem = Collision_Grid[j * Collision_TilesAcross + i];
            
            while (pItem != NULL) {
                // check x/z bounds 
                if (((i <= minx) || (!(pItem->collisionThing->verts[0].pos[0] < local_60) &&
                    !(pItem->collisionThing->verts[1].pos[0] < local_60) &&
                    !(pItem->collisionThing->verts[2].pos[0] < local_60))) && 
                    ((j <= minz) || (!(pItem->collisionThing->verts[0].pos[2] < local_64) &&
                    !(pItem->collisionThing->verts[1].pos[2] < local_64) &&
                    !(pItem->collisionThing->verts[2].pos[2] < local_64)))) {
                        // check ray bounds (including y) and then the triangle
                        if (!CheckItemSphereRay(pItem, &pRay->mMinPos, &pRay->mMaxPos) && 
                            (pItem->pTriangle->pCollisionInfo == NULL ||
                            pItem->pTriangle->pCollisionInfo->bEnabled) &&
                            (pItem->pTriangle->flags & flags) == 0) {
                            
                            Vector* p1 = (Vector*)&pItem->collisionThing->verts[0].pos;
                            Vector* p3 = (Vector*)&pItem->collisionThing->verts[1].pos;
                            Vector* p2 = (Vector*)&pItem->collisionThing->verts[2].pos;
                            
                            Vector normal = *(Vector*)&pItem->collisionThing->normal;
                            Vector invNormal = *(Vector*)&pItem->collisionThing->normal;

                            invNormal.Inverse();

                            bool swept;
                            if (InFrontOfItem(&pRay->mStart, pItem)) {
                                swept = SweepSphereToTri(pRay, p1, p3, p2, &normal, &invNormal, &lastCollision);
                            } else {
                                swept = SweepSphereToTri(pRay, p1, p2, p3, &invNormal, &normal, &lastCollision);
                            }

                            if (swept) {
                                bFound = true;
                                GetVertexColorFromPoly(&lastCollision.color, &lastCollision.pos, pItem);
                                lastCollision.collisionFlags = pItem->pTriangle->flags;
                                lastCollision.itemIdx = pItem->pTriangle->subObjectIdx;
                                lastCollision.pModel = NULL;
                                lastCollision.pInfo = pItem->pTriangle->pCollisionInfo;

                                if (pCr) {
                                    *pCr = lastCollision;
                                }

                                pRay->mLength = lastCollision.unk40;
                                pRay->mEnd.Scale(&pRay->mDir, pRay->mLength);
                                pRay->mEnd.Add(&pRay->mStart);
                            }
                        }
                    }

                pItem = pItem->next;
            }
        }
    }
}

static void Collision_AddItem(Item* pItem) {
    collisionPolysAdded++;
    Vector* pVert0 = (Vector*)&pItem->collisionThing->verts[0].pos;
    Vector* pVert1 = (Vector*)&pItem->collisionThing->verts[1].pos;
    Vector* pVert2 = (Vector*)&pItem->collisionThing->verts[2].pos;
    // find minimum/maximum X/Z point of the triangle
    int startX = (Min<float>(pVert0->x, Min<float>(pVert1->x, pVert2->x)) - Collision_MinX) / Collision_TileWidth;
    int startZ = (Min<float>(pVert0->z, Min<float>(pVert1->z, pVert2->z)) - Collision_MinZ) / Collision_TileHeight;
    int endX = (Max<float>(pVert0->x, Max<float>(pVert1->x, pVert2->x)) - Collision_MinX) / Collision_TileWidth;
    int endZ = (Max<float>(pVert0->z, Max<float>(pVert1->z, pVert2->z)) - Collision_MinZ) / Collision_TileHeight;
    if (startX >= 0 && endX < Collision_TilesAcross &&
        startZ >= 0 && endZ < Collision_TilesDown) {
        for (int j = startZ; j <= endZ; j++) {
            for (int i = startX; i <= endX; i++) {
                int idx = j * Collision_TilesAcross + i;
                pItem->next = Collision_Grid[idx];
                Collision_Grid[idx] = (Item*)CollisionHeap_Update(sizeof(Item), 0);
                *Collision_Grid[idx] = *pItem;
                bFound = true;
            }
        }
    }
}

float CalcDynamicItemRadius(SphereRay* pRay, DynamicItem* pDynItem) {
    return (pRay->mLength + (2.0f * pRay->radius) + pDynItem->unk8);
}

void Collision_Init(int heapSize, float minX, float /* unused */ minY, float minZ, 
        float width, float height, int tilesAcross, int tilesDown) {
    bCollisionInit = true;
    collisionHeapIndex = 0;
    collisionHeapSize = (heapSize + (0x20 - 1)) & ~(0x20 - 1); // round up to 32 bytes
    pCollisionHeap = Heap_MemAlloc(heapSize);
    dynamicModels.Init(768, sizeof(DynamicItem));
    
    Collision_TilesDown = tilesDown;
    Collision_TilesAcross = tilesAcross;
    // this call might use the parameters rather than the local variables
    Collision_Grid = (Item**)CollisionHeap_Update(Collision_TilesDown * Collision_TilesAcross * sizeof(Item**), 0);
    
    for (int i = 0; i < tilesDown * tilesAcross; i++) {
        Collision_Grid[i] = NULL;
    }
    
    Collision_MinX = minX;
    Collision_MinZ = minZ;
    Collision_MaxX = minX + width + 1.0f;
    Collision_MaxZ = minZ + height + 1.0f;
    
    Collision_TileWidth = (Collision_MaxX - Collision_MinX) / Collision_TilesAcross + 1;
    Collision_TileHeight = (Collision_MaxZ - Collision_MinZ) / Collision_TilesDown + 1;
    
    Collision_DynGridTileSizeX = (width / 32.0f) + 1.0f;
    
    Collision_DynGridTileSizeZ = (height / 32.0f) + 1.0f;
    
    for (int i = 0; i < NUM_DYNGRID_ENTRIES; i++) {
        dynGrid[i].PTR_0x8 = NULL;
        dynGrid[i].pNext = &dynGrid[i];
        dynGrid[i].pPrev = &dynGrid[i];
    }
}

// this function proves miny is a parameter to void Collision_Init(int, float, float, float, float, float, int, int);
// May be placed elsewhere in the file
/* Unused Function */
void Collision_Init(int heapSize, Model* pModel, int arg2, int arg3) {
    Vector vb;
    Vector va;

    arg2 = Max<int>(64, arg2);
    arg3 = Max<int>(64, arg3);

    va = pModel->pTemplate->pModelData->volume.v1;

    vb.Add(&va, &pModel->pTemplate->pModelData->volume.v2);

    va.ApplyMatrix(&pModel->matrices[0]);
    vb.ApplyMatrix(&pModel->matrices[0]);

    float minx = Min<float>(va.x, vb.x);
    float miny = Min<float>(va.y, vb.y);
    float minz = Min<float>(va.z, vb.z);
    float maxx = Max<float>(va.x, vb.x);
    float maxz = Max<float>(va.z, vb.z);
    Collision_Init(heapSize, minx - 1.0f, miny - 1.0f, minz - 1.0f, (maxx - minx) + 2.0f, (maxz - minz) + 2.0f, arg2, arg3);
}

// Updates all DynamicItems in dynamicModels
void Collision_Update(void) {
    DynamicItem** pDynamicItems = dynamicModels.GetMem();
    while (*pDynamicItems != NULL) {
        (*pDynamicItems)->Update();
        pDynamicItems++;
    }
}

void Collision_AddStaticModel(Model* pModel, CollisionInfo* pInfo, int subobject) {
    static Item item;

    ModelExplorer_GC* pExplorer = pModel->Explore(NULL, NULL, NULL);
    
    if (!pExplorer) {
        return;
    }

    CollisionTriangle* pTri = NULL;
    while (true) {
        bool ret;
        if (subobject >= 0 && subobject != pExplorer->subObjectIdx) {
            ret = pModel->ExploreNextSubObject((ModelExplorer*)pExplorer);
        } else {
            /*
            if the previous triangle is NULL or has different flags than the current material
            or has a different subobject index
            get the next triangle in the CollisionHeap
            */
            if (pTri == NULL || pTri->flags != pExplorer->pMaterial->collisionFlags ||
                    pTri->subObjectIdx != pExplorer->subObjectIdx) {
                pTri = (CollisionTriangle*)CollisionHeap_Update(sizeof(CollisionTriangle), 0);
                pTri->flags = pExplorer->pMaterial->collisionFlags;
                pTri->subObjectIdx = pExplorer->subObjectIdx;
                pTri->pCollisionInfo = pInfo;
            }

            CollisionThing* pTriData = (CollisionThing*)CollisionHeap_Update(sizeof(CollisionThing), 0);
            // loop through all vertices of the triangle
            for (int i = 0; i < 3; i++) {
                ((Vector*)&pTriData->verts[i].pos)->ApplyMatrix(&pExplorer->vertices[i].pos, &pModel->matrices[0]);
                float r = (pExplorer->vertices[i].color.x * pModel->colour.x);
                float g = (pExplorer->vertices[i].color.y * pModel->colour.y);
                float b = (pExplorer->vertices[i].color.z * pModel->colour.z);
                pTriData->verts[i].color[0] = r * 255.0f;
                pTriData->verts[i].color[1] = g * 255.0f;
                pTriData->verts[i].color[2] = b * 255.0f;
                pTriData->verts[i].color[3] = 255;
            }

            item.pTriangle = pTri;
            item.collisionThing = pTriData;
            Vector baDiff;
            Vector caDiff;
            baDiff.Sub((Vector*)&pTriData->verts[1].pos, (Vector*)&pTriData->verts[0].pos);
            caDiff.Sub((Vector*)&pTriData->verts[2].pos, (Vector*)&pTriData->verts[0].pos);

            ((Vector*)&pTriData->normal)->Cross(&caDiff, &baDiff);

            ((Vector*)&pTriData->normal)->Normalise();

            if (((Vector*)&pTriData->normal)->MagSquared() > 0.0f) {
                bFound = false;
                Collision_AddItem(&item);
            }

            ret = pModel->ExploreNextFace((ModelExplorer*)pExplorer);
        }

        if (!ret) {
            break;
        }
    }

    pModel->ExploreClose((ModelExplorer*)pExplorer);
}

bool Collision_SweepSphereCollideDynamicModel(SphereRay* pRay, CollisionResult* pCr, DynamicItem* pDynItem) {
    bool ret = false;
    BoundingVolume* pVolume = pDynItem->pModel->GetBoundingVolume(pDynItem->idx);
    Vector v1;
    Vector v2;
    v1.Copy(&pVolume->v1);
    v2.Add(&pVolume->v1, &pVolume->v2);

    float dist = pDynItem->GetMatrix()->Row3()->DistSq(&pRay->mStart);
    if (dist > Sqr<float>(pRay->mLength + (2.0f * pRay->radius) + pDynItem->unk8)) {
        return false;
    }

    Vector corners[8] = {
        {v1.x, v1.y, v1.z},
        {v1.x, v1.y, v2.z}, 
        {v2.x, v1.y, v2.z}, 
        {v2.x, v1.y, v1.z}, 
        {v1.x, v2.y, v1.z}, 
        {v1.x, v2.y, v2.z}, 
        {v2.x, v2.y, v2.z}, 
        {v2.x, v2.y, v1.z}, 
    };

    for (int i = 0; i < 8; i++) {
        corners[i].ApplyMatrix(pDynItem->GetMatrix());
    }

    int quads[6][4] = {
        {4, 5, 6, 7}, 
        {3, 2, 1, 0},
        {6, 2, 3, 7},
        {4, 0, 1, 5},
        {5, 1, 2, 6},
        {7, 3, 0, 4}
    };

    Vector normals[6];
    normals[0].Sub(&corners[4], &corners[0]);
    normals[0].Normalise();
    
    normals[1].Inverse(&normals[0]);
    normals[2].Sub(&corners[3], &corners[0]);
    normals[2].Normalise();
    
    normals[3].Inverse(&normals[2]);
    normals[4].Sub(&corners[1], &corners[0]);
    normals[4].Normalise();

    normals[5].Inverse(&normals[4]);
    
    for (int i = 0; i < 3; i++) {
        int dotTest = normals[i * 2].Dot(&pRay->mDir) > 0.0f ? 1 : 0;

        if (IsPointOnFrontSide(&pRay->mStart, &corners[quads[(dotTest + 2*i)][0]], 
                &corners[quads[(dotTest + 2*i)][1]], &corners[quads[(dotTest + 2*i)][2]]) &&
                SweepSphereToPoly(pRay, corners, &quads[(dotTest + 2*i)][0], 4, &normals[(dotTest + 2*i)], &normals[(1 - dotTest) + 2*i], &lastCollision)
            ) {
            lastCollision.pInfo = pDynItem->pInfo;
            lastCollision.pModel = pDynItem->pModel;
            lastCollision.collisionFlags = pDynItem->pModel->GetSubObjectMaterial(0, 0)->collisionFlags;
            lastCollision.itemIdx = Max<int>(0, pDynItem->idx);
            lastCollision.color = pDynItem->pModel->colour;
            pRay->mLength = lastCollision.unk40;
            pRay->mEnd.Scale(&pRay->mDir, pRay->mLength);
            pRay->mEnd.Add(&pRay->mEnd, &pRay->mStart);
            ret = true;
        }
    }

    // if a collision was found and pCr isn't NULL, copy the result of the collision to pCr
    if (ret && pCr) {
        *pCr = lastCollision;
    }

    return ret;
}

bool Collision_RayCollideDynamicModel(Vector* vec1, Vector* vec2, CollisionResult* pCr, Model* pModel, int subobjectIdx) {
    Matrix* pMatrix;
    Matrix inverseLTW;
    Vector newStart;
    Vector localVec2;
    Vector diff;
    pMatrix = subobjectIdx < 0 ? &pModel->matrices[0] : 
            &pModel->pMatrices[pModel->GetSubObjectMatrixIndex(subobjectIdx)];

    inverseLTW.Inverse(pMatrix);
    newStart.ApplyMatrix(vec1, &inverseLTW);
    localVec2.ApplyMatrix(vec2, &inverseLTW);
    diff.Sub(&localVec2, &newStart);

    BoundingVolume* pVolume = pModel->GetBoundingVolume(subobjectIdx);
    if ((((newStart.x < pVolume->v1.x) && 
        (localVec2.x < pVolume->v1.x)) ||
        ((newStart.y < pVolume->v1.y) && 
        (localVec2.y < pVolume->v1.y)) ||
        ((newStart.z < pVolume->v1.z) && 
        (localVec2.z < pVolume->v1.z))) ||
        
        (((newStart.x >= pVolume->v1.x + pVolume->v2.x) && 
        (localVec2.x >= pVolume->v1.x + pVolume->v2.x)) ||
        ((newStart.y >= pVolume->v1.y + pVolume->v2.y) && 
        (localVec2.y >= pVolume->v1.y + pVolume->v2.y)) ||
        ((newStart.z >= pVolume->v1.z + pVolume->v2.z) && 
        localVec2.z >= pVolume->v1.z + pVolume->v2.z)))
        {
        return false;
    }

    if (newStart.x >= pVolume->v1.x &&
        newStart.x < pVolume->v1.x + pVolume->v2.x &&
        newStart.y >= pVolume->v1.y &&
        newStart.y < pVolume->v1.y + pVolume->v2.y &&
        newStart.z >= pVolume->v1.z &&
        newStart.z < pVolume->v1.z + pVolume->v2.z) {
        return false;
    }

    float div = 1.0f;
    if (diff.x) {
        float f8 = div / diff.x;
        for (int i = 0; i < 2; i++) {
            float volumeX = i == 0 ? pVolume->v1.x : pVolume->v1.x + pVolume->v2.x;
            float f9 = f8 * (volumeX - newStart.x);
            if (f9 >= 0.0f && f9 < div) {
                float py = newStart.y + (f9 * diff.y);
                float pz = newStart.z + (f9 * diff.z);
                // if y and z are in bounds
                if (py >= pVolume->v1.y && py < pVolume->v1.y + pVolume->v2.y &&
                    pz >= pVolume->v1.z && pz < pVolume->v1.z + pVolume->v2.z) {
                    
                    div = f9;
                    bFound = true;
                    lastCollision.pos.Set(volumeX, py, pz);
                    lastCollision.normal.Set(i == 0 ? -1.0f : 1.0f, 0.0f, 0.0f);

                    if (pCr) {
                        pCr->pos = lastCollision.pos;
                        pCr->normal = lastCollision.normal;
                    }
                }
            }
        }
    }

    if (diff.y) {
        float f8 = 1.0f / diff.y;
        for (int i = 0; i < 2; i++) {
            float volumeY = i == 0 ? pVolume->v1.y : pVolume->v1.y + pVolume->v2.y;
            float f9 = f8 * (volumeY - newStart.y);
            if (f9 >= 0.0f && f9 < div) {
                float px = newStart.x + (f9 * diff.x);
                float pz = newStart.z + (f9 * diff.z);
                // if x and z are in bounds
                if (px >= pVolume->v1.x && px < pVolume->v1.x + pVolume->v2.x &&
                    pz >= pVolume->v1.z && pz < pVolume->v1.z + pVolume->v2.z) {
                    
                    div = f9;
                    bFound = true;
                    lastCollision.pos.Set(px, volumeY, pz);
                    lastCollision.normal.Set(0.0f, i == 0 ? -1.0f : 1.0f, 0.0f);

                    if (pCr) {
                        pCr->pos = lastCollision.pos;
                        pCr->normal = lastCollision.normal;
                    }
                }
            }
        }
    }

    if (diff.z) {
        float f8 = 1.0f / diff.z;
        for (int i = 0; i < 2; i++) {
            float volumeZ = i == 0 ? pVolume->v1.z : pVolume->v1.z + pVolume->v2.z;
            float f9 = f8 * (volumeZ - newStart.z);
            if (f9 >= 0.0f && f9 < div) {
                float px = newStart.x + (f9 * diff.x);
                float py = newStart.y + (f9 * diff.y);
                // if x and y are in bounds
                if (px >= pVolume->v1.x && px < pVolume->v1.x + pVolume->v2.x &&
                    py >= pVolume->v1.y && py < pVolume->v1.y + pVolume->v2.y) {
                    
                    div = f9;
                    bFound = true;
                    lastCollision.pos.Set(px, py, volumeZ);
                    lastCollision.normal.Set(0.0f, 0.0f, i == 0 ? -1.0f : 1.0f);

                    if (pCr) {
                        pCr->pos = lastCollision.pos;
                        pCr->normal = lastCollision.normal;
                    }
                }
            }
        }
    }

    if (bFound) {
        lastCollision.pInfo = NULL;
        lastCollision.pModel = pModel;
        lastCollision.color = pModel->colour;
        lastCollision.collisionFlags = pModel->GetSubObjectMaterial(0, 0)->collisionFlags;
        lastCollision.pos.ApplyMatrix(&lastCollision.pos, pMatrix);
        lastCollision.normal.ApplyRotMatrix(&lastCollision.normal, pMatrix);
        lastCollision.itemIdx = Max<int>(0, subobjectIdx);
        if (pCr) {
            *pCr = lastCollision;
        }

        return true;
    }

    return false;
}

inline bool Collision_RayCollideDynamicItem(Vector* vec1, Vector* vec2, CollisionResult* pCr,
                DynamicItem* pItem, float radius) {
    Vector tmp;
    tmp.Sub(pItem->GetMatrix()->Row3(), vec1);
    if (tmp.MagSquared() > Sqr<float>(radius * 2.0f + pItem->unk8)) {
        return false;
    }
    
    bool collide = Collision_RayCollideDynamicModel(vec1, vec2, pCr, pItem->pModel, pItem->idx);
    if (collide) {
        lastCollision.pInfo = pItem->pInfo;
        if (pCr) {
            pCr->pInfo = pItem->pInfo;
        }
    }

    return collide;
}

bool Collision_RayCollide(Vector* pStart, Vector* pEnd, CollisionResult* pCr, CollisionMode mode, int flags) {
    bFound = false;
    
    // Check dynamic collisions if the mode includes dynamic collisions
    if (mode == COLLISION_MODE_ALL || mode == COLLISION_MODE_DYNAMIC) {
        // Dynamic_Collides()
        Vector dist;
        dist.Sub(pStart, pEnd);
        float mag = dist.Magnitude();

        Vector min = {
            Min<float>(pStart->x, pEnd->x),
            Min<float>(pStart->y, pEnd->y),
            Min<float>(pStart->z, pEnd->z),
            1.0f
        };

        Vector max = {
            Max<float>(pStart->x, pEnd->x),
            Max<float>(pStart->y, pEnd->y),
            Max<float>(pStart->z, pEnd->z),
            1.0f
        };

        int overlapX;
        int overlapZ;
        int overlapXSize;
        int overlapZSize;
        // might not be the correct function, but it is equivalent
        Collision_FindSphereDynamicGrid(&min, &max, &overlapX, &overlapZ, &overlapXSize, &overlapZSize);
        for (int i = overlapZ; i < overlapZ + overlapZSize; i++) {
            for (int j = overlapX; j < overlapX + overlapXSize; j++) {
                CollisionNode* currNode = dynGrid[i * 0x20 + j].pNext;
                while (currNode != &dynGrid[i * 0x20 + j]) {
                    DynamicItem* currItem = currNode->PTR_0x8;
                    // potentially fix CollisionNode fields
                    
                    if (currItem->pInfo == NULL || currItem->pInfo->bEnabled) {
                        bool collideItem = Collision_RayCollideDynamicItem(pStart, pEnd, pCr, currItem, mag);
                        if (!collideItem) {
                            collideItem = bFound;
                        }

                        bFound = collideItem;
                    }

                    currNode = currNode->pNext;
                }
            }
        }
    }

    // Check for collisions with polygons if a collision wasn't found and
    // the mode includes polygon checks
    if (!bFound && (mode == COLLISION_MODE_ALL || mode == COLLISION_MODE_POLY)) {
        // Stack_Collides()
        Vector rayNormal;
        rayNormal.Sub(pEnd, pStart);
        return Collision_PolyCollide(pStart, pEnd, &rayNormal, pCr, flags);
    }

    return bFound;
}

bool Collision_SweepSphereCollide(Vector* pStart, Vector* pEnd, float sphereRadius,
    CollisionResult* pCr, CollisionMode pMode, int flags) {
    SphereRay ray;

    bFound = false;

    ray.Create(pStart, pEnd, sphereRadius);

    if (pMode == COLLISION_MODE_ALL || pMode == COLLISION_MODE_DYNAMIC) {
        int startX;
        int startZ;
        int lengthX;
        int lengthZ;
        Collision_FindSphereDynamicGrid(&ray.mMinPos, &ray.mMaxPos, &startX, &startZ, &lengthX, &lengthZ);

        for (int i = startZ; i < startZ + lengthZ; i++) {
            for (int j = startX; j < startX + lengthX; j++) {
                CollisionNode* currNode = dynGrid[i * 0x20 + j].pNext;
                while (currNode != &dynGrid[i * 0x20 + j]) {
                    DynamicItem* currItem = currNode->PTR_0x8;
                    // potentially fix CollisionNode fields
                    if (currItem->pInfo == NULL || currItem->pInfo->bEnabled) {
                        bFound = Collision_SweepSphereCollideDynamicModel(&ray, pCr, currNode->PTR_0x8) || bFound;
                    }
                    
                    currNode = currNode->pNext; // next might be at 0x4?
                }
            }
        }
    }

    if (pMode == COLLISION_MODE_ALL || pMode == COLLISION_MODE_POLY) {
        Collision_PolySweepSphereCollide(&ray, pCr, flags);
        // if a collision is found and the collision result parameter isn't NULL
        // normalise the result's normal field
        if (bFound && pCr != NULL) {
            pCr->normal.Normalise(&pCr->normal);
        }
    }

    return bFound;
}

void Collision_Deinit(void) {
    if (!bCollisionInit) {
        return;
    }

    DynamicItem** dynItems = dynamicModels.GetMem();
    while (*dynItems != NULL) {
        DynamicItem* item = *dynItems;
        item->Deinit();
        dynItems++;
    }

    dynamicModels.Deinit();

    collisionHeapIndex = 0;
    collisionHeapSize = 0;
    collisionPolysAdded = 0;

    if (pCollisionHeap != NULL) {
        Heap_MemFree(pCollisionHeap);
        pCollisionHeap = NULL;
    }

    bCollisionInit = false;
}

void Collision_AddDynamicModel(Model* pModel, CollisionInfo* pInfo, int subobjectIdx) {
    if (!bCollisionInit) {
        return;
    }

    DynamicItem* nextModel = dynamicModels.GetNextEntry();
    nextModel->pInfo = pInfo;
    nextModel->pModel = pModel;
    nextModel->idx = subobjectIdx;
    BoundingVolume* pVolume = nextModel->pModel->GetBoundingVolume(nextModel->idx);

    Vector tmp;
    tmp.ApplyRotMatrix(&pVolume->v2, &pModel->matrices[0]);

    float mag = tmp.Magnitude();
    nextModel->unk8 = mag;
    nextModel->unk10.Init(CalcNumDynamicLinks(mag), sizeof(CollisionNode));
    
    nextModel->unk44 = -1;
    nextModel->unk48 = -1;
    nextModel->unk4C = -1;
    nextModel->unk50 = -1;
    
    Vector* trans = nextModel->GetMatrix()->Row3();
    nextModel->pos.x = trans->x;
    nextModel->pos.y = trans->y;
    nextModel->pos.z = trans->z;
    nextModel->pos.w = trans->w;
    nextModel->pos.y += 100.0f;
    
    nextModel->Update();
    pModel->collisionTracking = true;
}

void Collision_AddDynamicSubobject(Model* pModel, int subobject, CollisionInfo* pInfo) {
    Collision_AddDynamicModel(pModel, pInfo, subobject);
}

void Collision_DeleteDynamicModel(Model* pModel) {
    if (!bCollisionInit) {
        return;
    }

    if (pModel == NULL) {
        DynamicItem** pDynItems = dynamicModels.GetMem();
        while (*pDynItems != NULL) {
            DynamicItem* pDynamicItem = *pDynItems;
            pDynamicItem->Deinit();
            pDynItems++;
        }

        dynamicModels.Reset();
        return;
    }

    DynamicItem** pItems = dynamicModels.GetMem();
    while (*pItems != NULL) {
        DynamicItem* pItem = *pItems;
        // deinit all DynamicItems which use this Model
        if (pItem->pModel == pModel) {
            pItem->Deinit();
            dynamicModels.Destroy(pItems);
            pModel->collisionTracking = false;
        }
        pItems++;
    }
}

void Collision_Draw(void) {}

void DynamicItem::Update(void) {
    // check if the object's position is different than its subobject position
    if (!pos.Equals(GetMatrix()->Row3())) {
        // if the position is different than the subobject's position
        // update the position and recompute AABB
        pos = *GetMatrix()->Row3();
        BoundingVolume* pVolume = pModel->GetBoundingVolume(idx);
        CalcAAB(pVolume, GetMatrix(), &unk14, &unk24);
        if (UpdateOverlap()) {
            Unlink();
            Link();
        }
    }
}

// might not be the correct inline?
bool DynamicItem::UpdateOverlap(void) {
    int old_unk44 = unk44;
    int old_unk48 = unk48;
    int old_unk4C = unk4C;
    int old_unk50 = unk50;
    Collision_FindSphereDynamicGrid(&unk14, &unk24, &unk44, &unk48, &unk4C, &unk50);
    return old_unk44 != unk44 || old_unk48 != unk48 || old_unk4C != unk4C || old_unk50 != unk50;
}

void DynamicItem::Unlink(void) {
    CollisionNode** pData = unk10.GetMem();
    while (*pData != NULL) {
        (*pData)->Remove();
        pData++;
    }

    GotoEnd();
}

void DynamicItem::Link(void) {
    for (int i = unk48; i < unk48 + unk50; i++) {
        for (int j = unk44; j < unk44 + unk4C; j++) {
            dynGrid[i * 32 + j].LINK(unk10.GetNextEntry(), this);
        }
    }
}
