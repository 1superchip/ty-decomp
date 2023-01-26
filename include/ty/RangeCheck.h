#ifndef RANGECHECK_H
#define RANGECHECK_H

#include "common/Model.h"
#include "common/KromeIni.h"

#define LODFlags_Alpha 1
#define LODFlags_CameraFade 2
#define LODFlags_Scissor 4
#define LODFlags_AlphaProp 8

void LOD_Deinit(void);
int Range_WhichZone(Vector*, float*);
bool Range_IsVisible(Vector*);
void Range_ModelSetAlpha(Model*, int, float, float, float, float, int);
void Range_Update(void);

struct LODEntry {
	char name[0x20];
	int subObjectFlags;
	int subObjectIndex;
	int unk28;
	
	void Init(Model*);
	bool CheckFlags(int);
};

struct LODDescriptor {
	int flags;
	LODEntry* pEntries;
	int nmbrOfEntries;
	int shadowFlags;
	int particleFlags;
	int soundFlags;
	int scissorFlags;
	float radius;
	float height;
	int invisibleZone;
	float minalpha;
	float maxScissorDist;
	
	void Init(KromeIni*, char*);
	void ParseIni(KromeIni*, KromeIniLine*);
	LODEntry* GetEntryFromString(char*);
	void ResolveSubObjects(Model*);
	void ReplicateLODData(int, int);
};

struct LODManager {
	LODDescriptor* pDescriptor;
	int subobjectEnableFlags;
	int unk8;
	
	void Init(Model*, int, LODDescriptor*);
	void InternalUpdate(Model*, int, float);
	bool Draw(Model*, int, float, float, bool);
	void EnableSubObjects(Model*);
	bool TestLOD(int testFlag) {
		return testFlag & ((subobjectEnableFlags >= 0) ? (1 << subobjectEnableFlags) : 0x40000000);
	}
};

#endif // RANGECHECK_H