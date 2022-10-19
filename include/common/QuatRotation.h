#include "types.h"
#include "common/StdMath.h" // _table_sinf

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