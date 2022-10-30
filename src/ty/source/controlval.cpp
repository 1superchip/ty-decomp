#include "types.h"
#include "ty/controlval.h"

ControlVal tyControl;

void ControlVal::Init(int setting) {
    buttonVals[2] = 0x80;
    buttonVals[3] = 0x20;
    buttonVals[4] = 0x10;
    buttonVals[5] = 0x40;
    buttonVals[0] = 8;
    buttonVals[1] = 1;
    buttonVals[8] = 0x2000;
    buttonVals[6] = 0x4000;
    buttonVals[7] = 0x8000;
    buttonVals[9] = 0x1000;
    buttonVals[10] = 0x400;
    buttonVals[11] = 0x100;
    buttonVals[12] = 2;
    buttonVals[13] = 0x800;
    buttonVals[14] = 0x200;
    buttonVals[15] = 4;
    buttonVals[16] = 0x4000;
    buttonVals[17] = 0x8000;
    buttonVals[18] = 0x2000;
    buttonVals[19] = 0x1000;
    buttonVals[20] = 0x400;
    buttonVals[21] = 0x800;
    buttonVals[22] = 0x200;
    UseControlConfig(setting);
}

void ControlVal::UseControlConfig(int controlConfig) {
    switch (controlConfig) {
        case 0:
            activeControls[0] = buttonVals[6];
            activeControls[2] = buttonVals[7];
            activeControls[1] = buttonVals[8];
            activeControls[3] = buttonVals[9];
            activeControls[5] = buttonVals[20];
            activeControls[4] = buttonVals[22];
            activeControls[6] = buttonVals[21];
            activeControls[7] = buttonVals[4];
            break;
        case 1:
            activeControls[0] = buttonVals[6];
            activeControls[2] = buttonVals[8];
            activeControls[1] = buttonVals[7];
            activeControls[3] = buttonVals[9];
            activeControls[5] = buttonVals[20];
            activeControls[4] = buttonVals[22];
            activeControls[6] = buttonVals[21];
            activeControls[7] = buttonVals[4];
            break;
        case 2:
            activeControls[0] = buttonVals[6];
            activeControls[2] = buttonVals[7];
            activeControls[1] = buttonVals[8];
            activeControls[3] = buttonVals[9];
            activeControls[5] = buttonVals[21];
            activeControls[4] = buttonVals[22];
            activeControls[6] = buttonVals[20];
            activeControls[7] = buttonVals[4];
            break;
        case 3:
            activeControls[0] = buttonVals[6];
            activeControls[2] = buttonVals[8];
            activeControls[1] = buttonVals[7];
            activeControls[3] = buttonVals[9];
            activeControls[5] = buttonVals[21];
            activeControls[4] = buttonVals[22];
            activeControls[6] = buttonVals[20];
            activeControls[7] = buttonVals[4];
            break;
    }
    buttonVals[0] = 8;
    buttonVals[1] = 1;
}