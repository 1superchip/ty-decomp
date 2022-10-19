#include "common/Utils.h"

extern "C" int strlen(char*);
int strnicmp(const char*, const char*, int);

// these are used by other files
// .sdata
int gEmptyPtrList[2] = {0x00000000, 0x00000000};
int gEmptyPtrListDL[2] = {0x00000000, 0x00000000};

void* Util_BinarySearch(void *arg0, void *arg1, int arg2, int arg3, int (*compareFunc)(void *, void *)) {
	s32 temp_r3;
	s32 var_r31;
	s32 var_r30;
	s32 temp_r29;
	void *temp_r28;

	var_r30 = arg2 - 1;
	var_r31 = 0;
	while (var_r31 <= var_r30) {
		temp_r29 = (var_r31 + var_r30) >> 1;
		temp_r28 = (u8 *)arg1 + ((temp_r29 * arg3)); // need the u8* cast here
		temp_r3 = compareFunc(arg0, temp_r28);
		if (temp_r3 == 0) {
			return temp_r28;
		}
		if (temp_r3 < 0) {
			var_r30 = temp_r29 - 1;
		} else {
			var_r31 = temp_r29 + 1;
		}
	}
	return 0;
}

bool Util_WildcardPatternMatch(char *str, char *pattern) {
    u8 var_r26;
    char *temp_r4;
    char *var_r3;
    char *strEnd;
    u8 var_r25;
    u8 var_r24;
    char *var_r23;
    s8 temp_r0;
    s32 strLen;
    s32 var_r5;
    s32 var_r29;
    u8 var_r26_2;
    s32 idx;

    strLen = strlen(str);
    strEnd = &str[strLen];
    while (*pattern != 0) {
        var_r29 = 0;
        var_r5 = 0;

        while (pattern[var_r5] != 0 && pattern[var_r5] != ';' && pattern[var_r5] != '*') {
			var_r5++;
		}
    
		if (pattern[var_r5] == '*') {
			var_r23 = &pattern[var_r5] + 1;
			var_r3 = var_r23;
			while(var_r3[var_r29] != 0 && var_r3[var_r29] != ';') {
				var_r29++;
			}
		} else {
			var_r23 = &pattern[var_r5];
			var_r29 = 0;
		}
        var_r25 = 1;
        idx = (&var_r23[var_r29] - pattern);
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
        pattern += idx;
        while (*pattern == ';') {
            pattern++;
        }
    }
    return false;
}