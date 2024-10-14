#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "common/Blitter.h"

#define NUM_QUADTRISTRIP_TRIS (4)

struct QuadTriStrip {
    Blitter_TriStrip mTris[NUM_QUADTRISTRIP_TRIS];
    Vector mPos;
    float unkD0;
    float unkD4;
    bool unkBool0;

    void Init(Vector* pPos, float param_2, float param_3, bool param_4, Vector* pUVs, Vector* pColors) {
        int i;
        unkD0 = param_2 * 0.5f;
        unkD4 = param_3 * 0.5f;
        mPos = *pPos;
        unkBool0 = param_4;
        if (param_4) {
            mTris[0].pos.Set(mPos.x - unkD0, mPos.y + unkD4, mPos.z);
            mTris[1].pos.Set(mPos.x + unkD0, mPos.y + unkD4, mPos.z);
            mTris[2].pos.Set(mPos.x - unkD0, mPos.y - unkD4, mPos.z);
            mTris[3].pos.Set(mPos.x + unkD0, mPos.y - unkD4, mPos.z);
        } else {
            mTris[0].pos.Set(mPos.x, mPos.y + unkD4, mPos.z);
            mTris[1].pos.Set(mPos.x + param_2, mPos.y + unkD4, mPos.z);
            mTris[2].pos.Set(mPos.x, mPos.y - unkD4, mPos.z);
            mTris[3].pos.Set(mPos.x + param_2, mPos.y - unkD4, mPos.z);
        }
        if (pUVs == NULL) {
            mTris[0].uv.x = 0.0f;
            mTris[0].uv.y = 0.0f;
            mTris[1].uv.x = 1.0f;
            mTris[1].uv.y = 0.0f;
            mTris[2].uv.x = 0.0f;
            mTris[2].uv.y = 1.0f;
            mTris[3].uv.x = 1.0f;
            mTris[3].uv.y = 1.0f;
        } else {
            mTris[0].uv.x = pUVs[0].x;
            mTris[0].uv.y = pUVs[0].y;
            mTris[1].uv.x = pUVs[0].z;
            mTris[1].uv.y = pUVs[0].w;
            mTris[2].uv.x = pUVs[1].x;
            mTris[2].uv.y = pUVs[1].y;
            mTris[3].uv.x = pUVs[1].z;
            mTris[3].uv.y = pUVs[1].w;
        }
        if (pColors == NULL) {
            for (i = 0; i < NUM_QUADTRISTRIP_TRIS; i++) {
                mTris[i].color.Set(1.0f, 1.0f, 1.0f, 1.0f);
            }
        } else {
            for (i = 0; i < NUM_QUADTRISTRIP_TRIS; i++) {
                mTris[i].color = pColors[i];
            }
        }
    }

    void Draw(Vector* pNewPos) {
        if (unkBool0) {
            mTris[0].pos.Set(pNewPos->x - unkD0, pNewPos->y + unkD4, pNewPos->z);
            mTris[1].pos.Set(pNewPos->x + unkD0, pNewPos->y + unkD4, pNewPos->z);
            mTris[2].pos.Set(pNewPos->x - unkD0, pNewPos->y - unkD4, pNewPos->z);
            mTris[3].pos.Set(pNewPos->x + unkD0, pNewPos->y - unkD4, pNewPos->z);
        } else {
            mTris[0].pos.Set(pNewPos->x, pNewPos->y + unkD4, pNewPos->z);
            mTris[1].pos.Set(pNewPos->x + (unkD0 * 2.0f), pNewPos->y + unkD4, pNewPos->z);
            mTris[2].pos.Set(pNewPos->x, pNewPos->y - unkD4, pNewPos->z);
            mTris[3].pos.Set(pNewPos->x + (unkD0 * 2.0f), pNewPos->y - unkD4, pNewPos->z);
        }
        mTris[0].Draw(NUM_QUADTRISTRIP_TRIS, 1.0f);
    }
};

/// @brief Structure used for visual progress bar in UI (Volume Settings)
struct ProgressBar {
    Blitter_Image foregroundImage;
    Material* pFGMat;
    Material* pMat;
    Blitter_Image backgroundImage;
    QuadTriStrip backgroundQuad;
    QuadTriStrip foregroundQuad;
    Vector mPos; // Center Position?
    float floatProgress; // progress as a float
    int mProgress;
    int maxProgress;
    float progressRatio;
    float unk268;
    float unk26C;
    float unk270;
    float mUpdateVal; // Value used when updating floatProgress
    bool bDrawQuad;
    bool bInitialised;
    
    void Init(char* pBGMatName, char* pFGMatName, int xPos, int yPos,
        float scale, int, float updateVal, float);
    void Update(void);
    void Draw(void);
    void Deinit(void);
    void SetProgress(int newProgress);
    void IncrementProgress(void);
    bool DecrementProgress(void);
    void SetBackgroundBlitterUV(float, float, float, float);
    void SetForegroundBlitterUV(float, float, float, float);
};


#endif // PROGRESSBAR_H
