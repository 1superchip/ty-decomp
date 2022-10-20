
// fix this
struct RkvFileEntry {
    int data[8];
    uint data1[7];
    short shorts[2];
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
    int rkvFd; // 0x40
    int nmbrOfEntries;
    int unk48;
    RkvFileEntry* pFileEntries;
    int unk50;
    int unk54;
    bool unk58;
	
	void Init(char*);
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