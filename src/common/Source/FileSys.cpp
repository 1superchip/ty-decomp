#include "types.h"
#include "common/FileSys.h"
#include "common/Heap.h"
#include "common/File.h"
#include "common/Str.h"
#include "common/Utils.h"

extern "C" int stricmp(char*, char*);
extern "C" void strcpy(char*, char*);
extern "C" char* strcat(char*, char*);
extern "C" int strlen(char*);
extern "C" char* strstr(char*, char*);
extern "C" void qsort(void*, int, int, int (*compar)(const void*,const void*));
char* System_GetCommandLineParameter(char*);


static RkvTOC data;
static RkvTOC patch;

static void* (*pLoadInterceptHandler)(char*, int*, void*, int*);
static u8 (*pExistInterceptHandler)(char*, int*, int);
static s16 fileOrderId;

// for RkvTOC
template <typename T> 
void ByteReverse(T& data);

void FileSys_InitModule(void) {
    data.Init(File_FileServerFilename("Data_GC.rkv"));
    patch.Init(File_FileServerFilename("Patch_GC.rkv"));
    pLoadInterceptHandler = NULL;
    pExistInterceptHandler = NULL;
    fileOrderId = 1;
}

void FileSys_DeinitModule(void) {
	return;
}

void FileSys_Update(void) {
	return;
}

static int EntryCompare(void* arg0, void* arg1) {
	return stricmp((char*)arg0, (char*)arg1);
}

void RkvTOC::Init(char *pName) {
    strcpy(name, pName);
    int fileLength = File_Length(pName);
    int fd;
    if (fileLength != 0) {
        fd = File_Open(pName, 0);
    } else {
        fd = -1;
    }
    rkvFd = fd;
    unk54 = -1;
    unk58 = false;

    if (rkvFd >= 0) {
        // alloc and read header
        void *pBuf = Heap_MemAlloc(0x20);
        File_Seek(rkvFd, -0x20, 2);
        File_Read(rkvFd, pBuf, 0x20, 0x20);
        nmbrOfEntries = *(int *)((int)pBuf + 0x18);
        unk48 = *(int *)((int)pBuf + 0x1C);
        ByteReverse<int>(nmbrOfEntries);
        ByteReverse<int>(unk48);
        Heap_MemFree(pBuf);
		int fileSize = nmbrOfEntries * sizeof(RkvFileEntry) + unk48 * 0x100;
        File_Seek(rkvFd, fileLength - (fileSize + 8), 0);
        pFileEntries = (RkvFileEntry *)Heap_MemAlloc(fileSize);

        unk50 = (int)pFileEntries + nmbrOfEntries * sizeof(RkvFileEntry);
        File_Read(rkvFd, (void *)pFileEntries, fileSize, fileSize);

        int entryIndex = 0;
        while (entryIndex < nmbrOfEntries) {
            ByteReverse<int>(pFileEntries[entryIndex].unk20);
            ByteReverse<int>(pFileEntries[entryIndex].length);
            ByteReverse<int>(pFileEntries[entryIndex].unk28);
            ByteReverse<int>(pFileEntries[entryIndex].unk2C);
            ByteReverse<uint>(pFileEntries[entryIndex].unk30);
            ByteReverse<uint>(pFileEntries[entryIndex].unk34);
            pFileEntries[entryIndex].unk38 = 0;
            pFileEntries[entryIndex].unk3C = 0;
            pFileEntries[entryIndex].unk3E = 0;
            entryIndex++;
        }
    } else {
        rkvFd = -1;
        nmbrOfEntries = 0;
        unk48 = 0;
        pFileEntries = 0;
        unk50 = 0;
    }
}

void* FileSys_SetLoadInterceptHandler(void* loadInterceptFunc(char*, int*, void*, int*)) {
    void* oldHandler = pLoadInterceptHandler;
    pLoadInterceptHandler = loadInterceptFunc;
    return oldHandler;
}

bool FileSys_Exists(char *arg0, int *arg1) {
    if (pExistInterceptHandler != 0) {
        if (pExistInterceptHandler(arg0, arg1, 0) != false) {
            return true;
        }
    }

    RkvFileEntry *pEntry = 0;
    if (patch.rkvFd >= 0) {
        pEntry = (RkvFileEntry *)Util_BinarySearch((void *)arg0, (void *)patch.pFileEntries, patch.nmbrOfEntries, 0x40, EntryCompare);
        if (pEntry != 0 && pEntry->unk2C < 0) {
            pEntry = 0;
        }
    }
    if (pEntry == 0) {
        pEntry = 0;
        if (data.rkvFd >= 0) {
            pEntry = (RkvFileEntry *)Util_BinarySearch((void *)arg0, (void *)data.pFileEntries, data.nmbrOfEntries, 0x40, EntryCompare);
            if (pEntry != 0 && pEntry->unk2C < 0) {
                pEntry = 0;
            }
        }
    }
    if (pEntry != 0 && arg1 != 0) {
        *arg1 = pEntry->length;
    }
    return !!pEntry;
}

static void FileSys_SetOrder(RkvFileEntry *pEntry) {
    if (pEntry->unk3C != 0) {
        return;
    }
    s16 orderId = fileOrderId;
    fileOrderId = orderId + 1;
    pEntry->unk3C = orderId;
    if (System_GetCommandLineParameter("-languageOrder") == 0) {
        return;
    }
    char* namePtr;
    char *pLang = strstr(pEntry->name, ".ENGLISH");
    char* entryLanguage = pLang;
    if (pLang != 0) {
        namePtr = (char *)((int)entryLanguage - (int)pEntry);
        s16 index = 0;
        RkvTOC *toc = &data;
        while (index < 4) {
            switch (index) {
            case 0:
                entryLanguage = Str_Printf("%.*s.FRENCH", namePtr, pEntry);
                break;
            case 1:
                entryLanguage = Str_Printf("%.*s.SPANISH", namePtr, pEntry);
                break;
            case 2:
                entryLanguage = Str_Printf("%.*s.GERMAN", namePtr, pEntry);
                break;
            case 3:
                entryLanguage = Str_Printf("%.*s.ITALIAN", namePtr, pEntry);
                break;
            default:
                break;
            }
            RkvFileEntry *pFoundEntry = 0;
            if (toc->rkvFd >= 0) {
                pFoundEntry = (RkvFileEntry *)Util_BinarySearch(entryLanguage, (void *)toc->pFileEntries, toc->nmbrOfEntries, 0x40, EntryCompare);
                if (pFoundEntry != 0 && pFoundEntry->unk2C < 0) {
                    pFoundEntry = 0;
                }
            }

            if (pFoundEntry->unk3C == 0) {
                pEntry->unk3E = 1;
                s16 localOrderId = fileOrderId;
                fileOrderId = localOrderId + 1;
                pFoundEntry->unk3C = localOrderId;
                pFoundEntry->unk3E = index + 2;
            }
            index++;
        }
    }
    pLang = strstr((char *)pEntry, "EN.");
    entryLanguage = pLang;
    namePtr = entryLanguage + 3;
    if (pLang != 0) {
        char *languageStringPtr = (char *)((int)entryLanguage - (int)pEntry);
        s16 index = 0;
        RkvTOC *toc = &data;
        while (index < 4) {
            switch (index) {
            case 0:
                entryLanguage = Str_Printf("%.*sFR.%s", languageStringPtr, pEntry, namePtr);
                break;
            case 1:
                entryLanguage = Str_Printf("%.*sES.%s", languageStringPtr, pEntry, namePtr);
                break;
            case 2:
                entryLanguage = Str_Printf("%.*sDE.%s", languageStringPtr, pEntry, namePtr);
                break;
            case 3:
                entryLanguage = Str_Printf("%.*sIT.%s", languageStringPtr, pEntry, namePtr);
                break;
            default:
                break;
            }
            RkvFileEntry *pFoundEntry = 0;
            if (toc->rkvFd >= 0) {
                pFoundEntry = (RkvFileEntry *)Util_BinarySearch(entryLanguage, (void *)toc->pFileEntries, toc->nmbrOfEntries, 0x40, EntryCompare);
                if (pFoundEntry != 0 && pFoundEntry->unk2C < 0) {
                    pFoundEntry = 0;
                }
            }

            if (pFoundEntry->unk3C == 0) {
                pEntry->unk3E = 1;
                s16 localOrderId = fileOrderId;
                fileOrderId = localOrderId + 1;
                pFoundEntry->unk3C = localOrderId;
                pFoundEntry->unk3E = index + 2;
            }
            index++;
        }
    }
}

void *FileSys_Load(char *pFilename, int *arg1, void *pMemoryAllocated, int spaceAllocated) {
    int foundFd = -1;
    RkvFileEntry *pEntry = 0;
    RkvFileEntry *pFoundEntry;
    if (patch.rkvFd >= 0) {
        pEntry = (RkvFileEntry *)Util_BinarySearch((void *)pFilename, (void *)patch.pFileEntries, patch.nmbrOfEntries, 0x40, EntryCompare);
        if (pEntry != 0 && (int)pEntry->unk2C < 0) {
            pEntry = 0;
        }
    }
    pFoundEntry = pEntry;
    if (pEntry) {
        foundFd = patch.rkvFd;
    } else {
        pEntry = 0;
        if (data.rkvFd >= 0) {
            pEntry = (RkvFileEntry *)Util_BinarySearch((void *)pFilename, (void *)data.pFileEntries, data.nmbrOfEntries, 0x40, EntryCompare);
            if (pEntry != 0 && (int)pEntry->unk2C < 0) {
                pEntry = 0;
            }
        }
        pFoundEntry = pEntry;
        if (pEntry) {
            foundFd = data.rkvFd;
        }
    }
    if (pLoadInterceptHandler != 0) {
        int unkArg = 0;
        void* handlerRet = pLoadInterceptHandler(pFilename, arg1, pMemoryAllocated, &unkArg);
        if (handlerRet != 0) {
            return handlerRet;
        }
    }
    if (pFoundEntry != 0) {
        if (arg1 != 0) {
            *arg1 = pFoundEntry->length;
        }
        if (pMemoryAllocated == NULL) {
			// if memory isn't already allocated, allocate the memory
            spaceAllocated = pFoundEntry->length + 1;
            pMemoryAllocated = Heap_MemAlloc(spaceAllocated);
            *((char *)pMemoryAllocated + pFoundEntry->length) = 0;
        } else if (spaceAllocated < 0) {
            spaceAllocated = pFoundEntry->length;
        }
        if (pFoundEntry->length != 0) {
            File_Seek(foundFd, pFoundEntry->unk2C, 0);
            File_Read(foundFd, pMemoryAllocated, pFoundEntry->length, spaceAllocated);
        }
        FileSys_SetOrder(pFoundEntry);
        return pMemoryAllocated;
    }

    return 0;
}

int FileSys_Save(char* name, bool arg1, void* arg2, int arg3) {
    arg1 = (arg1 & 0xff);
    arg1 = !!arg1;
    int fd = arg1 + 2;
    fd = File_Open(File_FileServerOutputFilename(name), fd);
    if (fd >= 0) {
        File_Write(fd, arg2, arg3);
        File_Close(fd);
    }
    return fd >> 31;
}

static int FileOrderSortCompare(const void* arg0, const void* arg1) {
    RkvFileEntry* pEntry1 = *(RkvFileEntry**)arg0;
    RkvFileEntry* pEntry = *(RkvFileEntry**)arg1;
    return pEntry1->unk3C - pEntry->unk3C;
}

static int LanguageSortCompare(const void* arg0, const void* arg1) {
    RkvFileEntry* pEntry1 = *(RkvFileEntry**)arg0;
    RkvFileEntry* pEntry = *(RkvFileEntry**)arg1;
    if ((pEntry1->unk3E - pEntry->unk3E) != 0) {
        return pEntry1->unk3E - pEntry->unk3E;
    }
    return pEntry1->unk3C - pEntry->unk3C;
}

void FileSys_OutputFileOrder(void) {
    void* sortedEntries = Heap_MemAlloc(data.nmbrOfEntries << 2);
    char* stringBuf = (char*)Heap_MemAlloc(0x2200);
    int openFd;
    int bufferIndex;
    int index = 0;
    while(index < data.nmbrOfEntries) {
        // store all entries in the buffer
        *((char**)sortedEntries + index) = (char*)&data.pFileEntries[index];
        index++;
    }

    // sort by order
    qsort(sortedEntries, data.nmbrOfEntries, 4, FileOrderSortCompare);

    index = 0;
    while(index < data.nmbrOfEntries) {
        RkvFileEntry** ppEntries = (RkvFileEntry**)((int)sortedEntries + (index << 2));
        RkvFileEntry* pEntry = *ppEntries;
        if (pEntry->unk3E != 0) {
            int c = 1;
            // fix this up?
            while(index + c < data.nmbrOfEntries && (*(RkvFileEntry**)((int)sortedEntries + ((index + c) << 2)))->unk3E != 0) {
                c++;
            }
            if (c > 1) {
                // only sort if there are 2 or more entries
                // sort by language?
                qsort(ppEntries, c, 4, LanguageSortCompare);
            }
            index += c;
        } else {
            index++;
        }
    }

    // output order to order.txt
    openFd = File_Open(File_FileServerOutputFilename("order.txt"), 2);
    if (openFd >= 0) { // check if fd is valid
        index = 0;
        *stringBuf = 0;
        bufferIndex = 0;
        RkvFileEntry* sorted = (RkvFileEntry*)sortedEntries;
        while(index < data.nmbrOfEntries) {
            RkvFileEntry* pCurrEntry = *(RkvFileEntry**)((int)sorted + (index << 2));
            if (pCurrEntry->unk2C != -1) {
                strcat(stringBuf, Str_Printf("%s\r\n", pCurrEntry->name));
                bufferIndex++;
            }
            if (bufferIndex >= 0x100) {
                File_Write(openFd, stringBuf, strlen(stringBuf));
                *stringBuf = 0;
                bufferIndex = 0;
            }
            index++;
        }
        if (bufferIndex != 0) {
            // write file if string size is not 0
            File_Write(openFd, stringBuf, strlen(stringBuf));
        }
        // close file
        File_Close(openFd);
    }
    // free allocated memory for entries and string buffer
    Heap_MemFree(stringBuf);
    Heap_MemFree(sortedEntries);
}

int FileSys_Open(char *pFilename, int *arg1, bool arg2) {
    int foundFd = -1;
    RkvFileEntry *pEntry = 0;
    RkvFileEntry *pFoundEntry = 0;
    if (patch.rkvFd >= 0) {
        pEntry = (RkvFileEntry *)Util_BinarySearch((void *)pFilename, (void *)patch.pFileEntries, patch.nmbrOfEntries, 0x40, EntryCompare);
        // fix the struct access
        if (pEntry != 0 && pEntry->unk2C < 0) {
            pEntry = 0;
        }
    }
    pFoundEntry = pEntry;
    if (pFoundEntry != 0) {
        if (arg2 & 0xff) {
            if (patch.unk58 == false) {
                patch.unk54 = File_Open(patch.name, 1);
            }
            patch.unk58 = false;
            foundFd = patch.unk54;
        } else {
            foundFd = File_Open(patch.name, 0);
        }
    } else {
        pEntry = 0;
        if (data.rkvFd >= 0) {
            pEntry = (RkvFileEntry *)Util_BinarySearch((void *)pFilename, (void *)data.pFileEntries, data.nmbrOfEntries, 0x40, EntryCompare);
            if (pEntry != 0 && pEntry->unk2C < 0) {
                pEntry = 0;
            }
        }
        pFoundEntry = pEntry;
        if (pEntry != 0) {
            FileSys_SetOrder(pFoundEntry);
            if (arg2 & 0xff) {
                if (data.unk58 == false) {
                    data.unk54 = File_Open(data.name, 1);
                }
                data.unk58 = false;
                foundFd = data.unk54;
            } else {
                foundFd = File_Open(data.name, 0);
            }
        }
    }
    if (foundFd != -1) {
        if (arg1 != 0) {
            *arg1 = pFoundEntry->length;
        }
        File_Seek(foundFd, pFoundEntry->unk2C, 0);
    }
    return foundFd;
}

void FileSys_Close(int arg0) {
    if (arg0 >= 0) {
        if (data.unk54 == arg0) {
            data.unk58 = true;
            return;
        } else {
            if (patch.unk54 == arg0) {
                patch.unk58 = true;
                return;
            }
        }
        File_Close(arg0);
    }
}

int FileSys_GetOffset(char* pFilename) {
    int offset;
    RkvFileEntry* pEntry = 0;
    RkvFileEntry* pFoundEntry;
    if (patch.rkvFd >= 0) {
        pEntry = (RkvFileEntry *)Util_BinarySearch((void *)pFilename, (void *)patch.pFileEntries, patch.nmbrOfEntries, 0x40, EntryCompare);
        if (pEntry != 0 && pEntry->unk2C < 0) {
            pEntry = 0;
        }
    }
    if (pEntry != 0) {
        pFoundEntry = 0;
    } else {
        pEntry = 0;
        if (data.rkvFd >= 0) {
            pEntry = (RkvFileEntry *)Util_BinarySearch((void *)pFilename, (void *)data.pFileEntries, data.nmbrOfEntries, 0x40, EntryCompare);
            if (pEntry != 0 && pEntry->unk2C < 0) {
                pEntry = 0;
            }
        }
        pFoundEntry = pEntry;
    }
    if (pFoundEntry != 0) {
        return pFoundEntry->unk2C;
    }
    return -1;
}