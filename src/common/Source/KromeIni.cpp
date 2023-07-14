#include "types.h"
#include "common/KromeIni.h"
#include "common/FileSys.h"
#include "common/Heap.h"
#include "common/Str.h"

extern "C" char* strrchr(const char*, char);
extern "C" void strcpy(char*, char*);
extern "C" int stricmp(char*, char*);
extern "C" int sscanf(const char*, const char*, ...);

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
    while ((pData < pEndOfFile) && (*pData != ' ' && *pData != '\t' && *pData != '=' && *pData != ',') && !(*pData == '\r' || *pData == '\n')) {
        pData++;
    }
    return pData < pEndOfFile;
}

bool Parser::IsName(void) {
    bool isName = false;
    if ((4 < pEndOfFile - pData) && (*pData | 0x20) == 'n' && (*(pData+1) | 0x20) == 'a' && (*(pData+2) | 0x20) == 'm' && (*(pData+3) | 0x20) == 'e' && *(pData + 4) == ' ') {
        isName = true;
    }
    return isName;
}

bool Parser::IsComment(void) {
    return (1 < (pEndOfFile - pData)) && (*pData == '/' && *(pData + 1) == '/');
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

bool Parser::IsEndOfData(void) {
    char* data = pData;
    while ((data < pEndOfFile) && (*data == ' ' || *data == '\t' || *data == '=' || *data == ',')) {
        data++;
    }
    return (data == pEndOfFile) || (*data == '\r') || (*data == '\n') || (data < (pEndOfFile - 1)) && (*data == '/' && *(data + 1) == '/');
}

// https://decomp.me/scratch/JYbFf
bool KromeIni::Init(char* pName) {
    unk34 = 0;
    if (pName == NULL || FileSys_Exists(pName, &fileSize) == false) {
        pFileMem = NULL;
        strcpy(name, "(NULL)");
        fileSize = 0;
        pLines = 0;
        nmbrOfLines = 0;
        unk38 = 0;
        return false;
    }
    pFileMem = Heap_MemAllocAligned(fileSize + 1, 0x40); // only use of Heap_MemAllocAligned and it doesn't allocate aligned memory!
    bFileMemAllocated = 1;
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


void Parser::UnkFunc(void) {
    int found = false;
    while (pData < pEndOfFile) {
        if (*pData == '\"') {
            found = !found;
            } else if ((found != 0) || ((*pData != ' ') && (*pData != '\t') && (*pData != '=') && (*pData != ','))) {
            if ((*pData == '\r') || (*pData == '\n')) {
                break;
            }
            } else {
                break;
            }
        pData++;
    }
}

bool Parser::UnkFunc2(void) {
    char* tempData = pData;
    while ((tempData < pEndOfFile) && ((*tempData == ' ') || (*tempData == '\t') || (*tempData == '=') || (*tempData == ','))) {
        tempData++;
    }
    bool retval = false;
    if ((tempData == pEndOfFile) || (*tempData == '\r') || (*tempData == '\n') || (tempData < (pEndOfFile - 1)) && (*tempData == '/' && *(tempData + 1) == '/')) {
        retval = true;
    }
    return retval;
}

void Parser::GrabIniSection(void) {
    pData++;
    currLine->section = pData;
    while ((pData < pEndOfFile) && (*pData != '\r') && (*pData != '\n') && (*pData != ']')) {
        pData++;
    }
    if (!IsComment() && !IsEOL() && (pData < pEndOfFile)) {
        *pData = '\0';
        pData++;
    }
}

void Parser::GrabMadSection(void) {
    pData += 4; // skip past "name "
    SkipSpaces();
    currLine->section = pData;
    while (pData < pEndOfFile && (*pData != '\r') && (*pData != '\n') && (*pData != ' ') && (*pData != '\t')) {
        pData++;
    }
    if (!IsComment() && !IsEOL() && (pData < pEndOfFile)) {
        *pData = 0;
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

// incorrect
void Parser::GrabData(void) {
    currLine->data = pData;
    SkipData();
    while(!UnkFunc2()) {
        if (!UnkFunc2()) {
            currLine->elementCount++;
            UnkFunc();
        }
    }
}

bool IsEOL(char c) {
    bool ret = 0;
    if (c == '\r' || c == '\n') {
        ret = 1;
    }
    return ret;
}

bool IsCommentInternal(char *c) {
    bool ret = 0;
    if (*c == '/' && *(c + 1) == '/') {
        ret = 1;
    }
    return ret;
}

inline bool IsComment(char* pEnd, char* pData) {
    bool end = 0;
    if ((pEnd - pData > 1) && IsCommentInternal(pData)) {
        end = 1;
    }
    return end;
}

void KromeIni::ParseData() {
    Parser parser;
    parser.pData = (char *)pFileMem;
    parser.pEndOfFile = parser.pData + fileSize;
    nmbrOfLines = 1;
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
    pLines = (KromeIniLine *)Heap_MemAlloc(parser.numLines * sizeof(KromeIniLine));
    parser.currLine = pLines;
    parser.pData = (char *)pFileMem;
    parser.lineNum = 0;
    parser.ResetCurrLine();

    while (parser.pData < parser.pEndOfFile) {
        parser.SkipSpaces();
        if (parser.pData >= parser.pEndOfFile) {
            break;
        }
        if (parser.IsEOL()) {
            parser.GrabEndOfLine();
        } else {
            if (parser.IsComment()) {
                parser.GrabComment();
                parser.GrabEndOfLine();
            } else {
                if (*parser.pData == '[') {
                    unk38++;
                    parser.GrabIniSection();
                    parser.SkipSpaces();
                    if (!parser.IsEndOfData()) {
                        parser.GrabFieldName();
                        parser.SkipSpaces();
                        // parser.GrabData();
                        // SkipSpaces()
                        // }
                        // this should be GrabData()
                        parser.currLine->data = parser.pData;
                        do {
                            parser.SkipSpaces();
                            if (!parser.UnkFunc2()) {
                                // var_r7_6 = 0;
                                // parser.currLine->elementCount++;
                                // while (parser.pData < parser.pEndOfFile) {
                                //     if (*parser.pData == '\"') {
                                //         var_r7_6 = !var_r7_6;
                                //     } else if ((var_r7_6 != 0) || ((*parser.pData != ' ') && (*parser.pData != '\t') && (*parser.pData != '=') && (*parser.pData != ','))) {
                                //         if ((*parser.pData == '\r') || (*parser.pData == '\n')) {
                                //             break;
                                //         }
                                //     } else {
                                //         break;
                                //     }
                                //     parser.pData++;
                                // }
                                parser.currLine->elementCount++;
                                parser.UnkFunc();
                            }
                        } while (!parser.UnkFunc2());
                        if (!IsComment(parser.pEndOfFile, parser.pData) && !IsEOL(*parser.pData) && (parser.pData < parser.pEndOfFile)) {
                            *parser.pData = 0;
                            parser.pData++;
                        }
                        parser.SkipSpaces();
                    }
                    if (parser.IsComment()) {
                        parser.GrabComment();
                    }
                    parser.GrabEndOfLine();
                } else {
                    if (parser.IsName()) {
                        parser.GrabMadSection();
                        parser.SkipSpaces();
                        if (!parser.UnkFunc2()) {
                            parser.GrabFieldName();
                            parser.SkipSpaces();
                            // parser.GrabData();
                            parser.currLine->data = parser.pData;
                            do {
								parser.SkipSpaces();
                                if (!parser.UnkFunc2()) {
                                    // var_r7_11 = 0;
                                    // parser.currLine->elementCount++;
                                    // while (parser.pData < parser.pEndOfFile) {
                                    //     if (*parser.pData == '\"') {
                                    //         var_r7_11 = !var_r7_11;
                                    //     } else if ((var_r7_11 != 0) || ((*parser.pData != ' ') && (*parser.pData != '\t') && (*parser.pData != '=') && (*parser.pData != ','))) {
                                    //         if ((*parser.pData == '\r') || (*parser.pData == '\n')) {
                                    //             break;
                                    //         }
                                    //     } else {
                                    //         break;
                                    //     }
                                    //     parser.pData++;
                                    // }
                                    parser.currLine->elementCount++;
                                    parser.UnkFunc();
                                }
                            } while (!parser.UnkFunc2());
                            if (!IsComment(parser.pEndOfFile, parser.pData) && (!IsEOL(*parser.pData) && (parser.pData < parser.pEndOfFile))) {
                                *parser.pData = 0;
                                parser.pData++;
                            }
                            parser.SkipSpaces();
                        }
                        if (parser.IsComment()) {
                            parser.GrabComment();
                        }
                        parser.GrabEndOfLine();
                    } else {
                        parser.GrabFieldName();
                        parser.SkipSpaces();
                        // parser.GrabData();
                        parser.currLine->data = parser.pData;
                        do {
                            parser.SkipSpaces();
                            if (!parser.UnkFunc2()) {
                                // var_r7_15 = 0;
                                // parser.currLine->elementCount++;
                                // while (parser.pData < parser.pEndOfFile) {
                                //     if (*parser.pData == '\"') {
                                //         var_r7_15 = !var_r7_15;
                                //     } else if ((var_r7_15 != 0) || ((*parser.pData != ' ') && (*parser.pData != '\t') && (*parser.pData != '=') && (*parser.pData != ','))) {
                                //         if ((*parser.pData == '\r') || !(*parser.pData != '\n')) {
                                //             break;
                                //         }
                                //     } else {
                                //         break;
                                //     }
                                //     parser.pData++;
                                // }
                                parser.currLine->elementCount++;
                                parser.UnkFunc();
                            }
                        } while (!parser.UnkFunc2());
                        if (!IsComment(parser.pEndOfFile, parser.pData) && !IsEOL(*parser.pData) && (parser.pData < parser.pEndOfFile)) {
                            *parser.pData = 0;
                            parser.pData++;
                        }
                        parser.SkipSpaces();
                        if (parser.IsComment()) {
                            parser.GrabComment();
                        }
                        parser.GrabEndOfLine();
                    }
                }
            }
        }
    }
}

void KromeIni::Deinit(void) {
    if (pFileMem == NULL)  {
        return;
    }
    Heap_MemFree(pLines);
    if (bFileMemAllocated != 0) {
        Heap_MemFreeAligned(pFileMem);
    }
    pFileMem = NULL;
}

KromeIniLine *KromeIni::GotoLine(char *pName, char *pField) {
    if (pFileMem == NULL) {
        return NULL;
    }
    int i = unk34; // get last line processed
    int temp_r27 = 0;
    if (pName == NULL && pField == NULL) {
        i = 0;
    }
    if (pName != NULL) {
        i = 0;
        while (i < nmbrOfLines) {
            if (pLines[i].section != NULL && stricmp(pName, pLines[i].section) == 0) {
                temp_r27 = 1;
                break;
            }
            i++;
        }
        if (i == nmbrOfLines) {
            return NULL;
        }
    }
    if (pField != NULL) {
        if (pName == NULL) {
            while (i > 0 && pLines[i].section == NULL) {
                i--;
            }
            if (pLines[i].section != NULL) {
                i++;
            }
        } else {
            if (temp_r27 & 0xFF) {
                i++;
            }
        }
        while (i < nmbrOfLines) {
            if (pLines[i].section != NULL) {
                return NULL;
            }
            if (pLines[i].pFieldName != NULL && stricmp(pField, pLines[i].pFieldName) == 0) {
                temp_r27 = 1;
                break;
            }
            i++;
        }
        if (i == nmbrOfLines) {
            return NULL;
        }
    }
    unk34 = i;
    if (i < nmbrOfLines) {
        return &pLines[i];
    }
    return NULL;
}

bool KromeIniLine::AsFlag(int arg1, int* pVal) {
    char* pString;
    bool retval = AsString(arg1, &pString);
    if (retval != false) {
        if (stricmp(pString, "on") == 0) {
            *pVal = 1;
            return true;
        }
        if (stricmp(pString, "off") == 0) {
            *pVal = 0;
            return true;
        }
        if (stricmp(pString, "true") == 0) {
            *pVal = 1;
            return true;
        }
        if (stricmp(pString, "false") == 0) {
            *pVal = 0;
            return true;
        }
        return sscanf(pString, "%d", pVal) == 1;
    }
    *pVal = 0;
    return retval;
}

bool KromeIniLine::AsInt(int arg1, int* pVal) {
    char* pString;
    bool retval = AsString(arg1, &pString);
    if (retval != false) {
        return sscanf(pString, "%d", pVal) == 1;
    }
    *pVal = 0;
    return retval;
}

bool KromeIniLine::AsFloat(int arg1, float* pVal) {
    char* pString;
    bool retval = AsString(arg1, &pString);
    if (retval != false) {
        return sscanf(pString, "%g", pVal) == 1;
    }
    *pVal = 0.0f;
    return retval;
}

bool KromeIniLine::AsString(int dataIndex, char** arg2) {
    bool temp_r5;
    int currentElement = 0;
    int temp_r8 = 0;
    char* string_r6;
    char* pChar;
    bool temp_r4;
    int dataStrLen;
    if (dataIndex >= elementCount) {
        return false;
    }
    pChar = data;
    temp_r4 = false;
    while (*pChar != '\0' && currentElement < dataIndex) {
        temp_r5 = !temp_r8 && (*pChar == ',' || *pChar == ' ' || *pChar == '\t');
        if (!temp_r5 && temp_r4) {
            currentElement++;
            if (currentElement == dataIndex) {
                // once the correct element is found, break from this loop
                break;
            }
        }
        if (*pChar == '\"') {
            temp_r8 = !temp_r8;
        }
        temp_r4 = temp_r5;
        pChar++;
    }
    temp_r5 = false;
    string_r6 = pChar;
    while (*string_r6 != '\0') {
        if (*string_r6 == '\"') {
            string_r6++;
            temp_r8 = !temp_r8;
        } else {
            temp_r4 = !temp_r8 && (*string_r6 == ',' || *string_r6 == ' ' || *string_r6 == '\t');
            if (temp_r4 && !temp_r5) {
                break;
            }
            temp_r5 = temp_r4;
            string_r6++;
        }
    }

    if (*pChar == '\"' && *(string_r6 - 1) == '\"') {
        pChar++;
        string_r6--;
    }
    dataStrLen = (int)string_r6 - (int)pChar;
    if (dataStrLen == 0) {
        *arg2 = "";
        return true;
    }
    *arg2 = Str_CopyString(pChar, dataStrLen);
    return true;
}

void KromeIni::Warning(char* arg1) {
    if (pFileMem) return;
}