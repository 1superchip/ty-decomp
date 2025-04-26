
#include "ty/WobbleTexture.h"
#include "ty/global.h"
#include "ty/tools.h"
#include "common/Heap.h"
#include "common/StdMath.h"
#include "common/View.h"

// this is needed to add 0x50 bytes to the beginning of rodata
// the map shows there are 5 different objects each 0x10 bytes
// most likely Vector literals from a stripped function?
static const Vector WobbleTexture_Cpp_RodataPadding0 = {};
static const Vector WobbleTexture_Cpp_RodataPadding1 = {};
static const Vector WobbleTexture_Cpp_RodataPadding2 = {};
static const Vector WobbleTexture_Cpp_RodataPadding3 = {};
static const Vector WobbleTexture_Cpp_RodataPadding4 = {};

void WobbleTexture::Init(int _width, int _height) {
    mWidth = _width;
    mHeight = _height;
    mpEntries = (WobbleEntry*)Heap_MemAlloc((mWidth * mHeight) * sizeof(WobbleEntry));

    for (int i = 0; i < _width; i++) {
        for (int j = 0; j < _height; j++) {
            GetWobbleEntry(i, j)->unk48 = RandomFR(&gb.mRandSeed, -0.5f, 0.5f);
            GetWobbleEntry(i, j)->unk4C = RandomFR(&gb.mRandSeed, -0.5f, 0.5f);
        }
    }

    mpTriStrips = (Blitter_TriStrip*)Heap_MemAlloc((mHeight * 2) * sizeof(Blitter_TriStrip));
    unk4 = 0.0f;
}

void WobbleTexture::Deinit(void) {
    Heap_MemFree((void*)mpEntries);
    mpEntries = NULL;

    Heap_MemFree((void*)mpTriStrips);
    mpTriStrips = NULL;
}

float WobbleTexture::SetUpGrid(Vector* pVec, float f1, float f2, float f3) {
    View* pCurrView = View::GetCurrent();
    Matrix* pUnk48 = &pCurrView->unk48;

    Vector pos = *pVec;
    pos.w = 0.0f;
    pos.y = (f3 * 2.0f) - pos.y;

    pos.ApplyMatrix(&pCurrView->unkC8);

    Vector top = {0.0f, pos.y + (f2 / 2.0f), pos.z};
    top.ApplyMatrix(pUnk48);

    Vector bottom = {0.0f, pos.y - (f2 / 2.0f), pos.z};
    bottom.ApplyMatrix(pUnk48);

    unk10 = (f3 - top.y) / (bottom.y - top.y);

    float _80 = 0.0f;
    float _84 = 1.0f;
    float _88 = f2 / 2.0f;
    
    Vector maxPos = pos;
    maxPos.Scale(10000.0f / pos.z);
    maxPos.ApplyMatrix(pUnk48);

    float _A4 = (f3 - maxPos.y) / pUnk48->Row1()->y;
    float _A8 = (pos.z * _A4) / 10000.0f;

    if (_A8 < f2 / 2.0f) {
        _80 = ((f2 / 2.0f) - _A8) / f2;
        _84 = (_A8 + (f2 / 2.0f)) / f2;
        _88 = _A8;
        f2 *= _84;
    }

    Vector corner = {pos.x - (f1 / 2.0f), pos.y + _88, pos.z};

    for (int j = 0; j < mHeight; j++) {
        Vector temp;
        temp.Set(
            corner.x, 
            corner.y - ((f2 * j) / (mHeight - 1.0f)), 
            corner.z, 
            0.0f
        );
        temp.ApplyMatrix(pUnk48);

        Vector* row3 = pCurrView->unk48.Row3();
        float _DC = (f3 - row3->y) / (temp.y - row3->y);

        for (int i = 0; i < mWidth; i++) {

            GetWobbleEntry(i, j)->mPos.Set(
                ((f1 * i) / (mWidth - 1.0f)) + corner.x,
                corner.y - ((f2 * j) / (mHeight - 1.0f)),
                corner.z
            );

            GetWobbleEntry(i, j)->mPos.Scale(_DC);
            GetWobbleEntry(i, j)->unk30 = i / (mWidth - 1.0f);
            GetWobbleEntry(i, j)->unk34 = ((_84 * j) / (mHeight - 1.0f)) + _80;
            GetWobbleEntry(i, j)->mColor.Set(1.0f, 1.0f, 1.0f, 1.0f);

            if (j == 0) {
                GetWobbleEntry(i, j)->mColor.w = 0.0f;
            }

            GetWobbleEntry(i, j)->mColor.w *= Clamp<float>(0.0f, 1.0f - GetWobbleEntry(i, j)->unk34, 1.0f);
        }
    }

    return f2;
}

void WobbleTexture::WobbleUVs(float f1) {
    for (int j = 1; j < mHeight - 1; j++) {
        for (int i = 1; i < mWidth - 1; i++) {
            float sin = _table_sinf((0.08f * unk4) + GetWobbleEntry(i, j)->unk48) * 0.15f;
            float cos = _table_cosf((0.08f * unk4) + GetWobbleEntry(i, j)->unk4C);

            if (cos < 0.0f) {
                GetWobbleEntry(i, j)->mColor.w *= 1.0f + (cos * 0.3f);
            }

            cos *= 0.2f;
            float f21 = (Tools_Wobble(unk4 * 0.01f, (j * 6) + i) + 2.0f) * f1;
            f21 *= (GetWobbleEntry(i, j)->unk34 - unk10) * sin;
            float f2 = (Tools_Wobble(unk4 * 0.015f, (j * 6) + i) + 2.0f) * f1;
            f2 *= (GetWobbleEntry(i, j)->unk34 - unk10) * cos;

            if (i % 2) {
                f21 = -f21;
            }

            if (j % 2) {
                f2 = -f2;
            }

            GetWobbleEntry(i, j)->unk30 += f21;
            GetWobbleEntry(i, j)->unk34 += f2;

            if (GetWobbleEntry(i, j)->unk30 > 0.99f) {
                GetWobbleEntry(i, j)->unk30 = 0.99f;
            } else if (GetWobbleEntry(i, j)->unk30 < 0.0f) {
                GetWobbleEntry(i, j)->unk30 = 0.0f;
            }

            if (GetWobbleEntry(i, j)->unk34 > 0.99f) {
                GetWobbleEntry(i, j)->unk34 = 0.99f;
            } else if (GetWobbleEntry(i, j)->unk34 < 0.0f) {
                GetWobbleEntry(i, j)->unk34 = 0.0f;
            }
        }
    }
}

void WobbleTexture::Draw(Material* pMat, bool r5) {
    View::GetCurrent()->SetLocalToWorldMatrix(&View::GetCurrent()->unk48);
    
    pMat->Use();

    for (int i = 0; i < mWidth - 1; i++) {
        for (int j = 0; j < mHeight; j++) {
            mpTriStrips[j * 2].pos = GetWobbleEntry(i, j)->mPos;
            mpTriStrips[j * 2].color = GetWobbleEntry(i, j)->mColor;
            mpTriStrips[j * 2].uv.x = GetWobbleEntry(i, j)->unk30;
            
            mpTriStrips[(j * 2) + 1].pos = GetWobbleEntry(i + 1, j)->mPos;
            mpTriStrips[(j * 2) + 1].color = GetWobbleEntry(i + 1, j)->mColor;
            mpTriStrips[(j * 2) + 1].uv.x = GetWobbleEntry(i + 1, j)->unk30;

            if (r5) {
                mpTriStrips[j * 2].uv.y = 1.0f - GetWobbleEntry(i, j)->unk34;
                mpTriStrips[(j * 2) + 1].uv.y = 1.0f - GetWobbleEntry(i + 1, j)->unk34;
            } else {
                mpTriStrips[j * 2].uv.y = GetWobbleEntry(i, j)->unk34;
                mpTriStrips[(j * 2) + 1].uv.y = GetWobbleEntry(i + 1, j)->unk34;
            }
        }

        mpTriStrips->DrawNoPerspective(mHeight * 2, 1.0f);
    }
}
