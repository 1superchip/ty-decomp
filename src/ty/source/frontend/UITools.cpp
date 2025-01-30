#include "ty/frontend/UITools.h"
#include "common/System_GC.h"
#include "common/Str.h"
#include "common/Heap.h"

extern struct {
    char padding0[0x8];
    Font* pFont0;
    Font* pFont1;
    char padding1[0xEC];
    View view;
    Blitter_UntexturedImage untexturedImage;
} gFERes;

UIButtonDescriptor buttonType1 = {
    true, // Animate text scale
    FONT_COLOR2(255, 135, 0, 128),      // 0x80004480, Orange (Unselected)
    FONT_COLOR2(255, 243, 0, 128),      // 0x80007A80, Bright Yellow (Selected)
    FONT_COLOR2(160, 160, 160, 128),    // 0x80505050, Gray (Disabled)
};

UIButtonDescriptor buttonType2 = {
    false, // No text scale animation
    FONT_COLOR2(160, 160, 160, 128),    // 0x80505050, Gray (Unselected)
    FONT_COLOR2(255, 255, 255, 128),    // 0x80808080, White (Selected)
    FONT_COLOR2(160, 160, 160, 128),    // 0x80505050, Gray (Disabled)
};

void UIHeading::Init(int stringIdx) {
    Vector pos = {320.0f, 40.0f, 0.0f, 0.0f};
    header.Init(gFERes.pFont0, stringIdx);
    header.SetPosition(&pos, 1);
    header.SetUnk8(272.0f);
    header.SetScale(2.0f); // Set scale of text to 2.0f because it is a header
}

void UIButton::Init(int stringIdx, UIButtonDescriptor* pButtonDesc, Font* pFont) {
    if (pButtonDesc) {
        pDescriptor = pButtonDesc;
    } else {
        pDescriptor = &buttonType1;
    }

    mAngle = 0.0f;
    unk34 = 1.0f;
    bUpdate = true;
    bEnabled = true;
    bSelected = false;
    mText.Init(pFont != NULL ? pFont : gFERes.pFont0, stringIdx);
    mText.SetColor(pDescriptor->unselectedColor);
    mText.SetScale(1.0f);
}

// Could have been placed after UIButton::Update and UIButton::Draw?
void UIButton::Reset(void) {
    if (bSelected) {
        mText.SetColor(pDescriptor->selectedColor);
        mText.SetScale(unk34 * 1.2f);
    } else {
        if (bEnabled) {
            mText.SetColor(pDescriptor->unselectedColor);
        } else {
            mText.SetColor(pDescriptor->disabledColor);
        }

        mText.SetScale(unk34);
    }

    mAngle = 0.0f;
}

void UIButton::Update(void) {
    if (!bUpdate) {
        return;
    }
    float f2 = mText.GetScale();
    if (bSelected) {
        // If selected, update text scale
        
        if (pDescriptor->bAnimateTextScale && mAngle > 0.0f) {
            mAngle += 6.0f / gDisplay.displayFreq;
            if (mAngle > (2.0f * PI)) {
                mAngle -= (2.0f * PI);
            }
            mText.SetScale((_table_sinf(mAngle) * 0.025f + 1.2f) * unk34);
            return;
        }
        
        if (f2 >= unk34 * 1.2f) {
            if (pDescriptor->bAnimateTextScale) {
                mAngle = 6.0f / gDisplay.displayFreq;
                mText.SetScale((_table_sinf(mAngle) * 0.025f + 1.2f) * unk34);
            } else {
                mText.SetScale(unk34 * 1.2f);
            }
        } else {
            mText.SetScale(f2 + (2.0f / gDisplay.displayFreq) * unk34);
        }

        return;
    }

    if (f2 <= unk34) {
        mText.SetScale(unk34);
        return;
    }

    mText.SetScale(mText.GetScale() - (2.0f / gDisplay.displayFreq) * unk34);
}

void UIButton::Draw(void) {
    if (bUpdate) {
        mText.Draw();
    }
}

/// @brief Enables or disables this button
/// @param bSetEnabled True to enable or false to disable
void UIButton::SetEnabled(bool bSetEnabled) {
    bEnabled = bSetEnabled;

    if (bEnabled) {
        // Set button to default selected state
        SetSelected(bSelected);
    } else {
        mText.SetColor(pDescriptor->disabledColor);
    }
}

/// @brief Selects or deselects this button
/// @param bSetSelected True to select or false to deselect
void UIButton::SetSelected(bool bSetSelected) {
    bSelected = bSetSelected;
    
    if (bSelected) {
        mText.SetColor(pDescriptor->selectedColor);
    } else {
        mText.SetColor(pDescriptor->unselectedColor);
    }

    mAngle = 0.0f;
}

void SelectionRang::Init(void) {
    // Selection Rang Model has no animation
    mRangModel.Init("fe_999_RangHiLight", NULL);
    mRangModel.pModel->renderType = 3;
}

void SelectionRang::SetTarget(Vector* pPos, char flags, View* pView) {
    Vector offset;
    mRangModel.mPos = *pPos;
    if (!pView) {
        pView = View::GetCurrent();
    }

    pView->TransformPoint2Dto3D(pPos->x, pPos->y, pPos->z, &mPos);

    if (flags & 1) {
        offset.Set(-85.0f, 0.0f, 0.0f, 0.0f);
        offset.ApplyRotMatrix(&mRangModel.pModel->matrices[0]);
        mPos.Add(&offset);
    } else if (flags & 2) {
        offset.Set(-170.0f, 0.0f, 0.0f, 0.0f);
        offset.ApplyRotMatrix(&mRangModel.pModel->matrices[0]);
        mPos.Add(&offset);
    }
}

void SelectionRang::FlipHorizontal(void) {
    Vector scale = {-1.0f, 1.0f, 1.0f, 1.0f};
    mRangModel.SetScale(&scale);
}

void SelectionRang::Update(void) {
    Vector offset;
    offset.Sub(&mPos, mRangModel.pModel->matrices[0].Row3());
    offset.Scale(0.5f);
    mRangModel.pModel->matrices[0].Row3()->Add(&offset);
    mRangModel.pModel->SetLocalToWorldDirty();
}

/// @brief Initiates a UIButtonGroup
/// @param numButtons Number of buttons in the group
void UIButtonGroup::Init(int numButtons) {
    marker = NULL;
    size = numButtons;
    selection = -1;
    mpButtons = (UIButton*)Heap_MemAlloc(size * sizeof(UIButton));
}

/// @brief Deinits a UIButtonGroup
/// @param  None
void UIButtonGroup::Deinit(void) {
    if (mpButtons) {
        // Deinitate all buttons
        for (int i = 0; i < size; i++) {
            mpButtons[i].Deinit();
        }

        // Free allocated memory for buttons
        Heap_MemFree((void*)mpButtons);
    }

    mpButtons = NULL;
    size = 0;
    selection = -1;
}

/// @brief Resets this button group and selects newSelectedButton
/// @param newSelectedButton Button to select
void UIButtonGroup::Reset(int newSelectedButton) {
    SetSelection(newSelectedButton);

    for (int i = 0; i < size; i++) {
        mpButtons[i].Reset();
    }

    if (marker) {
        // Reset the marker if the UIButtonGroup uses one
        *marker->mRangModel.pModel->matrices[0].Row3() = marker->mPos;
        marker->mRangModel.pModel->matrices[0].Row3()->w = 1.0f;
        marker->mRangModel.pModel->SetLocalToWorldDirty();
    }
}

/// @brief Selects a new button
/// @param newButtonIdx Button index to select
/// @return Whether the button was changed or not
bool UIButtonGroup::SetSelection(int newButtonIdx) {
    // Only select a button that is not disabled
    if (mpButtons[newButtonIdx].IsEnabled()) {
        
        // Deselect currently selected button
        if (selection >= 0 && selection < size) {
            mpButtons[selection].SetSelected(false);
        }
        
        // Select new button
        selection = newButtonIdx;
        mpButtons[selection].SetSelected(true);
        if (marker) {
            // if the UIButtonGroup has a marker, update it
            char textFlags = GetSelectedButton()->mText.GetFlags();
            if (marker->Check_Row0X()) {
                if (!(textFlags & 3)) {
                    textFlags = textFlags | 2;
                } else if (!(textFlags & 1)) {
                    textFlags = textFlags & ~3;
                }
            }

            Vector pos;
            GetSelectedButton()->GetPosition(&pos);
            pos.z = 510.0f;
            marker->SetTarget(&pos, textFlags, &gFERes.view);
        }

        return true;
    }

    return false;
}

/// @brief Selects the next button in this group
/// @param None
/// @return Whether or not a new button was selected
bool UIButtonGroup::SelectNext(void) {
    int originalSelection = selection;
    int currSelection = selection;
    while (true) {
        currSelection = (currSelection + 1) % size;
        if (currSelection == originalSelection) {
            break;
        }

        if (SetSelection(currSelection)) {
            break;
        }
    }

    return originalSelection != selection;
}

// Selects the previous button
// Returns whether or not a new button was selected
bool UIButtonGroup::SelectPrev(void) {
    int previousSelection = selection;
    int originalSelection = selection;
    while (true) {
        int nextSelection;

        if (previousSelection == 0) {
            // if the previous selection is the first button, the next will be the last button
            nextSelection = size - 1;
        } else {
            // else, the next is previous - 1
            nextSelection = previousSelection - 1;
        }

        previousSelection = nextSelection;
        if (nextSelection == originalSelection || SetSelection(nextSelection)) {
            break;
        }

    }
    return originalSelection != selection;
}

// Updates the marker and buttons
void UIButtonGroup::Update(void) {
    if (marker) {
        marker->Update();
    }

    for (int i = 0; i < size; i++) {
        mpButtons[i].Update();
    }
}

// Draws the marker and buttons
void UIButtonGroup::Draw(void) {
    if (marker) {
        marker->mRangModel.Draw();
    }

    for (int i = 0; i < size; i++) {
        mpButtons[i].Draw();
    }
}

char* buttonString[5] = {
    "\x88\x00",
    "\x87\x00",
    "\x84\x82\x00",
    "\x81\x83\x00",
    "\x7F\x89\x00"
};

void UIButtonPrompt::Draw(int numPrompts) {
    char* promptStr[MAX_PROMPTS];
    float strWidth[MAX_PROMPTS];
    Matrix matrix;
    int i;
    
    UIButtonPrompt* pPrompts = this;
    gFERes.untexturedImage.Draw(1);
    float f29 = 0.0f;
    for (i = 0; i < numPrompts; i++) {
        promptStr[i] = Str_Printf("%s %s", buttonString[pPrompts[i].buttonIdx], pPrompts[i].str);
        strWidth[i] = gFERes.pFont0->CalcLength(promptStr[i]) + 20.0f;
        f29 += strWidth[i] * 0.75f;
    }

    float f31;
    if (f29 > 544.0f) {
        f31 = 0.75f * (544.0f / f29);
        f29 = 0.0f;
    } else {
        f31 = 0.75f;
        f29 = (544.0f - f29) / (float)(numPrompts + 1);
    }

    matrix.SetIdentity();
    matrix.Scale(f31);
    matrix.Row3()->y = 458.5f;
    float f28 = 48.0f + (f29 * 0.5f);
    View::GetCurrent()->OrthoBegin();

    for (i = 0; i < numPrompts; i++) {
        f28 += ((strWidth[i] * f31) + f29) * 0.5f;
        matrix.Row3()->x = f28;
        gFERes.pFont0->DrawString(promptStr[i], strWidth[i], gFERes.pFont0->GetHeight(),
            &matrix, 5, 0x80808080, NULL, NULL);
        f28 += ((strWidth[i] * f31) + f29) * 0.5f;
    }
    
    View::GetCurrent()->OrthoEnd();
}
