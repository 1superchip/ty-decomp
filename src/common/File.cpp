#include "types.h"
#include "Dolphin/os.h"
#include "common/File.h"
#include "common/Heap.h"

#define MAX_FILES 8

static FileEntry gcFiles[MAX_FILES];

extern "C" void memset(void*, char, int);
extern "C" void memmove(void*, void*, int);
extern "C" int DVDGetCommandBlockStatus(DVDFileInfo*);
extern "C" int DVDReadPrio(DVDFileInfo*, void*, int, int, int prio);
extern "C" BOOL DVDReadAsyncPrio(DVDFileInfo *, void *, int length, int offset, uint callback, int prio);
extern "C" int DVDClose(void*);
extern "C" int DVDOpen(char*, void*);
extern "C" void strcpy(char*, char*);

/// @brief Initiaites all File entries
/// @param  None
void File_InitModule(void) {
    for(int i = 0; i < MAX_FILES; ++i) {
        memset(&gcFiles[i], 0, sizeof(FileEntry));
        gcFiles[i].unk48 = -1;
    }
}

/// @brief Opens and returns an fd for a file
/// @param filepath Name of the file
/// @param openMode Open mode parameter
/// @return FD of the opened file, -1 for failure to open
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
    while (*pBuf != '\0') {
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
        gcFiles[fd].callback = NULL;
        return (closeCode == 0) ? 0 : -1;
    }
    return -1;
}

// buf is the buffer the data is read into
int File_Read(int fd, void* buf, int size, int arg3) {
    int alignedSize = (size + 0x1f) & ~0x1f; // align size
    FileEntry* entry = &gcFiles[fd];
    entry->unk44 = buf;
    if ((alignedSize + 0x40 > arg3) || ((int)buf & 0x1f) || (arg3 & 0x1f)) {
        entry->unk54 = alignedSize + 0x40;
        entry->unk44 = Heap_MemAlloc(entry->unk54);
        entry->unk40 = entry->unk44;
	}
	if (gcFiles[fd].callback == NULL) {
		alignedSize = DVDReadPrio((DVDFileInfo*)&gcFiles[fd], entry->unk44, alignedSize, entry->unk48, 2);
		entry->unk48 = entry->unk48 + alignedSize;
		if (entry->unk54 > 0) {
			memmove(buf, entry->unk44, arg3);
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
            entry->unk40 = buf;
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

/// @brief Updates a file's position
/// @param fd Descriptor of file
/// @param offset Value to use
/// @param seekType Seek type
/// @return 
void* File_Seek(int fd, int offset, int seekType) {
    switch(seekType) {
        case 0:
            gcFiles[fd].unk48 = offset;
            break;
        case 1:
            gcFiles[fd].unk48 += offset;
            break;
        case 2:
			// unk34 is a field in DVDFileInfo
            gcFiles[fd].unk48 = gcFiles[fd].unk34 + offset; // seek from end of the file
        default:
            break;
    }
    return (void*)gcFiles[fd].unk48;
}

/// @brief Returns the length of a file
/// @param filename Name of the file
/// @return Length of the file, -1 if the file could not be opened
int File_Length(char* filename) {
    int fd = File_Open(filename, 0);
    int length = -1;

    if (fd >= 0) {
        length = gcFiles[fd].unk34;
		File_Close(fd);
    }
    
    return length;
}

char* File_FileServerFilename(char* pFilename) {
	return pFilename;
}

int File_IsBusy(int fd) {
    FileEntry* entry = &gcFiles[fd];
    if ((entry->unk48 == -1) || (entry->callback == NULL) || (entry->unk50 < 3)) {
        return false;
    } else if ((entry->unk50 == 3) || (DVDGetCommandBlockStatus((DVDFileInfo*)entry) != 0)) {
        return true;
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
    }
    return false;
}

// might return a bool?
int File_Sync(int fd, int arg1) {
    if (gcFiles[fd].unk48 == -1 || gcFiles[fd].callback == NULL) {
        return 0;
    }
    u32 startTick = OSGetTick();
    while (DVDGetCommandBlockStatus((DVDFileInfo*)&gcFiles[fd])) {
        FileEntry* entry = &gcFiles[fd];
        if (OSTicksToMilliseconds(OSGetTick() - startTick) > arg1) {
            return File_IsBusy(fd);
        }
        OSYieldThread();
    }
    return 0;
}

bool File_IsAnyBusy(void) {
    for (int fd = 0; fd < MAX_FILES; fd++) {
        if (gcFiles[fd].unk48 != -1 && File_IsBusy(fd)) {
            return true;
        }
    }
    return false;
}

char* File_FileServerOutputFilename(char* name) {
	return name;
}

int File_Write(int fd, void* pData, int len) {
	return 0;
}
