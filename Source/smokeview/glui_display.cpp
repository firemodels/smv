#define CPP
#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "glui_motion.h"
#include "colorbars.h"
#include "readlabel.h"

GLUI *glui_labels=NULL;

GLUI_EditText *EDIT_LB_label_string=NULL;

GLUI_Spinner *SPINNER_cullgeom_portsize=NULL;

GLUI_Listbox *LIST_LB_labels=NULL;
GLUI_Listbox *LIST_surfs=NULL;

#ifdef pp_REFRESH
GLUI_Spinner *SPINNER_refresh_rate=NULL;
#endif

GLUI_Spinner *SPINNER_sky_diam = NULL;

GLUI_Spinner *SPINNER_LB_tick_xbeg=NULL;
GLUI_Spinner *SPINNER_LB_tick_ybeg=NULL;
GLUI_Spinner *SPINNER_LB_tick_zbeg=NULL;
GLUI_Spinner *SPINNER_LB_tick_xdir=NULL;
GLUI_Spinner *SPINNER_LB_tick_ydir=NULL;
GLUI_Spinner *SPINNER_LB_tick_zdir=NULL;
GLUI_Spinner *SPINNER_ntick_decimals = NULL;


GLUI_Spinner *SPINNER_light_az0=NULL;
GLUI_Spinner *SPINNER_light_az1=NULL;
GLUI_Spinner *SPINNER_light_elev0=NULL;
GLUI_Spinner *SPINNER_light_elev1=NULL;

GLUI_Spinner *SPINNER_surf_color[4];
GLUI_Spinner *SPINNER_LB_time_start=NULL;
GLUI_Spinner *SPINNER_LB_time_stop=NULL;
GLUI_Spinner *SPINNER_LB_red=NULL;
GLUI_Spinner *SPINNER_LB_green=NULL;
GLUI_Spinner *SPINNER_LB_blue=NULL;
GLUI_Spinner *SPINNER_LB_x=NULL;
GLUI_Spinner *SPINNER_LB_y=NULL;
GLUI_Spinner *SPINNER_LB_z=NULL;
GLUI_Spinner *SPINNER_tick_xmin=NULL;
GLUI_Spinner *SPINNER_tick_ymin=NULL;
GLUI_Spinner *SPINNER_tick_zmin=NULL;
GLUI_Spinner *SPINNER_tick_xmax=NULL;
GLUI_Spinner *SPINNER_tick_ymax=NULL;
GLUI_Spinner *SPINNER_tick_zmax=NULL;
GLUI_Spinner *SPINNER_gridlinewidth = NULL;
GLUI_Spinner *SPINNER_ticklinewidth = NULL;
GLUI_Spinner *SPINNER_linewidth=NULL;
GLUI_Spinner *SPINNER_zone_hvac_diam = NULL;
GLUI_Spinner *SPINNER_tick_x0=NULL;
GLUI_Spinner *SPINNER_tick_y0=NULL;
GLUI_Spinner *SPINNER_tick_z0=NULL;
GLUI_Spinner *SPINNER_tick_dx0=NULL;
GLUI_Spinner *SPINNER_tick_dy0=NULL;
GLUI_Spinner *SPINNER_tick_dz0=NULL;
GLUI_Spinner *SPINNER_labels_transparency_face=NULL;
GLUI_Spinner *SPINNER_subtick=NULL;
GLUI_Spinner *SPINNER_scaled_font2d_height=NULL;
GLUI_Spinner *SPINNER_scaled_font3d_height=NULL;
GLUI_Spinner *SPINNER_scaled_font2d_height2width=NULL;
GLUI_Spinner *SPINNER_scaled_font3d_height2width=NULL;
GLUI_Spinner *SPINNER_scaled_font3d_thickness=NULL;
GLUI_Spinner *SPINNER_scaled_font2d_thickness=NULL;
GLUI_Spinner *SPINNER_northangle_position_x = NULL;
GLUI_Spinner *SPINNER_northangle_position_y = NULL;
GLUI_Spinner *SPINNER_northangle_position_z = NULL;
GLUI_Spinner *SPINNER_sliceoffset_factor=NULL;
GLUI_Spinner *SPINNER_ventoffset_factor=NULL;
GLUI_Spinner *SPINNER_amb_red=NULL;
GLUI_Spinner *SPINNER_amb_green=NULL;
GLUI_Spinner *SPINNER_amb_blue=NULL;
GLUI_Spinner *SPINNER_amb_grey=NULL;
GLUI_Spinner *SPINNER_diff_red=NULL;
GLUI_Spinner *SPINNER_diff_green=NULL;
GLUI_Spinner *SPINNER_diff_blue=NULL;
GLUI_Spinner *SPINNER_diff_grey=NULL;
GLUI_Spinner *SPINNER_spec_red = NULL;
GLUI_Spinner *SPINNER_spec_green = NULL;
GLUI_Spinner *SPINNER_spec_blue = NULL;
GLUI_Spinner *SPINNER_spec_grey = NULL;
GLUI_Spinner *SPINNER_shininess = NULL;
GLUI_Spinner *SPINNER_ngridloc_digits = NULL;
GLUI_Spinner *SPINNER_mesh_debug = NULL;
GLUI_Spinner *SPINNER_blockage_min_debug = NULL;
GLUI_Spinner *SPINNER_blockage_n_debug = NULL;
GLUI_Spinner *SPINNER_horizon_color[3];
GLUI_Spinner *SPINNER_zenith_color[3];
GLUI_Spinner *SPINNER_ground_color[3];
#ifdef pp_SPHERE
GLUI_Spinner *SPINNER_sphere_xyz0[3];
GLUI_Spinner *SPINNER_sphere_dxyz[3];
GLUI_Spinner *SPINNER_sphere_nxyz[3];
GLUI_Spinner *SPINNER_sphere_rgb[3];
GLUI_Spinner *SPINNER_sphere_blue=NULL;
GLUI_Spinner *SPINNER_sphere_diameter=NULL;
#endif

#ifdef pp_SPHERE
GLUI_Checkbox *CHECKBOX_sphere_show=NULL;
#endif
GLUI_Checkbox *CHECKBOX_visaxislabels = NULL;
GLUI_Checkbox *CHECKBOX_labels_showtick = NULL;
GLUI_Checkbox *CHECKBOX_labels_meshlabel = NULL;
GLUI_Checkbox *CHECKBOX_labels_version=NULL;
GLUI_Checkbox *CHECKBOX_visUSERticks=NULL;
GLUI_Checkbox *CHECKBOX_visUSERticks2=NULL;
GLUI_Checkbox *CHECKBOX_cullgeom=NULL;
GLUI_Checkbox *CHECKBOX_LB_visLabels=NULL;
GLUI_Checkbox *CHECKBOX_LB_label_use_foreground=NULL;
GLUI_Checkbox *CHECKBOX_LB_label_show_always=NULL;
GLUI_Checkbox *CHECKBOX_visColorbarVertical=NULL;
GLUI_Checkbox *CHECKBOX_visColorbarHorizontal = NULL;
GLUI_Checkbox *CHECKBOX_labels_timebar=NULL;
GLUI_Checkbox *CHECKBOX_labels_ticks=NULL;
GLUI_Checkbox *CHECKBOX_labels_title=NULL;
GLUI_Checkbox *CHECKBOX_labels_chid=NULL;
GLUI_Checkbox *CHECKBOX_labels_axis=NULL;
GLUI_Checkbox *CHECKBOX_labels_hms=NULL;
GLUI_Checkbox *CHECKBOX_labels_framerate=NULL;
GLUI_Checkbox *CHECKBOX_labels_timelabel=NULL;
GLUI_Checkbox *CHECKBOX_labels_framelabel=NULL;
GLUI_Checkbox *CHECKBOX_labels_frametimelabel = NULL;
GLUI_Checkbox *CHECKBOX_labels_hrrlabel=NULL;
#ifdef pp_memload
GLUI_Checkbox *CHECKBOX_labels_memload=NULL;
#endif
#ifdef pp_memusage
GLUI_Checkbox *CHECKBOX_labels_memusage = NULL;
#endif
GLUI_Checkbox *CHECKBOX_labels_labels = NULL;
GLUI_Checkbox *CHECKBOX_labels_gridloc=NULL;
GLUI_Checkbox *CHECKBOX_labels_average=NULL;
GLUI_Checkbox *CHECKBOX_user_ticks_show_x=NULL;
GLUI_Checkbox *CHECKBOX_user_ticks_show_y=NULL;
GLUI_Checkbox *CHECKBOX_user_ticks_show_z=NULL;
GLUI_Checkbox *CHECKBOX_tick_auto=NULL;
GLUI_Checkbox *CHECKBOX_label_2=NULL;
GLUI_Checkbox *CHECKBOX_label_3=NULL;
GLUI_Checkbox *CHECKBOX_labels_flip=NULL;
GLUI_Checkbox *CHECKBOX_labels_transparent_override=NULL;
GLUI_Checkbox *CHECKBOX_shownorth = NULL;
GLUI_Checkbox *CHECKBOX_ticks_inside = NULL;
GLUI_Checkbox *CHECKBOX_ticks_outside = NULL;
GLUI_Checkbox *CHECKBOX_labels_fds_title = NULL;
GLUI_Checkbox *CHECKBOX_texture_showall = NULL;
GLUI_Checkbox *CHECKBOX_texture_hideall = NULL;
GLUI_Checkbox *CHECKBOX_hide_scene = NULL;
GLUI_Checkbox *CHECKBOX_visSkybox = NULL;
GLUI_Checkbox *CHECKBOX_visSkysphere = NULL;
GLUI_Checkbox *CHECKBOX_visSkyground = NULL;
GLUI_Checkbox *CHECKBOX_visSkyboxoutline;

GLUI_Rollout *ROLLOUT_LB_tick0 = NULL;
GLUI_Rollout *ROLLOUT_font=NULL;
GLUI_Rollout *ROLLOUT_user_labels=NULL;
GLUI_Rollout *ROLLOUT_user_tick=NULL;
#ifdef pp_SPHERE
GLUI_Rollout *ROLLOUT_user_spheres = NULL;
#endif
GLUI_Rollout *ROLLOUT_general1 = NULL;
GLUI_Rollout *ROLLOUT_general2 = NULL;
GLUI_Rollout *ROLLOUT_north = NULL;
GLUI_Rollout *ROLLOUT_light2 = NULL;
GLUI_Rollout *ROLLOUT_sky = NULL;

#ifdef pp_SPHERE
GLUI_Panel *PANEL_sphere1 = NULL;
GLUI_Panel *PANEL_sphere2 = NULL;
GLUI_Panel *PANEL_sphere3 = NULL;
GLUI_Panel *PANEL_sphere4 = NULL;
#endif
GLUI_Panel *PANEL_blockage_drawing = NULL;
GLUI_Panel *PANEL_titles=NULL;
GLUI_Panel *PANEL_screen = NULL;
GLUI_Panel *PANEL_light=NULL;
GLUI_Panel *PANEL_position0=NULL;
GLUI_Panel *PANEL_position1 = NULL;
GLUI_Panel *PANEL_ambient=NULL;
GLUI_Panel *PANEL_diffuse=NULL;
GLUI_Panel *PANEL_specular = NULL;
GLUI_Panel *PANEL_positional = NULL;
GLUI_Panel *PANEL_positional2 = NULL;
GLUI_Panel *PANEL_timebar_overlap = NULL;
GLUI_Panel *PANEL_gen1=NULL, *PANEL_gen2=NULL;
GLUI_Panel *PANEL_LB_panel1=NULL, *PANEL_LB_panel2=NULL, *PANEL_LB_panel3=NULL;
GLUI_Panel *PANEL_LB_panel4=NULL, *PANEL_LB_panel5=NULL, *PANEL_LB_panel6=NULL;
GLUI_Panel *PANEL_LB_color=NULL, *PANEL_LB_time=NULL;
GLUI_Panel *PANEL_LB_position=NULL;
GLUI_Panel *PANEL_label2=NULL;
GLUI_Panel *PANEL_tick1;
GLUI_Panel *PANEL_tick1a;
GLUI_Panel *PANEL_tick1b;
GLUI_Panel *PANEL_tick2;
GLUI_Panel *PANEL_transparency = NULL;
GLUI_Panel *PANEL_font2d=NULL;
GLUI_Panel *PANEL_font3d=NULL;
GLUI_Panel *PANEL_LB_tick = NULL;
GLUI_Panel *PANEL_linewidth = NULL;
GLUI_Panel *PANEL_offset = NULL;
GLUI_Panel *PANEL_surfs = NULL;
GLUI_Panel *PANEL_texture_display = NULL;
GLUI_Panel *PANEL_sky = NULL;
GLUI_Panel *PANEL_sphere = NULL;
GLUI_Panel *PANEL_skycolor = NULL;
GLUI_Panel *PANEL_horizon_color = NULL;
GLUI_Panel *PANEL_zenith_color = NULL;
GLUI_Panel *PANEL_ground_color = NULL;
GLUI_Panel *PANEL_box = NULL;

GLUI_RadioGroup *RADIO_timebar_overlap = NULL;
GLUI_RadioGroup *RADIO_fontsize = NULL;
GLUI_RadioGroup *RADIOBUTTON_label_1 = NULL;
GLUI_RadioButton *RADIOBUTTON_label_1a=NULL;
GLUI_RadioButton *RADIOBUTTON_label_1b=NULL;
GLUI_RadioButton *RADIOBUTTON_label_1c=NULL;

GLUI_Button *BUTTON_LB_label_previous=NULL;
GLUI_Button *BUTTON_LB_label_next=NULL;
GLUI_Button *BUTTON_LB_label_update=NULL;
GLUI_Button *BUTTON_LB_label_add=NULL;
GLUI_Button *BUTTON_LB_label_delete=NULL;
GLUI_Button *BUTTON_LB_label_set=NULL;
GLUI_Button *BUTTON_label_1=NULL;
GLUI_Button *BUTTON_label_2=NULL;
GLUI_Button *BUTTON_label_3=NULL;
GLUI_Button *BUTTON_label_4=NULL;

#define TEXTURE_SHOWALL 0
#define TEXTURE_HIDEALL 1

#define FLIP                 19
#define APPLY_VENTOFFSET     20

#define SURFACE_COLOR        101
#define SURFACE_SELECT       102
#define SURFACE_REVERT_COLOR 103

#define LB_LIST 0
#define LB_ADD 1
#define LB_DELETE 2
#define LB_RGB 3
#define LB_XYZ 4
#define LB_STARTSTOP 5
#define LB_SHOWALWAYS 6
#define LB_FOREGROUND 7
#define LB_UPDATE 8
#define LB_PREVIOUS 9
#define LB_NEXT 10
#define LB_VISLABELS 11
#define LB_TICK_XYZ 12
#define LB_SHOW_TICK 13

#define LABELS_label           0
//#define LABELS_vcolorbar      34  movied to smokeviewdefs.h
//#define LABELS_hcolorbar      35  movied to smokeviewdefs.h
#define FRAME_label           21
#define HRR_label             22
#define FIRECUTOFF_label      23
#define LABELS_showall         1
#define LABELS_hideall         2
#define LABELS_close           3
#define LABELS_flip            4
//#define LABELS_shade           5   movied to smokeviewdefs.h
#define LABELS_transparent     6
#define LABELS_fontsize        7
#define LABELS_ticks           8
#define LABELS_drawface       24
#define LABELS_hide_overlaps  25
#define LABELS_version        26
#define LABELS_meshlabel      27
#define LABELS_usertick       28
#define LABELS_usertick2      29
//#define LABELS_shadedata      30   movied to smokeviewdefs.h
#define LABELS_shownorth      31
#define LABELS_tick_inside    32
#define LABELS_tick_outside   33
//#define LABELS_colorbar_shift 36  movied to smokeviewdefs.h
#ifdef pp_REFRESH
#define LABELS_REFRESH_RATE   37
#endif
#define LABELS_BOUNDING_BOX   38
#define LABELS_memload        39
#define LABELS_memusage       40

#define LABELS_HMS 18
#define SAVE_SETTINGS_DISPLAY 99

#define COLORBAND 115
#define CB_USE_LIGHTING 120
#define COLOR_AMB_GREY 116
#define COLOR_DIFF_GREY 117
#define COLOR_AMB_RGB 118
#define COLOR_DIFF_RGB 119
#define COLOR_SPEC_RGB 120
#define COLOR_SPEC_GREY 121
#define LIGHT_POSITION 122
#define COLORLABEL_DIGITS 123

#define DATA_transparent 26
#define TRANSPARENTLEVEL 110

#define UPDATEPLOT 10


#define LABELS_TITLES_ROLLOUT  0
#define LABELS_OFFSETS_ROLLOUT 1
#define LIGHT_ROLLOUT          2
#define FONTS_ROLLOUT          3
#define USER_TICKS_ROLLOUT     4
#define LABELS_TICKS_ROLLOUT   5
#define SKY_ROLLOUT            6
#ifdef pp_SPHERE
#define USER_SPHERES_ROLLOUT   7
#endif

#define UPDATEMENU 1

#ifdef pp_SPHERE
procdata displayprocinfo[8];
#else
procdata displayprocinfo[7];
#endif
int ndisplayprocinfo = 0;

/* ------------------ GLUIUpdateVisSkyboxOutline ------------------------ */

extern "C" void GLUIUpdateVisSkyboxOutline(void){
  if(CHECKBOX_visSkyboxoutline != NULL)CHECKBOX_visSkyboxoutline->set_int_val(visSkyboxoutline);
}

/* ------------------ GLUIUpdateVisAxisLabels ------------------------ */

extern "C" void GLUIUpdateVisAxisLabels(void){
  if(CHECKBOX_visaxislabels!=NULL)CHECKBOX_visaxislabels->set_int_val(visaxislabels);
}

/* ------------------ GLUIUpdateFrameTimelabel ------------------------ */

extern "C" void GLUIUpdateFrameTimelabel(void){
  CHECKBOX_labels_frametimelabel->set_int_val(visFrameTimelabel);
}

/* ------------------ GLUIUpdateBackgroundFlip ------------------------ */

extern "C" void GLUIUpdateBackgroundFlip(int flip){
  CHECKBOX_labels_flip->set_int_val(flip);
}

/* ------------------ GLUIUpdateTimebarOverlap ------------------------ */

extern "C" void GLUIUpdateTimebarOverlap(void){
  RADIO_timebar_overlap->set_int_val(timebar_overlap);
}


/* ------------------ GLUIUpdateTimebarOverlap ------------------------ */

extern "C" void GLUIUpdateFastBlockageDraw(void){
  if(RADIOBUTTON_label_1 != NULL)RADIOBUTTON_label_1->set_int_val(blockage_draw_option);
}

/* ------------------ DisplayRolloutCB ------------------------ */

void DisplayRolloutCB(int var){
  GLUIToggleRollout(displayprocinfo, ndisplayprocinfo, var);
}

/* ------------------ UpdateGluiLabelText ------------------------ */

void UpdateGluiLabelText(void){
  if(LabelGetNUserLabels(&global_scase.labelscoll)>0){
    labeldata *gl;

    gl=&LABEL_local;

    LIST_LB_labels->set_int_val(gl->glui_id);
    EDIT_LB_label_string->set_text(gl->name);
    SPINNER_LB_x->set_float_val(gl->xyz[0]);
    SPINNER_LB_y->set_float_val(gl->xyz[1]);
    SPINNER_LB_z->set_float_val(gl->xyz[2]);
    SPINNER_LB_tick_xbeg->set_float_val(gl->tick_begin[0]);
    SPINNER_LB_tick_ybeg->set_float_val(gl->tick_begin[1]);
    SPINNER_LB_tick_zbeg->set_float_val(gl->tick_begin[2]);
    SPINNER_LB_tick_xdir->set_float_val(gl->tick_direction[0]);
    SPINNER_LB_tick_ydir->set_float_val(gl->tick_direction[1]);
    SPINNER_LB_tick_zdir->set_float_val(gl->tick_direction[2]);
    CHECKBOX_labels_showtick->set_int_val(gl->show_tick);


    SPINNER_LB_time_start->set_float_val(gl->tstart_stop[0]);
    SPINNER_LB_time_stop->set_float_val(gl->tstart_stop[1]);
    CHECKBOX_LB_label_show_always->set_int_val(gl->show_always);

    SPINNER_LB_red->set_int_val(gl->rgb[0]);
    SPINNER_LB_green->set_int_val(gl->rgb[1]);
    SPINNER_LB_blue->set_int_val(gl->rgb[2]);
    CHECKBOX_LB_label_use_foreground->set_int_val(gl->useforegroundcolor);

    CHECKBOX_LB_visLabels->enable();
    LIST_LB_labels->enable();
    EDIT_LB_label_string->enable();
    ROLLOUT_LB_tick0->enable();
    PANEL_LB_position->enable();
    PANEL_LB_time->enable();
    PANEL_LB_color->enable();
  }
  else{
    CHECKBOX_LB_visLabels->disable();
    LIST_LB_labels->disable();
    EDIT_LB_label_string->disable();
    ROLLOUT_LB_tick0->disable();
    PANEL_LB_position->disable();
    PANEL_LB_time->disable();
    PANEL_LB_color->disable();
  }
}

  /* ------------------ GLUIUpdateFontIndex ------------------------ */

extern "C" void GLUIUpdateFontIndex(void){
  if(RADIO_fontsize!=NULL){
    if(fontindex==SCALED_FONT){
      SPINNER_scaled_font2d_height->enable();
      SPINNER_scaled_font3d_height->enable();
      SPINNER_scaled_font2d_height2width->enable();
      SPINNER_scaled_font3d_height2width->enable();
      SPINNER_scaled_font2d_thickness->enable();
      SPINNER_scaled_font3d_thickness->enable();
    }
    else{
      SPINNER_scaled_font2d_height->disable();
      SPINNER_scaled_font3d_height->disable();
      SPINNER_scaled_font2d_height2width->disable();
      SPINNER_scaled_font3d_height2width->disable();
      SPINNER_scaled_font2d_thickness->disable();
      SPINNER_scaled_font3d_thickness->disable();
    }
  }
}

/* ------------------ TextLabelsCB ------------------------ */

void TextLabelsCB(int var){
  labeldata *thislabel, *gl, *new_label;
  int count;
  char name[300];

  gl = &LABEL_local;
  switch(var){
  case LB_VISLABELS:
    updatemenu = 1;
    break;
  case LB_UPDATE:
    for(thislabel = global_scase.labelscoll.label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->glui_id < 0)continue;
      LIST_LB_labels->delete_item(thislabel->glui_id);
    }
    strcpy(LABEL_global_ptr->name, gl->name);
    //LabelResort(LABEL_global_ptr);

    count = 0;
    for(thislabel = global_scase.labelscoll.label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->labeltype == TYPE_SMV)continue;
      thislabel->glui_id = count;
      LIST_LB_labels->add_item(count++, thislabel->name);
    }
    break;
  case LB_STARTSTOP:
    memcpy(LABEL_global_ptr->tstart_stop, gl->tstart_stop, 2 * sizeof(float));
    break;
  case LB_SHOWALWAYS:
    memcpy(&LABEL_global_ptr->show_always, &gl->show_always, sizeof(int));
    break;
  case LB_FOREGROUND:
    memcpy(&LABEL_global_ptr->useforegroundcolor, &gl->useforegroundcolor, sizeof(int));
    break;
  case LB_PREVIOUS:
    new_label = LabelGet(&global_scase.labelscoll, LIST_LB_labels->curr_text);
    new_label = LabelPrevious(&global_scase.labelscoll, new_label);
    if(new_label == NULL)break;
    LABEL_global_ptr = new_label;
    if(new_label != NULL){
      LabelCopy(gl, new_label);
      UpdateGluiLabelText();
    }
    break;
  case LB_NEXT:
    new_label = LabelGet(&global_scase.labelscoll, LIST_LB_labels->curr_text);
    new_label = LabelNext(&global_scase.labelscoll, new_label);
    if(new_label == NULL)break;
    LABEL_global_ptr = new_label;
    if(new_label != NULL){
      LabelCopy(gl, new_label);
      UpdateGluiLabelText();
    }
    break;
  case LB_LIST:
    new_label = LabelGet(&global_scase.labelscoll, LIST_LB_labels->curr_text);
    LABEL_global_ptr = new_label;
    if(new_label != NULL){
      LabelCopy(gl, new_label);
    }
    UpdateGluiLabelText();
    break;
  case LB_ADD:
    updatemenu = 1;
    if(LabelGetNUserLabels(&global_scase.labelscoll) > 0){
      strcpy(name, "copy of ");
      strcat(name, gl->name);
      strcpy(gl->name, name);
    }
    else{
      gl = &LABEL_default;
    }
    gl->labeltype = TYPE_INI;
    for(thislabel = global_scase.labelscoll.label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->glui_id < 0)continue;
      LIST_LB_labels->delete_item(thislabel->glui_id);
    }
    LabelInsert(&global_scase.labelscoll, gl);
    count = 0;
    for(thislabel = global_scase.labelscoll.label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->labeltype == TYPE_SMV)continue;
      thislabel->glui_id = count;
      LIST_LB_labels->add_item(count++, thislabel->name);
    }
    TextLabelsCB(LB_LIST);
    break;
  case LB_DELETE:
    strcpy(name, LIST_LB_labels->curr_text);
    for(thislabel = global_scase.labelscoll.label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->glui_id < 0)continue;
      LIST_LB_labels->delete_item(thislabel->glui_id);
    }
    thislabel = LabelGet(&global_scase.labelscoll, name);
    if(thislabel != NULL){
      LabelDelete(thislabel);
    }
    count = 0;
    for(thislabel = global_scase.labelscoll.label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->labeltype == TYPE_SMV)continue;
      thislabel->glui_id = count;
      LIST_LB_labels->add_item(count++, thislabel->name);
    }
    TextLabelsCB(LB_LIST);
    break;
  case LB_RGB:
    gl->frgb[0] = gl->rgb[0] / 255.0;
    gl->frgb[1] = gl->rgb[1] / 255.0;
    gl->frgb[2] = gl->rgb[2] / 255.0;
    memcpy(LABEL_global_ptr->frgb, gl->frgb, 3 * sizeof(float));
    memcpy(LABEL_global_ptr->rgb, gl->rgb, 3 * sizeof(int));
    break;
  case LB_XYZ:
    memcpy(LABEL_global_ptr->xyz, gl->xyz, 3 * sizeof(float));
    break;
  case LB_TICK_XYZ:
    memcpy(LABEL_global_ptr->tick_begin, gl->tick_begin, 3 * sizeof(float));
    memcpy(LABEL_global_ptr->tick_direction, gl->tick_direction, 3 * sizeof(float));
    break;
  case LB_SHOW_TICK:
    memcpy(&LABEL_global_ptr->show_tick, &gl->show_tick, sizeof(int));
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ ColorCB ------------------------ */

void ColorCB(int var){
  int i;

  switch(var){
  case LIGHT_POSITION:
    light_position0[0] = (xyzmaxdiff/2.0)*cos(DEG2RAD*glui_light_az0)*cos(DEG2RAD*glui_light_elev0);
    light_position0[1] = (xyzmaxdiff/2.0)*sin(DEG2RAD*glui_light_az0)*cos(DEG2RAD*glui_light_elev0);
    light_position0[2] = (xyzmaxdiff/2.0)*sin(DEG2RAD*glui_light_elev0);

    light_position1[0] = (xyzmaxdiff/2.0)*cos(DEG2RAD*glui_light_az1)*cos(DEG2RAD*glui_light_elev1);
    light_position1[1] = (xyzmaxdiff/2.0)*sin(DEG2RAD*glui_light_az1)*cos(DEG2RAD*glui_light_elev1);
    light_position1[2] = (xyzmaxdiff/2.0)*sin(DEG2RAD*glui_light_elev1);
    break;
  case COLOR_SPEC_RGB:
    for(i = 0; i<3; i++){
      specularlight[i] = (float)glui_specularlight[i]/255.0;
    }
    glui_speculargrey = CLAMP(255*TOBW(specularlight), 0, 255);
    SPINNER_spec_grey->set_int_val(glui_speculargrey);
    break;
  case COLOR_AMB_RGB:
    for(i = 0; i<3; i++){
      ambientlight[i] = (float)glui_ambientlight[i]/255.0;
    }
    glui_ambientgrey = CLAMP(255*TOBW(ambientlight),0,255);
    SPINNER_amb_grey->set_int_val(glui_ambientgrey);
    break;
  case COLOR_DIFF_RGB:
    for(i = 0; i<3; i++){
      diffuselight[i] = (float)glui_diffuselight[i]/255.0;
    }
    glui_diffusegrey = CLAMP(255*TOBW(diffuselight),0,255);
    SPINNER_diff_grey->set_int_val(glui_diffusegrey);
    break;
  case COLOR_SPEC_GREY:
    for(i = 0; i<3; i++){
      glui_specularlight[i] = glui_speculargrey;
      specularlight[i] = CLAMP((float)glui_speculargrey/255.0, 0.0, 1.0);
    }
    for(i = 0; i<3; i++){
      ambientlight[i] = CLAMP((float)glui_ambientlight[i]/255.0, 0.0, 1.0);
    }
    SPINNER_spec_red->set_int_val(glui_speculargrey);
    SPINNER_spec_green->set_int_val(glui_speculargrey);
    SPINNER_spec_blue->set_int_val(glui_speculargrey);
    break;
  case COLOR_AMB_GREY:
    for(i = 0; i<3; i++){
      glui_ambientlight[i] = glui_ambientgrey;
      ambientlight[i] = CLAMP((float)glui_ambientgrey/255.0,0.0,1.0);
    }
    for(i = 0; i<3; i++){
      ambientlight[i] = CLAMP((float)glui_ambientlight[i]/255.0,0.0,1.0);
    }
    SPINNER_amb_red->set_int_val(glui_ambientgrey);
    SPINNER_amb_green->set_int_val(glui_ambientgrey);
    SPINNER_amb_blue->set_int_val(glui_ambientgrey);
  break;
  case COLOR_DIFF_GREY:
    if(glui_shininess<1.0){
      glui_shininess = 1.0;
      SPINNER_shininess->set_float_val(glui_shininess);
    }
    iso_shininess = glui_shininess;
    for(i = 0; i<3; i++){
      glui_diffuselight[i] = glui_diffusegrey;
      diffuselight[i] = (float)glui_diffusegrey/255.0;
    }
    SPINNER_diff_red->set_int_val(glui_diffusegrey);
    SPINNER_diff_green->set_int_val(glui_diffusegrey);
    SPINNER_diff_blue->set_int_val(glui_diffusegrey);
  break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIUpdateGridLocation ------------------------ */

extern "C" void GLUIUpdateGridLocation(void){
  SPINNER_ngridloc_digits->set_int_val(ngridloc_digits);
}

/* ------------------ UpdateMenuCB ------------------------ */

void UpdateMenuCB(int var){
  updatemenu = 1;
}

/* ------------------ SurfaceCB ------------------------ */

void SurfaceCB(int var){
  updatemenu = 1;
  switch(var){
  case SURFACE_REVERT_COLOR:
    {
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo + glui_surf_index;
      surfi->color = surfi->color_orig;
      surfi->transparent_level = surfi->transparent_level_orig;
      SurfaceCB(SURFACE_SELECT);
      SurfaceCB(SURFACE_COLOR);
    }
    break;
  case SURFACE_COLOR:
    {
      surfdata *surfi;
      float s_color[4];

      surfi = global_scase.surfcoll.surfinfo + glui_surf_index;
      s_color[0] = (float)glui_surface_color[0]/255.0;
      s_color[1] = (float)glui_surface_color[1]/255.0;
      s_color[2] = (float)glui_surface_color[2]/255.0;
      s_color[3] = (float)glui_surface_color[3]/255.0;
      if(glui_surface_color[3]<255){
        surfi->transparent=1;
        surfi->transparent_level = (float)glui_surface_color[3]/255.0;

      }
      else{
        surfi->transparent = 0;
        surfi->transparent_level = 1.0;
      }
      s_color[3] = surfi->transparent_level;
      surfi->color = GetColorPtr(&global_scase, s_color);
      updatefacelists = 1;
      global_scase.updatefaces = 1;
    }
    break;
  case SURFACE_SELECT:
    {
      surfdata *surfi;
      float s_color[4];
      int i;

      surfi = global_scase.surfcoll.surfinfo + glui_surf_index;
      memcpy(s_color, surfi->color, 3*sizeof(float));
      s_color[3] = surfi->transparent_level;

      for(i=0;i<4;i++){
        glui_surface_color[i] = CLAMP((int)(255.0*s_color[i]+0.5),0,255);
        SPINNER_surf_color[i]->set_int_val(glui_surface_color[i]);
      }
    }
    break;
  default:
    assert(FFALSE);
  }
}

/* ------------------ GLUIUpdateTextureDisplay ------------------------ */

extern "C" void GLUIUpdateTextureDisplay(void){
  texturedata *texti;
  int i;
  int showall = 1, hideall = 1, update=0;

  for(i = 0;i < global_scase.texture_coll.ntextureinfo;i++){
    texti = global_scase.texture_coll.textureinfo + i;
    if(texti->loaded == 0 || texti->used == 0)continue;
    if(texti->display == 0)showall=0;
    if(texti->display == 1)hideall = 0;
    update = 1;
  }
  if(update==1){
    if(CHECKBOX_texture_hideall!=NULL)CHECKBOX_texture_hideall->set_int_val(hideall);
    if(CHECKBOX_texture_showall!=NULL)CHECKBOX_texture_showall->set_int_val(showall);
  }

}

/* ------------------ GLUITextureCB ------------------------ */

extern "C" void GLUITextureCB(int var){
  switch(var){
    case TEXTURE_SHOWALL:
      if(texture_showall==1){
        TextureShowMenu(MENU_TEXTURE_SHOWALL);
        if(texture_hideall==1){
          texture_hideall = 0;
          if(CHECKBOX_texture_hideall!=NULL)CHECKBOX_texture_hideall->set_int_val(0);
        }
      }
      break;
    case TEXTURE_HIDEALL:
      if(texture_hideall==1){
        TextureShowMenu(MENU_TEXTURE_HIDEALL);
        if(texture_showall==1){
          texture_showall = 0;
          if(CHECKBOX_texture_showall!=NULL)CHECKBOX_texture_showall->set_int_val(0);
        }
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUISkyCB ------------------------ */

extern "C" void GLUISkyCB(int var){
  float farclip_before;

  farclip_before = farclip;
  if(visSkybox == 1 || visSkysphere == 1){
    farclip = MAX(farclip_save, sky_diam);
  }
  else{
    farclip = farclip_save;
  }
  if(farclip != farclip_before){
    GLUISceneMotionCB(NEARFARCLIP);
    GLUIUpdateFarclip();
  }

  switch (var){
    case SKY_BOX:
      if(sky_diam<1.0){
        sky_diam = 1.0;
        if(SPINNER_sky_diam!=NULL)SPINNER_sky_diam->set_float_val(sky_diam);
      }
      if(visSkybox==1&&visSkysphere==1){
        visSkysphere = 0;
        CHECKBOX_visSkysphere->set_int_val(0);
      }
      GetBoxSkyCorners();
      break;
    case SKY_SPHERE:
      if(visSkybox==1&&visSkysphere==1){
        visSkybox = 0;
        if(CHECKBOX_visSkybox!=NULL)CHECKBOX_visSkybox->set_int_val(0);
      }
      GetBoxSkyCorners();
      break;
    case RESET_COLORS:
      int i;
      for(i=0;i<3;i++){
        SPINNER_horizon_color[i]->set_int_val(horizon_color_save[i]);
        SPINNER_zenith_color[i]->set_int_val(zenith_color_save[i]);
        SPINNER_ground_color[i]->set_int_val(ground_color_save[i]);
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ GLUIDisplaySetup ------------------------ */

extern "C" void GLUIDisplaySetup(int main_window){
  labeldata *gl;

  if(glui_labels != NULL){
    glui_labels->close();
    glui_labels = NULL;
  }
  glui_labels = GLUI_Master.create_glui("Display", 0, dialogX0, dialogY0);
  glui_labels->hide();

  // -------------- Labels/Titles/Bounding box -------------------

  ROLLOUT_general1 = glui_labels->add_rollout(_("Labels/Titles"), true, LABELS_TITLES_ROLLOUT, DisplayRolloutCB);
  TOGGLE_ROLLOUT(displayprocinfo, ndisplayprocinfo, ROLLOUT_general1, LABELS_TITLES_ROLLOUT, glui_labels);

  PANEL_gen1 = glui_labels->add_panel_to_panel(ROLLOUT_general1, "", GLUI_PANEL_NONE);

  if(global_scase.slicecoll.nsliceinfo > 0)CHECKBOX_labels_average = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Average"), &vis_slice_average, LABELS_label, GLUILabelsCB);
  CHECKBOX_labels_axis = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Axis"), &visaxislabels, LABELS_label, GLUILabelsCB);
  CHECKBOX_visColorbarVertical = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Colorbar(vertical)"), &visColorbarVertical, LABELS_vcolorbar, GLUILabelsCB);
  CHECKBOX_visColorbarHorizontal = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Colorbar(horizontal)"), &visColorbarHorizontal, LABELS_hcolorbar, GLUILabelsCB);
  CHECKBOX_labels_timebar = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Timebar"), &visTimebar, LABELS_label, GLUILabelsCB);
  CHECKBOX_labels_framelabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Frame"), &visFramelabel, LABELS_label, GLUILabelsCB);
  CHECKBOX_labels_timelabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Time"), &visTimelabel, LABELS_label, GLUILabelsCB);
  CHECKBOX_labels_frametimelabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Frame/time label"), &visFrameTimelabel, LABELS_label, GLUILabelsCB);
  CHECKBOX_labels_framerate = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Frame rate"), &visFramerate, LABELS_label, GLUILabelsCB);
  CHECKBOX_labels_gridloc = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Grid location"), &visgridloc, LABELS_label, GLUILabelsCB);
  CHECKBOX_labels_hrrlabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("HRR"), &vis_hrr_label, HRR_label, GLUILabelsCB);
#ifdef pp_memload
  CHECKBOX_labels_memload = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Memory load"), &vismemload, LABELS_memload, GLUILabelsCB);
#endif
#ifdef pp_memusage
  CHECKBOX_labels_memusage = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Memory usage"), &vismemusage, LABELS_memusage, GLUILabelsCB);
#endif

  glui_labels->add_column_to_panel(PANEL_gen1, false);

  CHECKBOX_labels_meshlabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Mesh"), &visMeshlabel, LABELS_meshlabel, GLUILabelsCB);
  CHECKBOX_labels_labels = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Text labels"), &visLabels, LABELS_label, GLUILabelsCB);
  CHECKBOX_labels_ticks = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Ticks (FDS)"), &visFDSticks, LABELS_label, GLUILabelsCB);
  CHECKBOX_visUSERticks2 = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Ticks (User)"), &visUSERticks, LABELS_usertick2, GLUILabelsCB);
  glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Toggle dialogs"), &toggle_dialogs);
#ifdef pp_REFRESH
  SPINNER_refresh_rate = glui_labels->add_spinner_to_panel(PANEL_gen1, _("refresh rate (fps)"), GLUI_SPINNER_INT, &glui_refresh_rate, LABELS_REFRESH_RATE, GLUILabelsCB);
  SPINNER_refresh_rate->set_int_limits(0,10);
#endif

  CHECKBOX_hide_scene = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Hide scene when mouse is pressed"), &hide_scene, LABELS_BOUNDING_BOX, GLUILabelsCB);

  PANEL_titles=glui_labels->add_panel_to_panel(PANEL_gen1,"Titles");
  CHECKBOX_labels_title = glui_labels->add_checkbox_to_panel(PANEL_titles,     _("Smokeview build date"), &vis_title_smv_version, LABELS_label,   GLUILabelsCB);
  CHECKBOX_labels_version = glui_labels->add_checkbox_to_panel(PANEL_titles,   _("FDS, Smokeview version"),        &vis_title_gversion,    LABELS_version, GLUILabelsCB);
  CHECKBOX_labels_fds_title = glui_labels->add_checkbox_to_panel(PANEL_titles, _("Input file title"),              &vis_title_fds,         LABELS_label,   GLUILabelsCB);
  CHECKBOX_labels_chid = glui_labels->add_checkbox_to_panel(PANEL_titles,      _("CHID"),                          &vis_title_CHID,        LABELS_label,   GLUILabelsCB);

  if(global_scase.ntickinfo > 0){
    CHECKBOX_labels_ticks->enable();
  }
  else{
    CHECKBOX_labels_ticks->disable();
    visFDSticks=0;
    CHECKBOX_labels_ticks->set_int_val(visFDSticks);
  }

  PANEL_gen2=glui_labels->add_panel_to_panel(ROLLOUT_general1,"",GLUI_PANEL_NONE);

  BUTTON_label_1=glui_labels->add_button_to_panel(PANEL_gen2,_("Show all"),LABELS_showall,GLUILabelsCB);
  glui_labels->add_column_to_panel(PANEL_gen2,false);
  BUTTON_label_2=glui_labels->add_button_to_panel(PANEL_gen2,_("Hide all"),LABELS_hideall,GLUILabelsCB);

  // -------------- Lines/Offsets/Surfaces/Other -------------------

  ROLLOUT_general2 = glui_labels->add_rollout(_("Lines/Offsets/Surfaces/Other"), false, LABELS_OFFSETS_ROLLOUT, DisplayRolloutCB);
  TOGGLE_ROLLOUT(displayprocinfo, ndisplayprocinfo, ROLLOUT_general2, LABELS_OFFSETS_ROLLOUT, glui_labels);

  PANEL_linewidth=glui_labels->add_panel_to_panel(ROLLOUT_general2,"line width");
  SPINNER_linewidth=glui_labels->add_spinner_to_panel(PANEL_linewidth,_("blockage"),GLUI_SPINNER_FLOAT,&global_scase.linewidth);
  SPINNER_linewidth->set_float_limits(1.0,10.0,GLUI_LIMIT_CLAMP);
  SPINNER_gridlinewidth=glui_labels->add_spinner_to_panel(PANEL_linewidth,_("grid"),GLUI_SPINNER_FLOAT,&gridlinewidth);
  SPINNER_gridlinewidth->set_float_limits(1.0,10.0,GLUI_LIMIT_CLAMP);
  SPINNER_ticklinewidth = glui_labels->add_spinner_to_panel(PANEL_linewidth, _("tick"), GLUI_SPINNER_FLOAT, &ticklinewidth);
  SPINNER_ticklinewidth->set_float_limits(1.0, 10.0, GLUI_LIMIT_CLAMP);

  PANEL_offset=glui_labels->add_panel_to_panel(ROLLOUT_general2,"offset");
  SPINNER_ventoffset_factor=glui_labels->add_spinner_to_panel(PANEL_offset,_("vent"),GLUI_SPINNER_FLOAT,&ventoffset_factor,APPLY_VENTOFFSET,GLUILabelsCB);
//  SPINNER_ventoffset_factor->set_float_limits(-1.0,1.0,GLUI_LIMIT_CLAMP);
  SPINNER_sliceoffset_factor=glui_labels->add_spinner_to_panel(PANEL_offset,_("slice"),GLUI_SPINNER_FLOAT,&sliceoffset_factor);
//  SPINNER_sliceoffset_factor->set_float_limits(-1.0,1.0,GLUI_LIMIT_CLAMP);
  glui_labels->add_spinner_to_panel(PANEL_offset, _("boundary"), GLUI_SPINNER_FLOAT, &boundaryoffset);

  int i, surfcount = 0, first_surf=-1;

  for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
    surfdata *surfi;

    surfi = global_scase.surfcoll.surfinfo+i;
    if(surfi->used_by_geom==0&&surfi->used_by_obst==0)continue;
    if(strcmp(surfi->surfacelabel, "INERT")==0)continue;
    if(first_surf<0)first_surf = i;
    surfi->in_color_dialog = 1;
    surfcount++;
  }
  if(surfcount>0){
    glui_surf_index = first_surf;
    PANEL_surfs = glui_labels->add_panel_to_panel(ROLLOUT_general2, "Surface color");
    LIST_surfs = glui_labels->add_listbox_to_panel(PANEL_surfs, _("Select"), &glui_surf_index, SURFACE_SELECT, SurfaceCB);
    for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo+i;
      if(surfi->used_by_geom==0&&surfi->used_by_obst==0)continue;
      if(strcmp(surfi->surfacelabel, "INERT")==0)continue;
      LIST_surfs->add_item(i, surfi->surfacelabel);
    }
    SPINNER_surf_color[0] = glui_labels->add_spinner_to_panel(PANEL_surfs, _("red"),   GLUI_SPINNER_INT, glui_surface_color,   SURFACE_COLOR, SurfaceCB);
    SPINNER_surf_color[1] = glui_labels->add_spinner_to_panel(PANEL_surfs, _("green"), GLUI_SPINNER_INT, glui_surface_color+1, SURFACE_COLOR, SurfaceCB);
    SPINNER_surf_color[2] = glui_labels->add_spinner_to_panel(PANEL_surfs, _("blue"),  GLUI_SPINNER_INT, glui_surface_color+2, SURFACE_COLOR, SurfaceCB);
    SPINNER_surf_color[3] = glui_labels->add_spinner_to_panel(PANEL_surfs, _("alpha"), GLUI_SPINNER_INT, glui_surface_color+3, SURFACE_COLOR, SurfaceCB);
    SPINNER_surf_color[0]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_surf_color[1]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_surf_color[2]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    SPINNER_surf_color[3]->set_int_limits(0, 255, GLUI_LIMIT_CLAMP);
    glui_labels->add_button_to_panel(PANEL_surfs,"Revert (input file)",SURFACE_REVERT_COLOR,SurfaceCB);
    SurfaceCB(SURFACE_SELECT);
  }

  SPINNER_ngridloc_digits = glui_labels->add_spinner_to_panel(ROLLOUT_general2, _("grid location digits:"),
    GLUI_SPINNER_INT, &ngridloc_digits, UPDATEMENU, UpdateMenuCB);
  SPINNER_ngridloc_digits->set_int_limits(GRIDLOC_NDECIMALS_MIN, GRIDLOC_NDECIMALS_MAX, GLUI_LIMIT_CLAMP);

  CHECKBOX_visaxislabels = glui_labels->add_checkbox_to_panel(ROLLOUT_general2, _("Show axis labels"), &visaxislabels, UPDATEMENU, UpdateMenuCB);

  if(global_scase.nzoneinfo > 0){
    SPINNER_zone_hvac_diam = glui_labels->add_spinner_to_panel(ROLLOUT_general2, "HVAC (cfast)", GLUI_SPINNER_FLOAT, &zone_hvac_diam);
    SPINNER_zone_hvac_diam->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);
  }

  if(global_scase.have_northangle==1){
    ROLLOUT_north = glui_labels->add_rollout_to_panel(ROLLOUT_general2,_("North direction"),false);
    CHECKBOX_shownorth=glui_labels->add_checkbox_to_panel(ROLLOUT_north,_("show"),&vis_northangle,LABELS_shownorth,GLUILabelsCB);
    SPINNER_northangle_position_x = glui_labels->add_spinner_to_panel(ROLLOUT_north, "x:", GLUI_SPINNER_FLOAT, northangle_position);
    SPINNER_northangle_position_y = glui_labels->add_spinner_to_panel(ROLLOUT_north, "y:", GLUI_SPINNER_FLOAT, northangle_position+1);
    SPINNER_northangle_position_z = glui_labels->add_spinner_to_panel(ROLLOUT_north, "z:", GLUI_SPINNER_FLOAT, northangle_position+2);
  }

  glui_labels->add_column_to_panel(ROLLOUT_general2,false);

  CHECKBOX_labels_flip = glui_labels->add_checkbox_to_panel(ROLLOUT_general2, _("Flip background"), &background_flip, LABELS_flip, GLUILabelsCB);
  CHECKBOX_labels_hms = glui_labels->add_checkbox_to_panel(ROLLOUT_general2, _("hms time"), &vishmsTimelabel, LABELS_HMS, GLUILabelsCB);
  PANEL_timebar_overlap = glui_labels->add_panel_to_panel(ROLLOUT_general2,_("Overlap timebar region"));
  RADIO_timebar_overlap=glui_labels->add_radiogroup_to_panel(PANEL_timebar_overlap,&timebar_overlap);
  glui_labels->add_radiobutton_to_group(RADIO_timebar_overlap,_("Always"));
  glui_labels->add_radiobutton_to_group(RADIO_timebar_overlap,_("Never"));
  glui_labels->add_radiobutton_to_group(RADIO_timebar_overlap,_("Only if timebar hidden"));

  PANEL_blockage_drawing = glui_labels->add_panel_to_panel(ROLLOUT_general2,_("Surface/blockage drawing"));
  RADIOBUTTON_label_1 = glui_labels->add_radiogroup_to_panel(PANEL_blockage_drawing, &blockage_draw_option, LABELS_drawface, GLUILabelsCB);
  glui_labels->add_radiobutton_to_group(RADIOBUTTON_label_1, _("original"));
  glui_labels->add_radiobutton_to_group(RADIOBUTTON_label_1, _("default"));
  glui_labels->add_radiobutton_to_group(RADIOBUTTON_label_1, _("debug"));
  glui_labels->add_radiobutton_to_group(RADIOBUTTON_label_1, _("debug - draw only hidden faces"));
  SPINNER_mesh_debug = glui_labels->add_spinner_to_panel(PANEL_blockage_drawing, "mesh:", GLUI_SPINNER_INT, &mesh_index_debug);
  SPINNER_blockage_min_debug = glui_labels->add_spinner_to_panel(PANEL_blockage_drawing, "min blockage index:", GLUI_SPINNER_INT, &min_blockage_index_debug);
  SPINNER_blockage_n_debug = glui_labels->add_spinner_to_panel(PANEL_blockage_drawing, "number of blockages:", GLUI_SPINNER_INT, &n_blockages_debug);
  GLUILabelsCB(LABELS_drawface);

  CHECKBOX_label_2=glui_labels->add_checkbox_to_panel(ROLLOUT_general2,_("Sort transparent faces"),&sort_transparent_faces,LABELS_drawface,GLUILabelsCB);
  CHECKBOX_label_3=glui_labels->add_checkbox_to_panel(ROLLOUT_general2,_("Hide overlaps"),&hide_overlaps,LABELS_hide_overlaps,GLUILabelsCB);

  if(nface_transparent>0){
    glui_labels->add_column_to_panel(PANEL_gen1,true);
    PANEL_transparency = glui_labels->add_panel_to_panel(ROLLOUT_general2,_("Geometry transparency"));
    CHECKBOX_labels_transparent_override=glui_labels->add_checkbox_to_panel(PANEL_transparency,_("Use level:"),&use_transparency_geom,LABELS_transparent,GLUILabelsCB);
    SPINNER_labels_transparency_face=glui_labels->add_spinner_to_panel(PANEL_transparency,"",GLUI_SPINNER_FLOAT,&transparency_geom,LABELS_transparent,GLUILabelsCB);
    SPINNER_labels_transparency_face->set_float_limits(0.0,1.0,GLUI_LIMIT_CLAMP);
    GLUILabelsCB(LABELS_transparent);
  }

  PANEL_texture_display = glui_labels->add_panel_to_panel(ROLLOUT_general2, _("Textures"));
  CHECKBOX_texture_showall = glui_labels->add_checkbox_to_panel(PANEL_texture_display, _("show all"), &texture_showall, TEXTURE_SHOWALL, GLUITextureCB);
  CHECKBOX_texture_hideall = glui_labels->add_checkbox_to_panel(PANEL_texture_display, _("hide all"), &texture_hideall, TEXTURE_HIDEALL, GLUITextureCB);

  // -------------- Light -------------------

  ROLLOUT_light2 = glui_labels->add_rollout("Light",false,LIGHT_ROLLOUT,DisplayRolloutCB);
  TOGGLE_ROLLOUT(displayprocinfo, ndisplayprocinfo, ROLLOUT_light2, LIGHT_ROLLOUT, glui_labels);

  for(i = 0; i<3;i++){
    glui_ambientlight[i] = CLAMP(255*ambientlight[i],0,255);
    glui_diffuselight[i] = CLAMP(255*diffuselight[i],0,255);
  }

  glui_ambientgrey = 255*ambientgrey;
  glui_diffusegrey = 255*diffusegrey;
  glui_speculargrey = 255*speculargrey;

  PANEL_light = glui_labels->add_panel_to_panel(ROLLOUT_light2, "",false);
  PANEL_ambient = glui_labels->add_panel_to_panel(PANEL_light, "background/ambient");
  SPINNER_amb_red = glui_labels->add_spinner_to_panel(PANEL_ambient, _("red:"),     GLUI_SPINNER_INT, glui_ambientlight,   COLOR_AMB_RGB,ColorCB);
  SPINNER_amb_green = glui_labels->add_spinner_to_panel(PANEL_ambient, _("green:"), GLUI_SPINNER_INT, glui_ambientlight+1, COLOR_AMB_RGB,ColorCB);
  SPINNER_amb_blue = glui_labels->add_spinner_to_panel(PANEL_ambient, _("blue:"),   GLUI_SPINNER_INT, glui_ambientlight+2, COLOR_AMB_RGB,ColorCB);
  SPINNER_amb_grey = glui_labels->add_spinner_to_panel(PANEL_ambient, _("grey:"),   GLUI_SPINNER_INT, &glui_ambientgrey,   COLOR_AMB_GREY,ColorCB);
  SPINNER_amb_red->set_int_limits(0,255);
  SPINNER_amb_green->set_int_limits(0, 255);
  SPINNER_amb_blue->set_int_limits(0, 255);
  SPINNER_amb_grey->set_int_limits(0, 255);
  ColorCB(COLOR_AMB_RGB);

  glui_labels->add_column_to_panel(PANEL_light,false);

  PANEL_diffuse = glui_labels->add_panel_to_panel(PANEL_light, "light/diffuse");
  SPINNER_diff_red = glui_labels->add_spinner_to_panel(PANEL_diffuse, _("red:"),     GLUI_SPINNER_INT, glui_diffuselight,   COLOR_DIFF_RGB,ColorCB);
  SPINNER_diff_green = glui_labels->add_spinner_to_panel(PANEL_diffuse, _("green:"), GLUI_SPINNER_INT, glui_diffuselight+1, COLOR_DIFF_RGB,ColorCB);
  SPINNER_diff_blue = glui_labels->add_spinner_to_panel(PANEL_diffuse, _("blue:"),   GLUI_SPINNER_INT, glui_diffuselight+2, COLOR_DIFF_RGB,ColorCB);
  SPINNER_diff_grey = glui_labels->add_spinner_to_panel(PANEL_diffuse, _("grey:"),   GLUI_SPINNER_INT, &glui_diffusegrey,   COLOR_DIFF_GREY,ColorCB);
  SPINNER_diff_red->set_int_limits(0,255);
  SPINNER_diff_green->set_int_limits(0, 255);
  SPINNER_diff_blue->set_int_limits(0, 255);
  SPINNER_diff_grey->set_int_limits(0, 255);
  ColorCB(COLOR_DIFF_RGB);
  SPINNER_shininess = glui_labels->add_spinner_to_panel(PANEL_diffuse, "shininess", GLUI_SPINNER_FLOAT, &glui_shininess,
                                                        COLOR_DIFF_GREY, ColorCB);

  PANEL_positional = glui_labels->add_panel_to_panel(ROLLOUT_light2, "direction");
  glui_labels->add_checkbox_to_panel(PANEL_positional, "show directions", &drawlights);


  PANEL_positional2 = glui_labels->add_panel_to_panel(PANEL_positional, "",false);
  PANEL_position0 = glui_labels->add_panel_to_panel(PANEL_positional2, "light 1");
  glui_labels->add_checkbox_to_panel(PANEL_position0, _("show"), &use_light0);
  SPINNER_light_az0 = glui_labels->add_spinner_to_panel(PANEL_position0,   "azimuth:",   GLUI_SPINNER_FLOAT, &glui_light_az0,   LIGHT_POSITION,ColorCB);
  SPINNER_light_elev0 = glui_labels->add_spinner_to_panel(PANEL_position0, "elevation:", GLUI_SPINNER_FLOAT, &glui_light_elev0, LIGHT_POSITION,ColorCB);
  SPINNER_light_az0->set_float_limits(-360.0,360.0);
  SPINNER_light_elev0->set_float_limits(-90.0,90.0);

  glui_labels->add_column_to_panel(PANEL_positional2,false);

  PANEL_position1 = glui_labels->add_panel_to_panel(PANEL_positional2, "light 2");
  glui_labels->add_checkbox_to_panel(PANEL_position1, "show", &use_light1);
  SPINNER_light_az1 = glui_labels->add_spinner_to_panel(PANEL_position1,   "azimuth:",   GLUI_SPINNER_FLOAT, &glui_light_az1,   LIGHT_POSITION,ColorCB);
  SPINNER_light_elev1 = glui_labels->add_spinner_to_panel(PANEL_position1, "elevation:", GLUI_SPINNER_FLOAT, &glui_light_elev1, LIGHT_POSITION,ColorCB);
  SPINNER_light_az1->set_float_limits(-360.0,360.0);
  SPINNER_light_elev1->set_float_limits(-90.0,90.0);
  ColorCB(LIGHT_POSITION);

  // -------------- Fonts -------------------

  ROLLOUT_font = glui_labels->add_rollout("Fonts",false,FONTS_ROLLOUT,DisplayRolloutCB);
  TOGGLE_ROLLOUT(displayprocinfo, ndisplayprocinfo, ROLLOUT_font, FONTS_ROLLOUT, glui_labels);

  RADIO_fontsize = glui_labels->add_radiogroup_to_panel(ROLLOUT_font,&fontindex,LABELS_fontsize,GLUILabelsCB);
  RADIOBUTTON_label_1a=glui_labels->add_radiobutton_to_group(RADIO_fontsize,_("small"));
  RADIOBUTTON_label_1b=glui_labels->add_radiobutton_to_group(RADIO_fontsize,_("large"));
  RADIOBUTTON_label_1c=glui_labels->add_radiobutton_to_group(RADIO_fontsize,_("scaled"));

  PANEL_font2d = glui_labels->add_panel_to_panel(ROLLOUT_font,_("labels"),true);
  SPINNER_scaled_font2d_height=glui_labels->add_spinner_to_panel(PANEL_font2d,_("height:"),GLUI_SPINNER_INT,&scaled_font2d_height);
  SPINNER_scaled_font2d_height2width=glui_labels->add_spinner_to_panel(PANEL_font2d,_("height/width"),GLUI_SPINNER_FLOAT,&scaled_font2d_height2width);
  SPINNER_scaled_font2d_height2width->set_float_limits(0.5,1.5);
  SPINNER_scaled_font2d_thickness=glui_labels->add_spinner_to_panel(PANEL_font2d,_("thickness:"),GLUI_SPINNER_INT,&scaled_font2d_thickness);
  SPINNER_scaled_font2d_thickness->set_int_limits(1,10);

  PANEL_font3d = glui_labels->add_panel_to_panel(ROLLOUT_font,_("scene"),true);
  SPINNER_scaled_font3d_height=glui_labels->add_spinner_to_panel(PANEL_font3d,_("height:"),GLUI_SPINNER_INT,&scaled_font3d_height);
  SPINNER_scaled_font3d_height2width=glui_labels->add_spinner_to_panel(PANEL_font3d,_("height/width:"),GLUI_SPINNER_FLOAT,&scaled_font3d_height2width);
  SPINNER_scaled_font3d_height2width->set_float_limits(0.5,1.5);
  SPINNER_scaled_font3d_thickness=glui_labels->add_spinner_to_panel(PANEL_font3d,_("thickness:"),GLUI_SPINNER_INT,&scaled_font3d_thickness);
  SPINNER_scaled_font3d_thickness->set_int_limits(1,10);
  GLUIUpdateFontIndex();

  // -------------- User tick settings -------------------

  ROLLOUT_user_tick = glui_labels->add_rollout("User ticks",false,USER_TICKS_ROLLOUT,DisplayRolloutCB);
  TOGGLE_ROLLOUT(displayprocinfo, ndisplayprocinfo, ROLLOUT_user_tick, USER_TICKS_ROLLOUT, glui_labels);

  PANEL_tick1 = glui_labels->add_panel_to_panel(ROLLOUT_user_tick,_("Display"),true);
  PANEL_tick1a = glui_labels->add_panel_to_panel(PANEL_tick1,"",false);

  CHECKBOX_visUSERticks=glui_labels->add_checkbox_to_panel(PANEL_tick1a,_("Show user ticks"),&visUSERticks,LABELS_usertick,GLUILabelsCB);
  glui_labels->add_column_to_panel(PANEL_tick1a,false);

  if(user_tick_direction>0.0){
    glui_tick_inside=0;
    glui_tick_outside=1;
  }
  else{
    glui_tick_inside=1;
    glui_tick_outside=0;
  }
  CHECKBOX_ticks_inside=glui_labels->add_checkbox_to_panel(PANEL_tick1a, "inside", &glui_tick_inside, LABELS_tick_inside, GLUILabelsCB);
  glui_labels->add_column_to_panel(PANEL_tick1a, false);
  CHECKBOX_ticks_outside=glui_labels->add_checkbox_to_panel(PANEL_tick1a, "outside", &glui_tick_outside, LABELS_tick_outside, GLUILabelsCB);
  glui_labels->add_column_to_panel(PANEL_tick1a, false);
  SPINNER_ntick_decimals = glui_labels->add_spinner_to_panel(PANEL_tick1a, _("decimals"), GLUI_SPINNER_INT, &ntick_decimals);
  SPINNER_ntick_decimals->set_int_limits(0, 6, GLUI_LIMIT_CLAMP);
  GLUILabelsCB(LABELS_tick_inside);
  GLUILabelsCB(LABELS_tick_outside);

  PANEL_tick1b = glui_labels->add_panel_to_panel(PANEL_tick1,"",false);
  CHECKBOX_tick_auto=glui_labels->add_checkbox_to_panel(PANEL_tick1b,_("Auto place"),&auto_user_tick_placement,LABELS_ticks,GLUILabelsCB);
  glui_labels->add_column_to_panel(PANEL_tick1b,false);
  CHECKBOX_user_ticks_show_x=glui_labels->add_checkbox_to_panel(PANEL_tick1b,"x",&user_tick_show_x);
  glui_labels->add_column_to_panel(PANEL_tick1b,false);
  CHECKBOX_user_ticks_show_y=glui_labels->add_checkbox_to_panel(PANEL_tick1b,"y",&user_tick_show_y);
  glui_labels->add_column_to_panel(PANEL_tick1b,false);
  CHECKBOX_user_ticks_show_z=glui_labels->add_checkbox_to_panel(PANEL_tick1b,"z",&user_tick_show_z);
  GLUILabelsCB(LABELS_ticks);
  glui_labels->add_column_to_panel(PANEL_tick1b, false);
  SPINNER_subtick = glui_labels->add_spinner_to_panel(PANEL_tick1b, _("sub-intervals"), GLUI_SPINNER_INT, &user_tick_sub);
  SPINNER_subtick->set_int_limits(1, 10, GLUI_LIMIT_CLAMP);

  PANEL_tick2 = glui_labels->add_panel_to_panel(ROLLOUT_user_tick,_("Parameters"),true);
  glui_labels->add_statictext_to_panel(PANEL_tick2,"                    x");
  SPINNER_tick_x0=glui_labels->add_spinner_to_panel(PANEL_tick2,_("origin"),GLUI_SPINNER_FLOAT,user_tick_origin);
  SPINNER_tick_xmin=glui_labels->add_spinner_to_panel(PANEL_tick2,_("Min"),GLUI_SPINNER_FLOAT,user_tick_min);
  SPINNER_tick_xmax=glui_labels->add_spinner_to_panel(PANEL_tick2,_("Max"),GLUI_SPINNER_FLOAT,user_tick_max);
  SPINNER_tick_dx0=glui_labels->add_spinner_to_panel(PANEL_tick2,_("Step"),GLUI_SPINNER_FLOAT,user_tick_step);

  glui_labels->add_column_to_panel(PANEL_tick2,false);

  glui_labels->add_statictext_to_panel(PANEL_tick2,"                    y");
  SPINNER_tick_y0=glui_labels->add_spinner_to_panel(PANEL_tick2,"",GLUI_SPINNER_FLOAT,user_tick_origin+1);
  SPINNER_tick_ymin=glui_labels->add_spinner_to_panel(PANEL_tick2,"",GLUI_SPINNER_FLOAT,user_tick_min+1);
  SPINNER_tick_ymax=glui_labels->add_spinner_to_panel(PANEL_tick2,"",GLUI_SPINNER_FLOAT,user_tick_max+1);
  SPINNER_tick_dy0=glui_labels->add_spinner_to_panel(PANEL_tick2,"",GLUI_SPINNER_FLOAT,user_tick_step+1);

  glui_labels->add_column_to_panel(PANEL_tick2,false);

  glui_labels->add_statictext_to_panel(PANEL_tick2,"                    z");
  SPINNER_tick_z0=glui_labels->add_spinner_to_panel(PANEL_tick2,"",GLUI_SPINNER_FLOAT,user_tick_origin+2);
  SPINNER_tick_zmin=glui_labels->add_spinner_to_panel(PANEL_tick2,"",GLUI_SPINNER_FLOAT,user_tick_min+2);
  SPINNER_tick_zmax=glui_labels->add_spinner_to_panel(PANEL_tick2,"",GLUI_SPINNER_FLOAT,user_tick_max+2);
  SPINNER_tick_dz0=glui_labels->add_spinner_to_panel(PANEL_tick2,"",GLUI_SPINNER_FLOAT,user_tick_step+2);

#ifdef pp_SPHERE
  // -------------- User sphere settings -------------------

  ROLLOUT_user_spheres = glui_labels->add_rollout("User spheres", false, USER_SPHERES_ROLLOUT, DisplayRolloutCB);
  TOGGLE_ROLLOUT(displayprocinfo, ndisplayprocinfo, ROLLOUT_user_spheres, USER_SPHERES_ROLLOUT, glui_labels);
  PANEL_sphere1 = glui_labels->add_panel_to_panel(ROLLOUT_user_spheres,"",false);
  glui_labels->add_column_to_panel(ROLLOUT_user_spheres,false);
  PANEL_sphere2 = glui_labels->add_panel_to_panel(ROLLOUT_user_spheres,"",false);
  glui_labels->add_column_to_panel(ROLLOUT_user_spheres,false);
  PANEL_sphere3 = glui_labels->add_panel_to_panel(ROLLOUT_user_spheres,"",false);
  glui_labels->add_column_to_panel(ROLLOUT_user_spheres,false);
  PANEL_sphere4 = glui_labels->add_panel_to_panel(ROLLOUT_user_spheres,"",false);

  SPINNER_sphere_xyz0[0] = glui_labels->add_spinner_to_panel(PANEL_sphere1, "x0", GLUI_SPINNER_FLOAT, sphere_xyz0);
  SPINNER_sphere_xyz0[1] = glui_labels->add_spinner_to_panel(PANEL_sphere1, "y0", GLUI_SPINNER_FLOAT, sphere_xyz0+1);
  SPINNER_sphere_xyz0[2] = glui_labels->add_spinner_to_panel(PANEL_sphere1, "z0", GLUI_SPINNER_FLOAT, sphere_xyz0+2);

  SPINNER_sphere_dxyz[0] = glui_labels->add_spinner_to_panel(PANEL_sphere2,"dx",GLUI_SPINNER_FLOAT, sphere_dxyz);
  SPINNER_sphere_dxyz[1] = glui_labels->add_spinner_to_panel(PANEL_sphere2,"dy",GLUI_SPINNER_FLOAT, sphere_dxyz+1);
  SPINNER_sphere_dxyz[2] = glui_labels->add_spinner_to_panel(PANEL_sphere2,"dz",GLUI_SPINNER_FLOAT, sphere_dxyz+2);

  SPINNER_sphere_nxyz[0] = glui_labels->add_spinner_to_panel(PANEL_sphere3,"nx",GLUI_SPINNER_INT, sphere_nxyz);
  SPINNER_sphere_nxyz[1] = glui_labels->add_spinner_to_panel(PANEL_sphere3,"ny",GLUI_SPINNER_INT, sphere_nxyz+1);
  SPINNER_sphere_nxyz[2] = glui_labels->add_spinner_to_panel(PANEL_sphere3,"nz",GLUI_SPINNER_INT, sphere_nxyz+2);

  SPINNER_sphere_rgb[0]  = glui_labels->add_spinner_to_panel(PANEL_sphere4, "red",   GLUI_SPINNER_INT, sphere_rgb);
  SPINNER_sphere_rgb[1]  = glui_labels->add_spinner_to_panel(PANEL_sphere4, "green", GLUI_SPINNER_INT, sphere_rgb+1);
  SPINNER_sphere_rgb[2]  = glui_labels->add_spinner_to_panel(PANEL_sphere4, "blue",  GLUI_SPINNER_INT, sphere_rgb+2);

  SPINNER_sphere_rgb[0]->set_int_limits(0, 255);
  SPINNER_sphere_rgb[1]->set_int_limits(0, 255);
  SPINNER_sphere_rgb[2]->set_int_limits(0, 255);
  SPINNER_sphere_diameter=glui_labels->add_spinner_to_panel(PANEL_sphere1,"diameter",GLUI_SPINNER_FLOAT,&sphere_diameter);
  CHECKBOX_sphere_show=glui_labels->add_checkbox_to_panel(PANEL_sphere1,_("Show"),&sphere_show);
#endif

  // -------------- User labels -------------------

  gl=&LABEL_local;
  ROLLOUT_user_labels = glui_labels->add_rollout("Labels + Ticks",false,LABELS_TICKS_ROLLOUT,DisplayRolloutCB);
  TOGGLE_ROLLOUT(displayprocinfo, ndisplayprocinfo, ROLLOUT_user_labels, LABELS_TICKS_ROLLOUT, glui_labels);

  PANEL_LB_panel1 = glui_labels->add_panel_to_panel(ROLLOUT_user_labels,"",GLUI_PANEL_NONE);


  PANEL_LB_panel3 = glui_labels->add_panel_to_panel(ROLLOUT_user_labels,_("Labels"));

  CHECKBOX_LB_visLabels=glui_labels->add_checkbox_to_panel(PANEL_LB_panel3,_("Show labels"),&visLabels,LB_VISLABELS,TextLabelsCB);

  PANEL_LB_panel4 = glui_labels->add_panel_to_panel(PANEL_LB_panel3,"",GLUI_PANEL_NONE);
  BUTTON_LB_label_add=glui_labels->add_button_to_panel(PANEL_LB_panel4,"Add",LB_ADD,TextLabelsCB);
  glui_labels->add_column_to_panel(PANEL_LB_panel4,false);
  BUTTON_LB_label_delete=glui_labels->add_button_to_panel(PANEL_LB_panel4,_("Delete"),LB_DELETE,TextLabelsCB);

  LIST_LB_labels=glui_labels->add_listbox_to_panel(PANEL_LB_panel3,_("Select"),&label_list_index,LB_LIST,TextLabelsCB);
  {
    labeldata *thislabel;
    int count=0;

    for(thislabel=global_scase.labelscoll.label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
      if(thislabel->labeltype==TYPE_SMV){
        thislabel->glui_id=-1;
        continue;
      }
      thislabel->glui_id=count;
      LIST_LB_labels->add_item(count++,thislabel->name);
    }
  }
  PANEL_LB_panel2 = glui_labels->add_panel_to_panel(PANEL_LB_panel3,"",GLUI_PANEL_NONE);
  EDIT_LB_label_string=glui_labels->add_edittext_to_panel(PANEL_LB_panel2,_("Edit:"),GLUI_EDITTEXT_TEXT,gl->name,LB_UPDATE,TextLabelsCB);
  glui_labels->add_column_to_panel(PANEL_LB_panel2,false);
  BUTTON_LB_label_update=glui_labels->add_button_to_panel(PANEL_LB_panel2,_("Update"),LB_UPDATE,TextLabelsCB);

  PANEL_LB_panel6 = glui_labels->add_panel_to_panel(PANEL_LB_panel3,"",GLUI_PANEL_NONE);
  BUTTON_LB_label_previous=glui_labels->add_button_to_panel(PANEL_LB_panel6,_("Previous"),LB_PREVIOUS,TextLabelsCB);
  glui_labels->add_column_to_panel(PANEL_LB_panel6,false);
  BUTTON_LB_label_next=glui_labels->add_button_to_panel(PANEL_LB_panel6,_("Next"),LB_NEXT,TextLabelsCB);

  PANEL_LB_panel5 = glui_labels->add_panel_to_panel(ROLLOUT_user_labels,"",GLUI_PANEL_NONE);
  PANEL_LB_position=glui_labels->add_panel_to_panel(PANEL_LB_panel5,_("position"));
  SPINNER_LB_x=glui_labels->add_spinner_to_panel(PANEL_LB_position,"x",GLUI_SPINNER_FLOAT,gl->xyz,LB_XYZ,TextLabelsCB);
  SPINNER_LB_y=glui_labels->add_spinner_to_panel(PANEL_LB_position,"y",GLUI_SPINNER_FLOAT,gl->xyz+1,LB_XYZ,TextLabelsCB);
  SPINNER_LB_z=glui_labels->add_spinner_to_panel(PANEL_LB_position,"z",GLUI_SPINNER_FLOAT,gl->xyz+2,LB_XYZ,TextLabelsCB);
  {
    float xmin, ymin, zmin, xmax, ymax, zmax;

    xmin = xbar0ORIG - 0.25*(xbarORIG-xbar0ORIG);
    xmax = xbarORIG + 0.25*(xbarORIG-xbar0ORIG);
    ymin = ybar0ORIG - 0.25*(ybarORIG-ybar0ORIG);
    ymax = ybarORIG + 0.25*(ybarORIG-ybar0ORIG);
    zmin = zbar0ORIG - 0.25*(zbarORIG-zbar0ORIG);
    zmax = zbarORIG + 0.25*(zbarORIG-zbar0ORIG);
    SPINNER_LB_x->set_float_limits(xmin,xmax);
    SPINNER_LB_y->set_float_limits(ymin,ymax);
    SPINNER_LB_z->set_float_limits(zmin,zmax);

  }

  glui_labels->add_column_to_panel(PANEL_LB_panel5,false);
  PANEL_LB_time=glui_labels->add_panel_to_panel(PANEL_LB_panel5,_("time"));
  SPINNER_LB_time_start=glui_labels->add_spinner_to_panel(PANEL_LB_time,_("start"),GLUI_SPINNER_FLOAT,gl->tstart_stop,LB_STARTSTOP,TextLabelsCB);
  SPINNER_LB_time_stop=glui_labels->add_spinner_to_panel(PANEL_LB_time,_("stop"),GLUI_SPINNER_FLOAT,gl->tstart_stop+1,LB_STARTSTOP,TextLabelsCB);
  CHECKBOX_LB_label_show_always=glui_labels->add_checkbox_to_panel(PANEL_LB_time,_("Show always"),&gl->show_always,LB_SHOWALWAYS,TextLabelsCB);

  PANEL_LB_color=glui_labels->add_panel_to_panel(ROLLOUT_user_labels,_("color"));
  SPINNER_LB_red=glui_labels->add_spinner_to_panel(PANEL_LB_color,_("red"),GLUI_SPINNER_INT,gl->rgb,LB_RGB,TextLabelsCB);
  SPINNER_LB_green=glui_labels->add_spinner_to_panel(PANEL_LB_color,_("green"),GLUI_SPINNER_INT,gl->rgb+1,LB_RGB,TextLabelsCB);
  SPINNER_LB_blue=glui_labels->add_spinner_to_panel(PANEL_LB_color,_("blue"),GLUI_SPINNER_INT,gl->rgb+2,LB_RGB,TextLabelsCB);
  SPINNER_LB_red->set_int_limits(0,255);
  SPINNER_LB_green->set_int_limits(0,255);
  SPINNER_LB_blue->set_int_limits(0,255);
  CHECKBOX_LB_label_use_foreground=glui_labels->add_checkbox_to_panel(PANEL_LB_color,_("Use foreground color"),&gl->useforegroundcolor,LB_FOREGROUND,TextLabelsCB);

  ROLLOUT_LB_tick0 = glui_labels->add_rollout_to_panel(ROLLOUT_user_labels, "tick",false);
  CHECKBOX_labels_showtick = glui_labels->add_checkbox_to_panel(ROLLOUT_LB_tick0, "show tick", &gl->show_tick, LB_SHOW_TICK, TextLabelsCB);
  PANEL_LB_tick = glui_labels->add_panel_to_panel(ROLLOUT_LB_tick0, "",GLUI_PANEL_NONE);
  SPINNER_LB_tick_xbeg = glui_labels->add_spinner_to_panel(PANEL_LB_tick, "x", GLUI_SPINNER_FLOAT, gl->tick_begin, LB_TICK_XYZ, TextLabelsCB);
  SPINNER_LB_tick_ybeg = glui_labels->add_spinner_to_panel(PANEL_LB_tick, "y", GLUI_SPINNER_FLOAT, gl->tick_begin+1, LB_TICK_XYZ, TextLabelsCB);
  SPINNER_LB_tick_zbeg = glui_labels->add_spinner_to_panel(PANEL_LB_tick, "z", GLUI_SPINNER_FLOAT, gl->tick_begin+2, LB_TICK_XYZ, TextLabelsCB);
  glui_labels->add_column_to_panel(PANEL_LB_tick, false);
  SPINNER_LB_tick_xdir = glui_labels->add_spinner_to_panel(PANEL_LB_tick, "dx", GLUI_SPINNER_FLOAT, gl->tick_direction, LB_TICK_XYZ, TextLabelsCB);
  SPINNER_LB_tick_ydir = glui_labels->add_spinner_to_panel(PANEL_LB_tick, "dy", GLUI_SPINNER_FLOAT, gl->tick_direction+1, LB_TICK_XYZ, TextLabelsCB);
  SPINNER_LB_tick_zdir = glui_labels->add_spinner_to_panel(PANEL_LB_tick, "dz", GLUI_SPINNER_FLOAT, gl->tick_direction+2, LB_TICK_XYZ, TextLabelsCB);

  TextLabelsCB(LB_LIST);

  // -------------- Sky -------------------

  ROLLOUT_sky = glui_labels->add_rollout("Sky/ground",false,SKY_ROLLOUT,DisplayRolloutCB);
  TOGGLE_ROLLOUT(displayprocinfo, ndisplayprocinfo, ROLLOUT_sky, SKY_ROLLOUT, glui_labels);
  if(skyboxinfo != NULL){
    PANEL_box=glui_labels->add_panel_to_panel(ROLLOUT_sky,_("box"));
    CHECKBOX_visSkybox      = glui_labels->add_checkbox_to_panel(PANEL_box, _("show"), &visSkybox, SKY_BOX, GLUISkyCB);
    CHECKBOX_visSkyboxoutline = glui_labels->add_checkbox_to_panel(PANEL_box, _("show box outlines"), &visSkyboxoutline);
  }
  PANEL_sphere=glui_labels->add_panel_to_panel(ROLLOUT_sky,_("hemisphere"));
  CHECKBOX_visSkysphere = glui_labels->add_checkbox_to_panel(PANEL_sphere, _("show"), &visSkysphere, SKY_SPHERE, GLUISkyCB);
  CHECKBOX_visSkyground = glui_labels->add_checkbox_to_panel(PANEL_sphere, _("show ground"), &visSkyground, SKY_SPHERE, GLUISkyCB);
  PANEL_skycolor = glui_labels->add_panel_to_panel(PANEL_sphere, "", false);
  PANEL_horizon_color = glui_labels->add_panel_to_panel(PANEL_skycolor, _("horizon color"));
  SPINNER_horizon_color[0] = glui_labels->add_spinner_to_panel(PANEL_horizon_color, "red",   GLUI_SPINNER_INT, horizon_color);
  SPINNER_horizon_color[1] = glui_labels->add_spinner_to_panel(PANEL_horizon_color, "green", GLUI_SPINNER_INT, horizon_color+1);
  SPINNER_horizon_color[2] = glui_labels->add_spinner_to_panel(PANEL_horizon_color, "blue",  GLUI_SPINNER_INT, horizon_color+2);

  PANEL_zenith_color = glui_labels->add_panel_to_panel(PANEL_skycolor, _("zenith color"));
  SPINNER_zenith_color[0] = glui_labels->add_spinner_to_panel(PANEL_zenith_color, "red",   GLUI_SPINNER_INT, zenith_color);
  SPINNER_zenith_color[1] = glui_labels->add_spinner_to_panel(PANEL_zenith_color, "green", GLUI_SPINNER_INT, zenith_color+1);
  SPINNER_zenith_color[2] = glui_labels->add_spinner_to_panel(PANEL_zenith_color, "blue",  GLUI_SPINNER_INT, zenith_color+2);

  PANEL_ground_color = glui_labels->add_panel_to_panel(PANEL_skycolor, _("ground color"));
  SPINNER_ground_color[0] = glui_labels->add_spinner_to_panel(PANEL_ground_color, "red",   GLUI_SPINNER_INT, ground_color);
  SPINNER_ground_color[1] = glui_labels->add_spinner_to_panel(PANEL_ground_color, "green", GLUI_SPINNER_INT, ground_color+1);
  SPINNER_ground_color[2] = glui_labels->add_spinner_to_panel(PANEL_ground_color, "blue",  GLUI_SPINNER_INT, ground_color+2);
  glui_labels->add_button_to_panel(PANEL_skycolor,_("Reset colors"),RESET_COLORS,GLUISkyCB);

  for(i=0;i<3;i++){
    SPINNER_horizon_color[i]->set_int_limits(0,255);
    SPINNER_zenith_color[i]->set_int_limits(0,255);
    SPINNER_ground_color[i]->set_int_limits(0,255);
  }
  
  SPINNER_sky_diam = glui_labels->add_spinner_to_panel(PANEL_sphere, _("diameter"), GLUI_SPINNER_FLOAT, &sky_diam, SKY_BOX, GLUISkyCB);
  if(global_scase.sky_texture != NULL){
    glui_labels->add_checkbox_to_panel(PANEL_sphere, _("show texture"), &visSkySpheretexture, SKY_SPHERE, GLUISkyCB);
  }

  // --------------

  PANEL_label2 = glui_labels->add_panel("",false);
  glui_labels->add_column_to_panel(PANEL_label2,false);

  BUTTON_label_3=glui_labels->add_button_to_panel(PANEL_label2,_("Save settings"),SAVE_SETTINGS_DISPLAY,GLUILabelsCB);
  glui_labels->add_column_to_panel(PANEL_label2,false);

  BUTTON_label_4=glui_labels->add_button_to_panel(PANEL_label2,_("Close"),LABELS_close,GLUILabelsCB);
#ifdef pp_CLOSEOFF
  BUTTON_label_4->disable();
#endif

  glui_labels->set_main_gfx_window( main_window );
}

/* ------------------ GLUIHideDisplay ------------------------ */

extern "C" void GLUIHideDisplay(void){
  GLUICloseRollouts(glui_labels);
}

/* ------------------ GLUIShowDisplay ------------------------ */

extern "C" void GLUIShowDisplay(int menu_id){
  if(glui_labels!=NULL)glui_labels->show();
  switch(menu_id){
  case DIALOG_DISPLAY:
    DisplayRolloutCB(LABELS_TITLES_ROLLOUT);
    break;
  case DIALOG_FONTS:
    DisplayRolloutCB(FONTS_ROLLOUT);
    break;
  case DIALOG_USER_TICKS:
    DisplayRolloutCB(USER_TICKS_ROLLOUT);
    break;
  case DIALOG_LABELS_TICKS:
    DisplayRolloutCB(LABELS_TICKS_ROLLOUT);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIUpdateColorbarControls ------------------------ */

extern "C" void GLUIUpdateColorbarControls(void){
  if(CHECKBOX_visColorbarVertical!=NULL&&CHECKBOX_visColorbarVertical->get_int_val() != visColorbarVertical)CHECKBOX_visColorbarVertical->set_int_val(visColorbarVertical);
  if(CHECKBOX_visColorbarHorizontal!=NULL&&CHECKBOX_visColorbarHorizontal->get_int_val() != visColorbarHorizontal)CHECKBOX_visColorbarHorizontal->set_int_val(visColorbarHorizontal);
}

/* ------------------ GLUIUpdateGeomBoundingBox ------------------------ */

extern "C" void GLUIUpdateGeomBoundingBox(void){
  if(CHECKBOX_hide_scene != NULL)CHECKBOX_hide_scene->set_int_val(hide_scene);
}

/* ------------------ GLUILabelsCB ------------------------ */

extern "C" void GLUILabelsCB(int var){
  updatemenu=1;
  switch(var){
    case LABELS_BOUNDING_BOX:
      updatemenu = 1;
      break;
#ifdef pp_REFRESH
    case LABELS_REFRESH_RATE:
      if(glui_refresh_rate>0){
        refresh_interval = 1000/(float)glui_refresh_rate;
        if(glui_refresh_rate_old==0){
          PeriodicRefresh(refresh_interval);
          GLUTPOSTREDISPLAY;
        }
      }
      glui_refresh_rate_old = glui_refresh_rate;
      break;
#endif
  case LABELS_colorbar_shift:
    UpdateRGBColors(colorbar_select_index);
    break;
 // vis_colorbar      state
 //    0/COLORBAR_HIDDEN               hidden
 //    1/COLORBAR_SHOW_VERTICAL        vertical
 //    2->max/COLORBAR_SHOW_HORIZONTAL horizontal
  case LABELS_vcolorbar:
  case LABELS_hcolorbar:
    if(var==LABELS_vcolorbar){
      if(visColorbarVertical == 1)visColorbarHorizontal = 0;
    }
    else{
      if(visColorbarHorizontal == 1)visColorbarVertical = 0;
    }
    GLUIUpdateColorbarControls();
    GLUIUpdateColorbarControls2();
    vis_colorbar = GetColorbarState();
    break;
  case LABELS_tick_inside:
  case LABELS_tick_outside:
    if(var==LABELS_tick_inside){
      glui_tick_outside = 1 - glui_tick_inside;
      CHECKBOX_ticks_outside->set_int_val(glui_tick_outside);
    }
    if(var==LABELS_tick_outside){
      glui_tick_inside = 1 - glui_tick_outside;
      CHECKBOX_ticks_inside->set_int_val(glui_tick_inside);
    }
    user_tick_direction=1.0;
    if(glui_tick_inside==1)user_tick_direction=-1.0;
    break;
  case APPLY_VENTOFFSET:
    UpdateVentOffset();
    global_scase.updatefaces=1;
    break;
  case FLIP:
      colorbar_flip = 1 - colorbar_flip;
      ColorbarMenu(COLORBAR_FLIP);
      break;
  case LABELS_hide_overlaps:
    updatefacelists=1;
    glutPostRedisplay();
    break;
  case LABELS_drawface:
    updatefacelists=1;
    if(
        SPINNER_mesh_debug != NULL &&
        SPINNER_blockage_min_debug != NULL &&
        SPINNER_blockage_n_debug != NULL
      ){
      if(blockage_draw_option == 0 || blockage_draw_option == 1){
        SPINNER_mesh_debug->disable();
        SPINNER_blockage_min_debug->disable();
        SPINNER_blockage_n_debug->disable();
      }
      else{
        SPINNER_mesh_debug->enable();
        SPINNER_blockage_min_debug->enable();
        SPINNER_blockage_n_debug->enable();
      }
    }
    break;
  case CB_USE_LIGHTING:
  case LABELS_shownorth:
  case LABELS_version:
  case LABELS_meshlabel:
  case LABELS_label:
  case LABELS_HMS:
  case LABELS_transparent:
  case FIRECUTOFF_label:
    break;
#ifdef pp_memload
  case LABELS_memload:
#ifdef pp_memusage
    if(vismemload == 1 && vismemusage == 1){
      vismemusage = 0;
      CHECKBOX_labels_memusage->set_int_val(vismemusage);
    }
#endif
    break;
#endif
#ifdef pp_memusage
  case LABELS_memusage:
#ifdef pp_memload
    if(vismemload == 1 && vismemusage == 1){
      vismemload = 0;
      CHECKBOX_labels_memload->set_int_val(vismemload);
    }
#endif
    break;
#endif
  case LABELS_usertick:
    CHECKBOX_visUSERticks2->set_int_val(visUSERticks);
    break;
  case LABELS_usertick2:
    CHECKBOX_visUSERticks->set_int_val(visUSERticks);
    if(visUSERticks==1)ROLLOUT_user_tick->open();
    break;
  case SAVE_SETTINGS_DISPLAY:
    WriteIni(LOCAL_INI,NULL);
    break;
  case LABELS_showall:
    LabelMenu(MENU_LABEL_ShowAll);
    break;
  case LABELS_hideall:
    LabelMenu(MENU_LABEL_HideAll);
    break;
  case LABELS_flip:
    background_flip = 1 - background_flip;
    ShowHideMenu(MENU_SHOWHIDE_FLIP);
    break;
  case LABELS_shade:
    InitRGB();
    break;
  case LABELS_shadedata:
    if(setbwdata==1 && colorbars.bw_colorbar_index>=0){
      colorbartype_save=colorbartype;
      ColorbarMenu(colorbars.bw_colorbar_index);
    }
    else{
      if(colorbartype_save>-1)ColorbarMenu(colorbartype_save);
    }
    break;
  case LABELS_close:
    GLUIHideDisplay();
    break;
  case LABELS_fontsize:
    FontMenu(fontindex);
    break;
  case FRAME_label:
    visFramelabel=1-visFramelabel;
    LabelMenu(MENU_LABEL_framerate);
    break;
  case HRR_label:
    vis_hrr_label=1-vis_hrr_label;
    LabelMenu(MENU_LABEL_hrr);
    break;
  case LABELS_ticks:
    if(auto_user_tick_placement==1){
      CHECKBOX_user_ticks_show_x->disable();
      CHECKBOX_user_ticks_show_y->disable();
      CHECKBOX_user_ticks_show_z->disable();
    }
    else{
      CHECKBOX_user_ticks_show_x->enable();
      CHECKBOX_user_ticks_show_y->enable();
      CHECKBOX_user_ticks_show_z->enable();
    }
    break;
  default:
    assert(FFALSE);
  }
}

/* ------------------ GLUISetLabelControls ------------------------ */

extern "C" void GLUISetLabelControls(){

  if(CHECKBOX_LB_visLabels!=NULL)CHECKBOX_LB_visLabels->set_int_val(visLabels);
  if(CHECKBOX_visUSERticks!=NULL)CHECKBOX_visUSERticks->set_int_val(visUSERticks);
  if(CHECKBOX_labels_hrrlabel!=NULL)CHECKBOX_labels_hrrlabel->set_int_val(vis_hrr_label);
  if(CHECKBOX_labels_title!=NULL)CHECKBOX_labels_title->set_int_val(vis_title_smv_version);
  if(CHECKBOX_labels_fds_title!=NULL)CHECKBOX_labels_fds_title->set_int_val(vis_title_fds);
  if(CHECKBOX_labels_chid!=NULL)CHECKBOX_labels_chid->set_int_val(vis_title_CHID);
  if(CHECKBOX_visColorbarVertical!=NULL)CHECKBOX_visColorbarVertical->set_int_val(visColorbarVertical);
  if(CHECKBOX_labels_timebar!=NULL)CHECKBOX_labels_timebar->set_int_val(visTimebar);
  if(CHECKBOX_labels_timelabel!=NULL)CHECKBOX_labels_timelabel->set_int_val(visTimelabel);
  if(CHECKBOX_labels_frametimelabel != NULL)CHECKBOX_labels_frametimelabel->set_int_val(visFrameTimelabel);
  if(CHECKBOX_labels_framelabel != NULL)CHECKBOX_labels_framelabel->set_int_val(visFramelabel);
  if(CHECKBOX_labels_ticks!=NULL)CHECKBOX_labels_ticks->set_int_val(visFDSticks);
  if(CHECKBOX_labels_axis!=NULL)CHECKBOX_labels_axis->set_int_val(visaxislabels);
  if(CHECKBOX_labels_framerate!=NULL)CHECKBOX_labels_framerate->set_int_val(visFramerate);
  if(CHECKBOX_labels_average!=NULL)CHECKBOX_labels_average->set_int_val(vis_slice_average);
#ifdef pp_memload
  if(CHECKBOX_labels_memload!=NULL)CHECKBOX_labels_memload->set_int_val(vismemload);
#endif
#ifdef pp_memusage
  if(CHECKBOX_labels_memusage != NULL)CHECKBOX_labels_memusage->set_int_val(vismemusage);
#endif
  if(CHECKBOX_labels_labels != NULL)CHECKBOX_labels_labels->set_int_val(visLabels);

  if(CHECKBOX_labels_flip!=NULL)CHECKBOX_labels_flip->set_int_val(background_flip);
  if(RADIO_fontsize != NULL)RADIO_fontsize->set_int_val(fontindex);
  if(CHECKBOX_labels_hms!=NULL)CHECKBOX_labels_hms->set_int_val(vishmsTimelabel);
  if(CHECKBOX_labels_gridloc!=NULL)CHECKBOX_labels_gridloc->set_int_val(visgridloc);
  if(CHECKBOX_labels_version!=NULL)CHECKBOX_labels_version->set_int_val(vis_title_gversion);
  if(CHECKBOX_labels_meshlabel!=NULL)CHECKBOX_labels_meshlabel->set_int_val(visMeshlabel);
  if(CHECKBOX_visUSERticks2!=NULL)CHECKBOX_visUSERticks2->set_int_val(visUSERticks);
  GLUISetLabelControls2();

}
