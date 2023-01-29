#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

//*** define smokeview title

#ifdef pp_BETA
  #define PROGVERSION "Test"
  #define pp_DEBUG_SUBMENU       // debug output and testing for building menus
#else
  #define PROGVERSION ""
#endif

//#define pp_SMOKEFIX         // adjust smoke alphas
//#define pp_COLORBAR_DEBUG     // output colorbar debug info
#define pp_COLORBARS_CSV      // add csv colorbars//
//#define pp_COLORBAR_CONSTANT  // make colorbar brightness constant in colorbar dialog box
//define pp_COLORBAR_HSL        // add option to display color bar in terms of HSL (hue, saturation, lightness)

// use floating point to color data

#define pp_PARTVAL          // speed up part file color updating
#define pp_SLICEVAL         // speed up slice file color updating
#define pp_BOUNDVAL         // speed up boundary file color updating
#define pp_PLOT3DVAL        // speed up plot3d file color updating

//*** parallel file loading
#define pp_SLICE_MULTI        // load slice files in parallel
#define pp_PART_MULTI         // load particle files in parallel

// streaming directives

//#define pp_SMOKE3DSTREAM      // stream smoke3d data
//#define pp_PARTSTREAM         // stream particle data

#ifdef pp_SLICEVAL
#define pp_SLICEBOUNDVAL
#endif

#ifdef pp_BOUNDVAL
#undef pp_SLICEBOUNDVAL
#define pp_SLICEBOUNDVAL
#endif

// turn on pp_STREAM if streaming is on for any file type

#ifdef pp_SMOKE3DSTREAM
#define pp_STREAM
#endif
#ifdef pp_PARTSTREAM
#undef pp_STREAM
#define pp_STREAM
#endif

#define pp_THREADBUFFER
//#define pp_CRASH_TEST       // test detection of division by zero or use of undefined pointer
#define pp_GPU                // support the GPU
#define pp_THREAD             // turn on multi-threading
#define pp_DRAWISO            // turn on drawing routines
//#define pp_LOAD_NEWDATA     // add button for loading new data

#ifdef pp_GPU
#define pp_GPUTHROTTLE  // pp_GPU directive must also be set
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
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif

