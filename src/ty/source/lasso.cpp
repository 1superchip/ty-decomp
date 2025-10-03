#include "ty/Lasso.h"
#include "ty/global.h"
#include "common/View.h"

static StateMachine<Lasso>::State lassoStates[7] = {
    {
        // LS_Idle
        &Lasso::InitIdle,
        NULL,
        NULL
    },
    {
        // LS_Throw
        &Lasso::InitLassoThrow, 
        &Lasso::DeinitLassoThrow, 
        &Lasso::LassoThrow, 
        &Lasso::LassoDraw
    },
    {   // LS_PullInRope
        &Lasso::InitLassoPullIn, 
        &Lasso::DeinitLassoPullIn, 
        &Lasso::LassoPullIn, 
        &Lasso::LassoDraw
    },
    {   // LS_CaughtEmu
        &Lasso::InitLassoCaughtEmu, 
        &Lasso::DeinitLassoCaughtEmu, 
        &Lasso::LassoCaughtEmu, 
        &Lasso::LassoDrawSimple
    },
    {
        // LS_CaughtWaterTank
        &Lasso::InitLassoCaughtWaterTank, 
        &Lasso::DeinitLassoCaughtWaterTank, 
        &Lasso::LassoCaughtWaterTank, 
        &Lasso::LassoDrawSimple
    },
    {   // LS_CaughtFrillBike
        &Lasso::InitLassoCaughtFrillBike, 
        &Lasso::DeinitLassoCaughtFrillBike, 
        &Lasso::LassoCaughtFrillBike, 
        &Lasso::LassoDrawSimple
    },
    {   // LS_Snapped
        &Lasso::InitLassoSnapped, 
        &Lasso::DeinitLassoSnapped, 
        &Lasso::LassoSnapped, 
        &Lasso::LassoDraw
    }
};

static MKAnimScript lassoBad;

void Lasso_LoadResources(void) {
    lassoBad.Init("boomerang_01");
}

void Lasso::Init(void) {
    Reset();
    
    unk34 = Model::Create(lassoBad.GetMeshName(), lassoBad.GetAnimName());

    fsm.Init(lassoStates, LS_Idle);

    pRopeMat = Material::Create("rope_01");
    pTensionMat = Material::Create("tension");

    ropeQueue.Init(100);
}

void Lasso::Deinit(void) {
    ropeQueue.Deinit();

    unk34->Destroy();
    unk34 = NULL;

    pRopeMat->Destroy();
    pTensionMat->Destroy();
}

void Lasso::Reset(void) {
    pEmu = NULL;
    unk54 = NULL;
    pFrillBike = NULL;

    unk100 = 100.0f;
    unkFC = 0.0f;

    unk74 = false;
    unk75 = false;

    unk70 = 0;

    unk78 = 0.0f;

    fsm.SetState(LS_Idle, false);
}

void Lasso::Update(Vector* p, Vector* p1, float f1) {
    unk0 = *p;
    unk10 = *p1;

    unk20 = f1;

    if ((gb.logicGameCount % 2) != 0) {
        unk10C += 1.0f;

        if (unk10C > 8.0f) {
            unk10C = 1.0f;
        }
    }

    fsm.Update(this, false);
}

void Lasso::Draw(void) {
    fsm.DrawState(this);

    if (unk70 == 1) {
        float f31 = unkFC / unk100; // 0xC
        float x; // 0x10
        float y; // 0x14
        float sx; // 0x18
        float sy; // 0x1C
        float f1; // 0x20
        float f2; // 0x24
        float f5; // 0x28
        float f6; // 0x2C
        float f9; // 0x30
        float f10; // 0x34
        float f13; // 0x38
        float f14;
        float f3; // 0x40
        float f4;
        float f7;
        float f8;
        float f11; // 0x50
        float f12; // 0x54
        float f15; // 0x58
        float f16; // 0x5C

        x = 120.0f;
        y = 462.0f;
        sx = 400.0f;
        sy = 20.0f;

        f1 = x;
        f5 = (float)f1 + sx;
        f9 = f5;
        f13 = f1;
        
        f3 = 0.0f;
        f7 = 1.0f;
        f11 = f7;
        f15 = f3;
        
        f2 = y;
        f6 = f2;
        f10 = (float)f2 + sy;
        f14 = f10;

        f4 = 0.0f;
        f8 = f4;

        f12 = 0.5f;
        f16 = f12;
        
        Vector color = {1.0f, 1.0f, 1.0f, 1.0f};

        Tools_DrawOverlay(
            gb.pTensionMat, 
            &color, 
            f1, f2, f3, f4,
            f5, f6, f7, f8,
            f9, f10,
            f11, f12,
            f13, f14,
            f15, f16
        );

        f1 = x;
        f5 = f1 + f31 * sx;
        f9 = f5;
        f13 = f1;
        
        f3 = 0.0f;
        f7 = f31;
        f11 = f7;
        f15 = f3;
        
        f2 = y;
        f6 = f2;
        f10 = f2 + (float)sy;
        f14 = f10;

        f4 = 0.5f;
        f8 = f4;

        f12 = 1.0f;
        f16 = f12;

        Tools_DrawOverlay(
            gb.pTensionMat, 
            &color, 
            f1, f2, f3, f4,
            f5, f6, f7, f8,
            f9, f10,
            f11, f12,
            f13, f14,
            f15, f16
        );
    }
}

void DrawRope(CircularQueue<Vector>* pQueue, Material* pMaterial, float f1) {
    static Blitter_TriStrip rp[10];

    View::GetCurrent()->SetLocalToWorldMatrix(NULL);

    if (pMaterial) {
        pMaterial->Use();
    }

    for (int i = 0; i < 3; i++) {
        float dVar23;

        if (i == 0) {
            dVar23 = 0.0f;
        } else if (i == 1) {
            dVar23 = 0.5f;
        } else {
            dVar23 = 1.0f;
        }

        for (int j = 0; j < 4; j++) {
            
        }
    }
}

static float targetRadius = 600.0f;

void Lasso::GetControlPoints(Vector* p, Vector* p1, Vector* p2) {

}

void Lasso::InitLassoThrow(void) {
    unk104 = 2000.0f;
    unk78 = 0.0f;

    unk5C = RandomFR(&gb.mRandSeed, 0.5235988f, 0.3490659f); // (PI/6, PI/9)

    unk60 = RandomFR(&gb.mRandSeed, 400.0f, 600.0f);
    unk64 = 0.0f;

    unk68 = RandomFR(&gb.mRandSeed, 15.0f, 25.0f);

    unk6C = RandomFR(&gb.mRandSeed, 0.2f, 0.8f);

    unk74 = RandomIR(&gb.mRandSeed, 0, 2) == 1;

    Vector sp28;
    Vector sp38;
    Vector sp48;

    GetControlPoints(&sp48, &sp28, &sp38);

    Vector sp18;
    sp18.InterpolateLinear(&sp38, &sp48, targetRadius / unk104);
}

void Lasso::LassoThrow(void) {
    
}

void Lasso::DeinitLassoThrow(void) {
    unk74 = false;
}

void Lasso::InitLassoPullIn(void) {

}

void Lasso::LassoPullIn(void) {
    
}

void Lasso::DeinitLassoPullIn(void) {
    
}

void Lasso::InitLassoCaughtEmu(void) {
    unk9C.Set(0.0f, -50.0f, 0.0f);
    unkAC.SetZero();
}

void Lasso::LassoCaughtEmu(void) {
    
}

void Lasso::DeinitLassoCaughtEmu(void) {
    unk70 = 0;
    unk75 = false;
    pEmu = NULL;
}

void Lasso::PullRope(float f1) {

}

void Lasso::InitLassoCaughtWaterTank(void) {
    unk9C.Set(0.0f, -50.0f, 0.0f);
    unkAC.SetZero();
}

void Lasso::LassoCaughtWaterTank(void) {

}

void Lasso::DeinitLassoCaughtWaterTank(void) {
    unk54 = NULL;
}

void Lasso::InitLassoCaughtFrillBike(void) {
    unk9C.Set(0.0f, -50.0f, 0.0f);
    unkAC.SetZero();
}

void Lasso::LassoCaughtFrillBike(void) {

}

void Lasso::CheckPullBike(void) {

}

void Lasso::Snap(void) {
    if (unk70 != 0) {
        unk70 = 0;

        fsm.SetState(LS_Snapped, false);
    }
}

void Lasso::DeinitLassoCaughtFrillBike(void) {
    pFrillBike = NULL;
}

void Lasso::InitLassoSnapped(void) {

}

void Lasso::LassoSnapped(void) {

}

void Lasso::DeinitLassoSnapped(void) {
    
}

void Lasso::LassoDrawSimple(void) {

}

void Lasso::LassoDraw(void) {

}

void Lasso::ThrowLasso(void) {
    unk76 = true;

    fsm.SetState(LS_Throw, false);
}

void Lasso::InitIdle(void) {
    unk76 = false;
}

bool Lasso::NearRope(Vector* p, float f1, Vector* p1, Vector* p2) {

}
