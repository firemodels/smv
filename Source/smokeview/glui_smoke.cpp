#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "IOvolsmoke.h"

extern GLUI_Rollout *ROLLOUT_smoke3d;
extern GLUI *glui_bounds;

#define FIRE_RED 1
#define FIRE_GREEN 2
#define FIRE_BLUE 3
#define SMOKE_RED 51
#define SMOKE_GREEN 52
#define SMOKE_BLUE 53
#ifndef UPDATE_SMOKEFIRE_COLORS
#define UPDATE_SMOKEFIRE_COLORS 54
#endif
#define UPDATE_SMOKEFIRE_COLORS2 61
#define UPDATE_SMOKEFIRE_COLORS_COMMON 62
#define CO2SMOKE 63
#define SOOTSMOKE 64
#define UPDATE_SMOKECOLORS 4
#define GLOBAL_FIRE_CUTOFF 15
#define SMOKE_SHADE 7
#define SMOKE_COLORBAR_LIST 16
#define FIRECOLORMAP_TYPE 17
#define SHOW_FIRECOLORMAP 25
#ifdef pp_GPU
#define SMOKE_RTHICK 8
#else
#define SMOKE_THICK 8
#endif
#define SAVE_SETTINGS 9
#define FRAMELOADING 10
#define SMOKETEST 11
#ifdef pp_CULL
#define CULL_SMOKE 12
#define CULL_PORTSIZE 14
#endif
#define VOL_SMOKE 13
#define VOL_NGRID 18
#define SMOKE_OPTIONS 19
#define LOAD_COMPRESSED_DATA 20
#define TEMP_MIN 21
#define TEMP_CUTOFF 22
#define TEMP_MAX 23
#define COMBINE_MESHES 24
#define NONGPU_VOL_FACTOR 26
#define GPU_VOL_FACTOR 27
#define GENERATE_IMAGES 28
#define START_FRAME 29
#define SKIP_FRAME 30
#define CANCEL_GENERATE_IMAGES 31
#define VOL_TOUR_LIST 46
#define VOL_PREFIX 47
#define VOL_UNLOAD_ALL 48
#define LIGHT_XYZ 49
#define LIGHT_UPDATE 50
#define LOAD_SMOKEFRAME 55
#define LOAD_TIMEFRAME 58
#define UPDATE_FACTOROFFSETS 59
#define UPDATE_HRRPUV_CONTROLS 60
#define SMOKE3D_LOAD_INCREMENTAL 18
#define CO2_COLOR 71
#define UPDATE_FIRE_ALPHA 72
#define UPDATE_CO2_ALPHA 73


// two defines below are also defined elsewhere

#define SCRIPT_CANCEL_NOW 45
#define TOUR_LIST 24

GLUI *glui_3dsmoke=NULL;

GLUI_EditText *EDIT_vol_prefix=NULL;

GLUI_Listbox *LISTBOX_VOL_tour=NULL;

GLUI_Button *BUTTON_volunload=NULL;
GLUI_Button *BUTTON_startrender=NULL;
GLUI_Button *BUTTON_cancelrender=NULL;

GLUI_Listbox *LISTBOX_smoke_colorbar=NULL;

GLUI_RadioGroup *RADIO_alpha=NULL;
GLUI_RadioGroup *RADIO_render=NULL;
GLUI_RadioGroup *RADIO_skipframes=NULL;
GLUI_RadioGroup *RADIO_smokesensors=NULL;
GLUI_RadioGroup *RADIO_loadvol=NULL;
GLUI_RadioGroup *RADIO_use_colormap=NULL;
GLUI_RadioGroup *RADIO_light_type = NULL;
GLUI_RadioGroup *RADIO_scatter_type_glui = NULL;

GLUI_Spinner *SPINNER_sootfactor=NULL;
GLUI_Spinner *SPINNER_co2factor=NULL;
GLUI_Spinner *SPINNER_startframe=NULL;
GLUI_Spinner *SPINNER_skipframe=NULL;
#ifdef pp_CULL
GLUI_Spinner *SPINNER_cull_portsize=NULL;
#endif
GLUI_Spinner *SPINNER_hrrpuv_cutoff=NULL;
GLUI_Spinner *SPINNER_nongpu_vol_factor=NULL;
GLUI_Spinner *SPINNER_gpu_vol_factor=NULL;

GLUI_Spinner *SPINNER_temperature_min=NULL;
GLUI_Spinner *SPINNER_temperature_cutoff=NULL;
GLUI_Spinner *SPINNER_temperature_max=NULL;
GLUI_Spinner *SPINNER_fire_opacity_factor=NULL;
GLUI_Spinner *SPINNER_mass_extinct=NULL;
GLUI_Spinner *SPINNER_cvis=NULL;
GLUI_Spinner *SPINNER_smokedrawtest_nummin=NULL;
GLUI_Spinner *SPINNER_smokedrawtest_nummax=NULL;
#ifdef pp_GPU
GLUI_Spinner *SPINNER_smoke3d_rthick=NULL;
#else
GLUI_Spinner *SPINNER_smoke3d_thick=NULL;
#endif
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
GLUI_Spinner *SPINNER_extinct=NULL;
GLUI_Spinner *SPINNER_smokedens = NULL;
GLUI_Spinner *SPINNER_pathlength = NULL;
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

GLUI_Checkbox *CHECKBOX_freeze = NULL;
GLUI_Checkbox *CHECKBOX_combine_meshes = NULL;
#ifdef pp_CULL
GLUI_Checkbox *CHECKBOX_show_cullports = NULL;
#endif
GLUI_Checkbox *CHECKBOX_compress_volsmoke = NULL;
GLUI_Checkbox *CHECKBOX_smokecullflag = NULL;
GLUI_Checkbox *CHECKBOX_test_smokesensors = NULL;
GLUI_Checkbox *CHECKBOX_smokeGPU = NULL;
GLUI_Checkbox *CHECKBOX_smokedrawtest = NULL;
GLUI_Checkbox *CHECKBOX_smokedrawtest2 = NULL;
GLUI_Checkbox *CHECKBOX_zlib = NULL;
GLUI_Checkbox **CHECKBOX_meshvisptr = NULL;
GLUI_Checkbox *CHECKBOX_meshvis = NULL;
GLUI_Checkbox *CHECKBOX_show_smoketest = NULL;
GLUI_Checkbox *CHECKBOX_show_light_position_direction = NULL;
GLUI_Checkbox *CHECKBOX_smoke3d_load_incremental=NULL;
GLUI_Checkbox *CHECKBOX_edit_colormap=NULL;

GLUI_Panel *PANEL_fire_cutoff = NULL;
GLUI_Panel *PANEL_overall = NULL;
GLUI_Panel *PANEL_colormap2 = NULL;
GLUI_Panel *PANEL_colormap = NULL;
GLUI_Panel *PANEL_absorption = NULL, *PANEL_smokesensor = NULL;
GLUI_Panel *PANEL_testsmoke = NULL;
GLUI_Panel *PANEL_color = NULL;
GLUI_Panel *PANEL_smoke = NULL;
GLUI_Panel *PANEL_loadcutoff = NULL;
GLUI_Panel *PANEL_light_color = NULL;
GLUI_Panel *PANEL_light_position = NULL;
GLUI_Panel *PANEL_scatter = NULL;
GLUI_Panel *PANEL_loadframe = NULL;
GLUI_Panel *PANEL_voldisplay = NULL;
GLUI_Panel *PANEL_volsmoke_move = NULL;
GLUI_Panel *PANEL_alpha = NULL;

#ifdef pp_SMOKETEST
GLUI_Rollout *ROLLOUT_voltemp = NULL;
#endif
GLUI_Rollout *ROLLOUT_slicehrrpuv = NULL;
GLUI_Rollout *ROLLOUT_firecolor = NULL;
GLUI_Rollout *ROLLOUT_smokecolor = NULL;
GLUI_Rollout *ROLLOUT_generate_images = NULL;
GLUI_Rollout *ROLLOUT_loadframe = NULL;
GLUI_Rollout *ROLLOUT_light = NULL;
#ifdef pp_SMOKETEST
GLUI_Rollout *ROLLOUT_colormap_temp = NULL;
#endif
GLUI_Rollout *ROLLOUT_co2color = NULL;
GLUI_Rollout *ROLLOUT_temperature_settings=NULL;
#ifdef pp_SMOKETEST
GLUI_Rollout *ROLLOUT_colormap_hrrpuv = NULL;
#endif
GLUI_Rollout *ROLLOUT_meshvis = NULL;
GLUI_Rollout *ROLLOUT_slices = NULL;
GLUI_Rollout *ROLLOUT_volume = NULL;
GLUI_Rollout *ROLLOUT_smoke_test = NULL;
GLUI_Rollout *ROLLOUT_volsmoke_load = NULL;
GLUI_Rollout *ROLLOUT_volsmoke_compute = NULL;
GLUI_Rollout *ROLLOUT_smokedebug = NULL;
GLUI_Rollout *ROLLOUT_display=NULL;
GLUI_Rollout *ROLLOUT_slicegpu=NULL;

GLUI_StaticText *TEXT_smokealpha = NULL;
GLUI_StaticText *TEXT_smokedepth = NULL;
GLUI_StaticText *STATIC_smokeframelimit_min = NULL;
GLUI_StaticText *STATIC_smokeframelimit_max = NULL;
GLUI_StaticText *STATIC_globalframelimit_min = NULL;
GLUI_StaticText *STATIC_globalframelimit_max = NULL;
GLUI_StaticText *STATIC_timelimit_min = NULL;
GLUI_StaticText *STATIC_timelimit_max = NULL;


#define VOLRENDER_ROLLOUT 0
#define SLICERENDER_ROLLOUT 1

procdata smokeprocinfo[2];
int nsmokeprocinfo = 0;

#define FIRECOLOR_ROLLOUT 0
#define SMOKECOLOR_ROLLOUT 1
#define CO2COLOR_ROLLOUT 2

procdata colorprocinfo[3];
int ncolorprocinfo = 0;

/* ------------------ LoadIncrementalCB ------------------------ */

extern "C" void LoadIncrementalCB(int var){
  LoadIncrementalCB1(var);
  LoadIncrementalCB2(var);
  updatemenu=1;
}

/* ------------------ LoadIncrementalCB2 ------------------------ */

extern "C" void LoadIncrementalCB2(int var){
  if(CHECKBOX_smoke3d_load_incremental != NULL)CHECKBOX_smoke3d_load_incremental->set_int_val(load_incremental);
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
  RADIO_alpha->set_int_val(adjustalphaflag);
#ifdef pp_GPU
  if(CHECKBOX_smokeGPU!=NULL)CHECKBOX_smokeGPU->set_int_val(usegpu);
#endif
#ifdef pp_CULL
  CHECKBOX_smokecullflag->set_int_val(cullsmoke);
#else
  CHECKBOX_smokecullflag->set_int_val(smokecullflag);
#endif
  if(CHECKBOX_smokedrawtest!=NULL)CHECKBOX_smokedrawtest->set_int_val(smokedrawtest);
  if(CHECKBOX_smokedrawtest2!=NULL)CHECKBOX_smokedrawtest2->set_int_val(smokedrawtest2);
  RADIO_skipframes->set_int_val(smokeskipm1);
  Smoke3dCB(VOL_SMOKE);
#ifdef pp_CULL
  Smoke3dCB(CULL_SMOKE);
#endif
  glutPostRedisplay();
}

/* ------------------ UpdateAlpha ------------------------ */

void UpdateAlpha(void){
  char label[100];
  char label1[100],label2[100],label3[100];
  float depth;
  float factor;

  factor = 1.0 - exp(-smoke_extinct*smoke_dens*smoke_pathlength);
  smoke_alpha = (int)(255*factor);
  if(smoke_alpha<0)smoke_alpha=0;
  if(smoke_alpha>255)smoke_alpha=255;
  sprintf(label1,"%f",smoke_extinct);
  TrimZeros(label1);
  sprintf(label2,"%f",smoke_dens);
  TrimZeros(label2);
  sprintf(label3,"%f",smoke_pathlength);
  TrimZeros(label3);
  sprintf(label,"alpha=%i=255*(1.0-exp(-%s*%s*%s))",smoke_alpha,label1,label2,label3);
  if(PANEL_testsmoke!=NULL){
    TEXT_smokealpha->set_text(label);
  }

  if(smoke_extinct!=0.0&&smoke_dens!=0){
    depth=0.693147/(smoke_extinct*smoke_dens);
    sprintf(label,"50%s smoke depth=%f","%",depth);
  }
  else{
    sprintf(label,"50%s smoke depth=***","%");
  }
  if(PANEL_testsmoke!=NULL){
    TEXT_smokedepth->set_text(label);
  }
}

/* ------------------ Glui3dSmokeSetup ------------------------ */

extern "C" void Glui3dSmokeSetup(int main_window){

  int i;


  if(nsmoke3dinfo<=0&&nvolrenderinfo<=0)return;
  if(CHECKBOX_meshvisptr!=NULL)FREEMEMORY(CHECKBOX_meshvisptr);
  NewMemory((void **)&CHECKBOX_meshvisptr,nmeshes*sizeof(GLUI_Checkbox *));

  glui_3dsmoke=glui_bounds;

  if(smoketest==1){
    PANEL_testsmoke = glui_3dsmoke->add_panel_to_panel(ROLLOUT_smoke3d,_("Test smoke"));
    PANEL_testsmoke->set_alignment(GLUI_ALIGN_LEFT);
    CHECKBOX_show_smoketest=glui_3dsmoke->add_checkbox_to_panel(PANEL_testsmoke,_("Show test smoke"),&show_smoketest);
    SPINNER_extinct=glui_3dsmoke->add_spinner_to_panel(PANEL_testsmoke,_("Mass extinction coeff (m2/g)"),GLUI_SPINNER_FLOAT,&smoke_extinct,SMOKETEST,Smoke3dCB);
    SPINNER_extinct->set_float_limits(0.0,10.0);
    SPINNER_smokedens=glui_3dsmoke->add_spinner_to_panel(PANEL_testsmoke,_("Smoke density (g/m3)"),GLUI_SPINNER_FLOAT,&smoke_dens,SMOKETEST,Smoke3dCB);
    SPINNER_smokedens->set_float_limits(0.0,1.0);
    SPINNER_pathlength=glui_3dsmoke->add_spinner_to_panel(PANEL_testsmoke,_("Path length (m)"),GLUI_SPINNER_FLOAT,&smoke_pathlength,SMOKETEST,Smoke3dCB);
    SPINNER_pathlength->set_float_limits(0.0,20.0);
    TEXT_smokealpha=glui_3dsmoke->add_statictext_to_panel(PANEL_testsmoke,_("Alpha"));
    TEXT_smokedepth=glui_3dsmoke->add_statictext_to_panel(PANEL_testsmoke,_("Depth"));
    UpdateAlpha();
  }

  PANEL_overall = glui_3dsmoke->add_panel_to_panel(ROLLOUT_smoke3d,"",GLUI_PANEL_NONE);
#ifdef pp_GPU
  CHECKBOX_smokeGPU=glui_3dsmoke->add_checkbox_to_panel(PANEL_overall,_("Use GPU"),&usegpu,VOL_SMOKE,Smoke3dCB);
#endif

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

  PANEL_colormap = glui_3dsmoke->add_panel_to_panel(PANEL_overall,_("Color/opacity"));

  RADIO_use_colormap = glui_3dsmoke->add_radiogroup_to_panel(PANEL_colormap, &firecolormap_type, FIRECOLORMAP_TYPE, Smoke3dCB);
  glui_3dsmoke->add_radiobutton_to_group(RADIO_use_colormap, _("Use red/green/blue"));
  glui_3dsmoke->add_radiobutton_to_group(RADIO_use_colormap, _("Use colormap"));

  ROLLOUT_smokecolor = glui_3dsmoke->add_rollout_to_panel(PANEL_colormap, "smoke",false, SMOKECOLOR_ROLLOUT, ColorRolloutCB);
  ADDPROCINFO(colorprocinfo, ncolorprocinfo, ROLLOUT_smokecolor, SMOKECOLOR_ROLLOUT);

  SPINNER_smoke3d_smoke_red = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smokecolor, _("red"), GLUI_SPINNER_INT, &smoke_red, SMOKE_RED, Smoke3dCB);
  SPINNER_smoke3d_smoke_red->set_int_limits(0, 255);
  SPINNER_smoke3d_smoke_green = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smokecolor, _("green"), GLUI_SPINNER_INT, &smoke_green, SMOKE_GREEN, Smoke3dCB);
  SPINNER_smoke3d_smoke_green->set_int_limits(0, 255);
  SPINNER_smoke3d_smoke_blue = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smokecolor, _("blue"), GLUI_SPINNER_INT, &smoke_blue, SMOKE_BLUE, Smoke3dCB);
  SPINNER_smoke3d_smoke_blue->set_int_limits(0, 255);

  ROLLOUT_firecolor = glui_3dsmoke->add_rollout_to_panel(PANEL_colormap, _("fire"),false, FIRECOLOR_ROLLOUT, ColorRolloutCB);
  ADDPROCINFO(colorprocinfo, ncolorprocinfo, ROLLOUT_firecolor, FIRECOLOR_ROLLOUT);

  SPINNER_smoke3d_fire_red = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_firecolor, _("red"), GLUI_SPINNER_INT, &fire_red, FIRE_RED, Smoke3dCB);
  SPINNER_smoke3d_fire_red->set_int_limits(0,255);
  SPINNER_smoke3d_fire_green=glui_3dsmoke->add_spinner_to_panel(ROLLOUT_firecolor,_("green"),GLUI_SPINNER_INT,&fire_green,FIRE_GREEN,Smoke3dCB);
  SPINNER_smoke3d_fire_green->set_int_limits(0,255);
  SPINNER_smoke3d_fire_blue=glui_3dsmoke->add_spinner_to_panel(ROLLOUT_firecolor,_("blue"),GLUI_SPINNER_INT,&fire_blue,FIRE_BLUE,Smoke3dCB);
  SPINNER_smoke3d_fire_blue->set_int_limits(0,255);

  if(ncolorbars > 0){
    LISTBOX_smoke_colorbar = glui_3dsmoke->add_listbox_to_panel(ROLLOUT_firecolor, "colormap:", &fire_colorbar_index, SMOKE_COLORBAR_LIST, Smoke3dCB);
    for(i = 0;i < ncolorbars;i++){
      colorbardata *cbi;

      cbi = colorbarinfo + i;
      cbi->label_ptr = cbi->label;
      LISTBOX_smoke_colorbar->add_item(i, cbi->label_ptr);
    }
    LISTBOX_smoke_colorbar->set_int_val(fire_colorbar_index);
    CHECKBOX_edit_colormap=glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_firecolor, "Edit colormap", &show_firecolormap, SHOW_FIRECOLORMAP, Smoke3dCB);
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

  SPINNER_smoke3d_fire_halfdepth = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_firecolor, _("50% fire opacity (m)"), GLUI_SPINNER_FLOAT, &fire_halfdepth, UPDATE_SMOKEFIRE_COLORS, Smoke3dCB);
  SPINNER_smoke3d_fire_halfdepth->set_float_limits(0.0, 100.0);

#ifdef pp_SMOKETEST
  if (nsmoke3d_temp > 0) {
    char smokelabel[256];

    ROLLOUT_colormap_temp = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_firecolor, "Temperature");

    strcpy(smokelabel, _("settings (test)"));
    ROLLOUT_temperature_settings = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_colormap_temp, smokelabel, false);
    SPINNER_temperature_min = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_temperature_settings, _("min"), GLUI_SPINNER_FLOAT,
      &global_temp_min, TEMP_MIN, Smoke3dCB);
    SPINNER_temperature_max = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_temperature_settings, _("max"), GLUI_SPINNER_FLOAT,
      &global_temp_max, TEMP_MAX, Smoke3dCB);
    Smoke3dCB(TEMP_MIN);
    Smoke3dCB(TEMP_CUTOFF);
    Smoke3dCB(TEMP_MAX);
  }
#endif

  if(nsmoke3d_co2 > 0){
    ROLLOUT_co2color=glui_3dsmoke->add_rollout_to_panel(PANEL_colormap, "CO2", false, CO2COLOR_ROLLOUT, ColorRolloutCB);
    ADDPROCINFO(colorprocinfo, ncolorprocinfo, ROLLOUT_co2color, CO2COLOR_ROLLOUT);
    SPINNER_co2color[0] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("red"), GLUI_SPINNER_INT, global_co2color, CO2_COLOR, Smoke3dCB);
    SPINNER_co2color[1] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("green"), GLUI_SPINNER_INT, global_co2color + 1, CO2_COLOR, Smoke3dCB);
    SPINNER_co2color[2] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("blue"), GLUI_SPINNER_INT, global_co2color + 2, CO2_COLOR, Smoke3dCB);
    SPINNER_co2color[0]->set_int_limits(0, 255);
    SPINNER_co2color[1]->set_int_limits(0, 255);
    SPINNER_co2color[2]->set_int_limits(0, 255);
    if(nco2files > 0){
      SPINNER_smoke3d_co2_halfdepth = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("50% CO2 opacity (m)"), GLUI_SPINNER_FLOAT, &co2_halfdepth, UPDATE_SMOKEFIRE_COLORS, Smoke3dCB);
      SPINNER_sootfactor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("soot factor"), GLUI_SPINNER_FLOAT, &sootfactor, SOOTSMOKE, Smoke3dCB);
      SPINNER_co2factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_co2color, _("CO2 factor"), GLUI_SPINNER_FLOAT, &co2factor, CO2SMOKE, Smoke3dCB);
    }
  }

#ifdef pp_SMOKEALPHA
  PANEL_alpha = glui_3dsmoke->add_panel_to_panel(PANEL_colormap,"experimental");
  SPINNER_smoke3d_fire_alpha = glui_3dsmoke->add_spinner_to_panel(PANEL_alpha, _("50% fire alpha"), GLUI_SPINNER_INT, &glui_fire_alpha, UPDATE_FIRE_ALPHA, Smoke3dCB);
  SPINNER_smoke3d_fire_alpha->set_int_limits(1,254);
  if(nco2files>0){
    SPINNER_smoke3d_co2_alpha = glui_3dsmoke->add_spinner_to_panel(PANEL_alpha, _("50% CO2 alpha"), GLUI_SPINNER_INT, &glui_co2_alpha, UPDATE_CO2_ALPHA, Smoke3dCB);
    SPINNER_smoke3d_co2_alpha->set_int_limits(1, 254);
  }
#endif
  Smoke3dCB(UPDATE_SMOKEFIRE_COLORS);
  Smoke3dCB(UPDATE_SMOKEFIRE_COLORS2);


  PANEL_colormap2 = glui_3dsmoke->add_panel_to_panel(PANEL_colormap,"",GLUI_PANEL_NONE);

#ifdef pp_SMOKETEST
  ROLLOUT_colormap_hrrpuv = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_firecolor, "HRRPUV");
  ROLLOUT_slicehrrpuv = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_colormap_hrrpuv, _("Opacity correction (test)"),false);
  glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_slicehrrpuv, "Implement", &smoke3d_testsmoke, UPDATE_HRRPUV_CONTROLS, Smoke3dCB);
  SPINNER_slicehrrpuv_upper = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_slicehrrpuv, _("50% upper opacity (m)"), GLUI_SPINNER_FLOAT, &slicehrrpuv_upper, UPDATE_FACTOROFFSETS, Smoke3dCB);
  SPINNER_slicehrrpuv_cut2 = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_slicehrrpuv, _("middle/upper boundary"), GLUI_SPINNER_FLOAT, &slicehrrpuv_cut2, UPDATE_FACTOROFFSETS, Smoke3dCB);
  SPINNER_slicehrrpuv_cut2->set_float_limits(0.0, 1.0);
  SPINNER_slicehrrpuv_middle = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_slicehrrpuv, _("50% middle opacity (m)"), GLUI_SPINNER_FLOAT, &slicehrrpuv_middle, UPDATE_FACTOROFFSETS, Smoke3dCB);
  SPINNER_slicehrrpuv_cut1 = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_slicehrrpuv, _("lower/middle boundary"), GLUI_SPINNER_FLOAT, &slicehrrpuv_cut1, UPDATE_FACTOROFFSETS, Smoke3dCB);
  SPINNER_slicehrrpuv_cut1->set_float_limits(0.0, 1.0);
  SPINNER_slicehrrpuv_lower = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_slicehrrpuv, _("50% lower opacity (m)"), GLUI_SPINNER_FLOAT, &slicehrrpuv_lower, UPDATE_FACTOROFFSETS, Smoke3dCB);
  SPINNER_hrrpuvoffset = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_slicehrrpuv, _("HRRPUV offset"), GLUI_SPINNER_FLOAT, &slicehrrpuv_offset, UPDATE_FACTOROFFSETS, Smoke3dCB);
  Smoke3dCB(UPDATE_HRRPUV_CONTROLS);

  ROLLOUT_voltemp = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_colormap_hrrpuv, _("Intensity (test)"),false);
  glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltemp, _("factor"), GLUI_SPINNER_FLOAT, &voltemp_factor);
  glui_3dsmoke->add_spinner_to_panel(ROLLOUT_voltemp, _("offset"), GLUI_SPINNER_FLOAT, &voltemp_offset);
#endif

  glui_3dsmoke->add_column_to_panel(PANEL_overall,false);

  if(nsmoke3dinfo<=0||nvolrenderinfo<=0){
    smoke_render_option=RENDER_SLICE;
    if(nsmoke3dinfo>0)smoke_render_option=RENDER_SLICE;
    if(nvolrenderinfo>0)smoke_render_option=RENDER_VOLUME;
  }

  // slice render dialog

  if(nsmoke3dinfo>0){
    ROLLOUT_slices = glui_3dsmoke->add_rollout_to_panel(PANEL_overall,_("Slice rendered"),false, SLICERENDER_ROLLOUT, SmokeRolloutCB);
    ADDPROCINFO(smokeprocinfo, nsmokeprocinfo, ROLLOUT_slices, SLICERENDER_ROLLOUT);
    ROLLOUT_slices->set_alignment(GLUI_ALIGN_LEFT);

#ifdef pp_GPU
    if(gpuactive==0){
      usegpu=0;
      CHECKBOX_smokeGPU->disable();
    }
    ROLLOUT_slicegpu = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_slices, "GPU", false);
#endif
#ifdef pp_CULL
    CHECKBOX_smokecullflag=glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_slicegpu,_("Cull hidden slices"),&cullsmoke,CULL_SMOKE,Smoke3dCB);
    if(cullactive==0){
      cullsmoke=0;
      CHECKBOX_smokecullflag->disable();
    }
    CHECKBOX_show_cullports=glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_slicegpu,_("Show cull ports"),&show_cullports);
    SPINNER_cull_portsize=glui_3dsmoke->add_spinner_to_panel(ROLLOUT_slicegpu,_("Cull port size"),GLUI_SPINNER_INT,&cull_portsize,CULL_PORTSIZE,Smoke3dCB);
    {
      int ijk_max=0;
      for(i=0;i<nmeshes;i++){
        meshdata *meshi;

        meshi = meshinfo + i;
        if(ijk_max<meshi->ibar+1)ijk_max=meshi->ibar+1;
        if(ijk_max<meshi->jbar+1)ijk_max=meshi->jbar+1;
        if(ijk_max<meshi->kbar+1)ijk_max=meshi->kbar+1;
      }
      SPINNER_cull_portsize->set_int_limits(3,ijk_max);
    }
#else
    CHECKBOX_smokecullflag=glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_slicegpu, _("Cull hidden slices"),&smokecullflag);
#endif
#ifdef _DEBUG

    ROLLOUT_smokedebug = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_slices,_("Debug"),false);

    CHECKBOX_smokedrawtest=glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_smokedebug, _("Show only back slices"),&smokedrawtest);
    CHECKBOX_smokedrawtest2=glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_smokedebug, _("Show only X slices"),&smokedrawtest2);

    SPINNER_smokedrawtest_nummin=glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smokedebug, _("Back slice"),GLUI_SPINNER_INT,&smokedrawtest_nummin);
    SPINNER_smokedrawtest_nummin->set_int_limits(1,ijkbarmax);

    SPINNER_smokedrawtest_nummax=glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smokedebug, _("Front slice"),GLUI_SPINNER_INT,&smokedrawtest_nummax);
    SPINNER_smokedrawtest_nummax->set_int_limits(1,ijkbarmax);
#endif

    ROLLOUT_display=glui_3dsmoke->add_rollout_to_panel(ROLLOUT_slices,_("Slice"),false);
    RADIO_skipframes = glui_3dsmoke->add_radiogroup_to_panel(ROLLOUT_display,&smokeskipm1);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_skipframes,_("Display all"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_skipframes,_("   ... Every 2nd"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_skipframes,_("   ... Every 3rd"));

#ifdef pp_GPU
    SPINNER_smoke3d_rthick=glui_3dsmoke->add_spinner_to_panel(ROLLOUT_display,_("Thickness"),
      GLUI_SPINNER_FLOAT,&smoke3d_rthick,SMOKE_RTHICK,Smoke3dCB);
    SPINNER_smoke3d_rthick->set_float_limits(1.0,255.0);
    smoke3d_thick = LogBase2(smoke3d_rthick);
#else
    SPINNER_smoke3d_thick=glui_3dsmoke->add_spinner_to_panel(PANEL_colormap,"Thickness",
    GLUI_SPINNER_INT,&smoke3d_thick,SMOKE_THICK,Smoke3dCB);
    SPINNER_smoke3d_thick->set_int_limits(0,7);
#endif

    PANEL_absorption = glui_3dsmoke->add_panel_to_panel(ROLLOUT_display,_("Absorption adjustments"));
    PANEL_absorption->set_alignment(GLUI_ALIGN_LEFT);
    RADIO_alpha = glui_3dsmoke->add_radiogroup_to_panel(PANEL_absorption,&adjustalphaflag);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_alpha,_("None"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_alpha,_("adjust off-center"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_alpha,_("zero at boundaries"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_alpha,_("both"));
    CHECKBOX_smoke3d_load_incremental=glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_slices, _("incremental data loading"), &load_incremental, SMOKE3D_LOAD_INCREMENTAL, LoadIncrementalCB);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_slices, _("max blending"), &hrrpuv_max_blending);
  }

  // volume render dialog

  if(nvolrenderinfo > 0){
    ROLLOUT_volume = glui_3dsmoke->add_rollout_to_panel(PANEL_overall, _("Volume rendered"), false, VOLRENDER_ROLLOUT, SmokeRolloutCB);
    ADDPROCINFO(smokeprocinfo, nsmokeprocinfo, ROLLOUT_volume, VOLRENDER_ROLLOUT);

    if(have_volcompressed == 1){
      RADIO_loadvol = glui_3dsmoke->add_radiogroup_to_panel(ROLLOUT_volume, &glui_load_volcompressed, LOAD_COMPRESSED_DATA, Smoke3dCB);
      glui_3dsmoke->add_radiobutton_to_group(RADIO_loadvol, _("Load full data"));
      glui_3dsmoke->add_radiobutton_to_group(RADIO_loadvol, _("Load compressed data"));
    }
    PANEL_voldisplay = glui_3dsmoke->add_panel_to_panel(ROLLOUT_volume,_("Display"),true);
    glui_3dsmoke->add_checkbox_to_panel(PANEL_voldisplay, _("Show"), &usevolrender, VOL_SMOKE, Smoke3dCB);
    glui_3dsmoke->add_checkbox_to_panel(PANEL_voldisplay, _("black/white"), &volbw);
#ifdef _DEBUG
    glui_3dsmoke->add_checkbox_to_panel(PANEL_voldisplay, "block smoke", &block_volsmoke);
    glui_3dsmoke->add_checkbox_to_panel(PANEL_voldisplay, "debug", &smoke3dVoldebug);
#endif

    //*** move
    PANEL_volsmoke_move = glui_3dsmoke->add_panel_to_panel(ROLLOUT_volume, _("Scene movement"), true);
    glui_3dsmoke->add_checkbox_to_panel(PANEL_volsmoke_move, _("Auto freeze"), &autofreeze_volsmoke);
    CHECKBOX_freeze = glui_3dsmoke->add_checkbox_to_panel(PANEL_volsmoke_move, _("Freeze"), &freeze_volsmoke);
    glui_3dsmoke->add_checkbox_to_panel(PANEL_volsmoke_move, _("Show data while moving scene (GPU)"), &show_volsmoke_moving);

    //*** compute
    ROLLOUT_volsmoke_compute = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Compute"), false);
    SPINNER_fire_opacity_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("Fire opacity multiplier"), GLUI_SPINNER_FLOAT, &fire_opacity_factor);
    SPINNER_fire_opacity_factor->set_float_limits(1.0, 50.0);
    SPINNER_mass_extinct = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("Mass extinction coeff (m2/g)"), GLUI_SPINNER_FLOAT, &mass_extinct);
    SPINNER_mass_extinct->set_float_limits(100.0, 100000.0);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_compute, _("adaptive integration"), &vol_adaptive);
    CHECKBOX_combine_meshes = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_compute, _("Combine meshes"), &combine_meshes, COMBINE_MESHES, Smoke3dCB);
    SPINNER_nongpu_vol_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("non-gpu grid multiplier"), GLUI_SPINNER_FLOAT, &nongpu_vol_factor, NONGPU_VOL_FACTOR, Smoke3dCB);
    SPINNER_nongpu_vol_factor->set_float_limits(1.0, 10.0);
    SPINNER_gpu_vol_factor = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_volsmoke_compute, _("gpu grid multiplier"), GLUI_SPINNER_FLOAT, &gpu_vol_factor, GPU_VOL_FACTOR, Smoke3dCB);
    SPINNER_gpu_vol_factor->set_float_limits(1.0, 10.0);

    //*** load
    ROLLOUT_volsmoke_load = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Load"), false);
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_load, _("Load data in background"), &use_multi_threading);
    CHECKBOX_compress_volsmoke = glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_load, _("Compress data while loading"), &glui_compress_volsmoke);
    if(have_volcompressed == 1){
      Smoke3dCB(LOAD_COMPRESSED_DATA);
    }
    glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_volsmoke_load, _("Load data only at render times"), &load_at_rendertimes);

    ROLLOUT_light = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Light"), false);
    PANEL_light_position = glui_3dsmoke->add_panel_to_panel(ROLLOUT_light, _(""));
    RADIO_light_type = glui_3dsmoke->add_radiogroup_to_panel(PANEL_light_position,&light_type_glui);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_light_type,_("position"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_light_type,_("direction"));
    CHECKBOX_show_light_position_direction = glui_3dsmoke->add_checkbox_to_panel(PANEL_light_position, _("Show position/direction"), &show_light_position_direction);
    SPINNER_light_xyz[0] = glui_3dsmoke->add_spinner_to_panel(PANEL_light_position, "x:", GLUI_SPINNER_FLOAT, xyz_light_glui,   LIGHT_XYZ, Smoke3dCB);
    SPINNER_light_xyz[1] = glui_3dsmoke->add_spinner_to_panel(PANEL_light_position, "y:", GLUI_SPINNER_FLOAT, xyz_light_glui+1, LIGHT_XYZ, Smoke3dCB);
    SPINNER_light_xyz[2] = glui_3dsmoke->add_spinner_to_panel(PANEL_light_position, "z:", GLUI_SPINNER_FLOAT, xyz_light_glui+2, LIGHT_XYZ, Smoke3dCB);
    glui_3dsmoke->add_checkbox_to_panel(PANEL_light_position, _("Use light"), &use_light);
    glui_3dsmoke->add_button_to_panel(PANEL_light_position, _("Update"), LIGHT_UPDATE, Smoke3dCB);

    PANEL_light_color = glui_3dsmoke->add_panel_to_panel(ROLLOUT_light, _("color/intensity"));
    SPINNER_light_color[0] = glui_3dsmoke->add_spinner_to_panel(PANEL_light_color,   _("red:"), GLUI_SPINNER_INT, light_color);
    SPINNER_light_color[1] = glui_3dsmoke->add_spinner_to_panel(PANEL_light_color, _("green:"), GLUI_SPINNER_INT, light_color+1);
    SPINNER_light_color[2] = glui_3dsmoke->add_spinner_to_panel(PANEL_light_color,  _("blue:"), GLUI_SPINNER_INT, light_color+2);
    SPINNER_light_color[0]->set_int_limits(0, 255);
    SPINNER_light_color[1]->set_int_limits(0, 255);
    SPINNER_light_color[2]->set_int_limits(0, 255);
    SPINNER_light_intensity = glui_3dsmoke->add_spinner_to_panel(PANEL_light_color, _("intensity:"), GLUI_SPINNER_FLOAT, &light_intensity);
    SPINNER_light_intensity->set_float_limits(0.0,1000.0);

    PANEL_scatter = glui_3dsmoke->add_panel_to_panel(ROLLOUT_light, _("scatter"));
    RADIO_scatter_type_glui = glui_3dsmoke->add_radiogroup_to_panel(PANEL_scatter,&scatter_type_glui);
    glui_3dsmoke->add_radiobutton_to_group(RADIO_scatter_type_glui,_("isotropic"));
    glui_3dsmoke->add_radiobutton_to_group(RADIO_scatter_type_glui,"Henjey-Greenstein");
    glui_3dsmoke->add_radiobutton_to_group(RADIO_scatter_type_glui,"Schlick");
    SPINNER_scatter_param  = glui_3dsmoke->add_spinner_to_panel(PANEL_scatter,   _("param"), GLUI_SPINNER_FLOAT, &scatter_param);
    SPINNER_scatter_param->set_float_limits(-1.0,1.0);

    ROLLOUT_generate_images = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Generate images"), false);

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

    ROLLOUT_loadframe = glui_3dsmoke->add_rollout_to_panel(ROLLOUT_volume, _("Load frame"), false);
    PANEL_loadframe = glui_3dsmoke->add_panel_to_panel(ROLLOUT_loadframe, "", false);

    glui_3dsmoke->add_button_to_panel(PANEL_loadframe, _("Load"), LOAD_SMOKEFRAME, Smoke3dCB);
    glui_3dsmoke->add_button_to_panel(PANEL_loadframe, _("Load"), LOAD_TIMEFRAME, Smoke3dCB);

    glui_3dsmoke->add_column_to_panel(PANEL_loadframe, false);

    SPINNER_smokeloadframe = glui_3dsmoke->add_spinner_to_panel(PANEL_loadframe, _("smoke frame"), GLUI_SPINNER_INT, &smoke_framenumber);
    SPINNER_timeloadframe = glui_3dsmoke->add_spinner_to_panel(PANEL_loadframe, _("time"), GLUI_SPINNER_FLOAT, &time_frameval);
  }

#ifdef pp_SMOKETEST
  // smoke test dialog

  ROLLOUT_smoke_test = glui_3dsmoke->add_rollout_to_panel(PANEL_overall, _("Test"), false);
  glui_3dsmoke->add_checkbox_to_panel(ROLLOUT_smoke_test, _("Show"), &smoke_test);
  SPINNER_smoke_test_color[0] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smoke_test, _("red:"), GLUI_SPINNER_FLOAT, smoke_test_color);
  SPINNER_smoke_test_color[1] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smoke_test, _("green:"), GLUI_SPINNER_FLOAT, smoke_test_color+1);
  SPINNER_smoke_test_color[2] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smoke_test, _("blue:"), GLUI_SPINNER_FLOAT, smoke_test_color+2);
  SPINNER_smoke_test_color[3] = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smoke_test, _("opacity:"), GLUI_SPINNER_FLOAT, &smoke_test_opacity);
  SPINNER_smoke_test_color[0]->set_float_limits(0.0, 1.0);
  SPINNER_smoke_test_color[1]->set_float_limits(0.0, 1.0);
  SPINNER_smoke_test_color[2]->set_float_limits(0.0, 1.0);
  SPINNER_smoke_test_color[3]->set_float_limits(0.0, 1.0);
  SPINNER_smoke_test_nslices = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smoke_test, _("n slices"), GLUI_SPINNER_INT, &smoke_test_nslices);
  SPINNER_smoke_test_nslices->set_int_limits(3, 100);
  SPINNER_smoke_test_range = glui_3dsmoke->add_spinner_to_panel(ROLLOUT_smoke_test, _("range"), GLUI_SPINNER_FLOAT, &smoke_test_range);
#endif

  PANEL_loadcutoff = glui_3dsmoke->add_panel_to_panel(PANEL_overall, _("Load 3D smoke/HRRPUV when..."));
  SPINNER_load_3dsmoke = glui_3dsmoke->add_spinner_to_panel(PANEL_loadcutoff, _("soot alpha >"), GLUI_SPINNER_FLOAT, &load_3dsmoke_cutoff);
  SPINNER_load_3dsmoke->set_float_limits(0.0, 255.0);

  SPINNER_load_hrrpuv = glui_3dsmoke->add_spinner_to_panel(PANEL_loadcutoff, _("HRRPUV >"), GLUI_SPINNER_FLOAT, &load_hrrpuv_cutoff);
  SPINNER_load_hrrpuv->set_float_limits(0.0, HRRPUV_CUTOFF_MAX);


  if(nsmoke3dinfo>0){
    ROLLOUT_meshvis = glui_3dsmoke->add_rollout_to_panel(PANEL_overall, "Mesh Visibility", false);
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
}

/* ------------------ Smoke3dCB ------------------------ */

extern "C" void Smoke3dCB(int var){
  int i;
  char *tour_label;
  char *vol_prefixptr;

  updatemenu=1;
  switch(var){
  float temp_min, temp_max;

  case UPDATE_FIRE_ALPHA:
    fire_halfdepth2 = meshinfo->dx*log(0.5)/log(1.0-glui_fire_alpha/255.0);
    SPINNER_smoke3d_fire_halfdepth2->set_float_val(fire_halfdepth2);
    break;
  case UPDATE_CO2_ALPHA:
    co2_halfdepth = meshinfo->dx*log(0.5)/log(1.0-glui_co2_alpha/255.0);
    SPINNER_smoke3d_co2_halfdepth->set_float_val(co2_halfdepth);
    break;
  case SOOTSMOKE:
  case CO2SMOKE:
    if(co2factor < 0.0){
      co2factor = 0.0;
      SPINNER_co2factor->set_float_val(co2factor);
    }
    if(sootfactor < 0.0){
      sootfactor = 0.0;
      SPINNER_sootfactor->set_float_val(sootfactor);
    }
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
#ifdef pp_SMOKETEST
      if(ROLLOUT_colormap_temp!=NULL){
        ROLLOUT_colormap_temp->disable();
        ROLLOUT_colormap_temp->close();
      }
#endif
      firecolormap_type=firecolormap_type_save;
      RADIO_use_colormap->set_int_val(firecolormap_type);
    }
    else{
#ifdef pp_SMOKETEST
      if(ROLLOUT_colormap_temp!=NULL){
        ROLLOUT_colormap_temp->enable();
        ROLLOUT_colormap_temp->open();
      }
#endif
      if(PANEL_absorption!=NULL)PANEL_absorption->disable();
      firecolormap_type_save=firecolormap_type;
      firecolormap_type=FIRECOLORMAP_CONSTRAINT;
      RADIO_use_colormap->set_int_val(firecolormap_type);
    }
    Smoke3dCB(FIRECOLORMAP_TYPE);
    break;
  case FIRECOLORMAP_TYPE:
    switch(firecolormap_type){
    case FIRECOLORMAP_CONSTRAINT:
      LISTBOX_smoke_colorbar->enable();
      if(fire_colorbar_index_save != -1){
        SmokeColorbarMenu(fire_colorbar_index_save);
      }
      else{
        SmokeColorbarMenu(fire_colorbar_index);
      }
      SPINNER_smoke3d_fire_red->disable();
      SPINNER_smoke3d_fire_green->disable();
      SPINNER_smoke3d_fire_blue->disable();
      CHECKBOX_edit_colormap->enable();
      LISTBOX_smoke_colorbar->enable();
      break;
    case FIRECOLORMAP_DIRECT:
      LISTBOX_smoke_colorbar->disable();
      ROLLOUT_firecolor->enable();
      ROLLOUT_smokecolor->enable();
      SPINNER_smoke3d_fire_red->enable();
      SPINNER_smoke3d_fire_green->enable();
      SPINNER_smoke3d_fire_blue->enable();
      CHECKBOX_edit_colormap->disable();
      LISTBOX_smoke_colorbar->disable();
      SPINNER_smoke3d_smoke_red->enable();
      SPINNER_smoke3d_smoke_green->enable();
      SPINNER_smoke3d_smoke_blue->enable();

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
    Smoke3dCB(FIRE_RED);
    break;
  case SMOKE_COLORBAR_LIST:
    SmokeColorbarMenu(fire_colorbar_index);
    UpdateSmokeColormap(smoke_render_option);
    updatemenu=1;
    break;
  case SMOKETEST:
    UpdateAlpha();
    break;
  case FRAMELOADING:
    smoke3dframestep = smoke3dframeskip+1;
    updatemenu=1;
    break;
  case SAVE_SETTINGS:
    WriteIni(LOCAL_INI,NULL);
    break;
  case GLOBAL_FIRE_CUTOFF:
    glutPostRedisplay();
    force_redisplay=1;
    IdleCB();
    UpdateSmokeColormap(smoke_render_option);
    break;
  case UPDATE_SMOKEFIRE_COLORS2:
    fire_halfdepth2 = MAX(fire_halfdepth2, 0.001);
    SPINNER_smoke3d_fire_halfdepth->set_float_val(fire_halfdepth2);
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);

    glui_fire_alpha = 255*(1.0-pow(0.5,meshinfo->dx/fire_halfdepth2));
    glui_fire_alpha = CLAMP(glui_fire_alpha,1,254);
    if(SPINNER_smoke3d_fire_alpha!=NULL)SPINNER_smoke3d_fire_alpha->set_int_val(glui_fire_alpha);
    break;
  case UPDATE_SMOKEFIRE_COLORS:
    co2_halfdepth = MAX(co2_halfdepth, 0.001);
    if(SPINNER_smoke3d_co2_halfdepth!=NULL)SPINNER_smoke3d_co2_halfdepth->set_float_val(co2_halfdepth);

    if(SPINNER_smoke3d_fire_halfdepth2!=NULL)SPINNER_smoke3d_fire_halfdepth2->set_float_val(fire_halfdepth);
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS_COMMON);

    glui_co2_alpha = 255*(1.0-pow(0.5,meshinfo->dx/co2_halfdepth));
    glui_co2_alpha = CLAMP(glui_co2_alpha,1,254);
    if(SPINNER_smoke3d_co2_alpha!=NULL)SPINNER_smoke3d_co2_alpha->set_int_val(glui_co2_alpha);
    break;
  case UPDATE_SMOKEFIRE_COLORS_COMMON:
  case CO2_COLOR:
  case FIRE_RED:
  case FIRE_GREEN:
  case FIRE_BLUE:
  case SMOKE_RED:
  case SMOKE_GREEN:
  case SMOKE_BLUE:
  case SMOKE_SHADE:
    glutPostRedisplay();
    force_redisplay=1;
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    UpdateSmokeColormap(smoke_render_option);
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
#ifdef pp_GPU
  case SMOKE_RTHICK:

    smoke3d_thick = LogBase2(smoke3d_rthick);
    glutPostRedisplay();
    force_redisplay=1;
    IdleCB();
    break;
#else
  case SMOKE_THICK:
    glutPostRedisplay();
    force_redisplay=1;
    IdleCB();
    break;
#endif
#ifdef pp_CULL
  case CULL_PORTSIZE:
    InitCull(cullsmoke);
    break;
  case CULL_SMOKE:
    InitCull(cullsmoke);
    break;
#endif
  case VOL_NGRID:
    glutPostRedisplay();
    break;
  case VOL_SMOKE:
    {
      volrenderdata *vr;

      vr = &meshinfo->volrenderinfo;
      if(vr!=NULL&&vr->smokeslice!=NULL&&vr->smokeslice->slicetype==SLICE_CELL_CENTER){
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
    if(have_fire==HRRPUV&&smoke_render_option==RENDER_SLICE){
#ifdef pp_GPU
      if(usegpu==1){
        RADIO_skipframes->set_int_val(0);
        RADIO_skipframes->disable();
#ifdef pp_CULL
        if(cullactive==1){
          CHECKBOX_smokecullflag->enable();
        }
        SPINNER_cull_portsize->enable();
        CHECKBOX_show_cullports->enable();
#endif
      }
      else{
        RADIO_skipframes->enable();
#ifdef pp_CULL
        CHECKBOX_smokecullflag->disable();
        SPINNER_cull_portsize->disable();
        CHECKBOX_show_cullports->disable();
#endif
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
