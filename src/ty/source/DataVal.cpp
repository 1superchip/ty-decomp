#include "types.h"
#include "ty/DataVal.h"
#include "common/Str.h"

extern "C" int stricmp(char*, char*);

extern bool gAssertBool; // from Str.cpp

inline float kin_sqrtf(float x) {
    static const double _half=.5;
	static const double _three=3.0;
    double guess = __frsqrte(x);
    guess = _half * guess * (_three - ( x * (guess * guess)));
    guess = _half * guess * (_three - ( x * (guess * guess)));
    volatile float y = ( x * (_half * guess * (_three - ( x * (guess * guess)))));
    return y;
}

// should these functions be moved to KromeIniLine?
bool Load_Float(KromeIniLine* pIniLine, float* pFloat, char* name) {
    if (stricmp(pIniLine->pFieldName, name) == 0) {
        return pIniLine->AsFloat(0, pFloat);
    }
    return false;
}

bool Load_Int(KromeIniLine* pIniLine, int* pInt, char* name) {
    if (stricmp(pIniLine->pFieldName, name) == 0) {
        return pIniLine->AsInt(0, pInt);
    }
    return false;
}

void CalcYStep(float gravity, float height, float* pYStep) {
    if (height > 0.0f) {
        float jump = (2.0f * gravity) * height;
        if (jump > 0.0f) {
            jump = kin_sqrtf(jump);
        }
        *pYStep = jump;
    }
}

void DataVal::Load(void) {
    KromeIni dataIni;
    bool bFieldFound;
	
	// inline here?
	// void DataVal::Init(void)?
    nearPlane = 100.0f;
    enemyChaseSpeed = 5.0f;
    enemyPatrolSpeed = 2.0f;
    enemyHeroRange = 500.0f;
    enemyRoamRange = 500.0f;
    bounceHeight = 530.0f;
    warpSpeed = 2500.0f;
    iceAcceleration = 0.6f;
    walkMagnitude = 3.0f;
    runMagnitude = 12.0f;
    jumpMagnitude = 35.0f;
    jogMagnitude = 9.0f;
    sneakMagnitude = 1.4f;
    waterJumpMagnitude = 35.0f;
    glideMagnitude = 15.0f;
    jumpYStep = 18.0f;
    jumpGravity = 0.75f;
    jumpMaxGravity = 64.0f;
    jumpHeight = -1.0f;
    ledgeGrabTolerance = 8.0f;
    waterJumpHeight = -1.0f;
    glideJumpYStep = -25.0f;
    glideJumpGravity = 2.0f;
    glideJumpMaxGravity = 64.0f;
    glideJumpHeight = -1.0f;
    glideYStep = 0.0f;
    glideGravity = 0.8f;
    glideMaxGravity = 8.0f;
    fallTurnFactor = 0.07f;
    glideTurnFactor = 0.07f;
    walkJumpTurnFactor = 0.2f;
    runJumpTurnFactor = 0.02f;
    swimTurnFactor = 0.02f;
    fallJumpAllowance = 10;
    swimSpeedSlow = 2.0f;
    swimSpeedFast = 8.0f;
    swimSpeedAttack = 6.0f;
    swimRollRange = 0.75f;
    swimRollSmooth = 0.2f;
    swimRollFactor = 0.5f;
    swimPitchMax = 0.9f;
    swimPitchMin = -1.3f;
    swimPitchFactor = 0.02f;
    glideRollRange = 0.25f;
    glideRollSmooth = 0.2f;
    glideRollFactor = 0.5f;
    rollEpsilon = 0.00001f; // 1.0E-5
    swimDeceleration = 0.95f;
    swimMinSpeed = 0.7f;
    bunyipLifeSpan = 25;
    bunyipHitRange = 300.0f;
    bunyipStunRange = 500.0f;
    quickSandWiggleFactor = 0.9f;
    slideSpeedMax = 5.0f;
    
    WS.Init(); // initialise waterslide struct to default values
    
    underwaterDustThreshold = 64.0f;
    underwaterDustSizeMax = 10.0f;
    underwaterDustSizeMin = 54.0f;
    
    dataIni.Init("data.val");
    KromeIniLine* pLine = dataIni.GotoLine(NULL, NULL);

    while(pLine != NULL) {
        if (pLine->pFieldName != NULL) {
            bFieldFound = Load_Float(pLine, &nearPlane, "near plane");
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &enemyChaseSpeed, "enemyChaseSpeed");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &enemyPatrolSpeed, "enemyPatrolSpeed");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &enemyHeroRange, "enemyHeroRange");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &enemyRoamRange, "enemyRoamRange");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &resetPosX, "resetPosX");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &resetPosY, "resetPosY");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &resetPosZ, "resetPosZ");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &biteMagnitude, "biteMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &surfaceSwimMagnitude, "surfaceSwimMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &dogPaddleMagnitude, "dogPaddleMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimMagnitude, "swimMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &bounceHeight, "bounceHeight");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &warpSpeed, "warpSpeed");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &iceAcceleration, "iceAcceleration");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &walkMagnitude, "walkMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &runMagnitude, "runMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &jogMagnitude, "jogMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &sneakMagnitude, "sneakMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &jumpMagnitude, "jumpMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &waterJumpMagnitude, "waterJumpMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideMagnitude, "glideMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &jumpYStep, "jumpYStep");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &jumpGravity, "jumpGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &jumpMaxGravity, "jumpMaxGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &jumpHeight, "jumpHeight");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &ledgeGrabTolerance, "ledgeGrabTolerance");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &waterJumpYStep, "waterJumpYStep");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &waterJumpGravity, "waterJumpGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &waterJumpMaxGravity, "waterJumpMaxGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &waterJumpHeight, "waterJumpHeight");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideJumpYStep, "glideJumpYStep");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideJumpGravity, "glideJumpGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideJumpMaxGravity, "glideJumpMaxGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideJumpHeight, "glideJumpHeight");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideYStep, "glideYStep");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideGravity, "glideGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideMaxGravity, "glideMaxGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &fallTurnFactor, "fallTurnFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Int(pLine, &fallJumpAllowance, "fallJumpAllowance");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideTurnFactor, "glideTurnFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimTurnFactor, "swimTurnFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &walkJumpTurnFactor, "walkJumpTurnFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &runJumpTurnFactor, "runJumpTurnFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Int(pLine, &actionIdleCounter, "actionIdleCounter");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Int(pLine, &waitForRndIdleCounter, "waitForRndIdleCounter");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &startGlideReachAnimHeight, "startGlideReachAnimHeight");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &startLongFallDelta, "startLongFallDelta");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &fallMaxGravity, "fallMaxGravity");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &doomActiveSeconds, "doomActiveSeconds");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &doomMoveMagnitude, "doomMoveMagnitude");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimSpeedSlow, "swimSpeedSlow");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimSpeedFast, "swimSpeedFast");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimSpeedAttack, "swimSpeedAttack");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimRollRange, "swimRollRange");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimRollSmooth, "swimRollSmooth");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimRollFactor, "swimRollFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimPitchMax, "swimPitchMax");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimPitchMin, "swimPitchMin");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimPitchFactor, "swimPitchFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideRollRange, "glideRollRange");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideRollSmooth, "glideRollSmooth");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &glideRollFactor, "glideRollFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &rollEpsilon, "rollEpsilon");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimDeceleration, "swimDeceleration");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &swimMinSpeed, "swimMinSpeed");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &slideSpeedMax, "slideSpeedMax");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Int(pLine, &bunyipLifeSpan, "bunyipLifeSpan");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &bunyipHitRange, "bunyipHitRange");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &bunyipStunRange, "bunyipStunRange");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &quickSandWiggleFactor, "quickSandWiggleFactor");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &underwaterDustThreshold, "underwaterDustThreshold");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &underwaterDustSizeMin, "underwaterDustSizeMin");
            }
            if (!bFieldFound) {
                bFieldFound = Load_Float(pLine, &underwaterDustSizeMax, "underwaterDustSizeMax");
            }
            if (!bFieldFound) {
                bFieldFound = WS.Load(pLine);
            }
            gAssertBool = bFieldFound;
        }
        pLine = dataIni.GetNextLine();
    }

    dataIni.Deinit();

    CalcYStep(jumpGravity, jumpHeight, &jumpYStep);
    CalcYStep(waterJumpGravity, waterJumpHeight, &waterJumpYStep);
    CalcYStep(glideJumpGravity, glideJumpHeight, &glideJumpYStep);
}

void DataVal::WS::Init(void) {
    WSIDSlowSlow = 6.0f;
    WSIDSlowNormal = 10.0f;
    WSIDSlowFast = 15.0f;
    WSIDFastSlow = 15.0f;
    WSIDFastNormal = 20.0f;
    WSIDFastFast = 30.0f;
    WSIDNormalSlow = 6.0f;
    WSIDNormalNormal = 15.0f;
    WSIDNormalFast = 20.0f;
    WSSmoothFrames = 5.0f;
    WSDuckHeight = 40.0f;
    WSJumpHeight = 120.0f;
    WSWayPtScale = 0.7f;
    WSSlopeScale = 0.3f;
    WSTurnSmoothing = 0.02f;
}

bool DataVal::WS::Load(KromeIniLine* pLine) {
    bool bFieldFound = Load_Float(pLine, &WSIDFastFast, "WSIDFastFast");
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSIDFastSlow, "WSIDFastSlow");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSIDFastNormal, "WSIDFastNormal");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSIDSlowFast, "WSIDSlowFast");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSIDSlowSlow, "WSIDSlowSlow");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSIDSlowNormal, "WSIDSlowNormal");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSIDNormalFast, "WSIDNormalFast");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSIDNormalSlow, "WSIDNormalSlow");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSIDNormalNormal, "WSIDNormalNormal");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSSmoothFrames, "WSSmoothFrames");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSDuckHeight, "WSDuckHeight");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSJumpHeight, "WSJumpHeight");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSWayPtScale, "WSWayPtScale");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSSlopeScale, "WSSlopeScale");
    }
    if (!bFieldFound) {
        bFieldFound = Load_Float(pLine, &WSTurnSmoothing, "WSTurnSmoothing");
    }
    return bFieldFound;
}