#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

//#define pp_SHOW_UPDATE      // show what is being updated in the UpdateShowScene routine
#define pp_GPU              // support the GPU
#define pp_THREAD           // turn on multi-threading

//*** options: windows

#ifdef WIN32
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#ifdef pp_GPU
#define pp_WINGPU           // only draw 3d slices with the GPU on windows
#endif
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_REFRESH          // refresh glui dialogs when they change size
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#endif

//*** options: OSX

#ifdef pp_OSX
#define pp_SMOKE3D_FORCE        // always have at least one smoke3d entry to prevent crash when unloading slices
#define GL_SILENCE_DEPRECATION
#endif

#undef pp_OSX_HIGHRES
#ifdef pp_OSX
#define pp_REFRESH      // refresh glui dialogs when they change size
#ifndef pp_OSX_LOWRES
#define pp_OSX_HIGHRES
#endif
#endif

//*** options: for debugging

#ifdef _DEBUG
//#define pp_MEM_DEBUG_PRINT // output file/line number for each memory allocation call
#define pp_RECOMPUTE_DEBUG   // output debug message if bounds are recomputed
#define pp_BOUND_DEBUG       // output debug message in some bound routines
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif
