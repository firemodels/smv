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

#define pp_ZONETL
#define pp_DRAWISO      // turn on drawing routines
#define pp_ffmpeg       // support compression
#define pp_FILELIST     // use list of file names
#define pp_COLORBARFLIP // flip colorbar if soot visibility is shown

//#define pp_PARTDEFER    // defer particle bound and coloring until last particle file is loaded
//#define pp_SPECTRAL
//#define pp_OPACITYMAP

#define pp_GPU          // support the GPU
#ifdef pp_GPU
#ifndef pp_OSX
#define pp_CULL         // pp_GPU directive must also be set 
#endif
#define pp_GPUTHROTTLE  // pp_GPU directive must also be set
#endif

//#define pp_PARTTEST   // for debugging, set particle values to 100*parti->seq_id + small random number
#define pp_READBUFFER   // read .smv file from a memory buffer
#define pp_SLICELOAD     // use slice file parameters found in .smv file to construct menus

#define pp_THREAD       // turn on multi-threading
#ifdef pp_THREAD
#define pp_THREADIBLANK // construct iblank arrays in background
#endif


//*** options: windows

#ifdef WIN32
#define pp_LANG         // support other languages
#define pp_DEG          // output degree symbol
#define pp_memstatus
#define pp_COMPRESS     // support for smokezip
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_LANG         // support other languages
#define pp_DEG          // output degree symbol
#endif

//*** options: Mac

#ifdef pp_OSX
#define pp_GLUTGET      // use d and f key in place of CTRL and ALT key
// #define pp_OSXGLUT32 // used to test advanced OpenGL profile on mac
#endif

//*** options: options being tested on all platforms

#ifdef pp_BETA
#define pp_SHOWTERRAIN
#define pp_GEOMTEST      // used to test tetrahedron box intersections
#define pp_TIMINGS
//#define pp_GPUDEPTH
#define pp_SMOKETEST
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_RENDER360_DEBUG
#endif

#endif

