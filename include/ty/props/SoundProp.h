#include "types.h"
#include "common/Vector.h"
#include "ty/GameObject.h"

struct SoundEventHelper {
	int unk0;
	
	void Update(int, bool, bool, GameObject*, Vector*, float, int);
};

struct SoundProp : GameObject {
	Matrix localToWorld;
	SoundEventHelper unk7C;
	int unk80;
	int unk84;
	int unk88;
	int minDelay;
	int maxDelay;
	CommonGameObjFlagsComponent gameObjFlags;
	u16 defaultFlags; // both flags may be contained in CommonGameObjFlagsComponent
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
    void Play(void);
	
};

void SoundProp_LoadResources(KromeIni*);