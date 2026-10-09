#include "ty/props/MobilePlatform.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"
#include "common/StdMath.h"
#include "common/System_GC.h"

extern "C" double atan2(double, double);
void GameCamera_AddDynamicCollisionItem(Model* pModel, int subObjectIndex);

static ModuleInfo<MobilePlatform> mobilePlatformModuleInfo;
MobilePlatformLoadInfo MobilePlatform::mpfLoadInfo;

void MobilePlatformDesc::Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags) {
    PlatformDesc::Init(pMod, pMdlName, pDescrName, _searchMask, _flags | 0x80);
    bobSpeed = 0.0f;
    bobHeight = 0.0f;
}

void MobilePlatform_LoadResources(KromeIni* pIni) {
    MobilePlatformDesc defaultMobilePlatformDesc;
    defaultMobilePlatformDesc.Init(&mobilePlatformModuleInfo, "", "", GOID_Platform, 1);
    LoadDescriptors<MobilePlatformDesc>(pIni, "MobilePlatforms", &defaultMobilePlatformDesc);
}

void MobilePlatform::Init(GameObjDesc* pDesc) {
    Platform::Init(pDesc);
    mpfLoadInfo.wayPoints.Init();
    mpfLoadInfo.bNonCircular = false;
    mpfLoadInfo.speed = 7.0f;
    mpfLoadInfo.startTime = 0.0f;
    bReverse = 0;
    bCircular = false;
    bMoveOne = false;
    pathFrames = 0;
    turnSpeed = 0.0f;
    startFrame = 0;
    bobFrameOffset = 0;
    gameObjFlags.Init(GameObjFlags_All);
}

void MobilePlatform::Deinit(void) {
    spline.Deinit();
    Platform::Deinit();
}

void MobilePlatform::Reset(void) {
    Platform::Reset();
    gameObjFlags.Reset();
    startFrame = defaultStartFrame;
    UpdateMove();
}

bool MobilePlatform::LoadLine(KromeIniLine* pLine) {
    return gameObjFlags.LoadLine(pLine) ||
        LoadLevel_LoadBool(pLine, "bCircular", &bCircular) ||
        LoadLevel_LoadFloat(pLine, "turnSpeed", &turnSpeed) ||
        LoadLevel_LoadFloat(pLine, "speed", &mpfLoadInfo.speed) ||
        LoadLevel_LoadFloat(pLine, "startTime", &mpfLoadInfo.startTime) ||
        LoadLevel_LoadBool(pLine, "bMoveOne", &bMoveOne) ||
        LoadLevel_LoadBool(pLine, "bNonCircular", &mpfLoadInfo.bNonCircular) ||
        mpfLoadInfo.wayPoints.LoadLine(pLine, Tools_WayPoints::LOAD_MODE_1) ||
        Platform::LoadLine(pLine);
}

void MobilePlatform::LoadDone(void) {
    gameObjFlags.SetDefaultFlags();

    GetPos()->Copy(&mpfLoadInfo.wayPoints.vecs[0]);

    if (mpfLoadInfo.wayPoints.unk104 > 1) {
        if (mpfLoadInfo.wayPoints.unk104 > 2 && !mpfLoadInfo.bNonCircular) {
            bCircular = true;
        }

        // Face the platform towards the second waypoint
        float dx = mpfLoadInfo.wayPoints.vecs[1].x - mpfLoadInfo.wayPoints.vecs[0].x;
        float dz = mpfLoadInfo.wayPoints.vecs[1].z - mpfLoadInfo.wayPoints.vecs[0].z;
        if (Sqr<float>(dx) + Sqr<float>(dz) > Sqr<float>(0.01f) && turnSpeed > 0.0f) {
            StaticProp::loadInfo.defaultRot.y = PI2 + (float)atan2(dz, dx);
        }

        spline.Init(mpfLoadInfo.wayPoints.unk104 + bCircular, true);

        for (int i = 0; i < mpfLoadInfo.wayPoints.unk104; i++) {
            spline.AddNode(&mpfLoadInfo.wayPoints.vecs[i]);
        }

        if (bCircular) {
            spline.AddNode(&mpfLoadInfo.wayPoints.vecs[0]);
            spline.MergeEnds();
        } else {
            spline.mpPoints[0].unk10.SetZero();
            spline.mpPoints[spline.mNumPoints - 1].unk10.SetZero();
        }

        spline.RegulateSpeed();

        pathFrames = gDisplay.fps * (spline.unk8 / mpfLoadInfo.speed);
    } else {
        spline.Init(mpfLoadInfo.wayPoints.unk104, false);
        spline.AddNode(&mpfLoadInfo.wayPoints.vecs[0]);
    }

    startFrame = pathFrames * Clamp<float>(0.0f, mpfLoadInfo.startTime, 1.0f);
    defaultStartFrame = startFrame;
    frameOffset = startFrame - gb.logicGameCount;

    if (GetDesc()->bobSpeed) {
        bobFrameOffset = (gDisplay.fps * RandomFR(&gb.mRandSeed, 0.0f, 1.0f)) / GetDesc()->bobSpeed;
    }

    GameCamera_AddDynamicCollisionItem(pModel, -1);
    Platform::LoadDone();
}

void MobilePlatform::Update(void) {
    BeginUpdate();
    UpdateTilt();
    UpdateMove();
    UpdateBob();
    EndUpdate();
}

void MobilePlatform::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_Activate:
            if (!gameObjFlags.CheckFlags(GameObjFlags_Active)) {
                frameOffset = startFrame - gb.logicGameCount;
            }
            break;
        case MSG_Deactivate:
            if (gameObjFlags.CheckFlags(GameObjFlags_Active)) {
                startFrame = (frameOffset + gb.logicGameCount) % GetCycleFrames();
            }
            break;
        case MSG_Resolve:
            rider.Resolve();
            rider.Attach(this);
            for (int i = 0; i < spline.mNumPoints; i++) {
                rider.ToLocal(&spline.mpPoints[i].mPos);
                rider.ToLocalDir(&spline.mpPoints[i].unk10);
            }
            break;
        case MKMSG_UNK_3:
            if (turnSpeed > 0.0f && spline.mNumPoints > 1) {
                BeginUpdate();
                SetYaw(GetTime(), 2.0f * PI);
                EndUpdate();
            }
            break;
    }

    gameObjFlags.Message(pMsg);
    Platform::Message(pMsg);
}

void MobilePlatform::UpdateMove(void) {
    if (spline.mNumPoints > 1) {
        float time = GetTime();
        *GetPos() = spline.GetPosition(time);
        GetPos()->w = 1.0f;
        rider.ToWorld(GetPos());

        if (turnSpeed > 0.0f) {
            SetYaw(time, turnSpeed);
        }

        if (bMoveOne && gameObjFlags.CheckFlags(GameObjFlags_Active) && time > 0.99f && bReverse == 0) {
            MKMessage msg = {MSG_Deactivate};
            Message(&msg);
        }
    } else {
        *GetPos() = spline.mpPoints[0].mPos;
        GetPos()->w = 1.0f;
        rider.ToWorld(GetPos());
    }
}

void MobilePlatform::UpdateBob(void) {
    if (GetDesc()->bobHeight > 0.0f && GetDesc()->bobSpeed > 0.0f) {
        int bobFrames = gDisplay.fps / GetDesc()->bobSpeed;
        float t = (float)((gb.logicGameCount + bobFrameOffset) % bobFrames) / (float)bobFrames;
        GetPos()->y += _table_sinf((2.0f * PI) * t) * GetDesc()->bobHeight;
    }
}

float MobilePlatform::GetTime(void) {
    if (!gameObjFlags.CheckFlags(GameObjFlags_Active)) {
        frameOffset = startFrame - gb.logicGameCount;
    }

    float time = (float)((gb.logicGameCount + frameOffset) % GetCycleFrames()) / (float)pathFrames;

    if (time > 1.0f) {
        bReverse = 1;
        time = 2.0f - time;
    } else {
        bReverse = 0;
    }

    return time;
}

void MobilePlatform::SetYaw(float time, float maxTurn) {
    if (spline.mNumPoints > 0) {
        Vector dir = spline.GetVelocity(time);
        rider.ToWorldDir(&dir);
        dir.y = 0.0f;

        if (dir.MagSquared() > Sqr<float>(0.01f)) {
            float yaw = atan2(dir.z, dir.x);
            float direction;
            if (bReverse == 0) {
                direction = 1.0f;
            } else {
                direction = -1.0f;
            }

            float angle = GetSmallestAngle(mCurrRot.y, (2.0f * PI) * direction + (PI2 + yaw));
            mCurrRot.y += Clamp<float>(-Abs<float>(maxTurn), angle, Abs<float>(maxTurn));
        }
    }
}
