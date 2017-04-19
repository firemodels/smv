#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

//*** uncomment the following two lines to force all versions to be beta
//#undef pp_BETA
//#define pp_BETA

//*** define smokediff title

#ifdef pp_BETA
  #define PROGVERSION "Test"
#else
  #define PROGVERSION "1.0.10"
#endif

//*** options: windows

#ifdef WIN32
#include "pragmas.h"

#undef pp_append
#define _CRT_SECURE_NO_WARNINGS
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_append
#endif

//*** options: MAC

#ifdef pp_OSX
#define pp_append
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_MEMDEBUG
#endif

//*** referencing Fortran routines from C

#ifndef _F
#ifdef pp_append
#define _F(name) name ## _
#else
#define _F(name) name
#endif
#endif

//*** referenging C routines from C++

#ifdef CPP
#define CCC "C"
#define EXTERNCPP extern "C"
#else
#define CCC
#define EXTERNCPP
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

//*** declaring and defining variables

#ifdef INMAIN
#define SVEXTERN
#define SVDECL(var,val)  var=val
#else
#define SVEXTERN extern CCC
#define SVDECL(var,val)  var
#endif

#endif
