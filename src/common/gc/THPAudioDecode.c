#include "types.h"
#include "Dolphin/os.h"

extern uint ActivePlayer[];

void* PopReadedBuffer(void);
void PushReadedBuffer2(void*);
void* PopFreeAudioBuffer(void);

void PushDecodedAudioBuffer(void*);

static void* AudioDecoderForOnMemory(void*);
static void* AudioDecoder(void*);
static void* AudioDecode(void*);

int THPAudioDecode(int, int, int);

#define AUDIO_STACK_SIZE (0x1000)

static OSThread AudioDecodeThread;
static char AudioDecodeThreadStack[AUDIO_STACK_SIZE];
static OSMessageQueue FreeAudioBufferQueue;
static OSMessageQueue DecodedAudioBufferQueue;
static OSMessage FreeAudioBufferMessage[3];
static OSMessage DecodedAudioBufferMessage[3];

static int AudioDecodeThreadCreated;

int CreateAudioDecodeThread(long r3, void* audioMem) {
    if (audioMem) {
        if (OSCreateThread(&AudioDecodeThread, AudioDecoderForOnMemory, audioMem,
                &AudioDecodeThreadStack[AUDIO_STACK_SIZE], AUDIO_STACK_SIZE, r3, 1) == 0) {
            return 0;
        }
    } else {
        if (OSCreateThread(&AudioDecodeThread, AudioDecoder, 0,
                &AudioDecodeThreadStack[AUDIO_STACK_SIZE], AUDIO_STACK_SIZE, r3, 1) == 0) {
            return 0;
        }
    }
    OSInitMessageQueue(&FreeAudioBufferQueue, FreeAudioBufferMessage, 3);
    OSInitMessageQueue(&DecodedAudioBufferQueue, DecodedAudioBufferMessage, 3);
    AudioDecodeThreadCreated = 1;
    return 1;
}

void AudioDecodeThreadStart(void) {
    if (AudioDecodeThreadCreated) {
        OSResumeThread(&AudioDecodeThread);
    }
}

void AudioDecodeThreadCancel(void) {
    if (AudioDecodeThreadCreated) {
        OSCancelThread(&AudioDecodeThread);
        AudioDecodeThreadCreated = 0;
    }
}

static void* AudioDecoder(void* r3) {
    void* audioBuf;
    while (1) {
        audioBuf = PopReadedBuffer();
        AudioDecode(audioBuf);
        PushReadedBuffer2(audioBuf);
    }
}

static void* AudioDecoderForOnMemory(void* pStart) {
    int r29 = ActivePlayer[0xBC / 4];
    int r31 = 0;
    volatile int stack_r31[3];
    int* sp8 = (int*)pStart;
    int s;
    while (1) {
        stack_r31[0] = r31;
        AudioDecode(&sp8);
        s = ((r31 + ActivePlayer[0xC0 / 4]) % ActivePlayer[0x50 / 4]);
        if (s == ActivePlayer[0x50 / 4] - 1) {
            if (((char*)ActivePlayer)[0xA6] & 1) {
                r29 = sp8[0];
                sp8 = (int*)ActivePlayer[0xB4 / 4];
            } else {
                OSSuspendThread(&AudioDecodeThread);
            }
        } else {
            int s = sp8[0];
            sp8 = (int*)((char*)sp8 + r29);
            r29 = s;
        }
        r31++;
    }
}

static void* AudioDecode(void* audio) {
    int* r30 = (int*)(((int*)audio)[0] + 8);
    int* r31;
    int i;
    int r29 = ((int*)audio)[0] + (ActivePlayer[0x6C / 4] * 4) + 8;
    r31 = (int*)PopFreeAudioBuffer();
    for(i = 0; i < ActivePlayer[0x6C / 4]; i++) {
        switch (((u8*)ActivePlayer + i)[0x70]) {
            case 1:
                r31[2] = THPAudioDecode(r31[0], r30[0] * ActivePlayer[0xEC / 4] + r29, 0);
                r31[1] = r31[0];
                PushDecodedAudioBuffer(r31);
                return;
            default:
                r29 += r30[0];
                r30++;
        }
    }
}

void* PopFreeAudioBuffer(void) {
    OSMessage sp8;
    OSReceiveMessage(&FreeAudioBufferQueue, &sp8, 1);
    return sp8;
}

void PushFreeAudioBuffer(void* r3) {
    OSSendMessage(&FreeAudioBufferQueue, r3, 0);
}

void* PopDecodedAudioBuffer(int r3) {
    OSMessage sp8;
    if (OSReceiveMessage(&DecodedAudioBufferQueue, &sp8, r3) == 1) {
        return sp8;
    }
    return 0;
}

void PushDecodedAudioBuffer(void* r3) {
    OSSendMessage(&DecodedAudioBufferQueue, r3, 1);
}