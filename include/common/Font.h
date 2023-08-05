#ifndef COMMON_FONT
#define COMMON_FONT

#include "common/Material.h"
#include "common/Vector.h"
#include "common/Matrix.h"

struct FontData {
    int unk0;
    int unk4;
    float unk8;
    float unkC;
    float unk10;
    int unk14[2];
    int unk1C;
    int unk20;
    float unk24;
    void* unk28;
};

// Document this enum
enum FontJustify {};

struct TextSegmentInfo {
    char* unk0;
    float unk4;
    bool unk8;
};

struct LineDataSub {
    char* unk0;
    char* unk4;
    float unk8;
};

struct TextLineData {
    LineDataSub data[30];
    int x;
    float y;
    float z;
};

struct FontCharData {
    char unk0[0x8];
    float unk8;
    float unkC;
    float unk10;
    float unk14;
	float unk18;
	int unk1C;
};

struct CharStripVert {
    float x, y, z; // position
    float u, v; // uv
    uint color;
};

struct Font {

    struct TextCharStrip {
        CharStripVert verts[4];
    };

    int unk0;
    int unk4;
    float unk8;
    float unkC;
    float unk10;
    int unk14[2];
    int unk1C;
    int unk20;
    float mHeight;
    FontCharData* pChars;
    Material* pFontMaterial;
    char name[0x20];
    int referenceCount;
	
	static bool initialised;
	
	static void InitModule(void);
	static void DeinitModule(void);
	static Font* Create(char* pName);
	static Font* Create(char* pName, char*, float, float, float);
	static Font* Find(char* pName);

	void Destroy(void);
	float CalcLength(char*);
	void DrawText(char* pString, float xPos, float yPos, float xScale, float yScale, FontJustify justify, int color);
	float GetHeight(void);
	int DrawText3d(char* pText, Vector* pPos, float scaleX, float scaleY, FontJustify justify,
        Vector* pCharPositions, int numCharPositions, int charPosIndex, uint* pCharColors, 
        int numCharColors, int charColorIndex);
	char* GetTextSegment(char**, float, TextSegmentInfo*); // what does this return
	void DrawTextWrapped(char*, float, float, Vector*, FontJustify, uint, int);
	void DrawTextWrapped3d(char*, float, float, Vector*, float, float, FontJustify, uint, int);
	void BuildLines(char*, float, float, TextLineData*);
	float DrawString(char* pText, float, float, Matrix* pMatrix, int, uint color,
        void (*)(Font::TextCharStrip*, int), void (*)(Font::TextCharStrip*, int));
    void RenderChars(Font::TextCharStrip* pCharStrips, int count);
};

#endif // COMMON_FONT
