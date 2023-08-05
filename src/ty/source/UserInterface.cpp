#include "ty/UserInterface.h"
#include "common/Translation.h"

// probably a fake inline
static inline Texture* getTexture(Material* material) {
    if (material->unk58 != NULL) {
        return material->unk58;
    } else {
        return material->unk54;
    }
}

void UIModel::Init(char* pModelName, char* pAnimName) {
    pModel = Model::Create(pModelName, pAnimName);
    pModel->matrices[0].SetIdentity();
    BoundingVolume* pBV = pModel->GetBoundingVolume(-1);
    unk34 = pBV->v1.y + (pBV->v2.y * 0.5f);
    pModel->matrices[0].Row3()->Set(0.0f, unk34, 100.0f, 1.0f);
    pModel->SetLocalToWorldDirty();
    if (pAnimName != NULL) {
        mAnimScript.Init(pAnimName);
    }
}

void UIModel::Deinit(void) {
    if (pModel->GetAnimation() != NULL) {
        mAnimScript.Deinit();
    }
    if (pModel != NULL) {
        pModel->Destroy();
    }
    pModel = NULL;
}

/// @brief Updates the Model's Animation of this UIModel object
/// @param None
void UIModel::Update(void) {
    mAnimScript.Animate();
    mAnimScript.Apply(pModel->GetAnimation());
}

/// @brief Draws this UIModel
/// @param None
void UIModel::Draw(void) {
    if (pModel != NULL) {
        pModel->Draw(NULL);
    }
}

/// @brief Sets the position of the Model and UIModel object
/// @param pPos 
/// @param r5 Unused
/// @param pView 
void UIModel::SetPosition(Vector* pPos, char r5, View* pView) {
    if (pView == NULL) {
        pView = View::GetCurrent();
    }
    pView->TransformPoint2Dto3D(pPos->x, pPos->y, pPos->z, pModel->matrices[0].Row3());
    pModel->matrices[0].Row3()->w = 1.0f;
    pModel->SetLocalToWorldDirty();
    mPos = *pPos;
}

void UIImage::Init(char* pMatName) {
    pMaterial = Material::Create(pMatName);
    img.startX = 0.0f;
    img.startY = 0.0f;
    // this inline doesn't exist in the debug build so it might be fake?
    img.endX = getTexture(pMaterial)->width;
    img.endY = getTexture(pMaterial)->height;
    img.z = 0.0f;
    img.uvs[0] = 0.0f;
    img.uvs[1] = 0.0f;
    img.uvs[2] = 1.0f;
    img.uvs[3] = 1.0f;
    img.color.Set(1.0f, 1.0f, 1.0f, 1.0f);
}

void UIImage::Init(Material* t_pMaterial) {
    if (t_pMaterial != pMaterial) {
        if (pMaterial != NULL) {
            Deinit();
        }
        t_pMaterial->referenceCount++;
        pMaterial = t_pMaterial;
    }
}

/// @brief Initiates an UIImage object
/// @param endX End X position
/// @param endY End Y position
void UIImage::Init(float endX, float endY) {
    pMaterial = NULL;

    // Initialize Blitter_Image
    img.startX = 0.0f;
    img.startY = 0.0f;
    img.endX = endX;
    img.endY = endY;
    img.z = 0.0f;
    img.uvs[0] = 0.0f;
    img.uvs[1] = 0.0f;
    img.uvs[2] = 1.0f;
    img.uvs[3] = 1.0f;
    img.color.Set(1.0f, 1.0f, 1.0f, 1.0f);
}

void UIImage::Deinit(void) {
    if (pMaterial) {
        pMaterial->Destroy();
        pMaterial = NULL;
    }
}

/// @brief Draws the UIImage
/// @param None
void UIImage::Draw(void) {
    pMaterial->Use();
    img.Draw(1);
}

void UIImage::SetUVs(float uv0, float uv1, float uv2, float uv3) {
    img.uvs[0] = uv0;
    img.uvs[1] = uv1;
    img.uvs[2] = uv2;
    img.uvs[3] = uv3;
}

// TODO: Document these flags
void UIImage::SetPosition(Vector* pPos, char flags) {
    float dx = img.endX - img.startX;
    float dy = img.endY - img.startY;
    if (!(flags & 3)) {
        img.startX = pPos->x;
        img.endX = pPos->x + dx;
    } else if (flags & 1) {
        img.startX = pPos->x - (dx * 0.5f);
        img.endX = pPos->x + (dx * 0.5f);
    } else {
        img.startX = pPos->x - dx;
        img.endX = pPos->x;
    }
    if (!(flags & 12)) {
        img.startY = pPos->y;
        img.endY = pPos->y + dy;
    } else if (flags & 4) {
        img.startY = pPos->y - (dy * 0.5f);
        img.endY = pPos->y + (dy * 0.5f);
    } else {
        img.startY = pPos->y - dy;
        img.endY = pPos->y;
    }
    img.z = pPos->z;
}

void UIText::Init(Font* t_pFont, int stringIdx) {
    t_pFont->referenceCount++;
    pFont = t_pFont;
    mStringIdx = stringIdx;
    pText = NULL;
    // Inline here
    x = 0.0f;
    y = 0.0f;
    unk8 = 320.0f;
    fontHeight = pFont->GetHeight();
    mRoll = 0.0f;
    mScaleX = 1.0f;
    mScaleY = 1.0f;
    mFlags = 0;
    color = 0x80808080;
}

/// @brief Deinits UIText object
/// @param None
void UIText::Deinit(void) {
    if (pFont) {
        pFont->Destroy();
        pFont = NULL;
    }
}

float UIText::Draw(void) {
    if (pText) {
        return Draw(pText);
    } else if (mStringIdx > -1) {
        return Draw(gpTranslation_StringArray[mStringIdx]);
    }
    return 0.0f;
}

float UIText::Draw(char* pDrawString) {
    Vector s; // Matrix scale vector
    Matrix matrix;
    matrix.SetIdentity();
    matrix.SetRotationRoll(mRoll);
    s.Set(mScaleX, mScaleY, 1.0f, 1.0f);
    matrix.Scale(&s);
    if (!(mFlags & 3)) {
        matrix.Row3()->x = x + (unk8 / 2.0f) * mScaleX;
    } else if (mFlags & 1) {
        matrix.Row3()->x = x;
    } else {
        matrix.Row3()->x = x - (unk8 / 2.0f) * mScaleX;
    }
    if (!(mFlags & 12)) {
        matrix.Row3()->y = y + (fontHeight / 2.0f) * mScaleY;
    } else if (mFlags & 4) {
        matrix.Row3()->y = y;
    } else {
        matrix.Row3()->y = y - (fontHeight / 2.0f) * mScaleY;
    }
    
    View::GetCurrent()->OrthoBegin();
    
    // Interestingly, this code draws the string 2x for a background effect
    matrix.Row3()->x += 2.0f;
    matrix.Row3()->y += 2.0f;
    pFont->DrawString(pDrawString, unk8, fontHeight, &matrix, mFlags, 0x80000000, NULL, NULL);
    
    matrix.Row3()->x -= 2.0f;
    matrix.Row3()->y -= 2.0f;
    float ret = pFont->DrawString(pDrawString, unk8, fontHeight, &matrix, mFlags, color, NULL, NULL);
    
    View::GetCurrent()->OrthoEnd();
    return ret;
}

/// @brief Sets Position flags and Position if pPos is not NULL
/// @param pPos Position vector (Optional, pass NULL if not needed)
/// @param t_flags Position flags for UIText to use
void UIText::SetPosition(Vector* pPos, char t_flags) {
    mFlags = t_flags;
    if (pPos) {
        x = pPos->x;
        y = pPos->y;
    }
}

/// @brief Sets the X and Y scale to _scale
/// @param _scale X and Y scale
void UIText::SetScale(float _scale) {
    mScaleX = _scale;
    mScaleY = _scale;
}

/// @brief Sets the X and Y scale to xScale and yScale
/// @param xScale X scale for text
/// @param yScale Y scale for text
void UIText::SetScaleEx(float xScale, float yScale) {
    mScaleX = xScale;
    mScaleY = yScale;
}

/// @brief Copies the UIText position to pOutPos if pOutPos is not NULL
/// @param pOutPos Vector to store position to (Optional, pass NULL if not need)
void UIText::GetPosition(Vector* pOutPos) {
    if (pOutPos) {
        pOutPos->x = x;
        pOutPos->y = y;
        pOutPos->z = 0.0f;
    }
}
