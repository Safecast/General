//
//  AppDelegate.m
//  LutDemo
//
//  Created by Nicholas Dolezal on 1/20/16.
//
//

#import "AppDelegate.h"


// =======
// LutDemo
// =======
//
// This is a demonstration of two different ways to implement the Safecast
// color LUT (lookup table) code.  It is used to transform numerical values
// into RGB colors.
//
// All output is to the Xcode console.  Running it should produce the following:
//
// AppDelegate.m: SimpleLutDemo_PerformDemo: The value 0.15 uSvh is color r=59,
//                g=150, b=255.
// AppDelegate.m: LutDemo_Apply: idx=0 is color r=59, g=157, b=255.
//
//
// ----------------
// METHOD 1: SIMPLE
// ----------------
//
// If full customization or processing millions of points is not needed, the
// simple version should be used.
//
// This is fully encapsulated by SimpleLutDemo_PerformDemo().  Ultimately, you
// only need to copy and paste two functions into your code:
//
//  1. SimpleLutDemo_GetCyanHaloLUT()
//  2. SimpleLutDemo_ApplyLUT()
//
// You must retain the individual RGB components of the lookup table.
//
//
// --------------
// METHOD 2: FULL
// --------------
//
// A much more complicated version, with greater customization and much higher
// performance for very large numbers of points.
//
// Note that this code has been manually extracted from a beta version of the
// Safecast OS X app, and is subject to change.  It may also contain bugs.
//
// You will need to base your code off the following:
//
//  1. [LutDemo_Init]
//  2. [LutDemo_Apply]
//  3. [LutDemo_Destroy]
//
// You must include (in your .h file):
//
//  #import "gbThreadGuard.h"
//  #import "gbLUT.h"
//
// The full files required:
//  gbCommon_TypeDefs.h
//  gbDB.h
//  gbDB.c
//  gbImage_Utils.h
//  gbImage_Utils.c
//  gbThreadGuard.h
//  gbThreadGuard.c
//  gbLUT_Types.h
//  gbLUT.h
//  gbLUT.c
//  gbLUT_Utils.h
//  gbLUT_Utils.c
//  gbLUT_Transforms.h
//  gbLUT_Transforms.c
//  gbLUT_Numerics.h
//  gbLUT_Numerics.c
//  gbLUT_Numerics_SIMD.h
//  gbLUT_Numerics_SIMD.c
//  NEONvsSSE_5.h        ***(only on Intel platforms)
//  NEONvsSSE_5_gbExt.h  ***(only on Intel platforms)
//  lut_v2.sqlite




// ============
// KNOWN ISSUES
// ============
//
// gbThreadGuard.h does not compile properly as pure source in an ObjC+ARC
// project.  For that reason, ARC has been disabled in this project.
//
// The issue is that gbThreadGuard.h uses libdispatch's dispatch_sempahore_t
// within a C struct.  This is valid in ANSI C, but in a ObjC project, the file
// is instead treated as ObjC, and dispatch_sempahore_t becomes an ObjC object.
//
// This can likely be worked around by compiling as a static C library (untested)
// but will need to be investigated in more depth.
//
// Another workaround is to replace gbThreadGuard with a dispatch_semaphore_t
// and dispatch_sempahore_wait() / dispatch_sempahore_signal().  Or even a mutex
// lock.  Any of these approaches will be somewhat less performant.


@interface AppDelegate ()

//@property (weak) IBOutlet NSWindow *window;
@property IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    _lut_thread_guard = NULL;
    _gb_lut           = NULL;
    
    // 1. Run the simple case demo
    
    SimpleLutDemo_PerformDemo(0.15F);
    
    // 2. Run the full demo
    
    [self LutDemo_Init];
    [self LutDemo_Apply];
}//applicationDidFinishLaunching


- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    [self LutDemo_Destroy];
}//applicationWillTerminate









// =============================================================================
// =============================================================================
//
// "SimpleLutDemo" should be used at the basis for code using the Safecast color
// LUT in most cases.
//
// 1. Simple code -- no other requirements.
// 2. Copy-and-paste into your code.
// 3. Low overhead and RAM use.
//
// But:
//
// 1. Limited to single LUT
// 2. No options like scaling, discretize, interpolation, reverse LUT, etc.
// 3. Slower for many points (>100,000).
//
// =============================================================================
// =============================================================================


void SimpleLutDemo_GetCyanHaloLUT(uint8_t** r,
                                  uint8_t** g,
                                  uint8_t** b)
{
    uint8_t _rs[256] = { 1,8,9,10,12,14,16,16,18,18,19,20,21,22,22,24,24,25,25,25,26,26,26,26,25,26,27,26,25,26,26,24,24,25,24,21,21,21,17,16,9,7,0,7,15,23,28,32,34,38,40,43,45,46,50,51,54,55,56,56,56,58,59,59,59,59,59,59,59,59,57,56,56,56,54,51,48,45,43,39,37,33,29,23,10,0,29,39,60,67,84,90,97,105,110,120,124,133,137,143,148,153,161,163,171,173,178,181,185,191,194,200,202,208,210,214,217,220,225,226,233,235,240,242,245,249,251,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
    
    uint8_t _gs[256] = { 1,7,7,8,10,11,12,12,13,13,13,13,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,14,14,14,14,13,13,12,11,11,10,9,5,4,0,9,19,30,36,43,46,56,59,65,70,74,82,85,94,96,103,107,112,118,121,130,132,140,144,150,156,160,167,170,181,184,191,195,200,208,213,221,224,233,237,243,250,255,252,251,242,240,235,231,226,221,219,212,210,204,202,197,192,187,182,180,172,170,163,160,156,151,148,137,134,128,124,117,112,107,100,97,87,83,71,64,56,44,38,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,25,36,42,53,57,68,73,81,85,89,96,98,106,109,119,123,128,133,136,144,146,153,156,162,166,170,180,183,191,193,200,204,208,214,217,224,226,234,239,247,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
    
    uint8_t _bs[256] = { 1,10,12,15,19,23,29,31,38,39,43,48,54,59,61,70,72,79,83,89,94,99,109,112,122,125,135,140,145,153,158,169,173,186,191,199,206,212,223,228,239,244,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,254,247,242,234,227,221,211,207,196,192,183,179,174,166,161,154,151,140,136,128,123,119,113,111,102,100,94,88,81,75,72,65,63,55,52,47,42,38,32,29,18,13,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,10,20,23,31,36,41,47,51,62,65,74,78,84,90,94,102,105,115,118,128,133,138,145,148,157,159,168,172,179,186,191,198,202,211,215,222,226,232,238,242,253 };
    
    uint8_t* _r = malloc(sizeof(uint8_t) * 256);
    uint8_t* _g = malloc(sizeof(uint8_t) * 256);
    uint8_t* _b = malloc(sizeof(uint8_t) * 256);
    
    // Copy stack alloc from static constructor to heap to retain it.
    //
    // Might be easier to simply embed these as static globals.
    
    memcpy(_r, _rs, sizeof(uint8_t) * 256);
    memcpy(_g, _gs, sizeof(uint8_t) * 256);
    memcpy(_b, _bs, sizeof(uint8_t) * 256);
    
    *r = _r;
    *g = _g;
    *b = _b;
}//SimpleLutDemo_GetCyanHaloLUT

void SimpleLutDemo_ApplyLUT(const float    src,
                            const float    lut_min,
                            const float    lut_max,
                            const uint8_t* lut_r,
                            const uint8_t* lut_g,
                            const uint8_t* lut_b,
                            const size_t   lut_n,
                            uint8_t*       r_out,
                            uint8_t*       g_out,
                            uint8_t*       b_out)
{
    float norm;
    size_t idx;
    const size_t log_iter = 4;                          // number of LOG10 iterations to perform.
    
    norm = (src - lut_min) / (lut_max - lut_min);       // normalize the dose rate to the range of the LUT.
    norm = norm > 1.0 ? 1.0 : norm < 0.0 ? 0.0 : norm;  // clamp, [0.0 ... 1.0]
    
    for (size_t i = 0; i < log_iter; i++)               // rescale logarithmically, increasing contrast of low values
    {
        norm = norm * 9.0F + 1.0F;                      // [1.0 ... 10.0]
        norm = log10f(norm);                            // [0.0 ...  1.0]
    }//for
    
    idx = (size_t)(norm * ((float)lut_n - 1.0F));       // index of color
    
    *r_out = lut_r[idx];                                // return RGB values from LUT
    *g_out = lut_g[idx];
    *b_out = lut_b[idx];
}//SimpleLutDemo_ApplyLUT

void SimpleLutDemo_PerformDemo(const float usvh)
{
    uint8_t* lut_r = NULL;
    uint8_t* lut_g = NULL;
    uint8_t* lut_b = NULL;
    
    SimpleLutDemo_GetCyanHaloLUT(&lut_r, &lut_g, &lut_b);
    
    uint8_t red_out, green_out, blue_out;
    
    SimpleLutDemo_ApplyLUT(usvh, 0.03F, 65.535F, lut_r, lut_g, lut_b, 256, &red_out, &green_out, &blue_out);
    
    printf("AppDelegate.m: SimpleLutDemo_PerformDemo: The value %1.2f uSvh is color r=%d, g=%d, b=%d.\n",
           usvh,
           (int)red_out,
           (int)green_out,
           (int)blue_out);
    
    free(lut_r);
    lut_r = NULL;
    free(lut_g);
    lut_g = NULL;
    free(lut_b);
    lut_b = NULL;
}//SimpleLutDemo_PerformDemo










// =============================================================================
// =============================================================================
//
// "LutDemo" (without "Simple") functions demonstrate the "full" version of the
// LUT functions:
//
// 1. Many LUTs (from database)
// 2. Fully configurable parameters
// 3. Higher performance for many points (>100,000)
//
// But:
//
// 1. Complex -- many source files
// 2. Uses more RAM
// 3. Setup time cost before it can be used
//
// =============================================================================
// =============================================================================


- (void)LutDemo_Init
{
    // nb: gbThreadGuard is used to ensure performant threadsafety
    
    gbThreadGuard_Create(&_lut_thread_guard, GB_THREAD_GUARD_TIME_FOREVER);
    
    NSString* nss_db_path = [[NSBundle mainBundle] pathForResource:@"lut_v2" ofType:@"sqlite"];
    
    gbLUT_Properties props;
    
    props.bkg_alpha        = 0.0F;
    props.data_alpha       = 1.0F;
    props.rgba_type_id     = kGB_RGBAType_BGRA;
    props.buffer_type_id   = kGB_LUT_BufferType_RGBA;   // premultiplied interleaved 8-bit BGRA
    props.n                = 65536;                     // interpolate 256 -> 65536 colors
    props.lut_id           = 30;                        // Cyan Halo
    props.scale_type_id    = 2;                         // LOG10
    props.reverse          = false;
    props.discretize_steps = 256;                       // disabled
    props.min              = 0.03F;
    props.max              = 65.535F;

    gbLUT_LoadAndCreate(&props, nss_db_path.UTF8String, &_gb_lut);
}//LutDemo_Init


- (void)LutDemo_Apply
{
    gbThreadGuard_Wait(_lut_thread_guard); // nb: use WaitExclusive if destroying/recreating LUT while in use
    
    const size_t test_n     = 256;
    float*       test_data  = malloc(sizeof(float)   * test_n);
    float*       test_alpha = malloc(sizeof(float)   * test_n);
    uint8_t*     test_rgba  = malloc(sizeof(uint8_t) * test_n * 4);
    
    // fill with random data
    for (size_t i=0; i<test_n; i++)
    {
        test_data[i]  = ((float)(rand() % 70)) / 70.0F;
        test_alpha[i] = test_data[i] > 0.0F ? 1.0F : 0.0F;
    }//for
    
    // but cheat and hardcode the first to match the other demo.
    test_data[0]  = 0.15F;
    test_alpha[0] = 1.0F;
    
    gbLUT_Transform_DataAlpha_RGBA8888(test_data, test_alpha, &test_rgba, _gb_lut, test_n);
    
    gbThreadGuard_Signal(_lut_thread_guard); // release lock
    
    printf("AppDelegate.m: LutDemo_Apply: idx=0 is color r=%d, g=%d, b=%d.\n",
           (int)test_rgba[2],
           (int)test_rgba[1],
           (int)test_rgba[0]);
    
    free(test_data);
    test_data = NULL;
    free(test_alpha);
    test_alpha = NULL;
    free(test_rgba);
    test_rgba = NULL;
}//LutDemo_Apply


- (void)LutDemo_Destroy
{
    if (_lut_thread_guard != NULL)
    {
        gbThreadGuard_WaitExclusive(_lut_thread_guard);
        gbThreadGuard_SignalExclusive(_lut_thread_guard);
        gbThreadGuard_Destroy(_lut_thread_guard);
        _lut_thread_guard = NULL;
    }//if
    
    if (_gb_lut != NULL)
    {
        gbLUT_Destroy(_gb_lut);
        _gb_lut = NULL;
    }//if
}//LutDemo_Destroy

@end
