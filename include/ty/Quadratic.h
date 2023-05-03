#ifndef QUADRATIC_H
#define QUADRATIC_H

#include "common/Vector.h"

// represents a 3D quadratic equation ax^2 + bx + c
struct Quadratic {
	Vector pos;
	Vector coeffsX; // X coefficients
	Vector coeffsY; // Y coefficients
	Vector coeffsZ; // Z coefficients
	
	float GetClosestTime(Vector*);
	void Init(void);
	void SetPoints(Vector*, Vector*, Vector*);
};

#endif // QUADRATIC_H