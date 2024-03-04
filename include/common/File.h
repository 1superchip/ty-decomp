#ifndef COMMON_FILE
#define COMMON_FILE

#include "Dolphin/dvd.h"

// Seek Values

// Set the position relative to the beginning
#define SEEK_SET (0)

// Set the position relative to current position (current += seek_offset)
#define SEEK_CUR (1)

// Set the position relative to the end (seek_offset <(=?) file_length, current += seek_offset)
#define SEEK_END (2)

struct FileEntry {
    DVDFileInfo fileInfo;
    DVDCallback callback;
    void* unk40;
    void* unk44;
    int streamOffset;
    int unk4C;
    int unk50;
    int unk54;
};

void File_InitModule(void);
int File_Open(char* filepath, int openMode);
int File_Close(int fd);
int File_Read(int fd, void* buf, int size, int arg3);
void File_ReadCallback(s32, DVDFileInfo*);
int File_Seek(int fd, int offset, int seekType);
int File_Length(char* filename);
char* File_FileServerFilename(char* pFilename);
int File_Sync(int fd, int);
bool File_IsAnyBusy(void);
char* File_FileServerOutputFilename(char* name);
int File_Write(int fd, void* pData, int len);

#endif // COMMON_FILE
