#ifndef CONTROLVAL_H
#define CONTROLVAL_H

#include "types.h"

struct ControlVal {
	// come up with a better names
    int activeControls[8];
    int buttonVals[23];
    
    void Init(int);
    void UseControlConfig(int);
};

extern ControlVal tyControl;

#endif // CONTROLVAL_H
