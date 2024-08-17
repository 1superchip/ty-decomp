#include "ty/effects/Trails.h"

/// @brief Initiates the trail
/// @param pTrailName Material name
/// @param rgb Value used for RGB components of the trail (rgb, rgb, rgb, 1.0f)
void BoomerangTrail::Init(char* pTrailName, float rgb) {
    pMaterial = Material::Create(pTrailName);

    queue.Init(10);
    
    color = rgb;

    Reset();
}

/// @brief Deinits the trail object
/// @param None
void BoomerangTrail::Deinit(void) {
    if (pMaterial) {
        pMaterial->Destroy();
        pMaterial = NULL;
    }
    queue.Deinit();
}

/// @brief Resets the trail
/// @param None
void BoomerangTrail::Reset(void) {
    queue.Reset();
    unk3C = 0;
}

void BoomerangTrail::Update(Vector* pos0, Vector* pos1) {
    vec0 = *pos0;
    vec1 = *pos1;

    if (++unk3C & 1) {
        return;
    }

    TrailNode* newEntry = queue.Add();

    newEntry->pos0 = *pos0;
    newEntry->pos1 = *pos1;
}

/// @brief Draws the trail
/// @param None
void BoomerangTrail::Draw(void) {

    Blitter_TriStrip strip[22];
    int i;

    View::GetCurrent()->SetLocalToWorldMatrix(NULL);
    pMaterial->Use();

    float uvY = 1.0f;
    float f4;

    if (unk3C & 1) {
        f4 = 1.0f / queue.GetUnk4();
    } else {
        f4 = 1.0f / (queue.GetUnk4() - 1.0f);
    }

    for(i = 0; i < queue.GetUnk4(); i++) {
        TrailNode* p = queue.Get(i);

        if (p == NULL) {
            break;
        }

        strip[(i * 2) + 0].pos = p->pos0;
        strip[(i * 2) + 0].color.Set(color, color, color, 1.0f);
        strip[(i * 2) + 0].uv.x = 0.0f;
        strip[(i * 2) + 0].uv.y = uvY;

        strip[(i * 2) + 1].pos = p->pos1;
        strip[(i * 2) + 1].color = strip[(i * 2) + 0].color;
        strip[(i * 2) + 1].uv.x = 1.0f;
        strip[(i * 2) + 1].uv.y = uvY;

        uvY -= f4;
    }

    if (i != 0) {
        strip[(i * 2) + 0].pos = vec0;
        strip[(i * 2) + 0].color.Set(color, color, color, 1.0f);
        strip[(i * 2) + 0].uv.x = 0.0f;
        strip[(i * 2) + 0].uv.y = 0.0f;

        strip[(i * 2) + 1].pos = vec1;
        strip[(i * 2) + 1].color = strip[(i * 2) + 0].color;
        strip[(i * 2) + 1].uv.x = 1.0f;
        strip[(i * 2) + 1].uv.y = 0.0f;

        i++;
    }

    if (i >= 2) {
        strip[0].Draw(i * 2, 1.0f);
    }
}
