#ifndef RAIN_H
#define RAIN_H

#include "common/Model.h"
#include "common/View.h"
#include "common/Vector.h"

struct RainInit {
    float alpha;
    char* pModelName;
    Vector color;
    char* pMaterialName;
    float materialScrollSpeed;

    void Init(void); // Stripped
    // void CreateDebugMenus(DebugOptionMenu*);
};

struct Rain {
    RainInit* pInit;
    Model* pModel;
    Material* pMaterial;
    float unkC;
    
    void Init(RainInit* pNewInit);
    void Deinit(void);
    void Update(void);
    void Draw(View* pView, bool);
    void DrawDebugInfo(View* pView);
};

#endif // RAIN_H
