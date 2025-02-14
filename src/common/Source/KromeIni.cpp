#include "types.h"
#include "common/KromeIni.h"
#include "common/FileSys.h"
#include "common/Heap.h"
#include "common/Str.h"

extern "C" char* strrchr(const char*, char);
extern "C" void strcpy(char*, char*);
extern "C" int stricmp(char*, char*);
extern "C" int sscanf(const char*, const char*, ...);

#define MK_DEBUG 0
#define BARBIE_SPARKING_ICE_SHOW 0

// All assertions are from the Barbie Sparking Ice Show version

#if MK_DEBUG == 1
char* Str_Printf(const char*, ...);
void assert(char*, char*, char*, int);
#define ASSERT(cond, msg, file, line) if (!(cond)) assert(#cond, msg, file, line)
#else
#define ASSERT(cond, msg, file, line)
#endif

#define TY_GC 1

void Parser::ResetCurrLine(void) {
    currLine->elementCount = 0;
    currLine->comment = NULL;
    currLine->data = NULL;
    currLine->pFieldName = NULL;
    currLine->section = NULL;
}

bool Parser::SkipSpaces(void) {
    while ((pData < pEndOfFile) && (*pData == ' ' || *pData == '\t' || *pData == '=' || *pData == ',')) {
        pData++;
    }

    return pData < pEndOfFile;
}

bool Parser::SkipData(void) {

    #if TY_GC == 1

    int found = 0;
    while (pData < pEndOfFile) {
        if (*pData == '\"') {
            found = !found;
        } else if (found || ((*pData != ' ') && (*pData != '\t') && (*pData != '=') && (*pData != ','))) {
            if ((*pData == '\r') || (*pData == '\n')) {
                break;
            }
        } else {
            break;
        }

        pData++;
    }

    #else

    while ((pData < pEndOfFile) && (*pData != ' ' && *pData != '\t' && *pData != '=' && *pData != ',' && *pData != '\r' && *pData != '\n')) {
        pData++;
    }

    #endif
    
    return pData < pEndOfFile;
}

bool Parser::IsName(void) {
    // Check for "name "
    return (4 < pEndOfFile - pData) &&
        ((*pData | 0x20) == 'n') && 
        ((*(pData+1) | 0x20) == 'a') && 
        ((*(pData+2) | 0x20) == 'm') && 
        ((*(pData+3) | 0x20) == 'e') && 
        (*(pData + 4) == ' ');
}

bool Parser::IsComment(void) {
    return (1 < (pEndOfFile - pData) && (*pData == '/' && *(pData + 1) == '/'));
}

bool Parser::IsEOL(void) {
	return *pData == '\r' || *pData == '\n';
}

void Parser::GrabEndOfLine(void) {
    char r7 = '\0';
    while (pData < pEndOfFile && (r7 != '\r' && r7 != '\n')) {
        r7 = *pData;
        *pData++ = '\0';
    }
    
    if ((pData < pEndOfFile) && (((r7 == '\r') && (*pData == '\n')) || ((r7 == '\n') && (*pData == '\r')))) {
        *pData++ = '\0';
    }
    
    lineNum++;
    currLine++;
    
    if (lineNum < numLines) {
        ResetCurrLine();
    }
}

void Parser::GrabComment(void) {
    *pData = '\0';
    pData += 2;

    SkipSpaces();
    
    currLine->comment = pData;
    
    while (pData < pEndOfFile && !(*pData == '\r' || *pData == '\n')) {
        pData++;
    }
}

void Parser::GrabIniSection(void) {
    pData += 1; // skip '['
    
    currLine->section = pData;
    
    while ((pData < pEndOfFile) && (*pData != '\r') && (*pData != '\n') && (*pData != ']')) {
        pData++;
    }

    ASSERT(
        pChar[0] == ']',
        Str_Printf(
            "File %s, Malformed secton name, line %d", 
            pFilename,
            lineNum
        ),
        "D:\\Src\\IceSkating\\BarbieCommon\\Source\\KromeIni.cpp",
        173
    );
    
    if (!IsComment() && !IsEOL() && (pData < pEndOfFile)) {
        *pData = '\0';
        pData++;
    }
}

void Parser::GrabMadSection(void) {
    pData += 4; // skip past "name "

    int more = SkipSpaces();
    
    currLine->section = pData;

    ASSERT(
        more && !IsEOL() && !IsComment(),
        Str_Printf(
            "File %s, Malformed section name, line %d", 
            pFilename,
            lineNum
        ),
        "D:\\Src\\IceSkating\\BarbieCommon\\Source\\KromeIni.cpp",
        193
    );
    
    while (pData < pEndOfFile && (*pData != '\r') && (*pData != '\n') && (*pData != ' ') && (*pData != '\t')) {
        pData++;
    }
    
    if (!IsComment() && !IsEOL() && (pData < pEndOfFile)) {
        *pData = '\0';
        pData++;
    }
}

void Parser::GrabFieldName(void) {
    currLine->pFieldName = pData;
    while (pData < pEndOfFile && ((*pData != '\r') && (*pData != '\n') && (*pData != ' ') && (*pData != '\t') && (*pData != '='))) {
        pData++;
    }
    
    if (!IsComment() && !IsEOL() && (pData < pEndOfFile)) {
        *pData = '\0';
        pData++;
    }
}

void Parser::GrabData(void) {
    currLine->data = pData;
    do {
        SkipSpaces();
        if (!IsEndOfData()) {
            currLine->elementCount++;
            SkipData();
        }
    } while (!IsEndOfData());
    
    if (!IsComment() && !IsEOL() && (pData < pEndOfFile)) {
        *pData = '\0';
        pData++;
    }
}

bool Parser::IsEndOfData(void) {
    char* data = pData;
    
    while ((data < pEndOfFile) && (*data == ' ' || *data == '\t' || *data == '=' || *data == ',')) {
        data++;
    }

    return (data == pEndOfFile) || 
        (*data == '\r') || (*data == '\n') || 
        ((data < (pEndOfFile - 1)) && (*data == '/' && *(data + 1) == '/'));
}

bool KromeIni::Init(char* pName) {
    currentLineNum = 0;

    if (pName == NULL || FileSys_Exists(pName, &fileSize) == false) {
        pFileMem = NULL;
        strcpy(name, "(NULL)");
        fileSize = 0;
        pLines = NULL;
        nmbrOfLines = 0;
        unk38 = 0;
        return false;
    }
    
    // only use of Heap_MemAllocAligned and it doesn't allocate aligned memory!
    pFileMem = Heap_MemAllocAligned(fileSize + 1, 64);
    bFileMemAllocated = true;

    FileSys_Load(pName, &fileSize, pFileMem, -1);
    *((char*)pFileMem + (int)fileSize) = '\0'; // null terminator at end of file

    char* str = strrchr(pName, '\\');
    char* foundChar = str;
    if (str == NULL) {
        str = strrchr(pName, '/');
        foundChar = str;
        if (str == NULL) {
            foundChar = pName;
        }
    }

    strcpy(name, foundChar);

    ParseData();

    return true;
}

// void KromeIni::InitFromBuffer(char* pName, char* pBuf) {
//     bFileMemAllocated = false;
//     pFileMem = (void*)pBuf;
//     fileSize = strlen(pBuf);
//     strcpy(name, pName);
//     ParseData();
// }

void KromeIni::ParseData() {
    Parser parser;
    parser.pData = (char *)pFileMem;
    parser.pEndOfFile = parser.pData + fileSize;
    
    nmbrOfLines = 1;
    
    parser.pFilename = name;
    
    unk38 = 0;
    
    // get line count
    while (parser.pData < parser.pEndOfFile) {
        // search for newline characters, if found increment nmbrOfLines
        if (*parser.pData == '\n') {
            nmbrOfLines++;
        }
        parser.pData++;
    }


    parser.numLines = nmbrOfLines;
    pLines = (KromeIniLine*)Heap_MemAlloc(parser.numLines * sizeof(KromeIniLine));
    parser.currLine = pLines;
    parser.pData = (char*)pFileMem;
    parser.lineNum = 0;
    parser.ResetCurrLine();

    while (parser.pData < parser.pEndOfFile) {
        if (!parser.SkipSpaces()) {
            break;
        }
        
        if (parser.IsEOL()) {
            parser.GrabEndOfLine();
        } else if (parser.IsComment()) {
            parser.GrabComment();
            parser.GrabEndOfLine();
        } else if (*parser.pData == '[') {
            unk38++;
            
            parser.GrabIniSection();
            parser.SkipSpaces();
            
            if (!parser.IsEndOfData()) {
                parser.GrabFieldName();
                parser.SkipSpaces();
                parser.GrabData();
                parser.SkipSpaces();
            }
            
            if (parser.IsComment()) {
                parser.GrabComment();
            }
            
            parser.GrabEndOfLine();
        } else if (parser.IsName()) {
            parser.GrabMadSection();
            parser.SkipSpaces();
            if (!parser.IsEndOfData()) {
                parser.GrabFieldName();
                parser.SkipSpaces();
                parser.GrabData();
                parser.SkipSpaces();
            }
            
            if (parser.IsComment()) {
                parser.GrabComment();
            }
            
            parser.GrabEndOfLine();
        } else {
            parser.GrabFieldName();
            parser.SkipSpaces();
            parser.GrabData();
            parser.SkipSpaces();
            if (parser.IsComment()) {
                parser.GrabComment();
            }
            
            parser.GrabEndOfLine();
        }
    }
}

void KromeIni::Deinit(void) {
    if (pFileMem == NULL)  {
        return;
    }

    Heap_MemFree(pLines);

    if (bFileMemAllocated) {
        Heap_MemFreeAligned(pFileMem);
    }

    pFileMem = NULL;
}

// void KromeIni::Dump(void) {
//     if (pFileMem && nmbrOfLines > 0) {
//         int lineCount = nmbrOfLines;
//         while(lineCount != 0) {
//             lineCount--;
//         }
//     }
// }

KromeIniLine* KromeIni::GotoLine(char* pSectionName, char* pFieldName) {
    if (pFileMem == NULL) {
        return NULL;
    }
    
    int i = currentLineNum; // get last line processed
    bool bFoundSection = false;

    if (pSectionName == NULL && pFieldName == NULL) {
        i = 0;
    }
    
    // Find the start of the section of pSectionName
    if (pSectionName != NULL) {
        i = 0;
        while (i < nmbrOfLines) {
            if (pLines[i].section != NULL && stricmp(pSectionName, pLines[i].section) == 0) {
                bFoundSection = true;
                break;
            }

            i++;
        }

        if (i == nmbrOfLines) {
            return NULL;
        }
    }
    
    if (pFieldName != NULL) {
        if (pSectionName == NULL) {
            while (i > 0 && pLines[i].section == NULL) {
                i--;
            }
            
            if (pLines[i].section != NULL) {
                i++;
            }
        } else if (bFoundSection) {
            // Skip the section line
            i++;
        }
        
        while (i < nmbrOfLines) {
            if (pLines[i].section != NULL) {
                return NULL;
            }
            
            if (pLines[i].pFieldName != NULL && stricmp(pFieldName, pLines[i].pFieldName) == 0) {
                break;
            }
            
            i++;
        }
        
        if (i == nmbrOfLines) {
            return NULL;
        }
    }
    
    currentLineNum = i;
    
    return i < nmbrOfLines ? &pLines[i] : NULL;
}

bool KromeIniLine::AsFlag(int elementIndex, int* pVal) {

    ASSERT(pVal, "pVal pointer is NULL", "D:\\Src\\IceSkating\\BarbieCommon\\Source\\KromeIni.cpp", 657);
    
    char* pString;
    bool retval = AsString(elementIndex, &pString);
    
    if (retval != false) {
        if (stricmp(pString, "on") == 0) {
            *pVal = 1;
            retval = true;
        } else if (stricmp(pString, "off") == 0) {
            *pVal = 0;
            retval = true;
        } else if (stricmp(pString, "true") == 0) {
            *pVal = 1;
            retval = true;
        } else if (stricmp(pString, "false") == 0) {
            *pVal = 0;
            retval = true;
        } else {
            retval = sscanf(pString, "%d", pVal) == 1;
        }
    } else {
        *pVal = 0;
    }
    
    return retval;
}

bool KromeIniLine::AsInt(int elementIndex, int* pVal) {

    ASSERT(pVal, "pVal pointer is NULL", "D:\\Src\\IceSkating\\BarbieCommon\\Source\\KromeIni.cpp", 707);
    
    char* pString;
    bool retval = AsString(elementIndex, &pString);
    
    if (retval != false) {
        retval = sscanf(pString, "%d", pVal) == 1;
    } else {
        *pVal = 0;
    }
    
    return retval;
}

bool KromeIniLine::AsFloat(int elementIndex, float* pVal) {

    ASSERT(pVal, "pVal pointer is NULL", "D:\\Src\\IceSkating\\BarbieCommon\\Source\\KromeIni.cpp", 735);
    
    char* pString;
    bool retval = AsString(elementIndex, &pString);
    
    if (retval != false) {
        retval = sscanf(pString, "%g", pVal) == 1;
    } else {
        *pVal = 0.0f;
    }
    
    return retval;
}

#if TY_GC == 0

// July 1st prototype and Sparking Ice Show version

bool KromeIniLine::AsString(int index, char** pString) {
    
    int i = 0;
    char* pChar;
    int dataLen;

    ASSERT(pString, "pString pointer is NULL", "D:\\Src\\IceSkating\\BarbieCommon\\Source\\KromeIni.cpp", 763);
    
    if (index >= elementCount) {
        return false;
    }
	
	// finds the beginning of element data
    pChar = data;
    bool isPrevWhiteSpace = false; // Whether the previous character is whitespace or not
    while (*pChar != '\0' && i < index) {
        bool isWhiteSpace = *pChar == ',' || *pChar == ' ' || *pChar == '\t';
        
        if (!isWhiteSpace && isPrevWhiteSpace) {
            i++;
            if (i == index) {
                break;
            }
        }
        
        isPrevWhiteSpace = isWhiteSpace;
        pChar++;
    }
	
	// find the end of the data (next whitespace character)
    char* string_r6 = pChar;
    isPrevWhiteSpace = false; // Whether the previous character is whitespace or not
    
    while (*string_r6 != '\0') {
        bool isWhiteSpace = *string_r6 == ',' || *string_r6 == ' ' || *string_r6 == '\t';
        
        if (isWhiteSpace && !isPrevWhiteSpace) {
            break;
        }
        
        isPrevWhiteSpace = isWhiteSpace;
        string_r6++;
    }
    
    dataLen = string_r6 - pChar;
    if (dataLen == 0) {
        *pString = "";
        return true;
    } else {
        *pString = Str_CopyString(pChar, dataLen);
        return true;
    }
}

#elif TY_GC == 1

bool KromeIniLine::AsString(int elementIndex, char** pString) {
    int currentElement = 0;
    int temp_r8 = 0;
    char* pEndOfData;
    char* pData;
    bool isPrevWhiteSpace;
    int dataLen;
    
    if (elementIndex >= elementCount) {
        return false;
    }
    
    // find the beginning of the data
    pData = data;
    isPrevWhiteSpace = false;
    while (*pData != '\0' && currentElement < elementIndex) {
        bool isWhiteSpace = !temp_r8 && (*pData == ',' || *pData == ' ' || *pData == '\t');
        
        if (!isWhiteSpace && isPrevWhiteSpace) {
            currentElement++;
            if (currentElement == elementIndex) {
                // once the correct element is found, break from this loop
                break;
            }
        }
        
        if (*pData == '\"') {
            temp_r8 = !temp_r8;
        }
        
        isPrevWhiteSpace = isWhiteSpace;
        pData++;
    }

    // find the end of the data
    isPrevWhiteSpace = false;
    pEndOfData = pData;
    while (*pEndOfData != '\0') {
        if (*pEndOfData == '\"') {
            temp_r8 = !temp_r8;
            pEndOfData++;
        } else {
            bool isWhiteSpace = !temp_r8 && (*pEndOfData == ',' || *pEndOfData == ' ' || *pEndOfData == '\t');
            if (isWhiteSpace && !isPrevWhiteSpace) {
                break;
            }
            
            isPrevWhiteSpace = isWhiteSpace;
            pEndOfData++;
        }
        
    }

    if (*pData == '\"' && pEndOfData[-1] == '\"') {
        pData++;
        pEndOfData--;
    }
    
    
    dataLen = (int)pEndOfData - (int)pData;
    if (dataLen == 0) {
        *pString = "";
        return true;
    } else {
        *pString = Str_CopyString(pData, dataLen);
        return true;
    }
}

#endif

// Following KromeIniLine methods were most likely stripped

// bool KromeIniLine::AsStringRange(int index, char** pLow, char** pHi) {
//     char* pString;
    
//     if (AsString(index, &pString) == false) {
//         return false;
//     }

//     char* s0 = pString;
//     char* a2 = pString;

//     while (*s0 != '\0' && *s0 != '-') {
//         s0++;
//     }

//     if (*s0 != '-') {
//         *pLow = pString;
//         *pHi = pString;

//         return true;
//     }

//     *pLow = Str_CopyString(pString, s0 - a2 - 1);
//     *pHi = s0 + 1;

//     return true;
// }

// extern "C" int atoi(const char *str);

// bool KromeIniLine::AsIntRange(int index, int* pVal, int* pVal1) {
//     char* pLowStr;
//     char* pHiStr;
    
//     if (AsStringRange(index, &pLowStr, &pHiStr) == false) {
//         return false;
//     }
    
//     *pVal = atoi(pLowStr);
//     *pVal1 = atoi(pHiStr);

//     return true;
// }

// bool KromeIniLine::AsFloatRange(int index, float* pVal, float* pVal1) {
//     char* pLowStr;
//     char* pHiStr;
    
//     if (AsStringRange(index, &pLowStr, &pHiStr) == false) {
//         return false;
//     }
    
//     sscanf(pLowStr, "%g", pVal);
//     sscanf(pHiStr, "%g", pVal1);

//     return true;
// }

void KromeIni::Warning(char* arg1) {
    if (pFileMem) return;
}
