#include "types.h"
#include "common/Vector.h"
#include "common/DirectLight.h"

// operator new is from System_GC.cpp?

DirectLight* DirectLight::pDefaultLight;

static Vector defaultLightSetup[7] = {{-0.2f, -1.0f, 0.7f, 0.0f}, {0.6f, 0.6f, 0.6f, 0.0f}, 
								{0.2f, -1.0f, 0.5f, 0.0f}, {0.6f, 0.6f, 0.6f, 0.0f}, 
								{0.0f, 1.0f, 0.0f, 0.0f}, {0.2f, 0.2f, 0.2f, 0.0f}, {0.3f, 0.3f, 0.3f, 0.0f}};

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
    pad[8].Normalise(pLightArray);
    pad[11] = pLightArray[1];
    pad[9].Normalise(&pLightArray[2]);
    pad[12] = pLightArray[3];
    pad[10].Normalise(&pLightArray[4]);
    pad[13] = pLightArray[5];
    pad[14] = pLightArray[6];
    RecalcMatrices();
}

void DirectLight::SetLight(int lightIndex, Vector* pLight, Vector* pColour) {
    lightArray[lightIndex].Normalise(pLight);
    lightArray[lightIndex + 3] = *pColour;
    RecalcMatrices();
}

void DirectLight::SetAmbient(Vector* pAmbient) {
    ambient = *pAmbient;
    RecalcMatrices();
}

void DirectLight::RecalcMatrices(void) {
    pad[0].x = -pad[8].x;
    pad[0].y = -pad[9].x;
    pad[0].z = -pad[10].x;
    pad[0].w = 0.0f;

    pad[1].x = -pad[8].y;
    pad[1].y = -pad[9].y;
    pad[1].z = -pad[10].y;
    pad[1].w = 0.0f;

    pad[2].x = -pad[8].z;
    pad[2].y = -pad[9].z;
    pad[2].z = -pad[10].z;
    pad[2].w = 0.0f;

    pad[3].x = 0.0f;
    pad[3].y = 0.0f;
    pad[3].z = 0.0f;
    pad[3].w = 1.0f;

    pad[4].x = pad[11].x;
    pad[4].y = pad[11].y;
    pad[4].z = pad[11].z;
    pad[4].w = 0.0f;

    pad[5].x = pad[12].x;
    pad[5].y = pad[12].y;
    pad[5].z = pad[12].z;
    pad[5].w = 0.0f;

    pad[6].x = pad[13].x;
    pad[6].y = pad[13].y;
    pad[6].z = pad[13].z;
    pad[6].w = 0.0f;

    pad[7].x = ambient.x;
    pad[7].y = ambient.y;
    pad[7].z = ambient.z;
    pad[7].w = 1.0f;
}