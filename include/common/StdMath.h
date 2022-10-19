#include "types.h"

// union for _table_sinf
// using inlines to load a float as an int and an int as a float causes stack not to match
// use an anonymous union instead?
union sindata {
    float xf;
    int xi;
};

int RandomIR(int*, int, int);
int RandomI(int*);
float RandomFR(int*, float, float);
float _table_sinf(float);
float NormaliseAngle(float);