
struct ControlVal {
	// come up with a better names
    int activeControls[8];
    int buttonVals[(0x7C-0x20) / 4];
    void Init(int);
    void UseControlConfig(int);
};