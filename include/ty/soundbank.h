#ifndef SOUNDBANK_H
#define SOUNDBANK_H

#include "types.h"
#include "ty/GameObject.h"

enum MusicType {
    MUSIC_TYPE_0 = 0,
    MUSIC_TYPE_1 = 1,
    MUSIC_TYPE_2 = 2,
    MUSIC_TYPE_3 = 3,
    MUSIC_TYPE_4 = 4,
    MUSIC_TYPE_5 = 5,
    MUSIC_TYPE_6 = 6,
    MUSIC_TYPE_7 = 7,
};

void SoundBank_LoadResources(void);
void SoundBank_Init(void);
void SoundBank_Deinit(void);

int SoundBank_Play(int soundEventIndex, Vector*, uint);
void SoundBank_Stop(int* pVoiceCode);

int SoundBank_GetID(int soundEventIndex, unsigned int);

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

struct SoundEventFader {
    SoundEventHelper helper;
    FaderObject fader;
    float unk20;
    float unk24;
    
    void Init(float, float);
    void Reset(void);
    void Update(int, bool, bool, GameObject*, Vector*, float, int);
};

struct DynamicPhrasePlayer {
    int unk0;
    int unk4;
    int unk8;
    int unkC;
    int unk10;
    int unk14;
    
    void Init(void);
    void Deinit(void);
    void Reset(void);
    bool Preload(int, int, bool);
    bool IsLoaded(void);
    void Play(void);
    void Stop(void);
    void UnloadBank(void);
    void UnloadPackage(void);
    char* GetFileName(bool);
    bool HasPlayed(void);
    void Update(void);
};

struct SoundMaterial;

struct SoundEvent {
    int range;
    unsigned char numMaterials;
    SoundMaterial* pMaterials;

    void Init(KromeIni*, KromeIniLine*);
    void Reset(void);
    int Play(Vector*, unsigned int);
    int GetID(unsigned int);
    int CountSoundMaterials(KromeIni*);
};

/// @brief Sound to be chosen for a SoundMaterial
struct SoundMaterialSound {
    char* pSoundName;
    int soundId;
    float chance;
};

struct SoundMaterial {
    unsigned int unk0; // flags
    unsigned char volume;
    float minPitch;
    float maxPitch;
    unsigned char numSounds;
    SoundMaterialSound* pSounds;

    void Init(KromeIni*, unsigned char, float, float);
    int Play(Vector*, int, SoundEvent*);
    int GetID(void);
    int CountSounds(KromeIni*);
};

struct SoundEventManager {
    SoundEvent* pSoundEvents;
    int levelBankID;
    int voxLevelBankID;
    int unkC;
    int unk10;
    int unk14;
    int unk18;
    unsigned char unk1C;
    unsigned char unk1D;
    int unk20;
    int unk24;
    int unk28;
    FaderObject fader;
    char* unk48;

    void Init(void);
    void Reset(void);
    void ParseSoundEvents(KromeIni*);
    void BuildSoundNameTable(KromeIni*);
};

enum SoundID {

};

#endif // SOUNDBANK_H
