#ifndef CONTROLVAL_H
#define CONTROLVAL_H

struct ControlVal {
	// come up with a better names
    int activeControls[8];
    int buttonVals[(0x7C-0x20) / 4];
    void Init(int);
    void UseControlConfig(int);
};

extern ControlVal tyControl;

#endif // CONTROLVAL_H
