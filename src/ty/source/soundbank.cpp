#include "ty/soundbank.h"

static SoundEventManager soundEventManager;

void SoundBank_LoadResources(void) {
    soundEventManager.Init();
}

void SoundBank_Init(void) {

}

void SoundBank_Deinit(void) {

}

int SoundBank_Play(int r3, Vector* r4, uint flags) {
    if (soundEventManager.pSoundEvents && soundEventManager.pSoundEvents[r3].unk8) {
        return soundEventManager.pSoundEvents[r3].Play(r4, flags);
    }
    
    return -1;
}

void SoundBank_Stop(int* r3) {

}

int SoundBank_GetID(int r3, unsigned int r4) {
    if (soundEventManager.pSoundEvents) {
        return soundEventManager.pSoundEvents[r3].GetID(r4);
    }
    
    return -1;
}

void SoundBank_PlayExclusiveAmbientSound(bool r3) {

}

void SoundBank_StopExclusiveAmbientSound(void) {

}

void SoundBank_PlayMusic(MusicType type, float f1, float f2) {

}

void SoundBank_PauseMusic(bool r3, float f1) {

}

void SoundBank_PlayDialogMusic(bool r3) {

}

void SoundBank_Update(void) {

}

void SoundEventManager::Init(void) {

}

void SoundEventManager::Reset(void) {
    
}

void SoundEventManager::ParseSoundEvents(KromeIni* pIni) {
    
}

void SoundEventManager::BuildSoundNameTable(KromeIni* pIni) {
    
}

void SoundEvent::Init(KromeIni* pIni, KromeIniLine* pLine) {

}

void SoundEvent::Reset(void) {

}

int SoundEvent::Play(Vector*, unsigned int) {

}

int SoundEvent::GetID(unsigned int r4) {

}

int SoundEvent::CountSoundMaterials(KromeIni* pIni) {

}

void SoundMaterial::Init(KromeIni*, unsigned char, float, float) {

}

int SoundMaterial::Play(Vector*, int, SoundEvent*) {

}

int SoundMaterial::GetID(void) {

}

int SoundMaterial::CountSounds(KromeIni* pIni) {
    
}

int SoundBank_ResolveSoundEventIndex(char* pName) {
    
}
