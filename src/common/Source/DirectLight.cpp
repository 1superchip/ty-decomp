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
    pad[0].x = -mLightDirs[0].x;
    pad[0].y = -mLightDirs[1].x;
    pad[0].z = -mLightDirs[2].x;
    pad[0].w = 0.0f;

    pad[1].x = -mLightDirs[0].y;
    pad[1].y = -mLightDirs[1].y;
    pad[1].z = -mLightDirs[2].y;
    pad[1].w = 0.0f;

    pad[2].x = -mLightDirs[0].z;
    pad[2].y = -mLightDirs[1].z;
    pad[2].z = -mLightDirs[2].z;
    pad[2].w = 0.0f;

    pad[3].x = 0.0f;
    pad[3].y = 0.0f;
    pad[3].z = 0.0f;
    pad[3].w = 1.0f;

    pad[4].x = mLightColors[0].x;
    pad[4].y = mLightColors[0].y;
    pad[4].z = mLightColors[0].z;
    pad[4].w = 0.0f;

    pad[5].x = mLightColors[1].x;
    pad[5].y = mLightColors[1].y;
    pad[5].z = mLightColors[1].z;
    pad[5].w = 0.0f;

    pad[6].x = mLightColors[2].x;
    pad[6].y = mLightColors[2].y;
    pad[6].z = mLightColors[2].z;
    pad[6].w = 0.0f;

    pad[7].x = mAmbient.x;
    pad[7].y = mAmbient.y;
    pad[7].z = mAmbient.z;
    pad[7].w = 1.0f;
}
