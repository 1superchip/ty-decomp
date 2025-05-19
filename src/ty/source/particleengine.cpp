#include "ty/ParticleEngine.h"
#include "ty/global.h"

static bool bParticleResourcesLoaded = false;
static bool bParticleManagerInited = false;

static TyParticleManager actualParticleManager;

TyParticleManager* particleManager = &actualParticleManager;

void TyParticleManager::LoadResources(void) {
    bParticleResourcesLoaded = true;

    gb.pTensionMat = Material::Create("tension");
    gb.pShadowMat = Material::Create("shadow");
    gb.pBlackSquareMat = Material::Create("black");
}

void TyParticleManager::Init(void) {
    bParticleManagerInited = true;
}

void TyParticleManager::Deinit(void) {

}

void TyParticleManager::SpawnSpark(Vector* pPos) {

}

void TyParticleManager::SpawnAnts(Vector* p, Vector* p1, Vector* p2) {

}

void TyParticleManager::SpawnChomp(Vector* p, float f1) {

}

void TyParticleManager::SpawnExclamation(void) {

}

void TyParticleManager::StopExclamation(bool b) {

}

void TyParticleManager::SpawnGhost(Vector* p) {
    
}

void TyParticleManager::DrawPreWater(void) {
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);


}

void TyParticleManager::DrawPostWater(void) {
    View::GetCurrent()->SetLocalToWorldMatrix(NULL);

    
}
