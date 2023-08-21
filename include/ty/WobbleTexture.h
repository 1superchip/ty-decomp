#ifndef WOBBLETEXURE_H
#define WOBBLETEXURE_H

#include "common/Vector.h"
#include "common/Material.h"
#include "common/Blitter.h"

struct WobbleEntry {
    Vector mPos;
    char padding_0x10[0x20];
    float unk30;
    float unk34;
    Vector mColor;
    float unk48;
    float unk4C;
};

struct WobbleTexture {
    // 2D array of WobbleEntry structs
    WobbleEntry* mpEntries;
    float unk4;
    int mWidth;
    int mHeight;
    float unk10;
    Blitter_TriStrip* mpTriStrips;

    void Init(int _width, int _height);
    void Deinit(void);
    float SetUpGrid(Vector*, float, float, float);
    void WobbleUVs(float);
    void Draw(Material*, bool);

    /// @brief Returns the WobbleEntry at (i, j) of mpEntries
    WobbleEntry* GetWobbleEntry(int i, int j) {
        return &mpEntries[j * mWidth] + i;
    }
};

#endif // WOBBLETEXURE_H
