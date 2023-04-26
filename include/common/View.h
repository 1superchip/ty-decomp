#ifndef COMMON_VIEW
#define COMMON_VIEW

#include "types.h"
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
	Vector unk0;
	Vector unk10;
	Vector unk20;
	char unk30[4];
	bool bDisableZWrite;
	bool bOrtho;
    int unk38;
    int unk3C;
    int unk40;
    int unk44;
	Matrix unk48;
	Matrix unk88;
	Matrix unkC8;
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
    uint unk29C;
    char unk2A0[12];
    float unk2AC;
    float unk2B0;
    float unk2B4;
    float unk2B8;
    float unk2BC;
    float unk2C0;
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
	void SetCameraLookAt(Vector*, Vector*);
	void SetCameraRollAndLookAt(Vector*, Vector*, float);
	void SetProjection(float, float, float);
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
};

#endif // COMMON_VIEW