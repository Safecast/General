//
//  gbLUT_Numerics_SIMD.c
//  Safecast
//
//  Created by Nicholas Dolezal on 11/6/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#include "gbLUT_Numerics_SIMD.h"


#ifndef GB_IS_PTR_ALIGNED
    #define GB_IS_PTR_ALIGNED(POINTER) \
        (((uintptr_t)(const void *)(POINTER)) % (16) == 0)
#endif

void gbLUT_vsasmclip_NEONvsSSE_A(float*       data,
                                 const float  x,
                                 const float  y,
                                 const float  clipMin,
                                 const float  clipMax,
                                 const size_t n)
{
#ifdef NEON2SSE_H
    float32x4_t vsx   = vdupq_n_f32(x);
    float32x4_t vsy   = vdupq_n_f32(y);
    float32x4_t vsmin = vdupq_n_f32(clipMin);
    float32x4_t vsmax = vdupq_n_f32(clipMax);
    
    float32x4_t v0;
    float32x4_t v1;
    float32x4_t v2;
    float32x4_t v3;
    
    size_t i4;
    size_t i8;
    size_t i12;
    
    for (size_t i = 0UL; i < n; i += 16UL)
    {
        i4  = i +  4UL; // MATH
        i8  = i +  8UL;
        i12 = i + 12UL;
        
        v0 = gbA_vld1q_f32(&(data[i]));
        v1 = gbA_vld1q_f32(&(data[i4]));
        v2 = gbA_vld1q_f32(&(data[i8]));
        v3 = gbA_vld1q_f32(&(data[i12]));
        
        v0 = vaddq_f32(v0, vsx);
        v1 = vaddq_f32(v1, vsx);
        v2 = vaddq_f32(v2, vsx);
        v3 = vaddq_f32(v3, vsx);
        
        v0 = vmulq_f32(v0, vsy);
        v1 = vmulq_f32(v1, vsy);
        v2 = vmulq_f32(v2, vsy);
        v3 = vmulq_f32(v3, vsy);
        
        v0 = vmaxq_f32(v0, vsmin);
        v1 = vmaxq_f32(v1, vsmin);
        v2 = vmaxq_f32(v2, vsmin);
        v3 = vmaxq_f32(v3, vsmin);
        
        v0 = vminq_f32(v0, vsmax);
        v1 = vminq_f32(v1, vsmax);
        v2 = vminq_f32(v2, vsmax);
        v3 = vminq_f32(v3, vsmax);
        
        gbA_vst1q_f32(&(data[i]),   v0);
        gbA_vst1q_f32(&(data[i4]),  v1);
        gbA_vst1q_f32(&(data[i8]),  v2);
        gbA_vst1q_f32(&(data[i12]), v3);
    }//for
#endif
}//gbDSP_vsasmclip_NEONvsSSE_A





// gbLUT_vsasmclip_NEON: Used by ApplyLUT_XTREME
//
// vsadd, vsmul, vclip
//
// data[i] = MIN(MAX((data[i] + x) * y, clipMin), clipMax);
//
void gbLUT_vsasmclip_NEON(float*       data,
                          const float  x,
                          const float  y,
                          const float  clipMin,
                          const float  clipMax,
                          const size_t n)
{
#ifdef NEON2SSE_H
    if (GB_IS_PTR_ALIGNED(data))
    {
        gbLUT_vsasmclip_NEONvsSSE_A(data, x, y, clipMin, clipMax, n);
        return;
    }//if
#endif
    
    float32x4_t vsx   = vdupq_n_f32(x);
    float32x4_t vsy   = vdupq_n_f32(y);
    float32x4_t vsmin = vdupq_n_f32(clipMin);
    float32x4_t vsmax = vdupq_n_f32(clipMax);
    
    float32x4_t v0;
    float32x4_t v1;
    float32x4_t v2;
    float32x4_t v3;
    
    size_t i4;
    size_t i8;
    size_t i12;
    
    for (size_t i = 0UL; i < n; i += 16UL)
    {
        i4  = i +  4UL; // MATH
        i8  = i +  8UL;
        i12 = i + 12UL;
        
        v0 = vld1q_f32(&(data[i]));
        v1 = vld1q_f32(&(data[i4]));
        v2 = vld1q_f32(&(data[i8]));
        v3 = vld1q_f32(&(data[i12]));
        
        v0 = vaddq_f32(v0, vsx);
        v1 = vaddq_f32(v1, vsx);
        v2 = vaddq_f32(v2, vsx);
        v3 = vaddq_f32(v3, vsx);
        
        v0 = vmulq_f32(v0, vsy);
        v1 = vmulq_f32(v1, vsy);
        v2 = vmulq_f32(v2, vsy);
        v3 = vmulq_f32(v3, vsy);
        
        v0 = vmaxq_f32(v0, vsmin);
        v1 = vmaxq_f32(v1, vsmin);
        v2 = vmaxq_f32(v2, vsmin);
        v3 = vmaxq_f32(v3, vsmin);
        
        v0 = vminq_f32(v0, vsmax);
        v1 = vminq_f32(v1, vsmax);
        v2 = vminq_f32(v2, vsmax);
        v3 = vminq_f32(v3, vsmax);
        
        vst1q_f32(&(data[i]),   v0);
        vst1q_f32(&(data[i4]),  v1);
        vst1q_f32(&(data[i8]),  v2);
        vst1q_f32(&(data[i12]), v3);
    }//for
}//gbLUT_vsasmclip_NEON







// **** NEW TEST AREA FOR SUPER C-C-C-COMBO ****


// this is "part 2" of applying the LUT, and takes care of the alpha channel.
//
// it may not be worth it to do this inline, because it may lead to register
// loads/spills, making it worse than just L1 cache blocking it as a true
// 2nd step. (if that happens)
//
// the limiting factor is the LUT gather has to go through another load,
// and even if it didn't, it still has to be deinterleaved
//
/*
 static void FORCE_INLINE _alpha_inner_loop_NEON(float*       dest0_ptr,
 const float* alpha0_ptr,
 const float* alpha1_ptr,
 const float* alpha2_ptr,
 const float* alpha3_ptr,
 const        uint8x16_t* sa,
 const        float32x4_t* sf)
 {
 float32x4_t  va0, va1, va2, va3;
 uint8x16x4_t rgba;
 
 va0 = vld1q_f32(alpha0_ptr);                    // load unprocessed alpha channels
 va1 = vld1q_f32(alpha1_ptr);
 va2 = vld1q_f32(alpha2_ptr);
 va3 = vld1q_f32(alpha3_ptr);
 
 rgba = vld4q_u8((uint8_t*)dest0_ptr);           // load already transformed RGBA values from LUT
 
 uint16x8_t r0, r1, g0, g1, b0, b1, a0, a1;
 
 r0 = vmovl_u8( vget_low_u8(rgba.val[0]));       // promote to uint16_t for the primary
 r1 = vmovl_u8(vget_high_u8(rgba.val[0]));       // alpha premultiplication later
 g0 = vmovl_u8( vget_low_u8(rgba.val[1]));
 g1 = vmovl_u8(vget_high_u8(rgba.val[1]));
 b0 = vmovl_u8( vget_low_u8(rgba.val[2]));
 b1 = vmovl_u8(vget_high_u8(rgba.val[2]));
 a0 = vmovl_u8( vget_low_u8(rgba.val[3]));
 a1 = vmovl_u8(vget_high_u8(rgba.val[3]));
 
 va0 = vmulq_f32(va0, *sf);                      // convert norm range to uint8_t range
 va1 = vmulq_f32(va1, *sf);
 va2 = vmulq_f32(va2, *sf);
 va3 = vmulq_f32(va3, *sf);
 
 int32x4_t ai0, ai1, ai2, ai3;
 int32x4_t aa0, aa1, aa2, aa3;
 
 ai0 = vcvtq_s32_f32(va0);                       // float -> int
 ai1 = vcvtq_s32_f32(va1);
 ai2 = vcvtq_s32_f32(va2);
 ai3 = vcvtq_s32_f32(va3);
 
 aa0 = vabsq_s32(ai0);                           // this is abs'd because the alpha channel
 aa1 = vabsq_s32(ai1);                           // can be negated to denote shadow alpha
 aa2 = vabsq_s32(ai2);                           // which zeroes all RGB values.  this will
 aa3 = vabsq_s32(ai3);                           // become the true alpha channel value.
 
 uint16x4_t  au0,  au1,  au2,  au3;
 uint16x4_t aau0, aau1, aau2, aau3;
 
 au0 = vqmovun_s16(ai0);                         // if these were negated earlier, they are now
 au1 = vqmovun_s16(ai1);                         // clipped at 0 (for free!), which is critical
 au2 = vqmovun_s16(ai2);                         // to zeroing the RGB values
 au3 = vqmovun_s16(ai3);
 
 aau0 = vqmovun_s16(aa0);                        // same thing but these will never be negated.
 aau1 = vqmovun_s16(aa1);
 aau2 = vqmovun_s16(aa2);
 aau3 = vqmovun_s16(aa3);
 
 uint16x8_t  av0,  av1;
 uint16x8_t aav0, aav1;
 
 av0 = vcombine_u16(au0, au1);                   // rearrange narrower types
 av1 = vcombine_u16(au2, au3);
 
 aav0 = vcombine_u16(aau0, aau1);
 aav1 = vcombine_u16(aau2, aau3);
 
 r0 = vmulq_u16(r0, aav0);                       // perform alpha premultiplication
 r1 = vmulq_u16(r1, aav1);
 g0 = vmulq_u16(g0, aav0);
 g1 = vmulq_u16(g1, aav1);
 b0 = vmulq_u16(b0, aav0);
 b1 = vmulq_u16(b1, aav1);
 
 a0 = vmulq_u16(a0, aav0);                       // alpha channel gets premul too,
 a1 = vmulq_u16(a1, aav1);                       // to indirectly implement "point" or layer alpha.
 
 
 uint8x8_t sr0, sr1, sg0, sg1, sb0, sb1, sa0, sa1;
 
 sr0 = vqshrn_n_u16(r0, 8);                      // narrow back to 8-bit
 sr1 = vqshrn_n_u16(r1, 8);
 sg0 = vqshrn_n_u16(g0, 8);
 sg1 = vqshrn_n_u16(g1, 8);
 sb0 = vqshrn_n_u16(b0, 8);
 sb1 = vqshrn_n_u16(b1, 8);
 sa0 = vqshrn_n_u16(a0, 8);
 sa1 = vqshrn_n_u16(a1, 8);
 
 rgba.val[0] = vcombine_u8(sr0, sr1);            // combine narrowed types
 rgba.val[1] = vcombine_u8(sg0, sg1);
 rgba.val[2] = vcombine_u8(sb0, sb1);
 rgba.val[3] = vcombine_u8(sa0, sa1);
 rgba.val[3] = vmaxq_u8(rgba.val[3], *sa);       // clip to min alpha value ("mapAlpha")
 
 vst4q_u8((uint8_t*)dest0_ptr, rgba);
 }//_alpha_inner_loop_NEON
 
 
 
 // todo: make sure passing in pointers to the temp arrays doesn't confuse
 // the compiler.  in that case, inline declaration may be preferable, assuming
 // it doesn't do weird things as well.
 //
 static FORCE_INLINE void _scale_lut_inner_loop_NEON(float* dest0_ptr,
 float* dest1_ptr,
 float* dest2_ptr,
 float* dest3_ptr,
 const float* lut,
 const float* src0_ptr,
 const float* src1_ptr,
 const float* src2_ptr,
 const float* src3_ptr,
 const float32x4_t* vsx,
 const float32x4_t* vsy,
 const float32x4_t* vsmin,
 const float32x4_t* vsmax,
 int32_t* tmp0,
 int32_t* tmp1,
 int32_t* tmp2,
 int32_t* tmp3)
 {
 float32x4_t  v0,  v1,  v2,  v3;
 int32x4_t   vi0, vi1, vi2, vi3;
 
 v0 = vld1q_f32(src0_ptr);           // load and scale values to LUT indices
 v0 = vaddq_f32(v0, *vsx);           // add is for the minimum LUT value
 
 v1 = vld1q_f32(src1_ptr);
 v1 = vaddq_f32(v1, *vsx);
 
 v2 = vld1q_f32(src2_ptr);
 v2 = vaddq_f32(v2, *vsx);
 
 v3 = vld1q_f32(src3_ptr);
 v3 = vaddq_f32(v3, *vsx);
 
 v0 = vmulq_f32(v0, *vsy);           // multiply is for the max LUT value and/or range
 v1 = vmulq_f32(v1, *vsy);
 v2 = vmulq_f32(v2, *vsy);
 v3 = vmulq_f32(v3, *vsy);
 
 v0 = vmaxq_f32(v0, *vsmin);         // clip to LUT indices range
 v1 = vmaxq_f32(v1, *vsmin);
 v2 = vmaxq_f32(v2, *vsmin);
 v3 = vmaxq_f32(v3, *vsmin);
 
 v0 = vminq_f32(v0, *vsmax);
 v1 = vminq_f32(v1, *vsmax);
 v2 = vminq_f32(v2, *vsmax);
 v3 = vminq_f32(v3, *vsmax);
 
 vi0 = vcvtq_s32_f32(v0);            // prepare for gather
 vi1 = vcvtq_s32_f32(v1);
 vi2 = vcvtq_s32_f32(v2);
 vi3 = vcvtq_s32_f32(v3);
 
 vst1q_s32(tmp0, vi0);
 vst1q_s32(tmp1, vi1);
 vst1q_s32(tmp2, vi2);
 vst1q_s32(tmp3, vi3);
 
 dest0_ptr[0] = lut[tmp0[0]];        // perform gather, which is only a scalar
 dest0_ptr[1] = lut[tmp0[1]];        // op and probably stalls all over the place.
 dest0_ptr[2] = lut[tmp0[2]];
 dest0_ptr[3] = lut[tmp0[3]];
 
 dest1_ptr[0] = lut[tmp1[0]];        // (this sets dest to the RGBA8888 vals
 dest1_ptr[1] = lut[tmp1[1]];        //  from the LUT)
 dest1_ptr[2] = lut[tmp1[2]];
 dest1_ptr[3] = lut[tmp1[3]];
 
 dest2_ptr[0] = lut[tmp2[0]];
 dest2_ptr[1] = lut[tmp2[1]];
 dest2_ptr[2] = lut[tmp2[2]];
 dest2_ptr[3] = lut[tmp2[3]];
 
 dest3_ptr[0] = lut[tmp3[0]];
 dest3_ptr[1] = lut[tmp3[1]];
 dest3_ptr[2] = lut[tmp3[2]];
 dest3_ptr[3] = lut[tmp3[3]];
 }//_scale_lut_inner_loop_NEON
 
 
 
 
 // it's likely this is faster and can replace the vdsp functions(?)
 
 void gbLUT_vsasmvclipvindex_NEON(const float* src,
 const float* lut,
 const float  x,
 const float  y,
 const float  clipMin,
 const float  clipMax,
 float*       dest,
 const size_t n)
 {
 int32_t tmp0[4] __attribute__ ((aligned(16)));
 int32_t tmp1[4] __attribute__ ((aligned(16)));
 int32_t tmp2[4] __attribute__ ((aligned(16)));
 int32_t tmp3[4] __attribute__ ((aligned(16)));
 
 const float*  src0_ptr = (const float*)src;
 const float*  src1_ptr = (const float*)src + 4;
 const float*  src2_ptr = (const float*)src + 8;
 const float*  src3_ptr = (const float*)src + 12;
 
 float* dest0_ptr = (float*)dest;
 float* dest1_ptr = (float*)dest + 4;
 float* dest2_ptr = (float*)dest + 8;
 float* dest3_ptr = (float*)dest + 12;
 
 float32x4_t vsx   = vdupq_n_f32(x);
 float32x4_t vsy   = vdupq_n_f32(y);
 float32x4_t vsmin = vdupq_n_f32(clipMin);
 float32x4_t vsmax = vdupq_n_f32(clipMax);
 
 for (size_t i = 0UL; i < n; i += 16UL)
 {
 _scale_lut_inner_loop_NEON(dest0_ptr, dest1_ptr, dest2_ptr, dest3_ptr,
 lut,
 src0_ptr, src1_ptr, src2_ptr, src3_ptr,
 &vsx, &vsy, &vsmin, &vsmax,
 tmp0, tmp1, tmp2, tmp3);
 
 src0_ptr += 16;
 src1_ptr += 16;
 src2_ptr += 16;
 src3_ptr += 16;
 
 dest0_ptr += 16;
 dest1_ptr += 16;
 dest2_ptr += 16;
 dest3_ptr += 16;
 }//for
 }//gbLUT_vsasmvclipvindex_NEON
 
 
 
 // possible test version
 // ugly, but maybe faster?  dunno.
 static inline void gbLUT_vsasmvclipvindexa_NEON(const float* src,
 const float* alpha,
 const float* lut,
 const float  x,
 const float  y,
 const float  clipMin,
 const float  clipMax,
 const float  mapAlpha,
 float*       dest,
 const size_t n)
 {
 int32_t tmp0[4] __attribute__ ((aligned(16)));
 int32_t tmp1[4] __attribute__ ((aligned(16)));
 int32_t tmp2[4] __attribute__ ((aligned(16)));
 int32_t tmp3[4] __attribute__ ((aligned(16)));
 
 const float*  src0_ptr = (const float*)src;
 const float*  src1_ptr = (const float*)src + 4;
 const float*  src2_ptr = (const float*)src + 8;
 const float*  src3_ptr = (const float*)src + 12;
 
 const float*  alpha0_ptr = (const float*)alpha;
 const float*  alpha1_ptr = (const float*)alpha + 4;
 const float*  alpha2_ptr = (const float*)alpha + 8;
 const float*  alpha3_ptr = (const float*)alpha + 12;
 
 float* dest0_ptr = (float*)dest;
 float* dest1_ptr = (float*)dest + 4;
 float* dest2_ptr = (float*)dest + 8;
 float* dest3_ptr = (float*)dest + 12;
 
 float32x4_t vsx   = vdupq_n_f32(x);
 float32x4_t vsy   = vdupq_n_f32(y);
 float32x4_t vsmin = vdupq_n_f32(clipMin);
 float32x4_t vsmax = vdupq_n_f32(clipMax);
 
 uint8x16_t  sa = vdupq_n_u8((uint8_t)truncf(mapAlpha*255.0F));
 float32x4_t sf = vdupq_n_f32(255.0F);
 
 
 for (size_t i = 0UL; i < n; i += 16UL)
 {
 _scale_lut_inner_loop_NEON(dest0_ptr, dest1_ptr, dest2_ptr, dest3_ptr,
 lut,
 src0_ptr, src1_ptr, src2_ptr, src3_ptr,
 &vsx, &vsy, &vsmin, &vsmax,
 tmp0, tmp1, tmp2, tmp3);
 
 _alpha_inner_loop_NEON(dest0_ptr, alpha0_ptr, alpha1_ptr, alpha2_ptr, alpha3_ptr, &sa, &sf);
 
 src0_ptr += 16;
 src1_ptr += 16;
 src2_ptr += 16;
 src3_ptr += 16;
 
 dest0_ptr += 16;
 dest1_ptr += 16;
 dest2_ptr += 16;
 dest3_ptr += 16;
 
 alpha0_ptr += 16;
 alpha1_ptr += 16;
 alpha2_ptr += 16;
 alpha3_ptr += 16;
 }//for
 }//gbLUT_vsasmvclipvindexa_NEON
 */















// alpha first
// This one kind of sucks compared to new variants, but not actually used.
void gbLUT_ApplyAlphaToARGB_v6_NEON(uint8_t*     restrict rgba,
                                    const float* restrict alphaVector,
                                    const float           mapAlpha,
                                    const size_t          n)
{
    float32x4_t minAlpha = vdupq_n_f32(truncf(mapAlpha*255.0F));
    
    uint8x8x4_t rgba8x8x4;
    
    uint16x8_t r16x8;
    uint16x8_t g16x8;
    uint16x8_t b16x8;
    uint16x8_t a16x8;
    
    uint16x4_t r16x4_0;
    uint16x4_t r16x4_1;
    uint16x4_t g16x4_0;
    uint16x4_t g16x4_1;
    uint16x4_t b16x4_0;
    uint16x4_t b16x4_1;
    uint16x4_t a16x4_0;
    uint16x4_t a16x4_1;
    
    uint32x4_t r32x4_0;
    uint32x4_t r32x4_1;
    uint32x4_t g32x4_0;
    uint32x4_t g32x4_1;
    uint32x4_t b32x4_0;
    uint32x4_t b32x4_1;
    uint32x4_t a32x4_0;
    uint32x4_t a32x4_1;
    
    int32x4_t sr32x4_0;
    int32x4_t sr32x4_1;
    int32x4_t sg32x4_0;
    int32x4_t sg32x4_1;
    int32x4_t sb32x4_0;
    int32x4_t sb32x4_1;
    int32x4_t sa32x4_0;
    int32x4_t sa32x4_1;
    
    float32x4_t fr32x4_0;
    float32x4_t fr32x4_1;
    float32x4_t fg32x4_0;
    float32x4_t fg32x4_1;
    float32x4_t fb32x4_0;
    float32x4_t fb32x4_1;
    float32x4_t fa32x4_0;
    float32x4_t fa32x4_1;
    
    float32x4_t fAlphas32x4_0;
    float32x4_t fAlphas32x4_1;
    
    size_t rgbaIdx = 0UL;
    
    for (size_t i=0; i<n; i+=8UL) // alphaVector: 256-bit read; 32 bytes, 8 floats
    {
        fAlphas32x4_0 = vld1q_f32( &(alphaVector[i])     ); //  4 elements
        fAlphas32x4_1 = vld1q_f32( &(alphaVector[i+4UL]) ); //  4 elements
        rgba8x8x4     = vld4_u8(   &(rgba[rgbaIdx])      ); // 32 elements (8 RGBA groups)
        
        a16x8 = vmovl_u8(rgba8x8x4.val[0UL]);
        r16x8 = vmovl_u8(rgba8x8x4.val[1UL]);
        g16x8 = vmovl_u8(rgba8x8x4.val[2UL]);
        b16x8 = vmovl_u8(rgba8x8x4.val[3UL]);
        
        r16x4_0 =  vget_low_u16(r16x8);
        r16x4_1 = vget_high_u16(r16x8);
        g16x4_0 =  vget_low_u16(g16x8);
        g16x4_1 = vget_high_u16(g16x8);
        b16x4_0 =  vget_low_u16(b16x8);
        b16x4_1 = vget_high_u16(b16x8);
        a16x4_0 =  vget_low_u16(a16x8);
        a16x4_1 = vget_high_u16(a16x8);
        
        r32x4_0 = vmovl_u16(r16x4_0);
        r32x4_1 = vmovl_u16(r16x4_1);
        g32x4_0 = vmovl_u16(g16x4_0);
        g32x4_1 = vmovl_u16(g16x4_1);
        b32x4_0 = vmovl_u16(b16x4_0);
        b32x4_1 = vmovl_u16(b16x4_1);
        a32x4_0 = vmovl_u16(a16x4_0);
        a32x4_1 = vmovl_u16(a16x4_1);
        
        
        fr32x4_0 = vcvtq_f32_s32(r32x4_0);      // int32_t -> float
        fr32x4_1 = vcvtq_f32_s32(r32x4_1);
        fg32x4_0 = vcvtq_f32_s32(g32x4_0);      // uint/sint will be same here
        fg32x4_1 = vcvtq_f32_s32(g32x4_1);
        fb32x4_0 = vcvtq_f32_s32(b32x4_0);
        fb32x4_1 = vcvtq_f32_s32(b32x4_1);
        fa32x4_0 = vcvtq_f32_s32(a32x4_0);
        fa32x4_1 = vcvtq_f32_s32(a32x4_1);
        
        fr32x4_0 = vmulq_f32(fAlphas32x4_0, fr32x4_0);
        fr32x4_1 = vmulq_f32(fAlphas32x4_1, fr32x4_1);
        fg32x4_0 = vmulq_f32(fAlphas32x4_0, fg32x4_0);
        fg32x4_1 = vmulq_f32(fAlphas32x4_1, fg32x4_1);
        fb32x4_0 = vmulq_f32(fAlphas32x4_0, fb32x4_0);
        fb32x4_1 = vmulq_f32(fAlphas32x4_1, fb32x4_1);
        fa32x4_0 = vmulq_f32(fAlphas32x4_0, fa32x4_0);
        fa32x4_1 = vmulq_f32(fAlphas32x4_1, fa32x4_1);
        
        fa32x4_0 = vabsq_f32(fa32x4_0);
        fa32x4_1 = vabsq_f32(fa32x4_1);
        fa32x4_0 = vmaxq_f32(fa32x4_0, minAlpha);
        fa32x4_1 = vmaxq_f32(fa32x4_1, minAlpha);
        
        // float -> sint32: this is to exploit the less costly (?)
        //                  saturation that clips neg values to 0 later
        //                  (whereas float -> uint32 does a free(?) fabsf)
        
        sr32x4_0 = vcvtq_s32_f32(fr32x4_0);
        sr32x4_1 = vcvtq_s32_f32(fr32x4_1);
        sg32x4_0 = vcvtq_s32_f32(fg32x4_0);
        sg32x4_1 = vcvtq_s32_f32(fg32x4_1);
        sb32x4_0 = vcvtq_s32_f32(fb32x4_0);
        sb32x4_1 = vcvtq_s32_f32(fb32x4_1);
        sa32x4_0 = vcvtq_s32_f32(fa32x4_0);
        sa32x4_1 = vcvtq_s32_f32(fa32x4_1);
        
        r16x4_0 = vqmovun_s32(sr32x4_0);
        r16x4_1 = vqmovun_s32(sr32x4_1);
        g16x4_0 = vqmovun_s32(sg32x4_0);
        g16x4_1 = vqmovun_s32(sg32x4_1);
        b16x4_0 = vqmovun_s32(sb32x4_0);
        b16x4_1 = vqmovun_s32(sb32x4_1);
        a16x4_0 = vqmovun_s32(sa32x4_0);
        a16x4_1 = vqmovun_s32(sa32x4_1);
        
        r16x8 = vcombine_u16(r16x4_0, r16x4_1);
        g16x8 = vcombine_u16(g16x4_0, g16x4_1);
        b16x8 = vcombine_u16(b16x4_0, b16x4_1);
        a16x8 = vcombine_u16(a16x4_0, a16x4_1);
        
        rgba8x8x4.val[0UL] = vmovn_u16(a16x8);
        rgba8x8x4.val[1UL] = vmovn_u16(r16x8);
        rgba8x8x4.val[2UL] = vmovn_u16(g16x8);
        rgba8x8x4.val[3UL] = vmovn_u16(b16x8);
        
        vst4_u8( &(rgba[rgbaIdx]), rgba8x8x4);
        
        rgbaIdx += 32UL;
    }//for
}//_ApplyAlphatoRGBADirect_Scalar_HiP32_v6_NEON









void gbLUT_ApplyAlphaToRGBA_v8_NEONvsSSE_A(uint8_t*     restrict rgba,
                                           const float* restrict alphaVector,
                                           const float           mapAlpha,
                                           const size_t          n)
{
#ifdef NEON2SSE_H
    const uint8_t minAlpha_u8   = (uint8_t)truncf(mapAlpha*255.0F);
    uint8x8_t     minAlpha_u8x8 = vdupq_n_u8(minAlpha_u8);
    uint8x8x4_t rgba8x8x4;
    uint16x8_t r16x8;
    uint16x8_t g16x8;
    uint16x8_t b16x8;
    uint16x8_t a16x8;
    int32x4_t alphas_s32x4_0;
    int32x4_t alphas_s32x4_1;
    int32x4_t absals_s32x4_0;
    int32x4_t absals_s32x4_1;
    uint16x4_t alphas_u16x4_0;
    uint16x4_t alphas_u16x4_1;
    uint16x4_t absals_u16x4_0;
    uint16x4_t absals_u16x4_1;
    uint16x8_t alphas_u16x8;
    uint16x8_t absals_u16x8;
    float32x4_t fAlphas32x4_0;
    float32x4_t fAlphas32x4_1;
    float32x4_t f255_f32x4   = vdupq_n_f32(255.0F);
    uint16x8_t  one_u16x8   = vdupq_n_u16(1);   // 2015-02-26 ND: fix for rounding errors on premultiply?
    size_t rgbaIdx = 0;
    
    for (size_t i=0; i<n; i+=8UL) // 256-bit read; 32 bytes, 8 floats
    {
        fAlphas32x4_0   = gbA_vld1q_f32( &(alphaVector[i]    ) );       //  4 elements
        fAlphas32x4_1   = gbA_vld1q_f32( &(alphaVector[i+4UL]) );       //  4 elements
        rgba8x8x4       = gbA_vld4_u8(   &(rgba[rgbaIdx]     ) );       // 32 elements (8 RGBA groups)
        
        r16x8           = vmovl_u8(rgba8x8x4.val[0UL]);             // uint8_t -> uint16_t
        g16x8           = vmovl_u8(rgba8x8x4.val[1UL]);
        b16x8           = vmovl_u8(rgba8x8x4.val[2UL]);
        a16x8           = vmovl_u8(rgba8x8x4.val[3UL]);
        
        fAlphas32x4_0   = vmulq_f32(fAlphas32x4_0, f255_f32x4);     // norm [-1.0 ... 1.0] -> [-255.0 ... 255.0]
        fAlphas32x4_1   = vmulq_f32(fAlphas32x4_1, f255_f32x4);
        
        alphas_s32x4_0  = vcvtq_s32_f32(fAlphas32x4_0);             // rgb mul factor
        alphas_s32x4_1  = vcvtq_s32_f32(fAlphas32x4_1);
        absals_s32x4_0  = vabsq_s32(alphas_s32x4_0);                // alpha mul factor
        absals_s32x4_1  = vabsq_s32(alphas_s32x4_1);
        
        alphas_u16x4_0  = vqmovun_s16(alphas_s32x4_0);              // s32->u16, clip to 0.
        alphas_u16x4_1  = vqmovun_s16(alphas_s32x4_1);
        absals_u16x4_0  = vqmovun_s16(absals_s32x4_0);
        absals_u16x4_1  = vqmovun_s16(absals_s32x4_1);
        
        alphas_u16x8    = vcombine_u16(alphas_u16x4_0, alphas_u16x4_1);
        absals_u16x8    = vcombine_u16(absals_u16x4_0, absals_u16x4_1);
        
        // 2015-02-26 ND: replace vmul with vadd+vmul so rsn doesn't round down 255*255 to 254, etc.
        
        alphas_u16x8 = vaddq_u16(alphas_u16x8, one_u16x8);
        absals_u16x8 = vaddq_u16(absals_u16x8, one_u16x8);
        
        r16x8 = vmulq_u16(r16x8, alphas_u16x8);
        g16x8 = vmulq_u16(g16x8, alphas_u16x8);
        b16x8 = vmulq_u16(b16x8, alphas_u16x8);
        a16x8 = vmulq_u16(a16x8, absals_u16x8);
        
        rgba8x8x4.val[0UL] = vqshrn_n_u16(r16x8, 8);                // 16->8 with >>8
        rgba8x8x4.val[1UL] = vqshrn_n_u16(g16x8, 8);
        rgba8x8x4.val[2UL] = vqshrn_n_u16(b16x8, 8);
        rgba8x8x4.val[3UL] = vmaxq_u8(vqshrn_n_u16(a16x8, 8), minAlpha_u8x8);
        
        gbA_vst4_u8( &(rgba[rgbaIdx]), rgba8x8x4);
        
        rgbaIdx += 32UL;
    }//for
#endif
}//_ApplyAlphaToRGBADirect_HiP32_v8_NEONvsSSE_A



// v8 changes:
// ================
// - 2015-02-26 ND: Attempt fix for rounding error from uint8 * uint8 >> 8
// - 2014-09-23 ND: Complete rewrite using 1/2 the ops.  Instead of promoting
//                  LUT RGBAs to floats, instead take the 32-bit float alpha
//                  channel down to uint16_t.
//
void gbLUT_ApplyAlphaToRGBA_v8_NEON(uint8_t*     restrict rgba,
                                    const float* restrict alphaVector,
                                    const float           mapAlpha,
                                    const size_t          n)
{
#ifdef NEON2SSE_H
    if (GB_IS_PTR_ALIGNED(rgba) && GB_IS_PTR_ALIGNED(alphaVector))
    {
        gbLUT_ApplyAlphaToRGBA_v8_NEONvsSSE_A(rgba, alphaVector, mapAlpha, n);
        return;
    }//if
#endif
    
    const uint8_t minAlpha_u8   = (uint8_t)truncf(mapAlpha*255.0F);
    uint8x8_t     minAlpha_u8x8 = vdupq_n_u8(minAlpha_u8);
    
    uint8x8x4_t rgba8x8x4;
    uint16x8_t r16x8;
    uint16x8_t g16x8;
    uint16x8_t b16x8;
    uint16x8_t a16x8;
    
    int32x4_t alphas_s32x4_0;
    int32x4_t alphas_s32x4_1;
    int32x4_t absals_s32x4_0;
    int32x4_t absals_s32x4_1;
    
    uint16x4_t alphas_u16x4_0;
    uint16x4_t alphas_u16x4_1;
    uint16x4_t absals_u16x4_0;
    uint16x4_t absals_u16x4_1;
    
    uint16x8_t alphas_u16x8;
    uint16x8_t absals_u16x8;
    
    float32x4_t fAlphas32x4_0;
    float32x4_t fAlphas32x4_1;
    
    float32x4_t f255_f32x4  = vdupq_n_f32(255.0F);
    uint16x8_t  one_u16x8   = vdupq_n_u16(1);   // 2015-02-26 ND: fix for rounding errors on premultiply?
    size_t rgbaIdx = 0;
    
    for (size_t i=0; i<n; i+=8UL) // 256-bit read; 32 bytes, 8 floats
    {
        fAlphas32x4_0   = vld1q_f32( &(alphaVector[i]    ) );       //  4 elements
        fAlphas32x4_1   = vld1q_f32( &(alphaVector[i+4UL]) );       //  4 elements
        rgba8x8x4       = vld4_u8(   &(rgba[rgbaIdx]     ) );       // 32 elements (8 RGBA groups)
        
        r16x8           = vmovl_u8(rgba8x8x4.val[0UL]);             // uint8_t -> uint16_t
        g16x8           = vmovl_u8(rgba8x8x4.val[1UL]);
        b16x8           = vmovl_u8(rgba8x8x4.val[2UL]);
        a16x8           = vmovl_u8(rgba8x8x4.val[3UL]);
        
        fAlphas32x4_0   = vmulq_f32(fAlphas32x4_0, f255_f32x4);     // norm [-1.0 ... 1.0] -> [-255.0 ... 255.0]
        fAlphas32x4_1   = vmulq_f32(fAlphas32x4_1, f255_f32x4);
        
        alphas_s32x4_0  = vcvtq_s32_f32(fAlphas32x4_0);             // rgb mul factor
        alphas_s32x4_1  = vcvtq_s32_f32(fAlphas32x4_1);
        absals_s32x4_0  = vabsq_s32(alphas_s32x4_0);                // alpha mul factor
        absals_s32x4_1  = vabsq_s32(alphas_s32x4_1);
        
        alphas_u16x4_0  = vqmovun_s16(alphas_s32x4_0);              // s32->u16, clip to 0.
        alphas_u16x4_1  = vqmovun_s16(alphas_s32x4_1);
        absals_u16x4_0  = vqmovun_s16(absals_s32x4_0);
        absals_u16x4_1  = vqmovun_s16(absals_s32x4_1);
        
        // MATH EXPLOIT
        //
        // For source int32x4_t values [0 ... +255], vqmovun_s16 can be used
        // instead of vqmovun_s32 to convert s32->u16.  This has the advantage
        // of being faster when converted to SSE intrinsics.
        // (_mm_packus_epi16 vs. _mm_packus1_epi32, which is SSE41 only)
        
        // actual:
        // 00 00 00 AA | 00 00 00 BB | 00 00 00 CC | 00 00 00 DD
        // vqmovun_s16:
        // 00 00|00 AA | 00 00|00 BB | 00 00|00 CC | 00 00|00 DD
        //    00|   AA |    00|   BB |    00|   CC |    00|   DD
        
        // actual:
        // 00 00 00 AA | 00 00 00 BB | 00 00 00 CC | 00 00 00 DD
        // vqmovun_s32:
        // 00 00 00 AA | 00 00 00 BB | 00 00 00 CC | 00 00 00 DD
        //      |00 AA |      |00 BB |      |00 CC |      |00 DD
        
        alphas_u16x8    = vcombine_u16(alphas_u16x4_0, alphas_u16x4_1);
        absals_u16x8    = vcombine_u16(absals_u16x4_0, absals_u16x4_1);
        
        // 2015-02-26 ND: replace vmul with vadd+vmul so rsn doesn't round down 255*255 to 254, etc.
        
        alphas_u16x8 = vaddq_u16(alphas_u16x8, one_u16x8);
        absals_u16x8 = vaddq_u16(absals_u16x8, one_u16x8);
        
        r16x8 = vmulq_u16(r16x8, alphas_u16x8);
        g16x8 = vmulq_u16(g16x8, alphas_u16x8);
        b16x8 = vmulq_u16(b16x8, alphas_u16x8);
        a16x8 = vmulq_u16(a16x8, absals_u16x8);
        
        rgba8x8x4.val[0UL] = vqshrn_n_u16(r16x8, 8);                // 16->8 with >>8
        rgba8x8x4.val[1UL] = vqshrn_n_u16(g16x8, 8);
        rgba8x8x4.val[2UL] = vqshrn_n_u16(b16x8, 8);
        rgba8x8x4.val[3UL] = vmaxq_u8(vqshrn_n_u16(a16x8, 8), minAlpha_u8x8);
        
        vst4_u8( &(rgba[rgbaIdx]), rgba8x8x4);
        
        rgbaIdx += 32UL;
    }//for
}//_ApplyAlphaToRGBADirect_HiP32_v8_NEON











void gbLUT_ApplyAlphaToRGBA_NoPremultiply_v2_NEONvsSSE_A(uint8_t*     restrict rgba,
                                                         const float* restrict alphaVector,
                                                         const float           mapAlpha,
                                                         const size_t          n)
{
#ifdef NEON2SSE_H
    const uint8_t minAlpha_u8   = (uint8_t)truncf(mapAlpha*255.0F);
    uint8x8_t     minAlpha_u8x8 = vdupq_n_u8(minAlpha_u8);
    uint8x8x4_t rgba8x8x4;
    uint16x8_t a16x8;
    
    int32x4_t alphas_s32x4_0;
    int32x4_t alphas_s32x4_1;
    int32x4_t absals_s32x4_0;
    int32x4_t absals_s32x4_1;
    uint16x4_t alphas_u16x4_0;
    uint16x4_t alphas_u16x4_1;
    uint16x4_t absals_u16x4_0;
    uint16x4_t absals_u16x4_1;
    uint16x8_t alphas_u16x8;
    uint16x8_t absals_u16x8;
    float32x4_t fAlphas32x4_0;
    float32x4_t fAlphas32x4_1;
    float32x4_t f25532x4   = vdupq_n_f32(255.0F);
    
    uint8x8_t zero_u8x8 = vdupq_n_u8(0);
    uint8x8_t comp_u8x8;
    uint8x8_t alphas_u8x8;
    
    uint16x8_t  one_u16x8  = vdupq_n_u16(1);   // 2015-02-26 ND: fix for rounding errors on premultiply?
    
    size_t rgbaIdx = 0;
    
    for (size_t i=0; i<n; i+=8UL) // 256-bit read; 32 bytes, 8 floats
    {
        fAlphas32x4_0   = gbA_vld1q_f32( &(alphaVector[i]    ) );   //  4 elements
        fAlphas32x4_1   = gbA_vld1q_f32( &(alphaVector[i+4UL]) );   //  4 elements
        rgba8x8x4       = gbA_vld4_u8(   &(rgba[rgbaIdx]     ) );   // 32 elements (8 RGBA groups)
        
        a16x8           = vmovl_u8(rgba8x8x4.val[3UL]);
        
        fAlphas32x4_0   = vmulq_f32(fAlphas32x4_0, f25532x4);       // norm [-1.0 ... 1.0] -> [-255.0 ... 255.0]
        fAlphas32x4_1   = vmulq_f32(fAlphas32x4_1, f25532x4);
        
        alphas_s32x4_0  = vcvtq_s32_f32(fAlphas32x4_0);             // rgb mul factor
        alphas_s32x4_1  = vcvtq_s32_f32(fAlphas32x4_1);
        absals_s32x4_0  = vabsq_s32(alphas_s32x4_0);                // alpha mul factor
        absals_s32x4_1  = vabsq_s32(alphas_s32x4_1);
        
        alphas_u16x4_0  = vqmovun_s16(alphas_s32x4_0);              // s32->u16, clip to 0.
        alphas_u16x4_1  = vqmovun_s16(alphas_s32x4_1);
        absals_u16x4_0  = vqmovun_s16(absals_s32x4_0);
        absals_u16x4_1  = vqmovun_s16(absals_s32x4_1);
        
        alphas_u16x8    = vcombine_u16(alphas_u16x4_0, alphas_u16x4_1);
        absals_u16x8    = vcombine_u16(absals_u16x4_0, absals_u16x4_1);
        
        alphas_u8x8     = vmovn_u16(alphas_u16x8);
        comp_u8x8       = vceq_u8(alphas_u8x8, zero_u8x8);
        
        absals_u16x8    = vaddq_u16(one_u16x8, absals_u16x8);
        a16x8           = vmulq_u16(a16x8, absals_u16x8);
        
        rgba8x8x4.val[0UL] = vbslq_u8(comp_u8x8, zero_u8x8, rgba8x8x4.val[0UL]);
        rgba8x8x4.val[1UL] = vbslq_u8(comp_u8x8, zero_u8x8, rgba8x8x4.val[1UL]);
        rgba8x8x4.val[2UL] = vbslq_u8(comp_u8x8, zero_u8x8, rgba8x8x4.val[2UL]);
        rgba8x8x4.val[3UL] = vmaxq_u8(vqshrn_n_u16(a16x8, 8), minAlpha_u8x8);
        
        gbA_vst4_u8( &(rgba[rgbaIdx]), rgba8x8x4);
        
        rgbaIdx += 32UL;
    }//for
#endif
}//_ApplyAlphaToRGBADirect_NoPremultiply_v2_NEONvsSSE_A



// v2 changes:
// ================
// - 2015-02-26 ND: Fix for truncated rounding for alpha channel
// - 2014-09-23 ND: Complete rewrite using 1/2 the ops.  Instead of promoting
//                  LUT RGBAs to floats, instead take the 32-bit float alpha
//                  channel down to uint16_t.
//
//                  30 ops vs. 69 ops per loop (NEON)
//
void gbLUT_ApplyAlphaToRGBA_NoPremultiply_v2_NEON(uint8_t*     restrict rgba,
                                                  const float* restrict alphaVector,
                                                  const float           mapAlpha,
                                                  const size_t          n)
{
#ifdef NEON2SSE_H
    if (GB_IS_PTR_ALIGNED(rgba) && GB_IS_PTR_ALIGNED(alphaVector))
    {
        gbLUT_ApplyAlphaToRGBA_NoPremultiply_v2_NEONvsSSE_A(rgba, alphaVector, mapAlpha, n);
        return;
    }//if
#endif
    
    const uint8_t minAlpha_u8   = (uint8_t)truncf(mapAlpha*255.0F);
    uint8x8_t     minAlpha_u8x8 = vdupq_n_u8(minAlpha_u8);
    uint8x8x4_t rgba8x8x4;
    uint16x8_t a16x8;
    int32x4_t alphas_s32x4_0;
    int32x4_t alphas_s32x4_1;
    int32x4_t absals_s32x4_0;
    int32x4_t absals_s32x4_1;
    uint16x4_t alphas_u16x4_0;
    uint16x4_t alphas_u16x4_1;
    uint16x4_t absals_u16x4_0;
    uint16x4_t absals_u16x4_1;
    uint16x8_t alphas_u16x8;
    uint16x8_t absals_u16x8;
    float32x4_t fAlphas32x4_0;
    float32x4_t fAlphas32x4_1;
    float32x4_t f25532x4   = vdupq_n_f32(255.0F);
    uint8x8_t zero_u8x8 = vdupq_n_u8(0);
    uint8x8_t comp_u8x8;
    uint8x8_t alphas_u8x8;
    uint16x8_t  one_u16x8  = vdupq_n_u16(1);   // 2015-02-26 ND: fix for rounding errors on premultiply?
    
    size_t rgbaIdx = 0;
    
    // 30 OPS (NEON)/cycle
    
    for (size_t i=0; i<n; i+=8UL) // 256-bit read; 32 bytes, 8 floats
    {
        fAlphas32x4_0   = vld1q_f32( &(alphaVector[i]    ) );       //  4 elements
        fAlphas32x4_1   = vld1q_f32( &(alphaVector[i+4UL]) );       //  4 elements
        rgba8x8x4       = vld4_u8(   &(rgba[rgbaIdx]     ) );       // 32 elements (8 RGBA groups)
        
        a16x8           = vmovl_u8(rgba8x8x4.val[3UL]);
        
        fAlphas32x4_0   = vmulq_f32(fAlphas32x4_0, f25532x4);       // norm [-1.0 ... 1.0] -> [-255.0 ... 255.0]
        fAlphas32x4_1   = vmulq_f32(fAlphas32x4_1, f25532x4);
        
        alphas_s32x4_0  = vcvtq_s32_f32(fAlphas32x4_0);             // rgb mul factor
        alphas_s32x4_1  = vcvtq_s32_f32(fAlphas32x4_1);
        absals_s32x4_0  = vabsq_s32(alphas_s32x4_0);                // alpha mul factor
        absals_s32x4_1  = vabsq_s32(alphas_s32x4_1);
        
        alphas_u16x4_0  = vqmovun_s16(alphas_s32x4_0);              // s32->u16, clip to 0.
        alphas_u16x4_1  = vqmovun_s16(alphas_s32x4_1);
        absals_u16x4_0  = vqmovun_s16(absals_s32x4_0);
        absals_u16x4_1  = vqmovun_s16(absals_s32x4_1);
        
        alphas_u16x8    = vcombine_u16(alphas_u16x4_0, alphas_u16x4_1);
        absals_u16x8    = vcombine_u16(absals_u16x4_0, absals_u16x4_1);
        
        alphas_u8x8     = vmovn_u16(alphas_u16x8);
        comp_u8x8       = vceq_u8(alphas_u8x8, zero_u8x8);
        
        absals_u16x8    = vaddq_u16(one_u16x8, absals_u16x8);
        a16x8           = vmulq_u16(a16x8, absals_u16x8);
        
        rgba8x8x4.val[0UL] = vbslq_u8(comp_u8x8, zero_u8x8, rgba8x8x4.val[0UL]);
        rgba8x8x4.val[1UL] = vbslq_u8(comp_u8x8, zero_u8x8, rgba8x8x4.val[1UL]);
        rgba8x8x4.val[2UL] = vbslq_u8(comp_u8x8, zero_u8x8, rgba8x8x4.val[2UL]);
        rgba8x8x4.val[3UL] = vmaxq_u8(vqshrn_n_u16(a16x8, 8), minAlpha_u8x8);
        
        vst4_u8( &(rgba[rgbaIdx]), rgba8x8x4);
        
        rgbaIdx += 32UL;
    }//for
}//_ApplyAlphaToRGBADirect_NoPremultiply_v2_NEON





void gbLUT_ApplyAlphatoRGBA_v2_Scalar(uint8_t*     restrict rgba,
                                      const float* restrict alphaVector,
                                      const float           mapAlpha,
                                      const int             rgbaTypeId,
                                      const size_t          n)
{
    int rc;
    int gc;
    int bc;
    int ac;
    
    gbImage_RGBA_GetRGBAVectorEntryOffsets(rgbaTypeId, &rc, &gc, &bc, &ac);
    
    const uint8_t mapAlpha8 = (uint8_t)truncf(mapAlpha * 255.0F);
    
    uint32_t rcls3 = rc << 3;
    uint32_t gcls3 = gc << 3;
    uint32_t bcls3 = bc << 3;
    uint32_t acls3 = ac << 3;
    
    uint32_t compound_rgba;
    
    uint32_t r_u32;
    uint32_t g_u32;
    uint32_t b_u32;
    uint32_t a_u32;
    
    uint32_t ff_u32 = 0xFF;
    
    float f_R;
    float f_G;
    float f_B;
    float f_A;
    float f_amul;
    
    float minAlpha = (float)mapAlpha8;
    float minVal = 0.0F;
    
    for (size_t i=0; i<n; i++)
    {
        compound_rgba = ((uint32_t*)rgba)[i];
        
        f_amul = alphaVector[i];
        
        r_u32 = (compound_rgba >> rcls3) & ff_u32;
        g_u32 = (compound_rgba >> gcls3) & ff_u32;
        b_u32 = (compound_rgba >> bcls3) & ff_u32;
        a_u32 = (compound_rgba >> acls3) & ff_u32;
        
        f_R = (float)r_u32;
        f_G = (float)g_u32;
        f_B = (float)b_u32;
        f_A = (float)a_u32;
        
        f_R *= f_amul;
        f_G *= f_amul;
        f_B *= f_amul;
        f_A *= f_amul;
        
        // could be skipped if no shadow alpha
        f_R = MAX(f_R, minVal);
        f_G = MAX(f_G, minVal);
        f_B = MAX(f_B, minVal);
        f_A = fabsf(f_A);
        f_A = MAX(f_A, minAlpha);
        
        r_u32 = (uint32_t)f_R;
        g_u32 = (uint32_t)f_G;
        b_u32 = (uint32_t)f_B;
        a_u32 = (uint32_t)f_A;
        
        r_u32 <<= rcls3;
        g_u32 <<= gcls3;
        b_u32 <<= bcls3;
        a_u32 <<= acls3;
        
        compound_rgba = 0;
        
        compound_rgba |= r_u32;
        compound_rgba |= g_u32;
        compound_rgba |= b_u32;
        compound_rgba |= a_u32;
        
        ((uint32_t*)rgba)[i] = compound_rgba;
    }//for
}//gbLUT_ApplyAlphatoRGBA_v2_Scalar




void gbLUT_vindex_NEON(const float* restrict src,
                       const float* restrict idx,
                       float*       restrict dest,
                       const size_t n)
{
    //printf("gbLUT_vindex_NEON: Start.  src=%s, idx=%s, dest=%s, n=%zu\n", src == NULL ? "<NULL>" : "(non-NULL)", idx == NULL ? "<NULL>" : "(non-NULL)", dest == NULL ? "<NULL>" : "(non-NULL)", n);
    
#pragma ivdep
    
    for (size_t i=0; i<n; i++)
    {
        dest[i] = src[(int32_t)idx[i]];
    }
}//gbLUT_vindex_NEON


/*
void gbLUT_vindex_NEON(const float* src,
                       const float* idx,
                       float*       dest,
                       const size_t n)
{
    //printf("gbLUT_vindex_NEON: Start.  src=%s, idx=%s, dest=%s, n=%zu\n", src == NULL ? "<NULL>" : "(non-NULL)", idx == NULL ? "<NULL>" : "(non-NULL)", dest == NULL ? "<NULL>" : "(non-NULL)", n);
    
    int32_t tmp[16] __attribute__ ((aligned(16)));
    
    int32_t* tmp0 = tmp;
    int32_t* tmp1 = tmp + 4;
    int32_t* tmp2 = tmp + 8;
    int32_t* tmp3 = tmp + 12;
    
    float32x4_t src0, src1, src2, src3;
    int32x4_t   ssr0, ssr1, ssr2, ssr3;
    
    const float*  src0_ptr = (const float*)idx;
    const float*  src1_ptr = (const float*)idx + 4;
    const float*  src2_ptr = (const float*)idx + 8;
    const float*  src3_ptr = (const float*)idx + 12;
    
    float* dest0_ptr = (float*)dest;
    float* dest1_ptr = (float*)dest + 4;
    float* dest2_ptr = (float*)dest + 8;
    float* dest3_ptr = (float*)dest + 12;
    
    for (size_t i = 0; i < n; i += 16)
    {
        src0 = vld1q_f32(src0_ptr);
        ssr0 = vcvtq_s32_f32(src0);
        
        src1 = vld1q_f32(src1_ptr);
        ssr1 = vcvtq_s32_f32(src1);
        
        src2 = vld1q_f32(src2_ptr);
        ssr2 = vcvtq_s32_f32(src2);
        
        src3 = vld1q_f32(src3_ptr);
        ssr3 = vcvtq_s32_f32(src3);
        
        vst1q_s32(tmp0, ssr0);
        vst1q_s32(tmp1, ssr1);
        vst1q_s32(tmp2, ssr2);
        vst1q_s32(tmp3, ssr3);
        
        dest0_ptr[0] = src[tmp0[0]];
        dest0_ptr[1] = src[tmp0[1]];
        dest0_ptr[2] = src[tmp0[2]];
        dest0_ptr[3] = src[tmp0[3]];
        
        dest1_ptr[0] = src[tmp1[0]];
        dest1_ptr[1] = src[tmp1[1]];
        dest1_ptr[2] = src[tmp1[2]];
        dest1_ptr[3] = src[tmp1[3]];
        
        dest2_ptr[0] = src[tmp2[0]];
        dest2_ptr[1] = src[tmp2[1]];
        dest2_ptr[2] = src[tmp2[2]];
        dest2_ptr[3] = src[tmp2[3]];
        
        dest3_ptr[0] = src[tmp3[0]];
        dest3_ptr[1] = src[tmp3[1]];
        dest3_ptr[2] = src[tmp3[2]];
        dest3_ptr[3] = src[tmp3[3]];
        
        src0_ptr += 16;
        src1_ptr += 16;
        src2_ptr += 16;
        src3_ptr += 16;
        
        dest0_ptr += 16;
        dest1_ptr += 16;
        dest2_ptr += 16;
        dest3_ptr += 16;
    }//for
}//gbLUT_vindex_NEON
*/


/*
void gbLUT_vindex_NEON(const float* src,
                       const float* idx,
                       float*       dest,
                       const size_t n)
{
    //printf("gbLUT_vindex_NEON: Start.  src=%s, idx=%s, dest=%s, n=%zu\n", src == NULL ? "<NULL>" : "(non-NULL)", idx == NULL ? "<NULL>" : "(non-NULL)", dest == NULL ? "<NULL>" : "(non-NULL)", n);
    
    int32_t tmp0[4] __attribute__ ((aligned(16)));
    int32_t tmp1[4] __attribute__ ((aligned(16)));
    int32_t tmp2[4] __attribute__ ((aligned(16)));
    int32_t tmp3[4] __attribute__ ((aligned(16)));
    
    int32_t tmp4[4] __attribute__ ((aligned(16)));
    int32_t tmp5[4] __attribute__ ((aligned(16)));
    int32_t tmp6[4] __attribute__ ((aligned(16)));
    int32_t tmp7[4] __attribute__ ((aligned(16)));
    
    float32x4_t src0, src1, src2, src3;
    float32x4_t src4, src5, src6, src7;
    int32x4_t   ssr0, ssr1, ssr2, ssr3;
    int32x4_t   ssr4, ssr5, ssr6, ssr7;
    
    const float*  src0_ptr = (const float*)idx;
    const float*  src1_ptr = (const float*)idx + 4;
    const float*  src2_ptr = (const float*)idx + 8;
    const float*  src3_ptr = (const float*)idx + 12;
    
    const float*  src4_ptr = (const float*)idx + 16;
    const float*  src5_ptr = (const float*)idx + 20;
    const float*  src6_ptr = (const float*)idx + 24;
    const float*  src7_ptr = (const float*)idx + 28;
    
          float* dest0_ptr = (float*)dest;
          float* dest1_ptr = (float*)dest + 4;
          float* dest2_ptr = (float*)dest + 8;
          float* dest3_ptr = (float*)dest + 12;
    
          float* dest4_ptr = (float*)dest + 16;
          float* dest5_ptr = (float*)dest + 20;
          float* dest6_ptr = (float*)dest + 24;
          float* dest7_ptr = (float*)dest + 28;
    
    for (size_t i = 0; i < n; i += 32)
    {
        src0 = vld1q_f32(src0_ptr);
        ssr0 = vcvtq_s32_f32(src0);
        
        src1 = vld1q_f32(src1_ptr);
        ssr1 = vcvtq_s32_f32(src1);
        
        src2 = vld1q_f32(src2_ptr);
        ssr2 = vcvtq_s32_f32(src2);
        
        src3 = vld1q_f32(src3_ptr);
        ssr3 = vcvtq_s32_f32(src3);
        
        vst1q_s32(tmp0, ssr0);
        vst1q_s32(tmp1, ssr1);
        vst1q_s32(tmp2, ssr2);
        vst1q_s32(tmp3, ssr3);
        
        src4 = vld1q_f32(src4_ptr);
        ssr4 = vcvtq_s32_f32(src4);
        
        src5 = vld1q_f32(src5_ptr);
        ssr5 = vcvtq_s32_f32(src5);
        
        src6 = vld1q_f32(src6_ptr);
        ssr6 = vcvtq_s32_f32(src6);
        
        src7 = vld1q_f32(src7_ptr);
        ssr7 = vcvtq_s32_f32(src7);
        
        vst1q_s32(tmp4, ssr4);
        vst1q_s32(tmp5, ssr5);
        vst1q_s32(tmp6, ssr6);
        vst1q_s32(tmp7, ssr7);
        
        dest0_ptr[0] = src[tmp0[0]];
        dest0_ptr[1] = src[tmp0[1]];
        dest0_ptr[2] = src[tmp0[2]];
        dest0_ptr[3] = src[tmp0[3]];
        
        dest1_ptr[0] = src[tmp1[0]];
        dest1_ptr[1] = src[tmp1[1]];
        dest1_ptr[2] = src[tmp1[2]];
        dest1_ptr[3] = src[tmp1[3]];
        
        dest2_ptr[0] = src[tmp2[0]];
        dest2_ptr[1] = src[tmp2[1]];
        dest2_ptr[2] = src[tmp2[2]];
        dest2_ptr[3] = src[tmp2[3]];
        
        dest3_ptr[0] = src[tmp3[0]];
        dest3_ptr[1] = src[tmp3[1]];
        dest3_ptr[2] = src[tmp3[2]];
        dest3_ptr[3] = src[tmp3[3]];
        
         src0_ptr += 32;
         src1_ptr += 32;
         src2_ptr += 32;
         src3_ptr += 32;
        
        dest0_ptr += 32;
        dest1_ptr += 32;
        dest2_ptr += 32;
        dest3_ptr += 32;
        
        dest4_ptr[0] = src[tmp4[0]];
        dest4_ptr[1] = src[tmp4[1]];
        dest4_ptr[2] = src[tmp4[2]];
        dest4_ptr[3] = src[tmp4[3]];
        
        dest5_ptr[0] = src[tmp5[0]];
        dest5_ptr[1] = src[tmp5[1]];
        dest5_ptr[2] = src[tmp5[2]];
        dest5_ptr[3] = src[tmp5[3]];
        
        dest6_ptr[0] = src[tmp6[0]];
        dest6_ptr[1] = src[tmp6[1]];
        dest6_ptr[2] = src[tmp6[2]];
        dest6_ptr[3] = src[tmp6[3]];
        
        dest7_ptr[0] = src[tmp7[0]];
        dest7_ptr[1] = src[tmp7[1]];
        dest7_ptr[2] = src[tmp7[2]];
        dest7_ptr[3] = src[tmp7[3]];
        
        src4_ptr += 32;
        src5_ptr += 32;
        src6_ptr += 32;
        src7_ptr += 32;
        
        dest4_ptr += 32;
        dest5_ptr += 32;
        dest6_ptr += 32;
        dest7_ptr += 32;
    }//for
}//gbLUT_vindex_NEON
*/

/*
 // gbLUT_vabssmclip_NEON: UNUSED
 //
 // vabs, vsmul, vclip
 //
 // data[i] = MIN(MAX(fabsf(data[i]) * x, clipMin), clipMax);
 //
 static inline void gbLUT_vabssmclip_NEON(float* data,
 float  x,
 float  clipMin,
 float  clipMax,
 int    n)
 {
 float   _vsx[4] __attribute__ ((aligned(16)));
 float _vsmin[4] __attribute__ ((aligned(16)));
 float _vsmax[4] __attribute__ ((aligned(16)));
 
 for (int i=0; i<4; i++)
 {
 _vsx[i]   = x;
 _vsmin[i] = clipMin;
 _vsmax[i] = clipMax;
 }//for
 
 float32x4_t vsx;//   = { x, x, x, x };
 float32x4_t vsmin;// = { clipMin, clipMin, clipMin, clipMin };
 float32x4_t vsmax;// = { clipMax, clipMax, clipMax, clipMax };
 
 float32x4_t v0;
 float32x4_t v1;
 float32x4_t v2;
 float32x4_t v3;
 
 vsx   = vld1q_f32(&(_vsx[0]));
 vsmin = vld1q_f32(&(_vsmin[0]));
 vsmax = vld1q_f32(&(_vsmax[0]));
 
 int i4;
 int i8;
 int i12;
 
 for (int i = 0; i < n; i += 16)
 {
 i4  = i +  4; // MATH
 i8  = i +  8;
 i12 = i + 12;
 
 v0 = vld1q_f32(&(data[i]));
 v1 = vld1q_f32(&(data[i4]));
 v2 = vld1q_f32(&(data[i8]));
 v3 = vld1q_f32(&(data[i12]));
 
 v0 = vabsq_f32(v0);
 v1 = vabsq_f32(v1);
 v2 = vabsq_f32(v2);
 v3 = vabsq_f32(v3);
 
 v0 = vmulq_f32(v0, vsx);
 v1 = vmulq_f32(v1, vsx);
 v2 = vmulq_f32(v2, vsx);
 v3 = vmulq_f32(v3, vsx);
 
 v0 = vmaxq_f32(v0, vsmin);
 v1 = vmaxq_f32(v1, vsmin);
 v2 = vmaxq_f32(v2, vsmin);
 v3 = vmaxq_f32(v3, vsmin);
 
 v0 = vminq_f32(v0, vsmax);
 v1 = vminq_f32(v1, vsmax);
 v2 = vminq_f32(v2, vsmax);
 v3 = vminq_f32(v3, vsmax);
 
 vst1q_f32(&(data[i]),   v0);
 vst1q_f32(&(data[i4]),  v1);
 vst1q_f32(&(data[i8]),  v2);
 vst1q_f32(&(data[i12]), v3);
 }//for
 }//gbLUT_vabssmclip_NEON
 */
