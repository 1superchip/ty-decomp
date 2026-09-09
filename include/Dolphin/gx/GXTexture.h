#ifndef _DOLPHIN_GXTEXTURE
#define _DOLPHIN_GXTEXTURE

#include <Dolphin/gx/GXEnum.h>
#include <Dolphin/gx/GXStruct.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef GXTexRegion* (*GXTexRegionCallback)(const GXTexObj* obj, GXTexMapID id);
typedef GXTlutRegion* (*GXTlutRegionCallback)(u32 tlut);

void GXInitTexObj(GXTexObj* obj, const void* data, u16 width, u16 height, u32 format,
                  GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXBool mipmap);
void GXInitTexObjCI(GXTexObj* obj, const void* data, u16 width, u16 height, GXCITexFmt format,
                    GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXBool mipmap, u32 tlut);
void GXInitTexObjData(GXTexObj* obj, const void* data);
void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter min_filt, GXTexFilter mag_filt, f32 min_lod,
                     f32 max_lod, f32 lod_bias, GXBool bias_clamp, GXBool do_edge_lod,
                     GXAnisotropy max_aniso);
GXTexFmt GXGetTexObjFmt(const GXTexObj* obj);
GXBool GXGetTexObjMipMap(const GXTexObj* obj);
void GXLoadTexObjPreLoaded(GXTexObj* obj, GXTexRegion* region, GXTexMapID id);
void GXLoadTexObj(GXTexObj* obj, GXTexMapID id);

void GXGetTexObjAll(const GXTexObj *obj, void **image_ptr, 
                    u16 *width, u16 *height, GXTexFmt *format, GXTexWrapMode *wrap_s, 
                    GXTexWrapMode *wrap_t, u8 *mipmap);

u32 GXGetTexBufferSize(u16 width, u16 height, u32 format, GXBool mipmap, u8 max_lod);
void GXInvalidateTexAll();
void GXInitTexObjWrapMode(GXTexObj* obj, GXTexWrapMode s, GXTexWrapMode t);
void GXInitTlutObj(GXTlutObj* obj, void* data, GXTlutFmt format, u16 entries);
void GXLoadTlut(const GXTlutObj* obj, GXTlut idx);
void GXSetTexCoordBias(GXTexCoordID coord, GXBool s_enable, GXBool t_enable);
void GXSetTexCoordScaleManually(GXTexCoordID coord, GXBool enable, u16 ss, u16 ts);
void GXInitTexCacheRegion(GXTexRegion* region, GXBool is_32b_mipmap, u32 tmem_even,
                          GXTexCacheSize size_even, u32 tmem_odd, GXTexCacheSize size_odd);
GXTexRegionCallback GXSetTexRegionCallback(GXTexRegionCallback callback);
GXTlutRegionCallback GXSetTlutRegionCallback(GXTlutRegionCallback callback);
void GXInitTlutRegion(GXTlutRegion* region, u32 tmem_addr, GXTlutSize tlut_size);
void GXInvalidateTexRegion(const GXTexRegion* region);

#ifdef __cplusplus
}
#endif

#endif // _DOLPHIN_GXTEXTURE
