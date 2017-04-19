#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

//*** uncomment the following two lines to force all versions to be beta
//#undef pp_BETA
//#define pp_BETA

//*** define smokezip title

#ifdef pp_BETA
  #define PROGVERSION "Test"
#else
  #define PROGVERSION "1.4.9"
#endif


#include "lint.h"

//*** options: all platforms

#define pp_PART

//*** options: options being tested

#ifdef pp_BETA
  #define pp_PART2
#endif  

//*** options: Windows

#ifdef WIN32
#include "pragmas.h"

#undef pp_append
#define pp_THREAD
#define _CRT_SECURE_NO_WARNINGS
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_THREAD
#define pp_append
#endif

//*** options: Mac

#ifdef pp_OSX
#define pp_append
#endif

//*** options: Debug

#ifdef _DEBUG
#define pp_MEMDEBUG
#endif

//*** used to access Fortran routines from C

#ifndef _F
#ifdef pp_append
#define _F(name) name ## _
#else
#define _F(name) name
#endif
#endif

//*** long variables

#define LINT long int
#ifdef X64
#undef LINT
#ifdef WIN32
#define LINT __int64
#else
#define LINT long long int
#endif
#endif

//*** for files

#define FILE_SIZE unsigned long long
#ifdef X64
#define STRUCTSTAT struct __stat64
#define STAT _stat64
#else
#define STRUCTSTAT struct stat
#define STAT stat
#endif

//*** referencing C++ files

#ifdef CPP
#define CCC "C"
#define EXTERNCPP extern "C"
#else
#define CCC
#define EXTERNCPP
#endif
#endif
