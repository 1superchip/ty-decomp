#include "ty/Ty.h"
#include "ty/global.h"

static GameObjDesc tyDesc;
Ty ty;

static ModuleInfo<Ty> tyModuleInfo;
Hero* pHero;
static bool bInitialised = false;
static bool bResourcesLoaded = false;

bool TyFSM::SolidSurfaceState(int state) {
    if (
        state == 3 || 
        state == 5 ||
        state == 0x23 ||
        state == 0x20 ||
        state == 0x22 ||
        state == 0x1C ||
        state == 0x21 ||
        state == 0x2C ||
        state == 0x24 ||
        state == 0x2E ||
        state == 2 ||
        state == 4 ||
        state == 0x28 ||
        state == 0x29
    ) {
        return true;
    }
    
    return false;
}

bool TyFSM::BiteState(int state) {
    if (state == 1) {
        return true;
    }

    return false;
}

bool TyFSM::SwimmingState(int state) {
    if (
        state == 0xE ||
        state == 0xF ||
        state == 0x26 ||
        state == 0x10 ||
        state == 0x11 ||
        state == 0x27 ||
        state == 0x12 ||
        state == 0x31 ||
        state == 0x13 ||
        state == 0x14 ||
        state == 0x1D ||
        state == 0x16 ||
        state == 0x18
    ) {
        return true;
    }

    return false;
}

bool TyFSM::IdleState(int state) {
    if (
        state == 0x22 ||
        state == 0x23 ||
        state == 0x24 ||
        state == 0x26 ||
        state == 0x27 ||
        state == 0x25 ||
        state == 0x28
    ) {
        return true;
    }

    return false;
}

bool TyFSM::KnockBackState(int state) {
    if (state == 0x20 || state == 0x14 || state == 0x14 || state == 0x15) {
        return true;
    }

    return false;
}

bool TyFSM::MoveState(int state) {
    if (state == 5 || state == 3 || state == 2 || state == 4) {
        return true;
    }

    return false;
}

bool TyFSM::SneakState(int state) {
    return state == 2;
}

bool TyFSM::WaterSurfaceState(int state) {
    return state == 0xE || state == 0x25 || state == 0xF || state == 0x26 || state == 0x15;
}

bool TyFSM::FirstPersonState(int state) {
    return state == 0x2D;
}

bool TyFSM::AirState(int state) {
    if (
        state == 0x1A ||
        state == 0x7 ||
        state == 0x8 ||
        state == 0x9 ||
        state == 0x1B
    ) {
        return true;
    }

    return false;
}

void Ty_LoadResources(void) {
    tyDesc.Init(
        &tyModuleInfo,
        "Ty",
        "Ty",
        0,
        1
    );

    if (!bResourcesLoaded) {
        bResourcesLoaded = true;
    }
}

void Ty_Init(void) {
    if (!bInitialised) {
        pHero = &ty;

        ty.Init();

        bInitialised = true;
    }
}

void Ty_Deinit(void) {
    if (bInitialised) {
        ty.Deinit();

        bInitialised = false;
    }
}

void Ty_Update(void) {

}

void Ty_Draw(void) {

}

char* playerFileNames[] = {
    "act_01_ty",
    "act_01_tyshadow",
    "act_01_ty_01",
    "act_01_ty_02"
};

void Ty::LoadResources(void) {
    rangPropLeftAnimScript.Init("prop_0137_rangpropleft");
    rangPropRightAnimScript.Init("prop_0137_rangpropright");
    unk534.Init(playerFileNames[gb.unk100 * 4]);

    animScript.Init(&unk534);
    unk4EC.Init(&unk534);
    unk50C.Init(&unk534);

    unk52C = false;

    if (gb.level.GetCurrentLevel() == LN_OUTBACK_SAFARI) {
        pModel = Model::Create(unk534.GetMeshName(), NULL);
        // GetNodesAndSubObjects();
    } else {
        pModel = Model::Create(unk534.GetMeshName(), unk534.GetAnimName());

        pReflectionModel = Model::Create(playerFileNames[gb.unk100 * 4 + 1], NULL);
        pReflectionModel->pAnimation = pModel->GetAnimation();

        mShadow.Init(
            playerFileNames[gb.unk100 * 4 + 1],
            pModel,
            250.0f,
            NULL
        );

        pMatEyes = Material::Find(playerFileNames[gb.unk100 * 4 + 3]);
        pMatEyeballs = Material::Find("Act_01_TY_01a");
        // GetNodesAndSubObjects();
        // LoadAnimations();

        unk530 = 0.0;
    }
}

void Ty::Init(void) {
    GameObject::Init(&tyDesc);

    pLastCheckPoint = NULL;

    LoadResources();
}
