#include "types.h"
#include "ty/MessageMap.h"

extern "C" int stricmp(char*, char*);

int MessageMap::GetIdFromString(char* pString) const {
    for(int i = 0; i < nmbrOfMessages; i++) {
        if (stricmp(pString, ppMessages[i]) == 0) {
            return i + ids;
        }
    }
    return 0;
}