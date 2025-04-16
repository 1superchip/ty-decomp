#ifndef PICTURE_H
#define PICTURE_H

#include "ty/props/StaticProp.h"
#include "ty/Quadratic.h"

void Picture_LoadResources(KromeIni* pIni);

/// @brief Picture object dropped by InvisiCrate
struct Picture : StaticProp {
    Vector pos;
    float unk68;
    float mQuadraticTime; // time for quadratic equation
    float unk70;
    float unk74;
    float angle;
    int mFrameNumber;
    bool unk80; // bInitialised is set to this when the Picture is reset
    bool bInitialised;
    bool bShow; // TODO: Rename / not bShow but bCollected(?)
    bool unk83;
    Quadratic mQuadratic; // Quadratic equation for falling movement

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Init(GameObjDesc* pDesc);

    static Picture* Create(Vector* pPos);
    void Spawn(int frameNumber, Vector* pPos);
    void Destroy(void);

    static void PostDraw(void*);

    bool IsShowing(void) {
        return bShow;
    }
};

#endif // PICTURE_H
