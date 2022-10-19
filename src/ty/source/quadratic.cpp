#include "types.h"
#include "common/Vector.h"
#include "ty/Quadratic.h"

float OrderFloatQuadratic(float x) {
	return 4.0f * x;
}

float Quadratic::GetClosestTime(Vector *pPoint) {
    float f1 = 10.0f;
    float f0 = 100000000.0f;
    float t = 0.0f;
    float distSq;

    while (t <= 1.0f) {
        float t2 = t * t;
        pos.x = coeffsX.x * t2 + coeffsX.y * t + coeffsX.z;
        pos.y = coeffsY.x * t2 + coeffsY.y * t + coeffsY.z;
        pos.z = coeffsZ.x * t2 + coeffsZ.y * t + coeffsZ.z;
        distSq = pPoint->DistSq(&pos);
        if (distSq < f0) {
            f0 = distSq;
            f1 = t;
        }
        t += 0.1f;
    }

    if (f1 <= 1.0f) {
        float f2 = 0.05f;
        float f3 = 100000000.0f;
        float t = f1 - f2;

        while (t <= f2 + f1) {
            if (!(t < 0.0f) && !(t > 1.0f)) {
                float f8 = t * t;
                pos.x = coeffsX.x * f8 + coeffsX.y * t + coeffsX.z;
                pos.y = coeffsY.x * f8 + coeffsY.y * t + coeffsY.z;
                pos.z = coeffsZ.x * f8 + coeffsZ.y * t + coeffsZ.z;
                distSq = pPoint->DistSq(&pos);
                if (distSq < f3) {
                    f3 = distSq;
                    f1 = t;
                }
            }
            t += 0.01f;
        }
        return f1;
    }
    return 0.0f;
}

void Quadratic::Init(void) {
	pos.Set(0.0f, 0.0f, 0.0f);
	pos.w = 1.0f;
	coeffsZ = pos;
	coeffsY = coeffsZ;
	coeffsX = coeffsY;
}

inline void sub(Vector& a, Vector &b, Vector& c) {
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
}

// calculate equation coefficients from 3 points
// something here inlined? (float order?)
void Quadratic::SetPoints(Vector* arg0, Vector* arg1, Vector* arg2) {
    Vector diff1;
    sub(*arg1, *arg0, diff1); // would these have been operators?
    Vector diff;
    sub(*arg2, *arg0, diff);
    
    coeffsX.y = 4.0f * diff1.x - diff.x;
    coeffsX.x = diff.x - coeffsX.y;
    coeffsX.z = arg0->x;
    
    coeffsY.y = 4.0f * diff1.y - diff.y;
    coeffsY.x = diff.y - coeffsY.y;
    coeffsY.z = arg0->y;
    
    coeffsZ.y = 4.0f * diff1.z - diff.z;
    coeffsZ.x = diff.z - coeffsZ.y;
    coeffsZ.z = arg0->z;
    pos.z = 0.0f;
    pos.y = 0.0f;
    pos.x = 0.0f;
}

/*
// matches without sub inline
void Quadratic::SetPoints(Vector* arg0, Vector* arg1, Vector* arg2) {
    Vector diff;
    Vector diff1;
    diff1.x = arg1->x - arg0->x;
    diff1.y = arg1->y - arg0->y;
    diff1.z = arg1->z - arg0->z;
    diff.x = arg2->x - arg0->x;
    diff.y = arg2->y - arg0->y;
    diff.z = arg2->z - arg0->z;
    
    coeffsX.y = 4.0f * diff1.x - diff.x;
    coeffsX.x = diff.x - coeffsX.y;
    coeffsX.z = arg0->x;
    
    coeffsY.y = 4.0f * diff1.y - diff.y;
    coeffsY.x = diff.y - coeffsY.y;
    coeffsY.z = arg0->y;
    
    coeffsZ.y = 4.0f * diff1.z - diff.z;
    coeffsZ.x = diff.z - coeffsZ.y;
    coeffsZ.z = arg0->z;
    pos.z = 0.0f;
    pos.y = 0.0f;
    pos.x = 0.0f;
}*/