#ifndef COMMON_UTILS
#define COMMON_UTILS

#include "types.h"

void* Util_BinarySearch(void* valueToFind, void* arr, int count, int elemSize, int (*compareFunc)(void*, void*));
bool Util_WildcardPatternMatch(char* str, char* pattern);

#endif // COMMON_UTILS
