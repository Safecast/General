//
//  gbLUT_Utils.c
//  Safecast
//
//  Created by Nicholas Dolezal on 11/7/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#include "gbLUT_Utils.h"

void gbLUT_GetPointersRGB_f32(const gbLUT* lut, float** r, float** g, float** b)
{
    if (lut->props->buffer_type_id == kGB_LUT_BufferType_PlanarF)
    {
        *r = (float*)(lut->data);
        *g = (float*)(lut->data + lut->props->n * sizeof(float));
        *b = (float*)(lut->data + lut->props->n * sizeof(float) * 2);
    }//if
}//gbLUT_GetPointersRGB_f32

int gbLUT_GetEffectiveScaleTypeId(const gbLUT* lut)
{
    return lut->perms->grant_scale_type ? lut->props->scale_type_id
                                        : lut->perms->fixed_scale_type_id;
}//gbLUT_GetEffectiveScaleTypeId

bool gbLUT_GetEffectiveReverse(const gbLUT* lut)
{
    return lut->perms->grant_reverse && lut->props->reverse;
}//gbLUT_GetEffectiveReverse

int gbLUT_GetEffectiveDiscretizeSteps(const gbLUT* lut)
{
    return lut->perms->grant_discretize ? lut->props->discretize_steps : 256;
}//gbLUT_GetEffectiveDiscretizeSteps

float gbLUT_GetEffectiveMin(const gbLUT* lut)
{
    return lut->perms->grant_min_max ? lut->props->min : lut->perms->fixed_min;
}//gbLUT_GetEffectiveMin

float gbLUT_GetEffectiveMax(const gbLUT* lut)
{
    return lut->perms->grant_min_max ? lut->props->max : lut->perms->fixed_max;
}//gbLUT_GetEffectiveMax