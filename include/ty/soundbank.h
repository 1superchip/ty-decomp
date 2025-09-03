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

void SoundBank_StopActivePhrasePlayer(void);

void SoundBank_SetVolume(float, int);

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
    bool unk10;
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
    SFX_TyJump = 0x0,
    SFX_TyKnockDown = 0x1,
    SFX_TyStep = 0x2,
    SFX_TyWoodGroan = 0x3,
    SFX_TyLand = 0x4,
    SFX_TyLongfallLand = 0x5,

    SFX_Ty_0x9 = 0x9,

    SFX_TyBiteRelease = 0x13,

    SFX_TyRangStdThrow      = 0x14,
    SFX_TyRangStdCatch      = 0x15,
    SFX_TyRangStdDeflect    = 0x16,

    SFX_TyRangAquaThrow     = 0x17,
    SFX_TyRangAquaCatch     = 0x18,
    SFX_TyRangAquaDeflect   = 0x19,

    SFX_TyRangDoomLP = 0x1A,

    SFX_TyRangFlameDeflect = 0x1C,

    SFX_TyCollectEgg = 0x1F,
    SFX_TyBounce = 0x24,
    SFX_TyLedgeGrab = 0x26,
    SFX_TyLedgePullUp = 0x27,
    SFX_EnvExplosionMid = 0x7A,
    SFX_BunyipAppear = 0x91,
    SFX_OpalCollect = 0xB7,
    SFX_TyDiveBiteHitGround = 0x10C,
    SFX_TyWaterSlideJump = 0x127,
    SFX_TySkid = 0x12C,

    SFX_TyRangStdHit = 0x171,

    SFX_TyRangFrostyThrow   = 0x172,
    SFX_TyRangFrostyCatch   = 0x173,
    SFX_TyRangFrostyDeflect = 0x174,
    SFX_TyRangFrostyHit     = 0x175,

    SFX_TyRangFlameThrow    = 0x176,
    SFX_TyRangFlameCatch    = 0x177,
    SFX_TyRangFlameHit      = 0x178,

    SFX_TyRangKaboomThrow   = 0x17D,
    SFX_TyRangKaboomCatch   = 0x17E,
    SFX_TyRangKaboomDeflect = 0x17F,
    SFX_TyRangKaboomHit     = 0x180,

    SFX_TyRangDoomThrow     = 0x181,
    SFX_TyRangDoomCatch     = 0x182,
    SFX_TyRangDoomDeflect   = 0x183,
    SFX_TyRangDoomHit       = 0x184,

    SFX_TyRangMegaThrow     = 0x185,
    SFX_TyRangMegaCatch     = 0x186,
    SFX_TyRangMegaDeflect   = 0x187,
    SFX_TyRangMegaHit       = 0x188,

    SFX_TyRangZoomThrow     = 0x189,
    SFX_TyRangZoomCatch     = 0x18A,
    SFX_TyRangZoomDeflect   = 0x18B,
    SFX_TyRangZoomHit       = 0x18C,
    
    SFX_TyRangInfraThrow    = 0x18D,
    SFX_TyRangInfraCatch    = 0x18E,
    SFX_TyRangInfraDeflect  = 0x18F,
    SFX_TyRangInfraHit      = 0x190,

    SFX_191 = 0x191,
    
    SFX_TyRangZappyThrow    = 0x192,
    SFX_TyRangZappyCatch    = 0x193,
    SFX_TyRangZappyDeflect  = 0x194,
    SFX_TyRangZappyHit      = 0x195,

    SFX_TyRangAquaHit       = 0x196,

    SFX_TyRangMultiThrow    = 0x197,
    SFX_TyRangMultiCatch    = 0x198,
    SFX_TyRangMultiDeflect  = 0x199,
    SFX_TyRangMultiHit      = 0x19A,
    
    SFX_TyRangChronoHit = 0x268,

    SFX_Max = 0x269
};

#define SOUNDID_TO_STRING(id) ((char*)(#id + 4))

#endif // SOUNDBANK_H
