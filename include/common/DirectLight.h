#ifndef COMMON_DIRECTLIGHT
#define COMMON_DIRECTLIGHT

#include "common/Vector.h"
#include "common/Matrix.h"

#define NUM_DIRECTLIGHT_LIGHTS (3)

struct DirectLight {
    Matrix mDirMatrix;
    
    Vector mNewColors[NUM_DIRECTLIGHT_LIGHTS];
    Vector mNewAmbient;

    Vector mLightDirs[NUM_DIRECTLIGHT_LIGHTS];
    Vector mLightColors[NUM_DIRECTLIGHT_LIGHTS];
    Vector mAmbient;
    
    static void InitModule(void);
    static void DeinitModule(void);
    void Init(void);
    void Set(Vector*);
    void SetLight(int, Vector*, Vector*);
    void SetAmbient(Vector*);
    void RecalcMatrices(void);
    
    static DirectLight* pDefaultLight;

    static DirectLight* GetDefault(void) {
        return pDefaultLight;
    }
};

#endif // COMMON_DIRECTLIGHT
