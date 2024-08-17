#ifndef TRAILS_H
#define TRAILS_H

#include "ty/tools.h"
#include "common/Blitter.h"

struct TrailNode {
    Vector pos0; // Position of first point
    Vector pos1; // Position of second point
};

/// @brief Object used for boomerang trails
struct BoomerangTrail {
    int unk0;
    CircularQueue<TrailNode> queue;
    Vector vec0;
    Vector vec1;
    Material* pMaterial;
    float color; // Value of RGB colors for tristrips (color, color, color, 1.0f)
    int unk3C;

    void Init(char* pTrailName, float rgb);
    void Deinit(void);
    void Reset(void);
    void Update(Vector* pos0, Vector* pos1);
    void Draw(void);
};

#endif // TRAILS_H
