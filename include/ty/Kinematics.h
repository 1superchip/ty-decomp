
float Kin_GetInitialVelocity(float*, float*, float*, float*);
float Kin_GetDiscreteDistance(float*, float*, float*, float*);
float Kin_GetDiscreteInitialVelocity(float*, float*, float*, float*);
float Kin_GetDiscreteFinalVelocity(float*, float*, float*, float*);
float Kin_GetDiscreteAcceleration(float*, float*, float*, float*);

// this is not quite the same as the CW sqrtf inline
// CW sqrtf does not match in this file
// used in other places than this file
inline float kin_sqrtf(float x) {
    static const double _half=.5;
	static const double _three=3.0;
    double guess = __frsqrte(x);
    guess = _half * guess * (_three - ( x * (guess * guess)));
    guess = _half * guess * (_three - ( x * (guess * guess)));
    volatile float y = ( x * (_half * guess * (_three - ( x * (guess * guess)))));
    return y;
}