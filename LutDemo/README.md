LutDemo
=======

LutDemo is an ObjC project that shows how to get a RGB color for a data value in uSv/h, using the standard Safecast color LUT (lookup table), Cyan Halo.  The individual sources used by the project are written in ANSI C.

For example, 0.03 uSv/h is dark blue, 1.00 uSv/h is red, and 60.00 uSv/h is yellow.

This is a demonstration of two different methods to implement this.  All output is to the Xcode console.  Running it should produce the following:

`AppDelegate.m: SimpleLutDemo_PerformDemo: The value 0.15 uSvh is color r=59, g=150, b=255.`  
`AppDelegate.m: LutDemo_Apply: idx=0 is color r=59, g=157, b=255.`

Additionally, `lut_v2.sqlite` is a SQLite3 database which contains all of the LUTs used by the OS X and iOS apps.

### METHOD 1: SIMPLE
----------------

If full customization or processing millions of points is not needed, the simple version should be used.  It works on a single data value.

This is fully encapsulated by (in `AppDelegate.m`) the function `SimpleLutDemo_PerformDemo()`.  Ultimately, you only need to copy and paste two functions into your code:
  
1. `SimpleLutDemo_GetCyanHaloLUT()`  
2. `SimpleLutDemo_ApplyLUT()`  
  
You must retain the individual RGB components of the lookup table.  

A JavaScript implementation of this code can be found [here.](https://github.com/Safecast/Tilemap/blob/master/bgeigie_viewer.js#L1569)
  

### METHOD 2: FULL
----------------
  
A much more complicated version, with greater customization and much higher performance for very large numbers of points.  It does not work on single data values, but on vectors (arrays) of many values.  

Note that this code has been manually extracted from a beta version of the Safecast OS X app, and is subject to change.  It may also contain bugs.

You will need to base your code off the following in `AppDelegate.m`:  
  
1. `[LutDemo_Init]`  
2. `[LutDemo_Apply]`  
3. `[LutDemo_Destroy]`  
  
You must include (in your .h file):  
  
1. `#import "gbThreadGuard.h"`  
2. `#import "gbLUT.h"`  
  
The full files required:  
  
1. `gbCommon_TypeDefs.h`  
2. `gbDB.h`  
3. `gbDB.c`  
4. `gbImage_Utils.h`  
5. `gbImage_Utils.c`  
6. `gbThreadGuard.h`  
7. `gbThreadGuard.c`  
8. `gbLUT_Types.h`  
9. `gbLUT.h`  
10. `gbLUT.c`  
11. `gbLUT_Utils.h`  
12. `gbLUT_Utils.c`  
13. `gbLUT_Transforms.h`  
14. `gbLUT_Transforms.c`  
15. `gbLUT_Numerics.h`  
16. `gbLUT_Numerics.c`  
17. `gbLUT_Numerics_SIMD.h`  
18. `gbLUT_Numerics_SIMD.c`  
19. `NEONvsSSE_5.h`          *(only on Intel platforms)*
20. `NEONvsSSE_5_gbExt.h`    *(only on Intel platforms)*
21. `lut_v2.sqlite`  
  
  


### KNOWN ISSUES

`gbThreadGuard.h` does not compile properly as pure source in an ObjC+ARC project.  For that reason, ARC has been disabled in this project.

The issue is that `gbThreadGuard.h` uses libdispatch's `dispatch_sempahore_t` within a C struct.  This is valid in ANSI C, but in a ObjC project, the file is instead treated as ObjC, and `dispatch_sempahore_t` becomes an ObjC object.

This can likely be worked around by compiling as a static C library (untested) but will need to be investigated in more depth.

Another workaround is to replace gbThreadGuard with a `dispatch_semaphore_t` and `dispatch_sempahore_wait()` / `dispatch_sempahore_signal()`.  Or even a mutex lock.  Any of these approaches will be somewhat less performant.
