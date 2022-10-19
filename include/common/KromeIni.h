


/*
section = unk4/name?
field = unk8?
data = unkC?
comment = unk0?
*/


// elementCount
// pos = -2989, 240, 8238 // 3 elements: {-2989, 240, 8238}
// effect = 0,none // 2 elements: {0, none}
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
	bool Init(char*); // bool?
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
    void SkipSpaces(void); // returns a bool?
    void ResetCurrLine(void);
};