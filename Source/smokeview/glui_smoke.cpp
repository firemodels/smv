#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "glui_smoke.h"
#include "glui_tour.h"
#include "glui_bounds.h"
#include "glui_motion.h"
#include "IOscript.h"

extern GLUI_Rollout *ROLLOUT_smoke3d;
extern GLUI *glui_bounds;

// two defines below are also defined elsewhere

GLUI *glui_3dsmoke=NULL;

GLUI_EditText *EDIT_vol_prefix=NULL;

GLUI_Listbox *LISTBOX_VOL_tour=NULL;

GLUI_Button *BUTTON_fds_extinction_reset = NULL;
GLUI_Button *BUTTON_smv_extinction_reset = NULL;
GLUI_Button *BUTTON_cutoff_defaults = NULL;
GLUI_Button *BUTTON_volunload=NULL;
GLUI_Button *BUTTON_startrender=NULL;
GLUI_Button *BUTTON_cancelrender=NULL;

GLUI_Listbox *LISTBOX_smoke_colorbar=NULL;
GLUI_Listbox *LISTBOX_co2_colorbar = NULL;

GLUI_RadioGroup *RADIO_use_fire_alpha = NULL;
GLUI_RadioGroup *RADIO_smokealign = NULL;
GLUI_RadioGroup *RADIO_smoke_outline_type = NULL;
GLUI_RadioGroup *RADIO_newsmoke = NULL;
GLUI_RadioGroup *RADIO_render=NULL;
GLUI_RadioGroup *RADIO_smokesensors=NULL;
GLUI_RadioGroup *RADIO_sootdensity = NULL;
GLUI_RadioGroup *RADIO_use_fire_colormap = NULL;

GLUI_Spinner *SPINNER_smoke_num=NULL;
GLUI_Spinner *SPINNER_startframe=NULL;
GLUI_Spinner *SPINNER_skipframe=NULL;
GLUI_Spinner *SPINNER_hrrpuv_min=NULL;
GLUI_Spinner *SPINNER_hrrpuv_max = NULL;
GLUI_Spinner *SPINNER_cb_min_index = NULL;
GLUI_Spinner *SPINNER_cb_max_index = NULL;
GLUI_Spinner *SPINNER_nongpu_vol_factor=NULL;
GLUI_Spinner *SPINNER_gpu_vol_factor=NULL;
GLUI_Spinner *SPINNER_smoke3d_load_start=NULL;
GLUI_Spinner *SPINNER_smoke3d_load_skip=NULL;

GLUI_Spinner *SPINNER_fire_temp_min = NULL;
GLUI_Spinner *SPINNER_fire_temp_max = NULL;
GLUI_Spinner *SPINNER_nfire_colors  = NULL;
GLUI_Spinner *SPINNER_voltest_depth1  = NULL;
GLUI_Spinner *SPINNER_voltest_depth2  = NULL;
GLUI_Spinner *SPINNER_temperature_min=NULL;
GLUI_Spinner *SPINNER_temperature_max=NULL;
GLUI_Spinner *SPINNER_fire_opacity_factor=NULL;
GLUI_Spinner *SPINNER_mass_extinct=NULL;
GLUI_Spinner *SPINNER_cvis=NULL;
GLUI_Spinner *SPINNER_smoke3d_skip = NULL;
GLUI_Spinner *SPINNER_smoke3d_skipxy = NULL;
GLUI_Spinner *SPINNER_smoke3d_skipx = NULL;
GLUI_Spinner *SPINNER_smoke3d_skipy = NULL;
GLUI_Spinner *SPINNER_smoke3d_skipz = NULL;
GLUI_Spinner *SPINNER_smoke3d_kmax = NULL;
GLUI_Spinner *SPINNER_smoke3d_extinct = NULL;
GLUI_Spinner *SPINNER_smoke3d_extinct2 = NULL;
GLUI_Spinner *SPINNER_smoke3d_frame_inc = NULL;

GLUI_Spinner *SPINNER_smoke3d_fire_red=NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_green=NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_blue=NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_halfdepth=NULL;
GLUI_Spinner *SPINNER_smoke3d_co2_halfdepth = NULL;
GLUI_Spinner *SPINNER_smoke3d_co2_alpha = NULL;
GLUI_Spinner *SPINNER_smoke3d_smoke_red=NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_alpha = NULL;
GLUI_Spinner *SPINNER_smoke3d_smoke_green=NULL;
GLUI_Spinner *SPINNER_smoke3d_smoke_blue=NULL;
GLUI_Spinner *SPINNER_smoke_test_color[4];
GLUI_Spinner *SPINNER_smoke_test_range = NULL;
GLUI_Spinner *SPINNER_smoke_test_nslices = NULL;
GLUI_Spinner *SPINNER_smokeloadframe = NULL;
GLUI_Spinner *SPINNER_globalloadframe = NULL;
GLUI_Spinner *SPINNER_timeloadframe = NULL;
GLUI_Spinner *SPINNER_co2color[3];
GLUI_Spinner *SPINNER_emission_factor=NULL;

GLUI_Checkbox *CHECKBOX_smoke3d_use_skip=NULL;
GLUI_Checkbox *CHECKBOX_use_opacity_depth = NULL;
GLUI_Checkbox *CHECKBOX_use_opacity_multiplier = NULL;
GLUI_Checkbox *CHECKBOX_force_alpha_opaque = NULL;
GLUI_Checkbox *CHECKBOX_use_co2_colormap = NULL;
GLUI_Checkbox *CHECKBOX_use_co2_rgb = NULL;
GLUI_Checkbox *CHECKBOX_smoke_flip=NULL;
GLUI_Checkbox *CHECKBOX_triangle_display_rate = NULL;
GLUI_Checkbox *CHECKBOX_smoke_getvals=NULL;
GLUI_Checkbox *CHECKBOX_update_smokeplanes = NULL;
GLUI_Checkbox *CHECKBOX_plane_single = NULL;
GLUI_Checkbox *CHECKBOX_freeze = NULL;
GLUI_Checkbox *CHECKBOX_combine_meshes = NULL;
GLUI_Checkbox *CHECKBOX_smokecullflag = NULL;
GLUI_Checkbox *CHECKBOX_test_smokesensors = NULL;
GLUI_Checkbox *CHECKBOX_smokeGPU = NULL;
GLUI_Checkbox *CHECKBOX_zlib = NULL;
GLUI_Checkbox **CHECKBOX_meshvisptr = NULL;
GLUI_Checkbox *CHECKBOX_meshvis = NULL;
GLUI_Checkbox *CHECKBOX_edit_colormap=NULL;
GLUI_Checkbox *CHECKBOX_plane_normal=NULL;

GLUI_Panel *PANEL_specify_fire = NULL;
GLUI_Panel *PANEL_fire_colormap = NULL;
GLUI_Panel *PANEL_fire_opacity = NULL;
GLUI_Panel *PANEL_smoke_opacity = NULL;
GLUI_Panel *PANEL_slice_alignment = NULL;
GLUI_Panel *PANEL_smoke_outline_type = NULL;
GLUI_Panel *PANEL_smokealg = NULL;
GLUI_Panel *PANEL_gridres = NULL;
GLUI_Panel *PANEL_fire_cutoff = NULL;
GLUI_Panel *PANEL_hrrpuv_minmax = NULL;
GLUI_Panel *PANEL_temp_minmax = NULL;
GLUI_Panel *PANEL_blackbody = NULL;
GLUI_Panel *PANEL_settings1 = NULL;
GLUI_Panel *PANEL_skip_planes = NULL;
GLUI_Panel *PANEL_smokesensor = NULL;
GLUI_Panel *PANEL_color = NULL;
GLUI_Panel *PANEL_smoke = NULL;
GLUI_Panel *PANEL_loadcutoff = NULL;
GLUI_Panel *PANEL_loadframe = NULL;
GLUI_Panel *PANEL_node_display = NULL;
GLUI_Panel *PANEL_display = NULL;
GLUI_Panel *PANEL_load_options = NULL;
GLUI_Panel *PANEL_fire_rgb = NULL;
GLUI_Panel *PANEL_cb_index = NULL;

GLUI_Rollout *ROLLOUT_smoke_settings = NULL;
GLUI_Rollout *ROLLOUT_skip = NULL;
GLUI_Rollout *ROLLOUT_voltest = NULL;
GLUI_Rollout *ROLLOUT_opacity = NULL;
GLUI_Rollout *ROLLOUT_voldisplay = NULL;
GLUI_Rollout *ROLLOUT_volsmoke_move = NULL;
GLUI_Rollout *ROLLOUT_slicehrrpuv = NULL;
GLUI_Rollout *ROLLOUT_firecolor = NULL;
GLUI_Rollout *ROLLOUT_smokecolor = NULL;
GLUI_Rollout *ROLLOUT_generate_images = NULL;
GLUI_Rollout *ROLLOUT_loadframe = NULL;
GLUI_Rollout *ROLLOUT_co2color = NULL;
GLUI_Rollout *ROLLOUT_temperature_settings=NULL;
GLUI_Rollout *ROLLOUT_slices = NULL;
GLUI_Rollout *ROLLOUT_volume = NULL;
GLUI_Rollout *ROLLOUT_smoke_test = NULL;
GLUI_Rollout *ROLLOUT_volsmoke_load = NULL;
GLUI_Rollout *ROLLOUT_volsmoke_compute = NULL;
GLUI_Rollout *ROLLOUT_smokedebug = NULL;

GLUI_StaticText *STATIC_smokeframelimit_min = NULL;
GLUI_StaticText *STATIC_smokeframelimit_max = NULL;
GLUI_StaticText *STATIC_globalframelimit_min = NULL;
GLUI_StaticText *STATIC_globalframelimit_max = NULL;
GLUI_StaticText *STATIC_timelimit_min = NULL;
GLUI_StaticText *STATIC_timelimit_max = NULL;
GLUI_StaticText *STATIC_pixels_per_triangle=NULL;

#define VOLRENDER_ROLLOUT   0
#define SLICERENDER_ROLLOUT 1
#define SMOKECOLOR_ROLLOUT  2
#define FIRECOLOR_ROLLOUT   3
#define FIREOPACITY_ROLLOUT 4
#define CO2COLOR_ROLLOUT    5
#define SKIP_ROLLOUT        6
#define SMOKE_SETTINGS      7

procdata smokeprocinfo[8], volsmokeprocinfo[6];
int nsmokeprocinfo = 0, nvolsmokeprocinfo=0;

#define VOLSMOKE_DISPLAY           0
#define VOLSMOKE_MOVEMENT          1
#define VOLSMOKE_COMPUTE_ROLLOUT   2
#define VOLSMOKE_LOAD_ROLLOUT      3
#define VOLSMOKE_IMAGES_ROLLOUT    4
#define VOLSMOKE_LOADFRAME_ROLLOUT 5

#define SLICESMOKE_LOAD_ROLLOUT 0
#define SLICESMOKE_ORIG_ROLLOUT 1
#define SLICESMOKE_TEST_ROLLOUT 2

/* ------------------ GLUIUpdateFireAlpha ------------------------ */

extern "C" void GLUIUpdateFireAlpha(void){
  GLUISmoke3dCB(USE_FIRE_ALPHA);
}

/* ------------------ GLUIUpdateCO2ColorbarList ------------------------ */

extern "C" void GLUIUpdateCO2ColorbarList(int value){
  colorbars.co2_colorbar_index = CLAMP(value, 0, colorbars.ncolorbars-1);
  if(LISTBOX_co2_colorbar!=NULL)LISTBOX_co2_colorbar->set_int_val(colorbars.co2_colorbar_index);
  GLUISmoke3dCB(CO2_COLORBAR_LIST);
}

/* ------------------ GLUIUpdateFireColorbarList ------------------------ */

extern "C" void GLUIUpdateFireColorbarList(void){
  if(LISTBOX_smoke_colorbar!=NULL)LISTBOX_smoke_colorbar->set_int_val(colorbars.fire_colorbar_index);
}

/* ------------------ GLUIUpdateBackgroundFlip2 ------------------------ */

extern "C" void GLUIUpdateBackgroundFlip2(int flip){
  if(CHECKBOX_smoke_flip!=NULL)CHECKBOX_smoke_flip->set_int_val(flip);
}

/* ------------------ GLUIUpdateFreeze ------------------------ */

extern "C" void GLUIUpdateFreeze(int val){
  CHECKBOX_freeze->set_int_val(val);
}

/* ------------------ GLUIForceAlphaOpaque ------------------------ */

extern "C" void GLUIForceAlphaOpaque(void){
  CHECKBOX_force_alpha_opaque->set_int_val(force_alpha_opaque);
}

/* ------------------ GLUIUpdateLoadFrameVal ------------------------ */

extern "C" void GLUIUpdateLoadFrameVal(int frames){
  SPINNER_smokeloadframe->set_int_val(frames);
}

/* ------------------ GLUIUpdateLoadTimeVal ------------------------ */

extern "C" void GLUIUpdateLoadTimeVal(float val){
  SPINNER_timeloadframe->set_float_val(val);
}

/* ------------------ GLUIUpdateLoadFrameMax ------------------------ */

extern "C" void GLUIUpdateLoadFrameMax(int max_frames){
  int val;

  val = SPINNER_smokeloadframe->get_int_val();
  if(val<0){
    SPINNER_smokeloadframe->set_int_val(0);
  }
  else if(val>max_frames-1){
    SPINNER_smokeloadframe->set_int_val(max_frames-1);
  }
  SPINNER_smokeloadframe->set_int_limits(0, max_frames-1);
}

/* ------------------ GLUIUpdateTimeFrameBounds ------------------------ */

extern "C" void GLUIUpdateTimeFrameBounds(float time_min, float time_max){
  float val;

  val = SPINNER_timeloadframe->get_float_val();
  if(val>time_max){
    SPINNER_timeloadframe->set_float_val(time_max);
  }
  else if(val<time_min){
    SPINNER_timeloadframe->set_float_val(time_min);
  }
  SPINNER_timeloadframe->set_float_limits(time_min,time_max);
}

/* ------------------ VolSmokeRolloutCB ------------------------ */

void VolSmokeRolloutCB(int var){
  GLUIToggleRollout(volsmokeprocinfo, nvolsmokeprocinfo, var);
}

/* ------------------ SmokeRolloutCB ------------------------ */

void SmokeRolloutCB(int var){
  if(var == VOLRENDER_ROLLOUT)smoke_render_option = RENDER_VOLUME;
  if(var == SLICERENDER_ROLLOUT)smoke_render_option = RENDER_SLICE;
//  GLUISmoke3dCB(SMOKE_OPTIONS);
  GLUIToggleRollout(smokeprocinfo, nsmokeprocinfo, var);
}

/* ------------------ GLUIDeleteVolTourList ------------------------ */

 extern "C" void GLUIDeleteVolTourList(void){
  int i;

  if(LISTBOX_VOL_tour==NULL)return;
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    LISTBOX_VOL_tour->delete_item(i);
  }
}

/* ------------------ GLUICreateVolTourList ------------------------ */

extern "C" void GLUICreateVolTourList(void){
  int i;

  if(LISTBOX_VOL_tour==NULL)return;
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    tourdata *touri;
    char label[1000];

    touri = global_scase.tourcoll.tourinfo + i;
    strcpy(label,"");
    if(i==selectedtour_index)strcat(label,"*");
    if(strlen(touri->label)>0)strcat(label,touri->label);
    if(strlen(label)>0){
      LISTBOX_VOL_tour->add_item(i,label);
    }
    else{
      LISTBOX_VOL_tour->add_item(i,"error");
    }
  }
  if(selectedtour_index>=TOURINDEX_MANUAL&&selectedtour_index<global_scase.tourcoll.ntourinfo){
    LISTBOX_VOL_tour->set_int_val(selectedtour_index);
  }
}

/* ------------------ UpdateCombineMeshes ------------------------ */

void UpdateCombineMeshes(void){
  if(CHECKBOX_combine_meshes!=NULL)CHECKBOX_combine_meshes->set_int_val(combine_meshes);
}

/* ------------------ GLUIUpdateSmoke3dFlags ------------------------ */

extern "C" void GLUIUpdateSmoke3dFlags(void){
#ifdef pp_GPU
  if(CHECKBOX_smokeGPU!=NULL)CHECKBOX_smokeGPU->set_int_val(usegpu);
#endif
  if(CHECKBOX_smokecullflag!=NULL)CHECKBOX_smokecullflag->set_int_val(smokecullflag);
  if(SPINNER_smoke3d_frame_inc!=NULL)SPINNER_smoke3d_frame_inc->set_int_val(smoke3d_frame_inc);
  GLUISmoke3dCB(VOL_SMOKE);
  glutPostRedisplay();
}

/* ------------------ GLUISmoke3dColorbarCB ------------------------ */

extern "C" void GLUISmoke3dColorbarCB(int var){
  int update_local = 0;

  switch(var){
  case GLOBAL_HRRPUV_MIN:
  case GLOBAL_HRRPUV_MAX:
    if(global_hrrpuv_cb_min < 0.0){
      global_hrrpuv_cb_min = 0.0;
      update_local = 1;
    }
    if(global_hrrpuv_cb_min > global_hrrpuv_max){
      global_hrrpuv_cb_min = global_hrrpuv_max;
      update_local = 1;
    }
    if(global_hrrpuv_cb_max < 0.0){
      global_hrrpuv_cb_max = 0.0;
      update_local = 1;
    }
    if(global_hrrpuv_cb_max > global_hrrpuv_max){
      global_hrrpuv_cb_max = global_hrrpuv_max;
      update_local = 1;
    }
    if(global_hrrpuv_cb_min > global_hrrpuv_cb_max){
      if(var == GLOBAL_HRRPUV_MIN)global_hrrpuv_cb_min = global_hrrpuv_cb_max - 1.0;
      if(var == GLOBAL_HRRPUV_MAX)global_hrrpuv_cb_max = global_hrrpuv_cb_min + 1.0;
      update_local = 1;
    }
    if(update_local == 1){
      SPINNER_hrrpuv_min->set_float_val(global_hrrpuv_cb_min);
      SPINNER_hrrpuv_max->set_float_val(global_hrrpuv_cb_max);
    }
    break;
  case GLOBAL_TEMP_MIN:
  case GLOBAL_TEMP_MAX:
    if(global_temp_cb_min < 0.0){
      global_temp_cb_min = 0.0;
      update_local = 1;
    }
    if(global_temp_cb_min > global_temp_max){
      global_temp_cb_min = global_temp_max;
      update_local = 1;
    }
    if(global_temp_cb_max < 0.0){
      global_temp_cb_max = 0.0;
      update_local = 1;
    }
    if(global_temp_cb_max > global_temp_max){
      global_temp_cb_max = global_temp_max;
      update_local = 1;
    }
    if(global_temp_cb_min > global_temp_cb_max){
      if(var == GLOBAL_TEMP_MIN)global_temp_cb_min = global_temp_cb_max - 1.0;
      if(var == GLOBAL_TEMP_MAX)global_temp_cb_max = global_temp_cb_min + 1.0;
      update_local = 1;
    }
    if(update_local == 1){
      SPINNER_temperature_min->set_float_val(global_temp_cb_min);
      SPINNER_temperature_max->set_float_val(global_temp_cb_max);
    }
    break;
  case COLORBAR_INDEX_MIN:
  case COLORBAR_INDEX_MAX:
    if(global_cb_min_index < 0){
      global_cb_min_index = 0;
      update_local = 1;
    }
    if(global_cb_min_index >255){
      global_cb_min_index = 255;
      update_local = 1;
    }
    if(global_cb_max_index < 0){
      global_cb_max_index = 0;
      update_local = 1;
    }
    if(global_cb_max_index >255){
      global_cb_max_index = 255;
      update_local = 1;
    }
    if(global_cb_min_index > global_cb_max_index){
      if(var == COLORBAR_INDEX_MIN)global_cb_min_index = global_cb_max_index;
      if(var == COLORBAR_INDEX_MAX)global_cb_max_index = global_cb_min_index;
      update_local = 1;
    }
    if(update_local == 1){
      SPINNER_cb_min_index->set_int_val(global_cb_min_index);
      SPINNER_cb_max_index->set_int_val(global_cb_max_index);
    }
    GLUISmoke3dColorbarCB(GLOBAL_TEMP_MIN);
    GLUISmoke3dColorbarCB(GLOBAL_TEMP_MAX);
    GLUISmoke3dColorbarCB(GLOBAL_HRRPUV_MIN);
    GLUISmoke3dColorbarCB(GLOBAL_HRRPUV_MAX);
    break;
  default:
    assert(0);
    break;
  }
  ForceIdle();
  UpdateSmokeColormap(smoke_render_option);
  glutPostRedisplay();
}

/* ------------------ GLUI3dSmokeSetup ------------------------ */

extern "C" void GLUI3dSmokeSetup(int main_window){
  int i;


  if(global_scase.smoke3dcoll.nsmoke3dinfo<=0&&nvolrenderinfo<=0)return;
  if(CHECKBOX_meshvisptr!=NULL)FREEMEMORY(CHECKBOX_meshvisptr);
  NewMemory((void **)&CHECKBOX_meshvisptr,global_scase.meshescoll.nmeshes*sizeof(GLUI_Checkbox *));

  glui_3dsmoke=glui_bounds;

  if(fire_colormap_type== FIRECOLOR_RGB){
    use_fire_rgb       = 1;
    use_fire_colormap  = 0;
  }
  else{
    use_fire_rgb       = 0;
    use_fire_colormap  = 1;
  }

  //-----------------------------------------------------------------------------------------------------------

  ROLLOUT_smoke_settings = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_smoke3d, "Settings", true, SMOKE_SETTINGS, SmokeRolloutCB);
  TOGGLE_ROLLOUT(smokeprocinfo, nsmokeprocinfo, ROLLOUT_smoke_settings, SMOKE_SETTINGS, glui_3dsmoke);

  PANEL_settings1 = glui_3dsmoke->add_panel_to_panel(ROLLOUT_smoke_settings, "");
#ifdef pp_GPU
  CHECKBOX_smokeGPU=glui_3dsmoke->add_checkbox_to_panel(PANEL_settings1,_("Use GPU"),&usegpu,VOL_SMOKE,GLUISmoke3dCB);
#endif
  glui_3dsmoke->add_checkbox_to_panel(PANEL_settings1, _("max blending"), &hrrpuv_max_blending);
  CHECKBOX_smoke_flip    = glui_3dsmoke->add_checkbox_to_panel(PANEL_settings1, _("flip background"), &background_flip,BACKGROUND_FLIP, GLUISmoke3dCB);
  CHECKBOX_triangle_display_rate = glui_3dsmoke->add_checkbox_to_panel(PANEL_settings1, _("triangle display rate"), &show_trirates);

  //---------------------------------------------Slice render settings--------------------------------------------------------------

  if(global_scase.smoke3dcoll.nsmoke3dinfo>0){
    glui_3dsmoke->add_column_to_panel(ROLLOUT_smoke_settings, false);
    ROLLOUT_slices = ROLLOUT_smoke_settings;

    if(have_smoke_density == 1){
      PANEL_load_options = glui_3dsmoke->add_panel_to_panel(ROLLOUT_slices, _("Load using"));
      RADIO_sootdensity = glui_3dsmoke->add_radiogroup_to_panel(PANEL_load_options,&load_smoke_density);
      glui_3dsmoke->add_radiobutton_to_group(RADIO_sootdensity,_("soot opacity"));
      glui_3dsmoke->add_radiobutton_to_group(RADIO_sootdensity,_("soot density"));
    }
#ifdef pp_GPU
    if(gpuactive==0){
      usegpu=0;
      CHECKBOX_smokeGPU->disable();
    }
#endif

    PANEL_display = glui_3dsmoke->add_panel_to_panel(ROLLOUT_slices, "Display using");
    SPINNER_smoke3d_frame_inc = glui_3dsmoke->add_spinner_to_panel(PANEL_display, _("frame display increment"),
      GLUI_SPINNER_INT, &smoke3d_frame_inc, SMOKE_FRAME_INC, GLUISmoke3dCB);

    SPINNER_smoke3d_extinct = glui_3dsmoke->add_spinner_to_panel(PANEL_display, _("Extinction (m2/kg)"),
                                                                 GLUI_SPINNER_FLOAT, &glui_smoke3d_extinct, SMOKE_EXTINCT, GLUISmoke3dCB);
  }

  //---------------------------------------------Smoke/fire color--------------------------------------------------------------

  ROLLOUT_smokecolor = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_smoke3d, _("Smoke color"),false, SMOKECOLOR_ROLLOUT, SmokeRolloutCB);
  TOGGLE_ROLLOUT(smokeprocinfo, nsmokeprocinfo, ROLLOUT_smokecolor, SMOKECOLOR_ROLLOUT, glui_3dsmoke);

  SPINNER_smoke3d_smoke_red   = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smokecolor, _("red"),   GLUI_SPINNER_INT, smoke_color_int255,   SMOKE_RED,   GLUISmoke3dCB);
  SPINNER_smoke3d_smoke_green = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smokecolor, _("green"), GLUI_SPINNER_INT, smoke_color_int255+1, SMOKE_GREEN, GLUISmoke3dCB);
  SPINNER_smoke3d_smoke_blue  = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smokecolor, _("blue"),  GLUI_SPINNER_INT, smoke_color_int255+2, SMOKE_BLUE,  GLUISmoke3dCB);
                                glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_smokecolor, "force gray", &force_gray_smoke, FORCE_GRAY, GLUISmoke3dCB);
  SPINNER_smoke3d_smoke_red->set_int_limits(0, 255);
  SPINNER_smoke3d_smoke_green->set_int_limits(0, 255);
  SPINNER_smoke3d_smoke_blue->set_int_limits(0, 255);

  if(colorbars.ncolorbars > 0){
    ROLLOUT_firecolor = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_smoke3d, _("Fire color"), false, FIRECOLOR_ROLLOUT, SmokeRolloutCB);
    TOGGLE_ROLLOUT(smokeprocinfo, nsmokeprocinfo, ROLLOUT_firecolor, FIRECOLOR_ROLLOUT, glui_3dsmoke);

    PANEL_specify_fire = glui_3dsmoke->add_panel_to_panel(ROLLOUT_firecolor, "Specify fire colors using");
    RADIO_use_fire_colormap = glui_3dsmoke->add_radiogroup_to_panel(PANEL_specify_fire,  &use_fire_colormap, USE_FIRE_COLORMAP, GLUISmoke3dCB);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_use_fire_colormap, "red,green,blue");
    glui_3dsmoke->add_radiobutton_to_group(RADIO_use_fire_colormap, "colorbar");
    glui_3dsmoke->add_checkbox_to_panel(PANEL_specify_fire, "Show fire colorbar", &show_smoke3d_colorbar, USE_FIRE_COLORMAP, GLUISmoke3dCB);

    PANEL_fire_rgb = glui_3dsmoke->add_panel_to_panel(PANEL_specify_fire, "", false);
    SPINNER_smoke3d_fire_red   = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_rgb, _("red"),   GLUI_SPINNER_INT, fire_color_int255,     FIRE_RED,   GLUISmoke3dCB);
    SPINNER_smoke3d_fire_green = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_rgb, _("green"), GLUI_SPINNER_INT, fire_color_int255 + 1, FIRE_GREEN, GLUISmoke3dCB);
    SPINNER_smoke3d_fire_blue  = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_rgb, _("blue"),  GLUI_SPINNER_INT, fire_color_int255 + 2, FIRE_BLUE,  GLUISmoke3dCB);
    SPINNER_smoke3d_fire_red->set_int_limits(0, 255);
    SPINNER_smoke3d_fire_green->set_int_limits(0, 255);
    SPINNER_smoke3d_fire_blue->set_int_limits(0, 255);

    PANEL_fire_colormap = glui_3dsmoke->add_panel_to_panel(PANEL_specify_fire, "", false);
    LISTBOX_smoke_colorbar = glui_3dsmoke->add_listbox_to_panel(PANEL_fire_colormap, "colorbar:", &colorbars.fire_colorbar_index, SMOKE_COLORBAR_LIST, GLUISmoke3dCB);
    for(i = 0;i < colorbars.ncolorbars;i++){
      colorbardata *cbi;

      cbi = colorbars.colorbarinfo + i;
      LISTBOX_smoke_colorbar->add_item(i, cbi->menu_label);
    }
    LISTBOX_smoke_colorbar->set_int_val(colorbars.fire_colorbar_index);
    glui_3dsmoke->add_column_to_panel(PANEL_fire_colormap,false);
    CHECKBOX_edit_colormap = glui_3dsmoke->add_checkbox_to_panel(PANEL_fire_colormap, "Edit", &show_firecolormap, SHOW_FIRECOLORMAP, GLUISmoke3dCB);
    GLUISmoke3dCB(USE_FIRE_COLORMAP);
  }

  PANEL_fire_cutoff = glui_3dsmoke->add_panel_to_panel(ROLLOUT_firecolor, "Colorbar fire bounds");
  PANEL_hrrpuv_minmax = glui_3dsmoke->add_panel_to_panel(PANEL_fire_cutoff, "HRRPUV (kW/m3)");
  SPINNER_hrrpuv_min = glui_3dsmoke->add_spinner_to_panel(PANEL_hrrpuv_minmax, "min", GLUI_SPINNER_FLOAT, &global_hrrpuv_cb_min, GLOBAL_HRRPUV_MIN, GLUISmoke3dColorbarCB);
  SPINNER_hrrpuv_max = glui_3dsmoke->add_spinner_to_panel(PANEL_hrrpuv_minmax, "max", GLUI_SPINNER_FLOAT, &global_hrrpuv_cb_max, GLOBAL_HRRPUV_MAX, GLUISmoke3dColorbarCB);
  {
    char temp_cutoff_label[300];

    //sprintf(temp_cutoff_label, "temperature (%s) > ", degC);
    snprintf(temp_cutoff_label, sizeof(temp_cutoff_label), "temperature (%s)", degC);
    PANEL_temp_minmax = glui_3dsmoke->add_panel_to_panel(PANEL_fire_cutoff, temp_cutoff_label);
    SPINNER_temperature_min = glui_3dsmoke->add_spinner_to_panel(PANEL_temp_minmax, "min", GLUI_SPINNER_FLOAT, &global_temp_cb_min, GLOBAL_TEMP_MIN, GLUISmoke3dColorbarCB);
    SPINNER_temperature_max = glui_3dsmoke->add_spinner_to_panel(PANEL_temp_minmax, "max", GLUI_SPINNER_FLOAT, &global_temp_cb_max, GLOBAL_TEMP_MAX, GLUISmoke3dColorbarCB);
  }
  glui_3dsmoke->add_column_to_panel(PANEL_fire_cutoff, false);
  PANEL_cb_index = glui_3dsmoke->add_panel_to_panel(PANEL_fire_cutoff, "colorbar bounds");
  SPINNER_cb_min_index = glui_3dsmoke->add_spinner_to_panel(PANEL_cb_index, "min", GLUI_SPINNER_INT, &global_cb_min_index, COLORBAR_INDEX_MIN, GLUISmoke3dColorbarCB);
  SPINNER_cb_max_index = glui_3dsmoke->add_spinner_to_panel(PANEL_cb_index, "max", GLUI_SPINNER_INT, &global_cb_max_index, COLORBAR_INDEX_MAX, GLUISmoke3dColorbarCB);

  glui_3dsmoke->add_button_to_panel(PANEL_fire_cutoff, "Refresh", REFRESH_FIRE, GLUISmoke3dCB);
  BUTTON_cutoff_defaults = glui_3dsmoke->add_button_to_panel(PANEL_fire_cutoff, "Reset", CUTOFF_RESET, GLUISmoke3dCB);

  //---------------------------------------------Smoke/fire opacity--------------------------------------------------------------

  ROLLOUT_opacity = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_smoke3d, "Smoke/fire opacity",false, FIREOPACITY_ROLLOUT, SmokeRolloutCB);
  TOGGLE_ROLLOUT(smokeprocinfo, nsmokeprocinfo, ROLLOUT_opacity, FIREOPACITY_ROLLOUT, glui_3dsmoke);

  PANEL_smoke_opacity = glui_3dsmoke->add_panel_to_panel(ROLLOUT_opacity, "smoke opacity");
  SPINNER_smoke3d_extinct2 = glui_3dsmoke->add_spinner_to_panel(PANEL_smoke_opacity, _("Extinction (m2/kg)"),
                                                                GLUI_SPINNER_FLOAT, &glui_smoke3d_extinct, SMOKE_EXTINCT, GLUISmoke3dCB);
  BUTTON_fds_extinction_reset = glui_3dsmoke->add_button_to_panel(PANEL_smoke_opacity, "Reset(fds extinct)", EXTINCTION_RESET_FDS, GLUISmoke3dCB);
  BUTTON_fds_extinction_reset = glui_3dsmoke->add_button_to_panel(PANEL_smoke_opacity, "Reset(saved extinct)", EXTINCTION_RESET_SMV, GLUISmoke3dCB);

  PANEL_fire_opacity = glui_3dsmoke->add_panel_to_panel(ROLLOUT_opacity, "fire opacity");
  glui_use_fire_alpha = 1-use_fire_alpha;
  if(glui_use_fire_alpha==0){
    use_opacity_depth      = 1;
    use_opacity_multiplier = 0;
  }
  else{
    use_opacity_depth      = 0;
    use_opacity_multiplier = 1;
  }
  CHECKBOX_use_opacity_depth = glui_3dsmoke->add_checkbox_to_panel(PANEL_fire_opacity, "set 50% opacity depth",
    &use_opacity_depth, USE_OPACITY_DEPTH, GLUISmoke3dCB);
  SPINNER_smoke3d_fire_halfdepth = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_opacity, "50% opacity at depth (m):", GLUI_SPINNER_FLOAT, &fire_halfdepth, UPDATE_SMOKEFIRE_COLORS, GLUISmoke3dCB);

  CHECKBOX_use_opacity_multiplier = glui_3dsmoke->add_checkbox_to_panel(PANEL_fire_opacity, "set opacity multiplier (when smoke also loaded)",
    &use_opacity_multiplier, USE_OPACITY_MULTIPLIER_CHECK, GLUISmoke3dCB);
  SPINNER_emission_factor = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_opacity, "opacity multiplier:", GLUI_SPINNER_FLOAT, &emission_factor, USE_FIRE_ALPHA, GLUISmoke3dCB);
  glui_3dsmoke->add_checkbox_to_panel(PANEL_fire_opacity, "off axis planes", &smoke_offaxis);
  glui_3dsmoke->add_checkbox_to_panel(PANEL_fire_opacity, "adjust opacities", &smoke_adjust);
  SPINNER_smoke3d_fire_halfdepth->set_float_limits(0.01, 100.0);
  CHECKBOX_force_alpha_opaque = glui_3dsmoke->add_checkbox_to_panel(PANEL_fire_opacity, "force opaque", &force_alpha_opaque, FORCE_ALPHA_OPAQUE, GLUISmoke3dCB);
  GLUISmoke3dCB(USE_OPACITY_DEPTH);

  if(active_smokesensors == 1){
    PANEL_smokesensor = glui_3dsmoke->add_panel_to_panel(ROLLOUT_opacity, _("Visibility"));
    RADIO_smokesensors = glui_3dsmoke->add_radiogroup_to_panel(PANEL_smokesensor, &show_smokesensors);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_smokesensors, _("Hidden"));                     // SMOKESENSORS_HIDDEN
    glui_3dsmoke->add_radiobutton_to_group(RADIO_smokesensors, _("Grey (0-255)"));               // SMOKESENSORS_0255
    glui_3dsmoke->add_radiobutton_to_group(RADIO_smokesensors, "I/I0 (0.0-1.0)");                 // SMOKESENSORS_01P0
    glui_3dsmoke->add_radiobutton_to_group(RADIO_smokesensors, _("Scaled optical depth (SCD)")); // SMOKESENSORS_SCALED
    glui_3dsmoke->add_statictext_to_panel(PANEL_smokesensor, "SCD=C/K=C*L/Ln(I/I0) (0-Inf)");     // SMOKESENSORS_0INF
    SPINNER_cvis = glui_3dsmoke->add_spinner_to_panel(PANEL_smokesensor, "C", GLUI_SPINNER_FLOAT, &smoke3d_cvis);
    SPINNER_cvis->set_float_limits(1.0, 20.0);
#ifdef _DEBUG
    CHECKBOX_test_smokesensors = glui_3dsmoke->add_checkbox_to_panel(PANEL_smokesensor, _("Test visibility sensor"), &test_smokesensors);
#endif
  }

  if(co2_colormap_type==0){
    use_co2_rgb      = 1;
    use_co2_colormap = 0;
  }
  else{
    use_co2_rgb      = 0;
    use_co2_colormap = 1;
  }

  //---------------------------------------------CO2 color--------------------------------------------------------------

  if(nsmoke3d_co2 > 0){
    ROLLOUT_co2color=glui_3dsmoke->add_rollout_to_panel(ROLLOUT_smoke3d, "CO2 color", false, CO2COLOR_ROLLOUT, SmokeRolloutCB);
    TOGGLE_ROLLOUT(smokeprocinfo, nsmokeprocinfo, ROLLOUT_co2color, CO2COLOR_ROLLOUT, glui_3dsmoke);
    CHECKBOX_use_co2_rgb = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_co2color, "set red/green/blue", &use_co2_rgb, USE_CO2_RGB, GLUISmoke3dCB);

    SPINNER_co2color[0] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("red"),   GLUI_SPINNER_INT, co2_color_int255,     CO2_COLOR, GLUISmoke3dCB);
    SPINNER_co2color[1] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("green"), GLUI_SPINNER_INT, co2_color_int255 + 1, CO2_COLOR, GLUISmoke3dCB);
    SPINNER_co2color[2] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("blue"),  GLUI_SPINNER_INT, co2_color_int255 + 2, CO2_COLOR, GLUISmoke3dCB);
    SPINNER_co2color[0]->set_int_limits(0, 255);
    SPINNER_co2color[1]->set_int_limits(0, 255);
    SPINNER_co2color[2]->set_int_limits(0, 255);
    if(colorbars.ncolorbars > 0){
      CHECKBOX_use_co2_colormap = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_co2color, "set colormap", &use_co2_colormap, USE_CO2_COLORMAP, GLUISmoke3dCB);
      LISTBOX_co2_colorbar = glui_3dsmoke->add_listbox_to_panel(ROLLOUT_co2color, "colormap:", &colorbars.co2_colorbar_index, CO2_COLORBAR_LIST, GLUISmoke3dCB);
      for(i = 0; i < colorbars.ncolorbars; i++){
        colorbardata *cbi;

        cbi = colorbars.colorbarinfo+i;
        LISTBOX_co2_colorbar->add_item(i, cbi->menu_label);
      }
      LISTBOX_co2_colorbar->set_int_val(colorbars.co2_colorbar_index);
    }

    if(nco2files > 0){
      SPINNER_smoke3d_co2_halfdepth = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("50% CO2 opacity at depth (m)"), GLUI_SPINNER_FLOAT, &co2_halfdepth, UPDATE_CO2_COLORS, GLUISmoke3dCB);
    }
  }

  GLUISmoke3dCB(USE_FIRE_RGB);
  GLUISmoke3dCB(USE_FIRE_COLORMAP);
  GLUISmoke3dCB(USE_CO2_RGB);
  GLUISmoke3dCB(USE_CO2_COLORMAP);
  GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS);
  GLUISmoke3dCB(UPDATE_CO2_COLORS);
  GLUISmoke3dCB(USE_SMOKE_RGB);

  if(global_scase.smoke3dcoll.nsmoke3dinfo<=0||nvolrenderinfo<=0){
    smoke_render_option=RENDER_SLICE;
    if(global_scase.smoke3dcoll.nsmoke3dinfo>0)smoke_render_option=RENDER_SLICE;
    if(nvolrenderinfo>0)smoke_render_option=RENDER_VOLUME;
  }

  //---------------------------------------------Skip planes--------------------------------------------------------------

  ROLLOUT_skip = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_smoke3d, "Skip planes", false, SKIP_ROLLOUT, SmokeRolloutCB);
  TOGGLE_ROLLOUT(smokeprocinfo, nsmokeprocinfo, ROLLOUT_skip, SKIP_ROLLOUT, glui_3dsmoke);

  PANEL_skip_planes = glui_3dsmoke->add_panel_to_panel(ROLLOUT_skip, "", GLUI_PANEL_NONE);
  SPINNER_smoke3d_skip   = glui_3dsmoke->add_spinner_to_panel(PANEL_skip_planes, "x/y/z", GLUI_SPINNER_INT, &smoke3d_skip,   SMOKE_SKIP_XYZ, GLUISmoke3dCB);
  SPINNER_smoke3d_skipxy = glui_3dsmoke->add_spinner_to_panel(PANEL_skip_planes, "x/y",   GLUI_SPINNER_INT, &smoke3d_skipxy, SMOKE_SKIP_XY,  GLUISmoke3dCB);
  SPINNER_smoke3d_skipx  = glui_3dsmoke->add_spinner_to_panel(PANEL_skip_planes, "x",     GLUI_SPINNER_INT, &smoke3d_skipx,  SMOKE_SKIP_X,   GLUISmoke3dCB);
  SPINNER_smoke3d_skipy  = glui_3dsmoke->add_spinner_to_panel(PANEL_skip_planes, "y",     GLUI_SPINNER_INT, &smoke3d_skipy,  SMOKE_SKIP_Y,   GLUISmoke3dCB);
  SPINNER_smoke3d_skipz  = glui_3dsmoke->add_spinner_to_panel(PANEL_skip_planes, "z",     GLUI_SPINNER_INT, &smoke3d_skipz,  SMOKE_SKIP_Z,   GLUISmoke3dCB);
  char label[256];
  strcpy(label, "");
  STATIC_pixels_per_triangle = glui_3dsmoke->add_statictext_to_panel(PANEL_skip_planes, label);

  SPINNER_smoke3d_kmax = glui_3dsmoke->add_spinner_to_panel(PANEL_skip_planes, "max k", GLUI_SPINNER_INT, &smoke3d_kmax);
  CHECKBOX_smokecullflag = glui_3dsmoke->add_checkbox_to_panel(PANEL_skip_planes, "Cull hidden planes", &smokecullflag);
  GLUISmoke3dCB(SMOKE_SKIP_X);

  //---------------------------------------------Volume render settings--------------------------------------------------------------

  if(nvolrenderinfo > 0){
    ROLLOUT_volume = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_smoke3d, _("Volume render settings"), false, VOLRENDER_ROLLOUT, SmokeRolloutCB);
    TOGGLE_ROLLOUT(smokeprocinfo, nsmokeprocinfo, ROLLOUT_volume, VOLRENDER_ROLLOUT, glui_3dsmoke);

    //*** display

    ROLLOUT_voldisplay = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume,_("Display"),false, VOLSMOKE_DISPLAY, VolSmokeRolloutCB);
    TOGGLE_ROLLOUT(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_voldisplay, VOLSMOKE_DISPLAY, glui_3dsmoke);

    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_voldisplay, _("Show"), &usevolrender, VOL_SMOKE, GLUISmoke3dCB);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_voldisplay, _("black/white"), &volbw);
#ifdef _DEBUG
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_voldisplay, "block smoke", &block_volsmoke);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_voldisplay, "debug", &smoke3dVoldebug);
#endif
    PANEL_blackbody = glui_3dsmoke->add_panel_to_panel(ROLLOUT_voldisplay, "Black body colors");
    SPINNER_fire_temp_min = glui_3dsmoke->add_spinner_to_panel(PANEL_blackbody, "min temperature", GLUI_SPINNER_FLOAT, &fire_temp_min, BLACKBODY_TEMPS, GLUISmoke3dCB);
    SPINNER_fire_temp_max = glui_3dsmoke->add_spinner_to_panel(PANEL_blackbody, "max temperature", GLUI_SPINNER_FLOAT, &fire_temp_max, BLACKBODY_TEMPS, GLUISmoke3dCB);
    SPINNER_nfire_colors  = glui_3dsmoke->add_spinner_to_panel(PANEL_blackbody, "n temperatures",  GLUI_SPINNER_INT,   &nfire_colors,  BLACKBODY_TEMPS, GLUISmoke3dCB);
    glui_3dsmoke->add_checkbox_to_panel(PANEL_blackbody, "show blackbody colormap", &show_blackbody_colormap);
    glui_3dsmoke->add_checkbox_to_panel(PANEL_blackbody, "use blackbody colors", &use_blackbody_colors);
    ROLLOUT_voltest = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_voldisplay, "volrender test data");
    ROLLOUT_voltest->close();
    glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "temp 1",  GLUI_SPINNER_FLOAT, &voltest_temp1);
    glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "temp 2",  GLUI_SPINNER_FLOAT, &voltest_temp2);
    SPINNER_voltest_depth1 = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "depth 1", GLUI_SPINNER_FLOAT, &voltest_depth1, VOLTEST_DEPTH, GLUISmoke3dCB);
    SPINNER_voltest_depth2 = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "depth 2", GLUI_SPINNER_FLOAT, &voltest_depth2, VOLTEST_DEPTH, GLUISmoke3dCB);
    glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "rad 1",   GLUI_SPINNER_FLOAT, &voltest_r1);
    glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "rad 2",   GLUI_SPINNER_FLOAT, &voltest_r2);
    glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "rad 3",   GLUI_SPINNER_FLOAT, &voltest_r3);
    glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "x",       GLUI_SPINNER_FLOAT, voltest_center + 0);
    glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "y",       GLUI_SPINNER_FLOAT, voltest_center + 1);
    glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltest, "z",       GLUI_SPINNER_FLOAT, voltest_center + 2);
    glui_3dsmoke->add_button_to_panel(ROLLOUT_voltest, "Update volume render test data", VOLTEST_UPDATE, GLUISmoke3dCB);
    SPINNER_voltest_depth1->set_float_limits(0.01, 20.0);
    SPINNER_voltest_depth2->set_float_limits(0.01, 20.0);
    GLUISmoke3dCB(VOLTEST_DEPTH);

    //*** scene movement

    ROLLOUT_volsmoke_move = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Scene movement"), false, VOLSMOKE_MOVEMENT, VolSmokeRolloutCB);
    TOGGLE_ROLLOUT(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_volsmoke_move, VOLSMOKE_MOVEMENT, glui_3dsmoke);

    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_move, _("Auto freeze"), &autofreeze_volsmoke);
    CHECKBOX_freeze = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_move, _("Freeze"), &freeze_volsmoke);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_move, _("Show data while moving scene"), &show_volsmoke_moving);

    //*** compute

    ROLLOUT_volsmoke_compute = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Compute"), false, VOLSMOKE_COMPUTE_ROLLOUT, VolSmokeRolloutCB);
    TOGGLE_ROLLOUT(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_volsmoke_compute, VOLSMOKE_COMPUTE_ROLLOUT, glui_3dsmoke);

    SPINNER_fire_opacity_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("Fire opacity multiplier"), GLUI_SPINNER_FLOAT, &fire_opacity_factor);
    SPINNER_fire_opacity_factor->set_float_limits(1.0, 50.0);
    SPINNER_mass_extinct = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("Mass extinction coeff (m2/g)"), GLUI_SPINNER_FLOAT, &mass_extinct, MASS_EXTINCTION, GLUISmoke3dCB);
    GLUISmoke3dCB(MASS_EXTINCTION);
    CHECKBOX_combine_meshes = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_compute, _("Combine meshes"), &combine_meshes, COMBINE_MESHES, GLUISmoke3dCB);
    SPINNER_nongpu_vol_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("non-gpu grid multiplier"), GLUI_SPINNER_FLOAT, &nongpu_vol_factor, NONGPU_VOL_FACTOR, GLUISmoke3dCB);
    SPINNER_nongpu_vol_factor->set_float_limits(1.0, 10.0);
    SPINNER_gpu_vol_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("gpu grid multiplier"), GLUI_SPINNER_FLOAT, &gpu_vol_factor, GPU_VOL_FACTOR, GLUISmoke3dCB);
    SPINNER_gpu_vol_factor->set_float_limits(1.0, 10.0);

    //*** load

    ROLLOUT_volsmoke_load = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Load"), false, VOLSMOKE_LOAD_ROLLOUT, VolSmokeRolloutCB);
    TOGGLE_ROLLOUT(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_volsmoke_load, VOLSMOKE_LOAD_ROLLOUT, glui_3dsmoke);

    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_load, _("Load data in background"), &use_multi_threading);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_load, _("Load data only at render times"), &load_at_rendertimes);

    //*** generate images

    ROLLOUT_generate_images = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Generate images"), false, VOLSMOKE_IMAGES_ROLLOUT, VolSmokeRolloutCB);
    TOGGLE_ROLLOUT(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_generate_images, VOLSMOKE_IMAGES_ROLLOUT, glui_3dsmoke);

    SPINNER_startframe = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_generate_images, _("start frame"), GLUI_SPINNER_INT, &vol_startframe0, START_FRAME, GLUISmoke3dCB);
    SPINNER_skipframe = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_generate_images, _("skip frame"), GLUI_SPINNER_INT, &vol_skipframe0, SKIP_FRAME, GLUISmoke3dCB);
    GLUISmoke3dCB(START_FRAME);
    GLUISmoke3dCB(SKIP_FRAME);
    if(global_scase.tourcoll.ntourinfo > 0){
      selectedtour_index = TOURINDEX_MANUAL;
      selectedtour_index_old = TOURINDEX_MANUAL;
      LISTBOX_VOL_tour = glui_3dsmoke->add_listbox_to_panel(ROLLOUT_generate_images, "Tour:", &selectedtour_index, VOL_TOUR_LIST, GLUISmoke3dCB);

      LISTBOX_VOL_tour->add_item(TOURINDEX_MANUAL, "Manual");
      LISTBOX_VOL_tour->add_item(-999, "-");
      for(i = 0; i < global_scase.tourcoll.ntourinfo; i++){
        tourdata *touri;

        touri = global_scase.tourcoll.tourinfo + i;
        LISTBOX_VOL_tour->add_item(i, touri->label);
      }
      LISTBOX_VOL_tour->set_int_val(selectedtour_index);
    }

    strcpy(vol_prefix, global_scase.fdsprefix);
    EDIT_vol_prefix = glui_3dsmoke->add_edittext_to_panel(ROLLOUT_generate_images, "image prefix:", GLUI_EDITTEXT_TEXT, vol_prefix, VOL_PREFIX, GLUISmoke3dCB);
    EDIT_vol_prefix->set_w(200);

    BUTTON_startrender = glui_3dsmoke->add_button_to_panel(ROLLOUT_generate_images, _("Generate images"), GENERATE_IMAGES, GLUISmoke3dCB);
    BUTTON_cancelrender = glui_3dsmoke->add_button_to_panel(ROLLOUT_generate_images, _("Cancel"), CANCEL_GENERATE_IMAGES, GLUISmoke3dCB);
    BUTTON_volunload = glui_3dsmoke->add_button_to_panel(ROLLOUT_generate_images, _("Unload"), VOL_UNLOAD_ALL, GLUISmoke3dCB);
    UpdateSmokeColormap(RENDER_VOLUME);
    GLUISmoke3dCB(SMOKE_OPTIONS);

    ROLLOUT_loadframe = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Load frame"), false, VOLSMOKE_LOADFRAME_ROLLOUT, VolSmokeRolloutCB);
    TOGGLE_ROLLOUT(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_loadframe, VOLSMOKE_LOADFRAME_ROLLOUT, glui_3dsmoke);
    PANEL_loadframe = glui_3dsmoke->add_panel_to_panel(ROLLOUT_loadframe, "", false);

    glui_3dsmoke->add_button_to_panel(PANEL_loadframe, _("Load"), LOAD_SMOKEFRAME, GLUISmoke3dCB);
    glui_3dsmoke->add_button_to_panel(PANEL_loadframe, _("Load"), LOAD_TIMEFRAME, GLUISmoke3dCB);

    glui_3dsmoke->add_column_to_panel(PANEL_loadframe, false);

    SPINNER_smokeloadframe = glui_3dsmoke->add_spinner_to_panel(PANEL_loadframe, _("smoke frame"), GLUI_SPINNER_INT, &smoke_framenumber);
    SPINNER_timeloadframe = glui_3dsmoke->add_spinner_to_panel(PANEL_loadframe, _("time"), GLUI_SPINNER_FLOAT, &time_frameval);
  }

#ifdef pp_GPU
  GLUISmoke3dCB(VOL_SMOKE);
#endif
  GLUISmoke3dCB(SMOKE_OPTIONS);
  GLUISmoke3dCB(FIRECOLORMAP_TYPE);
  if(nsmoke3d_co2>0)GLUISmoke3dCB(CO2COLORMAP_TYPE);
}

/* ------------------ SetRGBColorMapVars ------------------------ */

void SetRGBColorMapVars(int use_rgb){
  int use_colormap;

  use_colormap       = 1 - use_rgb;
  use_fire_rgb       = use_rgb;
  use_fire_colormap  = use_colormap;
  use_co2_rgb        = use_rgb;
  use_co2_colormap   = use_colormap;
  GLUISmoke3dCB(ENABLE_DISABLE_FIRE);
  GLUISmoke3dCB(SET_RGB_COLORGB_CHECKBOXES);
  if(use_fire_rgb==1)fire_colormap_type      = FIRECOLOR_RGB;
  if(use_fire_colormap==1)fire_colormap_type = FIRECOLOR_COLORBAR;
  GLUISmoke3dCB(FIRECOLORMAP_TYPE);
  if(use_co2_rgb==1)co2_colormap_type      = CO2_RGB;
  if(use_co2_colormap==1)co2_colormap_type = CO2_COLORBAR;
  GLUISmoke3dCB(CO2COLORMAP_TYPE);
}

/* ------------------ GLUIGetPixelsPerTriangle ------------------------ */

void GLUIGetPixelsPerTriangle(void){
  float x_pixels_per_triangle=1000000.0, y_pixels_per_triangle=1000000.0, pixels_per_triangle;
  char label[500];

  if(STATIC_pixels_per_triangle == NULL)return;
  if(nplotx_all>0)x_pixels_per_triangle = smoke3d_skipx*(float)glui_screenWidth/(float)nplotx_all;
  if(nploty_all>0)y_pixels_per_triangle = smoke3d_skipy*(float)glui_screenHeight/(float)nploty_all;
  pixels_per_triangle = MIN(x_pixels_per_triangle, y_pixels_per_triangle);
  if(pixels_per_triangle>10.0){
    sprintf(label, "triangle/pixel size ratio: %.0f", pixels_per_triangle);
  }
  else if(pixels_per_triangle>1.0){
    sprintf(label, "triangle/pixel size ratio: %.1f", pixels_per_triangle);
  }
  else{
    sprintf(label, "triangle/pixel size ratio: %.2f", pixels_per_triangle);
  }
  STATIC_pixels_per_triangle->set_text(label);
}

/* ------------------ GLUISmoke3dCB ------------------------ */

extern "C" void GLUISmoke3dCB(int var){
  int i;
  char *tour_label;
  char *vol_prefixptr;

  updatemenu=1;
  switch(var){
  case FORCE_ALPHA_OPAQUE:
    global_scase.update_smoke_alphas = 1;
    updatemenu = 1;
    break;
  case USE_FIRE_ALPHA:
    use_fire_alpha = 1-glui_use_fire_alpha;
    if(have_fire!=NO_FIRE&&have_smoke==NO_SMOKE){
      SPINNER_smoke3d_fire_halfdepth->enable();
      SPINNER_emission_factor->disable();
      CHECKBOX_use_opacity_multiplier->disable();
    }
    else{
      SPINNER_smoke3d_fire_halfdepth->enable();
      SPINNER_emission_factor->enable();
      CHECKBOX_use_opacity_multiplier->enable();
    }
    if(emission_factor < 1.0){
      emission_factor = 1.0;
      SPINNER_emission_factor->set_float_val(emission_factor);
    }
    GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    glutPostRedisplay();
    break;
  case USE_OPACITY_DEPTH_CHECK:
    use_opacity_ini = 0;
    GLUISmoke3dCB(USE_OPACITY_DEPTH);
    break;
  case USE_OPACITY_DEPTH:
    if(have_fire!=NO_FIRE&&have_smoke==NO_SMOKE){
      use_opacity_depth      = 1;
      use_opacity_multiplier = 0;
    }
    glui_use_fire_alpha = 1 - use_opacity_depth;
    if(have_smoke!=NO_SMOKE&&have_fire==NO_FIRE){
        use_opacity_multiplier = 0;
    }
    else{
      use_opacity_multiplier = 1 - use_opacity_depth;
    }
    CHECKBOX_use_opacity_depth->set_int_val(use_opacity_depth);
    CHECKBOX_use_opacity_multiplier->set_int_val(use_opacity_multiplier);
    GLUISmoke3dCB(USE_FIRE_ALPHA);
    break;
  case USE_OPACITY_MULTIPLIER_CHECK:
    use_opacity_ini = 0;
    GLUISmoke3dCB(USE_OPACITY_MULTIPLIER);
    break;
  case USE_OPACITY_MULTIPLIER:
    if(have_fire!=NO_FIRE&&have_smoke==NO_SMOKE){
      use_opacity_depth      = 1;
      use_opacity_multiplier = 0;
    }
    if(have_fire != NO_FIRE && have_smoke != NO_SMOKE){
      if(use_opacity_depth_ini == 1 && use_opacity_ini == 1){
        use_opacity_depth = 1;
        use_opacity_multiplier = 0;
      }
      else{
        use_opacity_depth = 0;
        use_opacity_multiplier = 1;
      }
    }
    glui_use_fire_alpha = use_opacity_multiplier;
    use_opacity_depth =  1 - use_opacity_multiplier;
    CHECKBOX_use_opacity_depth->set_int_val(use_opacity_depth);
    CHECKBOX_use_opacity_multiplier->set_int_val(use_opacity_multiplier);
    GLUISmoke3dCB(USE_FIRE_ALPHA);
    break;
  case BACKGROUND_FLIP:
    background_flip = 1-background_flip;
    ShowHideMenu(MENU_SHOWHIDE_FLIP);
    updatemenu = 1;
    break;
  case SMOKE_BLACK:
    break;
  case SMOKE_SKIP_XYZ:
    if(smoke3d_skip<1)smoke3d_skip = 1;
    SPINNER_smoke3d_skip->set_int_val(smoke3d_skip);
    smoke3d_skipx = smoke3d_skip;
    smoke3d_skipy = smoke3d_skip;
    smoke3d_skipz = smoke3d_skip;
    SPINNER_smoke3d_skipx->set_int_val(smoke3d_skipx);
    SPINNER_smoke3d_skipy->set_int_val(smoke3d_skipy);
    SPINNER_smoke3d_skipz->set_int_val(smoke3d_skipz);
    GLUIGetPixelsPerTriangle();
    break;
  case SMOKE_SKIP_XY:
    if(smoke3d_skipxy<1)smoke3d_skipxy = 1;
    SPINNER_smoke3d_skipxy->set_int_val(smoke3d_skipxy);
    smoke3d_skipx = smoke3d_skipxy;
    smoke3d_skipy = smoke3d_skipxy;
    SPINNER_smoke3d_skipx->set_int_val(smoke3d_skipx);
    SPINNER_smoke3d_skipy->set_int_val(smoke3d_skipy);
    GLUIGetPixelsPerTriangle();
    break;
  case SMOKE_SKIP_X:
  case SMOKE_SKIP_Y:
  case SMOKE_SKIP_Z:
    if(smoke3d_skipx<1)smoke3d_skipx = 1;
    if(smoke3d_skipy<1)smoke3d_skipy = 1;
    if(smoke3d_skipz<1)smoke3d_skipz = 1;
    SPINNER_smoke3d_skipx->set_int_val(smoke3d_skipx);
    SPINNER_smoke3d_skipy->set_int_val(smoke3d_skipy);
    SPINNER_smoke3d_skipz->set_int_val(smoke3d_skipz);
    if(smoke3d_skipx==smoke3d_skipy&&smoke3d_skipx==smoke3d_skipz){
      smoke3d_skip = smoke3d_skipx;
      SPINNER_smoke3d_skip->set_int_val(smoke3d_skip);
    }
    GLUIGetPixelsPerTriangle();
    break;
  case CO2SMOKE:
    UpdateCO2Colormap();
    GLUISmoke3dCB(CO2COLORMAP_TYPE);
    GLUISmoke3dCB(UPDATE_SMOKECOLORS);
    glutPostRedisplay();
    break;
  case LOAD_SMOKEFRAME:
    LoadSmokeFrame(-1, smoke_framenumber);
    break;
  case LOAD_TIMEFRAME:
    LoadTimeFrame(-1, time_frameval);
    break;
  case VOL_UNLOAD_ALL:
    UnLoadVolsmoke3DMenu(-1);
    break;
  case VOL_PREFIX:
    break;
  case VOL_TOUR_LIST:
    TourCB(TOUR_LIST);
    break;
  case START_FRAME:
    if(vol_startframe0<0){
      vol_startframe0=0;
      SPINNER_startframe->set_int_val(vol_startframe0);
    }
    break;
  case SKIP_FRAME:
    if(vol_skipframe0<1){
      vol_skipframe0=1;
      SPINNER_skipframe->set_int_val(vol_skipframe0);
    }
    break;
  case CANCEL_GENERATE_IMAGES:
    ScriptCB(SCRIPT_CANCEL_NOW);
    break;
  case GENERATE_IMAGES:
    if(selected_tour==NULL){
      tour_label=NULL;
    }
    else{
      tour_label=selected_tour->label;
    }
    TrimBack(vol_prefix);
    vol_prefixptr=TrimFront(vol_prefix);
    if(strlen(vol_prefixptr)==0)vol_prefixptr=global_scase.fdsprefix;
    InitVolrenderScript(vol_prefixptr, tour_label, vol_startframe0, vol_skipframe0);
    break;
  case NONGPU_VOL_FACTOR:
    InitVolRenderSurface(NOT_FIRSTCALL);
    break;
  case MASS_EXTINCTION:
    if(mass_extinct<1.0){
      mass_extinct = 1.0;
      SPINNER_mass_extinct->set_float_val(mass_extinct);
    }
    break;
  case GPU_VOL_FACTOR:
    break;
  case COMBINE_MESHES:
    DefineVolsmokeTextures();
    break;
  case SHOW_FIRECOLORMAP:
    UpdateSmokeColormap(smoke_render_option);
    if(show_firecolormap!=0){
      GLUIShowColorbar();
    }
    else{
      GLUIHideColorbar();
    }
    break;
  case TEMP_MIN:
    if(global_temp_min < 0.0){
      global_temp_min = 0.0;
      SPINNER_temperature_min->set_float_val(global_temp_min);
    }
    if(global_temp_cb_max<global_temp_min){
      global_temp_cb_max = global_temp_min+1.0;
      SPINNER_temperature_max->set_float_val(global_temp_cb_max);
    }
    UpdateSmokeColormap(smoke_render_option);
    break;
  case EXTINCTION_RESET_FDS:
    if(SOOT_index>=0){
      glui_smoke3d_extinct = global_scase.smoke3dcoll.smoke3dtypes[SOOT_index].extinction;
      if(SPINNER_smoke3d_extinct2!=NULL)SPINNER_smoke3d_extinct2->set_float_val(glui_smoke3d_extinct);
      if(SPINNER_smoke3d_extinct!=NULL)SPINNER_smoke3d_extinct->set_float_val(glui_smoke3d_extinct);
      GLUISmoke3dCB(SMOKE_EXTINCT);
    }
    break;
  case EXTINCTION_RESET_SMV:
    glui_smoke3d_extinct = glui_smoke3d_extinct_default;
    if(SPINNER_smoke3d_extinct2!=NULL)SPINNER_smoke3d_extinct2->set_float_val(glui_smoke3d_extinct);
    if(SPINNER_smoke3d_extinct!=NULL)SPINNER_smoke3d_extinct->set_float_val(glui_smoke3d_extinct);
    GLUISmoke3dCB(SMOKE_EXTINCT);
    break;
  case CUTOFF_RESET:
    global_hrrpuv_cb_min = global_hrrpuv_cb_min_default;
    global_hrrpuv_cb_max = global_hrrpuv_cb_max_default;
    global_temp_cb_min   = global_temp_cb_min_default;
    global_temp_cb_max   = global_temp_cb_max_default;
    global_cb_min_index  = global_cb_min_index_default;
    global_cb_max_index  = global_cb_max_index_default;
    SPINNER_hrrpuv_min->set_float_val(global_hrrpuv_cb_min);
    SPINNER_hrrpuv_max->set_float_val(global_hrrpuv_cb_max);
    SPINNER_temperature_min->set_float_val(global_temp_cb_min);
    SPINNER_temperature_max->set_float_val(global_temp_cb_max);
    SPINNER_cb_min_index->set_int_val(global_cb_min_index);
    SPINNER_cb_max_index->set_int_val(global_cb_max_index);
    break;
  case VOLTEST_DEPTH:
    voltest_soot1 = log(2.0)/(mass_extinct*voltest_depth1);
    voltest_soot2 = log(2.0)/(mass_extinct*voltest_depth2);
    break;
  case VOLTEST_UPDATE:
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + i;
      if(meshi->volrenderinfo->loaded == 0)continue;
      meshi->voltest_update = 1;
    }
    break;
  case BLACKBODY_TEMPS:
    if(nfire_colors<256){
      nfire_colors = 256;
      SPINNER_nfire_colors->set_int_val(nfire_colors);
    }
    if(nfire_colors>4096){
      nfire_colors = 4096;
      SPINNER_nfire_colors->set_int_val(nfire_colors);
    }
    if(fire_temp_min<20.0){
      fire_temp_min = 20.0;
      SPINNER_fire_temp_min->set_float_val(fire_temp_min);
    }
    if(fire_temp_min>fire_temp_max){
      fire_temp_max = fire_temp_min + 1500.0;
      SPINNER_fire_temp_min->set_float_val(fire_temp_min);
      SPINNER_fire_temp_max->set_float_val(fire_temp_max);
    }
    MakeFireColors(fire_temp_min, fire_temp_max, nfire_colors);
    break;
  case SMOKE_OPTIONS:
    if(nsmoke3d_temp==0&&smoke_render_option==RENDER_SLICE){
      fire_colormap_type=fire_colormap_type_save;
    }
    else{
      fire_colormap_type_save = fire_colormap_type;
      fire_colormap_type      = FIRECOLOR_COLORBAR;
    }
    GLUISmoke3dCB(SET_RGB_COLORGB_CHECKBOXES);
    GLUISmoke3dCB(FIRECOLORMAP_TYPE);
    break;
  case CO2COLORMAP_TYPE:
    switch(co2_colormap_type){
    case CO2_RGB:
      break;
    case CO2_COLORBAR:
      break;
    default:
#ifdef _DEBUG
      abort();
#endif
      break;
    }
    UpdateCO2Colormap();
    GLUISmoke3dCB(UPDATE_SMOKECOLORS);
    break;
  case SET_RGB_COLORGB_CHECKBOXES:
    if(RADIO_use_fire_colormap!=NULL)RADIO_use_fire_colormap->set_int_val(use_fire_colormap);
    if(CHECKBOX_use_co2_rgb!=NULL)CHECKBOX_use_co2_rgb->set_int_val(use_co2_rgb);
    if(CHECKBOX_use_co2_colormap!=NULL)CHECKBOX_use_co2_colormap->set_int_val(use_co2_colormap);

    if(nsmoke3d_co2>0){
      if(use_co2_colormap==1){
        SPINNER_co2color[0]->disable();
        SPINNER_co2color[1]->disable();
        SPINNER_co2color[2]->disable();
        LISTBOX_co2_colorbar->enable();
      }
      else{
        SPINNER_co2color[0]->enable();
        SPINNER_co2color[1]->enable();
        SPINNER_co2color[2]->enable();
        LISTBOX_co2_colorbar->disable();
      }
    }
    break;
  case USE_SMOKE_RGB:
    SetRGBColorMapVars(1);
    break;
  case USE_FIRE_RGB:
    SetRGBColorMapVars(use_fire_rgb);
    break;
  case ENABLE_DISABLE_FIRE:
    if(use_fire_colormap == 1){
      if(PANEL_fire_rgb != NULL)PANEL_fire_rgb->disable();
      if(PANEL_fire_colormap != NULL)PANEL_fire_colormap->enable();
    }
    else{
      if(PANEL_fire_rgb != NULL)PANEL_fire_rgb->enable();
      if(PANEL_fire_colormap != NULL)PANEL_fire_colormap->disable();
    }
    break;
  case USE_FIRE_COLORMAP:
    SetRGBColorMapVars(1 - use_fire_colormap);
    GLUISmoke3dCB(ENABLE_DISABLE_FIRE);
    break;
  case USE_CO2_RGB:
    SetRGBColorMapVars(use_co2_rgb);
    break;
  case USE_CO2_COLORMAP:
    SetRGBColorMapVars(1 - use_co2_colormap);
    break;
  case FIRECOLORMAP_TYPE:
    GLUISmoke3dCB(SET_RGB_COLORGB_CHECKBOXES);
    switch(fire_colormap_type){
    case FIRECOLOR_COLORBAR:
      if(fire_colorbar_index_save != -1){
        SmokeColorbarMenu(fire_colorbar_index_save);
      }
      else{
        SmokeColorbarMenu(colorbars.fire_colorbar_index);
      }
      break;
    case FIRECOLOR_RGB:
      fire_colorbar_index_save = colorbars.fire_colorbar_index;
      UpdateRGBColors(colorbar_select_index);
      UpdateSmokeColormap(smoke_render_option);
      break;
    default:
#ifdef _DEBUG
      abort();
#endif
      break;
    }
    if(LISTBOX_smoke_colorbar->get_int_val()!=colorbars.fire_colorbar_index){
      LISTBOX_smoke_colorbar->set_int_val(colorbars.fire_colorbar_index);
    }
    UpdateSmokeColormap(smoke_render_option);
    break;
  case SMOKE_COLORBAR_LIST:
    SmokeColorbarMenu(colorbars.fire_colorbar_index);
    UpdateSmokeColormap(smoke_render_option);
    updatemenu=1;
    break;
  case CO2_COLORBAR_LIST:
    UpdateCO2Colormap();
    GLUISmoke3dCB(UPDATE_SMOKECOLORS);
    break;
  case SAVE_SETTINGS_SMOKE:
    WriteIni(LOCAL_INI,NULL);
    break;
  case REFRESH_FIRE:
    ForceIdle();
    break;
  case UPDATE_SMOKEFIRE_COLORS:
    fire_halfdepth = MAX(fire_halfdepth, 0.001);
    SPINNER_smoke3d_fire_halfdepth->set_float_val(fire_halfdepth);
    GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    break;
  case UPDATE_CO2_COLORS:
    if(SPINNER_smoke3d_co2_halfdepth!=NULL){
      co2_halfdepth = MAX(co2_halfdepth, 0.001);
      SPINNER_smoke3d_co2_halfdepth->set_float_val(co2_halfdepth);
      GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    }
    break;
  case CO2_COLOR:
    GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    UpdateCO2Colormap();
    break;
  case FORCE_GRAY:
  case SMOKE_RED:
  case SMOKE_GREEN:
  case SMOKE_BLUE:
    if(force_gray_smoke==1){
      int smoke_gray_level;

      smoke_gray_level = smoke_color_int255[0];
      if(var==SMOKE_GREEN)smoke_gray_level = smoke_color_int255[1];
      if(var==SMOKE_BLUE)smoke_gray_level = smoke_color_int255[2];
      smoke_color_int255[0] = smoke_gray_level;
      smoke_color_int255[1] = smoke_gray_level;
      smoke_color_int255[2] = smoke_gray_level;
      SPINNER_smoke3d_smoke_red->set_float_val(smoke_gray_level);
      SPINNER_smoke3d_smoke_green->set_float_val(smoke_gray_level);
      SPINNER_smoke3d_smoke_blue->set_float_val(smoke_gray_level);
    }
    GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    break;
  case FIRE_RED:
  case FIRE_GREEN:
  case FIRE_BLUE:
    GLUISmoke3dCB(USE_FIRE_RGB);
    GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    break;
  case UPDATE_SMOKEFIRE_COLORS_COMMON:
    glutPostRedisplay();
    force_redisplay = 1;
    UpdateRGBColors(colorbar_select_index);
    UpdateSmokeColormap(smoke_render_option);
    GLUISmoke3dCB(UPDATE_SMOKECOLORS);
    IdleCB();
    break;
  case UPDATE_SMOKECOLORS:
    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;

      meshi = global_scase.meshescoll.meshinfo + i;
      meshi->update_smoke3dcolors=1;
    }
    glutPostRedisplay();
    force_redisplay=1;
    UpdateSmokeColormap(RENDER_SLICE);
    UpdateSmokeColormap(smoke_render_option);
    IdleCB();
     break;
    case SMOKE_FRAME_INC:
      global_scase.update_smoke_alphas = 1;
      if(smoke3d_frame_inc<1){
        smoke3d_frame_inc=1;
        SPINNER_smoke3d_frame_inc->set_int_val(1);
      }
      break;
   case SMOKE_EXTINCT:
     global_scase.update_smoke_alphas = 1;
     glui_smoke3d_extinct = MAX(glui_smoke3d_extinct, 0.0);
     SPINNER_smoke3d_extinct->set_float_val(glui_smoke3d_extinct);
     SPINNER_smoke3d_extinct2->set_float_val(glui_smoke3d_extinct);
     break;
#ifdef pp_GPU
  case SMOKE_RTHICK:

    glutPostRedisplay();
    force_redisplay=1;
    IdleCB();
    break;
#endif
  case VOL_NGRID:
    glutPostRedisplay();
    break;

  case VOL_SMOKE:
    {
      volrenderdata *vr;

      vr = global_scase.meshescoll.meshinfo->volrenderinfo;
      if(vr!=NULL&&vr->smokeslice!=NULL&&vr->smokeslice->slice_filetype==SLICE_CELL_CENTER){
        if(usegpu==1&&combine_meshes==1){
          combine_meshes=0;
          UpdateCombineMeshes();
          GLUISmoke3dCB(COMBINE_MESHES);
        }
        if(usegpu==0&&combine_meshes==0){
          combine_meshes=1;
          UpdateCombineMeshes();
          GLUISmoke3dCB(COMBINE_MESHES);
        }
      }
    }
    if(SPINNER_smoke3d_frame_inc != NULL && CHECKBOX_triangle_display_rate != NULL){
#ifdef pp_GPU
      if(usegpu == 1){
        SPINNER_smoke3d_frame_inc->set_int_val(1);
        SPINNER_smoke3d_frame_inc->disable();
        CHECKBOX_triangle_display_rate->disable();
      }
      else{
        SPINNER_smoke3d_frame_inc->enable();
        CHECKBOX_triangle_display_rate->enable();
      }
#else
      SPINNER_smoke3d_frame_inc->enable();
      CHECKBOX_triangle_display_rate->enable();
#endif
    }
    break;
  default:
#ifdef _DEBUG
    abort();
#endif
    break;
  }
}
