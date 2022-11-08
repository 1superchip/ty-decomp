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
};

#endif // COMMON_QUATROTATION