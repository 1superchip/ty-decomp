#ifndef REFLECTION_H
#define REFLECTION_H

#include "ty/WobbleTexture.h"
#include "common/View.h"

struct ReflectionStruct {
    WobbleTexture mWobbleTex;
    View mView;
    Material* mpRefMat;
    float unk308;
    float unk30C;
    float unk310;
    bool unk314;

    void Init(void);
    void Deinit(void);
    void Render(void);
    void Draw(void);
    void Update(void);
};

#endif // REFLECTION_H
