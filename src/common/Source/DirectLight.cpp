#include "types.h"
#include "common/Vector.h"
#include "common/DirectLight.h"

// operator new is from System_GC.cpp?

DirectLight* DirectLight::pDefaultLight;

// Contains Info for 3 lights and an ambient light
// Light info is {{dir}, {color}}
static Vector defaultLightSetup[7] = {
    {-0.2f, -1.0f, 0.7f, 0.0f}, {0.6f, 0.6f, 0.6f, 0.0f}, // Light 0
    {0.2f, -1.0f, 0.5f, 0.0f}, {0.6f, 0.6f, 0.6f, 0.0f}, // Light 1
    {0.0f, 1.0f, 0.0f, 0.0f}, {0.2f, 0.2f, 0.2f, 0.0f}, // Light 2
    {0.3f, 0.3f, 0.3f, 0.0f} // Ambient Light
}; // __attribute__((aligned(16)))

void DirectLight::InitModule(void) {
    pDefaultLight = new DirectLight;
    pDefaultLight->Init();
}

void DirectLight::DeinitModule(void) {
    delete pDefaultLight;
    pDefaultLight = NULL;
}

void DirectLight::Init(void) {
    Set(&defaultLightSetup[0]);
}

void DirectLight::Set(Vector* pLightArray) {

    mLightDirs[0].Normalise(&pLightArray[0]);
    mLightColors[0] = pLightArray[1];

    mLightDirs[1].Normalise(&pLightArray[2]);
    mLightColors[1] = pLightArray[3];
    
    mLightDirs[2].Normalise(&pLightArray[4]);
    mLightColors[2] = pLightArray[5];

    mAmbient = pLightArray[6];
    RecalcMatrices();
}

void DirectLight::SetLight(int lightIndex, Vector* pLight, Vector* pColour) {
    mLightDirs[lightIndex].Normalise(pLight);
    mLightColors[lightIndex] = *pColour;
    RecalcMatrices();
}

void DirectLight::SetAmbient(Vector* pAmbient) {
    mAmbient = *pAmbient;
    RecalcMatrices();
}

void DirectLight::RecalcMatrices(void) {
    mDirMatrix.data[0][0] = -mLightDirs[0].x;
    mDirMatrix.data[0][1] = -mLightDirs[1].x;
    mDirMatrix.data[0][2] = -mLightDirs[2].x;
    mDirMatrix.data[0][3] = 0.0f;

    mDirMatrix.data[1][0] = -mLightDirs[0].y;
    mDirMatrix.data[1][1] = -mLightDirs[1].y;
    mDirMatrix.data[1][2] = -mLightDirs[2].y;
    mDirMatrix.data[1][3] = 0.0f;

    mDirMatrix.data[2][0] = -mLightDirs[0].z;
    mDirMatrix.data[2][1] = -mLightDirs[1].z;
    mDirMatrix.data[2][2] = -mLightDirs[2].z;
    mDirMatrix.data[2][3] = 0.0f;

    mDirMatrix.data[3][0] = 0.0f;
    mDirMatrix.data[3][1] = 0.0f;
    mDirMatrix.data[3][2] = 0.0f;
    mDirMatrix.data[3][3] = 1.0f;

    mNewColors[0].x = mLightColors[0].x;
    mNewColors[0].y = mLightColors[0].y;
    mNewColors[0].z = mLightColors[0].z;
    mNewColors[0].w = 0.0f;

    mNewColors[1].x = mLightColors[1].x;
    mNewColors[1].y = mLightColors[1].y;
    mNewColors[1].z = mLightColors[1].z;
    mNewColors[1].w = 0.0f;

    mNewColors[2].x = mLightColors[2].x;
    mNewColors[2].y = mLightColors[2].y;
    mNewColors[2].z = mLightColors[2].z;
    mNewColors[2].w = 0.0f;

    mNewAmbient.x = mAmbient.x;
    mNewAmbient.y = mAmbient.y;
    mNewAmbient.z = mAmbient.z;
    mNewAmbient.w = 1.0f;
}
