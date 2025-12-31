/// @file command_args.h
#ifndef COMMAND_ARGS_H_DEFINED
#define COMMAND_ARGS_H_DEFINED

#include <stdbool.h>
#include "options.h"

/// @brief The parsed version of commandline arguments.
typedef struct CommandlineArgs {
  /// @brief output smokeview parameter values to smokeview.ini
  bool ini;
  /// @brief non-graphics version of -ini
  bool ng_ini;
  /// @brief generate images of volume rendered smoke and fire
  bool volrender;
  /// @brief Either -version or -v was set, so print the version of smokeview
  /// and exit.
  bool print_version;
  bool update_bounds;
  bool large_case;
  bool load_co2;
  bool load_hrrpuv;
  bool load_soot;
  bool load_temp;
  bool encode_png;
  bool encode_png_commandline;
  bool no_graphics;
  /// @brief calculate slice file parameters
  bool update_slice;
  bool nogpu;
  /// @brief use demonstrator mode of Smokeview"
  bool demo;
  /// @brief generate casename.slcf and casename.viewpoint files containing
  /// slice file and viewpiont info
  bool info;
  /// @brief output files sizes then exit
  bool sizes;
  /// @brief activate stereo mode
  bool stereo;
  /// @brief hide scene and data when moving scene or selecting menus
  bool big;
#ifdef pp_OSX
  /// @brief On Mac, turn off 2x scene scaling (do not scale scene)
  bool x1;
  /// @brief On Mac, turn on 2x scene scaling
  bool x2;
#endif
  /// @brief show frame rates
  bool trirates;
  /// @brief show timings
  bool timings;
  bool convert_ini;
  bool convert_ssf;
  /// @brief update case.ini to the current format
  bool update_ini;
  bool update_ssf;
  bool isotest;
  /// @brief only show 3D smoke
  bool smoke3d;
  bool no_slcf;
  bool show_help_summary;
  bool show_help_all;
  bool nobounds;
  bool noblank;
  /// @brief check conversion between CIELabl and rgb color coordinates
  bool check_colorbar;
  /// @brief pre-calculate all FED slice files
  bool verbose;
  /// @brief show geometry bound boxes instead of geometry
  bool outline;
  /// @brief open the movie generating dialog box
  bool make_movie;
  /// @brief specify maximum memory used
  bool max_mem;
  bool csv;
  float max_mem_GB;
  /// @brief output information about geometry triangles
  bool geominfo;
  /// @brief assume slice files exist in order to reduce startup time
  bool fast;
  bool full;
  bool blank;
  bool gversion;
  bool redirect;
  /// @brief Run the default SSF script, i.e. CHID.ssf
  bool runscript;
  bool checkscript;
  bool runhtmlscript;
  /// @brief Run the SSF script at this path
  char *script;
  /// @brief Run the HTML script at this path
  char *htmlscript;
  /// @brief set directory containing script rendered images (override directory
  /// specified by RENDERDIR script keyword)
  char *scriptrenderdir;
  // TODO: consider sentinel values
  bool skipframe_defined;
  /// @brief render every n frames, only valid if skipframe_defined is true
  int skipframe;
  bool startframe_defined;
  /// @brief start rendering at frame n, only valid if startframe_defined is
  /// true
  int startframe;
  bool noexit;
  /// @brief only show geometry
  bool setup;
  bool threads_defined;
  /// @brief equivalent to -update_bounds and -update_slice
  bool update;
  int threads;
  char *prog;
  char *input_file;
  char *bindir;
  char *casedir;
  /// @brief language code e.g., de, es, fr, for German, Spanish, French or
  /// Italian
  char *lang;
  char *ini_from;
  char *ini_to;
  char *ssf_from;
  char *ssf_to;
  /// @brief initial screen coordinates
  bool have_x0, have_y0;
  int x0, y0;
  bool have_X0, have_Y0;
  int X0, Y0;
} CommandlineArgs;

enum CommandLineError {
  CLE_MULTIPLE_LANG,
  CLE_NO_LANG,
  CLE_NO_SSF_ARGS,
  CLE_NO_INI_ARGS,
  CLE_MULTIPLE_INPUTS,
  CLE_OK,
  CLE_ARGUMENT_EXPECTED,
  CLE_UNKNOWN_ARGUMENT
};
const char *CLE_Message(enum CommandLineError cle, char *message);
CommandlineArgs ParseCommandlineNew(int argc, char **argv, char *message,
                                    enum CommandLineError *error);
#endif
