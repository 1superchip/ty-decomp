
#include "ty/Spline.h"
#include "common/Heap.h"

// Documentation might be wrong about the matrix equations

// {t^3 - 2t^2 + t, 2t^3 - 3t^2 + 1, -2t^3 + 3t^2, t^3 - t^2}
static Matrix positionMixer = {
    1.0f, 2.0f, -2.0f, 1.0f,
    -2.0f, -3.0f, 3.0f, -1.0f,
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f
};

/*
// not sure this is 100% correct
// looks like it is a hermite spline with the first 2 equations swapped?
// seems to differ possibly due to matrix major differences?
// I think the equation order is changed due to the matrix rows being set
// in a different order than the parameters are passed
Vector PositionEquation(float t) {
    Vector final;
    // {t^3 - 2 t^2 + t + 0, 2 t^3 - 3 t^2 + 1, -2 t^3 + 3 t^2 + 0, t^3 - t^2 + 0}
    final.x = (t * t * t) - (2.0f * (t * t)) + t;
    final.y = (2.0f * (t * t * t)) - (3.0f * (t * t)) + 1.0f;
    final.z = (-2.0f * (t * t * t)) + (3.0f * (t * t));
    final.w = (t * t * t) - (t * t);
    return final;
}

// Velocity equation
// Derivative of PositionEquation
Vector CalcVelocity(float t) {
    // {3t^2 - 4t + 1, 6t^2 - 6t, -6t^2 + 6t, 3t^2 - 2t}
    Vector final;
    final.x = (3.0f * (t * t)) - (4.0f * t) + 1;
    final.y = (6.0f * (t * t)) - (6.0f * t);
    final.z = (-6.0f * (t * t)) + (6.0f * t);
    final.w = (3.0f * (t * t)) - (2.0f * t);
    return final;
}
*/

// Derivative of positionMixer
// {3t^2 - 4t + 1, 6t^2 - 6t, -6t^2 + 6t, 3t^2 - 2t}
static Matrix velocityMixer = {
    0.0f, 0.0f, 0.0f, 0.0f,
    3.0f, 6.0f, -6.0f, 3.0f,
    -4.0f, -6.0f, 6.0f, -2.0f,
    1.0f, 0.0f, 0.0f, 0.0f
};

Vector Spline_GetPosition(Vector* r4, Vector* r5, Vector* r6, Vector* r7, float t) {
    Matrix m;
    m.SetIdentity();
    /*
    // r4 and r6 are unk0 and r5 and r7 is unk10 of the spline points
    // so the matrix elements are different, so the positionMixer/velocityMixer matrices
    // have to be changed
    m = {
        {r5},
        {r4},
        {r6},
        {r7}
    }
    */
    m.Row0()->Copy(r5);
    m.Row1()->Copy(r4);
    m.Row2()->Copy(r6);
    m.Row3()->Copy(r7);
    m.Multiply4x4(&positionMixer, &m);
    Vector pos = {t * t * t, t * t, t, 1.0f}; // {t^3, t^2, t, 1.0f}
    pos.ApplyMatrix(&m);
    return pos;
}

Vector Spline_GetVelocity(Vector* r4, Vector* r5, Vector* r6, Vector* r7, float t) {
    Matrix m;
    m.SetIdentity();
    m.Row0()->Copy(r5);
    m.Row1()->Copy(r4);
    m.Row2()->Copy(r6);
    m.Row3()->Copy(r7);
    m.Multiply4x4(&velocityMixer, &m);
    Vector pos = {t * t * t, t * t, t, 1.0f}; // {t^3, t^2, t, 1.0f}
    pos.ApplyMatrix(&m);
    return pos;
}

/// @brief Allocates SplinePoints 
/// @param numPoints Number of points the spline will have
/// @param r5 Some setting for normalization?
void Spline::Init(int numPoints, bool r5) {

    if (numPoints > 0) {
        mpPoints = (SplinePoint*)Heap_MemAlloc(numPoints * sizeof(SplinePoint));
    } else {
        mpPoints = NULL;
    }

    mNumPoints = numPoints;
    nodeIndex = 0;
    unkC = r5;
}

/// @brief Resets all spline points and sets nodeIndex to 0
/// @param None
void Spline::Reset(void) {
    for (int i = 0; i < mNumPoints; i++) {
        mpPoints[i].mPos.SetZero();
        mpPoints[i].unk10.SetZero();
    }

    nodeIndex = 0;
}

/// @brief Adds a node to a Spline
/// @param pVec Position of point on spline?
/// @return True if the node was added, false otherwise
bool Spline::AddNode(Vector* pVec) {
    if (nodeIndex == mNumPoints) {
        return false;
    }

    mpPoints[nodeIndex].mPos = *pVec;

    if (nodeIndex == 0) {
        mpPoints[nodeIndex].unk10.SetZero();
        nodeIndex++;
        return true;
    }

    mpPoints[nodeIndex].unk10.Sub(pVec, &mpPoints[nodeIndex - 1].mPos);

    if (nodeIndex == 1) {
        mpPoints[0].unk10.Sub(pVec, &mpPoints[0].mPos);
        nodeIndex++;
        return true;
    }

    if (unkC) {
        Vector temp1;
        temp1.Sub(&mpPoints[nodeIndex - 1].mPos, &mpPoints[nodeIndex - 2].mPos);
        float f31 = temp1.Normalise();
        Vector temp2;
        temp2.Sub(pVec, &mpPoints[nodeIndex - 1].mPos);
        float f30 = temp2.Normalise();
        mpPoints[nodeIndex - 1].unk10.Add(&temp1, &temp2);
        mpPoints[nodeIndex - 1].unk10.Normalise();
        mpPoints[nodeIndex - 1].unk10.Scale((f31 + f30) * 0.5f);
    } else {
        mpPoints[nodeIndex - 1].unk10.Sub(pVec, &mpPoints[nodeIndex - 2].mPos);
        mpPoints[nodeIndex - 1].unk10.Scale(0.5f);
    }

    mpPoints[nodeIndex].unk10.Scale(3.0f);
    mpPoints[nodeIndex].unk10.Subtract(&mpPoints[nodeIndex - 1].unk10);
    
    if (unkC) {
        Vector temp1;
        temp1.Sub(&mpPoints[nodeIndex].mPos, &mpPoints[nodeIndex - 1].mPos);
        float f31 = temp1.Normalise();
        mpPoints[nodeIndex].unk10.Normalise();
        mpPoints[nodeIndex].unk10.Scale(f31);
    } else {
        mpPoints[nodeIndex].unk10.Scale(0.5f);
    }

    if (nodeIndex == 2) {
        mpPoints[0].unk10.Scale(3.0f);
        mpPoints[0].unk10.Subtract(&mpPoints[1].unk10);

        if (unkC) {
            Vector temp1;
            temp1.Sub(&mpPoints[1].mPos, &mpPoints[0].mPos);
            float f31 = temp1.Normalise();
            mpPoints[0].unk10.Normalise();
            mpPoints[0].unk10.Scale(f31);
        } else {
            mpPoints[0].unk10.Scale(0.5f);
        }
    }

    nodeIndex++;
    return true;
}

/// @brief Gets the position at a specific time
/// @param time 
/// @return Position Vector at time
Vector Spline::GetPosition(float time) {
    int idx = nodeIndex;
    idx--;
    float f31 = time;
    f31 *= idx;
    int cmp = (int)f31;
    f31 -= cmp;

    if (cmp == idx) {
        return mpPoints[cmp].mPos;
    }

    return Spline_GetPosition(
        &mpPoints[cmp].mPos, &mpPoints[cmp].unk10,
        &mpPoints[cmp + 1].mPos, &mpPoints[cmp + 1].unk10, 
        f31
    );
}

/// @brief Gets the velocity at a specific time
/// @param time 
/// @return Velocity Vector at time
Vector Spline::GetVelocity(float time) {
    int idx = nodeIndex;
    idx--;
    float f31 = time;
    f31 *= idx;
    int cmp = (int)f31;
    f31 -= cmp;

    if (cmp == idx) {
        return mpPoints[cmp].unk10;
    }

    return Spline_GetVelocity(
        &mpPoints[cmp].mPos, &mpPoints[cmp].unk10,
        &mpPoints[cmp + 1].mPos, &mpPoints[cmp + 1].unk10, 
        f31
    );
}

/// @brief Frees memory of this Spline
/// @param None
void Spline::Deinit(void) {
    if (mpPoints) {
        Heap_MemFree(mpPoints);
    }

    mpPoints = NULL;
    mNumPoints = 0;
}

bool Spline::MergeEnds(void) {
    if (!mpPoints[0].mPos.Equals(&mpPoints[nodeIndex - 1].mPos)) {
        return false;
    }

    if (nodeIndex < 3) {
        return false;
    }

    Vector loopVel;
    loopVel.Sub(&mpPoints[1].mPos, &mpPoints[nodeIndex - 2].mPos);
    loopVel.Scale(0.5f);

    if (unkC) {
        Vector temp;
        temp.Sub(&mpPoints[1].mPos, &mpPoints[0].mPos);
        float mag = temp.Magnitude();
        temp.Sub(&mpPoints[nodeIndex - 1].mPos, &mpPoints[nodeIndex - 2].mPos);
        float mag2 = temp.Magnitude();
        loopVel.Normalise();
        loopVel.Scale((mag + mag2) * 0.5f);
    }

    mpPoints[0].unk10 = loopVel;
    mpPoints[nodeIndex - 1].unk10 = loopVel;

    return true;
}

void Spline::Smooth(void) {
    Vector oldVel;
    Vector newVel;
    Vector tmp;

    if (nodeIndex > 1) {
        bool areEqual = false;
        if (mpPoints[0].mPos.Equals(&mpPoints[nodeIndex - 1].mPos) &&
            mpPoints[0].unk10.Equals(&mpPoints[nodeIndex - 1].unk10)) {
            areEqual = true;
        }

        // checking if end points aren't equal?
        if (!areEqual) {
            oldVel.Scale(&mpPoints[1].mPos, 3.0f);
            tmp.Scale(&mpPoints[0].mPos, 3.0f);
            oldVel.Subtract(&tmp);
            oldVel.Subtract(&mpPoints[1].unk10);
            oldVel.Scale(0.5f);
            for (int i = 1; i < nodeIndex - 1; i++) {
                newVel.Scale(&mpPoints[i + 1].mPos, 3.0f);
                newVel.Subtract(&mpPoints[i + 1].unk10);
                tmp.Scale(&mpPoints[i - 1].mPos, 3.0f);
                newVel.Subtract(&tmp);
                newVel.Subtract(&mpPoints[i - 1].unk10);
                newVel.Scale(0.25f);
                if (unkC) {
                    Vector temp;
                    temp.Sub(&mpPoints[i].mPos, &mpPoints[i - 1].mPos);
                    float f23 = temp.Magnitude();
                    temp.Sub(&mpPoints[i + 1].mPos, &mpPoints[i].mPos);
                    float f22 = temp.Magnitude();
                    newVel.Normalise();
                    newVel.Scale((f23 + f22) * 0.5f);
                }
                mpPoints[i - 1].unk10 = oldVel;
                oldVel = newVel;
            }

            newVel.Scale(&mpPoints[nodeIndex - 1].mPos, 3.0f);
            tmp.Scale(&mpPoints[nodeIndex - 2].mPos, 3.0f);
            newVel.Subtract(&tmp);
            newVel.Subtract(&mpPoints[nodeIndex - 2].unk10);
            newVel.Scale(0.5f);
            mpPoints[nodeIndex - 2].unk10 = oldVel;
            mpPoints[nodeIndex - 1].unk10 = newVel;
            return;
        }

        oldVel.SetZero();
        Vector startVel = mpPoints[1].unk10;
        for (int i = 1; i < nodeIndex; i++) {
            if (i == nodeIndex - 1) {
                newVel.Scale(&mpPoints[1].mPos, 3.0f);
                newVel.Subtract(&startVel);
            } else {
                newVel.Scale(&mpPoints[i + 1].mPos, 3.0f);
                newVel.Subtract(&mpPoints[i + 1].unk10);
            }

            tmp.Scale(&mpPoints[i - 1].mPos, 3.0f);
            newVel.Subtract(&tmp);
            newVel.Subtract(&mpPoints[i - 1].unk10);
            newVel.Scale(0.25f);

            if (unkC) {
                Vector temp;
                temp.Sub(&mpPoints[i].mPos, &mpPoints[i - 1].mPos);
                float f23 = temp.Magnitude();
                if (i == (nodeIndex - 1)) {
                    temp.Sub(&mpPoints[1].mPos, &mpPoints[i].mPos);
                } else {
                    temp.Sub(&mpPoints[i + 1].mPos, &mpPoints[i].mPos);
                }
                float f22 = temp.Magnitude();
                newVel.Normalise();
                newVel.Scale((f23 + f22) * 0.5f);
            }

            if (i != 1) {
                mpPoints[i - 1].unk10 = oldVel;
            }

            oldVel = newVel;
        }

        mpPoints[nodeIndex - 1].unk10 = oldVel;
        mpPoints[0].unk10 = mpPoints[nodeIndex - 1].unk10;
    }
}

/// @brief Allocates Spline Points
/// @param numPoints Number of points the spline will have
/// @param r5 Some setting for normalization?
void UniformSpline::Init(int numPoints, bool r5) {
    if (numPoints > 0) {
        mpPoints = (UniformSplinePoint*)Heap_MemAlloc(numPoints * sizeof(UniformSplinePoint));
    } else {
        mpPoints = NULL;
    }

    mNumPoints = numPoints;
    nodeIndex = 0;
    unk8 = 0.0f;
    unk10 = r5;
}

/// @brief Resets points of a UniformSpline
/// @param None
void UniformSpline::Reset(void) {
    for (int i = 0; i < mNumPoints; i++) {
        mpPoints[i].unk20 = 0.0f;
        mpPoints[i].mPos.SetZero();
        mpPoints[i].unk10.SetZero();
    }

    nodeIndex = 0;
    unk8 = 0.0f;
}

/// @brief Adds a node to a UniformSpline
/// @param pVec Position of point on spline?
/// @return True if the node was added, false otherwise
bool UniformSpline::AddNode(Vector* pVec) {

    if (nodeIndex == mNumPoints) {
        return false;
    }

    mpPoints[nodeIndex].mPos = *pVec;

    if (nodeIndex == 0) {
        mpPoints[nodeIndex].unk10.SetZero();
        mpPoints[nodeIndex].unk20 = 1.0f;
        unk8 = 0.0f;
        nodeIndex++;
        return true;
    }

    mpPoints[nodeIndex].unk10.Sub(pVec, &mpPoints[nodeIndex - 1].mPos);
    mpPoints[nodeIndex - 1].unk20 = mpPoints[nodeIndex].unk10.Normalise();
    unk8 += mpPoints[nodeIndex - 1].unk20;

    if (nodeIndex == 1) {
        mpPoints[0].unk10 = mpPoints[nodeIndex].unk10;
        nodeIndex++;
        return true;
    }

    Vector temp;
    temp.Sub(&mpPoints[nodeIndex - 1].mPos, &mpPoints[nodeIndex - 2].mPos);

    if (mpPoints[nodeIndex - 2].unk20) {
        temp.Scale(1.0f / mpPoints[nodeIndex - 2].unk20);
    }

    mpPoints[nodeIndex - 1].unk10.Add(&mpPoints[nodeIndex].unk10, &temp);
    mpPoints[nodeIndex - 1].unk10.Scale(0.5f);

    if (unk10) {
        mpPoints[nodeIndex - 1].unk10.Normalise();
    }

    mpPoints[nodeIndex].unk10.Scale(3.0f);
    mpPoints[nodeIndex].unk10.Subtract(&mpPoints[nodeIndex - 1].unk10);

    if (unk10) {
        mpPoints[nodeIndex].unk10.Normalise();
    } else {
        mpPoints[nodeIndex].unk10.Scale(0.5f);
    }

    if (nodeIndex == 2) {
        mpPoints[0].unk10.Scale(3.0f);
        mpPoints[0].unk10.Subtract(&mpPoints[1].unk10);
        if (unk10) {
            mpPoints[0].unk10.Normalise();
        } else {
            mpPoints[0].unk10.Scale(0.5f);
        }
    }

    nodeIndex++;

    return true;
}

/// @brief Gets the position at a specific time
/// @param time 
/// @return Position Vector at time
Vector UniformSpline::GetPosition(float time) {
    float time_ = time * unk8;
    int i = 0;
    float f2 = 0.0f;

    for (i; mpPoints[i].unk20 + f2 < time_ && i < nodeIndex - 1; i++) {
        f2 += mpPoints[i].unk20;
    }

    float f31 = time_ - f2;
    if (mpPoints[i].unk20) {
        f31 /= mpPoints[i].unk20;
    }

    if (i == (nodeIndex - 1)) {
        return mpPoints[i].mPos;
    }

    Vector startVel;
    startVel.Scale(&mpPoints[i].unk10, mpPoints[i].unk20);

    Vector endVel;
    endVel.Scale(&mpPoints[i + 1].unk10, mpPoints[i].unk20);

    return Spline_GetPosition(
        &mpPoints[i].mPos, &startVel,
        &mpPoints[i + 1].mPos, &endVel, 
        f31
    );
}

/// @brief Gets the velocity at a specific time
/// @param time 
/// @return Velocity Vector at time
Vector UniformSpline::GetVelocity(float time) {
    float time_ = time * unk8;
    int i = 0;
    float f2 = 0.0f;
    for (i; mpPoints[i].unk20 + f2 < time_ && i < nodeIndex - 1; i++) {
        f2 += mpPoints[i].unk20;
    }

    float f31 = time_ - f2;
    if (mpPoints[i].unk20) {
        f31 /= mpPoints[i].unk20;
    }

    if (i == (nodeIndex - 1)) {
        return mpPoints[i].mPos;
    }

    Vector startVel;
    Vector endVel;
    Vector newVel;

    startVel.Scale(&mpPoints[i].unk10, mpPoints[i].unk20);
    endVel.Scale(&mpPoints[i + 1].unk10, mpPoints[i].unk20);

    newVel = Spline_GetVelocity(
        &mpPoints[i].mPos, &startVel,
        &mpPoints[i + 1].mPos, &endVel, 
        f31
    );

    if (mpPoints[i].unk20) {
        newVel.Scale(1.0f / mpPoints[i].unk20);
    }

    return newVel;
}

/// @brief Deinits a UniformSpline
/// @param None
void UniformSpline::Deinit(void) {
    if (mpPoints) {
        Heap_MemFree(mpPoints);
    }

    mpPoints = NULL;
    mNumPoints = 0;
}

bool UniformSpline::MergeEnds(void) {
    Vector loopVel1;
    Vector loopVel2;
    Vector loopVel;

    if (!mpPoints[0].mPos.Equals(&mpPoints[nodeIndex - 1].mPos)) {
        return false;
    }

    if (nodeIndex < 3) {
        return false;
    }

    loopVel1.Sub(&mpPoints[1].mPos, &mpPoints[0].mPos);
    loopVel1.Scale(1.0f / mpPoints[0].unk20);

    loopVel2.Sub(&mpPoints[nodeIndex - 1].mPos, &mpPoints[nodeIndex - 2].mPos);
    loopVel2.Scale(1.0f / mpPoints[nodeIndex - 2].unk20);

    loopVel.Add(&loopVel1, &loopVel2);
    loopVel.Scale(0.5f);

    if (unk10) {
        loopVel.Normalise();
    }

    mpPoints[0].unk10 = loopVel;
    mpPoints[nodeIndex - 1].unk10 = loopVel;

    return true;
}

void UniformSpline::RegulateSpeed(void) {
    float f29 = mpPoints[0].unk10.Magnitude();
    unk8 = 0.0f;

    for (int i = 0; i < nodeIndex - 1; i++) {
        float f20 = f29;
        f29 = mpPoints[i + 1].unk10.Magnitude();
        float f2 = Sqr<float>((f20 + f29) * 0.5f);
        if (!f2) {
            unk8 += mpPoints[i].unk20;
        } else {
            Vector j;
            Vector k;
            j.Add(&mpPoints[i].unk10, &mpPoints[i + 1].unk10);
            k.Sub(&mpPoints[i].mPos, &mpPoints[i + 1].mPos);
            float sp0MagSq = j.MagSquared() - (f2 * 16.0f);
            float dot = j.Dot(&k) * 12.0f;
            float sq = sqrtf((dot * dot) - ((sp0MagSq * 4.0f) * (k.MagSquared() * 36.0f)));
            mpPoints[i].unk20 = (-dot - sq) / (sp0MagSq * 2.0f);
            unk8 += mpPoints[i].unk20;
        }
    }

}
