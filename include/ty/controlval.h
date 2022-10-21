
struct ControlVal {
    int activeControls[8];
    int unk[(0x7C-0x20) / 4];
    void Init(int);
    void UseControlConfig(int);
};