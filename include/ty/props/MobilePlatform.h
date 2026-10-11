#ifndef MOBILEPLATFORM_H
#define MOBILEPLATFORM_H

#include "ty/props/Platform.h"
#include "ty/CommonGameObjectFlags.h"
#include "ty/Spline.h"
#include "ty/tools.h"

void MobilePlatform_LoadResources(KromeIni* pIni);

struct MobilePlatformDesc : PlatformDesc {
    float bobSpeed; // Unofficial: unkF0. Number of bobs per second
    float bobHeight; // Unofficial: unkF4

    virtual void Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags);
};

// Temporary info used while loading a MobilePlatform
struct MobilePlatformLoadInfo {
    Tools_WayPoints wayPoints; // Unofficial: unk0
    float speed;
    float startTime; // Normalised starting position on the path [0, 1]
    bool bNonCircular;
};

struct MobilePlatform : Platform {
    float turnSpeed;
    int bReverse; // Unofficial: unkD8. Set when moving backwards along a non-circular path
    int pathFrames; // Unofficial: unkDC. Number of frames to travel the path once
    int startFrame; // Unofficial: unkE0
    int defaultStartFrame; // Unofficial: unkE4
    int frameOffset; // Unofficial: unkE8
    int bobFrameOffset; // Unofficial: unkEC
    UniformSpline spline; // Unofficial: unkF0
    bool bCircular;
    bool bMoveOne; // Deactivates once the end of the path is reached
    CommonGameObjFlagsComponent gameObjFlags; // Unofficial: unk106

    static MobilePlatformLoadInfo mpfLoadInfo; // Unofficial

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
    void SetYaw(float time, float maxTurn); // Unofficial: time, maxTurn

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
