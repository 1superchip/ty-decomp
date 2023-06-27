#include "types.h"
#include "ty/MessageMap.h"

extern "C" int stricmp(char*, char*);

// Unused / Stripped
char* MessageMap::GetStringFromId(int id) const {
    int unk = ids;
    if ((unk <= id) && (id < unk + nmbrOfMessages)) {
        return ppMessages[id - unk];
    }
    return 0;
}

int MessageMap::GetIdFromString(char* pString) const {
    for(int i = 0; i < nmbrOfMessages; i++) {
        if (stricmp(pString, ppMessages[i]) == 0) {
            return i + ids;
        }
    }
    return 0;
}