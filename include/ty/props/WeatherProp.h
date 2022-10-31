#include "ty/GameObjectManager.h"

void WeatherProp_LoadResources(KromeIni*);
void WeatherProp_Init(void);

struct WeatherProp : GameObject {
	int bEnabled;
	int unk40;
	int bWater;
	int unk48;
	int type;
	int unk50;
	int sound;
	
	virtual bool LoadLine(KromeIniLine*);
	virtual void LoadDone(void);
	virtual void Reset(void);
	virtual void Update(void);
	virtual void Message(MKMessage*);
	virtual void Init(GameObjDesc*);
	virtual void Deinit(void);
	
	void Activate(void);
	void Deactivate(void);
	void SetType(int);
};