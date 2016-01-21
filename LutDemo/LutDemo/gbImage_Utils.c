//
//  gbImage_Utils.c
//  Safecast
//
//  Created by Nicholas Dolezal on 9/9/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#include "gbImage_Utils.h"


void gbImage_GetVectorStartIdxForWindow(const int* rwx0,
                                        const int* rwy0,
                                        const int* rwx1,
                                        const int* rwy1,
                                        int* startIdx)
{
    *startIdx = (*rwy0 << 8) + *rwx0;
}//gbImage_GetVectorStartIdxForWindow

void gbImage_GetVectorLengthForWindow(const int* rwx0,
                                      const int* rwy0,
                                      const int* rwx1,
                                      const int* rwy1,
                                      const int* startIdx,
                                      int* n)
{
    *n = (*rwy1 << 8) + *rwx1 - *startIdx + 1;
}//gbImage_GetVectorStartIdxForWindow

void gbImage_GetVectorIdxsForWindow(const int* rwx0,
                                    const int* rwy0,
                                    const int* rwx1,
                                    const int* rwy1,
                                    int* startIdx,
                                    int* n)
{
    gbImage_GetVectorStartIdxForWindow(rwx0, rwy0, rwx1, rwy1, startIdx);
    gbImage_GetVectorLengthForWindow  (rwx0, rwy0, rwx1, rwy1, startIdx, n);
}//gbImage_GetVectorIdxsForWindow

// ==========================
// gbImage_CopyByRow_PlanarF:
// ==========================
//
// Performs a per-row copy from src to dest.  Assumes origin are the same.
//
// This is used for copying data by row to buffers of dissimilar width.
//
void gbImage_CopyByRow_PlanarF(const float* restrict src,
                               const size_t  src_width,
                               const size_t  src_height,
                               const size_t  src_rowBytes,
                               float* restrict dest,
                               const size_t  dest_width,
                               const size_t  dest_height,
                               const size_t  dest_rowBytes)
{
    const size_t src_rowWidth  = src_rowBytes  / sizeof(float);
    const size_t dest_rowWidth = dest_rowBytes / sizeof(float);
    const size_t rowCopyBytes  = (src_width < dest_width ? src_width : dest_width) * sizeof(float);
    
    for (size_t y = 0UL; y < (src_height < dest_height ? src_height : dest_height); y++)
    {
        memcpy(dest + y * dest_rowWidth,
               src  + y * src_rowWidth,
               rowCopyBytes);
    }//if
}//gbImage_CopyByRow_PlanarF



// =============================================================================
//                           Image Geometry Helpers
// =============================================================================



// ==================
// _GetROI_ForZtoXYZ:
// ==================
//
// Returns the ROI in pixels for an image at src_z, relative to that tile,
// which will be used to fill a buffer at dest_x, dest_y @ dest_z.
//
// Note this result is only meaningful for nearest neighbor interpolation.
// All other methods have a resampling kernel extent that must be accounted
// for by padding.
//
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
                              size_t*        roi_h)
{
    const uint32_t w_u32   = (uint32_t)w;
    const uint32_t h_u32   = (uint32_t)h;
    const uint32_t zs      = (dest_z - src_z);
    
    const uint32_t src_x   = dest_x >> zs;
    const uint32_t src_y   = dest_y >> zs;
    
    const uint32_t src_px  = src_x * w_u32;
    const uint32_t src_py  = src_y * h_u32;
    
    const uint32_t dest_px = dest_x * w_u32;
    const uint32_t dest_py = dest_y * h_u32;
    
    uint32_t sx = (dest_px >> zs) - src_px;
    uint32_t sy = (dest_py >> zs) - src_py;
    uint32_t sw = w_u32 >> zs;
    uint32_t sh = h_u32 >> zs;
    
    sw = sw > 1 ? sw : 1;
    sh = sh > 1 ? sh : 1;
    
    *roi_x = sx;
    *roi_y = sy;
    *roi_w = sw;
    *roi_h = sh;
}//gbImage_GetROI_ForZtoXYZ
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
                              size_t*        roi_h)
{
    const uint64_t w_u32   = (uint64_t)w;
    const uint64_t h_u32   = (uint64_t)h;
    const uint64_t zs      = (dest_z - src_z);
    
    const uint64_t src_x   = dest_x >> zs;
    const uint64_t src_y   = dest_y >> zs;
    
    const uint64_t src_px  = src_x * w_u32;
    const uint64_t src_py  = src_y * h_u32;
    
    const uint64_t dest_px = dest_x * w_u32;
    const uint64_t dest_py = dest_y * h_u32;
    
    uint64_t sx = (dest_px >> zs) - src_px;
    uint64_t sy = (dest_py >> zs) - src_py;
    uint64_t sw = w_u32 >> zs;
    uint64_t sh = h_u32 >> zs;
    
    sw = sw > 1 ? sw : 1;
    sh = sh > 1 ? sh : 1;
    
    *roi_x = sx;
    *roi_y = sy;
    *roi_w = sw;
    *roi_h = sh;
}//gbImage_GetROI_ForZtoXYZ

// ================================
// _GetAdjustedROI_ForKernelExtent:
// ================================
//
// For use with _GetROI_ForZtoXYZ.
//
// After obtaining the base ROI, this provides the necessary padding for both
// the src and dest image buffers for the given resampling kernel extent.
//
// If the images are not padded, they will ALL have border artifacts.
//

void gbImage_GetAdjustedROI_ForKernelExtent(const size_t kExtentPx,
                                            const size_t w,
                                            const size_t h,
                                            const size_t roi_w,
                                            const size_t roi_h,
                                            int64_t*     padded_w,
                                            int64_t*     padded_h,
                                            int64_t*     padded_roi_w,
                                            int64_t*     padded_roi_h)
{
    const size_t zoomScale   = w / roi_w;
    const size_t src_kReach  = kExtentPx % 2 != 0 ? (kExtentPx >> 1) + 1 : kExtentPx >> 1;
    const size_t dest_kReach = src_kReach * zoomScale;
    
    *padded_roi_w = roi_w + src_kReach;
    *padded_roi_h = roi_h + src_kReach;
    *padded_w     = w     + dest_kReach;
    *padded_h     = h     + dest_kReach;
}//gbImage_GetAdjustedROI_ForKernelExtent


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
                                                     int64_t*     padded_roi_h)
{
    const size_t zoomScale    = w / roi_w;
    const size_t src_kReachL  = kExtentPx > 0 && !kIsDownRightOnly ? kExtentPx - 1 : 0;
    const size_t src_kReachR  = kExtentPx > 0                      ? kExtentPx - 1 : 0;
    const size_t src_kReachU  = kExtentPx > 0 && !kIsDownRightOnly ? kExtentPx - 1 : 0;
    const size_t src_kReachD  = kExtentPx > 0                      ? kExtentPx - 1 : 0;
    
    const size_t dest_kReachL = src_kReachL * zoomScale;
    const size_t dest_kReachR = src_kReachR * zoomScale;
    const size_t dest_kReachU = src_kReachU * zoomScale;
    const size_t dest_kReachD = src_kReachD * zoomScale;
    
    *padded_roi_w = roi_w +  src_kReachL +  src_kReachR;
    *padded_roi_h = roi_h +  src_kReachU +  src_kReachD;
    
    *padded_w     =     w + dest_kReachL + dest_kReachR;
    *padded_h     =     h + dest_kReachU + dest_kReachD;
    
    *padded_roi_x = (int64_t)roi_x - src_kReachL;
    *padded_roi_y = (int64_t)roi_y - src_kReachU;
    
    *padded_x     = dest_kReachL;
    *padded_y     = dest_kReachU;
}//gbImage_GetAdjustedROI_ForKernelExtent_AllSides



// ==========================
// _ClampAdjustedROI_ToSrcWH:
// ==========================
//
// For use with _GetROI_ForZtoXYZ and _GetAdjustedROI_ForKernelExtent.
//
// Padding the src buffer naievely can result in a ROI that outside of the bounds
// of the image, which is bad.
//
// This clamps the ROI such that the ROI is constrainted to the bounds of the
// original image.
//
// Note, however, that this should not be used to change the size of the padded
// src image ultimately being resampled.  The reason is this will result in
// significant tile boundary artficats when the resampling kernel extent hits
// an edge too early.
//
// Thus, this should only be used to control what is copied from the src tile
// into the padded crop tile.  If this does any clamping, then the remainder
// should be filled in one of two ways:
//
// 1. A NODATA fill technique, such as edge extend and/or neighborhood mean
// 2. Loading 1-3 other tiles (if present) to provide actual image data.
//    Note that the tiles may not actually be present, so #1 will still be
//    required even here.

void gbImage_ClampAdjustedROI_ToSrcWH(const size_t  w,
                                      const size_t  h,
                                      const int64_t roi_x,
                                      const int64_t roi_y,
                                      int64_t*      padded_roi_w,
                                      int64_t*      padded_roi_h)
{
    if (roi_x + *padded_roi_w > w)
    {
        *padded_roi_w = w - roi_x;
    }//if
    
    if (roi_y + *padded_roi_h > h)
    {
        *padded_roi_h = h - roi_y;
    }//if
}//gbImage_ClampAdjustedROI_ToSrcWH


void gbImage_ClampAdjustedROI_ToSrcWH_AllSides(const size_t w,
                                               const size_t h,
                                               int64_t*     padded_roi_x,
                                               int64_t*     padded_roi_y,
                                               int64_t*     padded_roi_w,
                                               int64_t*     padded_roi_h)
{
    if (*padded_roi_x < 0)
    {
        *padded_roi_w = *padded_roi_w + *padded_roi_x;
        *padded_roi_x = 0;
    }//if
    
    if (*padded_roi_y < 0)
    {
        *padded_roi_h = *padded_roi_h + *padded_roi_y;
        *padded_roi_y = 0;
    }//if
    
    if (*padded_roi_x + *padded_roi_w > w)
    {
        *padded_roi_w = w - *padded_roi_x;
    }//if
    
    if (*padded_roi_y + *padded_roi_h > h)
    {
        *padded_roi_h = h - *padded_roi_y;
    }//if
}//gbImage_ClampAdjustedROI_ToSrcWH_AllSides


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
                                                             int64_t*     padded_roi_h)
{
    const size_t zoomScale    = w / roi_w;
    
    gbImage_GetAdjustedROI_ForKernelExtent_AllSides(kExtentPx, kIsDownRightOnly,
                                                    w, h,
                                                    roi_x, roi_y, roi_w, roi_h,
                                                    padded_x, padded_y, padded_w, padded_h,
                                                    padded_roi_x, padded_roi_y, padded_roi_w, padded_roi_h);
    
    gbImage_ClampAdjustedROI_ToSrcWH_AllSides(w, h,
                                              padded_roi_x, padded_roi_y, padded_roi_w, padded_roi_h);
    
    *padded_w     = *padded_roi_w * zoomScale;
    *padded_h     = *padded_roi_h * zoomScale;
    
    *padded_x     = ((int64_t)roi_x - *padded_roi_x) * zoomScale;
    *padded_y     = ((int64_t)roi_y - *padded_roi_y) * zoomScale;
}//gbImage_GetAdjustedROI_ForKernelExtent_AllSides_Clamped




void gbImage_RGBA_GetRGBAVectorEntryOffsets(const int rgbaTypeId,
                                            int*      rcOut,
                                            int*      gcOut,
                                            int*      bcOut,
                                            int*      acOut)
{
    int _rc;
    int _gc;
    int _bc;
    int _ac;
    
    switch (rgbaTypeId)
    {
        case kGB_RGBAType_RGBA:         // RGBA
            _rc = 0;
            _gc = 1;
            _bc = 2;
            _ac = 3;
            break;
        case kGB_RGBAType_BGRA:         // BGRA
            _rc = 2;
            _gc = 1;
            _bc = 0;
            _ac = 3;
            break;
        case kGB_RGBAType_ARGB:         // ARGB
            _rc = 1;
            _gc = 2;
            _bc = 3;
            _ac = 0;
            break;
        case kGB_RGBAType_ABGR:         // ABGR
            _rc = 3;
            _gc = 2;
            _bc = 1;
            _ac = 0;
            break;
        case kGB_RGBAType_RGBA_NoPremul: // RGBA
            _rc = 0;
            _gc = 1;
            _bc = 2;
            _ac = 3;
            break;
        default:                        // BGRA
            _rc = 2;
            _gc = 1;
            _bc = 0;
            _ac = 3;
            break;
    }//switch
    
    *rcOut = _rc;
    *gcOut = _gc;
    *bcOut = _bc;
    *acOut = _ac;
}//gbImage_RGBA_GetRGBAVectorEntryOffsets



