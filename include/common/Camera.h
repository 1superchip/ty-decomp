#ifndef COMMON_CAMERA
#define COMMON_CAMERA

#include "common/Input.h"
#include "common/Vector.h"

enum Camera_Mode {
    CM_FIRST = 0,
    // 
    CM_COUNT = 3,
};

enum Camera_Direction {

};

struct KeyMap {
    int button;
    bool unk4; // bool?
    bool bEnabled; // bool?
};

struct Camera {
    static void InitModule(void);
    static void DeinitModule(void);
    void Init(void);
    void Deinit(void);

    void Reposition(Camera_Mode camMode, Vector* pPosition, Vector* pTarget);

    float GetAnalogButtonState(InputDevices, int);
    float CalcAnalogCameraProp(InputDevices, float, bool, int);
    float CalcCameraProp(InputDevices, float, bool, int, int);

    void SetFixedDir(Camera_Direction);
    void SetMode(Camera_Mode);

    void DrawDebug(float x, float* pY);

    bool Update(InputDevices device, Vector*);
    
    Camera_Mode mode;
    int unk4;
    KeyMap* pMap;
    float unkC;
    Vector pos;
    Vector target;
    Vector dir;
    Vector unk40;
    
    static KeyMap keyMapDS[22];
    static bool flipYAxis;
};

#endif // COMMON_CAMERA
