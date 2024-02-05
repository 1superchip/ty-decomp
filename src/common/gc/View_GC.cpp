#include "types.h"
#include "common/System_GC.h"
#include "common/View.h"
// #include "Dolphin/gx.h"
#include "common/Heap.h"
#include "common/Material.h"
#include "common/StdMath.h"

extern "C" void memset(void*, int, int);

template <typename T>
T& ByteReverse(T&);

View* View::pCurrentView;
View* View::pDefaultView;

void View::Init(void) {
    Vector camPos;
    Vector camTarget;
    unk2AC = 1.0f;
    unk2B0 = 1.0f;
    camPos.Set(0.0f, 0.0f, 0.0f);
    camTarget.Set(0.0f, 0.0f, 1.0f);
    unk88.SetIdentity();
    unkC8.SetIdentity();
    unk108.SetIdentity();
    SetCameraLookAt(&camPos, &camTarget);
    SetProjection(PI2, 0.5f, 1000.0f);
    closeFogPlane = 500.0f;
    farFogPlane = 900.0f;
    SetFogIntensity(0.0f, 255.0f);
    SetFogColour(0x808080);
    SetDirectLight(DirectLight::GetDefault());
    unk2E8 = 0;
    unk29C = NULL;
    bDisableZWrite = false;
    unk2B4 = 1.0f;
    unk2B8 = 1.0f;
    unk288 = false;
    bOrtho = false;
}

// floating args are unused but they are viewport related in other builds
void View::Init(float arg1, float arg2, float arg3, float arg4) {
    Vector camPos;
    Vector camTarget;
    unk2AC = 1.0f;
    unk2B0 = 1.0f;
    camPos.Set(0.0f, 0.0f, 0.0f);
    camTarget.Set(0.0f, 0.0f, 1.0f);
    unk88.SetIdentity();
    unkC8.SetIdentity();
    unk108.SetIdentity();
    SetCameraLookAt(&camPos, &camTarget);
    SetProjection(PI2, 0.5f, 1000.0f);
    closeFogPlane = 500.0f;
    farFogPlane = 900.0f;
    SetFogIntensity(0.0f, 255.0f);
    SetFogColour(0x808080);
    SetDirectLight(DirectLight::GetDefault());
    unk2E8 = 0;
    unk2B4 = 1.0f;
    unk2B8 = 1.0f;
    unk29C = NULL;
    unk288 = false;
    bDisableZWrite = false;
    bOrtho = false;
}

void View::CalcMatrices(void) {
    unk1C8.Multiply4x4(&unkC8, &unk108);
    unk148.Multiply4x4(&unk88, &unk1C8);
}

void View::SetCameraMatrixLookAt(Matrix* pMatrix, Vector* pCamPos, Vector* pCamTarget) {
    Vector xDir;
    Vector yDir;
    Vector zDir;
    
    zDir.Sub(pCamTarget, pCamPos);
    zDir.Normalise(&zDir);
    
    yDir.Set(0.0f, 1.0f, 0.0f);
    
    xDir.Cross(&zDir, &yDir);
    xDir.Normalise(&xDir);
    
    yDir.Cross(&xDir, &zDir);
    
    pMatrix->Row0()->Copy(&xDir);
    pMatrix->Row0()->w = 0.0f;
    
    pMatrix->Row1()->Copy(&yDir);
    pMatrix->Row1()->w = 0.0f;
    
    pMatrix->Row2()->Copy(&zDir);
    pMatrix->Row2()->w = 0.0f;
    
    pMatrix->SetTranslation(pCamPos);
}

void View::SetCameraLookAt(Vector* pCamPos, Vector* pCamTarget) {
    mCamPos = *pCamPos;
    mCamTarget = *pCamTarget;
    View::SetCameraMatrixLookAt(&unk48, pCamPos, pCamTarget);
    mFwdDir = *unk48.Row2();
    unkC8.InverseSimple(&unk48);
	CalcMatrices();
}

void View::SetCameraRollAndLookAt(Vector* pCamPos, Vector* pCamTarget, float roll) {
    Vector xDir;
    Vector yDir;
    Vector zDir;
    Matrix rollMatrix;

    mCamPos = *pCamPos;
    mCamTarget = *pCamTarget;

    yDir.Set(0.0f, 1.0f, 0.0f);
    zDir.Sub(pCamTarget, pCamPos);
    zDir.Normalise(&zDir);
    rollMatrix.SetRotationRoll(roll);

    mFwdDir = zDir;

    xDir.Cross(&zDir, &yDir);
    xDir.Normalise(&xDir);
    yDir.Cross(&xDir, &zDir);

    // Create Matrix
    unk48.SetIdentity();
    unk48.Row0()->Copy(&xDir);
    unk48.Row0()->w = 0.0f;
    unk48.Row1()->Copy(&yDir);
    unk48.Row1()->w = 0.0f;
    unk48.Row2()->Copy(&zDir);
    unk48.Row2()->w = 0.0f;

    unk48.Multiply3x3(&rollMatrix, &unk48);
    unk48.SetTranslation(pCamPos);
    unkC8.InverseSimple(&unk48);
	CalcMatrices();
}

extern "C" double tan(double);

void View::SetProjection(float fov, float arg2, float arg3) {
    float far_minus_near = arg3 - arg2;
    float fVar27 = arg3 / far_minus_near;
    memset((void*)&unk108, 0, sizeof(Matrix));
    unk2CC = fov;
    float tangent = tan(fov / 2.0f);
    float fVar26 = 1.0f / tangent;
    float fVar31 = fVar26 / 0.8f;
    fVar26 *= unk2AC;
    fVar31 *= unk2B0;

    /*
    unk108 = 
    {
        {fVar26,       0.0f,         0.0f,             0.0f},
        {0.0f,         fVar31,       0.0f,             0.0f},
        {0.0f,         0.0f,         fVar27,           1.0f},
        {0.0f,         0.0f,         -fVar27 * arg2,   0.0f}
    }
    */

    unk108.data[0][0] = fVar26;
    unk108.data[1][1] = fVar31;
    unk108.data[2][2] = fVar27;
    unk108.data[3][2] = -fVar27 * arg2;
    unk108.data[2][3] = 1.0f;
    unk1C8.Multiply4x4(&unkC8, &unk108);
    unk148.Multiply4x4(&unk88, &unk1C8);

    float proj[4][4];
    proj[0][0] = fVar26;
    proj[0][1] = 0.0f;
    proj[0][2] = 0.0f;
    proj[0][3] = 0.0f;
    proj[1][0] = 0.0f;
    proj[1][1] = fVar31;
    proj[1][2] = 0.0f;
    proj[1][3] = 0.0f;
    proj[2][0] = 0.0f;
    proj[2][1] = 0.0f;
    proj[2][2] = (-arg2 * (1.0f / far_minus_near));
    proj[2][3] = -(arg3 * arg2) * (1.0f / far_minus_near);
    proj[3][0] = 0.0f;
    proj[3][1] = 0.0f;
    proj[3][2] = -1.0f;
    proj[3][3] = 0.0f;
    
    GXSetProjection(proj, GX_PERSPECTIVE);
    unk2C0 = arg2;
    unk2BC = arg3;
}

void View::SetLocalToWorldMatrix(Matrix* pMatrix) {
    Vector trans;
    Matrix matrix;
    trans.SetZero();
    if (pMatrix != NULL) {
        unk88 = *pMatrix;
        unk148.Multiply4x4(&unk88, &unk1C8);
    } else {
        unk88.SetIdentity();
        unk148 = unk1C8;
    }
    if (pLight != NULL) {
        unk208.Multiply3x3(&unk88, &pLight->mDirMatrix);
        unk208.SetTranslation(&trans);
    }
    matrix.Multiply(&unk88, &unkC8);
    if (unk288 != false) {
        matrix.Multiply(&matrix, &unk248);
    }
    matrix.data[0][2] *= -1.0f;
    matrix.data[1][2] *= -1.0f;
    matrix.data[2][2] *= -1.0f;
    matrix.data[3][2] *= -1.0f;
    matrix.Transpose(&matrix);
    GXLoadPosMtxImm(matrix.data, GX_PNMTX0);
    GXLoadNrmMtxImm(matrix.data, GX_PNMTX0);
}

void View::SetDirectLight(DirectLight* pDirectLight) {
    static GXLightObj lo;
    static Vector lightPos;
    if (pDirectLight != NULL) {
        pLight = pDirectLight;
    }
    if (pLight != NULL) {
        for(int i = 0; i < 3; i++) {
            GXColor color;
            int r,g,b;
            r = 255.0f * pLight->mLightColors[i].x;
            color.r = r > 0xff ? 0xff : r;
            g = 255.0f * pLight->mLightColors[i].y;
            color.g = g > 0xff ? 0xff : g;
            b = 255.0f * pLight->mLightColors[i].z;
            color.b = b > 0xff ? 0xff : b;
            color.a = 0xff;
            GXInitLightColor(&lo, color);
            Vector* vec = pLight->mLightDirs;
            lightPos.Set(vec[i].x, vec[i].y, vec[i].z);
            Matrix m;
            m.SetIdentity();
            m = unkC8;
            m.data[0][2] *= -1.0f;
            m.data[1][2] *= -1.0f;
            m.data[2][2] *= -1.0f;
            lightPos.ApplyRotMatrix(&lightPos, &m);
            lightPos.Normalise(&lightPos);
            lightPos.Scale(1000000.0f);
            GXInitLightPos(&lo, -lightPos.x, -lightPos.y, -lightPos.z);
            switch (i) {
                case 0:
                    GXLoadLightObjImm(&lo, GX_LIGHT0);
                    break;
                case 1:
                    GXLoadLightObjImm(&lo, GX_LIGHT1);
                    break;
                case 2:
                    GXLoadLightObjImm(&lo, GX_LIGHT2);
                    break;
            }
        }
        GXColor ambColor = (GXColor){0, 0, 0, 255};
        ambColor.r = 255.0f * pLight->mAmbient.x;
        ambColor.g = 255.0f * pLight->mAmbient.y;
        ambColor.b = 255.0f * pLight->mAmbient.z;
        GXSetChanAmbColor(GX_COLOR0A0, ambColor);
    }
}

void View::Use(void) {
    Matrix matrix;
    pCurrentView = this;
    if (unk29C != NULL) {
        unk38 = NULL;
        unk3C = NULL;
        unk40 = unk29C[8] - 1;
        unk44 = unk29C[9] - 1;
    } else {
        matrix.Multiply(&unk88, &unkC8);
        if (unk288 != false) {
            matrix.Multiply(&matrix, &unk248);
        }
        matrix.data[0][2] *= -1.0f;
        matrix.data[1][2] *= -1.0f;
        matrix.data[2][2] *= -1.0f;
        matrix.data[3][2] *= -1.0f;
        matrix.Transpose(&matrix);
        GXLoadPosMtxImm(matrix.data, GX_PNMTX0);
        GXLoadNrmMtxImm(matrix.data, GX_PNMTX0);
        SetProjection(unk2CC, unk2C0, unk2BC);
        SetDirectLight(NULL);
    }
}

void View::InitModule(void) {
    pDefaultView = (View*)Heap_MemAlloc(sizeof(View));
    pDefaultView->Init(0.0f, 0.0f, (float)(gDisplay.unk10 - 1), (float)(gDisplay.unk14 - 1));
    pCurrentView = NULL;
}

void View::DeinitModule(void) {
	// should this function set pDefaultView to NULL?
	Heap_MemFree((void*)pDefaultView);
}

// ProjectPoint?
float View::TransformPoint(IntVector* arg1, Vector* arg2) {
    Vector temp;
    temp.ApplyMatrix(arg2, &unk148);
    float oneOverW = 1.0f / temp.w;
    int tx = ((temp.x * oneOverW) * gDisplay.orthoXSize) * 0.5f;
    arg1->x = ((int)gDisplay.orthoXSize >> 1) + tx;
    int ty = ((temp.y * oneOverW) * gDisplay.orthoYSize) * 0.5f;
    int endY = ((int)gDisplay.orthoYSize >> 1) + ty;
    arg1->y = gDisplay.orthoYSize - (float)endY;
    arg1->z = temp.z * oneOverW;
    arg1->w = (int)temp.w;
    return oneOverW;
}

void View::ClearZBuffer(void) {
    float projection[7];
    float proj[4][4];
    GXGetProjectionv(projection);
    proj[0][0] = 0.02f;
    proj[0][1] = 0.0f;
    proj[0][2] = 0.0f;
    proj[0][3] = -1.0f;
    proj[1][0] = 0.0f;
    proj[1][1] = -0.02f;
    proj[1][2] = 0.0f;
    proj[1][3] = 1.0f;
    proj[2][0] = 0.0f;
    proj[2][1] = 0.0f;
    proj[2][2] = 1.0f;
    proj[2][3] = -1.0f;
    proj[3][0] = 0.0f;
    proj[3][1] = 0.0f;
    proj[3][2] = 0.0f;
    proj[3][3] = 1.0f;
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(3);
    Material::UseNone(-1);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
    
    float temp = 0.999999f;
    
    GXPosition3f32(0.0f, 0.0f, temp);
    GXColor4u8(0, 0, 0, 0);
    GXTexCoord2f32(0.0f, 0.0f);
    
    GXPosition3f32(100.0f, 0.0f, temp);
    GXColor4u8(0, 0, 0, 0);
    GXTexCoord2f32(0.0f, 0.0f);
    
    GXPosition3f32(0.0f, 100.0f, temp);
    GXColor4u8(0, 0, 0, 0);
    GXTexCoord2f32(0.0f, 0.0f);
    
    GXPosition3f32(100.0f, 100.0f, temp);
    GXColor4u8(0, 0, 0, 0);
    GXTexCoord2f32(0.0f, 0.0f);

    GXSetProjectionv(projection);
    GXSetCurrentMtx(0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
}

void View::ClearBuffer(int r, int g, int b, int alpha) {
    float oldProj[7];
    float proj[4][4];
    int dstAlpha;
    GXGetProjectionv(oldProj); // save projection

    // set new projection
    proj[0][0] = 0.02f;
    proj[0][1] = 0.0f;
    proj[0][2] = 0.0f;
    proj[0][3] = -1.0f;
    proj[1][0] = 0.0f;
    proj[1][1] = -0.02f;
    proj[1][2] = 0.0f;
    proj[1][3] = 1.0f;
    proj[2][0] = 0.0f;
    proj[2][1] = 0.0f;
    proj[2][2] = 1.0f;
    proj[2][3] = -1.0f;
    proj[3][0] = 0.0f;
    proj[3][1] = 0.0f;
    proj[3][2] = 0.0f;
    proj[3][3] = 1.0f;
    GXSetProjection(proj, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(3);

    Material::UseNone(-1);

    if (alpha < 0) {
        dstAlpha = 0;
    } else {
        dstAlpha = (alpha > 255) ? 255 : alpha;
    }

    GXSetDstAlpha(GX_ENABLE, dstAlpha);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_TRUE);

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);

    float temp = 0.999999f;

    GXPosition3f32(0.0f, 0.0f, temp);
    GXColor4u8(r, g, b, 255);
    GXTexCoord2f32(0.0f, 0.0f);

    GXPosition3f32(100.0f, 0.0f, temp);
    GXColor4u8(r, g, b, 255);
    GXTexCoord2f32(0.0f, 0.0f);

    GXPosition3f32(0.0f, 100.0f, temp);
    GXColor4u8(r, g, b, 255);
    GXTexCoord2f32(0.0f, 0.0f);

    GXPosition3f32(100.0f, 100.0f, temp);
    GXColor4u8(r, g, b, 255);
    GXTexCoord2f32(0.0f, 0.0f);
    
    // Restore projection
    GXSetProjectionv(oldProj);
    GXSetCurrentMtx(0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
}

void View::SetFogColour(uint colour) {
    fogColour = ByteReverse<uint>(colour);
}

void View::SetFogPlanes(float close, float far) {
	closeFogPlane = close;
	farFogPlane = far;
}

void View::SetFogIntensity(float arg1, float arg2) {
    unk2D0 = arg1;
    unk2D4 = arg2;
    unk2C4 = (farFogPlane * unk2D0 - closeFogPlane * unk2D4) / (farFogPlane - closeFogPlane);
    unk2C8 = ((farFogPlane * closeFogPlane) * (unk2D0 - unk2D4)) / (closeFogPlane - farFogPlane);
}

void View::SetCameraMatrix(Matrix* pCamera) {
    unk48 = *pCamera;
    unkC8.InverseSimple(&unk48);
	CalcMatrices();
}

void View::SetAspectRatio(float arg1, float arg2) {
	unk2AC = arg1;
	unk2B0 = arg2;
}

void View::TransformPoint2Dto3D(float arg1, float arg2, float arg3, Vector* out) {
    Vector centrePos;
    Vector point;
    float fVar1 = 2.0f * (float)tan(0.5f * unk2CC) * arg3;
    centrePos.Scale((Vector*)&unk48.data[2], arg3);
    point.x = fVar1 * ((arg1 / gDisplay.orthoXSize) - 0.5f);
    point.y = 0.8f * fVar1 * (0.5f - (arg2 / gDisplay.orthoYSize));
    point.z = 0.0f;
    point.ApplyRotMatrix(&point, &unk48);
    out->Add(&centrePos, &point);
    out->Add(unk48.Row3());
}

static float ortho_old[7] __attribute__ ((aligned (32)));

void View::OrthoBegin(void) {
    Mtx44 ortho;
    GXGetProjectionv(ortho_old);
    ortho.data[0][0] = 1.0f / (320.0f / unk2AC);
    ortho.data[0][1] = 0.0f;
    ortho.data[0][2] = 0.0f;
    ortho.data[0][3] = -1.0f * unk2AC;
    ortho.data[1][0] = 0.0f;
    ortho.data[1][1] = -1.0f / (256.0f / unk2B0);
    ortho.data[1][2] = 0.0f;
    ortho.data[1][3] = unk2B0;
    ortho.data[2][0] = 0.0f;
    ortho.data[2][1] = 0.0f;
    ortho.data[2][2] = 1.0f;
    ortho.data[2][3] = -1.0f;
    ortho.data[3][0] = 0.0f;
    ortho.data[3][1] = 0.0f;
    ortho.data[3][2] = 0.0f;
    ortho.data[3][3] = 1.0f;
    GXSetProjection(ortho.data, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(3);
    bOrtho = true;
}

void View::OrthoEnd(void) {
    GXSetProjectionv(ortho_old);
    GXSetCurrentMtx(0);
    bOrtho = false;
}
