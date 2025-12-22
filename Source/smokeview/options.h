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
//#define pp_HTML             // add option to render to html (html render not working)

//*** options: windows

#ifdef _WIN32
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#endif

//*** options: Linux

#ifdef __linux__
#define pp_REFRESH          // refresh glui dialogs when they change size
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#endif

//*** options: OSX

#ifdef pp_OSX
#define pp_REFRESH      // refresh glui dialogs when they change size
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_RECOMPUTE_DEBUG   // output debug message if bounds are recomputed
#define pp_BOUND_DEBUG       // output debug message in some bound routines
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif
