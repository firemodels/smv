#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

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

#define _CRT_SECURE_NO_DEPRECATE   // set to eliminate compiler warnings

#ifdef pp_GPU
#define pp_CULL
#define pp_GPUTHROTTLE
#endif

#ifdef VS2015            // needed in visual studio to prevent compiler warning/errors
#define HAVE_SNPRINTF
#define HAVE_STRUCT_TIMESPEC
#endif

//*** options: windows

#ifdef WIN32
#define pp_memstatus
#define pp_COMPRESS
#define pp_noappend
#include "pragmas.h"
#endif

//*** options: MAC

#ifdef pp_OSX
#undef pp_LANG   // turn off language support - doesn't work
#undef pp_DEG    // turn off degree symbol output - doesn't work
#define pp_GLUTGET // use d and f key in place of CTRL and ALT key
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

//*** options: debug options

#ifdef _DEBUG
#define pp_RENDER360_DEBUG
#define pp_MEMPRIN      // output memory allocation info
#define pp_MEMDEBUG     // comment this line when debugging REALLY large cases (to avoid memory checks)
#endif

//*** used to access fortran routines from C

#ifndef _F
#ifdef pp_noappend
#define _F(name) name
#else
#define _F(name) name ## _
#endif
#endif

//*** defines used by various headers

#define FILE_SIZE unsigned long long

#ifdef X64
#define STRUCTSTAT struct __stat64
#define STAT _stat64
#else
#define STRUCTSTAT struct stat
#define STAT stat
#endif

#define LINT long int
#ifdef X64
#undef LINT
#ifdef WIN32
#define LINT __int64
#else
#define LINT long long int
#endif
#endif

#ifdef CPP
#define CCC "C"
#define EXTERNCPP extern "C"
#else
#define CCC
#define EXTERNCPP
#endif

#ifdef INMAIN
#define SVEXTERN
#define SVDECL(var,val)  var=val
#else
#define SVEXTERN extern CCC
#define SVDECL(var,val)  var
#endif

#ifdef pp_OSX
#define GLUT_H <GLUT/glut.h>
#else
#define GLUT_H <GL/glut.h>
#endif
#include "lint.h"

#endif

