//
//  gbLUT_Types.h
//  Safecast
//
//  Created by Nicholas Dolezal on 11/6/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#ifndef gbLUT_TypeDefs_h
#define gbLUT_TypeDefs_h

#include <stdint.h>
#include <stdbool.h>
#include <Accelerate/Accelerate.h>

// =================
// GB_LUT_BufferType
// =================
//
// After the LUT is loaded from the DB, it is stored in a buffer via one of two
// ways that are mostly functionally equivalent, but should reflect how the LUT
// is to be used.
//
// kGB_LUT_BufferType_PlanarF:
// ---------------------------
// The LUT data is stored as a single buffer of 32-bit floating point values.
// The first third of the buffer contains the red channel, and so on.
// This type is best for use when it is desirable to apply a LUT and obtain
// the results as floating point values, rather than a bitmap.  This comes at
// the cost of being less efficient in creating RGBA bitmaps.
//
// kGB_LUT_BufferType_RGBA:
// ------------------------
// The LUT data is stored as a RGBA8888 buffer, with the exact channel order
// and alpha premultiplication controlled by the gbLUT_Properties object.
// The setup costs for this are slightly greater, but it is significantly
// faster for applying to large amounts of data.
//
// A caveat is that the channel order and alpha premultiplication are
// pre-applied to the LUT buffer, which makes working with the buffer directly
// less efficient.
//
typedef int GB_LUT_BufferType; enum
{
    kGB_LUT_BufferType_PlanarF = 0,
    kGB_LUT_BufferType_RGBA    = 1
};

// =================
// gbLUT_Properties:
// =================
//
// This defines settings for how the LUT is used; some of these may be user
// preferences, but others are more intrinsic to the caller's implementation.
//
// Unlike other structs, the caller is expected to set all values here,
// except ui_order, prior to calling gbLUT_LoadAndCreate.
//
//           lut_id: Database TableID of the LUT.
//         ui_order: Sorting ordinal for a list view, filled from the DB.
//    scale_type_id: An optional logarithmic stretch for the LUT, increasing
//                   contrast at the low end and reducing it at the high end.
//                   0=LN, 1=linear (no stretch), 2=LOG10
//          reverse: Reverses the order of the RGB entries in the LUT.
// discretize_steps: Reduces the colors of the LUT if set to a value <256.
//                   Set to 256 to disable.  [2 - 256]
//              min: The minimum value in the source data later passed in;
//                   anything below will be clamped.
//              max: The maximum value in the source data later passed in;
//                   anything above will be clamped.
//       data_alpha: When creating RGBA data, controls the alpha of the output
//                   pixels.  Should be [0.0 - 1.0].
//        bkg_alpha: When creating RGBA data, composits a black bitmap beneath
//                   the output with this alpha value.  Should be [0.0 - 1.0].
//     rgba_type_id: Controls the the RGBA data output channel order and alpha
//                   premultiplication.  Also applied to the LUT buffer if the
//                   buffer type is kGB_LUT_BufferType_RGBA.
//                n: The number of desired entries in the LUT, [256 - 65536].
//                   Must be a power of 256.  Higher values provide smoother
//                   color in the output.  All LUTs natively are 256 colors;
//                   greater values use linear interpolation between these.
//    use_semaphore: When using tile functions only, uses a semaphore to allow
//                   for threadsafe changes and reloads to the LUT.
//
typedef struct gbLUT_Properties
{
    int    lut_id;
    int    ui_order;
    
    int    scale_type_id;
    bool   reverse;
    int    discretize_steps;
    float  min;
    float  max;
    float  data_alpha;
    float  bkg_alpha;
    
    int    rgba_type_id;
    int    buffer_type_id;
    size_t n;
} gbLUT_Properties;

// ==================
// gbLUT_Permissions:
// ==================
//
// gbLUT_Permissions is filled from the DB and should normally not be accessed
// or modified.
//
// These values override the user / caller values in gbLUT_Properties, and
// allow for certain LUTs to display as intended.
//
typedef struct gbLUT_Permissions
{
    bool  grant_selection;
    bool  grant_discretize;
    bool  grant_reverse;
    bool  grant_scale_type;
    bool  grant_min_max;
    int   fixed_scale_type_id;
    float fixed_min;
    float fixed_max;
} gbLUT_Permissions;

// ======
// gbLUT:
// ======
//
// A struct representing a color lookup table and associated properties.  You
// should not access these values directly, except for props in conjunction with
// gbLUT_LoadAndSetLUT.
//
//    perms: Permissions for this LUT.
//    props: Properties for this LUT.
//     data: The LUT buffer itself.
//  db_path: The local path to the LUT sqlite3 database file.
// sema_lut: Optional semaphore for threadsafety when using tiling functions.
//
typedef struct gbLUT
{
    gbLUT_Permissions*   perms;
    gbLUT_Properties*    props;
    void*                data;
    //char*                db_path;
} gbLUT;

#endif