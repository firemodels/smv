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

#ifdef pp_LINUX
#define pp_OSXLINUX
#endif
#ifdef pp_OSX
#define pp_OSXLINUX
#endif

//*** options: all platforms

//#define pp_THINFACE         // don't draw sides of thin faces

#define pp_SORTSLICES        // split slices to improve transparent drawing
// use floating point to color data

#define pp_PARTVAL          // speed up part file color updating
#define pp_SLICEVAL         // speed up slice file color updating
#define pp_BOUNDVAL         // speed up boundary file color updating
#define pp_PLOT3DVAL        // speed up plot3d file color updating

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

#define pp_PART_MULTI        // load particles in parallel
//#define pp_CACHE_FILEBOUNDS   // cache slice and boundary file bounds
#define pp_THREADBUFFER
//#define pp_SMOKEBUFFER       // read 3d smoke files using memory buffer i/o routines
//#define pp_GEOM_DEBUG        // debug out in geometry routines
//#define pp_CRASH_TEST         // test detection of division by zero or use of undefined pointer
//#define pp_SHOW_BOUND_MIRROR  // add menu for showing boundary files on mirror/open vents
//#define pp_HTML_VR           // output VR html code
//#define pp_SKIP_BOUNDARY_GEOMS // skips reading of boundary geometry files
#define pp_GPU                // support the GPU
#define pp_THREAD             // turn on multi-threading
#define pp_DRAWISO            // turn on drawing routines
//#define pp_LOAD_NEWDATA     // add button for loading new data
//#define pp_TERRAIN_UPDATE     // add button to update terrain normals

//#define pp_WUI_VAO            // use opengl vao objects for drawing terrain

//*** debug: all platforms
//#define pp_ISOTIME          // output iso load times
//#define pp_PART_TEST        // for debugging, set particle values to 100*parti->seq_id + small random number

//*** in development: all platforms
#define pp_SLICETHREAD        // parallel slice file loading
//#define pp_SHOW_CACHE         // show file cache checkbox
//#define pp_PLOT3D_REDUCEMENUS // eliminate plot3d sub-menus
//#define pp_MERGE_GEOMS          // merge geometry and structure boundary file menu items

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
#endif

//*** options: for debugging

#ifdef _DEBUG
#define pp_MOVIE_BATCH_DEBUG // allow movei batch dialogs to be defined for testing
#define pp_SNIFF_ERROR
#endif
#define pp_RENDER360_DEBUG

#endif

