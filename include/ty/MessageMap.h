#ifndef MESSAGEMAP_H
#define MESSAGEMAP_H

// figure out names
struct MessageMap {
    int ids;
    int nmbrOfMessages;
    char** ppMessages;
    int GetIdFromString(char* pString) const;
    char* GetStringFromId(int) const; // Unused / Stripped
};

#endif // MESSAGEMAP_H
