#ifndef COMMON_DEBUG
#define COMMON_DEBUG

#include "common/Font.h"
#include "common/Str.h"

// these may be class methods of Vector
inline char* FormatVector(Vector* pVec) {
    return Str_Printf("(%5.3f, %5.3f, %5.3f)", pVec->x, pVec->y, pVec->z);
}

inline char* PrintVectorW(Vector* pVec) {
    return Str_Printf("(%5.3f, %5.3f, %5.3f, %5.3f)", pVec->x, pVec->y, pVec->z, pVec->w);
}

#ifndef BARBIE_ICESKATING
#define BARBIE_ICESKATING (0)
#endif // BARBIE_ICESKATING

#ifndef MK_DEBUG
#define MK_DEBUG 0
#endif // MK_DEBUG

#if MK_DEBUG == 1

void Debug_Assert(char*, char*, char*, int);
#ifndef ASSERT
#define ASSERT(cond, msg, file, line) if (!(cond)) Debug_Assert(#cond, msg, file, line)
#endif // ASSERT

#else

#ifndef ASSERT
#define ASSERT(cond, msg, file, line)
#endif // ASSERT

#endif

void Debug_InitModule(void);
void Debug_DeinitModule(void);
void Debug_SetFont(Font* mDebugFont);

extern Font* gpDebugFont;

#endif // COMMON_DEBUG
