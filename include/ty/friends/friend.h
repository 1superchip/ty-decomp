#ifndef FRIEND_H
#define FRIEND_H

#include "ty/props/Platform.h"
#include "common/MKAnimScript.h"
#include "ty/ImmediateFSM.h"

// Friend State Flags
#define FSF_Active      (1)
#define FSF_Visible     (2)
#define FSF_Enabled     (4)
#define FSF_Unknown8    (8) // this is always set in LoadDone

#define FSF_DEFAULT_FLAGS (FSF_Active | FSF_Visible)

void Friend_LoadResources(KromeIni* pIni);

/// @brief Descriptor for Friend class
struct FriendDescriptor : GameObjDesc {
    MKAnimScript mAnimScript;
    LODDescriptor mLodDesc;
    int unkD4;

    virtual void Init(ModuleInfoBase* pMod, char* pMdlName,
        char* pDescrName, int _searchMask, int _flags);
    virtual void Load(KromeIni* pIni);
};


/// @brief Base Class for Friend Objects such as Julius, Lenny, Shazza
struct Friend : GameObject {
    Vector mStartPos;
    Vector mSaveRot;
    Vector mPos;
    Vector mRot;
    Vector mVel;
    Vector mCollisionNormal;
    float mTyDistSq;
    uint mFlags; // Current flags (in use flags)
    uint mDefFlags; // Default flags
    ImmediateFSM<Friend> mStateManager;
    CollisionInfo mCollisionInfo;
    MKAnimScript mAnimScript;
    LODManager mLodManager;
    int unkActorIdx;
    PlatformRider mPlatformRider;
    int unk100;
    int mFlashTimer; // Flash timer for Zappyrang flash
    bool bFlashSkeleton; // Whether the Friend should flash or not

    // Declaration reorder needed to match vtable placement
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Message(MKMessage* pMsg);

    // Returns current position
    inline virtual Vector* GetPos(void) {
        return &mPos;
    }
    // Returns original position
    inline virtual Vector* GetStartPos(void) {
        return &mStartPos;
    }
    // Returns velocity
    inline virtual Vector* GetVel(void) {
        return &mVel;
    }

    void PreUpdate(void);
    void PostUpdate(void);

    // Not sure this is ever called?
    inline void SetState(int state, bool bAlways) {
        mStateManager.SetState(state, bAlways);
    }
    inline int GetState(void) {
        return mStateManager.prevState;
    }
    inline FriendDescriptor* GetDesc(void) {
        return descr_cast<FriendDescriptor*>(pDescriptor);
    }
};

#endif // FRIEND_H
