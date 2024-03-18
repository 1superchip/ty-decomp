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

static LoadInterceptFunc pLoadInterceptHandler;
static ExistInterceptFunc pExistInterceptHandler;
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

static int EntryCompare(void* pEntry0, void* pEntry1) {
	return stricmp(((RkvFileEntry*)pEntry0)->name, ((RkvFileEntry*)pEntry1)->name);
}

void RkvTOC::Init(char* pName) {
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
        RKVHeader* pHeader = (RKVHeader*)Heap_MemAlloc(sizeof(RKVHeader));
        File_Seek(rkvFd, -0x20, SEEK_END); // Header is last 32 bytes of the rkv file
        File_Read(rkvFd, pHeader, sizeof(RKVHeader), 0x20);
        nmbrOfEntries = pHeader->nmbrOfEntries;
        nmbrOfDirectories = pHeader->nmbrOfDirectories;

        // Flip endian of fields
        ByteReverse<int>(nmbrOfEntries);
        ByteReverse<int>(nmbrOfDirectories);

        // Free header
        Heap_MemFree(pHeader);

		int fileSize = nmbrOfEntries * sizeof(RkvFileEntry) + nmbrOfDirectories * sizeof(DirectoryEntry);
        File_Seek(rkvFd, fileLength - (fileSize + 8), SEEK_SET);
        pFileEntries = (RkvFileEntry *)Heap_MemAlloc(fileSize);

        // directory entries are right after the file entries
        pDirectoryEntries = (DirectoryEntry*)((char*)pFileEntries + nmbrOfEntries * sizeof(RkvFileEntry));
        File_Read(rkvFd, (void*)pFileEntries, fileSize, fileSize);

        // Correct endianness of all fields in all file entries
        for (int entryIndex = 0; entryIndex < nmbrOfEntries; entryIndex++) {
            ByteReverse<int>(pFileEntries[entryIndex].directoryIndex);
            ByteReverse<int>(pFileEntries[entryIndex].length);
            ByteReverse<int>(pFileEntries[entryIndex].unk28);
            ByteReverse<int>(pFileEntries[entryIndex].offset);
            ByteReverse<uint>(pFileEntries[entryIndex].crc);
            ByteReverse<uint>(pFileEntries[entryIndex].timestamp);
            pFileEntries[entryIndex].unk38 = 0;
            pFileEntries[entryIndex].unk3C = 0;
            pFileEntries[entryIndex].unk3E = 0;
        }
    } else {
        rkvFd = -1;
        nmbrOfEntries = 0;
        nmbrOfDirectories = 0;
        pFileEntries = NULL;
        pDirectoryEntries = NULL;
    }
}

RkvFileEntry* RkvTOC::GetEntry(char* pFileName) {
    RkvFileEntry* pEntry = NULL;
    if (rkvFd >= 0) {
        pEntry = (RkvFileEntry*)Util_BinarySearch(
            (void*)pFileName, (void*)pFileEntries, 
            nmbrOfEntries, sizeof(RkvFileEntry), 
            EntryCompare
        );
        if (pEntry != NULL && pEntry->offset < 0) {
            pEntry = NULL;
        }
    }
    return pEntry;
}

int RkvTOC::GetAsyncHandle(void) {
	if (unk58 == false) {
		unk54 = File_Open(name, 1);
	}
	unk58 = false;
	return unk54;
}

/// @brief Sets the new Load Intercept handler
/// @param newLoadHandler New Intercept handler function
/// @return Old intercept handler function
LoadInterceptFunc FileSys_SetLoadInterceptHandler(LoadInterceptFunc newLoadHandler) {
    LoadInterceptFunc oldHandler = pLoadInterceptHandler;
    pLoadInterceptHandler = newLoadHandler;
    return oldHandler;
}

/// @brief Checks if a file exists in either rkv or not
/// @param pFilename Name of the file
/// @param pOutLen Optional pointer to store the file length (NULL if not needed)
/// @return True if the entry is found, otherwise false
bool FileSys_Exists(char* pFilename, int* pOutLen) {

    if (pExistInterceptHandler != NULL) {
        // check the exists intercept handler if it isn't NULL
        if (pExistInterceptHandler(pFilename, pOutLen, 0)) {
            return true;
        }
    }

    // first check the patch rkv
    RkvFileEntry* pEntry = patch.GetEntry(pFilename);
    if (pEntry == NULL) {
        // if the patch rkv doesn't contain the file, load it from the regular rkv
        pEntry = data.GetEntry(pFilename);
    }

    if (pEntry != NULL && pOutLen != NULL) {
        // if the entry isn't null and pOutLen isn't NULL
        // store the file's length to pOutLen
        *pOutLen = pEntry->length;
    }

    return pEntry != NULL;
}

static void FileSys_SetOrder(RkvFileEntry* pEntry) {
    if (pEntry->unk3C != 0) {
        return;
    }
    
    pEntry->unk3C = fileOrderId++;

    if (System_GetCommandLineParameter("-languageOrder") == 0) {
        return;
    }

    char* pLang = strstr(pEntry->name, ".ENGLISH");
    char* entryLanguage = pLang;
    char* namePtr;

    if (pLang != NULL) {
		// if ".ENGLISH" is found in filename, run this block
        int nameLen = entryLanguage - pEntry->name; // width specifier
        for (s16 i = 0; i < 4; i++) {
            switch (i) {
                case 0:
                    entryLanguage = Str_Printf("%.*s.FRENCH", nameLen, pEntry->name);
                    break;
                case 1:
                    entryLanguage = Str_Printf("%.*s.SPANISH", nameLen, pEntry->name);
                    break;
                case 2:
                    entryLanguage = Str_Printf("%.*s.GERMAN", nameLen, pEntry->name);
                    break;
                case 3:
                    entryLanguage = Str_Printf("%.*s.ITALIAN", nameLen, pEntry->name);
                    break;
            }
            RkvFileEntry* pFoundEntry = data.GetEntry(entryLanguage);
            if (pFoundEntry->unk3C == 0) {
                pEntry->unk3E = 1;
                pFoundEntry->unk3C = fileOrderId++;
                pFoundEntry->unk3E = i + 2;
            }
        }
    }

    pLang = strstr(pEntry->name, "EN.");
    entryLanguage = pLang;
    namePtr = entryLanguage + (sizeof("EN.") - 1);

    if (pLang != NULL) {
        int nameLen = entryLanguage - pEntry->name;
        for (s16 i = 0; i < 4; i++) {
            switch (i) {
                case 0:
                    entryLanguage = Str_Printf("%.*sFR.%s", nameLen, pEntry, namePtr);
                    break;
                case 1:
                    entryLanguage = Str_Printf("%.*sES.%s", nameLen, pEntry, namePtr);
                    break;
                case 2:
                    entryLanguage = Str_Printf("%.*sDE.%s", nameLen, pEntry, namePtr);
                    break;
                case 3:
                    entryLanguage = Str_Printf("%.*sIT.%s", nameLen, pEntry, namePtr);
                    break;
            }
            RkvFileEntry *pFoundEntry = data.GetEntry(entryLanguage);
            if (pFoundEntry->unk3C == 0) {
                pEntry->unk3E = 1;
                pFoundEntry->unk3C = fileOrderId++;
                pFoundEntry->unk3E = i + 2;
            }
        }
    }
}

/// @brief Loads and returns the filedata of pFilename
/// @param pFilename Name of file to load and read
/// @param pOutLen Optional pointer to store file length to
/// @param pMemoryAllocated Optional pointer of already allocated memory. If NULL, memory will be allocated
/// @param spaceAllocated Optional parameter for space allocated. If less than 0, size in file entry is used
/// @return Pointer to file in memory, NULL if error
void* FileSys_Load(char* pFilename, int* pOutLen, void* pMemoryAllocated, int spaceAllocated) {
    int foundFd = -1;

    RkvFileEntry *pFoundEntry = patch.GetEntry(pFilename);
    if (pFoundEntry) {
        foundFd = patch.rkvFd;
    } else {
        pFoundEntry = data.GetEntry(pFilename);
        if (pFoundEntry) {
            foundFd = data.rkvFd;
        }
    }

    // Check if there is a Load Intercept Handler
    if (pLoadInterceptHandler != NULL) {
        int unkArg = 0;
        void* pFileData = pLoadInterceptHandler(pFilename, pOutLen, pMemoryAllocated, &unkArg);
        if (pFileData != NULL) {
            return pFileData;
        }
    }

    if (pFoundEntry != NULL) {
        if (pOutLen != NULL) {
            // If pOutLen isn't NULL, store the length of the file to it
            *pOutLen = pFoundEntry->length;
        }

        if (pMemoryAllocated == NULL) {
			// if memory isn't already allocated, allocate the memory
            spaceAllocated = pFoundEntry->length + 1;
            pMemoryAllocated = Heap_MemAlloc(spaceAllocated);
            *((char *)pMemoryAllocated + pFoundEntry->length) = 0;
        } else if (spaceAllocated < 0) {
            // If spaceAllocated is less than 0, use the length in the file entry
            spaceAllocated = pFoundEntry->length;
        }

        if (pFoundEntry->length != 0) {
            File_Seek(foundFd, pFoundEntry->offset, SEEK_SET); // seek to beginning of file data
            File_Read(foundFd, pMemoryAllocated, pFoundEntry->length, spaceAllocated);
        }

        FileSys_SetOrder(pFoundEntry);
        return pMemoryAllocated;
    }

    return NULL;
}

int FileSys_Save(char* name, bool arg1, void* pData, int dataLen) {
    int fd = File_Open(File_FileServerOutputFilename(name), arg1 ? 3 : 2);
    if (fd >= 0) {
        File_Write(fd, pData, dataLen);
        File_Close(fd);
    }
    return fd < 0 ? -1 : 0;
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
    RkvFileEntry** sortedEntries = (RkvFileEntry**)Heap_MemAlloc(data.nmbrOfEntries * sizeof(RkvFileEntry*));
    char* stringBuf = (char*)Heap_MemAlloc(0x2200);
    int openFd;
    int bufferIndex;
    int index = 0;
    for(; index < data.nmbrOfEntries; index++) {
        // store all entries in the buffer
        sortedEntries[index] = &data.pFileEntries[index];
    }

    // sort by order
    qsort(sortedEntries, data.nmbrOfEntries, 4, FileOrderSortCompare);
	
    index = 0;
    while(index < data.nmbrOfEntries) {
        if (sortedEntries[index]->unk3E != 0) {
            int c = 1;
            // fix this up?
            while(index + c < data.nmbrOfEntries && sortedEntries[index + c]->unk3E != 0) {
                c++;
            }
            if (c > 1) {
                // only sort if there are 2 or more entries
                // sort by language?
                qsort(&sortedEntries[index], c, 4, LanguageSortCompare);
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
        while(index < data.nmbrOfEntries) {
            RkvFileEntry* pCurrEntry = sortedEntries[index];
            if (pCurrEntry->offset != -1) {
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

/// @brief Opens a file within an RKV
/// @param pFilename Filename
/// @param pOutLen Optional pointer to place file length
/// @param bUseAsyncHandle Use the async handle
/// @return Fd of file, possibly -1
int FileSys_Open(char* pFilename, int* pOutLen, bool bUseAsyncHandle) {
    int foundFd = -1;
    RkvFileEntry* pFoundEntry = patch.GetEntry(pFilename);
    if (pFoundEntry != NULL) {
        if (bUseAsyncHandle) {
			foundFd = patch.GetAsyncHandle();
        } else {
            foundFd = File_Open(patch.name, 0);
        }
    } else {
        pFoundEntry = data.GetEntry(pFilename);
        if (pFoundEntry != NULL) {
            FileSys_SetOrder(pFoundEntry);
            if (bUseAsyncHandle) {
				foundFd = data.GetAsyncHandle();
            } else {
                foundFd = File_Open(data.name, 0);
            }
        }
    }
    if (foundFd != -1) {
        if (pOutLen != NULL) {
            *pOutLen = pFoundEntry->length;
        }
        File_Seek(foundFd, pFoundEntry->offset, SEEK_SET);
    }
    return foundFd;
}

void FileSys_Close(int fd) {
    if (fd >= 0) {
        if (data.unk54 == fd) {
            data.unk58 = true;
            return;
        } else if (patch.unk54 == fd) {
            patch.unk58 = true;
            return;
        }
        File_Close(fd);
    }
}

/// @brief Returns the offset into the RKV of a file
/// @param pFilename Name of file
/// @return Offset of file inside the RKV, can return -1
int FileSys_GetOffset(char* pFilename) {
    int offset;
    RkvFileEntry* pFoundEntry = patch.GetEntry(pFilename);
    if (pFoundEntry != NULL) {
        pFoundEntry = NULL;
    } else {
        pFoundEntry = data.GetEntry(pFilename);
    }
    if (pFoundEntry != NULL) {
        return pFoundEntry->offset;
    }
    return -1;
}
