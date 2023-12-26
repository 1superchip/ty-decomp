#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "ty/GameObject.h"

enum ProjectileState {};

/// @brief Projectile Descriptor
struct ProjectileDesc : GameObjDesc {
    // No Load function, fields must be set when loading resources
    int unk84;
    float unk88;
    float unk8C;
    int groundHitSound;
    int hitSound;
    float unk98;
};

struct Projectile : GameObject {
    Vector mPosDiff;
    // {pitch, yaw, roll, rot spin speed scale}
    Vector mRot;
    Vector unk5C;
    int unk6C;
    int unk70;
    bool unk74;
    bool unk75;
    ProjectileState mState;

    virtual void Update(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    virtual void Fire(Vector*);
    virtual void UpdatePhysics(void);
    virtual void DamageTy(void);
    virtual void KnockBackTy(void);
    virtual bool ResolveHit(void);
    virtual void CheckForHit(void);
    virtual void AddSpin(void);

    void EndLife(void);
    void FireAtTarget(Vector*, Vector*);
    bool CheckShotPossible(Vector*, Vector*);
    void SetState(ProjectileState);
    void SetPos(Vector*);

    ProjectileDesc* GetDesc(void) {
        return descr_cast<ProjectileDesc*>(pDescriptor);
    }
};

#endif // PROJECTILE_H
