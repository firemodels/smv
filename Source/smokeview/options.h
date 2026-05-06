#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

// processing directives defined by compilers to identify the platform (windows, Linux or OSX) or compiler
// __linux__   use instead of pp_LINUX
// __GNUC__    use instead of pp_GCC
// defined(__APPLE__) && defined(__MACH__)   __MACH__ is included to exclude iOS, use instead of pp_OSX
// _WIN32      use instead of WIN32
// _WIN64      use instead of X64
// __cplusplus use instead of CPP

#include "options_common.h"

//#define pp_SHOW_UPDATE      // show what is being updated in the UpdateShowScene routine
//#define pp_TERRAIN_HIDE     // hide terrain when hiding internal blockages
//#define pp_OPACITY_DEBUG    // output hrrpuv opacity in center of each mesh
//#define pp_GLUT_DEBUG       // add debug print for glut debugging
#define pp_SPEEDUP            // speed up smokeview
//#define pp_READ_KEYBOARD      // read keyboard in the command shell so the visualization can be paused 
//#define pp_GETMESH_TEST
//#define pp_COMPRESS         // new compression code

//*** options: windows

#ifdef _WIN32
#endif

//*** options: Linux

#ifdef __linux__
#endif

//*** options: OSX

#ifdef pp_OSX
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_BNDF_DEBUG        // add 3 checkboxes for controlling different types of boundary file output
#define pp_BOUND_DEBUG       // output debug message in some bound routines and if bounds are recomputed
#endif

#endif
