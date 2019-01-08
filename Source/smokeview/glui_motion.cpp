#define CPP
#include "options.h"

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"

#define TRANSLATE_XY 101
#define ROTATE_2AXIS 102
#define GLUI_Z         2
#define MESH_LIST      4
#define EYE_ROTATE     5
#define EYE_ROTATE_90  6
#define EYELEVEL       7
#define FLOORLEVEL     8
#define ROTATE_90    103
#define RESET_VIEW   104

#define LABEL_VIEW 4

#define CUSTOM_ROTATION_X 9
#define CUSTOM_ROTATION_Y 10
#define CUSTOM_ROTATION_Z 11
#define LIST_VIEW 5
#define ADD_VIEW 6
#define DELETE_VIEW 7
#define RESTORE_VIEW 8
#define REPLACE_VIEW 9
#define STARTUP 10
#define CYCLEVIEWS 11
#define ZOOM 12
#define APERTURE 15
#define CURSOR 13
#define SAVE_SETTINGS 14
#define WINDOW_RESIZE 16
#define WINDOWSIZE_LIST 17
#define SNAPSCENE 21
#define SET_VIEW_XYZ 22
#define ZAXIS_CUSTOM 25
#define USE_GVEC 28
#define GSLICE_TRANSLATE 24
#define GSLICE_NORMAL 27
#define PLAY_MOVIE 29
#define MOVIE_NAME 30
#define CLOSE_MOTION 1
#ifdef pp_RENDER360_DEBUG
#define SHOWALL_SCREENS 31
#define HIDEALL_SCREENS 32
#endif
#define WINDOW_COLORS 33
#define COLOR_FLIP 34
#define CLIP_SHOW_ROTATE 35
#define ZAXIS_UP 41
#define NEARFARCLIP 42
#define CUSTOM_VIEW 43

#define RENDER_TYPE 0
#define RENDER_RESOLUTION 1
#define RENDER_SKIP 2
#define RENDER_START_HIGHRES 11
#define RENDER_STOP 4
#define RENDER_LABEL 5
#define RENDER_MULTIPLIER 6
#define MOVIE_FILETYPE 7
#define RENDER_MODE 8
#define RENDER_START_TOP 13
#define RENDER_DEBUG_360 14

#define SLICE_ROLLOUT 0
#define VIEWPOINTS_ROLLOUT 1
#define WINDOW_ROLLOUT 2
#define SCALING_ROLLOUT 3
#define RENDER_ROLLOUT 4
#define TRANSLATEROTATE_ROLLOUT 5
#define ROTATION_ROLLOUT 6
#define ORIENTATION_ROLLOUT 7
#define MOVIE_ROLLOUT 8
#define VIEW_ROLLOUT 9

#define RENDER_360CB 9

#ifdef pp_DEG
unsigned char deg360[] = { '3','6','0',DEG_SYMBOL,0 };
unsigned char deg90[] = {'9', '0', DEG_SYMBOL, 0};
#else
unsigned char deg360[] = { '3','6','0',0 };
unsigned char deg90[] = {'9', '0', 0};
#endif

GLUI *glui_motion=NULL;

GLUI_Panel *PANEL_360 = NULL;
GLUI_Panel *PANEL_360_debug = NULL;
GLUI_Panel *PANEL_custom_view=NULL;
GLUI_Panel *PANEL_render_file = NULL;
GLUI_Panel *PANEL_render_format = NULL;
GLUI_Panel *PANEL_movie_type = NULL;
GLUI_Panel *PANEL_motion = NULL;
GLUI_Panel *PANEL_viewA = NULL;
GLUI_Panel *PANEL_user_center = NULL;
GLUI_Panel *PANEL_rotate=NULL, *PANEL_translate=NULL,*PANEL_close=NULL;
GLUI_Panel *PANEL_file_suffix=NULL, *PANEL_file_type=NULL;
GLUI_Panel *PANEL_radiorotate=NULL;
GLUI_Panel *PANEL_gslice_center=NULL;
GLUI_Panel *PANEL_gslice_normal=NULL;
GLUI_Panel *PANEL_gslice_show=NULL;
GLUI_Panel *PANEL_speed=NULL;
GLUI_Panel *PANEL_height=NULL;
GLUI_Panel *PANEL_translate2=NULL,*PANEL_translate3=NULL;
GLUI_Panel *PANEL_anglebuttons=NULL;
GLUI_Panel *PANEL_reset1=NULL;
GLUI_Panel *PANEL_reset2=NULL;
GLUI_Rollout *ROLLOUT_scale=NULL;
GLUI_Panel *PANEL_reset=NULL;
GLUI_Panel *PANEL_specify=NULL;
GLUI_Panel *PANEL_change_zaxis=NULL;
GLUI_Panel *PANEL_colors=NULL;

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
GLUI_Rollout *ROLLOUT_gslice = NULL;
GLUI_Rollout *ROLLOUT_translaterotate=NULL;
#ifdef pp_RENDER360_DEBUG
GLUI_Rollout *ROLLOUT_screenvis = NULL;
GLUI_Rollout *ROLLOUT_lower = NULL;
GLUI_Rollout *ROLLOUT_middle = NULL;
GLUI_Rollout *ROLLOUT_upper = NULL;
#endif
GLUI_Rollout *ROLLOUT_background = NULL;
GLUI_Rollout *ROLLOUT_foreground = NULL;

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
GLUI_Spinner *SPINNER_set_view_x=NULL;
GLUI_Spinner *SPINNER_set_view_y=NULL;
GLUI_Spinner *SPINNER_set_view_z=NULL;
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
GLUI_Checkbox *CHECKBOX_blockpath=NULL,*CHECKBOX_cursor_blockpath=NULL;
GLUI_Checkbox *CHECKBOX_gslice_data=NULL;
GLUI_Checkbox *CHECKBOX_showgravity_vector=NULL;
GLUI_Checkbox *CHECKBOX_overwrite_movie = NULL;
#ifdef pp_RENDER360_DEBUG
GLUI_Checkbox **CHECKBOX_screenvis = NULL;
#endif

GLUI_Translation *ROTATE_2axis=NULL,*ROTATE_eye_z=NULL;
GLUI_Translation *TRANSLATE_z=NULL,*TRANSLATE_xy=NULL;

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

GLUI_Button *BUTTON_rotate90=NULL;
GLUI_Button *BUTTON_90_z=NULL,*BUTTON_eyelevel=NULL, *BUTTON_floorlevel=NULL, *BUTTON_reset_saved_view=NULL;
GLUI_Button *BUTTON_replace_view=NULL,*BUTTON_add_view=NULL,*BUTTON_delete_view=NULL;
GLUI_Button *BUTTON_startup=NULL,*BUTTON_cycle_views=NULL;
GLUI_Button *BUTTON_snap=NULL;
GLUI_Button *BUTTON_render_start=NULL ;
GLUI_Button *BUTTON_motion_1=NULL;
GLUI_Button *BUTTON_motion_2=NULL;
GLUI_Button *BUTTON_window_update=NULL;
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

GLUI_Listbox *LIST_viewpoints=NULL;
GLUI_Listbox *LIST_windowsize=NULL;
GLUI_Listbox *LIST_mesh2=NULL;
GLUI_Listbox *LIST_render_skip=NULL;

procdata motionprocinfo[10];
int nmotionprocinfo = 0;

/* ------------------ UpdateRenderRadioButtons ------------------------ */

extern "C" void UpdateRenderRadioButtons(int width_low, int height_low, int width_high, int height_high){
  char label[1024];

  if(width_low > 2){
    sprintf(label, "%ix%i (current)", width_low, height_low);
    if(RADIOBUTTON_render_current != NULL)RADIOBUTTON_render_current->set_name(label);

    sprintf(label, "%ix%i ( %i x current)", width_high, height_high, glui_resolution_multiplier);
    if(RADIOBUTTON_render_high != NULL)RADIOBUTTON_render_high->set_name(label);
  }

  if(nwidth360 > 2){
    sprintf(label, "%s %ix%i", deg360, nwidth360, nheight360);
    if(RADIOBUTTON_render_360 != NULL)RADIOBUTTON_render_360->set_name(label);
  }
}

/* ------------------ UpdatePosView ------------------------ */

extern "C" void UpdatePosView(void){
  SPINNER_set_view_x->set_float_val(set_view_xyz[0]);
  SPINNER_set_view_y->set_float_val(set_view_xyz[1]);
  SPINNER_set_view_z->set_float_val(set_view_xyz[2]);
  SPINNER_customview_azimuth->set_float_val(customview_azimuth);
  SPINNER_customview_elevation->set_float_val(customview_elevation);
  CHECKBOX_use_customview->set_int_val(use_customview);
}

/* ------------------ UpdateZAxisCustom ------------------------ */

extern "C" void UpdateZAxisCustom(void){
  update_zaxis_custom = 0;
  SceneMotionCB(ZAXIS_CUSTOM);
}

/* ------------------ UpdateShowRotationCenter ------------------------ */

extern "C" void UpdateShowRotationCenter(void){
  if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->set_int_val(show_rotation_center);
}

/* ------------------ UpdateGluiRotateAbout ------------------------ */

void UpdateGluiRotateAbout(int val){
  if(LIST_mesh2 != NULL)LIST_mesh2->set_int_val(val);
  SceneMotionCB(MESH_LIST);
}

/* ------------------ UpdateShowGravityVector ------------------------ */

extern "C" void UpdateShowGravityVector(void){
  if(CHECKBOX_showgravity_vector!=NULL)CHECKBOX_showgravity_vector->set_int_val(showgravity_vector);
}

/* ------------------ SetColorControls ------------------------ */

extern "C" void SetColorControls(void){
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

/* ------------------ MotionRolloutCB ------------------------ */

void MotionRolloutCB(int var){
  ToggleRollout(motionprocinfo, nmotionprocinfo, var);
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

/* ------------------ EnableDisablePlayMovie ------------------------ */

void EnableDisablePlayMovie(void){
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

void EnableDisableMakeMovie(int onoff){
  if(onoff == ON){
    if(BUTTON_make_movie != NULL)BUTTON_make_movie->enable();
    if(BUTTON_play_movie != NULL)BUTTON_play_movie->enable();
  }
  else{
    if(BUTTON_make_movie != NULL)BUTTON_make_movie->enable();
    if(BUTTON_play_movie != NULL)BUTTON_play_movie->disable();
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

/* ------------------ UpdateResolutionMultiplier ------------------------ */

extern "C" void UpdateResolutionMultiplier(void){
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
    gslice_norm[1] = sin(az)*cos(elev);;
    gslice_norm[2] = sin(elev);
    break;
  case GSLICE_TRANSLATE:
    gslice_xyz[0] = CLAMP(gslice_xyz[0], xbar0, DENORMALIZE_X(xbar));
    gslice_xyz[1] = CLAMP(gslice_xyz[1], ybar0, DENORMALIZE_Y(ybar));
    gslice_xyz[2] = CLAMP(gslice_xyz[2], zbar0, DENORMALIZE_Z(zbar));
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ UpdateGsliceParms ------------------------ */

extern "C" void UpdateGsliceParms(void){
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


/* ------------------ UpdateRotationType ------------------------ */

extern "C" void UpdateRotationType(int val){
  if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
}

/* ------------------ UpdateGluiSetViewXYZ ------------------------ */

extern "C" void UpdateGluiSetViewXYZ(float *xyz){
  if(xyz==NULL)return;
  if(SPINNER_set_view_x==NULL||SPINNER_set_view_y==NULL||SPINNER_set_view_z==NULL)return;

  DENORMALIZE_XYZ(set_view_xyz,xyz);

  SPINNER_set_view_x->set_float_val(set_view_xyz[0]);
  SPINNER_set_view_y->set_float_val(set_view_xyz[1]);
  SPINNER_set_view_z->set_float_val(set_view_xyz[2]);
}

/* ------------------ GluiIdle ------------------------ */

extern "C" void GluiIdle(void){
  GLUI_Master.set_glutIdleFunc(IdleCB);
}

/* ------------------ GluiIdleNULL ------------------------ */

extern "C" void GluiIdleNULL(void){
  GLUI_Master.set_glutIdleFunc(NULL);
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
      sprintf(viewlabel, "view %i", i);
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
  int ival;
  cameradata *cex;

  ival = LIST_viewpoints->get_int_val();
  if(ival >= 0){

    selected_view = ival;

    cex = &camera_list_first;
    cex = cex->next;
    cex = cex->next;
    cex = cex->next;
    if(cex->next == NULL){
      BUTTON_cycle_views->disable();
    }
    else{
      BUTTON_cycle_views->enable();
    }
  }

  switch(ival){
  case -1:
  case 0:
  case 1:
    BUTTON_replace_view->disable();
    BUTTON_delete_view->disable();
    break;
  default:
    BUTTON_replace_view->enable();
    BUTTON_delete_view->enable();
    break;
  }
}

/* ------------------ ViewpointCB ------------------------ */

void ViewpointCB(int var){
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
  case RESTORE_INTERIOR_VIEW:
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
    if(ival == ca->view_id){
      cam1 = ca;
    }
    else{
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
    AddListView(NULL);
    ViewpointCB(LIST_VIEW);
    break;
  case DELETE_VIEW:
    ival = LIST_viewpoints->get_int_val();
    label = EDIT_view_label->get_text();
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
    ViewpointCB(RESTORE_VIEW);
    EnableDisableViews();
    break;
  case RESTORE_VIEW:
    ival = LIST_viewpoints->get_int_val();
    selected_view = ival;
    for(ca = camera_list_first.next;ca->next != NULL;ca = ca->next){
      if(ca->view_id == ival)break;
    }

    rotation_type_save = ca->rotation_type;
    CopyCamera(camera_current, ca);
    if(rotation_type == ROTATION_3AXIS)Camera2Quat(camera_current, quat_general, quat_rotation);
    if(strcmp(ca->name, "external") == 0 || strcmp(ca->name, "internal") == 0)updatezoommenu = 1;
    camera_current->rotation_type = rotation_type_save;
    EDIT_view_label->set_text(ca->name);
    break;
  case LIST_VIEW:
    ival = LIST_viewpoints->get_int_val();
    old_listview = -2;
    if(ival == -1 && delete_view_is_disabled == 0){
      BUTTON_delete_view->disable();
      delete_view_is_disabled = 1;
      break;
    }
    else{
      if(delete_view_is_disabled == 1){
        BUTTON_delete_view->enable();
        delete_view_is_disabled = 0;
      }
    }
    ViewpointCB(RESTORE_VIEW);
    updatezoommenu = 1;
    EnableDisableViews();
    break;
  case STARTUP:
    startup_view_ini = LIST_viewpoints->get_int_val();
    {
      char *cam_label;

      cam_label = GetCameraLabel(startup_view_ini);
      if(cam_label != NULL){
        strcpy(startup_view_label, cam_label);
      }
    }
    selected_view = startup_view_ini;
    WriteIni(LOCAL_INI, NULL);
    break;
  case CYCLEVIEWS:
    ival = LIST_viewpoints->get_int_val();
    selected_view = ival;
    cex = &camera_list_first;
    cex = cex->next;
    cex = cex->next;
    switch(ival){
    case -1:
    case 0:
    case 1:
      cex = cex->next;
      if(cex->next == NULL)return;
      ival = cex->view_id;
      break;
    default:
      for(ca = cex;ca->next != NULL;ca = ca->next){
        if(ca->view_id == ival)break;
      }
      cex = ca->next;
      if(cex->next == NULL){
        cex = &camera_list_first;
        cex = cex->next;
        cex = cex->next;
        cex = cex->next;
        if(cex->next == NULL)return;
        ival = cex->view_id;
      }
      else{
        ival = cex->view_id;
      }
      break;
    }
    LIST_viewpoints->set_int_val(ival);
    selected_view = ival;
    ViewpointCB(RESTORE_VIEW);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ ResetGluiView ------------------------ */

extern "C" void ResetGluiView(int ival){
  ASSERT(ival>=0);
#ifdef pp_LUA
  LIST_viewpoints->set_int_val(ival);
#else
  if(ival!=old_listview)LIST_viewpoints->set_int_val(ival);
#endif
  selected_view=ival;
  BUTTON_replace_view->enable();
  ViewpointCB(RESTORE_VIEW);
  EnableDisableViews();
}

/* ------------------ EnableResetSavedView ------------------------ */

extern "C" void EnableResetSavedView(void){
  if(BUTTON_reset_saved_view!=NULL)BUTTON_reset_saved_view->enable();
}


/* ------------------ UpdateGluiFileLabel ------------------------ */

extern "C" void UpdateGluiFileLabel(int var){
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

/* ------------------ UpdateGluiZoom ------------------------ */

extern "C" void UpdateGluiZoom(void){
  if(SPINNER_zoom!=NULL)SPINNER_zoom->set_float_val(zoom);
  aperture_glui=Zoom2Aperture(zoom);
  if(SPINNER_aperture!=NULL)SPINNER_aperture->set_float_val(aperture_glui);
}

/* ------------------ UpdateCameraLabel ------------------------ */

extern "C" void UpdateCameraLabel(void){
  EDIT_view_label->set_text(camera_label);
}

/* ------------------ UpdateCursorCheckbox ------------------------ */

extern "C" void UpdateCursorCheckbox(void){
  CHECKBOX_cursor_blockpath->set_int_val(cursorPlot3D);
}

/* ------------------ UpdateGluiCameraViewList ------------------------ */

extern "C" void UpdateGluiCameraViewList(void){
  cameradata *ca;
  int i;

  if(LIST_viewpoints == NULL)return;
  for(ca=camera_list_first.next;ca->next!=NULL;ca=ca->next){
    LIST_viewpoints->delete_item(ca->name);
  }
  SortCameras();
  for(i = 0; i < ncameras_sorted;i++){
    ca = cameras_sorted[i];
    LIST_viewpoints->add_item(ca->view_id, ca->name);
  }
  LIST_viewpoints->set_int_val(startup_view_ini);
  selected_view=startup_view_ini;
  EnableDisableViews();
  ViewpointCB(RESTORE_VIEW);
}

/* ------------------ MotionDlgCB ------------------------ */

void MotionDlgCB(int var){
  switch(var){
  case CLOSE_MOTION:
    if(glui_motion != NULL)glui_motion->hide();
    updatemenu = 1;
    break;
  case SAVE_SETTINGS:
    updatemenu = 1;
    WriteIni(LOCAL_INI, NULL);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ GluiMotionSetup ------------------------ */

extern "C" void GluiMotionSetup(int main_window){
  int i;
#define TRANSLATE_SPEED 0.005
  int *rotation_index;
  float *eye_xyz;

  if(camera_label!=NULL){
    FREEMEMORY(camera_label);
  }
  NewMemory((void **)&camera_label,sizeof(GLUI_String));

  strcpy(camera_label,"current");

  eye_xyz=camera_current->eye;

  update_glui_motion=0;
  if(glui_motion!=NULL){
    glui_motion->close();
    glui_motion=NULL;
  }
  glui_motion = GLUI_Master.create_glui(_("Motion/View/Render"),0,0,0);
  glui_motion->hide();

  PANEL_motion = glui_motion->add_panel("Motion",true);

  ROLLOUT_translaterotate=glui_motion->add_rollout_to_panel(PANEL_motion, _("Translate/Rotate"), true, TRANSLATEROTATE_ROLLOUT, MotionRolloutCB);
  ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_translaterotate, TRANSLATEROTATE_ROLLOUT);

  PANEL_translate2 = glui_motion->add_panel_to_panel(ROLLOUT_translaterotate,_("Translate"));
  d_eye_xyz[0]=0.0;
  d_eye_xyz[1]=0.0;
  d_eye_xyz[2]=0.0;
  dsave_eye_xyz[0]=0.0;
  dsave_eye_xyz[1]=0.0;
  dsave_eye_xyz[2]=0.0;

  TRANSLATE_xy=glui_motion->add_translation_to_panel(PANEL_translate2,_("Horizontal"),GLUI_TRANSLATION_XY,d_eye_xyz,TRANSLATE_XY,SceneMotionCB);
  TRANSLATE_xy->set_speed(TRANSLATE_SPEED);

  glui_motion->add_column_to_panel(PANEL_translate2,false);

  TRANSLATE_z=glui_motion->add_translation_to_panel(PANEL_translate2,_("Vertical"),GLUI_TRANSLATION_Y,eye_xyz+2,GLUI_Z,SceneMotionCB);
  TRANSLATE_z->set_speed(TRANSLATE_SPEED);

  PANEL_rotate = glui_motion->add_panel_to_panel(ROLLOUT_translaterotate,_("Rotate"));

  ROTATE_2axis=glui_motion->add_translation_to_panel(PANEL_rotate,_("2 axis"),GLUI_TRANSLATION_XY,motion_ab,ROTATE_2AXIS,SceneMotionCB);
  glui_motion->add_column_to_panel(PANEL_rotate,false);

  ROTATE_eye_z=glui_motion->add_translation_to_panel(PANEL_rotate,_("View"),GLUI_TRANSLATION_X,motion_dir,EYE_ROTATE,SceneMotionCB);
  ROTATE_eye_z->set_speed(180.0/(float)screenWidth);
  ROTATE_eye_z->disable();

  ROLLOUT_view = glui_motion->add_rollout_to_panel(PANEL_motion, _("Position/View"), false, VIEW_ROLLOUT, MotionRolloutCB);
  ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_view, VIEW_ROLLOUT);

  PANEL_specify = glui_motion->add_panel_to_panel(ROLLOUT_view, _("Position"));

  SPINNER_set_view_x = glui_motion->add_spinner_to_panel(PANEL_specify, "x:", GLUI_SPINNER_FLOAT,   set_view_xyz, SET_VIEW_XYZ, SceneMotionCB);
  SPINNER_set_view_y = glui_motion->add_spinner_to_panel(PANEL_specify, "y:", GLUI_SPINNER_FLOAT, set_view_xyz+1, SET_VIEW_XYZ, SceneMotionCB);
  SPINNER_set_view_z = glui_motion->add_spinner_to_panel(PANEL_specify, "z:", GLUI_SPINNER_FLOAT, set_view_xyz+2, SET_VIEW_XYZ, SceneMotionCB);

  PANEL_custom_view = glui_motion->add_panel_to_panel(ROLLOUT_view, _("View"));

  CHECKBOX_use_customview = glui_motion->add_checkbox_to_panel(PANEL_custom_view,_("Use azimuth/elevation"),&use_customview, CUSTOM_VIEW, SceneMotionCB);
  SPINNER_customview_azimuth = glui_motion->add_spinner_to_panel(PANEL_custom_view,"azimuth:",GLUI_SPINNER_FLOAT,&customview_azimuth,CUSTOM_VIEW,SceneMotionCB);
  SPINNER_customview_elevation = glui_motion->add_spinner_to_panel(PANEL_custom_view,"elevation:", GLUI_SPINNER_FLOAT, &customview_elevation, CUSTOM_VIEW, SceneMotionCB);
  //glui_motion->add_spinner_to_panel(PANEL_custom_view,"     up:", GLUI_SPINNER_FLOAT, &customview_up, CUSTOM_VIEW, SceneMotionCB);
  {
    char rotate_label[100];

    sprintf(rotate_label,"%s rotate",deg90);
    BUTTON_rotate90 = glui_motion->add_button_to_panel(PANEL_custom_view, rotate_label, ROTATE_90, SceneMotionCB);
  }
  glui_motion->add_button_to_panel(PANEL_custom_view, "Reset", RESET_VIEW, SceneMotionCB);

  ROLLOUT_rotation_type = glui_motion->add_rollout_to_panel(PANEL_motion,_("Specify Rotation"),false,ROTATION_ROLLOUT,MotionRolloutCB);
  ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_rotation_type, ROTATION_ROLLOUT);

  PANEL_radiorotate = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type, "Rotation type:");
  RADIO_rotation_type=glui_motion->add_radiogroup_to_panel(PANEL_radiorotate,&rotation_type,0, RotationTypeCB);
  RADIOBUTTON_1c=glui_motion->add_radiobutton_to_group(RADIO_rotation_type,_("2 axis"));
  RADIOBUTTON_1d=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("eye centered"));
  RADIOBUTTON_1e=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("level (1 axis)"));
  RADIOBUTTON_1e=glui_motion->add_radiobutton_to_group(RADIO_rotation_type, _("3 axis"));
  RotationTypeCB(rotation_type);
  rotation_index=&camera_current->rotation_index;
  *rotation_index=glui_rotation_index_ini;

  LIST_mesh2 = glui_motion->add_listbox_to_panel(ROLLOUT_rotation_type,_("Rotate about:"),rotation_index,MESH_LIST,SceneMotionCB);
  LIST_mesh2->add_item(ROTATE_ABOUT_CLIPPING_CENTER, _("center of clipping planes"));
  LIST_mesh2->add_item(ROTATE_ABOUT_USER_CENTER,_("user specified center"));
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    LIST_mesh2->add_item(i,meshi->label);
  }
  LIST_mesh2->add_item(nmeshes,_("world center"));
  LIST_mesh2->set_int_val(*rotation_index);

  PANEL_user_center = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type, "rotation center");
  CHECKBOX_show_rotation_center=glui_motion->add_checkbox_to_panel(PANEL_user_center,_("Show"),&show_rotation_center, CLIP_SHOW_ROTATE, SceneMotionCB);
  xcenCUSTOMsmv = DENORMALIZE_X(xcenCUSTOM);
  ycenCUSTOMsmv = DENORMALIZE_Y(ycenCUSTOM);
  zcenCUSTOMsmv = DENORMALIZE_Z(zcenCUSTOM);
  SPINNER_xcenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"x:",GLUI_SPINNER_FLOAT,&xcenCUSTOMsmv,CUSTOM_ROTATION_X,SceneMotionCB);
  SPINNER_ycenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"y:",GLUI_SPINNER_FLOAT,&ycenCUSTOMsmv,CUSTOM_ROTATION_Y,SceneMotionCB);
  SPINNER_zcenCUSTOM=glui_motion->add_spinner_to_panel(PANEL_user_center,"z:",GLUI_SPINNER_FLOAT,&zcenCUSTOMsmv,CUSTOM_ROTATION_Z,SceneMotionCB);
  SPINNER_xcenCUSTOM->set_float_limits(DENORMALIZE_X(0.0),DENORMALIZE_X(1.0));
  SPINNER_ycenCUSTOM->set_float_limits(DENORMALIZE_Y(0.0),DENORMALIZE_Y(1.0));
  SPINNER_zcenCUSTOM->set_float_limits(DENORMALIZE_Z(0.0),DENORMALIZE_Z(1.0));

  SceneMotionCB(MESH_LIST);

  PANEL_anglebuttons = glui_motion->add_panel_to_panel(ROLLOUT_rotation_type,"",GLUI_PANEL_NONE);
  BUTTON_90_z=glui_motion->add_button_to_panel(PANEL_anglebuttons,"90 deg",EYE_ROTATE_90,SceneMotionCB);
  BUTTON_90_z->disable();
  BUTTON_90_z->set_alignment(GLUI_ALIGN_LEFT);
//  glui_motion->add_column_to_panel(PANEL_anglebuttons,false);
  BUTTON_snap=glui_motion->add_button_to_panel(PANEL_anglebuttons,_("Snap"),SNAPSCENE,SceneMotionCB);

  //glui_motion->add_column(false);

  ROLLOUT_orientation=glui_motion->add_rollout_to_panel(PANEL_motion,_("Orientation"),false,ORIENTATION_ROLLOUT,MotionRolloutCB);
  ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_orientation, ORIENTATION_ROLLOUT);

  PANEL_change_zaxis = glui_motion->add_panel_to_panel(ROLLOUT_orientation,_("z axis"));

  if(zaxis_custom==0){
    float vv[3];

    if(have_gvec==1){
      vv[0] = -gvecphys[0];
      vv[1] = -gvecphys[1];
      vv[2] = -gvecphys[2];
    }
    else{
      vv[0] = -gvecphys_orig[0];
      vv[1] = -gvecphys_orig[1];
      vv[2] = -gvecphys_orig[2];
    }
    XYZ2AzElev(vv, zaxis_angles, zaxis_angles+1);
  }
  SPINNER_zaxis_angles[0] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("azimuth:"),GLUI_SPINNER_FLOAT, zaxis_angles,ZAXIS_CUSTOM, SceneMotionCB);
  SPINNER_zaxis_angles[1] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("elevation:"),GLUI_SPINNER_FLOAT,zaxis_angles+1,ZAXIS_CUSTOM,SceneMotionCB);
  SPINNER_zaxis_angles[2] = glui_motion->add_spinner_to_panel(PANEL_change_zaxis, _("angle (about z axis):"),GLUI_SPINNER_FLOAT,zaxis_angles+2,ZAXIS_CUSTOM,SceneMotionCB);
  SPINNER_zaxis_angles[0]->set_float_limits(-180.0,180.0);
  SPINNER_zaxis_angles[1]->set_float_limits(-90.0,90.0);
  SPINNER_zaxis_angles[2]->set_float_limits(-180.0,180.0);

  glui_motion->add_button_to_panel(PANEL_change_zaxis, "z vector up", ZAXIS_UP, SceneMotionCB);
  if(have_gvec==1){
    glui_motion->add_button_to_panel(PANEL_change_zaxis, "Gravity vector down", USE_GVEC, SceneMotionCB);
    CHECKBOX_showgravity_vector = glui_motion->add_checkbox_to_panel(PANEL_change_zaxis, "Show gravity, axis vectors", &showgravity_vector);
  }
  else{
    CHECKBOX_showgravity_vector = glui_motion->add_checkbox_to_panel(PANEL_change_zaxis,_("Show axis vectors"),&showgravity_vector);
  }
  SceneMotionCB(ZAXIS_CUSTOM);
  ROLLOUT_orientation->close();
  zaxis_custom=0;

  ROLLOUT_gslice = glui_motion->add_rollout_to_panel(PANEL_motion, _("Slice motion"),false,SLICE_ROLLOUT,MotionRolloutCB);
  ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_gslice,SLICE_ROLLOUT);

  if(gslice_xyz[0]<-1000000.0&&gslice_xyz[1]<-1000000.0&&gslice_xyz[2]<-1000000.0){
    gslice_xyz[0]=(xbar0+DENORMALIZE_X(xbar))/2.0;
    gslice_xyz[1]=(ybar0+DENORMALIZE_Y(ybar))/2.0;
    gslice_xyz[2]=(zbar0+DENORMALIZE_Z(zbar))/2.0;
  }

  PANEL_gslice_center = glui_motion->add_panel_to_panel(ROLLOUT_gslice,_("rotation center"),true);
  SPINNER_gslice_center_x=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"x:",GLUI_SPINNER_FLOAT,gslice_xyz,GSLICE_TRANSLATE, GSliceCB);
  SPINNER_gslice_center_y=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"y:",GLUI_SPINNER_FLOAT,gslice_xyz+1,GSLICE_TRANSLATE, GSliceCB);
  SPINNER_gslice_center_z=glui_motion->add_spinner_to_panel(PANEL_gslice_center,"z:",GLUI_SPINNER_FLOAT,gslice_xyz+2,GSLICE_TRANSLATE, GSliceCB);
  SPINNER_gslice_center_x->set_float_limits(xbar0,DENORMALIZE_X(xbar),GLUI_LIMIT_CLAMP);
  SPINNER_gslice_center_y->set_float_limits(ybar0,DENORMALIZE_Y(ybar),GLUI_LIMIT_CLAMP);
  SPINNER_gslice_center_z->set_float_limits(zbar0,DENORMALIZE_Z(zbar),GLUI_LIMIT_CLAMP);
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

  PANEL_viewA = glui_motion->add_panel(_("View"), true);
  ROLLOUT_viewpoints = glui_motion->add_rollout_to_panel(PANEL_viewA,_("Viewpoints"), false,VIEWPOINTS_ROLLOUT,MotionRolloutCB);
  ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_viewpoints,VIEWPOINTS_ROLLOUT);

  LIST_viewpoints = glui_motion->add_listbox_to_panel(ROLLOUT_viewpoints, _("Select:"), &i_view_list, LIST_VIEW, ViewpointCB);
  LIST_viewpoints->set_alignment(GLUI_ALIGN_CENTER);

  PANEL_reset = glui_motion->add_panel_to_panel(ROLLOUT_viewpoints, "", false);

  PANEL_reset1 = glui_motion->add_panel_to_panel(PANEL_reset, "", false);

  BUTTON_delete_view = glui_motion->add_button_to_panel(PANEL_reset1, _("Delete"), DELETE_VIEW, ViewpointCB);
  delete_view_is_disabled = 0;
  BUTTON_startup = glui_motion->add_button_to_panel(PANEL_reset1, _("Apply at startup"), STARTUP, ViewpointCB);
  BUTTON_cycle_views = glui_motion->add_button_to_panel(PANEL_reset1, _("Cycle"), CYCLEVIEWS, ViewpointCB);

  glui_motion->add_column_to_panel(PANEL_reset, true);
  PANEL_reset2 = glui_motion->add_panel_to_panel(PANEL_reset, "", false);

  BUTTON_replace_view = glui_motion->add_button_to_panel(PANEL_reset2, _("Replace"), REPLACE_VIEW, ViewpointCB);
  BUTTON_add_view = glui_motion->add_button_to_panel(PANEL_reset2, _("Add"), ADD_VIEW, ViewpointCB);
  EDIT_view_label = glui_motion->add_edittext_to_panel(PANEL_reset2, _("Edit:"), GLUI_EDITTEXT_TEXT, camera_label, LABEL_VIEW, ViewpointCB);

  ROLLOUT_projection = glui_motion->add_rollout_to_panel(PANEL_viewA,_("Window properties"), false,WINDOW_ROLLOUT,MotionRolloutCB);
  ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_projection,WINDOW_ROLLOUT);

  RADIO_projection = glui_motion->add_radiogroup_to_panel(ROLLOUT_projection, &projection_type, PROJECTION, SceneMotionCB);
  RADIOBUTTON_1a = glui_motion->add_radiobutton_to_group(RADIO_projection, _("Perspective"));
  RADIOBUTTON_1b = glui_motion->add_radiobutton_to_group(RADIO_projection, _("Size preserving"));
  SPINNER_zoom = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("Zoom"), GLUI_SPINNER_FLOAT, &zoom, ZOOM, SceneMotionCB);
  SPINNER_zoom->set_float_limits(zoom_min, zoom_max, GLUI_LIMIT_CLAMP);
  aperture_glui = Zoom2Aperture(zoom);
  aperture_min = Zoom2Aperture(zoom_max);
  aperture_max = Zoom2Aperture(zoom_min);
  SPINNER_aperture = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("aperture"), GLUI_SPINNER_FLOAT, &aperture_glui,
    APERTURE, SceneMotionCB);
  glui_motion->add_separator_to_panel(ROLLOUT_projection);

  LIST_windowsize = glui_motion->add_listbox_to_panel(ROLLOUT_projection, _("Size:"), &windowsize_pointer, WINDOWSIZE_LIST, SceneMotionCB);
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
  UpdateWindowSizeList();

  SPINNER_window_width = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("width"), GLUI_SPINNER_INT, &glui_screenWidth);
  SPINNER_window_width->set_int_limits(100, max_screenWidth);
  SPINNER_window_height = glui_motion->add_spinner_to_panel(ROLLOUT_projection, _("height"), GLUI_SPINNER_INT, &glui_screenHeight);
  SPINNER_window_height->set_int_limits(100, max_screenHeight);

  PANEL_colors = glui_motion->add_panel_to_panel(ROLLOUT_projection, "Colors", true);

  ROLLOUT_foreground = glui_motion->add_rollout_to_panel(PANEL_colors,_("Background"), false);
  SPINNER_foreground_red = glui_motion->add_spinner_to_panel(ROLLOUT_foreground,_("red"),GLUI_SPINNER_INT,glui_foregroundbasecolor,WINDOW_COLORS,SceneMotionCB);
  SPINNER_foreground_green = glui_motion->add_spinner_to_panel(ROLLOUT_foreground, _("green"), GLUI_SPINNER_INT, glui_foregroundbasecolor+1, WINDOW_COLORS, SceneMotionCB);
  SPINNER_foreground_blue = glui_motion->add_spinner_to_panel(ROLLOUT_foreground, _("blue"), GLUI_SPINNER_INT, glui_foregroundbasecolor+2, WINDOW_COLORS, SceneMotionCB);
  SPINNER_foreground_red->set_int_limits(0, 255);
  SPINNER_foreground_green->set_int_limits(0, 255);
  SPINNER_foreground_blue->set_int_limits(0, 255);

  ROLLOUT_background = glui_motion->add_rollout_to_panel(PANEL_colors, _("Foreground"), false);
  SPINNER_background_red = glui_motion->add_spinner_to_panel(ROLLOUT_background,_("red"),GLUI_SPINNER_INT,glui_backgroundbasecolor,WINDOW_COLORS,SceneMotionCB);
  SPINNER_background_green = glui_motion->add_spinner_to_panel(ROLLOUT_background,_("green"),GLUI_SPINNER_INT,glui_backgroundbasecolor+1,WINDOW_COLORS,SceneMotionCB);
  SPINNER_background_blue = glui_motion->add_spinner_to_panel(ROLLOUT_background,_("blue"),GLUI_SPINNER_INT,glui_backgroundbasecolor+2,WINDOW_COLORS,SceneMotionCB);
  SPINNER_background_red->set_int_limits(0, 255);
  SPINNER_background_green->set_int_limits(0, 255);
  SPINNER_background_blue->set_int_limits(0, 255);

  BUTTON_flip = glui_motion->add_button_to_panel(PANEL_colors, _("Flip"), COLOR_FLIP, SceneMotionCB);

  BUTTON_window_update = glui_motion->add_button_to_panel(ROLLOUT_projection, _("Apply"), WINDOW_RESIZE, SceneMotionCB);

  ROLLOUT_scale = glui_motion->add_rollout_to_panel(PANEL_viewA,_("Scaling"),false,SCALING_ROLLOUT,MotionRolloutCB);
  ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_scale,SCALING_ROLLOUT);

  SPINNER_scalex=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_A(_("Scale")," x"),GLUI_SPINNER_FLOAT,mscale);
  SPINNER_scalex->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

  SPINNER_scaley=glui_motion->add_spinner_to_panel(ROLLOUT_scale, _A(_("Scale"), " y"),GLUI_SPINNER_FLOAT,mscale+1);
  SPINNER_scaley->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

  SPINNER_scalez=glui_motion->add_spinner_to_panel(ROLLOUT_scale, _A(_("Scale"), " z"),GLUI_SPINNER_FLOAT,mscale+2);
  SPINNER_scalez->set_float_limits(0.01,100.0,GLUI_LIMIT_CLAMP);

  SPINNER_nearclip=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_("Near depth"),GLUI_SPINNER_FLOAT,&nearclip, NEARFARCLIP, SceneMotionCB);

  SPINNER_farclip=glui_motion->add_spinner_to_panel(ROLLOUT_scale,_("Far depth"),GLUI_SPINNER_FLOAT,&farclip, NEARFARCLIP, SceneMotionCB);

  ROLLOUT_render = glui_motion->add_rollout(_("Render"), false,RENDER_ROLLOUT,MotionRolloutCB);
  ADDPROCINFO(motionprocinfo,nmotionprocinfo,ROLLOUT_render,RENDER_ROLLOUT);


  BUTTON_render_start = glui_motion->add_button_to_panel(ROLLOUT_render, _("Start rendering"), RENDER_START_TOP, RenderCB);
  glui_motion->add_button_to_panel(ROLLOUT_render, _("Stop rendering"), RENDER_STOP, RenderCB);

  ROLLOUT_name = glui_motion->add_rollout_to_panel(ROLLOUT_render, "File name/type", false);
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

  LIST_render_skip = glui_motion->add_listbox_to_panel(ROLLOUT_render, _("Show:"), &render_skip, RENDER_SKIP, RenderCB);
  LIST_render_skip->add_item(RENDER_CURRENT_SINGLE, _("Current"));
  LIST_render_skip->add_item(1, _("All frames"));
  LIST_render_skip->add_item(2, _("Every 2nd frame"));
  LIST_render_skip->add_item(3, _("Every 3rd frame"));
  LIST_render_skip->add_item(4, _("Every 4th frame"));
  LIST_render_skip->add_item(5, _("Every 5th frame"));
  LIST_render_skip->add_item(10, _("Every 10th frame"));
  LIST_render_skip->add_item(20, _("Every 20th frame"));
  LIST_render_skip->set_int_val(render_skip);

  ROLLOUT_image_size = glui_motion->add_rollout_to_panel(ROLLOUT_render, "size/type", false);
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

  ROLLOUT_screenvis = glui_motion->add_rollout_to_panel(ROLLOUT_render, "screenvis", false);
  CHECKBOX_screenview = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "view screens", &screenview);
  CHECKBOX_screenvis[0] = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "bottom", screenvis);

  ROLLOUT_lower = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "lower", false);
  CHECKBOX_screenvis[1] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "1", screenvis + 1);
  CHECKBOX_screenvis[2] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "2", screenvis + 2);
  CHECKBOX_screenvis[3] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "3", screenvis + 3);
  CHECKBOX_screenvis[4] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "4", screenvis + 4);
  CHECKBOX_screenvis[5] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "5", screenvis + 5);
  CHECKBOX_screenvis[6] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "6", screenvis + 6);
  CHECKBOX_screenvis[7] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "7", screenvis + 7);
  CHECKBOX_screenvis[8] = glui_motion->add_checkbox_to_panel(ROLLOUT_lower, "8", screenvis + 8);


  ROLLOUT_middle = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "middle", false);
  CHECKBOX_screenvis[9] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "1", screenvis + 9);
  CHECKBOX_screenvis[10] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "2", screenvis + 10);
  CHECKBOX_screenvis[11] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "3", screenvis + 11);
  CHECKBOX_screenvis[12] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "4", screenvis + 12);
  CHECKBOX_screenvis[13] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "5", screenvis + 13);
  CHECKBOX_screenvis[14] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "6", screenvis + 14);
  CHECKBOX_screenvis[15] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "7", screenvis + 15);
  CHECKBOX_screenvis[16] = glui_motion->add_checkbox_to_panel(ROLLOUT_middle, "8", screenvis + 16);

  ROLLOUT_upper = glui_motion->add_rollout_to_panel(ROLLOUT_screenvis, "upper", false);
  CHECKBOX_screenvis[17] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "1", screenvis + 17);
  CHECKBOX_screenvis[18] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "2", screenvis + 18);
  CHECKBOX_screenvis[19] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "3", screenvis + 19);
  CHECKBOX_screenvis[20] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "4", screenvis + 20);
  CHECKBOX_screenvis[21] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "5", screenvis + 21);
  CHECKBOX_screenvis[22] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "6", screenvis + 22);
  CHECKBOX_screenvis[23] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "7", screenvis + 23);
  CHECKBOX_screenvis[24] = glui_motion->add_checkbox_to_panel(ROLLOUT_upper, "8", screenvis + 24);

  CHECKBOX_screenvis[25] = glui_motion->add_checkbox_to_panel(ROLLOUT_screenvis, "top", screenvis + 25);
  BUTTON_screen_showall = glui_motion->add_button_to_panel(ROLLOUT_screenvis, _("Show all"), SHOWALL_SCREENS, ViewpointCB);
  BUTTON_screen_hideall = glui_motion->add_button_to_panel(ROLLOUT_screenvis, _("Hide all"), HIDEALL_SCREENS, ViewpointCB);
#endif

  UpdateGluiFileLabel(render_label_type);

  ROLLOUT_scene_clip = glui_motion->add_rollout_to_panel(ROLLOUT_render, "Clipping region", false);
  SPINNER_clip_left = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "left:", GLUI_SPINNER_INT, &render_clip_left);
  SPINNER_clip_left->set_int_limits(0, screenWidth);

  SPINNER_clip_right = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "right:", GLUI_SPINNER_INT, &render_clip_right);
  SPINNER_clip_right->set_int_limits(0, screenWidth);

  SPINNER_clip_bottom = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "bottom:", GLUI_SPINNER_INT, &render_clip_bottom);
  SPINNER_clip_bottom->set_int_limits(0, screenHeight);

  SPINNER_clip_top = glui_motion->add_spinner_to_panel(ROLLOUT_scene_clip, "top:", GLUI_SPINNER_INT, &render_clip_top);
  SPINNER_clip_top->set_int_limits(0, screenHeight);

  CHECKBOX_clip_rendered_scene = glui_motion->add_checkbox_to_panel(ROLLOUT_scene_clip, "clip rendered scene", &clip_rendered_scene);

  if(have_ffmpeg == 1){
    ROLLOUT_make_movie = glui_motion->add_rollout("Movie", false, MOVIE_ROLLOUT,MotionRolloutCB);
    ADDPROCINFO(motionprocinfo, nmotionprocinfo, ROLLOUT_make_movie, MOVIE_ROLLOUT);

    CHECKBOX_overwrite_movie = glui_motion->add_checkbox_to_panel(ROLLOUT_make_movie, "Overwrite movie", &overwrite_movie);
    glui_motion->add_button_to_panel(ROLLOUT_make_movie, _("Render normal"), RENDER_START_NORMAL, RenderCB);
    BUTTON_make_movie = glui_motion->add_button_to_panel(ROLLOUT_make_movie, "Make movie", MAKE_MOVIE, RenderCB);
    if(have_ffplay==1){
      BUTTON_play_movie = glui_motion->add_button_to_panel(ROLLOUT_make_movie, "Play movie", PLAY_MOVIE, RenderCB);
      EnableDisablePlayMovie();
    }
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
  }

  CHECKBOX_cursor_blockpath=glui_motion->add_checkbox(_("Map cursor keys for Plot3D use"),&cursorPlot3D,CURSOR,SceneMotionCB);
  PANEL_close = glui_motion->add_panel("",GLUI_PANEL_NONE);

  BUTTON_motion_1=glui_motion->add_button_to_panel(PANEL_close,_("Save settings"),SAVE_SETTINGS, MotionDlgCB);

  glui_motion->add_column_to_panel(PANEL_close,false);

  BUTTON_motion_2=glui_motion->add_button_to_panel(PANEL_close,_("Close"),1, MotionDlgCB);

  ShowHideTranslate(rotation_type);
  glui_motion->set_main_gfx_window( main_window );
}

/* ------------------ UpdateWindowSizeList ------------------------ */

extern "C" void UpdateWindowSizeList(void){
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
}

/* ------------------ UpdateTranslate ------------------------ */

extern "C" void UpdateTranslate(void){
  float *eye_xyz,*az_elev;

  eye_xyz = camera_current->eye;
  az_elev = camera_current->az_elev;

  d_eye_xyz[0]=eye_xyz[0]-eye_xyz0[0];
  d_eye_xyz[1]=eye_xyz[1]-eye_xyz0[1];
  d_eye_xyz[2]=eye_xyz[2]-eye_xyz0[2];

  TRANSLATE_xy->set_x(d_eye_xyz[0]);
  if(rotation_type==ROTATION_1AXIS){
    d_eye_xyz[1]=0.0;
  }
  TRANSLATE_xy->set_y(d_eye_xyz[1]);
  TRANSLATE_z->set_y(eye_xyz[2]);
  if(rotation_type==ROTATION_3AXIS){
  }
  else{
    ROTATE_2axis->set_x(az_elev[0]);
    ROTATE_2axis->set_y(az_elev[1]);
    ROTATE_eye_z->set_x(camera_current->azimuth);
  }
  UpdateGluiSetViewXYZ(camera_current->eye);
}

/* ------------------ UpdateRotationIndex ------------------------ */

extern "C" void UpdateRotationIndex(int val){
  float *az_elev;
  int *rotation_index;

  rotation_index = &camera_current->rotation_index;

  *rotation_index=val;
  camera_current->rotation_index=val;
  if(*rotation_index>=0&&*rotation_index<nmeshes){
    meshdata *meshi;

    meshi = meshinfo + *rotation_index;
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
      else{
        camera_current->xcen = ((camera_current->clip_xmin == 1 ? clipinfo.xmin : xbar0ORIG) + (camera_current->clip_xmax == 1 ? clipinfo.xmax : xbarORIG)) / 2.0;
        camera_current->ycen = ((camera_current->clip_ymin == 1 ? clipinfo.ymin : ybar0ORIG) + (camera_current->clip_ymax == 1 ? clipinfo.ymax : ybarORIG)) / 2.0;
        camera_current->zcen = ((camera_current->clip_zmin == 1 ? clipinfo.zmin : zbar0ORIG) + (camera_current->clip_zmax == 1 ? clipinfo.zmax : zbarORIG)) / 2.0;
        camera_current->xcen = NORMALIZE_X(camera_current->xcen);
        camera_current->ycen = NORMALIZE_Y(camera_current->ycen);
        camera_current->zcen = NORMALIZE_Z(camera_current->zcen);
      }
    }
  }
  if(*rotation_index!=ROTATE_ABOUT_USER_CENTER){
    xcenCUSTOM = camera_current->xcen;
    ycenCUSTOM = camera_current->ycen;
    zcenCUSTOM = camera_current->zcen;
    xcenCUSTOMsmv = DENORMALIZE_X(xcenCUSTOM);
    ycenCUSTOMsmv = DENORMALIZE_Y(ycenCUSTOM);
    zcenCUSTOMsmv = DENORMALIZE_Z(zcenCUSTOM);
    if(SPINNER_xcenCUSTOM!=NULL)SPINNER_xcenCUSTOM->set_float_val(xcenCUSTOMsmv);
    if(SPINNER_ycenCUSTOM!=NULL)SPINNER_ycenCUSTOM->set_float_val(ycenCUSTOMsmv);
    if(SPINNER_zcenCUSTOM!=NULL)SPINNER_zcenCUSTOM->set_float_val(zcenCUSTOMsmv);
  }

  az_elev = camera_current->az_elev;

  az_elev[0]=0.;
  az_elev[1]=0.;

  camera_current->azimuth=0.0;

  camera_current->view_angle=0.0;

  UpdateMeshList1(val);

  glutPostRedisplay();

}

/* ------------------ UpdateProjectionType ------------------------ */

extern "C" void UpdateProjectionType(void){
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

/* ------------------ ShowHideTranslate ------------------------ */

extern "C" void ShowHideTranslate(int var){
  float *eye_xyz;

  eye_xyz = camera_current->eye;

  eye_xyz0[0]=eye_xyz[0];
  eye_xyz0[1]=eye_xyz[1];
  eye_xyz0[2]=eye_xyz[2];
  d_eye_xyz[0]=0.0;
  d_eye_xyz[1]=0.0;
  switch(var){
  case ROTATION_3AXIS:
    if(PANEL_translate!=NULL)PANEL_translate->enable();
    if(ROTATE_2axis!=NULL)ROTATE_2axis->disable();
    if(ROTATE_eye_z!=NULL)ROTATE_eye_z->disable();
    if(BUTTON_90_z!=NULL)BUTTON_90_z->disable();
    if(CHECKBOX_blockpath!=NULL)CHECKBOX_blockpath->disable();
    if(PANEL_speed!=NULL)PANEL_speed->disable();
    if(PANEL_height!=NULL)PANEL_height->disable();
    if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
    if(BUTTON_eyelevel!=NULL)BUTTON_eyelevel->disable();
    if(BUTTON_floorlevel!=NULL)BUTTON_floorlevel->disable();
    if(LIST_mesh2!=NULL)LIST_mesh2->enable();
    if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->enable();
    if(BUTTON_snap!=NULL)BUTTON_snap->enable();
    break;
  case ROTATION_2AXIS:
    if(PANEL_translate!=NULL)PANEL_translate->enable();
    if(ROTATE_2axis!=NULL)ROTATE_2axis->enable();
    if(ROTATE_eye_z!=NULL)ROTATE_eye_z->disable();
    if(BUTTON_90_z!=NULL)BUTTON_90_z->disable();
    if(CHECKBOX_blockpath!=NULL)CHECKBOX_blockpath->disable();
    if(PANEL_speed!=NULL)PANEL_speed->disable();
    if(PANEL_height!=NULL)PANEL_height->disable();
    if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
    if(BUTTON_eyelevel!=NULL)BUTTON_eyelevel->disable();
    if(BUTTON_floorlevel!=NULL)BUTTON_floorlevel->disable();
    if(LIST_mesh2!=NULL)LIST_mesh2->enable();
    if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->enable();
    if(BUTTON_snap!=NULL)BUTTON_snap->enable();
    break;
  case EYE_CENTERED:
    if(PANEL_translate!=NULL)PANEL_translate->enable();
    if(ROTATE_2axis!=NULL)ROTATE_2axis->disable();
    if(ROTATE_eye_z!=NULL)ROTATE_eye_z->enable();
    if(BUTTON_90_z!=NULL)BUTTON_90_z->enable();
    if(CHECKBOX_blockpath!=NULL)CHECKBOX_blockpath->enable();
    if(PANEL_speed!=NULL)PANEL_speed->enable();
    if(PANEL_height!=NULL)PANEL_height->enable();
    if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
    if(BUTTON_eyelevel!=NULL)BUTTON_eyelevel->enable();
    if(BUTTON_floorlevel!=NULL)BUTTON_floorlevel->enable();
    if(LIST_mesh2!=NULL)LIST_mesh2->disable();
    if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->disable();
    if(BUTTON_snap!=NULL)BUTTON_snap->disable();
    break;
  case ROTATION_1AXIS:
    if(PANEL_translate!=NULL)PANEL_translate->enable();
    if(ROTATE_2axis!=NULL)ROTATE_2axis->enable();
    if(ROTATE_eye_z!=NULL)ROTATE_eye_z->disable();
    if(BUTTON_90_z!=NULL)BUTTON_90_z->disable();
    if(CHECKBOX_blockpath!=NULL)CHECKBOX_blockpath->disable();
    if(PANEL_speed!=NULL)PANEL_speed->disable();
    if(PANEL_height!=NULL)PANEL_height->disable();
    if(RADIO_rotation_type!=NULL)RADIO_rotation_type->set_int_val(rotation_type);
    if(BUTTON_eyelevel!=NULL)BUTTON_eyelevel->disable();
    if(BUTTON_floorlevel!=NULL)BUTTON_floorlevel->disable();
    if(LIST_mesh2!=NULL)LIST_mesh2->enable();
    if(CHECKBOX_show_rotation_center!=NULL)CHECKBOX_show_rotation_center->enable();
    if(BUTTON_snap!=NULL)BUTTON_snap->enable();
    break;
  default:
    ASSERT(FFALSE);
  }

}

/* ------------------ ToggleRollout ------------------------ */

extern "C" void ToggleRollout(procdata *procinfo, int nprocinfo, int motion_id){
  int i;

  for(i=0;i<nprocinfo;i++){
    procdata *mi;

    mi = procinfo + i;
    if(mi->rollout_id==motion_id){
      mi->rollout->open();
    }
    else{
      if(toggle_dialogs==1)mi->rollout->close();
    }
  }
}

  /* ------------------ SceneMotionCB ------------------------ */

extern "C" void SceneMotionCB(int var){
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
    SceneMotionCB(CUSTOM_VIEW);
    return;
  }
  if(var==ROTATE_90){
    customview_azimuth=fmod(customview_azimuth+90.0,360.0);
    SPINNER_customview_azimuth->set_float_val(customview_azimuth);
    SceneMotionCB(CUSTOM_VIEW);
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
      nearclip=0.001;
      SPINNER_nearclip->set_float_val(nearclip);
    }
    if(farclip<0.0){
      farclip=0.001;
      SPINNER_farclip->set_float_val(farclip);
    }
    return;
  }

  if(var==CLIP_SHOW_ROTATE){
    UpdateShowRotationCenter2();
    return;
  }

#ifdef pp_GPUTHROTTLE
  if(usegpu==1&&showvolrender==1&&show_volsmoke_moving==1&&
     (var==EYE_ROTATE||var==EYE_ROTATE_90||var==ROTATE_2AXIS||var==TRANSLATE_XY||var==GLUI_Z)
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
        UpdateTranslate();
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
      SceneMotionCB(EYE_ROTATE);
      glui_move_mode=EYE_ROTATE_90;
      return;
    case ROTATE_2AXIS:
      if(rotation_type==ROTATION_2AXIS){
        float *az_elev;

        az_elev = camera_current->az_elev;
        az_elev[0] = ROTATE_2axis->get_x();
        az_elev[1] = -ROTATE_2axis->get_y();
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
          ASSERT(FFALSE);
          break;
      }
      if(windowsize_pointer>=2){
        SPINNER_window_width->set_int_val(glui_screenWidth);
        SPINNER_window_height->set_int_val(glui_screenHeight);
        SetScreenSize(&glui_screenWidth,&glui_screenHeight);
        ResizeWindow(screenWidth,screenHeight);
      }
      {
        int width_low, height_low, width_high, height_high;
 
        GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
        UpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
      }
      break;
    case SNAPSCENE:
      SnapScene();
      break;
    case WINDOW_RESIZE:
      SetScreenSize(&glui_screenWidth,&glui_screenHeight);
      UpdateWindowSizeList();
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
    case CUSTOM_ROTATION_X:
      xcenCUSTOM = NORMALIZE_X(xcenCUSTOMsmv);
      UpdateRotationIndex(ROTATE_ABOUT_USER_CENTER);
      break;
    case CUSTOM_ROTATION_Y:
      ycenCUSTOM = NORMALIZE_Y(ycenCUSTOMsmv);
      UpdateRotationIndex(ROTATE_ABOUT_USER_CENTER);
      break;
    case CUSTOM_ROTATION_Z:
      zcenCUSTOM = NORMALIZE_Z(zcenCUSTOMsmv);
      UpdateRotationIndex(ROTATE_ABOUT_USER_CENTER);
      break;
    case MESH_LIST:
      glui_rotation_index = *rotation_index;
      if(*rotation_index==ROTATE_ABOUT_USER_CENTER){
        custom_worldcenter=1;
        SPINNER_xcenCUSTOM->enable();
        SPINNER_ycenCUSTOM->enable();
        SPINNER_zcenCUSTOM->enable();
      }
      else if(*rotation_index==ROTATE_ABOUT_CLIPPING_CENTER){
        custom_worldcenter = 1;
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
      if(*rotation_index>=0&&*rotation_index<nmeshes){
        UpdateCurrentMesh(meshinfo + (*rotation_index));
        UpdateRotationIndex(*rotation_index);
      }
      else if(*rotation_index==ROTATE_ABOUT_USER_CENTER){
        UpdateRotationIndex(ROTATE_ABOUT_USER_CENTER);
      }
      else if(*rotation_index==ROTATE_ABOUT_CLIPPING_CENTER){
        UpdateRotationIndex(ROTATE_ABOUT_CLIPPING_CENTER);
      }
      else{
        UpdateCurrentMesh(meshinfo);
        UpdateRotationIndex(nmeshes);
      }
      update_rotation_center=1;
      return;
    case ZOOM:
      zoomindex=-1;
      for(i=0;i<5;i++){
        if(ABS(zoom-zooms[i])<0.001){
          zoomindex=i;
          zoom=zooms[i];
          break;
        }
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
      for(i=0;i<5;i++){
        if(ABS(zoom-zooms[i])<0.001){
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
    case GLUI_Z:
      break;
    default:
      ASSERT(FFALSE);
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

        gvec_down=1;
        SceneMotionCB(ZAXIS_UP);
        gvec_down=1;
        vv[0] = -gvecphys[0];
        vv[1] = -gvecphys[1];
        vv[2] = -gvecphys[2];
        XYZ2AzElev(vv, zaxis_angles, zaxis_angles+1);
        UpdateZaxisAngles();

        az = zaxis_angles;
        elev = zaxis_angles+1;
        user_zaxis[0] = cos(DEG2RAD*(*az))*cos(DEG2RAD*(*elev));
        user_zaxis[1] = sin(DEG2RAD*(*az))*cos(DEG2RAD*(*elev));
        user_zaxis[2] = sin(DEG2RAD*(*elev));
        if(LIST_viewpoints!=NULL)LIST_viewpoints->set_int_val(EXTERNAL_LIST_ID);
        ViewpointCB(LIST_VIEW);
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
      SceneMotionCB(ZAXIS_CUSTOM);
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
      NORMALIZE_XYZ(eye_xyz,set_view_xyz);
      eye_xyz0[0]=eye_xyz[0];
      eye_xyz0[1]=eye_xyz[1];
      eye_xyz0[2]=eye_xyz[2];
      UpdateTranslate();
      break;
    case EYE_ROTATE:
    case TRANSLATE_XY:
      if(glui_move_mode==EYE_ROTATE){
        eye_xyz0[0]=eye_xyz[0];
        eye_xyz0[1]=eye_xyz[1];
      }
      if(TRANSLATE_xy!=NULL){
        TRANSLATE_xy->set_x(d_eye_xyz[0]);
        TRANSLATE_xy->set_y(d_eye_xyz[1]);
      }
      glui_move_mode=TRANSLATE_XY;
      UpdateTranslate();
      break;
    case GLUI_Z:
      if(glui_move_mode==EYE_ROTATE){
        eye_xyz0[0]=eye_xyz[0];
        eye_xyz0[1]=eye_xyz[1];
      }
      glui_move_mode=GLUI_Z;
      UpdateTranslate();
      break;
    case APERTURE:
    case ZOOM:
    case FLOORLEVEL:
    case PROJECTION:
    case WINDOW_RESIZE:
    case WINDOWSIZE_LIST:
    case SNAPSCENE:
    case CUSTOM_ROTATION_X:
    case CUSTOM_ROTATION_Y:
    case CUSTOM_ROTATION_Z:
    case ROTATE_2AXIS:
    case WINDOW_COLORS:
    case COLOR_FLIP:
      break;
    default:
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ UpdateMeshList1 ------------------------ */

extern "C" void UpdateMeshList1(int val){
  if(LIST_mesh2==NULL)return;
  LIST_mesh2->set_int_val(val);
  if(val>=0&&val<nmeshes){
    RADIO_rotation_type->set_int_val(ROTATION_2AXIS);
    HandleRotationType(ROTATION_2AXIS);
  }
}

/* ------------------ HideGluiMotion ------------------------ */

extern "C" void HideGluiMotion(void){
  if(glui_motion!=NULL)glui_motion->hide();
}

/* ------------------ ShowGluiMotion ------------------------ */

extern "C" void ShowGluiMotion(int menu_id){
  glui_motion->show();
  if(glui_motion != NULL){
    switch(menu_id){
    case DIALOG_VIEW:
      MotionRolloutCB(VIEWPOINTS_ROLLOUT);
      break;
    case DIALOG_MOTION:
      MotionRolloutCB(TRANSLATEROTATE_ROLLOUT);
      break;
    case DIALOG_RENDER:
      MotionRolloutCB(RENDER_ROLLOUT);
      break;
    case DIALOG_MOVIE:
      MotionRolloutCB(MOVIE_ROLLOUT);
      break;
    case DIALOG_WINDOW:
      MotionRolloutCB(WINDOW_ROLLOUT);
      break;
    case DIALOG_SCALING:
      MotionRolloutCB(SCALING_ROLLOUT);
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
  }
}

/* ------------------ SetStartupView ------------------------ */

extern "C" void SetStartupView(void){
  ViewpointCB(STARTUP);
}

/* ------------------ AddListView ------------------------ */

extern "C" void AddListView(char *label_in){
  int ival;
  char *label;
  cameradata *cam1,*cam2,*cex,*ca;

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

/* ------------------ RotationTypeCB ------------------------ */

extern "C" void RotationTypeCB(int var){
  if(var==ROTATION_3AXIS){
    Camera2Quat(camera_current,quat_general,quat_rotation);
  }
  else{
    camera_current->quat_defined=0;
  }
  HandleRotationType(ROTATION_2AXIS);
}

/* ------------------ Enable360Zoom ------------------------ */

extern "C" void Enable360Zoom(void){
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
      sprintf(widthlabel,"width: %i",nwidth360);
      {
        int width_low, height_low, width_high, height_high;

        GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
        if(width_low>2){
          UpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
        }
      }
      break;
    case MOVIE_NAME:
      EnableDisablePlayMovie();
      break;
    case PLAY_MOVIE:
      PlayMovie();
      break;
    case OUTPUT_FFMPEG:
      output_ffmpeg_command=1;
      break;
    case MAKE_MOVIE:
      if(have_ffmpeg == 0){
        PRINTF("*** Error: The movie generating program ffmpeg is not available\n");
        break;
      }
      EnableDisableMakeMovie(OFF);
      update_makemovie = 1;
      break;
    case RENDER_SKIP:
    case RENDER_LABEL:
    case RENDER_TYPE:
      break;
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
        UpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
      }
      break;
    case MOVIE_FILETYPE:
      switch (movie_filetype){
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
        ASSERT(FFALSE);
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
        UpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
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
        SceneMotionCB(WINDOW_RESIZE);
      }
      Disable360Zoom();
      RenderMenu(RENDER_CURRENT_360);
      if(render_skip != RENDER_CURRENT_SINGLE){
        Keyboard('0', FROM_SMOKEVIEW);
      }
      {
        int width_low, height_low, width_high, height_high;
 
        GetRenderResolution(&width_low, &height_low, &width_high, &height_high);
        UpdateRenderRadioButtons(width_low, height_low, width_high, height_high);
      }
      break;
    case RENDER_START_TOP:
      switch (render_resolution){
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
      }
    break;
    case RENDER_START_HIGHRES:
      RenderMenu(RenderStartHIGHRES);
      break;
    case RENDER_START_NORMAL:
      RenderMenu(RenderStartORIGRES);
      break;
    case RENDER_START:
      if (render_frame != NULL) {
        int i;

        for (i = 0; i < nglobal_times; i++) {
          render_frame[i] = 0;
        }
      }
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
      ASSERT(FFALSE);
      break;
  }
}

/* ------------------ UpdateRenderListSkip ------------------------ */

void UpdateRenderListSkip(void){
  if(LIST_render_skip!=NULL)LIST_render_skip->set_int_val(render_skip);
}

/* ------------------ UpdateGluiRender ------------------------ */

extern "C" void UpdateGluiRender(void){
  if(RenderTime==1&&RenderTimeOld==0){
    if(LIST_render_skip!=NULL&&render_skip==RENDER_CURRENT_SINGLE){
      render_skip=1;
      LIST_render_skip->set_int_val(render_skip);
    }
  }
  RenderTimeOld=RenderTime;
}
