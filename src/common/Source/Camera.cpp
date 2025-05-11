#include "types.h"
#include "common/Camera.h"
#include "common/Debug.h"

KeyMap Camera::keyMapDS[22];

void Camera::InitModule(void) {
    keyMapDS[21].button = 11;
    keyMapDS[21].unk4 = false;
    keyMapDS[21].bEnabled = true;

    keyMapDS[0].bEnabled = false;

    keyMapDS[1].bEnabled = false;

    keyMapDS[2].button = 17;
    keyMapDS[2].unk4 = false;
    keyMapDS[2].bEnabled = true;

    keyMapDS[3].bEnabled = false;

    keyMapDS[4].bEnabled = false;

    keyMapDS[5].button = 16;
    keyMapDS[5].unk4 = false;
    keyMapDS[5].bEnabled = true;

    keyMapDS[6].bEnabled = false;

    keyMapDS[7].bEnabled = false;

    keyMapDS[8].button = 16;
    keyMapDS[8].unk4 = true;
    keyMapDS[8].bEnabled = true;

    keyMapDS[10].bEnabled = false;

    keyMapDS[11].bEnabled = false;

    keyMapDS[12].button = 19;
    keyMapDS[12].unk4 = false;
    keyMapDS[12].bEnabled = true;

    keyMapDS[13].bEnabled = false;

    keyMapDS[14].bEnabled = false;

    keyMapDS[15].button = 18;
    keyMapDS[15].unk4 = false;
    keyMapDS[15].bEnabled = true;

    keyMapDS[16].bEnabled = false;

    keyMapDS[17].bEnabled = false;

    keyMapDS[18].button = 18;
    keyMapDS[18].unk4 = true;
    keyMapDS[18].bEnabled = true;

    keyMapDS[19].bEnabled = false;

    keyMapDS[20].bEnabled = false;

    keyMapDS[9].button = 9;
    keyMapDS[9].unk4 = false;
    keyMapDS[9].bEnabled = true;
}

void Camera::DeinitModule(void) {
    
}

void Camera::Init(void) {
    pMap = keyMapDS;

    mode = (Camera_Mode)0;
    unk4 = 2;
    unkC = 1.0f;

    pos.Set(0.0f, 0.0f, -10.0f);
    target.Set(0.0f, 0.0f, 0.0f);
    dir.Set(0.0f, 0.0f, 1.0f, 10.0f);
    unk40.Set(0.0f, 0.0f, 1.0f, 0.0f);
}

void Camera::Deinit(void) {

}

void Camera::Reposition(Camera_Mode camMode, Vector* pPosition, Vector* pTarget) {
    #if defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
    ASSERT(
        pPosition!=0 && pTarget!=0, 
        "invalid pointer was passed to Camera::Reposition()",
        "D:\\Src\\IceSkating\\BarbieCommon\\Source\\Camera.cpp",
        194
    );
    ASSERT(
        camMode>=CM_FIRST && camMode<CM_COUNT, 
        "invalid camera mode - Camera::Reposition()",
        "D:\\Src\\IceSkating\\BarbieCommon\\Source\\Camera.cpp",
        195
    );
    #endif // defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
    
    mode = camMode;
    unk4 = (Camera_Direction)2;
    pos = *pPosition;
    target = *pTarget;
    
    dir.Sub(pTarget, pPosition);
    dir.w = dir.Normalise();

    unk40.Set(0.0f, 0.0f, 1.0f, 0.0f);
}

void Camera::SetMode(Camera_Mode camMode) {
    if (mode != (int)camMode) {
        mode = camMode;
    }
}

void Camera::SetFixedDir(Camera_Direction camDir) {
    unk40.w = 1.0f;
    
    if (unk4 != (int)camDir) {
        unk4 = camDir;
    }
}

bool Camera::Update(InputDevices device, Vector* p) {
    bool ret = false;
    
    int status = Input_GetDeviceStatus(device);
    if (status != 0) {
        return false;
    }
    
    bool bChangeMode;
    bool bChangeDir;
    float scaledSpeed;
    
    float f21;
    float f23;
    float f28;
    float f24;
    float f25;
    float f27;
    float f22;
    float f26;
    
    if (pMap) {
        if (pMap[9].bEnabled && !pMap[9].unk4) {
            #if defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
            scaledSpeed = 1.0f + (Input_GetButtonState(device, pMap[9].button, NULL) / 255.0f);
            #else
            // July 1st
            scaledSpeed = 1.0f + ((Input_GetButtonState(device, pMap[9].button, NULL) * 50.0f) / 255.0f);
            #endif
        } else {
            scaledSpeed = 1.0f;
        }

        scaledSpeed *= unkC;
        
        if (keyMapDS[21].bEnabled) {
            ret = Input_GetButtonState(device, keyMapDS[21].button, NULL) > 0;
        } else {
            ret = false;
        }

        bChangeMode = 
            pMap[19].bEnabled && ret == pMap[19].unk4 && Input_WasButtonPressed(device, pMap[19].button, NULL);

        bChangeDir = 
            pMap[20].bEnabled && ret == pMap[20].unk4 && Input_WasButtonPressed(device, pMap[20].button, NULL);

        f21 = CalcCameraProp(device, scaledSpeed, ret, 17, 16);
        f21 += CalcAnalogCameraProp(device, scaledSpeed, ret, 18);
        
        f23 = CalcCameraProp(device, scaledSpeed, ret, 6, 7);
        f23 -= CalcAnalogCameraProp(device, scaledSpeed, ret, 8);
        
        f28 = CalcCameraProp(device, 1.0f, ret, 1, 0);
        f28 += CalcAnalogCameraProp(device, 1.0f, ret, 2);

        
        f25 = CalcCameraProp(device, 1.0f, ret, 3, 4);
        f25 -= CalcAnalogCameraProp(device, 1.0f, ret, 5);

        f24 = scaledSpeed * f28;
        f26 = scaledSpeed * f25;
        
        f27 = CalcCameraProp(device, 1.0f, ret, 11, 10);
        f27 += CalcAnalogCameraProp(device, 1.0f, ret, 12);
        
        f22 = CalcCameraProp(device, 1.0f, ret, 13, 14);
        f22 -= CalcAnalogCameraProp(device, 1.0f, ret, 15);

        if (!flipYAxis) {
            f22 = -f22;
        }
        
    } else {
        f26 = 0.0f;
        f24 = 0.0f;
        f23 = 0.0f;
        f21 = 0.0f;
        f25 = 0.0f;
        f28 = 0.0f;
        f22 = 0.0f;
        f27 = 0.0f;
        
        bChangeDir = false;
        bChangeMode = false;
        scaledSpeed = unkC;
    }

    if (bChangeMode) {
        SetMode((Camera_Mode)(mode + 1));
    }

    if (p == NULL && mode != 0) {
        SetMode((Camera_Mode)0);
    }

    if (bChangeDir) {
        SetFixedDir((Camera_Direction)(unk4 + 1));
    }

    if (unk40.w == 1.0f && (Abs<float>(f27) > 1e-06f || Abs<float>(f22) > 1e-06f)) {
        unk40.w = 0.0f;
    }

    Vector tempv;

    QuatRotation rotQ;

    float ySignMod;

    float mag;

    switch (mode) {
        case 0:
            ySignMod = 1.0f;
            if (Abs<float>(f23) > 1e-06f) {
                tempv.Set(0.0f, f23 * 0.2f, 0.0f);
                pos.Add(&tempv);
            }

            if (Abs<float>(f26) > 1e-06f) {
                tempv.Set(dir.x, 0.0f, dir.z);
                tempv.Normalise();
                tempv.Scale(f26 * 0.2f);
                pos.Add(&tempv);
            }

            if (Abs<float>(f24) > 1e-06f) {
                tempv = gYAxis;
                tempv.Cross(&dir);
                tempv.Normalise();
                tempv.Scale(f24 * -0.2f);
                pos.Add(&tempv);
            }
            break;
        case 1:
            if (bChangeMode) {
                tempv.Sub(p, &pos);
                dir.w = tempv.Magnitude();
                if (dir.w < 0.5f) {
                    dir.w = 0.5f;
                }
                
                tempv.Scale(&dir, dir.w);
                target.Add(&pos, &tempv);
            }
            
            ySignMod = -1.0f;
            f22 = -f25;
            f27 = f28;

            dir.w += f21 * 0.2f;
            if (dir.w < 0.5f) {
                dir.w = 0.5f;
            }

            tempv.Sub(p, &target);
            mag = tempv.MagSquared();
            if (mag > scaledSpeed * 0.2f) {
                tempv.Scale((scaledSpeed * 0.2f) / sqrtf(mag));
                target.Add(&tempv);
            } else {
                target = *p;
            }
            break;
        case 2:
            if (bChangeMode) {
                tempv.Sub(p, &pos);
                dir.w = tempv.Magnitude();
                if (dir.w < 0.5f) {
                    dir.w = 0.5f;
                }
                
                tempv.Scale(&dir, dir.w);
                target.Add(&pos, &tempv);
            }
            
            ySignMod = -1.0f;
            f22 = -f25;
            f27 = f28;

            dir.w += f21 * 0.2f;
            if (dir.w < 0.5f) {
                dir.w = 0.5f;
            }

            tempv.Sub(p, &target);
            mag = tempv.MagSquared();
            if (mag > scaledSpeed * 0.2f) {
                tempv.Scale((scaledSpeed * 0.2f) / sqrtf(mag));
                target.Add(&tempv);
            } else {
                target = *p;
            }

            tempv.Sub(p, &pos);
            if (tempv.Normalise()) {
                if (dir.IsInsideSphere(&tempv, 0.03f)) {
                    if (Abs<float>(tempv.y) < 0.999999f) {
                        dir.Copy(&tempv);
                    }
                } else {
                    tempv.Cross(&dir);
                    tempv.Normalise();
                    rotQ.ConvertNormal(&tempv, 0.03f);
                    dir.ApplyQuaternion(&rotQ);
                }
            }
            break;
        default:
            #if defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
            ASSERT(
                false,
                "invalid case value - Camera::Update()",
                "D:\\Src\\IceSkating\\BarbieCommon\\Source\\Camera.cpp",
                457
            );
            #endif // defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
            ySignMod = 1.0f;
            break;
    }

    if (unk40.w == 1.0f) {
        if (dir.IsInsideSphere(&unk40, 0.03f)) {
            if (unk40.y == 0.0f) {
                dir.Copy(&unk40);
            }
            unk40.w = 0.0f;
        } else {
            tempv.Cross(&unk40, &dir);
            tempv.Normalise();
            rotQ.ConvertNormal(&tempv, 0.03f);
            dir.ApplyQuaternion(&rotQ);
        }
    } else {
        tempv = gYAxis;
        tempv.Cross(&dir);
        mag = tempv.Normalise();
        if (mag > 0.005f) {
            if (Abs<float>(f22) > 1e-06f) {
                if (mag < 0.07f && dir.y * ySignMod * f22 < 1e-06f) {
                    mag -= 0.02f;
                } else {
                    mag = 0.05f;
                }

                rotQ.ConvertNormal(&tempv, -mag * ySignMod * f22);
                dir.ApplyQuaternion(&rotQ);
            }
        } else {
            if (Abs<float>(dir.x) > Abs<float>(dir.z)) {
                if (dir.x > 0.0f) {
                    dir.x += 0.01f;
                } else {
                    dir.x -= 0.01f;
                }
            } else if (dir.z > 0.0f) {
                dir.z += 0.01f;
            } else {
                dir.z -= 0.01f;
            }

            dir.Normalise();
        }

        if (Abs<float>(f27) > 1e-06f) {
            tempv = gYAxis;
            rotQ.ConvertNormal(&tempv, ySignMod * 0.05f * f27);
            dir.ApplyQuaternion(&rotQ);
        }
    }
    
    switch (mode) {
        case 0:
            target.Add(&pos, &dir);
            break;
        case 1:
        case 2:
            tempv.Sub(&pos, &target);
            mag = tempv.Magnitude();
            if (mag - dir.w > scaledSpeed * 0.2f) {
                mag -= scaledSpeed * 0.2f;
            } else if (dir.w - mag > scaledSpeed * 0.2f) {
                mag += scaledSpeed * 0.2f;
            } else {
                mag = dir.w;
            }

            tempv.Scale(&dir, mag);
            pos.Sub(&target, &tempv);
            break;
        default:
            #if defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
            ASSERT(
                false,
                "invalid case value - Camera::Update()",
                "D:\\Src\\IceSkating\\BarbieCommon\\Source\\Camera.cpp",
                543
            );
            #endif // defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
            break;
    }

    return ret;
}

// This is closer for the July 1st build
// float Camera::CalcCameraProp(InputDevices device, float f1, bool b, int arg2, int arg3) {
//     float f;

//     bool b2 = pMap[arg2].bEnabled && b == pMap[arg2].unk4;
//     if (b2) {
//         f = Input_GetButtonState(device, pMap[arg2].button, NULL);
//     } else {
//         f = 0.0f;
//     }

//     if (f > 0.0f) {
//         return (f1 * f) / 255.0f;
//     }

//     float f2;

//     b2 = pMap[arg3].bEnabled && b == pMap[arg3].unk4;

//     if (b2) {
//         f2 = Input_GetButtonState(device, pMap[arg3].button, NULL);
//     } else {
//         f2 = f;
//     }

//     return (f1 * -f2) / 255.0f;
// }

float Camera::CalcCameraProp(InputDevices device, float f1, bool b, int arg2, int arg3) {
    float f;

    bool b2 = pMap[arg2].bEnabled && b == pMap[arg2].unk4;
    if (b2) {
        f = Input_GetButtonState(device, pMap[arg2].button, NULL);
        if (f > 0.0f) {
            return (f1 * f) / 255.0f;
        }
    }

    float f2;

    b2 = pMap[arg3].bEnabled && b == pMap[arg3].unk4;

    if (b2) {
        f2 = Input_GetButtonState(device, pMap[arg3].button, NULL);
    } else {
        f2 = 0.0f;
    }
    
    return (f1 * f2) / -255.0f;
}

float Camera::CalcAnalogCameraProp(InputDevices inputDevice, float f1, bool b, int arg3) {
    if (pMap[arg3].bEnabled && b == pMap[arg3].unk4) {
        return f1 * GetAnalogButtonState(inputDevice, pMap[arg3].button) / 255.0f;
    } else {
        return 0.0f;
    }
}

#if defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
#define PSX_BID_LVERTANALOG (0x10)
#define PSX_BID_LHORIZANALOG (0x11)
#define PSX_BID_RHORIZANALOG (0x13)
#define PSX_BID_RVERTANALOG (0x12)
#endif // defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1

float Camera::GetAnalogButtonState(InputDevices inputDevice, int keyId) {
    #if defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
    ASSERT(
        keyId == PSX_BID_LHORIZANALOG || keyId == PSX_BID_LVERTANALOG || keyId == PSX_BID_RHORIZANALOG || keyId == PSX_BID_RVERTANALOG,
        "Key %d is not an analog control",
        "D:\\Src\\IceSkating\\BarbieCommon\\Source\\Camera.cpp",
        630
    );
    #endif // defined(BARBIE_ICESKATING) && BARBIE_ICESKATING == 1
    
    float ret = Input_GetButtonState(inputDevice, keyId, NULL);

    if (keyId == 0x11 || keyId == 0x10 || keyId == 0x13 || keyId == 0x12) {
        if (ret < 128.0f) {
            ret = (127.0f - ret) * -1.0f;
        } else {
            ret -= 127.0f;
        }

        ret += ret;

        float x = 102.0f;
        float z = (5.0f / 3.0f);

        if (ret < -x) {
            ret = (ret + x) * z;
        } else if (ret > x) {
            ret = (ret - x) * z;
        } else {
            ret = 0.0f;
        }
        
    }

    return ret;
}

void Camera::DrawDebug(float x, float* pY) {
    gpDebugFont->DrawText(
        Str_Printf("Camera:"), 
        x, *pY, 
        1.0f, 1.0f, 
        FONT_JUSTIFY_0, 0x40808080
    );


    *pY += 20.0f;
    x += 10.0f;

    gpDebugFont->DrawText(
        Str_Printf("pos: %s", FormatVector(&pos)), 
        x, *pY, 
        1.0f, 1.0f, 
        FONT_JUSTIFY_0, 0x40808080
    );

    *pY += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("target: %s", FormatVector(&target)), 
        x, *pY, 
        1.0f, 1.0f, 
        FONT_JUSTIFY_0, 0x40808080
    );

    *pY += 20.0f;

    gpDebugFont->DrawText(
        Str_Printf("dir: %s", FormatVector(&dir)), 
        x, *pY, 
        1.0f, 1.0f, 
        FONT_JUSTIFY_0, 0x40808080
    );

    *pY += 20.0f;

    char* modeNames[3] = {
        "no target",
        "target (fixed)",
        "target (free)"
    };

    gpDebugFont->DrawText(
        Str_Printf("mode: %s", modeNames[mode]), 
        x, *pY, 
        1.0f, 1.0f, 
        FONT_JUSTIFY_0, 0x40808080
    );

    *pY += 20.0f;
}
