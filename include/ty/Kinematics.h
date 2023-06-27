#ifndef KINEMATICS_H
#define KINEMATICS_H

float Kin_GetInitialVelocity(float*, float*, float*, float*);
float Kin_GetDiscreteDistance(float*, float*, float*, float*);
float Kin_GetDiscreteInitialVelocity(float*, float*, float*, float*);
float Kin_GetDiscreteFinalVelocity(float*, float*, float*, float*);
float Kin_GetDiscreteAcceleration(float*, float*, float*, float*);

#endif // KINEMATICS_H
