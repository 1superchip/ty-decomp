#include "common/Utils.h"

extern "C" int strlen(char*);
int strnicmp(const char*, const char*, int);

int gEmptyPtrList[2] = {0x00000000, 0x00000000};
int gEmptyPtrListDL[2] = {0x00000000, 0x00000000};

void* Util_BinarySearch(void *valueToFind, void *arr, int count, int elemSize, int (*compareFunc)(void *, void *)) {
	int cmpResult;
	int low;
	int high;
	int mid;
	void *currEntry;

	high = count - 1;
	low = 0;
	while (low <= high) {
		mid = (low + high) >> 1;
		currEntry = (u8 *)arr + ((mid * elemSize)); // need the u8* cast here
		cmpResult = compareFunc(valueToFind, currEntry);
		if (cmpResult == 0) {
			return currEntry;
		}
		if (cmpResult < 0) {
			high = mid - 1;
		} else {
			low = mid + 1;
		}
	}
	return NULL;
}

// ';' is the delimiter for patterns
// '*' is a wildcard
bool Util_WildcardPatternMatch(char *str, char *pattern) {
    u8 var_r26;
    char *strEnd;
    u8 var_r25;
    u8 var_r24;
    char *var_r23;
    s32 strLen;
    s32 var_r5;
    s32 var_r29;
    u8 var_r26_2;
    int patternLength;

    strLen = strlen(str);
    strEnd = &str[strLen];
    while (*pattern != '\0') {
        var_r29 = 0;
        var_r5 = 0;

        while (pattern[var_r5] != '\0' && pattern[var_r5] != ';' && pattern[var_r5] != '*') {
			var_r5++;
		}
		if (pattern[var_r5] == '*') {
			var_r23 = &pattern[var_r5] + 1;
			// this loop finds the end of the wildcard pattern
			while(var_r23[var_r29] != '\0' && var_r23[var_r29] != ';') {
				var_r29++; // index of pattern delimiter
			}
		} else {
			var_r23 = &pattern[var_r5];
			var_r29 = 0;
		}
        var_r25 = 1;
        patternLength = (&var_r23[var_r29] - pattern);
        if (var_r5 != 0) {
            var_r26 = 0;
            if ((strLen >= var_r5) && (strnicmp(pattern, str, var_r5) == 0)) {
                var_r26 = 1;
            }
            if (var_r26 == 0) {
                var_r25 = 0;
            }
        }
        var_r24 = 1;
        if (var_r29 != 0) {
            var_r26_2 = 0;
			// (strEnd - var_r29) is the wildcard string
			// pattern of "*.lv2;"
			// (strEnd - var_r29) would be ".lv2"
            if ((strLen >= var_r29) &&
                (strnicmp(var_r23, strEnd - var_r29, var_r29) == 0)) {
                var_r26_2 = 1;
            }
            if (var_r26_2 == 0) {
                var_r24 = 0;
            }
        }
        if ((var_r25 != 0) && (var_r24 != 0)) {
            return true;
        }
        pattern += patternLength;
        while (*pattern == ';') {
            pattern++; // find the next string
        }
    }
    return false;
}