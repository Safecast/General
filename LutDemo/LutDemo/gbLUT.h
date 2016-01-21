//
//  gbLUT.h
//  Safecast
//
//  Created by Nicholas Dolezal on 10/12/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#ifndef gbLUT_h
#define gbLUT_h

#include <stdio.h>
#include <Accelerate/Accelerate.h>
#include "gbCommon_TypeDefs.h"
#include "gbImage_Utils.h"
//#include "gbIO.h"
#include "gbDB.h"
#include "gbLUT_Types.h"
#include "gbLUT_Numerics.h"
#include "gbLUT_Transforms.h"
#include "gbLUT_Utils.h"

#ifndef ARM_NEON
    #import "NEONvsSSE_5.h"
    #import "NEONvsSSE_gbExt.h"
#endif

// =============================================================================
//                                  gbLUT.h
// =============================================================================
//
// gbLUT.h provides structs, typedefs and functions necessary to use a database-
// backed color lookup table.  It is used to "color" buffers of floating point
// data values.
//
// gbLUT.h has been designed for high throughput, at the cost of code complexity
// and initial setup overhead.
//
// Special support is included for map tile buffers, which also have an alpha
// channel.  However, this is also a generalized solution and works with any
// input non-negative floating point data.
//
// Existing app usage:
// - converting map PlanarF tiles to BGRA8888 bitmaps for rendering
// - the map scale, a visualization of the LUT itself
// - the LUT selection list, a visualization of all LUTs available
// - the gamma spectroscopy histogram, for coloring data by keV/bin. (iOS)
// - user logged map markers. (iOS)






// ====================
// gbLUT_LoadAndCreate:
// ====================
//
// Loads a LUT, given props.lut_id, from the database at db_path, and creates
// the gbLUT object dest.
//
// This is the primary method to load/create a LUT.
//
// All values in props must be set, except for props.ui_order.
//
void gbLUT_LoadAndCreate(gbLUT_Properties* props,
                         const char*       db_path,
                         gbLUT**           dest);


// ==============
// gbLUT_Destroy:
// ==============
//
// Frees/releases the resources associated with a LUT and the LUT itself.
//
// Should be used whenever you are done working with a LUT.
//
void gbLUT_Destroy(gbLUT* lut);



// =============
// gbLUT_Create:
// =============
//
// Low-level function, should generally not be used.  Creates a LUT without
// performing a database load.
//
/*
void gbLUT_Create(gbLUT_Properties*  props,
                  gbLUT_Permissions* perms,
                  const char*        db_path,
                  const uint8_t*     r,
                  const uint8_t*     g,
                  const uint8_t*     b,
                  const size_t       src_n,
                  gbLUT**            dest);
*/

// ====================
// gbLUT_LoadAndSetLUT:
// ====================
//
// For a LUT that has already been loaded, this applies new properties in props,
// which in most cases will cause it to be reloaded from the DB.
//
// If you are reusing a LUT object to provide transforms, this should be the
// primary function called to apply changes.
//
// All values in props must be set, except for props.ui_order.  It is expected
// that the caller creates a copy of lut.props, then passes the copy in with
// changes as props.
//
/*
void gbLUT_LoadAndSetLUT(gbLUT*            lut,
                         gbLUT_Properties* props);
*/

// =============
// gbLUT_SetLUT:
// =============
//
// Low-level function, not for general use.  For a LUT that has already been
// loaded, sets all properties, permissions, and source RGB data without reading
// from the database.
//
/*
void gbLUT_SetLUT(gbLUT*             lut,
                  gbLUT_Properties*  props,
                  gbLUT_Permissions* perms,
                  const uint8_t*     r,
                  const uint8_t*     g,
                  const uint8_t*     b,
                  const size_t       src_n);
*/

// ===========
// gbLUT_Load:
// ===========
//
// Low-level function, not for general use.  Loads the data for a LUT from the
// DB, without actually creating a LUT object.
//
/*
void gbLUT_Load(const int           lut_id,
                const char*         db_path,
                int*                ui_order,
                gbLUT_Permissions** perms,
                uint8_t**           rgb,
                size_t*             lut_n);
*/











#endif
