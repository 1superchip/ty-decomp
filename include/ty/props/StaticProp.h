#ifndef STATICPROP_H
#define STATICPROP_H

#include "common/System_GC.h"
#include "ty/GameObject.h"
#include "ty/RangeCheck.h"
#include "common/Vector.h"
#include "common/Collision.h"
#include "ty/Ty.h"

/*struct NameFlagPair {
    char* name;
    int flag;
};
bool LoadLevel_LoadBool(KromeIniLine*, char*, bool*);
bool LoadLevel_LoadFlags(KromeIniLine*, char*, NameFlagPair*, int, int*);
bool LoadLevel_LoadVector(KromeIniLine*, char*, Vector*);
bool LoadLevel_LoadString(KromeIniLine*, char*, char*, int, int);*/

#define FX_Shake        (1)
#define FX_WaterRipple  (2)
#define FX_SpawnLeaf    (4)
#define FX_Dynamic      (8)
#define FX_Rotate       (16)

struct StaticPropDescriptor : GameObjDesc {
    char subObjectName[0x20]; // subobject used for collision
    bool bDynamic;
    bool bUseGroundColor;
    LODDescriptor lodDesc;
    int collisionInfoFlags;
    
    virtual void Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags);
    virtual void Load(KromeIni* pIni);
};

// Not sure if this was actually a structure
// but this makes it clearer what the 2 Vectors represent
struct StaticPropLoadInfo {
    Vector defaultRot;
    Vector defaultScale;
};

struct StaticProp : GameObject {
    bool collide;
    CollisionInfo collisionInfo;
    LODManager lodManager;
    
    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Draw(void);
    virtual void Init(GameObjDesc* pDesc);
    virtual void Deinit(void);

    StaticPropDescriptor* GetDesc(void) {
        return descr_cast<StaticPropDescriptor*>(pDescriptor);
    }

    Vector* GetPos(void) {
        return pModel->matrices[0].Row3();
    }

    bool TyOn(void) {
        bool cond = ty.mContext.floor.bUnderFeet && ty.mContext.floor.GetDiff(&ty.pos) < 10.0f;
        return (ty.mContext.floor.bOn || cond) && (ty.mContext.floor.res.pInfo == &collisionInfo);
    }

    static StaticPropLoadInfo loadInfo;
};

struct StaticFXPropDesc : StaticPropDescriptor {
    int effectFlags;
    Vector autoRotate;
    char rotateSubObj[0x20];

    // might be wrong
    virtual void Init(ModuleInfoBase* pMod, char* pMdlName, char* pDescrName, int _searchMask, int _flags) {
        StaticPropDescriptor::Init(pMod, pMdlName, pDescrName, _searchMask, _flags);
        effectFlags = 0;
        autoRotate.SetZero();
        rotateSubObj[0] = '\0';
    }
    
    virtual void Load(KromeIni* pIni) {
        StaticPropDescriptor::Load(pIni);
        KromeIniLine* pLine = pIni->GotoLine(modelName, NULL);
        if (pLine != NULL) {
            while (pLine != NULL && (pLine->section != NULL || pLine->pFieldName != NULL || pLine->comment != NULL)) {
                if (pLine->pFieldName != NULL) {

                    // this matches rodata ordering at the beginning
                    // should be 3 individual vectors somewhere else
                    const Vector unused_vectors[3] = { 
                        {0.0f, 0.0f, 0.0f, 0.0f}, 
                        {0.0f, 0.0f, 0.0f, 0.0f}, 
                        {0.0f, 0.0f, 0.0f, 0.0f},
                    };

                    NameFlagPair flagsTmp[5] = {
                        {"shake", FX_Shake}, 
                        {"waterRipple", FX_WaterRipple}, 
                        {"spawnLeaf", FX_SpawnLeaf},
                        {"dynamic", FX_Dynamic}, 
                        {"rotate", FX_Rotate}
                    };

                    if (LoadLevel_LoadFlags(pLine, "effectFlags", flagsTmp, ARRAY_SIZE(flagsTmp), &effectFlags) == false) {
                        if (LoadLevel_LoadVector(pLine, "autoRotate", &autoRotate)) {
                            autoRotate.Scale(gDisplay.dt);
                        } else {
                            LoadLevel_LoadString(pLine, "rotateSubObj", rotateSubObj, sizeof(rotateSubObj), 0);
                        }
                    }
                }

                pLine = pIni->GetLineWithLine(pLine);
            }
        }
    }
    
};

struct StaticFXProp : StaticProp {
    Vector unk58;
    Vector mDefaultRot; // default prop rotation
    // autoRotation is the rotation that is constantly updated by the description's autoRotate parameter
    // this is added to the prop rotation (rot) on stack and rotates the default matrix in the prop's model
    Vector autoRotation; // autoRotate is added to this vector
    Vector waterCollisionPos; // position of water that was collided with
    
    // these flags may have been part of a union with s32
    bool bCollidesWithWater : 1;
    bool b1 : 1;
    bool bVisible : 1;
    bool bDefaultVisible : 1;
    u8 b4 : 1;
    u8 b5 : 1;
    u8 b6 : 1;
    u8 b7 : 1;
    
    int unk9C;
    int rotateSubObjIndex;
    
    virtual bool LoadLine(KromeIniLine* pLine);
    virtual void LoadDone(void);
    virtual void Reset(void);
    virtual void Update(void);
    virtual void Draw(void);
    virtual void Message(MKMessage*);
    virtual void Init(GameObjDesc* pDesc);
    void UpdateShake(void);
    void UpdateWaterRipple(void);
    void UpdateDropLeaf(void);
    void UpdateRotate(void);
    void Show(bool);

    StaticFXPropDesc* GetDesc(void) {
        return descr_cast<StaticFXPropDesc*>(pDescriptor);
    }

    Vector* GetPos(void) {
        return &unk58;
    }

    static bool bTempVisible;
};

void StaticProp_LoadResources(KromeIni* pIni);

extern "C" void memset(void*, int, int);

#endif // STATICPROP_H
