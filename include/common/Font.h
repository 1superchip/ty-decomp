#ifndef COMMON_FONT
#define COMMON_FONT

#include "common/Material.h"
#include "common/Vector.h"
#include "common/Matrix.h"

// #define ABGR_TO_RGBA(color) 

#define COLOR_RED       (0x800000FF)
#define COLOR_RED_2     (0xFF0000FF)
#define COLOR_WHITE     (0x80FFFFFF) // Decodes to 0xFFFDFC00
#define COLOR_DARK_BLUE (0xFFFF0000)
#define COLOR_BLACK     (0xFF000000)
#define COLOR_MAGENTA   (0xFF8000FF)
#define COLOR_GREEN     (0xFF00FF00)
#define COLOR_YELLOW    (0xFF00FFFF)

#define FONTCOLOR(r, g, b, a) \
    ((r) | ((g) << 0x8) | ((b) << 0x10) | ((((a) * 128) / 255) << 0x18))

// these macros seem to work?

#define FONT_COLOR(r, g, b, a) (((((a) * 256) / 255) << 0x18) \
    | (((b * (256 / 255))) << 0x10) \
    | (((g * (256 / 255))) << 0x8) \
    | (((r * (256 / 255))) << 0x0))

#define FONT_COLOR2(r, g, b, a) (((((a) * 256) / 255) << 0x18) \
    | ((((b) + 1) / 2) << 0x10) \
    | ((((g) + 1) / 2) << 0x8) \
    | ((((r) + 1) / 2) << 0x0))

#define FONT_COLOR3(r, g, b, a) \
    ((int)((r) * 128.0f) << 0x0) |  \
    ((int)((g) * 128.0f) << 0x8) |  \
    ((int)((b) * 128.0f) << 0x10) | \
    ((int)((a) * 128.0f) << 0x18)

// #define COLOR_U(c) ((u8)((c) * 255.0f))

// #define COLOR_ELE(c) (((c) == 0.0f) ? 0 : \
//     (COLOR_U(c) & 1 ? ((COLOR_U(c)+1)>>1) : (((COLOR_U(c)>>1)+1) | 0x80)))

// #define COLOUR(r, g, b, a) ((COLOR_ELE(a) << 0x18) | (COLOR_ELE(b) << 0x10) | (COLOR_ELE(g) << 0x8) | (COLOR_ELE(r) << 0x0))

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
	static Font* Create(char* pName, char* pCharMap, float, float, float);
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

    // should exist in some header
    // void DrawTextWrapped(char* pString, float f1, float f2, float box0, float box1, float box2,
    //         float box3, FontJustify justify, uint color, int flags) {
    //     Vector box = {box0, box1, box2, box3};

    //     DrawTextWrapped(pString, f1, f2, &box, justify, color, flags);
    // }
};

#endif // COMMON_FONT
