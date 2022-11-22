#pragma once on

struct KromeIniLine {
    char* comment;
    char* section;
    char* pFieldName;
    char* data;
    int elementCount;
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
    int unk34;
    int unk38; // count for "[]" sections
	
	inline KromeIniLine* GetNextLine(void);
	inline KromeIniLine* GetLineWithLine(KromeIniLine*);
	inline KromeIniLine* GetLine3(void);
	bool Init(char*);
	void ParseData(void);
	void Deinit(void);
	KromeIniLine* GotoLine(char*, char*);
	void Warning(char*);
};

inline KromeIniLine* KromeIni::GetNextLine(void) {
    KromeIniLine* line;
    if (pFileMem == NULL) {
        line = NULL;
    } else {
        if (unk34 < nmbrOfLines) {
            unk34++;
        }
        if (unk34 < nmbrOfLines) {
            line = &pLines[unk34];
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
        unk34 = ((int)pLine + 0x14 - (int)pLines) / sizeof(KromeIniLine);
        if (unk34 > nmbrOfLines) {
            unk34 = nmbrOfLines;
        }
        if (unk34 < nmbrOfLines) {
            line = &pLines[unk34];
        } else {
            return NULL;
        }
    }
    return line;
}

inline KromeIniLine* KromeIni::GetLine3(void) {
    KromeIniLine* line;
    if (pFileMem == NULL) {
        return NULL;
    }
    return (unk34 >= 0 && unk34 < nmbrOfLines) ? &pLines[unk34] : NULL;
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
    void SkipData(void);
    void SkipSpaces(void); // returns a bool
    void ResetCurrLine(void);
};