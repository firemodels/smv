#ifndef OPTIONS_COMMON_H_DEFINED
#define OPTIONS_COMMON_H_DEFINED

#if defined(__APPLE__) && defined(__MACH__)
#define pp_OSX
#endif

//*** options: all platforms

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifdef __INTEL_COMPILER
#define INTEL_COMPILER_ANY
#endif

#ifdef __INTEL_LLVM_COMPILER
#undef INTEL_COMPILER_ANY
#define INTEL_COMPILER_ANY
#define INTEL_LLVM_COMPILER
#endif

#ifdef INTEL_LLVM_COMPILER_FORCE
#undef INTEL_COMPILER_ANY
#define INTEL_COMPILER_ANY
#undef INTEL_LLVM_COMPILER
#define INTEL_LLVM_COMPILER
#endif

#ifdef INTEL_COMPILER_ANY
#define pp_FSEEK
#ifdef _WIN32
#define HAVE_MSVS
#define INTEL_WIN_COMPILER
#endif
#endif

// Microsofts MSVC has timespec defined
#ifdef _MSC_VER
#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC
#endif
#endif

//*** options: windows

#ifdef _WIN32

//*** needed when using Windows Intel compilers
//    to prevent warnings/errors

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef HAVE_SNPRINTF
#define HAVE_SNPRINTF
#endif

#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC
#endif

#include "pragmas.h"
#endif

//*** options: debug options

#ifdef _DEBUG
//#define pp_MEMPRINT     // output memory allocation info
#define pp_MEMDEBUG     // comment this line when debugging REALLY large cases (to avoid memory checks)
#endif
#ifdef pp_MEMDEBUG
#define pp_memusage
#endif

//*** hash output

#define PRINTVERSION(a,opts) PRINTversion(a,(opts)->hash_option)

#define FILE_SIZE unsigned long long

#ifdef __cplusplus
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

#define GL_H <GL/gl.h>
#ifdef pp_OSX
#undef  GL_H
#define GL_H <OpenGL/gl.h>
#endif

#define GLU_H <GL/glu.h>
#ifdef pp_OSX
#undef  GLU_H
#define GLU_H <OpenGL/glu.h>
#endif

#ifndef START_TIMER
#define START_TIMER(a) a = (float)clock()/(float)CLOCKS_PER_SEC
#endif

#ifndef STOP_TIMER
#define STOP_TIMER(a) a = (float)clock()/(float)CLOCKS_PER_SEC - a
#endif

#ifndef CUM_TIMER
#define CUM_TIMER(a,b) b += ((float)clock()/(float)CLOCKS_PER_SEC - a)
#endif

#ifndef INIT_PRINT_TIMER
#define INIT_PRINT_TIMER(timer)   float timer;START_TIMER(timer)
#endif

#ifndef PRINT_TIMER
#define PRINT_TIMER(timer, label) PrintTime(__FILE__, __LINE__, &timer, label, 1)
#endif

#ifndef PRINT_CUM_TIMER
#define PRINT_CUM_TIMER(timer, label) PrintTime(__FILE__, __LINE__, &timer, label, 0)
#endif

// Define a NORETURN macro that marks a function as never returning. This is
// needed to mark that SMV_EXIT never returns, otherwise tools like clang-tidy
// would find spurious issues.
#ifndef noreturn
#  if (__STDC_VERSION__ >= 201112L) && !defined(_WIN32)
     // C11 provides a standard 'noreturn' macro that can be used. Conflicts
     // means this doesn't work well on windows
#    include <stdnoreturn.h>
#    define NORETURN noreturn
#  elif defined(_WIN32)
     // noreturn as defined on windows
#    define NORETURN _declspec(noreturn)
#  else
     // noreturn as defined on other platforms
#    define NORETURN __attribute__((noreturn))
#  endif
#endif

#include "lint.h"

#define pp_GPU              // support the GPU
#define pp_THREAD           // turn on multi-threading

//*** options: windows

#ifdef WIN32
#ifdef pp_GPU
#define pp_WINGPU           // only draw 3d slices with the GPU on windows
#endif
#endif

//*** options: OSX

#ifdef pp_OSX
#define pp_SMOKE3D_FORCE        // always have at least one smoke3d entry to prevent crash when unloading slices
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#endif

#undef pp_OSX_HIGHRES
#ifdef pp_OSX
#define pp_OSX_HIGHRES
#endif

//*** options: for debugging

#ifdef _DEBUG
//#define pp_MEM_DEBUG_PRINT // output file/line number for each memory allocation call
#endif

#endif
