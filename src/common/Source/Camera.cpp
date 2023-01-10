#include "types.h"
#include "common/Camera.h"

KeyMap Camera::keyMapDS;

void Camera::InitModule(void) {
	// possible struct array in Camera::keyMapDS
    keyMapDS.unkA8 = 0xB;
    keyMapDS.unkAC = 0;
    keyMapDS.unkAD = 1;
    keyMapDS.unk5 = 0;
    keyMapDS.unkD = 0;
    keyMapDS.unk10 = 0x11;
    keyMapDS.unk14 = 0;
    keyMapDS.unk15 = 1;
    keyMapDS.unk1D = 0;
    keyMapDS.unk25 = 0;
    keyMapDS.unk28 = 0x10;
    keyMapDS.unk2C = 0;
    keyMapDS.unk2D = 1;
    keyMapDS.unk35 = 0;
    keyMapDS.unk3D = 0;
    keyMapDS.unk40 = 0x10;
    keyMapDS.unk44 = 1;
    keyMapDS.unk45 = 1;
    keyMapDS.unk55 = 0;
    keyMapDS.unk5D = 0;
    keyMapDS.unk60 = 0x13;
    keyMapDS.unk64 = 0;
    keyMapDS.unk65 = 1;
    keyMapDS.unk6D = 0;
    keyMapDS.unk75 = 0;
    keyMapDS.unk78 = 0x12;
    keyMapDS.unk7C = 0;
    keyMapDS.unk7D = 1;
    keyMapDS.unk85 = 0;
    keyMapDS.unk8D = 0;
    keyMapDS.unk90 = 0x12;
    keyMapDS.unk94 = 1;
    keyMapDS.unk95 = 1;
    keyMapDS.unk9D = 0;
    keyMapDS.unkA5 = 0;
    keyMapDS.unk48 = 9;
    keyMapDS.unk4C = 0;
    keyMapDS.unk4D = 1;
}

void Camera::DeinitModule(void) {}

void Camera::Init(void) {
    pMap = &keyMapDS;
    unk0 = 0;
    unk4 = 2;
    unkC = 1.0f;
    unk10 = 0.0f;
    unk14 = 0.0f;
    unk18 = -10.0f;
    unk20 = 0.0f;
    unk24 = 0.0f;
    unk28 = 0.0f;
    unk30 = 0.0f;
    unk34 = 0.0f;
    unk38 = 1.0f;
    unk3C = 10.0f;
    unk40 = 0.0f;
    unk44 = 0.0f;
    unk48 = 1.0f;
    unk4C = 0.0f;
}

void Camera::Deinit(void) {}
