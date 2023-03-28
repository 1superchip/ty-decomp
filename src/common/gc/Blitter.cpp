#include "Dolphin/gx.h"
#include "common/View.h"
#include "common/StdMath.h"
#include "common/Blitter.h"
#include "common/Material.h"

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
    GXSetVtxDesc(9, 1); // POS, DIRECT
    GXSetVtxDesc(11, 1); // CLR0, DIRECT
    GXSetVtxDesc(13, 1); // TEX0, DIRECT
	
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
		
        GXBegin(0xB0, 1, 8); // LINESTRIP
		
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

        GXBegin(0xB0, 1, 8); // LINESTRIP
        
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
    GXSetVtxDesc(9, 1); // POS, DIRECT
    GXSetVtxDesc(11, 1); // CLR0, DIRECT
    GXSetVtxDesc(13, 1); // TEX0, DIRECT
	
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
        GXBegin(0xA8, 1, 2); // LINES
		
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
    GXSetVtxDesc(9, 1); // POS, DIRECT
    GXSetVtxDesc(11, 1); // CLR0, DIRECT
    GXSetVtxDesc(13, 1); // TEX0, DIRECT
	
    for(int i = 0; i < count; i++) {
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
        
        GXBegin(0x98, 1, 4); // TRIANGLESTRIP
        
        WGPIPE.f = pParticle->pos.x + dVar29;
        WGPIPE.f = pParticle->pos.y + dVar28;
        WGPIPE.f = pParticle->pos.z + dVar25;
        GXColor4u8(red, green, blue, alpha);
        WGPIPE.f = pUV[0][0];
        WGPIPE.f = pUV[0][1];
        
        WGPIPE.f = pParticle->pos.x - dVar27;
        WGPIPE.f = pParticle->pos.y - dVar26;
        WGPIPE.f = pParticle->pos.z - dVar24;
        GXColor4u8(red, green, blue, alpha);
        WGPIPE.f = pUV[2][0];
        WGPIPE.f = pUV[2][1];
        
        WGPIPE.f = pParticle->pos.x + dVar27;
        WGPIPE.f = pParticle->pos.y + dVar26;
        WGPIPE.f = pParticle->pos.z + dVar24;
        GXColor4u8(red, green, blue, alpha);
        WGPIPE.f = pUV[1][0];
        WGPIPE.f = pUV[1][1];
        
        WGPIPE.f = pParticle->pos.x - dVar29;
        WGPIPE.f = pParticle->pos.y - dVar28;
        WGPIPE.f = pParticle->pos.z - dVar25;
        GXColor4u8(red, green, blue, alpha);
        WGPIPE.f = pUV[3][0];
        WGPIPE.f = pUV[3][1];
        
        pParticle++;
    }
}

void OrthoProject(void) {
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
    GXSetProjection((float*)&projection, 1);
    GXSetCurrentMtx(3);
}

void Blitter_Particle::DrawFrames(int count, float arg1) {
    
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1); // POS, DIRECT
    GXSetVtxDesc(11, 1); // CLR0, DIRECT
    GXSetVtxDesc(13, 1); // TEX0, DIRECT
    
    Blitter_Particle* pParticle = this;
    
    View* pView = View::GetCurrent();
    Vector* viewVec = pView->unk48.Row0();
    Vector* viewVec1 = pView->unk48.Row1();
    for(int i = 0; i < count; i++) {
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
        
        GXBegin(0x98, 1, 4); // TRIANGLESTRIP
        
        WGPIPE.f = pParticle->pos.x + dVar29;
        WGPIPE.f = pParticle->pos.y + dVar28;
        WGPIPE.f = pParticle->pos.z + dVar25;
        GXColor4u8(red, green, blue, alpha);
        WGPIPE.f = arg1 * pParticle->unk24;
        WGPIPE.f = 0.0f;
        
        WGPIPE.f = pParticle->pos.x - dVar27;
        WGPIPE.f = pParticle->pos.y - dVar26;
        WGPIPE.f = pParticle->pos.z - dVar24;
        GXColor4u8(red, green, blue, alpha);
        WGPIPE.f = arg1 * pParticle->unk24;
        WGPIPE.f = 1.0f;
        
        WGPIPE.f = pParticle->pos.x + dVar27;
        WGPIPE.f = pParticle->pos.y + dVar26;
        WGPIPE.f = pParticle->pos.z + dVar24;
        GXColor4u8(red, green, blue, alpha);
        WGPIPE.f = arg1 * (pParticle->unk24 + 1);
        WGPIPE.f = 0.0f;
        
        WGPIPE.f = pParticle->pos.x - dVar29;
        WGPIPE.f = pParticle->pos.y - dVar28;
        WGPIPE.f = pParticle->pos.z - dVar25;
        GXColor4u8(red, green, blue, alpha);
        WGPIPE.f = arg1 * (pParticle->unk24 + 1);
        WGPIPE.f = 1.0f;
        
        pParticle++;
    }
}

void Blitter_Image::Draw(int count) {
    Blitter_Image* pImage = this;
    float projection[7];
	
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1); // POS, DIRECT
    GXSetVtxDesc(11, 1); // CLR0, DIRECT
    GXSetVtxDesc(13, 1); // TEX0, DIRECT
	
    GXGetProjectionv((float*)&projection);
	
	OrthoProject();
	
    while (count-- > 0) {
        float posX = pImage->pos.x;
        float posY = pImage->pos.y;
        float posZ = pImage->pos.z;
        float posW = pImage->pos.w;
        u8 red = 255.0f * pImage->color.x;
        u8 green = 255.0f * pImage->color.y;
        u8 blue = 255.0f * pImage->color.z;
        u8 alpha = 255.0f * pImage->color.w;
        GXBegin(0x98, 1, 4); // TRIANGLESTRIP

        GXPosition3f32(posX, posY, pImage->unk10.x);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pImage->unk10.y, pImage->unk10.z);

        GXPosition3f32(posX, posW, pImage->unk10.x);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pImage->unk10.y, pImage->unk20);

        GXPosition3f32(posZ, posY, pImage->unk10.x);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pImage->unk10.w, pImage->unk10.z);

        GXPosition3f32(posZ, posW, pImage->unk10.x);
        GXColor4u8(red, green, blue, alpha);
        GXTexCoord2f32(pImage->unk10.w, pImage->unk20);
        pImage++;
    }

    GXSetProjectionv((float*)&projection);
    GXSetCurrentMtx(0);
}
void Blitter_UntexturedImage::Draw(int count) {
    u8 red;
	Blitter_UntexturedImage* pImage = this;
    float projection[7];
	
    // initiate GX
    GXClearVtxDesc();
    GXSetVtxDesc(9, 1); // POS, DIRECT
    GXSetVtxDesc(11, 1); // CLR0, DIRECT
	
	Material::UseNone(-1);
	
    GXGetProjectionv((float*)&projection);
	OrthoProject();
	
    while (count-- > 0) {
        float posX = pImage->pos.x;
        float posY = pImage->pos.y;
        float posZ = pImage->pos.z;
        float posW = pImage->pos.w;
        int alpha = pImage->color[3];
        red = (pImage->color[0] * 255) >> 7;
        u8 blue = pImage->color[2];
        u8 green = pImage->color[1];
        GXBegin(0x98, 1, 4); // TRIANGLESTRIP

        GXPosition3f32(posX, posY, pImage->unk10);
        GXColor4u8(alpha, blue, green, red);

        GXPosition3f32(posX, posW, pImage->unk10);
        GXColor4u8(alpha, blue, green, red);

        GXPosition3f32(posZ, posY, pImage->unk10);
        GXColor4u8(alpha, blue, green, red);

        GXPosition3f32(posZ, posW, pImage->unk10);
        GXColor4u8(alpha, blue, green, red);
        pImage++;
    }

    GXSetProjectionv((float*)&projection);
    GXSetCurrentMtx(0);
}

void Blitter_TriStrip::Draw(int count, float arg2) {
    Blitter_TriStrip* pTriStrip = this;
    GXClearVtxDesc();
	
    // initiate GX
    GXSetVtxDesc(9, 1); // POS, DIRECT
    GXSetVtxDesc(11, 1); // CLR0, DIRECT
    GXSetVtxDesc(13, 1); // TEX0, DIRECT
    GXBegin(0x98, 1, (u16)count);

    for(int i = 0; i < count; i++) {
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
    GXSetVtxDesc(9, 1); // POS, DIRECT
    GXSetVtxDesc(11, 1); // CLR0, DIRECT
    GXSetVtxDesc(13, 1); // TEX0, DIRECT
    GXBegin(0xA0, 1, (u16)count);

    for(int i = 0; i < count; i++) {
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