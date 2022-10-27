//#include "common/MKSceneManager.h"
#include "ty/GameObject.h"

struct GameObjectManager {
	GameObjDesc* pDescs;
	bool bLevelInitialised;
	void* unk8;
	int unkC;
	
	void Init(void);
	void Deinit(void);
	void InitLevel(void);
	void DeinitLevel(void);
	void LoadLevel(KromeIni*);
	void AddObject(GameObject*, Matrix*, BoundingVolume*);
	void AddObject(GameObject*, Model*);
	void RemoveObject(GameObject*);
	void UpdateModules(void);
	void DrawModules(void);
	void SendMessage(MKMessage*, uint, Vector*, float, bool);
	void SendMessageToAll(MKMessage*, int);
	GameObject* GetObjectFromID(uint);
	void AddDescriptor(GameObjDesc*);
	GameObjDesc* FindDescriptor(char*);
	void GetObjectsInRange(GameObject**, int, Vector*, float, int); // may return a value
	GameObject* GetClosestObjectInRange(Vector*, float, int);
	BeginStruct Begin(void);
	int CountEntities(KromeIni*, KromeIniLine*);
};

