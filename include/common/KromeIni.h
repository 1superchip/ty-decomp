#ifndef COMMON_KROMEINI
#define COMMON_KROMEINI

#include "types.h"

struct KromeIniLine {
    char* comment; // Line comment (denoted by //), begins at first non-whitespace character after "//"
    char* section; // name of ini section "name sectionName" Only lines that have "name sectionName" have this field
    char* pFieldName; // Line field name "pos -1, 0, 3" -> pFieldName = "pos"
    char* data; // Line data, "pos -1, 0, 3" -> data = "-1, 0, 3"
    int elementCount; // Number of data elements

    bool AsFlag(int, int*);
    bool AsInt(int, int*);
    bool AsFloat(int, float*);
    bool AsString(int, char**);
};

struct KromeIni {
    char name[0x20];
    void* pFileMem;
    bool bFileMemAllocated;
    int fileSize;
    KromeIniLine* pLines;
    int nmbrOfLines;
    int currentLineNum;
    int unk38; // count for "[]" sections
    
    inline KromeIniLine* GetNextLine(void);
    
    // seems to just get the next line with the current line
    inline KromeIniLine* GetLineWithLine(KromeIniLine* pLine);
    inline KromeIniLine* GetCurrentLine(void);
    bool Init(char* pName);
    void ParseData(void);
    void Deinit(void);
    KromeIniLine* GotoLine(char* pName, char* pField);
    void Warning(char*);
};

inline KromeIniLine* KromeIni::GetNextLine(void) {
    KromeIniLine* line;
    if (pFileMem == NULL) {
        line = NULL;
    } else {
        if (currentLineNum < nmbrOfLines) {
            currentLineNum++;
        }
        if (currentLineNum < nmbrOfLines) {
            line = &pLines[currentLineNum];
        } else {
            line = NULL;
        }
    }
    return line;
}

inline KromeIniLine* KromeIni::GetLineWithLine(KromeIniLine* pLine) {
    KromeIniLine* line;
    if (pFileMem == NULL) {
        line = NULL;
    } else {
        currentLineNum = ((int)pLine + 0x14 - (int)pLines) / sizeof(KromeIniLine);
        if (currentLineNum > nmbrOfLines) {
            currentLineNum = nmbrOfLines;
        }
        if (currentLineNum < nmbrOfLines) {
            line = &pLines[currentLineNum];
        } else {
            return NULL;
        }
    }
    return line;
}

inline KromeIniLine* KromeIni::GetCurrentLine(void) {
    KromeIniLine* line;
    if (pFileMem == NULL) {
        return NULL;
    }
    return (currentLineNum >= 0 && currentLineNum < nmbrOfLines) ? &pLines[currentLineNum] : NULL;
}

// all of parser was inlined
struct Parser {
    int lineNum;
    int numLines;
    char* pData;
    char* pEndOfFile;
    KromeIniLine* currLine;

    void UnkFunc(void);
    bool UnkFunc2(void);
    void GrabIniSection(void);
    void GrabMadSection(void);
    void GrabFieldName(void);
    void GrabData(void);
    bool IsEndOfData(void);
    void GrabComment(void);
    void GrabEndOfLine(void);
    bool IsEOL(void);
    bool IsComment(void);
    bool IsName(void);
    bool SkipData(void);
    bool SkipSpaces(void); // returns a bool
    void ResetCurrLine(void);
};

#endif // COMMON_KROMEINI
