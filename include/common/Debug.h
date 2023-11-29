#ifndef COMMON_DEBUG
#define COMMON_DEBUG

#include "common/Font.h"
#include "common/Str.h"

// inline char* FormatVector(Vector* pVec) {
//     return Str_Printf("(%5.3f, %5.3f, %5.3f)", pVec->x, pVec->y, pVec->z);
// }

// inline char* PrintVector(Vector* pVec) {
//     return Str_Printf("(%5.3f, %5.3f, %5.3f, %5.3f)", pVec->x, pVec->y, pVec->z, pVec->w);
// }

void Debug_InitModule(void);
void Debug_DeinitModule(void);
void Debug_SetFont(Font* mDebugFont);

extern Font* gpDebugFont;

#endif // COMMON_DEBUG
