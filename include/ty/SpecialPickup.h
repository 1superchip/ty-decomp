#ifndef SPECIALPICKUP_H
#define SPECIALPICKUP_H

#include "ty/GameObject.h"
#include "ty/RangeCheck.h"
#include "ty/GameEnums.h"

void SpecialPickup_HideAll(void);
void SpecialPickup_ShowAll(void);

void SpecialPickup_LoadResources(KromeIni* pIni);

enum SpecialPickupState {
    SpecialPickupState_0 = 0,
    SpecialPickupState_1 = 1,
    SpecialPickupState_2 = 2,
    SpecialPickupState_3 = 3,
    SpecialPickupState_4 = 4,
    SPS_Collected = 5,
};

#include "ty/props/Platform.h"

enum SpecialPickupType {
    SPT_ThunderEgg  = 0,
    SPT_GoldenCog   = 1
};

struct SpecialPickupStruct : GameObject {
    Vector pos;
    Vector unk4C;

    bool bInitiallyVisible;
    bool unk5D;
    bool unk5E;

    SpecialPickupType type;
    int subType; // GoldenCogType or ThunderEggType
    int subState;

    float unk6C;

    float unk70;
    float unk74;

    float unk78;
    
    Quadratic mQuadratic;

    SpecialPickupState state;

    int unk[2];
    
    LODManager mLodManager;
    
    Vector cameraSource;
    Vector cameraTarget;
    Vector camSrc;
    Vector camTarget;
    Vector cameraDir;
    float unk124;

    EventMessage mEventMessage;
    PlatformRider mRider;

    
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Reset(void);
    virtual void Message(MKMessage* pMsg);

    Vector* GetPos(void) {
        return pModel->matrices[0].Row3();
    }
    
    void Idle(void);
    void Collecting(void);
    void SetState(SpecialPickupState, bool);
    void Thrown(void);
    void Controlled(void);
    void ScaleIn(void);
    void ScaleOut(void);
    void SetShowPos(Vector*);
    void UpdateShadow(float);
    bool IsCollected(void);
    void SetCollected(bool);
    void SetTransparent(bool);
};

struct SpecialPickupMessage : MKMessage {
    void Init(void);
};

void SpecialPickup_EnableCheatLines(bool);

SpecialPickupStruct* GetThunderEgg(ThunderEggType type);
SpecialPickupStruct* GetGoldenCog(GoldenCogType);

#endif // SPECIALPICKUP_H
