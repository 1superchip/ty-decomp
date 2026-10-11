#ifndef TURNINGPLATFORM_H
#define TURNINGPLATFORM_H

#include "ty/props/Platform.h"
#include "ty/tools.h"

void TurningPlatform_LoadResources(KromeIni* pIni);

// A platform that turns to face its next waypoint when hit by a boomerang or activated
struct TurningPlatform : Platform {
    float angles[MAX_WAYPOINTS]; // Unofficial: unkD4. Yaw facing each waypoint from the first waypoint
    int numAngles; // Unofficial: unk114
    int targetAngle; // Unofficial: unk118
    int currAngle; // Unofficial: unk11C
    float turnSpeed;
    int turnStartFrame; // Unofficial: unk124

    static Tools_WayPoints loadPoints;

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Message(MKMessage* pMsg);
    virtual void Init(GameObjDesc* pDesc);
    void TurnToNext(void);
};

#endif // TURNINGPLATFORM_H
