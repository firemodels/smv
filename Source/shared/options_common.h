#ifndef OPTIONS_COMMON_H_DEFINED
#define OPTIONS_COMMON_H_DEFINED

//*** options: all platforms

#define pp_HASH   // md5, sha1 and sha255 hashing

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
#define PRINTVERSION(a,b) PRINTversion(a,b)
#endif

//*** options: Mac

#ifdef pp_OSX
#define pp_append // append underscore to Fortran file names
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_append // append underscore to Fortran file names
#endif

//*** options: debug options

#ifdef _DEBUG
#define pp_MEMPRINT     // output memory allocation info
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

