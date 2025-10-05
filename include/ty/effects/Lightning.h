#ifndef LIGHTNING_H
#define LIGHTNING_H

#include "common/View.h"
#include "common/Model.h"

struct LightningInit {
    char** modelNames;
    int numModels;
    int numFlashes;
    float minSpawnDelay;
    float maxSpawnDelay;
    Vector minColor;
    Vector maxColor;
    float innerRadius;
    float outerRadius;
    float numSecsFlashOff;
    float numSecsFlashOn;

    void Init(void); // Stripped
    // void CreateDebugMenus(DebugOptionMenu*);
};

struct Lightning {
    LightningInit* pInit;
    bool bActive;
    int nextActivationChange;
    int minFramesDelay;
    int maxFramesDelay;
    int numFlashes;
    int numFramesOff;
    int numFramesOn;
    int randNum;
    int randNum1;
    Model** ppModels;
    Vector basePos;
    float height;
    Vector lightningPos;

    void Init(LightningInit* pNewInit);
    void Deinit(void);
    void Update(void);
    void Draw(View* pView);
    void DrawDebugInfo(View* pView);
    void ResetActivation(void);
    void ChangeActivation(void);
    void CalcNextSpawnPoint(void);
};

#endif // LIGHTNING_H
