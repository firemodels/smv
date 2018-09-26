#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

//*** uncomment the following two lines to force all versions to be beta
//#undef pp_BETA
//#define pp_BETA

//*** define smokezip title

#ifdef pp_BETA
  #define PROGVERSION "Test"
#else
  #define PROGVERSION "1.5.0"
#endif


#include "lint.h"

//*** options: all platforms

#define pp_PART

//*** options: options being tested

#define pp_SMOKE3D_FORT

#ifdef pp_BETA
  #define pp_PART2
#endif

//*** options: Windows

#ifdef WIN32
#define pp_THREAD
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_THREAD
#endif

#ifdef pp_OSX
#define pp_THREAD
#endif

#endif
