#include "common/Collision.h"

CollisionResult lastCollision;
static bool bCollisionInit;
static bool bFound;
int collisionHeapSize;
void* pCollisionHeap;
int collisionHeapIndex;
static int collisionPolysAdded;
static PtrListDL<DynamicItem> dynamicModels;
bool Collision_BInteriorPoint;
static Item** Collision_Grid;
static int Collision_MinX;
static int Collision_MaxX;
static int Collision_MinZ;
static int Collision_MaxZ;
static int Collision_TilesAcross;
static int Collision_TilesDown;
static int Collision_TileWidth;
static int Collision_TileHeight;
static float Collision_DynGridTileSizeX;
static float Collision_DynGridTileSizeZ;
static CollisionNode dynGrid[1024];

#define EPSILON 1e-05f

void* CollisionHeap_Update(int size, int alignment) {
    if (alignment == 0) {
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

    *pStartX = Clamp(startX, 0, 0x1F);
    *pStartZ = Clamp(startZ, 0, 0x1F);
    *pLengthX = Clamp(endX, 0, 0x1F) - *pStartX + 1;
    *pLengthZ = Clamp(endZ, 0, 0x1F) - *pStartZ + 1;
}

bool CheckPoint(Vector* pVec, Vector* pVec1, Vector* point1, Vector* point2) {
    Vector v1;
    Vector v2;
    Vector v3;
    Vector v4;
	// Vector_Cross
    v1.x = point1->x - pVec1->x;
    v1.y = point1->y - pVec1->y;
    v1.z = point1->z - pVec1->z;
    v2.x = point2->x - pVec1->x;
    v2.y = point2->y - pVec1->y;
    v2.z = point2->z - pVec1->z;
    v3.x = v1.y * v2.z - v1.z * v2.y;
    v3.y = v1.z * v2.x - v1.x * v2.z;
    v3.z = v1.x * v2.y - v1.y * v2.x;
	// v3 is the surface normal?
    v4.x = pVec->x - pVec1->x;
    v4.y = pVec->y - pVec1->y;
    v4.z = pVec->z - pVec1->z;
    return v3.x * v4.x + v3.y * v4.y + v3.z * v4.z >= 0.0f;
}

static void CalcAAB(BoundingVolume* pVolume, Matrix* pMatrix1, Vector* pVec, Vector* pVec2) {
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
    
    for(int i = 0; i < 8; i++) {
        vecs[i].ApplyMatrix(&vecs[i], pMatrix1);
    }
    pVec->x = vecs[0].x;
    pVec->y = vecs[0].y;
    pVec->z = vecs[0].z;
    pVec->w = vecs[0].w;
    pVec2->x = vecs[0].x;
    pVec2->y = vecs[0].y;
    pVec2->z = vecs[0].z;
    pVec2->w = vecs[0].w;
    for(int i = 1; i < 8; i++) {
        pVec->x = Min<float>(pVec->x, vecs[i].x);
        pVec2->x = Max<float>(pVec2->x, vecs[i].x);
        pVec->y = Min<float>(pVec->y, vecs[i].y);
        pVec2->y = Max<float>(pVec2->y, vecs[i].y);
        pVec->z = Min<float>(pVec->z, vecs[i].z);
        pVec2->z = Max<float>(pVec2->z, vecs[i].z);
    }
}

void InterpolateVertexColor(Vector* pVec, Vector* pVec1, Vector* pos0, Vector* color0,
            Vector* pos1, Vector* color1, Vector* pos2, Vector* color2) {
    Vector diff0;
    Vector diff1;
    Vector cross;
    Vector diff2;
    diff0.Sub(pos1, pos0);
    diff1.Sub(pos2, pos0);
    cross.Cross(&diff0, &diff1);
    diff2.Sub(pVec, pos0);
    Vector cross2;
    cross2.Cross(&cross, &diff2);
    float dot = cross2.Dot(&diff0);
    float dot1 = dot - cross2.Dot(&diff1);
    float y = 0.0f;
    if (dot1) {
        y = dot / dot1;
    }
    dot = Clamp<float>(y, 0.0f, 1.0f);
    pVec1->InterpolateLinear(color1, color2, dot);
    Vector unkB8;
    unkB8.InterpolateLinear(pos1, pos2, dot);
    unkB8.Subtract(pos0);
    float t = unkB8.Dot(&diff2);
    float mag = unkB8.MagSquared();
    float x = 0.0f;
    if (mag) {
        x = t / mag;
    }
    pVec1->InterpolateLinear(color0, pVec1, Clamp<float>(x, 0.0f, 1.0f));
}

void ConvertRGBA(Vector* pOut, u8* pColors) {
    pOut->Set(pColors[0] / 255.0f, pColors[1] / 255.0f, pColors[2] / 255.0f, 1.0f);
}

void GetVertexColorFromPoly(Vector* vec1, Vector* vec2, Item* pItem) {
    Vector color0;
    Vector color1;
    Vector color2;
    u8* colors = (u8*)pItem->collisionThing->verts[0].color;
    ConvertRGBA(&color0, colors);
    ConvertRGBA(&color1, colors += 0x10);
    ConvertRGBA(&color2, colors += 0x10);
    InterpolateVertexColor(vec2, vec1, (Vector*)pItem->collisionThing->verts[0].pos, &color0, 
            (Vector*)pItem->collisionThing->verts[1].pos,
            &color1, (Vector*)pItem->collisionThing->verts[2].pos, &color2);
}

// pA and pB are line endpoints
// pVec is the vector to project on line AB
// pProj is the projected point if the point is not projected on pA or pB
Vector* PROJECT_TO_LINE_SEGMENT(Vector *pVec, Vector *pProj, Vector *pA, Vector *pB)
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
    Vector* pNearestOnAB = PROJECT_TO_LINE_SEGMENT(pVec, &onAB, pA, pB);
    Vector* pNearestOnBC = PROJECT_TO_LINE_SEGMENT(pVec, &onBC, pB, pC);
    Vector* pNearestOnCA = PROJECT_TO_LINE_SEGMENT(pVec, &onCA, pC, pA);
    Vector* pNearest = NEAREST_POINT(pVec, pNearestOnAB, NEAREST_POINT(pVec, pNearestOnBC, pNearestOnCA));
	
    pVec->Copy(pNearest);
}

static void NearestPointOnQuadEdge(Vector* pVec, Vector* pA, Vector* pB, Vector* pC, Vector* pD) {
    Vector onAB;
    Vector onBC;
    Vector onCD;
    Vector onDA;
    Vector* pNearestOnAB = PROJECT_TO_LINE_SEGMENT(pVec, &onAB, pA, pB);
    Vector* pNearestOnBC = PROJECT_TO_LINE_SEGMENT(pVec, &onBC, pB, pC);
    Vector* pNearestOnCD = PROJECT_TO_LINE_SEGMENT(pVec, &onCD, pC, pD);
    Vector* pNearestOnDA = PROJECT_TO_LINE_SEGMENT(pVec, &onDA, pD, pA);

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
    for(int i = 0; i < nmbrOfVertices - 1; i++) {
        if (CheckPoint(pVec, pVec1, &pVec2[indices[i]], &pVec2[indices[i + 1]])) {
            return false;
        }
    }
    return !CheckPoint(pVec, pVec1, &pVec2[indices[nmbrOfVertices - 1]], &pVec2[indices[0]]);
}

float SubDot(Vector* vec, Vector* vec1, Vector* vec2) {
    Vector tmp;
    tmp.Sub(vec, vec1);
    return tmp.Dot(vec2);
}

static bool SweepSphereToPoly(SphereRay* pRay, Vector* pVectors, int* indices, int numVerts, Vector* arg5, Vector* arg6, CollisionResult* pResult) {
    Vector pos = pRay->pos;
    Vector vec = *arg5;
    float min;
    if (arg6->Dot(&pRay->dXYZ) < 0.0f) {
        return false;
    }
    float d = SubDot(&pRay->pos, &pVectors[indices[0]], arg5);
    if (d > pRay->unk60 + pRay->radius) {
        return false;
    }
    if (d <= pRay->radius) {
        pos.Scale(arg6, d);
        min = 0.0f;
        pos.x += pRay->pos.x;
        pos.y += pRay->pos.y;
        pos.z += pRay->pos.z;
    } else {
        Vector tmp;
        tmp.Scale(arg6, pRay->radius);
        tmp.Add(&tmp, &pRay->pos);
        float t;
        if (arg5->Dot(&pRay->dXYZ) == 0.0f) {
            t = -1e+12f;
        } else {
            t = -(arg5->x * (tmp.x - pVectors[indices[0]].x) + 
                arg5->y * (tmp.y - pVectors[indices[0]].y) + 
                arg5->z * (tmp.z - pVectors[indices[0]].z)) / arg5->Dot(&pRay->dXYZ);
        }
        min = t;
        if (t <= 0.0f || t > pRay->unk60) {
            return false;
        }
        pos.x = (min * pRay->dXYZ.x);
        pos.y = (min * pRay->dXYZ.y);
        pos.z = (min * pRay->dXYZ.z);
        pos.x += tmp.x;
        pos.y += tmp.y;
        pos.z += tmp.z;
    }
    if (!PointInPoly(&pos, &pRay->pos, pVectors, indices, numVerts)) {
		// point isn't in the polygon
        NearestPointOnPolyEdge(&pos, pVectors, indices, numVerts);
        // pos is now the coordinate on the polygon's edge 
        if (pos.CheckSphereRadius(&pRay->pos, pRay->radius)) {
            // if the position on the polygon's edge is in the SphereRay's sphere
            Collision_BInteriorPoint = true;
            min = 0.0f;
            vec.Sub(&pRay->pos, &pos);
            if (vec.Dot(&pRay->dXYZ) > 0.0f) {
                return false;
            }
        } else {
            // position is in polygon
            min = GetDiv(&pos, &pRay->pos, &pRay->negDXYZ, pRay->radius);
            if (min <= 0.0f || min > pRay->unk60) {
                return false;
            }
            Vector tmp;
            tmp.Scale(&pRay->negDXYZ, min);
            tmp.Add(&tmp, &pos);
            vec.x = pRay->pos.x - tmp.x;
            vec.y = pRay->pos.y - tmp.y;
            vec.z = pRay->pos.z - tmp.z;
        }
    } else {
        Collision_BInteriorPoint = 0.0f == min;
    }
    pResult->normal = vec;
    pResult->pos = pos;
    pResult->unk40 = min;
    return true;
}

static bool SweepSphereToTri(SphereRay* pRay, Vector* pVec, Vector* pVec1, 
            Vector* pVec2, Vector* pVec3, Vector* pVec4, CollisionResult* pResult) {
    Vector pos = pRay->pos;
    Vector vec = *pVec3;
    float min;
    if (pVec4->Dot(&pRay->dXYZ) < 0.0f) {
        return false;
    }
    float d = SubDot(&pRay->pos, pVec, pVec3);
    if (d > pRay->unk60 + pRay->radius) {
        return false;
    }
    if (d <= pRay->radius) {
        pos.Scale(pVec4, d);
        min = 0.0f;
        pos.x += pRay->pos.x;
        pos.y += pRay->pos.y;
        pos.z += pRay->pos.z;
    } else {
        Vector tmp;
        tmp.Scale(pVec4, pRay->radius);
        tmp.Add(&tmp, &pRay->pos);
        float t;
        if (pVec3->Dot(&pRay->dXYZ) == 0.0f) {
            t = -1e+12f;
        } else {
            t = -(pVec3->x * (tmp.x - pVec->x) + 
                pVec3->y * (tmp.y - pVec->y) + 
                pVec3->z * (tmp.z - pVec->z)) / pVec3->Dot(&pRay->dXYZ);
        }
        min = t;
        if (t <= 0.0f || t > pRay->unk60) {
            return false;
        }
        pos.x = (min * pRay->dXYZ.x);
        pos.y = (min * pRay->dXYZ.y);
        pos.z = (min * pRay->dXYZ.z);
        pos.x += tmp.x;
        pos.y += tmp.y;
        pos.z += tmp.z;
    }
    if (!CheckPoint(&pos, &pRay->pos, pVec1, pVec) || 
            !CheckPoint(&pos, &pRay->pos, pVec2, pVec1) || !CheckPoint(&pos, &pRay->pos, pVec, pVec2)) {
		// point isn't in the triangle
        NearestPointOnTriEdge(&pos, pVec, pVec1, pVec2);
        // pos is now the coordinate on the polygon's edge 
        if (pos.CheckSphereRadius(&pRay->pos, pRay->radius)) {
            // if the position on the polygon's edge is in the SphereRay's sphere
            Collision_BInteriorPoint = true;
            min = 0.0f;
            vec.Sub(&pRay->pos, &pos);
            if (vec.Dot(&pRay->dXYZ) > 0.0f) {
                return false;
            }
        } else {
            // position is in polygon
            min = GetDiv(&pos, &pRay->pos, &pRay->negDXYZ, pRay->radius);
            if (min <= 0.0f || min > pRay->unk60) {
                return false;
            }
            Vector tmp;
            tmp.Scale(&pRay->negDXYZ, min);
            tmp.Add(&tmp, &pos);
            vec.x = pRay->pos.x - tmp.x;
            vec.y = pRay->pos.y - tmp.y;
            vec.z = pRay->pos.z - tmp.z;
        }
    } else {
        Collision_BInteriorPoint = 0.0f == min;
    }
    pResult->normal = vec;
    pResult->pos = pos;
    pResult->unk40 = min;
    return true;
}

void FindGridBoundaries(Vector* pMin, Vector* pMax, int* minX, int* minZ, int* maxX, int* maxZ) {
    int local_24 = (Min<float>(pMin->x, pMax->x) - Collision_MinX) / Collision_TileWidth;
    int local_28 = (Min<float>(pMin->z, pMax->z) - Collision_MinZ) / Collision_TileHeight;
    int local_2C = (Max<float>(pMin->x, pMax->x) - Collision_MinX) / Collision_TileWidth;
    int local_30 = (Max<float>(pMin->z, pMax->z) - Collision_MinZ) / Collision_TileHeight;
    *minX = Max<int>(local_24, 0);
    *minZ = Max<int>(local_28, 0);
    *maxX = Min<int>(local_2C, Collision_TilesAcross - 1);
    *maxZ = Min<int>(local_30, Collision_TilesDown - 1);
}

void FindGridBoundariesSphere(Vector* pPos, float radius, int* minX, int* minZ, int* maxX, int* maxZ) {
    int local_24 = ((pPos->x - radius) - Collision_MinX) / Collision_TileWidth;
    int local_28 = ((pPos->z - radius) - Collision_MinZ) / Collision_TileHeight;
    int local_2C = ((pPos->x + radius) - Collision_MinX) / Collision_TileWidth;
    int local_30 = ((pPos->z + radius) - Collision_MinZ) / Collision_TileHeight;
    *minX = Max<int>(local_24, 0);
    *minZ = Max<int>(local_28, 0);
    *maxX = Min<int>(local_2C, Collision_TilesAcross - 1);
    *maxZ = Min<int>(local_30, Collision_TilesDown - 1);
}

void Collision_InitModule(void) {}

void Collision_DeinitModule(void) {}

static void StoreSphereResult(Item* pItem, Vector* pVec, CollisionResult* pResult, float arg4) {
    lastCollision.pos = *(Vector*)&pItem->collisionThing->verts[0].pos;
    lastCollision.normal = pItem->collisionThing->normal;
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
int Collision_SphereCollide(Vector *pPos, float radius, CollisionResult *pCr, int flags, int maxCollisions) {
    int minx;
    int minz;
    int maxx;
    int maxz;
    int numCollisions = 0;
    FindGridBoundariesSphere(pPos, radius, &minx, &minz, &maxx, &maxz);
    Item *pItem;

    for (int j = minz; j <= maxz; j++) {
        for (int i = minx; i <= maxx; i++) {
            pItem = Collision_Grid[j * Collision_TilesAcross + i];

            while (pItem != NULL) {
                if ((pItem->pTriangle->pCollisionInfo == NULL || pItem->pTriangle->pCollisionInfo->bEnabled) &&
                    !(pItem->pTriangle->flags & flags)) {
                    CollisionVertex *pVerts = pItem->collisionThing->verts;
                    Vector *triVert0 = (Vector *)&pItem->collisionThing->verts[0];
                    Vector *triVert1 = (Vector *)&pItem->collisionThing->verts[1];
                    Vector *triVert2 = (Vector *)&pItem->collisionThing->verts[2];
                    float fVar19 =
                        SubDot(pPos, (Vector *)&pItem->collisionThing->verts[0], &pItem->collisionThing->normal);
                    float fVar0 = fVar19 < 0.0f ? -fVar19 : fVar19; // abs
                    if (!(fVar0 > radius)) {
                        Vector v1Diff;
                        v1Diff.Sub(triVert0, pPos);
                        if (v1Diff.MagSquared() <= radius * radius) {
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
                            v1Diff.Sub(triVert1, pPos);
                            if (v1Diff.MagSquared() <= radius * radius) {
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
                                v1Diff.Sub(triVert2, pPos);
                                if (v1Diff.MagSquared() <= radius * radius) {
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
                                    Vector tmp25;
                                    Vector cross;
                                    Vector diff;
                                    v1Diff.Sub(triVert1, triVert0);
                                    if (CylTest_CapsFirst(triVert0, triVert1, v1Diff.MagSquared(), radius * radius,
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
                                        v1Diff.Sub(triVert2, triVert1);
                                        if (CylTest_CapsFirst(triVert1, triVert2, v1Diff.MagSquared(), radius * radius,
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
                                            Vector cross;
                                            v1Diff.Sub(triVert0, triVert2);
                                            if (CylTest_CapsFirst(triVert2, triVert0, v1Diff.MagSquared(),
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
                                                v1Diff.Sub(triVert1, triVert0);
                                                cross.Cross(&v1Diff, &pItem->collisionThing->normal);
                                                Vector tmp;
                                                tmp.Sub(pPos, triVert0);
                                                if (tmp.Dot(&cross) >= 0.0f) {

                                                    v1Diff.Sub(triVert2, triVert1);
                                                    cross.Cross(&v1Diff, &pItem->collisionThing->normal);

                                                    tmp.Sub(pPos, triVert1);

                                                    if (tmp.Dot(&cross) >= 0.0f) {

                                                        v1Diff.Sub(triVert0, triVert2);
                                                        cross.Cross(&v1Diff, &pItem->collisionThing->normal);
                                                        Vector tmp;
                                                        tmp.Sub(pPos, triVert2);

                                                        if (tmp.Dot(&cross) >= 0.0f) {
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


bool CheckTrianglePoint(float* pPoint, Item* pItem) {
    Vector v;
    v.x = pPoint[0] - pItem->collisionThing->verts[0].pos[0];
    v.y = pPoint[1] - pItem->collisionThing->verts[0].pos[1];
    v.z = pPoint[2] - pItem->collisionThing->verts[0].pos[2];
    return v.x * pItem->collisionThing->normal.x +
        v.y * pItem->collisionThing->normal.y +
        v.z * pItem->collisionThing->normal.z >= 0.0f;
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

void SwapPtrs(Vector** p0, Vector** p1) {
    Vector* tmp = *p0;
    *p0 = *p1;
    *p1 = tmp;
}

static bool Collision_PolyCollide(Vector* pVec0, Vector* pVec1, Vector* pVec2, CollisionResult* pCr, int flags) {
    int minx;
    int minz;
    int maxx;
    int maxz;
    FindGridBoundaries(pVec0, pVec1, &minx, &minz, &maxx, &maxz);
    bFound = false;
    for(int j = minz; j <= maxz; j++) {
        for(int i = minx; i <= maxx; i++) {
            Item* pItem = Collision_Grid[j * Collision_TilesAcross + i];
            while (pItem != NULL) {
                if ((pItem->pTriangle->pCollisionInfo == NULL ||
                    pItem->pTriangle->pCollisionInfo->bEnabled) &&
                    !(pItem->pTriangle->flags & flags)) {
                    bool b0 = CheckTrianglePoint((float*)pVec1, pItem);
                    bool b1 = CheckTrianglePoint((float*)pVec0, pItem);
                    if (b1 != b0) {
                        Vector* p1 = (Vector*)&pItem->collisionThing->verts[0].pos;
                        Vector* p2 = (Vector*)&pItem->collisionThing->verts[1].pos;
                        Vector* p3 = (Vector*)&pItem->collisionThing->verts[2].pos;
                        Vector surfaceNormal = pItem->collisionThing->normal;
                        if (b0) {
                            SwapPtrs(&p2, &p3);
                            surfaceNormal.Inverse();
                        }
                        if (CheckPoint(pVec1, pVec0, p2, p1) &&
                            CheckPoint(pVec1, pVec0, p3, p2) &&
                            CheckPoint(pVec1, pVec0, p1, p3)) {
							// ray is intersecting the polygon
                            float d = surfaceNormal.Dot(pVec2);
                            if (d < 0.0f) {
                                Vector newRayNormal;
                                newRayNormal.Sub(pVec0, p1);
								// finalPos is the intersection point of the ray and polygon
								// finalPos may be called "a"?
                                Vector finalPos;
                                float scale = -surfaceNormal.Dot(&newRayNormal) / d;
                                finalPos.Scale(pVec2, scale);
                                finalPos.Add(pVec0);
                                lastCollision.pos = finalPos;
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
                                pVec1 = &lastCollision.pos; // set the new end position to the position of the collision
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
    FindGridBoundaries(&pRay->unk40, &pRay->unk50, &minx, &minz, &maxx, &maxz);

    for(int j = minz; j <= maxz; j++) {
        for(int i = minx; i <= maxx; i++) {
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
                        if (!CheckItemSphereRay(pItem, &pRay->unk40, &pRay->unk50) && 
                            (pItem->pTriangle->pCollisionInfo == NULL ||
                            pItem->pTriangle->pCollisionInfo->bEnabled) &&
                            !(pItem->pTriangle->flags & flags)) {
                            Vector* p1 = (Vector*)&pItem->collisionThing->verts[0].pos;
                            Vector* p3 = (Vector*)&pItem->collisionThing->verts[1].pos;
                            Vector* p2 = (Vector*)&pItem->collisionThing->verts[2].pos;
                            Vector normal = pItem->collisionThing->normal;
                            Vector invNormal = pItem->collisionThing->normal;
                            invNormal.Inverse();
                            bool swept;
                            if (CheckTrianglePoint((float*)&pRay->pos, pItem)) {
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
                                    // if a CollisionResult was passed, copy the result to it
                                    *pCr = lastCollision;
                                }
                                pRay->unk60 = lastCollision.unk40;
                                pRay->pos1.Scale(&pRay->dXYZ, pRay->unk60);
                                pRay->pos1.Add(&pRay->pos1, &pRay->pos);
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
    int local_24 = (Min<float>(pVert0->x, Min<float>(pVert1->x, pVert2->x)) - Collision_MinX) / Collision_TileWidth;
    int local_28 = (Min<float>(pVert0->z, Min<float>(pVert1->z, pVert2->z)) - Collision_MinZ) / Collision_TileHeight;
    int local_2C = (Max<float>(pVert0->x, Max<float>(pVert1->x, pVert2->x)) - Collision_MinX) / Collision_TileWidth;
    int local_30 = (Max<float>(pVert0->z, Max<float>(pVert1->z, pVert2->z)) - Collision_MinZ) / Collision_TileHeight;
    if (local_24 >= 0 && local_2C < Collision_TilesAcross &&
        local_28 >= 0 && local_30 < Collision_TilesDown) {
        for(int j = local_28; j <= local_30; j++) {
            for(int i = local_24; i <= local_2C; i++) {
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
	return (pRay->unk60 + (2.0f * pRay->radius) + pDynItem->unk8);
}

void Collision_Init(int heapSize, float minX, float /* unused */ minY, float minZ, 
		float width, float height, int tilesAcross, int tilesDown) {
    bCollisionInit = true;
    collisionHeapIndex = 0;
    collisionHeapSize = (heapSize + 0x1f) & ~0x1f; // round up to 32 bytes
    pCollisionHeap = Heap_MemAlloc(heapSize);
    dynamicModels.Init(768, sizeof(DynamicItem));
    
    Collision_TilesDown = tilesDown;
    Collision_TilesAcross = tilesAcross;
    // this call might use the parameters rather than the local variables
    Collision_Grid = (Item**)CollisionHeap_Update(Collision_TilesDown * Collision_TilesAcross * sizeof(Item**), 0);
    
    for(int i = 0; i < tilesDown * tilesAcross; i++) {
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
    
    for(int i = 0; i < 1024; i++)
    {
        dynGrid[i].PTR_0x8 = NULL;
        dynGrid[i].pNext = &dynGrid[i];
        dynGrid[i].pPrev = &dynGrid[i];
    }
}

// this function proves miny is a parameter to void Collision_Init(int, float, float, float, float, float, int, int);
// May be placed elsewhere in the file
/* Unused Function */
void Collision_Init(int heapSize, Model* pModel, int arg2, int arg3) {
    arg2 = Max<int>(0x40, arg2);
    arg3 = Max<int>(0x40, arg3);
    Vector sp0;
    Vector sp10;
    sp10 = pModel->pTemplate->pModelData->volume.v1;
    sp0.Add(&sp10, &pModel->pTemplate->pModelData->volume.v2);
    sp10.ApplyMatrix(&pModel->matrices[0]);
    sp0.ApplyMatrix(&pModel->matrices[0]);
    float minx = Min<float>(sp10.x, sp0.x);
    float miny = Min<float>(sp10.y, sp0.y);
    float minz = Min<float>(sp10.z, sp0.z);
    float maxx = Max<float>(sp10.x, sp0.x);
    float maxy = Max<float>(sp10.y, sp0.y);
    float maxz = Max<float>(sp10.z, sp0.z);
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
    if (!pExplorer) return;
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
                pTri = (CollisionTriangle*)CollisionHeap_Update(sizeof(Item), 0);
                pTri->flags = pExplorer->pMaterial->collisionFlags;
                pTri->subObjectIdx = pExplorer->subObjectIdx;
                pTri->pCollisionInfo = pInfo;
            }

            CollisionThing* pTriData = (CollisionThing*)CollisionHeap_Update(sizeof(CollisionTriangle), 0);
            // loop through all vertices of the triangle
            for(int i = 0; i < 3; i++) {
                ((Vector*)&pTriData->verts[i].pos)->ApplyMatrix(&pExplorer->vertices[i].pos, &pModel->matrices[0]);
                float r = (pExplorer->vertices[i].color.x * pModel->colour.x);
                float g = (pExplorer->vertices[i].color.y * pModel->colour.y);
                float b = (pExplorer->vertices[i].color.z * pModel->colour.z);
                pTriData->verts[i].color[0] = r * 255.0f;
                pTriData->verts[i].color[1] = g * 255.0f;
                pTriData->verts[i].color[2] = b * 255.0f;
                pTriData->verts[i].color[3] = 0xff;
            }
            item.pTriangle = pTri;
            item.collisionThing = pTriData;
            Vector baDiff;
            Vector caDiff;
            baDiff.Sub((Vector*)&pTriData->verts[1].pos, (Vector*)&pTriData->verts[0].pos);
            caDiff.Sub((Vector*)&pTriData->verts[2].pos, (Vector*)&pTriData->verts[0].pos);
            pTriData->normal.Cross(&caDiff, &baDiff);
            pTriData->normal.Normalise(&pTriData->normal);
            if (pTriData->normal.Dot(&pTriData->normal) > 0.0f) {
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
    if (pDynItem->GetMatrix()->Row3()->DistSq(&pRay->pos) >
        Sqr<float>(pRay->unk60 + (2.0f * pRay->radius) + pDynItem->unk8)) return false;
    // "corners"?
    Vector vecs[8] = {{}, {}, {}, {}, {}, {}, {}, {}};
    vecs[0].x = v1.x;
    vecs[0].y = v1.y;
    vecs[0].z = v1.z;
    vecs[1].x = v1.x;
    vecs[1].y = v1.y;
    vecs[1].z = v2.z;
    vecs[2].x = v2.x;
    vecs[2].y = v1.y;
    vecs[2].z = v2.z;
    vecs[3].x = v2.x;
    vecs[3].y = v1.y;
    vecs[3].z = v1.z;
    vecs[4].x = v1.x;
    vecs[4].y = v2.y;
    vecs[4].z = v1.z;
    vecs[5].x = v1.x;
    vecs[5].y = v2.y;
    vecs[5].z = v2.z;
    vecs[6].x = v2.x;
    vecs[6].y = v2.y;
    vecs[6].z = v2.z;
    vecs[7].x = v2.x;
    vecs[7].y = v2.y;
    vecs[7].z = v1.z;
    for(int i = 0; i < 8; i++) {
        vecs[i].ApplyMatrix(&vecs[i], pDynItem->GetMatrix());
    }
    // might not be the best way to format this?
    // originally named "quads"?
    int indices[6][4] = {
        {4, 5, 6, 7}, 
        {3, 2, 1, 0},
        {6, 2, 3, 7},
        {4, 0, 1, 5},
        {5, 1, 2, 6},
        {7, 3, 0, 4}
    };
    // "normals"?
    Vector vecs2[6];
    vecs2[0].Sub(&vecs[4], &vecs[0]);
    vecs2[0].Normalise(&vecs2[0]);
    
    vecs2[1].Scale(&vecs2[0], -1.0f);
    vecs2[2].Sub(&vecs[3], &vecs[0]);
    vecs2[2].Normalise(&vecs2[2]);
    
    vecs2[3].Scale(&vecs2[2], -1.0f);
    vecs2[4].Sub(&vecs[1], &vecs[0]);
    vecs2[4].Normalise(&vecs2[4]);

    vecs2[5].Scale(&vecs2[4], -1.0f);
    
    for(int i = 0; i < 3; i++) {
        int dotTest = vecs2[i * 2].Dot(&pRay->dXYZ) > 0.0f ? 1 : 0;
        if (CheckPoint(&pRay->pos, &vecs[indices[(dotTest + 2*i)][0]], &vecs[indices[(dotTest + 2*i)][1]], &vecs[indices[(dotTest + 2*i)][2]]) &&
                    SweepSphereToPoly(pRay, vecs, &indices[(dotTest + 2*i)][0], 4, &vecs2[(dotTest + 2*i)], &vecs2[(1 - dotTest) + 2*i], &lastCollision)) {
            lastCollision.pInfo = pDynItem->pInfo;
            lastCollision.pModel = pDynItem->pModel;
            lastCollision.collisionFlags = pDynItem->pModel->GetSubObjectMaterial(0, 0)->collisionFlags;
            lastCollision.itemIdx = Max<int>(0, pDynItem->idx);
            lastCollision.color = pDynItem->pModel->colour;
            pRay->unk60 = lastCollision.unk40;
            pRay->pos1.Scale(&pRay->dXYZ, pRay->unk60);
            pRay->pos1.Add(&pRay->pos1, &pRay->pos);
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
        int r6 = 0;
        for(int i = 0; i < 2; i++) {
            float f6 = !r6 ? pVolume->v1.x : pVolume->v1.x + pVolume->v2.x;
            float f9 = f8 * (f6 - newStart.x);
            if (f9 >= 0.0f && f9 < div) {
                float py = newStart.y + (f9 * diff.y);
                float pz = newStart.z + (f9 * diff.z);
                // if y and z are in bounds
                if (py >= pVolume->v1.y && py < pVolume->v1.y + pVolume->v2.y &&
                    pz >= pVolume->v1.z && pz < pVolume->v1.z + pVolume->v2.z) {
                    div = f9;
                    bFound = true;
                    lastCollision.pos.x = f6;
                    lastCollision.pos.y = py;
                    lastCollision.pos.z = pz;
                    lastCollision.normal.x = r6 == 0 ? -1.0f : 1.0f;
                    lastCollision.normal.y = 0.0f;
                    lastCollision.normal.z = 0.0f;
                    if (pCr) {
                        pCr->pos = lastCollision.pos;
                        pCr->normal = lastCollision.normal;
                    }
                }
            }
            r6++;
        }
    }

    if (diff.y) {
        float f8 = 1.0f / diff.y;
        int r6 = 0;
        for(int i = 0; i < 2; i++) {
            float f6 = !r6 ? pVolume->v1.y : pVolume->v1.y + pVolume->v2.y;
            float f9 = f8 * (f6 - newStart.y);
            if (f9 >= 0.0f && f9 < div) {
                float px = newStart.x + (f9 * diff.x);
                float pz = newStart.z + (f9 * diff.z);
                // if x and z are in bounds
                if (px >= pVolume->v1.x && px < pVolume->v1.x + pVolume->v2.x &&
                    pz >= pVolume->v1.z && pz < pVolume->v1.z + pVolume->v2.z) {
                    div = f9;
                    bFound = true;
                    lastCollision.pos.x = px;
                    lastCollision.pos.y = f6;
                    lastCollision.pos.z = pz;
                    lastCollision.normal.x = 0.0f;
                    lastCollision.normal.y = r6 == 0 ? -1.0f : 1.0f;
                    lastCollision.normal.z = 0.0f;
                    if (pCr) {
                        pCr->pos = lastCollision.pos;
                        pCr->normal = lastCollision.normal;
                    }
                }
            }
            r6++;
        }
    }

    if (diff.z) {
        float f8 = 1.0f / diff.z;
        int r6 = 0;
        for(int i = 0; i < 2; i++) {
            float f6 = r6 == 0 ? pVolume->v1.z : pVolume->v1.z + pVolume->v2.z;
            float f9 = f8 * (f6 - newStart.z);
            if (f9 >= 0.0f && f9 < div) {
                float px = newStart.x + (f9 * diff.x);
                float py = newStart.y + (f9 * diff.y);
                // if x and y are in bounds
                if (px >= pVolume->v1.x && px < pVolume->v1.x + pVolume->v2.x &&
                    py >= pVolume->v1.y && py < pVolume->v1.y + pVolume->v2.y) {
                    div = f9;
                    bFound = true;
                    lastCollision.pos.x = px;
                    lastCollision.pos.y = py;
                    lastCollision.pos.z = f6;
                    lastCollision.normal.x = 0.0f;
                    lastCollision.normal.y = 0.0f;
                    lastCollision.normal.z = r6 == 0 ? -1.0f : 1.0f;
                    if (pCr) {
                        pCr->pos = lastCollision.pos;
                        pCr->normal = lastCollision.normal;
                    }
                }
            }
            r6++;
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

bool Collision_RayCollide(Vector* vec1, Vector* vec2, CollisionResult* pCr, CollisionMode mode, int r7) {
    bFound = false;
    if (mode == (CollisionMode)0 || mode == (CollisionMode)1) {
		// Dynamic_Collides()
        Vector diff;
        diff.Sub(vec1, vec2);
        float mag = diff.Magnitude();
        Vector min = {
            Min<float>(vec1->x, vec2->x),
            Min<float>(vec1->y, vec2->y),
            Min<float>(vec1->z, vec2->z),
            1.0f
        };
        Vector max = {
            Max<float>(vec1->x, vec2->x),
            Max<float>(vec1->y, vec2->y),
            Max<float>(vec1->z, vec2->z),
            1.0f
        };
        int overlapX;
        int overlapZ;
        int overlapXSize;
        int overlapZSize;
        // might not be the correct function, but it is equivalent
        Collision_FindSphereDynamicGrid(&min, &max, &overlapX, &overlapZ, &overlapXSize, &overlapZSize);
        for(int i = overlapZ; i < overlapZ + overlapZSize; i++) {
            for(int j = overlapX; j < overlapX + overlapXSize; j++) {
                CollisionNode* currNode = dynGrid[i * 0x20 + j].pNext;
                while (currNode != &dynGrid[i * 0x20 + j]) {
                    DynamicItem* currItem = (DynamicItem*)currNode->PTR_0x8;
                    // potentially fix CollisionNode fields
                    if (currItem->pInfo == NULL || currItem->pInfo->bEnabled) {
                        bool collideItem = Collision_RayCollideDynamicItem(vec1, vec2, pCr, currItem, mag);
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
    if (!bFound && (mode == (CollisionMode)0 || mode == (CollisionMode)2)) {
        // Stack_Collides()
        Vector vec;
        vec.Sub(vec2, vec1);
        return Collision_PolyCollide(vec1, vec2, &vec, pCr, r7);
    }
    return bFound;
}

bool Collision_SweepSphereCollide(Vector* pVec, Vector* pVec1, float sphereRadius,
    CollisionResult* pCr, CollisionMode pMode, int arg3) {
    // create SphereRay
    SphereRay ray;
    bFound = false;
    ray.Create(pVec, pVec1, sphereRadius);
    if (pMode == 0 || pMode == 1) {
        int startX;
        int startZ;
        int lengthX;
        int lengthZ;
        Collision_FindSphereDynamicGrid(&ray.unk40, &ray.unk50, &startX, &startZ, &lengthX, &lengthZ);
        for(int i = startZ; i < startZ + lengthZ; i++) {
            for (int j = startX; j < startX + lengthX; j++) {
                CollisionNode* currNode = dynGrid[i * 0x20 + j].pNext;
                while (currNode != &dynGrid[i * 0x20 + j]) {
                    DynamicItem* currItem = (DynamicItem*)currNode->PTR_0x8;
                    // potentially fix CollisionNode fields
                    if (currItem->pInfo == NULL || currItem->pInfo->bEnabled) {
                        bFound = Collision_SweepSphereCollideDynamicModel(&ray, pCr, (DynamicItem*)currNode->PTR_0x8) || bFound;
                    }
                    currNode = currNode->pNext; // next might be at 0x4?
                }
            }
        }
    }
    if (pMode == 0 || pMode == 2) {
        Collision_PolySweepSphereCollide(&ray, pCr, arg3);
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
    Vector vec;
    vec.ApplyRotMatrix(&pVolume->v2, &pModel->matrices[0]);
    float mag = vec.Magnitude();
    nextModel->unk8 = mag;
    int gridMag = mag / Min<float>(Collision_DynGridTileSizeX, Collision_DynGridTileSizeZ);
    int size = Sqr<int>(gridMag + 2);
	nextModel->unk10.Init(size, sizeof(CollisionNode));
	
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
    pModel->flags.bits.b4 = true;
}

void Collision_AddDynamicSubobject(Model* pModel, int subobject, CollisionInfo* pInfo) {
	Collision_AddDynamicModel(pModel, pInfo, subobject);
}

inline void Swap(DynamicItem** p, DynamicItem** p1) {
    DynamicItem* tmp = *p;
    *p = *p1;
    *p1 = tmp;
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
        while (*dynamicModels.pMem != NULL) {
            dynamicModels.pMem++;
        }
        return;
    }
    DynamicItem** pItems = dynamicModels.GetMem();
    while (*pItems != NULL) {
        DynamicItem* pItem = *pItems;
		// deinit all DynamicItems which use this Model
        if (pItem->pModel == pModel) {
            pItem->Deinit();
            Swap(dynamicModels.pMem++, pItems);
            pModel->flags.bits.b4 = false;
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
    for (int i = unk48; i < unk48 + unk50; i++)
    {
        for (int j = unk44; j < unk44 + unk4C; j++)
        {
            dynGrid[i * 32 + j].LINK(unk10.GetNextEntry(), this);
        }
    }
}