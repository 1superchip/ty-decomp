#ifndef BOOMERANG_H
#define BOOMERANG_H

#include "ty/props/Projectile.h"
#include "common/ParticleSystem.h"
#include "ty/LevelObjective.h"
#include "common/PtrList.h"
#include "ty/Spline.h"

struct BoomerangInfo {
    char* pModelName;
    int nameTranslationId;
    int descriptionTranslationId;
    int unkC;
    int unk10;
    int unk14;
    int unk18;
};

enum BoomerangType {
    BR_Standard     = 0,
    BR_Zappyrang    = 8,
    BR_MAX          = 12,
};

enum BoomerangSound {};

struct BoomerangStaticInfo {

};

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

struct BoomerangWeapon;

struct Boomerang : GameObject {
    Vector mPos;
    int unk4C;
    BoomerangType mRangType;
    bool unk54;
    bool unk55;
    bool unk56;
    Vector mCatchPos;
    BoomerangWeapon* mpWeapon;
    int unk6C;
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
    int unkA8;
    float mag;
    float unkB0;
    char mTrail[0x40]; // BoomerangTrail
    Model* pModelUnkF4;
    ParticleSystem* pFireSys;

    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Deinit(void);
    virtual void HitWorld(Vector*, int collisionFlags);
    virtual void Init(GameObjDesc* pDesc, BoomerangWeapon* pWeapon);
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

    int PlaySound(BoomerangSound, int);

    BoomerangDesc* GetDesc(void) {
        return descr_cast<BoomerangDesc*>(pDescriptor);
    }
};

struct EndGameObjective : LevelObjective {
    virtual void Message(MKMessage* pMsg) {
        switch (pMsg->unk0) {
            case 10:
                break;
            default:
                LevelObjective::Message(pMsg);
                break;
        }
    }
};

#endif // BOOMERANG_H
