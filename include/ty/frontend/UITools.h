#ifndef UITOOLS_H
#define UITOOLS_H

#include "ty/UserInterface.h"

/// @brief Heading for UI, contains larger text
struct UIHeading {
    UIText header;

    void Init(int stringIdx);
};

struct UIButtonDescriptor {
    bool bAnimateTextScale; // If enabled, makes the text size change as if it is "bouncing"
    u32 unselectedColor; // Color of text when the button is unselected
    u32 selectedColor; // Color of text when the button is selected
    u32 disabledColor; // Color of text when the button is disabled
};

struct UIButton {
    UIButtonDescriptor* pDescriptor;
    UIText mText;
    float unk34;
    float mAngle;
    bool bEnabled; // Default: true
    bool bSelected; // Default: false
    bool bUpdate; // Default: true

    void Init(int stringIdx, UIButtonDescriptor* pButtonDesc, Font* pFont);
    void Update(void);
    void Draw(void);
    void Reset(void);
    void SetEnabled(bool);
    void SetSelected(bool);

    void Deinit(void) {
        mText.Deinit();
    }

    void GetPosition(Vector* pPos) {
        mText.GetPosition(pPos);
    }

    bool IsEnabled(void) {
        return bEnabled;
    }
};

struct SelectionRang {
    UIModel mRangModel;
    Vector mPos;

    void Init(void);
    void SetTarget(Vector*, char, View*);
    void FlipHorizontal(void);
    void Update(void);

    // Checks if the Model's X scale is flipped horizontally
    bool Check_Row0X(void) {
        return mRangModel.pModel->matrices[0].data[0][0] < 0.0f;
    }
};

/// @brief Structure to hold a list of UIButtons
struct UIButtonGroup {
    int size; // Number of buttons in this group
    int selection; // Index of button that is selected
    UIButton* mpButtons; // Pointer to array of UIButtons
    SelectionRang* marker; // Model of selected button

    void Init(int numButtons);
    void Deinit(void);
    void Reset(int newSelectedButton);
    bool SetSelection(int newButtonIdx);
    bool SelectNext(void);
    bool SelectPrev(void);
    void Update(void);
    void Draw(void);

    UIButton* GetSelectedButton(void) {
        return &mpButtons[selection];
    }
    
    UIButton* GetButton(int index) {
        return &mpButtons[index];
    }
};

#define MAX_PROMPTS (5)
/// @brief Structure used on the FrontEnd screen to tell the user the controls
// B = Back, A = Accept, Up/Down = Select
struct UIButtonPrompt {
    int buttonIdx;
    char* str;
    void Draw(int numPrompts);
};

extern UIButtonDescriptor buttonType1;
extern UIButtonDescriptor buttonType2;

#endif // UITOOLS_H
