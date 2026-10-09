#ifndef MOBILEPLATFORM_H
#define MOBILEPLATFORM_H

#include "ty/props/Platform.h"
#include "ty/CommonGameObjectFlags.h"
#include "ty/Spline.h"
#include "ty/tools.h"

void MobilePlatform_LoadResources(KromeIni* pIni);

struct MobilePlatformDesc : PlatformDesc {
    float bobSpeed; // Number of bobs per second
    float bobHeight;

    virtual void Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags);
};

// Temporary info used while loading a MobilePlatform
struct MobilePlatformLoadInfo {
    Tools_WayPoints wayPoints;
    float speed;
    float startTime; // Normalised starting position on the path [0, 1]
    bool bNonCircular;
};

struct MobilePlatform : Platform {
    float turnSpeed;
    int bReverse; // Set when moving backwards along a non-circular path
    int pathFrames; // Number of frames to travel the path once
    int startFrame;
    int defaultStartFrame;
    int frameOffset;
    int bobFrameOffset;
    UniformSpline spline;
    bool bCircular;
    bool bMoveOne; // Deactivates once the end of the path is reached
    CommonGameObjFlagsComponent gameObjFlags;

    static MobilePlatformLoadInfo mpfLoadInfo;

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
    void UpdateMove(void);
    void UpdateBob(void);
    float GetTime(void);
    void SetYaw(float time, float maxTurn);

    MobilePlatformDesc* GetDesc(void) {
        return descr_cast<MobilePlatformDesc*>(pDescriptor);
    }

    // Returns the number of frames of a full cycle
    // Non-circular paths travel forwards then backwards
    uint GetCycleFrames(void) {
        return pathFrames * (!bCircular + 1);
    }
};

#endif // MOBILEPLATFORM_H
