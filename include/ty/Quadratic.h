#ifndef QUADRATIC_H
#define QUADRATIC_H

#include "common/Vector.h"

// represents a 3D quadratic equation ax^2 + bx + c
struct Quadratic {
	Vector pos;
	Vector coeffsX; // X coefficients
	Vector coeffsY; // Y coefficients
	Vector coeffsZ; // Z coefficients
	
	float GetClosestTime(Vector* pPoint);
	void Init(void);
	void SetPoints(Vector*, Vector*, Vector*);
	void Update(float t) {
		float t2 = t * t;
		pos.x = (coeffsX.x * t2) + (coeffsX.y * t) + coeffsX.z;
		pos.y = (coeffsY.x * t2) + (coeffsY.y * t) + coeffsY.z;
		pos.z = (coeffsZ.x * t2) + (coeffsZ.y * t) + coeffsZ.z;
	}
};

#endif // QUADRATIC_H