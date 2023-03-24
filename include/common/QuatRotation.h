#ifndef COMMON_QUATROTATION
#define COMMON_QUATROTATION

#include "common/Vector.h"

struct AxisRotation {
    Vector rot;
    float unkC;
};

struct QuatRotation {
    Vector quat;
	void ConvertRotation(AxisRotation*);
	void ConvertNormal(Vector*, float);
	void ConvertVector(Vector*);
	void Multiply(QuatRotation*, QuatRotation*); //https://decomp.me/scratch/cue99
	// https://decomp.me/scratch/WoxtP
    float Dot(QuatRotation* pOther) {
        return quat.x * pOther->quat.x + quat.y * pOther->quat.y + 
            quat.z * pOther->quat.z + quat.w * pOther->quat.w;
    }
    void Scale(QuatRotation* pOther, float scalar) {
        quat.x = scalar * pOther->quat.x;
        quat.y = scalar * pOther->quat.y;
        quat.z = scalar * pOther->quat.z;
        quat.w = scalar * pOther->quat.w;
    }
	void Scale(float scalar) {
		Scale(this, scalar);
	}
};

#endif // COMMON_QUATROTATION