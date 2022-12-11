#include "ty/GameObject.h"
#include "common/KromeIni.h"
#include "common/Matrix.h"

enum GemState {};

#define GEMS_MAXOPALS 300

struct Gem : GameObject {
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Draw(void);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
	
	void SetState(GemState);
	
	char unk3C[0x30];
    int unk6C;
	GemState state;
	Vector pos;
	float unk84[3];
	float time; // used for Linear Interpolation
	Vector unk94;
	Vector collisionNormal;
	Matrix unkB4;
	s16 unkF4;
    u8 unkF6b0 : 1;
    u8 unkF6b1 : 1;
    u8 unkF6b2 : 1;
    u8 unkF6b3 : 1;
    u8 unkF6b4 : 1;
    u8 unkF6b5 : 1;
    u8 unkF6b6 : 1;
    u8 unkF6b7 : 1;
	float yOffsetAngle;
	float unkFC;
	char unk100[12];
	
	static int totalGems;
	static Gem** gemPtrList;
};