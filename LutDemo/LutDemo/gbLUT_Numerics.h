//
//  gbLUT_Numerics.h
//  Safecast
//
//  Created by Nicholas Dolezal on 11/6/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#ifndef gbLUT_Numerics_h
#define gbLUT_Numerics_h

#include <stdio.h>
#include <Accelerate/Accelerate.h>
#include "gbCommon_TypeDefs.h"
#include "gbImage_Utils.h"
#include "gbLUT_Types.h"
#include "gbLUT_Numerics_SIMD.h"

void gbLUT_ConvertPlanarRGBAFFFFtoRGBA8888(const float* restrict f_R,
                                           const float* restrict f_G,
                                           const float* restrict f_B,
                                           const float* restrict f_A,
                                           const bool            isNormalized,
                                           const float           alphaVal,
                                           uint8_t*     restrict rgba,
                                           const int             rgbaTypeId,
                                           const size_t          n);

void gbLUT_ReverseLUT(float*       restrict r,
                      float*       restrict g,
                      float*       restrict b,
                      const size_t          n);

void gbLUT_DiscretizeLUT(float*       restrict r,
                         float*       restrict g,
                         float*       restrict b,
                         const int             steps,
                         const size_t          n);

void gbLUT_MakeLinIdxVector(float**      linIdxOut,
                            const size_t n);

void gbLUT_MakeLogIdx(float*       restrict ref_lutln,
                      float*       restrict ref_f_linIdx,
                      const size_t          n);

void gbLUT_MakeLog10Idx(float*       restrict ref_lutln,
                        float*       restrict ref_f_linIdx,
                        const size_t          n);

void gbLUT_InterpolateLUT(const float* restrict src_r,
                          const float* restrict src_g,
                          const float* restrict src_b,
                          const size_t          src_n,
                          float*       restrict dest_r,
                          float*       restrict dest_g,
                          float*       restrict dest_b,
                          const size_t          dest_n,
                          const bool            nearest_neighbor_only);

void gbLUT_ApplyMapAlpha(float*       restrict r,
                         float*       restrict g,
                         float*       restrict b,
                         float*       restrict a,
                         const float           map_alpha,
                         const bool            is_opaque,
                         const size_t          n);

void gbLUT_LUTtoLUT_XTREME(float*       restrict r,
                           float*       restrict g,
                           float*       restrict b,
                           float*       restrict xtreme,
                           const float           map_alpha,
                           const float           pt_alpha,
                           const int             rgbaTypeId,
                           const size_t          n);

void gbLUT_U08_to_F32_vImage(const uint8_t* restrict src_u08,
                             float*         restrict dest_f32,
                             const size_t   n);



#endif
