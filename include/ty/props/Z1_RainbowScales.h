#ifndef Z1RAINBOWSCALES_H
#define Z1RAINBOWSCALES_H

#include "ty/LevelObjective.h"

void Z1RainbowScaleObjective_LoadResources(KromeIni* pIni);

struct RainbowScaleObjective : LevelObjective {
    bool bStarted;

    virtual void Reset(void);
    virtual void Update(void);
    virtual void Message(MKMessage* pMessage);
    virtual void Init(GameObjDesc* pDesc);
};

#endif // Z1RAINBOWSCALES_H
