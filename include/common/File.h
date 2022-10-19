// clean this file up

struct FileEntry {
	char unk[0x2c];
    int unk2C;
    int unk30;
    int unk34;
    int unk38;
    uint callback;
    void* unk40;
    void* unk44;
    int unk48; // pointer but it needs to be casted to int and then void* in functions
    int unk4C;
    int unk50;
    int unk54;
};

// default struct to compile

typedef struct
{
    char gameName[4];
    char company[2];
    u8 diskNumber;
    u8 gameVersion;
    u8 streaming;
    u8 streamingBufSize;
    u8 padding[22];
} DVDDiskID;

typedef struct DVDCommandBlock DVDCommandBlock;
typedef void (*DVDCBCallback)(s32 result, DVDCommandBlock *block);

struct DVDCommandBlock
{
    /*0x00*/ DVDCommandBlock *next;
    /*0x04*/ DVDCommandBlock *prev;
    /*0x08*/ u32 command;
    /*0x0C*/ s32 state;
    /*0x10*/ u32 offset;
    /*0x14*/ u32 length;
    /*0x18*/ void *addr;
    /*0x1C*/ u32 currTransferSize;
    /*0x20*/ u32 transferredSize;
    /*0x24*/ DVDDiskID *id;
    /*0x28*/ DVDCBCallback callback;
    /*0x2C*/ void *userData;
};

typedef struct DVDFileInfo  DVDFileInfo;
typedef void (*DVDCallback)(s32 result, DVDFileInfo *fileInfo);

struct DVDFileInfo
{
	/*0x00*/ DVDCommandBlock cb;
    /*0x30*/ u32 startAddr;
    /*0x34*/ u32 length;
    /*0x38*/ DVDCallback callback;
};

void File_InitModule(void);
int File_Open(char*, int);
int File_Close(int);
int File_Read(int, void*, int, int);
void File_ReadCallback(s32, DVDFileInfo*);
void* File_Seek(int, int, int);
int File_Length(char*);
char* File_FileServerFilename(char*);
int File_Sync(int, int);
bool File_IsAnyBusy(void);
char* File_FileServerOutputFilename(char*); // may return a char* (returns param_1)
int File_Write(int, void*, int);