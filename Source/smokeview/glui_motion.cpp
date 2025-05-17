#define CPP
#include "options.h"

#include <assert.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "glui_motion.h"
#include "readgeom.h"
#include "paths.h"

#define ROTATE_TRANSLATE
#ifdef pp_OSX_HIGHRES
#undef ROTATE_TRANSLATE
#endif

unsigned char deg360[] = { '3','6','0',0 };
unsigned char deg90[] = {'9', '0', 0};

GLUI *glui_motion=NULL;

GLUI_EditText *EDITTEXT_movie_email    = NULL;
GLUI_EditText *EDITTEXT_movie_htmldir  = NULL;
GLUI_EditText *EDITTEXT_movie_url      = NULL;

GLUI_Panel *PANEL_select = NULL;
GLUI_Panel *PANEL_360 = NULL;
GLUI_Panel *PANEL_360_debug = NULL;
GLUI_Panel *PANEL_custom_view=NULL;
GLUI_Panel *PANEL_render_file = NULL;
GLUI_Panel *PANEL_render_format = NULL;
GLUI_Panel *PANEL_movie_type = NULL;
GLUI_Panel *PANEL_user_center = NULL;
#ifdef ROTATE_TRANSLATE
GLUI_Panel *PANEL_rotate = NULL;
#endif
GLUI_Panel *PANEL_close = NULL;
GLUI_Panel *PANEL_file_suffix=NULL, *PANEL_file_type=NULL;
GLUI_Panel *PANEL_radiorotate=NULL;
GLUI_Panel *PANEL_gslice_center=NULL;
GLUI_Panel *PANEL_gslice_normal=NULL;
GLUI_Panel *PANEL_gslice_show=NULL;
GLUI_Panel *PANEL_speed=NULL;
GLUI_Panel *PANEL_height=NULL;
#ifdef ROTATE_TRANSLATE
GLUI_Panel *PANEL_translate2 = NULL;
#endif
GLUI_Panel *PANEL_translate3 = NULL;
GLUI_Panel *PANEL_anglebuttons=NULL;
GLUI_Panel *PANEL_reset1=NULL;
GLUI_Panel *PANEL_reset2=NULL;
GLUI_Panel *PANEL_reset=NULL;
GLUI_Panel *PANEL_specify=NULL;
GLUI_Panel *PANEL_change_zaxis=NULL;
GLUI_Panel *PANEL_colors=NULL;
GLUI_Panel *PANEL_background = NULL;
GLUI_Panel *PANEL_foreground = NULL;

GLUI_Rollout *ROLLOUT_scale = NULL;
GLUI_Rollout *ROLLOUT_viewA = NULL;
GLUI_Rollout *ROLLOUT_motion = NULL;
GLUI_Rollout *ROLLOUT_view = NULL;
GLUI_Rollout *ROLLOUT_image_size = NULL;
GLUI_Rollout *ROLLOUT_name = NULL;
GLUI_Rollout *ROLLOUT_rotation_type = NULL;
GLUI_Rollout *ROLLOUT_orientation=NULL;
GLUI_Rollout *ROLLOUT_scene_clip=NULL;
GLUI_Rollout *ROLLOUT_projection=NULL;
GLUI_Rollout *ROLLOUT_render=NULL;
GLUI_Rollout *ROLLOUT_viewpoints=NULL;
GLUI_Rollout *ROLLOUT_make_movie = NULL;
GLUI_Rollout *ROLLOUT_make_movie_batch = NULL;
GLUI_Rollout *ROLLOUT_gslice = NULL;
#ifdef ROTATE_TRANSLATE
GLUI_Rollout *ROLLOUT_translaterotate=NULL;
#endif
#ifdef pp_RENDER360_DEBUG
GLUI_Rollout *ROLLOUT_screenvis = NULL;
GLUI_Rollout *ROLLOUT_lower = NULL;
GLUI_Rollout *ROLLOUT_middle = NULL;
GLUI_Rollout *ROLLOUT_upper = NULL;
#endif

GLUI_Spinner *SPINNER_movie_nprocs=NULL;
GLUI_Spinner *SPINNER_360_skip_x=NULL;
GLUI_Spinner *SPINNER_360_skip_y=NULL;
GLUI_Spinner *SPINNER_movie_crf = NULL;
GLUI_Spinner *SPINNER_bitrate = NULL;
GLUI_Spinner *SPINNER_customview_elevation=NULL;
GLUI_Spinner *SPINNER_customview_azimuth=NULL;
GLUI_Spinner *SPINNER_resolution_multiplier=NULL;
GLUI_Spinner *SPINNER_clip_left=NULL;
GLUI_Spinner *SPINNER_clip_right=NULL;
GLUI_Spinner *SPINNER_clip_bottom=NULL;
GLUI_Spinner *SPINNER_clip_top=NULL;
GLUI_Spinner *SPINNER_gslice_center_x=NULL;
GLUI_Spinner *SPINNER_gslice_center_y=NULL;
GLUI_Spinner *SPINNER_gslice_center_z=NULL;
GLUI_Spinner *SPINNER_gslice_normal_az=NULL;
GLUI_Spinner *SPINNER_gslice_normal_elev=NULL;
GLUI_Spinner *SPINNER_glui_xyz_fds[3];
GLUI_Spinner **SPINNER_glui_xyz_fds_check=NULL;
GLUI_Spinner *SPINNER_zaxis_angles[3];
GLUI_Spinner *SPINNER_zoom=NULL,*SPINNER_aperture=NULL;
GLUI_Spinner *SPINNER_window_width=NULL, *SPINNER_window_height=NULL;
GLUI_Spinner *SPINNER_scalex=NULL;
GLUI_Spinner *SPINNER_scaley=NULL;
GLUI_Spinner *SPINNER_scalez=NULL;
GLUI_Spinner *SPINNER_nearclip=NULL;
GLUI_Spinner *SPINNER_farclip=NULL;
GLUI_Spinner *SPINNER_xcenCUSTOM=NULL;
GLUI_Spinner *SPINNER_ycenCUSTOM=NULL;
GLUI_Spinner *SPINNER_zcenCUSTOM=NULL;
GLUI_Spinner *SPINNER_framerate = NULL;
GLUI_Spinner *SPINNER_window_height360=NULL;
GLUI_Spinner *SPINNER_foreground_red=NULL;
GLUI_Spinner *SPINNER_foreground_green=NULL;
GLUI_Spinner *SPINNER_foreground_blue=NULL;
GLUI_Spinner *SPINNER_background_red=NULL;
GLUI_Spinner *SPINNER_background_green=NULL;
GLUI_Spinner *SPINNER_background_blue=NULL;

GLUI_Checkbox *CHECKBOX_fix_window_aspect = NULL;
GLUI_Checkbox *CHECKBOX_use_geom_factors = NULL;
GLUI_Checkbox *CHECKBOX_use_customview=NULL;
GLUI_Checkbox *CHECKBOX_custom_view = NULL;
GLUI_Checkbox *CHECKBOX_clip_show_rotation_center = NULL;
GLUI_Checkbox *CHECKBOX_render360 = NULL;
#ifdef pp_RENDER360_DEBUG
GLUI_Checkbox *CHECKBOX_screenview = NULL;
#endif
GLUI_Checkbox *CHECKBOX_show_rotation_center=NULL;
GLUI_Checkbox *CHECKBOX_clip_rendered_scene=NULL;
GLUI_Checkbox *CHECKBOX_general_rotation=NULL;
GLUI_Checkbox *CHECKBOX_blockpath=NULL;
GLUI_Checkbox *CHECKBOX_gslice_data=NULL;
GLUI_Checkbox *CHECKBOX_showgravity_vector=NULL;
GLUI_Checkbox *CHECKBOX_overwrite_movie = NULL;
#ifdef pp_RENDER360_DEBUG
GLUI_Checkbox **CHECKBOX_screenvis = NULL;
#endif

#ifdef ROTATE_TRANSLATE
GLUI_Translation *ROTATE_2axis = NULL;
GLUI_Translation *TRANSLATE_z=NULL,*TRANSLATE_xy=NULL, *ROTATE_eye_z = NULL;
GLUI_Translation *TRANSLATE_x = NULL, *TRANSLATE_y = NULL;
#endif

GLUI_RadioGroup *RADIO_render_resolution = NULL;
GLUI_RadioGroup *RADIO_projection=NULL,*RADIO_rotation_type=NULL;
GLUI_RadioGroup *RADIO_render_type=NULL;
GLUI_RadioGroup *RADIO_render_label=NULL;
GLUI_RadioGroup *RADIO_movie_type = NULL;

GLUI_RadioButton *RADIOBUTTON_render_current = NULL;
GLUI_RadioButton *RADIOBUTTON_render_high = NULL;
GLUI_RadioButton *RADIOBUTTON_render_360=NULL;

GLUI_RadioButton *RADIOBUTTON_movie_type[4];
GLUI_RadioButton *RADIOBUTTON_1a=NULL;
GLUI_RadioButton *RADIOBUTTON_1b=NULL;
GLUI_RadioButton *RADIOBUTTON_1c=NULL;
GLUI_RadioButton *RADIOBUTTON_1d=NULL;
GLUI_RadioButton *RADIOBUTTON_1e=NULL;
GLUI_RadioButton *RADIOBUTTON_1f=NULL;
GLUI_RadioButton *RADIOBUTTON_1g=NULL;

GLUI_Button *BUTTON_make_movie_batch=NULL;
GLUI_Button *BUTTON_rotate90=NULL;
GLUI_Button *BUTTON_90_z=NULL,*BUTTON_eyelevel=NULL, *BUTTON_floorlevel=NULL, *BUTTON_reset_saved_view=NULL;
GLUI_Button *BUTTON_replace_view=NULL,*BUTTON_add_view=NULL,*BUTTON_delete_view=NULL;
GLUI_Button *BUTTON_startup = NULL;
GLUI_Button *BUTTON_cycle_views_user    = NULL;
GLUI_Button *BUTTON_cycle_views_default = NULL;
GLUI_Button *BUTTON_cycle_views_all     = NULL;
GLUI_Button *BUTTON_snap=NULL;
GLUI_Button *BUTTON_render_start=NULL;
GLUI_Button *BUTTON_motion_1=NULL;
GLUI_Button *BUTTON_motion_2=NULL;
GLUI_Button *BUTTON_make_movie = NULL;
GLUI_Button *BUTTON_play_movie = NULL;
#ifdef pp_RENDER360_DEBUG
GLUI_Button *BUTTON_screen_hideall = NULL;
GLUI_Button *BUTTON_screen_showall = NULL;
#endif
GLUI_Button *BUTTON_flip = NULL;

GLUI_EditText *EDIT_view_label=NULL;
GLUI_EditText *EDIT_movie_name = NULL;
GLUI_EditText *EDIT_render_file_base = NULL;

GLUI_Listbox *LIST_movie_slice_index=NULL;
GLUI_Listbox *LIST_movie_queue_index=NULL;
GLUI_Listbox *LIST_viewpoints=NULL;
GLUI_Listbox *LIST_windowsize=NULL;
GLUI_Listbox *LIST_rotate_about=NULL;
GLUI_Listbox *LIST_render_skip=NULL;

rolloutlistdata first_rollout, last_rollout;

procdata motionprocinfo[9], mvrprocinfo[5], subrenderprocinfo[4], screenprocinfo[3];
int nmotionprocinfo = 0, nmvrprocinfo=0, nsubrenderprocinfo=0, nscreenprocinfo=0;

/* ------------------ MakeMovieBashScript ------------------------ */

void MakeMovieBashScript(void){
  char *firemodels=NULL;
  char *email_ptr=NULL;

  FILE *stream=NULL;
  char command_line[1000];

  firemodels = getenv("FIREMODELS");
  if(firemodels==NULL){
    printf("***error: The environment variable FIREMODELS is not defined\n");
    printf("          Movie making script aborted\n");
    return;
  }

  stream = fopen(movie_bash_script, "w");
  if(stream==NULL)return;

  fprintf(stream, "#/bin/bash\n");
  fprintf(stream, "NPROCS=%i\n", movie_nprocs);
  fprintf(stream, "QUEUE=%s\n", movie_queues[movie_queue_index]);

  fprintf(stream, "FIREMODELS=%s\n", firemodels);
  fprintf(stream, "MAKEMOVIE=$FIREMODELS/smv/Utilities/Scripts/make_movie.sh\n");
  fprintf(stream, "QSMV=$FIREMODELS/smv/Utilities/Scripts/qsmv.sh\n");
  fprintf(stream, "SMOKEVIEW=$FIREMODELS/smv/Build/smokeview/intel_linux_64/smokeview_linux_64\n");


  fprintf(stream, "$QSMV -j SV_ -P $NPROCS -q $QUEUE -e $SMOKEVIEW -c %s %s\n", movie_ssf_script, global_scase.fdsprefix);
  fprintf(stream, "$MAKEMOVIE -i . -j SV_ -o %s %s %s\n", movie_htmldir, movie_basename, movie_basename);

  email_ptr = TrimFrontBack(movie_email);
  if(email_ptr!=NULL&&strlen(email_ptr)>0){
    char full_animation_file[256];
    slicedata *slicei;
    slicemenudata *slicemi;
    char *slicelabel, label[256];

    slicemi = slicemenu_sorted[movie_slice_index];
    slicei = slicemi->sliceinfo;
    slicelabel = slicei->label.longlabel;
    if(slicelabel!=NULL && strlen(slicelabel)>0){
      strcpy(label, "animation: ");
      strcat(label, slicelabel);
    }
    else{
      strcpy(label, "animation results");
    }

    strcpy(full_animation_file, movie_htmldir);
    strcat(full_animation_file, "/");
    strcat(full_animation_file, movie_basename);
    strcat(full_animation_file, ".mp4");
    fprintf(stream, "if [ -e %s ]; then\n", full_animation_file);
    fprintf(stream, "  echo \"emailing results to %s\"\n", email_ptr);
    fprintf(stream, "  echo \"%s/%s.mp4\" | mail -s \"%s\" %s\n", movie_url, movie_basename, label, email_ptr);
    fprintf(stream, "else\n");
    fprintf(stream, "  echo \"Animation file, %s, failed to build\"\n", full_animation_file);
    fprintf(stream, "fi\n");
  }

  fclose(stream);

  //sprintf(command_line, "bash %s", movie_bash_script);
  snprintf(command_line, sizeof(command_line), "bash %s", movie_bash_script);
  system(command_line);
}

/* ------------------ MakeMovieSMVScript ------------------------ */

void MakeMovieSMVScript(void){
  FILE *stream=NULL;
  slicedata *slicei;
  slicemenudata *slicemi;

  stream = fopen(movie_ssf_script, "w");
  if(stream==NULL)return;
  slicemi = slicemenu_sorted[movie_slice_index];
  slicei = slicemi->sliceinfo;

  fprintf(stream, "RENDERDIR\n");
  fprintf(stream, " .\n");
  fprintf(stream, "UNLOADALL\n");
  fprintf(stream, "LOADINIFILE\n");
  fprintf(stream, " %s\n", movie_ini_filename);
  fprintf(stream, "SETVIEWPOINT\n");
  fprintf(stream, " current\n");
  fprintf(stream, "LOADSLICERENDER\n");
  fprintf(stream, " %s\n", slicei->label.longlabel);
  fprintf(stream, " %i %f\n", slicei->idir, slicei->position_orig);
  fprintf(stream, " %s\n", movie_basename);
  fprintf(stream, " 0 1\n");
  fclose(stream);
}

/* ------------------ GLUICloseRollouts ------------------------ */

extern "C" void GLUICloseRollouts(GLUI *dialog){
#ifndef pp_CLOSEOFF
  rolloutlistdata *this_rollout;

  for(this_rollout = first_rollout.next; this_rollout->next!=NULL; this_rollout = this_rollout->next){
    if(dialog==NULL||this_rollout->dialog==dialog)this_rollout->rollout->close();
  }
  if(dialog!=NULL){
    dialog->hide();
    updatemenu = 1;
  }
#endif
}

/* ------------------ GLUIUpdateFarclip ------------------------ */

extern "C" void GLUIUpdateFarclip(void){
  if(SPINNER_farclip != NULL)SPINNER_farclip->set_float_val(farclip);
}

/* ------------------ GLUIUpdateMovieParms ------------------------ */

extern "C" void GLUIUpdateMovieParms(void){
  if(LIST_movie_slice_index!=NULL)LIST_movie_slice_index->set_int_val(movie_slice_index);
  if(LIST_movie_queue_index!=NULL)LIST_movie_queue_index->set_int_val(movie_queue_index);
  if(SPINNER_movie_nprocs!=NULL)SPINNER_movie_nprocs->set_int_val(movie_nprocs);
}

/* ------------------ GLUIShrinkDialogs ------------------------ */

extern "C" void GLUIShrinkDialogs(void){
  GLUICloseRollouts(NULL);
}

/* ------------------ InitRolloutList ------------------------ */

void InsertRollout(GLUI_Rollout *rollout, GLUI *dialog){
  rolloutlistdata *thisrollout, *prev, *next;

  NewMemory((void **)&thisrollout,sizeof(rolloutlistdata));

  prev = &first_rollout;
  next = prev->next;

  prev->next = thisrollout;
  next->prev = thisrollout;

  thisrollout->prev = prev;
  thisrollout->next = next;
  thisrollout->rollout = rollout;
  thisrollout->dialog  = dialog;
  rollout_count++;
}

/* ------------------ InitRolloutList ------------------------ */

void InitRolloutList(void){
  first_rollout.prev = NULL;
  first_rollout.next = &last_rollout;
  first_rollout.rollout = NULL;
  first_rollout.dialog = NULL;

  last_rollout.prev = &first_rollout;
  last_rollout.next = NULL;
  last_rollout.rollout = NULL;
  last_rollout.dialog = NULL;
}

/* ------------------ GLUIUpdateRenderRadioButtons ------------------------ */

extern "C" void GLUIUpdateRenderRadioButtons(int width_low, int height_low, int width_high, int height_high){
  char label[1024];

  if(width_low > 2){
    //sprintf(label, "%ix%i (current)", width_low, height_low);
    snprintf(label, sizeof(label), "%ix%i (current)", width_low, height_low);
    if(RADIOBUTTON_render_current != NULL)RADIOBUTTON_render_current->set_name(label);

    //sprintf(label, "%ix%i ( %i x current)", width_high, height_high, glui_resolution_multiplier);
    snprintf(label, sizeof(label), "%ix%i ( %i x current)", width_high, height_high, glui_resolution_multiplier);
    if(RADIOBUTTON_render_high != NULL)RADIOBUTTON_render_high->set_name(label);
  }

  if(nwidth360 > 2){
    //sprintf(label, "%s %ix%i", deg360, nwidth360, nheight360);
    snprintf(label, sizeof(label), "%s %ix%i", deg360, nwidth360, nheight360);
    if(RADIOBUTTON_render_360 != NULL)RADIOBUTTON_render_360->set_name(label);
  }
}

/* ------------------ UpdateGeomFactor ------------------------ */

extern "C" void GLUIUpdateUseGeomFactors(void){
  if(CHECKBOX_use_geom_factors!=NULL)CHECKBOX_use_geom_factors->set_int_val(use_geom_factors);
}

/* ------------------ GLUIUpdatePosView ------------------------ */

extern "C" void GLUIUpdatePosView(void){
  SPINNER_glui_xyz_fds[0]->set_float_val(glui_xyz_fds[0]);
  SPINNER_glui_xyz_fds[1]->set_float_val(glui_xyz_fds[1]);
  SPINNER_glui_xyz_fds[2]->set_float_val(glui_xyz_fds[2]);
  SPINNER_customview_azimuth->set_float_val(customview_azimuth);
  SPINNER_customview_elevation->set_float_val(customview_elevation);
  CHECKBOX_use_customview->set_int_val(use_customview);
}

/* ------------------ GLUIUpdateZAxisCustom ------------------------ */

extern "C" void GLUIUpdateZAxisCustom(void){
  update_zaxis_custom = 0;
  GLUISceneMotionCB(ZAXIS_CUSTOM);
}

/* ------------------ GLUIUpdateShowRotationCenter ------------------------ */

extern "C" void GLUIUpdateShowRotationCenter(void){
  if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->set_int_val(show_rotation_center);
}

/* ------------------ UpdateGluiRotateAbout ------------------------ */

void UpdateGluiRotateAbout(int val){
  if(LIST_rotate_about != NULL)LIST_rotate_about->set_int_val(val);
  GLUISceneMotionCB(ROTATE_ABOUT);
}

/* ------------------ GLUIUpdateShowGravityVector ------------------------ */

extern "C" void GLUIUpdateShowGravityVector(void){
  if(CHECKBOX_showgravity_vector!=NULL)CHECKBOX_showgravity_vector->set_int_val(showgravity_vector);
}

/* ------------------ GLUISetColorControls ------------------------ */

extern "C" void GLUISetColorControls(void){
  if(background_flip == 1){
    glui_foregroundbasecolor[0] = 255.0*foregroundbasecolor[0];
    glui_foregroundbasecolor[1] = 255.0*foregroundbasecolor[1];
    glui_foregroundbasecolor[2] = 255.0*foregroundbasecolor[2];
    glui_backgroundbasecolor[0] = 255.0*backgroundbasecolor[0];
    glui_backgroundbasecolor[1] = 255.0*backgroundbasecolor[1];
    glui_backgroundbasecolor[2] = 255.0*backgroundbasecolor[2];
  }
  else{
    glui_foregroundbasecolor[0] = 255.0*backgroundbasecolor[0];
    glui_foregroundbasecolor[1] = 255.0*backgroundbasecolor[1];
    glui_foregroundbasecolor[2] = 255.0*backgroundbasecolor[2];
    glui_backgroundbasecolor[0] = 255.0*foregroundbasecolor[0];
    glui_backgroundbasecolor[1] = 255.0*foregroundbasecolor[1];
    glui_backgroundbasecolor[2] = 255.0*foregroundbasecolor[2];
  }
  if(SPINNER_foreground_red  !=NULL)  SPINNER_foreground_red->set_int_val(glui_foregroundbasecolor[0]);
  if(SPINNER_foreground_green!=NULL)SPINNER_foreground_green->set_int_val(glui_foregroundbasecolor[1]);
  if(SPINNER_foreground_blue !=NULL) SPINNER_foreground_blue->set_int_val(glui_foregroundbasecolor[2]);
  if(SPINNER_background_red  !=NULL)  SPINNER_background_red->set_int_val(glui_backgroundbasecolor[0]);
  if(SPINNER_background_green!=NULL)SPINNER_background_green->set_int_val(glui_backgroundbasecolor[1]);
  if(SPINNER_background_blue !=NULL) SPINNER_background_blue->set_int_val(glui_backgroundbasecolor[2]);
}

/* ------------------ MovieCB ------------------------ */

void MovieCB(int val){
  switch(val){
    case MOVIE_SLICE_INDEX:
      //sprintf(movie_basename, "%s_slice_%i", global_scase.fdsprefix, movie_slice_index+1);
      snprintf(movie_basename, sizeof(movie_basename), "%s_slice_%i", global_scase.fdsprefix, movie_slice_index+1);

      strcpy(movie_ssf_script, movie_basename);
      strcat(movie_ssf_script, ".ssf");

      strcpy(movie_bash_script, movie_basename);
      strcat(movie_bash_script, ".sh");

      strcpy(movie_ini_filename, movie_basename);
      strcat(movie_ini_filename, ".ini");
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ ScreenRolloutCB ------------------------ */

void ScreenRolloutCB(int var){
  GLUIToggleRollout(screenprocinfo, nscreenprocinfo, var);
}

/* ------------------ SubRenderRolloutCB ------------------------ */

void SubRenderRolloutCB(int var){
  GLUIToggleRollout(subrenderprocinfo, nsubrenderprocinfo, var);
}

/* ------------------ MVRRolloutCB ------------------------ */

void MVRRolloutCB(int var){
  GLUIToggleRollout(mvrprocinfo, nmvrprocinfo, var);
}

/* ------------------ MotionRolloutCB ------------------------ */

void MotionRolloutCB(int var){
  GLUIToggleRollout(motionprocinfo, nmotionprocinfo, var);
}

/* ------------------ UpdateRenderStartButton ------------------------ */

void UpdateRenderStartButton(void){
  int is_enabled;

  is_enabled = BUTTON_render_start->enabled;
  if(render_status == RENDER_ON&&is_enabled == 1){
    BUTTON_render_start->disable();
  }
  else if(render_status == RENDER_OFF&&is_enabled == 0&&update_makemovie==0){
    BUTTON_render_start->enable();
  }
}

/* ------------------ GLUIEnableDisablePlayMovieCPP ------------------------ */

extern "C" void GLUIEnableDisablePlayMovieCPP(void){
  char moviefile_path[1024];

  if(BUTTON_play_movie != NULL){
    if(FILE_EXISTS(GetMovieFilePath(moviefile_path)) == YES&&play_movie_now == 1){
      BUTTON_play_movie->enable();
    }
    else{
      BUTTON_play_movie->disable();
    }
  }
}

/* ------------------ EnableDisableMakeMovie ------------------------ */

extern "C" void GLUIEnableDisableMakeMovieCPP(int onoff){
  if(BUTTON_make_movie!=NULL){
    if(onoff == ON){
      BUTTON_make_movie->enable();
    }
    else{
      BUTTON_make_movie->disable();
    }
  }
}

/* ------------------ UpdateMovieType ------------------------ */

void UpdateMovieType(int type){
  movie_filetype = type;
  if(RADIO_movie_type!=NULL)RADIO_movie_type->set_int_val(movie_filetype);
}

/* ------------------ UpdateRenderType ------------------------ */

void UpdateRenderType(int type){
  render_filetype = type;
  if(RADIO_render_type!=NULL)RADIO_render_type->set_int_val(render_filetype);
}

/* ------------------ UpdateZaxisAngles ------------------------ */

void UpdateZaxisAngles(void){
  SPINNER_zaxis_angles[0]->set_float_val(zaxis_angles[0]);
  SPINNER_zaxis_angles[1]->set_float_val(zaxis_angles[1]);
}

/* ------------------ GLUIUpdateResolutionMultiplier ------------------------ */

extern "C" void GLUIUpdateResolutionMultiplier(void){
  if(SPINNER_resolution_multiplier!=NULL&&resolution_multiplier!=SPINNER_resolution_multiplier->get_int_val()){
    SPINNER_resolution_multiplier->set_int_val(resolution_multiplier);
  }
}

/* ------------------ GSliceCB ------------------------ */

void GSliceCB(int var){
  float az, elev;

  switch(var){
  case GSLICE_NORMAL:
    az = gslice_normal_azelev[0];
    if(az<-180.0 || az>180.0){
      az += 180.0;
      az = fmod((double)az, 360.0);
      if(az < 0.0)az += 360.0;
      az -= 180.0;
      SPINNER_gslice_normal_az->set_float_val(az);
    }
    elev = gslice_normal_azelev[1];
    if(elev<-180.0 || elev>180.0){
      elev += 180.0;
      elev = fmod((double)elev, 360.0);
      if(elev < 0)elev += 360.0;
      elev -= 180.0;
      SPINNER_gslice_normal_elev->set_float_val(elev);
    }
    az *= DEG2RAD;
    elev *= DEG2RAD;
    gslice_norm[0] = cos(az)*cos(elev);
    gslice_norm[1] = sin(az)*cos(elev);
    gslice_norm[2] = sin(elev);
    break;
  case GSLICE_TRANSLATE:
    gslice_xyz[0] = CLAMP(gslice_xyz[0], global_scase.xbar0, SMV2FDS_X(global_scase.xbar));
    gslice_xyz[1] = CLAMP(gslice_xyz[1], global_scase.ybar0, SMV2FDS_Y(global_scase.ybar));
    gslice_xyz[2] = CLAMP(gslice_xyz[2], global_scase.zbar0, SMV2FDS_Z(global_scase.zbar));
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIUpdateGsliceParms ------------------------ */

extern "C" void GLUIUpdateGsliceParms(void){
  update_gslice=0;
  GSliceCB(GSLICE_NORMAL);
  GSliceCB(GSLICE_TRANSLATE);
  SPINNER_gslice_center_x->set_float_val(gslice_xyz[0]);
  SPINNER_gslice_center_y->set_float_val(gslice_xyz[1]);
  SPINNER_gslice_center_z->set_float_val(gslice_xyz[2]);
  SPINNER_gslice_normal_az->set_float_val(gslice_normal_azelev[0]);
  SPINNER_gslice_normal_elev->set_float_val(gslice_normal_azelev[1]);
  CHECKBOX_gslice_data->set_int_val(vis_gslice_data);
}


/* ------------------ GLUIUpdateRotationType ------------------------ */

extern "C" void GLUIUpdateRotationType(int val){
  if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
}

/* ------------------ GLUISetPosXYZSMV ------------------------ */

extern "C" void GLUISetPosXYZSMV(float *xyz){
  if(xyz==NULL)return;
  if(SPINNER_glui_xyz_fds_check == NULL)return;

  SMV2FDS_XYZ(glui_xyz_fds,xyz);

  SPINNER_glui_xyz_fds[0]->set_float_val(glui_xyz_fds[0]);
  SPINNER_glui_xyz_fds[1]->set_float_val(glui_xyz_fds[1]);
  SPINNER_glui_xyz_fds[2]->set_float_val(glui_xyz_fds[2]);
}

/* ------------------ GLUISetPosXYZFDS ------------------------ */

extern "C" void GLUISetPosXYZFDS(float *xyz){
  if(SPINNER_glui_xyz_fds_check == NULL)return;
  SPINNER_glui_xyz_fds[0]->set_float_val(xyz[0]);
  SPINNER_glui_xyz_fds[1]->set_float_val(xyz[1]);
  SPINNER_glui_xyz_fds[2]->set_float_val(xyz[2]);
  memcpy(glui_xyz_fds, xyz, 3*sizeof(float));
  GLUISceneMotionCB(SET_VIEW_XYZ);
}


/* ------------------ ViewExist ------------------------ */

int ViewExist(char *view){
  cameradata *ca;

  if(view == NULL)return 0;
  for(ca = camera_list_first.next;ca->next != NULL;ca = ca->next){
    if(strcmp(view, ca->name) == 0)return 1;
  }
  return 0;
}

/* ------------------ GetUniqueViewName ------------------------ */

void GetUniqueViewName(void){
  char *label, viewlabel[300];

  label = EDIT_view_label->get_text();
  if(ViewExist(label) == 1){
    int i;

    for(i = 1;;i++){
      //sprintf(viewlabel, "view %i", i);
      snprintf(viewlabel, sizeof(viewlabel),"view %i", i);
      if(ViewExist(viewlabel) == 0)break;
    }
    EDIT_view_label->set_text(viewlabel);
  }
}

/* ------------------ Camera2Quat ------------------------ */

void Camera2Quat(cameradata *ca, float *quat, float *rotation){
  if(ca->quat_defined == 1){
    quat[0] = ca->quaternion[0];
    quat[1] = ca->quaternion[1];
    quat[2] = ca->quaternion[2];
    quat[3] = ca->quaternion[3];
  }
  else{
    float quat_temp[4];
    float azimuth, elevation, axis[3];

    azimuth = ca->az_elev[0] * DEG2RAD;
    elevation = (ca->az_elev[1])*DEG2RAD;

    axis[0] = 1.0;
    axis[1] = 0.0;
    axis[2] = 0.0;

    AngleAxis2Quat(elevation, axis, quat_temp);

    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;

    AngleAxis2Quat(azimuth, axis, quat);

    MultQuat(quat_temp, quat, quat);
  }

  if(rotation != NULL)Quat2Rot(quat, rotation);
}

/* ------------------ EnableDisableViews ------------------------ */

void EnableDisableViews(void){
  int i, ival, have_user;
  cameradata *cex;

  ival = LIST_viewpoints->get_int_val();
  if(ival >= 0){

    selected_view = ival;

    cex = &camera_list_first;
    cex = cex->next; // skip over first
    cex = cex->next; // skip over external
    if(cex->next == NULL){
      BUTTON_cycle_views_user->disable();
      BUTTON_cycle_views_default->disable();
      BUTTON_cycle_views_all->disable();
    }
    else{
      BUTTON_cycle_views_user->enable();
      BUTTON_cycle_views_default->enable();
      BUTTON_cycle_views_all->enable();
    }
  }
  have_user = 0;
  for(i = 0; i<ncameras_sorted; i++){
    cameradata *ca;

    ca = cameras_sorted[i];
    if(ca->view_id>1){
      have_user = 1;
      break;
    }
  }
  if(have_user==0){
    BUTTON_cycle_views_user->disable();
    BUTTON_cycle_views_all->disable();
  }
  else{
    BUTTON_cycle_views_user->enable();
    BUTTON_cycle_views_all->enable();
  }
  if(ival<=1){
    BUTTON_replace_view->disable();
    BUTTON_delete_view->disable();
  }
  else{
    BUTTON_replace_view->enable();
    BUTTON_delete_view->enable();
  }
}

/*------------------GLUISetCurrentViewPoint------------------------ */

extern "C" void GLUISetCurrentViewPoint(char *viewpoint_label){
  int i;

  if(strlen(viewpoint_label)==0)return;
  for(i = 0; i<ncameras_sorted; i++){
    cameradata *ca;

    ca = cameras_sorted[i];
    if(strcmp(ca->name, viewpoint_label)==0){
      LIST_viewpoints->set_int_val(ca->view_id);
      GLUIViewpointCB(LIST_VIEW);
      glutPostRedisplay();
      break;
    }
  }
}

/* ------------------ NextViewpoint ------------------------ */

int NextViewpoint(int this_view, int view_type){
  int i, istart;

  istart = ncameras_sorted-1;
  for(i = 0; i<ncameras_sorted; i++){
    cameradata *ca;

    ca = cameras_sorted[i];
    if(ca->view_id==this_view){
      istart = i;
      break;
    }
  }
  for(i = istart+1; i<2*ncameras_sorted; i++){
    cameradata *ca;
    int ii;

    ii = i%ncameras_sorted;
    ca = cameras_sorted[ii];
    if(view_type==CYCLEVIEWS_ALL){
      return ca->view_id;
    }
    else if(view_type==CYCLEVIEWS_USER){
      if(ca->view_id<=1)continue;
      return ca->view_id;
    }
    else{
      if(ca->view_id>1)continue;
      return ca->view_id;
    }
  }
  return cameras_sorted[istart+1]->view_id;
}

/* ------------------ GLUIViewpointCB ------------------------ */

extern "C" void GLUIViewpointCB(int var){
  int ival;
  int rotation_type_save;
  cameradata *cam1, *cex, *ca;
  char *label;
  cameradata *prev, *next;
  int view_id;
#ifdef pp_RENDER360_DEBUG
  int i;
#endif

  switch(var){
  case GEOM_FACTORS:
    updatemenu = 1;
    use_geom_factors = 1 - use_geom_factors;
    void ResetDefaultMenu(int var);
    ResetDefaultMenu(2);
    break;
#ifdef pp_RENDER360_DEBUG
  case SHOWALL_SCREENS:
    for(i = 0;i < nscreeninfo;i++){
      screenvis[i] = 1;
      CHECKBOX_screenvis[i]->set_int_val(screenvis[i]);
    }
    break;
  case HIDEALL_SCREENS:
    for(i = 0;i < nscreeninfo;i++){
      screenvis[i] = 0;
      CHECKBOX_screenvis[i]->set_int_val(screenvis[i]);
    }
    break;
#endif
  case RESTORE_EXTERIOR_VIEW:
    SetViewPoint(var);
    break;
  case SAVE_VIEW:
    strcpy(camera_current->name, camera_label);
    BUTTON_reset_saved_view->enable();
    ViewpointMenu(SAVE_VIEW);
    break;
  case LABEL_VIEW:
    updatemenu = 1;
    break;
  case REPLACE_CURRENT_VIEW:
    {
      int current_view_id=-1;

      cex = &camera_list_first;
      cex = cex->next;
      cex = cex->next;
      for(ca = cex; ca->next!=NULL; ca = ca->next){
        if(strcmp(ca->name,"current")==0){
          current_view_id = ca->view_id;
        }
      }
      if(current_view_id!=-1){
        LIST_viewpoints->set_int_val(current_view_id);
        GLUIViewpointCB(REPLACE_VIEW);
      }
    }
    break;
  case REPLACE_VIEW:
    ival = LIST_viewpoints->get_int_val();
    selected_view = ival;
    label = EDIT_view_label->get_text();
    cex = &camera_list_first;
    cex = cex->next;
    cex = cex->next;
    for(ca = cex;ca->next != NULL;ca = ca->next){
      if(ca->view_id == ival)break;
    }
    if(ival != ca->view_id){
      return;
    }
    prev = ca->prev;
    next = ca->next;
    view_id = ca->view_id;
    CopyCamera(ca, camera_current);
    ca->prev = prev;
    ca->next = next;
    ca->view_id = view_id;

    LIST_viewpoints->delete_item(ival);
    LIST_viewpoints->add_item(ival, label);
    strcpy(ca->name, label);

    break;
  case ADD_VIEW:

    GetUniqueViewName();
    GLUIAddListView(NULL);
    GLUIViewpointCB(LIST_VIEW);
    break;
  case DELETE_VIEW:
    ival = LIST_viewpoints->get_int_val();
    cex = &camera_list_first;
    cex = cex->next;
    cex = cex->next;
    for(ca = cex;ca->next != NULL;ca = ca->next){
      if(ca->view_id == ival)break;
    }
    if(ival == ca->view_id){
      cam1 = ca;
    }
    else{
      return;
    }
    LIST_viewpoints->delete_item(ival);
    prev = cam1->prev;
    DeleteCamera(cam1);
    if(prev->view_id != -1){
      LIST_viewpoints->set_int_val(prev->view_id);
      selected_view = prev->view_id;
    }
    else{
      LIST_viewpoints->set_int_val(0);
      selected_view = 0;
    }
    if(selected_view<=1){
      ResetDefaultMenu(selected_view);
    }
    else{
      GLUIResetView(selected_view);
    }
    EnableDisableViews();
    break;
  case RESTORE_VIEW:
    if(use_customview==1){
      use_customview=0;
      if(CHECKBOX_use_customview!=NULL)CHECKBOX_use_customview->set_int_val(use_customview);
      GLUISceneMotionCB(CUSTOM_VIEW);
    }
    ival = LIST_viewpoints->get_int_val();
    selected_view = ival;
    for(ca = camera_list_first.next;ca->next != NULL;ca = ca->next){
      if(ca->view_id == ival)break;
    }

    rotation_type_save = ca->rotation_type;
    CopyViewCamera(camera_current, ca);
    if(rotation_type == ROTATION_3AXIS)Camera2Quat(camera_current, quat_general, quat_rotation);
    if(strcmp(ca->name, "external") == 0)updatezoommenu = 1;
    camera_current->rotation_type = rotation_type_save;
    EDIT_view_label->set_text(ca->name);
    break;
  case LIST_VIEW:
  case LIST_VIEW_FROM_DIALOG:
    {
      int camera_id;

      camera_id = LIST_viewpoints->get_int_val();
      if(camera_id<=1){
        ResetDefaultMenu(camera_id);
      }
      else{
        ResetMenu(camera_id);
      }
    }
    break;
  case MOTION_STARTUP:
    startup_view_ini = LIST_viewpoints->get_int_val();
    {
      char *cam_label;

      cam_label = GetCameraLabel(startup_view_ini);
      if(cam_label != NULL){
        strcpy(viewpoint_label_startup, cam_label);
      }
    }
    selected_view = startup_view_ini;
    WriteIni(LOCAL_INI, NULL);
    break;
  case CYCLEVIEWS_USER:
  case CYCLEVIEWS_DEFAULT:
  case CYCLEVIEWS_ALL:
    ival = LIST_viewpoints->get_int_val();
    ival = NextViewpoint(ival, var);
    LIST_viewpoints->set_int_val(ival);
    selected_view = ival;
    if(ival<=1){
      ResetDefaultMenu(ival);
    }
    else{
      GLUIResetView(ival);
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIResetView ------------------------ */

extern "C" void GLUIResetView(int ival){
  assert(ival>=-5);
  if(ival!=old_listview){
    old_listview = ival;
    LIST_viewpoints->set_int_val(ival);
  }
  selected_view=ival;
  BUTTON_replace_view->enable();
  GLUIViewpointCB(RESTORE_VIEW);
  EnableDisableViews();
}

/* ------------------ GLUIEnableResetSavedView ------------------------ */

extern "C" void GLUIEnableResetSavedView(void){
  if(BUTTON_reset_saved_view!=NULL)BUTTON_reset_saved_view->enable();
}


/* ------------------ GLUIUpdateFileLabel ------------------------ */

extern "C" void GLUIUpdateFileLabel(int var){
  if(var==0||var==1){
    if(RADIO_render_label!=NULL){
      int val1;

      val1 = RADIO_render_label->get_int_val();
      if(val1!=var){
        RADIO_render_label->set_int_val(var);
      }
    }
  }
}

/* ------------------ GLUIUpdateWindowAspect ------------------------ */

extern "C" void GLUIUpdateWindowAspect(void){
  if(CHECKBOX_fix_window_aspect != NULL)CHECKBOX_fix_window_aspect->set_int_val(fix_window_aspect);
}

/* ------------------ GLUIUpdateZoom ------------------------ */

extern "C" void GLUIUpdateZoom(void){
  if(SPINNER_zoom!=NULL)SPINNER_zoom->set_float_val(zoom);
  aperture_glui=Zoom2Aperture(zoom);
  if(SPINNER_aperture!=NULL)SPINNER_aperture->set_float_val(aperture_glui);
}

/* ------------------ GLUIUpdateCameraLabel ------------------------ */

extern "C" void GLUIUpdateCameraLabel(void){
  EDIT_view_label->set_text(camera_label);
}

/* ------------------ GLUIUpdateViewpointList ------------------------ */

extern "C" void GLUIUpdateViewpointList(void){
  cameradata *ca;
  int i;

  if(LIST_viewpoints == NULL)return;
  for(ca=camera_list_first.next;ca->next!=NULL;ca=ca->next){
    LIST_viewpoints->delete_item(ca->name);
  }
  SortCamerasID();
  for(i = 0; i < ncameras_sorted;i++){
    ca = cameras_sorted[i];
    LIST_viewpoints->add_item(ca->view_id, ca->name);
  }
  LIST_viewpoints->set_int_val(startup_view_ini);
  selected_view=startup_view_ini;
  EnableDisableViews();
  GLUIViewpointCB(RESTORE_VIEW);
}

/* ------------------ MotionDlgCB ------------------------ */

void MotionDlgCB(int var){
  switch(var){
  case CLOSE_MOTION:
#ifndef pp_CLOSEOFF
    if(glui_motion != NULL)glui_motion->hide();
#endif
    updatemenu = 1;
    break;
  case SAVE_SETTINGS_MOTION:
    updatemenu = 1;
    WriteIni(LOCAL_INI, NULL);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIMotionSetup ------------------------ */

extern "C" void GLUIMotionSetup(int main_window){
  int i;
#define TRANSLATE_SPEED 0.005
  int *rotation_index;
#ifdef ROTATE_TRANSLATE
  float *eye_xyz;
#endif

  if(camera_label!=NULL){
    FREEMEMORY(camera_label);
  }
  NewMemory((void **)&camera_label,sizeof(GLUI_String));

  strcpy(camera_label,"current");

#ifdef ROTATE_TRANSLATE
  eye_xyz=camera_current->eye;
#endif

  if(glui_motion!=NULL){
    glui_motion->close();
    glui_motion=NULL;
  }
  glui_motion = GLUI_Master.create_glui(_("Motion/View/Render"),0,dialogX0,dialogY0);
  glui_motion->hide();

  ROLLOUT_motion = glui_motion->add_rollout("Motion",false, MOTION_ROLLOUT, MVRRolloutCB);
  TOGGLE_ROLLOUT(mvrprocinfo, nmvrprocinfo, ROLLOUT_motion, MOTION_ROLLOUT, glui_motion);

#ifdef ROTATE_TRANSLATE
  ROLLOUT_translaterotate=glui_motion->add_rollout_to_panel(ROLLOUT_motion, _("Translate/Rotate"), true, TRANSLATEROTATE_ROLLOUT, MotionRolloutCB);
  TOGGLE_ROLLOUT(motionprocinfo, nmotionprocinfo, ROLLOUT_translaterotate, TRANSLATEROTATE_ROLLOUT, glui_motion);

  PANEL_translate2 = glui_motion->add_panel_to_panel(ROLLOUT_translaterotate,_("Translate"));
  d_eye_xyz[0]=0.0;
  d_eye_xyz[1]=0.0;
  d_eye_xyz[2]=0.0;
  dsave_eye_xyz[0]=0.0;
  dsave_eye_xyz[1]=0.0;
  dsave_eye_xyz[2]=0.0;

  TRANSLATE_xy=glui_motion->add_translation_to_panel(PANEL_translate2,_("Horizontal"),GLUI_TRANSLATION_XY,d_eye_xyz,TRANSLATE_XY,GLUISceneMotionCB);
  TRANSLATE_xy->set_speed(TRANSLATE_SPEED);

  TRANSLATE_x = glui_motion->add_translation_to_panel(PANEL_translate2, _("Left/Right"), GLUI_TRANSLATION_X, d_eye_xyz, TRANSLATE_X, GLUISceneMotionCB);
  TRANSLATE_x->set_speed(TRANSLATE_SPEED);

  glui_motion->add_column_to_panel(PANEL_translate2,false);

  TRANSLATE_z=glui_motion->add_translation_to_panel(PANEL_translate2,_("Vertical"),GLUI_TRANSLATION_Y,eye_xyz+2,GLUI_Z,GLUISceneMotionCB);
  TRANSLATE_z->set_speed(TRANSLATE_SPEED);

  TRANSLATE_y = glui_motion->add_translation_to_panel(PANEL_translate2, _("Front/Back"), GLUI_TRANSLATION_Y, d_eye_xyz+1, TRANSLATE_Y, GLUISceneMotionCB);
  TRANSLATE_y->set_speed(TRANSLATE_SPEED);

  PANEL_rotate = glui_motion->add_panel_to_panel(ROLLOUT_translaterotate,_("Rotate"));

  ROTATE_2axis=glui_motion->add_translation_to_panel(PANEL_rotate,_("2 axis"),GLUI_TRANSLATION_XY,motion_ab,ROTATE_2AXIS,GLUISceneMotionCB);
  glui_motion->add_column_to_panel(PANEL_rotate,false);

  ROTATE_eye_z=glui_motion->add_translation_to_panel(PANEL_rotate,_("View"),GLUI_TRANSLATION_X,motion_dir,EYE_ROTATE,GLUISceneMotionCB);
  ROTATE_eye_z->set_speed(180.0/(float)screenWidth);
  ROTATE_eye_z->disable();
  glui_motion->add_button_to_panel(ROLLOUT_translaterotate, _("Snap"), SNAPSCENE, GLUISceneMotionCB);
#endif

  ROLLOUT_view = glui_motion->add_rollout_to_panel(ROLLOUT_motion, _("Position/View"), false, POSITION_VIEW_ROLLOUT, MotionRolloutCB);
  TOGGLE_ROLLOUT(motionprocinfo, nmotionprocinfo, ROLLOUT_view, POSITION_VIEW_ROLLOUT, glui_motion);

  PANEL_specify = glui_motion->add_panel_to_panel(ROLLOUT_view, _("Position"));

  SPINNER_glui_xyz_fds[0] = glui_motion->add_spinner_to_panel(PANEL_specify, "x:", GLUI_SPINNER_FLOAT, glui_xyz_fds,   SET_VIEW_XYZ, GLUISceneMotionCB);
  SPINNER_glui_xyz_fds[1] = glui_motion->add_spinner_to_panel(PANEL_specify, "y:", GLUI_SPINNER_FLOAT, glui_xyz_fds+1, SET_VIEW_XYZ, GLUISceneMotionCB);
  SPINNER_glui_xyz_fds[2] = glui_motion->add_spinner_to_panel(PANEL_specify, "z:", GLUI_SPINNER_FLOAT, glui_xyz_fds+2, SET_VIEW_XYZ, GLUISceneMotionCB);
  SPINNER_glui_xyz_fds_check = SPINNER_glui_xyz_fds;

  PANEL_custom_view = glui_motion->add_panel_to_panel(ROLLOUT_view, _("View"));

  CHECKBOX_use_customview = glui_motion->add_checkbox_to_panel(PANEL_custom_view,_("Use azimuth/elevation"),&use_customview, CUSTOM_VIEW, GLUISceneMotionCB);
  SPINNER_customview_azimuth = glui_motion->add_spinner_to_panel(PANEL_custom_view,"azimuth:",GLUI_SPINNER_FLOAT,&customview_azimuth,CUSTOM_VIEW,GLUISceneMotionCB);
  SPINNER_customview_elevation = glui_motion->add_spinner_to_panel(PANEL_custom_view,"elevation:", GLUI_SPINNER_FLOAT, &customview_elevation, CUSTOM_VIEW, GLUISceneMotionCB);
  //glui_motion->add_spinner_to_panel(PANEL_custom_view,"     up:", GLUI_SPINNER_FLOAT, &customview_up, CUSTOM_VIEW, GLUISceneMotionCB);
  {
    char rotate_label[100];

    //sprintf(rotate_label,"%s rotate",deg90);
    snprintf(rotate_label,sizeof(rotate_label),"%s rotate",deg90);
    BUTTON_rotate90 = glui_motion->add_button_to_panel(PANEL_custom_view, rotate_label, ROTATE_90, GLUISceneMotionCB);
  }
  glui_motion->add_button_to_panel(PANEL_custom_view, "Reset", RESET_VIEW, GLUISceneMotionCB);

  ROLLOUT_rotation_type = glui_motion->add_rollout_to_panel(ROLLOUT_motion,_("Specify Rotation"),false,ROTATION_ROLLOUT,MotionRolloutCB);
  TOGGLE_ROLLOUT(motionprocinfo, nmotionprocinfo, ROLLOUT_rotation_type, ROTATION_ROLLOUT, glui_motion);

  PANEL_radiorotate = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type, "Rotation type:");
  RADIO_rotation_type=glui_motion->add_radiogroup_to_panel(PANEL_radiorotate,&rotation_type,0, GLUIRotationTypeCB);
  RADIOBUTTON_1c=glui_motion->add_radiobutton_to_group(RADIO_rotation_type,_("2 axis"));
  RADIOBUTTON_1d=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("eye centered"));
  RADIOBUTTON_1e=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("level (1 axis)"));
  RADIOBUTTON_1e=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("3 axis"));
  GLUIRotationTypeCB(rotation_type);
  rotation_index=&camera_current->rotation_index;
  *rotation_index=glui_rotation_index_ini;

  LIST_rotate_about = glui_motion->add_listbox_to_panel(ROLLOUT_rotation_type,_("Rotate about:"), rotation_index, ROTATE_ABOUT,GLUISceneMotionCB);
  LIST_rotate_about->add_item(ROTATE_ABOUT_CLIPPING_CENTER, _("center of clipping planes"));
  LIST_rotate_about->add_item(ROTATE_ABOUT_USER_CENTER,_("user specified center"));
  if(have_geom_bb==1){
    LIST_rotate_about->add_item(ROTATE_ABOUT_FDS_CENTER, _("FDS domain center"));
    LIST_rotate_about->add_item(ROTATE_ABOUT_WORLD_CENTER, _("FDS+Geometry domain center"));
  }
  else{
    LIST_rotate_about->add_item(ROTATE_ABOUT_WORLD_CENTER, _("FDS domain center"));
  }
  LIST_rotate_about->set_int_val(ROTATE_ABOUT_WORLD_CENTER);

  PANEL_user_center = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type, "rotation center");
  CHECKBOX_show_rotation_center=glui_motion->add_checkbox_to_panel(PANEL_user_center,_("Show"),&show_rotation_center, CLIP_SHOW_ROTATE, GLUISceneMotionCB);
  xcenCUSTOMsmv = SMV2FDS_X(xcenCUSTOM);
  ycenCUSTOMsmv = SMV2FDS_Y(ycenCUSTOM);
  zcenCUSTOMsmv = SMV2FDS_Z(zcenCUSTOM);
  SPINNER_xcenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"x:",GLUI_SPINNER_FLOAT,&xcenCUSTOMsmv, CUSTOM_ROTATION_XYZ,GLUISceneMotionCB);
  SPINNER_ycenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"y:",GLUI_SPINNER_FLOAT,&ycenCUSTOMsmv, CUSTOM_ROTATION_XYZ,GLUISceneMotionCB);
  SPINNER_zcenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"z:",GLUI_SPINNER_FLOAT,&zcenCUSTOMsmv, CUSTOM_ROTATION_XYZ,GLUISceneMotionCB);
  SPINNER_xcenCUSTOM->set_float_limits(SMV2FDS_X(0.0),SMV2FDS_X(1.0));
  SPINNER_ycenCUSTOM->set_float_limits(SMV2FDS_Y(0.0),SMV2FDS_Y(1.0));
  SPINNER_zcenCUSTOM->set_float_limits(SMV2FDS_Z(0.0),SMV2FDS_Z(1.0));

  GLUISceneMotionCB(ROTATE_ABOUT);

  PANEL_anglebuttons = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type,"",GLUI_PANEL_NONE);
  BUTTON_90_z=glui_motion->add_button_to_panel(PANEL_anglebuttons,"90 deg",EYE_ROTATE_90,GLUISceneMotionCB);
  BUTTON_90_z->disable();
  BUTTON_90_z->set_alignment(GLUI_ALIGN_LEFT);
//  glui_motion->add_column_to_panel(PANEL_anglebuttons,false);
  BUTTON_snap=glui_motion->add_button_to_panel(PANEL_anglebuttons,_("Snap"),SNAPSCENE,GLUISceneMotionCB);

  //glui_motion->add_column(false);

  ROLLOUT_orientation=glui_motion->add_rollout_to_panel(ROLLOUT_motion,_("Orientation"),false,ORIENTATION_ROLLOUT,MotionRolloutCB);
  TOGGLE_ROLLOUT(motionprocinfo, nmotionprocinfo, ROLLOUT_orientation, ORIENTATION_ROLLOUT, glui_motion);

  PANEL_change_zaxis = glui_motion->add_panel_to_panel(ROLLOUT_orientation,_("z axis"));

  if(zaxis_custom==0){
    float vv[3], maxvv;


    if(global_scase.have_gvec==1){
      vv[0] = -global_scase.gvecphys[0];
      vv[1] = -global_scase.gvecphys[1];
      vv[2] = -global_scase.gvecphys[2];
    }
    else{
      vv[0] = -gvecphys_orig[0];
      vv[1] = -gvecphys_orig[1];
      vv[2] = -gvecphys_orig[2];
    }
    maxvv = MAXABS3(vv);
    vv[0] /= maxvv;
    vv[1] /= maxvv;
    vv[2] /= maxvv;
    XYZ2AzElev(vv, zaxis_angles, zaxis_angles+1);
  }
  SPINNER_zaxis_angles[0] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("azimuth:"),GLUI_SPINNER_FLOAT, zaxis_angles,ZAXIS_CUSTOM, GLUISceneMotionCB);
  SPINNER_zaxis_angles[1] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("elevation:"),GLUI_SPINNER_FLOAT,zaxis_angles+1,ZAXIS_CUSTOM,GLUISceneMotionCB);
  SPINNER_zaxis_angles[2] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("angle (about z axis):"),GLUI_SPINNER_FLOAT,zaxis_angles+2,ZAXIS_CUSTOM,GLUISceneMotionCB);
  SPINNER_zaxis_angles[0]->set_float_limits(-180.0,180.0);
  SPINNER_zaxis_angles[1]->set_float_limits(-90.0,90.0);
  SPINNER_zaxis_angles[2]->set_float_limits(-180.0,180.0);

  glui_motion->add_button_to_panel(PANEL_change_zaxis, "z vector up", ZAXIS_UP, GLUISceneMotionCB);
  if(global_scase.have_gvec==1){
    glui_motion->add_button_to_panel(PANEL_change_zaxis, "Gravity vector down", USE_GVEC, GLUISceneMotionCB);
    CHECKBOX_showgravity_vector = glui_motion->add_checkbox_to_panel(PANEL_change_zaxis, "Show gravity, axis vectors", &showgravity_vector);
  }
  else{
    CHECKBOX_showgravity_vector = glui_motion->add_checkbox_to_panel(PANEL_change_zaxis,_("Show axis vectors"),&showgravity_vector);
  }
  GLUISceneMotionCB(ZAXIS_CUSTOM);
  ROLLOUT_orientation->close();
  zaxis_custom=0;

  ROLLOUT_gslice = glui_motion->add_rollout_to_panel(ROLLOUT_motion, _("Slice motion"),false,SLICE_ROLLOUT_MOTION,MotionRolloutCB);
  TOGGLE_ROLLOUT(motionprocinfo,nmotionprocinfo,ROLLOUT_gslice,SLICE_ROLLOUT_MOTION, glui_motion);

  if(gslice_xyz[0]<-1000000.0&&gslice_xyz[1]<-1000000.0&&gslice_xyz[2]<-1000000.0){
    gslice_xyz[0]=(global_scase.xbar0+SMV2FDS_X(global_scase.xbar))/2.0;
    gslice_xyz[1]=(global_scase.ybar0+SMV2FDS_Y(global_scase.ybar))/2.0;
    gslice_xyz[2]=(global_scase.zbar0+SMV2FDS_Z(global_scase.zbar))/2.0;
  }

  PANEL_gslice_center = glui_motion->add_panel_to_panel(ROLLOUT_gslice,_("rotation center"),true);
  SPINNER_gslice_center_x=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"x:",GLUI_SPINNER_FLOAT,gslice_xyz,  GSLICE_TRANSLATE, GSliceCB);
  SPINNER_gslice_center_y=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"y:",GLUI_SPINNER_FLOAT,gslice_xyz+1,GSLICE_TRANSLATE, GSliceCB);
  SPINNER_gslice_center_z=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"z:",GLUI_SPINNER_FLOAT,gslice_xyz+2,GSLICE_TRANSLATE, GSliceCB);
  SPINNER_gslice_center_x->set_float_limits(global_scase.xbar0,SMV2FDS_X(global_scase.xbar),GLUI_LIMIT_CLAMP);
  SPINNER_gslice_center_y->set_float_limits(global_scase.ybar0,SMV2FDS_Y(global_scase.ybar),GLUI_LIMIT_CLAMP);
  SPINNER_gslice_center_z->set_float_limits(global_scase.zbar0,SMV2FDS_Z(global_scase.zbar),GLUI_LIMIT_CLAMP);
  GSliceCB(GSLICE_TRANSLATE);

  PANEL_gslice_normal = glui_motion->add_panel_to_panel(ROLLOUT_gslice,_("normal"),true);
  SPINNER_gslice_normal_az=glui_motion->add_spinner_to_panel(PANEL_gslice_normal,"az:",GLUI_SPINNER_FLOAT,gslice_normal_azelev,GSLICE_NORMAL, GSliceCB);
  SPINNER_gslice_normal_elev=glui_motion->add_spinner_to_panel(PANEL_gslice_normal,"elev:",GLUI_SPINNER_FLOAT,gslice_normal_azelev+1,GSLICE_NORMAL, GSliceCB);
  GSliceCB(GSLICE_NORMAL);

  PANEL_gslice_show = glui_motion->add_panel_to_panel(ROLLOUT_gslice,_("show"),true);
  CHECKBOX_gslice_data=glui_motion->add_checkbox_to_panel(PANEL_gslice_show,_("data"),&vis_gslice_data);
  glui_motion->add_checkbox_to_panel(PANEL_gslice_show,"triangle outline",&show_gslice_triangles);
  glui_motion->add_checkbox_to_panel(PANEL_gslice_show,"triangulation",&show_gslice_triangulation);
  glui_motion->add_checkbox_to_panel(PANEL_gslice_show,"plane normal",&show_gslice_normal);

  ROLLOUT_viewA = glui_motion->add_rollout(_("View"), false, VIEW_ROLLOUT, MVRRolloutCB);
  TOGGLE_ROLLOUT(mvrprocinfo, nmvrprocinfo, ROLLOUT_viewA, VIEW_ROLLOUT, glui_motion);

  ROLLOUT_viewpoints = glui_motion->add_rollout_to_panel(ROLLOUT_viewA,_("Viewpoints"), false,VIEWPOINTS_ROLLOUT,MotionRolloutCB);
  TOGGLE_ROLLOUT(motionprocinfo,nmotionprocinfo,ROLLOUT_viewpoints,VIEWPOINTS_ROLLOUT, glui_motion);

  PANEL_select = glui_motion->add_panel_to_panel(ROLLOUT_viewpoints, "", false);
  LIST_viewpoints = glui_motion->add_listbox_to_panel(PANEL_select, _("Select:"), &i_view_list, LIST_VIEW_FROM_DIALOG, GLUIViewpointCB);
  LIST_viewpoints->set_alignment(GLUI_ALIGN_CENTER);
  if(have_geom_factors==1){
    CHECKBOX_use_geom_factors = glui_motion->add_checkbox_to_panel(PANEL_select, "include geometry", &use_geom_factors, GEOM_FACTORS, GLUIViewpointCB);
  }
  PANEL_reset = glui_motion->add_panel_to_panel(ROLLOUT_viewpoints, "", false);

  PANEL_reset1 = glui_motion->add_panel_to_panel(PANEL_reset, "", false);

  BUTTON_delete_view = glui_motion->add_button_to_panel(PANEL_reset1, _("Delete"), DELETE_VIEW, GLUIViewpointCB);
  delete_view_is_disabled = 0;
  BUTTON_cycle_views_default = glui_motion->add_button_to_panel(PANEL_reset1, _("Cycle Default"), CYCLEVIEWS_DEFAULT, GLUIViewpointCB);
  BUTTON_cycle_views_user    = glui_motion->add_button_to_panel(PANEL_reset1, _("Cycle User"),    CYCLEVIEWS_USER,    GLUIViewpointCB);
  BUTTON_cycle_views_all     = glui_motion->add_button_to_panel(PANEL_reset1, _("Cycle All"),     CYCLEVIEWS_ALL,     GLUIViewpointCB);

  glui_motion->add_column_to_panel(PANEL_reset, true);
  PANEL_reset2 = glui_motion->add_panel_to_panel(PANEL_reset, "", false);

  BUTTON_add_view = glui_motion->add_button_to_panel(PANEL_reset2, _("Add"), ADD_VIEW, GLUIViewpointCB);
  BUTTON_replace_view = glui_motion->add_button_to_panel(PANEL_reset2, _("Replace"), REPLACE_VIEW, GLUIViewpointCB);
  BUTTON_startup = glui_motion->add_button_to_panel(PANEL_reset2, _("Apply at startup"),  MOTION_STARTUP,            GLUIViewpointCB);
  EDIT_view_label = glui_motion->add_edittext_to_panel(PANEL_reset2, _("Edit:"), GLUI_EDITTEXT_TEXT, camera_label, LABEL_VIEW, GLUIViewpointCB);

  ROLLOUT_projection = glui_motion->add_rollout_to_panel(ROLLOUT_viewA,_("Window properties"), false,WINDOW_ROLLOUT,MotionRolloutCB);
  TOGGLE_ROLLOUT(motionprocinfo,nmotionprocinfo,ROLLOUT_projection,WINDOW_ROLLOUT, glui_motion);

  RADIO_projection = glui_motion->add_radiogroup_to_panel(ROLLOUT_projection, &projection_type, PROJECTION, GLUISceneMotionCB);
  RADIOBUTTON_1a = glui_motion->add_radiobutton_to_group(RADIO_projection, _("Perspective"));
  RADIOBUTTON_1b = glui_motion->add_radiobutton_to_group(RADIO_projection, _("Size preserving"));
  SPINNER_zoom = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("Zoom"), GLUI_SPINNER_FLOAT, &zoom, ZOOM, GLUISceneMotionCB);
  SPINNER_zoom->set_float_limits(zoom_min, zoom_max, GLUI_LIMIT_CLAMP);
  aperture_glui = Zoom2Aperture(zoom);
  aperture_min = Zoom2Aperture(zoom_max);
  aperture_max = Zoom2Aperture(zoom_min);
  SPINNER_aperture = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("aperture"), GLUI_SPINNER_FLOAT, &aperture_glui,
    APERTURE, GLUISceneMotionCB);
  glui_motion->add_separator_to_panel(ROLLOUT_projection);

  LIST_windowsize = glui_motion->add_listbox_to_panel(ROLLOUT_projection, _("Size:"), &windowsize_pointer, WINDOWSIZE_LIST, GLUISceneMotionCB);
  LIST_windowsize->add_item(0, _("Custom"));
  LIST_windowsize->add_item(1, "-");
  LIST_windowsize->add_item(2, "320x240");
  LIST_windowsize->add_item(3, "640x480");
  LIST_windowsize->add_item(7, "720x480");
  if(max_screenWidth >= 800 && max_screenHeight >= 480)LIST_windowsize->add_item(4, "800x640");
  if(max_screenWidth >= 1024 && max_screenHeight >= 768)  LIST_windowsize->add_item(5, "1024x768");
  if(max_screenWidth >= 1280 && max_screenHeight >= 720)  LIST_windowsize->add_item(9, "1280x720");
  if(max_screenWidth >= 1280 && max_screenHeight >= 1024)  LIST_windowsize->add_item(6, "1280x1024");
  if(max_screenWidth >= 1440 && max_screenHeight >= 1024)  LIST_windowsize->add_item(10, "1440x1080");
  if(max_screenWidth >= 1920 && max_screenHeight >= 1080)  LIST_windowsize->add_item(8, "1920x1080");
  GLUIUpdateWindowSizeList();

  SPINNER_window_width = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("width"),   GLUI_SPINNER_INT, &glui_screenWidth,  WINDOW_RESIZE_WIDTH,  GLUISceneMotionCB);
  SPINNER_window_width->set_int_limits(100, max_screenWidth);
  SPINNER_window_height = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("height"), GLUI_SPINNER_INT, &glui_screenHeight, WINDOW_RESIZE_HEIGHT, GLUISceneMotionCB);
  SPINNER_window_height->set_int_limits(100, max_screenHeight);
  CHECKBOX_fix_window_aspect = glui_motion->add_checkbox_to_panel(ROLLOUT_projection, "fix width/height", &fix_window_aspect, WINDOW_PRESERVE, GLUISceneMotionCB);

  PANEL_colors = glui_motion->add_panel_to_panel(ROLLOUT_projection, "Colors", true);

  PANEL_foreground = glui_motion->add_panel_to_panel(PANEL_colors,_("Background"), true);
  SPINNER_foreground_red = glui_motion->add_spinner_to_panel(PANEL_foreground,_("red"),GLUI_SPINNER_INT,glui_foregroundbasecolor,WINDOW_COLORS,GLUISceneMotionCB);
  SPINNER_foreground_green = glui_motion->add_spinner_to_panel(PANEL_foreground, _("green"), GLUI_SPINNER_INT, glui_foregroundbasecolor+1, WINDOW_COLORS, GLUISceneMotionCB);
  SPINNER_foreground_blue = glui_motion->add_spinner_to_panel(PANEL_foreground, _("blue"), GLUI_SPINNER_INT, glui_foregroundbasecolor+2, WINDOW_COLORS, GLUISceneMotionCB);
  SPINNER_foreground_red->set_int_limits(0, 255);
  SPINNER_foreground_green->set_int_limits(0, 255);
  SPINNER_foreground_blue->set_int_limits(0, 255);

  PANEL_background = glui_motion->add_panel_to_panel(PANEL_colors, _("Foreground"), true);
  SPINNER_background_red = glui_motion->add_spinner_to_panel(PANEL_background,_("red"),GLUI_SPINNER_INT,glui_backgroundbasecolor,WINDOW_COLORS,GLUISceneMotionCB);
  SPINNER_background_green = glui_motion->add_spinner_to_panel(PANEL_background,_("green"),GLUI_SPINNER_INT,glui_backgroundbasecolor+1,WINDOW_COLORS,GLUISceneMotionCB);
  SPINNER_background_blue = glui_motion->add_spinner_to_panel(PANEL_background,_("blue"),GLUI_SPINNER_INT,glui_backgroundbasecolor+2,WINDOW_COLORS,GLUISceneMotionCB);
  SPINNER_background_red->set_int_limits(0, 255);
  SPINNER_background_green->set_int_limits(0, 255);
  SPINNER_background_blue->set_int_limits(0, 255);

  BUTTON_flip = glui_motion->add_button_to_panel(PANEL_colors, _("Flip"), COLOR_FLIP, GLUISceneMotionCB);

  ROLLOUT_scale = glui_motion->add_rollout_to_panel(ROLLOUT_viewA,_("Scaling"),false,SCALING_ROLLOUT,MotionRolloutCB);
  TOGGLE_ROLLOUT(motionprocinfo,nmotionprocinfo,ROLLOUT_scale,SCALING_ROLLOUT, glui_motion);

  SPINNER_scalex=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_A(_("Scale")," x"),GLUI_SPINNER_FLOAT,mscale);
  SPINNER_scalex->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

  SPINNER_scaley=glui_motion->add_spinner_to_panel(ROLLOUT_scale, _A(_("Scale"), " y"),GLUI_SPINNER_FLOAT,mscale+1);
  SPINNER_scaley->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

  SPINNER_scalez=glui_motion->add_spinner_to_panel(ROLLOUT_scale, _A(_("Scale"), " z"),GLUI_SPINNER_FLOAT,mscale+2);
  SPINNER_scalez->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

  SPINNER_nearclip=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_("Near depth"),GLUI_SPINNER_FLOAT,&nearclip, NEARFARCLIP, GLUISceneMotionCB);

  SPINNER_farclip=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_("Far depth"),GLUI_SPINNER_FLOAT,&farclip, NEARFARCLIP, GLUISceneMotionCB);

  ROLLOUT_render = glui_motion->add_rollout(_("Render"), false, RENDER_ROLLOUT, MVRRolloutCB);
  TOGGLE_ROLLOUT(mvrprocinfo, nmvrprocinfo, ROLLOUT_render, RENDER_ROLLOUT, glui_motion);

  BUTTON_render_start = glui_motion->add_button_to_panel(ROLLOUT_render, _("Start rendering"), RENDER_START_TOP, RenderCB);
  glui_motion->add_button_to_panel(ROLLOUT_render, _("Stop rendering"), RENDER_STOP, RenderCB);

  ROLLOUT_name = glui_motion->add_rollout_to_panel(ROLLOUT_render, "File name/type", false, RENDER_FILE_ROLLOUT, SubRenderRolloutCB);
  TOGGLE_ROLLOUT(subrenderprocinfo,nsubrenderprocinfo,ROLLOUT_name,RENDER_FILE_ROLLOUT, glui_motion);

  EDIT_render_file_base = glui_motion->add_edittext_to_panel(ROLLOUT_name, "prefix:", GLUI_EDITTEXT_TEXT, render_file_base);
  EDIT_render_file_base->set_w(200);

  PANEL_render_file = glui_motion->add_panel_to_panel(ROLLOUT_name, "", false);

  PANEL_file_suffix = glui_motion->add_panel_to_panel(PANEL_render_file, "suffix:", true);
  RADIO_render_label = glui_motion->add_radiogroup_to_panel(PANEL_file_suffix, &render_label_type, RENDER_LABEL, RenderCB);
  RADIOBUTTON_1f = glui_motion->add_radiobutton_to_group(RADIO_render_label, "frame number");
  RADIOBUTTON_1g = glui_motion->add_radiobutton_to_group(RADIO_render_label, "time (s)");

  glui_motion->add_column_to_panel(PANEL_render_file, false);

  PANEL_file_type = glui_motion->add_panel_to_panel(PANEL_render_file, "type:", true);
  RADIO_render_type = glui_motion->add_radiogroup_to_panel(PANEL_file_type, &render_filetype, RENDER_TYPE, RenderCB);
  glui_motion->add_radiobutton_to_group(RADIO_render_type, "png");
  glui_motion->add_radiobutton_to_group(RADIO_render_type, "jpg");

  glui_motion->add_button_to_panel(PANEL_render_file, "Render to html", RENDER_HTML, RenderCB);

  LIST_render_skip = glui_motion->add_listbox_to_panel(ROLLOUT_render, _("Show:"), &render_skip, RENDER_SKIP, RenderCB);
  for(i = 0; i<NRENDER_SKIPS; i++){
    LIST_render_skip->add_item(render_skips[i], crender_skips[i]);
  }
  LIST_render_skip->set_int_val(render_skip);

  ROLLOUT_image_size = glui_motion->add_rollout_to_panel(ROLLOUT_render, "size/type", false, RENDER_SIZE_ROLLOUT, SubRenderRolloutCB);
  TOGGLE_ROLLOUT(subrenderprocinfo,nsubrenderprocinfo,ROLLOUT_image_size,RENDER_SIZE_ROLLOUT, glui_motion);

  RADIO_render_resolution = glui_motion->add_radiogroup_to_panel(ROLLOUT_image_size, &render_resolution);
  glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("320x240"));
  glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("640x480"));
  {
    RADIOBUTTON_render_current=glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("current"));
    RADIOBUTTON_render_high=glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("high"));
  }
  RADIOBUTTON_render_360=glui_motion->add_radiobutton_to_group(RADIO_render_resolution, _("360"));

  render_size_index = RenderWindow;
  RenderCB(RENDER_RESOLUTION);

  glui_resolution_multiplier=CLAMP(resolution_multiplier,2,10);
  SPINNER_resolution_multiplier = glui_motion->add_spinner_to_panel(ROLLOUT_image_size, "multiplier:", GLUI_SPINNER_INT, &glui_resolution_multiplier, RENDER_MULTIPLIER, RenderCB);
  SPINNER_resolution_multiplier->set_int_limits(2, 10);
  RenderCB(RENDER_MULTIPLIER);

  PANEL_360 = glui_motion->add_panel_to_panel(ROLLOUT_image_size, (char *)deg360, true);
  SPINNER_window_height360 = glui_motion->add_spinner_to_panel(PANEL_360, "height", GLUI_SPINNER_INT, &nheight360, RENDER_360CB, RenderCB);
  SPINNER_window_height360->set_int_limits(100, max_screenHeight);
  RenderCB(RENDER_360CB);
  glui_motion->add_spinner_to_panel(PANEL_360, "margin", GLUI_SPINNER_INT, &margin360_size);

#ifdef pp_RENDER360_DEBUG
  PANEL_360_debug = glui_motion->add_panel_to_panel(PANEL_360, "grid", true);
  glui_motion->add_checkbox_to_panel(PANEL_360_debug, "show", &debug_360);
  SPINNER_360_skip_x = glui_motion->add_spinner_to_panel(PANEL_360_debug, "horizontal skip", GLUI_SPINNER_INT, &debug_360_skip_x, RENDER_DEBUG_360, RenderCB);
  SPINNER_360_skip_y = glui_motion->add_spinner_to_panel(PANEL_360_debug, "vertical skip", GLUI_SPINNER_INT, &debug_360_skip_y, RENDER_DEBUG_360, RenderCB);
  RenderCB(RENDER_DEBUG_360);

  NewMemory((void **)&CHECKBOX_screenvis, nscreeninfo * sizeof(GLUI_Checkbox *));

  ROLLOUT_screenvis = glui_motion->add_rollout_to_panel(ROLLOUT_render, "screenvis", false, RENDER_SCREEN_ROLLOUT, SubRenderRolloutCB);
  TOGGLE_ROLLOUT(subrenderprocinfo,nsubrenderprocinfo,ROLLOUT_screenvis,RENDER_SCREEN_ROLLOUT, glui_motion);

  CHECKBOX_screenview = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "view screens", &screenview);
  CHECKBOX_screenvis[0] = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "bottom", screenvis);

  ROLLOUT_lower = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "lower", false, LOWER_SCREEN_ROLLOUT, ScreenRolloutCB);
  TOGGLE_ROLLOUT(screenprocinfo,nscreenprocinfo,ROLLOUT_lower, LOWER_SCREEN_ROLLOUT, glui_motion);
  CHECKBOX_screenvis[1] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "1", screenvis + 1);
  CHECKBOX_screenvis[2] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "2", screenvis + 2);
  CHECKBOX_screenvis[3] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "3", screenvis + 3);
  CHECKBOX_screenvis[4] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "4", screenvis + 4);
  CHECKBOX_screenvis[5] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "5", screenvis + 5);
  CHECKBOX_screenvis[6] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "6", screenvis + 6);
  CHECKBOX_screenvis[7] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "7", screenvis + 7);
  CHECKBOX_screenvis[8] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "8", screenvis + 8);


  ROLLOUT_middle = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "middle", false, MIDDLE_SCREEN_ROLLOUT, ScreenRolloutCB);
  TOGGLE_ROLLOUT(screenprocinfo,nscreenprocinfo,ROLLOUT_middle, MIDDLE_SCREEN_ROLLOUT, glui_motion);
  CHECKBOX_screenvis[9] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "1", screenvis + 9);
  CHECKBOX_screenvis[10] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "2", screenvis + 10);
  CHECKBOX_screenvis[11] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "3", screenvis + 11);
  CHECKBOX_screenvis[12] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "4", screenvis + 12);
  CHECKBOX_screenvis[13] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "5", screenvis + 13);
  CHECKBOX_screenvis[14] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "6", screenvis + 14);
  CHECKBOX_screenvis[15] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "7", screenvis + 15);
  CHECKBOX_screenvis[16] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "8", screenvis + 16);

  ROLLOUT_upper = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "upper", false, UPPER_SCREEN_ROLLOUT, ScreenRolloutCB);
  TOGGLE_ROLLOUT(screenprocinfo,nscreenprocinfo,ROLLOUT_upper, UPPER_SCREEN_ROLLOUT, glui_motion);
  CHECKBOX_screenvis[17] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "1", screenvis + 17);
  CHECKBOX_screenvis[18] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "2", screenvis + 18);
  CHECKBOX_screenvis[19] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "3", screenvis + 19);
  CHECKBOX_screenvis[20] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "4", screenvis + 20);
  CHECKBOX_screenvis[21] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "5", screenvis + 21);
  CHECKBOX_screenvis[22] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "6", screenvis + 22);
  CHECKBOX_screenvis[23] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "7", screenvis + 23);
  CHECKBOX_screenvis[24] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "8", screenvis + 24);

  CHECKBOX_screenvis[25] = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "top", screenvis + 25);
  BUTTON_screen_showall = glui_motion->add_button_to_panel(ROLLOUT_screenvis, _("Show all"), SHOWALL_SCREENS, GLUIViewpointCB);
  BUTTON_screen_hideall = glui_motion->add_button_to_panel(ROLLOUT_screenvis, _("Hide all"), HIDEALL_SCREENS, GLUIViewpointCB);
#endif

  GLUIUpdateFileLabel(render_label_type);

  ROLLOUT_scene_clip = glui_motion->add_rollout_to_panel(ROLLOUT_render, "Clipping region", false, RENDER_CLIP_ROLLOUT, SubRenderRolloutCB);
  TOGGLE_ROLLOUT(subrenderprocinfo,nsubrenderprocinfo,ROLLOUT_scene_clip,RENDER_CLIP_ROLLOUT, glui_motion);

  SPINNER_clip_left = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "left:", GLUI_SPINNER_INT, &render_clip_left);
  SPINNER_clip_left->set_int_limits(0, screenWidth);

  SPINNER_clip_right = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "right:", GLUI_SPINNER_INT, &render_clip_right);
  SPINNER_clip_right->set_int_limits(0, screenWidth);

  SPINNER_clip_bottom = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "bottom:", GLUI_SPINNER_INT, &render_clip_bottom);
  SPINNER_clip_bottom->set_int_limits(0, screenHeight);

  SPINNER_clip_top = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "top:", GLUI_SPINNER_INT, &render_clip_top);
  SPINNER_clip_top->set_int_limits(0, screenHeight);

  CHECKBOX_clip_rendered_scene = glui_motion->add_checkbox_to_panel(ROLLOUT_scene_clip, "clip rendered scene", &clip_rendered_scene);

  if(have_slurm==1){
    ROLLOUT_make_movie = glui_motion->add_rollout("Movie(local)", false, MOVIE_ROLLOUT, MVRRolloutCB);
  }
  else{
    ROLLOUT_make_movie = glui_motion->add_rollout("Movie", false, MOVIE_ROLLOUT, MVRRolloutCB);
  }
  TOGGLE_ROLLOUT(mvrprocinfo,nmvrprocinfo,ROLLOUT_make_movie,MOVIE_ROLLOUT, glui_motion);

  CHECKBOX_overwrite_movie = glui_motion->add_checkbox_to_panel(ROLLOUT_make_movie, "Overwrite movie", &overwrite_movie);
  glui_motion->add_button_to_panel(ROLLOUT_make_movie, _("Render normal"), RENDER_START_NORMAL, RenderCB);
  BUTTON_make_movie = glui_motion->add_button_to_panel(ROLLOUT_make_movie, "Make movie", MAKE_MOVIE, RenderCB);
  BUTTON_play_movie = glui_motion->add_button_to_panel(ROLLOUT_make_movie, "Play movie", PLAY_MOVIE, RenderCB);
  EnableDisablePlayMovie();
  glui_motion->add_separator_to_panel(ROLLOUT_make_movie);

  EDIT_movie_name = glui_motion->add_edittext_to_panel(ROLLOUT_make_movie, "Movie prefix:", GLUI_EDITTEXT_TEXT, movie_name, MOVIE_NAME, RenderCB);
  EDIT_movie_name->set_w(200);
  PANEL_movie_type = glui_motion->add_panel_to_panel(ROLLOUT_make_movie, "Movie type:", true);
  RADIO_movie_type = glui_motion->add_radiogroup_to_panel(PANEL_movie_type, &movie_filetype, MOVIE_FILETYPE, RenderCB);
  RADIOBUTTON_movie_type[0]=glui_motion->add_radiobutton_to_group(RADIO_movie_type, "avi");
  RADIOBUTTON_movie_type[1]=glui_motion->add_radiobutton_to_group(RADIO_movie_type, "mp4");
  RADIOBUTTON_movie_type[2]=glui_motion->add_radiobutton_to_group(RADIO_movie_type, "wmv");
  RADIOBUTTON_movie_type[3]=glui_motion->add_radiobutton_to_group(RADIO_movie_type, "mov");
  SPINNER_framerate = glui_motion->add_spinner_to_panel(ROLLOUT_make_movie, "Frame rate", GLUI_SPINNER_INT, &movie_framerate);
  SPINNER_framerate->set_int_limits(1, 100);
  SPINNER_movie_crf = glui_motion->add_spinner_to_panel(ROLLOUT_make_movie, "quality", GLUI_SPINNER_INT, &movie_crf);
  SPINNER_movie_crf->set_int_limits(0,51);
  SPINNER_bitrate = glui_motion->add_spinner_to_panel(ROLLOUT_make_movie, "Bit rate (Kb/s)", GLUI_SPINNER_INT, &movie_bitrate);
  SPINNER_bitrate->set_int_limits(1, 100000);
  glui_motion->add_button_to_panel(ROLLOUT_make_movie, "Output ffmpeg command", OUTPUT_FFMPEG, RenderCB);
  RenderCB(MOVIE_FILETYPE);

  if(have_slurm==1&&nmovie_queues>0){
    ROLLOUT_make_movie_batch = glui_motion->add_rollout("Movie(cluster)", false, MOVIE_ROLLOUT_BATCH, MVRRolloutCB);
    TOGGLE_ROLLOUT(mvrprocinfo, nmvrprocinfo, ROLLOUT_make_movie_batch, MOVIE_ROLLOUT_BATCH, glui_motion);

    LIST_movie_slice_index = glui_motion->add_listbox_to_panel(ROLLOUT_make_movie_batch, "slice:", &movie_slice_index, MOVIE_SLICE_INDEX, MovieCB);
    for(i = 0; i<nslicemenuinfo; i++){
      const char *cdir[] = {" ", "x=", "y=", "z=", " "};
      slicedata *slicei;
      slicemenudata *slicemi;
      char label[100], *quantity, cposition[25];
      float position;
      int idir;

      slicemi = slicemenu_sorted[i];
      slicei = slicemi->sliceinfo;
      quantity = slicei->label.longlabel;
      idir = CLAMP(slicei->idir, 0, 4);
      position = slicei->position_orig;
      //sprintf(cposition, "%f", position);
      snprintf(cposition, sizeof(cposition),"%f", position);
      TrimZeros(cposition);
      strcpy(label, " ");
      if(idir>=1&&idir<=3){
        strcat(label, quantity);
        strcat(label, ", ");
        strcat(label, cdir[idir]);
        strcat(label, cposition);
      }
      LIST_movie_slice_index->add_item(i, label);
    }
    MovieCB(MOVIE_SLICE_INDEX);

    LIST_movie_queue_index = glui_motion->add_listbox_to_panel(ROLLOUT_make_movie_batch, "queue:", &movie_queue_index);
    for(i = 0; i<nmovie_queues; i++){
      LIST_movie_queue_index->add_item(i, movie_queues[i]);
    }
    SPINNER_movie_nprocs = glui_motion->add_spinner_to_panel(ROLLOUT_make_movie_batch, _("processors"), GLUI_SPINNER_INT, &movie_nprocs);
    SPINNER_movie_nprocs->set_int_limits(1, 36);

    EDITTEXT_movie_email=glui_motion->add_edittext_to_panel(ROLLOUT_make_movie_batch,"email:",GLUI_EDITTEXT_TEXT, movie_email);
    EDITTEXT_movie_email->set_w(300);

    EDITTEXT_movie_htmldir=glui_motion->add_edittext_to_panel(ROLLOUT_make_movie_batch,"html directory:",GLUI_EDITTEXT_TEXT,movie_htmldir);
    EDITTEXT_movie_htmldir->set_w(300);

    EDITTEXT_movie_url = glui_motion->add_edittext_to_panel(ROLLOUT_make_movie_batch, "url:", GLUI_EDITTEXT_TEXT, movie_url);
    EDITTEXT_movie_url->set_w(300);

    BUTTON_make_movie_batch = glui_motion->add_button_to_panel(ROLLOUT_make_movie_batch, "Make movie", MAKE_MOVIE_BATCH, RenderCB);
  }

  PANEL_close = glui_motion->add_panel("",GLUI_PANEL_NONE);

  BUTTON_motion_1=glui_motion->add_button_to_panel(PANEL_close,_("Save settings"),SAVE_SETTINGS_MOTION, MotionDlgCB);

  glui_motion->add_column_to_panel(PANEL_close,false);

  BUTTON_motion_2=glui_motion->add_button_to_panel(PANEL_close,_("Close"),1, MotionDlgCB);
#ifdef pp_CLOSEOFF
  BUTTON_motion_2->disable();
#endif

  GLUIShowHideTranslate(rotation_type);
  glui_motion->set_main_gfx_window( main_window );
}

/* ------------------ GLUIUpdateWindowSizeList ------------------------ */

extern "C" void GLUIUpdateWindowSizeList(void){
  windowsize_pointer=0;
  glui_screenWidth=screenWidth;
  glui_screenHeight=screenHeight;
  if(SPINNER_window_width!=NULL)SPINNER_window_width->set_int_val(screenWidth);
  if(SPINNER_window_height!=NULL)SPINNER_window_height->set_int_val(screenHeight);

  if(screenWidth==320&&screenHeight==240){
    windowsize_pointer=2;
  }
  if(screenWidth==720&&screenHeight==480){
    windowsize_pointer=7;
  }
  if(screenWidth==640&&screenHeight==480){
    windowsize_pointer=3;
  }
  if(screenWidth==800&&screenHeight==640){
    windowsize_pointer=4;
  }
  if(screenWidth==1024&&screenHeight==768){
    windowsize_pointer=5;
  }
  if(screenWidth==1280&&screenHeight==1024){
    windowsize_pointer=6;
  }
  if(screenWidth==1920&&screenHeight==1080){
    windowsize_pointer=8;
  }
  if(screenWidth==1440&&screenHeight==1080){
    windowsize_pointer=10;
  }
  if(screenWidth==1280&&screenHeight==720){
    windowsize_pointer=9;
  }
  if(LIST_windowsize!=NULL)LIST_windowsize->set_int_val(windowsize_pointer);
   windowsize_pointer_old = windowsize_pointer;
}

/* ------------------ GLUIUpdateTranslate ------------------------ */

extern "C" void GLUIUpdateTranslate(void){
  float *eye_xyz;
#ifdef ROTATE_TRANSLATE
  float *az_elev;
#endif

  eye_xyz = camera_current->eye;
#ifdef ROTATE_TRANSLATE
  az_elev = camera_current->az_elev;
#endif

  d_eye_xyz[0]=eye_xyz[0]-eye_xyz0[0];
  d_eye_xyz[1]=eye_xyz[1]-eye_xyz0[1];
  d_eye_xyz[2]=eye_xyz[2]-eye_xyz0[2];

#ifdef ROTATE_TRANSLATE
  if(TRANSLATE_xy != NULL){
    TRANSLATE_xy->set_x(d_eye_xyz[0]);
  }
#endif
  if(rotation_type==ROTATION_1AXIS){
    d_eye_xyz[1]=0.0;
  }
#ifdef ROTATE_TRANSLATE
  if(TRANSLATE_xy != NULL){
    TRANSLATE_xy->set_y(d_eye_xyz[1]);
    TRANSLATE_z->set_y(eye_xyz[2]);
  }
#endif
  if(rotation_type==ROTATION_3AXIS){
  }
  else{
#ifdef ROTATE_TRANSLATE
    ROTATE_2axis->set_x(az_elev[0]);
    ROTATE_2axis->set_y(az_elev[1]);
    ROTATE_eye_z->set_x(camera_current->azimuth);
#endif
  }
  GLUISetPosXYZSMV(camera_current->eye);
}

/* ------------------ GLUIUpdateRotationIndex ------------------------ */

extern "C" void GLUIUpdateRotationIndex(int val){
  int *rotation_index;

  rotation_index = &camera_current->rotation_index;

  *rotation_index=val;
  camera_current->rotation_index=val;
  if(*rotation_index>=0&&*rotation_index<global_scase.meshescoll.nmeshes){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo + *rotation_index;
    camera_current->xcen=meshi->xcen;
    camera_current->ycen=meshi->ycen;
    camera_current->zcen=meshi->zcen;
  }
  else{
    if(custom_worldcenter==0){
      camera_current->xcen=xcenGLOBAL;
      camera_current->ycen=ycenGLOBAL;
      camera_current->zcen=zcenGLOBAL;
    }
    else{
      if(*rotation_index==ROTATE_ABOUT_USER_CENTER){
        camera_current->xcen = xcenCUSTOM;
        camera_current->ycen = ycenCUSTOM;
        camera_current->zcen = zcenCUSTOM;
      }
      else if(*rotation_index==ROTATE_ABOUT_FDS_CENTER){
        camera_current->xcen = FDS2SMV_X((xbar0FDS + xbarFDS)/2.0);
        camera_current->ycen = FDS2SMV_Y((ybar0FDS + ybarFDS)/2.0);
        camera_current->zcen = FDS2SMV_Z((zbar0FDS + zbarFDS)/2.0);
      }
      else{
        camera_current->xcen = ((camera_current->clip_xmin == 1 ? clipinfo.xmin : xbar0ORIG) + (camera_current->clip_xmax == 1 ? clipinfo.xmax : xbarORIG)) / 2.0;
        camera_current->ycen = ((camera_current->clip_ymin == 1 ? clipinfo.ymin : ybar0ORIG) + (camera_current->clip_ymax == 1 ? clipinfo.ymax : ybarORIG)) / 2.0;
        camera_current->zcen = ((camera_current->clip_zmin == 1 ? clipinfo.zmin : zbar0ORIG) + (camera_current->clip_zmax == 1 ? clipinfo.zmax : zbarORIG)) / 2.0;
        camera_current->xcen = FDS2SMV_X(camera_current->xcen);
        camera_current->ycen = FDS2SMV_Y(camera_current->ycen);
        camera_current->zcen = FDS2SMV_Z(camera_current->zcen);
      }
    }
  }

  xcenCUSTOM = camera_current->xcen;
  ycenCUSTOM = camera_current->ycen;
  zcenCUSTOM = camera_current->zcen;
  xcenCUSTOMsmv = SMV2FDS_X(xcenCUSTOM);
  ycenCUSTOMsmv = SMV2FDS_Y(ycenCUSTOM);
  zcenCUSTOMsmv = SMV2FDS_Z(zcenCUSTOM);
  if(SPINNER_xcenCUSTOM!=NULL)SPINNER_xcenCUSTOM->set_float_val(xcenCUSTOMsmv);
  if(SPINNER_ycenCUSTOM!=NULL)SPINNER_ycenCUSTOM->set_float_val(ycenCUSTOMsmv);
  if(SPINNER_zcenCUSTOM!=NULL)SPINNER_zcenCUSTOM->set_float_val(zcenCUSTOMsmv);

  GLUIUpdateMeshList1(val);

  glutPostRedisplay();
}

/* ------------------ GLUIUpdateProjectionType ------------------------ */

extern "C" void GLUIUpdateProjectionType(void){
  if(RADIO_projection!=NULL)RADIO_projection->set_int_val(projection_type);
  if(projection_type==PROJECTION_ORTHOGRAPHIC){
    if(SPINNER_zoom!=NULL)    SPINNER_zoom->disable();
    if(SPINNER_aperture!=NULL)SPINNER_aperture->disable();
  }
  else{
    if(SPINNER_zoom!=NULL)    SPINNER_zoom->enable();
    if(SPINNER_aperture!=NULL)SPINNER_aperture->enable();
  }
}

/* ------------------ GLUIShowHideTranslate ------------------------ */

extern "C" void GLUIShowHideTranslate(int var){
  float *eye_xyz;

  eye_xyz = camera_current->eye;

  eye_xyz0[0]=eye_xyz[0];
  eye_xyz0[1]=eye_xyz[1];
  eye_xyz0[2]=eye_xyz[2];
  d_eye_xyz[0]=0.0;
  d_eye_xyz[1]=0.0;
  switch(var){
  case ROTATION_3AXIS:
#ifdef ROTATE_TRANSLATE
    if(ROTATE_2axis!=NULL)ROTATE_2axis->disable();
    if(ROTATE_eye_z!=NULL)ROTATE_eye_z->disable();
#endif
    if(BUTTON_90_z!=NULL)BUTTON_90_z->disable();
    if(CHECKBOX_blockpath!=NULL)CHECKBOX_blockpath->disable();
    if(PANEL_speed!=NULL)PANEL_speed->disable();
    if(PANEL_height!=NULL)PANEL_height->disable();
    if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
    if(BUTTON_eyelevel!=NULL)BUTTON_eyelevel->disable();
    if(BUTTON_floorlevel!=NULL)BUTTON_floorlevel->disable();
    if(LIST_rotate_about!=NULL)LIST_rotate_about->enable();
    if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->enable();
    if(BUTTON_snap!=NULL)BUTTON_snap->enable();
    break;
  case ROTATION_2AXIS:
#ifdef ROTATE_TRANSLATE
    if(ROTATE_2axis!=NULL)ROTATE_2axis->enable();
    if(ROTATE_eye_z!=NULL)ROTATE_eye_z->disable();
#endif
    if(BUTTON_90_z!=NULL)BUTTON_90_z->disable();
    if(CHECKBOX_blockpath!=NULL)CHECKBOX_blockpath->disable();
    if(PANEL_speed!=NULL)PANEL_speed->disable();
    if(PANEL_height!=NULL)PANEL_height->disable();
    if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
    if(BUTTON_eyelevel!=NULL)BUTTON_eyelevel->disable();
    if(BUTTON_floorlevel!=NULL)BUTTON_floorlevel->disable();
    if(LIST_rotate_about!=NULL)LIST_rotate_about->enable();
    if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->enable();
    if(BUTTON_snap!=NULL)BUTTON_snap->enable();
    break;
  case EYE_CENTERED:
#ifdef ROTATE_TRANSLATE
    if(ROTATE_2axis!=NULL)ROTATE_2axis->disable();
    if(ROTATE_eye_z!=NULL)ROTATE_eye_z->enable();
#endif
    if(BUTTON_90_z!=NULL)BUTTON_90_z->enable();
    if(CHECKBOX_blockpath!=NULL)CHECKBOX_blockpath->enable();
    if(PANEL_speed!=NULL)PANEL_speed->enable();
    if(PANEL_height!=NULL)PANEL_height->enable();
    if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
    if(BUTTON_eyelevel!=NULL)BUTTON_eyelevel->enable();
    if(BUTTON_floorlevel!=NULL)BUTTON_floorlevel->enable();
    if(LIST_rotate_about!=NULL)LIST_rotate_about->disable();
    if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->disable();
    if(BUTTON_snap!=NULL)BUTTON_snap->disable();
    break;
  case ROTATION_1AXIS:
#ifdef ROTATE_TRANSLATE
    if(ROTATE_2axis!=NULL)ROTATE_2axis->enable();
    if(ROTATE_eye_z!=NULL)ROTATE_eye_z->disable();
#endif
    if(BUTTON_90_z!=NULL)BUTTON_90_z->disable();
    if(CHECKBOX_blockpath!=NULL)CHECKBOX_blockpath->disable();
    if(PANEL_speed!=NULL)PANEL_speed->disable();
    if(PANEL_height!=NULL)PANEL_height->disable();
    if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
    if(BUTTON_eyelevel!=NULL)BUTTON_eyelevel->disable();
    if(BUTTON_floorlevel!=NULL)BUTTON_floorlevel->disable();
    if(LIST_rotate_about!=NULL)LIST_rotate_about->enable();
    if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->enable();
    if(BUTTON_snap!=NULL)BUTTON_snap->enable();
    break;
  default:
    assert(FFALSE);
  }

}

/* ------------------ GLUIToggleRollout ------------------------ */

extern "C" void GLUIToggleRollout(procdata *procinfo, int nprocinfo, int motion_id){
  int i;

  if(toggle_dialogs==1){
    for(i=0;i<nprocinfo;i++){
      procdata *mi;

      mi = procinfo + i;
      if(mi->rollout_id!=motion_id)mi->rollout->close();
    }
  }
  for(i=0;i<nprocinfo;i++){
    procdata *mi;

    mi = procinfo + i;
    if(mi->rollout_id==motion_id){
#ifdef pp_OSX
      int h;
#endif

      mi->rollout->open();
#ifdef pp_OSX
      h = mi->dialog->h;
      if(monitor_screen_height!=-1&&h>monitor_screen_height){
        mi->rollout->close();
        printf("*** warning: screen not large enough to expand this dialog\n");
      }
#endif
    }
  }
}

  /* ------------------ GLUISceneMotionCB ------------------------ */

extern "C" void GLUISceneMotionCB(int var){
  float dx, dy;
  float dx2, dy2;
  float *eye_xyz;
  float *azimuth;
  int *rotation_index;
  int i;

  if(var==RESET_VIEW){
    customview_elevation = 0.0;
    customview_azimuth = 0.0;
    SPINNER_customview_azimuth->set_float_val(customview_azimuth);
    SPINNER_customview_elevation->set_float_val(customview_elevation);
    GLUISceneMotionCB(CUSTOM_VIEW);
    return;
  }
  if(var==ROTATE_90){
    customview_azimuth=fmod(customview_azimuth+90.0,360.0);
    SPINNER_customview_azimuth->set_float_val(customview_azimuth);
    GLUISceneMotionCB(CUSTOM_VIEW);
    return;
  }
  if(var==CUSTOM_VIEW){
    if(use_customview==1){
      float *view, *up, *right;
      float c_az, c_elev;

      NewMemory((void **)&screenglobal, sizeof(screendata));
      view = screenglobal->view;
      up = screenglobal->up;
      right = screenglobal->right;

      c_az = -customview_azimuth;
      c_elev = -customview_elevation;
      view[0] = -sin(DEG2RAD*c_az)*cos(DEG2RAD*c_elev);
      view[1] = cos(DEG2RAD*c_az)*cos(DEG2RAD*c_elev);
      view[2] = -sin(DEG2RAD*c_elev);
      up[0] = cos(DEG2RAD*customview_up);
      up[1] = 0.0;
      up[2] = sin(DEG2RAD*customview_up);
      right[0] = sin(DEG2RAD*customview_up);
      right[1] = 0.0;
      right[2] = cos(DEG2RAD*customview_up);
    }
    else{
      FREEMEMORY(screenglobal);
    }
    return;
  }
  if(var == NEARFARCLIP){
    if(nearclip<0.0){
      nearclip=0.00001;
      SPINNER_nearclip->set_float_val(nearclip);
    }
    if(farclip<0.0){
      farclip=0.00001;
      SPINNER_farclip->set_float_val(farclip);
    }
    return;
  }

  if(var==CLIP_SHOW_ROTATE){
    GLUIUpdateShowRotationCenter2();
    return;
  }

#ifdef pp_GPU
  if(usegpu==1&&showvolrender==1&&show_volsmoke_moving==1&&
     (var==EYE_ROTATE||var==EYE_ROTATE_90||var==ROTATE_2AXIS||
      var==TRANSLATE_XY||var==TRANSLATE_X||TRANSLATE_Y||
      var==GLUI_Z)
    ){
    float fps;

    START_TIMER(thisMOTIONtime);
    fps = MOTIONnframes/(thisMOTIONtime-lastMOTIONtime);
    if(fps>GPU_VOLframemax)return;
    MOTIONnframes++;
    if(thisMOTIONtime>lastMOTIONtime+0.25){
      PRINTF("MOTION: %4.1f fps\n",fps);
      lastMOTIONtime=thisMOTIONtime;
      MOTIONnframes=0;
    }
  }
#endif

  if(var==CURSOR){
    updatemenu=1;
    return;
  }
  eye_xyz = camera_current->eye;
  azimuth=&camera_current->azimuth;
  rotation_index = &camera_current->rotation_index;
  if(selected_view!=-999){
    selected_view=-999;
    updatemenu=1;
  }

  switch(var){
    case WINDOW_COLORS:
      foregroundbasecolor[0] = (float)glui_foregroundbasecolor[0] / 255.0;
      foregroundbasecolor[1] = (float)glui_foregroundbasecolor[1] / 255.0;
      foregroundbasecolor[2] = (float)glui_foregroundbasecolor[2] / 255.0;
      backgroundbasecolor[0] = (float)glui_backgroundbasecolor[0] / 255.0;
      backgroundbasecolor[1] = (float)glui_backgroundbasecolor[1] / 255.0;
      backgroundbasecolor[2] = (float)glui_backgroundbasecolor[2] / 255.0;
      ShowHideMenu(MENU_SHOWHIDE_FLIP);
      ShowHideMenu(MENU_SHOWHIDE_FLIP);
      break;
    case COLOR_FLIP:
      ShowHideMenu(MENU_SHOWHIDE_FLIP);
      break;
    case EYE_ROTATE:
      *azimuth=motion_dir[0];
      if(glui_move_mode!=EYE_ROTATE){
        eye_xyz0[0]=eye_xyz[0];
        eye_xyz0[1]=eye_xyz[1];
        GLUIUpdateTranslate();
      }
      glui_move_mode=EYE_ROTATE;
      return;
    case EYE_ROTATE_90:
      {
        float diffangle;
        int intangle;

        intangle = (int)((*azimuth+45)/90)*90;
        diffangle = *azimuth-intangle;
        if(diffangle<0.0)diffangle = -diffangle;
        if(diffangle>1.0){
          *azimuth=intangle;
        }
        else{
          *azimuth+=90.0;
        }
      }
      if(*azimuth>=360.0)*azimuth-=360.0;
      motion_dir[0] = *azimuth;
      GLUISceneMotionCB(EYE_ROTATE);
      glui_move_mode=EYE_ROTATE_90;
      return;
    case ROTATE_2AXIS:
      if(rotation_type==ROTATION_2AXIS){
#ifdef ROTATE_TRANSLATE
        float *az_elev;

        az_elev = camera_current->az_elev;
        az_elev[0] = ROTATE_2axis->get_x();
        az_elev[1] = -ROTATE_2axis->get_y();
#endif
      }
      break;
    case WINDOWSIZE_LIST:
      switch(windowsize_pointer){
        case 0:
        case 1:
          break;
        case 2:
          glui_screenWidth=320;
          glui_screenHeight=240;
          break;
        case 3:
          glui_screenWidth=640;
          glui_screenHeight=480;
          break;
        case 7:
          glui_screenWidth=720;
          glui_screenHeight=480;
          break;
        case 8:
          glui_screenWidth=1920;
          glui_screenHeight=1080;
          break;
        case 9:
          glui_screenWidth=1280;
          glui_screenHeight=720;
          break;
        case 10:
          glui_screenWidth=1440;
          glui_screenHeight=1080;
          break;
        case 4:
          glui_screenWidth=800;
          glui_screenHeight=640;
          break;
        case 5:
          glui_screenWidth=1024;
          glui_screenHeight=768;
          break;
        case 6:
          glui_screenWidth=1280;
          glui_screenHeight=1024;
          break;
        default:
          assert(FFALSE);
          break;
      }
      if(windowsize_pointer>=2){
        if(windowsize_pointer==windowsize_pointer_old)break;
        windowsize_pointer_old = windowsize_pointer;
#ifdef pp_OSX_HIGHRES
        if(double_scale==1){
          glui_screenWidth  /= 2;
          glui_screenHeight /= 2;
        }
#endif
        SetScreenSize(&glui_screenWidth, &glui_screenHeight);
        screenWidth  = glui_screenWidth;
        screenHeight = glui_screenHeight;
        SPINNER_window_width->set_int_val(screenWidth);
        SPINNER_window_height->set_int_val(screenHeight);
        ResizeWindow(screenWidth,screenHeight);
      }
      {
        int width_low, height_low, width_high, height_high;

        GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
        GLUIUpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
      }
      break;
    case SNAPSCENE:
      SnapScene();
      break;
    case WINDOW_PRESERVE:
      if(fix_window_aspect==1){
        float width, height;

        width  = (float)glutGet(GLUT_WINDOW_WIDTH);
        height = (float)glutGet(GLUT_WINDOW_HEIGHT);
        window_aspect = 1.0;
        if(width>0.0&&height>0.0)window_aspect = height/width;
      }
      break;
    case WINDOW_RESIZE_WIDTH:
    case WINDOW_RESIZE_HEIGHT:
      if(fix_window_aspect==1){
        if(var==WINDOW_RESIZE_WIDTH){
          glui_screenHeight = window_aspect*glui_screenWidth;
          SPINNER_window_height->set_int_val(glui_screenHeight);
        }
        else{
          glui_screenWidth = glui_screenHeight/window_aspect;
          SPINNER_window_width->set_int_val(glui_screenWidth);
        }
      }
      GLUISceneMotionCB(WINDOW_RESIZE);
      break;
    case WINDOW_RESIZE:
#ifdef pp_OSX
      glui_screenWidth  /= 2;
      glui_screenHeight /= 2;
#endif
      SetScreenSize(&glui_screenWidth, &glui_screenHeight);
      screenWidth = glui_screenWidth;
      screenHeight = glui_screenHeight;
      SPINNER_window_width->set_int_val(screenWidth);
      SPINNER_window_height->set_int_val(screenHeight);
      GLUIUpdateWindowSizeList();
      ResizeWindow(screenWidth,screenHeight);
      break;
    case PROJECTION:
      ZoomMenu(UPDATE_PROJECTION);
      camera_current->projection_type=projection_type;
      return;
    case EYELEVEL:
      desired_view_height=1.5;
      break;
    case FLOORLEVEL:
      desired_view_height=0.6;
      break;
    case CUSTOM_ROTATION_XYZ:
      xcenCUSTOM = FDS2SMV_X(xcenCUSTOMsmv);
      ycenCUSTOM = FDS2SMV_Y(ycenCUSTOMsmv);
      zcenCUSTOM = FDS2SMV_Z(zcenCUSTOMsmv);
      GLUIUpdateRotationIndex(ROTATE_ABOUT_USER_CENTER);
      break;
    case ROTATE_ABOUT:
      glui_rotation_index = *rotation_index;
      if(*rotation_index==ROTATE_ABOUT_USER_CENTER){
        custom_worldcenter=1;
        SPINNER_xcenCUSTOM->enable();
        SPINNER_ycenCUSTOM->enable();
        SPINNER_zcenCUSTOM->enable();
      }
      else if(*rotation_index==ROTATE_ABOUT_FDS_CENTER){
        custom_worldcenter = 1;
        SPINNER_xcenCUSTOM->disable();
        SPINNER_ycenCUSTOM->disable();
        SPINNER_zcenCUSTOM->disable();
      }
      else if(*rotation_index==ROTATE_ABOUT_CLIPPING_CENTER){
        custom_worldcenter = 1;
        SPINNER_xcenCUSTOM->disable();
        SPINNER_ycenCUSTOM->disable();
        SPINNER_zcenCUSTOM->disable();
      }
      else if(*rotation_index==ROTATE_ABOUT_WORLD_CENTER){
        SPINNER_xcenCUSTOM->disable();
        SPINNER_ycenCUSTOM->disable();
        SPINNER_zcenCUSTOM->disable();
      }
      else{
        custom_worldcenter=0;
        SPINNER_xcenCUSTOM->disable();
        SPINNER_ycenCUSTOM->disable();
        SPINNER_zcenCUSTOM->disable();
      }
      if(*rotation_index>=0&&*rotation_index<global_scase.meshescoll.nmeshes){
        UpdateCurrentMesh(global_scase.meshescoll.meshinfo + (*rotation_index));
        GLUIUpdateRotationIndex(*rotation_index);
      }
      else if(*rotation_index==ROTATE_ABOUT_USER_CENTER){
        GLUIUpdateRotationIndex(ROTATE_ABOUT_USER_CENTER);
      }
      else if(*rotation_index==ROTATE_ABOUT_FDS_CENTER){
        GLUIUpdateRotationIndex(ROTATE_ABOUT_FDS_CENTER);
      }
      else if(*rotation_index==ROTATE_ABOUT_CLIPPING_CENTER){
        GLUIUpdateRotationIndex(ROTATE_ABOUT_CLIPPING_CENTER);
      }
      else if(*rotation_index==ROTATE_ABOUT_WORLD_CENTER){
        UpdateCurrentMesh(global_scase.meshescoll.meshinfo);
        GLUIUpdateRotationIndex(ROTATE_ABOUT_WORLD_CENTER);
      }
      else{
        UpdateCurrentMesh(global_scase.meshescoll.meshinfo);
        GLUIUpdateRotationIndex(ROTATE_ABOUT_WORLD_CENTER);
      }
      update_rotation_center=1;
      return;
    case ZOOM:
      updatemenu = 1;
      zoomindex=-1;
      for(i=0;i<MAX_ZOOMS;i++){
        if(ABS(zoom-zooms[i])<0.001){
          zoomindex=i;
          zoom=zooms[i];
          break;
        }
      }
      if(zoomindex==-1&&zoom>0.0){
        zooms[MAX_ZOOMS] = zoom;
        zoomindex = MAX_ZOOMS;
      }
      camera_current->zoom=zoom;
      aperture_glui=Zoom2Aperture(zoom);
      if(SPINNER_aperture!=NULL)SPINNER_aperture->set_float_val(aperture_glui);
      break;
    case APERTURE:
      zoom=Aperture2Zoom(aperture_glui);
      if(zoom<0.1||zoom>10.0){
        if(zoom<0.1)zoom=0.1;
        if(zoom>10.0)zoom=10.0;
        aperture_glui=Zoom2Aperture(zoom);
        if(SPINNER_aperture!=NULL)SPINNER_aperture->set_float_val(aperture_glui);
      }
      zoomindex=-1;
      for(i=0;i<MAX_ZOOMS+1;i++){
        if(ABS(zoom-zooms[i])<0.001&&zooms[i]>0.0){
          zoomindex=i;
          zoom=zooms[i];
          aperture_glui=Zoom2Aperture(zoom);
          if(SPINNER_aperture!=NULL)SPINNER_aperture->set_float_val(aperture_glui);
          break;
        }
      }
      camera_current->zoom=zoom;
      if(SPINNER_zoom!=NULL)SPINNER_zoom->set_float_val(zoom);
      break;
    case USE_GVEC:
      updatemenu = 1;
      break;
    case ZAXIS_CUSTOM:
    case ZAXIS_UP:
    case SET_VIEW_XYZ:
    case TRANSLATE_XY:
    case TRANSLATE_X:
    case TRANSLATE_Y:
    case GLUI_Z:
      break;
    default:
      assert(FFALSE);
      break;
  }

  dx = d_eye_xyz[0];
  dy = d_eye_xyz[1];
  if(var==EYE_ROTATE){
    dy=motion_dir[1]*TRANSLATE_SPEED*(float)screenWidth/1800.0;
  }
  if(rotation_type==EYE_CENTERED){
    float cs_az, sn_az;

    cs_az = cos((*azimuth)*DEG2RAD);
    sn_az = sin((*azimuth)*DEG2RAD);
    dx2 = cs_az*dx + sn_az*dy;
    dy2 = -sn_az*dx + cs_az*dy;
    dx = dx2;
    dy = dy2;
  }

  if(glui_move_mode==EYE_ROTATE){
    GetNewPos(eye_xyz,dx,dy,0.0,1.0);
    eye_xyz0[0]=eye_xyz[0];
    eye_xyz0[1]=eye_xyz[1];
    eye_xyz0[2]=eye_xyz[2];
  }
  else{
    eye_xyz[0] = eye_xyz0[0] + dx;
    eye_xyz[1] = eye_xyz0[1] + dy;
  }

  switch(var){
    case USE_GVEC:
      {
        float vv[3];
        float *elev, *az;
        float maxvv;

        gvec_down=1;
        GLUISceneMotionCB(ZAXIS_UP);
        gvec_down=1;
        maxvv = MAXABS3(global_scase.gvecphys);
        vv[0] = -global_scase.gvecphys[0]/maxvv;
        vv[1] = -global_scase.gvecphys[1]/maxvv;
        vv[2] = -global_scase.gvecphys[2]/maxvv;
        XYZ2AzElev(vv, zaxis_angles, zaxis_angles+1);
        UpdateZaxisAngles();

        az = zaxis_angles;
        elev = zaxis_angles+1;
        user_zaxis[0] = cos(DEG2RAD*(*az))*cos(DEG2RAD*(*elev));
        user_zaxis[1] = sin(DEG2RAD*(*az))*cos(DEG2RAD*(*elev));
        user_zaxis[2] = sin(DEG2RAD*(*elev));
        if(LIST_viewpoints!=NULL)LIST_viewpoints->set_int_val(EXTERNAL_LIST_ID);
        GLUIViewpointCB(LIST_VIEW);
        updatemenu = 1;
        glutPostRedisplay();
      }
      break;
    case ZAXIS_UP:
      zaxis_angles[0] = 0.0;
      zaxis_angles[1] = 90.0;
      zaxis_angles[2] = 0.0;
      SPINNER_zaxis_angles[0]->set_float_val(zaxis_angles[0]);
      SPINNER_zaxis_angles[1]->set_float_val(zaxis_angles[1]);
      SPINNER_zaxis_angles[2]->set_float_val(zaxis_angles[2]);
      GLUISceneMotionCB(ZAXIS_CUSTOM);
      break;
    case ZAXIS_CUSTOM:
      {
        float *elev, *az;

        az=zaxis_angles;
        elev=zaxis_angles+1;
        user_zaxis[0]=cos(DEG2RAD*(*az))*cos(DEG2RAD*(*elev));
        user_zaxis[1]=sin(DEG2RAD*(*az))*cos(DEG2RAD*(*elev));
        user_zaxis[2]=sin(DEG2RAD*(*elev));
        zaxis_custom=1;
      }
      break;
    case SET_VIEW_XYZ:
      FDS2SMV_XYZ(eye_xyz,glui_xyz_fds);
      eye_xyz0[0]=eye_xyz[0];
      eye_xyz0[1]=eye_xyz[1];
      eye_xyz0[2]=eye_xyz[2];
      GLUIUpdateTranslate();
      break;
    case EYE_ROTATE:
    case TRANSLATE_XY:
    case TRANSLATE_X:
    case TRANSLATE_Y:
      if(glui_move_mode==EYE_ROTATE){
        eye_xyz0[0]=eye_xyz[0];
        eye_xyz0[1]=eye_xyz[1];
      }
#ifdef ROTATE_TRANSLATE
      if(TRANSLATE_xy!=NULL){
        TRANSLATE_xy->set_x(d_eye_xyz[0]);
        TRANSLATE_xy->set_y(d_eye_xyz[1]);
      }
      if(TRANSLATE_x!=NULL){
        TRANSLATE_x->set_x(d_eye_xyz[0]);
      }
      if(TRANSLATE_y!=NULL){
        TRANSLATE_y->set_y(d_eye_xyz[1]);
      }
#endif
      glui_move_mode=TRANSLATE_XY;
      GLUIUpdateTranslate();
      break;
    case GLUI_Z:
      if(glui_move_mode==EYE_ROTATE){
        eye_xyz0[0]=eye_xyz[0];
        eye_xyz0[1]=eye_xyz[1];
      }
      glui_move_mode=GLUI_Z;
      GLUIUpdateTranslate();
      break;
    case APERTURE:
    case ZOOM:
    case FLOORLEVEL:
    case PROJECTION:
    case WINDOW_RESIZE:
    case WINDOW_RESIZE_WIDTH:
    case WINDOW_RESIZE_HEIGHT:
    case WINDOW_PRESERVE:
    case WINDOWSIZE_LIST:
    case SNAPSCENE:
    case CUSTOM_ROTATION_XYZ:
    case ROTATE_2AXIS:
    case WINDOW_COLORS:
    case COLOR_FLIP:
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIUpdateMeshList1 ------------------------ */

extern "C" void GLUIUpdateMeshList1(int val){
  if(LIST_rotate_about==NULL)return;
  LIST_rotate_about->set_int_val(val);
  if(val>=0&&val<global_scase.meshescoll.nmeshes){
    RADIO_rotation_type->set_int_val(ROTATION_2AXIS);
    HandleRotationType(ROTATION_2AXIS);
  }
}

/* ------------------ GLUIHideMotion ------------------------ */

extern "C" void GLUIHideMotion(void){
  GLUICloseRollouts(glui_motion);
}

/* ------------------ GLUIShowMotion ------------------------ */

extern "C" void GLUIShowMotion(int menu_id){
  glui_motion->show();
  if(glui_motion != NULL){
    switch(menu_id){
    case DIALOG_VIEW:
      MVRRolloutCB(VIEW_ROLLOUT);
      MotionRolloutCB(VIEWPOINTS_ROLLOUT);
      break;
    case DIALOG_MOTION:
      MVRRolloutCB(RENDER_ROLLOUT);
#ifdef ROTATE_TRANSLATE
      MotionRolloutCB(TRANSLATEROTATE_ROLLOUT);
#endif
      break;
    case DIALOG_RENDER:
      MVRRolloutCB(RENDER_ROLLOUT);
      break;
    case DIALOG_MOVIE:
      MVRRolloutCB(MOVIE_ROLLOUT);
      break;
    case DIALOG_MOVIE_BATCH:
      MVRRolloutCB(MOVIE_ROLLOUT_BATCH);
      break;
    case DIALOG_WINDOW:
      MVRRolloutCB(VIEW_ROLLOUT);
      MotionRolloutCB(WINDOW_ROLLOUT);
      break;
    case DIALOG_SCALING:
      MVRRolloutCB(VIEW_ROLLOUT);
      MotionRolloutCB(SCALING_ROLLOUT);
      break;
    default:
      assert(FFALSE);
      break;
    }
  }
}

/* ------------------ GLUISetStartupView ------------------------ */

 extern "C" void GLUISetStartupView(void){
  GLUIViewpointCB(MOTION_STARTUP);
}

/* ------------------ GLUIAddListView ------------------------ */

extern "C" void GLUIAddListView(char *label_in){
  int ival;
  char *label;
  cameradata *cam1,*cam2,*cex,*ca;

  // ignore duplicate labels
  if(label_in!=NULL&&strlen(label_in)>0){
    cex = &camera_list_first;
    cex = cex->next;
    cex = cex->next;
    for(ca = cex; ca->next!=NULL; ca = ca->next){
      if(strcmp(ca->name,label_in)==0)return;
    }
  }

  ival=LIST_viewpoints->get_int_val();
  if(ival==-1){
    LIST_viewpoints->set_int_val(0);
    ival=LIST_viewpoints->get_int_val();
  }
  selected_view=ival;
  label=label_in;
  if(label==NULL)label=EDIT_view_label->get_text();
  cex=&camera_list_first;
  cex=cex->next;
  cex=cex->next;
  for(ca=cex;ca->next!=NULL;ca=ca->next){
    if(ca->view_id==ival)break;
  }
  if(ival==ca->view_id){
    cam1=ca;
  }
  else{
    cam1=cex;
  }
  cam2 = InsertCamera(cam1,camera_current,label);
  if(cam2!=NULL){
    LIST_viewpoints->set_int_val(cam2->view_id);
    selected_view=cam2->view_id;
  }
  EnableDisableViews();
}

/* ------------------ GLUIRotationTypeCB ------------------------ */

extern "C" void GLUIRotationTypeCB(int var){
  if(var==ROTATION_3AXIS){
    Camera2Quat(camera_current,quat_general,quat_rotation);
  }
  else{
    camera_current->quat_defined=0;
  }
  HandleRotationType(ROTATION_2AXIS);
}

/* ------------------ GLUIEnable360Zoom ------------------------ */

extern "C" void GLUIEnable360Zoom(void){
  if(disable_reshape==1){
    SPINNER_window_height360->enable();
    disable_reshape=0;
  }
}

/* ------------------ Disable360Zoom ------------------------ */

void Disable360Zoom(void){
  SPINNER_window_height360->disable();
  disable_reshape=1;
}

/* ------------------ RenderCB ------------------------ */

void RenderCB(int var){
  char widthlabel[1024];

  updatemenu=1;
  switch(var){
    case RENDER_360CB:
      nwidth360 = nheight360*2;
      //sprintf(widthlabel,"width: %i",nwidth360);
      snprintf(widthlabel,sizeof(widthlabel),"width: %i",nwidth360);
      {
        int width_low, height_low, width_high, height_high;

        GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
        if(width_low>2){
          GLUIUpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
        }
      }
      break;
    case MOVIE_NAME:
      EnableDisablePlayMovie();
      break;
    case PLAY_MOVIE:
      if(playmovie_threads == NULL){
        playmovie_threads = THREADinit(&n_playmovie_threads, &use_playmovie_threads, PlayMovie);
      }
      THREADrun(playmovie_threads);
      break;
    case OUTPUT_FFMPEG:
      output_ffmpeg_command=1;
      break;
    case MAKE_MOVIE_BATCH:
      GLUIViewpointCB(REPLACE_CURRENT_VIEW);
      ResetMenu(SAVE_CURRENT_VIEWPOINT);
      WriteIni(SCRIPT_INI, movie_ini_filename);
      MakeMovieSMVScript();
      MakeMovieBashScript();
      break;
    case MAKE_MOVIE:
      HandleMakeMovie();
      break;
    case RENDER_SKIP:
    case RENDER_LABEL:
    case RENDER_TYPE:
      break;
    case RENDER_HTML: {
      char *html_filename = CasePathHtml(&global_scase);
      Smv2Html(html_filename, HTML_CURRENT_TIME, FROM_SMOKEVIEW);
      FREEMEMORY(html_filename);
    } break;
#ifdef pp_RENDER360_DEBUG
    case RENDER_DEBUG_360:
      if(debug_360_skip_x<2){
        debug_360_skip_x = 2;
        SPINNER_360_skip_x->set_int_val(2);
      }
      if(debug_360_skip_y<2){
        debug_360_skip_y = 2;
        SPINNER_360_skip_y->set_int_val(2);
      }
      break;
#endif
    case RENDER_MULTIPLIER:
      {
        int width_low, height_low, width_high, height_high;

        GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
        GLUIUpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
      }
      break;
    case MOVIE_FILETYPE:
      switch(movie_filetype){
      case AVI:
        strcpy(movie_ext, ".avi");
        SPINNER_movie_crf->disable();
        SPINNER_bitrate->enable();
        break;
      case MP4:
        strcpy(movie_ext, ".mp4");
        SPINNER_movie_crf->enable();
        SPINNER_bitrate->disable();
        break;
      case WMV:
        strcpy(movie_ext, ".wmv");
        SPINNER_movie_crf->disable();
        SPINNER_bitrate->enable();
        break;
      case MOV:
        strcpy(movie_ext, ".mov");
        SPINNER_movie_crf->enable();
        SPINNER_bitrate->disable();
        break;
      default:
        assert(FFALSE);
        break;
      }
      break;
    case RENDER_MODE:
      break;
    case RENDER_RESOLUTION:
      RenderMenu(render_size_index);
      {
        int width_low, height_low, width_high, height_high;

        GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
        GLUIUpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
      }
      break;
    case RENDER_START_360:
      resolution_multiplier = 1;
      render_mode=RENDER_360;
      if(render_skip == RENDER_CURRENT_SINGLE){
        UpdateFrameNumber(0);
      }
      if(glui_screenWidth != glui_screenHeight){
        glui_screenWidth = MAX(glui_screenWidth, glui_screenHeight);
        glui_screenHeight = MAX(glui_screenWidth, glui_screenHeight);
        GLUISceneMotionCB(WINDOW_RESIZE);
      }
      Disable360Zoom();
      RenderMenu(RENDER_CURRENT_360);
      if(render_skip != RENDER_CURRENT_SINGLE){
        Keyboard('0', FROM_SMOKEVIEW);
      }
      {
        int width_low, height_low, width_high, height_high;

        GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
        GLUIUpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
      }
      break;
    case RENDER_START_TOP:
      switch(render_resolution){
      case RENDER_RESOLUTION_320x240:
        resolution_multiplier = 1;
        render_size_index=Render320;
        RenderCB(RENDER_RESOLUTION);
        RenderCB(RENDER_START_NORMAL);
        break;
      case RENDER_RESOLUTION_640x480:
        resolution_multiplier = 1;
        render_size_index=Render640;
        RenderCB(RENDER_RESOLUTION);
        RenderCB(RENDER_START_NORMAL);
        break;
      case RENDER_RESOLUTION_CURRENT:
        resolution_multiplier = 1;
        render_size_index=RenderWindow;
        RenderCB(RENDER_RESOLUTION);
        RenderCB(RENDER_START_NORMAL);
        break;
      case RENDER_RESOLUTION_HIGH:
        resolution_multiplier = glui_resolution_multiplier;
        render_size_index=RenderWindow;
        RenderCB(RENDER_RESOLUTION);
        RenderCB(RENDER_START_HIGHRES);
        break;
      case RENDER_RESOLUTION_360:
        render_size_index=RenderWindow;
        resolution_multiplier = 1;
        RenderCB(RENDER_RESOLUTION);
        RenderCB(RENDER_START_360);
        break;
      default:
	assert(FFALSE);
	break;
      }
    break;
    case RENDER_START_HIGHRES:
      RenderMenu(RenderStartHIGHRES);
      break;
    case RENDER_START_NORMAL:
      RenderMenu(RenderStartORIGRES);
      break;
    case RENDER_START:
      if(render_mode==RENDER_360){
        RenderMenu(RenderStart360);
      }
      else{
        if(resolution_multiplier==1){
          RenderMenu(RenderStartORIGRES);
        }
        else{
          RenderMenu(RenderStartHIGHRES);
        }
      }
      break;
    case RENDER_STOP:
      RenderMenu(RenderCancel);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ UpdateRenderListSkip ------------------------ */

void UpdateRenderListSkip(void){
  if(LIST_render_skip!=NULL){
    int i;

    for(i = 0; i<NRENDER_SKIPS; i++){
      if(render_skip==render_skips[i]){
        LIST_render_skip->set_int_val(render_skip);
        break;
      }
    }
  }
}

/* ------------------ GLUIUpdateRender ------------------------ */

extern "C" void GLUIUpdateRender(void){
  if(RenderTime==1&&RenderTimeOld==0){
    if(LIST_render_skip!=NULL&&render_skip==RENDER_CURRENT_SINGLE){
      render_skip=1;
      LIST_render_skip->set_int_val(render_skip);
    }
  }
  RenderTimeOld=RenderTime;
}
