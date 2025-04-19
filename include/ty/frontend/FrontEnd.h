#ifndef FRONTEND_H
#define FRONTEND_H

#include "ty/frontend/UITools.h"
#include "ty/tools.h"
#include "common/View.h"
#include "common/Font.h"
#include "common/Blitter.h"

enum tagFrontEndScreen {

};

// Called ScreenData in July 1st and debug builds
struct ScreenFaderObject : FaderObject {
    uint baseColor;
    Blitter_UntexturedImage image;

    void Init(Vector* pColour, float, float);
    bool Update(void);
    void Draw(void);
};

struct FrontEndCommonResources {
    bool bInitialised;
    int state;
    Font* pFont;
    DirectLight mDirectLight;
    View mView;
    Blitter_UntexturedImage mUntexturedImage;
    ScreenFaderObject mFader;
    UIText mText;

    bool ControllerRemoved(void);
};

extern FrontEndCommonResources gFERes;

#endif // FRONTEND_H
