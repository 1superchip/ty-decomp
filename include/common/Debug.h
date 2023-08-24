#ifndef COMMON_DEBUG
#define COMMON_DEBUG

#include "common/Font.h"

void Debug_InitModule(void);
void Debug_DeinitModule(void);
void Debug_SetFont(Font* mDebugFont);

extern Font* gpDebugFont;

#endif // COMMON_DEBUG
