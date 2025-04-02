#ifndef BOOMERANG_H
#define BOOMERANG_H

#include "ty/props/Projectile.h"
#include "common/ParticleSystem.h"
#include "ty/LevelObjective.h"
#include "common/PtrList.h"
#include "ty/Spline.h"
#include "ty/effects/Trails.h"
#include "ty/effects/Flame.h"
#include "ty/effects/Bubble.h"
#include "ty/effects/ChronorangEffects.h"
#include "ty/effects/Explosion.h"
#include "ty/soundbank.h"

struct BoomerangStaticInfo {
    char* pModelName;
    int nameTranslationId;
    int descriptionTranslationId;
    int sounds[4];
};

enum BoomerangType {
    BR_Standard     = 0, // Boomerang
    BR_Frostyrang   = 1,
    BR_Flamerang    = 2,
    BR_Kaboomerang  = 3,
    BR_4            = 4,
    BR_Megarang     = 5,
    BR_Zoomerang    = 6,
    BR_Infrarang    = 7,
    BR_Zappyrang    = 8,
    BR_Aquarang     = 9,
    BR_Multirang    = 10,
    BR_Chronorang   = 11,
    BR_MAX
};

#define NUM_BOOMERANGS (BR_MAX)

enum BoomerangSound {};

struct BoomerangDesc : GameObjDesc {
    char trailMatName[0x20];
    int ammoCount;
    int unkA8;
    int unkAC;
    float range;
    float flightTime;
    BoomerangType mType;
    BoomerangStaticInfo* mpStaticInfo;

    void Init(ModuleInfoBase* pMod, char*, char*, BoomerangType, BoomerangStaticInfo* pStaticInfo);
    virtual void Load(KromeIni* pIni);
};

struct DoomerangDesc : BoomerangDesc {
    float pitchSpeed;
    float turnSpeed;
    float speed;
    
    virtual void Load(KromeIni* pIni);
};

struct Boomerang;
struct BoomerangManager;
struct BoomerangWeapon;

// Made up
enum BoomerangState {
    BOOMERANG_STATE_0 = 0,
    BOOMERANG_STATE_1 = 1,
    BOOMERANG_STATE_2 = 2,
};

struct Boomerang : GameObject {
    Vector mOldPos; // Previous position
    int mFireTimer; // Fire particle timer
    BoomerangType mRangType;
    bool unk54; // bHitWorld?
    bool bParticlesEnabled;
    bool unk56;
    Vector mPos; // Current position
    BoomerangWeapon* mpWeapon;
    BoomerangState unk6C;
    Spline mSpline;
    float time;
    float unk84;
    bool unk88;
    bool unk89;
    int rang01Idx;
    int rang02Idx;
    int unk94;
    float unk98;
    float unk9C;
    int attachIdx;
    int subObjectIndex;
    SoundEventHelper unkA8;
    float mag;
    float unkB0;
    BoomerangTrail mTrail;
    Model* pModelUnkF4;
    ParticleSystem* pFireSys;

    virtual void HitWorld(Vector* pPos, int collisionFlags);
    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void Deinit(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Reset(void);
    virtual void Load(void);
    virtual void Fire(Vector*, Vector*);
    virtual void Deactivate(void);

    virtual void UpdateParticleEffect(void) {
        // inline, weak
    }

    virtual void CollideWithEnvironment(CollisionResult* pCr) {
        // inline, weak
    }
    
    virtual void StopSounds(void);
    virtual void InitIdle(void);
    virtual void UpdateIdle(void);
    virtual void InitLoaded(void);
    virtual void UpdateLoaded(void);
    virtual void InitFired(void);
    virtual void UpdateFired(void);

    void UpdateColor(void);
    void UpdateTrail(void);
    void HandleFire(void);

    int PlaySound(BoomerangSound, int collisionFlags);

    void DoDeflection(void);

    Vector* GetCatchPos(void);

    void DrawReflection(void);
    void DrawShadow(Vector*);

    void UpdateSplineEndPosition(void);

    bool SendHitMessage(CollisionResult* pCr);

    void SetOrientation(float, float, float);
    void OrientTowardsCamera(float weight);
    void ScaleRang(void);

    void DoCurvedTargeting(float);

    BoomerangDesc* GetDesc(void) {
        return descr_cast<BoomerangDesc*>(pDescriptor);
    }
};

struct Flamerang : Boomerang {
    ParticleSystem* mpFlameSys;
    bool bSpawnFlameParticles; // Spawn extra flame particles (Trigger by gasjets)
    Flame mFlame;

    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void Deinit(void);
    virtual void UpdateParticleEffect(void);
    virtual void HitWorld(Vector* pPos, int collisionFlags);
    virtual void Deactivate(void);

    void ModifyRangeInFlight(float);

    void EnableFlameParticles(void) {
        bSpawnFlameParticles = true;
    }

    void DisableFlameParticles(void) {
        bSpawnFlameParticles = false;
    }

    bool AreFlameParticlesEnabled(void) {
        return bSpawnFlameParticles;
    }
};

struct Frostyrang : Boomerang {
    ParticleSystem* mpParticleSys;

    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void Deinit(void);
    virtual void HitWorld(Vector* pPos, int collisionFlags);
    virtual void UpdateParticleEffect(void);
    virtual void CollideWithEnvironment(CollisionResult* pCr);
};

struct Aquarang : Boomerang {
    Bubble mBubble;

    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void InitFired(void);
    virtual void UpdateFired(void);
    virtual void UpdateParticleEffect(void);
    virtual void Deinit(void);
};

struct Zappyrang : Boomerang {
    Vector unkFC;

    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void UpdateParticleEffect(void);
    virtual void CollideWithEnvironment(CollisionResult* pCr);
};

struct Megarang : Boomerang {
    int unkFC;
    float unk100;
    float unk104;
    Vector* unk108;
    int unk10C;

    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void HitWorld(Vector* pPos, int collisionFlags);
    virtual void UpdateLoaded(void);
    virtual void Fire(Vector*, Vector*);
    virtual void UpdateFired(void);
    virtual void Reset(void);

    void UpdateEndPosition(void);
};

struct Kaboomerang : Boomerang {
    char unkFC[0x4];
    float boomerangCatchDistance;
    float mKaboomerangFlightTime;
    Kaboomerang* mpOtherKaboomerang;
    Explosion mExplosion;
    Vector mCentrePos; // Center position of kaboomerangs

    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void Deinit(void);
    virtual void Fire(Vector*, Vector*);
    virtual void UpdateFired(void);
};

struct Doomerang : Boomerang {
    Vector unkFC;
    Vector unk10C;
    Vector unk11C;
    float unk12C;
    float unk130;
    float unk134;
    Explosion mExplosion;
    Vector velocity;
    bool unk538;
    bool unk539;
    SoundEventHelper mSoundHelper;
    
    virtual void StopSounds(void);
    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void Deinit(void);
    virtual void Fire(Vector*, Vector*);
    virtual void InitFired(void);
    virtual void UpdateFired(void);
    virtual void UpdateParticleEffect(void);
    virtual void Deactivate(void);
    
    void ResetEndGame(void);
    void CheckForEnteringRegions(void);

    DoomerangDesc* GetDesc(void) {
        return descr_cast<DoomerangDesc*>(pDescriptor);
    }
};

struct Chronorang : Boomerang {
    Vector unkFC;
    Vector mVel;
    Pollen mPollen;

    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
    virtual void Deinit(void);
    virtual void UpdateParticleEffect(void);
};

#define MKMSG_BoomerangMsg (8)
#define MKMSG_ExplosionMsg (9)

struct BoomerangMessage : MKMessage {
    Boomerang* pBoomerang;
    bool unk8;

    void SetUnk8(bool b) {
        unk8 = b;
    }
};

struct ExplosionMessage : MKMessage {
    Vector pos;

    void Init(Vector* pPos) {
        unk0 = MKMSG_ExplosionMsg;
        pos = *pPos;
    }
};


Boomerang* Boomerang_CheckForHitSphere(Vector* pVec, float radius, bool r4);
Boomerang* Boomerang_CheckForHit(Model* pModel, int subobjectIdx, CollisionResult* pCr);
Boomerang* Boomerang_CheckForHit(Vector* p, float f1, float f2, float f3);

Doomerang* Boomerang_GetInAirDoomerang(void);

struct EndGameObjective : LevelObjective {
    virtual void Message(MKMessage* pMsg);
};

#endif // BOOMERANG_H
