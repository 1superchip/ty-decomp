#include "types.h"
#include "Dolphin/os.h"
#include "Dolphin/THP/THPRead.h"
#include "Dolphin/THP/THPAudioDecode.h"
#include "Dolphin/THP/THPPlayer.h"

void* PopReadedBuffer(void);
void* PopFreeAudioBuffer(void);

void PushDecodedAudioBuffer(void*);

static void* AudioDecoderForOnMemory(void*);
static void* AudioDecoder(void*);

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

static void* AudioDecoderForOnMemory(void* bufPtr) {
    s32 readSize;
    s32 frame;
    THPReadBuffer readBuffer;

    frame           = 0;
    readSize        = ActivePlayer.mInitReadSize;
    readBuffer.mPtr = (u8*)bufPtr;

    while (TRUE) {
        s32 remaining;
        readBuffer.mFrameNumber = frame;
        AudioDecode(&readBuffer);

        remaining = (frame + ActivePlayer.mInitReadFrame) % ActivePlayer.mHeader.mNumFrames;

        if (remaining == ActivePlayer.mHeader.mNumFrames - 1) {
            if ((ActivePlayer.mPlayFlag & 1)) {
                readSize        = *(s32*)readBuffer.mPtr;
                readBuffer.mPtr = ActivePlayer.mMovieData;
            } else {
                OSSuspendThread(&AudioDecodeThread);
            }
        } else {
            s32 size = *(s32*)readBuffer.mPtr;
            readBuffer.mPtr += readSize;
            readSize = size;
        }
        frame++;
    }
}


static void AudioDecode(THPReadBuffer* readBuffer) {
    THPAudioBuffer* audioBuf;
    s32 i;
    u32* offsets;
    u8* audioData;

    offsets   = (u32*)(readBuffer->mPtr + 8);
    audioData = &readBuffer->mPtr[ActivePlayer.mCompInfo.mNumComponents * 4] + 8;
    audioBuf  = (THPAudioBuffer*)PopFreeAudioBuffer();

    for (i = 0; i < ActivePlayer.mCompInfo.mNumComponents; i++) {
        switch (ActivePlayer.mCompInfo.mFrameComp[i]) {
            case 1: {
                audioBuf->mValidSample = THPAudioDecode(audioBuf->mBuffer, (audioData + *offsets * ActivePlayer.mCurAudioTrack), 0);
                audioBuf->mCurPtr      = audioBuf->mBuffer;
                PushDecodedAudioBuffer(audioBuf);
                return;
            }
        }

        audioData += *offsets;
        offsets++;
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

void* PopDecodedAudioBuffer(long r3) {
    OSMessage sp8;
    if (OSReceiveMessage(&DecodedAudioBufferQueue, &sp8, r3) == 1) {
        return sp8;
    }
    return 0;
}

void PushDecodedAudioBuffer(void* r3) {
    OSSendMessage(&DecodedAudioBufferQueue, r3, 1);
}
