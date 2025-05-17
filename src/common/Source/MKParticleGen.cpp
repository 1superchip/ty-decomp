#include "common/MKParticleGen.h"
#include "common/System_GC.h"
#include "common/Debug.h"
#include "common/Str.h"
#include "common/Blitter.h"

MKParticleGenType::Envelope wobbleEnv[16] = {
    {0.0f, -10.0f, 10.0f, 0.0f, 0.0f, 0.0f},
    {3.8f, -7.10f, 9.20f, 0.0f, 0.0f, 0.0f},
    {7.1f, 0.000f, 7.10f, 0.0f, 0.0f, 0.0f},
    {9.20f, 7.10f, 3.80f, 0.0f, 0.0f, 0.0f},
    {10.0f, 10.0f, 0.00f, 0.0f, 0.0f, 0.0f},
    {9.20f, 7.10f, -3.8f, 0.0f, 0.0f, 0.0f},
    {7.10f, 0.00f, -7.1f, 0.0f, 0.0f, 0.0f},
    {3.8f, -7.10f, -9.2f, 0.0f, 0.0f, 0.0f},
    {0.0f, -10.0f, -10.0f, 0.0f, 0.0f, 0.0f},
    {-3.8f, -7.1f, -9.2f, 0.0f, 0.0f, 0.0f},
    {-7.1f, 0.00f, -7.1f, 0.0f, 0.0f, 0.0f},
    {-9.2f, 7.10f, -3.8f, 0.0f, 0.0f, 0.0f},
    {-10.0f, 10.0f, 0.00f, 0.0f, 0.0f, 0.0f},
    {-9.2f, 7.10f, 3.80f, 0.0f, 0.0f, 0.0f},
    {-7.1f, 0.00f, 7.10f, 0.0f, 0.0f, 0.0f},
    {-3.8f, -7.10f, 9.2f, 0.0f, 0.0f, 0.0f},
};

MKParticleGenType::Envelope testEnv[12] = {
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {-10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {-30.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {-60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {-30.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {-10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {30.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {30.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
};

MKParticleGenType::Envelope defaultPGTEnvelope = {};

static bool gbDrawParticles = true;

static bool gbDrawParticlesDebugInfo = false;
static bool gbDrawParticlesBox = false;
static bool gbMakeParticlesStandOut = false;

// Unused / stripped
void MKParticleGenType::Init(void) {
    alpha = 1.0f;
    radius = 500.0f;
    primitiveType = 2;
    halfWidth = 5.0f;
    halfHeight = 5.0f;
    color0.Set(1.0f, 1.0f, 1.0f, 1.0f);
    color1.Set(1.0f, 1.0f, 1.0f, 1.0f);
    color2.Set(1.0f, 1.0f, 1.0f, 1.0f);
    color3.Set(1.0f, 1.0f, 1.0f, 1.0f);
    xVel = 0.0f;
    yVel = -100.0f;
    zVel = 0.0f;
    nmbrOfParticles = 500;
    pMatName = "fx_073";
    SetEnvelope(testEnv, ARRAY_SIZE(testEnv));
    SetAnimRate(3.0f);
    unk74 = 9.090909E+10f;
    unk78 = 9.090909E+10f;
}

void MKParticleGenType::SetEnvelope(MKParticleGenType::Envelope* _pEnvelopes, int _numEnvelopes) {
    pEnvelopes = _pEnvelopes;
    numEnvEntries = (float)_numEnvelopes;
    for (int i = 0; i < _numEnvelopes - 1; i++) {
        _pEnvelopes[i].unkC = _pEnvelopes[i + 1].unk0 - _pEnvelopes[i].unk0;
        _pEnvelopes[i].unk10 = _pEnvelopes[i + 1].unk4 - _pEnvelopes[i].unk4;
        _pEnvelopes[i].unk14 = _pEnvelopes[i + 1].unk8 - _pEnvelopes[i].unk8;
    }
    int index = _numEnvelopes - 1;
    _pEnvelopes[index].unkC = _pEnvelopes[0].unk0 - _pEnvelopes[index].unk0;
    _pEnvelopes[index].unk10 = _pEnvelopes[0].unk4 - _pEnvelopes[index].unk4;
    _pEnvelopes[index].unk14 = _pEnvelopes[0].unk8 - _pEnvelopes[index].unk8;
}

void MKParticleGen::Init(MKParticleGenType* _pType, Material* _pMat) {
    pType = _pType;
    if (_pMat != NULL) {
        pMat = _pMat;
        _pMat->referenceCount++;
    } else {
        if (_pType->pMatName != NULL) {
            pMat = Material::Create(_pType->pMatName);
        } else {
            pMat = NULL;
        }
    }
    unk8 = 0.0f;
    unkC = 0.0f;
    unk10 = 0.0f;
    xPos = 0.0f;
    yPos = 0.0f;
    zPos = 0.0f;
    unk20 = 0.0f;
    unk24 = 1.0f;
    mSrcPos.Set(0.0f, 0.0f, 0.0f, 1.0f);
}

void MKParticleGen::Deinit(void) {
    if (pMat != NULL) {
        pMat->Destroy();
        pMat = NULL;
    }
}

void MKParticleGen::Update(void) {
    xPos += (unk8 * gDisplay.dt) + (pType->xVel * gDisplay.dt);
    yPos += (unkC * gDisplay.dt) + (pType->yVel * gDisplay.dt);
    zPos += (unk10 * gDisplay.dt) + (pType->zVel * gDisplay.dt);
    unk20 += (pType->envAnimRate * gDisplay.dt);
    unk20 = unk20 - (pType->numEnvEntries * (int)(unk20 * (1.0f / pType->numEnvEntries)));
}

void MKParticleGen::Draw(View* pView, Vector* pNewPos, Vector* pVec1) {
    if (!gbDrawParticles) return;
    if (pNewPos) {
        mSrcPos = *pNewPos;
    }
    Vector BoxPos;
    BoxPos.Set(0.0f, 0.0f, pType->radius, 0.0f);
    BoxPos.ApplyRotMatrix(&pView->unk48);
    BoxPos.Add(&mSrcPos);
    BoxPos.w += mSrcPos.w;
    Matrix m;
    m.SetIdentity();
    *m.Row3() = BoxPos;
    pView->SetLocalToWorldMatrix(&m);
    Vector particleOffset;
    float s = pType->radius * 2.0f;
    float inv_s = 1.0f / s;
    particleOffset.Sub(pVec1, &BoxPos);
    particleOffset.x += xPos;
    particleOffset.y += yPos;
    particleOffset.z += zPos;
    particleOffset.x = particleOffset.x - (int)(particleOffset.x * inv_s) * s;
    if (particleOffset.x < 0.0f) {
        particleOffset.x += s;
    }
    particleOffset.y = particleOffset.y - (int)(particleOffset.y * inv_s) * s;
    if (particleOffset.y < 0.0f) {
        particleOffset.y += s;
    }
    particleOffset.z = particleOffset.z - (int)(particleOffset.z * inv_s) * s;
    if (particleOffset.z < 0.0f) {
        particleOffset.z += s;
    }
    if (pType->nmbrOfParticles > 0) {
        switch (pType->primitiveType) {
            case 0:
                break;
            case 1:
                DrawLines(pView, &particleOffset);
                break;
            case 2:
                DrawQuads(pView, &particleOffset);
                break;
        }
    }
    if (gbDrawParticlesBox) {
        Blitter_Box box;
        box.origin.Set(pType->radius, pType->radius, pType->radius);
        box.extent.Set(-pType->radius, -pType->radius, -pType->radius);
        box.extent.Subtract(&box.origin);
        box.color.Set(0.0f, 1.0f, 0.0f, 1.0f);
        box.color1.Set(0.0f, 1.0f, 0.0f, 1.0f);
        Material::UseNone(-1);
        box.Draw(1);
    }
    if (gbDrawParticlesDebugInfo) {
        DrawDebugInfo(pView);
    }
}

void MKParticleGen::DrawLines(View* pView, Vector* pBoxOff) {
    int particleStartPosSeed = 0;
    float f20 = pType->radius * 2.0f;
    Vector color[2];
    Blitter_Line3D line[60];
    float f19 = 1.0f / f20;
    if (gbMakeParticlesStandOut != false) {
        color[0].Set(1.0f, 1.0f, 1.0f, 1.0f);
        color[1].Set(1.0f, 1.0f, 1.0f, 1.0f);
    } else {
        color[0] = pType->color0;
        color[1] = pType->color1;
    }
    if (mSrcPos.y < pType->unk78 - pType->radius ||
        mSrcPos.y > pType->unk74 + pType->radius) {
        return;
    }
    int numParticles = pType->nmbrOfParticles;
    float radiusSq = pType->radius * pType->radius;
    float sp_f54 = pType->unk78 - pView->unk88.Row3()->y;
    float sp_f50 = pType->unk74 - pView->unk88.Row3()->y;
    while (numParticles != 0) {
        int tmp;
        int j;
        int t;
        if (60 <= numParticles) {
            tmp = 60;
        } else {
            tmp = numParticles;
        }
        j = tmp - 1;
        t = tmp;
        
        for (int i = 0; i <= j; j--) {
            float f17 = 1.0f / radiusSq;
            float f16;
            float randParticleIndex = RandomFR(&particleStartPosSeed, 0.0f, pType->numEnvEntries);
            randParticleIndex += unk20;
            if (randParticleIndex > pType->numEnvEntries) {
                randParticleIndex -= pType->numEnvEntries;
            }
            float fraction = randParticleIndex - (int)randParticleIndex;
            int envIndex = (int)randParticleIndex;
            line[j].point.x = RandomFR(&particleStartPosSeed, 0.0f, f20);
            line[j].point.x += pBoxOff->x;
            line[j].point.x -= (f20 * (int)(line[j].point.x * f19));
            line[j].point.x -= pType->radius;
            line[j].point.x += pType->pEnvelopes[envIndex].unk0 + (pType->pEnvelopes[envIndex].unkC * fraction);
            line[j].point1.x = line[j].point.x;
            f16 = Sqr<float>(line[j].point.x);
            
            line[j].point.y = RandomFR(&particleStartPosSeed, 0.0f, f20);
            line[j].point.y += pBoxOff->y;
            line[j].point.y -= f20 * (int)(line[j].point.y * f19);
            line[j].point.y -= pType->radius;
            line[j].point.y += pType->pEnvelopes[envIndex].unk4 + (pType->pEnvelopes[envIndex].unk10 * fraction);
            line[j].point1.y = line[j].point.y - pType->halfHeight;
            f16 += Sqr<float>(line[j].point.y);
            line[j].point.y += pType->halfHeight;
            
            line[j].point.z = RandomFR(&particleStartPosSeed, 0.0f, f20);
            line[j].point.z += pBoxOff->z;
            line[j].point.z -= f20 * (int)(line[j].point.z * f19);
            line[j].point.z -= pType->radius;
            line[j].point.z += pType->pEnvelopes[envIndex].unk8 + (pType->pEnvelopes[envIndex].unk14 * fraction);
            line[j].point1.z = line[j].point.z;
            
            f16 += Sqr<float>(line[j].point.z);

            float alphaScale = Min<float>(radiusSq, f16);
            alphaScale *= f17;
            alphaScale = 1.0f - alphaScale;
            
            line[j].color = color[0];
            line[j].color1 = color[1];
            *line[j].color.GetAlpha() *= alphaScale;
            *line[j].color1.GetAlpha() *= alphaScale;
            float sy = Max<float>(0.0f, Min<float>(1.0f, sp_f54 - line[j].point.y));
            float sx = Max<float>(0.0f, Min<float>(1.0f, line[j].point.y - sp_f50));
            float m = 1.0f - Min<float>(1.0f, sy + sx);
            *line[j].color.GetAlpha() *= m;
            *line[j].color1.GetAlpha() *= m;
        }
        numParticles -= t;
        line[0].DrawNoMat(t, (pType->alpha * unk24) * 0.5f);
    }
}

void MKParticleGen::DrawQuads(View* pView, Vector* pBoxOff) {
    Vector color[4];
    int particleStartPosSeed = 0;
    Vector pos;
    float f24 = pType->radius * 2.0f;
    float f23 = 1.0f / f24;

    if (gbMakeParticlesStandOut) {
        color[0].Set(1.0f, 1.0f, 1.0f, 1.0f);
        color[1].Set(1.0f, 1.0f, 1.0f, 1.0f);
        color[2].Set(1.0f, 1.0f, 1.0f, 1.0f);
        color[3].Set(1.0f, 1.0f, 1.0f, 1.0f);
    } else {
        color[0] = pType->color0;
        color[1] = pType->color1;
        color[2] = pType->color2;
        color[3] = pType->color3;
    }

    float sp_f54 = pType->unk78 - pView->unk88.Row3()->y;
    float sp_f50 = pType->unk74 - pView->unk88.Row3()->y;

    if (mSrcPos.y < pType->unk78 - pType->radius ||
        mSrcPos.y > pType->unk74 + pType->radius) {
        return;
    }

    pMat->Use();

    Vector cameraX = *pView->unk48.Row0();
    Vector cameraY = *pView->unk48.Row1();

    cameraY.Scale(pType->halfHeight);
    cameraX.Scale(pType->halfWidth);

    float f22 = cameraY.x - cameraX.x;
    float f21 = cameraY.y - cameraX.y;
    float f20 = cameraY.z - cameraX.z;
    
    float f19 = cameraY.x + cameraX.x;
    float f18 = cameraY.y + cameraX.y;
    float f17 = cameraY.z + cameraX.z;
    
    float radiusSq = pType->radius * pType->radius;
    float sp130 = 1.0f / radiusSq;
    
    Blitter_TriStrip strip[4];
    strip[0].uv.x = 0.0f;
    strip[0].uv.y = 0.0f;
    
    strip[1].uv.x = 1.0f;
    strip[1].uv.y = 0.0f;
    
    strip[2].uv.x = 0.0f;
    strip[2].uv.y = 1.0f;
    
    strip[3].uv.x = 1.0f;
    strip[3].uv.y = 1.0f;
    
    int numParticles = pType->nmbrOfParticles;
    while (numParticles > 0) {
        float f15;
        float randParticleIndex = RandomFR(&particleStartPosSeed, 0.0f, pType->numEnvEntries);
        randParticleIndex += unk20;
        if (randParticleIndex > pType->numEnvEntries) {
            randParticleIndex -= pType->numEnvEntries;
        }
        float fraction = randParticleIndex - (int)randParticleIndex;
        int envIndex = (int)randParticleIndex;
        pos.x = RandomFR(&particleStartPosSeed, 0.0f, f24);
        pos.x += pBoxOff->x;
        pos.x -= (f24 * (int)(pos.x * f23));
        pos.x -= pType->radius;
        pos.x += pType->pEnvelopes[envIndex].unk0 + (pType->pEnvelopes[envIndex].unkC * fraction);
        f15 = pos.x * pos.x;
        
        pos.y = RandomFR(&particleStartPosSeed, 0.0f, f24);
        pos.y += pBoxOff->y;
        pos.y -= (f24 * (int)(pos.y * f23));
        pos.y -= pType->radius;
        pos.y += pType->pEnvelopes[envIndex].unk4 + (pType->pEnvelopes[envIndex].unk10 * fraction);
        f15 += pos.y * pos.y;
        
        pos.z = RandomFR(&particleStartPosSeed, 0.0f, f24);
        pos.z += pBoxOff->z;
        pos.z -= (f24 * (int)(pos.z * f23));
        pos.z -= pType->radius;
        pos.z += pType->pEnvelopes[envIndex].unk8 + (pType->pEnvelopes[envIndex].unk14 * fraction);
        f15 += pos.z * pos.z;
        
        float min = Min<float>(radiusSq, f15);
        float alphaScale = 1.0f - (min * sp130);

        strip[0].color = color[0];
        strip[1].color = color[1];
        strip[2].color = color[2];
        strip[3].color = color[3];

        if (pMat != NULL && (pMat->blendMode == 3 || pMat->blendMode == 4)) {
            strip[0].color.Scale(alphaScale);
            strip[1].color.Scale(alphaScale);
            strip[2].color.Scale(alphaScale);
            strip[3].color.Scale(alphaScale);
        } else {
            strip[0].color.w *= alphaScale;
            strip[1].color.w *= alphaScale;
            strip[2].color.w *= alphaScale;
            strip[3].color.w *= alphaScale;
        }

        if (!(pos.y < sp_f54) && !(pos.y > sp_f50)) {
            strip[0].pos.Set(pos.x + f22, pos.y + f21, pos.z + f20);
            strip[1].pos.Set(pos.x + f19, pos.y + f18, pos.z + f17);
            strip[2].pos.Set(pos.x - f19, pos.y - f18, pos.z - f17);
            strip[3].pos.Set(pos.x - f22, pos.y - f21, pos.z - f20);
            
            strip[0].Draw(ARRAY_SIZE(strip), 1.0f);
        }
        
        numParticles--;
    }
}

/// @brief Draws information about a MKParticleGen
/// @param pView Unused
void MKParticleGen::DrawDebugInfo(View* pView) {
    float startY = 200.0f;
    gpDebugFont->DrawText(Str_Printf("Rain Particles: %d", pType->nmbrOfParticles), 10.0f, startY,
        1.5f, 1.5f, FONT_JUSTIFY_0, 0x800000ff);
    startY += 20.0f;
    gpDebugFont->DrawText(Str_Printf("xPos: %g", xPos), 10.0f, startY,
        1.5f, 1.5f, FONT_JUSTIFY_0, 0x800000ff);
    startY += 20.0f;
    gpDebugFont->DrawText(Str_Printf("yPos: %g", yPos), 10.0f, startY,
        1.5f, 1.5f, FONT_JUSTIFY_0, 0x800000ff);
    startY += 20.0f;
    gpDebugFont->DrawText(Str_Printf("zPos: %g", zPos), 10.0f, startY,
        1.5f, 1.5f, FONT_JUSTIFY_0, 0x800000ff);
    startY += 20.0f;
}
