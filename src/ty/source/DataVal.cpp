#include "types.h"
#include "ty/DataVal.h"
#include "common/Str.h"
#include "common/StdMath.h"

extern "C" int stricmp(char*, char*);

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
        *pYStep = sqrtf((2.0f * gravity) * height);
    }
}

void DataVal::Load(void) {
    KromeIni dataIni;
	
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
    rollEpsilon = 0.00001f;
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
            gAssertBool = Load_Float(pLine, &nearPlane, "near plane") || 
                Load_Float(pLine, &enemyChaseSpeed, "enemyChaseSpeed") ||
                Load_Float(pLine, &enemyPatrolSpeed, "enemyPatrolSpeed") ||
                Load_Float(pLine, &enemyHeroRange, "enemyHeroRange") ||
                Load_Float(pLine, &enemyRoamRange, "enemyRoamRange") ||
                Load_Float(pLine, &resetPosX, "resetPosX") ||
                Load_Float(pLine, &resetPosY, "resetPosY") ||
                Load_Float(pLine, &resetPosZ, "resetPosZ") ||
                Load_Float(pLine, &biteMagnitude, "biteMagnitude") ||
                Load_Float(pLine, &surfaceSwimMagnitude, "surfaceSwimMagnitude") ||
                Load_Float(pLine, &dogPaddleMagnitude, "dogPaddleMagnitude") ||
                Load_Float(pLine, &swimMagnitude, "swimMagnitude") ||
                Load_Float(pLine, &bounceHeight, "bounceHeight") ||
                Load_Float(pLine, &warpSpeed, "warpSpeed") ||
                Load_Float(pLine, &iceAcceleration, "iceAcceleration") ||
                Load_Float(pLine, &walkMagnitude, "walkMagnitude") ||
                Load_Float(pLine, &runMagnitude, "runMagnitude") ||
                Load_Float(pLine, &jogMagnitude, "jogMagnitude") ||
                Load_Float(pLine, &sneakMagnitude, "sneakMagnitude") ||
                Load_Float(pLine, &jumpMagnitude, "jumpMagnitude") ||
                Load_Float(pLine, &waterJumpMagnitude, "waterJumpMagnitude") ||
                Load_Float(pLine, &glideMagnitude, "glideMagnitude") ||
                Load_Float(pLine, &jumpYStep, "jumpYStep") ||
                Load_Float(pLine, &jumpGravity, "jumpGravity") ||
                Load_Float(pLine, &jumpMaxGravity, "jumpMaxGravity") ||
                Load_Float(pLine, &jumpHeight, "jumpHeight") ||
                Load_Float(pLine, &ledgeGrabTolerance, "ledgeGrabTolerance") ||
                Load_Float(pLine, &waterJumpYStep, "waterJumpYStep") ||
                Load_Float(pLine, &waterJumpGravity, "waterJumpGravity") ||
                Load_Float(pLine, &waterJumpMaxGravity, "waterJumpMaxGravity") ||
                Load_Float(pLine, &waterJumpHeight, "waterJumpHeight") ||
                Load_Float(pLine, &glideJumpYStep, "glideJumpYStep") ||
                Load_Float(pLine, &glideJumpGravity, "glideJumpGravity") ||
                Load_Float(pLine, &glideJumpMaxGravity, "glideJumpMaxGravity") ||
                Load_Float(pLine, &glideJumpHeight, "glideJumpHeight") ||
                Load_Float(pLine, &glideYStep, "glideYStep") ||
                Load_Float(pLine, &glideGravity, "glideGravity") ||
                Load_Float(pLine, &glideMaxGravity, "glideMaxGravity") ||
                Load_Float(pLine, &fallTurnFactor, "fallTurnFactor") ||
                Load_Int(pLine, &fallJumpAllowance, "fallJumpAllowance") ||
                Load_Float(pLine, &glideTurnFactor, "glideTurnFactor") ||
                Load_Float(pLine, &swimTurnFactor, "swimTurnFactor") ||
                Load_Float(pLine, &walkJumpTurnFactor, "walkJumpTurnFactor") ||
                Load_Float(pLine, &runJumpTurnFactor, "runJumpTurnFactor") ||
                Load_Int(pLine, &actionIdleCounter, "actionIdleCounter") ||
                Load_Int(pLine, &waitForRndIdleCounter, "waitForRndIdleCounter") ||
                Load_Float(pLine, &startGlideReachAnimHeight, "startGlideReachAnimHeight") ||
                Load_Float(pLine, &startLongFallDelta, "startLongFallDelta") ||
                Load_Float(pLine, &fallMaxGravity, "fallMaxGravity") ||
                Load_Float(pLine, &doomActiveSeconds, "doomActiveSeconds") ||
                Load_Float(pLine, &doomMoveMagnitude, "doomMoveMagnitude") ||
                Load_Float(pLine, &swimSpeedSlow, "swimSpeedSlow") ||
                Load_Float(pLine, &swimSpeedFast, "swimSpeedFast") ||
                Load_Float(pLine, &swimSpeedAttack, "swimSpeedAttack") ||
                Load_Float(pLine, &swimRollRange, "swimRollRange") ||
                Load_Float(pLine, &swimRollSmooth, "swimRollSmooth") ||
                Load_Float(pLine, &swimRollFactor, "swimRollFactor") ||
                Load_Float(pLine, &swimPitchMax, "swimPitchMax") ||
                Load_Float(pLine, &swimPitchMin, "swimPitchMin") ||
                Load_Float(pLine, &swimPitchFactor, "swimPitchFactor") ||
                Load_Float(pLine, &glideRollRange, "glideRollRange") ||
                Load_Float(pLine, &glideRollSmooth, "glideRollSmooth") ||
                Load_Float(pLine, &glideRollFactor, "glideRollFactor") ||
                Load_Float(pLine, &rollEpsilon, "rollEpsilon") ||
                Load_Float(pLine, &swimDeceleration, "swimDeceleration") ||
                Load_Float(pLine, &swimMinSpeed, "swimMinSpeed") ||
                Load_Float(pLine, &slideSpeedMax, "slideSpeedMax") ||
                Load_Int(pLine, &bunyipLifeSpan, "bunyipLifeSpan") ||
                Load_Float(pLine, &bunyipHitRange, "bunyipHitRange") ||
                Load_Float(pLine, &bunyipStunRange, "bunyipStunRange") ||
                Load_Float(pLine, &quickSandWiggleFactor, "quickSandWiggleFactor") ||
                Load_Float(pLine, &underwaterDustThreshold, "underwaterDustThreshold") ||
                Load_Float(pLine, &underwaterDustSizeMin, "underwaterDustSizeMin") ||
                Load_Float(pLine, &underwaterDustSizeMax, "underwaterDustSizeMax") || WS.Load(pLine);
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
    return Load_Float(pLine, &WSIDFastFast, "WSIDFastFast") ||
        Load_Float(pLine, &WSIDFastSlow, "WSIDFastSlow") ||
        Load_Float(pLine, &WSIDFastNormal, "WSIDFastNormal") ||
        Load_Float(pLine, &WSIDSlowFast, "WSIDSlowFast") ||
        Load_Float(pLine, &WSIDSlowSlow, "WSIDSlowSlow") ||
        Load_Float(pLine, &WSIDSlowNormal, "WSIDSlowNormal") ||
        Load_Float(pLine, &WSIDNormalFast, "WSIDNormalFast") ||
        Load_Float(pLine, &WSIDNormalSlow, "WSIDNormalSlow") ||
        Load_Float(pLine, &WSIDNormalNormal, "WSIDNormalNormal") ||
        Load_Float(pLine, &WSSmoothFrames, "WSSmoothFrames") ||
        Load_Float(pLine, &WSDuckHeight, "WSDuckHeight") ||
        Load_Float(pLine, &WSJumpHeight, "WSJumpHeight") ||
        Load_Float(pLine, &WSWayPtScale, "WSWayPtScale") ||
        Load_Float(pLine, &WSSlopeScale, "WSSlopeScale") ||
        Load_Float(pLine, &WSTurnSmoothing, "WSTurnSmoothing");
}
