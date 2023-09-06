#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#include "common/Model.h"
#include "common/Blitter.h"
#include "common/MKAnimScript.h"
#include "common/View.h"
#include "common/Font.h"

struct UIModel {
    Vector mPos;
    Model* pModel;
    MKAnimScript mAnimScript;
    float mCenterY; // Y Center of the Model's BoundingVolume

    void Init(char* pModelName, char* pAnimName);
    void Deinit(void);
    void Update(void);
    void Draw(void);
    void SetPosition(Vector* pPos, char, View* pView);
    void SetScale(Vector* pScale) {
        pModel->matrices[0].SetRotationToScaleXYZ(pScale);
    }
};

struct UIImage {
    Blitter_Image img;
    Material* pMaterial;

    void Init(char* pMatName);
    void Init(Material* t_pMaterial);
    void Init(float endX, float endY);
    void Deinit(void);
    void Draw(void);
    void SetUVs(float uv0, float uv1, float uv2, float uv3);
    void SetPosition(Vector* pPos, char flags);
};

struct UIText {
    float x;
    float y;
    float unk8;
    float fontHeight;
    float mRoll;
    float mScaleX;
    float mScaleY;
    int mFlags;
    uint color;
    Font* pFont;
    char* pText;
    int mStringIdx;

    void Init(Font* t_pFont, int stringIdx);
    void Deinit(void);
    float Draw(void);
    float Draw(char* pDrawString);
    void SetPosition(Vector* pPos, char t_flags);
    void SetScale(float scale);
    void SetScaleEx(float xScale, float yScale);
    void GetPosition(Vector* pPos);

    void SetUnk8(float _unk8) {
        unk8 = _unk8;
    }
    void SetColor(u32 _color) {
        color = _color;
    }
    char GetFlags(void) {
        return mFlags;
    }
    float GetScale(void) {
        return mScaleX;
    }
};

#endif // USERINTERFACE_H
