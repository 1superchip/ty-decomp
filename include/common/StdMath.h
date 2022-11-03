#include "types.h"

union sindata {
    float xf;
    int xi;
};

int RandomIR(int*, int, int);
int RandomI(int*);
float RandomFR(int*, float, float);
float _table_sinf(float);
float NormaliseAngle(float);