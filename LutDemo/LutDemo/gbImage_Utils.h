//
//  gbImage_Utils.h
//  Safecast
//
//  Created by Nicholas Dolezal on 9/9/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#ifndef __gbImage_Utils__
#define __gbImage_Utils__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "gbCommon_TypeDefs.h"

void gbImage_RGBA_GetRGBAVectorEntryOffsets(const int rgbaTypeId,
                                            int*      rcOut,
                                            int*      gcOut,
                                            int*      bcOut,
                                            int*      acOut);

void gbImage_GetVectorStartIdxForWindow(const int* rwx0,
                                        const int* rwy0,
                                        const int* rwx1,
                                        const int* rwy1,
                                        int*       startIdx);

void gbImage_GetVectorLengthForWindow(const int* rwx0,
                                      const int* rwy0,
                                      const int* rwx1,
                                      const int* rwy1,
                                      const int* startIdx,
                                      int*       n);

void gbImage_GetVectorIdxsForWindow(const int* rwx0,
                                    const int* rwy0,
                                    const int* rwx1,
                                    const int* rwy1,
                                    int*       startIdx,
                                    int*       n);

void gbImage_CopyByRow_PlanarF(const float* restrict src,
                               const size_t          src_width,
                               const size_t          src_height,
                               const size_t          src_rowBytes,
                               float* restrict       dest,
                               const size_t          dest_width,
                               const size_t          dest_height,
                               const size_t          dest_rowBytes);

/*
void gbImage_GetROI_ForZtoXYZ(const uint32_t src_z,
                              const uint32_t dest_x,
                              const uint32_t dest_y,
                              const uint32_t dest_z,
                              const size_t   w,
                              const size_t   h,
                              size_t*        roi_x,
                              size_t*        roi_y,
                              size_t*        roi_w,
                              size_t*        roi_h);
*/

void gbImage_GetROI_ForZtoXYZ(const uint64_t src_z,
                              const uint64_t dest_x,
                              const uint64_t dest_y,
                              const uint64_t dest_z,
                              const size_t   w,
                              const size_t   h,
                              size_t*        roi_x,
                              size_t*        roi_y,
                              size_t*        roi_w,
                              size_t*        roi_h);

void gbImage_GetAdjustedROI_ForKernelExtent(const size_t kExtentPx,
                                            const size_t w,
                                            const size_t h,
                                            const size_t roi_w,
                                            const size_t roi_h,
                                            int64_t*     padded_w,
                                            int64_t*     padded_h,
                                            int64_t*     padded_roi_w,
                                            int64_t*     padded_roi_h);

void gbImage_GetAdjustedROI_ForKernelExtent_AllSides(const size_t kExtentPx,
                                                     const bool   kIsDownRightOnly,
                                                     const size_t w,
                                                     const size_t h,
                                                     const size_t roi_x,
                                                     const size_t roi_y,
                                                     const size_t roi_w,
                                                     const size_t roi_h,
                                                     int64_t*     padded_x,
                                                     int64_t*     padded_y,
                                                     int64_t*     padded_w,
                                                     int64_t*     padded_h,
                                                     int64_t*     padded_roi_x,
                                                     int64_t*     padded_roi_y,
                                                     int64_t*     padded_roi_w,
                                                     int64_t*     padded_roi_h);

void gbImage_ClampAdjustedROI_ToSrcWH(const size_t  w,
                                      const size_t  h,
                                      const int64_t roi_x,
                                      const int64_t roi_y,
                                      int64_t*      padded_roi_w,
                                      int64_t*      padded_roi_h);

void gbImage_ClampAdjustedROI_ToSrcWH_AllSides(const size_t w,
                                               const size_t h,
                                               int64_t*     padded_roi_x,
                                               int64_t*     padded_roi_y,
                                               int64_t*     padded_roi_w,
                                               int64_t*     padded_roi_h);

void gbImage_GetAdjustedROI_ForKernelExtent_AllSides_Clamped(const size_t kExtentPx,
                                                             const bool   kIsDownRightOnly,
                                                             const size_t w,
                                                             const size_t h,
                                                             const size_t roi_x,
                                                             const size_t roi_y,
                                                             const size_t roi_w,
                                                             const size_t roi_h,
                                                             int64_t*     padded_x,
                                                             int64_t*     padded_y,
                                                             int64_t*     padded_w,
                                                             int64_t*     padded_h,
                                                             int64_t*     padded_roi_x,
                                                             int64_t*     padded_roi_y,
                                                             int64_t*     padded_roi_w,
                                                             int64_t*     padded_roi_h);


#endif
