
struct DirectoryEntry {
	char name[0x100];
};

struct RkvFileEntry {
	char name[0x20];
    int directoryIndex;
    int length;
	// unk28 would have been some value for decompression in earlier games
    int unk28; // unused in Ty
    int offset; // offset of file in RKV
    uint crc;
    uint timestamp;
    int unk38;
    short unk3C; // load order id?
    short unk3E;
};

struct RkvTOC {
    char name[0x20];
    int unk20;
    int unk24;
    int unk28;
    int unk2C;
    int unk30;
    int unk34;
    int unk38;
    int unk3C;
    int rkvFd;
    int nmbrOfEntries;
    int nmbrOfDirectories;
    RkvFileEntry* pFileEntries;
    DirectoryEntry* pDirectoryEntries;
    int unk54;
    bool unk58;
	
	void Init(char*);
	RkvFileEntry* GetEntry(char*);
	int GetAsyncHandle(void);
};

void FileSys_InitModule(void);
void FileSys_DeinitModule(void);
void FileSys_Update(void);
static int EntryCompare(void*, void*);
void* FileSys_SetLoadInterceptHandler(void* (*)(char*, int*, void*, int*));
bool FileSys_Exists(char*, int*);
static void FileSys_SetOrder(RkvFileEntry*);
void* FileSys_Load(char*, int*, void*, int);
int FileSys_Save(char*, bool, void*, int) ;
void FileSys_OutputFileOrder(void);
int FileSys_Open(char*, int*, bool);
void FileSys_Close(int);
int FileSys_GetOffset(char*);