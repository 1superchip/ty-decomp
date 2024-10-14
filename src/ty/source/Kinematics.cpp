#include "types.h"
#include "ty/Kinematics.h"
#include "common/StdMath.h"

// check inlines

// https://decomp.me/scratch/EJZlC
float Kin_GetInitialVelocity(float* arg0, float* arg1, float* arg2, float* arg3) {
    float temp;

    if (arg0 == NULL) {
        return *arg1 - (*arg3 * *arg2);
    }
    if (arg1 == NULL) {
        return (*arg0 / *arg2) - ((*arg3 * 0.5f) * *arg2);
    }
    if (arg2 == NULL) {
        temp = 2.0f * *arg3;
        return sqrtf(Sqr<float>(*arg1) - (temp * *arg0));
    }
    if (arg3 != NULL) {
        return 0.0f;
    }
    return ((*arg0 * 2.0f) / *arg2) - *arg1;
}

float Kin_GetDiscreteDistance(float* arg0, float* arg1, float* arg2, float* arg3) {
    float temp_f2;
    float temp_f3;
    float temp_f4;

    if (arg0 == NULL) {
        return (*arg1 * *arg2) - ((*arg2 - 1.0f) * (0.5f * *arg3 * *arg2));
    }
    if (arg1 == NULL) {
        return (*arg0 * *arg2) + ((1.0f + *arg2) * (0.5f * *arg3 * *arg2));
    }
    if (arg2 == NULL) {
        temp_f4 = *arg3;
        temp_f2 = *arg1;
        temp_f3 = *arg0;
        return ((temp_f2 - temp_f3) * (temp_f3 + (temp_f2 + temp_f4))) / (2.0f * temp_f4);
    }
    if (arg3 != NULL) {
        return 0.0f;
    }
    return *arg2 * (1.0f + *arg0 + *arg1) / 2.0f;
}

float Kin_GetDiscreteInitialVelocity(float* arg0, float* arg1, float* arg2, float* arg3) {
    float temp_f2;
    float temp_f4;
    float temp_f6;

    if (arg0 == NULL) {
        return *arg1 - (*arg3 * *arg2);
    }
    if (arg1 == NULL) {
        temp_f4 = *arg2;
        return (*arg0 / temp_f4) - (0.5f * *arg3 * (1.0f + temp_f4));
    }
    if (arg2 == NULL) {
        temp_f6 = *arg3;
        temp_f2 = *arg1;
        return 0.5f * (-*arg3 + sqrtf(Sqr<float>(temp_f6) + (4.0f * (((temp_f2 * temp_f6) + Sqr<float>(temp_f2)) - (2.0f * *arg0 * temp_f6)))));
    }
    if (arg3 != NULL) {
        return 0.0f;
    }
    return (((2.0f * *arg0) / *arg2) - *arg1) - 1.0f;
}

float Kin_GetDiscreteFinalVelocity(float* arg0, float* arg1, float* arg2, float* arg3) {
    float temp_f1;
    float temp_f4;
    float temp_f6;
    float var_f4;

    if (arg0 == NULL) {
        return *arg1 + (*arg3 * *arg2);
    }
    if (arg1 == NULL) {
        temp_f4 = *arg2;
        return (*arg0 / temp_f4) + (0.5f * *arg3 * (temp_f4 - 1.0f));
    }
    if (arg2 == NULL) {
        temp_f6 = *arg3;
        temp_f1 = *arg1;
        return 0.5f * (-*arg3 + sqrtf(Sqr<float>(temp_f6) + (4.0f * ((2.0f * *arg0 * temp_f6) + ((temp_f1 * temp_f6) + Sqr<float>(temp_f1))))));
    }
    if (arg3 != NULL) {
        return 0.0f;
    }
    return (((2.0f * *arg0) / *arg2) - *arg1) - 1.0f;
}

float Kin_GetDiscreteAcceleration(float* arg0, float* arg1, float* arg2, float* arg3) {
    float temp_f3;
    float temp_f4;
    float temp_f4_2;
    float temp_f4_3;

    if (arg0 == NULL) {
        return (*arg2 - *arg1) / *arg3;
    }
    if (arg1 == NULL) {
        temp_f4_3 = *arg3;
        return (2.0f * ((*arg2 * temp_f4_3) - *arg0)) / (temp_f4_3 * (temp_f4_3 - 1.0f));
    }
    if (arg2 == NULL) {
        temp_f4_2 = *arg3;
        return (2.0f * (*arg0 - (*arg1 * temp_f4_2))) / (temp_f4_2 * (1.0f + temp_f4_2));
    }
    if (arg3 == NULL) {
        temp_f3 = *arg1;
        temp_f4 = *arg2;
        return ((temp_f4 * temp_f4) - Sqr<float>(temp_f3)) / (((2.0f * *arg0) + temp_f3) - temp_f4);
    }
    return 0.0f;
}
