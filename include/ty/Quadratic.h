

struct Quadratic {
	Vector pos;
	Vector coeffsX;
	Vector coeffsY;
	Vector coeffsZ;
	
	float GetClosestTime(Vector*);
	void Init(void);
	void SetPoints(Vector*, Vector*, Vector*);
};