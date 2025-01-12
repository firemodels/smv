#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

//#define pp_FRAME           // turn on frame code
//#define pp_SHOW_UPDATE      // show what is being update in UpdateShowScene routine
//#define pp_PARTBOUND_MULTI // compute particle bounds in the background
//#define pp_LUA            // turn on LUA option
//#define pp_SMOKE16        // load 16 bit smoke files
#define pp_FAST             // set fast startup by default
#define pp_GPU              // support the GPU
#define pp_THREAD           // turn on multi-threading
//#define pp_SKYBOX_FLOOR     // use DrawFloor routine when using a skybox (probably not necessary)
//#define pp_SKY              // add sky, a blue hemisphere, above the scene
//#define pp_TOUR_SNAP         // turn on tour snap option
//#define pp_BOUND_FACE       // show hide obst face independently when visualizing boundary files

#ifdef pp_FRAME          // turn on each frame type if pp_FRAME is set
#define pp_BOUNDFRAME    // turn on frame code for boundary files
#define pp_SMOKEFRAME    // turn on frame code for 3d smoke
#define pp_ISOFRAME      // turn on frame code for isosurfaces
#define pp_SLICEFRAME    // turn on frame code for slices
#define pp_PARTFRAME     // turn on frame code for particles
//#define pp_OPEN_TEST     // count number of file opens and closes
//#define pp_FRAME_DEBUG   // frame timing test
//#define pp_FRAME_DEBUG2  // output frame times
#endif

#ifndef pp_FRAME          // turn on option if pp_FRAME is not set
#define pp_PART_COUNT    // precompute particle frame counting when loading all particle files
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
#define GL_SILENCE_DEPRECATION
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
//#define pp_MEM_DEBUG_PRINT   // output file/line number for each memory allocation call
#define pp_RECOMPUTE_DEBUG   // output debug message if bounds are recomputed
#define pp_BOUND_DEBUG       // output debug message in some bound routines
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif

