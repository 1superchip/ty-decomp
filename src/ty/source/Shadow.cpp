#include "ty/Shadow.h"

StructList<Shadow*> animatingSet;

static bool bInitialised = false;

void Shadow_Init(void) {
    if (!bInitialised) {
        animatingSet.Init(20);
        bInitialised = true;
    }
}

void Shadow_Deinit(void) {
    if (bInitialised) {
        
        // Deinit all shadows
        Shadow** pShadow = animatingSet.GetCurrEntry();
        while (pShadow) {
            (*pShadow)->Deinit();
            pShadow = animatingSet.GetNextEntryWithEntry(pShadow);
        }

        animatingSet.Deinit();

        bInitialised = false;
    }
}

void Shadow_DrawAnimatingShadows(void) {
    Shadow** pShadow = animatingSet.GetCurrEntry();
    while (pShadow) {
        if ((*pShadow)->bDraw) {
            (*pShadow)->pModel->pAnimation->CalculateMatrices();
            (*pShadow)->Draw();
        }
        pShadow = animatingSet.GetNextEntryWithEntry(pShadow);
    }
}

void Shadow::AddAnimatingShadow(char* arg1, Model* pShadowModel, float f1, Vector* pVec) {
    Init(arg1, pShadowModel, f1, pVec);

    // Add this to the animating shadow list
    *animatingSet.GetNextEntry() = this;
}

void Shadow::Init(char* arg1, Model* pShadowModel, float f1, Vector* pVec) {
    pModel = pShadowModel;

    pMKShadow = MKShadow_CreateAnimatedFromModel(arg1);

    unk4 = f1;
    unk10.SetZero();
    if (pVec) {
        unk10 = *pVec;
    }

    unkC = 200.0f;
    unk8 = 500.0f;

    unk20.SetZero();

    bDraw = true;
    unk35 = false;

    unk38 = 1.0f;

    if (pModel) {
        unk3C = pModel->GetModelVolume()->v2.Magnitude() / 2.0f;
    }

    unk40 = 1.0f;
    mLightPos.SetZero();
}

void Shadow::Deinit(void) {
    if (pMKShadow) {
        MKShadow_Destroy(pMKShadow);
        pMKShadow = NULL;
    }
}

void Shadow::GetLightPos(Vector* pLightPos) {
    if (pModel) {
        Vector centre;
        pModel->GetCentre(&centre);

        pLightPos->Scale(&unk10, unk8);
        pLightPos->Add(&centre);
    }
}

void Shadow::Draw(void) {
    GetLightPos(&mLightPos);
    Draw(&mLightPos, true);
}

void Shadow::Draw(Vector* pLightPos, bool arg2) {
    unk35 = false;
    
    if (pModel) {
        mLightPos = *pLightPos;

        Vector centre;

        pModel->GetCentre(&centre);

        if (arg2) {
            Vector dir;

            dir.Sub(&mLightPos, &centre);
            float len = dir.Normalise();

            if (len < unkC) {
                float absLen = Abs<float>(len);

                if (absLen < 0.001f) {
                    return;
                }

                mLightPos.Subtract(&centre);
                mLightPos.Scale(unkC / len);
                mLightPos.Add(&centre);
            }

            CheckForShadowShift(unk3C, &dir, len, &mLightPos);
        }

        if (!unk35) {
            MKShadow_DetectEdges(pMKShadow, &centre, pModel, &mLightPos);
            MKShadow_Render(pMKShadow, &centre, &mLightPos, unk4 * unk40, 0, NULL, &unk20);
        }
    }
}

bool Shadow::CheckForShadowShift(float f1, Vector* pVec, float f2, Vector* pVec2) {
    Vector centre;
    pModel->GetCentre(&centre);

    float f25;

    float f24 = unk38 * f1;
    float f31 = unk4;
    float f30;
    
    f31 += (((f31 + f2) * f24) / f2);

    float f20 = View::GetCurrent()->unk2C0 * 1.5f;
    float f23 = ((f24 * (f31 + f2)) / f2);

    f30 = (f31 + 100.0f) + f20;
    f23 += f20;
    f24 += f20;

    Vector camPos = *View::GetCurrent()->unk48.Row3();

    if (camPos.IsInsideSphere(&centre, f24)) {
        unk35 = true;
        return false;
    }

    Vector toEndPoint;
    Vector toCam;
    Vector endPos;

    toEndPoint.Scale(pVec, -f31);
    endPos.Add(&toEndPoint, &centre);
    toCam.Sub(&camPos, &centre);

    float dot = toCam.Dot(&toEndPoint);
    float mag = toEndPoint.MagSquared();

    if (!mag) {
        return false;
    }

    float f29 = dot / mag;

    if (f29 < 0.0f) {
        return false;
    }

    if (f29 < 1.0f) {
        Vector temp;

        temp.Scale(&toEndPoint, f29);
        temp.Add(&centre);
        temp.Subtract(&camPos);

        float val = ((f23 - f24) * f29) + f24;
        float mag1 = temp.MagSquared();

        if (mag1 > Sqr<float>(val)) {
            return false;
        }

        float f3 = val - temp.Normalise();

        temp.Scale((f3 * f2) / (f29 * f31));

        pVec2->Subtract(&temp);

        return true;
    }

    Vector toTaperPoint;
    toTaperPoint.Scale(pVec, -f30);
    toTaperPoint.Subtract(&toEndPoint);
    toCam.Sub(&camPos, &endPos);

    float endDot = toCam.Dot(&toTaperPoint);
    float mag2 = toTaperPoint.MagSquared();

    if (!mag2) {
        mag2 = 1.0f;
    }

    f25 = endDot / mag2;

    if (f25 < 0.0f) {
        return false;
    }

    if (f25 < 1.0f) {
        Vector temp;
        temp.Scale(&toTaperPoint, f25);
        temp.Add(&endPos);
        temp.Subtract(&camPos);

        float f20 = f23 * (1.0f - f25);

        if (temp.MagSquared() > Sqr<float>(f20)) {
            return false;
        }

        float f3 = f20 - temp.Normalise();

        temp.Scale((f3 * f2) / (f31 + (f25 * (f30 - f31))));

        pVec2->Subtract(&temp);
        return true;
    }

    return false;
}

void Shadow_BeginDraw(void) {
    MKShadow_BeginScene(0);
    MKShadow_CaptureZBuffer();
    MKShadow_ClearBuffer();
}

void Shadow_EndDraw(void) {
    MKShadow_BlitBuffer(0x40808080, 1);
    MKShadow_EndScene();
}

void DropShadow::Init(Vector* pMatrixPos, Vector* pVec1, Vector* pVec2, Vector* pVec3, float f1, bool b) {
    mMatrix.SetIdentity();

    mMatrix.Row2()->Set(pVec2->x, pVec2->y, pVec2->z);
    mMatrix.Row1()->Set(pVec1->x, pVec1->y, pVec1->z);

    mMatrix.Row0()->Cross(mMatrix.Row2(), mMatrix.Row1());
    mMatrix.Row0()->Normalise();

    mMatrix.Row2()->Cross(mMatrix.Row1(), mMatrix.Row0());

    mMatrix.Row3()->Set(pMatrixPos->x, pMatrixPos->y, pMatrixPos->z);

    mStrips[0].pos.Set(-f1 / 2.0f, 0.0f, f1 / 2.0f);
    mStrips[1].pos.Set(f1 / 2.0f, 0.0f, f1 / 2.0f);
    mStrips[2].pos.Set(-f1 / 2.0f, 0.0f, -f1 / 2.0f);
    mStrips[3].pos.Set(f1 / 2.0f, 0.0f, -f1 / 2.0f);

    if (pVec3) {
        mStrips[3].color = mStrips[2].color = mStrips[1].color = mStrips[0].color = *pVec3;
    } else {
        mStrips[0].color.Set(1.0f, 1.0f, 1.0f, 1.0f);
        mStrips[1].color = mStrips[0].color;
        mStrips[2].color = mStrips[1].color;
        mStrips[3].color = mStrips[2].color;
    }

    if (b) {
        mStrips[0].uv.x = 0.0f;
        mStrips[0].uv.y = 0.0f;
        mStrips[1].uv.x = 1.0f;
        mStrips[1].uv.y = 0.0f;
        mStrips[2].uv.x = 0.0f;
        mStrips[2].uv.y = 1.0f;
        mStrips[3].uv.x = 1.0f;
        mStrips[3].uv.y = 1.0f;
    } else {
        mStrips[0].uv.x = 0.0f;
        mStrips[0].uv.y = 1.0f;
        mStrips[1].uv.x = 1.0f;
        mStrips[1].uv.y = 1.0f;
        mStrips[2].uv.x = 0.0f;
        mStrips[2].uv.y = 0.0f;
        mStrips[3].uv.x = 1.0f;
        mStrips[3].uv.y = 0.0f;
    }
}

void DropShadow::SetRadius(float f) {
    for (int i = 0; i < 4; i++) {
        mStrips[i].pos.Normalise();
        mStrips[i].pos.Scale(f / 2.0f);
    }
}

void DropShadow::SetColor(Vector* pColor) {
    if (pColor) {
        mStrips[0].color = *pColor;
        mStrips[1].color = mStrips[0].color;
        mStrips[2].color = mStrips[1].color;
        mStrips[3].color = mStrips[2].color;
    } else {
        mStrips[0].color.Set(1.0f, 1.0f, 1.0f, 1.0f);
        mStrips[1].color = mStrips[0].color;
        mStrips[2].color = mStrips[1].color;
        mStrips[3].color = mStrips[2].color;
    }
}

void DropShadow::Draw(void) {
    View::GetCurrent()->SetLocalToWorldMatrix(&mMatrix);
    mStrips[0].Draw(ARRAY_SIZE(mStrips), 1.0f);
}
