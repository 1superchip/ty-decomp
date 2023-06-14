#ifndef TALISMAN_H
#define TALISMAN_H

#include "ty/props/StaticProp.h"

void Talisman_LoadResources(KromeIni* pIni);

#define TALISMAN_COUNT 5

// Contains information about Talisman props such as the model's name and the descriptor's name
struct TalismanInfo {
    char* pModelName; // Model name
    char* pDescrName; // Descriptor name
};

struct ParticleSystem {
    char padding0[0x34];
    float scale;
};

struct Talisman : StaticProp {
    ParticleSystem* pParticleSystem;
    bool bCurrentVisible; // Current visibility
    bool bVisible; // Default visibility setting for this talisman
	
	virtual bool LoadLine(KromeIniLine* pLine);
	virtual void LoadDone(void);
    virtual void Reset(void);
	virtual void Update(void);
	virtual void Draw(void);
	virtual void Message(MKMessage* pMessage);
	virtual void Init(GameObjDesc* pDesc);
	virtual void Deinit(void);
};

#endif // TALISMAN_H
