#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

#ifdef pp_BETA
#define pp_DEBUG_SUBMENU       // debug output and testing for building menus
#endif

//#define pp_PARTBOUND_MULTI // compute particle bounds in the background
//#define pp_LUA            // turn on LUA option
#define pp_CHOPFIX          // fix to saving chop values
//#define pp_SLICE_MENU     // make slice menu formation consistent (sort step, build step)
#define pp_SLICE_MULTI      // load slice files in parallel
//#define pp_CSV_MENU       // add menu for loading CSV files
//#define pp_SMOKE16        // load 16 bit smoke files
//#define pp_GAMMA          // show gamma correction checkbox
// #define pp_BLACKBODY_OUT // output generated blackbody color data 
//#define pp_DECIMATE       // decimate terrain geometry
#define pp_FAST             // set fast startup by default
//#define pp_DPRINT         // turn on debug print (file, line number)
//#define pp_LOAD_INC       // add menu item for loading new data
#define pp_PART_SPEEDUP     // improve efficiency of loading particles
#define pp_SMOKE_SPEEDUP    // improve efficiency of loading 3d smoke
#define pp_GPU              // support the GPU
#define pp_THREAD           // turn on multi-threading
#ifdef pp_GPU
#define pp_GPUTHROTTLE      // pp_GPU directive must also be set
#endif
#define pp_LOADALL_SLICE    // add menus to load all x, y, z slice files
#define pp_ONEBUFFER        // consolidate memory allocations when parsing .smv file
#define pp_MESH_BOUNDARY    // display boundary files on blockages that are on mesh boundaries
#define pp_MESHSKIP         // compute mesh skip arrays to display vector slices uniformly
#define pp_NOBOUNDS           // add option to not compute global bounds at startup

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
#define pp_RECOMPUTE_DEBUG   // output debug message if bounds are recomputed
#define pp_BOUND_DEBUG       // output debug message in some bound routines
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif

