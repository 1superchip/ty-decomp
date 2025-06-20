#include "ty/tools.h"
#include "common/Timer.h"
#include "common/StdMath.h"
#include "ty/controlval.h"
#include "ty/StructList.h"
#include "ty/RangeCheck.h"
#include "common/Str.h"
#include "common/System_GC.h"
#include "common/system_extras.h"
#include "ty/global.h"

// array of pregenerated random floats
float randomFloats[32] = { 
    -0.14911457896232605f, 0.2968113124370575f,
    -0.242182657122612f, -0.03677469864487648,
    0.23941156268119812f, -0.0910588949918747,
    0.3864505887031555f, 0.1572219431400299,
    -0.3298359513282776f, -0.41071179509162903,
    0.38811826705932617f, 0.47512078285217285,
    -0.13219785690307617f, 0.3332674503326416,
    0.21101564168930054f, -0.36352109909057617,
    0.3474756181240082f, -0.2174292355775833,
    -0.08759672194719315f, 0.3332221508026123,
    -0.4258323311805725f, 0.05197399854660034,
    0.1978643536567688f, -0.18719148635864258,
    0.44926562905311584f, 0.37327125668525696,
    -0.10966271907091141f, 0.18100666999816895,
    -0.13058409f, 0.2125595f,
    -0.16995443f, -0.4468846f
};

extern "C" void Sound_SetPitch(int, float);
extern "C" void strcpy(char*, const char*);

static bool bDropShadowsIsInit = false;
static StructList<ShadowInfo> shadows;

extern "C" char* strstr(char*, char*);

/*
// Stripped function in the ps2 build
uint Tools_ColorToABGR(Vector* pColor) {
    uint r = (uint)(pColor->x * 128.0f);
    uint g = (uint)(pColor->y * 128.0f);
    uint b = (uint)(pColor->z * 128.0f);
    uint a = (uint)(pColor->w * 128.0f);
    uint abgr = r;
    abgr |= (g << 0x8);
    abgr |= (b << 0x10);
    abgr |= (a << 0x18);
    return abgr;
}
*/

void Tools_ApplyDoppler(int arg0, Vector* pVec, Vector* pVec1, Vector* pVec2, Vector* pVec3) {
    float f1 = 300.0f;

    Vector toPos;
    toPos.Sub(pVec2, pVec);
    toPos.Normalise();

    Vector relativeVel;
    relativeVel.Sub(pVec3, pVec1);

    float f2 = relativeVel.Dot(&toPos);

    if (f2 <= -f1) {
        return;
    }
    
    Sound_SetPitch(arg0, f1 / (f1 + f2));
}

Model* Tools_GetGroundLightModel(Vector* pVec, int* pSubObjectIndex, float maxDist) {
    Model* pGroundModel = NULL;

    for (int i = 0; i < gb.level.nmbrOfLayers; i++) {
        if (gb.level.layers[i].pModel && strstr(gb.level.layers[i].pModel->GetName(), "_alm")) {
            pGroundModel = gb.level.layers[i].pModel;
            pGroundModel->renderType = 7;
            pGroundModel->colour = gb.level.alphaLightMapColor;
            break;
        }
    }

    int numSubObjects = pGroundModel->GetNmbrOfSubObjects();
    float closestDist = Sqr<float>(100000.0f);
    int groundSubObject = -1;

    for (int subObjectIndex = 0; subObjectIndex < numSubObjects; subObjectIndex++) {
        float subObjDist = SquareDistance(pGroundModel->GetSubObjectOrigin(subObjectIndex), pVec);
        if (subObjDist < closestDist) {
            closestDist = subObjDist;
            groundSubObject = subObjectIndex;
        }
    }

    if (closestDist > maxDist * maxDist) {
        return NULL;
    }
    
    pGroundModel->SetAlphaLightIntensity(groundSubObject, 0.0f);

    *pSubObjectIndex = groundSubObject;

    return pGroundModel;
}

bool Tools_CollideXZ(Vector* pVec, Vector* pVec1, Vector* pVec2, Vector* pVec3,
        float f1, float radius, float f3, float f4) {
    
    if (radius && !pVec->IsInsideSphere(pVec1, radius)) {
        return false;
    }

    Vector toPos2;
    toPos2.Sub(pVec1, pVec);
    toPos2.y = 0.0f;

    float len = toPos2.Normalise();

    float dot0 = pVec2->Dot(&toPos2);
    float dot1 = pVec3->Dot(&toPos2);

    if (dot1 < dot0) {
        float f3_ = (f3 * dot0) + (f4 * dot1);
        float f0 = f1 * (dot0 - dot1);
        
        Vector v;
        v.Scale(&toPos2, (f3_ + (f0 * f3)) / (f3 + f4) - dot1);

        pVec3->x += v.x;
        pVec3->z += v.z;

        v.Scale(&toPos2, (f3_ - (f0 * f4)) / (f3 + f4) - dot0);

        pVec2->x += v.x;
        pVec2->z += v.z;
    }

    if (radius) {
        Vector temp = toPos2;

        temp.Scale(&toPos2, ((radius - len) * f3) / (f3 + f4));

        pVec1->Add(&temp);

        temp.Scale(&toPos2, ((radius - len) * f4) / (f3 + f4));

        pVec->Subtract(&temp);

        return true;
    }

    return dot1 < dot0;
    // next 2 lines match PS2 build
    // if (dot1 < dot0) return true;
    // return false;
}

// Copies pSrc to pDest until '.' is found and then sets the '.' to '\0' in pDest
void Tools_StripExtension(char* pDest, const char* pSrc) {
    strcpy(pDest, pSrc);
    char* stripped = pDest;
    while (*stripped != '\0') {
        if (*stripped == '.') {
            *stripped = '\0';
            break;
        }
        stripped++;
    }
}

Vector Tools_GroundColor(CollisionResult* pCr) {
    Vector color;

    if (pCr->pModel) {
        color = pCr->color;
        color.w = 1.0f;
        return color;
    }

    color = pCr->color;
    color.w = 1.0f;

    if (!pCr->pInfo) {
        color.x += 0.2f;
        color.y += 0.2f;
        color.z += 0.2f;
    }

    if (color.x > 1.0f) {
        color.x = 1.0f;
    }

    if (color.y > 1.0f) {
        color.y = 1.0f;
    }

    if (color.z > 1.0f) {
        color.z = 1.0f;
    }

    return color;
}

float Tools_RandomGaussian(void) {
    float angle = RandomFR(&gb.mRandSeed, -1.0f, 1.0f);
    float randomy = RandomFR(&gb.mRandSeed, -1.0f, 1.0f);
    
    float f1 = _table_cosf(angle * PI);

    if (f1 < randomy) {
        if (angle > 0.0f) {
            return angle - 1.0f;
        } else {
            return angle += 1.0f;
        }
    }

    return angle;
}

void Tools_ApplyRotationToSubObject(Model* pModel, int subobjectIdx, Matrix* pMatrix, int secondSubObject) {
    int matrixIdx = pModel->GetSubObjectMatrixIndex(subobjectIdx);
    Matrix* pSubObjectMatrix = &pModel->pMatrices[matrixIdx];
    Vector objectOrigin = *pModel->GetSubObjectOrigin(subobjectIdx);

    Vector tmp;
    tmp.Scale(&objectOrigin, -1.0f);

    pSubObjectMatrix->SetIdentity();
    pSubObjectMatrix->Translate(&tmp);
    pSubObjectMatrix->Multiply(pMatrix);

    if (secondSubObject != 0) {
        int matrixIdx2 = pModel->GetSubObjectMatrixIndex(secondSubObject);
        Matrix* pSubObjectMatrix2 = &pModel->pMatrices[matrixIdx2];
        pSubObjectMatrix->Translate(&objectOrigin);
        pSubObjectMatrix->Multiply(pSubObjectMatrix2);
        pSubObjectMatrix->Translate(&tmp);
    }

    pSubObjectMatrix->Translate(&objectOrigin);
}

// Unused, stripped
void Tools_AlignDirectionToGround(Vector* pVec, Vector* pVec1) {
    Vector perp;
    perp.Cross(pVec, pVec1);
    pVec1->Cross(&perp, pVec);
    pVec1->Normalise(pVec1);
}

void Tools_CollisionBounce(Vector* pVec, Vector* pVec1, CollisionResult* pCr, float f1) {
    Vector reaction;
    Vector extra;

    extra.Add(pVec, pVec1);
    extra.Subtract(&pCr->pos);
    
    reaction.Scale(&pCr->normal, -extra.Dot(&pCr->normal) * (f1 + 1.0f));

    extra.Add(&reaction);

    pVec->Add(&pCr->pos, &extra);

    reaction.Scale(&pCr->normal, -pVec1->Dot(&pCr->normal) * (f1 + 1.0f));

    pVec1->Add(&reaction);
}

float Tools_SCurve(float x) {
    return (_table_cosf(x * PI) - 1.0f) * -0.5f;
}

extern "C" double atan2(double, double);

void Tools_GetPYR(Matrix* pRotMatrix, float* pPitch, float* pYaw, float* pRoll) {
    Matrix tempm = *pRotMatrix;
    float data00 = tempm.data[0][0];
    float f1;
    if (tempm.data[0][0] < 0.0f) {
        f1 = -tempm.data[0][0];
    } else {
        f1 = tempm.data[0][0];
    }

    if (f1 < 0.001f && Abs(tempm.data[0][1]) < 0.001f) {
        *pRoll = 0.0f;
    } else if (tempm.data[0][0] > 0.0f) {
        *pRoll = atan2(-tempm.data[0][1], tempm.data[0][0]);
    } else if (tempm.data[0][0] < 0.0f) {
        *pRoll = atan2(tempm.data[0][1], -tempm.data[0][0]);
    }
    
    tempm.RotateRoll(&tempm, -*pRoll);

    *pYaw = atan2(tempm.data[0][2], tempm.data[0][0]);
    tempm.RotateYaw(&tempm, -*pYaw);

    *pPitch = atan2(tempm.data[2][1], tempm.data[2][2]);
}

// Does nothing
void DebugInfo_Line3d(char*, Vector*, Vector*, int, bool) {}

float GetFloor(Vector* pPos, float arg1, CollisionResult* pCr) {
    Vector start;
    Vector end;
    
    float collideY = 10000.0f;

    start.Set(pPos->x, pPos->y + arg1, pPos->z);
    end.Set(pPos->x, pPos->y - 100000.0f, pPos->z);
    
    // if pCr is not NULL, use the pCr argument otherwise use a local CollisionResult
    if (pCr) {
        // Only collide with ground
        if (Collision_RayCollide(&start, &end, pCr, COLLISION_MODE_POLY, 0)) {
            collideY = pCr->pos.y;
        }
    } else {
        CollisionResult cr;
        // Only collide with ground
        if (Collision_RayCollide(&start, &end, &cr, COLLISION_MODE_POLY, 0)) {
            collideY = cr.pos.y;
        }
    }

    return collideY;
}

float Tools_GetCollideHeight(Vector* pVec, Vector* pVec1, bool* pOut, float f1) {
    CollisionResult cr;
    Vector top;
    Vector end;

    if (pOut) {
        *pOut = false;
    }

    float f13 = pVec1->y / f1;
    if (f13 < 0.0f) {
        f13 = 0.0f;
    }

    top.y = pVec->y + (pVec1->y * f13 - f1 * 0.5f * f13 * f13);
    top.x = pVec->x + (f13 * pVec1->x);
    top.z = pVec->z + (f13 * pVec1->z);

    f13 += 60.0f;

    end.y = pVec->y + (pVec1->y * f13 - f1 * 0.5f * f13 * f13);
    end.x = pVec->x + (f13 * pVec1->x);
    end.z = pVec->z + (f13 * pVec1->z);

    end.Sub(&end, &top);
    end.Scale(4.0f);
    end.Add(&top);
    
    // Only check for ground collisions
    if (Collision_RayCollide(&top, &end, &cr, COLLISION_MODE_POLY, 0)) {
        if (pOut && (cr.collisionFlags & ID_WATER_BLUE)) {
            // collision against water
            *pOut = true;
        }
        
        if (cr.normal.y > 0.5f) {
            return cr.pos.y;
        }
    }

    return -1e+06f;
}

void Tools_DrawOverlay(
    Material* pMaterial, 
    Vector* pColor, 
    float x, float y, 
    float uv0, float uv1, 
    float f5, float f6, 
    float f7, float f8, 
    float endX, float endY, 
    float uv2, float uv3, 
    float f13, float f14, 
    float f15, float f16
) {
    Blitter_Image bImage;

    if (pMaterial) {
        pMaterial->Use();
    }

    bImage.startX = x;
    bImage.endX = endX;

    bImage.startY = y;
    bImage.endY = endY;

    bImage.z = 0.0f;

    bImage.uv0 = uv0;
    bImage.uv2 = uv2;
    bImage.uv1 = uv1;
    bImage.uv3 = uv3;

    bImage.color = *pColor;

    bImage.unk40[0] = 1.0f;
    bImage.unk40[1] = 1.0f;
    bImage.unk40[2] = 1.0f;
    bImage.unk40[3] = 1.0f;

    bImage.Draw(1);
}

// Unused
void Tools_RenderToTarget(Material* pMat, float, float, float, float, float, float, float) {}
/*
// July 1st function
void Tools_RenderToTarget(Material* pMaterial, float f0, float fv0f, float fa1, float fa2, float fa3, float fa4, float fa5) {
    Blitter_Image image;
    image.startX = 0.0f;
    image.startY = 0.0f;
    image.endX = 640.0f;
    image.endY = 512.0f;
    image.z = 0.0f;
    image.uv0 = fa2 + f0;
    image.uv2 = fa3 + f0;
    image.uv1 = fa4 + fv0f;
    image.uv3 = fa5 + fv0f;
    
    image.unk40.x = 1.0f;
    image.unk40.y = 1.0f;
    image.unk40.z = 1.0f;
    image.unk40.w = 1.0f;
    image.color.Set(1.0f, 1.0f, 1.0f);
    image.color.w = fa1;
    pMaterial->Use();
    image.Draw(1);
}
*/

// UNUSED
float Extra_WobbleFunction(float f1) {
    // this isn't what the code was, but it orders floats correct so
    return (int)f1;
}

float Tools_Wobble(float f1, int r3) {
    float f0;
    float f4 = f1 - (int)f1;
    float f5;
    float f6;
    float f7;
    if (f4 < 0.5f) {
        int r7 = (int)f1 + r3;
        f5 = randomFloats[(r7 + 31) & 31];
        f6 = randomFloats[r7 & 31];
        f7 = randomFloats[(r7 + 1) & 31];
        f0 = randomFloats[(r7 + 2) & 31];
    } else {
        int r7 = (int)f1 + r3;
        f0 = randomFloats[(r7 + 31) & 31];
        f7 = randomFloats[r7 & 31];
        f6 = randomFloats[(r7 + 1) & 31];
        f5 = randomFloats[(r7 + 2) & 31];
        f4 = 1.0f - f4;
    }
    
    float t_f5 = f7 - f5;
    float x = ((((3.5f * (f7 - f6)) - ((f0 - f5) * 0.5f)) - t_f5));
    return ((x * f4) * f4) + ((f6 + f6) + (t_f5 * f4));
}

int Round(float x) {
    int rounded = (int)x;
    if (x - (float)rounded > 0.5f) {
        return rounded + 1;
    }

    return rounded;
}

extern "C" double acos(double);
float GetPitch2Points(Vector* pVec, Vector* pVec1) {
    float dx = pVec->x - pVec1->x;
    float dy = pVec->y - pVec1->y;
    float dz = pVec->z - pVec1->z;

    if (!dy) {
        return 0.0f;
    }

    float f1 = ExactMag(dx, dy, dz);
    if (f1) {
        return (float)acos(Clamp<float>(-1.0f, dy / f1, 1.0f)) - PI2;
    }

    return 0.0f;
}

// Approximates the magnitude of the vector (pVector1 - pVector) using Heron's Method
// of approximating a square root
float ApproxMag(Vector* pVector, Vector* pVector1) {
    float x = pVector1->x - pVector->x;
    float y = pVector1->y - pVector->y;
    float z = pVector1->z - pVector->z;
    // absolute value
    *(int*)&x &= ~0x80000000;
    *(int*)&y &= ~0x80000000;
    *(int*)&z &= ~0x80000000;
    float guess = x + y + z; // initial guess
    if (guess > 0.0f) {
        float sq_mag = x * x + y * y + z * z;
        // 2 iterations
        guess = ((sq_mag / guess) + guess) * 0.5f;
        guess = ((sq_mag / guess) + guess) * 0.5f;
    }
    return guess;
}

// Approximates the magnitude of the vector (x, y, z) using Heron's Method
// of approximating a square root
float ApproxMag(float x, float y, float z) {
    // absolute value
    *(int*)&x &= ~0x80000000;
    *(int*)&y &= ~0x80000000;
    *(int*)&z &= ~0x80000000;
    float guess = x + y + z; // initial guess
    if (guess > 0.0f) {
        float sq_mag = x * x + y * y + z * z;
        // 2 iterations
        guess = ((sq_mag / guess) + guess) * 0.5f;
        guess = ((sq_mag / guess) + guess) * 0.5f;
    }
    return guess;
}

// Approximates the magnitude of the 2D vector (pVector1 - pVector) (x, z) using Heron's Method
// of approximating a square root
float ApproxMagXZ(Vector* pVector, Vector* pVector1) {
    float x = pVector1->x - pVector->x;
    float z = pVector1->z - pVector->z;
    // absolute value
    *(int*)&x &= ~0x80000000;
    *(int*)&z &= ~0x80000000;
    float guess = x + z; // initial guess
    if (guess > 0.0f) {
        float sq_mag = x * x + z * z;
        // 2 iterations
        guess = ((sq_mag / guess) + guess) * 0.5f;
        guess = ((sq_mag / guess) + guess) * 0.5f;
    }
    return guess;
}

// Approximates the magnitude of the 2D vector (x, z) using Heron's Method
// of approximating a square root
float ApproxMag(float x, float y) {
    // absolute value
    *(int*)&x &= ~0x80000000;
    *(int*)&y &= ~0x80000000;
    float guess = x + y; // initial guess
    if (guess > 0.0f) {
        float sq_mag = x * x + y * y;
        // 2 iterations
        guess = ((sq_mag / guess) + guess) * 0.5f;
        guess = ((sq_mag / guess) + guess) * 0.5f;
    }
    return guess;
}

// Calculates exact magnitude of (pVector - pVector1)
float ExactMag(const Vector* pVector, const Vector* pVector1) {
    float x = pVector->x - pVector1->x;
    float y = pVector->y - pVector1->y;
    float z = pVector->z - pVector1->z;
    return sqrtf(x * x + y * y + z * z);
}

// Calculates Exact Magnitude of the Vector (x, y, z)
float ExactMag(float x, float y, float z) {
    return sqrtf(x * x + y * y + z * z);
}

// Calculates Exact Magnitude of XZ components of (pVector - pVector1)
float ExactMagXZ(Vector* pVector, Vector* pVector1) {
    float x = pVector->x - pVector1->x;
    float z = pVector->z - pVector1->z;
    return sqrtf(x * x + z * z);
}

// Calculates Exact Magnitude of X and Z
float ExactMag(float x, float z) {
    return sqrtf(x * x + z * z);
}

void Tools_Dampen(Vector* arg0, Vector* arg1, float f1) {
    Vector damping;
    Vector force;
    
    damping.Scale(arg0, 2.0f * f1);
    force.Scale(arg1, f1 * f1);
    arg0->Add(&force);
    arg0->Subtract(&damping);
}

// about line 1118 on debug build (has quite a few more functions though)
float Tools_CriticalDamp(float f1, float f2, float f3, float strength) {
    return ((Sqr<float>(strength) * (f2 - f1)) + f3) - ((f3 * 2.0f) * (strength));
}

bool Tools_BuildLTWMatrix(Matrix* m, Vector* forward, Vector* pVec) {
    static Vector j = {0.0f, 1.0f, 0.0f, 0.0f};

    Vector* s = (pVec != NULL) ? pVec : &j;

    Vector tmp;
    tmp.Inverse(forward);
    tmp.Cross(&tmp, s);

    if (tmp.MagSquared() == 0.0f) {
        return false;
    }

    m->SetRotationToNone();

    m->Row0()->Copy(&tmp);
    m->Row0()->Normalise();
    m->Row1()->Copy(s);
    m->Row2()->Cross(s, m->Row0());
    
    return true;
}

bool Tools_BuildMatrixFromFwd(Matrix* pM, Vector* pFwd, Vector* pUp) {
    static Vector up = {0.0f, 1.0f, 0.0f, 0.0f};

    if (pUp == NULL) {
        pUp = &up;
    }

    Vector tmp = *pFwd;
    Vector cross;
    tmp.Inverse(&tmp);
    tmp.Normalise();

    cross.Cross(&tmp, pUp);

    if (cross.MagSquared() == 0.0f) {
        return false;
    }

    cross.Normalise();

    pM->SetRotationToNone();
    pM->Row0()->Copy(&cross);
    pM->Row2()->Copy(&tmp);
    pM->Row1()->Cross(&cross, &tmp);

    return true;
}

// Returns if a collision occurs under pt
bool Tools_TestFloor(Vector* pt, CollisionResult* pCr, float f1, bool bCollisionMode) {
    CollisionResult localCR;
    pCr = pCr ? pCr : &localCR;
    Vector end = {pt->x, pt->y - f1, pt->z};
    return Collision_RayCollide(pt, &end, pCr,
        bCollisionMode ? COLLISION_MODE_ALL : COLLISION_MODE_POLY, 0);
}

// Returns the y coordinate of the floor under pPos
// if a collision does not occur, the input y position is returned
float Tools_GetFloor(const Vector& pPos, CollisionResult* pCr, float maxRayDist, bool bCollisionMode, uint collisionFlags) {
    CollisionResult localCR;
    pCr = pCr ? pCr : &localCR;
    Vector end = {pPos.x, pPos.y - maxRayDist, pPos.z};
    if (Collision_RayCollide((Vector*)&pPos, &end, pCr, 
            bCollisionMode ? COLLISION_MODE_ALL : COLLISION_MODE_POLY, collisionFlags)) {
        return pCr->pos.y;
    }
    return pPos.y;
}

bool RayToSphere(Vector* pVec, Vector* pVec1, Vector* pVec2, float radius, float f2, bool r6) {
    if (f2 == -1.0f) {
        f2 = radius;
    }

    if (r6 && (pVec2->IsInsideSphere(pVec, radius) || pVec2->IsInsideSphere(pVec1, f2))) {
        return true;
    }

    Vector tmp;
    Vector tmp2;
    tmp2.Sub(pVec1, pVec);
    tmp.Sub(pVec2, pVec);

    float tmp_dot_tmp2 = tmp.Dot(&tmp2);

    float f5 = tmp2.MagSquared();
    if (!f5) {
        return false;
    }

    float t = tmp_dot_tmp2 / f5;
    if (t >= 0.0f && t <= 1.0f) {
        Vector interpolated;
        interpolated.InterpolateLinear(pVec, pVec1, t);
        if (interpolated.IsInsideSphere(pVec2, radius + ((f2 - radius) * t))) {
            return true;
        }
    }

    return false;
}

bool Tools_ClipSphereToDynamicModel(const Vector& vec, float f1, Vector* pVec1, Model* pModel, int subobjectIdx) {
    Matrix* pLTW = (subobjectIdx > -1) ? 
        &pModel->pMatrices[pModel->GetSubObjectMatrixIndex(subobjectIdx)] :
        pModel->matrices;

    BoundingVolume* pVolume = pModel->GetBoundingVolume(subobjectIdx);

    Matrix inverseLTW;
    inverseLTW.Inverse(pLTW);

    Vector localPos;
    localPos.ApplyMatrix((Vector*)&vec, &inverseLTW);

    Vector boxMax;
    boxMax.Add(&pVolume->v1, &pVolume->v2);

    Vector radSca = {0.0f, 0.0f, 0.0f, 1.0f};
    radSca.x = f1 / pLTW->Row0()->Magnitude();
    radSca.y = f1 / pLTW->Row1()->Magnitude();
    radSca.z = f1 / pLTW->Row2()->Magnitude();

    if ((localPos.x + radSca.x >= pVolume->v1.x && localPos.x - radSca.x < boxMax.x) &&
        (localPos.y + radSca.y >= pVolume->v1.y && localPos.y - radSca.y < boxMax.y) &&
        (localPos.z + radSca.z >= pVolume->v1.z && localPos.z - radSca.z < boxMax.z)) {
        if (pVec1) {
            Vector d = {};
            if (localPos.x > boxMax.x * 0.5f) {
                d.x = (boxMax.x - localPos.x) + (radSca.x + 0.001f);
            } else {
                d.x = (pVolume->v1.x - localPos.x) - (radSca.x + 0.001f);
            }

            if (localPos.y > boxMax.y * 0.5f) {
                d.y = (boxMax.y - localPos.y) + (radSca.y + 0.001f);
            } else {
                d.y = (pVolume->v1.y - localPos.y) - (radSca.y + 0.001f);
            }

            if (localPos.z > boxMax.z * 0.5f) {
                d.z = (boxMax.z - localPos.z) + (radSca.z + 0.001f);
            } else {
                d.z = (pVolume->v1.z - localPos.z) - (radSca.z + 0.001f);
            }

            if (Abs<float>(d.z) > Abs<float>(d.x)) {
                if (Abs<float>(d.y) > Abs<float>(d.x)) {
                    d.y = 0.0f;
                } else {
                    d.x = 0.0f;
                }

                d.z = 0.0f;
            } else {
                if (Abs<float>(d.y) > Abs<float>(d.z)) {
                    d.y = 0.0f;
                } else {
                    d.z = 0.0f;
                }

                d.x = 0.0f;
            }

            pVec1->Add(&d, &localPos);
            pVec1->ApplyMatrix(pLTW);
        }

        return true;
    }

    return false;
}

// Inlined multiple times throughout the binary
// should be a class method based on calling convention?
inline float Vector_MagSquared(Vector* p) {
    return p->x * p->x + p->y * p->y + p->z * p->z;
}

bool Tools_CapsuleTest(Vector* pVec, Vector* pVec1, Vector* pVec2, float f1, bool r6) {
    // pVec1 and pVec2 are the ends of the capsule?
    // r6 is a bool to determine if the capsule ends should be checked as spheres
    if (r6 && (pVec->IsInsideSphere(pVec1, f1) || pVec->IsInsideSphere(pVec2, f1))) {
        return true;
    }

    Vector toPoint2;
    Vector toSphere;

    toPoint2.Sub(pVec2, pVec1);
    toSphere.Sub(pVec, pVec1);

    float dot = toSphere.Dot(&toPoint2);

    float mag = Vector_MagSquared(&toPoint2);

    if (!mag) {
        return false;
    }
    
    float t = dot / mag;
    if (t >= 0.0f && t <= 1.0f) {
        Vector temp;
        temp.InterpolateLinear(pVec1, pVec2, t);
        if (temp.IsInsideSphere(pVec, f1)) {
            return true;
        }
    }

    return false;
}

bool Tools_RayToVertCyl(Vector* pRayStart, Vector* pRayEnd, Vector* pCylPos, float f1, float f2) {
    // assertions at 0x5CC, 0x5CD, 0x5CE
    Vector tmp;
    Vector tmp2;
    tmp.Sub(pRayEnd, pRayStart);
    tmp2.Sub(pCylPos, pRayStart);
    float t = tmp.MagSquared();
    t = tmp2.Dot(&tmp) / (t != 0.0f ? t : 1.0f);
    Vector interpolated;
    interpolated.InterpolateLinear(pRayStart, pRayEnd, Clamp<float>(0.0f, t, 1.0f));
    if (Dist2D(&interpolated, pCylPos) < Sqr<float>(f1)) {
        return interpolated.y > pCylPos->y && interpolated.y < pCylPos->y + f2;
    }
    return false;
}

/// @brief Tests for a intersection of a plane by the line formed by (pStart, pEnd)
/// @param pStart Ray Start
/// @param pEnd Ray End
/// @param pNormal Plane Vector
/// @param pCr Optional, CollisionResult to store collision info to
/// @return True when the line intersected the plane, otherwise false
bool Tools_PlaneTest(Vector* pStart, Vector* pEnd, Vector* pNormal, CollisionResult* pCr) {
    Vector ray;
    ray.Sub(pEnd, pStart);
    
    float f5 = pNormal->Dot(&ray);
    if (f5 < 0.0f) {
        return false;
    }

    float t = (pNormal->Dot(pStart) - pNormal->w) / f5;
    if (t >= 0.0f && t <= 1.0f) {
        // Ray intersects plane
        if (pCr) {
            // Store collision information if a CollisionResult was poseed
            pCr->pos.InterpolateLinear(pStart, pEnd, t);
            pCr->unk40 = t;
            pCr->normal = *pNormal;
        }

        // Intersection did occur
        return true;
    }

    // Plane Intersection did not occur
    return false;
}

bool Tools_SweepSphereToPlane(Vector* pStart, Vector* pEnd, float radius, Vector* pPlane, CollisionResult* pCr) {
    Vector ray;
    Vector spherePos;
    Vector sphereRayEnd;

    ray.Sub(pEnd, pStart);

    if (ray.Dot(pPlane) < 0.0f) {
        return false;
    }

    float f5 = pPlane->Dot(pStart) - pPlane->w;
    if (f5 < radius) {
        // inside the sphere?
        if (pCr) {
            // If a CollisionResult was passed, store the collision information
            pCr->unk40 = 0.0f;
            pCr->normal = *pPlane;
            pCr->pos.Scale(pPlane, -f5);
            pCr->pos.Add(pStart);
        }

        return true;
    }

    spherePos.Scale(pPlane, -radius);
    spherePos.Add(pStart);
    sphereRayEnd.Add(&spherePos, &ray);

    return Tools_PlaneTest(&spherePos, &sphereRayEnd, pPlane, pCr);
}

/// @brief Creates a plane vector from a Normal and a Position
/// @param pNormal Normal Vector of the plane
/// @param pPos Position Vector of the plane
/// @return Plane Vector
Vector Tools_MakePlane(Vector* pNormal, Vector* pPos) {
    // Creates a plane vector of ax + by + cz = d
    Vector res = *pNormal;
    res.w = pNormal->Dot(pPos);
    return res;
}

bool Tools_CapsuleTestMagSq(Vector* pVec, Vector* pVec1, Vector* pVec2, float f1) {
    if (SquareDistance(pVec, pVec1) < f1 || SquareDistance(pVec, pVec2) < f1) {
        return true;
    }

    Vector toPoint2;
    Vector toSphere;

    toPoint2.Sub(pVec2, pVec1);
    toSphere.Sub(pVec, pVec1);

    float dot = toSphere.Dot(&toPoint2);

    float mag = Vector_MagSquared(&toPoint2); // figure this out

    if (!mag) {
        return false;
    }
    
    float t = dot / mag;
    if (t >= 0.0f && t <= 1.0f) {
        Vector temp;
        temp.InterpolateLinear(pVec1, pVec2, t);
        if (SquareDistance(&temp, pVec) <= f1) {
            return true;
        }
    }

    return false;
}

void Tools_AnimDesc::Init(Tools_AnimEntry* _pAnims, int _count, MKAnimScript* pAnimScript) {
    pEntries = _pAnims;
    unk4 = _count;
    for (int i = 0; i < unk4; i++) {
        pEntries[i].pAnim = pAnimScript->GetAnim(pEntries[i].pName);
    }
}

void Tools_AnimManager::Init(Tools_AnimDesc* pAnimDesc) {
    pDescriptor = pAnimDesc;
}

void Tools_AnimEventDesc::Init(Tools_AnimEvent* pAnimEvent, int count, MKAnimScript* pAnimScript) {
    mCount = count;
    pEvents = pAnimEvent;
    for (int i = 0; i < mCount; i++) {
        pEvents[i].pEventName = pAnimScript->GetEventByName(pEvents[i].pName);
    }
}

void Tools_AnimEventManager::Init(Tools_AnimEventDesc* pAnimEventDesc) {
    pDescriptor = pAnimEventDesc;
    unk8 = 0;
    unk4 = 0;
}

void Tools_AnimEventManager::Update(MKAnimScript* pAnimScript) {
    // this is different in the July 1st build
    /*  
        unk4 = 0;
        // no change of unk4 to unk8
    */
    unk8 = unk4;
    unk4 = 0;
    int index = 0;
    if (!pDescriptor || !pDescriptor->pEvents) {
        return;
    }
    
    char* name = (char*)pDescriptor;
    while (name = pAnimScript->GetEvent(index++)) {
        for (int i = 0; i < pDescriptor->mCount; i++) {
            if (name == pDescriptor->pEvents[i].pEventName) {
                unk4 |= 1 << i;
            }
        }
    }
}

int Tools_GetAnimationLength(MKAnimScript* pAnimScript) {
    if (pAnimScript != NULL) {
        return pAnimScript->UpdatesUntilFinished() + 2;
    }

    return 0;
}

void Tools_ParticleRef::Init(Model* pModel, char* pRefPointName) {
    refPointIdx = pModel->GetRefPointIndex(Str_Printf("%s_origin", pRefPointName));
    pModel->GetRefPointWorldPosition(refPointIdx, &refPointWorldPos);
    targetPointIdx = pModel->GetRefPointIndex(Str_Printf("%s_target", pRefPointName));
    pModel->GetRefPointWorldPosition(targetPointIdx, &targetWorldPos);
    targetWorldPos.Sub(&targetWorldPos, &refPointWorldPos);
    targetWorldPos.Normalise();
}

void Tools_ParticleRef::Update(Model* pModel) {
    pModel->GetRefPointWorldPosition(refPointIdx, &refPointWorldPos);
    pModel->GetRefPointWorldPosition(targetPointIdx, &targetWorldPos);
    targetWorldPos.Sub(&targetWorldPos, &refPointWorldPos);
    targetWorldPos.Normalise();
}

void Tools_DropShadow_Init(void) {
    if (bDropShadowsIsInit) {
        return;
    }

    bDropShadowsIsInit = true;
    shadows.Init(128);
}

void Tools_DropShadow_Deinit(void) {
    if (!bDropShadowsIsInit) {
        return;
    }

    shadows.Deinit();
    bDropShadowsIsInit = false;
}

void Tools_DropShadow_Add(float f1, Vector* pPos, Vector* pNormal, float alpha) {
    if (!bDropShadowsIsInit) {
        return;
    }

    if (shadows.full()) {
        return;
    }

    ShadowInfo* pShadow = shadows.GetNextEntry();

    pShadow->alpha = alpha;
    pShadow->unk20 = f1;
    pShadow->pos.Copy(pPos);
    pShadow->unk10.Copy(pNormal);
}

// may have been defined within the header?
// might be a different header?
// need the inline qualifer so it generates as weak after Tools_DropShadow_Draw 
inline void ShadowInfo::Draw(void) {
    Matrix transform;
    Vector sp18 = {0.0f, 0.0f, 1.0f, 0.0f};

    transform.SetIdentity();
    Tools_BuildLTWMatrix(&transform, &sp18, &unk10);

    transform.Row3()->Copy(&pos);
    transform.Row3()->y += 3.0f;

    View::GetCurrent()->SetLocalToWorldMatrix(&transform);

    Blitter_TriStrip triStrips[4];
    triStrips[0].pos.Set(-unk20 / 2.0f, 0.0f, unk20 / 2.0f);
    triStrips[1].pos.Set(unk20 / 2.0f, 0.0f, unk20 / 2.0f);
    triStrips[2].pos.Set(-unk20 / 2.0f, 0.0f, -unk20 / 2.0f);
    triStrips[3].pos.Set(unk20 / 2.0f, 0.0f, -unk20 / 2.0f);
    
    triStrips[0].color.Set(1.0f, 1.0f, 1.0f, alpha);
    triStrips[3].color = triStrips[2].color = triStrips[1].color = triStrips[0].color;
    
    triStrips[0].uv.x = 0.0f;
    triStrips[0].uv.y = 1.0f;
    
    triStrips[1].uv.x = 1.0f;
    triStrips[1].uv.y = 1.0f;
    
    triStrips[2].uv.x = 0.0f;
    triStrips[2].uv.y = 0.0f;
    
    triStrips[3].uv.x = 1.0f;
    triStrips[3].uv.y = 0.0f;
    
    triStrips[0].Draw(ARRAY_SIZE(triStrips), 1.0f);
}

void Tools_DropShadow_Draw(void) {
    if (shadows.empty()) {
        return;
    }

    gb.pShadowMat->Use();

    ShadowInfo* pInfo = shadows.GetCurrEntry();
    for (int i = 0; i < shadows.size(); i++) {
        pInfo[i].Draw();
    }
}

void Tools_DropShadow_Update(void) {
    shadows.UnknownSetPointer();
}

void Tools_DrawDropShadow(Material* pMat, Vector* pColor /* Optional, pass NULL if unneeded */, float f1,
        float x, float y, float z, float f5, float f6, float f7, float angle, bool uvSetting) {
    
    Vector testPos = {x, y, z, 0.0f};
    Matrix shadowMatrix;
    
    if (!Range_IsVisible(&testPos)) {
        return;
    }

    if (Range_WhichZone(&testPos, NULL) >= 2) {
        return;
    }

    pMat->Use();

    shadowMatrix.SetIdentity();

    shadowMatrix.Row2()->Set(_table_cosf(angle), 0.0f, _table_sinf(angle));
    shadowMatrix.Row1()->Set(f5, f6, f7);
    shadowMatrix.Row0()->Cross(shadowMatrix.Row2(), shadowMatrix.Row1());
    shadowMatrix.Row0()->Normalise();
    shadowMatrix.Row2()->Cross(shadowMatrix.Row1(), shadowMatrix.Row0());
    shadowMatrix.Row3()->Set(x, y, z);

    View::GetCurrent()->SetLocalToWorldMatrix(&shadowMatrix);

    Blitter_TriStrip vertices[4];
    vertices[0].pos.Set(-f1 / 2.0f, 0.0f, f1 / 2.0f);
    vertices[1].pos.Set(f1 / 2.0f, 0.0f, f1 / 2.0f);
    vertices[2].pos.Set(-f1 / 2.0f, 0.0f, -f1 / 2.0f);
    vertices[3].pos.Set(f1 / 2.0f, 0.0f, -f1 / 2.0f);

    if (pColor) {
        vertices[0].color = *pColor;
        vertices[3].color = vertices[2].color = vertices[1].color = vertices[0].color;
    } else {
        vertices[0].color.Set(1.0f, 1.0f, 1.0f, 1.0f);
        vertices[3].color = vertices[2].color = vertices[1].color = vertices[0].color;
    }
    
    if (uvSetting) {
        vertices[0].uv.x = 0.0f;
        vertices[0].uv.y = 0.0f;
        
        vertices[1].uv.x = 1.0f;
        vertices[1].uv.y = 0.0f;
        
        vertices[2].uv.x = 0.0f;
        vertices[2].uv.y = 1.0f;
        
        vertices[3].uv.x = 1.0f;
        vertices[3].uv.y = 1.0f;
    } else {
        vertices[0].uv.x = 0.0f;
        vertices[0].uv.y = 1.0f;
        
        vertices[1].uv.x = 1.0f;
        vertices[1].uv.y = 1.0f;
        
        vertices[2].uv.x = 0.0f;
        vertices[2].uv.y = 0.0f;
        
        vertices[3].uv.x = 1.0f;
        vertices[3].uv.y = 0.0f;
    }

    vertices[0].Draw(ARRAY_SIZE(vertices), 1.0f);
}


bool LoadLevel_LoadVector(KromeIniLine*, char*, Vector*);

/// @brief Loads data from a KromeIniLine for WayPointLoadInfo
/// @param pLine Line to load from
/// @param bLoadPos Whether to test for the "pos" key or not
/// @return 
bool WayPointLoadInfo::LoadLine(KromeIniLine* pLine, bool bLoadPos) {
    if (stricmp(pLine->pFieldName, "waypoints") == 0) {
        return true;
    }
    
    if (bLoadPos) {
        if (LoadLevel_LoadVector(pLine, "pos", &vecs[unk0]) || LoadLevel_LoadVector(pLine, "waypoint", &vecs[unk0])) {
            unk0++;
            return true;
        }
    } else {
        if (LoadLevel_LoadVector(pLine, "waypoint", &vecs[unk0])) {
            unk0++;
            return true;
        }
    }

    return false;
}

void Tools_WayPoints::Init(void) {
    unk100 = 0;
    unk104 = 0;
}

bool Tools_WayPoints::Load(WayPointLoadInfo* pLoadInfo, Tools_WayPoints::LoadMode loadMode) {
    Init(); // may not be inlined here?

    while (unk104 < MAX_WAYPOINTS && unk104 < pLoadInfo->unk0) {
        vecs[unk104] = pLoadInfo->vecs[unk104];
        
        if (loadMode == 0 && unk104) {
            if (Abs<float>(vecs[unk104].y - vecs[unk104 - 1].y) > 1.0f) {
                vecs[unk104].y = vecs[unk104 - 1].y;
            }
        }
        
        unk104++;
    }

    return unk104 > 0 ? true : false;
}

bool Tools_WayPoints::LoadLine(KromeIniLine* pLine, Tools_WayPoints::LoadMode loadMode) {
    vecs[unk104].w = 1.0f;

    if (LoadLevel_LoadVector(pLine, "pos", &vecs[unk104]) || LoadLevel_LoadVector(pLine, "waypoint", &vecs[unk104])) {
        if (loadMode == 0 && unk104 > 0) {
            vecs[unk104].y = vecs[unk104 - 1].y;
        }

        unk104++;
        return true;
    }

    return false;
}

// Unused, stripped function
void Tools_WayPoints::Reset(void) {
    unk100 = 0;
}

// returns the node index of the model's animation if it exists otherwise returns -1
int Tools_GetAnimationNode(Model* pModel, char* pName) {
    int nodeIdx = -1;
    if (pModel->pAnimation && pModel->pAnimation->NodeExists(pName, &nodeIdx)) {
        return nodeIdx;
    } else {
        return -1;
    }
}

void Tools_SetNode(Animation* pAnimation, int idx, Matrix* pMatrix, Tools_SetNodeFlag eNodeFlag) {
    switch (eNodeFlag) {
        case TOOLS_NODEFLAG_0:
            if (pAnimation) {
                pAnimation->SetNodeMatrix(idx, NULL, false);
            }
            break;
        case TOOLS_NODEFLAG_1:
            if (pAnimation && pMatrix) {
                pAnimation->SetNodeMatrix(idx, pMatrix, false);
            }
            break;
        case TOOLS_NODEFLAG_2:
            if (pAnimation && pMatrix) {
                pAnimation->SetNodeMatrix(idx, pMatrix, true);
            }
            break;
    }
}

void Tools_MatriceRotate(float f1, Vector* pRot, Matrix* pMatrix) {
    QuatRotation quat;
    AxisRotation axisRot;
    axisRot.rot.Set(pRot->x, pRot->y, pRot->z);
    axisRot.unk10 = -f1;
    quat.ConvertRotation(&axisRot);
    pMatrix->RotateQ(pMatrix, &quat);
}

Vector* Tools_GetRefPointPos(Model* pModel, char* pRefName) {
    int refPointIdx = pModel->GetRefPointIndex(pRefName);
    return pModel->GetRefPointOrigin(refPointIdx);
}

void Tools_ApplyFrictionAndGravity(Vector* pVec, Vector* pVec1, Vector* pVec2, float f1) {
    Vector force = *pVec1;
    Vector r;

    if (pVec->MagSquared() > Sqr<float>(0.25f)) {
        f1 = f1 / 2.0f;
    }

    r.Scale(pVec2, -pVec1->Dot(pVec2));

    force.Add(pVec1, &r);

    float f30 = f1 * r.Magnitude();
    float f5 = (force.Magnitude() == 0.0f) ? 0.0f : force.Normalise() - f30;

    force.Scale(Max<float>(f5, 0.0f));

    pVec->Scale(1.0f - (f1 / 2.0f));

    pVec->Add(&force);
}

// Generates a Random Radial Vector X/Z Components
void Tools_RandomRadialVectorXZ(Vector& pOut) {
    // Get random angle in the range of 0, 2PI
    float angle = RandomFR(&gb.mRandSeed, 0.0f, 2 * PI);
    pOut.x = _table_sinf(angle);
    pOut.y = 0.0f;
    pOut.z = -_table_cosf(angle);
}

Vector* Tools_RandomNormal(Vector* pOut) {
    Tools_RandomRadialVectorXZ(*pOut);
    float randomy = RandomFR(&gb.mRandSeed, -1.0f, 1.0f);
    pOut->Scale(sqrtf(1.0f - Sqr<float>(randomy)));
    // maybe this isn't meant to return a random normalized vector?
    pOut->y = randomy; // possibly should be stored before normalizing?
    return pOut;
}

float Tools_TurnToAngle(float currentAngle, float maxAngle, float maxTurnRate) {

    float angleDiff = GetSmallestAngle(currentAngle, maxAngle);

    float finalAngle;

    if (Abs<float>(angleDiff) <= maxTurnRate) {
        finalAngle = maxAngle;
    } else if (angleDiff > PI || angleDiff < 0.0f) {
        finalAngle = currentAngle - maxTurnRate;
    } else {
        finalAngle = currentAngle + maxTurnRate;
    }

    return Simple_NormaliseAngle(finalAngle);
}

// Tools_TurnToTarget

float Tools_SmoothToValue2(float f1, float f2, float f3, float f4, float f5) {
    float f6 = Abs<float>(f1 - f2);

    float f0;

    f0 = f6 * f3;
    if (f0 < f4) {
        f0 = f4;
    } else if (f0 > f5) {
        f0 = f5;
    }

    if (f6 > f0) {
        if (f1 > f2) {
            f2 = f1 - f0;
        } else {
            f2 = f1 + f0;
        }
    }

    return f2;
}

float Tools_SmoothToAngle2(float f1, float f2, float f3, float f4, float f5) {
    f1 = NormaliseAngle(f1);
    f2 = NormaliseAngle(f2);

    if (f1 < f2 - PI) {
        f1 += (2 * PI);
    } else if (f1 > f2 + PI) {
        f1 -= (2 * PI);
    }

    return NormaliseAngle(Tools_SmoothToValue2(f1, f2, f3, f4, f5));
}

Vector* Tools_RandomBox(Vector* pVector, float maxExtent) {
    pVector->x += RandomFR(&gb.mRandSeed, -maxExtent, maxExtent);
    pVector->y += RandomFR(&gb.mRandSeed, -maxExtent, maxExtent);
    pVector->z += RandomFR(&gb.mRandSeed, -maxExtent, maxExtent);
    return pVector;
}

// pMin is the minimum values, pMax is the maximum values
void Tools_RandomVector(Vector* pResult, Vector* pMin, Vector* pMax) {
    pResult->x = RandomFR(&gb.mRandSeed, pMin->x, pMax->x);
    pResult->y = RandomFR(&gb.mRandSeed, pMin->y, pMax->y);
    pResult->z = RandomFR(&gb.mRandSeed, pMin->z, pMax->z);
    pResult->w = 0.0f;
}

int Tools_GetTimeInSeconds(void) {
    TimerInfo currTime;
    Timer_GetSystemTime(&currTime);
    int seconds = currTime.month * MONTH_TO_SECONDS;
    seconds += currTime.day * DAYS_TO_SECONDS;
    seconds += currTime.hours * HOURS_TO_SECONDS;
    seconds += currTime.minutes * MINUTES_TO_SECONDS;
    seconds += currTime.seconds;
    return seconds;
}

void Tools_DynamicStringTable::Init(void) {
    pStringTable = NULL;
    currentLen = 0;
    nmbrOfStrings = 0;
}

extern "C" int strlen(char*);
// extern "C" void strcpy(char*, char*);
extern "C" void memcpy(void*, void*, int);
// extern "C" int stricmp(char*, char*);

// Adds a string to the end of the dynamic string table
// returns the current address of the string in the string table
char* Tools_DynamicStringTable::AppendString(char* pString) {
    int len = strlen(pString) + 1;
    char* str;
    // if pStringTable is NULL, it isn't initialized (struct contains no strings)
    if (!pStringTable) {
        currentLen = len;
        pStringTable = (char*)Heap_MemAlloc(currentLen);
        str = pStringTable;
        strcpy(str, pString);
    } else {
        // allocate new string table (oldLen + new string length)
        void* pNewTable = Heap_MemAlloc(currentLen + len);
        memcpy(pNewTable, (void*)pStringTable, currentLen);
        if (pStringTable) {
            // deallocate old string table
            Heap_MemFree((void*)pStringTable);
        }
        // set pStringTable to the new string table
        pStringTable = (char*)pNewTable;
        str = pStringTable + currentLen; // get index of new string
        currentLen += len; // update table length with the new string's length
        strcpy(str, pString);
    }

    nmbrOfStrings++;
    return str; // returns the string within the string table
}

// Searches the dynamic string table to find pString
// if not found, returns NULL
char* Tools_DynamicStringTable::FindString(char* pString) {
    char* pCurrStr = pStringTable;

    for (int i = 0; i < nmbrOfStrings; i++) {
        if (stricmp(pCurrStr, pString) == 0) {
            return pCurrStr;
        }

        pCurrStr += strlen(pCurrStr) + 1; // add length of current string + 1
    }

    return NULL;
}


void Tools_EnableWideScreen(View* pView, bool bEnableWideScreen) {
    pView->SetAspectRatio(bEnableWideScreen ? 0.75f : 1.0f, 1.0f);
}

/*
// Unused, Stripped function
bool Tools_SideOfLine2D(Vector* pVec, Vector* pVec1, Vector* pVec2) {
    return (pVec->x - pVec1->x) * (pVec2->z - pVec1->z) +
         (pVec->z - pVec1->z) * (pVec1->x - pVec2->x) < 0.0f;
}
*/

// // pVec is the bottom of the cylinder?
// // pVec1 is the top of the cylinder?
// float Tools_CylinderTest(Vector* pVec, Vector* pVec1, float radius, Vector* pTestPoint) {
//     float radSq = Sqr<float>(radius);
//     Vector tmp; // difference between pVec1 and pVec (pVec1 - pVec)
//     Vector tmp2;
//     tmp.Sub(pVec1, pVec);
//     tmp2.Sub(pTestPoint, pVec);
//     float f3 = tmp.MagSquared(); // distance between the 2 cylinder ends (pVec1, pVec)
//     float f1_ = tmp.Dot(&tmp2);
//     if (f1_ < 0.0f || f1_ > f3) {
//         return -1.0f;
//     }
//     float f0 = tmp2.MagSquared() - ((f1_ * f1_) / f3);
//     if (f0 > radSq) {
//         return -1.0f;
//     }
//     return f0;
// }

// pVec is the bottom of the cylinder?
// pVec1 is the top of the cylinder?
float Tools_CylinderTest(Vector* pVec, Vector* pVec1, float radius, Vector* pTestPoint) {
    float radSq = Sqr<float>(radius);
    Vector tmp; // difference between pVec1 and pVec (pVec1 - pVec)
    Vector tmp2;
    tmp.Sub(pVec1, pVec);
    tmp2.Sub(pTestPoint, pVec);
    float cylinderEndDist = tmp.MagSquared(); // distance between the 2 cylinder ends (pVec1, pVec)
    float f1_ = tmp.Dot(&tmp2);
    
    if (f1_ < 0.0f || f1_ > cylinderEndDist) {
        // cylinder end test?
        return -1.0f;
    }

    float f0 = tmp2.MagSquared() - ((f1_ * f1_) / cylinderEndDist);
    if (f0 > radSq) {
        return -1.0f;
    }

    return f0;
}


bool Tools_CapsuleTest(Vector* pVec, Vector* pVec1, float f1, float f2, Vector* pVec2) {
    return pVec->IsInsideSphere(pVec2, f1) || pVec1->IsInsideSphere(pVec2, f1) ||
        Tools_CylinderTest(pVec, pVec1, f2, pVec2) >= 0.0f;
}

// should this be defined here?
static PadKey padKeyMap[NUM_PADKEYS] = {
    {0x400,     0x7F}, {0x100,  0x80},
    {0x10,      0x81}, {0x20,   0x82},
    {0x40,      0x83}, {0x80,   0x84},
    {0x1000,    0x85}, {0x2000, 0x86},
    {0x4000,    0x87}, {0x8000, 0x88},
    {0x800,     0x89}, {0x200,  0x8A},
    {0x1,       0x8B}, {0x8,    0x8D},
    {0x2,       0x8E}, {0x4,    0x8F}
};
static int padKeyMapSize = NUM_PADKEYS;

int Tools_MapPadToKey(int target) {
    for (int i = 0; i < padKeyMapSize; i++) {
        if (padKeyMap[i].unk0 == target) {
            return padKeyMap[i].unk4;
        }
    }

    return ' ';
}

void Tools_ProcessString(char* pStr) {
    u8* pString = (u8*)pStr;
    while (pString && *pString) {
        switch (*pString) {
            case 0x90:
                *pString = Tools_MapPadToKey(tyControl.activeControls[0]);
                break;
            case 0x91:
                *pString = Tools_MapPadToKey(tyControl.activeControls[1]);
                break;
            case 0x92:
                *pString = Tools_MapPadToKey(tyControl.activeControls[2]);
                break;
            case 0x93:
                *pString = Tools_MapPadToKey(tyControl.activeControls[3]);
                break;
            case 0x94:
                *pString = Tools_MapPadToKey(2);
                break;
            case 0x95:
                *pString = Tools_MapPadToKey(4);
                break;
            case 0x96:
                *pString = Tools_MapPadToKey(tyControl.activeControls[10]);
                break;
            case 0x97:
                *pString = Tools_MapPadToKey(tyControl.activeControls[11]);
                break;
            case 0x9B:
                *pString = Tools_MapPadToKey(tyControl.activeControls[5]);
                break;
            case 0x9D:
                *pString = Tools_MapPadToKey(tyControl.activeControls[4]);
                break;
            case 0x9E:
                *pString = Tools_MapPadToKey(tyControl.activeControls[6]);
                break;
        }
        
        pString++;
    }
}

void FaderObject::Reset(void) {
    fadeMode = FADEMODE_1;
    prevFadeState = FADESTATE_0;
    currFadeState = FADESTATE_0;
    unkC = 0;
    unk18 = -1;
    unk14 = -1;
    unk10 = -1;
}

void FaderObject::Update(void) {
    if (currFadeState != FADESTATE_0 && --unkC < 0) {
        unkC = 0;
        FadeState nextState = GetNextState(currFadeState, fadeMode);
        prevFadeState = currFadeState;
        currFadeState = nextState;
    }
}

void FaderObject::Fade(FaderObject::FadeMode mode, float f1, float f2, float f3, bool r5) {
    float f31 = 1.0f;

    if (currFadeState != FADESTATE_0 && unkC > 0 && r5) {
        int r0 = unkC;
        if (currFadeState == FADESTATE_1) {
            r0 = unk10;
        } else if (currFadeState == FADESTATE_2) {
            r0 = unk14;
        } else if (currFadeState == FADESTATE_3) {
            r0 = unk18;
        }

        f31 = (float)unkC / (float)r0;
        // if the current state is 1 and the next state is 2 OR
        //    the current state is 2 and the next state is 1
        if ((currFadeState == FADESTATE_1 && GetNextState(FADESTATE_0, mode) == FADESTATE_2)
            || (currFadeState == FADESTATE_2 && GetNextState(FADESTATE_0, mode) == FADESTATE_1)) {
                f31 = 1.0f - f31;
        }
    }

    unk10 = gDisplay.fps * f1;
    unk14 = gDisplay.fps * f2;
    unk18 = gDisplay.fps * f3;

    fadeMode = mode;

    FaderObject::FadeState nextState = GetNextState(FADESTATE_0, mode);

    prevFadeState = currFadeState;
    currFadeState = nextState;

    if (currFadeState == FADESTATE_1) {
        unkC = unk10;
    } else if (currFadeState == FADESTATE_2) {
        unkC = unk14;
    } else if (currFadeState == FADESTATE_3) {
        unkC = unk18;
    } else {
        unkC = 0;
    }

    unkC = (float)unkC * f31;
}

FaderObject::FadeState FaderObject::GetNextState(FaderObject::FadeState currState, FaderObject::FadeMode currMode) {
    FadeState nextState;

    switch (currState) {
        case FADESTATE_0:
        default:
            if (currMode == FADEMODE_1 || currMode == FADEMODE_2 || currMode == FADEMODE_3 || currMode == FADEMODE_4) {
                nextState = FADESTATE_1;
            } else if (currMode == FADEMODE_5 || currMode == FADEMODE_6 || currMode == FADEMODE_7 || currMode == FADEMODE_8) {
                nextState = FADESTATE_2;
            } else {
                nextState = FADESTATE_3;
            }
            break;
        case FADESTATE_1:
            if (currMode == FADEMODE_3) {
                nextState = FADESTATE_2;
            } else if (currMode == FADEMODE_2 || currMode == FADEMODE_4) {
                nextState = FADESTATE_3;
            } else {
                nextState = FADESTATE_0;
            }
            break;
        case FADESTATE_2:
            if (currMode == FADEMODE_7) {
                nextState = FADESTATE_1;
            } else if (currMode == FADEMODE_6 || currMode == FADEMODE_8) {
                nextState = FADESTATE_3;
            } else {
                nextState = FADESTATE_0;
            }
            break;
        case FADESTATE_3:
            if (currMode == FADEMODE_10 || currMode == FADEMODE_8) {
                nextState = FADESTATE_1;
            } else if (currMode == FADEMODE_11 || currMode == FADEMODE_4) {
                nextState = FADESTATE_2;
            } else {
                nextState = FADESTATE_0;
            }
            break;
    }

    switch (nextState) {
        case FADESTATE_1:
            unkC = unk10;
            break;
        case FADESTATE_2:
            unkC = unk14;
            break;
        case FADESTATE_3:
            unkC = unk18;
            break;
    }
    
    return nextState;
}

float FaderObject::GetFadePercentage(void) {
    if (currFadeState == FADESTATE_1 || (currFadeState == FADESTATE_0 && prevFadeState == FADESTATE_1)) {
        return 1.0f - ((float)unkC / (float)unk10);
    }

    if (currFadeState == FADESTATE_2 || (currFadeState == FADESTATE_0 && prevFadeState == FADESTATE_2)) {
        return (float)unkC / (float)unk14;
    }

    return 0.0f;
}

// struct Tools_BitField {
//     uint val;
    
//     char* Str(void);
// };

// char* Tools_BitField::Str(void) {
//     char str[33] = {};
//     for(int i = 0; i <= 31; i++) {
//         int shift = 1 << ((31 - i) & 31);
//         str[i] = (val & shift) ? '1' : '0';
//     }
//     return Str_Printf("%s", str);
// }
