#include "options.h"
#ifdef pp_LUA
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "smokeheaders.h"
#include "IOvolsmoke.h"
#include "infoheader.h"
#include "glui_bounds.h"
#include "glui_motion.h"
#include "glui_smoke.h"
#include "c_api.h"
#include "gd.h"
#include "IOscript.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include "readsmoke.h"
#include "shared_structures.h"

// function prototypes for functions drawn from other areas of smokeview
// from startup.c
void ReadBoundINI(void);
void InitTranslate(char *bindir, char *tr_name);
void InitMisc(void);
// from menus.c
void UpdateMenu(void);
void LoadVolsmoke3DMenu(int value);
void UnLoadVolsmoke3DMenu(int value);
void UpdateSliceBounds(void);
void OutputSliceData(void);
void UnloadBoundaryMenu(int value);

/// @brief Given a quantity type, return the appropriate index into the
/// slicebounds array.
/// @param slice_type A string describing the slice quantity.
/// @return If successful, the index > 0. If not found -1.
int GetSliceBoundIndex(const char *slice_type) {
  for(int i = 0; i < nslicebounds; i++) {
    if(strcmp(slicebounds[i].shortlabel, slice_type) == 0) {
      return i;
    }
  }
  return -1;
}

/// @brief Set the minimum bound of a given slice quantity.
/// @param slice_type The quantity string
/// @param set
/// @param value
/// @return Non-zero on error
int SetSliceBoundMin(const char *slice_type, int set, float value) {
  int slice_type_index = GetSliceBoundIndex(slice_type);
  if(slice_type_index < 0) {
    // Slice type index could not be found.
    return 1;
  }
  slicebounds[slice_type_index].dlg_setvalmin = set;
  slicebounds[slice_type_index].dlg_valmin = value;
  int error = 0;
  UpdateSliceBounds();
  // Update the colors given the bounds set above
  UpdateAllSliceColors(slice_type_index, &error);
  return error;
}

/// @brief Set the maximum bound of a given slice quantity.
/// @param[in] slice_type
/// @param[in] set
/// @param[in] value
/// @return
int SetSliceBoundMax(const char *slice_type, int set, float value) {
  int slice_type_index = GetSliceBoundIndex(slice_type);
  if(slice_type_index < 0) {
    // Slice type index could not be found.
    return 1;
  }
  slicebounds[slice_type_index].dlg_setvalmax = set;
  slicebounds[slice_type_index].dlg_valmax = value;
  int error = 0;
  UpdateSliceBounds();
  // Update the colors given the bounds set above
  UpdateAllSliceColors(slice_type_index, &error);
  return error;
}

/// @brief Set the bounds of a given slice quantity.
/// @param[in] slice_type
/// @param[in] set_valmin
/// @param[in] valmin
/// @param[in] set_valmax
/// @param[in] valmax
/// @return Non-zero on error
int CApiSetSliceBounds(const char *slice_type, int set_valmin, float valmin,
                       int set_valmax, float valmax) {
  int slice_type_index = GetSliceBoundIndex(slice_type);
  if(slice_type_index < 0) {
    // Slice type index could not be found.
    return 1;
  }
  // Set the requested bounds
  slicebounds[slice_type_index].dlg_setvalmin = set_valmin;
  slicebounds[slice_type_index].dlg_setvalmax = set_valmax;
  slicebounds[slice_type_index].dlg_valmin = valmin;
  slicebounds[slice_type_index].dlg_valmax = valmax;
  int error = 0;
  GLUISetMinMax(BOUND_SLICE, slicebounds[slice_type_index].shortlabel,
                set_valmin, valmin, set_valmax, valmax);
  // Update the colors given the bounds set above
  UpdateAllSliceColors(slice_type_index, &error);
  return 0;
}

/// @brief Get the slice bounds of a given slice quantity.
/// @param[in] slice_type
/// @param[out] A simple_bounds struct containing the min and max bounds being
/// used.
/// @return Non-zero on error
ERROR_CODE GetSliceBounds(const char *slice_type, simple_bounds *bounds) {
  int slice_type_index = GetSliceBoundIndex(slice_type);
  if(slice_type_index < 0) {
    // Slice type index could not be found.
    return ERR_NOK;
  }
  (*bounds).min = slicebounds[slice_type_index].dlg_valmin;
  (*bounds).max = slicebounds[slice_type_index].dlg_valmax;
  return ERR_OK;
}

/// @brief Loads an SMV file into smokeview.
///
/// This should be completely independent from the setup of smokeview, and
/// should be able to be run multiple times (or not at all). This is based on
/// setup_case from startup.c. Currently it features initialisation of some GUI
/// elements that have yet to be factored out.
/// @param[in] input_filename
/// @return Non-zero on error
int Loadsmvall(const char *input_filename) {
  int return_code;
  // fdsprefix and input_filename_ext are global and defined in smokeviewvars.h
  // TODO: move these into the model information namespace
  ParseSmvFilepath(input_filename, global_scase.fdsprefix, input_filename_ext);
  return_code = Loadsmv(global_scase.fdsprefix, input_filename_ext);
#ifdef pp_HIST
  if(return_code == 0 && update_bounds == 1) return_code = Update_Bounds();
#endif
  if(return_code != 0) return 1;
  // if(convert_ini==1){
  // ReadIni(ini_from);
  // }
  return 0;
}

/// @brief Takes a filepath to an smv file an finds the casename and the
/// extension, which are returned in the 2nd and 3rd arguments (the 2nd and 3rd
/// aguments a pre-existing strings).
/// @param[in] smv_filepath
/// @param[out] fdsprefix
/// @param[out] input_filename_ext
/// @return
int ParseSmvFilepath(const char *smv_filepath, char *fdsprefix_arg,
                     char *input_filename_ext_arg) {
  int len_casename;
  strcpy(input_filename_ext_arg, "");
  len_casename = (int)strlen(smv_filepath);
  if(len_casename > 4) {
    char *c_ext;

    c_ext = strrchr(smv_filepath, '.');
    if(c_ext != NULL) {
      STRCPY(input_filename_ext_arg, c_ext);
      ToLower(input_filename_ext_arg);

      if(c_ext != NULL && (strcmp(input_filename_ext_arg, ".smv") == 0 ||
                           strcmp(input_filename_ext_arg, ".svd") == 0 ||
                           strcmp(input_filename_ext_arg, ".smt") == 0)) {
        // c_ext[0]=0;
        STRCPY(fdsprefix_arg, smv_filepath);
        fdsprefix_arg[strlen(fdsprefix_arg) - 4] = 0;
        strcpy(movie_name, fdsprefix_arg);
        strcpy(render_file_base, fdsprefix_arg);
        FREEMEMORY(trainer_filename);
        NewMemory((void **)&trainer_filename, (unsigned int)(len_casename + 7));
        STRCPY(trainer_filename, smv_filepath);
        STRCAT(trainer_filename, ".svd");
        FREEMEMORY(test_filename);
        NewMemory((void **)&test_filename, (unsigned int)(len_casename + 7));
        STRCPY(test_filename, smv_filepath);
        STRCAT(test_filename, ".smt");
      }
    }
  }
  return 0;
}

int Loadsmv(char *input_filename, char *input_filename_ext_arg) {
  int return_code;
  char *input_file;

  FREEMEMORY(part_globalbound_filename);
  NewMemory((void **)&part_globalbound_filename,
            strlen(fdsprefix) + strlen(".prt5.gbnd") + 1);
  STRCPY(part_globalbound_filename, fdsprefix);
  STRCAT(part_globalbound_filename, ".prt5.gbnd");
  char *smokeview_scratchdir = GetUserConfigDir();
  part_globalbound_filename = GetFileName(
      smokeview_scratchdir, part_globalbound_filename, NOT_FORCE_IN_DIR);
  FREEMEMORY(smokeview_scratchdir);
  // setup input files names

  input_file = smv_filename;
  if(strcmp(input_filename_ext_arg, ".svd") == 0 || demo_option == 1) {
    trainer_mode = 1;
    trainer_active = 1;
    if(strcmp(input_filename_ext_arg, ".svd") == 0) {
      input_file = trainer_filename;
    }
    else if(strcmp(input_filename_ext_arg, ".smt") == 0) {
      input_file = test_filename;
    }
  }
  {
    bufferstreamdata *smv_streaminfo = NULL;
    smv_streaminfo = GetSMVBuffer(input_file);
    smv_streaminfo = AppendFileBuffer(smv_streaminfo, iso_filename);
    smv_streaminfo = AppendFileBuffer(smv_streaminfo, fedsmv_filename);
    return_code = ReadSMV(smv_streaminfo);
    if(smv_streaminfo != NULL) {
      FCLOSE(smv_streaminfo);
    }
  }
  if(return_code == 0 && trainer_mode == 1) {
    GLUIShowTrainer();
    GLUIShowAlert();
  }
  switch(return_code) {
  case 1:
    fprintf(stderr, "*** Error: Smokeview file, %s, not found\n", input_file);
    return 1;
  case 2:
    fprintf(stderr, "*** Error: problem reading Smokeview file, %s\n",
            input_file);
    return 2;
  case 0:
    UpdateSMVDynamic(input_file);
    break;
  case 3:
    return 3;
  default:
    assert(FFALSE);
  }

  /* initialize units */

  InitUnits();
  InitUnitDefs();
  SetUnitVis();

  CheckMemory;
  ReadIni(NULL);
  ReadBoundINI();

  UpdateRGBColors(COLORBAR_INDEX_NONE);

  if(use_graphics == 0) return 0;
  glui_defined = 1;
  char *smv_bindir = GetSmvRootDir();
  InitTranslate(smv_bindir, tr_name);
  FREEMEMORY(smv_bindir);

  if(global_scase.tourcoll.ntourinfo == 0) SetupTour();
  InitRolloutList();
  GLUIColorbarSetup(mainwindow_id);
  GLUIMotionSetup(mainwindow_id);
  GLUIBoundsSetup(mainwindow_id);
  GLUIShooterSetup(mainwindow_id);
  GLUIGeometrySetup(mainwindow_id);
  GLUIClipSetup(mainwindow_id);
  GLUIDisplaySetup(mainwindow_id);
  GLUIDeviceSetup(mainwindow_id);
  GLUITourSetup(mainwindow_id);
  GLUIAlertSetup(mainwindow_id);
  GLUIStereoSetup(mainwindow_id);
  GLUI3dSmokeSetup(mainwindow_id);

  UpdateLights(light_position0, light_position1);

  glutReshapeWindow(screenWidth, screenHeight);

  SetMainWindow();
  glutShowWindow();
  glutSetWindowTitle(fdsprefix);
  InitMisc();
  GLUITrainerSetup(mainwindow_id);
  glutDetachMenu(GLUT_RIGHT_BUTTON);
  InitMenus();
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  if(trainer_mode == 1) {
    GLUIShowTrainer();
    GLUIShowAlert();
  }
  // initialize info header
  initialiseInfoHeader(&titleinfo, release_title, smv_githash, fds_githash,
                       chidfilebase, fds_title);
  return 0;
}

int Loadfile(const char *filename) {
  int errorcode = 0;
  if(filename == NULL) {
    // Return an error if passed a null pointer.
    return 1;
  }

  for(size_t i = 0; i < global_scase.slicecoll.nsliceinfo; i++) {
    slicedata *sd;

    sd = global_scase.slicecoll.sliceinfo + i;
    if(strcmp(sd->file, filename) == 0) {
      ReadSlice(sd->file, i, ALL_FRAMES, NULL, LOAD, SET_SLICECOLOR,
                &errorcode);
      return errorcode;
    }
  }
  for(size_t i = 0; i < global_scase.npatchinfo; i++) {
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(strcmp(patchi->file, filename) == 0) {
      ReadBoundary(i, LOAD, &errorcode);
      return errorcode;
    }
  }
  for(size_t i = 0; i < global_scase.npartinfo; i++) {
    partdata *parti;

    parti = global_scase.partinfo + i;
    if(strcmp(parti->file, filename) == 0) {
      LoadParticleMenu(i);
      return errorcode;
    }
  }
  CancelUpdateTriangles();
  for(size_t i = 0; i < global_scase.nisoinfo; i++) {
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(strcmp(isoi->file, filename) == 0) {
      ReadIso(isoi->file, i, LOAD, NULL, &errorcode);
      if(update_readiso_geom_wrapup == UPDATE_ISO_ONE_NOW)
        ReadIsoGeomWrapup(FOREGROUND);
      return errorcode;
    }
  }
  for(size_t i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++) {
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(strcmp(smoke3di->file, filename) == 0) {
      smoke3di->finalize = 1;
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
      return errorcode;
    }
  }
  for(size_t i = 0; i < global_scase.nzoneinfo; i++) {
    zonedata *zonei;

    zonei = global_scase.zoneinfo + i;
    if(strcmp(zonei->file, filename) == 0) {
      ReadZone(i, LOAD, &errorcode);
      return errorcode;
    }
  }
  for(size_t i = 0; i < global_scase.nplot3dinfo; i++) {
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    if(strcmp(plot3di->file, filename) == 0) {
      ReadPlot3D(plot3di->file, i, LOAD, &errorcode);
      UpdateMenu();
      return errorcode;
    }
  }

  fprintf(stderr, "*** Error: file %s failed to load\n", filename);
  if(stderr2 != NULL)
    fprintf(stderr2, "*** Error: file %s failed to load\n", filename);
  return 0;
}

void Loadinifile(const char *filepath) {
  windowresized = 0;
  char f[1048];
  strcpy(f, filepath);
  ReadIni(f);
}

int Loadvfile(const char *filepath) {
  for(size_t i = 0; i < global_scase.slicecoll.nvsliceinfo; i++) {
    slicedata *val;
    vslicedata *vslicei;

    vslicei = global_scase.slicecoll.vsliceinfo + i;
    val = global_scase.slicecoll.sliceinfo + vslicei->ival;
    if(val == NULL) continue;
    if(strcmp(val->reg_file, filepath) == 0) {
      LoadVSliceMenu(i);
      return 0;
    }
  }
  fprintf(stderr, "*** Error: Vector slice file %s was not loaded\n", filepath);
  return 1;
}

void Loadboundaryfile(const char *filepath) {
  int errorcode;
  int count = 0;

  for(size_t i = 0; i < global_scase.npatchinfo; i++) {
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(strcmp(patchi->label.longlabel, filepath) == 0) {
      THREADcontrol(compress_threads, THREAD_LOCK);
      ReadBoundary(i, LOAD, &errorcode);
      count++;
      THREADcontrol(compress_threads, THREAD_UNLOCK);
    }
  }
  if(count == 0)
    fprintf(stderr,
            "*** Error: Boundary files of type %s failed to"
            "load\n",
            filepath);
  force_redisplay = 1;
  updatemenu = 1;
  UpdateFrameNumber(0);
}

/// @brief Specify offset clip in pixels
/// @param flag
/// @param left
/// @param right
/// @param bottom
/// @param top
void Renderclip(int flag, int left, int right, int bottom, int top) {
  clip_rendered_scene = flag;
  render_clip_left = left;
  render_clip_right = right;
  render_clip_bottom = bottom;
  render_clip_top = top;
}

ERROR_CODE CApiRender(const char *filename) {
  DisplayCB();
  // strcpy(render_file_base,filename);
  return RenderFrameLua(VIEW_CENTER, filename);
}

/// @brief Construct filepath for image to be rendered.
/// @param view_mode
/// @param renderfile_name
/// @param renderfile_dir
/// @param renderfile_path
/// @param woffset
/// @param hoffset
/// @param screenH
/// @param basename
/// @return
char *FormFilename(int view_mode, char *renderfile_name, char *renderfile_dir,
                   char *renderfile_path, int woffset, int hoffset, int screenH,
                   const char *basename) {
  char *renderfile_ext;
  char *view_suffix;

  // determine the extension to be used, and set renderfile_ext to it
  switch(render_filetype) {
  case 0:
    renderfile_ext = ext_png;
    break;
  case 1:
    renderfile_ext = ext_jpg;
    break;
  default:
    render_filetype = 2;
    renderfile_ext = ext_png;
    break;
  }

  // if the basename has not been specified, use a predefined method to
  // determine the filename
  if(basename == NULL) {
    view_suffix = "";
    switch(view_mode) {
    case VIEW_LEFT:
      if(stereotype == STEREO_LR) {
        view_suffix = "_L";
      }
      break;
    case VIEW_RIGHT:
      if(stereotype == STEREO_LR) {
        view_suffix = "_R";
      }
      break;
    case VIEW_CENTER:
      break;
    default:
      assert(FFALSE);
      break;
    }

    if(Writable(renderfile_dir) == NO) {
      // TODO: ensure this can be made cross-platform
      if(strlen(renderfile_dir) > 0) {
#if defined(__MINGW32__)
        mkdir(renderfile_dir);
#elif defined(WIN32)
        CreateDirectory(renderfile_dir, NULL);
#else // linux or osx
        mkdir(renderfile_dir, 0755);
#endif
        // #ifdef __MINGW32__
        //                 fprintf(stderr, "%s\n", "making directory(mingw)\n");
        //                 mkdir(renderfile_dir);
        // #elif defined(pp_LINUX)
        //                 fprintf(stderr, "%s\n", "making directory(linux)\n");
        //                 mkdir(renderfile_dir, 0755);
        // #endif
        // #ifdef pp_OSX
        //                 mkdir(renderfile_dir, 0755);
        // #endif
      }
    }
    if(stereotype == STEREO_LR &&
       (view_mode == VIEW_LEFT || view_mode == VIEW_RIGHT)) {
    }

    snprintf(renderfile_name, 1024, "%s%s%s", chidfilebase, view_suffix,
             renderfile_ext);
  }
  else {
    snprintf(renderfile_name, 1024, "%s%s", basename, renderfile_ext);
  }
  return renderfile_name;
}

/// @brief Render a frame to file.
///
/// This is function fulfills the exact same purpose as the original
/// RenderFrame function, except that it takes a second argument, basename. This
/// could be be used as a drop in replacement as long as all existing calls are
/// modified to use basename = NULL.
/// @param view_mode
/// @param basename This is the name that should be given to the rendered file.
/// If NULL then a default filename is formed based on the chosen frame and
/// rendering options.
/// @return
int RenderFrameLua(int view_mode, const char *basename) {
  char renderfile_name[1024]; // the name the file (including extension)
  char renderfile_dir[1024];  // the directory into which the image will be
                              // rendered
  char renderfile_path[2048]; // the full path of the rendered image
  int woffset = 0, hoffset = 0;
  int screen_h;
  int return_code;

  if(script_dir_path != NULL) {
    strcpy(renderfile_dir, script_dir_path);
  }
  else {
    strcpy(renderfile_dir, ".");
  }

#ifdef WIN32
  // reset display idle timer to prevent screen saver from activating
  SetThreadExecutionState(ES_DISPLAY_REQUIRED);
#endif

  screen_h = screenHeight;
  // we should not be rendering under these conditions
  if(view_mode == VIEW_LEFT && stereotype == STEREO_RB) return 0;
  // construct filename for image to be rendered
  FormFilename(view_mode, renderfile_name, renderfile_dir, renderfile_path,
               woffset, hoffset, screen_h, basename);
  // render image
  return_code =
      SmokeviewImage2File(renderfile_dir, renderfile_name, render_filetype,
                          woffset, screenWidth, hoffset, screen_h);
  if(RenderTime == 1 && output_slicedata == 1) {
    OutputSliceData();
  }
  return return_code;
}

int RenderFrameLuaVar(int view_mode, gdImagePtr *RENDERimage) {
  int woffset = 0, hoffset = 0;
  int screen_h;
  int return_code;

#ifdef WIN32
  // reset display idle timer to prevent screen saver from activating
  SetThreadExecutionState(ES_DISPLAY_REQUIRED);
#endif

  screen_h = screenHeight;
  // we should not be rendering under these conditions
  if(view_mode == VIEW_LEFT && stereotype == STEREO_RB) return 0;
  // render image
  return_code = SVimage2var(render_filetype, woffset, screenWidth, hoffset,
                            screen_h, RENDERimage);
  if(RenderTime == 1 && output_slicedata == 1) {
    OutputSliceData();
  }
  return return_code;
}

void Settourkeyframe(float keyframe_time) {
  keyframe *keyj, *minkey = NULL;
  tourdata *touri;
  float minkeytime = 1000000000.0;

  if(selected_tour == NULL) return;
  touri = selected_tour;
  for(keyj = (touri->first_frame).next; keyj->next != NULL; keyj = keyj->next) {
    float diff_time;

    if(keyj == (touri->first_frame).next) {
      minkey = keyj;
      minkeytime = ABS(keyframe_time - keyj->time);
      continue;
    }
    diff_time = ABS(keyframe_time - keyj->time);
    if(diff_time < minkeytime) {
      minkey = keyj;
      minkeytime = diff_time;
    }
  }
  if(minkey != NULL) {
    NewSelect(minkey);
    GLUISetTourKeyframe();
    GLUIUpdateTourControls();
  }
}

void Gsliceview(int data, int show_triangles, int show_triangulation,
                int show_normal) {
  vis_gslice_data = data;
  show_gslice_triangles = show_triangles;
  show_gslice_triangulation = show_triangulation;
  show_gslice_normal = show_normal;
  update_gslice = 1;
}

void Gslicepos(float x, float y, float z) {
  gslice_xyz[0] = x;
  gslice_xyz[1] = y;
  gslice_xyz[2] = z;
  update_gslice = 1;
}

void Gsliceorien(float az, float elev) {
  gslice_normal_azelev[0] = az;
  gslice_normal_azelev[1] = elev;
  update_gslice = 1;
}

void Settourview(int edittourArg, int mode, int show_tourlocusArg,
                 float tour_global_tensionArg) {
  edittour = edittourArg;
  show_avatar = show_tourlocusArg;
  switch(mode) {
  case 0:
    viewtourfrompath = 0;
    break;
  case 1:
    viewtourfrompath = 1;
    break;
  case 2:
    viewtourfrompath = 0;
    break;
  default:
    viewtourfrompath = 0;
    break;
  }
  GLUIUpdateTourState();
}

/// @brief Get the current frame number.
/// @return Time value in seconds.
int Getframe() {
  int framenumber = itimes;
  return framenumber;
}

/// @brief Get the time value of the current frame.
/// @return
float Gettime() { return global_times[itimes]; }

/// @brief Set the currrent time.
///
/// Switch to the frame with the closest time value to @p timeval.
/// @param timeval Time in seconds
/// @return Non-zero on error
int Settime(float timeval) {
  if(global_times != NULL && nglobal_times > 0) {
    if(timeval < global_times[0]) timeval = global_times[0];
    if(timeval > global_times[nglobal_times - 1] - 0.0001) {
#ifdef pp_SETTIME
      float dt;

      dt = timeval - global_times[nglobal_times - 1] - 0.0001;
      if(nglobal_times > 1 && dt > global_times[1] - global_times[0]) {
        fprintf(stderr, "*** Error: data not available at time requested\n");
        fprintf(stderr, "           time: %f s, min time: %f, max time: %f s\n",
                timeval, global_times[0], global_times[nglobal_times - 1]);
        if(script_labelstring != NULL)
          fprintf(stderr,
                  "                 "
                  "label: %s\n",
                  script_labelstring);
      }
#endif
      timeval = global_times[nglobal_times - 1] - 0.0001;
    }
    float valmin = ABS(global_times[0] - timeval);
    int imin = 0;
    for(int i = 1; i < nglobal_times; i++) {
      float val = ABS(global_times[i] - timeval);
      if(val < valmin) {
        valmin = val;
        imin = i;
      }
    }
    itimes = imin;
    script_itime = imin;
    stept = 0;
    force_redisplay = 1;
    UpdateFrameNumber(0);
    UpdateTimeLabels();
    return 0;
  }
  else {
    return 1;
  }
}

/// @brief Show slices in blockages.
/// @param setting Boolean
void SetSliceInObst(int setting) {
  global_scase.show_slice_in_obst = setting;
  // UpdateSliceFilenum();
  // plotstate=GetPlotState(DYNAMIC_PLOTS);
  //
  // UpdateSliceListIndex(slicefilenum);
  // UpdateShow();
}

/// @brief Check if slices are being shown in obstructions.
/// @return
int GetSliceInObst() { return global_scase.show_slice_in_obst; }

/// @brief Set the colorbar to one named @p name
/// @param name
/// @return
ERROR_CODE SetNamedColorbar(const char *name) {
  size_t index = 0;
  if(GetNamedColorbar(name, &index)) {
    return ERR_NOK;
  }
  SetColorbar(index);
  return ERR_OK;
}

ERROR_CODE GetNamedColorbar(const char *name, size_t *index) {
  for (size_t i = 0; i < colorbars.ncolorbars; i++) {
    if (strcmp(colorbars.colorbarinfo[i].menu_label, name) == 0) {
      *index = i;
      return 0;
    }
  }
  return 1;
}

/// @brief Set the colorbar to the given colorbar index.
/// @param value
void SetColorbar(size_t value) {
  colorbartype = value;
  colorbars.iso_colorbar_index = value;
  iso_colorbar = colorbars.colorbarinfo + colorbars.iso_colorbar_index;
  update_texturebar = 1;
  GLUIUpdateListIsoColorobar();
  UpdateCurrentColorbar(colorbars.colorbarinfo + colorbartype);
  GLUIUpdateColorbarType();
  if(colorbartype == colorbars.bw_colorbar_index && colorbars.bw_colorbar_index >= 0) {
    setbwdata = 1;
  }
  else {
    setbwdata = 0;
  }
  GLUIIsoBoundCB(ISO_COLORS);
  GLUISetLabelControls();
  if(value > -10) {
    UpdateRGBColors(COLORBAR_INDEX_NONE);
  }
}

void SetColorbarVisibilityVertical(int setting) {
  visColorbarVertical = setting;
  if(visColorbarVertical == 1 && visColorbarHorizontal == 0) {
    vis_colorbar = 1;
  }
  else if(visColorbarVertical == 0 && visColorbarHorizontal == 1) {
    vis_colorbar = 2;
  }
  else {
    vis_colorbar = 0;
  }
}

int GetColorbarVisibilityVertical() { return visColorbarVertical; }

void ToggleColorbarVisibilityVertical() {
  visColorbarVertical = 1 - visColorbarVertical;
  if(visColorbarVertical == 1 && visColorbarHorizontal == 0) {
    vis_colorbar = 1;
  }
  else if(visColorbarVertical == 0 && visColorbarHorizontal == 1) {
    vis_colorbar = 2;
  }
  else {
    vis_colorbar = 0;
  }
}

void SetColorbarVisibilityHorizontal(int setting) {
  visColorbarHorizontal = setting;
  if(visColorbarVertical == 1 && visColorbarHorizontal == 0) {
    vis_colorbar = 1;
  }
  else if(visColorbarVertical == 0 && visColorbarHorizontal == 1) {
    vis_colorbar = 2;
  }
  else {
    vis_colorbar = 0;
  }
}

int GetColorbarVisibilityHorizontal() { return visColorbarHorizontal; }

void ToggleColorbarVisibilityHorizontal() {
  visColorbarHorizontal = 1 - visColorbarHorizontal;
  if(visColorbarVertical == 1 && visColorbarHorizontal == 0) {
    vis_colorbar = 1;
  }
  else if(visColorbarVertical == 0 && visColorbarHorizontal == 1) {
    vis_colorbar = 2;
  }
  else {
    vis_colorbar = 0;
  }
}

void SetColorbarVisibility(int setting) {
  SetColorbarVisibilityVertical(setting);
}

int GetColorbarVisibility() { return GetColorbarVisibilityVertical(); }

void ToggleColorbarVisibility() { ToggleColorbarVisibilityVertical(); }

void SetTimebarVisibility(int setting) { visTimebar = setting; }

int GetTimebarVisibility() { return visTimebar; }

void ToggleTimebarVisibility() { visTimebar = 1 - visTimebar; }

/// @brief Set whether the title of the simulation is visible.
/// @param setting Boolean value.
void SetTitleVisibility(int setting) { vis_title_fds = setting; }

/// @brief Check whether the title of the simulation is visible.
/// @return
int GetTitleVisibility() { return vis_title_fds; }

void ToggleTitleVisibility() { vis_title_fds = 1 - vis_title_fds; }

void SetSmvVersionVisibility(int setting) { vis_title_smv_version = setting; }

int GetSmvVersionVisibility() { return vis_title_smv_version; }

void ToggleSmvVersionVisibility() {
  vis_title_smv_version = 1 - vis_title_smv_version;
}

void SetChidVisibility(int setting) { vis_title_CHID = setting; }

int GetChidVisibility() { return vis_title_CHID; }

void ToggleChidVisibility() { vis_title_CHID = 1 - vis_title_CHID; }

void BlockagesShowAll() {
  if(global_scase.isZoneFireModel) visFrame = 1;
  /*
  visFloor=1;
  visWalls=1;
  visCeiling=1;
  */
  show_faces_shaded = 1;
  visVents = 1;
  BlockageMenu(visBLOCKAsInput);
}

void ImmersedMenu(int value);
void BlockageMenu(int value);
void BlockagesHideAll() { BlockageMenu(visBLOCKHide); }
// TODO: clarify behaviour under isZoneFireModel
void OutlinesHide() {
  if(global_scase.isZoneFireModel == 0) visFrame = 0;
}
void OutlinesShow() {
  if(global_scase.isZoneFireModel == 0) visFrame = 1;
}

void SurfacesHideAll() {
  visVents = 0;
  visOpenVents = 0;
  visDummyVents = 0;
  global_scase.visOtherVents = 0;
  visCircularVents = VENT_HIDE;
}

void DevicesHideAll() {
  for(size_t i = 0; i < global_scase.objectscoll.nobject_defs; i++) {
    sv_object *objecti = global_scase.objectscoll.object_defs[i];
    objecti->visible = 0;
  }
}

// axis visibility
void SetAxisVisibility(int setting) {
  visaxislabels = setting;
  GLUIUpdateVisAxisLabels();
}

int GetAxisVisibility() { return visaxislabels; }

void ToggleAxisVisibility() {
  visaxislabels = 1 - visaxislabels;
  GLUIUpdateVisAxisLabels();
}

// framelabel visibility
void SetFramelabelVisibility(int setting) {
  visFramelabel = setting;
  // The frame label should not be shown without the timebar
  // so show timebar if necessary.
  if(visFramelabel == 1) visTimebar = 1;
  if(visFramelabel == 1) {
    vis_hrr_label = 0;
    if(hrrinfo != NULL) {
      UpdateTimes();
    }
  }
}

int GetFramelabelVisibility() { return visFramelabel; }

void ToggleFramelabelVisibility() {
  visFramelabel = 1 - visFramelabel;
  // The frame label should not be shown without the timebar
  // so show timebar if necessary.
  if(visFramelabel == 1) visTimebar = 1;
  if(visFramelabel == 1) {
    vis_hrr_label = 0;
    if(hrrinfo != NULL) {
      UpdateTimes();
    }
  }
}

void SetFramerateVisibility(int setting) { visFramerate = setting; }

int GetFramerateVisibility() { return visFramerate; }

void ToggleFramerateVisibility() { visFramerate = 1 - visFramerate; }

// grid locations visibility
void SetGridlocVisibility(int setting) { visgridloc = setting; }

int GetGridlocVisibility() { return visgridloc; }

void ToggleGridlocVisibility() { visgridloc = 1 - visgridloc; }

// HRRPUV cutoff visibility
void SetHrrcutoffVisibility(int setting) { show_hrrcutoff_active = setting; }

int GetHrrcutoffVisibility() { return show_hrrcutoff_active; }

void ToggleHrrcutoffVisibility() {
  show_hrrcutoff_active = 1 - show_hrrcutoff_active;
}

// HRR label
void SetHrrlabelVisibility(int setting) { vis_hrr_label = setting; }

int GetHrrlabelVisibility() { return vis_hrr_label; }

void ToggleHrrlabelVisibility() { vis_hrr_label = 1 - vis_hrr_label; }

// memory load
#ifdef pp_memstatus
void set_memload_visibility(int setting) { visAvailmemory = setting; }

int get_memload_visibility() { return visAvailmemory; }

void toggle_memload_visibility() { visAvailmemory = 1 - visAvailmemory; }
#endif

// mesh label
void SetMeshlabelVisibility(int setting) { visMeshlabel = setting; }

int GetMeshlabelVisibility() { return visMeshlabel; }

void ToggleMeshlabelVisibility() { visMeshlabel = 1 - visMeshlabel; }

// slice average
void SetSliceAverageVisibility(int setting) { vis_slice_average = setting; }

int GetSliceAverageVisibility() { return vis_slice_average; }

void ToggleSliceAverageVisibility() {
  vis_slice_average = 1 - vis_slice_average;
}

// time
void SetTimeVisibility(int setting) { visTimelabel = setting; }

int GetTimeVisibility() { return visTimelabel; }

void ToggleTimeVisibility() { visTimelabel = 1 - visTimelabel; }

// user settable ticks
void SetUserTicksVisibility(int setting) { visUSERticks = setting; }

int GetUserTicksVisibility() { return visUSERticks; }

void ToggleUserTicksVisibility() { visUSERticks = 1 - visUSERticks; }

// version info
void SetVersionInfoVisibility(int setting) { vis_title_gversion = setting; }

int GetVersionInfoVisibility() { return vis_title_gversion; }

void ToggleVersionInfoVisibility() {
  vis_title_gversion = 1 - vis_title_gversion;
}

void SetAllLabelVisibility(int setting) {
  SetColorbarVisibility(setting);
  SetTimebarVisibility(setting);
  SetTitleVisibility(setting);
  SetAxisVisibility(setting);
  SetFramelabelVisibility(setting);
  SetFramerateVisibility(setting);
  SetGridlocVisibility(setting);
  SetHrrcutoffVisibility(setting);
  SetHrrlabelVisibility(setting);
#ifdef pp_memstatus
  set_memload_visibility(setting);
#endif
  SetMeshlabelVisibility(setting);
  SetSliceAverageVisibility(setting);
  SetTimeVisibility(setting);
  SetUserTicksVisibility(setting);
  SetVersionInfoVisibility(setting);
}

// Display Units
// time
void SetTimehms(int setting) {
  vishmsTimelabel = 1 - vishmsTimelabel;
  GLUISetLabelControls();
}

int GetTimehms() { return vishmsTimelabel; }

void ToggleTimehms() {
  vishmsTimelabel = 1 - vishmsTimelabel;
  GLUISetLabelControls();
}

void SetUnits(int unitclass, int unit_index) {
  unitclasses[unitclass].unit_index = unit_index;
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

void SetUnitsDefault() {
  for(size_t i = 0; i < nunitclasses; i++) {
    unitclasses[i].unit_index = 0;
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

void SetUnitclassDefault(int unitclass) {
  unitclasses[unitclass].unit_index = 0;
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
}

// Show/Hide Geometry
// Obstacles
// View Method

/// @brief Set the way blockages are rendered.
/// @param[im] setting One of the following:
/// - 0 - Defined in input file
/// - 1 - Solid
/// - 2 - Outine only
/// - 3 - Outline added
/// - 4 - Hidden
/// @return
int BlockageViewMethod(int setting) {
  int value;
  switch(setting) {
  case 0:
    value = visBLOCKAsInput;
    break;
  case 1:
    value = visBLOCKNormal;
    break;
  case 2:
    value = visBLOCKOutline;
    break;
  case 3:
    value = visBLOCKAddOutline;
    break;
  case 4:
    value = visBLOCKHide;
    break;
  default:
    return 1;
    break;
  }
  // TODO
  // The below is the menu code verbatim. Simplify to contain only the
  // necessary code.
  BlockageMenu(value);
  return 0;
}

/// @brief Set the color to be used when drawing blockage outlines.
/// @param setting One of the ofollowing
/// - 0 - Use blockage
/// - 1 - Use foreground
/// @return Non-zero on error
int BlockageOutlineColor(int setting) {
  switch(setting) {
  case 0:
    outline_color_flag = 0;
    global_scase.updatefaces = 1;
    break;
  case 1:
    outline_color_flag = 1;
    global_scase.updatefaces = 1;
    break;
  default:
    return 1;
    break;
  }
  return 0;
}

/// @brief Determine how the blockages should be displayed. This is used for the
/// BLOCKLOCATION .ini option.
/// @param setting An integer dictating the display mode as follows:
///   - 0 - grid - Snapped to the grid as used by FDS.
///   - 1 - exact - As specified.
///   - 2 - cad - Using CAD geometry.
/// @return
int BlockageLocations(int setting) {
  switch(setting) {
  case 0:
    blocklocation = BLOCKlocation_grid;
    break;
  case 1:
    blocklocation = BLOCKlocation_exact;
  case 2:
    blocklocation = BLOCKlocation_cad;
    break;
  default:
    assert(FFALSE);
    break;
  }
  return 0;
}

void Setframe(int framenumber) {
  itimes = framenumber;
  script_itime = itimes;
  stept = 0;
  force_redisplay = 1;
  UpdateFrameNumber(0);
  UpdateTimeLabels();
}

/// @brief Load files needed to view volume rendered smoke. One may either load
/// files for all meshes or for one particular mesh. Use meshnumber = -1 for all
/// meshes.
/// @param meshnumber
void Loadvolsmoke(int meshnumber) {
  int imesh;

  imesh = meshnumber;
  if(imesh == -1) {
    read_vol_mesh = VOL_READALL;
    ReadVolsmokeAllFramesAllMeshes2(NULL);
  }
  else if(imesh >= 0 && imesh < global_scase.meshescoll.nmeshes) {
    meshdata *meshi;
    volrenderdata *vr;

    meshi = global_scase.meshescoll.meshinfo + imesh;
    vr = meshi->volrenderinfo;
    ReadVolsmokeAllFrames(vr);
  }
}

/// @brief As with loadvolsmoke, but for a single frame indicated my
/// framenumber. Flag is set to 1 when calling from a script. Reason unkown.
/// @param meshnumber
/// @param framenumber
/// @param flag
void Loadvolsmokeframe(int meshnumber, int framenumber, int flag) {
  int framenum, index;
  int first = 1;

  index = meshnumber;
  framenum = framenumber;
  if(index > global_scase.meshescoll.nmeshes - 1) index = -1;
  for(size_t i = 0; i < global_scase.meshescoll.nmeshes; i++) {
    if(index == i || index < 0) {
      meshdata *meshi;
      volrenderdata *vr;

      meshi = global_scase.meshescoll.meshinfo + i;
      vr = meshi->volrenderinfo;
      FreeVolsmokeFrame(vr, framenum);
      ReadVolsmokeFrame(vr, framenum, &first);
      if(vr->times_defined == 0) {
        vr->times_defined = 1;
        GetVolsmokeAllTimes(vr);
      }
      vr->loaded = 1;
      vr->display = 1;
    }
  }
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  stept = 1;
  UpdateTimes();
  force_redisplay = 1;
  UpdateFrameNumber(framenum);
  int i = framenum;
  itimes = i;
  script_itime = i;
  stept = 1;
  force_redisplay = 1;
  UpdateFrameNumber(0);
  UpdateTimeLabels();
  // TODO: replace with a call to render()
  Keyboard('r', FROM_SMOKEVIEW);
  if(flag == 1) script_render = 1; // called when only rendering a single frame
}

void Load3dsmoke(const char *smoke_type) {
  int errorcode;
  int count = 0;
  int lastsmoke;

  for(size_t i = global_scase.smoke3dcoll.nsmoke3dinfo - 1; i >= 0; i--) {
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(MatchUpper(smoke3di->label.longlabel, smoke_type) == MATCH) {
      lastsmoke = i;
      break;
    }
  }

  for(size_t i = global_scase.smoke3dcoll.nsmoke3dinfo - 1; i >= 0; i--) {
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(MatchUpper(smoke3di->label.longlabel, smoke_type) == MATCH) {
      lastsmoke = i;
      break;
    }
  }

  for(size_t i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++) {
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(MatchUpper(smoke3di->label.longlabel, smoke_type) == MATCH) {
      smoke3di->finalize = 0;
      if(lastsmoke == i) smoke3di->finalize = 1;
      ReadSmoke3D(ALL_SMOKE_FRAMES, i, LOAD, FIRST_TIME, &errorcode);
      count++;
    }
  }
  if(count == 0) {
    fprintf(stderr, "*** Error: Smoke3d files of type %s failed to load\n",
            smoke_type);
    if(stderr2 != NULL)
      fprintf(stderr2, "*** Error: Smoke3d files of type %s failed to load\n",
              smoke_type);
  }
  force_redisplay = 1;
  updatemenu = 1;
}

int SetRendertype(const char *type) {
  if(STRCMP(type, "JPG") == 0 || STRCMP(type, "JPEG") == 0) {
    render_filetype = JPEG;
    return 0;
  }
  else if(STRCMP(type, "PNG") == 0) {
    render_filetype = PNG;
    return 0;
  }
  else {
    return 1;
  }
  UpdateRenderType(render_filetype);
}

int GetRendertype() { return render_filetype; }

void SetMovietype(const char *type) {
  if(STRCMP(type, "WMV") == 0) {
    UpdateMovieType(WMV);
  }
  if(STRCMP(type, "MP4") == 0) {
    UpdateMovieType(MP4);
  }
  else {
    UpdateMovieType(AVI);
  }
}

int GetMovietype() { return movie_filetype; }

void Makemovie(const char *name, const char *base, float framerate_arg) {
  strcpy(movie_name, name);
  strcpy(render_file_base, base);
  movie_framerate = framerate_arg;
  RenderCB(MAKE_MOVIE);
}

int Loadtour(const char *tourname) {
  int count = 0;
  int errorcode = 0;

  for(size_t i = 0; i < global_scase.tourcoll.ntourinfo; i++) {
    tourdata *touri;

    touri = global_scase.tourcoll.tourinfo + i;
    if(strcmp(touri->label, tourname) == 0) {
      TourMenu(i);
      viewtourfrompath = 0;
      TourMenu(MENU_TOUR_VIEWFROMROUTE);
      count++;
      break;
    }
  }

  if(count == 0) {
    fprintf(stderr, "*** Error: The tour %s failed to load\n", tourname);
    errorcode = 1;
  }
  force_redisplay = 1;
  updatemenu = 1;
  return errorcode;
}

void Loadparticles(const char *name) {
  int errorcode;
  int count = 0;

  npartframes_max = GetMinPartFrames(PARTFILE_LOADALL);
  for(size_t i = 0; i < global_scase.npartinfo; i++) {
    partdata *parti;

    parti = global_scase.partinfo + i;
    ReadPart(parti->file, i, UNLOAD, &errorcode);
    count++;
  }
  for(size_t i = 0; i < global_scase.npartinfo; i++) {
    partdata *parti;

    parti = global_scase.partinfo + i;
    ReadPart(parti->file, i, LOAD, &errorcode);
    count++;
  }
  if(count == 0) fprintf(stderr, "*** Error: Particles files failed to load\n");
  force_redisplay = 1;
  UpdateFrameNumber(0);
  updatemenu = 1;
}

void Partclasscolor(const char *color) {
  int count = 0;

  for(size_t i = 0; i < npart5prop; i++) {
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->longlabel, color) == 0) {
      ParticlePropShowMenu(i);
      count++;
    }
  }
  if(count == 0)
    fprintf(stderr, "*** Error: particle class color: %s failed to be set\n",
            color);
}

void Partclasstype(const char *part_type) {
  int count = 0;

  for(size_t i = 0; i < npart5prop; i++) {
    partpropdata *propi;
    int j;

    propi = part5propinfo + i;
    if(propi->display == 0) continue;
    for(j = 0; j < global_scase.npartclassinfo; j++) {
      partclassdata *partclassj;

      if(propi->class_present[j] == 0) continue;
      partclassj = global_scase.partclassinfo + j;
      if(strcmp(partclassj->name, part_type) == 0) {
        ParticlePropShowMenu(-10 - j);
        count++;
      }
    }
  }
  if(count == 0)
    fprintf(stderr,
            "*** Error: particle class type %s failed to be "
            "set\n",
            part_type);
}

void Plot3dprops(int variable_index, int showvector, int vector_length_index,
                 int display_type, float vector_length) {
  int p_index;

  p_index = variable_index;
  if(p_index < 1) p_index = 1;
  if(p_index > 5) p_index = 5;

  visVector = showvector;
  if(visVector != 1) visVector = 0;

  plotn = p_index;
  if(plotn < 1) {
    plotn = numplot3dvars;
  }
  if(plotn > numplot3dvars) {
    plotn = 1;
  }
  UpdateAllPlotSlices();
  if(visiso == 1) UpdateSurface();
  GLUIUpdatePlot3dListIndex();

  vecfactor = 1.0;
  if(vector_length >= 0.0) vecfactor = vector_length;
  GLUIUpdateVectorWidgets();

  contour_type = CLAMP(display_type, 0, 2);
  GLUIUpdatePlot3dDisplay();

  if(visVector == 1 && nplot3dloaded == 1) {
    meshdata *gbsave, *gbi;

    gbsave = current_mesh;
    for(size_t i = 0; i < global_scase.meshescoll.nmeshes; i++) {
      gbi = global_scase.meshescoll.meshinfo + i;
      if(gbi->plot3dfilenum == -1) continue;
      UpdateCurrentMesh(gbi);
      UpdatePlotSlice(XDIR);
      UpdatePlotSlice(YDIR);
      UpdatePlotSlice(ZDIR);
    }
    UpdateCurrentMesh(gbsave);
  }
}

void ShowPlot3dData(int meshnumber, int plane_orientation, int display,
                    int showhide, float position, int isolevel) {
  meshdata *meshi;
  int dir;
  float val;

  if(meshnumber < 0 || meshnumber > global_scase.meshescoll.nmeshes - 1) return;

  meshi = global_scase.meshescoll.meshinfo + meshnumber;
  UpdateCurrentMesh(meshi);

  dir = CLAMP(plane_orientation, XDIR, ISO);

  plotn = display;
  val = position;

  switch(dir) {
  case XDIR:
    visx_all = showhide;
    iplotx_all = GetGridIndex(val, XDIR, plotx_all, nplotx_all);
    NextXIndex(1, 0);
    NextXIndex(-1, 0);
    break;
  case YDIR:
    visy_all = showhide;
    iploty_all = GetGridIndex(val, YDIR, ploty_all, nploty_all);
    NextYIndex(1, 0);
    NextYIndex(-1, 0);
    break;
  case ZDIR:
    visz_all = showhide;
    iplotz_all = GetGridIndex(val, ZDIR, plotz_all, nplotz_all);
    NextZIndex(1, 0);
    NextZIndex(-1, 0);
    break;
  case ISO:
    plotiso[plotn - 1] = isolevel;
    UpdateShowStep(showhide, ISO);
    UpdateSurface();
    updatemenu = 1;
    break;
  default:
    assert(FFALSE);
    break;
  }
  UpdatePlotSlice(dir);
  // UpdateAllPlotSlices();
  // if(visiso==1&&cache_qdata==1)UpdateSurface();
  // UpdatePlot3dListIndex();
  // GLUTPOSTREDISPLAY;
}

void Loadplot3d(int meshnumber, float time_local) {
  size_t count = 0;
  int blocknum = meshnumber - 1;

  for(size_t i = 0; i < global_scase.nplot3dinfo; i++) {
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo + i;
    if(plot3di->blocknumber == blocknum &&
       ABS(plot3di->time - time_local) < 0.5) {
      count++;
      LoadPlot3dMenu(i);
    }
  }
  UpdateRGBColors(COLORBAR_INDEX_NONE);
  GLUISetLabelControls();
  if(count == 0) fprintf(stderr, "*** Error: Plot3d file failed to load\n");

  // UpdateMenu();
}

void Loadiso(const char *type) {
  int count = 0;

  update_readiso_geom_wrapup = UPDATE_ISO_START_ALL;
  for(size_t i = 0; i < global_scase.nisoinfo; i++) {
    int errorcode;
    isodata *isoi;

    isoi = global_scase.isoinfo + i;
    if(STRCMP(isoi->surface_label.longlabel, type) == 0) {
      ReadIso(isoi->file, i, LOAD, NULL, &errorcode);
      count++;
    }
  }
  if(update_readiso_geom_wrapup == UPDATE_ISO_ALL_NOW)
    ReadIsoGeomWrapup(FOREGROUND);
  update_readiso_geom_wrapup = UPDATE_ISO_OFF;
  if(count == 0)
    fprintf(stderr,
            "*** Error: Isosurface files of type %s failed "
            "to load\n",
            type);
  force_redisplay = 1;
  updatemenu = 1;
}

FILE_SIZE Loadsliceindex(size_t index, int *errorcode) {
  return ReadSlice(global_scase.slicecoll.sliceinfo[index].file, (int)index, ALL_FRAMES,
                   NULL, LOAD, SET_SLICECOLOR, errorcode);
}

void Loadslice(const char *type, int axis, float distance) {
  int count = 0;
  for(int i = 0; i < global_scase.slicecoll.nmultisliceinfo; i++) {
    multislicedata *mslicei;
    slicedata *slicei;
    int j;
    float delta_orig;

    mslicei = global_scase.slicecoll.multisliceinfo + i;
    if(mslicei->nslices <= 0) continue;
    slicei = global_scase.slicecoll.sliceinfo + mslicei->islices[0];
    if(MatchUpper(slicei->label.longlabel, type) == 0) continue;
    if(slicei->idir != axis) continue;
    delta_orig = slicei->position_orig - distance;
    if(delta_orig < 0.0) delta_orig = -delta_orig;
    if(delta_orig > slicei->delta_orig) continue;

    for(j = 0; j < mslicei->nslices; j++) {
      LoadSliceMenu(mslicei->islices[j]);
      count++;
    }
    break;
  }
  if(count == 0)
    fprintf(stderr,
            "*** Error: Slice files of type %s failed to "
            "load\n",
            type);
}

void Loadvslice(const char *type, int axis, float distance) {
  float delta_orig;
  int count = 0;
  for(int i = 0; i < global_scase.slicecoll.nmultivsliceinfo; i++) {
    multivslicedata *mvslicei;
    int j;
    slicedata *slicei;

    mvslicei = global_scase.slicecoll.multivsliceinfo + i;
    if(mvslicei->nvslices <= 0) continue;
    slicei = global_scase.slicecoll.sliceinfo + mvslicei->ivslices[0];
    if(MatchUpper(slicei->label.longlabel, type) == 0) continue;
    if(slicei->idir != axis) continue;
    delta_orig = slicei->position_orig - distance;
    if(delta_orig < 0.0) delta_orig = -delta_orig;
    if(delta_orig > slicei->delta_orig) continue;

    for(j = 0; j < mvslicei->nvslices; j++) {
      LoadVSliceMenu(mvslicei->ivslices[j]);
      count++;
    }
    break;
  }
  if(count == 0)
    fprintf(stderr,
            "*** Error: Vector slice files of type %s failed "
            "to load\n",
            type);
}

void Unloadslice(int value) {
  int errorcode;

  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  if(value >= 0) {
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo + value;

    if(slicei->slice_filetype == SLICE_GEOM) {
      ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, 0,
                   &errorcode);
    }
    else {
      ReadSlice("", value, ALL_FRAMES, NULL, UNLOAD, SET_SLICECOLOR,
                &errorcode);
    }
  }
  if(value <= -3) {
    UnloadBoundaryMenu(-3 - value);
  }
  else {
    if(value == UNLOAD_ALL) {
      for(size_t i = 0; i < global_scase.slicecoll.nsliceinfo; i++) {
        slicedata *slicei;

        slicei = global_scase.slicecoll.sliceinfo + i;
        if(slicei->slice_filetype == SLICE_GEOM) {
          ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, 0,
                       &errorcode);
        }
        else {
          ReadSlice("", i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR,
                    &errorcode);
        }
      }
      for(size_t i = 0; i < global_scase.npatchinfo; i++) {
        patchdata *patchi;

        patchi = global_scase.patchinfo + i;
        if(patchi->filetype_label != NULL &&
           strcmp(patchi->filetype_label, "INCLUDE_GEOM") == 0) {
          UnloadBoundaryMenu(i);
        }
      }
    }
  }
}

/// @brief Unload all the currently loaded data.
int Unloadall() {
  int errorcode = 0;

  if(scriptoutstream != NULL) {
    fprintf(scriptoutstream, "UNLOADALL\n");
  }
  if(global_scase.paths.hrr_csv_filename != NULL) {
    ReadHRR(&global_scase, UNLOAD);
  }
  if(nvolrenderinfo > 0) {
    LoadVolsmoke3DMenu(UNLOAD_ALL);
  }
  for(size_t i = 0; i < global_scase.slicecoll.nsliceinfo; i++) {
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo + i;
    if(slicei->loaded == 1) {
      if(slicei->slice_filetype == SLICE_GEOM) {
        ReadGeomData(slicei->patchgeom, slicei, UNLOAD, ALL_FRAMES, NULL, 0,
                     &errorcode);
      }
      else {
        ReadSlice(slicei->file, i, ALL_FRAMES, NULL, UNLOAD, DEFER_SLICECOLOR,
                  &errorcode);
      }
    }
  }
  for(size_t i = 0; i < global_scase.nplot3dinfo; i++) {
    ReadPlot3D("", i, UNLOAD, &errorcode);
  }
  for(size_t i = 0; i < global_scase.npatchinfo; i++) {
    ReadBoundary(i, UNLOAD, &errorcode);
  }
  for(size_t i = 0; i < global_scase.npartinfo; i++) {
    ReadPart("", i, UNLOAD, &errorcode);
  }
  for(size_t i = 0; i < global_scase.nisoinfo; i++) {
    ReadIso("", i, UNLOAD, NULL, &errorcode);
  }
  for(size_t i = 0; i < global_scase.nzoneinfo; i++) {
    ReadZone(i, UNLOAD, &errorcode);
  }
  for(size_t i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++) {
    ReadSmoke3D(ALL_SMOKE_FRAMES, i, UNLOAD, FIRST_TIME, &errorcode);
  }
  if(nvolrenderinfo > 0) {
    UnLoadVolsmoke3DMenu(UNLOAD_ALL);
  }
  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  return errorcode;
}

void Unloadtour() { TourMenu(MENU_TOUR_MANUAL); }

/// @brief Exit smokeview.
void ExitSmokeview() { exit(EXIT_SUCCESS); }

int Setviewpoint(const char *viewpoint) {
  int count = 0;
  int errorcode = 0;
  for(cameradata *ca = camera_list_first.next; ca->next != NULL;
      ca = ca->next) {
    if(strcmp(viewpoint, ca->name) == 0) {
      ResetMenu(ca->view_id);
      count++;
      break;
    }
  }
  if(count == 0) {
    errorcode = 1;
    fprintf(stderr, "*** Error: The viewpoint %s was not found\n", viewpoint);
  }
  fprintf(stderr, "Viewpoint set to %s\n", camera_current->name);
  return errorcode;
}

/// @brief Switch to a preset orthographic view.
/// @param viewpoint A string describe the view. Currently one of:
/// - "XMIN"
/// - "XMAX"
/// - "YMIN"
/// - "YMAX"
/// - "ZMIN"
/// - "ZMAX"
/// @return
int SetOrthoPreset(const char *viewpoint) {
  int command;
  fprintf(stderr, "setting ortho %s\n", viewpoint);
  if(STRCMP(viewpoint, "XMIN") == 0) {
    command = SCRIPT_VIEWXMIN;
  }
  else if(STRCMP(viewpoint, "XMAX") == 0) {
    command = SCRIPT_VIEWXMAX;
  }
  else if(STRCMP(viewpoint, "YMIN") == 0) {
    command = SCRIPT_VIEWYMIN;
  }
  else if(STRCMP(viewpoint, "YMAX") == 0) {
    command = SCRIPT_VIEWYMAX;
  }
  else if(STRCMP(viewpoint, "ZMIN") == 0) {
    command = SCRIPT_VIEWZMIN;
  }
  else if(STRCMP(viewpoint, "ZMAX") == 0) {
    command = SCRIPT_VIEWZMAX;
  }
  else {
    return 1;
  }
  ScriptViewXYZMINMAXOrtho(command);
  return 0;
}

int GetClippingMode() { return clip_mode; }

void SetClippingMode(int mode) {
  clip_mode = mode;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipX(int clipMin, float min, int clipMax, float max) {
  clipinfo.clip_xmin = clipMin;
  clipinfo.xmin = min;

  clipinfo.clip_xmax = clipMax;
  clipinfo.xmax = max;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipXMin(int flag, float value) {
  clipinfo.clip_xmin = flag;
  clipinfo.xmin = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipXMax(int flag, float value) {
  clipinfo.clip_xmax = flag;
  clipinfo.xmax = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipY(int clipMin, float min, int clipMax, float max) {
  clipinfo.clip_ymin = clipMin;
  clipinfo.ymin = min;

  clipinfo.clip_ymax = clipMax;
  clipinfo.ymax = max;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipYMin(int flag, float value) {
  clipinfo.clip_ymin = flag;
  clipinfo.ymin = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipYMax(int flag, float value) {
  clipinfo.clip_ymax = flag;
  clipinfo.ymax = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipZ(int clipMin, float min, int clipMax, float max) {
  clipinfo.clip_zmin = clipMin;
  clipinfo.zmin = min;

  clipinfo.clip_zmax = clipMax;
  clipinfo.zmax = max;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipZMin(int flag, float value) {
  clipinfo.clip_zmin = flag;
  clipinfo.zmin = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

void SetSceneclipZMax(int flag, float value) {
  clipinfo.clip_zmax = flag;
  clipinfo.zmax = value;
  updatefacelists = 1;
  GLUIUpdateClip();
  GLUIUpdateClipAll();
}

int Setrenderdir(const char *dir) {
  // TODO: as lua gives us consts, but most smv code uses non-const, we
  // must make a non-const copy
  int l = strlen(dir);
  char *dir_path_temp = malloc(l + 1);
  strncpy(dir_path_temp, dir, l + 1);
  // TODO: should we make the directory at this point?
  if(dir != NULL && strlen(dir_path_temp) > 0) {
#if defined(__MINGW32__)
    fprintf(stderr, "%s\n", "making directory(mingw)\n");
    mkdir(dir_path_temp);
#elif defined(WIN32)
    fprintf(stderr, "%s\n", "making directory(win32)\n");
    CreateDirectory(dir_path_temp, NULL);
#else // linux or osx
    fprintf(stderr, "%s\n", "making directory(linux/osx)\n");
    mkdir(dir_path_temp, 0755);
#endif
    if(Writable(dir_path_temp) == NO) {
      fprintf(stderr,
              "*** Error: Cannot write to the RENDERDIR "
              "directory: %s\n",
              dir_path_temp);
      return 1;
    }
    else {
      free(script_dir_path);
      script_dir_path = dir_path_temp;
      return 0;
    }
  }
  else {
    // TODO: why would we ever want to set the render directory to NULL
    script_dir_path = NULL;
    FREEMEMORY(dir_path_temp);
    return 1;
  }
}

void Setcolorbarindex(int chosen_index) { UpdateRGBColors(chosen_index); }

int Getcolorbarindex() { return global_colorbar_index; }

void Setwindowsize(int width, int height) {
  glutReshapeWindow(width, height);
  ResizeWindow(width, height);
  ReshapeCB(width, height);
}

/// @brief Set the visibility of the grid.
/// @param selection One of:
/// - #NOGRID_NOPROBE
/// - #GRID_NOPROBE
/// - #GRID_PROBE
/// - #NOGRID_PROBE
void Setgridvisibility(int selection) {
  visGrid = selection;
  // selection may be one of:
  if(visGrid == GRID_PROBE || visGrid == NOGRID_PROBE) visgridloc = 1;
}

void Setgridparms(int x_vis, int y_vis, int z_vis, int x_plot, int y_plot,
                  int z_plot) {
  visx_all = x_vis;
  visy_all = y_vis;
  visz_all = z_vis;

  iplotx_all = x_plot;
  iploty_all = y_plot;
  iplotz_all = z_plot;

  if(iplotx_all > nplotx_all - 1) iplotx_all = 0;
  if(iploty_all > nploty_all - 1) iploty_all = 0;
  if(iplotz_all > nplotz_all - 1) iplotz_all = 0;
}

/// @brief Set the firection of the colorbar.
/// @param flip Boolean. If true, the colorbar runs in the opposite direction
/// than default.
void Setcolorbarflip(int flip) {
  colorbar_flip = flip;
  GLUIUpdateColorbarFlip();
  UpdateRGBColors(COLORBAR_INDEX_NONE);
}

/// @brief Get whether the direction of the colorbar is flipped.
/// @return
int Getcolorbarflip() { return colorbar_flip; }

// Camera API
// These function live-modify the current view by modifying "camera_current".
void CameraSetRotationType(int rotation_typev) {
  rotation_type = rotation_typev;
  camera_current->rotation_type = rotation_typev;
  GLUIRotationTypeCB(rotation_type);
  GLUIUpdateRotationType(rotation_type);
  HandleRotationType(ROTATION_2AXIS);
}

int CameraGetRotationType() { return camera_current->rotation_type; }

// TODO: How does the rotation index work.
void CameraSetRotationIndex(int rotation_index) {
  camera_current->rotation_index = rotation_index;
}

int CameraGetRotationIndex() { return camera_current->rotation_index; }

void CameraSetViewdir(float xcen, float ycen, float zcen) {
  camera_current->xcen = xcen;
  camera_current->ycen = ycen;
  camera_current->zcen = zcen;
}

// xcen
void CameraSetXcen(float xcen) { camera_current->xcen = xcen; }
float CameraGetXcen() { return camera_current->xcen; }

// ycen
void CameraSetYcen(float ycen) { camera_current->ycen = ycen; }
float CameraGetYcen() { return camera_current->ycen; }

// zcen
void CameraSetZcen(float zcen) { camera_current->zcen = zcen; }
float CameraGetZcen() { return camera_current->zcen; }

// eyex
void CameraModEyex(float delta) {
  camera_current->eye[0] = camera_current->eye[0] + delta;
}
void CameraSetEyex(float eyex) { camera_current->eye[0] = eyex; }

float CameraGetEyex() { return camera_current->eye[0]; }

// eyey
void CameraModEyey(float delta) {
  camera_current->eye[1] = camera_current->eye[1] + delta;
}
void CameraSetEyey(float eyey) { camera_current->eye[1] = eyey; }
float CameraGetEyey() { return camera_current->eye[1]; }

// eyez
void CameraModEyez(float delta) {
  camera_current->eye[2] = camera_current->eye[2] + delta;
}
void CameraSetEyez(float eyez) { camera_current->eye[2] = eyez; }
float CameraGetEyez() { return camera_current->eye[2]; }

// azimuth
void CameraModAz(float delta) {
  camera_current->az_elev[0] = camera_current->az_elev[0] + delta;
}
void CameraSetAz(float az) { camera_current->az_elev[0] = az; }
float CameraGetAz() { return camera_current->az_elev[0]; }

// elevation
void CameraModElev(float delta) {
  camera_current->az_elev[1] = camera_current->az_elev[1] + delta;
}
void CameraSetElev(float elev) { camera_current->az_elev[1] = elev; }
float CameraGetElev() { return camera_current->az_elev[1]; }

void MoveScene(int xm, int ym);
int CameraZoomToFit() {
  float offset = (global_scase.zbar - global_scase.ybar) / 2.0;
  camera_current->eye[1] += offset * 2;
  eye_xyz0[1] = camera_current->eye[1];
  in_external = 0;
  first_display = 0;
  return 0;
}

// projection_type
void CameraToggleProjectionType() {
  camera_current->projection_type = 1 - camera_current->projection_type;
}
int CameraSetProjectionType(int pt) {
  camera_current->projection_type = pt;
  projection_type = pt;
  ZoomMenu(UPDATE_PROJECTION);
  camera_current->projection_type = projection_type;
  // 1 is orthogonal
  // 0 is perspective
  return 0;
}
int CameraGetProjectionType() { return camera_current->projection_type; }

// .ini config options
// *** COLOR/LIGHTING ***
int SetAmbientlight(float r, float g, float b) {
  ambientlight[0] = r;
  ambientlight[1] = g;
  ambientlight[2] = b;
  return 0;
} // AMBIENTLIGHT

int SetBackgroundcolor(float r, float g, float b) {
  backgroundbasecolor[0] = r;
  backgroundbasecolor[1] = g;
  backgroundbasecolor[2] = b;
  GLUISetColorControls();
  GLUTPOSTREDISPLAY;
  return 0;
} // BACKGROUNDCOLOR

int SetBlockcolor(float r, float g, float b) {
  global_scase.color_defs.block_ambient2[0] = r;
  global_scase.color_defs.block_ambient2[1] = g;
  global_scase.color_defs.block_ambient2[2] = b;
  return 0;
} // BLOCKCOLOR

int SetBlockshininess(float v) {
  block_shininess = v;
  return 0;
} // BLOCKSHININESS

int SetBlockspecular(float r, float g, float b) {
  block_specular2[0] = r;
  block_specular2[1] = g;
  block_specular2[2] = b;
  return 0;
} // BLOCKSPECULAR

int SetBoundcolor(float r, float g, float b) {
  boundcolor[0] = r;
  boundcolor[1] = g;
  boundcolor[2] = b;
  return 0;
} // BOUNDCOLOR

// int set_colorbar_textureflag(int v)  {
//   usetexturebar = v;
//   return 0;
// }
// int get_colorbar_textureflag() {
//   return usetexturebar;
// }

int SetColorbarColors(int ncolors, float *colors) {

  float *rgb_ini_copy;
  float *rgb_ini_copy_p;
  CheckMemory;
  if(NewMemory((void **)&rgb_ini_copy, 4 * ncolors * sizeof(float)) == 0)
    return 2;
  rgb_ini_copy_p = rgb_ini_copy;
  for(size_t i = 0; i < ncolors; i++) {
    float *r = rgb_ini_copy_p;
    float *g = rgb_ini_copy_p + 1;
    float *b = rgb_ini_copy_p + 2;
    *r = colors[i * 3];
    *g = colors[i * 3 + 1];
    *b = colors[i * 3 + 2];
    rgb_ini_copy_p += 3;
  }

  FREEMEMORY(rgb_ini);
  rgb_ini = rgb_ini_copy;
  nrgb_ini = ncolors;
  InitRGB();
  return 0;
}

int SetColor2barColors(int ncolors, float *colors) {
  float *rgb_ini_copy;
  float *rgb_ini_copy_p;
  CheckMemory;
  if(NewMemory((void **)&rgb_ini_copy, 4 * ncolors * sizeof(float)) == 0)
    return 2;
  ;
  rgb_ini_copy_p = rgb_ini_copy;
  for(size_t i = 0; i < ncolors; i++) {
    float *r = rgb_ini_copy_p;
    float *g = rgb_ini_copy_p + 1;
    float *b = rgb_ini_copy_p + 2;
    *r = colors[i * 3];
    *g = colors[i * 3 + 1];
    *b = colors[i * 3 + 2];
    rgb_ini_copy_p += 3;
  }

  FREEMEMORY(rgb2_ini);
  rgb2_ini = rgb_ini_copy;
  nrgb2_ini = ncolors;
  return 0;
}

int SetDiffuselight(float r, float g, float b) {
  diffuselight[0] = r;
  diffuselight[1] = g;
  diffuselight[2] = b;
  return 0;
} // DIFFUSELIGHT

int SetDirectioncolor(float r, float g, float b) {
  direction_color[0] = r;
  direction_color[1] = g;
  direction_color[2] = b;
  return 0;
} // DIRECTIONCOLOR

/// @brief Set whether the foreground/background colors are flipped.
///
/// By default they are flipped.
/// @param v
/// @return
int SetFlip(int v) {
  background_flip = v;
  return 0;
} // FLIP

int GetFlip() { return background_flip; }

int SetForegroundcolor(float r, float g, float b) {
  foregroundbasecolor[0] = r;
  foregroundbasecolor[1] = g;
  foregroundbasecolor[2] = b;
  return 0;
} // FOREGROUNDCOLOR

int SetHeatoffcolor(float r, float g, float b) {
  heatoffcolor[0] = r;
  heatoffcolor[1] = g;
  heatoffcolor[2] = b;
  return 0;
} // HEATOFFCOLOR

int SetHeatoncolor(float r, float g, float b) {
  heatoncolor[0] = r;
  heatoncolor[1] = g;
  heatoncolor[2] = b;
  return 0;
} // HEATONCOLOR

int SetIsocolors(float shininess, float transparency, int transparency_option,
                 int opacity_change, float specular[3], int n_colors,
                 float colors[][4]) {
  iso_shininess = shininess;
  iso_transparency = transparency;
  iso_transparency_option = transparency_option;
  iso_opacity_change = opacity_change;
  iso_specular[0] = specular[0];
  iso_specular[1] = specular[1];
  iso_specular[2] = specular[2];

  for(int nn = 0; nn < n_colors; nn++) {
    float *iso_color;
    iso_color = iso_colors + 4 * nn;
    iso_color[0] = CLAMP(colors[nn][0], 0.0, 1.0);
    iso_color[1] = CLAMP(colors[nn][1], 0.0, 1.0);
    iso_color[2] = CLAMP(colors[nn][2], 0.0, 1.0);
    iso_color[3] = CLAMP(colors[nn][3], 0.0, 1.0);
  }
  UpdateIsoColors();
  GLUIUpdateIsoColorlevel();
  return 0;
} // ISOCOLORS

int SetColortable(int ncolors, int colors[][4], char **names) {
  int nctableinfo;

  colortabledata *ctableinfo = NULL;
  nctableinfo = ncolors;
  nctableinfo = MAX(nctableinfo, 0);
  if(nctableinfo > 0) {
    NewMemory((void **)&ctableinfo, nctableinfo * sizeof(colortabledata));
    for(size_t i = 0; i < nctableinfo; i++) {
      colortabledata *rgbi;
      rgbi = ctableinfo + i;
      // TODO: This sets the default alpha value to 255, as per the
      // original ReadSMV.c function, but is defunct in this context
      // as this value is required by the function prototype.
      // color[i][3] = 255;
      strcpy(rgbi->label, names[i]);
      rgbi->color[0] = CLAMP(colors[i][0], 0, 255);
      rgbi->color[1] = CLAMP(colors[i][1], 0, 255);
      rgbi->color[2] = CLAMP(colors[i][2], 0, 255);
      rgbi->color[3] = CLAMP(colors[i][3], 0, 255);
    }
    UpdateColorTable(ctableinfo, nctableinfo);
    FREEMEMORY(ctableinfo);
  }
  return 0;
} // COLORTABLE

int SetLightpos0(float a, float b, float c, float d) {
  light_position0[0] = a;
  light_position0[1] = a;
  light_position0[2] = a;
  light_position0[3] = a;
  return 0;
} // LIGHTPOS0

int SetLightpos1(float a, float b, float c, float d) {
  light_position1[0] = a;
  light_position1[1] = a;
  light_position1[2] = a;
  light_position1[3] = a;
  return 0;
} // LIGHTPOS1

int SetSensorcolor(float r, float g, float b) {
  sensorcolor[0] = r;
  sensorcolor[1] = g;
  sensorcolor[2] = b;
  return 0;
} // SENSORCOLOR

int SetSensornormcolor(float r, float g, float b) {
  sensornormcolor[0] = r;
  sensornormcolor[1] = g;
  sensornormcolor[2] = b;
  return 0;
} // SENSORNORMCOLOR

int SetBw(int geo_setting, int data_setting) {
  setbw = geo_setting;
  setbwdata = data_setting;
  return 0;
} // SETBW

int SetSprinkleroffcolor(float r, float g, float b) {
  sprinkoffcolor[0] = r;
  sprinkoffcolor[1] = g;
  sprinkoffcolor[2] = b;
  return 0;
} // SPRINKOFFCOLOR

int SetSprinkleroncolor(float r, float g, float b) {
  sprinkoncolor[0] = r;
  sprinkoncolor[1] = g;
  sprinkoncolor[2] = b;
  return 0;
} // SPRINKONCOLOR

int SetStaticpartcolor(float r, float g, float b) {
  static_color[0] = r;
  static_color[1] = g;
  static_color[2] = b;
  return 0;
} // STATICPARTCOLOR

int SetTimebarcolor(float r, float g, float b) {
  timebarcolor[0] = r;
  timebarcolor[1] = g;
  timebarcolor[2] = b;
  return 0;
} // TIMEBARCOLOR

int SetVentcolor(float r, float g, float b) {
  ventcolor[0] = r;
  ventcolor[1] = g;
  ventcolor[2] = b;
  return 0;
} // VENTCOLOR

// --    *** SIZES/OFFSETS ***
int SetGridlinewidth(float v) {
  gridlinewidth = v;
  return 0;
} // GRIDLINEWIDTH

int SetIsolinewidth(float v) {
  isolinewidth = v;
  return 0;
} // ISOLINEWIDTH

int SetIsopointsize(float v) {
  isopointsize = v;
  return 0;
} // ISOPOINTSIZE

int SetLinewidth(float v) {
  global_scase.linewidth = v;
  return 0;
} // LINEWIDTH

int SetPartpointsize(float v) {
  partpointsize = v;
  return 0;
} // PARTPOINTSIZE

int SetPlot3dlinewidth(float v) {
  plot3dlinewidth = v;
  return 0;
} // PLOT3DLINEWIDTH

int SetPlot3dpointsize(float v) {
  plot3dpointsize = v;
  return 0;
} // PLOT3DPOINTSIZE

int SetSensorabssize(float v) {
  sensorabssize = v;
  return 0;
} // SENSORABSSIZE

int SetSensorrelsize(float v) {
  sensorrelsize = v;
  return 0;
} // SENSORRELSIZE

int SetSliceoffset(float v) {
  sliceoffset_factor = v;
  return 0;
} // SLICEOFFSET

int SetSmoothlines(int v) {
  antialiasflag = v;
  return 0;
} // SMOOTHLINES

int SetSpheresegs(int v) {
  device_sphere_segments = v;
  return 0;
} // SPHERESEGS

int SetSprinklerabssize(float v) {
  sprinklerabssize = v;
  return 0;
} // SPRINKLERABSSIZE

int SetStreaklinewidth(float v) {
  streaklinewidth = v;
  return 0;
} // STREAKLINEWIDTH

int SetTicklinewidth(float v) {
  ticklinewidth = v;
  return 0;
} // TICKLINEWIDTH

int SetUsenewdrawface(int v) {
  blockage_draw_option = v;
  return 0;
} // USENEWDRAWFACE

int SetVeclength(float vf, int vec_uniform_length_in,
                 int vec_uniform_spacing_in) {
  vecfactor = vf;
  vec_uniform_spacing = vec_uniform_spacing_in;
  vec_uniform_length = vec_uniform_length_in;
  return 0;
} // VECLENGTH

int SetVectorlinewidth(float a, float b) {
  vectorlinewidth = a;
  slice_line_contour_width = b;
  return 0;
} // VECTORLINEWIDTH

int SetVectorpointsize(float v) {
  vectorpointsize = v;
  return 0;
} // VECTORPOINTSIZE

int SetVentlinewidth(float v) {
  global_scase.ventlinewidth = v;
  return 0;
} // VENTLINEWIDTH

int SetVentoffset(float v) {
  ventoffset_factor = v;
  return 0;
} // VENTOFFSET

int SetWindowoffset(int v) {
  titlesafe_offsetBASE = v;
  return 0;
} // WINDOWOFFSET

int SetWindowwidth(int v) {
  screenWidth = v;
  return 0;
} // WINDOWWIDTH

int SetWindowheight(int v) {
  screenHeight = v;
  return 0;
} // WINDOWHEIGHT

// --  *** DATA LOADING ***
int SetBoundzipstep(int v) {
  tload_zipstep = v;
  return 0;
} // BOUNDZIPSTEP

int SetIsozipstep(int v) {
  tload_zipstep = v;
  return 0;
} // ISOZIPSTEP

int SetNopart(int v) {
  nopart = v;
  return 0;
} // NOPART

// int set_partpointstep(int v) {
//   partpointstep = v;
//   return 0;
// } // PARTPOINTSTEP

int SetSliceaverage(int flag, float interval, int vis) {
  slice_average_flag = flag;
  slice_average_interval = interval;
  vis_slice_average = vis;
  return 0;
} // SLICEAVERAGE

int SetSlicedataout(int v) {
  output_slicedata = v;
  return 0;
} // SLICEDATAOUT

int SetSlicezipstep(int v) {
  tload_zipstep = v;
  return 0;
} // SLICEZIPSTEP

int SetSmoke3dzipstep(int v) {
  tload_zipstep = v;
  return 0;
} // SMOKE3DZIPSTEP

int SetUserrotate(int index, int show_center, float x, float y, float z) {
  glui_rotation_index = index;
  slice_average_interval = show_center;
  xcenCUSTOM = x;
  ycenCUSTOM = y;
  zcenCUSTOM = z;
  return 0;
} // USER_ROTATE

// --  *** VIEW PARAMETERS ***
int SetAperture(int v) {
  apertureindex = v;
  return 0;
} // APERTURE

// int set_axissmooth(int v) {
//   axislabels_smooth = v;
//   return 0;
// } // AXISSMOOTH

// provided above
int SetBlocklocation(int v) {
  blocklocation = v;
  return 0;
} // BLOCKLOCATION

int SetBoundarytwoside(int v) {
  showpatch_both = v;
  return 0;
} // BOUNDARYTWOSIDE

int SetClip(float v_near, float v_far) {
  nearclip = v_near;
  farclip = v_far;
  return 0;
} // CLIP

int SetContourtype(int v) {
  contour_type = v;
  return 0;
} // CONTOURTYPE

int SetCullfaces(int v) {
  cullfaces = v;
  return 0;
} // CULLFACES

int SetTexturelighting(int v) {
  enable_texture_lighting = v;
  return 0;
} // ENABLETEXTURELIGHTING

int SetEyeview(int v) {
  rotation_type = v;
  return 0;
} // EYEVIEW

int SetEyex(float v) {
  eyexfactor = v;
  return 0;
} // EYEX

int SetEyey(float v) {
  eyeyfactor = v;
  return 0;
} // EYEY

int SetEyez(float v) {
  eyezfactor = v;
  return 0;
} // EYEZ

int SetFontsize(int v) {
  FontMenu(v);
  return 0;
} // FONTSIZE

int SetFrameratevalue(int v) {
  frameratevalue = v;
  return 0;
} // FRAMERATEVALUE

// int set_geomshow(int )
// GEOMSHOW
int SetShowfacesSolid(int v) {
  frameratevalue = v;
  return 0;
}
int SetShowfacesOutline(int v) {
  show_faces_outline = v;
  return 0;
}
int SetSmoothgeomnormal(int v) {
  smooth_geom_normal = v;
  return 0;
}
int SetGeomvertexag(int v) {
  geom_vert_exag = v;
  return 0;
}

int SetGversion(int v) {
  vis_title_gversion = v;
  return 0;
} // GVERSION

int SetIsotran2(int v) {
  transparent_state = v;
  return 0;
} // ISOTRAN2

int SetMeshvis(int n, int vals[]) {
  meshdata *meshi;
  for(size_t i = 0; i < n; i++) {
    if(i > global_scase.meshescoll.nmeshes - 1) break;
    meshi = global_scase.meshescoll.meshinfo + i;
    meshi->blockvis = vals[i];
    ONEORZERO(meshi->blockvis);
  }
  return 0;
} // MESHVIS

int SetMeshoffset(int meshnum, int value) {
  if(meshnum >= 0 && meshnum < global_scase.meshescoll.nmeshes) {
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + meshnum;
    meshi->mesh_offset_ptr = meshi->mesh_offset;
    return 0;
  }
  return 1;
} // MESHOFFSET

int SetNorthangle(int vis, float x, float y, float z) {
  vis_northangle = vis;
  northangle_position[0] = x;
  northangle_position[1] = y;
  northangle_position[2] = z;
  return 0;
} // NORTHANGLE

int SetOffsetslice(int v) {
  offset_slice = v;
  return 0;
} // OFFSETSLICE

int SetOutlinemode(int a, int b) {
  outline_mode = a;
  outline_color_flag = b;
  return 0;
} // OUTLINEMODE

int SetP3dsurfacetype(int v) {
  p3dsurfacetype = v;
  return 0;
} // P3DSURFACETYPE

int SetP3dsurfacesmooth(int v) {
  p3dsurfacesmooth = v;
  return 0;
} // P3DSURFACESMOOTH

int SetScaledfont(int height2d, float height2dwidth, int thickness2d,
                  int height3d, float height3dwidth, int thickness3d) {
  scaled_font2d_height = height2d;
  scaled_font2d_height2width = height2dwidth;
  scaled_font3d_height = height3d;
  scaled_font3d_height2width = height3dwidth;
  scaled_font2d_thickness = thickness2d;
  scaled_font3d_thickness = thickness3d;
  return 0;
} // SCALEDFONT

int GetScaledfontHeight2d() { return scaled_font2d_height; }

int SetScaledfontHeight2d(int height2d) {
  scaled_font2d_height = height2d;
  return 0;
}

int SetShowalltextures(int v) {
  showall_textures = v;
  return 0;
} // SHOWALLTEXTURES

int SetShowaxislabels(int v) {
  visaxislabels = v;
  return 0;
} // SHOWAXISLABELS TODO: duplicate

int SetShowblocklabel(int v) {
  visMeshlabel = v;
  return 0;
} // SHOWBLOCKLABEL

int SetShowblocks(int v) {
  visBlocks = v;
  return 0;
} // SHOWBLOCKS

int SetShowcadandgrid(int v) {
  show_cad_and_grid = v;
  return 0;
} // SHOWCADANDGRID

int SetShowcadopaque(int v) {
  viscadopaque = v;
  return 0;
} // SHOWCADOPAQUE

int SetShowceiling(int v) {
  visCeiling = v;
  return 0;
} // SHOWCEILING

int SetShowcolorbars(int v) {
  visColorbarVertical = v;
  return 0;
} // SHOWCOLORBARS

int SetShowcvents(int a, int b) {
  visCircularVents = a;
  circle_outline = b;
  return 0;
} // SHOWCVENTS

int SetShowdummyvents(int v) {
  visDummyVents = v;
  return 0;
} // SHOWDUMMYVENTS

int SetShowfloor(int v) {
  visFloor = v;
  return 0;
} // SHOWFLOOR

int SetShowframe(int v) {
  visFrame = v;
  return 0;
} // SHOWFRAME

int SetShowframelabel(int v) {
  visFramelabel = v;
  return 0;
} // SHOWFRAMELABEL

int SetShowframerate(int v) {
  visFramerate = v;
  return 0;
} // SHOWFRAMERATE

int SetShowgrid(int v) {
  visGrid = v;
  return 0;
} // SHOWGRID

int SetShowgridloc(int v) {
  visgridloc = v;
  return 0;
} // SHOWGRIDLOC

int SetShowhmstimelabel(int v) {
  vishmsTimelabel = v;
  return 0;
} // SHOWHMSTIMELABEL

int SetShowhrrcutoff(int v) {
  vis_hrr_label = v;
  return 0;
} // SHOWHRRCUTOFF

int SetShowiso(int v) {
  visAIso = v;
  return 0;
} // SHOWISO

int SetShowisonormals(int v) {
  show_iso_normal = v;
  return 0;
} // SHOWISONORMALS

int SetShowlabels(int v) {
  visLabels = v;
  return 0;
} // SHOWLABELS

#ifdef pp_memstatus
int SetShowmemload(int v) {
  visAvailmemory = v;
  return 0;
} // SHOWMEMLOAD
#endif

// int set_shownormalwhensmooth(int v); // SHOWNORMALWHENSMOOTH
int SetShowopenvents(int a, int b) {
  visOpenVents = a;
  visOpenVentsAsOutline = b;
  return 0;
} // SHOWOPENVENTS

int SetShowothervents(int v) {
  global_scase.visOtherVents = v;
  return 0;
} // SHOWOTHERVENTS

int SetShowsensors(int a, int b) {
  visSensor = a;
  visSensorNorm = b;
  return 0;
} // SHOWSENSORS

int SetShowsliceinobst(int v) {
  global_scase.show_slice_in_obst = v;
  return 0;
} // SHOWSLICEINOBST

int SetShowsmokepart(int v) {
  visSmokePart = v;
  return 0;
} // SHOWSMOKEPART

int SetShowsprinkpart(int v) {
  visSprinkPart = v;
  return 0;
} // SHOWSPRINKPART

int SetShowstreak(int show, int step, int showhead, int index) {
  streak5show = show;
  streak5step = step;
  showstreakhead = showhead;
  streak_index = index;
  return 0;
} // SHOWSTREAK

int SetShowterrain(int v) {
  global_scase.visTerrainType = v;
  return 0;
} // SHOWTERRAIN

int SetShowthreshold(int a, int b, float c) {
  vis_threshold = a;
  vis_onlythreshold = b;
  temp_threshold = c;
  return 0;
} // SHOWTHRESHOLD

int SetShowticks(int v) {
  visFDSticks = v;
  return 0;
} // SHOWTICKS

int SetShowtimebar(int v) {
  visTimebar = v;
  return 0;
} // SHOWTIMEBAR

int SetShowtimelabel(int v) {
  visTimelabel = v;
  return 0;
} // SHOWTIMELABEL

int SetShowtitle(int v) {
  vis_title_fds = v;
  return 0;
} // SHOWTITLE

int SetShowtracersalways(int v) {
  show_tracers_always = v;
  return 0;
} // SHOWTRACERSALWAYS

int SetShowtriangles(int a, int b, int c, int d, int e, int f) {
  show_iso_shaded = a;
  show_iso_outline = b;
  show_iso_points = c;
  show_iso_normal = d;
  smooth_iso_normal = e;
  return 0;
} // SHOWTRIANGLES

int SetShowtransparent(int v) {
  visTransparentBlockage = v;
  return 0;
} // SHOWTRANSPARENT

int SetShowtransparentvents(int v) {
  show_transparent_vents = v;
  return 0;
} // SHOWTRANSPARENTVENTS

int SetShowtrianglecount(int v) {
  show_triangle_count = v;
  return 0;
} // SHOWTRIANGLECOUNT

int SetShowventflow(int a, int b, int c, int d, int e) {
  visVentHFlow = a;
  visventslab = b;
  visventprofile = c;
  visVentVFlow = d;
  visVentMFlow = e;
  return 0;
} // SHOWVENTFLOW

int SetShowvents(int v) {
  visVents = v;
  return 0;
} // SHOWVENTS

int SetShowwalls(int v) {
  visWalls = v;
  return 0;
} // SHOWWALLS

int SetSkipembedslice(int v) {
  skip_slice_in_embedded_mesh = v;
  return 0;
} // SKIPEMBEDSLICE

#ifdef pp_SLICEUP
int set_slicedup(int a, int b) {
  slicedup_option = a;
  vectorslicedup_option = b;
  return 0;
} // SLICEDUP
#endif

int SetSmokesensors(int a, int b) {
  show_smokesensors = a;
  test_smokesensors = b;
  return 0;
} // SMOKESENSORS

// int set_smoothblocksolid(int v); // SMOOTHBLOCKSOLID
#ifdef pp_LANG
int set_startuplang(const char *lang) {
  char *bufptr;

  strncpy(startup_lang_code, lang, 2);
  startup_lang_code[2] = '\0';
  if(tr_name == NULL) {
    int langlen;

    langlen = strlen(bufptr);
    NewMemory((void **)&tr_name, langlen + 48 + 1);
    strcpy(tr_name, bufptr);
  }
  return 0;
} // STARTUPLANG
#endif

int SetStereo(int v) {
  stereotype = v;
  return 0;
} // STEREO

int SetSurfinc(int v) {
  surfincrement = v;
  return 0;
} // SURFINC

int SetTerrainparams(int r_min, int g_min, int b_min, int r_max, int g_max,
                     int b_max, int v) {
  terrain_rgba_zmin[0] = r_min;
  terrain_rgba_zmin[1] = g_min;
  terrain_rgba_zmin[2] = b_min;
  terrain_rgba_zmin[0] = r_max;
  terrain_rgba_zmin[1] = g_max;
  terrain_rgba_zmin[2] = b_max;
  vertical_factor = v;
  return 0;
} // TERRAINPARMS

int SetTitlesafe(int v) {
  titlesafe_offset = v;
  return 0;
} // TITLESAFE

int SetTrainermode(int v) {
  trainer_mode = v;
  return 0;
} // TRAINERMODE

int SetTrainerview(int v) {
  trainerview = v;
  return 0;
} // TRAINERVIEW

int SetTransparent(int use_flag, float level) {
  use_transparency_data = use_flag;
  transparent_level = level;
  return 0;
} // TRANSPARENT

int SetTreeparms(int minsize, int visx, int visy, int visz) {
  mintreesize = minsize;
  vis_xtree = visx;
  vis_ytree = visy;
  vis_ztree = visz;
  return 0;
} // TREEPARMS

int SetTwosidedvents(int internal, int external) {
  show_bothsides_int = internal;
  show_bothsides_ext = external;
  return 0;
} // TWOSIDEDVENTS

int SetVectorskip(int v) {
  vectorskip = v;
  return 0;
} // VECTORSKIP

int SetVolsmoke(int a, int b, int c, int d, int e, float f, float g, float h,
                float i, float j, float k, float l) {
  glui_compress_volsmoke = a;
  use_multi_threading = b;
  load_at_rendertimes = c;
  volbw = d;
  show_volsmoke_moving = e;
  global_temp_min = f;
  global_temp_cb_min = g;
  global_temp_cb_max = h;
  fire_opacity_factor = i;
  mass_extinct = j;
  gpu_vol_factor = k;
  nongpu_vol_factor = l;
  return 0;
} // VOLSMOKE

int SetZoom(int a, float b) {
  zoomindex = a;
  zoom = b;
  return 0;
} // ZOOM

// *** MISC ***
int SetCellcentertext(int v) {
  show_slice_values_all_regions = v;
  return 0;
} // CELLCENTERTEXT

int SetInputfile(const char *filename) {
  size_t len;
  len = strlen(filename);

  FREEMEMORY(INI_fds_filein);
  if(NewMemory((void **)&INI_fds_filein, (unsigned int)(len + 1)) == 0)
    return 2;
  STRCPY(INI_fds_filein, filename);
  return 0;
} // INPUT_FILE

int SetLabelstartupview(const char *startupview) {
  strcpy(viewpoint_label_startup, startupview);
  update_startup_view = 3;
  return 0;
} // LABELSTARTUPVIEW

// DEPRECATED
// int set_pixelskip(int v) {
//   pixel_skip = v;
//   return 0;
// } // PIXELSKIP

int SetRenderclip(int use_flag, int left, int right, int bottom, int top) {
  clip_rendered_scene = use_flag;
  render_clip_left = left;
  render_clip_right = right;
  render_clip_bottom = bottom;
  render_clip_top = top;
  return 0;
} // RENDERCLIP

// DEPRECATED
// int set_renderfilelabel(int v) {
//   renderfilelabel = v;
//   return 0;
// } // RENDERFILELABEL

int SetRenderfiletype(int render, int movie) {
  render_filetype = render;
  movie_filetype = movie;
  return 0;
} // RENDERFILETYPE

int SetSkybox() {
  // skyboxdata *skyi;

  // free_skybox();
  // nskyboxinfo = 1;
  // NewMemory((void **)&skyboxinfo, nskyboxinfo*sizeof(skyboxdata));
  // skyi = skyboxinfo;

  // for(i = 0; i<6; i++){
  //   fgets(buffer, 255, stream);
  //   loadskytexture(buffer, skyi->face + i);
  // }
  assert(FFALSE);
  return 0;
} // SKYBOX TODO

// DEPRECATED
// int set_renderoption(int opt, int rows) {
//   render_option = opt;
//   nrender_rows = rows;
//   return 0;
// } // RENDEROPTION

int SetUnitclasses(int n, int indices[]) {

  for(size_t i = 0; i < n; i++) {
    if(i > nunitclasses - 1) continue;
    unitclasses[i].unit_index = indices[i];
  }
  return 0;
} // UNITCLASSES

int SetZaxisangles(float a, float b, float c) {
  zaxis_angles[0] = a;
  zaxis_angles[1] = b;
  zaxis_angles[2] = c;
  return 0;
}

int SetColorbartype(int type, const char *label) {
  update_colorbartype = 1;
  colorbartype = type;
  strcpy(colorbarname, label);
  return 0;
} // COLORBARTYPE

int SetExtremecolors(int rmin, int gmin, int bmin, int rmax, int gmax,
                     int bmax) {
  rgb_below_min[0] = CLAMP(rmin, 0, 255);
  rgb_below_min[1] = CLAMP(gmin, 0, 255);
  rgb_below_min[2] = CLAMP(bmin, 0, 255);

  rgb_above_max[0] = CLAMP(rmax, 0, 255);
  rgb_above_max[1] = CLAMP(gmax, 0, 255);
  rgb_above_max[2] = CLAMP(bmax, 0, 255);
  return 0;
} // EXTREMECOLORS

int SetFirecolor(int r, int g, int b) {
  fire_color_int255[0] = r;
  fire_color_int255[1] = g;
  fire_color_int255[2] = b;
  return 0;
} // FIRECOLOR

int SetFirecolormap(int type, int index) {
  fire_colormap_type = type;
  colorbars.fire_colorbar_index = index;
  return 0;
} // FIRECOLORMAP

int SetFiredepth(float v) {
  fire_halfdepth = v;
  return 0;
} // FIREDEPTH

// int set_gcolorbar(int ncolorbarini, ) {
//   colorbardata *cbi;
//   int r1, g1, b1;
//   int n;

//   initdefaultcolorbars();

//   ncolorbars = ndefaultcolorbars + ncolorbarini;
//   if(ncolorbarini>0)ResizeMemory((void **)&colorbarinfo,
//   ncolorbars*sizeof(colorbardata));

//   for(n = ndefaultcolorbars; n<ncolorbars; n++){
//     char *cb_buffptr;

//     cbi = colorbarinfo + n;
//     fgets(buffer, 255, stream);
//     trim_back(buffer);
//     cb_buffptr = trim_front(buffer);
//     strcpy(cbi->label, cb_buffptr);

//     fgets(buffer, 255, stream);
//     sscanf(buffer, "%i %i", &cbi->nnodes, &cbi->nodehilight);
//     if(cbi->nnodes<0)cbi->nnodes = 0;
//     if(cbi->nodehilight<0 || cbi->nodehilight >= cbi->nnodes){
//       cbi->nodehilight = 0;
//     }

//     cbi->label_ptr = cbi->label;
//     for(i = 0; i<cbi->nnodes; i++){
//       int icbar;
//       int nn;

//       fgets(buffer, 255, stream);
//       r1 = -1; g1 = -1; b1 = -1;
//       sscanf(buffer, "%i %i %i %i", &icbar, &r1, &g1, &b1);
//       cbi->index_node[i] = icbar;
//       nn = 3 * i;
//       cbi->rgb_node[nn] = r1;
//       cbi->rgb_node[nn + 1] = g1;
//       cbi->rgb_node[nn + 2] = b1;
//     }
//     remapcolorbar(cbi);
//   }
//   return 0;
// } // GCOLORBAR

int SetShowextremedata(int show_extremedata, int below, int above) {
  // int below = -1, above = -1, show_extremedata;
  if(below == -1 && above == -1) {
    if(below == -1) below = 0;
    if(below != 0) below = 1;
    if(above == -1) above = 0;
    if(above != 0) above = 1;
  }
  else {
    if(show_extremedata != 1) show_extremedata = 0;
    if(show_extremedata == 1) {
      below = 1;
      above = 1;
    }
    else {
      below = 0;
      above = 0;
    }
  }
  show_extreme_mindata = below;
  show_extreme_maxdata = above;
  return 0;
} // SHOWEXTREMEDATA

int SetSmokecolor(int r, int g, int b) {
  smoke_color_int255[0] = r;
  smoke_color_int255[1] = g;
  smoke_color_int255[2] = b;
  return 0;
} // SMOKECOLOR

int SetSmokecull(int v) {
#ifdef pp_CULL
  if(gpuactive == 1) {
    cullsmoke = v;
    if(cullsmoke != 0) cullsmoke = 1;
  }
  else {
    cullsmoke = 0;
  }
#else
  smokecullflag = v;
#endif
  return 0;
} // SMOKECULL

int SetSmokeskip(int v) {
  smoke3d_frame_inc = v+1;
  return 0;
} // SMOKESKIP

int SetSmokealbedo(float v) {
  global_scase.smoke_albedo = v;
  return 0;
} // SMOKEALBEDO

#ifdef pp_GPU
int SetSmokerthick(float v) {
  // smoke3d_rthick = v;
  // smoke3d_rthick = CLAMP(smoke3d_rthick, 1.0, 255.0);
  // smoke3d_thick = LogBase2(smoke3d_rthick);
  return 0;
} // SMOKERTHICK
#endif

// int set_smokethick(float v) {
//   smoke3d_thick = v;
//   return 0;
// } // SMOKETHICK

#ifdef pp_GPU
int SetUsegpu(int v) {
  usegpu = v;
  return 0;
}
#endif

// *** ZONE FIRE PARAMETRES ***
int SetShowhazardcolors(int v) {
  zonecolortype = v;
  return 0;
} // SHOWHAZARDCOLORS

int SetShowhzone(int v) {
  if(v) {
    visZonePlane = ZONE_ZPLANE;
  }
  else {
    visZonePlane = ZONE_HIDDEN;
  }
  return 0;
} // SHOWHZONE

int SetShowszone(int v) {
  visSZone = v;
  return 0;
} // SHOWSZONE

int SetShowvzone(int v) {
  if(v) {
    visZonePlane = ZONE_YPLANE;
  }
  else {
    visZonePlane = ZONE_HIDDEN;
  }
  return 0;
} // SHOWVZONE

int SetShowzonefire(int v) {
  viszonefire = v;
  return 0;
} // SHOWZONEFIRE

// *** TOUR INFO ***
int SetShowpathnodes(int v) {
  show_path_knots = v;
  return 0;
} // SHOWPATHNODES

int SetShowtourroute(int v) {
  edittour = v;
  return 0;
} // SHOWTOURROUTE

// TOURCOLORS
int SetTourcolorsSelectedpathline(float r, float g, float b) {
  tourcol_selectedpathline[0] = r;
  tourcol_selectedpathline[1] = g;
  tourcol_selectedpathline[2] = b;
  return 0;
}
int SetTourcolorsSelectedpathlineknots(float r, float g, float b) {
  tourcol_selectedpathlineknots[0] = r;
  tourcol_selectedpathlineknots[1] = g;
  tourcol_selectedpathlineknots[2] = b;
  return 0;
}
int SetTourcolorsSelectedknot(float r, float g, float b) {
  tourcol_selectedknot[0] = r;
  tourcol_selectedknot[1] = g;
  tourcol_selectedknot[2] = b;
  return 0;
}
int SetTourcolorsPathline(float r, float g, float b) {
  tourcol_pathline[0] = r;
  tourcol_pathline[1] = g;
  tourcol_pathline[2] = b;
  return 0;
}
int SetTourcolorsPathknots(float r, float g, float b) {
  tourcol_pathknots[0] = r;
  tourcol_pathknots[1] = g;
  tourcol_pathknots[2] = b;
  return 0;
}
int SetTourcolorsText(float r, float g, float b) {
  tourcol_text[0] = r;
  tourcol_text[1] = g;
  tourcol_text[2] = b;
  return 0;
}
int SetTourcolorsAvatar(float r, float g, float b) {
  tourcol_avatar[0] = r;
  tourcol_avatar[1] = g;
  tourcol_avatar[2] = b;
  return 0;
}

int SetViewalltours(int v) {
  viewalltours = v;
  return 0;
} // VIEWALLTOURS

int SetViewtimes(float start, float stop, int ntimes) {
  global_scase.tourcoll.tour_tstart = start;
  global_scase.tourcoll.tour_tstop = stop;
  global_scase.tourcoll.tour_ntimes = ntimes;
  return 0;
} // VIEWTIMES

int SetViewtourfrompath(int v) {
  viewtourfrompath = v;
  return 0;
} // VIEWTOURFROMPATH

int SetDevicevectordimensions(float baselength, float basediameter,
                              float headlength, float headdiameter) {
  vector_baselength = baselength;
  vector_basediameter = basediameter;
  vector_headlength = headlength;
  vector_headdiameter = headdiameter;
  return 0;
} // DEVICEVECTORDIMENSIONS

int SetDevicebounds(float min, float max) {
  device_valmin = min;
  device_valmax = max;
  return 0;
} // DEVICEBOUNDS

int SetDeviceorientation(int a, float b) {
  show_device_orientation = a;
  orientation_scale = b;
  show_device_orientation = CLAMP(show_device_orientation, 0, 1);
  orientation_scale = CLAMP(orientation_scale, 0.1, 10.0);
  return 0;
} // DEVICEORIENTATION

int SetGridparms(int vx, int vy, int vz, int px, int py, int pz) {
  visx_all = vx;
  visy_all = vy;
  visz_all = vz;

  iplotx_all = px;
  iploty_all = py;
  iplotz_all = pz;

  if(iplotx_all > nplotx_all - 1) iplotx_all = 0;
  if(iploty_all > nploty_all - 1) iploty_all = 0;
  if(iplotz_all > nplotz_all - 1) iplotz_all = 0;

  return 0;
} // GRIDPARMS

int SetGsliceparms(int vis_data, int vis_triangles, int vis_triangulation,
                   int vis_normal, float xyz[], float azelev[]) {
  vis_gslice_data = vis_data;
  show_gslice_triangles = vis_triangles;
  show_gslice_triangulation = vis_triangulation;
  show_gslice_normal = vis_normal;
  ONEORZERO(vis_gslice_data);
  ONEORZERO(show_gslice_triangles);
  ONEORZERO(show_gslice_triangulation);
  ONEORZERO(show_gslice_normal);

  gslice_xyz[0] = xyz[0];
  gslice_xyz[1] = xyz[1];
  gslice_xyz[2] = xyz[2];

  gslice_normal_azelev[0] = azelev[0];
  gslice_normal_azelev[1] = azelev[1];

  update_gslice = 1;

  return 0;
} // GSLICEPARMS

int SetLoadfilesatstartup(int v) {
  loadfiles_at_startup = v;
  return 0;
} // LOADFILESATSTARTUP
int SetMscale(float a, float b, float c) {
  mscale[0] = a;
  mscale[1] = b;
  mscale[2] = c;
  return 0;
} // MSCALE

int SetSliceauto(int n, int vals[]) {

  int n3dsmokes = 0;
  int seq_id;
  n3dsmokes = n; // TODO: is n3dsmokes the right variable.
  // TODO: this discards  the values. Verify.
  for(size_t i = 0; i < n3dsmokes; i++) {
    seq_id = vals[i];
    GetStartupSlice(seq_id);
  }
  update_load_files = 1;
  return 0;
} // SLICEAUTO

int SetMsliceauto(int n, int vals[]) {

  int n3dsmokes = 0;
  int seq_id;
  n3dsmokes = n; // TODO: is n3dsmokes the right variable
  for(size_t i = 0; i < n3dsmokes; i++) {
    seq_id = vals[i];

    if(seq_id >= 0 && seq_id < global_scase.slicecoll.nmultisliceinfo) {
      multislicedata *mslicei;

      mslicei = global_scase.slicecoll.multisliceinfo + seq_id;
      mslicei->autoload = 1;
    }
  }
  update_load_files = 1;
  return 0;
} // MSLICEAUTO

int SetCompressauto(int v) {
  compress_autoloaded = v;
  return 0;
} // COMPRESSAUTO

// int set_part5propdisp(int vals[]) {
//   char *token;

//   for(i = 0; i<npart5prop; i++){
//     partpropdata *propi;
//     int j;

//     propi = part5propinfo + i;
//     fgets(buffer, 255, stream);

//     trim_back(buffer);
//     token = strtok(buffer, " ");
//     j = 0;
//     while(token != NULL&&j<global_scase.npartclassinfo){
//       int visval;

//       sscanf(token, "%i", &visval);
//       propi->class_vis[j] = visval;
//       token = strtok(NULL, " ");
//       j++;
//     }
//   }
//   CheckMemory;
//   continue;
// } // PART5PROPDISP

// int set_part5color(int n, int vals[]) {
//   int i;
//   for(i = 0; i<npart5prop; i++){
//     partpropdata *propi;

//     propi = part5propinfo + i;
//     propi->display = 0;
//   }
//   part5colorindex = 0;
//   i = n;
//   if(i >= 0 && i<npart5prop){
//     partpropdata *propi;

//     part5colorindex = i;
//     propi = part5propinfo + i;
//     propi->display = 1;
//   }
//   continue;
//   return 0;
// } // PART5COLOR

int SetPropindex(int nvals, int *vals) {

  for(size_t i = 0; i < nvals; i++) {
    propdata *propi;
    int ind, val;
    ind = *(vals + (i * PROPINDEX_STRIDE + 0));
    val = *(vals + (i * PROPINDEX_STRIDE + 1));
    if(ind < 0 || ind > global_scase.propcoll.npropinfo - 1) return 0;
    propi = global_scase.propcoll.propinfo + ind;
    if(val < 0 || val > propi->nsmokeview_ids - 1) return 0;
    propi->smokeview_id = propi->smokeview_ids[val];
    propi->smv_object = propi->smv_objects[val];
  }
  for(size_t i = 0; i < global_scase.npartclassinfo; i++) {
    partclassdata *partclassi;

    partclassi = global_scase.partclassinfo + i;
    UpdatePartClassDepend(partclassi);
  }
  return 0;
} // PROPINDEX

int SetShooter(float xyz[], float dxyz[], float uvw[], float velmag,
               float veldir, float pointsize, int fps, int vel_type, int nparts,
               int vis, int cont_update, float duration, float v_inf) {
  shooter_xyz[0] = xyz[0];
  shooter_xyz[1] = xyz[1];
  shooter_xyz[2] = xyz[2];

  shooter_dxyz[0] = dxyz[0];
  shooter_dxyz[1] = dxyz[1];
  shooter_dxyz[2] = dxyz[2];

  shooter_uvw[0] = uvw[0];
  shooter_uvw[1] = uvw[1];
  shooter_uvw[2] = uvw[2];

  shooter_velmag = velmag;
  shooter_veldir = veldir;
  shooterpointsize = pointsize;

  shooter_fps = fps;
  shooter_vel_type = vel_type;
  shooter_nparts = nparts;
  visShooter = vis;
  shooter_cont_update = cont_update;

  shooter_duration = duration;
  shooter_v_inf = v_inf;

  return 0;
} // SHOOTER

int SetShowdevices(int ndevices_ini, const char *const *names) {
  sv_object *obj_typei;

  char tempname[255]; // temporary buffer to convert from const string

  for(size_t i = 0; i < global_scase.objectscoll.nobject_defs; i++) {
    obj_typei = global_scase.objectscoll.object_defs[i];
    obj_typei->visible = 0;
  }
  for(size_t i = 0; i < ndevices_ini; i++) {
    strncpy(tempname, names[i], 255 - 1); // use temp buffer
    obj_typei = GetSmvObject(&global_scase.objectscoll, tempname);
    // obj_typei = GetSmvObject(names[i]);
    if(obj_typei != NULL) {
      obj_typei->visible = 1;
    }
  }
  return 0;
} // SHOWDEVICES

int SetShowdevicevals(int vshowdeviceval, int vshowvdeviceval,
                      int vdevicetypes_index, int vcolordeviceval,
                      int vvectortype, int vviswindrose, int vshowdevicetype,
                      int vshowdeviceunit) {
  showdevice_val = vshowdeviceval;
  showvdevice_val = vshowvdeviceval;
  devicetypes_index = vdevicetypes_index;
  colordevice_val = vcolordeviceval;
  vectortype = vvectortype;
  viswindrose = vviswindrose;
  showdevice_type = vshowdevicetype;
  showdevice_unit = vshowdeviceunit;
  devicetypes_index = CLAMP(vdevicetypes_index, 0, ndevicetypes - 1);
  GLUIUpdateDevices();
  return 0;
} // SHOWDEVICEVALS

int SetShowmissingobjects(int v) {
  show_missing_objects = v;
  ONEORZERO(show_missing_objects);
  return 0;
} // SHOWMISSINGOBJECTS

int SetTourindex(int v) {
  selectedtour_index_ini = v;
  if(selectedtour_index_ini < 0) selectedtour_index_ini = -1;
  update_selectedtour_index = 1;
  return 0;
} // TOURINDEX

int SetUserticks(int vis, int auto_place, int sub, float origin[], float min[],
                 float max[], float step[], int show_x, int show_y,
                 int show_z) {
  visUSERticks = vis;
  auto_user_tick_placement = auto_place;
  user_tick_sub = sub;

  user_tick_origin[0] = origin[0];
  user_tick_origin[1] = origin[1];
  user_tick_origin[2] = origin[2];

  user_tick_min[0] = min[0];
  user_tick_min[1] = min[1];
  user_tick_min[2] = min[2];

  user_tick_max[0] = max[0];
  user_tick_max[1] = max[1];
  user_tick_max[2] = max[2];

  user_tick_step[0] = step[0];
  user_tick_step[1] = step[1];
  user_tick_step[2] = step[2];

  user_tick_show_x = show_x;
  user_tick_show_y = show_y;
  user_tick_show_z = show_z;

  return 0;
} // USERTICKS

int SetCParticles(int minFlag, float minValue, int maxFlag, float maxValue,
                  const char *label) {
  if(label == NULL) {
    label = "";
  }
  int l = strlen(label);
  char *label_copy = malloc(sizeof(char) * (l + 1));
  // convert to mutable string (mainly to avoid discard const warnings)
  strcpy(label_copy, label);
  if(npart5prop > 0) {
    int label_index = 0;
    if(strlen(label) > 0) label_index = GetPartPropIndexS(label_copy);
    if(label_index >= 0 && label_index < npart5prop) {
      partpropdata *propi;

      propi = part5propinfo + label_index;
      propi->setchopmin = minFlag;
      propi->setchopmax = maxFlag;
      propi->chopmin = minValue;
      propi->chopmax = maxValue;
    }
  }
  free(label_copy);
  return 0;
}

int SetCSlice(int minFlag, float minValue, int maxFlag, float maxValue,
              const char *label) {

  // if there is a label, use it
  if(strcmp(label, "") != 0) {
    for(size_t i = 0; i < nslicebounds; i++) {
      if(strcmp(slicebounds[i].shortlabel, label) != 0) continue;
      slicebounds[i].setchopmin = minFlag;
      slicebounds[i].setchopmax = maxFlag;
      slicebounds[i].chopmin = minValue;
      slicebounds[i].chopmax = maxValue;
      break;
    }
    // if there is no label apply values to all slice types
  }
  else {
    for(size_t i = 0; i < nslicebounds; i++) {
      slicebounds[i].setchopmin = minFlag;
      slicebounds[i].setchopmax = maxFlag;
      slicebounds[i].chopmin = minValue;
      slicebounds[i].chopmax = maxValue;
    }
  }
  return 0;
}

int SetCacheBoundarydata(int setting) {
  cache_boundary_data = setting;
  return 0;
} // CACHE_BOUNDARYDATA

int SetCacheQdata(int setting) {
  cache_plot3d_data = setting;
  return 0;
} // CACHE_QDATA

#ifdef pp_HIST
int SetPercentilelevel(float p_level_min, float p_level_max) {
  percentile_level_min = CLAMP(p_level_min, 0.0, 1.0);
  if(p_level_max < 0.0) p_level_max = 1.0 - percentile_level_min;
  percentile_level_max = CLAMP(p_level_max, percentile_level_min + 0.0001, 1.0);
  return 0;
} // PERCENTILELEVEL
#endif

int SetTimeoffset(int setting) {
  timeoffset = setting;
  return 0;
} // TIMEOFFSET

int SetPatchdataout(int outputFlag, float tmin, float tmax, float xmin,
                    float xmax, float ymin, float ymax, float zmin,
                    float zmax) {
  output_patchdata = outputFlag;
  patchout_tmin = tmin;
  patchout_tmax = tmax;
  patchout_xmin = xmin;
  patchout_xmax = xmax;
  patchout_ymin = ymin;
  patchout_ymax = ymax;
  patchout_zmin = zmin;
  patchout_zmax = zmax;
  ONEORZERO(output_patchdata);
  return 0;
} // PATCHDATAOUT

int SetCPlot3d(int n3d, int minFlags[], int minVals[], int maxFlags[],
               int maxVals[]) {

  if(n3d > MAXPLOT3DVARS) n3d = MAXPLOT3DVARS;
  for(size_t i = 0; i < n3d; i++) {
    setp3chopmin[i] = minFlags[i];
    setp3chopmax[i] = maxFlags[i];
    p3chopmin[i] = minVals[i];
    p3chopmax[i] = maxVals[i];
  }
  return 0;
} // C_PLOT3D

int SetVPlot3d(int n3d, int minFlags[], int minVals[], int maxFlags[],
               int maxVals[]) {

  if(n3d > MAXPLOT3DVARS) n3d = MAXPLOT3DVARS;
  for(size_t i = 0; i < n3d; i++) {
    setp3min_all[i] = minFlags[i];
    setp3max_all[i] = maxFlags[i];
    p3min_global[i] = minVals[i];
    p3max_global[i] = maxVals[i];
  }
  return 0;
} // V_PLOT3D

int SetPl3dBoundMin(int pl3dValueIndex, int set, float value) {
  setp3min_all[pl3dValueIndex] = set;
  p3min_all[pl3dValueIndex] = value;
  // TODO: remove this reload and hardcoded value
  GLUIPlot3DBoundCB(7);
  return 0;
}

int SetPl3dBoundMax(int pl3dValueIndex, int set, float value) {
  setp3max_all[pl3dValueIndex] = set;
  p3max_all[pl3dValueIndex] = value;
  // TODO: remove this reload and hardcoded value
  GLUIPlot3DBoundCB(7);
  return 0;
}

int SetTload(int beginFlag, float beginVal, int endFlag, int endVal,
             int skipFlag, int skipVal) {
  use_tload_begin = beginFlag;
  global_scase.tload_begin = beginVal;
  use_tload_end = endFlag;
  global_scase.tload_end = endVal;
  use_tload_skip = skipFlag;
  tload_skip = skipVal;
  return 0;
} // TLOAD

int SetV5Particles(int minFlag, float minValue, int maxFlag, float maxValue,
                   const char *label) {
  if(label == NULL) {
    label = "";
  }
  int l = strlen(label);
  char *label_copy = malloc(sizeof(char) * (l + 1));
  // convert to mutable string (mainly to avoid discard const warnings)
  strcpy(label_copy, label);
  if(npart5prop > 0) {
    int label_index = 0;

    if(strlen(label) > 0) label_index = GetPartPropIndexS(label_copy);
    if(label_index >= 0 && label_index < npart5prop) {
      partpropdata *propi;

      propi = part5propinfo + label_index;
      propi->setvalmin = minFlag;
      propi->setvalmax = maxFlag;
      propi->valmin = minValue;
      propi->valmax = maxValue;
      switch(minFlag) {
      case PERCENTILE_MIN:
#ifdef pp_HIST
        propi->percentile_min = minValue;
#endif
        break;
      case GLOBAL_MIN:
        propi->dlg_global_valmin = minValue;
        break;
      case SET_MIN:
        propi->user_min = minValue;
        break;
      default:
        assert(FFALSE);
        break;
      }
      switch(maxFlag) {
      case PERCENTILE_MAX:
#ifdef pp_HIST
        propi->percentile_max = maxValue;
#endif
        break;
      case GLOBAL_MAX:
        propi->dlg_global_valmax = maxValue;
        break;
      case SET_MAX:
        propi->user_max = maxValue;
        break;
      default:
        assert(FFALSE);
        break;
      }
    }
  }
  free(label_copy);
  return 0;
}

int SetVParticles(int minFlag, float minValue, int maxFlag, float maxValue) {
  setpartmin = minFlag;
  glui_partmin = minValue;
  setpartmax = maxFlag;
  glui_partmax = maxValue;
  return 0;
} // V_PARTICLES

int SetVTarget(int minFlag, float minValue, int maxFlag, float maxValue) {
  settargetmin = minFlag;
  targetmin = minValue;
  settargetmax = maxFlag;
  targetmax = maxValue;
  return 0;
} // V_TARGET

int SetVSlice(int minFlag, float minValue, int maxFlag, float maxValue,
              const char *label, float lineMin, float lineMax, int lineNum) {

  // if there is a label to apply, use it
  if(strcmp(label, "") != 0) {
    for(size_t i = 0; i < nslicebounds; i++) {
      if(strcmp(slicebounds[i].shortlabel, label) != 0) continue;
      slicebounds[i].dlg_setvalmin = minFlag;
      slicebounds[i].dlg_setvalmax = maxFlag;
      slicebounds[i].dlg_valmin = minValue;
      slicebounds[i].dlg_valmax = maxValue;

      slicebounds[i].line_contour_min = lineMin;
      slicebounds[i].line_contour_max = lineMax;
      slicebounds[i].line_contour_num = lineNum;
      break;
    }
    // if there is no label apply values to all slice types
  }
  else {
    for(size_t i = 0; i < nslicebounds; i++) {
      slicebounds[i].dlg_setvalmin = minFlag;
      slicebounds[i].dlg_setvalmax = maxFlag;
      slicebounds[i].dlg_valmin = minValue;
      slicebounds[i].dlg_valmax = maxValue;

      slicebounds[i].line_contour_min = lineMin;
      slicebounds[i].line_contour_max = lineMax;
      slicebounds[i].line_contour_num = lineNum;
    }
  }
  return 0;
} // V_SLICE

int ShowSmoke3dShowall() {
  smoke3ddata *smoke3di;

  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  plotstate = DYNAMIC_PLOTS;
  for(size_t i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++) {
    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->loaded == 1) smoke3di->display = 1;
  }
  GLUTPOSTREDISPLAY;
  UpdateShow();
  return 0;
}

int ShowSmoke3dHideall() {
  smoke3ddata *smoke3di;

  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  for(size_t i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++) {
    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->loaded == 1) smoke3di->display = 0;
  }
  UpdateShow();
  return 0;
}

int ShowSlicesShowall() {

  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  for(size_t i = 0; i < global_scase.slicecoll.nsliceinfo; i++) {
    global_scase.slicecoll.sliceinfo[i].display = 1;
  }
  showall_slices = 1;
  UpdateSliceFilenum();
  plotstate = GetPlotState(DYNAMIC_PLOTS);

  UpdateShow();
  GLUTPOSTREDISPLAY;
  return 0;
}

int ShowSlicesHideall() {

  updatemenu = 1;
  GLUTPOSTREDISPLAY;
  for(size_t i = 0; i < global_scase.slicecoll.nsliceinfo; i++) {
    global_scase.slicecoll.sliceinfo[i].display = 0;
  }
  showall_slices = 0;
  UpdateSliceFilenum();
  plotstate = GetPlotState(DYNAMIC_PLOTS);

  UpdateShow();
  return 0;
}
#endif
