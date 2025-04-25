#ifdef pp_LUA

#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include "options.h"

#include "smokeviewvars.h"

#include "infoheader.h"

#include "c_api.h"
#include "lua_api.h"

#include GLUT_H
#include "gd.h"

#if defined(_WIN32)
#include <direct.h>
#endif

// NOLINTNEXTLINE
lua_State *lua_instance;
int LuaDisplayCb(lua_State *L);

#ifdef WIN32
#define snprintf _snprintf
#else
#include <unistd.h>
#endif

char *ParseCommandline(int argc, char **argv);

int CheckSMVFileLua(char *file, char *subdir) {
  char casedir[256], *casedirptr, casename[256];
  FILE *stream;

  if(file == NULL) return 1;

  strcpy(casename, file);
  if(subdir == NULL) {
    casedirptr = casedir;
    strcpy(casedir, casename);
  }
  else {
    casedirptr = subdir;
  }
  stream = fopen(casename, "r");
  if(stream == NULL) {
    stream = fopen_indir(casedirptr, casename, "r");
    if(stream == NULL) {
      printf("***error: unable to open %s\n", casename);
      return 0;
    }
    CHDIR(casedirptr);
  }
  fclose(stream);
  return 1;
}

int ProgramSetupLua(lua_State *L, int argc, char **argv) {
  InitVars();
  ParseCommonOptions(argc, argv);
  smv_filename = ParseCommandline(argc, argv);
  printf("smv_filename: %s\n", smv_filename);

  if(show_version == 1 || smv_filename == NULL) {
    DisplayVersionInfo("Smokeview ");
    SMV_EXIT(0);
  }
  if(CheckSMVFileLua(smv_filename, smokeview_casedir) == 0) {
    SMV_EXIT(1);
  }
  InitTextureDir();
  InitScriptErrorFiles();
  char *smv_bindir = GetSmvRootDir();
  smokezippath = GetSmokeZipPath(smv_bindir);
#ifdef WIN32
  have_ffmpeg = HaveProg("ffmpeg -version> Nul 2>Nul");
  have_ffplay = HaveProg("ffplay -version> Nul 2>Nul");
#else
  have_ffmpeg = HaveProg("ffmpeg -version >/dev/null 2>/dev/null");
  have_ffplay = HaveProg("ffplay -version >/dev/null 2>/dev/null");
#endif
  DisplayVersionInfo("Smokeview ");

  FREEMEMORY(smv_bindir)
  return 0;
}

/// @brief We can only take strings from the Lua interpreter as consts, as they
/// are 'owned' by the Lua code and we should not change them in C. This
/// function creates a copy that we can change in C.
char **CopyArgv(const int argc, const char *const *argv_sv) {
  char **argv_sv_non_const;
  // Allocate pointers for list of smokeview arguments
  NewMemory((void **)&argv_sv_non_const, argc * sizeof(char *));
  // Allocate space for each smokeview argument
  int i;
  for(i = 0; i < argc; i++) {
    int length = strlen(argv_sv[i]);
    NewMemory((void **)&argv_sv_non_const[i], (length + 1) * sizeof(char));
    strcpy(argv_sv_non_const[i], argv_sv[i]);
  }
  return argv_sv_non_const;
}

/// @brief The corresponding function to free the memory allocated by copy_argv.
void FreeArgv(const int argc, char **argv_sv_non_const) {
  // Free the memory allocated for each argument
  int i;
  for(i = 0; i < argc; i++) {
    FREEMEMORY(argv_sv_non_const[i]);
  }
  // Free the memory allocated for the array
  FREEMEMORY(argv_sv_non_const);
}

int LuaSetupGlut(lua_State *L) {
  int argc = lua_tonumber(L, 1);
  const char *const *argv_sv = lua_topointer(L, 2);
  // Here we must copy the arguments received from the Lua interperter to
  // allow them to be non-const (i.e. let the C code modify them).
  char **argv_sv_non_const = CopyArgv(argc, argv_sv);
  InitStartupDirs();
  SetupGlut(argc, argv_sv_non_const);
  FreeArgv(argc, argv_sv_non_const);
  return 0;
}

int LuaSetupCase(lua_State *L) {
  const char *filename = lua_tostring(L, -1);
  char *filename_mut;
  // Allocate some new memory in case smv tries to modify it.
  int f_len = strlen(filename);
  if(NewMemory((void **)&filename_mut, sizeof(char) * f_len + 1) == 0) return 2;
  strncpy(filename_mut, filename, f_len);
  filename_mut[f_len] = '\0';
  int return_code = SetupCase(filename_mut);
  lua_pushnumber(L, return_code);
  FREEMEMORY(filename_mut);
  return 1;
}

int RunLuaBranch(lua_State *L, int argc, char **argv) {
  int return_code;
  SetStdOut(stdout);
  initMALLOC();
  InitRandAB(1000000);
  // Setup the program, including parsing commandline arguments. Does not
  // initialise any graphical components.
  ProgramSetupLua(L, argc, argv);
  // From here on out, control is passed to the lua interpreter. All further
  // setup, including graphical display setup, is handled (or at least
  // triggered) by the interpreter.
  // TODO: currently the commands are issued here via C, but they are designed
  // such that they can be issued from lua.
  // Setup glut. TODO: this is currently done via to C because the commandline
  // arguments are required for glutInit.

  lua_pushnumber(L, argc);
  lua_pushlightuserdata(L, argv);
  LuaSetupGlut(L);
  START_TIMER(startup_time);
  // Load information about smokeview into the lua interpreter.
  LuaInitsmvproginfo(L);

  if(smv_filename == NULL) {
    return 0;
  }
  lua_pushstring(L, smv_filename);
  // TODO: only set up a case if one is specified, otherwise leave it to the
  // interpreter to call this.
  LuaSetupCase(L);
  return_code = lua_tonumber(L, -1);
#ifdef pp_HIST
  if(return_code == 0 && update_bounds == 1) {
    INIT_PRINT_TIMER(timer_update_bounds);
    return_code = Update_Bounds();
    PRINT_TIMER(timer_update_bounds, "Update_Bounds");
  }
#endif
  if(return_code != 0) return 1;
  if(convert_ini == 1) {
    INIT_PRINT_TIMER(timer_read_ini);
    ReadIni(ini_from);
    PRINT_TIMER(timer_read_ini, "ReadIni");
  }
  if(runhtmlscript == 1) {
    DoScriptHtml();
  }
  STOP_TIMER(startup_time);
  if(runhtmlscript == 1) {
    return 0;
  }

  glutMainLoop();
  return 0;
}

/// @brief Run a script.
/// @details There are two options for scripting, Lua and SSF. Which is run is
/// set here based on the commandline arguments. If either (exclusive) of these
/// values are set to true, then that script will run from within the display
/// callback (DisplayCB, in callbacks.c). These two loading routines are
/// included to load the scripts early in the piece, before the display
/// callback. Both runluascript and runscript are global.
int LoadScript(const char *filename) {
  if(runluascript == 1 && runscript == 1) {
    fprintf(stderr, "Both a Lua script and an SSF script cannot be run "
                    "simultaneously\n");
    exit(1);
  }
  if(runluascript == 1) {
    // Load the Lua script in order for it to be run later.
    if(LoadLuaScript(filename) != LUA_OK) {
      fprintf(stderr, "There was an error loading the script, and so it "
                      "will not run.\n");
      if(exit_on_script_crash) {
        exit(1); // exit with an error code
      }
      runluascript = 0; // set this to false so that the smokeview no longer
                        // tries to run the script as it failed to load
      fprintf(stderr, "Running smokeview normally.\n");
    }
    else {
      fprintf(stderr, "%s successfully loaded\n", filename);
    }
  }
#ifdef pp_LUA_SSF
  if(runscript == 1) {
    // Load the ssf script in order for it to be run later
    // This still uses the Lua interpreter
    if(loadSSFScript(filename) != LUA_OK) {
      fprintf(stderr, "There was an error loading the script, and so it "
                      "will not run.\n");
      if(exit_on_script_crash) {
        exit(1); // exit with an error code
      }
      runluascript = 0; // set this to false so that the smokeview no longer
                        // tries to run the script as it failed to load
      fprintf(stderr, "Running smokeview normally.\n");
    }
  }
#endif
  return 1;
}

/// @brief Load a .smv file. This is currently not used as it is dependent on
/// Smokeview being able to run without a .smv file loaded.
int LuaLoadsmvall(lua_State *L) {
  // The first argument is taken from the stack as a string.
  const char *filepath = lua_tostring(L, 1);
  // The function from the C api is called using this string.
  Loadsmvall(filepath);
  // 0 arguments are returned.
  return 0;
}

/// @brief Set render clipping.
int LuaRenderclip(lua_State *L) {
  int flag = lua_toboolean(L, 1);
  int left = lua_tonumber(L, 2);
  int right = lua_tonumber(L, 3);
  int bottom = lua_tonumber(L, 4);
  int top = lua_tonumber(L, 5);
  Renderclip(flag, left, right, bottom, top);
  return 0;
}

/// @brief Render the current frame to a file.
int LuaRender(lua_State *L) {
  LuaDisplayCb(L);
  const char *basename = lua_tostring(L, 1);
  int ret = CApiRender(basename);
  lua_pushnumber(L, ret);
  return 1;
}

/// @brief Returns an error code then the image data.
int LuaRenderVar(lua_State *L) {
  gdImagePtr rende_rimage;
  int return_code;
  char *image_data;
  int image_size;

  // render image to RENDERimage gd buffer
  return_code = RenderFrameLuaVar(VIEW_CENTER, &rende_rimage);
  lua_pushnumber(L, return_code);
  // convert to a simpler byte-buffer
  image_data = gdImagePngPtr(rende_rimage, &image_size);
  // push to stack
  lua_pushlstring(L, image_data, image_size);
  // destroy C copy
  gdImageDestroy(rende_rimage);

  return 2;
}

int LuaGsliceview(lua_State *L) {
  int data = lua_tonumber(L, 1);
  int show_triangles = lua_toboolean(L, 2);
  int show_triangulation = lua_toboolean(L, 3);
  int show_normal = lua_toboolean(L, 4);
  Gsliceview(data, show_triangles, show_triangulation, show_normal);
  return 0;
}

int LuaShowplot3ddata(lua_State *L) {
  int meshnumber = lua_tonumber(L, 1);
  int plane_orientation = lua_tonumber(L, 2);
  int display = lua_tonumber(L, 3);
  int showhide = lua_toboolean(L, 4);
  float position = lua_tonumber(L, 5);
  ShowPlot3dData(meshnumber, plane_orientation, display, showhide, position, 0);
  return 0;
}

int LuaGslicepos(lua_State *L) {
  float x = lua_tonumber(L, 1);
  float y = lua_tonumber(L, 2);
  float z = lua_tonumber(L, 3);
  Gslicepos(x, y, z);
  return 0;
}
int LuaGsliceorien(lua_State *L) {
  float az = lua_tonumber(L, 1);
  float elev = lua_tonumber(L, 2);
  Gsliceorien(az, elev);
  return 0;
}

int LuaSettourview(lua_State *L) {
  int edittour_arg = lua_tonumber(L, 1);
  int mode = lua_tonumber(L, 2);
  int show_tourlocus_arg = lua_toboolean(L, 3);
  float tour_global_tension_arg = lua_tonumber(L, 4);
  Settourview(edittour_arg, mode, show_tourlocus_arg, tour_global_tension_arg);
  return 0;
}

int LuaSettourkeyframe(lua_State *L) {
  float keyframe_time = lua_tonumber(L, 1);
  Settourkeyframe(keyframe_time);
  return 0;
}

/// @brief Trigger the display callback.
int LuaDisplayCb(lua_State *L) {
  // runluascript=0;
  DisplayCB();
  // runluascript=1;
  return 0;
}

/// @brief Hide the smokeview window. This should not currently be used as it
/// prevents the display callback being called, and therefore the script will
/// not continue (the script is called as part of the display callback).
int LuaHidewindow(lua_State *L) {
  glutHideWindow();
  // once we hide the window the display callback is never called
  return 0;
}

/// @brief By calling yieldscript, the script is suspended and the smokeview
/// display is updated. It is necessary to call this before producing any
/// outputs (such as renderings).
int LuaYieldscript(lua_State *L) {
  lua_yield(L, 0 /*zero results*/);
  return 0;
}

/// @brief As with lua_yieldscript, but immediately resumes the script after
/// letting the display callback run.
int LuaTempyieldscript(lua_State *L) {
  runluascript = 1;
  lua_yield(L, 0 /*zero results*/);
  return 0;
}

/// @brief Return the current frame number which Smokeivew has loaded.
int LuaGetframe(lua_State *L) {
  int framenumber = Getframe();
  // Push a return value to the Lua stack.
  lua_pushinteger(L, framenumber);
  // Tell Lua that there is a single return value left on the stack.
  return 1;
}

/// @brief Shift to a specific frame number.
int LuaSetframe(lua_State *L) {
  int f = lua_tonumber(L, 1);
  Setframe(f);
  return 0;
}

/// @brief Get the time value of the currently loaded frame.
int LuaGettime(lua_State *L) {
  if(global_times != NULL && nglobal_times > 0) {
    float time = Gettime();
    lua_pushnumber(L, time);
    return 1;
  }
  else {
    return 0;
  }
}

/// @brief Shift to the closest frame to given a time value.
int LuaSettime(lua_State *L) {
  LuaDisplayCb(L);
  float t = lua_tonumber(L, 1);
  int return_code = Settime(t);
  lua_pushnumber(L, return_code);
  return 1;
}

/// @brief Load an FDS data file directly (i.e. as a filepath).
int LuaLoaddatafile(lua_State *L) {
  const char *filename = lua_tostring(L, 1);
  int return_value = Loadfile(filename);
  lua_pushnumber(L, return_value);
  return 1;
}

/// @brief Load a Smokeview config (.ini) file.
int LuaLoadinifile(lua_State *L) {
  const char *filename = lua_tostring(L, 1);
  Loadinifile(filename);
  return 0;
}

/// @brief Load an FDS vector data file directly (i.e. as a filepath). This
/// function handles the loading of any additional data files necessary to
/// display vectors.
int LuaLoadvdatafile(lua_State *L) {
  const char *filename = lua_tostring(L, 1);
  int return_value = Loadvfile(filename);
  lua_pushnumber(L, return_value);
  return 1;
}

/// @brief Load an FDS boundary file directly (i.e. as a filepath). This is
/// equivalent to lua_loadfile, but specialised for boundary files. This is
/// included to reflect the underlying code.
int LuaLoadboundaryfile(lua_State *L) {
  const char *filename = lua_tostring(L, 1);
  Loadboundaryfile(filename);
  return 0;
}

/// @brief Load a slice file given the type of slice, the axis along which it
/// exists and its position along this axis.
int LuaLoadslice(lua_State *L) {
  const char *type = lua_tostring(L, 1);
  int axis = lua_tonumber(L, 2);
  float distance = lua_tonumber(L, 3);
  Loadslice(type, axis, distance);
  return 0;
}

/// @brief Load a slice based on its index in slicecoll.sliceinfo.
int LuaLoadsliceindex(lua_State *L) {
  size_t index = lua_tonumber(L, 1);
  int error = 0;
  Loadsliceindex(index, &error);
  if(error) {
    return luaL_error(L, "Could not load slice at index %zu", index);
  }
  return 0;
}

int LuaGetClippingMode(lua_State *L) {
  lua_pushnumber(L, GetClippingMode());
  return 1;
}

/// @brief Set the clipping mode, which determines which parts of the model are
/// clipped (based on the set clipping values). This function takes an int,
/// which is one
///  of:
///    0: No clipping.
///    1: Clip blockages and data.
///    2: Clip blockages.
///    3: Clip data.
int LuaSetClippingMode(lua_State *L) {
  int mode = lua_tonumber(L, 1);
  SetClippingMode(mode);
  return 0;
}

int LuaSetSceneclipX(lua_State *L) {
  int clip_min = lua_toboolean(L, 1);
  float min = lua_tonumber(L, 2);
  int clip_max = lua_toboolean(L, 3);
  float max = lua_tonumber(L, 4);
  SetSceneclipX(clip_min, min, clip_max, max);
  return 0;
}

int LuaSetSceneclipXMin(lua_State *L) {
  int flag = lua_toboolean(L, 1);
  float value = lua_tonumber(L, 2);
  SetSceneclipXMin(flag, value);
  return 0;
}

int LuaSetSceneclipXMax(lua_State *L) {
  int flag = lua_toboolean(L, 1);
  float value = lua_tonumber(L, 2);
  SetSceneclipXMax(flag, value);
  return 0;
}

int LuaSetSceneclipY(lua_State *L) {
  int clip_min = lua_toboolean(L, 1);
  float min = lua_tonumber(L, 2);
  int clip_max = lua_toboolean(L, 3);
  float max = lua_tonumber(L, 4);
  SetSceneclipY(clip_min, min, clip_max, max);
  return 0;
}

int LuaSetSceneclipYMin(lua_State *L) {
  int flag = lua_toboolean(L, 1);
  float value = lua_tonumber(L, 2);
  SetSceneclipYMin(flag, value);
  return 0;
}

int LuaSetSceneclipYMax(lua_State *L) {
  int flag = lua_toboolean(L, 1);
  float value = lua_tonumber(L, 2);
  SetSceneclipYMax(flag, value);
  return 0;
}

int LuaSetSceneclipZ(lua_State *L) {
  int clip_min = lua_toboolean(L, 1);
  float min = lua_tonumber(L, 2);
  int clip_max = lua_toboolean(L, 3);
  float max = lua_tonumber(L, 4);
  SetSceneclipZ(clip_min, min, clip_max, max);
  return 0;
}

int LuaSetSceneclipZMin(lua_State *L) {
  int flag = lua_toboolean(L, 1);
  float value = lua_tonumber(L, 2);
  SetSceneclipZMin(flag, value);
  return 0;
}

int LuaSetSceneclipZMax(lua_State *L) {
  int flag = lua_toboolean(L, 1);
  float value = lua_tonumber(L, 2);
  SetSceneclipZMax(flag, value);
  return 0;
}

/// @brief Return a table (an array) of the times available in Smokeview. They
/// key of the table is an int representing the frame number, and the value of
/// the table is a float representing the time.
/// @param L The lua interpreter
/// @return Number of stack items left on stack.
int LuaGetGlobalTimes(lua_State *L) {
  lua_createtable(L, 0, nglobal_times);
  int i;
  for(i = 0; i < nglobal_times; i++) {
    lua_pushnumber(L, i);
    lua_pushnumber(L, global_times[i]);
    lua_settable(L, -3);
  }
  return 1;
}

/// @brief Given a frame number return the time.
/// @param L The lua interpreter
/// @return Number of stack items left on stack.
int LuaGetGlobalTime(lua_State *L) {
  int frame_number = lua_tonumber(L, 1);
  if(frame_number >= 0 && frame_number < nglobal_times) {
    lua_pushnumber(L, global_times[frame_number]);
  }
  else {
    lua_pushnil(L);
  }
  return 1;
}

/// @brief Get the number of (global) frames available to smokeview.
/// @param L
/// @return
int LuaGetNglobalTimes(lua_State *L) {
  lua_pushnumber(L, nglobal_times);
  return 1;
}

/// @brief Get the number of meshes in the loaded model.
int LuaGetNmeshes(lua_State *L) {
  lua_pushnumber(L, nmeshes);
  return 1;
}

/// @brief Get the number of particle files in the loaded model.
int LuaGetNpartinfo(lua_State *L) {
  lua_pushnumber(L, npartinfo);
  return 1;
}

int LuaGetiblankcell(lua_State *L) {
  // The offset in the global meshinfo table.
  int mesh_index = lua_tonumber(L, lua_upvalueindex(1));
  // The offsets into the mesh requested
  int i = lua_tonumber(L, 1);
  int j = lua_tonumber(L, 2);
  int k = lua_tonumber(L, 3);

  meshdata *mesh = &meshinfo[mesh_index];
  char iblank =
      mesh->c_iblank_cell[(i) + (j)*mesh->ibar + (k)*mesh->ibar * mesh->jbar];
  if(iblank == GAS) {
    lua_pushboolean(L, 1);
  }
  else {
    lua_pushboolean(L, 0);
  }
  return 1;
}

int LuaGetiblanknode(lua_State *L) {
  // The offset in the global meshinfo table.
  int mesh_index = lua_tonumber(L, lua_upvalueindex(1));
  // The offsets into the mesh requested.
  int i = lua_tonumber(L, 1);
  int j = lua_tonumber(L, 2);
  int k = lua_tonumber(L, 3);

  meshdata *mesh = &meshinfo[mesh_index];
  char iblank = mesh->c_iblank_node[(i) + (j) * (mesh->ibar + 1) +
                                    (k) * (mesh->ibar + 1) * (mesh->jbar + 1)];
  if(iblank == GAS) {
    lua_pushboolean(L, 1);
  }
  else {
    lua_pushboolean(L, 0);
  }
  return 1;
}

/// @brief Build a Lua table with information on the meshes of the model. The
/// key of the table is the mesh number.
// TODO: provide more information via this interface.
int LuaGetMeshes(lua_State *L) {
  int entries = nmeshes;
  meshdata *infotable = meshinfo;
  lua_createtable(L, 0, entries);
  int i;
  for(i = 0; i < entries; i++) {
    lua_pushnumber(L, i);
    lua_createtable(L, 0, 5);

    lua_pushnumber(L, infotable[i].ibar);
    lua_setfield(L, -2, "ibar");

    lua_pushnumber(L, infotable[i].jbar);
    lua_setfield(L, -2, "jbar");

    lua_pushnumber(L, infotable[i].kbar);
    lua_setfield(L, -2, "kbar");

    lua_pushstring(L, infotable[i].label);
    lua_setfield(L, -2, "label");

    lua_pushnumber(L, infotable[i].kbar);
    lua_setfield(L, -2, "cellsize");

    lua_pushnumber(L, xbar0);
    lua_setfield(L, -2, "xbar0");

    lua_pushnumber(L, ybar0);
    lua_setfield(L, -2, "ybar0");

    lua_pushnumber(L, zbar0);
    lua_setfield(L, -2, "zbar0");

    lua_pushnumber(L, xyzmaxdiff);
    lua_setfield(L, -2, "xyzmaxdiff");

    lua_pushnumber(L, i);
    lua_pushcclosure(L, LuaGetiblankcell, 1);
    lua_setfield(L, -2, "iblank_cell");

    lua_pushnumber(L, i);
    lua_pushcclosure(L, LuaGetiblanknode, 1);
    lua_setfield(L, -2, "iblank_node");

    // loop for less than ibar
    int j;
    lua_createtable(L, 0, infotable[i].ibar);
    for(j = 0; j < infotable[i].ibar; j++) {
      lua_pushnumber(L, j);
      lua_pushnumber(L, infotable[i].xplt[j]);
      lua_settable(L, -3);
    }
    lua_setfield(L, -2, "xplt");

    lua_createtable(L, 0, infotable[i].jbar);
    for(j = 0; j < infotable[i].jbar; j++) {
      lua_pushnumber(L, j);
      lua_pushnumber(L, infotable[i].yplt[j]);
      lua_settable(L, -3);
    }
    lua_setfield(L, -2, "yplt");

    lua_createtable(L, 0, infotable[i].kbar);
    for(j = 0; j < infotable[i].kbar; j++) {
      lua_pushnumber(L, j);
      lua_pushnumber(L, infotable[i].zplt[j]);
      lua_settable(L, -3);
    }
    lua_setfield(L, -2, "zplt");

    lua_createtable(L, 0, infotable[i].ibar);
    for(j = 0; j < infotable[i].ibar; j++) {
      lua_pushnumber(L, j);
      lua_pushnumber(L, infotable[i].xplt_orig[j]);
      lua_settable(L, -3);
    }
    lua_setfield(L, -2, "xplt_orig");

    lua_createtable(L, 0, infotable[i].jbar);
    for(j = 0; j < infotable[i].jbar; j++) {
      lua_pushnumber(L, j);
      lua_pushnumber(L, infotable[i].yplt_orig[j]);
      lua_settable(L, -3);
    }
    lua_setfield(L, -2, "yplt_orig");

    lua_createtable(L, 0, infotable[i].kbar);
    for(j = 0; j < infotable[i].kbar; j++) {
      lua_pushnumber(L, j);
      lua_pushnumber(L, infotable[i].zplt_orig[j]);
      lua_settable(L, -3);
    }
    lua_setfield(L, -2, "zplt_orig");

    lua_settable(L, -3);
  }
  // Leaves one returned value on the stack, the mesh table.
  return 1;
}

/// @brief Get the number of meshes in the loaded model.
int LuaGetNdevices(lua_State *L) {
  lua_pushnumber(L, ndeviceinfo);
  return 1;
}

/// @brief Build a Lua table with information on the devices of the model.
int LuaGetDevices(lua_State *L) {
  int entries = ndeviceinfo;
  devicedata *infotable = deviceinfo;
  lua_createtable(L, 0, entries);
  int i;
  for(i = 0; i < entries; i++) {
    lua_pushstring(L, infotable[i].deviceID);
    lua_createtable(L, 0, 2);

    lua_pushstring(L, infotable[i].deviceID);
    lua_setfield(L, -2, "label");

    lua_settable(L, -3);
  }
  return 1;
}

int LuaCreateVector(lua_State *L, csvdata *csv_x, csvdata *csv_y) {
  size_t i;
  lua_createtable(L, 0, 3);

  lua_pushstring(L, csv_y->label.longlabel);
  lua_setfield(L, -2, "name");

  // x-vector
  lua_createtable(L, 0, 3);
  lua_pushstring(L, csv_x->label.longlabel);
  lua_setfield(L, -2, "name");
  lua_pushstring(L, csv_x->label.unit);
  lua_setfield(L, -2, "units");
  lua_createtable(L, 0, csv_x->nvals);
  for(i = 0; i < csv_x->nvals; ++i) {
    lua_pushnumber(L, i + 1);
    lua_pushnumber(L, csv_x->vals[i]);
    lua_settable(L, -3);
  }
  lua_setfield(L, -2, "values");
  lua_setfield(L, -2, "x");
  // y-vector
  lua_createtable(L, 0, 3);
  lua_pushstring(L, csv_y->label.longlabel);
  lua_setfield(L, -2, "name");
  lua_pushstring(L, csv_y->label.unit);
  lua_setfield(L, -2, "units");
  lua_createtable(L, 0, csv_y->nvals);
  for(i = 0; i < csv_y->nvals; ++i) {
    lua_pushnumber(L, i + 1);
    lua_pushnumber(L, csv_y->vals[i]);
    lua_settable(L, -3);
  }
  lua_setfield(L, -2, "values");
  lua_setfield(L, -2, "y");
  return 1;
}

/// @brief Get the number of CSV files available to the model.
int LuaGetNcsvinfo(lua_State *L) {
  lua_pushnumber(L, ncsvfileinfo);
  return 1;
}

csvfiledata *GetCsvinfo(const char *key) {
  // Loop through csvinfo until we find the right entry
  size_t i;
  for(i = 0; i < ncsvfileinfo; ++i) {
    if(strcmp(csvfileinfo[i].c_type, key) == 0) {
      return &csvfileinfo[i];
    }
  }
  return NULL;
}

int GetCsvindex(const char *key) {
  // Loop through csvinfo until we find the right entry
  size_t i;
  for(i = 0; i < ncsvfileinfo; ++i) {
    if(strcmp(csvfileinfo[i].c_type, key) == 0) {
      return i;
    }
  }
  return -1;
}

void LoadCsv(csvfiledata *csventry) {
// add global_scase to first argument of ReadCSVFile
  ReadCSVFile(NULL, csventry, LOAD);
  csventry->loaded = 1;
}

int LuaLoadCsv(lua_State *L) {
  lua_pushstring(L, "c_type");
  lua_gettable(L, 1);
  const char *key = lua_tostring(L, -1);
  csvfiledata *csventry = GetCsvinfo(key);
  if(csventry == NULL) return 0;
  LoadCsv(csventry);
  return 0;
}

int AccessCsventryProp(lua_State *L) {
  // Take the index from the table.
  lua_pushstring(L, "index");
  lua_gettable(L, 1);
  int index = lua_tonumber(L, -1);
  const char *field = lua_tostring(L, 2);
  if(strcmp(field, "loaded") == 0) {
    lua_pushboolean(L, csvfileinfo[index].loaded);
    return 1;
  }
  else if(strcmp(field, "display") == 0) {
    lua_pushboolean(L, csvfileinfo[index].display);
    return 1;
  }
  else if(strcmp(field, "vectors") == 0) {
    csvfiledata *csventry = &csvfileinfo[index];
    if(!csventry->loaded) {
      LoadCsv(csventry);
    }
    // TODO: don't create every time
    lua_createtable(L, 0, csventry->ncsvinfo);
    size_t j;
    for(j = 0; j < csventry->ncsvinfo; j++) {
      // Load vector data into lua.
      // TODO: change to access indirectly rater than copying via stack
      // printf("adding: %s\n", csventry->vectors[j].y->name);

      LuaCreateVector(L, csventry->time, &(csventry->csvinfo[j]));
      lua_setfield(L, -2, csventry->csvinfo[j].label.longlabel);
    }
    return 1;
  }
  else {
    return 0;
  }
}

int LuaCsvIsLoaded(lua_State *L) {
  const char *key = lua_tostring(L, lua_upvalueindex(1));
  csvfiledata *csventry = GetCsvinfo(key);
  lua_pushboolean(L, csventry->loaded);
  return 1;
}

/// @brief Create a table so that a metatable can be used.
int LuaGetCsvdata(lua_State *L) {
  // L1 is the table
  // L2 is the string key
  const char *key = lua_tostring(L, 2);
  // char *file = lua_tostring(L, 1);
  csvfiledata *csventry = GetCsvinfo(key);
  // Check if the chosen csv data is loaded
  if(!csventry->loaded) {
    // Load the data.
    LoadCsv(csventry);
  }
  // TODO: put userdata on stack
  lua_pushlightuserdata(L, csventry->csvinfo);
  return 1;
}

int AccessPl3dentryProp(lua_State *L) {
  // Take the index from the table.
  lua_pushstring(L, "index");
  lua_gettable(L, 1);
  int index = lua_tonumber(L, -1);
  const char *field = lua_tostring(L, 2);
  if(strcmp(field, "loaded") == 0) {
    lua_pushboolean(L, plot3dinfo[index].loaded);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaSetPl3dBoundMin(lua_State *L) {
  int pl3d_value_index = lua_tonumber(L, 1);
  int set = lua_toboolean(L, 2);
  float value = lua_tonumber(L, 3);
  SetPl3dBoundMin(pl3d_value_index, set, value);
  return 0;
}

int LuaSetPl3dBoundMax(lua_State *L) {
  int pl3d_value_index = lua_tonumber(L, 1);
  int set = lua_toboolean(L, 2);
  float value = lua_tonumber(L, 3);
  SetPl3dBoundMax(pl3d_value_index, set, value);
  return 0;
}

/// @brief Get the number of PL3D files available to the model.
int LuaGetNplot3dinfo(lua_State *L) {
  lua_pushnumber(L, nplot3dinfo);
  return 1;
}

int LuaGetPlot3dentry(lua_State *L) {
  int lua_index = lua_tonumber(L, -1);
  int index = lua_index - 1;
  int i;

  // csvdata *csventry = get_csvinfo(key);
  // fprintf(stderr, "csventry->file: %s\n", csventry->file);
  lua_createtable(L, 0, 4);

  lua_pushnumber(L, index);
  lua_setfield(L, -2, "index");

  lua_pushstring(L, plot3dinfo[index].file);
  lua_setfield(L, -2, "file");

  lua_pushstring(L, plot3dinfo[index].reg_file);
  lua_setfield(L, -2, "reg_file");

  lua_pushstring(L, plot3dinfo[index].longlabel);
  lua_setfield(L, -2, "longlabel");

  lua_pushnumber(L, plot3dinfo[index].time);
  lua_setfield(L, -2, "time");

  lua_pushnumber(L, plot3dinfo[index].u);
  lua_setfield(L, -2, "u");
  lua_pushnumber(L, plot3dinfo[index].v);
  lua_setfield(L, -2, "v");
  lua_pushnumber(L, plot3dinfo[index].w);
  lua_setfield(L, -2, "w");

  lua_pushnumber(L, plot3dinfo[index].nplot3dvars);
  lua_setfield(L, -2, "nplot3dvars");

  lua_pushnumber(L, plot3dinfo[index].blocknumber);
  lua_setfield(L, -2, "blocknumber");

  lua_pushnumber(L, plot3dinfo[index].display);
  lua_setfield(L, -2, "display");

  lua_createtable(L, 0, 6);
  for(i = 0; i < 6; ++i) {
    lua_pushnumber(L, i + 1);

    lua_createtable(L, 0, 3);
    lua_pushstring(L, plot3dinfo[index].label[i].longlabel);
    lua_setfield(L, -2, "longlabel");
    lua_pushstring(L, plot3dinfo[index].label[i].shortlabel);
    lua_setfield(L, -2, "shortlabel");
    lua_pushstring(L, plot3dinfo[index].label[i].unit);
    lua_setfield(L, -2, "unit");

    lua_settable(L, -3);
  }
  lua_setfield(L, -2, "label");

  // Create a metatable.
  // TODO: this metatable might be more easily implemented directly in Lua
  // so that we don't need to reimplement table access.
  lua_createtable(L, 0, 1);
  lua_pushcfunction(L, &AccessPl3dentryProp);
  lua_setfield(L, -2, "__index");
  // then set the metatable
  lua_setmetatable(L, -2);

  return 1;
}

int LuaGetPlot3dinfo(lua_State *L) {
  lua_createtable(L, 0, nplot3dinfo);
  int i;
  for(i = 0; i < nplot3dinfo; i++) {
    lua_pushnumber(L, i + 1);
    LuaGetPlot3dentry(L);

    lua_settable(L, -3);
  }
  return 1;
}

int LuaGetQdataSum(lua_State *L) {
  int meshnumber = lua_tonumber(L, 1);
  int vari, i, j, k;
  meshdata mesh = meshinfo[meshnumber];
  int ntotal = (mesh.ibar + 1) * (mesh.jbar + 1) * (mesh.kbar + 1);
  int vars = 5;
  float totals[5];
  totals[0] = 0.0;
  totals[1] = 0.0;
  totals[2] = 0.0;
  totals[3] = 0.0;
  totals[4] = 0.0;
  for(vari = 0; vari < 5; ++vari) {
    int offset = vari * ntotal;
    for(k = 0; k <= mesh.kbar; ++k) {
      for(j = 0; j <= mesh.jbar; ++j) {
        for(i = 0; i <= mesh.ibar; ++i) {
          int n = offset + k * (mesh.jbar + 1) * (mesh.ibar + 1) +
                  j * (mesh.ibar + 1) + i;
          totals[vari] += mesh.qdata[n];
        }
      }
    }
  }
  for(vari = 0; vari < vars; ++vari) {
    lua_pushnumber(L, totals[vari]);
  }
  lua_pushnumber(L, ntotal);
  return vars + 1;
}

/// @brief Sum bounded data in a given mesh
int LuaGetQdataSumBounded(lua_State *L) {
  int meshnumber = lua_tonumber(L, 1);
  int vari, i, j, k;
  int i1, i2, j1, j2, k1, k2;
  i1 = lua_tonumber(L, 2);
  i2 = lua_tonumber(L, 3);
  j1 = lua_tonumber(L, 4);
  j2 = lua_tonumber(L, 5);
  k1 = lua_tonumber(L, 6);
  k2 = lua_tonumber(L, 7);
  meshdata mesh = meshinfo[meshnumber];
  int ntotal = (mesh.ibar + 1) * (mesh.jbar + 1) * (mesh.kbar + 1);
  int bounded_total = (i2 - i1 + 1) * (j2 - j1 + 1) * (k2 - k1 + 1);
  int vars = 5;
  float totals[5];
  totals[0] = 0.0;
  totals[1] = 0.0;
  totals[2] = 0.0;
  totals[3] = 0.0;
  totals[4] = 0.0;
  for(vari = 0; vari < 5; ++vari) {
    int offset = vari * ntotal;
    for(k = k1; k <= k2; ++k) {
      for(j = j1; j <= j2; ++j) {
        for(i = i1; i <= i2; ++i) {
          int n = offset + k * (mesh.jbar + 1) * (mesh.ibar + 1) +
                  j * (mesh.ibar + 1) + i;
          totals[vari] += mesh.qdata[n];
        }
      }
    }
  }

  for(vari = 0; vari < vars; ++vari) {
    lua_pushnumber(L, totals[vari]);
  }
  lua_pushnumber(L, bounded_total);
  return vars + 1;
}

/// @brief Sum bounded data in a given mesh
int LuaGetQdataMaxBounded(lua_State *L) {
  int meshnumber = lua_tonumber(L, 1);
  int vari, i, j, k;
  int i1, i2, j1, j2, k1, k2;
  i1 = lua_tonumber(L, 2);
  i2 = lua_tonumber(L, 3);
  j1 = lua_tonumber(L, 4);
  j2 = lua_tonumber(L, 5);
  k1 = lua_tonumber(L, 6);
  k2 = lua_tonumber(L, 7);
  meshdata mesh = meshinfo[meshnumber];
  int ntotal = (mesh.ibar + 1) * (mesh.jbar + 1) * (mesh.kbar + 1);
  int bounded_total = (i2 - i1 + 1) * (j2 - j1 + 1) * (k2 - k1 + 1);
  int vars = 5;
  float maxs[5];
  maxs[0] = -1 * FLT_MAX;
  maxs[1] = -1 * FLT_MAX;
  maxs[2] = -1 * FLT_MAX;
  maxs[3] = -1 * FLT_MAX;
  maxs[4] = -1 * FLT_MAX;
  for(vari = 0; vari < 5; ++vari) {
    int offset = vari * ntotal;
    for(k = k1; k <= k2; ++k) {
      for(j = j1; j <= j2; ++j) {
        for(i = i1; i <= i2; ++i) {
          int n = offset + k * (mesh.jbar + 1) * (mesh.ibar + 1) +
                  j * (mesh.ibar + 1) + i;
          if(maxs[vari] < mesh.qdata[n]) {
            maxs[vari] = mesh.qdata[n];
          }
        }
      }
    }
  }

  for(vari = 0; vari < vars; ++vari) {
    lua_pushnumber(L, maxs[vari]);
  }
  lua_pushnumber(L, bounded_total);
  return vars + 1;
}

int LuaGetQdataMean(lua_State *L) {
  int meshnumber = lua_tonumber(L, 1);
  int vari, i, j, k;
  meshdata mesh = meshinfo[meshnumber];
  int ntotal = (mesh.ibar + 1) * (mesh.jbar + 1) * (mesh.kbar + 1);
  int vars = 5;
  float totals[5];
  totals[0] = 0.0;
  totals[1] = 0.0;
  totals[2] = 0.0;
  totals[3] = 0.0;
  totals[4] = 0.0;
  for(vari = 0; vari < 5; ++vari) {
    int offset = vari * ntotal;
    for(k = 0; k <= mesh.kbar; ++k) {
      for(j = 0; j <= mesh.jbar; ++j) {
        for(i = 0; i <= mesh.ibar; ++i) {
          int n = offset + k * (mesh.jbar + 1) * (mesh.ibar + 1) +
                  j * (mesh.ibar + 1) + i;
          totals[vari] += mesh.qdata[n];
        }
      }
    }
  }
  for(vari = 0; vari < vars; ++vari) {
    lua_pushnumber(L, totals[vari] / ntotal);
  }
  return vars;
}

int LuaGetGlobalTimeN(lua_State *L) {
  // argument 1 is the table, argument 2 is the index
  int index = lua_tonumber(L, 2);
  if(index < 0 || index >= nglobal_times) {
    return luaL_error(L, "%d is not a valid global time index\n", index);
  }
  lua_pushnumber(L, global_times[index]);
  return 1;
}

// TODO: remove this from a hardcoded string.
int SetupPl3dtables(lua_State *L) {
  luaL_dostring(L, "\
    pl3d = {}\
    local allpl3dtimes = {}\
    for i,v in ipairs(plot3dinfo) do\
        if not allpl3dtimes[v.time] then allpl3dtimes[v.time] = {} end\
        assert(not allpl3dtimes[v.time][v.blocknumber+1])\
        allpl3dtimes[v.time][v.blocknumber+1] = v\
    end\
    local pl3dtimes = {}\
    for k,v in pairs(allpl3dtimes) do\
        pl3dtimes[#pl3dtimes+1] = {time = k, entries = v}\
    end\
    table.sort( pl3dtimes, function(a,b) return a.time < b.time end)\
    pl3d.entries = plot3dinfo\
    pl3d.frames = pl3dtimes");
  return 0;
}

int LuaCaseTitle(lua_State *L) {
  lua_pushstring(L, "chid");
  lua_gettable(L, 1);
  const char *chid = lua_tostring(L, -1);
  const char *name = lua_tostring(L, 2);
  lua_pushfstring(L, "%s for %s", name, chid);
  return 1;
}

int LuaCaseIndex(lua_State *L) {
  const char *field = lua_tostring(L, 2);
  if(strcmp(field, "chid") == 0) {
    lua_pushstring(L, chidfilebase);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaCaseNewindex(lua_State *L) {
  const char *field = lua_tostring(L, 2);
  if(strcmp(field, "chid") == 0) {
    luaL_error(L, "case.chid is read-only");
    // lua_pushstring(L, value);
    // lua_setrenderdir(L);
    return 0;
  }
  else {
    return 0;
  }
}

/// @brief Load data about the loaded module into the lua interpreter. This
/// initsmvdata is necessary to bring some data into the Lua interpreter from
/// the model. This is included here rather than doing in the Smokeview code to
/// increase separation. This will likely be removed in future versions.
// TODO: Consider converting most of these to userdata, rather than copying them
// into the lua interpreter.
int LuaCreateCase(lua_State *L) {
  // Create case table
  lua_newtable(L);
  // lua_pushstring(L, chidfilebase);
  // lua_setfield(L, -2, "chid");
  lua_pushstring(L, global_scase.fdsprefix);
  lua_setfield(L, -2, "global_scase.fdsprefix");

  lua_pushcfunction(L, &LuaCaseTitle);
  lua_setfield(L, -2, "plot_title");

  // TODO: copying the array into lua allows for slightly faster access,
  // but is less ergonomic, leave direct access as the default, with copying
  // in cases where it is shown to be a useful speedup
  // lua_get_global_times(L);
  // global_times is currently on the stack
  // add a metatable to it.
  // first create the table

  // Create "global_times" table
  lua_newtable(L);
  // Create "global_times" metatable
  lua_newtable(L);
  lua_pushcfunction(L, &LuaGetNglobalTimes);
  lua_setfield(L, -2, "__len");
  lua_pushcfunction(L, &LuaGetGlobalTimeN);
  lua_setfield(L, -2, "__index");
  // then set the metatable
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "global_times");

  // while the meshes themselve will rarely change, the information about them
  // will change regularly. This is handled by the mesh table.
  LuaGetMeshes(L);
  // meshes is currently on the stack
  // add a metatable to it.
  // first create the table
  lua_createtable(L, 0, 1);
  lua_pushcfunction(L, &LuaGetNmeshes);
  lua_setfield(L, -2, "__len");
  // then set the metatable
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "meshes");

  // As with meshes the number and names of devices is unlikely to change
  LuaGetDevices(L);
  // devices is currently on the stack
  // add a metatable to it.
  // first create the table
  lua_createtable(L, 0, 1);
  lua_pushcfunction(L, &LuaGetNdevices);
  lua_setfield(L, -2, "__len");
  // then set the metatable
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "devices");

  // slicecoll.sliceinfo is a 1-indexed array so the lua length operator
  // works without the need for a metatable
  LuaGetSliceinfo(L);
  lua_setfield(L, -2, "slices");

  // lua_get_rampinfo(L);
  // lua_setglobal(L, "rampinfo");

  LuaGetCsvinfo(L);
  // csvinfo is currently on the stack
  // add a metatable to it.
  // first create the table
  lua_createtable(L, 0, 1);
  lua_pushcfunction(L, &LuaGetNcsvinfo);
  lua_setfield(L, -2, "__len");
  // then set the metatable
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "csvs");

  LuaGetPlot3dinfo(L);
  // plot3dinfo is currently on the stack
  // add a metatable to it.
  // first create the table
  lua_createtable(L, 0, 1);
  lua_pushcfunction(L, &LuaGetNplot3dinfo);
  lua_setfield(L, -2, "__len");
  // then set the metatable
  lua_setmetatable(L, -2);
  lua_setfield(L, -2, "pl3ds");

  // set up tables to access pl3dinfo better
  // setup_pl3dtables(L);

  // lua_get_geomdata(L);
  // lua_setglobal(L, "geomdata");

  // Case metatable
  lua_createtable(L, 0, 1);
  lua_pushcfunction(L, &LuaCaseIndex);
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, &LuaCaseNewindex);
  lua_setfield(L, -2, "__newindex");
  lua_setmetatable(L, -2);

  return 1;
}

/// @brief As with lua_initsmvdata(), but for information relating to Smokeview
/// itself.
int LuaInitsmvproginfo(lua_State *L) {
  // char githash[256];
  char version[256];

  strcpy(version, "");
  AddLuaPaths(L);
  // getGitHash(githash);

  lua_createtable(L, 0, 6);

  lua_pushstring(L, version);
  lua_setfield(L, -2, "version");

  // lua_pushstring(L, githash);
  // lua_setfield(L, -2, "githash");

  lua_pushstring(L, __DATE__);
  lua_setfield(L, -2, "builddate");

  lua_pushstring(L, fds_githash);
  lua_setfield(L, -2, "fdsgithash");

  lua_pushstring(L, smokeviewpath);
  lua_setfield(L, -2, "smokeviewpath");

  lua_pushstring(L, smokezippath);
  lua_setfield(L, -2, "smokezippath");

  lua_pushstring(L, texturedir);
  lua_setfield(L, -2, "texturedir");

  lua_setglobal(L, "smokeviewProgram");
  return 0;
}

int LuaGetSlice(lua_State *L) {
  // This should push a lightuserdata onto the stack which is a pointer to the
  // slicedata. This takes the index of the slice (in the slicecoll.sliceinfo array) as an
  // argument.
  // Get the index of the slice as an argument to the lua function.
  int slice_index = lua_tonumber(L, 1);
  // Get the pointer to the slicedata struct.
  slicedata *slice = &slicecoll.sliceinfo[slice_index];
  // Push the pointer onto the lua stack as lightuserdata.
  lua_pushlightuserdata(L, slice);
  // lua_newuserdata places the data on the stack, so return a single stack
  // item.
  return 1;
}

/// @brief This takes a lightuserdata pointer as an argument, and returns the
/// slice label as a string.
int LuaSliceGetLabel(lua_State *L) {
  // get the lightuserdata from the stack, which is a pointer to the 'slicedata'
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  // Push the string onto the stack
  lua_pushstring(L, slice->slicelabel);
  return 1;
}

/// @brief This takes a lightuserdata pointer as an argument, and returns the
/// slice filename as a string.
int LuaSliceGetFilename(lua_State *L) {
  // Get the lightuserdata from the stack, which is a pointer to the
  // 'slicedata'.
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  // Push the string onto the stack
  lua_pushstring(L, slice->file);
  return 1;
}

int LuaSliceGetData(lua_State *L) {
  // get the lightuserdata from the stack, which is a pointer to the 'slicedata'
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  // Push a lightuserdata (a pointer) onto the lua stack that points to the
  // qslicedata.
  lua_pushlightuserdata(L, slice->qslicedata);
  return 1;
}

int LuaSliceGetTimes(lua_State *L) {
  int i;
  // get the lightuserdata from the stack, which is a pointer to the 'slicedata'
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  // Push a lightuserdata (a pointer) onto the lua stack that points to the
  // qslicedata.
  lua_createtable(L, slice->ntimes, 0);
  for(i = 0; i < slice->ntimes; i++) {
    lua_pushnumber(L, i + 1);
    lua_pushnumber(L, slice->times[i]);
    lua_settable(L, -3);
  }
  return 1;
}

int LuaGetPart(lua_State *L) {
  // This should push a lightuserdata onto the stack which is a pointer to the
  // partdata. This takes the index of the part (in the partinfo array) as an
  // argument.
  // Get the index of the slice as an argument to the lua function.
  int part_index = lua_tonumber(L, 1);
  // Get the pointer to the slicedata struct.
  partdata *part = &partinfo[part_index];
  // Push the pointer onto the lua stack as lightuserdata.
  lua_pushlightuserdata(L, part);
  // lua_newuserdata places the data on the stack, so return a single stack
  // item.
  return 1;
}

// pass in the part data
int LuaGetPartNpoints(lua_State *L) {
  int index;
  partdata *parti = (partdata *)lua_touserdata(L, 1);
  if(!parti->loaded) {
    return luaL_error(L, "particle file %s not loaded", parti->file);
  }

  // Create a table with an entry for x, y and name
  lua_createtable(L, 3, 0);

  lua_pushstring(L, "name");
  lua_pushstring(L, "(unknown)");
  lua_settable(L, -3);

  // x entries
  lua_pushstring(L, "x");
  lua_createtable(L, 3, 0);

  lua_pushstring(L, "units");
  lua_pushstring(L, "s");
  lua_settable(L, -3);

  lua_pushstring(L, "name");
  lua_pushstring(L, "Time");
  lua_settable(L, -3);

  lua_pushstring(L, "values");
  lua_createtable(L, parti->ntimes, 0);

  // Create a table with an entry for each time
  for(index = 0; index < parti->ntimes; index++) {
    part5data *part5 = parti->data5 + index * parti->nclasses;
    // sum += part5->npoints_file;

    // use a 1-indexed array to match lua
    lua_pushnumber(L, index + 1);
    lua_pushnumber(L, part5->time);
    lua_settable(L, -3);
  }
  lua_settable(L, -3);
  lua_settable(L, -3);

  // y entries
  lua_pushstring(L, "y");
  lua_createtable(L, 3, 0);

  lua_pushstring(L, "units");
  lua_pushstring(L, "#");
  lua_settable(L, -3);

  lua_pushstring(L, "name");
  lua_pushstring(L, "# Particles");
  lua_settable(L, -3);

  lua_pushstring(L, "values");
  lua_createtable(L, parti->ntimes, 0);

  // Create a table with an entry for each time
  for(index = 0; index < parti->ntimes; index++) {
    part5data *part5 = parti->data5 + index * parti->nclasses;
    // sum += part5->npoints_file;

    // use a 1-indexed array to match lua
    lua_pushnumber(L, index + 1);
    lua_pushnumber(L, part5->npoints_file);
    lua_settable(L, -3);
  }
  lua_settable(L, -3);
  lua_settable(L, -3);

  // Return a table of values.
  return 1;
}

// int lua_get_all_part_

int LuaSliceDataMapFrames(lua_State *L) {
  // The first argument to this function is the slice pointer. This function
  // receives the values of the slice at a particular frame as an array.
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  if(!slice->loaded) {
    return luaL_error(L, "slice %s not loaded", slice->file);
  }
  int framepoints = slice->nslicex * slice->nslicey;
  // Pointer to the first frame.
  float *qslicedata = slice->qslicedata;
  // The second argument is the function to be called on each frame.
  lua_createtable(L, slice->ntimes, 0);
  // framenumber is the index of the frame (0-based).
  int framenumber;
  for(framenumber = 0; framenumber < slice->ntimes; framenumber++) {
    // duplicate the function so that we can use it and keep it
    lua_pushvalue(L, 2);
    // Push the first frame onto the stack by first putting them into a lua
    // table. Values are indexed from 1.
    // Feed the lua function a lightuserdata (pointer) that is can use
    // with a special function to index the array.
    lua_pushnumber(L, framepoints);
    // lua_pushlightuserdata(L, &qslicedata[framenumber*framepoints]);

    // this table method is more flexible but slower
    lua_createtable(L, framepoints, 0);
    // pointnumber is the index of the data point in the frame (0-based).
    int pointnumber;
    for(pointnumber = 0; pointnumber < framepoints; pointnumber++) {
      // adjust the index to start from 1
      lua_pushnumber(L, pointnumber + 1);
      lua_pushnumber(L, qslicedata[framenumber * framepoints + pointnumber]);
      lua_settable(L, -3);
    }

    // The function takes 2 arguments and returns 1 result.
    lua_call(L, 2, 1);
    // Add the value to the results table.
    lua_pushnumber(L, framenumber + 1);
    lua_pushvalue(L, -2);
    lua_settable(L, -4);
    lua_pop(L, 1);
  }
  // Return a table of values.
  return 1;
}

int LuaSliceDataMapFramesCountLess(lua_State *L) {
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  if(!slice->loaded) {
    return luaL_error(L, "slice %s not loaded", slice->file);
  }
  float threshold = lua_tonumber(L, 2);
  int framepoints = slice->nslicex * slice->nslicey;
  // Pointer to the first frame.
  float *qslicedata = slice->qslicedata;
  lua_createtable(L, slice->ntimes, 0);
  int framenumber;
  for(framenumber = 0; framenumber < slice->ntimes; framenumber++) {
    int count = 0;
    int pointnumber;
    for(pointnumber = 0; pointnumber < framepoints; pointnumber++) {
      if(*qslicedata < threshold) {
        count++;
      }
      qslicedata++;
    }
    lua_pushnumber(L, framenumber + 1);
    lua_pushnumber(L, count);
    lua_settable(L, -3);
  }
  // Return a table of values.
  return 1;
}

int LuaSliceDataMapFramesCountLessEq(lua_State *L) {
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  if(!slice->loaded) {
    return luaL_error(L, "slice %s not loaded", slice->file);
  }
  float threshold = lua_tonumber(L, 2);
  int framepoints = slice->nslicex * slice->nslicey;
  // Pointer to the first frame.
  float *qslicedata = slice->qslicedata;
  lua_createtable(L, slice->ntimes, 0);
  int framenumber;
  for(framenumber = 0; framenumber < slice->ntimes; framenumber++) {
    int count = 0;
    int pointnumber;
    for(pointnumber = 0; pointnumber < framepoints; pointnumber++) {
      if(*qslicedata <= threshold) {
        count++;
      }
      qslicedata++;
    }
    lua_pushnumber(L, framenumber + 1);
    lua_pushnumber(L, count);
    lua_settable(L, -3);
  }
  // Return a table of values.
  return 1;
}

int LuaSliceDataMapFramesCountGreater(lua_State *L) {
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  if(!slice->loaded) {
    return luaL_error(L, "slice %s not loaded", slice->file);
  }
  float threshold = lua_tonumber(L, 2);
  int framepoints = slice->nslicex * slice->nslicey;
  // Pointer to the first frame.
  float *qslicedata = slice->qslicedata;
  lua_createtable(L, slice->ntimes, 0);
  int framenumber;
  for(framenumber = 0; framenumber < slice->ntimes; framenumber++) {
    int count = 0;
    int pointnumber;
    for(pointnumber = 0; pointnumber < framepoints; pointnumber++) {
      if(*qslicedata > threshold) {
        count++;
      }
      qslicedata++;
    }
    lua_pushnumber(L, framenumber + 1);
    lua_pushnumber(L, count);
    lua_settable(L, -3);
  }
  // Return a table of values.
  return 1;
}

int LuaSliceDataMapFramesCountGreaterEq(lua_State *L) {
  slicedata *slice = (slicedata *)lua_touserdata(L, 1);
  if(!slice->loaded) {
    return luaL_error(L, "slice %s not loaded", slice->file);
  }
  float threshold = lua_tonumber(L, 2);
  int framepoints = slice->nslicex * slice->nslicey;
  // Pointer to the first frame.
  float *qslicedata = slice->qslicedata;
  lua_createtable(L, slice->ntimes, 0);
  int framenumber;
  for(framenumber = 0; framenumber < slice->ntimes; framenumber++) {
    int count = 0;
    int pointnumber;
    for(pointnumber = 0; pointnumber < framepoints; pointnumber++) {
      if(*qslicedata >= threshold) {
        count++;
      }
      qslicedata++;
    }
    lua_pushnumber(L, framenumber + 1);
    lua_pushnumber(L, count);
    lua_settable(L, -3);
  }
  // Return a table of values.
  return 1;
}

/// @brief Pushes a value from a slice onto the stack (a single slice, not
/// multi). The arguments are
/// 1. int framenumber
/// 2. int i
/// 3. int j
/// 4. ink k
/// The slice index is stored as part of a closure.
int LuaGetslicedata(lua_State *L) {
  // The offset in the global slicecoll.sliceinfo table of the slice.
  int slice_index = lua_tonumber(L, lua_upvalueindex(1));
  // The time frame to use
  int f = lua_tonumber(L, 1);
  // The offsets into the mesh requested (NOT the data array)
  int i = lua_tonumber(L, 2);
  int j = lua_tonumber(L, 3);
  int k = lua_tonumber(L, 4);
  // printf("getting slice data: %d, %d, %d-%d-%d\n", slice_index, f, i, j, k);
  // print all the times
  // printf("times: %d\n", slicecoll.sliceinfo[slice_index].ntimes);
  // int n = 0;
  // for (n; n < slicecoll.sliceinfo[slice_index].ntimes; n++) {
  //   fprintf(stderr, "t:%.2f s\n", slicecoll.sliceinfo[slice_index].times[n]);
  // }
  // fprintf(stderr, "f:%d i:%d j:%d  k:%d\n", f, i,j,k);

  int imax = slicecoll.sliceinfo[slice_index].ijk_max[0];
  int jmax = slicecoll.sliceinfo[slice_index].ijk_max[1];
  int kmax = slicecoll.sliceinfo[slice_index].ijk_max[2];

  int di = slicecoll.sliceinfo[slice_index].nslicei;
  int dj = slicecoll.sliceinfo[slice_index].nslicej;
  int dk = slicecoll.sliceinfo[slice_index].nslicek;
  // Check that the offsets do not exceed the bounds of a single data frame
  if(i > imax || j > jmax || k > kmax) {
    fprintf(stderr, "ERROR: offsets exceed bounds");
    exit(1);
  }
  // Convert the offsets into the mesh into offsets into the data array
  int i_offset = i - slicecoll.sliceinfo[slice_index].ijk_min[0];
  int j_offset = j - slicecoll.sliceinfo[slice_index].ijk_min[1];
  int k_offset = k - slicecoll.sliceinfo[slice_index].ijk_min[2];

  // Offset into a single frame
  int offset = (dk * dj) * i_offset + dk * j_offset + k_offset;
  int framesize = di * dj * dk;
  float val = slicecoll.sliceinfo[slice_index].qslicedata[offset + f * framesize];
  // lua_pushstring(L,sliceinfo[slice_index].file);
  lua_pushnumber(L, val);
  return 1;
}

/// @brief Build a Lua table with information on the slices of the model.
// TODO: change this to use userdata instead
int LuaGetSliceinfo(lua_State *L) {
  lua_createtable(L, 0, slicecoll.nsliceinfo);
  int i;
  for(i = 0; i < slicecoll.nsliceinfo; i++) {
    lua_pushnumber(L, i + 1);
    lua_createtable(L, 0, 21);

    if(slicecoll.sliceinfo[i].slicelabel != NULL) {
      lua_pushstring(L, slicecoll.sliceinfo[i].slicelabel);
      lua_setfield(L, -2, "label");
    }

    lua_pushnumber(L, i);
    lua_setfield(L, -2, "n");

    if(slicecoll.sliceinfo[i].label.longlabel != NULL) {
      lua_pushstring(L, slicecoll.sliceinfo[i].label.longlabel);
      lua_setfield(L, -2, "longlabel");
    }

    if(slicecoll.sliceinfo[i].label.shortlabel != NULL) {
      lua_pushstring(L, slicecoll.sliceinfo[i].label.shortlabel);
      lua_setfield(L, -2, "shortlabel");
    }

    lua_pushstring(L, slicecoll.sliceinfo[i].file);
    lua_setfield(L, -2, "file");

    lua_pushnumber(L, slicecoll.sliceinfo[i].slice_filetype);
    lua_setfield(L, -2, "slicefile_type");

    lua_pushnumber(L, slicecoll.sliceinfo[i].idir);
    lua_setfield(L, -2, "idir");

    lua_pushnumber(L, slicecoll.sliceinfo[i].sliceoffset);
    lua_setfield(L, -2, "sliceoffset");

    lua_pushnumber(L, slicecoll.sliceinfo[i].ijk_min[0]);
    lua_setfield(L, -2, "imin");

    lua_pushnumber(L, slicecoll.sliceinfo[i].ijk_max[0]);
    lua_setfield(L, -2, "imax");

    lua_pushnumber(L, slicecoll.sliceinfo[i].ijk_min[1]);
    lua_setfield(L, -2, "jmin");

    lua_pushnumber(L, slicecoll.sliceinfo[i].ijk_max[1]);
    lua_setfield(L, -2, "jmax");

    lua_pushnumber(L, slicecoll.sliceinfo[i].ijk_min[2]);
    lua_setfield(L, -2, "kmin");

    lua_pushnumber(L, slicecoll.sliceinfo[i].ijk_max[2]);
    lua_setfield(L, -2, "kmax");

    lua_pushnumber(L, slicecoll.sliceinfo[i].blocknumber);
    lua_setfield(L, -2, "blocknumber");

    lua_pushnumber(L, slicecoll.sliceinfo[i].position_orig);
    lua_setfield(L, -2, "position_orig");

    lua_pushnumber(L, slicecoll.sliceinfo[i].nslicex);
    lua_setfield(L, -2, "nslicex");

    lua_pushnumber(L, slicecoll.sliceinfo[i].nslicey);
    lua_setfield(L, -2, "nslicey");

    lua_pushstring(L, slicecoll.sliceinfo[i].cdir);
    lua_setfield(L, -2, "slicedir");

    // can't be done until loaded
    // lua_pushnumber(L, slicecoll.sliceinfo[i].ntimes);
    // lua_setfield(L, -2, "ntimes");

    // Push the slice index so that getslicedata knows which slice to operate
    // on.
    lua_pushnumber(L, i);
    // Push a closure which has been provided with the first argument (the slice
    // index)
    lua_pushcclosure(L, LuaGetslicedata, 1);
    lua_setfield(L, -2, "getdata");

    lua_settable(L, -3);
  }
  return 1;
}

int LuaGetCsventry(lua_State *L) {
  const char *key = lua_tostring(L, -1);
  csvfiledata *csventry = GetCsvinfo(key);
  int index = GetCsvindex(key);
  lua_createtable(L, 0, 4);
  lua_pushstring(L, csventry->file);
  lua_setfield(L, -2, "file");

  lua_pushstring(L, csventry->c_type);
  lua_setfield(L, -2, "c_type");

  lua_pushnumber(L, index);
  lua_setfield(L, -2, "index");

  lua_pushstring(L, key);
  lua_pushcclosure(L, &LuaLoadCsv, 1);
  lua_setfield(L, -2, "load");

  // Create a metatable.
  // TODO: this metatable might be more easily implemented directly in Lua.
  lua_createtable(L, 0, 1);
  lua_pushcfunction(L, &AccessCsventryProp);
  lua_setfield(L, -2, "__index");
  // then set the metatable
  lua_setmetatable(L, -2);
  return 1;
}

/// @brief Build a Lua table with information on the CSV files available to the
/// model.
// TODO: provide more information via this interface.
// TODO: use metatables so that the most up-to-date information is retrieved.
int LuaGetCsvinfo(lua_State *L) {
  lua_createtable(L, 0, ncsvfileinfo);
  int i;
  for(i = 0; i < ncsvfileinfo; i++) {
    lua_pushstring(L, csvfileinfo[i].c_type);
    LuaGetCsventry(L);
    lua_settable(L, -3);
  }
  return 1;
}

int LuaLoadvslice(lua_State *L) {
  const char *type = lua_tostring(L, 1);
  int axis = lua_tonumber(L, 2);
  float distance = lua_tonumber(L, 3);
  Loadvslice(type, axis, distance);
  return 0;
}

int LuaLoadiso(lua_State *L) {
  const char *type = lua_tostring(L, 1);
  Loadiso(type);
  return 0;
}

int LuaLoad3dsmoke(lua_State *L) {
  const char *smoke_type = lua_tostring(L, 1);
  Load3dsmoke(smoke_type);
  return 0;
}

int LuaLoadvolsmoke(lua_State *L) {
  int meshnumber = lua_tonumber(L, 1);
  Loadvolsmoke(meshnumber);
  return 0;
}

int LuaLoadvolsmokeframe(lua_State *L) {
  int meshnumber = lua_tonumber(L, 1);
  int framenumber = lua_tonumber(L, 1);
  Loadvolsmokeframe(meshnumber, framenumber, 1);
  // returnval = 1; // TODO: determine if this is the correct behaviour.
  // this is what is done in the SSF code.
  return 0;
}

/// @brief Set the format of images which will be exported. The value should be
/// a string. The acceptable values are:
///   "JPG"
///   "PNG"
int LuaSetRendertype(lua_State *L) {
  const char *type = lua_tostring(L, 1);
  if(SetRendertype(type)) {
    return luaL_error(L, "%s is not a valid render type", type);
  }
  return 0;
}

int LuaGetRendertype(lua_State *L) {
  int render_type = GetRendertype();
  switch(render_type) {
  case JPEG:
    lua_pushstring(L, "JPG");
    break;
  case PNG:
    lua_pushstring(L, "PNG");
    break;
  default:
    lua_pushstring(L, NULL);
    break;
  }
  return 1;
}

/// @brief Set the format of movies which will be exported. The value should be
/// a string. The acceptable values are:
///    - "WMV"
///    - "MP4"
///    - "AVI"
int LuaSetMovietype(lua_State *L) {
  const char *type = lua_tostring(L, 1);
  SetMovietype(type);
  return 0;
}

int LuaGetMovietype(lua_State *L) {
  int movie_type = GetMovietype();
  switch(movie_type) {
  case WMV:
    lua_pushstring(L, "WMV");
    break;
  case MP4:
    lua_pushstring(L, "MP4");
    break;
  case AVI:
    lua_pushstring(L, "AVI");
    break;
  default:
    lua_pushstring(L, NULL);
    break;
  }
  return 1;
}

int LuaMakemovie(lua_State *L) {
  const char *name = lua_tostring(L, 1);
  const char *base = lua_tostring(L, 2);
  float framerate_local = lua_tonumber(L, 3);
  Makemovie(name, base, framerate_local);
  return 0;
}

int LuaLoadtour(lua_State *L) {
  const char *name = lua_tostring(L, 1);
  int error_code = Loadtour(name);
  lua_pushnumber(L, error_code);
  return 1;
}

int LuaLoadparticles(lua_State *L) {
  const char *name = lua_tostring(L, 1);
  Loadparticles(name);
  return 0;
}

int LuaPartclasscolor(lua_State *L) {
  const char *color = lua_tostring(L, 1);
  Partclasscolor(color);
  return 0;
}

int LuaPartclasstype(lua_State *L) {
  const char *type = lua_tostring(L, 1);
  Partclasstype(type);
  return 0;
}

int LuaPlot3dprops(lua_State *L) {
  int variable_index = lua_tonumber(L, 1);
  int showvector = lua_toboolean(L, 2);
  int vector_length_index = lua_tonumber(L, 3);
  int display_type = lua_tonumber(L, 4);
  float vector_length = lua_tonumber(L, 5);
  Plot3dprops(variable_index, showvector, vector_length_index, display_type,
              vector_length);
  return 0;
}

int LuaLoadplot3d(lua_State *L) {
  int meshnumber = lua_tonumber(L, 1);
  float time_local = lua_tonumber(L, 2);
  Loadplot3d(meshnumber, time_local);
  return 0;
}

int LuaUnloadall(lua_State *L) {
  Unloadall();
  return 0;
}

int LuaUnloadtour(lua_State *L) {
  Unloadtour();
  return 0;
}

int LuaSetrenderdir(lua_State *L) {
  const char *dir = lua_tostring(L, -1);
  int return_code = Setrenderdir(dir);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaGetrenderdir(lua_State *L) {
  lua_pushstring(L, script_dir_path);
  return 1;
}

int LuaSetOrthoPreset(lua_State *L) {
  const char *viewpoint = lua_tostring(L, 1);
  int errorcode = SetOrthoPreset(viewpoint);
  lua_pushnumber(L, errorcode);
  return 1;
}

int LuaSetviewpoint(lua_State *L) {
  const char *viewpoint = lua_tostring(L, 1);
  int errorcode = Setviewpoint(viewpoint);
  lua_pushnumber(L, errorcode);
  return 1;
}

int LuaGetviewpoint(lua_State *L) {
  lua_pushstring(L, camera_current->name);
  return 1;
}

int LuaExitSmokeview(lua_State *L) {
  ExitSmokeview();
  return 0;
}

int LuaSetwindowsize(lua_State *L) {
  int width = lua_tonumber(L, 1);
  int height = lua_tonumber(L, 2);
  Setwindowsize(width, height);
  // Using the DisplayCB is not sufficient in this case,
  // control must be temporarily returned to the main glut loop.
  LuaTempyieldscript(L);
  return 0;
}

int LuaSetgridvisibility(lua_State *L) {
  int selection = lua_tonumber(L, 1);
  Setgridvisibility(selection);
  return 0;
}

int LuaSetgridparms(lua_State *L) {
  int x_vis = lua_tonumber(L, 1);
  int y_vis = lua_tonumber(L, 2);
  int z_vis = lua_tonumber(L, 3);

  int x_plot = lua_tonumber(L, 4);
  int y_plot = lua_tonumber(L, 5);
  int z_plot = lua_tonumber(L, 6);

  Setgridparms(x_vis, y_vis, z_vis, x_plot, y_plot, z_plot);

  return 0;
}

int LuaSetcolorbarflip(lua_State *L) {
  int flip = lua_toboolean(L, 1);
  Setcolorbarflip(flip);
  LuaTempyieldscript(L);
  return 0;
}

int LuaGetcolorbarflip(lua_State *L) {
  int flip = Getcolorbarflip();
  lua_pushboolean(L, flip);
  return 1;
}

int LuaSetcolorbarindex(lua_State *L) {
  int chosen_index = lua_tonumber(L, 1);
  Setcolorbarindex(chosen_index);
  return 0;
}

int LuaGetcolorbarindex(lua_State *L) {
  int index = Getcolorbarindex();
  lua_pushnumber(L, index);
  return 1;
}

int LuaSetSliceInObst(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetSliceInObst(setting);
  return 0;
}

int LuaGetSliceInObst(lua_State *L) {
  int setting = GetSliceInObst();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaSetColorbar(lua_State *L) {
  int index = lua_tonumber(L, 1);
  SetColorbar(index);
  return 0;
}

int LuaSetNamedColorbar(lua_State *L) {
  const char *name = lua_tostring(L, 1);
  int err = SetNamedColorbar(name);
  if(err == 1) {
    luaL_error(L, "%s is not a valid colorbar name", name);
  }
  return 0;
}

// int lua_get_named_colorbar(lua_State *L) {
//   int err = GetNamedColorbar();
//   if (err == 1) {
//     luaL_error(L, "%s is not a valid colorbar name", name);
//   }
//   return 0;
// }

//////////////////////

int LuaSetColorbarVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetColorbarVisibility(setting);
  return 0;
}

int LuaGetColorbarVisibility(lua_State *L) {
  int setting = GetColorbarVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleColorbarVisibility(lua_State *L) {
  ToggleColorbarVisibility();
  return 0;
}

int LuaSetColorbarVisibilityHorizontal(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetColorbarVisibilityHorizontal(setting);
  return 0;
}

int LuaGetColorbarVisibilityHorizontal(lua_State *L) {
  int setting = GetColorbarVisibilityHorizontal();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleColorbarVisibilityHorizontal(lua_State *L) {
  ToggleColorbarVisibilityHorizontal();
  return 0;
}

int LuaSetColorbarVisibilityVertical(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetColorbarVisibilityVertical(setting);
  return 0;
}

int LuaGetColorbarVisibilityVertical(lua_State *L) {
  int setting = GetColorbarVisibilityVertical();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleColorbarVisibilityVertical(lua_State *L) {
  ToggleColorbarVisibilityVertical();
  return 0;
}

int LuaSetTimebarVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetTimebarVisibility(setting);
  return 0;
}

int LuaGetTimebarVisibility(lua_State *L) {
  int setting = GetTimebarVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleTimebarVisibility(lua_State *L) {
  ToggleTimebarVisibility();
  return 0;
}

// title
int LuaSetTitleVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetTitleVisibility(setting);
  return 0;
}

int LuaGetTitleVisibility(lua_State *L) {
  int setting = GetTitleVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleTitleVisibility(lua_State *L) {
  ToggleTitleVisibility();
  return 0;
}

// smv_version
int LuaSetSmvVersionVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetSmvVersionVisibility(setting);
  return 0;
}

int LuaGetSmvVersionVisibility(lua_State *L) {
  int setting = GetSmvVersionVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleSmvVersionVisibility(lua_State *L) {
  ToggleSmvVersionVisibility();
  return 0;
}

// chid
int LuaSetChidVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetChidVisibility(setting);
  return 0;
}

int LuaGetChidVisibility(lua_State *L) {
  int setting = GetChidVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleChidVisibility(lua_State *L) {
  ToggleChidVisibility();
  return 0;
}

// blockages
int LuaBlockagesHideAll(lua_State *L) {
  BlockagesHideAll();
  return 0;
}

// outlines
int LuaOutlinesHide(lua_State *L) {
  OutlinesHide();
  return 0;
}
int LuaOutlinesShow(lua_State *L) {
  OutlinesShow();
  return 0;
}

// surfaces
int LuaSurfacesHideAll(lua_State *L) {
  SurfacesHideAll();
  return 0;
}

// devices
int LuaDevicesHideAll(lua_State *L) {
  DevicesHideAll();
  return 0;
}

// axis
int LuaSetAxisVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetAxisVisibility(setting);
  return 0;
}

int LuaGetAxisVisibility(lua_State *L) {
  int setting = GetAxisVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleAxisVisibility(lua_State *L) {
  ToggleAxisVisibility();
  return 0;
}

// frame
int LuaSetFramelabelVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetFramelabelVisibility(setting);
  return 0;
}

int LuaGetFramelabelVisibility(lua_State *L) {
  int setting = GetFramelabelVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleFramelabelVisibility(lua_State *L) {
  ToggleFramelabelVisibility();
  return 0;
}

// framerate
int LuaSetFramerateVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetFramerateVisibility(setting);
  return 0;
}

int LuaGetFramerateVisibility(lua_State *L) {
  int setting = GetFramerateVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleFramerateVisibility(lua_State *L) {
  ToggleFramerateVisibility();
  return 0;
}

// grid locations
int LuaSetGridlocVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetGridlocVisibility(setting);
  return 0;
}

int LuaGetGridlocVisibility(lua_State *L) {
  int setting = GetGridlocVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleGridlocVisibility(lua_State *L) {
  ToggleGridlocVisibility();
  return 0;
}

// hrrpuv cutoff
int LuaSetHrrcutoffVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetHrrcutoffVisibility(setting);
  return 0;
}

int LuaGetHrrcutoffVisibility(lua_State *L) {
  int setting = GetHrrcutoffVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleHrrcutoffVisibility(lua_State *L) {
  ToggleHrrcutoffVisibility();
  return 0;
}

// HRR Label Visbility
int LuaSetHrrlabelVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetHrrlabelVisibility(setting);
  return 0;
}

int LuaGetHrrlabelVisibility(lua_State *L) {
  int setting = GetHrrlabelVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleHrrlabelVisibility(lua_State *L) {
  ToggleHrrlabelVisibility();
  return 0;
}
// memory load
#ifdef pp_memstatus
int LuaSetMemloadVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  set_memload_visibility(setting);
  return 0;
}

int LuaGetMemloadVisibility(lua_State *L) {
  int setting = get_memload_visibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleMemloadVisibility(lua_State *L) {
  toggle_memload_visibility();
  return 0;
}
#endif

// mesh label
int LuaSetMeshlabelVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetMeshlabelVisibility(setting);
  return 0;
}

int LuaGetMeshlabelVisibility(lua_State *L) {
  int setting = GetMeshlabelVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleMeshlabelVisibility(lua_State *L) {
  ToggleMeshlabelVisibility();
  return 0;
}

// slice average
int LuaSetSliceAverageVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetSliceAverageVisibility(setting);
  return 0;
}

int LuaGetSliceAverageVisibility(lua_State *L) {
  int setting = GetSliceAverageVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleSliceAverageVisibility(lua_State *L) {
  ToggleSliceAverageVisibility();
  return 0;
}

// time
int LuaSetTimeVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetTimeVisibility(setting);
  return 0;
}

int LuaGetTimeVisibility(lua_State *L) {
  int setting = GetTimeVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleTimeVisibility(lua_State *L) {
  ToggleTimeVisibility();
  return 0;
}

// user settable ticks
int LuaSetUserTicksVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetUserTicksVisibility(setting);
  return 0;
}

int LuaGetUserTicksVisibility(lua_State *L) {
  int setting = GetUserTicksVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleUserTicksVisibility(lua_State *L) {
  ToggleUserTicksVisibility();
  return 0;
}

// version info
int LuaSetVersionInfoVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetVersionInfoVisibility(setting);
  return 0;
}

int LuaGetVersionInfoVisibility(lua_State *L) {
  int setting = GetVersionInfoVisibility();
  lua_pushboolean(L, setting);
  return 1;
}

int LuaToggleVersionInfoVisibility(lua_State *L) {
  ToggleVersionInfoVisibility();
  return 0;
}

// set all
int LuaSetAllLabelVisibility(lua_State *L) {
  int setting = lua_toboolean(L, 1);
  SetAllLabelVisibility(setting);
  return 0;
}

//////////////////////////////////////

int LuaBlockageViewMethod(lua_State *L) {
  int setting = lua_tonumber(L, 1);
  int return_code = BlockageViewMethod(setting);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaBlockageOutlineColor(lua_State *L) {
  int setting = lua_tonumber(L, 1);
  int return_code = BlockageOutlineColor(setting);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaBlockageLocations(lua_State *L) {
  int setting = lua_tonumber(L, 1);
  int return_code = BlockageLocations(setting);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaCameraModEyex(lua_State *L) {
  float delta = lua_tonumber(L, 1);
  CameraModEyex(delta);
  return 0;
}

int LuaCameraSetEyex(lua_State *L) {
  float eyex = lua_tonumber(L, 1);
  CameraSetEyex(eyex);
  return 0;
}

int LuaCameraModEyey(lua_State *L) {
  float delta = lua_tonumber(L, 1);
  CameraModEyey(delta);
  return 0;
}

int LuaCameraSetEyey(lua_State *L) {
  float eyey = lua_tonumber(L, 1);
  CameraSetEyey(eyey);
  return 0;
}

int LuaCameraModEyez(lua_State *L) {
  float delta = lua_tonumber(L, 1);
  CameraModEyez(delta);
  return 0;
}

int LuaCameraSetEyez(lua_State *L) {
  float eyez = lua_tonumber(L, 1);
  CameraSetEyez(eyez);
  return 0;
}

int LuaCameraModAz(lua_State *L) {
  float delta = lua_tonumber(L, 1);
  CameraModAz(delta);
  return 0;
}

int LuaCameraSetAz(lua_State *L) {
  float az = lua_tonumber(L, 1);
  CameraSetAz(az);
  return 0;
}

int LuaCameraGetAz(lua_State *L) {
  lua_pushnumber(L, CameraGetAz());
  return 1;
}

int LuaCameraModElev(lua_State *L) {
  float delta = lua_tonumber(L, 1);
  CameraModElev(delta);
  return 0;
}
int LuaCameraZoomToFit(lua_State *L) {
  CameraZoomToFit();
  return 0;
}

int LuaCameraSetElev(lua_State *L) {
  float elev = lua_tonumber(L, 1);
  CameraSetElev(elev);
  return 0;
}

int LuaCameraGetElev(lua_State *L) {
  lua_pushnumber(L, CameraGetElev());
  return 1;
}
int LuaCameraGetProjectionType(lua_State *L) {
  float projection_type_local = CameraGetProjectionType();
  lua_pushnumber(L, projection_type_local);
  return 1;
}
int LuaCameraSetProjectionType(lua_State *L) {
  float projection_type_local = lua_tonumber(L, 1);
  int return_value = CameraSetProjectionType(projection_type_local);
  lua_pushnumber(L, return_value);
  return 1;
}

int LuaCameraGetRotationType(lua_State *L) {
  float rotation_type_local = CameraGetRotationType();
  lua_pushnumber(L, rotation_type_local);
  return 1;
}

int LuaCameraGetRotationIndex(lua_State *L) {
  float rotation_index = CameraGetRotationIndex();
  lua_pushnumber(L, rotation_index);
  return 1;
}

int LuaCameraSetRotationType(lua_State *L) {
  int rotation_type_local = lua_tonumber(L, 1);
  CameraSetRotationType(rotation_type_local);
  return 0;
}

int LuaCameraGetZoom(lua_State *L) {
  lua_pushnumber(L, zoom);
  return 1;
}

int LuaCameraSetZoom(lua_State *L) {
  float x = lua_tonumber(L, 1);
  zoom = x;
  return 0;
}

int LuaCameraGetEyex(lua_State *L) {
  float eyex = CameraGetEyex();
  lua_pushnumber(L, eyex);
  return 1;
}

int LuaCameraGetEyey(lua_State *L) {
  float eyey = CameraGetEyex();
  lua_pushnumber(L, eyey);
  return 1;
}

int LuaCameraGetEyez(lua_State *L) {
  float eyez = CameraGetEyez();
  lua_pushnumber(L, eyez);
  return 1;
}
int LuaCameraSetViewdir(lua_State *L) {
  float xcen = lua_tonumber(L, 1);
  float ycen = lua_tonumber(L, 2);
  float zcen = lua_tonumber(L, 3);
  CameraSetViewdir(xcen, ycen, zcen);
  return 0;
}

int LuaCameraGetViewdir(lua_State *L) {
  float xcen = CameraGetXcen();
  float ycen = CameraGetYcen();
  float zcen = CameraGetZcen();

  lua_createtable(L, 0, 3);

  lua_pushstring(L, "x");
  lua_pushnumber(L, xcen);
  lua_settable(L, -3);

  lua_pushstring(L, "y");
  lua_pushnumber(L, ycen);
  lua_settable(L, -3);

  lua_pushstring(L, "z");
  lua_pushnumber(L, zcen);
  lua_settable(L, -3);

  return 1;
}

int LuaSetSliceBounds(lua_State *L) {
  const char *slice_type = lua_tostring(L, 1);
  int set_min = lua_tonumber(L, 2);
  float value_min = lua_tonumber(L, 3);
  int set_max = lua_tonumber(L, 4);
  float value_max = lua_tonumber(L, 5);
  CApiSetSliceBounds(slice_type, set_min, value_min, set_max, value_max);
  return 0;
}
int LuaSetSliceBoundMin(lua_State *L) {
  const char *slice_type = lua_tostring(L, 1);
  int set = lua_toboolean(L, 2);
  float value = lua_tonumber(L, 3);
  SetSliceBoundMin(slice_type, set, value);
  return 0;
}

int LuaGetSliceBounds(lua_State *L) {
  const char *slice_type = lua_tostring(L, 1);
  simple_bounds bounds;
  if(GetSliceBounds(slice_type, &bounds)) {
    luaL_error(L, "Could not get slice bounds for %s", slice_type);
  }
  lua_pushnumber(L, bounds.min);
  lua_pushnumber(L, bounds.max);
  return 2;
}

int LuaSetSliceBoundMax(lua_State *L) {
  const char *slice_type = lua_tostring(L, 1);
  int set = lua_toboolean(L, 2);
  float value = lua_tonumber(L, 3);
  SetSliceBoundMax(slice_type, set, value);
  return 0;
}

int LuaSetAmbientlight(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetAmbientlight(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaGetBackgroundcolor(lua_State *L) {
  lua_createtable(L, 0, 3);

  lua_pushnumber(L, backgroundbasecolor[0]);
  lua_setfield(L, -2, "r");

  lua_pushnumber(L, backgroundbasecolor[1]);
  lua_setfield(L, -2, "g");

  lua_pushnumber(L, backgroundbasecolor[2]);
  lua_setfield(L, -2, "b");

  return 1;
}

int LuaSetBackgroundcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetBackgroundcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetBlockcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetBlockcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetBlockshininess(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetBlockshininess(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetBlockspecular(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetBlockspecular(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetBoundcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetBoundcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

// int lua_set_colorbar_textureflag(lua_State *L) {
//   int setting = lua_tonumber(L, 1);
//   int return_code = set_colorbar_textureflag(setting);
//   lua_pushnumber(L, 1);
//   return 1;
// }

float Getcolorfield(lua_State *L, int stack_index, const char *key) {
  if(!lua_istable(L, stack_index)) {
    fprintf(stderr,
            "stack is not a table at index, cannot use getcolorfield\n");
    exit(1);
  }
  // if stack index is relative (negative) convert to absolute (positive)
  if(stack_index < 0) {
    stack_index = lua_gettop(L) + stack_index + 1;
  }
  lua_pushstring(L, key);
  lua_gettable(L, stack_index);
  float result = lua_tonumber(L, -1);
  lua_pop(L, 1);
  return result;
}

int GetColor(lua_State *L, int stack_index, float *color) {
  if(!lua_istable(L, stack_index)) {
    fprintf(stderr, "color table is not present\n");
    return 1;
  }
  float r = Getcolorfield(L, stack_index, "r");
  float g = Getcolorfield(L, stack_index, "g");
  float b = Getcolorfield(L, stack_index, "b");
  color[0] = r;
  color[1] = g;
  color[2] = b;
  return 0;
}
int LuaSetColorbarColors(lua_State *L) {
  printf("running: lua_set_colorbar_colors\n");
  if(!lua_istable(L, 1)) {
    fprintf(stderr, "colorbar table is not present\n");
    return 1;
  }
  int ncolors = 0;
  lua_pushnil(L);
  while(lua_next(L, 1) != 0) {
    ncolors++;
    lua_pop(L, 1);
  }
  if(ncolors > 0) {
    float *colors = malloc(sizeof(float) * ncolors * 3);
    for(int i = 1; i <= ncolors; i++) {
      lua_pushnumber(L, i);
      lua_gettable(L, 1);
      GetColor(L, -1, &colors[i - 1]);
    }

    int return_code = SetColorbarColors(ncolors, colors);
    lua_pushnumber(L, return_code);
    free(colors);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaGetColorbarColors(lua_State *L) {
  int i;
  float *rgb_ini_copy_p = rgb_ini;
  lua_createtable(L, 0, nrgb_ini);
  for(i = 0; i < nrgb_ini; i++) {
    lua_pushnumber(L, i + 1);
    lua_createtable(L, 0, 2);

    lua_pushnumber(L, *rgb_ini_copy_p);
    lua_setfield(L, -2, "r");

    lua_pushnumber(L, *(rgb_ini_copy_p + 1));
    lua_setfield(L, -2, "g");

    lua_pushnumber(L, *(rgb_ini_copy_p + 2));
    lua_setfield(L, -2, "b");

    lua_settable(L, -3);
    rgb_ini_copy_p += 3;
  }
  // Leaves one returned value on the stack, the mesh table.
  return 1;
}

int LuaSetColor2barColors(lua_State *L) {
  int ncolors = lua_tonumber(L, 1);
  if(!lua_istable(L, -1)) {
    fprintf(stderr, "colorbar table is not present\n");
    return 1;
  }
  if(ncolors > 0) {
    float *colors = malloc(sizeof(float) * ncolors * 3);
    for(size_t i = 1; i <= ncolors; i++) {
      lua_pushnumber(L, i);
      lua_gettable(L, -2);
      GetColor(L, -1, &colors[i - 1]);
    }

    int return_code = SetColor2barColors(ncolors, colors);
    lua_pushnumber(L, return_code);
    free(colors);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaGetColor2barColors(lua_State *L) {
  int i;
  float *rgb_ini_copy_p = rgb2_ini;
  lua_createtable(L, 0, nrgb2_ini);
  for(i = 0; i < nrgb2_ini; i++) {
    lua_pushnumber(L, i + 1);
    lua_createtable(L, 0, 2);

    lua_pushnumber(L, *rgb_ini_copy_p);
    lua_setfield(L, -2, "r");

    lua_pushnumber(L, *(rgb_ini_copy_p + 1));
    lua_setfield(L, -2, "g");

    lua_pushnumber(L, *(rgb_ini_copy_p + 2));
    lua_setfield(L, -2, "b");

    lua_settable(L, -3);
    rgb_ini_copy_p += 3;
  }
  // Leaves one returned value on the stack, the mesh table.
  return 1;
}

int LuaSetDiffuselight(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetDiffuselight(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetDirectioncolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetDirectioncolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetFlip(lua_State *L) {
  int v = lua_toboolean(L, 1);
  int return_code = SetFlip(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaGetFlip(lua_State *L) {
  lua_pushboolean(L, GetFlip());
  return 1;
}

int LuaGetForegroundcolor(lua_State *L) {
  lua_createtable(L, 0, 3);

  lua_pushnumber(L, foregroundbasecolor[0]);
  lua_setfield(L, -2, "r");

  lua_pushnumber(L, foregroundbasecolor[1]);
  lua_setfield(L, -2, "g");

  lua_pushnumber(L, foregroundbasecolor[2]);
  lua_setfield(L, -2, "b");

  return 1;
}

int LuaSetForegroundcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetForegroundcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetHeatoffcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetHeatoffcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetHeatoncolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetHeatoncolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetIsocolors(lua_State *L) {
  float shininess = lua_tonumber(L, 1);
  float transparency = lua_tonumber(L, 2);
  int transparency_option = lua_tonumber(L, 3);
  int opacity_change = lua_tonumber(L, 4);
  float specular[3];
  GetColor(L, 5, specular);
  int n_colors = 0;
  // count the number of colours
  lua_pushnil(L); /* first key */
  while(lua_next(L, 6) != 0) {
    lua_pop(L, 1); // remove value (leave key for next iteration)
    n_colors++;
  }
  int i;
  float colors[MAX_ISO_COLORS][4];
  for(i = 1; i <= n_colors; i++) {
    if(!lua_istable(L, 6)) {
      fprintf(stderr, "isocolor table is not present\n");
      return 1;
    }
    lua_pushnumber(L, i);
    lua_gettable(L, 6);
    GetColor(L, -1, colors[i - 1]);
  }
  int return_code = SetIsocolors(shininess, transparency, transparency_option,
                                 opacity_change, specular, n_colors, colors);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetColortable(lua_State *L) {
  // int ncolors = lua_tonumber(L, 1);
  int ncolors = 0;
  int i = 0;
  // count the number of colours
  lua_pushnil(L); /* first key */
  while(lua_next(L, 1) != 0) {
    lua_pop(L, 1); // remove value (leave key for next iteration)
    ncolors++;
  }
  if(ncolors > 0) {
    // initialise arrays using the above count info
    float *colors = malloc(sizeof(float) * ncolors * 3);
    // char *names = malloc(sizeof(char)*ncolors*255);
    // char **names = malloc(sizeof(char*));
    /* table is in the stack at index 't' */
    lua_pushnil(L); /* first key */
    while(lua_next(L, 1) != 0) {
      /* uses 'key' (at index -2) and 'value' (at index -1) */
      // strncpy(names[i], lua_tostring(L, -2), 255);
      GetColor(L, -1, &colors[i]);
      /* removes 'value'; keeps 'key' for next iteration */
      lua_pop(L, 1);
      i++;
    }
    free(colors);
    // free(names);
  }
  return 0;
}

int LuaSetLightpos0(lua_State *L) {
  float a = lua_tonumber(L, 1);
  float b = lua_tonumber(L, 2);
  float c = lua_tonumber(L, 3);
  float d = lua_tonumber(L, 4);
  int return_code = SetLightpos0(a, b, c, d);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetLightpos1(lua_State *L) {
  float a = lua_tonumber(L, 1);
  float b = lua_tonumber(L, 2);
  float c = lua_tonumber(L, 3);
  float d = lua_tonumber(L, 4);
  int return_code = SetLightpos1(a, b, c, d);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSensorcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetSensorcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSensornormcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetSensornormcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetBw(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  int return_code = SetBw(a, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSprinkleroffcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetSprinkleroffcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSprinkleroncolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetSprinkleroncolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetStaticpartcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetStaticpartcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTimebarcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetTimebarcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetVentcolor(lua_State *L) {
  float r = lua_tonumber(L, 1);
  float g = lua_tonumber(L, 2);
  float b = lua_tonumber(L, 3);
  int return_code = SetVentcolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetGridlinewidth(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetGridlinewidth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetIsolinewidth(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetIsolinewidth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetIsopointsize(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetIsopointsize(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetLinewidth(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetLinewidth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetPartpointsize(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetPartpointsize(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetPlot3dlinewidth(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetPlot3dlinewidth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetPlot3dpointsize(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetPlot3dpointsize(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSensorabssize(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetSensorabssize(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSensorrelsize(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetSensorrelsize(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSliceoffset(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetSliceoffset(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSmoothlines(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetSmoothlines(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSpheresegs(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetSpheresegs(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSprinklerabssize(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetSprinklerabssize(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetStreaklinewidth(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetStreaklinewidth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTicklinewidth(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetTicklinewidth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetUsenewdrawface(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetUsenewdrawface(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetVeclength(lua_State *L) {
  float vf = lua_tonumber(L, 1);
  int vec_uniform_length_local = lua_tonumber(L, 2);
  int vec_uniform_spacing_local = lua_tonumber(L, 3);
  int return_code = SetVeclength(vf, vec_uniform_length_local, vec_uniform_spacing_local);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetVectorlinewidth(lua_State *L) {
  float a = lua_tonumber(L, 1);
  float b = lua_tonumber(L, 2);
  int return_code = SetVectorlinewidth(a, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetVectorpointsize(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetVectorpointsize(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetVentlinewidth(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetVentlinewidth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetVentoffset(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetVentoffset(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetWindowoffset(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetWindowoffset(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetWindowwidth(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetWindowwidth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetWindowheight(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetWindowheight(v);
  lua_pushnumber(L, return_code);
  return 1;
}

// --  *** DATA LOADING ***

int LuaSetBoundzipstep(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetBoundzipstep(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetIsozipstep(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetIsozipstep(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetNopart(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetNopart(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSliceaverage(lua_State *L) {
  int flag = lua_tonumber(L, 1);
  float interval = lua_tonumber(L, 2);
  int vis = lua_tonumber(L, 3);
  int return_code = SetSliceaverage(flag, interval, vis);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSlicedataout(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSlicedataout(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSlicezipstep(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSlicezipstep(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSmoke3dzipstep(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSmoke3dzipstep(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetUserrotate(lua_State *L) {
  int index = lua_tonumber(L, 1);
  int show_center = lua_tonumber(L, 2);
  float x = lua_tonumber(L, 3);
  float y = lua_tonumber(L, 4);
  float z = lua_tonumber(L, 5);
  int return_code = SetUserrotate(index, show_center, x, y, z);
  lua_pushnumber(L, return_code);
  return 1;
}

// --  *** VIEW PARAMETERS ***
int LuaSetAperture(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetAperture(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetBlocklocation(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetBlocklocation(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetBoundarytwoside(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetBoundarytwoside(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetClip(lua_State *L) {
  float v_near = lua_tonumber(L, 1);
  float v_far = lua_tonumber(L, 2);
  int return_code = SetClip(v_near, v_far);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetContourtype(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetContourtype(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetCullfaces(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetCullfaces(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTexturelighting(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetTexturelighting(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetEyeview(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetEyeview(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetEyex(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetEyex(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetEyey(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetEyey(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetEyez(lua_State *L) {
  float v = lua_tonumber(L, 1);
  int return_code = SetEyez(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetFontsize(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetFontsize(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetFrameratevalue(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetFrameratevalue(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowfacesSolid(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowfacesSolid(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowfacesOutline(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowfacesOutline(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSmoothgeomnormal(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSmoothgeomnormal(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetGeomvertexag(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetGeomvertexag(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetGversion(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetGversion(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetIsotran2(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetIsotran2(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetMeshvis(lua_State *L) {
  int n = 0;
  int i = 0;
  // count the number of values
  lua_pushnil(L);
  while(lua_next(L, 1) != 0) {
    lua_pop(L, 1); // remove value (leave key for next iteration)
    n++;
  }
  if(n > 0) {
    // initialise arrays using the above count info
    int *vals = malloc(sizeof(int) * n);
    /* table is in the stack at index 't' */
    lua_pushnil(L); /* first key */
    while(lua_next(L, 1) != 0) {
      vals[i] = lua_tonumber(L, -2);
      /* removes 'value'; keeps 'key' for next iteration */
      lua_pop(L, 1);
      i++;
    }
    int return_code = SetMeshvis(n, vals);
    lua_pushnumber(L, return_code);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaSetMeshoffset(lua_State *L) {
  int meshnum = lua_tonumber(L, 1);
  int value = lua_tonumber(L, 2);
  int return_code = SetMeshoffset(meshnum, value);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetNorthangle(lua_State *L) {
  int vis = lua_tonumber(L, 1);
  float x = lua_tonumber(L, 2);
  float y = lua_tonumber(L, 3);
  float z = lua_tonumber(L, 4);
  int return_code = SetNorthangle(vis, x, y, z);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetOffsetslice(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetOffsetslice(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetOutlinemode(lua_State *L) {
  int highlight = lua_tonumber(L, 1);
  int outline = lua_tonumber(L, 2);
  int return_code = SetOutlinemode(highlight, outline);
  lua_pushnumber(L, return_code);
  ;
  return 1;
}

int LuaSetP3dsurfacetype(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetP3dsurfacetype(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetP3dsurfacesmooth(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetP3dsurfacesmooth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetScaledfont(lua_State *L) {
  int height2d = lua_tonumber(L, 1);
  int height2dwidth = lua_tonumber(L, 2);
  int thickness2d = lua_tonumber(L, 3);
  int height3d = lua_tonumber(L, 3);
  int height3dwidth = lua_tonumber(L, 5);
  int thickness3d = lua_tonumber(L, 6);
  int return_code = SetScaledfont(height2d, height2dwidth, thickness2d,
                                  height3d, height3dwidth, thickness3d);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaGetFontsize(lua_State *L) {
  switch(fontindex) {
  case SMALL_FONT:
    lua_pushstring(L, "small");
    return 1;
    break;
  case LARGE_FONT:
    lua_pushstring(L, "large");
    return 1;
    break;
  case SCALED_FONT:
    lua_pushnumber(L, scaled_font2d_height);
    return 1;
    break;
  default:
    return luaL_error(L, "font size is invalid");
    break;
  }
}

int LuaSetScaledfontHeight2d(lua_State *L) {
  int height2d = lua_tonumber(L, 1);
  int return_code = SetScaledfontHeight2d(height2d);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowalltextures(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowalltextures(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowaxislabels(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowaxislabels(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowblocklabel(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowblocklabel(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowblocks(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowblocks(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowcadandgrid(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowcadandgrid(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowcadopaque(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowcadopaque(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowceiling(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowceiling(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowcolorbars(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowcolorbars(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowcvents(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 1);
  int return_code = SetShowcvents(a, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowdummyvents(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowdummyvents(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowfloor(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowfloor(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowframe(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowframe(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowframelabel(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowframelabel(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowframerate(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowframerate(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowgrid(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowgrid(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowgridloc(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowgridloc(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowhmstimelabel(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowhmstimelabel(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowhrrcutoff(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowhrrcutoff(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowiso(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowiso(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowisonormals(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowisonormals(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowlabels(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowlabels(v);
  lua_pushnumber(L, return_code);
  return 1;
}

#ifdef pp_memstatus
int LuaSetShowmemload(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowmemload(v);
  lua_pushnumber(L, return_code);
  return 1;
}
#endif

int LuaSetShowopenvents(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 1);
  int return_code = SetShowopenvents(a, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowothervents(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowothervents(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowsensors(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  int return_code = SetShowsensors(a, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowsliceinobst(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowsliceinobst(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowsmokepart(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowsmokepart(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowsprinkpart(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowsprinkpart(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowstreak(lua_State *L) {
  int show = lua_tonumber(L, 1);
  int step = lua_tonumber(L, 2);
  int showhead = lua_tonumber(L, 3);
  int index = lua_tonumber(L, 4);
  int return_code = SetShowstreak(show, step, showhead, index);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowterrain(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowterrain(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowthreshold(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  float c = lua_tonumber(L, 3);
  int return_code = SetShowthreshold(a, b, c);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowticks(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowticks(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtimebar(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowtimebar(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtimelabel(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowtimelabel(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtitle(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowtitle(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtracersalways(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowtracersalways(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtriangles(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  int c = lua_tonumber(L, 3);
  int d = lua_tonumber(L, 4);
  int e = lua_tonumber(L, 5);
  int f = lua_tonumber(L, 6);
  int return_code = SetShowtriangles(a, b, c, d, e, f);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtransparent(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowtransparent(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtranparentvents(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowtransparentvents(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtrianglecount(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowtrianglecount(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowventflow(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  int c = lua_tonumber(L, 3);
  int d = lua_tonumber(L, 4);
  int e = lua_tonumber(L, 5);
  int return_code = SetShowventflow(a, b, c, d, e);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowvents(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowvents(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowwalls(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowwalls(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSkipembedslice(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSkipembedslice(v);
  lua_pushnumber(L, return_code);
  return 1;
}

#ifdef pp_SLICEUP
int lua_set_slicedup(lua_State *L) {
  int scalar = lua_tonumber(L, 1);
  int vector = lua_tonumber(L, 1);
  int return_code = set_slicedup(scalar, vector);
  lua_pushnumber(L, return_code);
  return 1;
}
#endif

int LuaSetSmokesensors(lua_State *L) {
  int show = lua_tonumber(L, 1);
  int test = lua_tonumber(L, 2);
  int return_code = SetSmokesensors(show, test);
  lua_pushnumber(L, return_code);
  return 1;
}

// int set_smoothblocksolid(int v); // SMOOTHBLOCKSOLID
#ifdef pp_LANG
int lua_set_startuplang(lua_State *L) {
  const char *lang = lua_tostring(L, 1);
  int return_code = set_startuplang(lang);
  lua_pushnumber(L, return_code);
  return 1;
}
#endif

int LuaSetStereo(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetStereo(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSurfinc(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSurfinc(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTerrainparams(lua_State *L) {
  int r_min = lua_tonumber(L, 1);
  int g_min = lua_tonumber(L, 2);
  int b_min = lua_tonumber(L, 3);
  int r_max = lua_tonumber(L, 4);
  int g_max = lua_tonumber(L, 5);
  int b_max = lua_tonumber(L, 6);
  int vert_factor = lua_tonumber(L, 7);
  int return_code =
      SetTerrainparams(r_min, g_min, b_min, r_max, g_max, b_max, vert_factor);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTitlesafe(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetTitlesafe(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTrainermode(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetTrainermode(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTrainerview(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetTrainerview(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTransparent(lua_State *L) {
  int use_flag = lua_tonumber(L, 1);
  float level = lua_tonumber(L, 2);
  int return_code = SetTransparent(use_flag, level);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTreeparms(lua_State *L) {
  int minsize = lua_tonumber(L, 1);
  int visx = lua_tonumber(L, 2);
  int visy = lua_tonumber(L, 3);
  int visz = lua_tonumber(L, 4);
  int return_code = SetTreeparms(minsize, visx, visy, visz);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTwosidedvents(lua_State *L) {
  int internal = lua_tonumber(L, 1);
  int external = lua_tonumber(L, 2);
  int return_code = SetTwosidedvents(internal, external);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetVectorskip(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetVectorskip(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetVolsmoke(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  int c = lua_tonumber(L, 3);
  int d = lua_tonumber(L, 4);
  int e = lua_tonumber(L, 5);
  float f = lua_tonumber(L, 6);
  float g = lua_tonumber(L, 7);
  float h = lua_tonumber(L, 8);
  float i = lua_tonumber(L, 9);
  float j = lua_tonumber(L, 10);
  float k = lua_tonumber(L, 11);
  float l = lua_tonumber(L, 12);
  int return_code = SetVolsmoke(a, b, c, d, e, f, g, h, i, j, k, l);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetZoom(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  int return_code = SetZoom(a, b);
  lua_pushnumber(L, return_code);
  return 1;
}

// *** MISC ***
int LuaSetCellcentertext(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetCellcentertext(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetInputfile(lua_State *L) {
  const char *inputfile = lua_tostring(L, 1);
  int return_code = SetInputfile(inputfile);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetLabelstartupview(lua_State *L) {
  const char *viewname = lua_tostring(L, 1);
  int return_code = SetLabelstartupview(viewname);
  lua_pushnumber(L, return_code);
  return 1;
}

// DEPRECATED
// int lua_set_pixelskip(lua_State *L) {
//   int v = lua_tonumber(L, 1);
//   int return_code = set_pixelskip(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

int LuaSetRenderclip(lua_State *L) {
  int use_flag = lua_tonumber(L, 1);
  int left = lua_tonumber(L, 2);
  int right = lua_tonumber(L, 3);
  int bottom = lua_tonumber(L, 4);
  int top = lua_tonumber(L, 5);
  int return_code = SetRenderclip(use_flag, left, right, bottom, top);
  lua_pushnumber(L, return_code);
  return 1;
}

// DEPRECATED
// int lua_set_renderfilelabel(lua_State *L) {
//   int v = lua_tonumber(L, 1);
//   int return_code = set_renderfilelabel(v);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

int LuaSetRenderfiletype(lua_State *L) {
  int render = lua_tonumber(L, 1);
  int movie = lua_tonumber(L, 2);
  int return_code = SetRenderfiletype(render, movie);
  lua_pushnumber(L, return_code);
  return 1;
}

// int lua_set_skybox(lua_State *L){
//   return 0;
// }

// DEPRECATED
// int lua_set_renderoption(lua_State *L) {
//   int opt = lua_tonumber(L, 1);
//   int rows = lua_tonumber(L, 1);
//   int return_code = set_renderoption(opt, rows);
//   lua_pushnumber(L, return_code);
//   return 1;
// }

int LuaGetUnitDefs(lua_State *L, f_units unitclass) {
  lua_createtable(L, 0, 4);
  // Loop through all of the units
  int j;
  for(j = 0; j < unitclass.nunits; j++) {
    lua_pushstring(L, unitclass.units[j].unit);
    lua_createtable(L, 0, 4);

    lua_pushstring(L, unitclass.units[j].unit);
    lua_setfield(L, -2, "unit");

    lua_pushstring(L, "scale");
    lua_createtable(L, 0, 2);
    lua_pushnumber(L, unitclass.units[j].scale[0]);
    lua_setfield(L, -2, "factor");
    lua_pushnumber(L, unitclass.units[j].scale[1]);
    lua_setfield(L, -2, "offset");
    lua_settable(L, -3);

    lua_pushstring(L, unitclass.units[j].rel_val);
    lua_setfield(L, -2, "rel_val");

    lua_pushboolean(L, unitclass.units[j].rel_defined);
    lua_setfield(L, -2, "rel_defined");

    lua_settable(L, -3);
  }
  return 1;
}

// TODO: implement iterators for this table
int LuaGetUnitclass(lua_State *L) {
  const char *classname = lua_tostring(L, 1);
  int i;
  for(i = 0; i < nunitclasses_default; i++) {
    // if the classname matches, put a table on the stack
    if(strcmp(classname, unitclasses_default[i].unitclass) == 0) {
      lua_createtable(L, 0, 4);
      // Loop through all of the units
      LuaGetUnitDefs(L, unitclasses_default[i]);
      return 1;
    }
  }
  return 0;
}

int LuaGetUnits(lua_State *L) {
  const char *classname = lua_tostring(L, 1);
  int i;
  for(i = 0; i < nunitclasses_default; i++) {
    // if the classname matches, put a table on the stack
    if(strcmp(classname, unitclasses_default[i].unitclass) == 0) {
      // lua_createtable(L, 0, 4);
      // // Loop through all of the units
      // lua_get_units(L, unitclasses_default[i]);
      lua_pushstring(
          L,
          unitclasses_default[i].units[unitclasses_default[i].unit_index].unit);
      return 1;
    }
  }
  return 0;
}

int LuaSetUnits(lua_State *L) {
  const char *unitclassname = lua_tostring(L, 1);
  const char *unitname = lua_tostring(L, 2);

  size_t unitclass_index;
  bool unit_class_found = false;
  size_t unit_index;
  bool unit_index_found = false;
  for(size_t i = 0; i < nunitclasses_default; i++) {
    if(strcmp(unitclasses[i].unitclass, unitclassname) == 0) {
      unitclass_index = i;
      unit_class_found = true;
      break;
    }
  }
  if(!unit_class_found) {
    return luaL_error(L, "unit class index not found");
  }
  for(size_t i = 0; i < unitclasses[unitclass_index].nunits; i++) {
    if(strcmp(unitclasses[unitclass_index].units[i].unit, unitname) == 0) {
      unit_index = i;
      unit_index_found = true;
      break;
    }
  }
  if(!unit_index_found) {
    return luaL_error(L, "unit index not found");
  }
  SetUnits(unitclass_index, unit_index);
  return 0;
}

int LuaSetUnitclasses(lua_State *L) {
  int i = 0;
  int n = 0;
  if(!lua_istable(L, -1)) {
    fprintf(stderr, "stack is not a table at index\n");
    exit(1);
  }
  lua_pushnil(L);
  while(lua_next(L, -2) != 0) {
    lua_pop(L, 1);
    n++;
  }
  if(n > 0) {
    int *indices = malloc(sizeof(int) * n);
    lua_pushnil(L);
    while(lua_next(L, -2) != 0) {
      indices[i] = lua_tonumber(L, -1);
      lua_pop(L, 1);
      i++;
    }
    int return_code = SetUnitclasses(n, indices);
    lua_pushnumber(L, return_code);
    free(indices);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaSetZaxisangles(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  int c = lua_tonumber(L, 3);
  int return_code = SetZaxisangles(a, b, c);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetColorbartype(lua_State *L) {
  int type = lua_tonumber(L, 1);
  const char *label = lua_tostring(L, 2);
  int return_code = SetColorbartype(type, label);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetExtremecolors(lua_State *L) {
  int rmin = lua_tonumber(L, 1);
  int gmin = lua_tonumber(L, 2);
  int bmin = lua_tonumber(L, 3);
  int rmax = lua_tonumber(L, 4);
  int gmax = lua_tonumber(L, 5);
  int bmax = lua_tonumber(L, 6);
  int return_code = SetExtremecolors(rmin, gmin, bmin, rmax, gmax, bmax);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetFirecolor(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetFirecolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetFirecolormap(lua_State *L) {
  int type = lua_tonumber(L, 1);
  int index = lua_tonumber(L, 2);
  int return_code = SetFirecolormap(type, index);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetFiredepth(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetFiredepth(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowextremedata(lua_State *L) {
  int show_extremedata = lua_tonumber(L, 1);
  int below = lua_tonumber(L, 2);
  int above = lua_tonumber(L, 3);
  int return_code = SetShowextremedata(show_extremedata, below, above);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSmokecolor(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetSmokecolor(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSmokecull(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSmokecull(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSmokeskip(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSmokeskip(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSmokealbedo(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSmokealbedo(v);
  lua_pushnumber(L, return_code);
  return 1;
}

#ifdef pp_GPU
int LuaSetSmokerthick(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetSmokerthick(v);
  lua_pushnumber(L, return_code);
  return 1;
}
#endif

#ifdef pp_GPU
int LuaSetUsegpu(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetUsegpu(v);
  lua_pushnumber(L, return_code);
  return 1;
}
#endif

// *** ZONE FIRE PARAMETRES ***
int LuaSetShowhazardcolors(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowhazardcolors(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowhzone(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowhzone(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowszone(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowszone(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowvzone(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowvzone(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowzonefire(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowzonefire(v);
  lua_pushnumber(L, return_code);
  return 1;
}

// *** TOUR INFO ***
int LuaSetShowpathnodes(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowpathnodes(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetShowtourroute(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowtourroute(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetTourcolorsSelectedpathline(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetTourcolorsSelectedpathline(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}
int LuaSetTourcolorsSelectedpathlineknots(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetTourcolorsSelectedpathlineknots(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}
int LuaSetTourcolorsSelectedknot(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetTourcolorsSelectedknot(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}
int LuaSetTourcolorsPathline(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetTourcolorsSelectedpathline(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}
int LuaSetTourcolorsPathknots(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetTourcolorsPathknots(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}
int LuaSetTourcolorsText(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetTourcolorsText(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}
int LuaSetTourcolorsAvatar(lua_State *L) {
  int r = lua_tonumber(L, 1);
  int g = lua_tonumber(L, 2);
  int b = lua_tonumber(L, 3);
  int return_code = SetTourcolorsAvatar(r, g, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetViewalltours(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetViewalltours(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetViewtimes(lua_State *L) {
  float start = lua_tonumber(L, 1);
  float stop = lua_tonumber(L, 2);
  int ntimes = lua_tonumber(L, 3);
  int return_code = SetViewtimes(start, stop, ntimes);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetViewtourfrompath(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetViewtourfrompath(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetDevicevectordimensions(lua_State *L) {
  float baselength = lua_tonumber(L, 1);
  float basediameter = lua_tonumber(L, 2);
  float headlength = lua_tonumber(L, 3);
  float headdiameter = lua_tonumber(L, 4);
  int return_code = SetDevicevectordimensions(baselength, basediameter,
                                              headlength, headdiameter);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetDevicebounds(lua_State *L) {
  float min = lua_tonumber(L, 1);
  float max = lua_tonumber(L, 2);
  int return_code = SetDevicebounds(min, max);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetDeviceorientation(lua_State *L) {
  int a = lua_tonumber(L, 1);
  float b = lua_tonumber(L, 2);
  int return_code = SetDeviceorientation(a, b);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetGridparms(lua_State *L) {
  int vx = lua_tonumber(L, 1);
  int vy = lua_tonumber(L, 2);
  int vz = lua_tonumber(L, 3);
  int px = lua_tonumber(L, 4);
  int py = lua_tonumber(L, 5);
  int pz = lua_tonumber(L, 6);
  int return_code = SetGridparms(vx, vy, vz, px, py, pz);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetGsliceparms(lua_State *L) {
  int i;
  int vis_data = lua_tonumber(L, 1);
  int vis_triangles = lua_tonumber(L, 2);
  int vis_triangulation = lua_tonumber(L, 3);
  int vis_normal = lua_tonumber(L, 4);
  float xyz[3];
  // TODO: use named fields (e.g. xyz)
  for(i = 0; i < 3; i++) {
    lua_pushnumber(L, i);
    lua_gettable(L, 5);
    xyz[i] = lua_tonumber(L, -1);
    lua_pop(L, 1);
    i++;
  }
  float azelev[2];
  for(i = 0; i < 2; i++) {
    lua_pushnumber(L, i);
    lua_gettable(L, 6);
    azelev[i] = lua_tonumber(L, -1);
    lua_pop(L, 1);
    i++;
  }
  int return_code = SetGsliceparms(vis_data, vis_triangles, vis_triangulation,
                                   vis_normal, xyz, azelev);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetLoadfilesatstartup(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetLoadfilesatstartup(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetMscale(lua_State *L) {
  float a = lua_tonumber(L, 1);
  float b = lua_tonumber(L, 2);
  float c = lua_tonumber(L, 3);
  int return_code = SetMscale(a, b, c);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetSliceauto(lua_State *L) {
  lua_pushnil(L);
  int n = 0;
  while(lua_next(L, -2) != 0) {
    lua_pop(L, 1);
    n++;
  }
  if(n > 0) {
    int i = 0;
    int *vals = malloc(sizeof(int) * n);
    lua_pushnil(L);
    while(lua_next(L, -2) != 0) {
      vals[i] = lua_tonumber(L, -1);
      lua_pop(L, 1);
      i++;
    }
    int return_code = SetSliceauto(n, vals);
    lua_pushnumber(L, return_code);
    free(vals);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaSetMsliceauto(lua_State *L) {
  lua_pushnil(L);
  int n = 0;
  while(lua_next(L, -2) != 0) {
    lua_pop(L, 1);
    n++;
  }
  if(n > 0) {
    int i = 0;
    int *vals = malloc(sizeof(int) * n);
    lua_pushnil(L);
    while(lua_next(L, -2) != 0) {
      vals[i] = lua_tonumber(L, -1);
      lua_pop(L, 1);
      i++;
    }
    int return_code = SetMsliceauto(n, vals);
    lua_pushnumber(L, return_code);
    free(vals);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaSetCompressauto(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetCompressauto(v);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetPropindex(lua_State *L) {
  lua_pushnil(L);
  int n = 0;
  while(lua_next(L, -2) != 0) {
    lua_pop(L, 1);
    n++;
  }
  if(n > 0) {
    int i = 0;
    int *vals = malloc(sizeof(int) * n * PROPINDEX_STRIDE);
    lua_pushnil(L);
    while(lua_next(L, -2) != 0) {
      lua_pushnumber(L, 1);
      lua_gettable(L, -2);
      *(vals + (i * PROPINDEX_STRIDE + 0)) = lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_pushnumber(L, 1);
      lua_gettable(L, -2);
      *(vals + (i * PROPINDEX_STRIDE + 1)) = lua_tonumber(L, -1);
      lua_pop(L, 1);

      lua_pop(L, 1);
      i++;
    }
    int return_code = SetPropindex(n, vals);
    lua_pushnumber(L, return_code);
    return 1;
  }
  else {
    return 0;
  }
}

int LuaSetShowdevices(lua_State *L) {
  lua_pushnil(L);
  int n = 0;
  while(lua_next(L, -2) != 0) {
    lua_pop(L, 1);
    n++;
  }
  if(n > 0) {
    int i = 0;
    const char **names = malloc(sizeof(char *) * n);
    lua_pushnil(L);
    while(lua_next(L, -2) != 0) {
      names[i] = lua_tostring(L, -1);
      lua_pop(L, 1);
      i++;
    }
    int return_code = SetShowdevices(n, names);
    lua_pushnumber(L, return_code);
    free(names);
    return 1;
  }
  else {
    return 0;
  }
} // SHOWDEVICES

int LuaSetShowdevicevals(lua_State *L) {
  int a = lua_tonumber(L, 1);
  int b = lua_tonumber(L, 2);
  int c = lua_tonumber(L, 3);
  int d = lua_tonumber(L, 4);
  int e = lua_tonumber(L, 5);
  int f = lua_tonumber(L, 6);
  int g = lua_tonumber(L, 7);
  int h = lua_tonumber(L, 8);
  int return_code = SetShowdevicevals(a, b, c, d, e, f, g, h);
  lua_pushnumber(L, return_code);
  return 1;
} // SHOWDEVICEVALS

int LuaSetShowmissingobjects(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetShowmissingobjects(v);
  lua_pushnumber(L, return_code);
  return 1;
} // SHOWMISSINGOBJECTS

int LuaSetTourindex(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetTourindex(v);
  lua_pushnumber(L, return_code);
  return 1;
} // TOURINDEX

int LuaSetCParticles(lua_State *L) {
  int min_flag = lua_tonumber(L, 1);
  float min_value = lua_tonumber(L, 2);
  int max_flag = lua_tonumber(L, 3);
  float max_value = lua_tonumber(L, 4);
  const char *label = NULL;
  if(lua_gettop(L) == 5) {
    label = lua_tostring(L, 5);
  }
  int return_code =
      SetCParticles(min_flag, min_value, max_flag, max_value, label);
  lua_pushnumber(L, return_code);
  return 1;
} // C_PARTICLES

int LuaSetCSlice(lua_State *L) {
  int min_flag = lua_tonumber(L, 1);
  float min_value = lua_tonumber(L, 2);
  int max_flag = lua_tonumber(L, 3);
  float max_value = lua_tonumber(L, 4);
  const char *label = NULL;
  if(lua_gettop(L) == 5) {
    label = lua_tostring(L, 5);
  }
  int return_code = SetCSlice(min_flag, min_value, max_flag, max_value, label);
  lua_pushnumber(L, return_code);
  return 1;
} // C_SLICE

int LuaSetCacheBoundarydata(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetCacheBoundarydata(v);
  lua_pushnumber(L, return_code);
  return 1;
} // CACHE_BOUNDARYDATA

int LuaSetCacheQdata(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetCacheQdata(v);
  lua_pushnumber(L, return_code);
  return 1;
} // CACHE_QDATA

#ifdef pp_HIST
int LuaSetPercentilelevel(lua_State *L) {
  float p_level_min = lua_tonumber(L, 1);
  float p_level_max = lua_tonumber(L, 2);
  int return_code = SetPercentilelevel(p_level_min, p_level_max);
  lua_pushnumber(L, return_code);
  return 1;
} // PERCENTILELEVEL
#endif

int LuaSetTimeoffset(lua_State *L) {
  int v = lua_tonumber(L, 1);
  int return_code = SetTimeoffset(v);
  lua_pushnumber(L, return_code);
  return 1;
} // TIMEOFFSET

int LuaSetTload(lua_State *L) {
  int begin_flag = lua_tonumber(L, 1);
  float begin_val = lua_tonumber(L, 2);
  int end_flag = lua_tonumber(L, 3);
  float end_val = lua_tonumber(L, 4);
  int skip_flag = lua_tonumber(L, 5);
  float skip_val = lua_tonumber(L, 6);
  int return_code =
      SetTload(begin_flag, begin_val, end_flag, end_val, skip_flag, skip_val);
  lua_pushnumber(L, return_code);
  return 1;
} // TLOAD

int LuaSetVSlice(lua_State *L) {
  int min_flag = lua_tonumber(L, 1);
  float min_value = lua_tonumber(L, 2);
  int max_flag = lua_tonumber(L, 3);
  float max_value = lua_tonumber(L, 4);
  const char *label = lua_tostring(L, 5);
  float line_min = lua_tonumber(L, 6);
  float line_max = lua_tonumber(L, 7);
  int line_num = lua_tonumber(L, 8);
  int return_code = SetVSlice(min_flag, min_value, max_flag, max_value, label,
                              line_min, line_max, line_num);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaSetPatchdataout(lua_State *L) {
  int output_flag = lua_tonumber(L, 1);
  int tmin = lua_tonumber(L, 1);
  int tmax = lua_tonumber(L, 2);
  int xmin = lua_tonumber(L, 3);
  int xmax = lua_tonumber(L, 4);
  int ymin = lua_tonumber(L, 5);
  int ymax = lua_tonumber(L, 6);
  int zmin = lua_tonumber(L, 7);
  int zmax = lua_tonumber(L, 8);
  int return_code = SetPatchdataout(output_flag, tmin, tmax, xmin, xmax, ymin,
                                    ymax, zmin, zmax);
  lua_pushnumber(L, return_code);
  return 1;
} // PATCHDATAOUT

int LuaShowSmoke3dShowall(lua_State *L) {
  int return_code = ShowSmoke3dShowall();
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaShowSmoke3dHideall(lua_State *L) {
  int return_code = ShowSmoke3dHideall();
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaShowSlicesShowall(lua_State *L) {
  int return_code = ShowSlicesShowall();
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaShowSlicesHideall(lua_State *L) {
  int return_code = ShowSlicesHideall();
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaAddTitleLine(lua_State *L) {
  const char *string = lua_tostring(L, 1);
  int return_code = addTitleLine(&titleinfo, string);
  lua_pushnumber(L, return_code);
  return 1;
}

int LuaClearTitleLines(lua_State *L) {
  int return_code = clearTitleLines(&titleinfo);
  lua_pushnumber(L, return_code);
  return 1;
}

/// @brief Add paths for the Lua interpreter to find scripts and libraries that
/// are written in Lua.
/// @param L The Lua interpreter state
void AddScriptPath(lua_State *L) {
  char *smv_bindir = GetSmvRootDir();
  // package.path is a path variable where Lua scripts and modules may be
  // found, typically text based files with the .lua extension.
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "path");
  const char *original_path = lua_tostring(L, -1);
  int new_length = strlen(original_path) + 1;
#ifdef pp_LINUX
  // Add script path for the linux install
  char *linux_share_path = ";/usr/share/smokeview/?.lua";
  new_length += strlen(linux_share_path);
#endif
  // Add the location of the smokeview binary as a place for scripts. This is
  // mostly useful for running tests.
  char *bin_path = malloc(sizeof(char) * (strlen(smv_bindir) + 8));
  sprintf(bin_path, ";%s/?.lua", smv_bindir);
  new_length += strlen(bin_path);
  // Create the path.
  char *new_path = malloc(sizeof(char) * new_length);
  strcpy(new_path, original_path);
#ifdef pp_LINUX
  strcat(new_path, linux_share_path);
#endif
  strcat(new_path, bin_path);
  lua_pushstring(L, new_path);
  lua_setfield(L, -3, "path");
  lua_pop(L, 1); // pop the now redundant "path" variable from the stack
  lua_pop(L, 1); // pop the now redundant "package" variable from the stack
  free(new_path);
  free(bin_path);
  FREEMEMORY(smv_bindir);
}

/// @brief Add paths for the Lua interpreter to find libraries that are compiled
/// to shared libraries.
/// @param L The Lua interpreter state
void AddCPath(lua_State *L) {
  char *smv_bindir = GetSmvRootDir();
  // package.path is a path variable where Lua scripts and modules may be
  // found, typically text based files with the .lua extension.
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "cpath");
  const char *original_path = lua_tostring(L, -1);
  int new_length = strlen(original_path) + 1;
#ifdef pp_LINUX
  char *so_extension = ".so";
#else
  char *so_extension = ".dll";
#endif
  // Add the location of the smokeview binary as a place for scripts. This is
  // mostly useful for running tests.
  char *bin_path = malloc(sizeof(char) * (strlen(smv_bindir) + 8));
  sprintf(bin_path, ";%s/?%s", smv_bindir, so_extension);
  new_length += strlen(bin_path);
  // Create the path.
  char *new_path = malloc(sizeof(char) * new_length);
  strcpy(new_path, original_path);
  strcat(new_path, bin_path);
  lua_pushstring(L, new_path);
  lua_setfield(L, -3, "cpath");
  lua_pop(L, 1); // pop the now redundant "path" variable from the stack
  lua_pop(L, 1); // pop the now redundant "package" variable from the stack
  free(new_path);
  free(bin_path);
  FREEMEMORY(smv_bindir);
}

/// @brief Add paths for the Lua interpreter to find scripts and libraries.
/// @param L The Lua interpreter state
void AddLuaPaths(lua_State *L) {
  // Add the paths for *.lua files.
  AddScriptPath(L);
  // Ad the path for native (*.dll, and *.so) libs
  AddCPath(L);
  return;
}

static luaL_Reg const SMVLIB[] = {
    {"set_slice_bounds", LuaSetSliceBounds},
    {"set_slice_bound_min", LuaSetSliceBoundMin},
    {"set_slice_bound_max", LuaSetSliceBoundMax},
    {"get_slice_bounds", LuaGetSliceBounds},
    {"loadsmvall", LuaLoadsmvall},
    {"hidewindow", LuaHidewindow},
    {"yieldscript", LuaYieldscript},
    {"tempyieldscript", LuaTempyieldscript},
    {"displayCB", LuaDisplayCb},
    {"renderclip", LuaRenderclip},
    {"renderC", LuaRender},
    {"render_var", LuaRenderVar},
    {"gsliceview", LuaGsliceview},
    {"showplot3ddata", LuaShowplot3ddata},
    {"gslicepos", LuaGslicepos},
    {"gsliceorien", LuaGsliceorien},
    {"settourkeyframe", LuaSettourkeyframe},
    {"settourview", LuaSettourview},
    {"getframe", LuaGetframe},
    {"setframe", LuaSetframe},
    {"gettime", LuaGettime},
    {"settime", LuaSettime},
    {"loaddatafile", LuaLoaddatafile},
    {"loadinifile", LuaLoadinifile},
    {"loadvdatafile", LuaLoadvdatafile},
    {"loadboundaryfile", LuaLoadboundaryfile},
    {"load3dsmoke", LuaLoad3dsmoke},
    {"loadvolsmoke", LuaLoadvolsmoke},
    {"loadvolsmokeframe", LuaLoadvolsmokeframe},
    {"set_rendertype", LuaSetRendertype},
    {"get_rendertype", LuaGetRendertype},
    {"set_movietype", LuaSetMovietype},
    {"get_movietype", LuaGetMovietype},
    {"makemovie", LuaMakemovie},
    {"loadtour", LuaLoadtour},
    {"loadparticles", LuaLoadparticles},
    {"partclasscolor", LuaPartclasscolor},
    {"partclasstype", LuaPartclasstype},
    {"plot3dprops", LuaPlot3dprops},
    {"loadplot3d", LuaLoadplot3d},
    {"loadslice", LuaLoadslice},
    {"loadsliceindex", LuaLoadsliceindex},
    {"loadvslice", LuaLoadvslice},
    {"loadiso", LuaLoadiso},
    {"unloadall", LuaUnloadall},
    {"unloadtour", LuaUnloadtour},
    {"setrenderdir", LuaSetrenderdir},
    {"getrenderdir", LuaGetrenderdir},
    {"set_ortho_preset", LuaSetOrthoPreset},
    {"setviewpoint", LuaSetviewpoint},
    {"getviewpoint", LuaGetviewpoint},
    {"exit", LuaExitSmokeview},
    {"getcolorbarflip", LuaGetcolorbarflip},
    {"setcolorbarflip", LuaSetcolorbarflip},
    {"setwindowsize", LuaSetwindowsize},
    // {"window.setwindowsize", lua_setwindowsize},
    {"setgridvisibility", LuaSetgridvisibility},
    {"setgridparms", LuaSetgridparms},
    {"setcolorbarindex", LuaSetcolorbarindex},
    {"getcolorbarindex", LuaGetcolorbarindex},

    {"set_slice_in_obst", LuaSetSliceInObst},
    {"get_slice_in_obst", LuaGetSliceInObst},

    // colorbar
    {"set_colorbar", LuaSetColorbar},
    {"set_named_colorbar", LuaSetNamedColorbar},
    // {"get_named_colorbar", lua_get_named_colorbar},

    {"set_colorbar_visibility", LuaSetColorbarVisibility},
    {"get_colorbar_visibility", LuaGetColorbarVisibility},
    {"toggle_colorbar_visibility", LuaToggleColorbarVisibility},

    {"set_colorbar_visibility_horizontal", LuaSetColorbarVisibilityHorizontal},
    {"get_colorbar_visibility_horizontal", LuaGetColorbarVisibilityHorizontal},
    {"toggle_colorbar_visibility_horizontal",
     LuaToggleColorbarVisibilityHorizontal},

    {"set_colorbar_visibility_vertical", LuaSetColorbarVisibilityVertical},
    {"get_colorbar_visibility_vertical", LuaGetColorbarVisibilityVertical},
    {"toggle_colorbar_visibility_vertical",
     LuaToggleColorbarVisibilityVertical},

    // timebar
    {"set_timebar_visibility", LuaSetTimebarVisibility},
    {"get_timebar_visibility", LuaGetTimebarVisibility},
    {"toggle_timebar_visibility", LuaToggleTimebarVisibility},

    // title
    {"set_title_visibility", LuaSetTitleVisibility},
    {"get_title_visibility", LuaGetTitleVisibility},
    {"toggle_title_visibility", LuaToggleTitleVisibility},

    // smv_version
    {"set_smv_version_visibility", LuaSetSmvVersionVisibility},
    {"get_smv_version_visibility", LuaGetSmvVersionVisibility},
    {"toggle_smv_version_visibility", LuaToggleSmvVersionVisibility},

    // chid
    {"set_chid_visibility", LuaSetChidVisibility},
    {"get_chid_visibility", LuaGetChidVisibility},
    {"toggle_chid_visibility", LuaToggleChidVisibility},

    // blockages
    {"blockages_hide_all", LuaBlockagesHideAll},
    // {"get_chid_visibility", lua_get_chid_visibility},
    // {"toggle_chid_visibility", lua_toggle_chid_visibility},

    // outlines
    {"outlines_show", LuaOutlinesShow},
    {"outlines_hide", LuaOutlinesHide},

    // surfaces
    {"surfaces_hide_all", LuaSurfacesHideAll},
    // devices
    {"devices_hide_all", LuaDevicesHideAll},

    // axis
    {"set_axis_visibility", LuaSetAxisVisibility},
    {"get_axis_visibility", LuaGetAxisVisibility},
    {"toggle_axis_visibility", LuaToggleAxisVisibility},

    // frame label
    {"set_framelabel_visibility", LuaSetFramelabelVisibility},
    {"get_framelabel_visibility", LuaGetFramelabelVisibility},
    {"toggle_framelabel_visibility", LuaToggleFramelabelVisibility},

    // framerate
    {"set_framerate_visibility", LuaSetFramerateVisibility},
    {"get_framerate_visibility", LuaGetFramerateVisibility},
    {"toggle_framerate_visibility", LuaToggleFramerateVisibility},

    // grid locations
    {"set_gridloc_visibility", LuaSetGridlocVisibility},
    {"get_gridloc_visibility", LuaGetGridlocVisibility},
    {"toggle_gridloc_visibility", LuaToggleGridlocVisibility},

    // hrrpuv cutoff
    {"set_hrrcutoff_visibility", LuaSetHrrcutoffVisibility},
    {"get_hrrcutoff_visibility", LuaGetHrrcutoffVisibility},
    {"toggle_hrrcutoff_visibility", LuaToggleHrrcutoffVisibility},

    // hrr label
    {"set_hrrlabel_visibility", LuaSetHrrlabelVisibility},
    {"get_hrrlabel_visibility", LuaGetHrrlabelVisibility},
    {"toggle_hrrlabel_visibility", LuaToggleHrrlabelVisibility},

// memory load
#ifdef pp_memstatus
    {"set_memload_visibility", LuaSetMemloadVisibility},
    {"get_memload_visibility", LuaGetMemloadVisibility},
    {"toggle_memload_visibility", LuaToggleMemloadVisibility},
#endif

    // mesh label
    {"set_meshlabel_visibility", LuaSetMeshlabelVisibility},
    {"get_meshlabel_visibility", LuaGetMeshlabelVisibility},
    {"toggle_meshlabel_visibility", LuaToggleMeshlabelVisibility},

    // slice average
    {"set_slice_average_visibility", LuaSetSliceAverageVisibility},
    {"get_slice_average_visibility", LuaGetSliceAverageVisibility},
    {"toggle_slice_average_visibility", LuaToggleSliceAverageVisibility},

    // time
    {"set_time_visibility", LuaSetTimeVisibility},
    {"get_time_visibility", LuaGetTimeVisibility},
    {"toggle_time_visibility", LuaToggleTimeVisibility},

    // user settable ticks
    {"set_user_ticks_visibility", LuaSetUserTicksVisibility},
    {"get_user_ticks_visibility", LuaGetUserTicksVisibility},
    {"toggle_user_ticks_visibility", LuaToggleUserTicksVisibility},

    // version info
    {"set_version_info_visibility", LuaSetVersionInfoVisibility},
    {"get_version_info_visibility", LuaGetVersionInfoVisibility},
    {"toggle_version_info_visibility", LuaToggleVersionInfoVisibility},

    // set all
    {"set_all_label_visibility", LuaSetAllLabelVisibility},

    // set the blockage view method
    {"blockage_view_method", LuaBlockageViewMethod},
    {"blockage_outline_color", LuaBlockageOutlineColor},
    {"blockage_locations", LuaBlockageLocations},

    {"set_colorbar_colors", LuaSetColorbarColors},
    {"get_colorbar_colors", LuaGetColorbarColors},
    {"set_color2bar_colors", LuaSetColor2barColors},
    {"get_color2bar_colors", LuaGetColor2barColors},

    // Camera API
    {"camera_mod_eyex", LuaCameraModEyex},
    {"camera_set_eyex", LuaCameraSetEyex},
    {"camera_get_eyex", LuaCameraGetEyex},

    {"camera_mod_eyey", LuaCameraModEyey},
    {"camera_set_eyey", LuaCameraSetEyey},
    {"camera_get_eyey", LuaCameraGetEyey},

    {"camera_mod_eyez", LuaCameraModEyez},
    {"camera_set_eyez", LuaCameraSetEyez},
    {"camera_get_eyez", LuaCameraGetEyez},

    {"camera_mod_az", LuaCameraModAz},
    {"camera_set_az", LuaCameraSetAz},
    {"camera_get_az", LuaCameraGetAz},
    {"camera_mod_elev", LuaCameraModElev},
    {"camera_zoom_to_fit", LuaCameraZoomToFit},
    {"camera_set_elev", LuaCameraSetElev},
    {"camera_get_elev", LuaCameraGetElev},

    {"camera_set_viewdir", LuaCameraSetViewdir},
    {"camera_get_viewdir", LuaCameraGetViewdir},

    {"camera_get_zoom", LuaCameraGetZoom},
    {"camera_set_zoom", LuaCameraSetZoom},

    {"camera_get_rotation_type", LuaCameraGetRotationType},
    {"camera_get_rotation_index", LuaCameraGetRotationIndex},
    {"camera_set_rotation_type", LuaCameraSetRotationType},
    {"camera_get_projection_type", LuaCameraGetProjectionType},
    {"camera_set_projection_type", LuaCameraSetProjectionType},

    {"get_clipping_mode", LuaGetClippingMode},
    {"set_clipping_mode", LuaSetClippingMode},
    {"set_sceneclip_x", LuaSetSceneclipX},
    {"set_sceneclip_x_min", LuaSetSceneclipXMin},
    {"set_sceneclip_x_max", LuaSetSceneclipXMax},
    {"set_sceneclip_y", LuaSetSceneclipY},
    {"set_sceneclip_y_min", LuaSetSceneclipYMin},
    {"set_sceneclip_y_max", LuaSetSceneclipYMax},
    {"set_sceneclip_z", LuaSetSceneclipZ},
    {"set_sceneclip_z_min", LuaSetSceneclipZMin},
    {"set_sceneclip_z_max", LuaSetSceneclipZMax},

    {"set_ambientlight", LuaSetAmbientlight},
    {"get_backgroundcolor", LuaGetBackgroundcolor},
    {"set_backgroundcolor", LuaSetBackgroundcolor},
    {"set_blockcolor", LuaSetBlockcolor},
    {"set_blockshininess", LuaSetBlockshininess},
    {"set_blockspecular", LuaSetBlockspecular},
    {"set_boundcolor", LuaSetBoundcolor},
    {"set_diffuselight", LuaSetDiffuselight},
    {"set_directioncolor", LuaSetDirectioncolor},
    {"get_flip", LuaGetFlip},
    {"set_flip", LuaSetFlip},
    {"get_foregroundcolor", LuaGetForegroundcolor},
    {"set_foregroundcolor", LuaSetForegroundcolor},
    {"set_heatoffcolor", LuaSetHeatoffcolor},
    {"set_heatoncolor", LuaSetHeatoncolor},
    {"set_isocolors", LuaSetIsocolors},
    {"set_colortable", LuaSetColortable},
    {"set_lightpos0", LuaSetLightpos0},
    {"set_lightpos1", LuaSetLightpos1},
    {"set_sensorcolor", LuaSetSensorcolor},
    {"set_sensornormcolor", LuaSetSensornormcolor},
    {"set_bw", LuaSetBw},
    {"set_sprinkleroffcolor", LuaSetSprinkleroffcolor},
    {"set_sprinkleroncolor", LuaSetSprinkleroncolor},
    {"set_staticpartcolor", LuaSetStaticpartcolor},
    {"set_timebarcolor", LuaSetTimebarcolor},
    {"set_ventcolor", LuaSetVentcolor},
    {"set_gridlinewidth", LuaSetGridlinewidth},
    {"set_isolinewidth", LuaSetIsolinewidth},
    {"set_isopointsize", LuaSetIsopointsize},
    {"set_linewidth", LuaSetLinewidth},
    {"set_partpointsize", LuaSetPartpointsize},
    {"set_plot3dlinewidth", LuaSetPlot3dlinewidth},
    {"set_plot3dpointsize", LuaSetPlot3dpointsize},
    {"set_sensorabssize", LuaSetSensorabssize},
    {"set_sensorrelsize", LuaSetSensorrelsize},
    {"set_sliceoffset", LuaSetSliceoffset},
    {"set_smoothlines", LuaSetSmoothlines},
    {"set_spheresegs", LuaSetSpheresegs},
    {"set_sprinklerabssize", LuaSetSprinklerabssize},
    {"set_streaklinewidth", LuaSetStreaklinewidth},
    {"set_ticklinewidth", LuaSetTicklinewidth},
    {"set_usenewdrawface", LuaSetUsenewdrawface},
    {"set_veclength", LuaSetVeclength},
    {"set_vectorlinewidth", LuaSetVectorlinewidth},
    {"set_vectorpointsize", LuaSetVectorpointsize},
    {"set_ventlinewidth", LuaSetVentlinewidth},
    {"set_ventoffset", LuaSetVentoffset},
    {"set_windowoffset", LuaSetWindowoffset},
    {"set_windowwidth", LuaSetWindowwidth},
    {"set_windowheight", LuaSetWindowheight},

    {"set_boundzipstep", LuaSetBoundzipstep},
    {"set_isozipstep", LuaSetIsozipstep},
    {"set_nopart", LuaSetNopart},
    {"set_sliceaverage", LuaSetSliceaverage},
    {"set_slicedataout", LuaSetSlicedataout},
    {"set_slicezipstep", LuaSetSlicezipstep},
    {"set_smoke3dzipstep", LuaSetSmoke3dzipstep},
    {"set_userrotate", LuaSetUserrotate},

    {"set_aperture", LuaSetAperture},
    // { "set_axissmooth", lua_set_axissmooth },
    {"set_blocklocation", LuaSetBlocklocation},
    {"set_boundarytwoside", LuaSetBoundarytwoside},
    {"set_clip", LuaSetClip},
    {"set_contourtype", LuaSetContourtype},
    {"set_cullfaces", LuaSetCullfaces},
    {"set_texturelighting", LuaSetTexturelighting},
    {"set_eyeview", LuaSetEyeview},
    {"set_eyex", LuaSetEyex},
    {"set_eyey", LuaSetEyey},
    {"set_eyez", LuaSetEyez},
    {"get_fontsize", LuaGetFontsize},
    {"set_fontsize", LuaSetFontsize},
    {"set_frameratevalue", LuaSetFrameratevalue},
    {"set_showfaces_solid", LuaSetShowfacesSolid},
    {"set_showfaces_outline", LuaSetShowfacesOutline},
    {"set_smoothgeomnormal", LuaSetSmoothgeomnormal},
    {"set_geomvertexag", LuaSetGeomvertexag},
    {"set_gversion", LuaSetGversion},
    {"set_isotran2", LuaSetIsotran2},
    {"set_meshvis", LuaSetMeshvis},
    {"set_meshoffset", LuaSetMeshoffset},

    {"set_northangle", LuaSetNorthangle},
    {"set_offsetslice", LuaSetOffsetslice},
    {"set_outlinemode", LuaSetOutlinemode},
    {"set_p3dsurfacetype", LuaSetP3dsurfacetype},
    {"set_p3dsurfacesmooth", LuaSetP3dsurfacesmooth},
    {"set_scaledfont", LuaSetScaledfont},
    {"set_scaledfont_height2d", LuaSetScaledfontHeight2d},
    {"set_showalltextures", LuaSetShowalltextures},
    {"set_showaxislabels", LuaSetShowaxislabels},
    {"set_showblocklabel", LuaSetShowblocklabel},
    {"set_showblocks", LuaSetShowblocks},
    {"set_showcadandgrid", LuaSetShowcadandgrid},
    {"set_showcadopaque", LuaSetShowcadopaque},
    {"set_showceiling", LuaSetShowceiling},
    {"set_showcolorbars", LuaSetShowcolorbars},
    {"set_showcvents", LuaSetShowcvents},
    {"set_showdummyvents", LuaSetShowdummyvents},
    {"set_showfloor", LuaSetShowfloor},
    {"set_showframe", LuaSetShowframe},
    {"set_showframelabel", LuaSetShowframelabel},
    {"set_showframerate", LuaSetShowframerate},
    {"set_showgrid", LuaSetShowgrid},
    {"set_showgridloc", LuaSetShowgridloc},
    {"set_showhmstimelabel", LuaSetShowhmstimelabel},
    {"set_showhrrcutoff", LuaSetShowhrrcutoff},
    {"set_showiso", LuaSetShowiso},
    {"set_showisonormals", LuaSetShowisonormals},
    {"set_showlabels", LuaSetShowlabels},
#ifdef pp_memstatus
    {"set_showmemload", LuaSetShowmemload},
#endif
    {"set_showopenvents", LuaSetShowopenvents},
    {"set_showothervents", LuaSetShowothervents},
    {"set_showsensors", LuaSetShowsensors},
    {"set_showsliceinobst", LuaSetShowsliceinobst},
    {"set_showsmokepart", LuaSetShowsmokepart},
    {"set_showsprinkpart", LuaSetShowsprinkpart},
    {"set_showstreak", LuaSetShowstreak},
    {"set_showterrain", LuaSetShowterrain},
    {"set_showthreshold", LuaSetShowthreshold},
    {"set_showticks", LuaSetShowticks},
    {"set_showtimebar", LuaSetShowtimebar},
    {"set_showtimelabel", LuaSetShowtimelabel},
    {"set_showtitle", LuaSetShowtitle},
    {"set_showtracersalways", LuaSetShowtracersalways},
    {"set_showtriangles", LuaSetShowtriangles},
    {"set_showtransparent", LuaSetShowtransparent},
    {"set_showtransparentvents", LuaSetShowtranparentvents},
    {"set_showtrianglecount", LuaSetShowtrianglecount},
    {"set_showventflow", LuaSetShowventflow},
    {"set_showvents", LuaSetShowvents},
    {"set_showwalls", LuaSetShowwalls},
    {"set_skipembedslice", LuaSetSkipembedslice},
#ifdef pp_SLICEUP
    {"set_slicedup", lua_set_slicedup},
#endif
    {"set_smokesensors", LuaSetSmokesensors},
#ifdef pp_LANG
    {"set_startuplang", lua_set_startuplang},
#endif
    {"set_stereo", LuaSetStereo},
    {"set_surfinc", LuaSetSurfinc},
    {"set_terrainparams", LuaSetTerrainparams},
    {"set_titlesafe", LuaSetTitlesafe},
    {"set_trainermode", LuaSetTrainermode},
    {"set_trainerview", LuaSetTrainerview},
    {"set_transparent", LuaSetTransparent},
    {"set_treeparms", LuaSetTreeparms},
    {"set_twosidedvents", LuaSetTwosidedvents},
    {"set_vectorskip", LuaSetVectorskip},
    {"set_volsmoke", LuaSetVolsmoke},
    {"set_zoom", LuaSetZoom},
    {"set_cellcentertext", LuaSetCellcentertext},
    {"set_inputfile", LuaSetInputfile},
    {"set_labelstartupview", LuaSetLabelstartupview},
    // { "set_pixelskip", lua_set_pixelskip },
    {"set_renderclip", LuaSetRenderclip},
    // { "set_renderfilelabel", lua_set_renderfilelabel },
    {"set_renderfiletype", LuaSetRenderfiletype},

    // { "set_skybox", lua_set_skybox },
    // { "set_renderoption", lua_set_renderoption },
    {"get_units", LuaGetUnits},
    {"get_unitclass", LuaGetUnitclass},

    {"set_pl3d_bound_min", LuaSetPl3dBoundMin},
    {"set_pl3d_bound_max", LuaSetPl3dBoundMax},

    {"set_units", LuaSetUnits},
    {"set_unitclasses", LuaSetUnitclasses},
    {"set_zaxisangles", LuaSetZaxisangles},
    {"set_colorbartype", LuaSetColorbartype},
    {"set_extremecolors", LuaSetExtremecolors},
    {"set_firecolor", LuaSetFirecolor},
    {"set_firecolormap", LuaSetFirecolormap},
    {"set_firedepth", LuaSetFiredepth},
    // { "set_golorbar", lua_set_gcolorbar },
    {"set_showextremedata", LuaSetShowextremedata},
    {"set_smokecolor", LuaSetSmokecolor},
    {"set_smokecull", LuaSetSmokecull},
    {"set_smokeskip", LuaSetSmokeskip},
    {"set_smokealbedo", LuaSetSmokealbedo},
#ifdef pp_GPU // TODO: register anyway, but tell user it is not available
    {"set_smokerthick", LuaSetSmokerthick},
#endif
// { "set_smokethick", lua_set_smokethick },
#ifdef pp_GPU
    {"set_usegpu", LuaSetUsegpu},
#endif
    {"set_showhazardcolors", LuaSetShowhazardcolors},
    {"set_showhzone", LuaSetShowhzone},
    {"set_showszone", LuaSetShowszone},
    {"set_showvzone", LuaSetShowvzone},
    {"set_showzonefire", LuaSetShowzonefire},
    {"set_showpathnodes", LuaSetShowpathnodes},
    {"set_showtourroute", LuaSetShowtourroute},
    {"set_tourcolors_selectedpathline", LuaSetTourcolorsSelectedpathline},
    {"set_tourcolors_selectedpathlineknots",
     LuaSetTourcolorsSelectedpathlineknots},
    {"set_tourcolors_selectedknot", LuaSetTourcolorsSelectedknot},
    {"set_tourcolors_pathline", LuaSetTourcolorsPathline},
    {"set_tourcolors_pathknots", LuaSetTourcolorsPathknots},
    {"set_tourcolors_text", LuaSetTourcolorsText},
    {"set_tourcolors_avatar", LuaSetTourcolorsAvatar},
    {"set_viewalltours", LuaSetViewalltours},
    {"set_viewtimes", LuaSetViewtimes},
    {"set_viewtourfrompath", LuaSetViewtourfrompath},
    {"set_devicevectordimensions", LuaSetDevicevectordimensions},
    {"set_devicebounds", LuaSetDevicebounds},
    {"set_deviceorientation", LuaSetDeviceorientation},
    {"set_gridparms", LuaSetGridparms},
    {"set_gsliceparms", LuaSetGsliceparms},
    {"set_loadfilesatstartup", LuaSetLoadfilesatstartup},
    {"set_mscale", LuaSetMscale},
    {"set_sliceauto", LuaSetSliceauto},
    {"set_msliceauto", LuaSetMsliceauto},
    {"set_compressauto", LuaSetCompressauto},
    // { "set_part5propdisp", lua_set_part5propdisp },
    // { "set_part5color", lua_set_part5color },
    {"set_propindex", LuaSetPropindex},
    // { "set_shooter", lua_set_shooter },
    {"set_showdevices", LuaSetShowdevices},
    {"set_showdevicevals", LuaSetShowdevicevals},
    {"set_showmissingobjects", LuaSetShowmissingobjects},
    {"set_tourindex", LuaSetTourindex},
    // { "set_userticks", lua_set_userticks },
    {"set_c_particles", LuaSetCParticles},
    {"set_c_slice", LuaSetCSlice},
    {"set_cache_boundarydata", LuaSetCacheBoundarydata},
    {"set_cache_qdata", LuaSetCacheQdata},
#ifdef pp_HIST
    {"set_percentilelevel", LuaSetPercentilelevel},
#endif
    {"set_timeoffset", LuaSetTimeoffset},
    {"set_tload", LuaSetTload},
    {"set_v_slice", LuaSetVSlice},
    {"set_patchdataout", LuaSetPatchdataout},

    {"show_smoke3d_showall", LuaShowSmoke3dShowall},
    {"show_smoke3d_hideall", LuaShowSmoke3dHideall},
    {"show_slices_showall", LuaShowSlicesShowall},
    {"show_slices_hideall", LuaShowSlicesHideall},
    {"add_title_line", LuaAddTitleLine},
    {"clear_title_lines", LuaClearTitleLines},

    {"get_nglobal_times", LuaGetNglobalTimes},
    {"get_global_time", LuaGetGlobalTime},
    {"get_npartinfo", LuaGetNpartinfo},

    {"get_slice", LuaGetSlice},
    {"slice_get_label", LuaSliceGetLabel},
    {"slice_get_filename", LuaSliceGetFilename},
    {"slice_get_data", LuaSliceGetData},
    {"slice_data_map_frames", LuaSliceDataMapFrames},
    {"slice_data_map_frames_count_less", LuaSliceDataMapFramesCountLess},
    {"slice_data_map_frames_count_less_eq", LuaSliceDataMapFramesCountLessEq},
    {"slice_data_map_frames_count_greater", LuaSliceDataMapFramesCountGreater},
    {"slice_data_map_frames_count_greater_eq",
     LuaSliceDataMapFramesCountGreaterEq},
    {"slice_get_times", LuaSliceGetTimes},

    {"get_part", LuaGetPart},
    {"get_part_npoints", LuaGetPartNpoints},

    {"get_qdata_sum", LuaGetQdataSum},
    {"get_qdata_sum_bounded", LuaGetQdataSumBounded},
    {"get_qdata_max_bounded", LuaGetQdataMaxBounded},
    {"get_qdata_mean", LuaGetQdataMean},
    // nglobal_times is the number of frames
    //  this cannot be set as a global at init as it will change
    //  on the loading of smokeview cases
    {NULL, NULL}};

int SmvlibNewindex(lua_State *L) {
  const char *field = lua_tostring(L, 2);
  const char *value = lua_tostring(L, 3);
  if(strcmp(field, "renderdir") == 0) {
    lua_pushstring(L, value);
    LuaSetrenderdir(L);
    return 0;
  }
  else {
    return 0;
  }
}

int SmvlibIndex(lua_State *L) {
  // Take the index from the table.
  // lua_pushstring(L, "index");
  // lua_gettable(L, 1);
  // int index = lua_tonumber(L, -1);
  const char *field = lua_tostring(L, 2);
  if(strcmp(field, "renderdir") == 0) {
    return LuaGetrenderdir(L);
  }
  else {
    return 0;
  }
}

lua_State *InitLua() {
  lua_instance = luaL_newstate();

  luaL_openlibs(lua_instance);

  luaL_newlib(lua_instance, SMVLIB);

  lua_pushcfunction(lua_instance, &LuaCreateCase);
  lua_setfield(lua_instance, -2, "load_default");

  lua_createtable(lua_instance, 0, 1);
  lua_pushcfunction(lua_instance, &SmvlibNewindex);
  lua_setfield(lua_instance, -2, "__newindex");
  lua_pushcfunction(lua_instance, &SmvlibIndex);
  lua_setfield(lua_instance, -2, "__index");
  // then set the metatable
  lua_setmetatable(lua_instance, -2);

  lua_setglobal(lua_instance, "smvlib");

  lua_pushstring(lua_instance, script_dir_path);
  lua_setglobal(lua_instance, "current_script_dir");

  // a boolean value that determines if lua is running in smokeview
  lua_pushboolean(lua_instance, 1);
  lua_setglobal(lua_instance, "smokeviewEmbedded");

  return lua_instance;
}

int RunScriptString(const char *string) { return luaL_dostring(lua_instance, string); }

int LoadLuaScript(const char *filename) {
  // The display callback needs to be run once initially.
  // PROBLEM: the display CB does not work without a loaded case.
  runluascript = 0;
  LuaDisplayCb(lua_instance);
  runluascript = 1;
  char cwd[1000];
#if defined(_WIN32)
  _getcwd(cwd, 1000);
#else
  getcwd(cwd, 1000);
#endif
  const char *err_msg;
  lua_Debug info;
  int level = 0;
  int return_code = luaL_loadfile(lua_instance, filename);
  switch(return_code) {
  case LUA_OK:
    break;
  case LUA_ERRSYNTAX:
    fprintf(stderr, "Syntax error loading %s\n", filename);
    err_msg = lua_tostring(lua_instance, -1);
    fprintf(stderr, "error:%s\n", err_msg);
    level = 0;
    while(lua_getstack(lua_instance, level, &info)) {
      lua_getinfo(lua_instance, "nSl", &info);
      fprintf(stderr, "  [%d] %s:%d -- %s [%s]\n", level, info.short_src,
              info.currentline, (info.name ? info.name : "<unknown>"),
              info.what);
      ++level;
    }
    break;
  case LUA_ERRMEM:
    break;
  case LUA_ERRFILE:
    fprintf(stderr, "Could not load file %s\n", filename);
    err_msg = lua_tostring(lua_instance, -1);
    fprintf(stderr, "error:%s\n", err_msg);
    level = 0;
    while(lua_getstack(lua_instance, level, &info)) {
      lua_getinfo(lua_instance, "nSl", &info);
      fprintf(stderr, "  [%d] %s:%d -- %s [%s]\n", level, info.short_src,
              info.currentline, (info.name ? info.name : "<unknown>"),
              info.what);
      ++level;
    }
    break;
  }
  return return_code;
}

int LoadSsfScript(const char *filename) {
  // char filename[1024];
  //   if (strlen(script_filename) == 0) {
  //       strncpy(filename, global_scase.fdsprefix, 1020);
  //       strcat(filename, ".ssf");
  //   } else {
  //       strncpy(filename, script_filename, 1024);
  //   }
  // The display callback needs to be run once initially.
  // PROBLEM: the display CB does not work without a loaded case.
  runscript = 0;
  LuaDisplayCb(lua_instance);
  runscript = 1;
  const char *err_msg;
  lua_Debug info;
  int level = 0;
  char l_string[1024];
  snprintf(l_string, 1024, "require(\"ssfparser\")\nrunSSF(\"%s.ssf\")",
           global_scase.fdsprefix);
  int ssfparser_loaded_err = luaL_dostring(lua_instance, "require \"ssfparser\"");
  if(ssfparser_loaded_err) {
    fprintf(stderr, "Failed to load ssfparser\n");
  }
  int return_code = luaL_loadstring(lua_instance, l_string);
  switch(return_code) {
  case LUA_OK:
    break;
  case LUA_ERRSYNTAX:
    fprintf(stderr, "Syntax error loading %s\n", filename);
    err_msg = lua_tostring(lua_instance, -1);
    fprintf(stderr, "error:%s\n", err_msg);
    level = 0;
    while(lua_getstack(lua_instance, level, &info)) {
      lua_getinfo(lua_instance, "nSl", &info);
      fprintf(stderr, "  [%d] %s:%d -- %s [%s]\n", level, info.short_src,
              info.currentline, (info.name ? info.name : "<unknown>"),
              info.what);
      ++level;
    }
    break;
  case LUA_ERRMEM:
    break;
  case LUA_ERRFILE:
    fprintf(stderr, "Could not load file %s\n", filename);
    err_msg = lua_tostring(lua_instance, -1);
    fprintf(stderr, "error:%s\n", err_msg);
    level = 0;
    while(lua_getstack(lua_instance, level, &info)) {
      lua_getinfo(lua_instance, "nSl", &info);
      fprintf(stderr, "  [%d] %s:%d -- %s [%s]\n", level, info.short_src,
              info.currentline, (info.name ? info.name : "<unknown>"),
              info.what);
      ++level;
    }
    break;
  }
  return 0;
}

int yield_or_ok_ssf = LUA_YIELD;
int RunSsfScript() {
  if(yield_or_ok_ssf == LUA_YIELD) {
    int nresults = 0;
#if LUA_VERSION_NUM < 502
    yield_or_ok_ssf = lua_resume(L, 0);
#elif LUA_VERSION_NUM < 504
    yield_or_ok_ssf = lua_resume(L, NULL, 0);
#else
    yield_or_ok_ssf = lua_resume(lua_instance, NULL, 0, &nresults);
#endif
    if(yield_or_ok_ssf == LUA_YIELD) {
      printf("  LUA_YIELD\n");
    }
    else if(yield_or_ok_ssf == LUA_OK) {
      printf("  LUA_OK\n");
    }
    else if(yield_or_ok_ssf == LUA_ERRRUN) {
      printf("  LUA_ERRRUN\n");
      const char *err_msg;
      err_msg = lua_tostring(lua_instance, -1);
      fprintf(stderr, "error:%s\n", err_msg);
      lua_Debug info;
      int level = 0;
      while(lua_getstack(lua_instance, level, &info)) {
        lua_getinfo(lua_instance, "nSl", &info);
        fprintf(stderr, "  [%d] %s:%d -- %s [%s]\n", level, info.short_src,
                info.currentline, (info.name ? info.name : "<unknown>"),
                info.what);
        ++level;
      };
    }
    else if(yield_or_ok_ssf == LUA_ERRMEM) {
      printf("  LUA_ERRMEM\n");
    }
    else {
      printf("  resume code: %i\n", yield_or_ok_ssf);
    }
  }
  else {
    lua_close(lua_instance);
    glutIdleFunc(NULL);
  }
  return yield_or_ok_ssf;
}

int yield_or_ok = LUA_YIELD;
int RunLuaScript() {
  if(yield_or_ok == LUA_YIELD) {
    int nresults = 0;
#if LUA_VERSION_NUM < 502
    yield_or_ok_ssf = lua_resume(L, 0);
#elif LUA_VERSION_NUM < 504
    yield_or_ok_ssf = lua_resume(L, NULL, 0);
#else
    yield_or_ok = lua_resume(lua_instance, NULL, 0, &nresults);
#endif
    if(yield_or_ok == LUA_YIELD) {
      printf("  LUA_YIELD\n");
    }
    else if(yield_or_ok == LUA_OK) {
      printf("  LUA_OK\n");
    }
    else if(yield_or_ok == LUA_ERRRUN) {
      printf("  LUA_ERRRUN\n");
      const char *err_msg;
      err_msg = lua_tostring(lua_instance, -1);
      fprintf(stderr, "error:%s\n", err_msg);
      lua_Debug info;
      int level = 0;
      while(lua_getstack(lua_instance, level, &info)) {
        lua_getinfo(lua_instance, "nSl", &info);
        fprintf(stderr, "  [%d] %s:%d -- %s [%s]\n", level, info.short_src,
                info.currentline, (info.name ? info.name : "<unknown>"),
                info.what);
        ++level;
      };
    }
    else if(yield_or_ok == LUA_ERRMEM) {
      printf("  LUA_ERRMEM\n");
    }
    else {
      printf("  resume code: %i\n", yield_or_ok);
    }
  }
  else {
    lua_close(lua_instance);
    glutIdleFunc(NULL);
  }
  GLUTPOSTREDISPLAY;
  return yield_or_ok;
}
#if LUA_VERSION_NUM < 502
LUA_API lua_Number lua_version(lua_State *L) {
  UNUSED(L);
  return LUA_VERSION_NUM;
}

LUALIB_API void luaL_checkversion_(lua_State *L, lua_Number ver, size_t sz) {
  lua_Number v = lua_version(L);
  if(sz != LUAL_NUMSIZES) /* check numeric types */
    luaL_error(L, "core and library have incompatible numeric types");
  else if(v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
               (LUAI_UACNUMBER)ver, (LUAI_UACNUMBER)v);
}

LUALIB_API void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  for(; l->name != NULL; l++) { /* fill the table with given functions */
    if(l->func == NULL)         /* place holder? */
      lua_pushboolean(L, 0);
    else {
      int i;
      for(i = 0; i < nup; i++) /* copy upvalues to the top */
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup); /* closure with those upvalues */
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_pop(L, nup); /* remove upvalues */
}
#endif
#endif
