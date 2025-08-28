#ifndef SOUNDBANK_H
#define SOUNDBANK_H

#include "types.h"
#include "ty/GameObject.h"

enum MusicType {
    MUSIC_TYPE_7 = 7,
};

void SoundBank_LoadResources(void);
void SoundBank_Init(void);
void SoundBank_Deinit(void);

int SoundBank_Play(int, Vector*, uint);
void SoundBank_Stop(int*);

int SoundBank_GetID(int, unsigned int);

void SoundBank_PlayExclusiveAmbientSound(bool);

void SoundBank_StopExclusiveAmbientSound(void);

void SoundBank_PlayMusic(MusicType, float, float);

void SoundBank_PauseMusic(bool, float);

void SoundBank_PlayDialogMusic(bool);

void SoundBank_Update(void);

int SoundBank_ResolveSoundEventIndex(char*);

struct SoundEventHelper {
    int unk0;
    
    void Update(int, bool, bool, GameObject*, Vector*, float, int);
    void Message(MKMessage*);
    
    void Init(void) {
        unk0 = -1;
    }

    void Deinit(void) {
        SoundBank_Stop(&unk0);
    }
    
    void Stop(void) {
        SoundBank_Stop(&unk0);
    }

    void Reset(void) {
        SoundBank_Stop(&unk0);
    }
};

struct SoundMaterial;

struct SoundEvent {
    int unk0;
    int unk4;
    uint unk8;

    void Init(KromeIni*, KromeIniLine*);
    void Reset(void);
    int Play(Vector*, unsigned int);
    int GetID(unsigned int);
    int CountSoundMaterials(KromeIni*);
};

struct SoundMaterial {
    char padding[0x18];

    void Init(KromeIni*, unsigned char, float, float);
    int Play(Vector*, int, SoundEvent*);
    int GetID(void);
    int CountSounds(KromeIni*);
};

struct SoundEventManager {
    SoundEvent* pSoundEvents;
    char padding[0x48];

    void Init(void);
    void Reset(void);
    void ParseSoundEvents(KromeIni*);
    void BuildSoundNameTable(KromeIni*);
};

#endif // SOUNDBANK_H
