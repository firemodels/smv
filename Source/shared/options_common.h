#ifndef OPTIONS_COMMON_H_DEFINED
#define OPTIONS_COMMON_H_DEFINED

//*** options: all platforms

#ifndef _DEBUG
#define pp_HASH   // md5, sha1 and sha255 hashing
#endif

#ifdef pp_INTEL
#define pp_FSEEK
#endif

//*** options: windows

#ifdef WIN32
#undef pp_append

#ifdef VS2015                      // needed in visual studio to prevent compiler warning/errors
#define _CRT_SECURE_NO_DEPRECATE   // set to eliminate compiler warnings
#define _CRT_SECURE_NO_WARNINGS
#define HAVE_SNPRINTF
#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC
#endif
#endif

#include "pragmas.h"
#endif

#ifdef pp_HASH
#define PRINTVERSION(a,b) PRINTversion(a,b,hash_option)
#else
#define PRINTVERSION(a,b) PRINTversion(a)
#endif

//*** options: Mac

#ifdef pp_OSX
#define pp_append // append underscore to Fortran file names
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_append // append underscore to Fortran file names
#endif

#ifdef __MINGW32__
#ifdef WIN32
#ifndef pp_append
#define pp_append // append underscore to Fortran file names
#endif
#endif
#endif

//*** options: debug options

#ifdef _DEBUG
//#define pp_MEMPRINT     // output memory allocation info
#define pp_MEMDEBUG     // comment this line when debugging REALLY large cases (to avoid memory checks)
#endif

// used to access Fortran routines from C

#ifndef _F
#ifdef pp_append
#define _F(name) name ## _
#else
#define _F(name) name
#endif
#endif

// debugging macros

#ifdef pp_TRACE
#define BTRACE \
  printf("entering, file: %s, line: %d\n",__FILE__,__LINE__)
#define TTRACE \
  printf("in, file: %s, line: %d\n",__FILE__,__LINE__)
#define ETRACE \
  printf("leaving, file: %s, line: %d\n",__FILE__,__LINE__)
#else
#define BTRACE
#define TTRACE
#define ETRACE
#endif

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


#define GLUT_H <GL/glut.h>
#ifdef pp_OSX
#undef  GLUT_H
#define GLUT_H <GLUT/glut.h>
#endif
#ifdef pp_QUARTZ
#undef  GLUT_H
#define GLUT_H <GL/glut.h>
#endif

#include "lint.h"

#endif

