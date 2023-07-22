#include "types.h"
#include "common/Debug.h"

Font* gpDebugFont;

void Debug_InitModule(void) {
    gpDebugFont = NULL;
}

void Debug_DeinitModule(void) {

}

void Debug_SetFont(Font* mDebugFont) {
    gpDebugFont = mDebugFont;
}
