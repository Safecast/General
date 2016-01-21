//
//  gbLUT_Numerics.c
//  Safecast
//
//  Created by Nicholas Dolezal on 11/6/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#include "gbLUT_Numerics.h"

void gbLUT_ConvertPlanarRGBAFFFFtoRGBA8888(const float* restrict f_R,
                                           const float* restrict f_G,
                                           const float* restrict f_B,
                                           const float* restrict f_A,
                                           const bool   isNormalized,
                                           const float  alphaVal,
                                           uint8_t*     restrict rgba,
                                           const int    rgbaTypeId,
                                           const size_t n)
{
    const float f_min = 0.0F;
    const float f_max = isNormalized ? 1.0F : 255.0F;
    
    Pixel_FFFF pxMin  = { f_min, f_min, f_min, f_min };
    Pixel_FFFF pxMax  = { f_max, f_max, f_max, f_max };
    
    int width;
    int height;
    
    if (n % 256 == 0)
    {
        width  = (int)n >> 8;
        height = (int)n / width;
    }//else if
    else
    {
        width  = (int)n;
        height = 1;
    }//else
    
    vImage_Buffer viRGBA = { rgba,        height, width, sizeof(uint8_t)*width*4 };
    vImage_Buffer viR    = { (void*)f_R,  height, width, sizeof(float)  * width  };
    vImage_Buffer viG    = { (void*)f_G,  height, width, sizeof(float)  * width  };
    vImage_Buffer viB    = { (void*)f_B,  height, width, sizeof(float)  * width  };
    vImage_Buffer viA    = { (void*)f_A,  height, width, sizeof(float)  * width  };
    
    const uint32_t vf    = 0;
    
    if (f_A != NULL)
    {
        if (   rgbaTypeId == kGB_RGBAType_RGBA
            || rgbaTypeId == kGB_RGBAType_RGBA_NoPremul)        // RGBA
        {
            vImageConvert_PlanarFToARGB8888(&viR, &viG, &viB, &viA, &viRGBA, pxMax, pxMin, vf);
        }//if
        else if (   rgbaTypeId == kGB_RGBAType_BGRA
                 || rgbaTypeId == kGB_RGBAType_BGRA_NoPremul)   // BGRA
        {
            vImageConvert_PlanarFToARGB8888(&viB, &viG, &viR, &viA, &viRGBA, pxMax, pxMin, vf);
        }//else if
        else                      // ARGB
        {
            vImageConvert_PlanarFToARGB8888(&viA, &viR, &viG, &viB, &viRGBA, pxMax, pxMin, vf);
        }//else
    }//if
    else
    {
        Pixel_8 alpha8 = isNormalized ? truncf(255.0F * alphaVal) : truncf(alphaVal);
        
        if (   rgbaTypeId == kGB_RGBAType_RGBA
            || rgbaTypeId == kGB_RGBAType_RGBA_NoPremul)        // RGBA
        {
            vImageConvert_PlanarFToRGBX8888(&viR, &viG, &viB, alpha8, &viRGBA, pxMax, pxMin, vf);
        }//if
        else if (   rgbaTypeId == kGB_RGBAType_BGRA
                 || rgbaTypeId == kGB_RGBAType_BGRA_NoPremul)   // BGRA
        {
            vImageConvert_PlanarFToBGRX8888(&viB, &viG, &viR, alpha8, &viRGBA, pxMax, pxMin, vf);
        }//else if
        else                                                    // ARGB
        {
            vImageConvert_PlanarFToXRGB8888(alpha8, &viR, &viG, &viB, &viRGBA, pxMax, pxMin, vf);
        }//else
    }//else (fill alpha)
}//gbLUT_ConvertPlanarRGBAFFFFtoRGBA8888



void gbLUT_ReverseLUT(float*       restrict r,
                      float*       restrict g,
                      float*       restrict b,
                      const size_t n)
{
    vDSP_vrvrs(r, 1, n);
    vDSP_vrvrs(g, 1, n);
    vDSP_vrvrs(b, 1, n);
}//gbLUT_ReverseLUT

void gbLUT_DiscretizeLUT(float*       restrict r,
                         float*       restrict g,
                         float*       restrict b,
                         const int    steps,
                         const size_t n)
{
    int minVal   = 0;
    int maxVal   = (int)n;
    int curStart = minVal;
    int stride   = MAX((int)n/steps,1);
    int curEnd   = curStart + stride <= maxVal ? curStart + stride : maxVal;
    
    float mean;
    
    // while loop to handle remainders
    while (curStart <= maxVal)
    {
        // not sure if mean or median better
        vDSP_meanv(r + curStart, 1, &mean, curEnd - curStart);
        vDSP_vfill(&mean, r + curStart, 1, curEnd - curStart);
        
        vDSP_meanv(g + curStart, 1, &mean, curEnd - curStart);
        vDSP_vfill(&mean, g + curStart, 1, curEnd - curStart);
        
        vDSP_meanv(b + curStart, 1, &mean, curEnd - curStart);
        vDSP_vfill(&mean, b + curStart, 1, curEnd - curStart);
        
        curStart += stride;
        curEnd    = curStart + stride <= maxVal ? curStart + stride : maxVal;
    }//while
}//gbLUT_DiscretizeLUT






void gbLUT_MakeLinIdxVector(float**      linIdxOut,
                            const size_t n)
{
    float f_zero = 0.0F;
    float f_one  = 1.0F;
    
    float *linIdx = malloc(sizeof(float) * n);
    
    vDSP_vramp(&f_zero, &f_one, linIdx, 1, n);
    
    *linIdxOut = linIdx;
}//gbLUT_MakeLinIdxVector

void gbLUT_MakeLogIdx(float* restrict ref_lutln,
                      float* restrict ref_f_linIdx,
                      const size_t n)
{
    float *f_idx        = malloc(sizeof(float) * n);
    float  f_calcIdxMax = (float)n - 1.0F;
    float  f_kLn        = M_E - 1.0F;
    int    logPasses;
    int    i;
    
    // 1. take input vector of indices [0...n]
    // 2. normalize to [0...1]
    // 3. convert to [0...1.71828]
    // 4. take log1pf, which re-normalizes back [0...1], only with log scaling now
    // 5. multiply by n-1 to convert normalized [0...1] back to [0...n] relative to input
    // 6. repeatedly reshuffle index values through the index log vector
    //    as many times as needed for adequate log-iness
    
    float f_recip = 1.0F / f_calcIdxMax;
    vDSP_vsmul(ref_f_linIdx, 1, &f_recip, ref_lutln, 1, n);
    
    vDSP_vsmul(ref_lutln,    1, &f_kLn,   ref_lutln, 1, n);
    
    int n_s32 = (int)n;
    vvlog1pf(ref_lutln, ref_lutln, &n_s32);
    
    vDSP_vsmul(ref_lutln, 1, &f_calcIdxMax, ref_lutln, 1, n);
    
    vDSP_vindex(ref_lutln, ref_f_linIdx, 1, f_idx, 1, n);
    
    logPasses = 3;//[GeigerBotHelper GetPref_CONFIG_TILE_ENGINE_LN_ITERATIONS];
    logPasses -= 1; // account for out-of-place only 1st vindex already done above
    
    for (i = 0; i < logPasses; i++)
    {
        vDSP_vindex(ref_lutln, f_idx, 1, f_idx, 1, n);
    }//for
    
    memcpy(ref_lutln, f_idx, sizeof(float) * n);
    
    free(f_idx);
    f_idx = NULL;
}//gbLUT_MakeLogIdx


void gbLUT_MakeLog10Idx(float*       restrict ref_lutln,
                        float*       restrict ref_f_linIdx,
                        const size_t n)
{
    float *f_idx        = malloc(sizeof(float) * n);
    float  f_calcIdxMax = (float)n - 1.0F;
    float  f_kLn        = 9.0F; // log base - 1
    int    logPasses;
    int    i;
    
    logPasses = 4;//[GeigerBotHelper GetPref_CONFIG_TILE_ENGINE_LOG10_ITERATIONS];
    logPasses -= 1; // account for out-of-place only 1st vindex already done above
    
    // 1. take input vector of indices [0...n]
    // 2. normalize to [0...1]
    // 3. convert to [0...9]
    // 4. take log10+1, which re-normalizes back [0...1], only with log scaling now
    // 5. multiply by n-1 to convert normalized [0...1] back to [0...n] relative to input
    // 6. repeatedly reshuffle index values through the index log vector
    //    as many times as needed for adequate log-iness
    
    int width;
    int height;
    int y_width;
    
    if (n % 256 == 0)
    {
        width  = (int)n >> 8;
        height = (int)n / width;
    }//if
    else
    {
        width  = (int)n;
        height = 1;
    }//else
    
    //vDSP_vsdiv(ref_f_linIdx, 1, &f_calcIdxMax, ref_lutln, 1, n);
    float f_recip = 1.0F / f_calcIdxMax;
    float f_one = 1.0F;
    
    for (int y=0; y<height; y++)
    {
        y_width = y*width;
        
        vDSP_vsmul(ref_f_linIdx + y_width, 1,
                   &f_recip,
                   ref_lutln    + y_width, 1,
                   width);
        
        vDSP_vsmsa(ref_lutln + y_width, 1,
                   &f_kLn, &f_one,
                   ref_lutln + y_width, 1,
                   width);
        
        vvlog10f(ref_lutln + y_width,
                 ref_lutln + y_width,
                 &width);
        
        vDSP_vsmul(ref_lutln + y_width, 1,
                   &f_calcIdxMax,
                   ref_lutln + y_width, 1,
                   width);
    }//for
    
    vDSP_vindex(ref_lutln, ref_f_linIdx, 1, f_idx, 1, n);
    
    for (int y=0; y<height; y++)
    {
        y_width = y*width;
        
        for (i = 0; i < logPasses; i++)
        {
            vDSP_vindex(ref_lutln,
                        f_idx + y_width, 1,
                        f_idx + y_width, 1,
                        width);
        }//for
    }//for
    
    memcpy(ref_lutln, f_idx, sizeof(float) * n);
    
    free(f_idx);
    f_idx = NULL;
}//gbLUT_MakeLog10Idx






// nearest neighbor resample LUT to increased color depth -- no interpolation
void gbLUT_InterpolateNN_LUT(const float* restrict src_r,
                             const float* restrict src_g,
                             const float* restrict src_b,
                             const size_t src_n,
                             float*       restrict dest_r,
                             float*       restrict dest_g,
                             float*       restrict dest_b,
                             const size_t dest_n)
{
    size_t stride = dest_n / src_n;
    size_t vOffset;
    
    for (size_t i = 0; i < src_n; i ++)
    {
        vOffset = i * stride;
        vDSP_vfill(&src_r[i], dest_r + vOffset, 1, stride);
        vDSP_vfill(&src_g[i], dest_g + vOffset, 1, stride);
        vDSP_vfill(&src_b[i], dest_b + vOffset, 1, stride);
    }//for
}//ResampleLUT_NN


void gbLUT_InterpolateLUT(const float* restrict src_r,
                          const float* restrict src_g,
                          const float* restrict src_b,
                          const size_t src_n,
                          float*       restrict dest_r,
                          float*       restrict dest_g,
                          float*       restrict dest_b,
                          const size_t dest_n,
                          const bool   nearest_neighbor_only)
{
    // interpolation needs to be bypassed if the LUT is pre-discretized or a
    // smoothed/antialiased type effect will be added.
    if (nearest_neighbor_only)
    {
        gbLUT_InterpolateNN_LUT(src_r, src_g, src_b, src_n, dest_r, dest_g, dest_b, dest_n);
        
        return;
    }//if
    
    // create copy of original vectors n+1 in size to allow for vlint end handling
    float *r2     = malloc(sizeof(float) * (src_n+1));
    float *g2     = malloc(sizeof(float) * (src_n+1));
    float *b2     = malloc(sizeof(float) * (src_n+1));
    float *vIdx   = malloc(sizeof(float) * dest_n);
    float  f_seed = 0.0F;
    float  f_inc  = (float)src_n / (float)dest_n;
    
    // fill temp copy with original
    memcpy(r2, src_r, sizeof(float) * src_n);
    memcpy(g2, src_g, sizeof(float) * src_n);
    memcpy(b2, src_b, sizeof(float) * src_n);
    
    // pad new element with the last one
    r2[src_n]  = src_r[src_n-1];
    g2[src_n]  = src_g[src_n-1];
    b2[src_n]  = src_b[src_n-1];
    
    vDSP_vramp(&f_seed, &f_inc, vIdx, 1, dest_n);
    
    vDSP_vlint(r2, vIdx, 1, dest_r, 1, dest_n, src_n+1);
    vDSP_vlint(g2, vIdx, 1, dest_g, 1, dest_n, src_n+1);
    vDSP_vlint(b2, vIdx, 1, dest_b, 1, dest_n, src_n+1);
    
    free(vIdx);
    free(r2);
    free(g2);
    free(b2);
}//gbLUT_InterpolateLUT





void gbLUT_ApplyMapAlpha(float*       restrict r,
                         float*       restrict g,
                         float*       restrict b,
                         float*       restrict a,
                         const float  map_alpha,
                         const bool   is_opaque,
                         const size_t n)
{
    float _map_alpha = map_alpha;
    
    if (_map_alpha > 0.99F)
    {
        return;
    }//if
    else if (_map_alpha < 0.01F)
    {
        _map_alpha = 0.01F;
    }//else if
    
    float *f_work   = malloc(sizeof(float) * n);
    float  f_negone = -1.0F;
    
    //float bkgAlpha = 1.0F - mapAlpha;
    //float neg_bkgAlpha = 0.0F - bkgAlpha;
    float neg_mapAlpha = 0.0F - _map_alpha;
    
    // 1. calc new alpha channel
    //    use 2x neg values to avoid filling a vector with 1s or using vabs
    vDSP_vsadd(a, 1, &f_negone, f_work, 1, n);
    //     vDSP_vsmul(f_work, 1, &neg_mapAlpha, f_work, 1, n);
    //     vDSP_vadd(f_work, 1, f_A, 1, f_A, 1, n);
    // -> fused multiply-add
    vDSP_vsma(f_work, 1, &neg_mapAlpha, a, 1, a, 1, n);
    
    // 2. since background is black (=0) only need to divide RGB value by new alpha
    //     (???) is this alpha unpremultiply??
    // **** disabling as test *****
    vDSP_vdiv(a, 1, r, 1, r, 1, n);
    vDSP_vdiv(a, 1, g, 1, g, 1, n);
    vDSP_vdiv(a, 1, b, 1, b, 1, n);
    
    // 3.(?) now apply alpha to nodata sections...
    //float f_max = 1.0F;
    //vDSP_vclip(f_A, 1, &mapAlpha, &f_max, f_A, 1, n);
    
    // 4. Make opaque if needed
    if (is_opaque)
    {
        float f_one = 1.0F;
        vDSP_vfill(&f_one, a, 1, n);
    }//if
    
    free(f_work);
    f_work = NULL;
}//ApplyMapAlpha



void gbLUT_LUTtoLUT_XTREME(float*       restrict r,
                           float*       restrict g,
                           float*       restrict b,
                           float*       restrict xtreme,
                           const float  map_alpha,
                           const float  pt_alpha,
                           const int    rgbaTypeId,
                           const size_t n)
{
    float f_alpha = (pt_alpha < 0.9F ? pt_alpha : 1.0F);
    
    if (pt_alpha < 0.9F)
    {
        vDSP_vsmul(r, 1, &f_alpha, r, 1, n);
        vDSP_vsmul(g, 1, &f_alpha, g, 1, n);
        vDSP_vsmul(b, 1, &f_alpha, b, 1, n);
    }//if
    
    /*
     *   8-bit:
     *       if( isImagePremultiplied )
     *           color = (color * 255 + (255 - alpha) * backgroundColor + 127) / 255
     *       else
     *           color = (color * alpha + (255 - alpha) * backgroundColor + 127) / 255
     *
     *   floating point:
     *       if( isImagePremultiplied )
     *           color = color + (1.0f - alpha) * backgroundColor
     *       else
     *           color = color * alpha + (1.0f - alpha) * backgroundColor
     */
    
    
    if (map_alpha > 0.1F)
    {
        float newAlpha  = (f_alpha - 1.0F) * (0.0F - map_alpha) + f_alpha;
        float newPremul = 1.0F/newAlpha;
        
        vDSP_vsmul(r, 1, &newPremul, r, 1, n);
        vDSP_vsmul(g, 1, &newPremul, g, 1, n);
        vDSP_vsmul(b, 1, &newPremul, b, 1, n);
    }//if
    
    float alphaVal = MAX(f_alpha, map_alpha);
    
    gbLUT_ConvertPlanarRGBAFFFFtoRGBA8888(r, g, b, NULL, true, alphaVal, (void*)xtreme, rgbaTypeId, n);
    
    // 2014-09-05 ND: Adding support for non-premultiplied alpha variants
    //                to reduce redundancy in PNG exports.
    //
    if ((   rgbaTypeId == kGB_RGBAType_BGRA_NoPremul
         || rgbaTypeId == kGB_RGBAType_RGBA_NoPremul))
    {
        printf("LUT: Apply alpha unpremultiplication 0!\n");
        vImage_Buffer vi_src = { (void*)xtreme, 1, n, sizeof(float) * n };
        
        vImageUnpremultiplyData_BGRA8888(&vi_src, &vi_src, kvImageDoNotTile);
    }//if
    else if ((   rgbaTypeId == kGB_RGBAType_ABGR_NoPremul
              || rgbaTypeId == kGB_RGBAType_ARGB_NoPremul))
    {
        printf("LUT: Apply alpha unpremultiplication 1!\n");
        vImage_Buffer vi_src = { (void*)xtreme, 1, n, sizeof(float) * n };
        
        vImageUnpremultiplyData_ARGB8888(&vi_src, &vi_src, kvImageDoNotTile);
    }//else if
}//_LUTtoLUT_XTREME

void gbLUT_U08_to_F32_vImage(const uint8_t* restrict src_u08,
                             float*         restrict dest_f32,
                             const size_t            n)
{
    vImage_Buffer src  = { (void*)src_u08, 1, n, sizeof(uint8_t) * n };
    vImage_Buffer dest = {       dest_f32, 1, n, sizeof(float)   * n };
    vImageConvert_Planar8toPlanarF(&src, &dest, 1.0F, 0.0F, kvImageNoFlags);
}//gbLUT_U08_to_F32_vImage



















