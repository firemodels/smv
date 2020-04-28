#ifndef OPTIONS_H_DEFINED
#define OPTIONS_H_DEFINED

#include "options_common.h"

//*** uncomment the following two lines to force all versions to be beta
//#undef pp_BETA
//#define pp_BETA

//*** define smokeview title

#ifdef pp_BETA
  #define PROGVERSION "Test"
#else
  #define PROGVERSION ""
#endif

//*** options: all platforms

#define pp_GPU                // support the GPU
#define pp_THREAD             // turn on multi-threading
#define pp_GPUSMOKE           // code to speed up 3d smoke using the gpu
#define pp_DRAWISO            // turn on drawing routines
//#define pp_UPDATE_FILELOAD  // updates fileload variables when a file has been loaded or unloaded
//#define pp_SPECULAR         // add widgets for specular lighting parameters to the lighting dialog box

//*** debug: all platforms
//#define pp_ISOTIME          // output iso load times
//#define pp_PART_TEST        // for debugging, set particle values to 100*parti->seq_id + small random number

//*** in development: all platforms
#define pp_C_SLICE              // use C to read in slice file headers
#define pp_SMOKE_FAST           // load 3d smoke in parallel
//#define pp_SMOKETEST          // max blending for fire, regular blending for smoke
//#define pp_SPECTRAL           // use black body colors - not fully implemented
//#define pp_SLICETHREAD        // parallel slice file loading
//#define pp_BLOCK_COLOR        //  new algorithm for assigning obst colors
//#define pp_NEWBOUND_DIALOG    // redo bound dialog
//#define pp_PLOT3D_REDUCEMENUS // eliminate plot3d sub-menus
//#define pp_PLOT3D_STATIC      // use static memory for plot3d labels

#ifdef pp_GPU
#define pp_GPUTHROTTLE  // pp_GPU directive must also be set
#endif

#ifdef pp_THREAD
#define pp_THREADIBLANK // construct iblank arrays in background
#endif


//*** options: windows

#ifdef WIN32
//#define pp_OPENVR       // implement virtual reality using openvr library
#define pp_DEG          // output degree symbol
#define pp_memstatus
#define pp_COMPRESS     // support for smokezip
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_DEG          // output degree symbol
#endif

//*** options: Mac

#ifdef pp_OSX
#define pp_GLUTGET      // use d and f key in place of CTRL and ALT key
//#define pp_CLOSEOFF     // turn off and disable close buttons in dialog box
#endif

//*** options: options being tested on all platforms

#ifdef pp_BETA
#define pp_DEBUG_TERRAIN       // output spreadsheet version of terrain elevation file
//#define pp_SLICE_DEBUG         // slice debug output
#define pp_DEBUG_SUBMENU       // debug output and testing for building menus
//#define pp_SMOKEALPHA        // experimental smoke alpha settings
#define pp_TIMINGS
//#define pp_SMOKETEST         // experimental smoke dialog box entries
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_RENDER360_DEBUG
#define pp_SNIFF_ERROR
#endif

#endif

