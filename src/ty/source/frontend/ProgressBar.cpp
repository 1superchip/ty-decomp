#include "ty/frontend/ProgressBar.h"

void ProgressBar::Init(char* pBGMatName, char* pFGMatName, int xPos,
        int yPos, float scale, int tMaxProgress, float updateVal, float f3) {
    floatProgress = 0.0f;
    mProgress = 0;
    progressRatio = 0.0f;
    maxProgress = tMaxProgress;
    mUpdateVal = updateVal;

    mPos.Set(xPos, yPos, 0.0f);

    if (pBGMatName) {
        pMat = Material::Create(pBGMatName);
    } else {
        pMat = NULL;
    }

    pFGMat = Material::Create(pFGMatName);
    if (pBGMatName) {
        int w = pMat->GetTexture()->width;
        int h = pMat->GetTexture()->height;
        backgroundImage.startX = xPos - w / 2;
        backgroundImage.startY = yPos - h / 2;
        backgroundImage.endX = backgroundImage.startX + (float)w;
        backgroundImage.endY = backgroundImage.startY + (float)h;
    }

    unk268 = (float)pFGMat->GetTexture()->width * scale;
    float temp = (float)pFGMat->GetTexture()->height * scale;
    backgroundImage.z = 0.0f;
    backgroundImage.uv0 = 0.0f;
    backgroundImage.uv1 = 0.0f;
    backgroundImage.uv2 = 1.0f;
    backgroundImage.uv3 = 1.0f;
    backgroundImage.color.Set(1.0f, 1.0f, 1.0f, 1.0f);

    backgroundImage.unk40[0] = f3;
    backgroundImage.unk40[1] = f3;
    backgroundImage.unk40[2] = f3;
    backgroundImage.unk40[3] = f3;

    foregroundImage = backgroundImage;

    foregroundImage.unk40[0] = 1.0f;
    foregroundImage.unk40[1] = 1.0f;
    foregroundImage.unk40[2] = 1.0f;
    foregroundImage.unk40[3] = 1.0f;
    foregroundImage.startX = (float)xPos - (unk268 / 2.0f);
    foregroundImage.startY = (float)yPos - (temp / 2.0f);
    foregroundImage.endX = foregroundImage.startX + (unk268 * progressRatio);
    foregroundImage.endY = foregroundImage.startY + temp;
    foregroundImage.uv2 = progressRatio;

    bDrawQuad = false;

    bInitialised = true;
}

void ProgressBar::Update(void) {
    if (floatProgress < (float)mProgress) {
        floatProgress += mUpdateVal;

        if (floatProgress > (float)mProgress) {
            floatProgress = mProgress;
        }
    }

    if (floatProgress > (float)mProgress) {
        floatProgress -= mUpdateVal;

        if (floatProgress < (float)mProgress) {
            floatProgress = mProgress;
        }
    }

    progressRatio = floatProgress / (float)maxProgress;
}

void ProgressBar::Draw(void) {
    // Background Image and Bar are optional
    if (pMat) {
        pMat->Use();

        if (!bDrawQuad) {
            backgroundImage.Draw(1);
        } else {
            backgroundQuad.Draw(&mPos);
        }
    }

    // Foreground material is not optional!
    pFGMat->Use();

    if (!bDrawQuad) {
        foregroundImage.endX = foregroundImage.startX + unk268 * progressRatio;
        foregroundImage.uv2 = progressRatio;
        foregroundImage.Draw(1);
    } else {
        foregroundQuad.unkD0 = (unk270 * progressRatio) * 0.5f;
        foregroundQuad.mTris[1].uv.x = progressRatio;
        foregroundQuad.mTris[3].uv.x = progressRatio;
        foregroundQuad.Draw(&mPos);
    }
}

// Destroys the materials
void ProgressBar::Deinit(void) {
    if (pMat) {
        pMat->Destroy();
        // Does not set to NULL
    }

    if (pFGMat) {
        pFGMat->Destroy();
        // Does not set to NULL
    }
}

void ProgressBar::SetProgress(int newProgress) {
    floatProgress = (float)newProgress;
    mProgress = newProgress;
    progressRatio = floatProgress / maxProgress;
}

/// @brief Increments the progress of a ProgressBar
/// @param  None
void ProgressBar::IncrementProgress(void) {
    if (!(floatProgress < (float)mProgress) && ++mProgress > maxProgress) {
        mProgress = maxProgress;
    }
}

/// @brief Decrements the progress of a ProgressBar
/// @param  None
/// @return False if clamped to 0 or floatProgess is greater than mProgress, otherwise true
bool ProgressBar::DecrementProgress(void) {
    if (floatProgress > (float)mProgress) {
        return false;
    }

    if (--mProgress < 0) {
        mProgress = 0;
        return false;
    }

    return true;
}

void ProgressBar::SetBackgroundBlitterUV(float f1, float f2, float f3, float f4) {
    backgroundImage.uv0 = f1;
    backgroundImage.uv1 = f2;
    backgroundImage.uv2 = f3;
    backgroundImage.uv3 = f4;
    backgroundQuad.mTris[0].uv.x = f1;
    backgroundQuad.mTris[0].uv.y = f2;
    backgroundQuad.mTris[1].uv.x = f3;
    backgroundQuad.mTris[1].uv.y = f2;
    backgroundQuad.mTris[2].uv.x = f1;
    backgroundQuad.mTris[2].uv.y = f4;
    backgroundQuad.mTris[3].uv.x = f3;
    backgroundQuad.mTris[3].uv.y = f4;
}

void ProgressBar::SetForegroundBlitterUV(float f1, float f2, float f3, float f4) {
    foregroundImage.uv0 = f1;
    foregroundImage.uv1 = f2;
    foregroundImage.uv2 = f3;
    foregroundImage.uv3 = f4;
    foregroundQuad.mTris[0].uv.x = f1;
    foregroundQuad.mTris[0].uv.y = f2;
    foregroundQuad.mTris[1].uv.x = f3;
    foregroundQuad.mTris[1].uv.y = f2;
    foregroundQuad.mTris[2].uv.x = f1;
    foregroundQuad.mTris[2].uv.y = f4;
    foregroundQuad.mTris[3].uv.x = f3;
    foregroundQuad.mTris[3].uv.y = f4;
}
