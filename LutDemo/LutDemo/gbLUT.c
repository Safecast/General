//
//  gbLUT.c
//  Safecast
//
//  Created by Nicholas Dolezal on 10/12/15.
//  Copyright (c) 2015 The Momoko Ito Foundation. All rights reserved.
//

#include "gbLUT.h"


void gbLUT_ApplyTransformsToLUT_v2(float*       r,
                                   float*       g,
                                   float*       b,
                                   const bool   reverse,
                                   const int    discretize_steps,
                                   const size_t n)
{
    if (discretize_steps < 256 && discretize_steps > 0)
    {
        gbLUT_DiscretizeLUT(r, g, b, discretize_steps, n);
    }//if
    
    if (reverse)
    {
        gbLUT_ReverseLUT(r, g, b, n);
    }//if
}//gbLUT_ApplyTransformsToLUT_v2


bool gbLUT_InPlaceLogTransformIfNeeded_v2(float*       r,
                                          float*       g,
                                          float*       b,
                                          const int    scaleTypeId,
                                          const size_t n)
{
    if (scaleTypeId != 0 && scaleTypeId != 2)
    {
        return false;
    }//if
    
    float *linIdx = NULL;
    float *lutln  = malloc(sizeof(float) * n);
    
    gbLUT_MakeLinIdxVector(&linIdx, n);
    
    if (scaleTypeId == 0)
    {
        gbLUT_MakeLogIdx(lutln, linIdx, n);
    }//if
    else
    {
        gbLUT_MakeLog10Idx(lutln, linIdx, n);
    }//else
    
    free(linIdx);
    linIdx = NULL;
    
    float *rLog = malloc(sizeof(float) * n);
    float *gLog = malloc(sizeof(float) * n);
    float *bLog = malloc(sizeof(float) * n);
    
    vDSP_vindex(r, lutln, 1, rLog, 1, n);
    vDSP_vindex(g, lutln, 1, gLog, 1, n);
    vDSP_vindex(b, lutln, 1, bLog, 1, n);
    
    free(lutln);
    lutln = NULL;
    
    memcpy(r, rLog, sizeof(float) * n);
    memcpy(g, gLog, sizeof(float) * n);
    memcpy(b, bLog, sizeof(float) * n);
    
    free(rLog);
    rLog = NULL;
    free(gLog);
    gLog = NULL;
    free(bLog);
    bLog = NULL;
    
    return true;
}//InPlaceLogTransformIfNeeded_v2



void gbLUT_SetBuffer(gbLUT*         lut,
                     const uint8_t* r,
                     const uint8_t* g,
                     const uint8_t* b,
                     const size_t   src_n)
{
    float* _r32 = malloc(sizeof(float) * src_n);
    float* _g32 = malloc(sizeof(float) * src_n);
    float* _b32 = malloc(sizeof(float) * src_n);
    
    gbLUT_U08_to_F32_vImage(r, _r32, src_n);
    gbLUT_U08_to_F32_vImage(g, _g32, src_n);
    gbLUT_U08_to_F32_vImage(b, _b32, src_n);
    
    gbLUT_ApplyTransformsToLUT_v2(_r32, _g32, _b32,
                                  gbLUT_GetEffectiveReverse(lut),
                                  gbLUT_GetEffectiveDiscretizeSteps(lut),
                                  src_n);
    
    if (lut->props->n > src_n)
    {
        float* _src_r32 = _r32;
        float* _src_g32 = _g32;
        float* _src_b32 = _b32;
        
        _r32 = malloc(sizeof(float) * lut->props->n);
        _g32 = malloc(sizeof(float) * lut->props->n);
        _b32 = malloc(sizeof(float) * lut->props->n);
        
        bool nn = gbLUT_GetEffectiveDiscretizeSteps(lut) < 256;
        
        gbLUT_InterpolateLUT(_src_r32, _src_g32, _src_b32, src_n,
                             _r32,_g32, _b32, lut->props->n,
                             nn);
        
        free(_src_r32);
        _src_r32 = NULL;
        free(_src_g32);
        _src_g32 = NULL;
        free(_src_b32);
        _src_b32 = NULL;
    }//if
    
    gbLUT_InPlaceLogTransformIfNeeded_v2(_r32, _g32, _b32,
                                         gbLUT_GetEffectiveScaleTypeId(lut),
                                         lut->props->n);
    
    if (lut->data != NULL)
    {
        free(lut->data);
        lut->data = NULL;
    }//if
    
    if (lut->props->buffer_type_id == kGB_LUT_BufferType_PlanarF)
    {
        lut->data = malloc(sizeof(float) * lut->props->n * 3);
        
        memcpy(lut->data,
               _r32, sizeof(float) * lut->props->n);
        
        memcpy(lut->data + sizeof(float) * lut->props->n,
               _g32, sizeof(float) * lut->props->n);
        
        memcpy(lut->data + sizeof(float) * lut->props->n * 2,
               _b32, sizeof(float) * lut->props->n);
    }//if
    else if (lut->props->buffer_type_id == kGB_LUT_BufferType_RGBA)
    {
        lut->data = malloc(sizeof(uint32_t) * lut->props->n);
        
        gbLUT_LUTtoLUT_XTREME(_r32, _g32, _b32, (float*)lut->data,
                              lut->props->bkg_alpha, lut->props->data_alpha,
                              lut->props->rgba_type_id, lut->props->n);
    }//else if
    else
    {
        printf("gbLUT.c: gbLUT_SetBuffer: [ERR] Bad buffer_type_id!\n");
    }//else
    
    free(_r32);
    _r32 = NULL;
    free(_g32);
    _g32 = NULL;
    free(_b32);
    _b32 = NULL;
}//gbLUT_SetBuffer


void gbLUT_Create(gbLUT_Properties*  props,
                  gbLUT_Permissions* perms,
                  const char*        db_path,
                  const uint8_t*     r,
                  const uint8_t*     g,
                  const uint8_t*     b,
                  const size_t       src_n,
                  gbLUT**            dest)
{
    gbLUT* lut   = malloc(sizeof(gbLUT));
    lut->perms   = malloc(sizeof(gbLUT_Permissions));
    lut->props   = malloc(sizeof(gbLUT_Properties));
    lut->data    = NULL;
    //lut->db_path = malloc(strnlen(db_path, 1024) + 1UL);
    
    memcpy(lut->perms, perms, sizeof(gbLUT_Permissions));
    memcpy(lut->props, props, sizeof(gbLUT_Properties));
    //memcpy(lut->db_path, db_path, strnlen(db_path, 1024) + 1UL);
    
    gbLUT_SetBuffer(lut, r, g, b, src_n);
    
    *dest = lut;
}//gbLUT_Create


void gbLUT_Destroy(gbLUT* lut)
{
    free(lut->data);
    lut->data = NULL;
    
    free(lut->perms);
    lut->perms = NULL;
    
    free(lut->props);
    lut->props = NULL;
    
    //free(lut->db_path);
    //lut->db_path = NULL;
    
    free(lut);
    lut = NULL;
}//gbLUT_Destroy

/*
bool gbLUT_ShouldReloadFromPropChange(gbLUT* lut, gbLUT_Properties* props)
{
    return  lut->props->lut_id           != props->lut_id
        ||  lut->props->scale_type_id    != props->scale_type_id
        ||  lut->props->reverse          != props->reverse
        ||  lut->props->discretize_steps != props->discretize_steps
        ||  lut->props->buffer_type_id   != props->buffer_type_id
        ||  lut->props->n                != props->n
        || (lut->props->buffer_type_id   == kGB_LUT_BufferType_RGBA
            &&   props->buffer_type_id   == kGB_LUT_BufferType_RGBA
            && (   lut->props->data_alpha   != props->data_alpha
                || lut->props->bkg_alpha    != props->bkg_alpha
                || lut->props->rgba_type_id != props->rgba_type_id));
}//gbLUT_ShouldReloadFromPropChange


void gbLUT_SetProperties(gbLUT* lut, gbLUT_Properties* props)
{
    memcpy(lut->props, props, sizeof(gbLUT_Properties));
}//gbLUT_SetProperties
*/

bool gbLUT_SelectLUT(const int   lut_id,
                     const char* db_path,
                     int*        ui_order,
                     bool*       grant_selection,
                     bool*       grant_discretize,
                     bool*       grant_reverse,
                     bool*       grant_scale_type,
                     bool*       grant_min_max,
                     int*        fixed_scale_type_id,
                     float*      fixed_min,
                     float*      fixed_max,
                     uint8_t**   lut,
                     size_t*     lut_n)
{
    bool    retVal = false;
    sqlite3* db;
    char sql[1024];
    
    snprintf(sql, 1024, "SELECT UIOrder, GrantSelection, GrantDiscretize, GrantReverse, GrantScaleType, GrantMinMax, FixedScaleTypeID, FixedMin, FixedMax, LUT FROM ColorLUT WHERE TableID=%d;", lut_id);
    
    if (sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, NULL) == SQLITE_OK)
    {
        sqlite3_stmt *selectStmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &selectStmt, NULL) == SQLITE_OK)
        {
            // UIOrder, GrantSelection, GrantDiscretize, GrantReverse, GrantScaleType, GrantMinMax, FixedScaleTypeID, FixedMin, FixedMax, LUT
            //    0           1               2                3                4           5                6          7          8       9
            
            while (sqlite3_step(selectStmt) == SQLITE_ROW)
            {
                *ui_order            = sqlite3_column_int(selectStmt, 0);
                *grant_selection     = sqlite3_column_int(selectStmt, 1) == 1;
                *grant_discretize    = sqlite3_column_int(selectStmt, 2) == 1;
                *grant_reverse       = sqlite3_column_int(selectStmt, 3) == 1;
                *grant_scale_type    = sqlite3_column_int(selectStmt, 4) == 1;
                *grant_min_max       = sqlite3_column_int(selectStmt, 5) == 1;
                *fixed_scale_type_id = sqlite3_column_int(selectStmt, 6);
                *fixed_min           = sqlite3_column_double(selectStmt, 7);
                *fixed_max           = sqlite3_column_double(selectStmt, 8);
                const void *blob     =  sqlite3_column_blob(selectStmt, 9);
                int blob_bytes       = sqlite3_column_bytes(selectStmt, 9);
                
                uint8_t* _lut = malloc(blob_bytes);
                memcpy(_lut, blob, blob_bytes);
                
                *lut_n = blob_bytes / 3;
                *lut   = _lut;
                retVal = true;
                break;
            }//while
        }//if
        else
        {
            printf("DataProvider: Error executing LUT query.\n");
        }//else
        sqlite3_finalize(selectStmt);
    }//if
    else
    {
        printf("ERR: couldn't open LUT DB.\n");
    }
    
    if (sqlite3_close(db) != SQLITE_OK)
    {
        printf("error closing db: %s\n", sqlite3_errmsg(db));
    }//if
    
    return retVal;
}//SelectLUT



/*
void gbLUT_SetLUT(gbLUT*             lut,
                  gbLUT_Properties*  props,
                  gbLUT_Permissions* perms,
                  const uint8_t*     r,
                  const uint8_t*     g,
                  const uint8_t*     b,
                  const size_t       src_n)
{
    const bool reload = gbLUT_ShouldReloadFromPropChange(lut, props);
    
    gbLUT_SetProperties(lut, props);
    memcpy(lut->perms, perms, sizeof(gbLUT_Permissions));
    
    if (reload)
    {
        gbLUT_SetBuffer(lut, r, g, b, src_n);
    }//if
}//gbLUT_SetLUT
*/

void gbLUT_Load(const int           lut_id,
                const char*         db_path,
                int*                ui_order,
                gbLUT_Permissions** perms,
                uint8_t**           rgb,
                size_t*             lut_n)
{
    gbLUT_Permissions* _perms    = malloc(sizeof(gbLUT_Permissions));
    uint8_t*           _rgb      = NULL;
    int                _ui_order;
    size_t             _lut_n;
    
    gbLUT_SelectLUT(lut_id, db_path, &_ui_order,
                    &_perms->grant_selection, &_perms->grant_discretize,
                    &_perms->grant_reverse, &_perms->grant_scale_type,
                    &_perms->grant_min_max, &_perms->fixed_scale_type_id,
                    &_perms->fixed_min, &_perms->fixed_max, &_rgb, &_lut_n);

    *ui_order = _ui_order;
    *perms    = _perms;
    *rgb      = _rgb;
    *lut_n    = _lut_n;
}//gbLUT_Load

void gbLUT_LoadAndCreate(gbLUT_Properties*  props,
                         const char*        db_path,
                         gbLUT**            dest)
{
    gbLUT_Permissions* perms = NULL;
    uint8_t* rgb             = NULL;
    size_t   lut_n           = 0;
    gbLUT_Load(props->lut_id, db_path, &props->ui_order, &perms, &rgb, &lut_n);
    
    uint8_t* r = rgb;
    uint8_t* g = rgb + lut_n;
    uint8_t* b = rgb + lut_n * 2;
    
    gbLUT_Create(props, perms, db_path, r, g, b, lut_n, dest);
    
    free(rgb);
    free(perms);
}//gbLUT_LoadAndCreate

/*
void gbLUT_InitProperties(gbLUT_Properties* props)
{
    props->lut_id = 26;
    props->bkg_alpha = 0.0F;
    props->buffer_type_id = kGB_LUT_BufferType_PlanarF;
    props->data_alpha = 1.0F;
    props->discretize_steps = 256;
    props->max = 65.535F;
    props->min = 0.03F;
    props->n = 256;
    props->reverse = false;
    props->rgba_type_id = kGB_RGBAType_BGRA;
    props->scale_type_id = 2;
    props->ui_order = props->lut_id;
}//gbLUT_InitProperties
*/

/*
void gbLUT_LoadAndSetLUT(gbLUT*             lut,
                         gbLUT_Properties*  props)
{
    gbLUT_Permissions* perms = NULL;
    uint8_t* rgb             = NULL;
    size_t   lut_n           = 0;
    gbLUT_Load(props->lut_id, lut->db_path, &props->ui_order, &perms, &rgb, &lut_n);

    uint8_t* r = rgb;
    uint8_t* g = rgb + lut_n;
    uint8_t* b = rgb + lut_n * 2;
    
    gbLUT_SetLUT(lut, props, perms, r, g, b, lut_n);
    
    free(perms);
    free(rgb);
}//gbLUT_LoadAndSetLUT
*/












