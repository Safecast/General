//
//  gbLUT_Transforms.c
//  Safecast
//
//  Created by Nicholas Dolezal on 11/7/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#include "gbLUT_Transforms.h"

void gbLUT_Transform_Indices_PlanarF(const float* data,
                                     gbLUT*       lut,
                                     float**      r,
                                     float**      g,
                                     float**      b,
                                     const size_t n)
{
    float *_r = malloc(sizeof(float) * n);
    float *_g = malloc(sizeof(float) * n);
    float *_b = malloc(sizeof(float) * n);
    
    float* lut_r = NULL;
    float* lut_g = NULL;
    float* lut_b = NULL;
    
    gbLUT_GetPointersRGB_f32(lut, &lut_r, &lut_g, &lut_b);
    
    vDSP_vindex(lut_r, data, 1, _r, 1, n);
    vDSP_vindex(lut_g, data, 1, _g, 1, n);
    vDSP_vindex(lut_b, data, 1, _b, 1, n);

    *r = _r;
    *g = _g;
    *b = _b;
}//gbLUT_Transform_Indices_PlanarF


void gbLUT_Transform_IndicesAlpha_RGBA8888(const float* data,
                                           gbLUT*       lut,
                                           uint8_t*     rgba,
                                           const size_t n)
{
    int rc, gc, bc, ac;
    
    gbImage_RGBA_GetRGBAVectorEntryOffsets(lut->props->rgba_type_id, &rc, &gc, &bc, &ac);
    
    if (lut->props->buffer_type_id == kGB_LUT_BufferType_PlanarF)
    {
        float *r = NULL;
        float *g = NULL;
        float *b = NULL;
        
        gbLUT_Transform_Indices_PlanarF(data, lut, &r, &g, &b, n);
        
        vDSP_vsmul(r, 1, &lut->props->data_alpha, r, 1, n);
        vDSP_vsmul(g, 1, &lut->props->data_alpha, g, 1, n);
        vDSP_vsmul(b, 1, &lut->props->data_alpha, b, 1, n);
        
        gbLUT_ConvertPlanarRGBAFFFFtoRGBA8888(r, g, b, NULL, true, lut->props->data_alpha, rgba, lut->props->rgba_type_id, n);
        
        free(r); r = NULL;
        free(g); g = NULL;
        free(b); b = NULL;
    }//if
    else if (lut->props->buffer_type_id == kGB_LUT_BufferType_RGBA)
    {
        vDSP_vindex((float*)lut->data, data, 1, (float*)rgba, 1, n);
    }//else if
    else
    {
        printf("gbLUT_Transforms.c: gbLUT_Transform_IndicesAlpha_RGBA8888: [ERR]: Bad buffer type.\n");
    }//else
}//gbLUT_Transform_IndicesAlpha_RGBA8888







void gbLUT_Transform_Data_PlanarF(float*       data,
                                  gbLUT*       lut,
                                  float**      r,
                                  float**      g,
                                  float**      b,
                                  const size_t n)
{
    float* _r       = malloc(sizeof(float) * n);
    float* _g       = malloc(sizeof(float) * n);
    float* _b       = malloc(sizeof(float) * n);
    float  _lutMin  = gbLUT_GetEffectiveMin(lut);
    float  _lutMax  = gbLUT_GetEffectiveMax(lut);
    float  f_hi     = (float)lut->props->n - 1.0F;
    float  f_zero   = 0.0F;
    float  f_negMin = 0.0F - _lutMin;             // negate for vsadd (no vssub)
    float  f_mul    = f_hi / (_lutMax - _lutMin); // tricksy but saves an op
    
    float* lut_r = NULL;
    float* lut_g = NULL;
    float* lut_b = NULL;
    
    gbLUT_GetPointersRGB_f32(lut, &lut_r, &lut_g, &lut_b);
    
    size_t y_width;
    size_t width  = n % 256 == 0 ? n >> 8    : n;
    size_t height = n % 256 == 0 ? n / width : 1;
    
    for (size_t y=0; y<height; y++)
    {
        y_width = y * width;
        
        vDSP_vsadd(data + y_width, 1,
                   &f_negMin,
                   data + y_width, 1,
                   width);
        
        vDSP_vsmul(data + y_width, 1,
                   &f_mul,
                   data + y_width, 1,
                   width);
        
        vDSP_vclip(data + y_width, 1,
                   &f_zero, &f_hi,
                   data + y_width, 1,
                   width);
        
        vDSP_vindex(lut_r,
                    data + y_width, 1,
                    _r   + y_width, 1,
                    width);
        
        vDSP_vindex(lut_g,
                    data + y_width, 1,
                    _g   + y_width, 1,
                    width);
        
        vDSP_vindex(lut_b,
                    data + y_width, 1,
                    _b   + y_width, 1,
                    width);
    }//for
    
    *r = _r;
    *g = _g;
    *b = _b;
}//gbLUT_Transform_Data_PlanarF









void gbLUT_Transform_Data_RGBA8888(float*       data,
                                   float**      rgba,
                                   gbLUT*       lut,
                                   const size_t n)
{
    float *_f_RGBA = NULL;
    
    bool noAlloc = *rgba != NULL;
    
    if (!noAlloc)
    {
        _f_RGBA = malloc(sizeof(float) * n);
    }//if
    else
    {
        _f_RGBA = (float*)(*rgba);
    }//if
    
    float  _lutMin  = gbLUT_GetEffectiveMin(lut);
    float  _lutMax  = gbLUT_GetEffectiveMax(lut);
    float  f_hi     = (float)lut->props->n - 1.0F;
    float  f_zero   = 0.0F;
    float  f_negMin = 0.0F - _lutMin;             // negate for vsadd (no vssub)
    float  f_mul    = f_hi / (_lutMax - _lutMin); // tricksy but saves an op
    
    size_t y_width;
    size_t width  = n % 256 == 0 ? n >> 8    : n;
    size_t height = n % 256 == 0 ? n / width : 1;

    for (size_t y=0; y<height; y++)
    {
        y_width = y * width;
        
        gbLUT_vsasmclip_NEON(data + y_width,
                             f_negMin, f_mul, f_zero, f_hi,
                             width);
        
        vDSP_vindex((float*)lut->data,
                    data    + y_width, 1,
                    _f_RGBA + y_width, 1,
                    width);
    }//for
    
    if (!noAlloc)
    {
        *rgba = _f_RGBA;
    }//if
}//gbLUT_Transform_Data_RGBA8888

















void gbLUT_Transform_ApplyAlpha_RGBA8888(uint8_t*     rgba,
                                         const float* alphaVector,
                                         const float  mapAlpha,
                                         const int    rgbaTypeId,
                                         const size_t n)
{
    if (   rgbaTypeId == kGB_RGBAType_ABGR
        || rgbaTypeId == kGB_RGBAType_ARGB)
    {
        gbLUT_ApplyAlphaToARGB_v6_NEON(rgba, alphaVector, mapAlpha, n);
    }//if
    else if (   rgbaTypeId == kGB_RGBAType_BGRA
             || rgbaTypeId == kGB_RGBAType_RGBA)
    {
        gbLUT_ApplyAlphaToRGBA_v8_NEON(rgba, alphaVector, mapAlpha, n);
    }//else if
    else if (   rgbaTypeId == kGB_RGBAType_BGRA_NoPremul
             || rgbaTypeId == kGB_RGBAType_RGBA_NoPremul)
    {
        gbLUT_ApplyAlphaToRGBA_NoPremultiply_v2_NEON(rgba, alphaVector, mapAlpha, n);
    }//else if
    else
    {
        gbLUT_ApplyAlphatoRGBA_v2_Scalar(rgba, alphaVector, mapAlpha, rgbaTypeId, n);
    }//else
}//gbLUT_Transform_ApplyAlpha_RGBA8888



void gbLUT_Transform_DataAlpha_RGBA8888_BufferRGBA(float*       data,
                                                   const float* alpha,
                                                   uint8_t**    rgba,
                                                   gbLUT*       lut,
                                                   const size_t n)
{
    const bool no_alloc = *rgba != NULL;
    float*     f_rgba   = no_alloc ? (float*)(*rgba) : NULL;
    
    gbLUT_Transform_Data_RGBA8888(data, &f_rgba, lut, n);
    
    uint8_t *u_rgba = (uint8_t*)f_rgba;
    
    gbLUT_Transform_ApplyAlpha_RGBA8888(u_rgba, alpha, lut->props->bkg_alpha, lut->props->rgba_type_id, n);
    
    if (!no_alloc)
    {
        *rgba = u_rgba;
    }//if
}//gbLUT_Transform_DataAlpha_RGBA8888_BufferRGBA






void gbLUT_Transform_ApplyAlpha_PlanarF(float* r,
                                        float* g,
                                        float* b,
                                        float* alpha,
                                        gbLUT*       lut,
                                        const size_t n)
{
    float *shadow = malloc(sizeof(float) * n);
    float f_zero = 0.0F;
    
    // Negative input alpha values are used to encode shadow, which means
    // zero out the RGB value.
    
    // shadowVector: clip negated values to 0, multiply by RGB to zero them
    //               regardless of LUT... also performs alpha premultiply!
    
    // alphaVector: take absolute value and apply to alpha channel.  preserves
    //              sublayer alpha.
    
    vDSP_vthres(alpha, 1, &f_zero, shadow, 1, n);
    vDSP_vabs(alpha, 1, alpha, 1, n);
    
    vDSP_vmul(shadow, 1, r, 1, r, 1, n);
    vDSP_vmul(shadow, 1, g, 1, g, 1, n);
    vDSP_vmul(shadow, 1, b, 1, b, 1, n);
    
    free(shadow); shadow = NULL;
    
    size_t y_width;
    size_t width  = n % 256 == 0 ? n >> 8    : n;
    size_t height = n % 256 == 0 ? n / width : 1;
    
    for (size_t y=0; y<height; y++)
    {
        y_width = y * width;
        
        vDSP_vmul(alpha + y_width, 1,
                  r     + y_width, 1,
                  r     + y_width, 1,
                  width);
        
        vDSP_vmul(alpha + y_width, 1,
                  g     + y_width, 1,
                  g     + y_width, 1,
                  width);
        
        vDSP_vmul(alpha + y_width, 1,
                  b     + y_width, 1,
                  b     + y_width, 1,
                  width);
    }//for
    
    // alpha premultiplication complete
    if (lut->props->bkg_alpha > 0.1F)
    {
        gbLUT_ApplyMapAlpha(r, g, b, alpha, lut->props->bkg_alpha, false, n);
    }//if
    
    // dump to 8-bit out for image bytes
    const float f_255 = 255.0F;
    vDSP_vsmul(alpha, 1, &f_255, alpha, 1, n);
    
    // THESE TWO LINES NEEDED FOR MAP ALPHA
    if (lut->props->bkg_alpha > 0.1F)
    {
        const float min_alpha = truncf(lut->props->bkg_alpha * 255.0F);
        vDSP_vclip(alpha, 1, &min_alpha, &f_255, alpha, 1, n);
    }//if
}//gbLUT_Transform_ApplyAlpha_PlanarF


void gbLUT_Transform_DataAlpha_RGBA8888_BufferPlanarF(float*       data,
                                                      float*       alpha,
                                                      gbLUT*       lut,
                                                      uint8_t**    rgba,
                                                      const size_t n)
{
    float* r = NULL;
    float* g = NULL;
    float* b = NULL;
    
    gbLUT_Transform_Data_PlanarF(data, lut, &r, &g, &b, n);
    
    gbLUT_Transform_ApplyAlpha_PlanarF(r, g, b, alpha, lut, n);
    
    uint8_t *_rgba = malloc(sizeof(uint8_t) * n * 4);

    gbLUT_ConvertPlanarRGBAFFFFtoRGBA8888(r, g, b, alpha, true, 1.0F, _rgba, lut->props->rgba_type_id, n);
    
    free(r); r = NULL;
    free(g); g = NULL;
    free(b); b = NULL;
    
    *rgba = _rgba;
}//gbLUT_Transform_DataAlpha_RGBA8888_BufferPlanarF





void gbLUT_Transform_DataAlpha_RGBA8888(float*       data,
                                        float*       alpha,
                                        uint8_t**    rgba,
                                        gbLUT*       lut,
                                        const size_t n)
{
    if (lut->props->buffer_type_id == kGB_LUT_BufferType_RGBA)
    {
        gbLUT_Transform_DataAlpha_RGBA8888_BufferRGBA(data, alpha, rgba, lut, n);
    }//if
    else if (lut->props->buffer_type_id == kGB_LUT_BufferType_PlanarF)
    {
        gbLUT_Transform_DataAlpha_RGBA8888_BufferPlanarF(data, alpha, lut, rgba, n);
    }//else if
    else
    {
        printf("gbLUT_Transforms.c: gbLUT_Transform_DataAlpha_RGBA8888: [ERR] Unsupported buffer type.\n");
    }//else
}//gbLUT_Transform_DataAlpha_RGBA8888





void gbLUT_Transform_LUT_RGBA8888(gbLUT*       lut,
                                  const size_t width,
                                  const size_t height,
                                  const bool   is_vertical,
                                  uint8_t**    rgba)
{
    float*   lin_idxs = NULL;
    float*   idxs     = NULL;
    uint8_t* _rgba    = NULL;
    size_t   render_w = width;
    size_t   render_h = height;
    
    if (is_vertical && render_h != lut->props->n)
    {
        render_w = 1;
        render_h = lut->props->n;
    }//if
    else if (!is_vertical && render_w != lut->props->n)
    {
        render_w = lut->props->n;
        render_h = 1;
    }//else if
    
    idxs  = malloc(sizeof(float) * render_w * render_h);
    _rgba = malloc(render_w * render_h * 4);

    gbLUT_MakeLinIdxVector(&lin_idxs, render_w * render_h);
    
    if (is_vertical)           // fill all values in row with single index value
    {
        vDSP_vrvrs(lin_idxs, 1, render_w * render_h);
        
        for (size_t y=0; y<render_h; y++)
        {
            vDSP_vfill(&lin_idxs[y], idxs + y * render_w, 1, render_w);
        }//for
    }//if
    else                       // just copy indices to every row
    {
        for (size_t y=0; y<render_h; y++)
        {
            //vDSP_vsadd(linIdx, 1, &f_zero, dataVector + y * render_w, 1, render_w);
            memcpy(idxs + y * render_w, lin_idxs, render_w * sizeof(float));
        }//for
    }//else
    
    gbLUT_Transform_IndicesAlpha_RGBA8888(idxs, lut, _rgba, render_w * render_h);
    
    *rgba = _rgba;
    
    free(idxs);
    free(lin_idxs);
}//gbLUT_Transform_LUT_RGBA8888





