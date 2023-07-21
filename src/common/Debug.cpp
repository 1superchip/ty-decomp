#include "types.h"

struct Font {};

Font* gpDebugFont;

void Debug_InitModule(void) {
    gpDebugFont = NULL;
}

void Debug_DeinitModule(void) {

}

void Debug_SetFont(Font* mDebugFont) {
    gpDebugFont = mDebugFont;
}