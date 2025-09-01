#include "ty/soundbank.h"
#include "ty/global.h"
#include "common/Translation.h"
#include "common/FileSys.h"
#include "common/Str.h"

extern "C" int Sound_IsVoicePlaying(int);
extern "C" void Sound_Update3d(int, int, Vector*);
extern "C" void Sound_Stop3d(int);
extern "C" void Sound_Stop(int);
extern "C" int Sound_Play(int);
extern "C" void Sound_StopAll(void);
extern "C" void Sound_MusicStop(void);
extern "C" void Sound_UnloadBank(int);
extern "C" void Sound_MusicPlay(char*, int, int);
extern "C" void Sound_SetVolume(int, int, int);

void SoundBank_SetVolume(float, int);

extern "C" void Sound_MusicPause(bool);

extern "C" int stricmp(char*, char*);

extern "C" int Sound_LoadBank(char*);

extern "C" void memset(void*, int, int);

extern "C" char* sprintf(char*, ...);

static Tools_DynamicStringTable soundStringTable;
static SoundEventManager soundEventManager;

static DynamicPhrasePlayer* gpActivePhrasePlayer;

void SoundBank_LoadResources(void) {
    soundEventManager.Init();
}

void SoundBank_Init(void) {
    if (gb.mLogicState.currState != STATE_9) {
        soundEventManager.unk10 = Sound_LoadBank("SFX_MainMenu");
    } else {
        char name[32];
        sprintf(name, "SFX_%s", gb.level.GetID());
        soundEventManager.levelBankID = Sound_LoadBank(name);

        sprintf(name, "VOX_%s_%s", gb.level.GetID(), Translation_GetLanguageCode(Translation_GetLanguage()));

        if (FileSys_Exists(Str_Printf("%s.gsb", name), NULL)) {
            soundEventManager.voxLevelBankID = Sound_LoadBank(name);
        }
    }
    
    soundEventManager.Reset();
    gpActivePhrasePlayer = NULL;
}

void SoundBank_Deinit(void) {
    Sound_StopAll();
    Sound_MusicStop();

    if (soundEventManager.voxLevelBankID > -1) {
        Sound_UnloadBank(soundEventManager.voxLevelBankID);
    }

    if (soundEventManager.levelBankID > -1) {
        Sound_UnloadBank(soundEventManager.levelBankID);
    }

    if (soundEventManager.unk10 > -1) {
        Sound_UnloadBank(soundEventManager.unk10);
    }

    soundEventManager.levelBankID = -1;
    soundEventManager.voxLevelBankID = -1;
    soundEventManager.unk10 = -1;
    soundEventManager.unk48 = NULL;

    if (gpActivePhrasePlayer) {
        gpActivePhrasePlayer->Deinit();
    }

    gpActivePhrasePlayer = NULL;
}

int SoundBank_Play(int soundEventIndex, Vector* r4, uint flags) {
    if (soundEventManager.pSoundEvents && soundEventManager.pSoundEvents[soundEventIndex].pMaterials) {
        return soundEventManager.pSoundEvents[soundEventIndex].Play(r4, flags);
    }
    
    return -1;
}

void SoundBank_Stop(int* pVoiceCode) {
    if (*pVoiceCode > -1 && Sound_IsVoicePlaying(*pVoiceCode)) {
        Sound_Stop3d(*pVoiceCode);
    }

    *pVoiceCode = -1;
}

int SoundBank_GetID(int soundEventIndex, unsigned int r4) {
    if (soundEventManager.pSoundEvents) {
        return soundEventManager.pSoundEvents[soundEventIndex].GetID(r4);
    }
    
    return -1;
}

void SoundBank_PlayExclusiveAmbientSound(bool r3) {
    if (soundEventManager.unk14 > -1) {
        Sound_Stop(soundEventManager.unk14);
    }

    if (!r3) {
        soundEventManager.unk14 = SoundBank_Play(0x71, NULL, 0);
    } else {
        switch (gb.level.GetCurrentLevel()) {
            case LN_TWO_UP:
                soundEventManager.unk14 = SoundBank_Play(0x70, NULL, 0);
                break;
            case LN_BRIDGE_RIVER_TY:
                soundEventManager.unk14 = SoundBank_Play(0x76, NULL, 0);
                break;
        }
    }
}

void SoundBank_StopExclusiveAmbientSound(void) {
    SoundBank_Stop(&soundEventManager.unk14);
}

void SoundBank_PlayMusic(MusicType type, float f1, float f2) {
    static char buffer[32];

    switch (type) {
        case MUSIC_TYPE_1:
            sprintf(buffer, "music_%s%s", gb.level.GetID(), gb.level.IsBossEnabled() ? "_boss" : "");
            break;
        case MUSIC_TYPE_2:
            sprintf(buffer, Str_Printf("music_frontend_%s", Translation_GetLanguageCode(Translation_GetLanguage())));
            break;
        case MUSIC_TYPE_3:
            sprintf(buffer, "music_%s_mb", gb.level.GetID());
            break;
        case MUSIC_TYPE_4:
            sprintf(buffer, "music_%s_mg", gb.level.GetID());
            break;
        case MUSIC_TYPE_5:
            sprintf(buffer, "music_%s_ta", gb.level.GetID());
            break;
        case MUSIC_TYPE_7:
            if (Translation_GetLanguage() == LANGUAGE_ENGLISH || Translation_GetLanguage() == Language_American) {
                sprintf(buffer, "music_credits");
            } else {
                sprintf(buffer, "music_credits_le");
            }
            break;
        default:
            return;
    }

    soundEventManager.unk48 = NULL;

    if (f2 > 0.0f) {
        soundEventManager.unk48 = buffer;
        
        soundEventManager.unk18 = 3;

        soundEventManager.fader.Fade(FaderObject::FADEMODE_7, f1, f2, 0.0f, true);

        return;
    } else if (f1 > 0.0f) {
        soundEventManager.unk18 = 1;

        soundEventManager.fader.Fade(FaderObject::FADEMODE_1, f1, 0.0f, 0.0f, true);
    } else {
        soundEventManager.unk18 = 0;

        SoundBank_SetVolume(1.0f, 3);
    }

    Sound_MusicStop();
    Sound_MusicPlay(buffer, 1, 0);
    SoundBank_SetVolume(1.0f, 3);
}

void SoundBank_PauseMusic(bool r3, float f1) {
    if (f1 > 0.0f) {
        if (r3) {
            soundEventManager.unk18 = 2;
            soundEventManager.fader.Fade(FaderObject::FADEMODE_5, 0.0f, f1, 0.0f, true);
        } else {
            soundEventManager.unk18 = 1;
            soundEventManager.fader.Fade(FaderObject::FADEMODE_1, f1, 0.0f, 0.0f, true);
            Sound_MusicPause(0);
        }
    } else {
        Sound_MusicPause(r3);
        soundEventManager.unk18 = 0;
    }
}

void SoundBank_PlayDialogMusic(bool r3) {
    gb.mGameData.SetMusicDucked(r3);
    SoundBank_SetVolume(1.0f, 3);
}

void SoundBank_Update(void) {
    switch (soundEventManager.unk18) {
        default:
            return;
        case 1:
            if (soundEventManager.fader.currFadeState == FaderObject::FADESTATE_0) {
                soundEventManager.unk18 = 0;
                return;
            }
            break;
        case 2:
            if (soundEventManager.fader.currFadeState == FaderObject::FADESTATE_0) {
                soundEventManager.unk18 = 0;
                Sound_MusicPause(1);
                return;
            }
            break;
        case 3:
            if (soundEventManager.fader.GetFadePercentage() == 0.0f) {
                Sound_MusicStop();

                if (soundEventManager.unk48) {
                    soundEventManager.unk18 = 1;
                    Sound_MusicPlay(soundEventManager.unk48, 1, 0);
                    soundEventManager.unk48 = NULL;
                } else {
                    soundEventManager.unk18 = 0;
                    return;
                }
            }
            break;
    }

    SoundBank_SetVolume(soundEventManager.fader.GetFadePercentage(), 2);
    soundEventManager.fader.Update();
}

void SoundEventManager::Init(void) {
    unk14 = -1;
    levelBankID = -1;
    voxLevelBankID = -1;
    unk10 = -1;

    unkC = Sound_LoadBank("SFX_frontend");

    pSoundEvents = NULL;
    unk48 = NULL;

    fader.Reset();

    soundStringTable.Init();

    KromeIni ini;

    if (ini.Init("global.sound")) {
        BuildSoundNameTable(&ini);

        pSoundEvents = (SoundEvent*)Heap_MemAlloc(0x269 * sizeof(SoundEvent));
        memset(pSoundEvents, 0, 0x269 * sizeof(SoundEvent));

        ParseSoundEvents(&ini);

        ini.Deinit();

        Reset();
    }
}

void SoundEventManager::Reset(void) {
    if (pSoundEvents) {
        for (int i = 0; i < 0x269; i++) {
            pSoundEvents[i].Reset();
        }
    }

    unk18 = 0;
    unk20 = 0;

    unk28 = -1;

    unk1C = 0;
    unk1D = 0;
    unk48 = NULL;
    
    fader.Reset();
}

void SoundEventManager::ParseSoundEvents(KromeIni* pIni) {
    KromeIniLine* pLine = pIni->GotoLine(NULL, NULL);

    int index = 0;

    while (pLine) {
        if (pLine->pFieldName && stricmp("SoundEvent", pLine->pFieldName) == 0) {
            if (index >= 0x269) {
                index++;
                pLine = pIni->GetNextLine();
            } else {
                pSoundEvents[index].Init(pIni, pLine);
                pLine = pIni->GetCurrentLine();
                index++;
            }
        } else {
            pLine = pIni->GetNextLine();
        }

    }
}

void SoundEventManager::BuildSoundNameTable(KromeIni* pIni) {
    pIni->GotoLine(NULL, NULL);

    char* pString = NULL;

    KromeIniLine* pLine = pIni->GotoLine("setup", NULL);

    if (stricmp("setup", pLine->section) == 0) {
        pLine = pIni->GetNextLine();

        while (pLine) {
            if (pLine->section) {
                break;
            }

            pLine = pIni->GetNextLine();
        }
    }

    while (pLine) {
        if (pLine->pFieldName && stricmp("sound", pLine->pFieldName) == 0) {
            if (pLine->AsString(0, &pString)) {
                if (!soundStringTable.FindString(pString)) {
                    soundStringTable.AppendString(pString);
                }
            }
        }

        pLine = pIni->GetNextLine();
    }
}

void SoundEvent::Init(KromeIni* pIni, KromeIniLine* pLine) {
    float minPitch = 1.0f;
    float maxPitch = 1.0f;

    unsigned char volume = 255;

    range = 0;
    pMaterials = NULL;
    numMaterials = 0;

    pLine = pIni->GetNextLine();

    while (pLine && pLine->pFieldName && stricmp("SoundEvent", pLine->pFieldName) != 0) {
        if (stricmp("material", pLine->pFieldName) == 0) {
            numMaterials = CountSoundMaterials(pIni);

            if (numMaterials) {
                pMaterials = (SoundMaterial*)Heap_MemAlloc(numMaterials * sizeof(SoundMaterial));
                memset(pMaterials, 0, numMaterials * sizeof(SoundMaterial));

                pIni->GetLineWithLine(pLine);

                pIni->GetPreviousLine();

                int index = 0;

                while (
                    pLine && 
                    pLine->pFieldName && 
                    stricmp("SoundEvent", pLine->pFieldName) != 0 && 
                    stricmp("material", pLine->pFieldName) == 0
                ) {
                    pMaterials[index].Init(pIni, volume, minPitch, maxPitch);

                    pLine = pIni->GetCurrentLine();
                    index++;
                }
            }
        } else {
            if (stricmp("minPitch", pLine->pFieldName) == 0) {
                pLine->AsFloat(0, &minPitch);
            } else if (stricmp("maxPitch", pLine->pFieldName) == 0) {
                pLine->AsFloat(0, &maxPitch);
            } else if (stricmp("range", pLine->pFieldName) == 0) {
                pLine->AsInt(0, &range);
            } else if (stricmp("volume", pLine->pFieldName) == 0) {
                int tempVolume;
                pLine->AsInt(0, &tempVolume);

                volume = tempVolume;
            } else {
                pIni->Warning("Unknown SoundEvent field");
            }

            pLine = pIni->GetNextLine();
        }
    }
}

extern "C" int Sound_GetID(char*);

void SoundEvent::Reset(void) {
    for (int i = 0; i < numMaterials; i++) {
        for (int j = 0; j < pMaterials[i].numSounds; j++) {
            if (pMaterials[i].pSounds[j].pSoundName) {
                pMaterials[i].pSounds[j].soundId = Sound_GetID(pMaterials[i].pSounds[j].pSoundName);
            }
        }
    }
}

int SoundEvent::Play(Vector* r4, unsigned int r5) {
    for (int i = 0; i < numMaterials; i++) {
        if (pMaterials[i].unk0 & r5) {
            return pMaterials[i].Play(r4, range, this);
        }
    }

    if (pMaterials[0].unk0 == 0) {
        return pMaterials[0].Play(r4, range, this);
    }

    return -1;
}

int SoundEvent::GetID(unsigned int r4) {
    for (int i = 0; i < numMaterials; i++) {
        if (pMaterials[i].unk0 & r4) {
            return pMaterials[i].GetID();
        }
    }

    if (pMaterials[0].unk0 == 0) {
        return pMaterials[0].GetID();
    }

    return -1;
}

/// @brief Returns the number of sounds materials for a sound event
/// @param pIni The ini to continue reading from
/// @return Number of sounds materials for a sound event
int SoundEvent::CountSoundMaterials(KromeIni* pIni) {
    KromeIniLine* pLine = pIni->GetCurrentLine();

    int count = 0;

    while (pLine && pLine->pFieldName) {
        if (stricmp("material", pLine->pFieldName) == 0) {
            count++;
        }

        pLine = pIni->GetNextLine();
    }

    return count;
}

void SoundMaterial::Init(KromeIni* pIni, unsigned char _volume, float _minPitch, float _maxPitch) {
    KromeIniLine* pLine = pIni->GetCurrentLine();

    volume = _volume;
    minPitch = _minPitch;
    maxPitch = _maxPitch;
    unk0 = 0;

    for (int i = 0; i < pLine->elementCount; i++) {
        char* pString;

        if (pLine->AsString(i, &pString)) {
            int b = stricmp(pString, "ID_NONE") == 0 ? 1 : 0;

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_WALL") == 0) {
                unk0 |= 0x1;
                b = 1;
            } else {
                b = 0;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_SLIPPERY") == 0) {
                unk0 |= 0x2;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_CAMERA_IGNORE") == 0) {
                unk0 |= 0x4;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_ICE") == 0) {
                unk0 |= 0x8;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_SAND") == 0) {
                unk0 |= 0x10;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_ENEMY_COLLIDE") == 0) {
                unk0 |= 0x20;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_MUD") == 0) {
                unk0 |= 0x40;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_QUICKSAND") == 0) {
                unk0 |= 0x80;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_BOOMERANG_IGNORE") == 0) {
                unk0 |= 0x100;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_LAVA") == 0) {
                unk0 |= 0x200;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_WATER_BLUE") == 0) {
                unk0 |= 0x400;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_SNOW") == 0) {
                unk0 |= 0x800;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_WOOD") == 0) {
                unk0 |= 0x1000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_TUNNEL") == 0) {
                unk0 |= 0x2000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_INVISCOLLIDE") == 0) {
                unk0 |= 0x4000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_SNOW_TOP") == 0) {
                unk0 |= 0x8000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_SOFT") == 0) {
                unk0 |= 0x10000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_WATER_SLIDE") == 0) {
                unk0 |= 0x20000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_GRASS_THIN") == 0) {
                unk0 |= 0x40000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_GRASS_THICK") == 0) {
                unk0 |= 0x80000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_ROCK") == 0) {
                unk0 |= 0x100000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_TREELEAVES") == 0) {
                unk0 |= 0x200000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_JUMP_CAMERA") == 0) {
                unk0 |= 0x400000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_NORMAL_CAM") == 0) {
                unk0 |= 0x800000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_CAM_GO_THROUGH") == 0) {
                unk0 |= 0x1000000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_FAST") == 0) {
                unk0 |= 0x2000000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_SLOW") == 0) {
                unk0 |= 0x4000000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_TURNAWAY") == 0) {
                unk0 |= 0x8000000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_METAL") == 0) {
                unk0 |= 0x10000000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_ICE_SLIDE") == 0) {
                unk0 |= 0x20000000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ID_HOLLOWWOOD") == 0) {
                unk0 |= 0x40000000;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }
            
            if (stricmp(pString, "ANY_WATER_ID") == 0) {
                unk0 |= 0x400;
                b = true;
            } else {
                b = false;
            }

            if (b) {
                continue;
            }

            pIni->Warning(Str_Printf("Unknown material ID - %s", pString));
        }
    }

    pLine = pIni->GetNextLine();

    while (pLine) {
        if (pLine->pFieldName) {
            if (stricmp("SoundEvent", pLine->pFieldName) == 0 || stricmp("material", pLine->pFieldName) == 0) {
                return;
            }

            if (stricmp("sound", pLine->pFieldName) != 0) {
                if (stricmp("minPitch", pLine->pFieldName) == 0) {
                    pLine->AsFloat(0, &minPitch);
                } else if (stricmp("maxPitch", pLine->pFieldName) == 0) {
                    pLine->AsFloat(0, &maxPitch);
                } else if (stricmp("volume", pLine->pFieldName) == 0) {
                    int tempVolume;
                    pLine->AsInt(0, &tempVolume);
                    volume = tempVolume;
                } else {
                    pIni->Warning("Unknown SoundMaterial field");
                }
            } else {
                break;
            }
        }

        pLine = pIni->GetNextLine();
    }

    numSounds = CountSounds(pIni);

    if (numSounds == 0) {
        return;
    }

    pSounds = (SoundMaterialSound*)Heap_MemAlloc(numSounds * sizeof(SoundMaterialSound));
    memset(pSounds, 0, numSounds * sizeof(SoundMaterialSound));

    pIni->GetLineWithLine(pLine);
    pLine = pIni->GetPreviousLine();

    int index = 0;
    while (pLine && pLine->pFieldName && stricmp("material", pLine->pFieldName) != 0) {
        if (stricmp("sound", pLine->pFieldName) == 0) {
            char* pString;

            if (pLine->AsString(0, &pString)) {
                pSounds[index].soundId = -1;
                pSounds[index].pSoundName = soundStringTable.FindString(pString);
                pSounds[index].chance = 1.0f;

                pLine = pIni->GetNextLine();

                while (pLine && pLine->pFieldName) {
                    if (stricmp("chance", pLine->pFieldName) == 0) {
                        pLine->AsFloat(0, &pSounds[index].chance);
                        pLine = pIni->GetNextLine();
                    } else {
                        break;
                    }
                }

                index++;
                continue;
            }
        }


        pLine = pIni->GetNextLine();
    }
}

extern "C" int Sound_PlayV(int, int, int);
extern "C" int Sound_Play3d(int, int, Vector*, float);

extern "C" void Sound_SetPitch3d(int, float);
extern "C" void Sound_SetPitch(int, float);

int SoundMaterial::Play(Vector* r4, int r5, SoundEvent*) {
    float f2 = 1.0f;
    float f31 = 0.0f;

    if (numSounds && pSounds[0].chance != 1.0f) {
        f2 = RandomFR(&gb.mRandSeed, 0.0f, 1.0f);
    }

    for (int i = 0; i < numSounds; i++) {
        f31 += pSounds[i].chance;

        if (pSounds[i].soundId > 0 && f2 <= f31) {
            if (r4) {
                r5 = Sound_Play3d(pSounds[i].soundId, volume, r4, r5);

                if (minPitch != 1.0f || maxPitch != 1.0f) {
                    Sound_SetPitch3d(r5, RandomFR(&gb.mRandSeed, minPitch, maxPitch));
                }
            } else {
                r5 = Sound_PlayV(pSounds[i].soundId, volume, volume);

                if (minPitch != 1.0f || maxPitch != 1.0f) {
                    Sound_SetPitch(r5, RandomFR(&gb.mRandSeed, minPitch, maxPitch));
                }
            }

            return r5;
        }
    }

    return -1;
}

int SoundMaterial::GetID(void) {
    if (numSounds) {
        return pSounds[0].soundId;
    }

    return -1;
}

/// @brief Returns the number of sounds for a sound material
/// @param pIni The ini to continue reading from
/// @return Number of sounds for a sound material
int SoundMaterial::CountSounds(KromeIni* pIni) {
    KromeIniLine* pLine = pIni->GetCurrentLine();

    int count = 0;

    while (pLine && pLine->pFieldName) {
        if (stricmp("material", pLine->pFieldName) == 0) {
            break;
        }

        if (stricmp("sound", pLine->pFieldName) == 0) {
            count++;
        }

        pLine = pIni->GetNextLine();
    }

    return count;
}

int SoundBank_ResolveSoundEventIndex(char* pName) {
    if (stricmp((char*)"SFX_A2JeepBonnetClose" + 4, pName) == 0) {
        return 0x1cc;
    } else if (stricmp((char*)"SFX_A2JeepDriveOff" + 4, pName) == 0) {
        return 0x1cd;
    } else if (stricmp((char*)"SFX_A2ShazzaHitsHead" + 4, pName) == 0) {
        return 0x1f8;
    } else if (stricmp((char*)"SFX_BlueTBoulderHit" + 4, pName) == 0) {
        return 0x39;
    } else if (stricmp((char*)"SFX_BullB3Snort" + 4, pName) == 0) {
        return 0x1fa;
    } else if (stricmp((char*)"SFX_BullFall" + 4, pName) == 0) {
        return 0xd0;
    } else if (stricmp((char*)"SFX_BullHitShed" + 4, pName) == 0) {
        return 0x152;
    } else if (stricmp((char*)"SFX_CassCockpitLP" + 4, pName) == 0) {
        return 0x21d;
    } else if (stricmp((char*)"SFX_CassFootstep" + 4, pName) == 0) {
        return 0x21e;
    } else if (stricmp((char*)"SFX_CassInteriorLP" + 4, pName) == 0) {
        return 0x21f;
    } else if (stricmp((char*)"SFX_CaveBatsHit" + 4, pName) == 0) {
        return 0x11a;
    } else if (stricmp((char*)"SFX_ConveyorLP" + 4, pName) == 0) {
        return 0x97;
    } else if (stricmp((char*)"SFX_D1RobotCollapse" + 4, pName) == 0) {
        return 0x23f;
    } else if (stricmp((char*)"SFX_D1TalismanSpin" + 4, pName) == 0) {
        return 0x240;
    } else if (stricmp((char*)"SFX_DoorSlideUpDown" + 4, pName) == 0) {
        return 0x1e6;
    } else if (stricmp((char*)"SFX_DoorThud" + 4, pName) == 0) {
        return 0x1e8;
    } else if (stricmp((char*)"SFX_EnemyFallLarge" + 4, pName) == 0) {
        return 0x34;
    } else if (stricmp((char*)"SFX_EnemyPunchMed" + 4, pName) == 0) {
        return 0x30;
    } else if (stricmp((char*)"SFX_EnemyStepLarge" + 4, pName) == 0) {
        return 0x2e;
    } else if (stricmp((char*)"SFX_EnemyStepSmall" + 4, pName) == 0) {
        return 0x2c;
    } else if (stricmp((char*)"SFX_EnvBirdChirp1" + 4, pName) == 0) {
        return 0x72;
    } else if (stricmp((char*)"SFX_EnvBirdChirp2" + 4, pName) == 0) {
        return 0x73;
    } else if (stricmp((char*)"SFX_EnvBirdChirp3" + 4, pName) == 0) {
        return 0x74;
    } else if (stricmp((char*)"SFX_EnvBirdChirp4" + 4, pName) == 0) {
        return 0x1ae;
    } else if (stricmp((char*)"SFX_EnvBirdSongLP" + 4, pName) == 0) {
        return 0x1af;
    } else if (stricmp((char*)"SFX_EnvBushFireLP" + 4, pName) == 0) {
        return 0x1b0;
    } else if (stricmp((char*)"SFX_EnvBushFire2LP" + 4, pName) == 0) {
        return 0x1b1;
    } else if (stricmp((char*)"SFX_EnvCaveDrip" + 4, pName) == 0) {
        return 0x12a;
    } else if (stricmp((char*)"SFX_EnvCowLP" + 4, pName) == 0) {
        return 0x214;
    } else if (stricmp((char*)"SFX_EnvCicadasLP" + 4, pName) == 0) {
        return 0x77;
    } else if (stricmp((char*)"SFX_EnvCricketsLP" + 4, pName) == 0) {
        return 0x76;
    } else if (stricmp((char*)"SFX_EnvCrowCall" + 4, pName) == 0) {
        return 0x78;
    } else if (stricmp((char*)"SFX_EnvCrows" + 4, pName) == 0) {
        return 0x79;
    } else if (stricmp((char*)"SFX_EnvE4TunnelLP" + 4, pName) == 0) {
        return 0x223;
    } else if (stricmp((char*)"SFX_EnvE4Tunnel" + 4, pName) == 0) {
        return 0x224;
    } else if (stricmp((char*)"SFX_EnvExplosionMid" + 4, pName) == 0) {
        return 0x7a;
    } else if (stricmp((char*)"SFX_EnvFarThunder" + 4, pName) == 0) {
        return 0x1b2;
    } else if (stricmp((char*)"SFX_EnvFrog" + 4, pName) == 0) {
        return 0x7e;
    } else if (stricmp((char*)"SFX_EnvFrogsLP" + 4, pName) == 0) {
        return 0x7f;
    } else if (stricmp((char*)"SFX_EnvKookaburra" + 4, pName) == 0) {
        return 0x80;
    } else if (stricmp((char*)"SFX_EnvMountWindLP" + 4, pName) == 0) {
        return 0x1b3;
    } else if (stricmp((char*)"SFX_EnvPontoonAreaLP" + 4, pName) == 0) {
        return 0x1b4;
    } else if (stricmp((char*)"SFX_EnvSeagullsLP" + 4, pName) == 0) {
        return 0x85;
    } else if (stricmp((char*)"SFX_EnvSeagull" + 4, pName) == 0) {
        return 0x1d1;
    } else if (stricmp((char*)"SFX_EnvSeaWashLP" + 4, pName) == 0) {
        return 0x86;
    } else if (stricmp((char*)"SFX_EnvSheepLP" + 4, pName) == 0) {
        return 0x215;
    } else if (stricmp((char*)"SFX_EnvWaterFallCloseLP" + 4, pName) == 0) {
        return 0x88;
    } else if (stricmp((char*)"SFX_EnvWaterSlideLP" + 4, pName) == 0) {
        return 0x8b;
    } else if (stricmp((char*)"SFX_FrillJunkThrow" + 4, pName) == 0) {
        return 0x44;
    } else if (stricmp((char*)"SFX_GateOpen" + 4, pName) == 0) {
        return 0x1ab;
    } else if (stricmp((char*)"SFX_GateClose" + 4, pName) == 0) {
        return 0x1ac;
    } else if (stricmp((char*)"SFX_GiantSnowBallImpact" + 4, pName) == 0) {
        return 0xc0;
    } else if (stricmp((char*)"SFX_IceCracking" + 4, pName) == 0) {
        return 0xa2;
    } else if (stricmp((char*)"SFX_JuliusRangMachineWorkLP" + 4, pName) == 0) {
        return 0x1c9;
    } else if (stricmp((char*)"SFX_JuliusTalMachineLP" + 4, pName) == 0) {
        return 0x1c7;
    } else if (stricmp((char*)"SFX_JuliusTalMachineEvent1" + 4, pName) == 0) {
        return 0x1d2;
    } else if (stricmp((char*)"SFX_JuliusTalMachineEvent2" + 4, pName) == 0) {
        return 0x1d3;
    } else if (stricmp((char*)"SFX_JuliusRangMachineEvent1" + 4, pName) == 0) {
        return 0x1d4;
    } else if (stricmp((char*)"SFX_LavaFlowLP" + 4, pName) == 0) {
        return 0x1cf;
    } else if (stricmp((char*)"SFX_LennyFootstep" + 4, pName) == 0) {
        return 0xee;
    } else if (stricmp((char*)"SFX_MaurieFlapLP" + 4, pName) == 0) {
        return 0xb2;
    } else if (stricmp((char*)"SFX_MuddieWalkLP" + 4, pName) == 0) {
        return 0x51;
    } else if (stricmp((char*)"SFX_PlatformHoverLP" + 4, pName) == 0) {
        return 0x1bc;
    } else if (stricmp((char*)"SFX_PortalAppears" + 4, pName) == 0) {
        return 0xb9;
    } else if (stricmp((char*)"SFX_PortalShimmerLP" + 4, pName) == 0) {
        return 0xba;
    } else if (stricmp((char*)"SFX_PropWobble" + 4, pName) == 0) {
        return 0x12f;
    } else if (stricmp((char*)"SFX_RangCollection" + 4, pName) == 0) {
        return 0x203;
    } else if (stricmp((char*)"SFX_RexElleStep" + 4, pName) == 0) {
        return 0x1e1;
    } else if (stricmp((char*)"SFX_RexElleLand" + 4, pName) == 0) {
        return 0x1e2;
    } else if (stricmp((char*)"SFX_RingoIdleLP" + 4, pName) == 0) {
        return 0x1db;
    } else if (stricmp((char*)"SFX_RufusHopHitGround" + 4, pName) == 0) {
        return 0x56;
    } else if (stricmp((char*)"SFX_RufusHopLeaveGround" + 4, pName) == 0) {
        return 0x57;
    } else if (stricmp((char*)"SFX_ShadowBatFall" + 4, pName) == 0) {
        return 0xe7;
    } else if (stricmp((char*)"SFX_ShadowBatFlap" + 4, pName) == 0) {
        return 0xe2;
    } else if (stricmp((char*)"SFX_ShadowBatHitsGround" + 4, pName) == 0) {
        return 0xe8;
    } else if (stricmp((char*)"SFX_ShadowBatRoar" + 4, pName) == 0) {
        return 0xe3;
    } else if (stricmp((char*)"SFX_ShadowBatShriek" + 4, pName) == 0) {
        return 0xe4;
    } else if (stricmp((char*)"SFX_ShadowBatSwoop" + 4, pName) == 0) {
        return 0xe6;
    } else if (stricmp((char*)"SFX_SlyStep" + 4, pName) == 0) {
        return 0x220;
    } else if (stricmp((char*)"SFX_SlyStepMetal" + 4, pName) == 0) {
        return 0x221;
    } else if (stricmp((char*)"SFX_SlyLand" + 4, pName) == 0) {
        return 0x222;
    } else if (stricmp((char*)"SFX_TrapdoorOpen" + 4, pName) == 0) {
        return 0x1e7;
    } else if (stricmp((char*)"SFX_TyBiteRelease" + 4, pName) == 0) {
        return 0x13;
    } else if (stricmp((char*)"SFX_TyBounce" + 4, pName) == 0) {
        return 0x24;
    } else if (stricmp((char*)"SFX_TyCollectEgg" + 4, pName) == 0) {
        return 0x1f;
    } else if (stricmp((char*)"SFX_TyDiveBiteHitGround" + 4, pName) == 0) {
        return 0x10c;
    } else if (stricmp((char*)"SFX_TyJump" + 4, pName) == 0) {
        return 0x0;
    } else if (stricmp((char*)"SFX_TyKnockDown" + 4, pName) == 0) {
        return 0x1;
    } else if (stricmp((char*)"SFX_TyLand" + 4, pName) == 0) {
        return 0x4;
    } else if (stricmp((char*)"SFX_TyLedgeGrab" + 4, pName) == 0) {
        return 0x26;
    } else if (stricmp((char*)"SFX_TyLedgePullUp" + 4, pName) == 0) {
        return 0x27;
    } else if (stricmp((char*)"SFX_TyLongfallLand" + 4, pName) == 0) {
        return 0x5;
    } else if (stricmp((char*)"SFX_TyRangDoomLP" + 4, pName) == 0) {
        return 0x1a;
    } else if (stricmp((char*)"SFX_TyRangStdDeflect" + 4, pName) == 0) {
        return 0x16;
    } else if (stricmp((char*)"SFX_TyRangStdThrow" + 4, pName) == 0) {
        return 0x14;
    } else if (stricmp((char*)"SFX_TySkid" + 4, pName) == 0) {
        return 0x12c;
    } else if (stricmp((char*)"SFX_TyStep" + 4, pName) == 0) {
        return 0x2;
    } else if (stricmp((char*)"SFX_TyWaterSlideJump" + 4, pName) == 0) {
        return 0x127;
    } else if (stricmp((char*)"SFX_TyWoodGroan" + 4, pName) == 0) {
        return 0x3;
    } else if (stricmp((char*)"SFX_UWGateOpens" + 4, pName) == 0) {
        return 0x103;
    } else if (stricmp((char*)"SFX_A1RufusSnore" + 4, pName) == 0) {
        return 0x241;
    } else if (stricmp((char*)"SFX_A1RufusStretch" + 4, pName) == 0) {
        return 0x242;
    } else if (stricmp((char*)"SFX_B2Avalanche" + 4, pName) == 0) {
        return 0x243;
    } else if (stricmp((char*)"SFX_B2YabbyPickup" + 4, pName) == 0) {
        return 0x244;
    } else if (stricmp((char*)"SFX_B3BullBreathing" + 4, pName) == 0) {
        return 0x245;
    } else if (stricmp((char*)"SFX_C1Flashes" + 4, pName) == 0) {
        return 0x246;
    } else if (stricmp((char*)"SFX_C4FluffyJump" + 4, pName) == 0) {
        return 0x247;
    } else if (stricmp((char*)"SFX_D4CrikeyJump" + 4, pName) == 0) {
        return 0x248;
    } else if (stricmp((char*)"SFX_D4CrikeySwimLP" + 4, pName) == 0) {
        return 0x249;
    } else if (stricmp((char*)"SFX_D4CrikeySplash" + 4, pName) == 0) {
        return 0x24a;
    } else if (stricmp((char*)"SFX_D2Latch" + 4, pName) == 0) {
        return 0x24b;
    } else if (stricmp((char*)"SFX_D2MetalDoor1Open" + 4, pName) == 0) {
        return 0x24c;
    } else if (stricmp((char*)"SFX_D2MetalDoor2Open" + 4, pName) == 0) {
        return 0x24d;
    } else if (stricmp((char*)"SFX_D2ShadowBatJump" + 4, pName) == 0) {
        return 0x24e;
    } else if (stricmp((char*)"SFX_CSTyStepMetal" + 4, pName) == 0) {
        return 0x24f;
    } else if (stricmp((char*)"SFX_CSTyLandMetal" + 4, pName) == 0) {
        return 0x250;
    } else if (stricmp((char*)"SFX_CSWhoosh" + 4, pName) == 0) {
        return 0x251;
    } else if (stricmp((char*)"SFX_CSLongJump" + 4, pName) == 0) {
        return 0x252;
    } else if (stricmp((char*)"SFX_CSPickup" + 4, pName) == 0) {
        return 0x253;
    } else if (stricmp((char*)"SFX_CSDropWood" + 4, pName) == 0) {
        return 0x254;
    } else if (stricmp((char*)"SFX_CSDropMetal" + 4, pName) == 0) {
        return 0x255;
    } else if (stricmp((char*)"SFX_D2RobotLegMove" + 4, pName) == 0) {
        return 0x256;
    } else if (stricmp((char*)"SFX_D2RobotFootstep" + 4, pName) == 0) {
        return 0x257;
    } else if (stricmp((char*)"SFX_E4DoorExplode" + 4, pName) == 0) {
        return 0x229;
    } else if (stricmp((char*)"SFX_E4RobotHeadRattleLP" + 4, pName) == 0) {
        return 0x258;
    } else if (stricmp((char*)"SFX_E4RobotHeadDetach" + 4, pName) == 0) {
        return 0x259;
    } else if (stricmp((char*)"SFX_EZoneGateOpen" + 4, pName) == 0) {
        return 0x25a;
    } else if (stricmp((char*)"SFX_EZoneGateClose" + 4, pName) == 0) {
        return 0x25b;
    } else if (stricmp((char*)"SFX_B3HouseCollapse1" + 4, pName) == 0) {
        return 0x23c;
    } else if (stricmp((char*)"SFX_B3HouseCollapse2" + 4, pName) == 0) {
        return 0x23d;
    } else if (stricmp((char*)"SFX_B3HouseCollapse3" + 4, pName) == 0) {
        return 0x23e;
    } else if (stricmp((char*)"SFX_CSLaserLP" + 4, pName) == 0) {
        return 0x264;
    } else if (stricmp((char*)"SFX_CSShazzaKiss" + 4, pName) != 0) {
        return 0x1B5;
    } else {
        return 0x265;
    }
}

void SoundEventHelper::Message(MKMessage* pMsg) {
    if (pMsg->unk0 == -4) {
        SoundBank_Stop(&unk0);
    }
}

void SoundEventHelper::Update(int voiceCode, bool r5, bool r6, GameObject* pGameObject, Vector* pVec, float f1, int flags) {
    if (unk0 == -1) {
        if (pGameObject) {
            if (r6) {
                if (pGameObject->distSquared < Sqr<int>(soundEventManager.pSoundEvents[voiceCode].range)) {
                    unk0 = SoundBank_Play(voiceCode, pVec ? pVec : pGameObject->GetPos(), flags);
                }
            }
        } else if (r6) {
            if (!pVec || (f1 > 0.0f && f1 < Sqr<int>(soundEventManager.pSoundEvents[voiceCode].range))) {
                unk0 = SoundBank_Play(voiceCode, pVec, flags);
            }
        }
    } else if (!r6) {
        SoundBank_Stop(&unk0);
    } else if (pGameObject && pGameObject->distSquared > Sqr<int>(soundEventManager.pSoundEvents[voiceCode].range)) {
        SoundBank_Stop(&unk0);
    } else if (f1 > 0.0f && f1 > Sqr<int>(soundEventManager.pSoundEvents[voiceCode].range)) {
        SoundBank_Stop(&unk0);
    } else if (Sound_IsVoicePlaying(unk0) == 0) {
        unk0 = -1;
    } else if (r5 && (pVec || pGameObject)) {
        Sound_Update3d(unk0, -1, pVec ? pVec : pGameObject->GetPos());
    }
}

void SoundEventFader::Init(float f1, float f2) {
    helper.Init();
    fader.Reset();
    unk20 = f1;
    unk24 = f2;
}

void SoundEventFader::Reset(void) {
    helper.Reset();
    fader.Reset();
}

void SoundEventFader::Update(int voiceCode, bool r5, bool r6, GameObject* pGameObject, Vector* pVec, float f1, int flags) {
    int helperVoiceCode = helper.unk0;
    if (helperVoiceCode != -1) {
        if (!r6 && fader.currFadeState != FaderObject::FADESTATE_2 && unk24 > 0.0f) {
            r6 = true;
            fader.Fade(FaderObject::FADEMODE_5, 0.0f, unk24, 0.0f, true);
        } else if (r6 && fader.currFadeState == FaderObject::FADESTATE_2) {
            fader.Fade(FaderObject::FADEMODE_1, unk20, 0.0f, 0.0f, true);
        }

        if (fader.currFadeState != FaderObject::FADESTATE_0) {
            fader.Update();

            int percentage = fader.GetFadePercentage() * 255.0f;

            Sound_SetVolume(helper.unk0, percentage, percentage);
            
            if (fader.currFadeState == FaderObject::FADESTATE_2) {
                r6 = true;
            }
        }
    }

    helper.Update(voiceCode, r5, r6, pGameObject, pVec, f1, flags);

    if (helperVoiceCode != helper.unk0 && (helperVoiceCode == -1 || helper.unk0 != -1)) {
        fader.Fade(FaderObject::FADEMODE_1, unk20, 0.0f, 0.0f, true);

        int percentage = fader.GetFadePercentage() * 255.0f;

        Sound_SetVolume(helper.unk0, percentage, percentage);
    }
}

void DynamicPhrasePlayer::Init(void) {
    unk0 = unk4 = unk8 = unkC = -1;
    unk14 = 0;
}

void DynamicPhrasePlayer::Deinit(void) {
    UnloadPackage();
    Stop();
    UnloadBank();

    unk14 = 0;

    if (gpActivePhrasePlayer == this) {
        gpActivePhrasePlayer = NULL;
    }
}

void DynamicPhrasePlayer::Reset(void) {
    Deinit();
}

bool DynamicPhrasePlayer::Preload(int, int, bool) {

}

bool DynamicPhrasePlayer::IsLoaded(void) {
    return unk14 == 2;
}

void DynamicPhrasePlayer::Play(void) {
    UnloadBank();
    
    if (!FileSys_Exists(GetFileName(true), NULL)) {
        unk14 = 4;
    } else {
        char* pName = GetFileName(false);

        unk4 = Sound_LoadBank(pName);
        unk0 = Sound_Play(Sound_GetID(pName));

        UnloadPackage();
        unk14 = 3;

        gpActivePhrasePlayer = this;
    }
}

void DynamicPhrasePlayer::Stop(void) {
    if (unk14 == 3) {
        SoundBank_Stop(&unk0);
        unk14 = 4;
    }
}

void DynamicPhrasePlayer::UnloadBank(void) {

}

void DynamicPhrasePlayer::UnloadPackage(void) {
    
}

char* DynamicPhrasePlayer::GetFileName(bool r4) {
    
}

bool DynamicPhrasePlayer::HasPlayed(void) {
    return unk14 == 4;
}

void DynamicPhrasePlayer::Update(void) {

}

