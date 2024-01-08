#ifndef DATAVAL_H
#define DATAVAL_H

#include "common/KromeIni.h"

struct DataVal {
    struct WS {
		float WSIDFastFast;
		float WSIDFastSlow;
		float WSIDFastNormal;
		float WSIDSlowFast;
		float WSIDSlowSlow;
		float WSIDSlowNormal;
		float WSIDNormalFast;
		float WSIDNormalSlow;
		float WSIDNormalNormal;
		float WSSmoothFrames;
		float WSDuckHeight;
		float WSJumpHeight;
		float WSWayPtScale;
		float WSSlopeScale;
		float WSTurnSmoothing;
		
		void Init(void);
		bool Load(KromeIniLine*);
	}; // struct WS

    float nearPlane;
	float enemyChaseSpeed;
	float enemyPatrolSpeed;
	float enemyHeroRange;
	float enemyRoamRange;
	float resetPosX;
	float resetPosY;
	float resetPosZ;
	float unk20;
	float bounceHeight;
	float warpSpeed;
	float iceAcceleration;
	float walkMagnitude;
	float runMagnitude;
	float jogMagnitude;
	float sneakMagnitude;
	float jumpMagnitude;
	float waterJumpMagnitude;
	float glideMagnitude;
	float surfaceSwimMagnitude;
	float dogPaddleMagnitude;
	float biteMagnitude;
	float swimMagnitude;
	float jumpYStep;
	float jumpGravity;
	float jumpMaxGravity;
	float jumpHeight;
	float glideJumpYStep;
	float glideJumpGravity;
	float glideJumpMaxGravity;
	float glideJumpHeight;
	float glideYStep;
	float glideGravity;
	float glideMaxGravity;
	float ledgeGrabTolerance;
	float startLongFallDelta;
	float fallMaxGravity;
	float fallTurnFactor;
	int fallJumpAllowance;
	float glideTurnFactor;
	float runJumpTurnFactor;
	float walkJumpTurnFactor;
	float swimTurnFactor;
	int actionIdleCounter;
	int waitForRndIdleCounter;
	float startGlideReachAnimHeight;
	float waterJumpYStep;
	float waterJumpGravity;
    float waterJumpMaxGravity;
    float waterJumpHeight;
    float doomActiveSeconds;
    float doomMoveMagnitude;
    float swimSpeedSlow;
    float swimSpeedFast;
    float swimSpeedAttack;
    float swimRollRange;
    float swimRollSmooth;
    float swimRollFactor;
    float swimPitchMax;
    float swimPitchMin;
    float swimPitchFactor;
    float glideRollRange;
    float glideRollSmooth;
    float glideRollFactor;
    float rollEpsilon;
    float swimDeceleration;
    float swimMinSpeed;
    float slideSpeedMax;
    int bunyipLifeSpan;
    float bunyipHitRange;
    float bunyipStunRange;
    float quickSandWiggleFactor;
    DataVal::WS WS;
    float underwaterDustThreshold;
    float underwaterDustSizeMax;
    float underwaterDustSizeMin;
	
	void Load(void);
};

#endif // DATAVAL_H
