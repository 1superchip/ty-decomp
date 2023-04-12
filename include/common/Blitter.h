#ifndef COMMON_BLITTER
#define COMMON_BLITTER

#include "types.h"
#include "common/Vector.h"

void Blitter_InitModule(void);
void Blitter_DeinitModule(void);

// Most blitter structs are designed to draw multiple at once (see Blitter_TriStrip)
// An array should be created and then the Draw method should called from the first element of the array

/*
Reword this?
Blitter_Box describes a 3D box with an origin point, length of each xyz component, and color

Origin: (1, 0, 0)
Extent: (1, 1, 1)

Box's Minimum Vertex Position: (1, 0, 0)
Box's Maximum Vertex Position: (2, 1, 1)

Maximum = origin + extent

*/

struct Blitter_Box {
    Vector origin; // origin point (minimum)
    Vector extent; // extent vector (edge length)
    Vector color;
	char unk[0x10]; // unused Vector?
	
	void Draw(int);
};

struct Blitter_Line3D {
	Vector point; // vertex 0 position
	Vector point1; // vertex 1 position
	Vector color; // vertex 0 color
	Vector color1; // vertex 1 color
	
	void Draw(int, float);
};

struct Blitter_Particle {
    Vector pos;
    Vector color;
    float unk20;
    uint unk24;
    float angle;
    int unk2C;
	
	void Draw(int);
	void DrawFrames(int, float);
};

struct Blitter_Image {
    Vector pos;
    Vector unk10;
    float unk20;
    Vector color;
	char unk[16];
	
	void Draw(int);
};

struct Blitter_UntexturedImage {
    Vector pos;
	float unk10;
	u8 color[4];
    float unk20;
	
	void Draw(int);
};

struct Blitter_TriStrip {
    Vector pos;
    Vector color;
    Vector uv;
	
	void Draw(int, float);
	void DrawNoPerspective(int, float);
};

struct Blitter_TriFan {
    Vector pos;
    Vector color;
    Vector uv;
	
	void Draw2D(int, float);
};

#endif // COMMON_BLITTER