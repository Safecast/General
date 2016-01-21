//
//  gbLUT_Transforms.h
//  Safecast
//
//  Created by Nicholas Dolezal on 11/7/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#ifndef gbLUT_Transforms_h
#define gbLUT_Transforms_h

#include <stdio.h>
#include <stdbool.h>
#include "gbLUT_Types.h"
#include "gbLUT_Numerics.h"
#include "gbLUT_Utils.h"

// +------+--------+--------+-----------+ +-----------+--------------+
// | Name | Alpha? | Norms? | Dest Type | | Buf RGBA? | Buf PlanarF? |
// +------+--------+--------+-----------+ +-----------+--------------+
// | Raw  |   Y    |   N    | RGBA      | |     Y     |       Y      |
// | Vec  |   N    |   Y    | PlanarF   | |     N     |       Y      |
// | VecX |   N    |   Y    | RGB(A)    | |     Y     |       N      |
// | Tile |   Y    |   Y    | RGBA      | |     Y     |       Y      |
// +------+--------+--------+-----------+ +-----------+--------------+

// gbLUT_Transform_IndicesAlpha_RGBA8888()   // "raw"
// gblUT_Transform_Data_PlanarF()            // "vec"
// gblUT_Transform_Data_RGBA8888()           // "vecX"
// gbLUT_Transform_DataAlpha_RGBA8888()      // "tile"

// "gbLUT_ApplyRawLUTtoVector_v2"
void gbLUT_Transform_IndicesAlpha_RGBA8888(const float* data,
                                           gbLUT*       lut,
                                           uint8_t*     rgba,
                                           const size_t n);

// "gbLUT_ApplyLUTtoVector_v2"
void gbLUT_Transform_Data_PlanarF(float*       data,
                                  gbLUT*       lut,
                                  float**      r,
                                  float**      g,
                                  float**      b,
                                  const size_t n);

// "gbLUT_GetRGBA_ForTileVector_v2"
void gbLUT_Transform_DataAlpha_RGBA8888(float*       data,
                                        float*       alpha,
                                        uint8_t**    rgba,
                                        gbLUT*       lut,
                                        const size_t n);


// nb:   This is NOT robust, the width/height are currently more or less ignored
//       and must be lut->props->n for the primary dimension, and 1 for the
//       secondary.
//       eg: For a 256 color LUT:
//           - A vertical   image is always 1x256
//           - A horizontal image is always 256x1
//
// todo: Fix by:
//       1. For w/h > n, interpolate via NN/lerp
//       2. For w/h < n, interpolate via NN(eg decimate) or mean
//       3. When 2nd dimension is > 1, fill by duping rows/cols appropriately.
void gbLUT_Transform_LUT_RGBA8888(gbLUT*       lut,
                                  const size_t width,
                                  const size_t height,
                                  const bool   is_vertical,
                                  uint8_t**    rgba);


#endif
