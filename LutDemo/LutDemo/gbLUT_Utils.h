//
//  gbLUT_Utils.h
//  Safecast
//
//  Created by Nicholas Dolezal on 11/7/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#ifndef gbLUT_Utils_h
#define gbLUT_Utils_h

#include <stdio.h>
#include "gbLUT_Types.h"
#include <stdbool.h>

// =========================
// gbLUT_GetPointersRGB_f32:
// =========================
//
// Returns pointers to the internal lut.data buffer for each color channel.
// The buffer type must be kGB_LUT_BufferType_PlanarF.  These pointers should
// NOT be freed or modified.
//
void  gbLUT_GetPointersRGB_f32(const gbLUT* lut, float** r, float** g, float** b);

int   gbLUT_GetEffectiveScaleTypeId(const gbLUT* lut);
bool  gbLUT_GetEffectiveReverse(const gbLUT* lut);
int   gbLUT_GetEffectiveDiscretizeSteps(const gbLUT* lut);
float gbLUT_GetEffectiveMin(const gbLUT* lut);
float gbLUT_GetEffectiveMax(const gbLUT* lut);

#endif
