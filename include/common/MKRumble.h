#ifndef COMMON_MKRUMBLE
#define COMMON_MKRUMBLE

#define INPUTDEVICE_COUNT (5)
#define NUM_VIBRA_EFFECTS (8)

enum InputDevices {};

struct InputDeviceStruct {
    int unk0;
    bool unk4;
    float unk8;
};

struct VibrationEffect {
    float unk0;
    float unk4;
    float unk8;
    float unkC;
    char flags;
    int unk14;
    float unk18;
};

void MKRumble_Reset(void);
void MKRumble_Update(void);
void MKRumble_Pause(void);
void MKRumble_Resume(void);
void MKRumble_Play(InputDevices, float, float, float, char effectFlags, float);
void MKRumble_Stop(InputDevices targetDevice);

#endif // COMMON_MKRUMBLE
