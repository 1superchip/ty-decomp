#include "types.h"
#include "common/system_extras.h"

extern u8 __lower_map[256];
extern u8 __upper_map[256];

inline char GetLower(s8 s) {
    return (s == -1) ? -1 : __lower_map[s & 0xff];
}
inline char GetUpper(s8 s) {
    return (s == -1) ? -1 : __upper_map[s & 0xff];
}

extern "C" int stricmp(char* s1, char* s2) {
    while (1) {
        char s = GetLower(*s1++);
        char s_ = GetLower(*s2++);
        if (s < s_) {
            return -1;
        }
        if (s > s_) {
            return 1;
        }
        if (s == '\0') return 0;
    }
}

extern "C" int strcmpi(char* s1, char* s2) {
    while (1) {
        char s = GetLower(*s1++);
        char s_ = GetLower(*s2++);
        if (s < s_) {
            return -1;
        }
        if (s > s_) {
            return 1;
        }
        if (s == '\0') return 0;
    }
}

int strnicmp(const char* s1, const char* s2, int n) {
    for(int i = 0; i < n; i++) {
        char s = GetLower(*s1++);
        char s_ = GetLower(*s2++);
        if (s < s_) {
            return -1;
        }
        if (s > s_) {
            return 1;
        }
        if (s == '\0') {
            return 0;
        }
    }
    return 0;
}

extern "C" char* strlwr(char* s1) {
    char* s = s1;
    while (*s != '\0') {
        *s = GetLower(*s);
        s++;
    }
    return s1;
}

extern "C" char* strupr(char* s1) {
    char* s = s1;
    while (*s != '\0') {
        *s = GetUpper(*s);
        s++;
    }
    return s1;
}