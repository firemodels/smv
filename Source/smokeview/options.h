#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

#define pp_SMOKEFRAME    // turn on frame code for 3d smoke
#define pp_ISOFRAME      // turn on frame code for isosurfaces
#define pp_SLICEFRAME    // turn on frame code for slices
#define pp_PARTFRAME     // turn on frame code for particles
//#define pp_FRAME_DEBUG   // output frames read in
//#define pp_FRAME_DEBUG2  // output frame times

//#define pp_PARTBOUND_MULTI // compute particle bounds in the background
//#define pp_LUA            // turn on LUA option
#define pp_SLICE_MULTI      // load slice files in parallel
//#define pp_SMOKE_MULTI      // load 3d smoke in parallel
//#define pp_SMOKE16        // load 16 bit smoke files
//#define pp_GAMMA          // show gamma correction checkbox
// #define pp_BLACKBODY_OUT // output generated blackbody color data 
#define pp_FAST             // set fast startup by default
#define pp_PART_SPEEDUP     // improve efficiency of loading particles
#define pp_SMOKE_SPEEDUP    // improve efficiency of loading 3d smoke
#define pp_SMOKEDRAW_SPEEDUP    // improve efficiency of drawing 3d smoke
#define pp_GPU              // support the GPU
#define pp_THREAD           // turn on multi-threading
#ifdef pp_GPU
#define pp_GPUTHROTTLE      // pp_GPU directive must also be set
#endif
//#define pp_FDS            // create a 1 mesh input file

// turn on pp_FRAME if a frame directive is set for one of the file types
#ifdef pp_SMOKEFRAME
#ifndef pp_FRAME
#define pp_FRAME
#endif
#endif
#ifdef pp_SLICEFRAME
#ifndef pp_FRAME
#define pp_FRAME
#endif
#endif
#ifdef pp_ISOFRAME
#ifndef pp_FRAME
#define pp_FRAME
#endif
#endif
#ifdef pp_PARTFRAME
#ifndef pp_FRAME
#define pp_FRAME
#endif
#endif

//*** options: windows

#ifdef WIN32
#define pp_memstatus
#define pp_COMPRESS         // support for smokezip
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

//*** options: Mac

#ifdef pp_OSX
#ifndef pp_NOQUARTZ     // if used, passed in from the command line so we don'thave to change source
#define pp_QUARTZ       // use Quartz
#endif
#endif

#ifdef pp_QUARTZ
#define pp_CLOSEOFF     // turn off and disable close buttons in dialog box
#endif

#undef pp_OSX_HIGHRES
#ifdef pp_OSX
#ifndef pp_QUARTZ
#define pp_REFRESH      // refresh glui dialogs when they change size
#ifndef pp_OSX_LOWRES
#define pp_OSX_HIGHRES
#endif
#endif
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_DEBUG_SUBMENU     // debug output and testing for building menus
#define pp_RECOMPUTE_DEBUG   // output debug message if bounds are recomputed
#define pp_BOUND_DEBUG       // output debug message in some bound routines
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif

