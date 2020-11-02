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

//#define pp_SHOW_BOUND_MIRROR  // add menu for showing boundary files on mirror/open vents
#define pp_MULTI_RES          // read in multi-resolution slice files
//#define pp_HTML_VR           // output VR html code
//#define pp_PART_HIST            // compute particle histograms
//#define pp_SKIP_TERRAIN_DATA  // skip reading in terrain data if case is using immersed geometry
//#define pp_SKIP_BOUNDARY_GEOMS // skips reading of boundary geometry files
#define pp_GPU                // support the GPU
#define pp_THREAD             // turn on multi-threading
#define pp_GPUSMOKE           // code to speed up 3d smoke using the gpu
#define pp_DRAWISO            // turn on drawing routines
//#define pp_UPDATE_FILELOAD  // updates fileload variables when a file has been loaded or unloaded
//#define pp_SPECULAR         // add widgets for specular lighting parameters to the lighting dialog box

//#define pp_WUI_VAO            // use opengl vao objects for drawing terrain

//*** debug: all platforms
//#define pp_ISOTIME          // output iso load times
//#define pp_PART_TEST        // for debugging, set particle values to 100*parti->seq_id + small random number

//*** in development: all platforms
#define pp_SMOKE_FAST           // load 3d smoke in parallel
//#define pp_SMOKETEST          // max blending for fire, regular blending for smoke
//#define pp_SPECTRAL           // use black body colors - not fully implemented
//#define pp_SLICETHREAD        // parallel slice file loading
//#define pp_BLOCK_COLOR        //  new algorithm for assigning obst colors
#define pp_CPPBOUND_DIALOG    // redo bound dialog using c++
//#define pp_PERCENTILES        // add percentile widget to new bounds dialog box
//#define pp_PLOT3D_REDUCEMENUS // eliminate plot3d sub-menus
//#define pp_PLOT3D_STATIC      // use static memory for plot3d labels
//#define pp_LOAD_INCREMENTAL   // load data incrementally
//#define pp_SLICE_BUFFER       // read  slice file into a buffer before processing
//#define pp_RESEARCH_DEBUG     // output whether data is reloaded or colors reemapped when toggling to/from research mode
//#define pp_MERGE_GEOMS          // merge geometry and structure boundary file menu items


// define old dialog if not using new dialogs
#ifndef pp_CPPBOUND_DIALOG
#define pp_OLDBOUND_DIALOG
#endif

#ifdef pp_GPU
#define pp_GPUTHROTTLE  // pp_GPU directive must also be set
#endif

#ifdef pp_THREAD
#define pp_THREADIBLANK // construct iblank arrays in background
#endif


//*** options: windows

#ifdef WIN32
#define pp_DEG              // output degree symbol
#define pp_memstatus
#define pp_COMPRESS         // support for smokezip
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_DEG          // output degree symbol
#define pp_REFRESH      // refresh glui dialogs when they change size
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#endif

//*** options: Mac

#ifdef pp_OSX
#define pp_GLUTGET      // use d and f key in place of CTRL and ALT key
#define pp_QUARTZ         // use Quartz
#define pp_REFRESH      // refresh glui dialogs when they change size
// no dialog short cuts on the mac
#endif

#ifdef pp_QUARTZ
#define pp_CLOSEOFF     // turn off and disable close buttons in dialog box
#endif

//*** options: options being tested on all platforms

#ifdef pp_BETA
#define pp_DEBUG_TERRAIN       // output spreadsheet version of terrain elevation file
//#define pp_SLICE_DEBUG         // slice debug output
#define pp_DEBUG_SUBMENU       // debug output and testing for building menus
#define pp_TIMINGS
//#define pp_SMOKETEST         // experimental smoke dialog box entries
#endif

//*** options: for debugging

#ifdef _DEBUG
#ifndef pp_RESEARCH_DEBUG
#define pp_RESEARCH_DEBUG
#endif
#define pp_RENDER360_DEBUG
#define pp_SNIFF_ERROR
#endif

#endif

