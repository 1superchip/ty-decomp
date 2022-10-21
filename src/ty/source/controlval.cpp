#include "types.h"
#include "ty/controlval.h"

ControlVal tyControl;

void ControlVal::Init(int setting) {
    unk[2] = 0x80;
    unk[3] = 0x20;
    unk[4] = 0x10;
    unk[5] = 0x40;
    unk[0] = 8;
    unk[1] = 1;
    unk[8] = 0x2000;
    unk[6] = 0x4000;
    unk[7] = 0x8000;
    unk[9] = 0x1000;
    unk[10] = 0x400;
    unk[11] = 0x100;
    unk[12] = 2;
    unk[13] = 0x800;
    unk[14] = 0x200;
    unk[15] = 4;
    unk[16] = 0x4000;
    unk[17] = 0x8000;
    unk[18] = 0x2000;
    unk[19] = 0x1000;
    unk[20] = 0x400;
    unk[21] = 0x800;
    unk[22] = 0x200;
    UseControlConfig(setting);
}

void ControlVal::UseControlConfig(int config) {
    switch (config) {
        case 0:
            activeControls[0] = unk[6];
            activeControls[2] = unk[7];
            activeControls[1] = unk[8];
            activeControls[3] = unk[9];
            activeControls[5] = unk[20];
            activeControls[4] = unk[22];
            activeControls[6] = unk[21];
            activeControls[7] = unk[4];
            break;
        case 1:
            activeControls[0] = unk[6];
            activeControls[2] = unk[8];
            activeControls[1] = unk[7];
            activeControls[3] = unk[9];
            activeControls[5] = unk[20];
            activeControls[4] = unk[22];
            activeControls[6] = unk[21];
            activeControls[7] = unk[4];
            break;
        case 2:
            activeControls[0] = unk[6];
            activeControls[2] = unk[7];
            activeControls[1] = unk[8];
            activeControls[3] = unk[9];
            activeControls[5] = unk[21];
            activeControls[4] = unk[22];
            activeControls[6] = unk[20];
            activeControls[7] = unk[4];
            break;
        case 3:
            activeControls[0] = unk[6];
            activeControls[2] = unk[8];
            activeControls[1] = unk[7];
            activeControls[3] = unk[9];
            activeControls[5] = unk[21];
            activeControls[4] = unk[22];
            activeControls[6] = unk[20];
            activeControls[7] = unk[4];
            break;
    }
    unk[0] = 8;
    unk[1] = 1;
}