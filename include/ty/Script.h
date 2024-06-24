#include "ty/GameObject.h"
#include "ty/Messages.h"

#define MessageCount 10

struct ScriptProp : GameObject {
    virtual bool LoadLine(KromeIniLine*);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Message(MKMessage*);
    virtual void Init(GameObjDesc*);
    void Execute(void);
    
    float Delay;
    float unk40;
    bool bActive;
    bool bDelayEachMessage;
    bool bEnabled;
    bool bDefaultEnabled; // Default Enable/Disable setting used when prop is reset
    char currentMessageIndex;
    EventMessage messages[MessageCount];
    // size 0x9C
};

struct ConditionalScriptProp : ScriptProp {
    virtual bool LoadLine(KromeIniLine*);
    virtual void Message(MKMessage*);
    virtual void Init(GameObjDesc*);
    bool CheckConditions(void);
    
    int condition;
    bool bExecuteOnStart;
    bool bNegative;
    // size 0xA4
};

#define Conditon_LearntToSwim 0
#define Conditon_HasBothRangs 1
#define Conditon_GameComplete 9
#define Conditon_DefaultTrue  16