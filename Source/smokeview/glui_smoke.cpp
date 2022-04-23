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
GLUI_RadioGroup *RADIO_skipframes=NULL;
GLUI_RadioGroup *RADIO_smokesensors=NULL;
GLUI_RadioGroup *RADIO_loadvol=NULL;
GLUI_RadioGroup *RADIO_light_type = NULL;
GLUI_RadioGroup *RADIO_scatter_type_glui = NULL;

GLUI_Spinner *SPINNER_smoke_num=NULL;
GLUI_Spinner *SPINNER_startframe=NULL;
GLUI_Spinner *SPINNER_skipframe=NULL;
GLUI_Spinner *SPINNER_hrrpuv_cutoff=NULL;
GLUI_Spinner *SPINNER_nongpu_vol_factor=NULL;
GLUI_Spinner *SPINNER_gpu_vol_factor=NULL;
GLUI_Spinner *SPINNER_smoke3d_threads = NULL;

GLUI_Spinner *SPINNER_temperature_min=NULL;
GLUI_Spinner *SPINNER_temperature_cutoff=NULL;
GLUI_Spinner *SPINNER_temperature_max=NULL;
GLUI_Spinner *SPINNER_fire_opacity_factor=NULL;
GLUI_Spinner *SPINNER_mass_extinct=NULL;
GLUI_Spinner *SPINNER_cvis=NULL;
GLUI_Spinner *SPINNER_smoke3d_skip = NULL;
GLUI_Spinner *SPINNER_smoke3d_skipx = NULL;
GLUI_Spinner *SPINNER_smoke3d_skipy = NULL;
GLUI_Spinner *SPINNER_smoke3d_skipz = NULL;
GLUI_Spinner *SPINNER_smoke3d_kmax = NULL;
GLUI_Spinner *SPINNER_smoke3d_extinct = NULL;
GLUI_Spinner *SPINNER_smoke3d_extinct2 = NULL;

GLUI_Spinner *SPINNER_smoke3d_fire_red=NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_green=NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_blue=NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_halfdepth=NULL;
GLUI_Spinner *SPINNER_smoke3d_co2_halfdepth = NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_halfdepth2=NULL;
GLUI_Spinner *SPINNER_smoke3d_co2_alpha = NULL;
GLUI_Spinner *SPINNER_smoke3d_smoke_red=NULL;
GLUI_Spinner *SPINNER_smoke3d_fire_alpha = NULL;
GLUI_Spinner *SPINNER_smoke3d_smoke_green=NULL;
GLUI_Spinner *SPINNER_smoke3d_smoke_blue=NULL;
GLUI_Spinner *SPINNER_load_3dsmoke = NULL;
GLUI_Spinner *SPINNER_load_hrrpuv = NULL;
GLUI_Spinner *SPINNER_light_xyz[3];
GLUI_Spinner *SPINNER_light_color[3];
GLUI_Spinner *SPINNER_smoke_test_color[4];
GLUI_Spinner *SPINNER_smoke_test_range = NULL;
GLUI_Spinner *SPINNER_smoke_test_nslices = NULL;
GLUI_Spinner *SPINNER_light_intensity = NULL;
GLUI_Spinner *SPINNER_scatter_param = NULL;
GLUI_Spinner *SPINNER_smokeloadframe = NULL;
GLUI_Spinner *SPINNER_globalloadframe = NULL;
GLUI_Spinner *SPINNER_timeloadframe = NULL;
GLUI_Spinner *SPINNER_slicehrrpuv_upper = NULL;
GLUI_Spinner *SPINNER_slicehrrpuv_middle = NULL;
GLUI_Spinner *SPINNER_slicehrrpuv_lower = NULL;
GLUI_Spinner *SPINNER_slicehrrpuv_cut2 = NULL;
GLUI_Spinner *SPINNER_slicehrrpuv_cut1 = NULL;
GLUI_Spinner *SPINNER_hrrpuvoffset=NULL;
GLUI_Spinner *SPINNER_co2color[3];
GLUI_Spinner *SPINNER_emission_factor=NULL;

GLUI_Checkbox *CHECKBOX_use_opacity_depth = NULL;
GLUI_Checkbox *CHECKBOX_use_opacity_multiplier = NULL;
GLUI_Checkbox *CHECKBOX_use_co2_colormap = NULL;
GLUI_Checkbox *CHECKBOX_use_fire_colormap = NULL;
GLUI_Checkbox *CHECKBOX_use_fire_rgb = NULL;
GLUI_Checkbox *CHECKBOX_use_co2_rgb = NULL;
GLUI_Checkbox *CHECKBOX_smoke_flip=NULL;
GLUI_Checkbox *CHECKBOX_smoke_getvals=NULL;
GLUI_Checkbox *CHECKBOX_update_smokeplanes = NULL;
GLUI_Checkbox *CHECKBOX_plane_single = NULL;
GLUI_Checkbox *CHECKBOX_freeze = NULL;
GLUI_Checkbox *CHECKBOX_combine_meshes = NULL;
GLUI_Checkbox *CHECKBOX_compress_volsmoke = NULL;
GLUI_Checkbox *CHECKBOX_smokecullflag = NULL;
GLUI_Checkbox *CHECKBOX_test_smokesensors = NULL;
GLUI_Checkbox *CHECKBOX_smokeGPU = NULL;
GLUI_Checkbox *CHECKBOX_zlib = NULL;
GLUI_Checkbox **CHECKBOX_meshvisptr = NULL;
GLUI_Checkbox *CHECKBOX_meshvis = NULL;
GLUI_Checkbox *CHECKBOX_show_light_position_direction = NULL;
GLUI_Checkbox *CHECKBOX_edit_colormap=NULL;
GLUI_Checkbox *CHECKBOX_plane_normal=NULL;

GLUI_Panel *PANEL_colormap3 = NULL;
GLUI_Panel *PANEL_fire_opacity = NULL;
GLUI_Panel *PANEL_smoke_opacity = NULL;
GLUI_Panel *PANEL_fire_color = NULL;
GLUI_Panel *PANEL_slice_alignment = NULL;
GLUI_Panel *PANEL_smoke_outline_type = NULL;
GLUI_Panel *PANEL_smokealg = NULL;
GLUI_Panel *PANEL_gridres = NULL;
GLUI_Panel *PANEL_fire_cutoff = NULL;
GLUI_Panel *PANEL_overall = NULL;
GLUI_Panel *PANEL_colormap2 = NULL;
GLUI_Panel *PANEL_smokesensor = NULL;
GLUI_Panel *PANEL_color = NULL;
GLUI_Panel *PANEL_smoke = NULL;
GLUI_Panel *PANEL_loadcutoff = NULL;
GLUI_Panel *PANEL_loadframe = NULL;
GLUI_Panel *PANEL_node_display = NULL;
GLUI_Panel *PANEL_display = NULL;
GLUI_Panel *PANEL_load_options = NULL;
GLUI_Panel *PANEL_smoke_rgb = NULL;
GLUI_Panel *PANEL_fire_rgb = NULL;
GLUI_Panel *PANEL_smokefire_rgb = NULL;

GLUI_Rollout *ROLLOUT_opacity = NULL;
GLUI_Rollout *ROLLOUT_light_color = NULL;
GLUI_Rollout *ROLLOUT_scatter = NULL;
GLUI_Rollout *ROLLOUT_light_position = NULL;
GLUI_Rollout *ROLLOUT_voldisplay = NULL;
GLUI_Rollout *ROLLOUT_volsmoke_move = NULL;
GLUI_Rollout *ROLLOUT_slicehrrpuv = NULL;
GLUI_Rollout *ROLLOUT_firecolor = NULL;
GLUI_Rollout *ROLLOUT_smokecolor = NULL;
GLUI_Rollout *ROLLOUT_generate_images = NULL;
GLUI_Rollout *ROLLOUT_loadframe = NULL;
GLUI_Rollout *ROLLOUT_light = NULL;
GLUI_Rollout *ROLLOUT_co2color = NULL;
GLUI_Rollout *ROLLOUT_temperature_settings=NULL;
GLUI_Rollout *ROLLOUT_meshvis = NULL;
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


#define VOLRENDER_ROLLOUT   0
#define SLICERENDER_ROLLOUT 1
#define MESH_ROLLOUT        2

procdata smokeprocinfo[3], volsmokeprocinfo[7], colorprocinfo[3], sublightprocinfo[3];
int nsmokeprocinfo = 0, nvolsmokeprocinfo=0, ncolorprocinfo = 0, nsublightprocinfo=0;

#define FIRECOLOR_ROLLOUT   0
#define FIREOPACITY_ROLLOUT 1
#define CO2COLOR_ROLLOUT    2

#define VOLSMOKE_DISPLAY           0
#define VOLSMOKE_MOVEMENT          1
#define VOLSMOKE_COMPUTE_ROLLOUT   2
#define VOLSMOKE_LOAD_ROLLOUT      3
#define VOLSMOKE_LIGHT_ROLLOUT     4
#define VOLSMOKE_IMAGES_ROLLOUT    5
#define VOLSMOKE_LOADFRAME_ROLLOUT 6

#define SLICESMOKE_LOAD_ROLLOUT 0
#define SLICESMOKE_ORIG_ROLLOUT 1
#define SLICESMOKE_TEST_ROLLOUT 2

#define LIGHT_POSITION_ROLLOUT 0
#define LIGHT_COLOR_ROLLOUT    1
#define LIGHT_SCATTER_ROLLOUT  2

/* ------------------ UpdateFireAlpha ------------------------ */

extern "C" void UpdateFireAlpha(void){
  Smoke3dCB(USE_FIRE_ALPHA);
}

/* ------------------ UpdateCO2ColorbarList ------------------------ */

extern "C" void UpdateCO2ColorbarList(int value){
  co2_colorbar_index = CLAMP(value, 0, ncolorbars-1);
  if(LISTBOX_co2_colorbar!=NULL)LISTBOX_co2_colorbar->set_int_val(co2_colorbar_index);
  Smoke3dCB(CO2_COLORBAR_LIST);
}

/* ------------------ UpdateFireColorbarList ------------------------ */

extern "C" void UpdateFireColorbarList(void){
  if(LISTBOX_smoke_colorbar!=NULL)LISTBOX_smoke_colorbar->set_int_val(fire_colorbar_index);
}

/* ------------------ UpdateBackgroundFlip2 ------------------------ */

extern "C" void UpdateBackgroundFlip2(int flip) {
  if(CHECKBOX_smoke_flip!=NULL)CHECKBOX_smoke_flip->set_int_val(flip);
}

/* ------------------ UpdateFreeze ------------------------ */

extern "C" void UpdateFreeze(int val){
  CHECKBOX_freeze->set_int_val(val);
}

/* ------------------ UpdateLoadFrameVal ------------------------ */

extern "C" void UpdateLoadFrameVal(int frames){
  SPINNER_smokeloadframe->set_int_val(frames);
}

/* ------------------ UpdateLoadTimeVal ------------------------ */

extern "C" void UpdateLoadTimeVal(float val){
  SPINNER_timeloadframe->set_float_val(val);
}

/* ------------------ UpdateLoadFrameMax ------------------------ */

extern "C" void UpdateLoadFrameMax(int max_frames){
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

/* ------------------ UpdateTimeFrameBounds ------------------------ */

extern "C" void UpdateTimeFrameBounds(float time_min, float time_max){
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

/* ------------------ SublightRolloutCB ------------------------ */

void SublightRolloutCB(int var){
  ToggleRollout(sublightprocinfo, nsublightprocinfo, var);
}

/* ------------------ VolSmokeRolloutCB ------------------------ */

void VolSmokeRolloutCB(int var){
  ToggleRollout(volsmokeprocinfo, nvolsmokeprocinfo, var);
}

/* ------------------ ColorRolloutCB ------------------------ */

void ColorRolloutCB(int var){
  ToggleRollout(colorprocinfo, ncolorprocinfo, var);
}

/* ------------------ SmokeRolloutCB ------------------------ */

extern "C" void SmokeRolloutCB(int var){
  if(var == VOLRENDER_ROLLOUT)smoke_render_option = RENDER_VOLUME;
  if(var == SLICERENDER_ROLLOUT)smoke_render_option = RENDER_SLICE;
  Smoke3dCB(SMOKE_OPTIONS);
  ToggleRollout(smokeprocinfo, nsmokeprocinfo, var);
}

/* ------------------ DeleteVolTourList ------------------------ */

extern "C" void DeleteVolTourList(void){
  int i;

  if(LISTBOX_VOL_tour==NULL)return;
  for(i=0;i<ntourinfo;i++){
    LISTBOX_VOL_tour->delete_item(i);
  }
}

/* ------------------ CreateVolTourList ------------------------ */

extern "C" void CreateVolTourList(void){
  int i;

  if(LISTBOX_VOL_tour==NULL)return;
  for(i=0;i<ntourinfo;i++){
    tourdata *touri;
    char label[1000];

    touri = tourinfo + i;
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
  if(selectedtour_index>=TOURINDEX_MANUAL&&selectedtour_index<ntourinfo){
    LISTBOX_VOL_tour->set_int_val(selectedtour_index);
  }
}

/* ------------------ UpdateCombineMeshes ------------------------ */

extern "C" void UpdateCombineMeshes(void){
  if(CHECKBOX_combine_meshes!=NULL)CHECKBOX_combine_meshes->set_int_val(combine_meshes);
}

/* ------------------ UpdateSmoke3dFlags ------------------------ */

extern "C" void UpdateSmoke3dFlags(void){
#ifdef pp_GPU
  if(CHECKBOX_smokeGPU!=NULL)CHECKBOX_smokeGPU->set_int_val(usegpu);
#endif
  CHECKBOX_smokecullflag->set_int_val(smokecullflag);
  RADIO_skipframes->set_int_val(smokeskipm1);
  Smoke3dCB(VOL_SMOKE);
  glutPostRedisplay();
}

/* ------------------ Glui3dSmokeSetup ------------------------ */

extern "C" void Glui3dSmokeSetup(int main_window){

  int i;


  if(nsmoke3dinfo<=0&&nvolrenderinfo<=0)return;
  if(CHECKBOX_meshvisptr!=NULL)FREEMEMORY(CHECKBOX_meshvisptr);
  NewMemory((void **)&CHECKBOX_meshvisptr,nmeshes*sizeof(GLUI_Checkbox *));

  glui_3dsmoke=glui_bounds;

  PANEL_overall = glui_3dsmoke->add_panel_to_panel(ROLLOUT_smoke3d,"",GLUI_PANEL_NONE);
#ifdef pp_GPU
  CHECKBOX_smokeGPU=glui_3dsmoke->add_checkbox_to_panel(PANEL_overall,_("Use GPU"),&usegpu,VOL_SMOKE,Smoke3dCB);
#endif
  glui_3dsmoke->add_checkbox_to_panel(PANEL_overall, _("max blending"), &hrrpuv_max_blending);
  CHECKBOX_smoke_flip = glui_3dsmoke->add_checkbox_to_panel(PANEL_overall, _("flip background"), &background_flip,BACKGROUND_FLIP, Smoke3dCB);
  CHECKBOX_smoke_flip = glui_3dsmoke->add_checkbox_to_panel(PANEL_overall, _("load in parallel"), &use_smoke_thread);
  SPINNER_smoke3d_threads = glui_3dsmoke->add_spinner_to_panel(PANEL_overall, _("threads"), GLUI_SPINNER_INT, &nsmoke_threads);
  SPINNER_smoke3d_threads->set_int_limits(1, 16);


  if(active_smokesensors==1){
    PANEL_smokesensor = glui_3dsmoke->add_panel_to_panel(PANEL_overall,_("Visibility"));
    RADIO_smokesensors = glui_3dsmoke->add_radiogroup_to_panel(PANEL_smokesensor,&show_smokesensors);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_smokesensors,_("Hidden"));                     // SMOKESENSORS_HIDDEN
    glui_3dsmoke->add_radiobutton_to_group(RADIO_smokesensors,_("Grey (0-255)"));               // SMOKESENSORS_0255
    glui_3dsmoke->add_radiobutton_to_group(RADIO_smokesensors,"I/I0 (0.0-1.0)");                 // SMOKESENSORS_01P0
    glui_3dsmoke->add_radiobutton_to_group(RADIO_smokesensors,_("Scaled optical depth (SCD)")); // SMOKESENSORS_SCALED
    glui_3dsmoke->add_statictext_to_panel(PANEL_smokesensor,"SCD=C/K=C*L/Ln(I/I0) (0-Inf)");     // SMOKESENSORS_0INF
    SPINNER_cvis=glui_3dsmoke->add_spinner_to_panel(PANEL_smokesensor,"C",GLUI_SPINNER_FLOAT,&smoke3d_cvis);
    SPINNER_cvis->set_float_limits(1.0,20.0);
#ifdef _DEBUG
    CHECKBOX_test_smokesensors=glui_3dsmoke->add_checkbox_to_panel(PANEL_smokesensor,_("Test visibility sensor"),&test_smokesensors);
#endif
  }

  if(fire_colormap_type==0){
    use_smoke_rgb      = 1;
    use_smoke_colormap = 0;
    use_fire_rgb       = 1;
    use_fire_colormap  = 0;
  }
  else{
    use_smoke_rgb      = 0;
    use_smoke_colormap = 1;
    use_fire_rgb       = 0;
    use_fire_colormap  = 1;
  }
  ROLLOUT_firecolor = glui_3dsmoke->add_rollout_to_panel(PANEL_overall, _("smoke/fire color"),false, FIRECOLOR_ROLLOUT, ColorRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_firecolor, glui_3dsmoke);
  ADDPROCINFO(colorprocinfo, ncolorprocinfo, ROLLOUT_firecolor, FIRECOLOR_ROLLOUT, glui_3dsmoke);

  PANEL_fire_color = glui_3dsmoke->add_panel_to_panel(ROLLOUT_firecolor, "red/green/blue");

  CHECKBOX_use_fire_rgb = glui_3dsmoke->add_checkbox_to_panel(PANEL_fire_color, "set", &use_fire_rgb, USE_FIRE_RGB, Smoke3dCB);

  PANEL_smokefire_rgb = glui_3dsmoke->add_panel_to_panel(PANEL_fire_color, "", GLUI_PANEL_NONE);

  PANEL_smoke_rgb = glui_3dsmoke->add_panel_to_panel(PANEL_smokefire_rgb, "smoke");
  SPINNER_smoke3d_smoke_red   = glui_3dsmoke->add_spinner_to_panel(PANEL_smoke_rgb, _("red"),   GLUI_SPINNER_INT, smoke_color_int255,   SMOKE_RED,   Smoke3dCB);
  SPINNER_smoke3d_smoke_green = glui_3dsmoke->add_spinner_to_panel(PANEL_smoke_rgb, _("green"), GLUI_SPINNER_INT, smoke_color_int255+1, SMOKE_GREEN, Smoke3dCB);
  SPINNER_smoke3d_smoke_blue  = glui_3dsmoke->add_spinner_to_panel(PANEL_smoke_rgb, _("blue"),  GLUI_SPINNER_INT, smoke_color_int255+2, SMOKE_BLUE,  Smoke3dCB);
                                glui_3dsmoke->add_checkbox_to_panel(PANEL_smoke_rgb, "force gray", &force_gray_smoke, FORCE_GRAY, Smoke3dCB);
  SPINNER_smoke3d_smoke_red->set_int_limits(0, 255);
  SPINNER_smoke3d_smoke_green->set_int_limits(0, 255);
  SPINNER_smoke3d_smoke_blue->set_int_limits(0, 255);

  glui_3dsmoke->add_column_to_panel(PANEL_smokefire_rgb, false);
  PANEL_fire_rgb = glui_3dsmoke->add_panel_to_panel(PANEL_smokefire_rgb, "fire");
  SPINNER_smoke3d_fire_red  =glui_3dsmoke->add_spinner_to_panel(PANEL_fire_rgb, _("red"), GLUI_SPINNER_INT, fire_color_int255,  FIRE_RED,  Smoke3dCB);
  SPINNER_smoke3d_fire_green=glui_3dsmoke->add_spinner_to_panel(PANEL_fire_rgb,_("green"),GLUI_SPINNER_INT, fire_color_int255+1,FIRE_GREEN,Smoke3dCB);
  SPINNER_smoke3d_fire_blue =glui_3dsmoke->add_spinner_to_panel(PANEL_fire_rgb,_("blue"), GLUI_SPINNER_INT, fire_color_int255+2,FIRE_BLUE, Smoke3dCB);
  SPINNER_smoke3d_fire_red->set_int_limits(0,255);
  SPINNER_smoke3d_fire_green->set_int_limits(0,255);
  SPINNER_smoke3d_fire_blue->set_int_limits(0,255);

  if(ncolorbars > 0){
    PANEL_colormap3 = glui_3dsmoke->add_panel_to_panel(ROLLOUT_firecolor, "colormap");
    CHECKBOX_use_fire_colormap = glui_3dsmoke->add_checkbox_to_panel(PANEL_colormap3, "set", &use_fire_colormap, USE_FIRE_COLORMAP, Smoke3dCB);
    LISTBOX_smoke_colorbar = glui_3dsmoke->add_listbox_to_panel(PANEL_colormap3, "Select:", &fire_colorbar_index, SMOKE_COLORBAR_LIST, Smoke3dCB);
    for(i = 0;i < ncolorbars;i++){
      colorbardata *cbi;

      cbi = colorbarinfo + i;
      cbi->label_ptr = cbi->label;
      LISTBOX_smoke_colorbar->add_item(i, cbi->label_ptr);
    }
    LISTBOX_smoke_colorbar->set_int_val(fire_colorbar_index);
    glui_3dsmoke->add_column_to_panel(PANEL_colormap3,false);
    CHECKBOX_edit_colormap = glui_3dsmoke->add_checkbox_to_panel(PANEL_colormap3, "Edit", &show_firecolormap, SHOW_FIRECOLORMAP, Smoke3dCB);
  }

#define HRRPUV_CUTOFF_MAX (hrrpuv_max_smv-0.01)

  PANEL_fire_cutoff = glui_3dsmoke->add_panel_to_panel(ROLLOUT_firecolor, "Color as fire when:");
  SPINNER_hrrpuv_cutoff = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_cutoff, "HRRPUV (kW/m3) > ", GLUI_SPINNER_FLOAT, &global_hrrpuv_cutoff, GLOBAL_FIRE_CUTOFF, Smoke3dCB);
  SPINNER_hrrpuv_cutoff->set_float_limits(0.0, HRRPUV_CUTOFF_MAX);
  {
    char temp_cutoff_label[300];

    sprintf(temp_cutoff_label, "temperature (%s) > ", degC);
    SPINNER_temperature_cutoff = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_cutoff, temp_cutoff_label, GLUI_SPINNER_FLOAT,
      &global_temp_cutoff, TEMP_CUTOFF, Smoke3dCB);
  }
  BUTTON_cutoff_defaults = glui_3dsmoke->add_button_to_panel(PANEL_fire_cutoff, "Reset", CUTOFF_RESET, Smoke3dCB);

  ROLLOUT_opacity = glui_3dsmoke->add_rollout_to_panel(PANEL_overall, "smoke/fire opacity",false, FIREOPACITY_ROLLOUT, ColorRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_opacity, glui_3dsmoke);
  ADDPROCINFO(colorprocinfo, ncolorprocinfo, ROLLOUT_opacity, FIREOPACITY_ROLLOUT, glui_3dsmoke);

  PANEL_smoke_opacity = glui_3dsmoke->add_panel_to_panel(ROLLOUT_opacity, "smoke");
  SPINNER_smoke3d_extinct2 = glui_3dsmoke->add_spinner_to_panel(PANEL_smoke_opacity, _("Extinction (m2/kg)"),
                                                                GLUI_SPINNER_FLOAT, &glui_smoke3d_extinct, SMOKE_EXTINCT, Smoke3dCB);
  BUTTON_fds_extinction_reset = glui_3dsmoke->add_button_to_panel(PANEL_smoke_opacity, "Reset(fds extinct)", EXTINCTION_RESET_FDS, Smoke3dCB);
  BUTTON_fds_extinction_reset = glui_3dsmoke->add_button_to_panel(PANEL_smoke_opacity, "Reset(saved extinct)", EXTINCTION_RESET_SMV, Smoke3dCB);

  PANEL_fire_opacity = glui_3dsmoke->add_panel_to_panel(ROLLOUT_opacity, "fire");
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
    &use_opacity_depth, USE_OPACITY_DEPTH, Smoke3dCB);
  SPINNER_smoke3d_fire_halfdepth = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_opacity, "50% opacity at depth (m):", GLUI_SPINNER_FLOAT, &fire_halfdepth, UPDATE_SMOKEFIRE_COLORS, Smoke3dCB);

  CHECKBOX_use_opacity_multiplier = glui_3dsmoke->add_checkbox_to_panel(PANEL_fire_opacity, "set opacity multiplier (when smoke also loaded)",
    &use_opacity_multiplier, USE_OPACITY_MULTIPLIER, Smoke3dCB);
  SPINNER_emission_factor = glui_3dsmoke->add_spinner_to_panel(PANEL_fire_opacity, "opacity multiplier:", GLUI_SPINNER_FLOAT, &emission_factor, USE_FIRE_ALPHA, Smoke3dCB);
  SPINNER_smoke3d_fire_halfdepth->set_float_limits(0.01, 100.0);
  Smoke3dCB(USE_OPACITY_DEPTH);

  if(co2_colormap_type==0){
    use_co2_rgb      = 1;
    use_co2_colormap = 0;
  }
  else{
    use_co2_rgb      = 0;
    use_co2_colormap = 1;
  }
  if(nsmoke3d_co2 > 0){
    ROLLOUT_co2color=glui_3dsmoke->add_rollout_to_panel(PANEL_overall, "CO2 color", false, CO2COLOR_ROLLOUT, ColorRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_co2color,glui_3dsmoke);
    ADDPROCINFO(colorprocinfo, ncolorprocinfo, ROLLOUT_co2color, CO2COLOR_ROLLOUT, glui_3dsmoke);
    CHECKBOX_use_co2_rgb = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_co2color, "set red/green/blue", &use_co2_rgb, USE_CO2_RGB, Smoke3dCB);

    SPINNER_co2color[0] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("red"),   GLUI_SPINNER_INT, co2_color_int255,     CO2_COLOR, Smoke3dCB);
    SPINNER_co2color[1] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("green"), GLUI_SPINNER_INT, co2_color_int255 + 1, CO2_COLOR, Smoke3dCB);
    SPINNER_co2color[2] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("blue"),  GLUI_SPINNER_INT, co2_color_int255 + 2, CO2_COLOR, Smoke3dCB);
    SPINNER_co2color[0]->set_int_limits(0, 255);
    SPINNER_co2color[1]->set_int_limits(0, 255);
    SPINNER_co2color[2]->set_int_limits(0, 255);
    if(ncolorbars > 0){
      CHECKBOX_use_co2_colormap = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_co2color, "set colormap", &use_co2_colormap, USE_CO2_COLORMAP, Smoke3dCB);
      LISTBOX_co2_colorbar = glui_3dsmoke->add_listbox_to_panel(ROLLOUT_co2color, "colormap:", &co2_colorbar_index, CO2_COLORBAR_LIST, Smoke3dCB);
      for(i = 0; i < ncolorbars; i++){
        colorbardata *cbi;

        cbi = colorbarinfo+i;
        cbi->label_ptr = cbi->label;
        LISTBOX_co2_colorbar->add_item(i, cbi->label_ptr);
      }
      LISTBOX_co2_colorbar->set_int_val(co2_colorbar_index);
    }

    if(nco2files > 0){
      SPINNER_smoke3d_co2_halfdepth = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("50% CO2 opacity at depth (m)"), GLUI_SPINNER_FLOAT, &co2_halfdepth, UPDATE_SMOKEFIRE_COLORS, Smoke3dCB);
    }
  }

  Smoke3dCB(USE_FIRE_RGB);
  Smoke3dCB(USE_FIRE_COLORMAP);
  Smoke3dCB(USE_CO2_RGB);
  Smoke3dCB(USE_CO2_COLORMAP);
  Smoke3dCB(UPDATE_SMOKEFIRE_COLORS);
  Smoke3dCB(UPDATE_SMOKEFIRE_COLORS2);
  Smoke3dCB(USE_SMOKE_RGB);


  PANEL_colormap2 = glui_3dsmoke->add_panel_to_panel(PANEL_overall,"",GLUI_PANEL_NONE);

  glui_3dsmoke->add_column_to_panel(PANEL_overall,false);

  if(nsmoke3dinfo<=0||nvolrenderinfo<=0){
    smoke_render_option=RENDER_SLICE;
    if(nsmoke3dinfo>0)smoke_render_option=RENDER_SLICE;
    if(nvolrenderinfo>0)smoke_render_option=RENDER_VOLUME;
  }

  // slice render dialog

  if(nsmoke3dinfo>0){
    ROLLOUT_slices = glui_3dsmoke->add_rollout_to_panel(PANEL_overall,_("Slice rendered"),false, SLICERENDER_ROLLOUT, SmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_slices, glui_3dsmoke);
    ADDPROCINFO(smokeprocinfo, nsmokeprocinfo, ROLLOUT_slices, SLICERENDER_ROLLOUT, glui_3dsmoke);
    ROLLOUT_slices->set_alignment(GLUI_ALIGN_LEFT);

    PANEL_load_options = glui_3dsmoke->add_panel_to_panel(ROLLOUT_slices, _("Load when:"));
    SPINNER_load_3dsmoke = glui_3dsmoke->add_spinner_to_panel(PANEL_load_options, _("soot alpha >"), GLUI_SPINNER_FLOAT, &load_3dsmoke_cutoff);
    SPINNER_load_3dsmoke->set_float_limits(0.0, 255.0);

    SPINNER_load_hrrpuv = glui_3dsmoke->add_spinner_to_panel(PANEL_load_options, _("HRRPUV >"), GLUI_SPINNER_FLOAT, &load_hrrpuv_cutoff);
    SPINNER_load_hrrpuv->set_float_limits(0.0, HRRPUV_CUTOFF_MAX);

#ifdef pp_GPU
    if(gpuactive==0){
      usegpu=0;
      CHECKBOX_smokeGPU->disable();
    }
#endif

    PANEL_display = glui_3dsmoke->add_panel_to_panel(ROLLOUT_slices, "smoke slice display");
    RADIO_skipframes = glui_3dsmoke->add_radiogroup_to_panel(PANEL_display,&smokeskipm1);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_skipframes,_("All"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_skipframes,_("Every 2nd"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_skipframes,_("Every 3rd"));

    SPINNER_smoke3d_extinct = glui_3dsmoke->add_spinner_to_panel(PANEL_display, _("Extinction (m2/kg)"),
                                                                 GLUI_SPINNER_FLOAT, &glui_smoke3d_extinct, SMOKE_EXTINCT, Smoke3dCB);

    glui_3dsmoke->add_column_to_panel(ROLLOUT_slices, false);

    PANEL_node_display = glui_3dsmoke->add_panel_to_panel(ROLLOUT_slices, "smoke node display");
    SPINNER_smoke3d_skip   = glui_3dsmoke->add_spinner_to_panel(PANEL_node_display, "x/y/z",   GLUI_SPINNER_INT, &smoke3d_skip,  SMOKE_SKIP,     Smoke3dCB);
    SPINNER_smoke3d_skipx  = glui_3dsmoke->add_spinner_to_panel(PANEL_node_display, "x",     GLUI_SPINNER_INT, &smoke3d_skipx, SMOKE_SKIP_XYZ, Smoke3dCB);
    SPINNER_smoke3d_skipy  = glui_3dsmoke->add_spinner_to_panel(PANEL_node_display, "y",     GLUI_SPINNER_INT, &smoke3d_skipy, SMOKE_SKIP_XYZ, Smoke3dCB);
    SPINNER_smoke3d_skipz  = glui_3dsmoke->add_spinner_to_panel(PANEL_node_display, "z",     GLUI_SPINNER_INT, &smoke3d_skipz, SMOKE_SKIP_XYZ, Smoke3dCB);
    SPINNER_smoke3d_kmax   = glui_3dsmoke->add_spinner_to_panel(PANEL_node_display, "max k", GLUI_SPINNER_INT, &smoke3d_kmax);
    CHECKBOX_smokecullflag = glui_3dsmoke->add_checkbox_to_panel(PANEL_node_display,"Cull hidden planes", &smokecullflag);
  }

  // volume render dialog

  if(nvolrenderinfo > 0){
    ROLLOUT_volume = glui_3dsmoke->add_rollout_to_panel(PANEL_overall, _("Volume rendered"), false, VOLRENDER_ROLLOUT, SmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_volume, glui_3dsmoke);
    ADDPROCINFO(smokeprocinfo, nsmokeprocinfo, ROLLOUT_volume, VOLRENDER_ROLLOUT, glui_3dsmoke);

    if(have_volcompressed == 1){
      RADIO_loadvol = glui_3dsmoke->add_radiogroup_to_panel(ROLLOUT_volume, &glui_load_volcompressed, LOAD_COMPRESSED_DATA, Smoke3dCB);
      glui_3dsmoke->add_radiobutton_to_group(RADIO_loadvol, _("Load full data"));
      glui_3dsmoke->add_radiobutton_to_group(RADIO_loadvol, _("Load compressed data"));
    }

    //*** display

    ROLLOUT_voldisplay = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume,_("Display"),false, VOLSMOKE_DISPLAY, VolSmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_voldisplay, glui_3dsmoke);
    ADDPROCINFO(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_voldisplay, VOLSMOKE_DISPLAY, glui_3dsmoke);

    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_voldisplay, _("Show"), &usevolrender, VOL_SMOKE, Smoke3dCB);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_voldisplay, _("black/white"), &volbw);
#ifdef _DEBUG
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_voldisplay, "block smoke", &block_volsmoke);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_voldisplay, "debug", &smoke3dVoldebug);
#endif

    //*** scene movement

    ROLLOUT_volsmoke_move = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Scene movement"), false, VOLSMOKE_MOVEMENT, VolSmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_volsmoke_move, glui_3dsmoke);
    ADDPROCINFO(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_volsmoke_move, VOLSMOKE_MOVEMENT, glui_3dsmoke);

    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_move, _("Auto freeze"), &autofreeze_volsmoke);
    CHECKBOX_freeze = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_move, _("Freeze"), &freeze_volsmoke);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_move, _("Show data while moving scene (GPU)"), &show_volsmoke_moving);

    //*** compute

    ROLLOUT_volsmoke_compute = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Compute"), false, VOLSMOKE_COMPUTE_ROLLOUT, VolSmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_volsmoke_compute, glui_3dsmoke);
    ADDPROCINFO(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_volsmoke_compute, VOLSMOKE_COMPUTE_ROLLOUT, glui_3dsmoke);

    SPINNER_fire_opacity_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("Fire opacity multiplier"), GLUI_SPINNER_FLOAT, &fire_opacity_factor);
    SPINNER_fire_opacity_factor->set_float_limits(1.0, 50.0);
    SPINNER_mass_extinct = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("Mass extinction coeff (m2/g)"), GLUI_SPINNER_FLOAT, &mass_extinct, MASS_EXTINCTION, Smoke3dCB);
    Smoke3dCB(MASS_EXTINCTION);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_compute, _("adaptive integration"), &vol_adaptive);
    CHECKBOX_combine_meshes = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_compute, _("Combine meshes"), &combine_meshes, COMBINE_MESHES, Smoke3dCB);
    SPINNER_nongpu_vol_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("non-gpu grid multiplier"), GLUI_SPINNER_FLOAT, &nongpu_vol_factor, NONGPU_VOL_FACTOR, Smoke3dCB);
    SPINNER_nongpu_vol_factor->set_float_limits(1.0, 10.0);
    SPINNER_gpu_vol_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("gpu grid multiplier"), GLUI_SPINNER_FLOAT, &gpu_vol_factor, GPU_VOL_FACTOR, Smoke3dCB);
    SPINNER_gpu_vol_factor->set_float_limits(1.0, 10.0);

    //*** load

    ROLLOUT_volsmoke_load = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Load"), false, VOLSMOKE_LOAD_ROLLOUT, VolSmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_volsmoke_load, glui_3dsmoke);
    ADDPROCINFO(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_volsmoke_load, VOLSMOKE_LOAD_ROLLOUT, glui_3dsmoke);

    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_load, _("Load data in background"), &use_multi_threading);
    CHECKBOX_compress_volsmoke = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_load, _("Compress data while loading"), &glui_compress_volsmoke);
    if(have_volcompressed == 1){
      Smoke3dCB(LOAD_COMPRESSED_DATA);
    }
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_load, _("Load data only at render times"), &load_at_rendertimes);

    //*** light

    ROLLOUT_light = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Light"), false, VOLSMOKE_LIGHT_ROLLOUT, VolSmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_light, glui_3dsmoke);
    ADDPROCINFO(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_light, VOLSMOKE_LIGHT_ROLLOUT, glui_3dsmoke);

    ROLLOUT_light_position = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_light, "position",false, LIGHT_POSITION_ROLLOUT, SublightRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_light_position, glui_3dsmoke);
    ADDPROCINFO(sublightprocinfo, nsublightprocinfo, ROLLOUT_light_position, LIGHT_POSITION_ROLLOUT, glui_3dsmoke);

    RADIO_light_type = glui_3dsmoke->add_radiogroup_to_panel(ROLLOUT_light_position,&light_type_glui);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_light_type,_("position"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_light_type,_("direction"));
    CHECKBOX_show_light_position_direction = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_light_position, _("Show position/direction"), &show_light_position_direction);
    SPINNER_light_xyz[0] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_light_position, "x:", GLUI_SPINNER_FLOAT, xyz_light_glui,   LIGHT_XYZ, Smoke3dCB);
    SPINNER_light_xyz[1] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_light_position, "y:", GLUI_SPINNER_FLOAT, xyz_light_glui+1, LIGHT_XYZ, Smoke3dCB);
    SPINNER_light_xyz[2] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_light_position, "z:", GLUI_SPINNER_FLOAT, xyz_light_glui+2, LIGHT_XYZ, Smoke3dCB);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_light_position, _("Use light"), &use_light);
    glui_3dsmoke->add_button_to_panel(ROLLOUT_light_position, _("Update"), LIGHT_UPDATE, Smoke3dCB);

    ROLLOUT_light_color = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_light, _("color/intensity"),false, LIGHT_COLOR_ROLLOUT, SublightRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_light_color, glui_3dsmoke);
    ADDPROCINFO(sublightprocinfo, nsublightprocinfo, ROLLOUT_light_color, LIGHT_COLOR_ROLLOUT, glui_3dsmoke);

    SPINNER_light_color[0] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_light_color,   _("red:"), GLUI_SPINNER_INT, light_color);
    SPINNER_light_color[1] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_light_color, _("green:"), GLUI_SPINNER_INT, light_color+1);
    SPINNER_light_color[2] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_light_color,  _("blue:"), GLUI_SPINNER_INT, light_color+2);
    SPINNER_light_color[0]->set_int_limits(0, 255);
    SPINNER_light_color[1]->set_int_limits(0, 255);
    SPINNER_light_color[2]->set_int_limits(0, 255);
    SPINNER_light_intensity = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_light_color, _("intensity:"), GLUI_SPINNER_FLOAT, &light_intensity);
    SPINNER_light_intensity->set_float_limits(0.0,1000.0);

    ROLLOUT_scatter = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_light, _("scatter"),false, LIGHT_SCATTER_ROLLOUT, SublightRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_scatter, glui_3dsmoke);
    ADDPROCINFO(sublightprocinfo, nsublightprocinfo, ROLLOUT_scatter, LIGHT_SCATTER_ROLLOUT, glui_3dsmoke);

    RADIO_scatter_type_glui = glui_3dsmoke->add_radiogroup_to_panel(ROLLOUT_scatter,&scatter_type_glui);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_scatter_type_glui,_("isotropic"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_scatter_type_glui,"Henjey-Greenstein");
    glui_3dsmoke->add_radiobutton_to_group(RADIO_scatter_type_glui,"Schlick");
    SPINNER_scatter_param  = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_scatter,   _("param"), GLUI_SPINNER_FLOAT, &scatter_param);
    SPINNER_scatter_param->set_float_limits(-1.0,1.0);

    //*** generate images

    ROLLOUT_generate_images = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Generate images"), false, VOLSMOKE_IMAGES_ROLLOUT, VolSmokeRolloutCB);
    ADDPROCINFO(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_generate_images, VOLSMOKE_IMAGES_ROLLOUT, glui_3dsmoke);

    SPINNER_startframe = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_generate_images, _("start frame"), GLUI_SPINNER_INT, &vol_startframe0, START_FRAME, Smoke3dCB);
    SPINNER_skipframe = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_generate_images, _("skip frame"), GLUI_SPINNER_INT, &vol_skipframe0, SKIP_FRAME, Smoke3dCB);
    Smoke3dCB(START_FRAME);
    Smoke3dCB(SKIP_FRAME);
    if(ntourinfo > 0){
      selectedtour_index = TOURINDEX_MANUAL;
      selectedtour_index_old = TOURINDEX_MANUAL;
      LISTBOX_VOL_tour = glui_3dsmoke->add_listbox_to_panel(ROLLOUT_generate_images, "Tour:", &selectedtour_index, VOL_TOUR_LIST, Smoke3dCB);

      LISTBOX_VOL_tour->add_item(TOURINDEX_MANUAL, "Manual");
      LISTBOX_VOL_tour->add_item(-999, "-");
      for(i = 0; i < ntourinfo; i++){
        tourdata *touri;

        touri = tourinfo + i;
        LISTBOX_VOL_tour->add_item(i, touri->label);
      }
      LISTBOX_VOL_tour->set_int_val(selectedtour_index);
    }

    strcpy(vol_prefix, fdsprefix);
    EDIT_vol_prefix = glui_3dsmoke->add_edittext_to_panel(ROLLOUT_generate_images, "image prefix:", GLUI_EDITTEXT_TEXT, vol_prefix, VOL_PREFIX, Smoke3dCB);
    EDIT_vol_prefix->set_w(200);

    BUTTON_startrender = glui_3dsmoke->add_button_to_panel(ROLLOUT_generate_images, _("Generate images"), GENERATE_IMAGES, Smoke3dCB);
    BUTTON_cancelrender = glui_3dsmoke->add_button_to_panel(ROLLOUT_generate_images, _("Cancel"), CANCEL_GENERATE_IMAGES, Smoke3dCB);
    BUTTON_volunload = glui_3dsmoke->add_button_to_panel(ROLLOUT_generate_images, _("Unload"), VOL_UNLOAD_ALL, Smoke3dCB);
    UpdateSmokeColormap(RENDER_VOLUME);
    Smoke3dCB(SMOKE_OPTIONS);

    ROLLOUT_loadframe = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Load frame"), false, VOLSMOKE_LOADFRAME_ROLLOUT, VolSmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_loadframe, glui_3dsmoke);
    ADDPROCINFO(volsmokeprocinfo, nvolsmokeprocinfo, ROLLOUT_loadframe, VOLSMOKE_LOADFRAME_ROLLOUT, glui_3dsmoke);
    PANEL_loadframe = glui_3dsmoke->add_panel_to_panel(ROLLOUT_loadframe, "", false);

    glui_3dsmoke->add_button_to_panel(PANEL_loadframe, _("Load"), LOAD_SMOKEFRAME, Smoke3dCB);
    glui_3dsmoke->add_button_to_panel(PANEL_loadframe, _("Load"), LOAD_TIMEFRAME, Smoke3dCB);

    glui_3dsmoke->add_column_to_panel(PANEL_loadframe, false);

    SPINNER_smokeloadframe = glui_3dsmoke->add_spinner_to_panel(PANEL_loadframe, _("smoke frame"), GLUI_SPINNER_INT, &smoke_framenumber);
    SPINNER_timeloadframe = glui_3dsmoke->add_spinner_to_panel(PANEL_loadframe, _("time"), GLUI_SPINNER_FLOAT, &time_frameval);
  }

  if(nsmoke3dinfo>0){
    ROLLOUT_meshvis = glui_3dsmoke->add_rollout_to_panel(PANEL_overall, "Mesh Visibility", false, MESH_ROLLOUT, SmokeRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_meshvis, glui_3dsmoke);
    ADDPROCINFO(smokeprocinfo, nsmokeprocinfo, ROLLOUT_meshvis, MESH_ROLLOUT, glui_3dsmoke);
    for(i = 0;i<nmeshes;i++){
      meshdata *meshi;

      meshi = meshinfo+i;
      glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_meshvis, meshi->label, meshvisptr+i);
    }
  }

#ifdef pp_GPU
  Smoke3dCB(VOL_SMOKE);
#endif
  Smoke3dCB(SMOKE_OPTIONS);
  Smoke3dCB(FIRECOLORMAP_TYPE);
  if(nsmoke3d_co2>0)Smoke3dCB(CO2COLORMAP_TYPE);
}

/* ------------------ SetRGBColorMapVars ------------------------ */

void SetRGBColorMapVars(int use_rgb){
  int use_colormap;

  use_colormap       = 1 - use_rgb;
  use_smoke_rgb      = use_rgb;
  use_smoke_colormap = use_colormap;
  use_fire_rgb       = use_rgb;
  use_fire_colormap  = use_colormap;
  use_co2_rgb        = use_rgb;
  use_co2_colormap   = use_colormap;
  Smoke3dCB(SET_RGB_COLORGB_CHECKBOXES);
  if(use_fire_rgb==1)fire_colormap_type = 0;
  if(use_fire_colormap==1)fire_colormap_type = 1;
  Smoke3dCB(FIRECOLORMAP_TYPE);
  if(use_co2_rgb==1)co2_colormap_type = 0;
  if(use_co2_colormap==1)co2_colormap_type = 1;
  Smoke3dCB(CO2COLORMAP_TYPE);
}

/* ------------------ Smoke3dCB ------------------------ */

extern "C" void Smoke3dCB(int var){
  int i;
  char *tour_label;
  char *vol_prefixptr;

  updatemenu=1;
  switch(var){
  float temp_min, temp_max;

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
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    glutPostRedisplay();
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
    Smoke3dCB(USE_FIRE_ALPHA);
    break;
  case USE_OPACITY_MULTIPLIER:
    if(have_fire!=NO_FIRE&&have_smoke==NO_SMOKE){
      use_opacity_depth      = 1;
      use_opacity_multiplier = 0;
    }
    glui_use_fire_alpha = use_opacity_multiplier;
    use_opacity_depth =  1 - use_opacity_multiplier;
    CHECKBOX_use_opacity_depth->set_int_val(use_opacity_depth);
    CHECKBOX_use_opacity_multiplier->set_int_val(use_opacity_multiplier);
    Smoke3dCB(USE_FIRE_ALPHA);
    break;
  case BACKGROUND_FLIP:
    background_flip = 1-background_flip;
    ShowHideMenu(MENU_SHOWHIDE_FLIP);
    updatemenu = 1;
    break;
  case SMOKE_BLACK:
    break;
  case SMOKE_SKIP:
    if(smoke3d_skip<1)smoke3d_skip = 1;
    SPINNER_smoke3d_skip->set_int_val(smoke3d_skip);
    smoke3d_skipx = smoke3d_skip;
    smoke3d_skipy = smoke3d_skip;
    smoke3d_skipz = smoke3d_skip;
    SPINNER_smoke3d_skipx->set_int_val(smoke3d_skipx);
    SPINNER_smoke3d_skipy->set_int_val(smoke3d_skipy);
    SPINNER_smoke3d_skipz->set_int_val(smoke3d_skipz);
    break;
  case SMOKE_SKIP_XYZ:
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
    break;
  case CO2SMOKE:
    UpdateCO2Colormap();
    Smoke3dCB(CO2COLORMAP_TYPE);
    Smoke3dCB(UPDATE_SMOKECOLORS);
    glutPostRedisplay();
    break;
  case UPDATE_HRRPUV_CONTROLS:
    if(
      SPINNER_slicehrrpuv_upper != NULL&&
      SPINNER_slicehrrpuv_middle != NULL&&
      SPINNER_slicehrrpuv_lower != NULL&&
      SPINNER_slicehrrpuv_cut1 != NULL&&
      SPINNER_slicehrrpuv_cut2 != NULL){
      if(smoke3d_testsmoke == 0){
        SPINNER_slicehrrpuv_upper->disable();
        SPINNER_slicehrrpuv_middle->disable();
        SPINNER_slicehrrpuv_lower->disable();
        SPINNER_slicehrrpuv_cut1->disable();
        SPINNER_slicehrrpuv_cut2->disable();
        SPINNER_hrrpuvoffset->disable();
      }
      else{
        if(SPINNER_slicehrrpuv_upper != NULL)SPINNER_slicehrrpuv_upper->enable();
        SPINNER_slicehrrpuv_middle->enable();
        SPINNER_slicehrrpuv_lower->enable();
        SPINNER_slicehrrpuv_cut1->enable();
        SPINNER_slicehrrpuv_cut2->enable();
        SPINNER_hrrpuvoffset->enable();
      }
    }
    break;
  case UPDATE_FACTOROFFSETS:
    for(i = 0;i<nmeshes;i++){
      meshdata *meshi;

      meshi = meshinfo+i;
      meshi->update_smoke3dcolors = 1;
    }
    UpdateOpacityMap();
    glutPostRedisplay();
    break;
  case LOAD_SMOKEFRAME:
    LoadSmokeFrame(-1, smoke_framenumber);
    break;
  case LOAD_TIMEFRAME:
    LoadTimeFrame(-1, time_frameval);
    break;
  case LIGHT_XYZ:
    break;
  case LIGHT_UPDATE:
    VEC3EQ(xyz_light_global,xyz_light_glui);
    update_vol_lights = 1;
    light_type_global = light_type_glui;
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
    if(strlen(vol_prefixptr)==0)vol_prefixptr=fdsprefix;
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
      ShowGluiColorbar();
    }
    else{
      HideGluiColorbar();
    }
    break;
  case TEMP_MIN:
    if(global_temp_min < 0.0){
      global_temp_min = 0.0;
      SPINNER_temperature_min->set_float_val(global_temp_min);
    }
    if(global_temp_max<global_temp_min){
      global_temp_max = global_temp_min+1.0;
      SPINNER_temperature_max->set_float_val(global_temp_max);
    }
    UpdateSmokeColormap(smoke_render_option);
    break;
  case EXTINCTION_RESET_FDS:
    if(SOOT_index>=0){
      glui_smoke3d_extinct = smoke3dtypes[SOOT_index].extinction;
      if(SPINNER_smoke3d_extinct2!=NULL)SPINNER_smoke3d_extinct2->set_float_val(glui_smoke3d_extinct);
      if(SPINNER_smoke3d_extinct!=NULL)SPINNER_smoke3d_extinct->set_float_val(glui_smoke3d_extinct);
      Smoke3dCB(SMOKE_EXTINCT);
    }
    break;
  case EXTINCTION_RESET_SMV:
    glui_smoke3d_extinct = glui_smoke3d_extinct_default;
    if(SPINNER_smoke3d_extinct2!=NULL)SPINNER_smoke3d_extinct2->set_float_val(glui_smoke3d_extinct);
    if(SPINNER_smoke3d_extinct!=NULL)SPINNER_smoke3d_extinct->set_float_val(glui_smoke3d_extinct);
    Smoke3dCB(SMOKE_EXTINCT);
    break;
  case CUTOFF_RESET:
    global_hrrpuv_cutoff = global_hrrpuv_cutoff_default;
    global_temp_cutoff = global_temp_cutoff_default;
    SPINNER_temperature_cutoff->set_float_val(global_temp_cutoff);
    SPINNER_hrrpuv_cutoff->set_float_val(global_hrrpuv_cutoff);
    break;
  case TEMP_CUTOFF:
    temp_min = (float)(10*(int)(global_temp_min/10.0) + 10.0);
    temp_max = (float)(10*(int)(global_temp_max/10.0) - 10.0);
    SPINNER_temperature_cutoff->set_float_limits(temp_min,temp_max);
    UpdateSmokeColormap(smoke_render_option);
    glutPostRedisplay();
    break;
  case TEMP_MAX:
    if(global_temp_max<global_temp_min){
      global_temp_max = global_temp_min+1.0;
      SPINNER_temperature_max->set_float_val(global_temp_max);
    }
    UpdateSmokeColormap(smoke_render_option);
    break;
  case LOAD_COMPRESSED_DATA:
    if(load_volcompressed==1){
      CHECKBOX_compress_volsmoke->disable();
    }
    else{
      CHECKBOX_compress_volsmoke->enable();
    }
    break;
  case SMOKE_OPTIONS:
    if(nsmoke3d_temp==0&&smoke_render_option==RENDER_SLICE){
      fire_colormap_type=fire_colormap_type_save;
      Smoke3dCB(SET_RGB_COLORGB_CHECKBOXES);
    }
    else{
      fire_colormap_type_save=fire_colormap_type;
      fire_colormap_type=FIRECOLORMAP_CONSTRAINT;
      Smoke3dCB(SET_RGB_COLORGB_CHECKBOXES);
    }
    Smoke3dCB(FIRECOLORMAP_TYPE);
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
    Smoke3dCB(UPDATE_SMOKECOLORS);
    break;
  case SET_RGB_COLORGB_CHECKBOXES:
    if(CHECKBOX_use_fire_rgb!=NULL)CHECKBOX_use_fire_rgb->set_int_val(use_fire_rgb);
    if(CHECKBOX_use_fire_colormap!=NULL)CHECKBOX_use_fire_colormap->set_int_val(use_fire_colormap);
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
    if(use_smoke_colormap==1){
      PANEL_fire_color->disable ();
      PANEL_colormap3->enable();
    }
    else{
      PANEL_fire_color->enable();
      PANEL_colormap3->disable();
    }
    break;
  case USE_SMOKE_RGB:
    SetRGBColorMapVars(use_smoke_rgb);
    break;
  case USE_FIRE_RGB:
    SetRGBColorMapVars(use_fire_rgb);
    break;
  case USE_FIRE_COLORMAP:
    SetRGBColorMapVars(1 - use_fire_colormap);
    break;
  case USE_CO2_RGB:
    SetRGBColorMapVars(use_co2_rgb);
    break;
  case USE_CO2_COLORMAP:
    SetRGBColorMapVars(1 - use_co2_colormap);
    break;
  case FIRECOLORMAP_TYPE:
    Smoke3dCB(SET_RGB_COLORGB_CHECKBOXES);
    switch(fire_colormap_type){
    case FIRECOLORMAP_CONSTRAINT:
      if(fire_colorbar_index_save != -1){
        SmokeColorbarMenu(fire_colorbar_index_save);
      }
      else{
        SmokeColorbarMenu(fire_colorbar_index);
      }
      break;
    case FIRECOLORMAP_DIRECT:
      fire_colorbar_index_save = fire_colorbar_index;
      UpdateRGBColors(COLORBAR_INDEX_NONE);
      UpdateSmokeColormap(smoke_render_option);
      break;
    default:
#ifdef _DEBUG
      abort();
#endif
      break;
    }
    if(LISTBOX_smoke_colorbar->get_int_val()!=fire_colorbar_index){
      LISTBOX_smoke_colorbar->set_int_val(fire_colorbar_index);
    }
    UpdateSmokeColormap(smoke_render_option);
    break;
  case SMOKE_COLORBAR_LIST:
    SmokeColorbarMenu(fire_colorbar_index);
    UpdateSmokeColormap(smoke_render_option);
    updatemenu=1;
    break;
  case CO2_COLORBAR_LIST:
    UpdateCO2Colormap();
    Smoke3dCB(UPDATE_SMOKECOLORS);
    break;
  case SAVE_SETTINGS_SMOKE:
    WriteIni(LOCAL_INI,NULL);
    break;
  case GLOBAL_FIRE_CUTOFF:
    glutPostRedisplay();
    ForceIdle();
    UpdateSmokeColormap(smoke_render_option);
    break;
  case UPDATE_SMOKEFIRE_COLORS2:
    fire_halfdepth2 = MAX(fire_halfdepth2, 0.001);
    SPINNER_smoke3d_fire_halfdepth->set_float_val(fire_halfdepth2);
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    break;
  case UPDATE_SMOKEFIRE_COLORS:
    co2_halfdepth = MAX(co2_halfdepth, 0.001);
    if(SPINNER_smoke3d_co2_halfdepth!=NULL)SPINNER_smoke3d_co2_halfdepth->set_float_val(co2_halfdepth);

    if(SPINNER_smoke3d_fire_halfdepth2!=NULL)SPINNER_smoke3d_fire_halfdepth2->set_float_val(fire_halfdepth);
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    break;
  case CO2_COLOR:
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
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
    Smoke3dCB(USE_SMOKE_RGB);
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    break;
  case FIRE_RED:
  case FIRE_GREEN:
  case FIRE_BLUE:
    Smoke3dCB(USE_FIRE_RGB);
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);
    break;
  case UPDATE_SMOKEFIRE_COLORS_COMMON:
    glutPostRedisplay();
    force_redisplay = 1;
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    UpdateSmokeColormap(smoke_render_option);
    Smoke3dCB(UPDATE_SMOKECOLORS);
    IdleCB();
    break;
  case UPDATE_SMOKECOLORS:
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;

      meshi = meshinfo + i;
      meshi->update_smoke3dcolors=1;
    }
    glutPostRedisplay();
    force_redisplay=1;
    UpdateSmokeColormap(RENDER_SLICE);
    UpdateSmokeColormap(smoke_render_option);
    IdleCB();
     break;
   case SMOKE_EXTINCT:
     update_smoke_alphas = 1;
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

      vr = &meshinfo->volrenderinfo;
      if(vr!=NULL&&vr->smokeslice!=NULL&&vr->smokeslice->slice_filetype==SLICE_CELL_CENTER){
        if(usegpu==1&&combine_meshes==1){
          combine_meshes=0;
          UpdateCombineMeshes();
          Smoke3dCB(COMBINE_MESHES);
        }
        if(usegpu==0&&combine_meshes==0){
          combine_meshes=1;
          UpdateCombineMeshes();
          Smoke3dCB(COMBINE_MESHES);
        }
      }
    }
    if(have_fire==HRRPUV_index&&smoke_render_option==RENDER_SLICE){
#ifdef pp_GPU
      if(usegpu==1){
        RADIO_skipframes->set_int_val(0);
        RADIO_skipframes->disable();
      }
      else{
        RADIO_skipframes->enable();
      }
#else
      RADIO_skipframes->enable();
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
