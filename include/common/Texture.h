#ifndef COMMON_TEXTURE
#define COMMON_TEXTURE

#include "types.h"
#include "Dolphin/gx.h"

struct Texture {
	char name[0x20];
	int width;
	int height;
	int referenceCount;
	bool bMpegTarget; // only set to true by CreateMpegTarget
	void* unk30;
	void* unk34;
	void* unk38;
	void* pFileData;
	bool bTlut;
	GXTexObj texObj;
	GXTlutObj tlutObj;
	int unk70;
	static bool initialised;

	static void InitModule(void);
	static void DeinitModule(void);
	static Texture* Create(char*);
	void Destroy(void);
	void Use(void);
	static Texture* Find(char*);
	static Texture* CreateRenderTarget(char*, int, int, int);
	static Texture* CreateFromRawData(char*, void*, int, int, int);
	static Texture* CreateMpegTarget(char*, void*, int, int);
};

#endif // COMMON_TEXTURE