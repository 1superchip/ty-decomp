#ifndef AQUARANG_H
#define AQUARANG_H

#include "ty/props/StaticProp.h"

struct AquarangProp : StaticProp {
    bool bCurrVisible; // Current visibility
    Blitter_Particle particle;
    float randAngle;
    float yaw;
    float yawUpdateVal;
    bool bVisible; // Default visibility setting for this object

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Message(MKMessage* pMsg);

    static Material* pGlow; // Material used for the glow
};

void AquarangProp_LoadResources(KromeIni* pIni);

#endif // AQUARANG_H
