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

//#define pp_CRASH_TEST         // test detection of division by zero or use of undefined pointer
//#define pp_SHOW_BOUND_MIRROR  // add menu for showing boundary files on mirror/open vents
//#define pp_HTML_VR           // output VR html code
//#define pp_SKIP_BOUNDARY_GEOMS // skips reading of boundary geometry files
#define pp_GPU                // support the GPU
#define pp_THREAD             // turn on multi-threading
#define pp_GPUSMOKE           // code to speed up 3d smoke using the gpu
#define pp_DRAWISO            // turn on drawing routines
//#define pp_UPDATE_FILELOAD  // updates fileload variables when a file has been loaded or unloaded
//#define pp_SPECULAR         // add widgets for specular lighting parameters to the lighting dialog box
//#define pp_LOAD_NEWDATA     // add button for loading new data
//#define pp_TIMEBAR_DIGITS   // add widget for setting the number of time bar digits
//#define pp_TERRAIN_UPDATE     // add button to update terrain buttons

//#define pp_WUI_VAO            // use opengl vao objects for drawing terrain

//*** debug: all platforms
//#define pp_ISOTIME          // output iso load times
//#define pp_PART_TEST        // for debugging, set particle values to 100*parti->seq_id + small random number

//*** in development: all platforms
#define pp_SMOKE_FAST           // load 3d smoke in parallel
//#define pp_SMOKETEST          // max blending for fire, regular blending for smoke
//#define pp_SPECTRAL           // use black body colors - not fully implemented
#define pp_SLICETHREAD        // parallel slice file loading
//#define pp_SHOW_CACHE         // show file cache checkbox
//#define pp_PLOT3D_REDUCEMENUS // eliminate plot3d sub-menus
//#define pp_SLICE_BUFFER       // read  slice file into a buffer before processing
//#define pp_RESEARCH_DEBUG     // output whether data is reloaded or colors re-mapped when toggling to/from research mode
//#define pp_MERGE_GEOMS          // merge geometry and structure boundary file menu items

#ifdef pp_GPU
#define pp_GPUTHROTTLE  // pp_GPU directive must also be set
#endif

//*** options: windows

#ifdef WIN32
#define pp_memstatus
#define pp_COMPRESS         // support for smokezip
#define pp_DIALOG_SHORTCUTS // dialog shortcuts
#endif

//*** options: Linux

#ifdef pp_LINUX
#define pp_REFRESH      // refresh glui dialogs when they change size
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

//*** options: options being tested on all platforms

#ifdef pp_BETA
#define pp_DEBUG_SUBMENU       // debug output and testing for building menus
//#define pp_SMOKETEST         // experimental smoke dialog box entries
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#ifndef pp_RESEARCH_DEBUG
#define pp_RESEARCH_DEBUG
#endif
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif

