//
//  NEONvsSSE_gbExt.h
//  GeigerBotOSX
//
//  Created by Nicholas Dolezal on 9/7/14.
//  Copyright (c) 2014 n/a. All rights reserved.
//

// Modified copypasta from NEONvsSSE_5.h


// This modifies all the single vec load/stores to assume 16-*BYTE* alignment
// prefixed by "gbA_".  Also includes a few multi load/stores.
//
// The reason to do this is performance.  Otherwise it tests the pointer in
// every load/store and SIMD and branchy don't get along well at all.
//
// Provided that all pointer increments / decrements are at least 16 bits,
// this requires testing the pointer(s) ONCE before the main work loop.


// This is no benefit to ARM archs and should never been called for ARM.  It
// is only for SSE, and is a lazy way of improving performance slightly without
// a rewrite using SSE intrinsics.


// ATTENTION: THIS MAY BREAK WITH NEW VERSIONS OF NEONvSSE_5.h.

// ATTENTION: THIS WILL FAIL/CRASH FOR NON-16 BYTE ALIGNED POINTERS.






#ifndef NEON2SSE_gbExt_h
#define NEON2SSE_gbExt_h

#include "NEONvsSSE_5.h"

#define GBU_LOAD_SI128(ptr) \
    _mm_loadu_si128((__m128i*)(ptr));


#define GBA_LOAD_SI128(ptr) \
    _mm_load_si128((__m128i*)(ptr))


uint8x16_t gbA_vld1q_u8(__transfersize(16) uint8_t const * ptr);         // VLD1.8 {d0, d1}, [r0]
#define gbA_vld1q_u8 GBA_LOAD_SI128

uint16x8_t gbA_vld1q_u16(__transfersize(8) uint16_t const * ptr);         // VLD1.16 {d0, d1}, [r0]
#define gbA_vld1q_u16 GBA_LOAD_SI128

uint32x4_t gbA_vld1q_u32(__transfersize(4) uint32_t const * ptr);         // VLD1.32 {d0, d1}, [r0]
#define gbA_vld1q_u32 GBA_LOAD_SI128

uint64x2_t gbA_vld1q_u64(__transfersize(2) uint64_t const * ptr);         // VLD1.64 {d0, d1}, [r0]
#define gbA_vld1q_u64 GBA_LOAD_SI128

int8x16_t gbA_vld1q_s8(__transfersize(16) int8_t const * ptr);         // VLD1.8 {d0, d1}, [r0]
#define gbA_vld1q_s8 GBA_LOAD_SI128

int16x8_t gbA_vld1q_s16(__transfersize(8) int16_t const * ptr);         // VLD1.16 {d0, d1}, [r0]
#define gbA_vld1q_s16 GBA_LOAD_SI128

int32x4_t gbA_vld1q_s32(__transfersize(4) int32_t const * ptr);         // VLD1.32 {d0, d1}, [r0]
#define gbA_vld1q_s32 GBA_LOAD_SI128

int64x2_t gbA_vld1q_s64(__transfersize(2) int64_t const * ptr);         // VLD1.64 {d0, d1}, [r0]
#define gbA_vld1q_s64 GBA_LOAD_SI128

float16x8_t gbA_vld1q_f16(__transfersize(8) __fp16 const * ptr);         // VLD1.16 {d0, d1}, [r0]

float32x4_t gbA_vld1q_f32(__transfersize(4) float32_t const * ptr);         // VLD1.32 {d0, d1}, [r0]
INLINE float32x4_t gbA_vld1q_f32(__transfersize(4) float32_t const * ptr)
{
    return _mm_load_ps(ptr);
}

poly8x16_t gbA_vld1q_p8(__transfersize(16) poly8_t const * ptr);         // VLD1.8 {d0, d1}, [r0]
#define gbA_vld1q_p8  gbA_LOAD_SI128

poly16x8_t gbA_vld1q_p16(__transfersize(8) poly16_t const * ptr);         // VLD1.16 {d0, d1}, [r0]
#define gbA_vld1q_p16 gbA_LOAD_SI128

uint8x8_t gbA_vld1_u8(__transfersize(8) uint8_t const * ptr);         // VLD1.8 {d0}, [r0]
#define gbA_vld1_u8(ptr) _mm_loadl_epi64((__m128i*)(ptr))

uint16x4_t gbA_vld1_u16(__transfersize(4) uint16_t const * ptr);         // VLD1.16 {d0}, [r0]
#define gbA_vld1_u16(ptr) _mm_loadl_epi64((__m128i*)(ptr))

uint32x2_t gbA_vld1_u32(__transfersize(2) uint32_t const * ptr);         // VLD1.32 {d0}, [r0]
#define gbA_vld1_u32(ptr) _mm_loadl_epi64((__m128i*)(ptr))


uint64x1_t gbA_vld1_u64(__transfersize(1) uint64_t const * ptr);         // VLD1.64 {d0}, [r0]
#define gbA_vld1_u64(ptr) _mm_loadl_epi64((__m128i*)(ptr))


int8x8_t gbA_vld1_s8(__transfersize(8) int8_t const * ptr);         // VLD1.8 {d0}, [r0]
#define gbA_vld1_s8 gbA_vld1_u8

int16x4_t gbA_vld1_s16(__transfersize(4) int16_t const * ptr);         // VLD1.16 {d0}, [r0]
#define gbA_vld1_s16 gbA_vld1_u16

int32x2_t gbA_vld1_s32(__transfersize(2) int32_t const * ptr);         // VLD1.32 {d0}, [r0]
#define gbA_vld1_s32 gbA_vld1_u32

int64x1_t gbA_vld1_s64(__transfersize(1) int64_t const * ptr);         // VLD1.64 {d0}, [r0]
#define gbA_vld1_s64 gbA_vld1_u64

float16x4_t gbA_vld1_f16(__transfersize(4) __fp16 const * ptr);         // VLD1.16 {d0}, [r0]
// IA32 SIMD doesn't work with 16bit floats currently, so need to go to 32 bit like _mm_set_ps (ptr[3], ptr[2], ptr[1], ptr[0]);

float32x2_t gbA_vld1_f32(__transfersize(2) float32_t const * ptr);         // VLD1.32 {d0}, [r0]
#define gbA_vld1_f32(ptr) _mm_castsi128_ps(_mm_loadl_epi64((__m128i*)(ptr)))

poly8x8_t gbA_vld1_p8(__transfersize(8) poly8_t const * ptr);         // VLD1.8 {d0}, [r0]
#define gbA_vld1_p8 gbA_vld1_u8

poly16x4_t gbA_vld1_p16(__transfersize(4) poly16_t const * ptr);         // VLD1.16 {d0}, [r0]
#define gbA_vld1_p16 gbA_vld1_u16














//*************************************************************************************
//********************************* Store **********************************************
//*************************************************************************************
// If ptr is 16bit aligned and you  need to store data without cache pollution then use void _mm_stream_si128 ((__m128i*)ptr, val);
//here we assume the case of  NOT 16bit aligned ptr possible. If it is aligned we could to use _mm_store_si128 like shown in the following macro
#define GBA_STORE_SI128(ptr, val) \
    _mm_store_si128 ((__m128i*)(ptr), val);

void gbA_vst1q_u8(__transfersize(16) uint8_t * ptr, uint8x16_t val);         // VST1.8 {d0, d1}, [r0]
#define gbA_vst1q_u8 GBA_STORE_SI128

void gbA_vst1q_u16(__transfersize(8) uint16_t * ptr, uint16x8_t val);         // VST1.16 {d0, d1}, [r0]
#define gbA_vst1q_u16 STORE_SI128

void gbA_vst1q_u32(__transfersize(4) uint32_t * ptr, uint32x4_t val);         // VST1.32 {d0, d1}, [r0]
#define gbA_vst1q_u32 STORE_SI128

void gbA_vst1q_u64(__transfersize(2) uint64_t * ptr, uint64x2_t val);         // VST1.64 {d0, d1}, [r0]
#define gbA_vst1q_u64 STORE_SI128

void gbA_vst1q_s8(__transfersize(16) int8_t * ptr, int8x16_t val);         // VST1.8 {d0, d1}, [r0]
#define gbA_vst1q_s8 STORE_SI128

void gbA_vst1q_s16(__transfersize(8) int16_t * ptr, int16x8_t val);         // VST1.16 {d0, d1}, [r0]
#define gbA_vst1q_s16 STORE_SI128

void gbA_vst1q_s32(__transfersize(4) int32_t * ptr, int32x4_t val);         // VST1.32 {d0, d1}, [r0]
#define gbA_vst1q_s32 STORE_SI128

void gbA_vst1q_s64(__transfersize(2) int64_t * ptr, int64x2_t val);         // VST1.64 {d0, d1}, [r0]
#define gbA_vst1q_s64 STORE_SI128

void gbA_vst1q_f16(__transfersize(8) __fp16 * ptr, float16x8_t val);         // VST1.16 {d0, d1}, [r0]
// IA32 SIMD doesn't work with 16bit floats currently

void gbA_vst1q_f32(__transfersize(4) float32_t * ptr, float32x4_t val);         // VST1.32 {d0, d1}, [r0]
INLINE void gbA_vst1q_f32(__transfersize(4) float32_t * ptr, float32x4_t val)
{
    _mm_store_ps (ptr, val);
}

void gbA_vst1q_p8(__transfersize(16) poly8_t * ptr, poly8x16_t val);         // VST1.8 {d0, d1}, [r0]
#define gbA_vst1q_p8  gbA_vst1q_u8

void gbA_vst1q_p16(__transfersize(8) poly16_t * ptr, poly16x8_t val);         // VST1.16 {d0, d1}, [r0]
#define gbA_vst1q_p16 gbA_vst1q_u16



void gbA_vst1_s8(__transfersize(8) int8_t * ptr, int8x8_t val);         // VST1.8 {d0}, [r0]
#define gbA_vst1_s8(ptr,val) gbA_vst1_u8((uint8_t*)ptr,val)

void gbA_vst1_s16(__transfersize(4) int16_t * ptr, int16x4_t val);         // VST1.16 {d0}, [r0]
#define gbA_vst1_s16(ptr,val) gbA_vst1_u16((uint16_t*)ptr,val)

void gbA_vst1_s32(__transfersize(2) int32_t * ptr, int32x2_t val);         // VST1.32 {d0}, [r0]
#define gbA_vst1_s32(ptr,val) gbA_vst1_u32((uint32_t*)ptr,val)

void gbA_vst1_s64(__transfersize(1) int64_t * ptr, int64x1_t val);         // VST1.64 {d0}, [r0]
#define gbA_vst1_s64(ptr,val) gbA_vst1_u64((uint64_t*)ptr,val)

void gbA_vst1_p8(__transfersize(8) poly8_t * ptr, poly8x8_t val);         // VST1.8 {d0}, [r0]
#define gbA_vst1_p8 gbA_vst1_u8

void gbA_vst1_p16(__transfersize(4) poly16_t * ptr, poly16x4_t val);         // VST1.16 {d0}, [r0]
#define gbA_vst1_p16 gbA_vst1_u16









uint8x8x4_t gbA_vld4_u8(__transfersize(32) uint8_t const * ptr);         // VLD4.8 {d0, d1, d2, d3}, [r0]
INLINE uint8x8x4_t gbA_vld4_u8(__transfersize(32) uint8_t const * ptr)         // VLD4.8 {d0, d1, d2, d3}, [r0]
{
    uint8x8x4_t v;
    __m128i sh0, sh1;
    ALIGN_16 int8_t mask4_8[16] = {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15};
    
    v.val[0] = gbA_vld1q_u8(( ptr));         //load first 64-bits in val[0] and val[1]
    v.val[1] = gbA_vld1q_u8(( ptr + 16));         //load third and forth 64-bits in val[2], val[3]
    
    sh0 = _mm_shuffle_epi8(v.val[0], *(__m128i*)mask4_8);
    sh1 = _mm_shuffle_epi8(v.val[1], *(__m128i*)mask4_8);
    v.val[0] = _mm_unpacklo_epi32(sh0,sh1);         //0,4,8,12,16,20,24,28, 1,5,9,13,17,21,25,29
    v.val[2] = _mm_unpackhi_epi32(sh0,sh1);         //2,6,10,14,18,22,26,30, 3,7,11,15,19,23,27,31
    v.val[1] = _mm_shuffle_epi32(v.val[0],SWAP_HI_LOW32);
    v.val[3] = _mm_shuffle_epi32(v.val[2],SWAP_HI_LOW32);
    
    return v;
}



INLINE void gbA_vst4_u8_ptr(__transfersize(32) uint8_t * ptr, uint8x8x4_t* val)
{
    uint8x8x4_t v;
    __m128i sh0, sh1;
    sh0 = _mm_unpacklo_epi8(val->val[0],val->val[1]);         // a0,b0,a1,b1,a2,b2,a3,b3,a4,b4,a5,b5, a6,b6,a7,b7,
    sh1 = _mm_unpacklo_epi8(val->val[2],val->val[3]);         // c0,d0,c1,d1,c2,d2,c3,d3, c4,d4,c5,d5,c6,d6,c7,d7
    v.val[0] = _mm_unpacklo_epi16(sh0,sh1);         // a0,b0,c0,d0,a1,b1,c1,d1,a2,b2,c2,d2,a3,b3,c3,d3,
    v.val[2] = _mm_unpackhi_epi16(sh0,sh1);         //a4,b4,c4,d4,a5,b5,c5,d5, a6,b6,c6,d6,a7,b7,c7,d7
    gbA_vst1q_u8(ptr,        v.val[0]);
    gbA_vst1q_u8((ptr + 16), v.val[2]);
}
#define gbA_vst4_u8(ptr, val) gbA_vst4_u8_ptr(ptr, &val)






#endif
