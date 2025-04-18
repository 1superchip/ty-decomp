#include "common/Input.h"
#include "common/StdMath.h"

// TODO: Document all unknown fields

// TODO: Use PAD button flags
static const u16 ButtonMasks[20] = {
    0x0000, 0x0000, 0x0000, 0x1000,
    0x0008, 0x0002, 0x0004, 0x0001,
    0x0000, 0x0010, 0x0040, 0x0020,
    0x0800, 0x0400, 0x0100, 0x0200,
    0x0000, 0x0000, 0x0000, 0x0000
};

static PADStatus pad[PAD_MAX_CONTROLLERS];

static int JoyStickCount = 1;
static bool bEnableDS2KeyMapping = true;

static Input_Joypad joyPad[PAD_MAX_CONTROLLERS] = {
    {&pad[0], 0x0000, 0x0000, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0},
    {&pad[1], 0x0000, 0x0000, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0},
    {&pad[2], 0x0000, 0x0000, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0},
    {&pad[3], 0x0000, 0x0000, 127, 127, 127, 127, 127, 127, 127, 127, 0, 0}
};

void Input_InitModule(void) {
    PADInit();
}

void Input_DeinitModule(void) {}

void Input_Update(bool bReadInputs) {
    if (bReadInputs) {
        PADRead(pad);
        PADClampCircle(pad);
    }
    
    static u32 connectedBits = 0;
    u32 invalidControllers = 0;
    for (int i = 0; i < JoyStickCount; i++) {
        u32 r9 = (0x80000000 >> i);

        // Copy inputs to previous frame inputs
        joyPad[i].mPrevButtonFlags = joyPad[i].mCurrButtonFlags;
        joyPad[i].mPrevStickX = joyPad[i].mCurrStickX;
        joyPad[i].mPrevStickY = joyPad[i].mCurrStickY;
        joyPad[i].mPrevSubStickX = joyPad[i].mCurrSubStickX;
        joyPad[i].mPrevSubStickY = joyPad[i].mCurrSubStickY;

        switch (pad[i].err) {
            case PAD_ERR_TRANSFER:
            case PAD_ERR_NONE:
                connectedBits |= r9;
                break;
            case PAD_ERR_NO_CONTROLLER:
                invalidControllers |= r9;
                break;
        }

        if (pad[i].err == PAD_ERR_NONE) {
            joyPad[i].mCurrButtonFlags = joyPad[i].pPad->button;
            
            joyPad[i].mCurrStickX = Clamp<int>(-128, (joyPad[i].pPad->stickX * 2), 127) + 128;
            joyPad[i].mCurrStickY = Clamp<int>(-128, -(joyPad[i].pPad->stickY * 2), 127) + 128;
            joyPad[i].mCurrSubStickX = Clamp<int>(-128, (joyPad[i].pPad->substickX * 2), 127) + 128;
            joyPad[i].mCurrSubStickY = Clamp<int>(-128, -(joyPad[i].pPad->substickY * 2), 127) + 128;
            
            if (joyPad[i].mCurrStickX < 0) {
                joyPad[i].mCurrStickX = 0;
            } else if (joyPad[i].mCurrStickX > 0xff) {
                joyPad[i].mCurrStickX = 0xff;
            }

            if (joyPad[i].mCurrStickY < 0) {
                joyPad[i].mCurrStickY = 0;
            } else if (joyPad[i].mCurrStickY > 0xff) {
                joyPad[i].mCurrStickY = 0xff;
            }

            if (joyPad[i].mCurrSubStickX < 0) {
                joyPad[i].mCurrSubStickX = 0;
            } else if (joyPad[i].mCurrSubStickX > 0xff) {
                joyPad[i].mCurrSubStickX = 0xff;
            }

            if (joyPad[i].mCurrSubStickY < 0) {
                joyPad[i].mCurrSubStickY = 0;
            } else if (joyPad[i].mCurrSubStickY > 0xff) {
                joyPad[i].mCurrSubStickY = 0xff;
            }
            
            joyPad[i].triggerL = joyPad[i].pPad->triggerL;
            joyPad[i].triggerR = joyPad[i].pPad->triggerR;
        } else {
            // if there is an error such as the controller is unplugged
            joyPad[i].mCurrButtonFlags = 0;
            joyPad[i].mCurrStickX = 0x7f;
            joyPad[i].mCurrStickY = 0x7f;
            joyPad[i].mCurrSubStickX = 0x7f;
            joyPad[i].mCurrSubStickY = 0x7f;
            joyPad[i].triggerL = 0;
            joyPad[i].triggerR = 0;
        }
    }

    if (connectedBits != 0) {
        invalidControllers &= connectedBits;
    }

    if (invalidControllers && bReadInputs) {
        PADReset(invalidControllers);
    }
}

/// @brief Gets the status of deviceID
/// @param deviceID Always checks CHAN_0 no matter passed value
/// @return 
int Input_GetDeviceStatus(InputDevices deviceID) {
    // InputDevices device = CHAN_0;
    int status = 2;
    deviceID = CHAN_0;
    if (deviceID == ALL_CHANS) {
        // Never will happen!
        // This code checks all channels
        for (int i = 0; i < 4; i++) {
            status = Input_GetDeviceStatus((InputDevices)i);
            if (status == 0) {
                break;
            }
        }
    } else if (deviceID != ALL_CHANS) {
        switch (deviceID) {
            default:
            case ALL_CHANS:
            case MAX_CHANS:
                break;
            case CHAN_0:
            case CHAN_1:
            case CHAN_2:
            case CHAN_3:
                switch (joyPad[deviceID].pPad->err) {
                    case PAD_ERR_NONE:
                        status = 0;
                        break;
                    case PAD_ERR_NOT_READY:
                        status = 1;
                        break;
                    case PAD_ERR_NO_CONTROLLER:
                    case PAD_ERR_TRANSFER:
                        status = 2;
                        break;
                }
                break;
        }
    }

    return status;
}

/// @brief 
/// @param deviceID Always checks CHAN_0 no matter the value passed
/// @param button Button to get state
/// @param pFoundDevice Pointer to store found Channel number to
/// @return 
int Input_GetButtonState(InputDevices deviceID, int button, InputDevices* pFoundDevice) {
    deviceID = CHAN_0; // Only check Channel 0
    if (deviceID == ALL_CHANS) {
        // Never will happen
        // This code checks every channel for the button
        bool isStickButton = Input_IsButtonStick(button);
        for (int i = 0; i < 4; i++) {
            int buttonState = Input_GetButtonState((InputDevices)i, button, pFoundDevice);
            if (isStickButton) {
                if (buttonState < 107 || buttonState > 147) {
                    return buttonState;
                }
            } else if (buttonState != 0) {
                return buttonState;
            }
        }

        return isStickButton ? 0x7f : 0;
    }

    int ret = 0;
    switch (deviceID) {
        case CHAN_0:
        case CHAN_1:
        case CHAN_2:
        case CHAN_3:
            switch (button) {
                case 17:
                    ret = joyPad[deviceID].mCurrStickX;
                    break;
                case 16:
                    ret = joyPad[deviceID].mCurrStickY;
                    break;
                case 19:
                    ret = joyPad[deviceID].mCurrSubStickX;
                    break;
                case 18:
                    ret = joyPad[deviceID].mCurrSubStickY;
                    break;
                case 11:
                    ret = joyPad[deviceID].triggerR;
                    break;
                case 10:
                    ret = joyPad[deviceID].triggerL;
                    break;
                case 12:
                default:
                    if (joyPad[deviceID].mCurrButtonFlags & ButtonMasks[button]) {
                        ret = 0xff;
                    }

                    break;
            }
    }

    return ret;
}

/// @brief Returns whether button was pressed or not
/// @param deviceID Always check CHAN_0 
/// @param button Button to check if pressed
/// @param pFoundDevice (Unused) Pointer to store found Channel number to
/// @return 
bool Input_WasButtonPressed(InputDevices deviceID, int button, InputDevices* pFoundDevice) {
    deviceID = CHAN_0; // Only check Channel 0

    if (deviceID == ALL_CHANS) {
        // Never will happen
        // This code checks if the button was pressed by any channel
        for (int i = 0; i < 4; i++) {
            int wasPressed = Input_WasButtonPressed((InputDevices)i, button, pFoundDevice);
            if (wasPressed) {
                return wasPressed;
            }
        }

        return false;
    }

    switch (deviceID) {
        case CHAN_0:
        case CHAN_1:
        case CHAN_2:
        case CHAN_3:
            if ((joyPad[deviceID].mCurrButtonFlags & ButtonMasks[button]) &&
                (joyPad[deviceID].mPrevButtonFlags & ButtonMasks[button]) == 0) {
                return true;
            }
            break;
    }

    return false;
}

void Input_Vibrate(InputDevices deviceID, int r4, bool r5) {
    int padChan;
    deviceID = CHAN_0; // Only ever vibrate channel 0
    switch (deviceID) {
        case CHAN_0:
            padChan = PAD_CHAN0;
            break;
        case CHAN_1:
            padChan = PAD_CHAN1;
            break;
        case CHAN_2:
            padChan = PAD_CHAN2;
            break;
        case CHAN_3:
            padChan = PAD_CHAN3;
            break;
        default:
            padChan = PAD_CHAN0;
            break;
    }

    if (r4 != 0) {
        PADControlMotor(padChan, PAD_MOTOR_RUMBLE);
    } else {
        PADControlMotor(padChan, PAD_MOTOR_STOP_HARD);
    }
}

/// @brief Clears certain data in all joypads
/// @param  None
void Input_ClearPadData(void) {
    for (int i = 0; i < PAD_MAX_CONTROLLERS; i++) {
        joyPad[i].mPrevButtonFlags = 0;
        joyPad[i].mCurrButtonFlags = 0;
        joyPad[i].mCurrStickX = 0x7f;
        joyPad[i].mCurrStickY = 0x7f;
        joyPad[i].mCurrSubStickX = 0x7f;
        joyPad[i].mCurrSubStickY = 0x7f;
    }
}

/// @brief Checks if any button has changed on channel 0
/// @param  None
/// @return Whether any button has changed
bool Input_HasAnyButtonChanged(void) {
    for (int i = 0; i <= 15; i++) {
        if (Input_WasButtonPressed(CHAN_0, i, NULL)) {
            return true;
        }
    }

    return false;
}

void Input_EnableKeyMapping(bool bEnableKeyMapping) {
    bEnableDS2KeyMapping = bEnableKeyMapping;
}

bool Input_IsKeyMappingEnabled(void) {
    return bEnableDS2KeyMapping;
}

void Input_StopAllVibration(void) {
    for (int i = 0; i < PAD_CHANMAX; i++) {
        PADControlMotor(i, PAD_MOTOR_STOP_HARD);
    }
}

bool Input_IsButtonStick(int buttonVal) {
    switch (buttonVal) {
        case 16:
        case 17:
        case 18:
        case 19:
            return true;
        default:
            return false;
    }
}

static void ClampGenericStick(s8*, s8*, float radius, float radiusSquared, long min);
static void ClampGenericTrigger(u8* p, u8 min, u8 max);

#define STICK_MIN           (15)
#define STICK_RADIUS        (56.0f)
#define SUBSTICK_RADIUS     (44.0f)
#define CLAMP_MIN_TRIGGER   (30)
#define CLAMP_MAX_TRIGGER   (180)

void PADClampCircle(PADStatus* status) {
    for (u32 i = 0; i < PAD_MAX_CONTROLLERS; ++i, status++) {
        if (status->err != 0) {
          continue;
        }

        ClampGenericStick(&status->stickX, &status->stickY, STICK_RADIUS, STICK_RADIUS * STICK_RADIUS, STICK_MIN);
        ClampGenericStick(&status->substickX, &status->substickY, SUBSTICK_RADIUS, SUBSTICK_RADIUS * SUBSTICK_RADIUS, STICK_MIN);
        ClampGenericTrigger(&status->triggerL, CLAMP_MIN_TRIGGER, CLAMP_MAX_TRIGGER);
        ClampGenericTrigger(&status->triggerR, CLAMP_MIN_TRIGGER, CLAMP_MAX_TRIGGER);
    }
}

static void ClampGenericStick(s8* px, s8* py, float radius, float radiusSquared, long min) {
  s32 x = *px;
  s32 y = *py;
  float squared;
  float length;

  if (x < min && x > -min) {
    x = 0;
  } else if (0 < x) {
    x -= min;
  } else {
    x += min;
  }

  if (y < min && y > -min) {
    y = 0;
  } else if (0 < y) {
    y -= min;
  } else {
    y += min;
  }

  squared = x * x + y * y;
  if (squared > radiusSquared) {
    length = sqrtf(squared);
    x = (x / length) * radius;
    y = (y / length) * radius;
  }
    *px = Clamp<float>(-128.0f, x * 1.5f, 127.0f);
    *py = Clamp<float>(-128.0f, y * 1.5f, 127.0f);
}

static void ClampGenericTrigger(u8* trigger, u8 min, u8 max) {
    if (*trigger <= min) {
        *trigger = 0;
    } else {
        if (*trigger > max) {
            *trigger = max;
        }
        *trigger -= min;
    }
}
