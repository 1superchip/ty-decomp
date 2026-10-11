#include "ty/props/TurningPlatform.h"
#include "ty/GameObjectManager.h"
#include "ty/global.h"
#include "common/StdMath.h"
#include "common/System_GC.h"

extern "C" double atan2(double, double);

static ModuleInfo<TurningPlatform> turnPlatformModuleInfo;
Tools_WayPoints TurningPlatform::loadPoints;

void TurningPlatform_LoadResources(KromeIni* pIni) { // Unofficial: pIni
    PlatformDesc defaultTurningPlatformDesc; // Unofficial
    defaultTurningPlatformDesc.Init(&turnPlatformModuleInfo, "", "", GOID_Platform, 1);
    LoadDescriptors<PlatformDesc>(pIni, "TurningPlatforms", &defaultTurningPlatformDesc);
}

void TurningPlatform::Init(GameObjDesc* pDesc) {
    Platform::Init(pDesc);
    loadPoints.Init();
    mCurrRot.Set(0.0f, 0.0f, 0.0f);
    targetAngle = 0;
    currAngle = 0;
    turnStartFrame = -1;
    turnSpeed = 0.2f;
}

bool TurningPlatform::LoadLine(KromeIniLine* pLine) {
    return loadPoints.LoadLine(pLine, Tools_WayPoints::LOAD_MODE_1) ||
        LoadLevel_LoadFloat(pLine, "turnSpeed", &turnSpeed) ||
        Platform::LoadLine(pLine);
}

void TurningPlatform::LoadDone(void) {
    GetPos()->Copy(&loadPoints.vecs[0]);
    GetPos()->w = 1.0f;

    numAngles = loadPoints.unk104 - 1;

    for (int i = 1; i < loadPoints.unk104; i++) { // Unofficial: i
        angles[i - 1] = PI2 + (float)atan2(loadPoints.vecs[i].z - GetPos()->z, loadPoints.vecs[i].x - GetPos()->x);
    }

    if (numAngles > 0) {
        StaticProp::loadInfo.defaultRot.y = angles[0];
    }

    Platform::LoadDone();
}

void TurningPlatform::Update(void) {
    BeginUpdate();
    UpdateTilt();

    if (currAngle != targetAngle) {
        float delta = GetSmallestAngle(angles[currAngle], angles[targetAngle]); // Unofficial

        float t = (float)(gb.logicGameCount - turnStartFrame) / Abs<float>(gDisplay.fps * (delta / turnSpeed)); // Unofficial: t
        t = Clamp<float>(0.0f, t, 1.0f);

        mCurrRot.y = angles[currAngle] + delta * _table_sinf(PI * (0.5f * t));

        if (t == 1.0f) {
            currAngle = targetAngle;
        }
    }

    EndUpdate();
}

void TurningPlatform::Reset(void) {
    mCurrRot = mDefaultRot;
    pModel->matrices[0].SetRotationPYR(&mCurrRot);
    unk58 = NULL;
}

void TurningPlatform::Message(MKMessage* pMsg) {
    switch (pMsg->unk0) {
        case MSG_BoomerangMsg:
        case MSG_Activate:
            if (currAngle == targetAngle) {
                TurnToNext();
            }
            break;
    }

    Platform::Message(pMsg);
}

void TurningPlatform::TurnToNext(void) {
    targetAngle = (targetAngle + 1) % numAngles;
    turnStartFrame = gb.logicGameCount;
}
