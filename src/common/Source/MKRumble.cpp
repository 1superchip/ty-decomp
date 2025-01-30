#include "common/MKRumble.h"
#include "common/StdMath.h"

/*
// File: MKRumble.cpp
// File to control rumble of controllers
*/

extern "C" double fmod(double, double);

static InputDeviceStruct ves[MAX_CHANS] = {
    {0, false, 0.0f}, 
    {0, false, 0.0f}, 
    {0, false, 0.0f},
    {0, false, 0.0f},
    {0, false, 0.0f}
};

static VibrationEffect vibrationEffect[NUM_VIBRA_EFFECTS];

static int effectsPlaying = 0;
static bool vibrationPaused = false;

/// @brief Resets all vibration effects
/// @param None
void MKRumble_Reset(void) {
    vibrationPaused = false;
    for(int i = 0; i < MAX_CHANS; i++) {
        ves[i].unk0 = 0;
        ves[i].unk4 = 0;
        ves[i].unk8 = 0;
        Input_Vibrate((InputDevices)i, 0, 0);
    }
    effectsPlaying = 0;
    for(int i = 0; i < NUM_VIBRA_EFFECTS; i++) {
        vibrationEffect[i].unk14 = 0;
        vibrationEffect[i].unk0 = 60.0f;
        vibrationEffect[i].unk4 = 1.0f;
        vibrationEffect[i].unk8 = 1.0f;
        vibrationEffect[i].flags = 0;
        vibrationEffect[i].unkC = 0.0f;
        vibrationEffect[i].unk18 = 0.0f;
    }
}

/// @brief Updates all vibration effects
/// @param None
void MKRumble_Update(void) {
    // if vibrations are paused, no need to update
    if (vibrationPaused) return;
    
    float largeActMag[NUM_VIBRA_EFFECTS] = {};
    float smallActMag[NUM_VIBRA_EFFECTS] = {};
    bool newSmallActState[NUM_VIBRA_EFFECTS];
    int newLargeActState[NUM_VIBRA_EFFECTS];

    for (int i = 0; i < effectsPlaying; i++) {
        VibrationEffect* pEffect = &vibrationEffect[i];
        float f31;

        if (pEffect->flags & 1) {
            f31 = 1.0f - pEffect->unk18;
        } else if (pEffect->flags & 2) {
            f31 = pEffect->unk18;
        } else {
            f31 = 1.0f;
        }

        if (pEffect->flags & 4) {
            f31 *= f31;
        }

        switch (pEffect->flags & 0xf0) {
            case 0x0:
                break;
            case 0x10:
                f31 *= _table_cosf(pEffect->unk18 * pEffect->unkC);
                break;
            case 0x20:
                float f2 = fmod(pEffect->unk18 * pEffect->unkC, 1.0f);
                f31 *= (float)__fabs((f2 * 2.0f) - 1.0f);
                break;
            case 0x40:
                if ((float)fmod(pEffect->unk18 * pEffect->unkC, 1.0f) > 0.5f) {
                    f31 = 0.0f;
                }
                break;
        }

        pEffect->unk18 += pEffect->unk0;
        smallActMag[pEffect->unk14] += f31 * pEffect->unk4;
        largeActMag[pEffect->unk14] += f31 * pEffect->unk8;

        if (pEffect->unk18 >= 1.0f) {
            if (i < effectsPlaying - 1) {
                vibrationEffect[i] = vibrationEffect[effectsPlaying - 1];
            }
            i--;
            effectsPlaying--;
        }
    }

    for (int i = 0; i < MAX_CHANS; i++) {
        if (smallActMag[i] > 1.0f) {
            smallActMag[i] = 1.0f;
        }

        if (largeActMag[i] > 1.0f) {
            largeActMag[i] = 1.0f;
        }

        ves[i].unk8 += smallActMag[i];

        if (ves[i].unk8 > 0.0f) {
            newSmallActState[i] = true;
            ves[i].unk8 -= 1.0f;
        } else {
            newSmallActState[i] = false;
        }

        newLargeActState[i] = (int)(largeActMag[i] * 255.0f);

        if (newSmallActState[i] != ves[i].unk4 || newLargeActState[i] != ves[i].unk0) {
            Input_Vibrate((InputDevices)i, newLargeActState[i], newSmallActState[i]);
            ves[i].unk0 = newLargeActState[i];
            ves[i].unk4 = newSmallActState[i];
        }
    }
}

/// @brief Pauses the rumble of all input devices
/// @param  None
void MKRumble_Pause(void) {
    vibrationPaused = true; // Set vibrations to paused
    for (int i = 0; i < MAX_CHANS; i++) {
        ves[i].unk0 = 0;
        ves[i].unk4 = false;
        ves[i].unk8 = 0.0f;
        Input_Vibrate((InputDevices)i, 0, false);
    }
}

/// @brief Resumes MKRumble vibrations
/// @param  None
void MKRumble_Resume(void) {
    // Resume vibrations
    vibrationPaused = false;
}

/// @brief Adds a vibration effect
/// @param param_1 
/// @param param_2 
/// @param param_3 
/// @param param_4 
/// @param effectFlags Flags for the vibration effect
/// @param param_6 
void MKRumble_Play(InputDevices param_1, float param_2, float param_3, 
        float param_4, char effectFlags, float param_6) {
    if (effectsPlaying >= NUM_VIBRA_EFFECTS) {
        return;
    }
    vibrationEffect[effectsPlaying].unk14 = param_1;
    vibrationEffect[effectsPlaying].unk4 = param_3;
    vibrationEffect[effectsPlaying].unk8 = param_2;
    vibrationEffect[effectsPlaying].unk0 = 1.0f / (60.0f * param_4);
    vibrationEffect[effectsPlaying].flags = effectFlags;
    vibrationEffect[effectsPlaying].unkC = param_4 * param_6;
    vibrationEffect[effectsPlaying].unk18 = 0.0f;
    ves[param_1].unk8 = 0.0f;
    effectsPlaying++;
}

/// @brief Stops all vibration effects that have their target equal to targetDevice
/// @param targetDevice Device ID to stop all vibration effects for
void MKRumble_Stop(InputDevices targetDevice) {
    for (int i = 0; i < effectsPlaying; i++) {
        if (vibrationEffect[i].unk14 == targetDevice) {
            if (i < effectsPlaying - 1) {
                vibrationEffect[i] = vibrationEffect[effectsPlaying - 1];
            }
            
            i--;
            effectsPlaying--;
        }
    }
}
