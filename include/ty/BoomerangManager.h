#ifndef BOOMERANGMANAGER_H
#define BOOMERANGMANAGER_H

#include "ty/boomerang.h"
#include "ty/StateMachine.h"
#include "ty/tools.h"

struct BoomerangManager;

enum BoomerangLauncherState {
    BLS_Idle        = 0,
    BLS_Firing      = 1,
    BLS_Catching    = 2,
};

enum BoomerangSide {
    BOOMERANG_SIDE_LEFT     = 0,
    BOOMERANG_SIDE_RIGHT    = 1,
    BOOMERANG_SIDE_COUNT
};

struct BoomerangWeapon {
    StateMachine<BoomerangWeapon> mFsm;
    
    Matrix catchMatrix;
    Vector direction;
    Vector pos;
    Vector colour;

    CircularQueue<Boomerang*> ammoQueue; // ammoQueue.numMax is ammoCount

    Boomerang* mpRangs;
    BoomerangDesc* pBoomerangDesc;
    Boomerang* pBoomerang;
    BoomerangSide mSide;
    BoomerangManager* pManager;

    bool unkA4;
    bool unkA5;
    bool unkA6;
    bool unkA7;
    
    float unkA8;

    void Init(BoomerangSide side, BoomerangDesc* pDesc, BoomerangManager* _pManager);
    void Deinit(void);
    void Reset(void);
    void Draw(void);
    void DrawShadow(Vector* p);
    void DrawReflection();
    void Idle(void);
    void InitFiring(void);
    void Firing(void);
    void DeinitFiring(void);
    void InitCatching(void);
    void Catching(void);
    void DeinitCatching(void);
    void DoFire(void);
    void DoCatch(Boomerang* pRang);
    DescriptorIterator BeginAmmo(void);
    void StartCatch(Boomerang* pRang);

    bool IsOwnRang(Boomerang* pRang);
    bool IsReady(void);

    bool Fire(Vector* p, Vector* p1);
    void Update(Matrix*, Vector*);

    void Disable(void);
    void Enable(void);

    void EnableParticles(void); // No symbol
    void DisableParticles(void); // No symbol
    void DisableSounds(void); // No symbol
};

struct BoomerangManagerAnims {
    bool unk0;
    MKAnim* unk4;
    MKAnim* unk8[2];
    MKAnim* unk10[2];
};

struct BoomerangManagerInit {
    BoomerangDesc** ppBoomerangDescs;
    uint maxTypes;
    BoomerangType defaultType;
    bool unkC;
    MKAnimScript* pAnimScript;
    MKAnim* pAnim;
    MKAnim* unk18[2]; // check type
    Tools_AnimEvent animEvents[5];
    BoomerangManagerAnims mAnims;
};

struct BoomerangManager {
    BoomerangWeapon* mpWeapons[BOOMERANG_SIDE_COUNT];
    bool bShowRangs[BOOMERANG_SIDE_COUNT];
    MKAnimScript unkC[BOOMERANG_SIDE_COUNT];
    MKAnim* unk4C[BOOMERANG_SIDE_COUNT]; // check type
    BoomerangSide mCurrentSide;
    bool bEnabled;
    bool unk59;
    BoomerangType mNextType; // Next type to switch to
    BoomerangType mType; // Current type
    int numFired;
    BoomerangManagerInit init;
    BoomerangSide unkC8;
    Tools_AnimEventDesc mAnimEventDesc;
    Tools_AnimEventManager mAnimEventManager;

    void Init(BoomerangManagerInit* pInitInfo);
    void Deinit(void);
    void Reset(void);
    void Update(Matrix*, Matrix*, Vector*);
    void Draw(void);
    void DrawShadow(Vector*);
    void DrawReflection(void);
    bool Fire(Vector*, Vector*, bool bFireBoth);
    void StartThrowAnim(BoomerangSide side);
    void StartCatchAnim(BoomerangSide side);
    bool IsOwnRang(Boomerang* pRang);
    void UpdateAnimation(Model* pModel);
    void SetType(BoomerangType newType);
    void SetHasBoth(bool);
    bool HasFired(void);
    void SetHasRang(BoomerangType rangType, bool);
    bool IsReady(BoomerangSide side);
    void Show(BoomerangSide side);
    void Hide(BoomerangSide side);
    void Disable(void);
    void Enable(void);
    void SetAnims(BoomerangManagerAnims*);
    void EnableBoomerangParticles(void);
    void DisableBoomerangParticles(void);
    void DisableBoomerangSounds(void);

    BoomerangType GetCurrentType(void) {
        return mType;
    }

    void HideAll(void) {
        Hide(BOOMERANG_SIDE_LEFT);
        Hide(BOOMERANG_SIDE_RIGHT);
    }

    void ShowAll(void) {
        Show(BOOMERANG_SIDE_LEFT);
        Show(BOOMERANG_SIDE_RIGHT);
    }

    bool AreBothReady(void) {
        return bEnabled && IsReady(BOOMERANG_SIDE_LEFT) && IsReady(BOOMERANG_SIDE_RIGHT);
    }
};

#endif // BOOMERANGMANAGER_H
