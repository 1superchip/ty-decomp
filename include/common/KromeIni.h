#ifndef COMMON_KROMEINI
#define COMMON_KROMEINI

#include "types.h"

struct KromeIniLine {
    char* comment; // Line comment (denoted by //), begins at first non-whitespace character after "//"
    char* section; // name of ini section "name sectionName" Only lines that have "name sectionName" have this field
    char* pFieldName; // Line field name "pos -1, 0, 3" -> pFieldName = "pos"
    char* data; // Line data, "pos -1, 0, 3" -> data = "-1, 0, 3"
    int elementCount; // Number of data elements

    bool AsFlag(int elementIndex, int* pVal);
    bool AsInt(int elementIndex, int* pVal);
    bool AsFloat(int elementIndex, float* pVal);
    bool AsString(int elementIndex, char** pString);
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
    inline KromeIniLine* GetPreviousLine(void);
    
    // seems to just get the next line with the current line
    inline KromeIniLine* GetLineWithLine(KromeIniLine* pLine);
    inline KromeIniLine* GetCurrentLine(void);
    bool Init(char* pName);
    void ParseData(void);
    void Deinit(void);
    KromeIniLine* GotoLine(char* pSectionName, char* pFieldName);
    void Warning(char*);
};

inline KromeIniLine* KromeIni::GetNextLine(void) {
    if (pFileMem == NULL) {
        return NULL;
    }

    if (currentLineNum < nmbrOfLines) {
        currentLineNum++;
    }

    return currentLineNum < nmbrOfLines ? &pLines[currentLineNum] : NULL;
}

inline KromeIniLine* KromeIni::GetPreviousLine(void) {
    if (pFileMem == NULL) {
        return NULL;
    }

    if (currentLineNum >= 0) {
        currentLineNum--;
    }

    return currentLineNum >= 0 ? &pLines[currentLineNum] : NULL;
}

inline KromeIniLine* KromeIni::GetLineWithLine(KromeIniLine* pLine) {
    if (pFileMem == NULL) {
        return NULL;
    }
    
    KromeIniLine* pNextLine = pLine + 1;
    currentLineNum = ((int)pNextLine - (int)pLines) / sizeof(KromeIniLine);
    
    if (currentLineNum > nmbrOfLines) {
        currentLineNum = nmbrOfLines;
    }

    return currentLineNum < nmbrOfLines ? &pLines[currentLineNum] : NULL;
}

inline KromeIniLine* KromeIni::GetCurrentLine(void) {
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
    char* pFilename;

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
    bool SkipSpaces(void);
    void ResetCurrLine(void);
};

#endif // COMMON_KROMEINI
