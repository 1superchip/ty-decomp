#ifndef COMMON_MKPARTICLEGEN_H
#define COMMON_MKPARTICLEGEN_H

#include "common/View.h"
#include "common/Vector.h"
#include "common/Material.h"

struct MKParticleGenType {
    struct Envelope {
        float unk0;
        float unk4;
        float unk8;
        float unkC;
        float unk10;
        float unk14;
    };
    float alpha;
    float radius;
    int primitiveType;
    float halfWidth;
    float halfHeight;

    // Color Vectors
    Vector color0;
    Vector color1;
    Vector color2;
    Vector color3;
    
    float xVel;
    float yVel;
    float zVel;
    int nmbrOfParticles;
    char* pMatName;
    Envelope* pEnvelopes;
    float numEnvEntries;
    float envAnimRate;
    float unk74;
    float unk78;
    
    void Init(void); // Stripped
    void SetEnvelope(MKParticleGenType::Envelope*, int);
    void SetAnimRate(float animRate) {
        envAnimRate = animRate;
    }
};

struct MKParticleGen {
    MKParticleGenType* pType;
    Material* pMat;
    float unk8;
    float unkC;
    float unk10;
    float xPos;
    float yPos;
    float zPos;
    float unk20;
    float unk24;
    Vector mSrcPos;
    
    void Init(MKParticleGenType* _pType, Material* _pMat);
    void Deinit(void);
    void Update(void);
    void Draw(View* pView, Vector*, Vector*);
    void DrawDebugInfo(View* pView);
    void DrawLines(View* pView, Vector*);
    void DrawQuads(View* pView, Vector*);
};

#endif // COMMON_MKPARTICLEGEN_H
