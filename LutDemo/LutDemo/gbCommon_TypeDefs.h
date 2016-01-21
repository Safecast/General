//
//  gbCommon_TypeDefs.h
//  Safecast
//
//  Created by Nicholas Dolezal on 9/14/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#ifndef gbCommon_TypeDefs_h
#define gbCommon_TypeDefs_h

#include <stdint.h>
#include <stdbool.h>


typedef int GB_DisplayUnitType; enum
{
    kGB_DisplayUnit_CPM           = 0,
    kGB_DisplayUnit_mRh           = 1,
    kGB_DisplayUnit_uSvh          = 2,
    kGB_DisplayUnit_nSvh          = 3,
    kGB_DisplayUnit_uRh           = 4,
    kGB_DisplayUnit_CPS           = 5
};

typedef int GB_DisplayStatType; enum
{
    kGB_DisplayStat_MSEP          = 0,
    kGB_DisplayStat_MSE           = 1,
    kGB_DisplayStat_StdDevP       = 2,
    kGB_DisplayStat_StdDev        = 3,
    kGB_DisplayStat_95CIP         = 4,
    kGB_DisplayStat_95CI          = 5,
    kGB_DisplayStat_99CIP         = 6,
    kGB_DisplayStat_99CI          = 7
};

typedef int GB_MesszeitType; enum
{
    kGB_Messzeit_Auto          = 0,
    kGB_Messzeit_All           = 1,
    kGB_Messzeit_Custom1       = 2,
    kGB_Messzeit_Custom2       = 3
};

typedef int GB_RGBAType; enum
{
    kGB_RGBAType_RGBA          = 1,
    kGB_RGBAType_BGRA          = 2,
    kGB_RGBAType_ARGB          = 3,
    kGB_RGBAType_ABGR          = 4,
    kGB_RGBAType_RGBA_NoPremul = 5,
    kGB_RGBAType_BGRA_NoPremul = 6,
    kGB_RGBAType_ARGB_NoPremul = 7,
    kGB_RGBAType_ABGR_NoPremul = 8
};

typedef int GB_MapFramework_Provider; enum
{
    kGB_MapFramework_Provider_AppleMaps  = 1,
    kGB_MapFramework_Provider_GoogleMaps = 2
};

typedef int GB_TileEngine_InterpolationType; enum
{
    kGB_TileEngine_Interp_NN              = 0,
    kGB_TileEngine_Interp_Bilinear        = 1,
    kGB_TileEngine_Interp_Lanczos3x3      = 2,
    kGB_TileEngine_Interp_Lanczos5x5      = 3,
    kGB_TileEngine_Interp_BilinearCareful = 4,
    kGB_TileEngine_Interp_EPX             = 5
};

typedef int GB_TileEngine_MaskInterpolationType; enum
{
    kGB_TileEngine_MaskInterp_NULL        = 0,
    kGB_TileEngine_MaskInterp_None        = 1,
    kGB_TileEngine_MaskInterp_NN          = 2,
    kGB_TileEngine_MaskInterp_EPX         = 3,
    kGB_TileEngine_MaskInterp_Bilinear    = 4,
    kGB_TileEngine_MaskInterp_Lanczos3x3  = 5,
    kGB_TileEngine_MaskInterp_Lanczos5x5  = 6
};

typedef int GB_TileEngine_InterfaceType; enum
{
    kGB_TileEngine_Direct_MKOverlay            = 0,
    kGB_TileEngine_Deferred_MKOverlay          = 1,
    kGB_TileEngine_Deferred_TIFF_MKTileOverlay = 2,
    kGB_TileEngine_Deferred_JPEG_MKTileOverlay = 3,
    kGB_TileEngine_Deferred_PNG_MKTileOverlay  = 4
};

// *** DEPRECATED *** do not use, will be removed.
typedef struct gbLUT_Options
{
    int   lutId;
    int   uiOrder;
    int   scaleTypeId;
    float min;
    float max;
    bool  isInverted;
    int   discretizeSteps;
    float pointAlpha;
    float mapAlpha;
    int   n;
    int   rgbaTypeId;
    bool  isNormalized;
    bool  isTransformedHDR;
    bool  isFixedScale;
    bool  isLogOnly;
    bool  isLinOnly;
    float fixedMin;
    float fixedMax;
} gbLUT_Options;













#endif
