#ifndef SHADOW_H
#define SHADOW_H

#include "common/MKShadow.h"
#include "ty/StructList.h"
#include "common/Model.h"
#include "common/Blitter.h"

void Shadow_Init(void);
void Shadow_Deinit(void);

void Shadow_DrawAnimatingShadows(void);

void Shadow_BeginDraw(void);
void Shadow_EndDraw(void);

struct Shadow {
    Model* pModel;

    float unk4;
    float unk8;
    float unkC;
    Vector unk10;
    Vector unk20;

    MKShadow* pMKShadow;

    bool bDraw;
    bool unk35;

    float unk38;
    float unk3C;
    float unk40;
    
    Vector mLightPos;

    void Init(char*, Model* pShadowModel, float, Vector*);
    void Deinit(void);

    void AddAnimatingShadow(char*, Model*, float, Vector*);

    void GetLightPos(Vector* pLightPos);

    void Draw(void);
    void Draw(Vector*, bool);

    bool CheckForShadowShift(float, Vector*, float, Vector*);
};

struct DropShadow {
    Matrix mMatrix;
    Blitter_TriStrip mStrips[4];

    void Init(Vector*, Vector*, Vector*, Vector*, float, bool);

    void SetRadius(float);
    void SetColor(Vector*);

    void Draw(void);
};

#endif // SHADOW_H
