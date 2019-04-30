#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

//*** uncomment the following two lines to force all versions to be beta
//#undef pp_BETA
//#define pp_BETA

//*** define smokeview title

#ifdef pp_BETA
  #define PROGVERSION "Test"
#else
  #define PROGVERSION ""
#endif

//*** options: all platforms

#define pp_HTML                // output html version of smokeview scene
//#define pp_PARTDEBUG        // debug output for particles file I/O
//#define pp_SPECULAR         // add widgets for specular lighting parameters to the lighting dialog box
//#define pp_VENTPROFILE      // show original zonefire vent flow profile (calculated in smokeview)
#define pp_GPUSMOKE           // code to speed up 3d smoke using the gpu
//#define pp_SMOKEDIAG        // output smoke3d diagnostics (number of meshes, total triangles, triangles drawn)
//#define pp_GEOMPRINT        // output geometry info
//#define pp_MAKE_SMOKEIBLANK // generate smoke iblank arrays
//#define pp_DPRINT           // debug print, printf line number and source file
#define pp_CLIP               // test frustum near/far clip planes
#define pp_DRAWISO            // turn on drawing routines
#define pp_FILELIST           // use list of file names
#define pp_LANG               // support other languages
#define pp_SLICE_USE_ID       // use slcf index
//#define pp_ISOTIME          // output iso load times
#define pp_ISOTHREAD          // use multithreading for isosurfaces
//#define pp_SMOKETEST        // max blending for fire, regular blending for smoke

//#define pp_SPECTRAL

#define pp_GPU                // support the GPU
#ifdef pp_GPU
#define pp_GPUTHROTTLE  // pp_GPU directive must also be set
#endif

//#define pp_PARTTEST   // for debugging, set particle values to 100*parti->seq_id + small random number
#define pp_READBUFFER   // read .smv file from a memory buffer

#define pp_THREAD       // turn on multi-threading
#ifdef pp_THREAD
#define pp_THREADIBLANK // construct iblank arrays in background
#endif


//*** options: windows

#ifdef WIN32
#define pp_DEG          // output degree symbol
#define pp_memstatus
#define pp_COMPRESS     // support for smokezip
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_DEG          // output degree symbol
#endif

//*** options: Mac

#ifdef pp_OSX
#define pp_GLUTGET      // use d and f key in place of CTRL and ALT key
// #define pp_OSXGLUT32 // used to test advanced OpenGL profile on mac
#endif

//*** options: options being tested on all platforms

#ifdef pp_BETA
//#define pp_SLICE_DEBUG         // slice debug output
#define pp_DEBUG_SUBMENU       // debug output and testing for building menus
//#define pp_SMOKE3D_LOADTEST  // load 3d smoke for all meshes for each time step 
//#define pp_SMOKEALPHA        // experimental smoke alpha settings
#define pp_SHOWTERRAIN
#define pp_TIMINGS
//#define pp_GPUDEPTH
//#define pp_SMOKETEST         // experimental smoke dialog box entries
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_RENDER360_DEBUG
#endif

#endif

