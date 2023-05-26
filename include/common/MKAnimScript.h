#ifndef COMMON_MKANIMSCRIPT
#define COMMON_MKANIMSCRIPT

#include "common/Animation.h"
#include "types.h"

#define CycleType_Stop 0
#define CycleType_Loop 1
#define CycleType_Rebound 2

void MKAnimScript_InitModule(void);
void MKAnimScript_DeinitModule(void);

struct AnimEvent {
    char* unk0_string;
    short startEventFrame;
    short endEventFrame;
};

struct AnimRange {
    s16 startFrame;
    s16 endFrame;
    s16 unk4; // speed
    s16 nmbrOfEvents;
    AnimEvent* pEvents;
    int unkC;
};

struct MKAnim {
    char* unk0;
    s16 nmbrOfRanges;
    s16 unk6;
    u16 unk8;
    u8 cycleType;
    u8 unkB;
    AnimRange* pAnimRanges;
};

struct MKAnimTemplateSection {
    char* pMeshName;
    char* pAnimName;
    u16 animCount;
    u16 unkA;
    u16 unkC;
    MKAnim anims[];
};

struct MKAnimScriptTemplate {
    char name[0x20];
    int referenceCount;
    int templateSize;
    int unk28;
    int unk2C;
    MKAnimTemplateSection* pSection;

    void UnpackTemplate(void);
};

struct MKAnimScript {
    MKAnimScriptTemplate* pTemplate;
    MKAnim* currAnim;
    MKAnim* nextAnim;
    float unkC;
    float unk10;
    float unk14;
    short unk18;
    short unk1A;
    short unk1C;
    u16 unk1E;
	
	static float advanceAmount;

    void Init(char*);
	void Init(MKAnimScript*);
    void Deinit(void);
	MKAnim* GetAnim(char*);
	MKAnim* GetAnim(int);
	bool Exists(char*);
	char* GetMeshName(void);
	char* GetAnimName(void);
	void SetAnim(MKAnim*);
	void TweenAnim(MKAnim*, short);
	void Animate(void);
	void Apply(Animation*);
	void ApplyNode(Animation*, int);
	char* GetEventByName(char*);
	char* GetEvent(int);
	bool HasLooped(void);
	int UpdatesUntilFinished(void);
	int GetLength(void);
	void GetStartAndEnd(MKAnim*, short*, short*);
	void SetAnimKeepingPosition(MKAnim*);
	float GetNormalPosition(void);
	float GetFrameOfNormalPosition(float, MKAnim*);
	void SetAnimNormalised(MKAnim*, float);

    // Initialises fields
    void Init(void) {
        nextAnim = NULL;
        currAnim = NULL;
        unk14 = unkC = unk10 = 0.0f;
        unk18 = 0;
        unk1A = 0;
        unk1E = 0;
        unk1C = 0;
    }
    void Reset(void) {
        nextAnim = NULL;
        currAnim = NULL;
        unkC = unk10 = unk14 = 0.0f;
        unk18 = 0;
        unk1C = 0;
        unk1A = 0;
    }
};

#endif // COMMON_MKANIMSCRIPT