#ifndef COMMON_VIEW
#define COMMON_VIEW

#include "types.h"
#include "common/Texture.h"
#include "common/Vector.h"
#include "common/Matrix.h"
#include "common/DirectLight.h"

// should this go in Vector.h?
struct IntVector {
    int x;
    int y;
    int z;
    int w;
};

struct View {
    Vector mCamPos; // Camera Position
    Vector mCamTarget; // Target position for Camera to lookat
    Vector mFwdDir; // Forward Direction of the Camera
    char unk30[4];
    bool bDisableZWrite;
    bool bOrtho;
    int unk38;
    int unk3C;
    int unk40;
    int unk44;
    Matrix unk48; // Camera view matrix?
    Matrix unk88;
    Matrix unkC8; // View matrix? InverseSimple(unk48)
    Matrix unk108;
    Matrix unk148;
    char unk188[64];
    Matrix unk1C8;
    Matrix unk208;
    Matrix unk248;
    bool unk288;
    float unk28C;
    float unk290;
    char unk294[8];
    Texture* unk29C;
    char unk2A0[12];
    float unk2AC;
    float unk2B0;
    float unk2B4;
    float unk2B8;
    float unk2BC; // farZ
    float unk2C0; // nearZ
    float unk2C4;
    float unk2C8;
    float unk2CC;
    float unk2D0;
    float unk2D4;
    float closeFogPlane;
    float farFogPlane;
    uint fogColour;
    DirectLight* pLight;
    int unk2E8;
    
    void Init(void);
    void Init(float, float, float, float);
    void CalcMatrices(void);
    static void SetCameraMatrixLookAt(Matrix* pMatrix, Vector* pCamPos, Vector* pCamTarget);
    void SetCameraLookAt(Vector* pCamPos, Vector* pCamTarget);
    void SetCameraRollAndLookAt(Vector*, Vector*, float);
    void SetProjection(float fov, float, float);
    void SetLocalToWorldMatrix(Matrix*);
    void SetDirectLight(DirectLight*);
    void Use(void);
    float TransformPoint(IntVector*, Vector*);
    void ClearZBuffer(void);
    void ClearBuffer(int, int, int, int);
    void SetFogColour(uint);
    void SetFogPlanes(float, float);
    void SetFogIntensity(float, float);
    void SetCameraMatrix(Matrix*);
    void SetAspectRatio(float xRatio, float yRatio);
    void TransformPoint2Dto3D(float, float, float, Vector*);
    void OrthoBegin(void);
    void OrthoEnd(void);
    
    static void InitModule(void);
    static void DeinitModule(void);
    static View* pCurrentView;
    static View* pDefaultView;
    static View* GetCurrent(void) {
        return pCurrentView;
    }
    static View* GetDefault(void) {
        return pDefaultView;
    }
};

inline void OrthoProject(void) {
    float projection[4][4];
    float unk2B0 = View::GetCurrent()->unk2B0;
    float unk2AC = View::GetCurrent()->unk2AC;
    // ortho
    projection[0][0] = 1.0f / (320.0f / unk2AC); // 0x24
    projection[0][1] = 0.0f; // 0x28
    projection[0][2] = 0.0f; // 0x2C
    projection[0][3] = -1.0f * unk2AC; // 0x30
    projection[1][0] = 0.0f; // 0x34
    projection[1][1] = -1.0f / (256.0f / unk2B0);
    projection[1][2] = 0.0f;
    projection[1][3] = unk2B0;
    projection[2][0] = 0.0f;
    projection[2][1] = 0.0f;
    projection[2][2] = 1.0f;
    projection[2][3] = -1.0f;
    projection[3][0] = 0.0f;
    projection[3][1] = 0.0f;
    projection[3][2] = 0.0f;
    projection[3][3] = 1.0f;
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    GXSetCurrentMtx(3);
}

#endif // COMMON_VIEW
