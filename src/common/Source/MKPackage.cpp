#include "common/MKPackage.h"
#include "common/System_GC.h"
#include "common/Str.h"
#include "common/Timer.h"
#include "common/FileSys.h"
#include "common/File.h"
#include "common/Heap.h"
#include "common/Utils.h"

static char packageName[0x20];
static PackageEntry pPackageEntries[1024];
static TimerInfo packageTime;

static bool packageLoaded = false;

static int nmbrOfPackageEntries = 0;
static int nmbrOfAutoEntries = 0;
static int nmbrEntriesLoaded = 0;

static int totalBytesInPackage = 0;
static int totalBytesLoaded = 0;

static char* pPackageText = NULL;

static int asyncState = 0;

static AutoEntry* pAutoEntries = NULL;

static LoadInterceptFunc pPreviousHandler = NULL;

int MKPackage_kLoadedPerSecond = 0;

/// @brief Variable to override async file loading
static bool disableAsyncPackages = false;

static bool disablePackageChunking = false;

static char* pPackageExclusionPatterns = "";

static int afd = -1;

extern "C" void strcpy(char*, char*);
extern "C" int strcmp(char*, const char*);
extern "C" int strcmpi(char*, char*);
extern "C" void* memcpy(void*, void*, size_t);
extern "C" void strcat(char*, char*);
extern "C" int strlen(char*);
extern "C" char* strtok(const char*, const char*);
extern "C" void qsort(void*, int, int, int (*compar)(const void*, const void*));
extern "C" void strncpy(char*, char*, int);

/// @brief MKPackage Load Intercepter
/// @param pFilename Filename to load
/// @param pOutLen Optional parameter to store file length to
/// @param pMemoryAllocated Optional parameter of already allocated memory
/// @param arg4 
/// @return Pointer to loaded filedata
static void* PackageIntercept(char* pFilename, int* pOutLen, void* pMemoryAllocated, int* arg4) {
    if (packageLoaded) {
        if (pAutoEntries && pPackageExclusionPatterns 
                && !Util_WildcardPatternMatch(pFilename, pPackageExclusionPatterns)) {
            strcpy(pAutoEntries[nmbrOfAutoEntries++].name, pFilename);
        }

        for (int i = 0; i < nmbrOfPackageEntries; i++) {
            if (strcmpi(pFilename, pPackageEntries[i].pFilename) == 0) {
                if (pPackageEntries[i].pFileData == NULL) {
                    return NULL;
                }
                
                if (pMemoryAllocated) {
                    // If memory for the filedata has been allocated
                    // Copy the data to it and free the allocated memory in the package entry
                    memcpy(pMemoryAllocated, pPackageEntries[i].pFileData, pPackageEntries[i].fileSize);
                    Heap_MemFree(pPackageEntries[i].pFileData);
                } else {
                    // if memory has not been allocated, set the pointer to the package data
                    pMemoryAllocated = pPackageEntries[i].pFileData;
                }
                
                pPackageEntries[i].pFileData = NULL;
                
                if (pOutLen) {
                    // Store the file length if pOutLen isn't NULL
                    *pOutLen = pPackageEntries[i].fileSize;
                }

                // Status?
                if (arg4) {
                    *arg4 = 2;
                }

                return pMemoryAllocated;
            }
        }
        if (totalBytesInPackage != 0) {
            strcmp(packageName, "<Dynamic>");
        }
    }

    if (pPreviousHandler) {
        return pPreviousHandler(pFilename, pOutLen, pMemoryAllocated, arg4);
    }

    return NULL;
}

void AddToPackage(char* pName) {
    int i = 0;
    
    for (i; i < nmbrOfPackageEntries; i++) {
        if (strcmpi(pPackageEntries[i].pFilename, pName) == 0) {
            break;
        }
    }

    if (i == nmbrOfPackageEntries && FileSys_Exists(pName, &pPackageEntries[nmbrOfPackageEntries].fileSize)) {
        pPackageEntries[nmbrOfPackageEntries].pFilename = pName;
        pPackageEntries[nmbrOfPackageEntries].fileOffset = FileSys_GetOffset(pName);
        pPackageEntries[nmbrOfPackageEntries].pFileData = NULL;
        totalBytesInPackage += pPackageEntries[nmbrOfPackageEntries].fileSize;
        nmbrOfPackageEntries++;
    }
}

void ProcessPackageFilenames(void) {
    char* tok = strtok(pPackageText, "\r\n");
    
    while (tok) {
        while (*tok == ' ' || *tok == '\t') {
            tok++;
        }
        int len = strlen(tok);

        while (len != 0 && tok[len - 1] == ' ') {
            tok[--len] = '\0';
        }

        if (*tok != '\0') {
            AddToPackage(tok);
        }

        tok = strtok(NULL, "\r\n");
    }
}

void MKPackage_CalculateLoadSpeed(void) {
    if (nmbrEntriesLoaded > 1) {
        TimerInfo finishTime;
        TimerInfo elapsed;

        Timer_GetSystemTime(&finishTime);
        Timer_GetDifference(&elapsed, &packageTime, &finishTime);
        int differenceInSecs = Timer_GetDHMSInSeconds(&elapsed);

        int totalKbsLoaded = totalBytesLoaded / 1024;

        if (differenceInSecs <= 0 || differenceInSecs > 60) {
            MKPackage_kLoadedPerSecond = totalKbsLoaded;
        } else {
            MKPackage_kLoadedPerSecond = totalKbsLoaded / differenceInSecs;
        }
    } else {
        MKPackage_kLoadedPerSecond = 0;
    }
}

void MKPackage_InitModule(void) {
    // Set new load intercept handler
    pPreviousHandler = FileSys_SetLoadInterceptHandler(&PackageIntercept);

    disablePackageChunking = System_GetCommandLineParameter("-outputOrder") ||
        System_GetCommandLineParameter("-outputPackages");
}


void MKPackage_DeinitModule(void) {
    FileSys_SetLoadInterceptHandler(pPreviousHandler);
    
    pPreviousHandler = NULL;
}

extern "C" void* malloc(int);
extern "C" void free(void*);

void MKPackage_Update(void) {
    static int nFilesInChunk = 0;
    static int chunkLength = 0;
    static int chunkBaseOffset = 0;
    static char* pChunkBuffer;
    static bool lowMemory = 0;

    int i;

    while (asyncState != 0 && !File_IsAnyBusy()) {
        switch (asyncState) {
            case 1:
                afd = FileSys_Open(pPackageEntries[nmbrEntriesLoaded].pFilename, NULL, true);
                asyncState = 2;
                if (!disablePackageChunking && !lowMemory) {
                    chunkLength = 0;
                    nFilesInChunk = 0;
                    chunkBaseOffset = pPackageEntries[nmbrEntriesLoaded].fileOffset;

                    for (i = 0; i + nmbrEntriesLoaded < nmbrOfPackageEntries
                         && pPackageEntries[i + nmbrEntriesLoaded].fileOffset > 0;
                         i++)
                    {
                        uint r12 = (pPackageEntries[i + nmbrEntriesLoaded].fileOffset +
                            pPackageEntries[i + nmbrEntriesLoaded].fileSize) - chunkBaseOffset;
                            
                        if (r12 > 0x00040000) {
                            break;
                        }
                        if (nFilesInChunk != 0 &&
                            pPackageEntries[i + nmbrEntriesLoaded].fileOffset - pPackageEntries[i + nmbrEntriesLoaded - 1].fileOffset > 0x28000) {
                            break;
                        }
                        chunkLength = r12;
                        nFilesInChunk++;
                    }

                    if (nFilesInChunk > 1) {
                        asyncState = 4;
                    }
                }
                break;
            case 2:
                pPackageEntries[nmbrEntriesLoaded].pFileData =
                    Heap_MemAlloc(pPackageEntries[nmbrEntriesLoaded].fileSize + 1);

                ((char*)pPackageEntries[nmbrEntriesLoaded].pFileData)[pPackageEntries[nmbrEntriesLoaded].fileSize] = '\0';

                File_Read(
                    afd, pPackageEntries[nmbrEntriesLoaded].pFileData, pPackageEntries[nmbrEntriesLoaded].fileSize,
                    pPackageEntries[nmbrEntriesLoaded].fileSize
                );
                asyncState = 3;
                break;
            case 3:
                File_Sync(afd, -1);
                FileSys_Close(afd);
                afd = -1;

                totalBytesLoaded += pPackageEntries[nmbrEntriesLoaded].fileSize;

                if (++nmbrEntriesLoaded < nmbrOfPackageEntries) {
                    asyncState = 1;
                } else {
                    asyncState = 0;
                    lowMemory = false;
                }

                MKPackage_CalculateLoadSpeed();
                break;
            case 4:
                for (int i = 0; i < nFilesInChunk; i++) {
                    pPackageEntries[nmbrEntriesLoaded + i].pFileData =
                        Heap_MemAlloc(pPackageEntries[nmbrEntriesLoaded + i].fileSize + 1);

                    ((char*)pPackageEntries[nmbrEntriesLoaded + i].pFileData)[pPackageEntries[nmbrEntriesLoaded + i].fileSize] = '\0';
                }
                pChunkBuffer = (char*)malloc(chunkLength + 0x2000);
                if (pChunkBuffer == NULL) {
                    lowMemory = true;
                    for (i = 0; i < nFilesInChunk; i++) {
                        Heap_MemFree(pPackageEntries[nmbrEntriesLoaded + i].pFileData);
                        pPackageEntries[nmbrEntriesLoaded + i].pFileData = NULL;
                    }
                    asyncState = 2;
                } else {
                    File_Read(afd, pChunkBuffer, chunkLength, chunkLength + 0x2000);
                    asyncState = 5;
                }
                break;
            case 5:
                while (nFilesInChunk--) {
                    memcpy(
                        pPackageEntries[nmbrEntriesLoaded].pFileData,
                        &pChunkBuffer[pPackageEntries[nmbrEntriesLoaded].fileOffset - chunkBaseOffset],
                        pPackageEntries[nmbrEntriesLoaded].fileSize
                    );
                    totalBytesLoaded += pPackageEntries[nmbrEntriesLoaded].fileSize;
                    nmbrEntriesLoaded++;
                }

                free(pChunkBuffer);
                File_Sync(afd, -1);
                FileSys_Close(afd);
                afd = -1;

                if (nmbrEntriesLoaded < nmbrOfPackageEntries) {
                    asyncState = 1;
                } else {
                    asyncState = 0;
                    lowMemory = false;
                }

                MKPackage_CalculateLoadSpeed();
                break;
        }
    }
}

static int PackageSort(const void* p0, const void* p1) {
    return ((PackageEntry*)p0)->fileOffset - ((PackageEntry*)p1)->fileOffset;
}

/// @brief 
/// @param bLoadFilesAsync 
void MKPackage_LoadInternal(bool bLoadFilesAsync) {

    MKPackage_kLoadedPerSecond = 0;
    
    Timer_GetSystemTime(&packageTime);

    if (disableAsyncPackages) {
        // Disable loading the files asynchronously
        bLoadFilesAsync = false;
    }

    nmbrOfPackageEntries = 0;
    nmbrOfAutoEntries = 0;
    nmbrEntriesLoaded = 0;
    totalBytesInPackage = 0;
    totalBytesLoaded = 0;

    if (pPackageText) {
        ProcessPackageFilenames();
    }

    if (nmbrOfPackageEntries) {
        if (!System_GetCommandLineParameter("-OutputPackages") &&
                !System_GetCommandLineParameter("-OutputOrder")) {
            qsort(pPackageEntries, nmbrOfPackageEntries, sizeof(PackageEntry), PackageSort);
        }

        if (bLoadFilesAsync) {
            // if the files should be loaded asynchronously, set asyncState to 1
            asyncState = 1;
        } else {
            // Otherwises load the files now
            for (int i = 0; i < nmbrOfPackageEntries; i++) {
                pPackageEntries[i].pFileData = FileSys_Load(pPackageEntries[i].pFilename, NULL, NULL, -1);
                totalBytesLoaded += pPackageEntries[i].fileSize;
                nmbrEntriesLoaded++;
            }
            
            MKPackage_CalculateLoadSpeed();
        }
    }

    packageLoaded = true;
}

void MKPackage_Load(char* pPackageName, bool bLoadFilesAsync) {
    pPackageText = NULL;

    strncpy(packageName, Str_Printf("%s.gpk", pPackageName), sizeof(packageName));

    int packageSize;
    if (FileSys_Exists(packageName, &packageSize)) {
        pPackageText = (char*)Heap_MemAlloc(packageSize + 1);

        FileSys_Load(packageName, &packageSize, pPackageText, -1);

        pPackageText[packageSize] = '\0';

        if (System_GetCommandLineParameter("-OutputPackages")) {
            Heap_MemFree(pPackageText);
            pPackageText = NULL;
        }
    }

    MKPackage_LoadInternal(bLoadFilesAsync);

    if (System_GetCommandLineParameter("-OutputPackages")) {
        pAutoEntries = (AutoEntry*)Heap_MemAlloc(sizeof(AutoEntry) * 1024);
    }
}

void MKPackage_LoadDynamic(char* pFilename, bool bLoadFilesAsync) {
    strcpy(packageName, "<Dynamic>");
    
    pPackageText = (char*)Heap_MemAlloc(strlen(pFilename) + 1);
    strcpy(pPackageText, pFilename);

    MKPackage_LoadInternal(bLoadFilesAsync);
}

/// @brief 
/// @param pProgress Optional parameter to store the load progress of the package
/// @return 
bool MKPackage_IsLoaded(float* pProgress) {
    if (packageLoaded) {
        if (nmbrEntriesLoaded < nmbrOfPackageEntries) {
            MKPackage_Update();
        }
        if (pProgress) {
            *pProgress = totalBytesInPackage != 0 ? (float)totalBytesLoaded / (float)totalBytesInPackage : 1.0f;
        }
        return nmbrEntriesLoaded == nmbrOfPackageEntries;
    }
    return true;
}

void MKPackage_Cancel(void) {

}

int MKPackage_Free(void) {
    int ret = 0;

    if (packageLoaded && !MKPackage_IsLoaded(NULL)) {
        while (!MKPackage_IsLoaded(NULL)) {
            MKPackage_Update();
        }
    }

    if (pAutoEntries) {
        int i = 0;

        if (nmbrOfPackageEntries == nmbrOfAutoEntries) {
            for (i = 0; i < nmbrOfPackageEntries; i++) {
                if (strcmpi(pPackageEntries[i].pFilename, pAutoEntries[i].name) != 0) {
                    break;
                }
            }
        }

        if (nmbrOfPackageEntries != nmbrOfAutoEntries || i != nmbrOfPackageEntries
            || System_GetCommandLineParameter("-OutputPackages"))
        {

            char* n = (char*)Heap_MemAlloc(nmbrOfAutoEntries * 0x24 + 1);

            n[0] = '\0';

            for (int i = 0; i < nmbrOfAutoEntries; i++) {
                strcat(n, pAutoEntries[i].name);
                strcat(n, "\r\n");
            }

            FileSys_Save(Str_Printf("%s", packageName), false, n, strlen(n) + 1);
            Heap_MemFree(n);
            ret = 1;
        }

        Heap_MemFree(pAutoEntries);
        pAutoEntries = NULL;
    }

    for (int i = 0; i < nmbrOfPackageEntries; i++) {
        if (pPackageEntries[i].pFileData) {
            Heap_MemFree(pPackageEntries[i].pFileData);
            pPackageEntries[i].pFileData = NULL;
        }
    }

    if (nmbrEntriesLoaded > 20 || nmbrOfAutoEntries > 20) {
        if (System_GetCommandLineParameter("-outputOrder")) {
            FileSys_OutputFileOrder();
        }
    }

    if (pPackageText) {
        Heap_MemFree(pPackageText);
    }

    pPackageText = NULL;
    nmbrOfPackageEntries = 0;
    nmbrOfAutoEntries = 0;
    nmbrEntriesLoaded = 0;
    totalBytesInPackage = 0;
    totalBytesLoaded = 0;
    packageLoaded = false;

    return ret;
}
