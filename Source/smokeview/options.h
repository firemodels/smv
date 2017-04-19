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
  #define PROGVERSION "6.4.4"
#endif

//*** options: all platforms

#define pp_GPU          // support the GPU
#define pp_ffmpeg       // support compression
#define pp_READBUFFER   // read .smv file from a memory buffer
#define pp_DRAWISO      // turn on drawing routines
#define pp_LANG         // support other languages
#define pp_DEG          // output degree symbol
#define pp_RENDER360    // render 360 images
//#define pp_PARTTEST   // for debugging, set particle values to 100*parti->seq_id + small random number
#define pp_THREAD       // turn on multi-threading
#define pp_THREADIBLANK // construct iblank arrays in background
#ifdef pp_THREADIBLANK  // turn on multi-threading
#define pp_THREAD
#endif

#ifdef pp_GPU
#define pp_CULL
#define pp_GPUTHROTTLE
#endif

//*** options: windows

#ifdef WIN32
#define pp_memstatus
#define pp_COMPRESS
#endif

//*** options: Mac

#ifdef pp_OSX
#undef pp_LANG          // turn off language support - doesn't work
#undef pp_DEG           // turn off degree symbol output - doesn't work
#define pp_GLUTGET      // use d and f key in place of CTRL and ALT key
// #define pp_OSXGLUT32 // used to test advanced OpenGL profile on mac
#endif

//*** options: options being tested on all platforms

#ifdef pp_BETA
#define pp_SLICELOAD     // use slice file parameters found in .smv file to construct menus
#define pp_SHOWTERRAIN
#define pp_GEOMTEST      // used to test tetrahedron box intersections
#define pp_HAZARD        //
//#define pp_GPUDEPTH
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_RENDER360_DEBUG
#endif

#endif

