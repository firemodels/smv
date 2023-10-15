#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"
#define pp_SMOKEZIP

//*** uncomment the following two lines to force all versions to be beta
//#undef pp_BETA
//#define pp_BETA

#include "lint.h"

//*** options: all platforms

#define pp_PART

//*** options: options being tested

#define pp_SMOKE3D_FORT

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
