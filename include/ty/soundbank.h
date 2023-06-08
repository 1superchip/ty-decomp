#ifndef SOUNDBANK_H
#define SOUNDBANK_H

#include "ty/GameObject.h"

extern void SoundBank_Stop(int*);

struct SoundEventHelper {
    int unk0;
    
    void Update(int, bool, bool, GameObject*, Vector*, float, int);
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

#endif // SOUNDBANK_H
