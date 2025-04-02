#ifndef BOOMERANGMANAGER_H
#define BOOMERANGMANAGER_H

#include "ty/boomerang.h"

enum BoomerangSide {
    BOOMERANG_SIDE_LEFT     = 0,
    BOOMERANG_SIDE_RIGHT    = 1,
};

struct BoomerangWeapon {
    int unk0;
    int unk4;
    int unk8;
    void* pStates;
    Matrix catchMatrix;
    Vector direction;
    Vector pos;
    Vector colour;
    int unk80;
    int unk84;
    int unk88;
    Boomerang** ppBoomerangs;
    BoomerangWeapon* pWeapon;
    BoomerangDesc* pBoomerangDesc;
    Boomerang* pBoomerang;
    BoomerangSide mSide;
    void* pManager; // BoomerangManager*
    bool unkA4;
    bool unkA5;
    bool unkA6;
    bool unkA7;
    float unkA8;

    void Init(BoomerangSide side, BoomerangDesc* pDesc, BoomerangManager* pManager);
    void Idle(void);
    void InitFiring(void);
    void Firing(void);
    void DeinitFiring(void);
    void InitCatching(void);
    void Catching(void);
    void DeinitCatching(void);
    void DoFire(void);
    void DoCatch(Boomerang*);
    BoomerangWeapon* BeginAmmo(void); // check return type
    void StartCatch(Boomerang*);

    void Disable(void);
    void Enable(void);
};

struct BoomerangManagerAnims {

};

struct BoomerangManagerInit {

};

struct BoomerangManager {
    BoomerangWeapon* pWeapons[2];
    bool bHasRangs[2];
    MKAnimScript unkC[2];
    int unk4C[2];
    int unk54;
    bool bEnabled;
    bool unk59;
    BoomerangType mType;
    BoomerangType mType2;
    int numFired;
    void* unk68;
    uint weaponCount;
    BoomerangType unk70;
    int unk74;
    MKAnimScript* pAnimScript;
    int unk7C;
    int unk80[2];
    Tools_AnimEvent mAnimEvents[5];
    bool unkB0;
    MKAnim* unkB4;
    MKAnim* unkB8[2];
    MKAnim* unkC0[2];
    BoomerangSide unkC8;
    Tools_AnimEventDesc mAnimEventDesc;
    Tools_AnimEventManager mAnimEventManager;

    void Init(BoomerangManagerInit*);
    void Deinit(void);
    void Reset(void);
    void Update(Matrix*, Matrix*, Vector*);
    void Draw(void);
    void DrawShadow(Vector*);
    void DrawReflection(void);
    void Fire(Vector*, Vector*, bool);
    void StartThrowAnim(BoomerangSide);
    void StartCatchAnim(BoomerangSide);
    bool IsOwnRang(Boomerang*);
    void UpdateAnimation(Model*);
    void SetType(BoomerangType);
    void SetHasBoth(bool);
    bool HasFired(void);
    void SetHasRang(BoomerangType, bool);
    bool IsReady(BoomerangSide);
    void Show(BoomerangSide);
    void Hide(BoomerangSide);
    void Disable(void);
    void Enable(void);
    void SetAnims(BoomerangManagerAnims*);
    void EnableBoomerangParticles(void);
    void DisableBoomerangParticles(void);
    void DisableBoomerangSounds(void);

    BoomerangType GetUnk60(void) {
        return mType2;
    }

    void HideAll(void) {
        Hide((BoomerangSide)0);
        Hide((BoomerangSide)1);
    }

    void ShowAll(void) {
        Show((BoomerangSide)0);
        Show((BoomerangSide)1);
    }
};

#endif // BOOMERANGMANAGER_H
