#include "types.h"
#include "ty/Quadratic.h"

float OrderFloatQuadratic(float x) {
	return 4.0f * x;
}

float Quadratic::GetClosestTime(Vector *pPoint) {
    float f1 = 10.0f;
    float closestDist = 100000000.0f;
    float t = 0.0f;
    float distSq;

    while (t <= 1.0f) {
        float t2 = t * t;
        pos.x = coeffsA.x * t2 + coeffsA.y * t + coeffsA.z;
        pos.y = coeffsB.x * t2 + coeffsB.y * t + coeffsB.z;
        pos.z = coeffsC.x * t2 + coeffsC.y * t + coeffsC.z;
        distSq = pPoint->DistSq(&pos);
        if (distSq < closestDist) {
            closestDist = distSq;
            f1 = t;
        }
        t += 0.1f;
    }

    if (f1 <= 1.0f) {
        float f2 = 0.05f;
        closestDist = 100000000.0f;
        float t = f1 - f2;

        while (t <= f2 + f1) {
            if (!(t < 0.0f) && !(t > 1.0f)) {
                float f8 = t * t;
                pos.x = coeffsA.x * f8 + coeffsA.y * t + coeffsA.z;
                pos.y = coeffsB.x * f8 + coeffsB.y * t + coeffsB.z;
                pos.z = coeffsC.x * f8 + coeffsC.y * t + coeffsC.z;
                distSq = pPoint->DistSq(&pos);
                if (distSq < closestDist) {
                    closestDist = distSq;
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
	coeffsC = pos;
	coeffsB = coeffsC;
	coeffsA = coeffsB;
}

// calculate equation coefficients from 3 points
// something here inlined? (float order?)
void Quadratic::SetPoints(Vector* arg0, Vector* arg1, Vector* arg2) {
    Vector diff;
    Vector diff1;
    diff1.Sub(arg1, arg0);
    diff.Sub(arg2, arg0);
    
    coeffsA.y = 4.0f * diff1.x - diff.x;
    coeffsA.x = diff.x - coeffsA.y;
    coeffsA.z = arg0->x;
    
    coeffsB.y = 4.0f * diff1.y - diff.y;
    coeffsB.x = diff.y - coeffsB.y;
    coeffsB.z = arg0->y;
    
    coeffsC.y = 4.0f * diff1.z - diff.z;
    coeffsC.x = diff.z - coeffsC.y;
    coeffsC.z = arg0->z;
    pos.SetZero();
}