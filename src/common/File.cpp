#include "types.h"
#include "common/File.h"
#include "common/Heap.h"
#include "Dolphin/os/OSTime.h"

#define MAX_FILES 8

static FileEntry gcFiles[MAX_FILES];

extern "C" void memset(void*, char, int);
extern "C" void memmove(void*, void*, int);
extern "C" u32 OSGetTick(void);
extern "C" int DVDGetCommandBlockStatus(DVDFileInfo*);
extern "C" void OSYieldThread(void);
extern "C" void DCStoreRange(uint*, int);
volatile u32 BUS_SPEED : 0x800000f8;
extern "C" int DVDReadPrio(DVDFileInfo*, void*, int, int, int);
extern "C" BOOL DVDReadAsyncPrio(DVDFileInfo *, void *, int length, int offset, uint callback, int prio);
extern "C" int DVDClose(void*);
extern "C" int DVDOpen(char*, void*);
extern "C" void strcpy(char*, char*);

void File_InitModule(void) {
    int i = 0;
    while (i < MAX_FILES) {
        memset(&gcFiles[i], 0, sizeof(FileEntry));
        gcFiles[i].unk48 = -1;
        i++;
    }
}

int File_Open(char* filepath, int openMode) {
    int fd = 0;
    for (; fd < MAX_FILES; fd++) {
        if (gcFiles[fd].unk48 == -1) { break; }
    }
    
    if (fd == MAX_FILES) {
        return -1;
    }

    char buffer[512] = {};

    char* pBuf = buffer;
    strcpy(pBuf, filepath);
    pBuf = buffer;
    while (*pBuf != 0) {
        if (*pBuf == '\\') {
            *pBuf = '/';
        }
        pBuf++;
    }

    if (DVDOpen(buffer, &gcFiles[fd]) == 1) {
        gcFiles[fd].unk48 = 0;
    } else {
        return -1;
    }
    
    FileEntry* tempEntry = &gcFiles[fd];
    gcFiles[fd].callback = (openMode & 1) ? (s32)File_ReadCallback : NULL;
    gcFiles[fd].unk50 = 1;
    tempEntry->unk2C = fd;
    return fd;
}

int File_Close(int fd) {
    if (gcFiles[fd].unk48 != -1) {
        int closeCode = DVDClose((void*)&gcFiles[fd]);
        gcFiles[fd].unk48 = -1;
        gcFiles[fd].unk50 = 0;
        gcFiles[fd].callback = 0;
        return (closeCode == 0) ? 0 : -1;
    }
    return -1;
}

int File_Read(int fd, void* arg1, int arg2, int arg3) {
    int alignedSize = (arg2 + 0x1f) & ~0x1f;
    FileEntry* entry = &gcFiles[fd];
    entry->unk44 = arg1;
    if ((alignedSize + 0x40 > arg3) || ((int)arg1 & 0x1f) || (arg3 & 0x1f)) {
        entry->unk54 = alignedSize + 0x40;
        entry->unk44 = Heap_MemAlloc(entry->unk54);
        entry->unk40 = entry->unk44;
        }
        if (gcFiles[fd].callback == 0) {
            alignedSize = DVDReadPrio((DVDFileInfo*)&gcFiles[fd], entry->unk44, alignedSize, entry->unk48, 2);
            entry->unk48 = entry->unk48 + alignedSize;

            if (entry->unk54 > 0) {
                memmove(arg1, entry->unk44, arg3);
                Heap_MemFree(entry->unk44);
                entry->unk44 = 0;
                entry->unk54 = 0;
            }
            return alignedSize;
        } else {
            entry->unk4C = arg3;
            if (entry->unk54 == 0) {
                entry->unk40 = 0;
            } else {
                entry->unk40 = arg1;
            }
            gcFiles[fd].unk50 = 3;
            Heap_Check("File.cpp", 206);
            int readPrioCode = DVDReadAsyncPrio((DVDFileInfo*)entry, entry->unk44, alignedSize, entry->unk48, entry->callback, 2);
            Heap_Check("File.cpp", 208);
            if (readPrioCode != 0) {
                return 0;
            }
        }
    return -1;
}

void File_ReadCallback(long arg0, DVDFileInfo* arg1) {
    int data = (int)arg1->cb.userData;
    if (DVDGetCommandBlockStatus((DVDFileInfo*)&gcFiles[data]) == 0) {
        gcFiles[data].unk50 = 4;
    }
}

void* File_Seek(int fd, int arg1, int arg2) {
    switch(arg2) {
        case 0:
            gcFiles[fd].unk48 = arg1;
            break;
        case 1:
            gcFiles[fd].unk48 += arg1;
            break;
        case 2:
			// unk34 is a field in DVDFileInfo
            gcFiles[fd].unk48 = gcFiles[fd].unk34 + arg1; // seek from end of the file
        default:
            break;
    }
    return (void*)gcFiles[fd].unk48;
}

int File_Length(char* filename) {
    int fd = File_Open(filename, 0);
    int length = -1;

    if (fd >= 0) {
        length = gcFiles[fd].unk34;
        if (gcFiles[fd].unk48 != -1) {
            DVDClose((void*)&gcFiles[fd]);
            gcFiles[fd].unk48 = -1;
            gcFiles[fd].unk50 = 0;
            gcFiles[fd].callback = 0;
        }
    }
    
    return length;
}


char* File_FileServerFilename(char* pFilename) {
	return pFilename;
}

int File_Sync(int fd, int arg1) {
    if (gcFiles[fd].unk48 == -1 || gcFiles[fd].callback == NULL) {
        return 0;
    }
    u32 startTick = OSGetTick();
    while (DVDGetCommandBlockStatus((DVDFileInfo*)&gcFiles[fd])) {
        FileEntry* entry = &gcFiles[fd];
        if (((OSGetTick() - startTick) / ((BUS_SPEED >> 2) / 1000)) > arg1) {
            if ((entry->unk48 == -1 || entry->callback == 0) || (3 > entry->unk50)) {
                return 0;
            } else {
                if (entry->unk50 == 3 || DVDGetCommandBlockStatus((DVDFileInfo*)entry) != 0) {
                    return 1;
                }
                Heap_Check("File.cpp", 321);
                if (entry->unk40 != 0) {
                    memmove(entry->unk40, entry->unk44, entry->unk4C);
                    DCStoreRange((uint*)entry->unk40, entry->unk4C);
                } else {
                    DCStoreRange((uint*)entry->unk44, entry->unk4C);
                }
                Heap_Check("File.cpp", 331);
                if (0 < entry->unk54) {
                    Heap_MemFree(entry->unk44);
                    entry->unk44 = 0;
                    entry->unk54 = 0;
                }
                entry->unk50 = 2;
                Heap_Check("File.cpp", 339);
                return 0;
            }
        }
        OSYieldThread();
    }
    return 0;
}

bool File_IsAnyBusy(void) {
    int var_r0;
    int fd = 0;

    while (fd < MAX_FILES) {
        FileEntry* entry = &gcFiles[fd];
        if (entry->unk48 != -1) {
            if ((entry->unk48 == -1) || (entry->callback == 0) || (entry->unk50 < 3)) {
                var_r0 = 0;
            } else if ((entry->unk50 == 3) || (DVDGetCommandBlockStatus((DVDFileInfo*)entry) != 0)) {
                var_r0 = 1;
            } else {
                Heap_Check("File.cpp", 321);
                if (entry->unk40 != NULL) {
                    memmove(entry->unk40, entry->unk44, entry->unk4C);
                    DCStoreRange((uint*)entry->unk40, entry->unk4C);
                } else {
                    DCStoreRange((uint*)entry->unk44, entry->unk4C);
                }
                Heap_Check("File.cpp", 331);
                if (entry->unk54 > 0) {
                    Heap_MemFree(entry->unk44);
                    entry->unk44 = NULL;
                    entry->unk54 = NULL;
                }
                entry->unk50 = 2;
                Heap_Check("File.cpp", 339);
                var_r0 = 0;
            }
            if (var_r0 != 0) {
                return true;
            }
        }
        fd++;
    }
    return false;
}

char* File_FileServerOutputFilename(char* name) {
	return name;
}

int File_Write(int fd, void* pData, int len) {
	return 0;
}