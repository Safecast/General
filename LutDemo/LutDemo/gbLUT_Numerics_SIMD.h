//
//  gbLUT_Numerics_SIMD.h
//  Safecast
//
//  Created by Nicholas Dolezal on 11/6/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#ifndef gbLUT_Numerics_SIMD_h
#define gbLUT_Numerics_SIMD_h

#include <stdio.h>
#include <Accelerate/Accelerate.h>
#include "gbCommon_TypeDefs.h"
#include "gbImage_Utils.h"

#ifndef ARM_NEON
    #import "NEONvsSSE_5.h"
    #import "NEONvsSSE_gbExt.h"
#endif

void gbLUT_vsasmclip_NEON(float*       data,
                          const float  x,
                          const float  y,
                          const float  clipMin,
                          const float  clipMax,
                          const size_t n);

void gbLUT_ApplyAlphatoRGBA_v2_Scalar(uint8_t*     restrict rgba,
                                      const float* restrict alphaVector,
                                      const float           mapAlpha,
                                      const int             rgbaTypeId,
                                      const size_t          n);

void gbLUT_ApplyAlphaToRGBA_NoPremultiply_v2_NEON(uint8_t*     restrict rgba,
                                                  const float* restrict alphaVector,
                                                  const float           mapAlpha,
                                                  const size_t          n);

void gbLUT_ApplyAlphaToRGBA_v8_NEON(uint8_t*     restrict rgba,
                                    const float* restrict alphaVector,
                                    const float           mapAlpha,
                                    const size_t          n);

void gbLUT_ApplyAlphaToARGB_v6_NEON(uint8_t*     rgba,
                                    const float* alphaVector,
                                    const float  mapAlpha,
                                    const size_t n);

void gbLUT_vindex_NEON(const float* restrict src,
                       const float* restrict idx,
                       float*       restrict dest,
                       const size_t          n);

#endif
