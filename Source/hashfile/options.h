#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#define pp_HASH

#include "options_common.h"

//*** uncomment the following two lines to force all versions to be beta
//#undef pp_BETA
//#define pp_BETA

//*** define dem2fds title

#ifdef pp_BETA
#define PROGVERSION "Test"
#else
#define PROGVERSION "1.0"
#endif

#endif
