#ifndef COMMON_MKPACKAGE
#define COMMON_MKPACKAGE

void MKPackage_InitModule(void);
void MKPackage_DeinitModule(void);
void MKPackage_Update(void);
void MKPackage_Load(char* pPackageName, bool bLoadFilesAsync);
void MKPackage_LoadDynamic(char* pFilename, bool bLoadFilesAsync);
bool MKPackage_IsLoaded(float* pProgress);
void MKPackage_Cancel(void);
int MKPackage_Free(void);

struct AutoEntry {
    char name[0x20];
};

struct PackageEntry {
    char* pFilename;
    int fileOffset;
    int fileSize;
    void* pFileData;
};

#endif // COMMON_MKPACKAGE
