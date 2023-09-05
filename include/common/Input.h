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

// "Stick" is the left joystick on the controller
// "C-stick" is the right stick (C-stick)

// TODO document all fields
struct Input_Joypad {
    PADStatus* pPad;

    // Label previous frame fields
    u16 buttonFlags; // Bitflags pressed buttons
    u16 buttonFlags2; // Bitflags pressed buttons

    int unk8; // Stick X
    int unkC; // Stick Y
    int unk10; // C-stick X
    int unk14; // C-stick Y
    int unk18; // Stick X
    int unk1C; // Stick Y
    int unk20; // C-stick X
    int unk24; // C-stick Y
    u8 triggerL; // Left analog trigger
    u8 triggerR; // Right analog trigger
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
