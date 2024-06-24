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
        Update(t);
        distSq = pPoint->DistSq(&pos);
        if (distSq < closestDist) {
            closestDist = distSq;
            f1 = t;
        }
        t += 0.1f;
    }

    if (f1 <= 1.0f) {
        closestDist = 100000000.0f;

        for (float t = f1 - 0.05f; t <= f1 + 0.05f; t += 0.01f) {
            if (!(t < 0.0f) && !(t > 1.0f)) {
                Update(t);
                distSq = pPoint->DistSq(&pos);
                if (distSq < closestDist) {
                    closestDist = distSq;
                    f1 = t;
                }
            }
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

// calculate equation coefficients from 3 points
void Quadratic::SetPoints(Vector* arg0, Vector* arg1, Vector* arg2) {
    Vector p3;
    Vector p2;
    p2.Sub(arg1, arg0);
    p3.Sub(arg2, arg0);
    
    coeffsX.y = 4.0f * p2.x - p3.x;
    coeffsX.x = p3.x - coeffsX.y;
    coeffsX.z = arg0->x;
    
    coeffsY.y = 4.0f * p2.y - p3.y;
    coeffsY.x = p3.y - coeffsY.y;
    coeffsY.z = arg0->y;
    
    coeffsZ.y = 4.0f * p2.z - p3.z;
    coeffsZ.x = p3.z - coeffsZ.y;
    coeffsZ.z = arg0->z;
    pos.SetZero();
}
