#include "Dolphin/gx.h"
#include "common/View.h"
#include "common/StdMath.h"
#include "common/Blitter.h"

static int pCounterBlitsDrawn; // unused variable

void Blitter_InitModule(void) {
	pCounterBlitsDrawn = 0;
}

void Blitter_DeinitModule(void) {
	return;
}

void Blitter_Box::Draw(int count) {
    u8 red1;
    u8 green1;
    u8 blue1;
    u8 alpha1;
    Blitter_Box* pBox = this;
	
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	
    while(count-- > 0) {
		
        int red = (int)(pBox->color.x * 255.0f);
        int green = (int)(pBox->color.y * 255.0f);
        int blue = (int)(pBox->color.z * 255.0f);
        int alpha = (int)(pBox->color.w * 255.0f);
        float posX = pBox->origin.x;
        float posY = pBox->origin.y;
        float posZ = pBox->origin.z;
        float endX = posX + pBox->extent.x;
        float endY = posY + pBox->extent.y;
        float endZ = posZ + pBox->extent.z;
		
        GXBegin(GX_LINESTRIP, GX_VTXFMT1, 8);
		
        red1 = (u8)red;
        green1 = (u8)green;
        blue1 = (u8)blue;
        alpha1 = (u8)alpha;
        
        GXPosition3f32(posX, posY, posZ);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(posX, posY, endZ);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(posX, endY, endZ);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(posX, endY, posZ);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(posX, posY, posZ);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(endX, posY, posZ);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(endX, endY, posZ);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(posX, endY, posZ);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);

        GXBegin(GX_LINESTRIP, GX_VTXFMT1, 8);
        
        GXPosition3f32(endX, endY, endZ);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(endX, endY, posZ);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(endX, posY, posZ);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(endX, posY, endZ);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(endX, endY, endZ);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(posX, endY, endZ);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(posX, posY, endZ);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        
        GXPosition3f32(endX, posY, endZ);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);

        pBox++;
    }
}

void Blitter_Line3D::Draw(int count, float arg2) {
    Blitter_Line3D* pLines = this;
	
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	
    while (count-- > 0) {
        // Vertex 1 Colors
        u8 red = 255.0f * pLines->color.x;
        u8 green = 255.0f * pLines->color.y;
        u8 blue = 255.0f * pLines->color.z;
        u8 alpha = 255.0f * (pLines->color.w * arg2);
		
        // Vertex 2 Colors
        u8 red1 = 255.0f * pLines->color1.x;
        u8 green1 = 255.0f * pLines->color1.y;
        u8 blue1 = 255.0f * pLines->color1.z;
        u8 alpha1 = 255.0f * (pLines->color1.w * arg2);
        GXBegin(GX_LINES, GX_VTXFMT1, 2);
		
        // Vertex 1
        GXPosition3f32(pLines->point.x, pLines->point.y, pLines->point.z);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(0.0f, 0.0f);
		
        // Vertex 2
        GXPosition3f32(pLines->point1.x, pLines->point1.y, pLines->point1.z);
        GXColor4u8(red1, green1, blue1, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
        pLines++;
    }
}

// fake!
// this function is used to order floats
void CalcUV(float& u, float& v, int y, float default00, float default01, float c, float s) {
	u = (default00 - 0.5f) * c - (default01 - 0.5f) * s + 0.5f;
	v = (default00 - 0.5f) * s + (default01 - 0.5f) * c + 0.5f;
	volatile float t = 1.0f;
	t = -1.0f;
	t = PI2;
	volatile unsigned int x = 30;
	t = x;
}

void Blitter_Particle::Draw(int count) {
    Blitter_Particle* pParticle = this;
    
    float uv[4][2];
    static float defaultUV[4][2] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
	
    View* pView = View::GetCurrent();
	
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	
    for (int i = 0; i < count; i++) {
        float (* pUV)[2] = defaultUV;
        if (pParticle->angle) {
            float c = _table_cosf(pParticle->angle);
            float s = _table_sinf(pParticle->angle);
            pUV = uv;
            for (int y = 0; y < 4; y++) {
                float u = (defaultUV[y][0] - 0.5f) * c - (defaultUV[y][1] - 0.5f) * s + 0.5f;
                float v = (defaultUV[y][0] - 0.5f) * s + (defaultUV[y][1] - 0.5f) * c + 0.5f;
                
                uv[y][0] = u;
                uv[y][1] = v;
            }
        }

        float fVar10;
        float fVar12;
        float fVar14;
        float fVar9;
        float fVar11;
        float fVar13;
        
        fVar9 = pParticle->unk20 * pView->unk48.data[1][0];
        fVar10 = pParticle->unk20 * pView->unk48.data[0][0];
        fVar11 = pParticle->unk20 * pView->unk48.data[1][1];
        fVar12 = pParticle->unk20 * pView->unk48.data[0][1];
        fVar13 = pParticle->unk20 * pView->unk48.data[1][2];
        fVar14 = pParticle->unk20 * pView->unk48.data[0][2];
        
        u8 red = 255.0f * pParticle->color.x;
        u8 green = 255.0f * pParticle->color.y;
        u8 blue = 255.0f * pParticle->color.z;
        u8 alpha = 255.0f * pParticle->color.w;
        
        float dVar29 = fVar9 - fVar10;
        float dVar28 = fVar11 - fVar12;
        float dVar27 = fVar9 + fVar10;
        float dVar26 = fVar11 + fVar12;
        float dVar25 = fVar13 - fVar14;
        float dVar24 = fVar13 + fVar14;
        
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
        
        GXWGFifo.f32 = pParticle->pos.x + dVar29;
        GXWGFifo.f32 = pParticle->pos.y + dVar28;
        GXWGFifo.f32 = pParticle->pos.z + dVar25;
        GXColor4u8(red, green, blue, alpha);
        GXWGFifo.f32 = pUV[0][0];
        GXWGFifo.f32 = pUV[0][1];
        
        GXWGFifo.f32 = pParticle->pos.x - dVar27;
        GXWGFifo.f32 = pParticle->pos.y - dVar26;
        GXWGFifo.f32 = pParticle->pos.z - dVar24;
        GXColor4u8(red, green, blue, alpha);
        GXWGFifo.f32 = pUV[2][0];
        GXWGFifo.f32 = pUV[2][1];
        
        GXWGFifo.f32 = pParticle->pos.x + dVar27;
        GXWGFifo.f32 = pParticle->pos.y + dVar26;
        GXWGFifo.f32 = pParticle->pos.z + dVar24;
        GXColor4u8(red, green, blue, alpha);
        GXWGFifo.f32 = pUV[1][0];
        GXWGFifo.f32 = pUV[1][1];
        
        GXWGFifo.f32 = pParticle->pos.x - dVar29;
        GXWGFifo.f32 = pParticle->pos.y - dVar28;
        GXWGFifo.f32 = pParticle->pos.z - dVar25;
        GXColor4u8(red, green, blue, alpha);
        GXWGFifo.f32 = pUV[3][0];
        GXWGFifo.f32 = pUV[3][1];
        
        pParticle++;
    }
}

void Blitter_Particle::DrawFrames(int count, float arg1) {
    
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    
    Blitter_Particle* pParticle = this;
    
    View* pView = View::GetCurrent();
    Vector* viewVec = pView->unk48.Row0();
    Vector* viewVec1 = pView->unk48.Row1();
    for (int i = 0; i < count; i++) {
        float dVar29, dVar28, dVar27, dVar26, dVar25, dVar24;
        dVar29 = pParticle->unk20 * (viewVec1->x - viewVec->x);
        dVar28 = pParticle->unk20 * (viewVec1->y - viewVec->y);
        dVar27 = pParticle->unk20 * (viewVec1->x + viewVec->x);
        dVar26 = pParticle->unk20 * (viewVec1->y + viewVec->y);
        dVar25 = pParticle->unk20 * (viewVec1->z - viewVec->z);
        dVar24 = pParticle->unk20 * (viewVec1->z + viewVec->z);
        
        u8 red = 255.0f * pParticle->color.x;
        u8 green = 255.0f * pParticle->color.y;
        u8 blue = 255.0f * pParticle->color.z;
        u8 alpha = 255.0f * pParticle->color.w;
        
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
        
        GXWGFifo.f32 = pParticle->pos.x + dVar29;
        GXWGFifo.f32 = pParticle->pos.y + dVar28;
        GXWGFifo.f32 = pParticle->pos.z + dVar25;
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(arg1 * pParticle->unk24, 0.0f);
        
        GXWGFifo.f32 = pParticle->pos.x - dVar27;
        GXWGFifo.f32 = pParticle->pos.y - dVar26;
        GXWGFifo.f32 = pParticle->pos.z - dVar24;
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(arg1 * pParticle->unk24, 1.0f);
        
        GXWGFifo.f32 = pParticle->pos.x + dVar27;
        GXWGFifo.f32 = pParticle->pos.y + dVar26;
        GXWGFifo.f32 = pParticle->pos.z + dVar24;
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(arg1 * (pParticle->unk24 + 1), 0.0f);
        
        GXWGFifo.f32 = pParticle->pos.x - dVar29;
        GXWGFifo.f32 = pParticle->pos.y - dVar28;
        GXWGFifo.f32 = pParticle->pos.z - dVar25;
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(arg1 * (pParticle->unk24 + 1), 1.0f);
        
        pParticle++;
    }
}

void Blitter_Image::Draw(int count) {
    Blitter_Image* pImage = this;
    float projection[7];
	
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	
    GXGetProjectionv((float*)&projection);
	
	OrthoProject();
	
    while (count-- > 0) {
        float _startX = pImage->startX;
        float _startY = pImage->startY;
        float _endX = pImage->endX;
        float _endY = pImage->endY;
        u8 red = 255.0f * pImage->color.x;
        u8 green = 255.0f * pImage->color.y;
        u8 blue = 255.0f * pImage->color.z;
        u8 alpha = 255.0f * pImage->color.w;
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);

        GXPosition3f32(_startX, _startY, pImage->z);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pImage->uv0, pImage->uv1);

        GXPosition3f32(_startX, _endY, pImage->z);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pImage->uv0, pImage->uv3);

        GXPosition3f32(_endX, _startY, pImage->z);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pImage->uv2, pImage->uv1);

        GXPosition3f32(_endX, _endY, pImage->z);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pImage->uv2, pImage->uv3);
        pImage++;
    }

    GXSetProjectionv((float*)&projection);
    GXSetCurrentMtx(0);
}

void Blitter_UntexturedImage::Draw(int count) {
    u8 alpha;
	Blitter_UntexturedImage* pImage = this;
    float projection[7];
	
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	
	Material::UseNone(-1);
	
    GXGetProjectionv((float*)&projection);
	OrthoProject();
	
    while (count-- > 0) {
        float posX = pImage->pos.x;
        float posY = pImage->pos.y;
        float posZ = pImage->pos.z;
        float posW = pImage->pos.w;
        int red = pImage->color[3];
        alpha = (pImage->color[0] * 255) >> 7;
        u8 green = pImage->color[2];
        u8 blue = pImage->color[1];
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);

        GXPosition3f32(posX, posY, pImage->unk10);
        GXColor4u8(red, green, blue, alpha);

        GXPosition3f32(posX, posW, pImage->unk10);
        GXColor4u8(red, green, blue, alpha);

        GXPosition3f32(posZ, posY, pImage->unk10);
        GXColor4u8(red, green, blue, alpha);

        GXPosition3f32(posZ, posW, pImage->unk10);
        GXColor4u8(red, green, blue, alpha);
        pImage++;
    }

    GXSetProjectionv((float*)&projection);
    GXSetCurrentMtx(0);
}

void Blitter_TriStrip::Draw(int count, float arg2) {
    Blitter_TriStrip* pTriStrip = this;
    GXClearVtxDesc();
	
    // initiate GX
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, (u16)count);

    for (int i = 0; i < count; i++) {
        int red = (int)(pTriStrip[i].color.x * 255.0f);
        int green = (int)(pTriStrip[i].color.y * 255.0f);
        int blue = (int)(pTriStrip[i].color.z * 255.0f);
        int alpha = (int)(pTriStrip[i].color.w * 255.0f);
        GXPosition3f32(pTriStrip[i].pos.x, pTriStrip[i].pos.y, pTriStrip[i].pos.z);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pTriStrip[i].uv.x, pTriStrip[i].uv.y);
    }
}

void Blitter_TriStrip::DrawNoPerspective(int count, float arg2) {
    Draw(count, arg2);
}

void Blitter_TriFan::Draw2D(int count, float arg2) {
    Blitter_TriFan* pTriStrip = this;
	
    float projection[7];
    GXGetProjectionv((float*)&projection);
	
	OrthoProject();
	
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXBegin(GX_TRIANGLEFAN, GX_VTXFMT1, (u16)count);

    for (int i = 0; i < count; i++) {
        int red = (int)(pTriStrip[i].color.x * 255.0f);
        int green = (int)(pTriStrip[i].color.y * 255.0f);
        int blue = (int)(pTriStrip[i].color.z * 255.0f);
        int alpha = (int)(pTriStrip[i].color.w * 255.0f);
        GXPosition3f32(pTriStrip[i].pos.x, pTriStrip[i].pos.y, pTriStrip[i].pos.z);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pTriStrip[i].uv.x, pTriStrip[i].uv.y);
    }

    GXSetProjectionv((float*)&projection);
    GXSetCurrentMtx(0);
}
