#include "common/Font.h"
#include "common/PtrListDL.h"
#include "common/Blitter.h"
#include "common/Str.h"
#include "common/FileSys.h"
#include "common/Heap.h"
#include "common/View.h"

// TODO! Document color format. Color parameters are not RGBA 
// TODO! Document FontJustify

extern "C" void memcpy(void*, void*, int);
extern "C" void strcpy(char*, char*);
extern "C" int stricmp(char*, char*);

bool Font::initialised;
static PtrListDL<Font> fontList;
static int pFontMenu;
static int pDebugShowFontWrapArea = 0;
static int pDebugCrossHair;
static bool showFontWrapArea = false;

// Several unused parameters
Font* Font::Create(char* pName, char* pCharMap, float, float aspectRatio, float) {
	return Create(pName);
}

/// @brief Returns an instance of a font with the name of pName
/// @param pName Name of font to create
/// @return Pointer to Font structure
Font* Font::Create(char* pName) {
    int size;
    Font* retFont;
    retFont = Find(pName);
    if (retFont != NULL) {
        retFont->referenceCount++;
    } else {
        retFont = fontList.GetNextEntry();
        void *fontFile = FileSys_Load(Str_Printf("%s.gfn", pName), 0, 0, -1);
        *((int*)fontFile + 10) = (int)fontFile + *((int*)fontFile + 10);
        *(FontData*)retFont = *(FontData*)fontFile; // not a struct copy?
        retFont->pChars = (FontCharData*)Heap_MemAlloc(0x2000);
        memcpy((void*)retFont->pChars, (void*)*((int*)fontFile + 10), sizeof(FontCharData) * 256);
        Heap_MemFree(fontFile);
        strcpy(retFont->name, pName);
        retFont->referenceCount = 1;
        retFont->pFontMaterial = Material::Create(pName);
        retFont->pFontMaterial->SetFlags(2);
        retFont->pFontMaterial->SetFlags(4);
        retFont->pFontMaterial->unk5C = 0.9f; // float might be wrong
    }
    return retFont;
}

/// @brief Iterates through the font list to find a font with the name of pName
/// @param pName Name of Font to search for
/// @return NULL if font has not been created or the Font pointer
Font* Font::Find(char* pName) {
    Font** ppFonts = fontList.GetMem();
    while (*ppFonts != NULL) {
        if (stricmp((*ppFonts)->name, pName) == 0) {
            return *ppFonts;
        }
        ppFonts++;
    }
    return NULL;
}

void Font::Destroy(void) {
	if (--referenceCount == 0) {
		pFontMaterial->Destroy();
		if ((void*)unk20 != NULL) {
			Heap_MemFree((void*)unk20);
		}
		Heap_MemFree((void*)pChars);
		fontList.Destroy(this);
	}
}

void Font::InitModule(void) {
	fontList.Init(7, sizeof(Font));
	initialised = true;
	pFontMenu = 0;
	pDebugShowFontWrapArea = 0;
	pDebugCrossHair = 0;
}

void Font::DeinitModule(void) {
	if (*fontList.pMem != NULL) {
		Font** ppFonts = fontList.pMem;
		while (*ppFonts != NULL) {
			ppFonts++;
		}
	}
	fontList.Deinit();
	initialised = false;
}

float Font::CalcLength(char* string) {
	float length = 0.0f;
    u8* str = (u8*)string;
	while (*str != '\0') {
        int c = *str;
		if (c > ' ' && c < 256) {
			if (pChars[c].unk1C) {
				float charLength = pChars[c].unk18;
				length += charLength + unk8;
			} else {
				length += unkC;
			}
		} else {
			length += unkC;
		}
		str++;
	}
	return length;
}

int GetScreenData(float x) {
    return (int)(16.0f * x);
}

float ConvertLongLong2Float(s64 x) {
    return (float)x;
}

/// @brief 2D - Draws a string on the screen 
/// @param pString String to draw
/// @param xPos X position
/// @param yPos Y position
/// @param xScale X scale
/// @param yScale Y scale
/// @param justify 
/// @param color Color of string
void Font::DrawText(char* pString, float xPos, float yPos, float xScale, float yScale,
        FontJustify justify, int color) {
    float projection[7];
    pFontMaterial->Use();
    View* pView = View::GetCurrent();
    xScale *= pView->unk2B4;
    yScale *= pView->unk2B8;
    u64 posX = GetScreenData(pView->unk28C + (pView->unk2B4 * xPos));
    u64 posY = GetScreenData(pView->unk290 + (pView->unk2B8 * yPos));
    u64 scaleY = GetScreenData(mHeight * yScale);

    float stringLength = 0.0f;
    u8* str = (u8*)pString;
	while (*str != '\0') {
        int c = *str;
		if (c > ' ' && c < 256) {
			if (pChars[c].unk1C) {
				stringLength += pChars[c].unk18 + unk8;
			}
		} else {
            stringLength += unkC;
        }
		str++;
	}
    stringLength *= xScale;
    switch (justify) {
        case 1:
            posX -= GetScreenData(stringLength * 0.5f);
            break;
        case 2:
            posX -= GetScreenData(stringLength);
            break;
        case 4:
            posY -= scaleY / 2;
            break;
        case 5:
            posX -= GetScreenData(stringLength * 0.5f);
            posY -= scaleY / 2;
            break;
        case 6:
            posX -= GetScreenData(stringLength);
            posY -= scaleY / 2;
            break;
        case 8:
            posY += scaleY;
            break;
        case 9:
            posX -= GetScreenData(stringLength * 0.5f);
            posY += scaleY;
            break;
        case 10:
            posX -= GetScreenData(stringLength);
            posY += scaleY;
            break;
    }
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_NRM, GX_NONE);
    GXSetZMode(1, GX_ALWAYS, 1);
    color = (((color >> 0x18 & 0xFF) * 255) >> 7) << 0x18 |
            (((color >> 0x10 & 0xFF) * 255) >> 7) << 0x10 |
            (((color >> 8 & 0xFF) * 255) >> 7) << 8 |
            (((color & 0xFF) * 255) >> 7) | 0xFF000000;
    GXGetProjectionv((float*)&projection);
    OrthoProject();

    float dVar19;
    float dVar17 = ConvertLongLong2Float((s64)posX >> 4);
    float dVar18 = ConvertLongLong2Float((s64)posY >> 4);
    dVar19 = dVar18 + ConvertLongLong2Float((s64)scaleY >> 4);

    u8* text = (u8*)pString;
    while (*text != '\0') {
        if (*text > ' ' && *text < 256) {
            int index = *text;
            FontCharData* charData = &pChars[index];
            if (charData->unk1C) {
                u8* colorData = (u8*)&color;
                float charLength = charData->unk18 * xScale;
                
                float u = charData->unk8;
                float v = charData->unkC;
                float char10 = charData->unk10;
                float char14 = charData->unk14;
                GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
                WGPIPE.f = dVar17;
                WGPIPE.f = dVar18;
                WGPIPE.f = 0.0f;
                WGPIPE.c = colorData[3];
                WGPIPE.c = colorData[2];
                WGPIPE.c = colorData[1];
                WGPIPE.c = colorData[0];
                WGPIPE.f = u;
                WGPIPE.f = v;
                
                WGPIPE.f = dVar17;
                WGPIPE.f = dVar19 + 1.0f;
                WGPIPE.f = 0.0f;
                WGPIPE.c = colorData[3];
                WGPIPE.c = colorData[2];
                WGPIPE.c = colorData[1];
                WGPIPE.c = colorData[0];
                WGPIPE.f = u;
                WGPIPE.f = char14;

                WGPIPE.f = dVar17 + charLength;
                WGPIPE.f = dVar18;
                WGPIPE.f = 0.0f;
                WGPIPE.c = colorData[3];
                WGPIPE.c = colorData[2];
                WGPIPE.c = colorData[1];
                WGPIPE.c = colorData[0];
                WGPIPE.f = char10;
                WGPIPE.f = v;
                
                WGPIPE.f = dVar17 + charLength;
                WGPIPE.f = dVar19 + 1.0f;
                WGPIPE.f = 0.0f;
                WGPIPE.c = colorData[3];
                WGPIPE.c = colorData[2];
                WGPIPE.c = colorData[1];
                WGPIPE.c = colorData[0];
                WGPIPE.f = char10;
                WGPIPE.f = char14;
                dVar17 += (charData->unk18 + unk8) * xScale;
            } else {
                dVar17 += unkC * xScale;
            }
        } else {
            dVar17 += unkC * xScale;
        }
        text++;
    }
    GXSetZMode(1, GX_LEQUAL, 1); // reset Z mode
    GXSetProjectionv((float*)&projection);
    GXSetCurrentMtx(0);
}

float Font::GetHeight(void) {
	return mHeight;
}

/// @brief Draws text in 3D space
/// @param pText String to draw
/// @param pPos Postion of string
/// @param scaleX X scale
/// @param scaleY Y scale
/// @param justify Justify value
/// @param pCharPositions (Optional, pass NULL if unneeded) / Individual character positions
/// @param numCharPositions Total character positions
/// @param charPosIndex Srart index of character positions
/// @param pCharColors (Optional, pass NULL if unneeded) / Individual character colors
/// @param numCharColors Total character colors
/// @param charColorIndex Start index of character colors
/// @return Number of characters?
int Font::DrawText3d(char* pText, Vector* pPos, float scaleX, float scaleY, FontJustify justify,
        Vector* pCharPositions, int numCharPositions, int charPosIndex, uint* pCharColors, 
        int numCharColors, int charColorIndex) {
    Vector topLeft;
    Vector charPos[4];
    float uv[8];
    int numContribChars = 0;
    pFontMaterial->Use();
	float sx = 0.0f;
    u8* text = (u8*)pText;
	while (*text != '\0') {
        int c = *text;
		if (c > ' ' && c < 256) {
            FontCharData* charData = &pChars[c];
			if (charData->unk1C) {
				sx += charData->unk18 + unk8;
                numContribChars++;
			} else {
				sx += unkC;
			}
		} else {
			sx += unkC;
		}
		text++;
	}
    if (numContribChars == 0) {
        return 0;
    }
    sx *= scaleX;
    float sy = mHeight * scaleY;
    Vector sp8;
    topLeft = *pPos;
    switch(justify) {
        case 1:
            topLeft.x -= 0.5f * sx;
            break;
        case 2:
            topLeft.x -= sx;
            break;
        case 4:
            topLeft.y += 0.5f * sy;
            break;
        case 5:
            topLeft.x -= 0.5f * sx;
            topLeft.y += 0.5f * sy;
            break;
        case 6:
            topLeft.x -= sx;
            topLeft.y += 0.5f * sy;
            break;
        case 8:
            topLeft.y += sy;
            break;
        case 9:
            topLeft.x -= 0.5f * sx;
            topLeft.y += sy;
            break;
        case 10:
            topLeft.x -= sx;
            topLeft.y += sy;
            break;
    }
    float f26 = unk8 * scaleX;
    text = (u8*)pText;
    int colorR;
    int colorG;
    int colorB;
    int colorA;
    int charIndex = charPosIndex;
    while (*text != '\0') {
        if (*text > ' ' && *text < 256) {
            int index = *text;
            FontCharData* charData = &pChars[index];
            if (charData->unk1C) {
                float scale = charData->unk18 * scaleX;
                if (pCharPositions != NULL) {
                    charIndex %= numCharPositions;
                    charPos[0].Add(&topLeft, &pCharPositions[charIndex]);
                    charIndex++;
                } else {
                    charPos[0] = topLeft;
                }
                charPos[1].Set(charPos[0].x + scale, charPos[0].y, charPos[0].z);
                charPos[2].Set(charPos[0].x, charPos[0].y - sy, charPos[0].z);
                charPos[3].Set(charPos[1].x, charPos[2].y, charPos[0].z);

                uv[0] = charData->unk8;
                uv[1] = charData->unkC;
                uv[2] = charData->unk10;
                uv[5] = charData->unk14;

                uv[3] = charData->unkC;
                uv[4] = charData->unk8;
                uv[6] = charData->unk10;
                uv[7] = charData->unk14;

                if (pCharColors != NULL) {
                    charColorIndex %= numCharColors;
                    colorR = pCharColors[charColorIndex] & 0xFF;
                    colorG = (pCharColors[charColorIndex] >> 8) & 0xFF;
                    colorB = (pCharColors[charColorIndex] >> 0x10) & 0xFF;
                    colorA = (pCharColors[charColorIndex] >> 0x18) & 0xFF;
                    charColorIndex++;
                } else {
                    colorR = colorG = colorB = colorA = 0x80;
                }
                GXClearVtxDesc();
                GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
                GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
                GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
                GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT1, 4);
                // float* uvs = (float*)uv;
                u8 colors[4];
                colors[0] = (colorR * 255) >> 7;
                colors[1] = (colorG * 255) >> 7;
                colors[2] = (colorB * 255) >> 7;
                colors[3] = (colorA * 255) >> 7;
                for(int i = 0; i < 4; i++) {
                    GXPosition3f32(charPos[i].x, charPos[i].y, charPos[i].z);
                    GXColor4u8(colors[0], colors[1], colors[2], colors[3]);
                    GXTexCoord2f32(uv[(i * 2)], uv[(i * 2) + 1]);
                    // uvs += 2;
                }
                topLeft.x += scale + f26;
            } else {
                topLeft.x += unkC * scaleX;
            }
        } else {
            topLeft.x += unkC * scaleX;
        }
        text++;
    }
    return numContribChars;
}

char* Font::GetTextSegment(char** strings, float f1, TextSegmentInfo* textInfo) {
    u8* pString = *(u8**)strings;
    u8* tempString = pString;
    u8* out;
    bool r31 = false;
    char* r30 = 0;
    float f31 = 0.0f;
    float f3 = 0.0f;
    while (true) {
        if (((*tempString == ' ' || *tempString == '\n') || *tempString == 0) && (f3 <= f1 || !r30)) {
            f31 = f3;
            r30 = (char*) tempString;
            if (*tempString == '\n') {
                r31 = true;
                break;
            }
        }
        if (*tempString == 0 || (f3 > f1 && r30)) {
            if (!r30) {
                r30 = (char*) tempString;
            }
            break;
        }
        /*float length;
        if (*tempString > 0x20 && *tempString <= 255 && pChars[*tempString].unk1C ? true : false) {
            length = pChars[*tempString].unk18 + unk8;
        } else {
            length = unkC;
        }
        f3 += length;*/
        f3 += (*tempString > 0x20 && *tempString <= 255 && pChars[*tempString].unk1C) ? 
                    pChars[*tempString].unk18 + unk8 : unkC;
        tempString++;
    }
    if (pString == NULL) {
        return NULL;
    }
    char* copy = Str_CopyString((char*)pString, (u8*)r30 - pString);
    if (r31) {
        r30++;
    }
    if (r31) {
        *strings = r30;
    } else {
        out = (u8*)r30;
        while (*out == ' ') {
            out++;
        }
        *strings = (char*)out;
    }

    if (textInfo != NULL) {
        textInfo->unk0 = copy;
        textInfo->unk4 = f31;
        textInfo->unk8 = r31;
    }
    return copy;
}

void Font::DrawTextWrapped(char* pText, float xScale, float yScale, Vector* pPos,
        FontJustify justify, uint color, int flags) {
    float xPos = pPos->x;
    float yPos = pPos->y;
    float zPos = pPos->z;
    float wPos = pPos->w; // not w pos?

    if (flags & 4) {
        xPos -= zPos * 0.5f;
    }
    if (flags & 8) {
        yPos -= wPos * 0.5f;
    }
    if (showFontWrapArea || flags & 1) {
        Blitter_UntexturedImage image;
        image.pos.x = xPos;
        image.pos.y = yPos;
        image.pos.z = xPos + zPos;
        image.pos.w = yPos + wPos;
        // fake?
        *((int*)&image.color + 1) = 0x40800000;
        *(int*)&image.color = 0x40800000;
        image.Draw(1);
    }
    int r24 = (u8)((flags & 2));
    TextSegmentInfo info[100];
    int r29;
    while (true) {
        char* text = pText;
        // r29 = 0;
        float max = 0.0f;
        for (r29 = 0; *text != '\0' && r29 < 100; r29++) {
            if (!GetTextSegment(&text, zPos / xScale, &info[r29])) {
                break;
            }
            info[r29].unk4 *= xScale;
            if (info[r29].unk4 > max) {
                max = info[r29].unk4;
            }
        }
        if (!r24) break;
        if (max > zPos) {
            xScale *= zPos / max;
            yScale *= zPos / max;
        } else {
            if (!((r29 * mHeight) * yScale > wPos)) break;
            xScale *= 0.975f;
            yScale *= 0.975f;
        }
    }

    float dVar17 = mHeight * yScale;
    bool bSave_fieldC = false;
    float dVar16 = mHeight;
    float dVar18;
    switch(justify) {
        case 3:
            bSave_fieldC = 1;
            justify = (FontJustify)0;
        case 0:
            dVar16 = 0.0f;
            wPos = 0.0f;
            break;
        case 1:
            wPos = 0.0f;
            dVar16 = zPos / 2.0f;
            break;
        case 2:
            dVar16 = zPos;
            wPos = 0.0f;
            break;
        case 7:
            bSave_fieldC = 1;
            justify = (FontJustify)4;
        case 4:
            dVar16 = 0.0f;
            wPos = (wPos - ((r29 - 1) * dVar17)) / 2.0f;
            break;
        case 5:
            dVar16 = zPos / 2.0f;
            wPos = (wPos - ((r29 - 1) * dVar17)) / 2.0f;
            break;
        case 6:
            dVar16 = zPos;
            wPos = (wPos - ((r29 - 1) * dVar17)) / 2.0f;
            break;
        case 11:
            bSave_fieldC = 1;
            justify = (FontJustify)8;
        case 8:
            dVar16 = 0.0f;
            wPos = wPos - (((r29 + 1) * dVar17));
            break;
        case 9:
            dVar16 = zPos / 2.0f;
            wPos = wPos - (((r29 + 1) * dVar17));
            break;
        case 10:
            dVar16 = zPos;
            wPos = wPos - (((r29 + 1) * dVar17));
            break;
        default:
            dVar16 = 0.0f;
            wPos = 0.0f;
            break;
    }

    xPos += dVar16;
    for(int i = 0; i < r29; i++) {
        float unkC_save = unkC;
        if (bSave_fieldC && !info[i].unk8) {
            int spaceCount = 0;
            int charIndex = 0;
            while (info[i].unk0[charIndex] != '\0') {
                if (info[i].unk0[charIndex] == ' ') {
                    spaceCount++;
                }
                charIndex++;
            }
            if (spaceCount) {
                unkC += ((zPos - info[i].unk4) / (float)spaceCount) / xScale;
            }
        }
        DrawText(info[i].unk0, xPos, yPos + wPos, xScale, yScale, justify, color);
        if (bSave_fieldC && !info[i].unk8) {
            unkC = unkC_save;
        }
        yPos += dVar17;
    }
}

void Font::DrawTextWrapped3d(char* pText, float f1, float f2, Vector* pPos,
        float f3, float f4, FontJustify justify, uint color, int flags) {
    TextSegmentInfo segs[100];
    int r29;
    int r24 = (u8)((flags & 2));
    while (true) {
        char* pTemp = pText;
        float max = 0.0f;
        for (r29 = 0; *pTemp != '\0' && r29 < 100; r29++) {
            if (!GetTextSegment(&pTemp, f3 / f1, &segs[r29])) {
                break;
            }
            segs[r29].unk4 *= f1;
            if (segs[r29].unk4 > max) {
                max = segs[r29].unk4;
            }
        }
        if (!r24) break;
        if (max > f3) {
            f1 *= f3 / max;
            f2 *= f3 / max;
        } else {
            if (!((r29 * mHeight) * f2 > f4)) break;
            f1 *= 0.975f;
            f2 *= 0.975f;
        }
    }

    float dVar17 = mHeight * f2;
    bool r27 = false;
    float dVar16 = f1;
    float dVar18;
    switch(justify) {
        case 3:
            r27 = true;
            justify = (FontJustify)0;
        case 0:
            dVar16 = 0.0f;
            dVar18 = 0.0f;
            break;
        case 1:
            dVar18 = 0.0f;
            dVar16 = f3 / 2.0f;
            break;
        case 2:
            dVar16 = f3;
            dVar18 = 0.0f;
            break;
        case 7:
            r27 = true;
            justify = (FontJustify)4;
        case 4:
            dVar16 = 0.0f;
            dVar18 = (f4 - ((r29 - 1) * dVar17)) / 2.0f;
            break;
        case 5:
            dVar16 = f3 / 2.0f;
            dVar18 = (f4 - ((r29 - 1) * dVar17)) / 2.0f;
            break;
        case 6:
            dVar16 = f3;
            dVar18 = (f4 - ((r29 - 1) * dVar17)) / 2.0f;
            break;
        case 11:
            r27 = 1;
            justify = (FontJustify)8;
        case 8:
            dVar16 = 0.0f;
            dVar18 = f4 - (((r29 - 1) * dVar17));
            break;
        case 9:
            dVar16 = f3 / 2.0f;
            dVar18 = f4 - (((r29 - 1) * dVar17));
            break;
        case 10:
            dVar16 = f3;
            dVar18 = f4 - (((r29 - 1) * dVar17));
            break;
        default:
            dVar16 = 0.0f;
            dVar18 = 0.0f;
            break;
    }
    Vector pos;
    pos.x = pPos->x + dVar16;
    pos.y = pPos->y - dVar18;
    pos.z = pPos->z;
    if (flags & 4) {
        pos.x -= f3 * 0.5f;
    }
    if (flags & 8) {
        pos.y += f4 * 0.5f;
    }

    if (showFontWrapArea || flags & 1) {
        Blitter_Box area;
        area.origin = *pPos;
        if (flags & 4) {
            area.origin.x -= dVar16;
        }
        if (flags & 8) {
            area.origin.y += dVar18;
        }
        area.extent.Set(f3, -f4, 0.0099999998f, 0.0f);
        area.color.Set(128.0f, 0.0f, 0.0f, 128.0f);
        area.color1.Set(128.0f, 0.0f, 0.0f, 128.0f);
        area.Draw(1);
    }
    
    for(int i = 0; i < r29; i++) {
        float unkC_save = unkC;
        if (r27 && !segs[i].unk8) {
            int spaceCount = 0;
            int c = 0;
            while (segs[i].unk0[c] != '\0') {
                if (segs[i].unk0[c] == ' ') {
                    spaceCount++;
                }
                c++;
            }
            if (spaceCount) {
                unkC += ((f3 - segs[i].unk4) / (float)spaceCount) / f1;
            }
        }
        DrawText3d(segs[i].unk0, &pos, f1, f2, justify, NULL, 0, 0, &color, 1, 0);
        if (r27 && !segs[i].unk8) {
            unkC = unkC_save;
        }
        pos.y -= dVar17;
    }
}

void Font::RenderChars(Font::TextCharStrip* pCharStrips, int count) {
    Blitter_TriStrip strip[4];
    for (int i = 0; i < count; i++) {
        for(int triIndex = 0; triIndex < 4; triIndex++) {
            CharStripVert* pCharVert = &pCharStrips[i].verts[triIndex];
            strip[triIndex].pos.Set(pCharVert->x, pCharVert->y, pCharVert->z);
            float a = (pCharVert->color) & 0xFF;
            float b = (pCharVert->color >> 0x8) & 0xFF;
            float g = (pCharVert->color >> 0x10) & 0xFF;
            float r = (pCharVert->color >> 0x18) & 0xFF;
            strip[triIndex].color.Set(
                a * (1.0f / 128.0f),
                b * (1.0f / 128.0f),
                g * (1.0f / 128.0f),
                r * (1.0f / 128.0f));
            strip[triIndex].uv.x = pCharVert->u;
            strip[triIndex].uv.y = pCharVert->v;
        }
        strip[0].Draw(4, 1.0f);
    }
}

void Font::BuildLines(char* pString, float f1, float f2, TextLineData* lineData) {
    float f6 = 1.0f;
    int r9;
    start:
    u8* text = (u8*)pString;
    r9 = 0;
    float f4 = 0.0f;
    float f7 = 0.0f;
    while (*text != 0) {
        int r11 = 0;
        int r12 = 1;
        int r31 = 0;
        float f8 = 0.0f;
        float f9 = 0.0f;
        float f10 = 0.0f;
        u8* r30 = NULL;
        lineData->data[r9].unk0 = (char*)text;
        while (f8 < f1) {
            if (*text <= 0x20 || !pChars[*text].unk1C) {
                if (*text == 0 || *text == '\n') {
                    if (*text == '\n') {
                        r31 = 1;
                    }
                    r30 = text;
                    f10 = f8;
                    f9 = f4;
                    break;
                }
                if (r11) {
                    f10 = f8;
                    f9 = f4;
                    r11 = 0;
                    r30 = text;
                }
                float m = unkC * f6;
                f8 += m;
                f4 += m;
            } else {
                r11 = 1;
                r12 = 0;
                float m = (pChars[*text].unk18 + unk8) * f6;
                f8 += m;
                f4 += m;
            }
            text++;
        }
        if (r12) {
            lineData->data[r9].unk4 = lineData->data[r9].unk0;
            lineData->data[r9].unk8 = 0.0f;
        } else {
            if (!r30) {
                // not in PS2?
                while (*text > 0x20 && pChars[*text].unk1C) {
                    f8 += (unk8 + pChars[*text].unk18) * f6;
                    text++;
                }
                f6 *= f1 / (1.0f + f8);
                goto start;
            } else {
                lineData->data[r9].unk4 = (char*)r30;
                lineData->data[r9].unk8 = f10;
            }
        }
        f4 = f9;
        text = r30;
        if (r31) {
            text = r30 + 1;
        } else {
            while (*text && *text <= 0x20) {
                text++;
                f4 += unkC * f6;
            }
        }
        f7 += mHeight * f6;
        r9++;
        if (f7 > f2) {
            if ((f4 < (2.0f * f1)) && (f2 < mHeight * 2.0f)) {
                f6 *= f1 / (1.0f + f4);
                goto start;
            }
            f6 *= (float)r9 / (float)(r9 + 1);
            goto start;
        }
    }
    lineData->y = f6;
    lineData->x = r9;
    lineData->z = f7;
}

/// @brief Draws a string supports passing a matrix
/// @param pText String to draw
/// @param f1 
/// @param f2 
/// @param pMatrix Rotation and translation matrix
/// @param r6 
/// @param color Color of string
/// @param stripFunc0 
/// @param stripFunc1 
/// @return 
float Font::DrawString(char* pText, float f1, float f2, Matrix* pMatrix, int r6, 
        uint color, void (*stripFunc0)(Font::TextCharStrip*, int), void (*stripFunc1)(Font::TextCharStrip*, int)) {
    Font::TextCharStrip chars[128];
    TextLineData tld;
    TextLineData* pTld = &tld; // this seems to get it a lot closer

    float f29;
    float f28;
    float f27;
    if (View::GetCurrent()->bOrtho) {
        f29 = 1.0f;
    } else {
        f29 = -1.0f;
    }
    pFontMaterial->Use();
    
    BuildLines(pText, f1, f2, &tld);
    f28 = (mHeight * tld.y) * f29;
    f27 = (-f2 * 0.5f) * f29;
    switch (r6) {
        case 4:
        case 5:
        case 6:
            f27 += ((f2 - tld.z) * 0.5f) * f29;
            break;
        case 8:
        case 9:
        case 10:
            f27 += (f2 - tld.z) * f29;
            break;
    }
    for(int i = 0; i < tld.x; i++) {
        LineDataSub* pSubData = &tld.data[i];
        if (pSubData->unk0 != pSubData->unk4) {
            float f5;
            float f6;
            float f0 = -f1 * 0.5f;
            switch (r6) {
                case 1:
                case 5:
                case 9:
                    f0 += (f1 - pSubData->unk8) * 0.5f;
                    break;
                case 2:
                case 6:
                case 10:
                    f0 += (f1 - pSubData->unk8);
                    break;
            }
            f5 = f27 + f28;
            int r27 = 0;
            u8* lineStart = (u8*)pSubData->unk0;
            Font::TextCharStrip* strip = chars;
            while (lineStart != (u8*)pSubData->unk4) {
                if (*lineStart > 0x20 && *lineStart <= 255 && pChars[*lineStart].unk1C != 0) {
                    FontCharData* pCharData = &pChars[*lineStart];
                    f6 = (pCharData->unk18 * pTld->y);
                    // Vertex 0
                    strip[r27].verts[0].x = f0;
                    strip[r27].verts[0].y = f27;
                    strip[r27].verts[0].z = 0.0f;
                    strip[r27].verts[0].u = pCharData->unk8;
                    strip[r27].verts[0].v = pCharData->unkC;
                    strip[r27].verts[0].color = color;
                    // Vertex 1
                    strip[r27].verts[1].x = f0 + f6;
                    strip[r27].verts[1].y = f27;
                    strip[r27].verts[1].z = 0.0f;
                    strip[r27].verts[1].u = pCharData->unk10;
                    strip[r27].verts[1].v = pCharData->unkC;
                    strip[r27].verts[1].color = color;
                    // Vertex 2
                    strip[r27].verts[2].x = f0;
                    strip[r27].verts[2].y = f5;
                    strip[r27].verts[2].z = 0.0f;
                    strip[r27].verts[2].u = pCharData->unk8;
                    strip[r27].verts[2].v = pCharData->unk14;
                    strip[r27].verts[2].color = color;
                    // Vertex 3
                    strip[r27].verts[3].x = strip[r27].verts[1].x;
                    strip[r27].verts[3].y = strip[r27].verts[2].y;
                    strip[r27].verts[3].z = 0.0f;
                    strip[r27].verts[3].u = pCharData->unk10;
                    strip[r27].verts[3].v = pCharData->unk14;
                    strip[r27].verts[3].color = color;
                    r27++;
                    f0 += (unk8 * pTld->y) + f6;
                } else {
                    f0 += unkC * pTld->y;
                }
                lineStart++;
            }
            if (stripFunc0 != NULL) {
                stripFunc0(chars, r27);
            }
            // Apply matrix to all characters
            // Rotation and translation
            for(int j = 0; j < r27; j++) {
                for(int k = 0; k < 4; k++) {
                    float x = chars[j].verts[k].x;
                    float y = chars[j].verts[k].y;
                    float tx = x * pMatrix->data[0][0] + y * pMatrix->data[1][0] + pMatrix->data[3][0];
                    float ty = x * pMatrix->data[0][1] + y * pMatrix->data[1][1] + pMatrix->data[3][1];
                    chars[j].verts[k].x = tx;
                    chars[j].verts[k].y = ty;
                }
            }
            if (stripFunc1 != NULL) {
                stripFunc1(chars, r27);
            }
            RenderChars(chars, r27);
        }
        f27 += f28;
    }
    return tld.z;
}