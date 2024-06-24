#ifndef WATERVOLUME_H
#define WATERVOLUME_H

#include "ty/GameObjectManager.h"

// Generic LoadResources function for WaterVolume
void WaterVolume_LoadResources(KromeIni* pIni);

// Checks if a point is within a WaterVolume
// Iterates over all WaterVolumes within a level
bool WaterVolume_IsWithin(Vector* point, float*);

/// @brief Object used to determine whether an object is in water or not
struct WaterVolume : GameObject {

    // Inverse Matrix
    Matrix mInvMtx; // worldToLocal matrix

    // Object World matrix
    Matrix mWorldMtx; // pos in ini is Row3 of this matrix

    float mMinY; // Minimum y point of volume
    float mMaxY; // Maximum y point of volume

    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);
};

#endif // WATERVOLUME_H
