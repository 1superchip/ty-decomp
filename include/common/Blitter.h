#ifndef COMMON_BLITTER
#define COMMON_BLITTER

#include "types.h"
#include "common/Material.h"

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
    Vector color1;
    
    void Draw(int count);
    void DrawNoMat(int count) {
        Material::UseNone(-1);
        Draw(count);
    }
};

struct Blitter_Line3D {
    Vector point; // vertex 0 position
    Vector point1; // vertex 1 position
    Vector color; // vertex 0 color
    Vector color1; // vertex 1 color
    
    void Draw(int count, float);
    void DrawNoMat(int count, float f1) {
        Material::UseNone(-1);
        Draw(count, f1);
    }
};

struct Blitter_Particle {
    Vector pos;
    Vector color;
    float unk20;
    uint unk24;
    float angle;
    float unk2C;
    
    void Draw(int);
    void DrawFrames(int, float);
};

struct Blitter_Image {
    float startX; // X position of Corner 1
    float startY; // Y position of Corner 1
    float endX; // X position of Corner 2
    float endY; // Y position of Corner 2
    float z;
    float uv0;
    float uv1;
    float uv2;
    float uv3;
    Vector color;
    Vector unk40;
    
    void Draw(int count);
};

struct Blitter_UntexturedImage {
    Vector pos;
    float unk10;
    u8 color[4];
    float unk20;
    
    void Draw(int count);
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