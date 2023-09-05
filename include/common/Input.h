#ifndef COMMON_INPUT
#define COMMON_INPUT

#include "Dolphin/pad.h"

// might be IDEV_JOY%d and IDEV_JOYALL?
enum InputDevices {
    CHAN_0      = 0,
    CHAN_1      = 1,
    CHAN_2      = 2,
    CHAN_3      = 3,
    ALL_CHANS   = 4,
    MAX_CHANS   = 5
};

// TODO document all fields
struct Input_Joypad {
    PADStatus* pPad;
    u16 unk4;
    u16 unk6;
    int unk8;
    int unkC;
    int unk10;
    int unk14;
    int unk18;
    int unk1C;
    int unk20;
    int unk24;
    u8 triggerL;
    u8 triggerR;
};

void Input_InitModule(void);
void Input_DeinitModule(void);
void Input_Update(bool bReadInputs);
int Input_GetDeviceStatus(InputDevices deviceID);
int Input_GetButtonState(InputDevices deviceID, int button, InputDevices* pFoundDevice);
bool Input_WasButtonPressed(InputDevices deviceID, int button, InputDevices* pFoundDevice);
void Input_Vibrate(InputDevices deviceID, int r4, bool r5);
void Input_ClearPadData(void);
bool Input_HasAnyButtonChanged(void);
void Input_EnableKeyMapping(bool bEnableKeyMapping);
bool Input_IsKeyMappingEnabled(void);
void Input_StopAllVibration(void);
bool Input_IsButtonStick(int buttonVal);
void PADClampCircle(PADStatus* status);

#endif // COMMON_INPUT
