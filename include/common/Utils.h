#ifndef COMMON_UTILS
#define COMMON_UTILS

#include "types.h"

void* Util_BinarySearch(void*, void*, int, int, int (*)(void*, void*));
bool Util_WildcardPatternMatch(char*, char*);

#endif // COMMON_UTILS