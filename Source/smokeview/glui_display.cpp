#define CPP
#include "options.h"
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

GLUI *glui_labels=NULL;

GLUI_EditText *EDIT_LB_label_string=NULL;

GLUI_Spinner *SPINNER_colorband=NULL;
GLUI_Spinner *SPINNER_labels_transparency_data=NULL;
#ifdef pp_BETA
GLUI_Spinner *SPINNER_cullgeom_portsize=NULL;
#endif

GLUI_Listbox *LIST_colorbar2=NULL;
GLUI_Listbox *LIST_LB_labels=NULL;

GLUI_Spinner *SPINNER_LB_tick_xbeg=NULL;
GLUI_Spinner *SPINNER_LB_tick_ybeg=NULL;
GLUI_Spinner *SPINNER_LB_tick_zbeg=NULL;
GLUI_Spinner *SPINNER_LB_tick_xdir=NULL;
GLUI_Spinner *SPINNER_LB_tick_ydir=NULL;
GLUI_Spinner *SPINNER_LB_tick_zdir=NULL;

GLUI_Spinner *SPINNER_light_az0=NULL;
GLUI_Spinner *SPINNER_light_az1=NULL;
GLUI_Spinner *SPINNER_light_elev0=NULL;
GLUI_Spinner *SPINNER_light_elev1=NULL;

GLUI_Spinner *SPINNER_down_red=NULL,*SPINNER_down_green=NULL,*SPINNER_down_blue=NULL;
GLUI_Spinner *SPINNER_up_red=NULL,*SPINNER_up_green=NULL,*SPINNER_up_blue=NULL;
GLUI_Spinner *SPINNER_colorsplit[12];
GLUI_Spinner *SPINNER_valsplit[3];
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

GLUI_Checkbox *CHECKBOX_use_lighting=NULL;
GLUI_Checkbox *CHECKBOX_labels_showtick = NULL;
GLUI_Checkbox *CHECKBOX_labels_meshlabel = NULL;
GLUI_Checkbox *CHECKBOX_labels_version=NULL;
GLUI_Checkbox *CHECKBOX_visUSERticks=NULL;
GLUI_Checkbox *CHECKBOX_visUSERticks2=NULL;
GLUI_Checkbox *CHECKBOX_show_extreme_mindata=NULL;
GLUI_Checkbox *CHECKBOX_show_extreme_maxdata=NULL;
GLUI_Checkbox *CHECKBOX_colorbar_flip=NULL;
GLUI_Checkbox *CHECKBOX_colorbar_autoflip = NULL;
#ifdef pp_BETA
GLUI_Checkbox *CHECKBOX_cullgeom=NULL;
#endif
GLUI_Checkbox *CHECKBOX_axislabels_smooth=NULL, *CHECKBOX_transparentflag=NULL;
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
GLUI_Checkbox *CHECKBOX_labels_hrrlabel=NULL;
GLUI_Checkbox *CHECKBOX_labels_firecutoff=NULL;
GLUI_Checkbox *CHECKBOX_labels_availmemory=NULL;
GLUI_Checkbox *CHECKBOX_labels_labels=NULL;
GLUI_Checkbox *CHECKBOX_labels_gridloc=NULL;
GLUI_Checkbox *CHECKBOX_labels_average=NULL;
GLUI_Checkbox *CHECKBOX_user_ticks_show_x=NULL;
GLUI_Checkbox *CHECKBOX_user_ticks_show_y=NULL;
GLUI_Checkbox *CHECKBOX_user_ticks_show_z=NULL;
GLUI_Checkbox *CHECKBOX_tick_auto=NULL;
GLUI_Checkbox *CHECKBOX_label_1=NULL;
GLUI_Checkbox *CHECKBOX_label_2=NULL;
GLUI_Checkbox *CHECKBOX_label_3=NULL;
GLUI_Checkbox *CHECKBOX_labels_flip=NULL;
GLUI_Checkbox *CHECKBOX_labels_shade=NULL;
GLUI_Checkbox *CHECKBOX_labels_shadedata=NULL;
GLUI_Checkbox *CHECKBOX_labels_transparent_override=NULL;
GLUI_Checkbox *CHECKBOX_shownorth = NULL;
GLUI_Checkbox *CHECKBOX_ticks_inside = NULL;
GLUI_Checkbox *CHECKBOX_ticks_outside = NULL;

GLUI_Rollout *ROLLOUT_LB_tick0 = NULL;
GLUI_Rollout *ROLLOUT_coloring=NULL;
GLUI_Rollout *ROLLOUT_font=NULL;
GLUI_Rollout *ROLLOUT_user_labels=NULL;
GLUI_Rollout *ROLLOUT_user_tick=NULL;
GLUI_Rollout *ROLLOUT_general=NULL;
GLUI_Rollout *ROLLOUT_north = NULL;
GLUI_Rollout *ROLLOUT_extreme2 = NULL;
GLUI_Rollout *ROLLOUT_split = NULL;
GLUI_Rollout *ROLLOUT_light2 = NULL;

GLUI_Panel *PANEL_light=NULL;
GLUI_Panel *PANEL_position0=NULL;
GLUI_Panel *PANEL_position1 = NULL;
GLUI_Panel *PANEL_ambient=NULL;
GLUI_Panel *PANEL_diffuse=NULL;
GLUI_Panel *PANEL_specular = NULL;
GLUI_Panel *PANEL_positional = NULL;
GLUI_Panel *PANEL_positional2 = NULL;
GLUI_Panel *PANEL_timebar_overlap = NULL;
GLUI_Panel *PANEL_split1L = NULL, *PANEL_split1H = NULL;
GLUI_Panel *PANEL_split2L = NULL, *PANEL_split2H = NULL;
GLUI_Panel *PANEL_split3 = NULL;
GLUI_Panel *PANEL_extreme=NULL,*PANEL_cb8=NULL,*PANEL_cb7=NULL;
GLUI_Panel *PANEL_extreme_min=NULL, *PANEL_extreme_max=NULL;
GLUI_Panel *PANEL_cb11=NULL;
GLUI_Panel *PANEL_contours=NULL;
GLUI_Panel *PANEL_gen1=NULL, *PANEL_gen2=NULL, *PANEL_gen3=NULL;
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

GLUI_RadioGroup *RADIO_timebar_overlap = NULL;
GLUI_RadioGroup *RADIO2_plot3d_display=NULL;
GLUI_RadioGroup *RADIO_fontsize = NULL;
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

#define COLORBAR_EXTREME_RGB 15
#define COLORBAR_EXTREME 16
#define FLIP 19
#define APPLY_VENTOFFSET 20

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

#define LABELS_label 0
#define LABELS_vcolorbar 34
#define LABELS_hcolorbar 35
#define FRAME_label 21
#define HRR_label 22
#define FIRECUTOFF_label 23
#define LABELS_showall 1
#define LABELS_hideall 2
#define LABELS_close 3
#define LABELS_flip 4
#define LABELS_shade 5
#define LABELS_transparent 6
#define LABELS_fontsize 7
#define LABELS_ticks 8
#define LABELS_drawface 24
#define LABELS_hide_overlaps 25
#define LABELS_version 26
#define LABELS_meshlabel 27
#define LABELS_usertick 28
#define LABELS_usertick2 29
#define LABELS_shadedata 30
#define LABELS_shownorth 31
#define LABELS_tick_inside 32
#define LABELS_tick_outside 33

#define SPLIT_COLORBAR 1

#define LABELS_HMS 18
#define SAVE_SETTINGS 99

#define COLORBAR_SMOOTH 113
#define COLORBAND 115
#define CB_USE_LIGHTING 120
#define COLOR_AMB_GREY 116
#define COLOR_DIFF_GREY 117
#define COLOR_AMB_RGB 118
#define COLOR_DIFF_RGB 119
#define COLOR_SPEC_RGB 120
#define COLOR_SPEC_GREY 121
#define LIGHT_POSITION 122

#define COLORBAR_LIST2 112
#define DATA_transparent 26
#define TRANSPARENTLEVEL 110

#define UPDATEPLOT 10

int cb_up_rgb[3],cb_down_rgb[3];

#define GENERAL_ROLLOUT 0
#define COLORING_ROLLOUT 1
#define FONTS_ROLLOUT 2
#define TICKS_ROLLOUT 3
#define LABELS_ROLLOUT 4
#define LIGHT_ROLLOUT 5

procdata displayprocinfo[6];
int ndisplayprocinfo = 0;

/* ------------------ UpdateBackgroundFlip ------------------------ */

extern "C" void UpdateBackgroundFlip(int flip) {
  CHECKBOX_labels_flip->set_int_val(flip);
}

/* ------------------ UpdateUseLighting ------------------------ */

extern "C" void UpdateUseLighting(void) {
  CHECKBOX_use_lighting->set_int_val(use_lighting);
}

/* ------------------ UpdateTimebarOverlap ------------------------ */

extern "C" void UpdateTimebarOverlap(void) {
  RADIO_timebar_overlap->set_int_val(timebar_overlap);
}

/* ------------------ DisplayRolloutCB ------------------------ */

void DisplayRolloutCB(int var){
  ToggleRollout(displayprocinfo, ndisplayprocinfo, var);
}

/* ------------------ UpdateGluiLabelText ------------------------ */

extern "C" void UpdateGluiLabelText(void){
  if(LabelGetNUserLabels()>0){
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

  /* ------------------ GluiUpdateFontIndex ------------------------ */

extern "C" void GluiUpdateFontIndex(void){
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
    for(thislabel = label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->glui_id < 0)continue;
      LIST_LB_labels->delete_item(thislabel->glui_id);
    }
    strcpy(LABEL_global_ptr->name, gl->name);
    //LabelResort(LABEL_global_ptr);

    count = 0;
    for(thislabel = label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
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
    new_label = LabelGet(LIST_LB_labels->curr_text);
    new_label = LabelPrevious(new_label);
    if(new_label == NULL)break;
    LABEL_global_ptr = new_label;
    if(new_label != NULL){
      LabelCopy(gl, new_label);
      UpdateGluiLabelText();
    }
    break;
  case LB_NEXT:
    new_label = LabelGet(LIST_LB_labels->curr_text);
    new_label = LabelNext(new_label);
    if(new_label == NULL)break;
    LABEL_global_ptr = new_label;
    if(new_label != NULL){
      LabelCopy(gl, new_label);
      UpdateGluiLabelText();
    }
    break;
  case LB_LIST:
    new_label = LabelGet(LIST_LB_labels->curr_text);
    LABEL_global_ptr = new_label;
    if(new_label != NULL){
      LabelCopy(gl, new_label);
    }
    UpdateGluiLabelText();
    break;
  case LB_ADD:
    updatemenu = 1;
    if(LabelGetNUserLabels() > 0){
      strcpy(name, "copy of ");
      strcat(name, gl->name);
      strcpy(gl->name, name);
    }
    else{
      gl = &LABEL_default;
    }
    gl->labeltype = TYPE_INI;
    for(thislabel = label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->glui_id < 0)continue;
      LIST_LB_labels->delete_item(thislabel->glui_id);
    }
    LabelInsert(gl);
    count = 0;
    for(thislabel = label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->labeltype == TYPE_SMV)continue;
      thislabel->glui_id = count;
      LIST_LB_labels->add_item(count++, thislabel->name);
    }
    TextLabelsCB(LB_LIST);
    break;
  case LB_DELETE:
    strcpy(name, LIST_LB_labels->curr_text);
    for(thislabel = label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
      if(thislabel->glui_id < 0)continue;
      LIST_LB_labels->delete_item(thislabel->glui_id);
    }
    thislabel = LabelGet(name);
    if(thislabel != NULL){
      LabelDelete(thislabel);
    }
    count = 0;
    for(thislabel = label_first_ptr->next;thislabel->next != NULL;thislabel = thislabel->next){
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
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ ColorCB ------------------------ */

extern "C" void ColorCB(int var){
  int i;

  switch (var){
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
    for(i = 0; i<3; i++){
      glui_diffuselight[i] = glui_diffusegrey;
      diffuselight[i] = (float)glui_diffusegrey/255.0;
    }
    SPINNER_diff_red->set_int_val(glui_diffusegrey);
    SPINNER_diff_green->set_int_val(glui_diffusegrey);
    SPINNER_diff_blue->set_int_val(glui_diffusegrey);
  break;
  }
}

/* ------------------ SplitCB ------------------------ */

extern "C" void SplitCB(int var){
  int isplit, i;
  float denom;

  switch(var){
  case SPLIT_COLORBAR:
    denom = splitvals[2] - splitvals[0];
    if(denom == 0.0)denom = 1.0;
    isplit = CLAMP(255 * (splitvals[1] - splitvals[0]) / denom, 0, 254);
    split_colorbar->index_node[1] = isplit;
    split_colorbar->index_node[2] = isplit + 1;

    for(i = 0; i < 12; i++){
      split_colorbar->rgb_node[i] = colorsplit[i] & 0xFF;
    }
    RemapColorbar(split_colorbar);
    UpdateColorbarSplits(split_colorbar);
    UpdateRGBColors(COLORBAR_INDEX_NONE);
    break;
  }
}

/* ------------------ GluiLabelsSetup ------------------------ */

extern "C" void GluiLabelsSetup(int main_window){
  labeldata *gl;

  update_glui_labels=0;
  if(glui_labels!=NULL){
    glui_labels->close();
    glui_labels=NULL;
  }
  glui_labels = GLUI_Master.create_glui("Display",0,0,0);
  glui_labels->hide();

  // -------------- General Settings -------------------

  ROLLOUT_general = glui_labels->add_rollout(_("General"),true,GENERAL_ROLLOUT,DisplayRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_general, glui_labels);
  ADDPROCINFO(displayprocinfo, ndisplayprocinfo, ROLLOUT_general, GENERAL_ROLLOUT, glui_labels);

  PANEL_gen1=glui_labels->add_panel_to_panel(ROLLOUT_general,"",GLUI_PANEL_NONE);

  if(nsliceinfo>0)CHECKBOX_labels_average = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Average"), &vis_slice_average, LABELS_label, LabelsCB);
  CHECKBOX_labels_axis = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Axis"), &visaxislabels, LABELS_label, LabelsCB);
  CHECKBOX_visColorbarVertical   = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Colorbar(vertical)"),   &visColorbarVertical,   LABELS_vcolorbar, LabelsCB);
  CHECKBOX_visColorbarHorizontal = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Colorbar(horizontal)"), &visColorbarHorizontal, LABELS_hcolorbar, LabelsCB);
  CHECKBOX_labels_framelabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Frame label"), &visFramelabel, LABELS_label, LabelsCB);
  CHECKBOX_labels_framerate = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Frame rate"), &visFramerate, LABELS_label, LabelsCB);
  CHECKBOX_labels_gridloc = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Grid location"), &visgridloc, LABELS_label, LabelsCB);
  CHECKBOX_labels_hrrlabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("HRR"), &visHRRlabel, HRR_label, LabelsCB);
  CHECKBOX_labels_firecutoff = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Fire cutoff"), &show_firecutoff, FIRECUTOFF_label, LabelsCB);
#ifdef pp_memstatus
  CHECKBOX_labels_availmemory = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Memory load"), &visAvailmemory, LABELS_label, LabelsCB);
#endif

  glui_labels->add_column_to_panel(PANEL_gen1, false);

  CHECKBOX_labels_meshlabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Mesh"), &visMeshlabel, LABELS_meshlabel, LabelsCB);
  CHECKBOX_labels_labels = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Text labels"), &visLabels, LABELS_label, LabelsCB);
  CHECKBOX_labels_ticks = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Ticks (FDS)"), &visFDSticks, LABELS_label, LabelsCB);
  CHECKBOX_visUSERticks2 = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Ticks (User)"), &visUSERticks, LABELS_usertick2, LabelsCB);
  CHECKBOX_labels_timelabel = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Time label"), &visTimelabel, LABELS_label, LabelsCB);
  CHECKBOX_labels_timebar = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Time bar"), &visTimebar, LABELS_label, LabelsCB);
  CHECKBOX_labels_title = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Title"), &visTitle, LABELS_label, LabelsCB);
  CHECKBOX_labels_chid = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("CHID"), &visCHID, LABELS_label, LabelsCB);
  glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Toggle dialogs"), &toggle_dialogs);
  CHECKBOX_labels_version = glui_labels->add_checkbox_to_panel(PANEL_gen1, _("Version info"), &gversion, LABELS_version, LabelsCB);

  if(ntickinfo>0){
    CHECKBOX_labels_ticks->enable();
  }
  else{
    CHECKBOX_labels_ticks->disable();
    visFDSticks=0;
    CHECKBOX_labels_ticks->set_int_val(visFDSticks);
  }

  PANEL_gen2=glui_labels->add_panel_to_panel(ROLLOUT_general,"",GLUI_PANEL_NONE);

  BUTTON_label_1=glui_labels->add_button_to_panel(PANEL_gen2,_("Show all"),LABELS_showall,LabelsCB);
  glui_labels->add_column_to_panel(PANEL_gen2,false);
  BUTTON_label_2=glui_labels->add_button_to_panel(PANEL_gen2,_("Hide all"),LABELS_hideall,LabelsCB);

  glui_labels->add_separator_to_panel(ROLLOUT_general);

  PANEL_gen3=glui_labels->add_panel_to_panel(ROLLOUT_general,"",GLUI_PANEL_NONE);

  PANEL_linewidth=glui_labels->add_panel_to_panel(PANEL_gen3,"line width");
  SPINNER_linewidth=glui_labels->add_spinner_to_panel(PANEL_linewidth,_("blockage"),GLUI_SPINNER_FLOAT,&linewidth);
  SPINNER_linewidth->set_float_limits(1.0,10.0,GLUI_LIMIT_CLAMP);
  SPINNER_gridlinewidth=glui_labels->add_spinner_to_panel(PANEL_linewidth,_("grid"),GLUI_SPINNER_FLOAT,&gridlinewidth);
  SPINNER_gridlinewidth->set_float_limits(1.0,10.0,GLUI_LIMIT_CLAMP);
  SPINNER_ticklinewidth = glui_labels->add_spinner_to_panel(PANEL_linewidth, _("tick"), GLUI_SPINNER_FLOAT, &ticklinewidth);
  SPINNER_ticklinewidth->set_float_limits(1.0, 10.0, GLUI_LIMIT_CLAMP);
  PANEL_offset=glui_labels->add_panel_to_panel(PANEL_gen3,"offset");
  SPINNER_ventoffset_factor=glui_labels->add_spinner_to_panel(PANEL_offset,_("vent"),GLUI_SPINNER_FLOAT,&ventoffset_factor,APPLY_VENTOFFSET,LabelsCB);
  SPINNER_ventoffset_factor->set_float_limits(-1.0,1.0,GLUI_LIMIT_CLAMP);
  SPINNER_sliceoffset_factor=glui_labels->add_spinner_to_panel(PANEL_offset,_("slice"),GLUI_SPINNER_FLOAT,&sliceoffset_factor);
  SPINNER_sliceoffset_factor->set_float_limits(-1.0,1.0,GLUI_LIMIT_CLAMP);

  if(nzoneinfo > 0){
    SPINNER_zone_hvac_diam = glui_labels->add_spinner_to_panel(PANEL_gen3, "HVAC (cfast)", GLUI_SPINNER_FLOAT, &zone_hvac_diam);
    SPINNER_zone_hvac_diam->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);
  }

  if(have_northangle==1){
    ROLLOUT_north = glui_labels->add_rollout_to_panel(PANEL_gen3,_("North direction"),false);
    INSERT_ROLLOUT(ROLLOUT_north, glui_labels);
    CHECKBOX_shownorth=glui_labels->add_checkbox_to_panel(ROLLOUT_north,_("show"),&vis_northangle,LABELS_shownorth,LabelsCB);
    SPINNER_northangle_position_x = glui_labels->add_spinner_to_panel(ROLLOUT_north, "x:", GLUI_SPINNER_FLOAT, northangle_position);
    SPINNER_northangle_position_y = glui_labels->add_spinner_to_panel(ROLLOUT_north, "y:", GLUI_SPINNER_FLOAT, northangle_position+1);
    SPINNER_northangle_position_z = glui_labels->add_spinner_to_panel(ROLLOUT_north, "z:", GLUI_SPINNER_FLOAT, northangle_position+2);
  }

  glui_labels->add_column_to_panel(PANEL_gen3,false);

  CHECKBOX_labels_flip = glui_labels->add_checkbox_to_panel(PANEL_gen3, _("Flip background"), &background_flip, LABELS_flip, LabelsCB);
  CHECKBOX_labels_hms = glui_labels->add_checkbox_to_panel(PANEL_gen3, _("hms time"), &vishmsTimelabel, LABELS_HMS, LabelsCB);
  PANEL_timebar_overlap = glui_labels->add_panel_to_panel(PANEL_gen3,_("Overlap timebar region"));
  RADIO_timebar_overlap=glui_labels->add_radiogroup_to_panel(PANEL_timebar_overlap,&timebar_overlap);
  glui_labels->add_radiobutton_to_group(RADIO_timebar_overlap,_("Always"));
  glui_labels->add_radiobutton_to_group(RADIO_timebar_overlap,_("Never"));
  glui_labels->add_radiobutton_to_group(RADIO_timebar_overlap,_("Only if timebar hidden"));

  CHECKBOX_label_1=glui_labels->add_checkbox_to_panel(PANEL_gen3,_("Fast blockage drawing"),&use_new_drawface,LABELS_drawface,LabelsCB);
  CHECKBOX_label_2=glui_labels->add_checkbox_to_panel(PANEL_gen3,_("Sort transparent faces"),&sort_transparent_faces,LABELS_drawface,LabelsCB);
  CHECKBOX_label_3=glui_labels->add_checkbox_to_panel(PANEL_gen3,_("Hide overlaps"),&hide_overlaps,LABELS_hide_overlaps,LabelsCB);

  if(nface_transparent>0){
    glui_labels->add_column_to_panel(PANEL_gen1,true);
    PANEL_transparency = glui_labels->add_panel_to_panel(PANEL_gen3,_("Geometry transparency"));
    CHECKBOX_labels_transparent_override=glui_labels->add_checkbox_to_panel(PANEL_transparency,_("Use level:"),&use_transparency_geom,LABELS_transparent,LabelsCB);
    SPINNER_labels_transparency_face=glui_labels->add_spinner_to_panel(PANEL_transparency,"",GLUI_SPINNER_FLOAT,&transparency_geom,LABELS_transparent,LabelsCB);
    SPINNER_labels_transparency_face->set_float_limits(0.0,1.0,GLUI_LIMIT_CLAMP);
    LabelsCB(LABELS_transparent);
  }

  // -------------- Data coloring -------------------

  ROLLOUT_coloring = glui_labels->add_rollout("Color",false,COLORING_ROLLOUT,DisplayRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_coloring, glui_labels);
  ADDPROCINFO(displayprocinfo, ndisplayprocinfo, ROLLOUT_coloring,COLORING_ROLLOUT, glui_labels);

  if(ncolorbars>0){
    int i;

    selectedcolorbar_index2=-1;
    LIST_colorbar2=glui_labels->add_listbox_to_panel(ROLLOUT_coloring,_("Colorbar:"),&selectedcolorbar_index2,COLORBAR_LIST2,SliceBoundCB);

    for(i=0;i<ncolorbars;i++){
      colorbardata *cbi;

      cbi = colorbarinfo + i;
      cbi->label_ptr=cbi->label;
      LIST_colorbar2->add_item(i,cbi->label_ptr);
    }
    LIST_colorbar2->set_int_val(colorbartype);
  }

  PANEL_cb11=glui_labels->add_panel_to_panel(ROLLOUT_coloring,"",GLUI_PANEL_NONE);

  PANEL_contours = glui_labels->add_panel_to_panel(PANEL_cb11,_("Colorbar type:"));
  RADIO2_plot3d_display=glui_labels->add_radiogroup_to_panel(PANEL_contours,&contour_type,UPDATEPLOT,Plot3DBoundCB);
  glui_labels->add_radiobutton_to_group(RADIO2_plot3d_display,_("Continuous"));
  glui_labels->add_radiobutton_to_group(RADIO2_plot3d_display,_("Stepped"));
  glui_labels->add_radiobutton_to_group(RADIO2_plot3d_display,_("Line"));
  CHECKBOX_colorbar_flip = glui_labels->add_checkbox_to_panel(PANEL_contours, _("flip"), &colorbar_flip, FLIP, LabelsCB);
  CHECKBOX_colorbar_autoflip = glui_labels->add_checkbox_to_panel(PANEL_contours, _("Auto flip"), &colorbar_autoflip, FLIP, LabelsCB);

  SPINNER_colorband=glui_labels->add_spinner_to_panel(PANEL_cb11,_("Selection width:"),GLUI_SPINNER_INT,&colorband,COLORBAND,SliceBoundCB);
  SPINNER_colorband->set_int_limits(1,10);

  glui_labels->add_column_to_panel(PANEL_cb11,false);

  CHECKBOX_labels_shade=glui_labels->add_checkbox_to_panel(PANEL_cb11,_("Black/White (geometry)"),&setbw,LABELS_shade,LabelsCB);
  CHECKBOX_labels_shadedata=glui_labels->add_checkbox_to_panel(PANEL_cb11,_("Black/White (data)"),&setbwdata,LABELS_shadedata,LabelsCB);
  CHECKBOX_transparentflag = glui_labels->add_checkbox_to_panel(PANEL_cb11, _("Transparent (data)"),
    &use_transparency_data, DATA_transparent, SliceBoundCB);
  SPINNER_labels_transparency_data = glui_labels->add_spinner_to_panel(PANEL_cb11, _("level"),
    GLUI_SPINNER_FLOAT, &transparent_level, TRANSPARENTLEVEL, SliceBoundCB);
  SPINNER_labels_transparency_data->set_w(0);
  SPINNER_labels_transparency_data->set_float_limits(0.0, 1.0, GLUI_LIMIT_CLAMP);
  CHECKBOX_axislabels_smooth = glui_labels->add_checkbox_to_panel(PANEL_cb11, _("Smooth colorbar labels"), &axislabels_smooth, COLORBAR_SMOOTH, SliceBoundCB);
  CHECKBOX_use_lighting = glui_labels->add_checkbox_to_panel(PANEL_cb11, _("Lighting"), &use_lighting, CB_USE_LIGHTING, LabelsCB);


  PANEL_extreme = glui_labels->add_panel_to_panel(ROLLOUT_coloring,"",GLUI_PANEL_NONE);

  if(use_data_extremes==1){
    ROLLOUT_extreme2 = glui_labels->add_rollout_to_panel(PANEL_extreme,"Highlight extreme data",false);
    INSERT_ROLLOUT(ROLLOUT_extreme2, glui_labels);
  }
  else{
    ROLLOUT_extreme2 = glui_labels->add_rollout_to_panel(PANEL_extreme,"Highlight extreme data (not supported with OpenGL 1.x)",false);
    INSERT_ROLLOUT(ROLLOUT_extreme2, glui_labels);
  }

  PANEL_extreme_min = glui_labels->add_panel_to_panel(ROLLOUT_extreme2,"",GLUI_PANEL_NONE);
  CHECKBOX_show_extreme_mindata=glui_labels->add_checkbox_to_panel(PANEL_extreme_min,_("Color below min"),&show_extreme_mindata,COLORBAR_EXTREME,ExtremeCB);

  SPINNER_down_red=  glui_labels->add_spinner_to_panel(PANEL_extreme_min,_("red"),  GLUI_SPINNER_INT,cb_down_rgb,COLORBAR_EXTREME_RGB,ExtremeCB);
  SPINNER_down_green=glui_labels->add_spinner_to_panel(PANEL_extreme_min,_("green"),GLUI_SPINNER_INT,cb_down_rgb+1,COLORBAR_EXTREME_RGB,ExtremeCB);
  SPINNER_down_blue= glui_labels->add_spinner_to_panel(PANEL_extreme_min,_("blue"), GLUI_SPINNER_INT,cb_down_rgb+2,COLORBAR_EXTREME_RGB,ExtremeCB);
  SPINNER_down_red->set_int_limits(0,255);
  SPINNER_down_green->set_int_limits(0,255);
  SPINNER_down_blue->set_int_limits(0,255);

  glui_labels->add_column_to_panel(ROLLOUT_extreme2,false);

  PANEL_extreme_max = glui_labels->add_panel_to_panel(ROLLOUT_extreme2,"",GLUI_PANEL_NONE);

  CHECKBOX_show_extreme_maxdata=glui_labels->add_checkbox_to_panel(PANEL_extreme_max,_("Color above max"),&show_extreme_maxdata,COLORBAR_EXTREME,ExtremeCB);

  SPINNER_up_red=  glui_labels->add_spinner_to_panel(PANEL_extreme_max,_("red"),  GLUI_SPINNER_INT,cb_up_rgb,COLORBAR_EXTREME_RGB,ExtremeCB);
  SPINNER_up_green=glui_labels->add_spinner_to_panel(PANEL_extreme_max,_("green"),GLUI_SPINNER_INT,cb_up_rgb+1,COLORBAR_EXTREME_RGB,ExtremeCB);
  SPINNER_up_blue= glui_labels->add_spinner_to_panel(PANEL_extreme_max,_("blue"), GLUI_SPINNER_INT,cb_up_rgb+2,COLORBAR_EXTREME_RGB,ExtremeCB);
  SPINNER_up_red->set_int_limits(0,255);
  SPINNER_up_green->set_int_limits(0,255);
  SPINNER_up_blue->set_int_limits(0,255);

  if(use_data_extremes == 0){
    ROLLOUT_extreme2->disable();
    CHECKBOX_show_extreme_maxdata->set_int_val(0);
    CHECKBOX_show_extreme_mindata->set_int_val(0);
    ExtremeCB(COLORBAR_EXTREME_RGB);
  }
  ColorbarGlobal2Local();

  ROLLOUT_split = glui_labels->add_rollout_to_panel(ROLLOUT_coloring, "Define split colorbar",false);
  INSERT_ROLLOUT(ROLLOUT_split, glui_labels);
  PANEL_split1H = glui_labels->add_panel_to_panel(ROLLOUT_split, "color below split");

  SPINNER_colorsplit[3] = glui_labels->add_spinner_to_panel(PANEL_split1H, _("red"), GLUI_SPINNER_INT, colorsplit+3, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[4] = glui_labels->add_spinner_to_panel(PANEL_split1H, _("green"), GLUI_SPINNER_INT, colorsplit + 4, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[5] = glui_labels->add_spinner_to_panel(PANEL_split1H, _("blue"), GLUI_SPINNER_INT, colorsplit + 5, SPLIT_COLORBAR, SplitCB);

  PANEL_split1L = glui_labels->add_panel_to_panel(ROLLOUT_split, "min color");

  SPINNER_colorsplit[0] = glui_labels->add_spinner_to_panel(PANEL_split1L,_("red"),  GLUI_SPINNER_INT,colorsplit, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[1] = glui_labels->add_spinner_to_panel(PANEL_split1L,_("green"),  GLUI_SPINNER_INT,colorsplit+1, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[2] = glui_labels->add_spinner_to_panel(PANEL_split1L,_("blue"),  GLUI_SPINNER_INT,colorsplit+2, SPLIT_COLORBAR, SplitCB);

  glui_labels->add_column_to_panel(ROLLOUT_split, false);

  PANEL_split2H = glui_labels->add_panel_to_panel(ROLLOUT_split, "max color");

  SPINNER_colorsplit[9] =  glui_labels->add_spinner_to_panel(PANEL_split2H,_("red"),  GLUI_SPINNER_INT,colorsplit+9, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[10] =  glui_labels->add_spinner_to_panel(PANEL_split2H,_("green"),  GLUI_SPINNER_INT,colorsplit+10, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[11] =  glui_labels->add_spinner_to_panel(PANEL_split2H,_("blue"),  GLUI_SPINNER_INT,colorsplit+11, SPLIT_COLORBAR, SplitCB);

  PANEL_split2L = glui_labels->add_panel_to_panel(ROLLOUT_split, "color above split");

  SPINNER_colorsplit[6] = glui_labels->add_spinner_to_panel(PANEL_split2L, _("red"), GLUI_SPINNER_INT, colorsplit+6, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[7] = glui_labels->add_spinner_to_panel(PANEL_split2L, _("green"), GLUI_SPINNER_INT, colorsplit + 7, SPLIT_COLORBAR, SplitCB);
  SPINNER_colorsplit[8] = glui_labels->add_spinner_to_panel(PANEL_split2L, _("blue"), GLUI_SPINNER_INT, colorsplit + 8, SPLIT_COLORBAR, SplitCB);

  glui_labels->add_column_to_panel(ROLLOUT_split, false);

  PANEL_split3 = glui_labels->add_panel_to_panel(ROLLOUT_split, "vals");

  glui_labels->add_spinner_to_panel(PANEL_split3,_("max"),  GLUI_SPINNER_FLOAT,splitvals+2, SPLIT_COLORBAR, SplitCB);
  glui_labels->add_spinner_to_panel(PANEL_split3,_("split"),  GLUI_SPINNER_FLOAT,splitvals+1, SPLIT_COLORBAR, SplitCB);
  glui_labels->add_spinner_to_panel(PANEL_split3,_("min"),  GLUI_SPINNER_FLOAT,splitvals, SPLIT_COLORBAR, SplitCB);

  {
    int i;

    for(i = 0; i < 12; i++){
      SPINNER_colorsplit[i]->set_int_limits(0, 255);
    }
  }
  SplitCB(SPLIT_COLORBAR);

  ROLLOUT_light2 = glui_labels->add_rollout("Light",false,LIGHT_ROLLOUT,DisplayRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_light2, glui_labels);
  ADDPROCINFO(displayprocinfo, ndisplayprocinfo, ROLLOUT_light2, LIGHT_ROLLOUT, glui_labels);

  {
    int i;

    for(i = 0; i<3;i++){
      glui_ambientlight[i] = CLAMP(255*ambientlight[i],0,255);
      glui_diffuselight[i] = CLAMP(255*diffuselight[i],0,255);
    }
  }
  glui_ambientgrey = 255*ambientgrey;
  glui_diffusegrey = 255*diffusegrey;
  glui_speculargrey = 255*speculargrey;

  PANEL_light = glui_labels->add_panel_to_panel(ROLLOUT_light2, "",false);
  PANEL_ambient = glui_labels->add_panel_to_panel(PANEL_light, "background/ambient");
  SPINNER_amb_red = glui_labels->add_spinner_to_panel(PANEL_ambient, _("red:"), GLUI_SPINNER_INT, glui_ambientlight,COLOR_AMB_RGB,ColorCB);
  SPINNER_amb_green = glui_labels->add_spinner_to_panel(PANEL_ambient, _("green:"), GLUI_SPINNER_INT, glui_ambientlight+1,COLOR_AMB_RGB,ColorCB);
  SPINNER_amb_blue = glui_labels->add_spinner_to_panel(PANEL_ambient, _("blue:"), GLUI_SPINNER_INT, glui_ambientlight+2,COLOR_AMB_RGB,ColorCB);
  SPINNER_amb_grey = glui_labels->add_spinner_to_panel(PANEL_ambient, _("grey:"), GLUI_SPINNER_INT, &glui_ambientgrey,COLOR_AMB_GREY,ColorCB);
  SPINNER_amb_red->set_int_limits(0,255);
  SPINNER_amb_green->set_int_limits(0, 255);
  SPINNER_amb_blue->set_int_limits(0, 255);
  SPINNER_amb_grey->set_int_limits(0, 255);
  ColorCB(COLOR_AMB_RGB);

  glui_labels->add_column_to_panel(PANEL_light,false);

  PANEL_diffuse = glui_labels->add_panel_to_panel(PANEL_light, "light/diffuse");
  SPINNER_diff_red = glui_labels->add_spinner_to_panel(PANEL_diffuse, _("red:"), GLUI_SPINNER_INT, glui_diffuselight,COLOR_DIFF_RGB,ColorCB);
  SPINNER_diff_green = glui_labels->add_spinner_to_panel(PANEL_diffuse, _("green:"), GLUI_SPINNER_INT, glui_diffuselight+1,COLOR_DIFF_RGB,ColorCB);
  SPINNER_diff_blue = glui_labels->add_spinner_to_panel(PANEL_diffuse, _("blue:"), GLUI_SPINNER_INT, glui_diffuselight+2,COLOR_DIFF_RGB,ColorCB);
  SPINNER_diff_grey = glui_labels->add_spinner_to_panel(PANEL_diffuse, _("grey:"), GLUI_SPINNER_INT, &glui_diffusegrey,COLOR_DIFF_GREY,ColorCB);
  SPINNER_diff_red->set_int_limits(0,255);
  SPINNER_diff_green->set_int_limits(0, 255);
  SPINNER_diff_blue->set_int_limits(0, 255);
  SPINNER_diff_grey->set_int_limits(0, 255);
  ColorCB(COLOR_DIFF_RGB);

#ifdef pp_SPECULAR
  PANEL_specular = glui_labels->add_panel_to_panel(ROLLOUT_light2, "specular");
  SPINNER_spec_red = glui_labels->add_spinner_to_panel(PANEL_specular, _("red:"), GLUI_SPINNER_INT, glui_specularlight, COLOR_SPEC_RGB, ColorCB);
  SPINNER_spec_green = glui_labels->add_spinner_to_panel(PANEL_specular, _("green:"), GLUI_SPINNER_INT, glui_specularlight+1, COLOR_SPEC_RGB, ColorCB);
  SPINNER_spec_blue = glui_labels->add_spinner_to_panel(PANEL_specular, _("blue:"), GLUI_SPINNER_INT, glui_specularlight+2, COLOR_SPEC_RGB, ColorCB);
  SPINNER_spec_grey = glui_labels->add_spinner_to_panel(PANEL_specular, _("grey:"), GLUI_SPINNER_INT, &glui_speculargrey, COLOR_SPEC_GREY, ColorCB);
  SPINNER_spec_red->set_int_limits(0, 255);
  SPINNER_spec_green->set_int_limits(0, 255);
  SPINNER_spec_blue->set_int_limits(0, 255);
  SPINNER_spec_grey->set_int_limits(0, 255);
#endif

  PANEL_positional = glui_labels->add_panel_to_panel(ROLLOUT_light2, "direction");
  glui_labels->add_checkbox_to_panel(PANEL_positional, "show directions", &drawlights);


  PANEL_positional2 = glui_labels->add_panel_to_panel(PANEL_positional, "",false);
  PANEL_position0 = glui_labels->add_panel_to_panel(PANEL_positional2, "light 1");
  glui_labels->add_checkbox_to_panel(PANEL_position0, _("show"), &use_light0);
  SPINNER_light_az0 = glui_labels->add_spinner_to_panel(PANEL_position0,   "azimuth:", GLUI_SPINNER_FLOAT, &glui_light_az0,LIGHT_POSITION,ColorCB);
  SPINNER_light_elev0 = glui_labels->add_spinner_to_panel(PANEL_position0, "elevation:", GLUI_SPINNER_FLOAT, &glui_light_elev0,LIGHT_POSITION,ColorCB);
  SPINNER_light_az0->set_float_limits(-360.0,360.0);
  SPINNER_light_elev0->set_float_limits(-90.0,90.0);

  glui_labels->add_column_to_panel(PANEL_positional2,false);

  PANEL_position1 = glui_labels->add_panel_to_panel(PANEL_positional2, "light 2");
  glui_labels->add_checkbox_to_panel(PANEL_position1, "show", &use_light1);
  SPINNER_light_az1 = glui_labels->add_spinner_to_panel(PANEL_position1,   "azimuth:", GLUI_SPINNER_FLOAT, &glui_light_az1,LIGHT_POSITION,ColorCB);
  SPINNER_light_elev1 = glui_labels->add_spinner_to_panel(PANEL_position1, "elevation:", GLUI_SPINNER_FLOAT, &glui_light_elev1,LIGHT_POSITION,ColorCB);
  SPINNER_light_az1->set_float_limits(-360.0,360.0);
  SPINNER_light_elev1->set_float_limits(-90.0,90.0);
  ColorCB(LIGHT_POSITION);

  // -------------- Fonts -------------------

  ROLLOUT_font = glui_labels->add_rollout("Fonts",false,FONTS_ROLLOUT,DisplayRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_font, glui_labels);
  ADDPROCINFO(displayprocinfo, ndisplayprocinfo, ROLLOUT_font, FONTS_ROLLOUT, glui_labels);

  RADIO_fontsize = glui_labels->add_radiogroup_to_panel(ROLLOUT_font,&fontindex,LABELS_fontsize,LabelsCB);
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
  GluiUpdateFontIndex();

  // -------------- User tick settings -------------------

  ROLLOUT_user_tick = glui_labels->add_rollout("User ticks",false,TICKS_ROLLOUT,DisplayRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_user_tick, glui_labels);
  ADDPROCINFO(displayprocinfo, ndisplayprocinfo, ROLLOUT_user_tick, TICKS_ROLLOUT, glui_labels);

  PANEL_tick1 = glui_labels->add_panel_to_panel(ROLLOUT_user_tick,_("Display"),true);
  PANEL_tick1a = glui_labels->add_panel_to_panel(PANEL_tick1,"",false);

  CHECKBOX_visUSERticks=glui_labels->add_checkbox_to_panel(PANEL_tick1a,_("Show user ticks"),&visUSERticks,LABELS_usertick,LabelsCB);
  glui_labels->add_column_to_panel(PANEL_tick1a,false);

  if(user_tick_direction>0.0){
    glui_tick_inside=0;
    glui_tick_outside=1;
  }
  else{
    glui_tick_inside=1;
    glui_tick_outside=0;
  }
  CHECKBOX_ticks_inside=glui_labels->add_checkbox_to_panel(PANEL_tick1a, "inside", &glui_tick_inside, LABELS_tick_inside, LabelsCB);
  glui_labels->add_column_to_panel(PANEL_tick1a, false);
  CHECKBOX_ticks_outside=glui_labels->add_checkbox_to_panel(PANEL_tick1a, "outside", &glui_tick_outside, LABELS_tick_outside, LabelsCB);
  LabelsCB(LABELS_tick_inside);
  LabelsCB(LABELS_tick_outside);

  PANEL_tick1b = glui_labels->add_panel_to_panel(PANEL_tick1,"",false);
  CHECKBOX_tick_auto=glui_labels->add_checkbox_to_panel(PANEL_tick1b,_("Auto place"),&auto_user_tick_placement,LABELS_ticks,LabelsCB);
  glui_labels->add_column_to_panel(PANEL_tick1b,false);
  CHECKBOX_user_ticks_show_x=glui_labels->add_checkbox_to_panel(PANEL_tick1b,"x",&user_tick_show_x);
  glui_labels->add_column_to_panel(PANEL_tick1b,false);
  CHECKBOX_user_ticks_show_y=glui_labels->add_checkbox_to_panel(PANEL_tick1b,"y",&user_tick_show_y);
  glui_labels->add_column_to_panel(PANEL_tick1b,false);
  CHECKBOX_user_ticks_show_z=glui_labels->add_checkbox_to_panel(PANEL_tick1b,"z",&user_tick_show_z);
  LabelsCB(LABELS_ticks);
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

  // -------------- User labels -------------------

  gl=&LABEL_local;
  ROLLOUT_user_labels = glui_labels->add_rollout("Labels + Ticks",false,LABELS_ROLLOUT,DisplayRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_user_labels, glui_labels);
  ADDPROCINFO(displayprocinfo, ndisplayprocinfo, ROLLOUT_user_labels, LABELS_ROLLOUT, glui_labels);

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

    for(thislabel=label_first_ptr->next;thislabel->next!=NULL;thislabel=thislabel->next){
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
  INSERT_ROLLOUT(ROLLOUT_LB_tick0, glui_labels);
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

  // --------------

  PANEL_label2 = glui_labels->add_panel("",false);
  glui_labels->add_column_to_panel(PANEL_label2,false);

  BUTTON_label_3=glui_labels->add_button_to_panel(PANEL_label2,_("Save settings"),SAVE_SETTINGS,LabelsCB);
  glui_labels->add_column_to_panel(PANEL_label2,false);

  BUTTON_label_4=glui_labels->add_button_to_panel(PANEL_label2,_("Close"),LABELS_close,LabelsCB);

  glui_labels->set_main_gfx_window( main_window );
}

/* ------------------ HideGluiDisplay ------------------------ */

extern "C" void HideGluiDisplay(void){
  CloseRollouts(glui_labels);
}

/* ------------------ ShowGluiDisplay ------------------------ */

extern "C" void ShowGluiDisplay(int menu_id){
  if(glui_labels!=NULL)glui_labels->show();
  switch(menu_id){
  case DIALOG_DISPLAY:
    DisplayRolloutCB(GENERAL_ROLLOUT);
    break;
  case DIALOG_FONTS:
    DisplayRolloutCB(FONTS_ROLLOUT);
    break;
  case DIALOG_TICKS:
    DisplayRolloutCB(TICKS_ROLLOUT);
    break;
  case DIALOG_COLORING:
    DisplayRolloutCB(COLORING_ROLLOUT);
    break;
  case DIALOG_LABELS:
    DisplayRolloutCB(LABELS_ROLLOUT);
    break;
  default:
    ASSERT(0);
    break;
  }
}

/* ------------------ LabelsCB ------------------------ */

extern "C" void LabelsCB(int var){
  updatemenu=1;
  switch(var){
  case LABELS_vcolorbar:
    if(visColorbarVertical==1){
      visColorbarHorizontal=0;
      CHECKBOX_visColorbarHorizontal->set_int_val(visColorbarHorizontal);
    }
 // toggle_colorbar   state
 //    0              hidden
 //    1              vertical
 //    2->max         horizontal

    if(visColorbarVertical == 1 && visColorbarHorizontal == 0){
      toggle_colorbar = 1;
    }
    else if(visColorbarVertical == 0 && visColorbarHorizontal == 1){
      toggle_colorbar = 2;
    }
    else{
      toggle_colorbar = 0;
    }
    break;
  case LABELS_hcolorbar:
    if(visColorbarHorizontal==1){
      visColorbarVertical = 0;
      CHECKBOX_visColorbarVertical->set_int_val(visColorbarVertical);
    }
    if(visColorbarVertical == 1 && visColorbarHorizontal == 0){
      toggle_colorbar = 1;
    }
    else if(visColorbarVertical == 0 && visColorbarHorizontal == 1){
      toggle_colorbar = 2;
    }
    else{
      toggle_colorbar = 0;
    }
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
    updatefaces=1;
    break;
  case FLIP:
      colorbar_flip = 1 - colorbar_flip;
      ColorbarMenu(COLORBAR_FLIP);
      break;
  case LABELS_hide_overlaps:
    updatefacelists=1;
    updatehiddenfaces=1;
    UpdateHiddenFaces();
    glutPostRedisplay();
    break;
#ifdef pp_BETA
  case LABELS_drawface:
    /*
    if(use_new_drawface==1){
      CHECKBOX_cullgeom->enable();
      if(cullgeom==1){
        SPINNER_cullgeom_portsize->enable();
      }
      else{
        SPINNER_cullgeom_portsize->disable();
      }
    }
    else{
      CHECKBOX_cullgeom->disable();
      SPINNER_cullgeom_portsize->disable();
    }
    update_initcullgeom=1;
    SetCullVis();
    */
    updatefacelists=1;
    break;
#endif

  case CB_USE_LIGHTING:
  case LABELS_shownorth:
  case LABELS_version:
  case LABELS_meshlabel:
  case LABELS_label:
  case LABELS_HMS:
  case LABELS_transparent:
  case FIRECUTOFF_label:
    break;
  case LABELS_usertick:
    CHECKBOX_visUSERticks2->set_int_val(visUSERticks);
    break;
  case LABELS_usertick2:
    CHECKBOX_visUSERticks->set_int_val(visUSERticks);
    if(visUSERticks==1)ROLLOUT_user_tick->open();
    break;
  case SAVE_SETTINGS:
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
    if(setbwdata==1){
      colorbartype_save=colorbartype;
      ColorbarMenu(bw_colorbar_index);
    }
    else{
      ColorbarMenu(colorbartype_save);
    }
    break;
  case LABELS_close:
    HideGluiDisplay();
    break;
  case LABELS_fontsize:
    FontMenu(fontindex);
    break;
  case FRAME_label:
    visFramelabel=1-visFramelabel;
    LabelMenu(MENU_LABEL_framerate);
    break;
  case HRR_label:
    visHRRlabel=1-visHRRlabel;
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
    ASSERT(FFALSE);
  }
}

/* ------------------ SetLabelControls ------------------------ */

  extern "C" void SetLabelControls(){

  if(CHECKBOX_LB_visLabels!=NULL)CHECKBOX_LB_visLabels->set_int_val(visLabels);
  if(CHECKBOX_visUSERticks!=NULL)CHECKBOX_visUSERticks->set_int_val(visUSERticks);
  if(CHECKBOX_labels_hrrlabel!=NULL)CHECKBOX_labels_hrrlabel->set_int_val(visHRRlabel);
  if(CHECKBOX_labels_firecutoff!=NULL)CHECKBOX_labels_firecutoff->set_int_val(show_firecutoff);
  if(CHECKBOX_labels_title!=NULL)CHECKBOX_labels_title->set_int_val(visTitle);
  if(CHECKBOX_labels_chid!=NULL)CHECKBOX_labels_chid->set_int_val(visCHID);
  if(CHECKBOX_visColorbarVertical!=NULL)CHECKBOX_visColorbarVertical->set_int_val(visColorbarVertical);
  if(CHECKBOX_labels_timebar!=NULL)CHECKBOX_labels_timebar->set_int_val(visTimebar);
  if(CHECKBOX_labels_timelabel!=NULL)CHECKBOX_labels_timelabel->set_int_val(visTimelabel);
  if(CHECKBOX_labels_framelabel!=NULL)CHECKBOX_labels_framelabel->set_int_val(visFramelabel);
  if(CHECKBOX_labels_ticks!=NULL)CHECKBOX_labels_ticks->set_int_val(visFDSticks);
  if(CHECKBOX_labels_axis!=NULL)CHECKBOX_labels_axis->set_int_val(visaxislabels);
  if(CHECKBOX_labels_framerate!=NULL)CHECKBOX_labels_framerate->set_int_val(visFramerate);
  if(CHECKBOX_labels_average!=NULL)CHECKBOX_labels_average->set_int_val(vis_slice_average);
#ifdef pp_memstatus
  if(CHECKBOX_labels_availmemory!=NULL)CHECKBOX_labels_availmemory->set_int_val(visAvailmemory);
#endif
  if(CHECKBOX_labels_labels!=NULL)CHECKBOX_labels_labels->set_int_val(visLabels);

  if(CHECKBOX_labels_flip!=NULL)CHECKBOX_labels_flip->set_int_val(background_flip);
  if(CHECKBOX_labels_shade!=NULL)CHECKBOX_labels_shade->set_int_val(setbw);
  if(CHECKBOX_labels_shadedata != NULL)CHECKBOX_labels_shadedata->set_int_val(setbwdata);
  if(RADIO_fontsize != NULL)RADIO_fontsize->set_int_val(fontindex);
  if(CHECKBOX_labels_hms!=NULL)CHECKBOX_labels_hms->set_int_val(vishmsTimelabel);
  if(CHECKBOX_labels_gridloc!=NULL)CHECKBOX_labels_gridloc->set_int_val(visgridloc);
  if(CHECKBOX_labels_version!=NULL)CHECKBOX_labels_version->set_int_val(gversion);
  if(CHECKBOX_labels_meshlabel!=NULL)CHECKBOX_labels_meshlabel->set_int_val(visMeshlabel);
  if(CHECKBOX_visUSERticks2!=NULL)CHECKBOX_visUSERticks2->set_int_val(visUSERticks);
}


/* ------------------ UpdateColorbarFlip ------------------------ */

extern "C" void UpdateColorbarFlip(void){
  CHECKBOX_colorbar_flip->set_int_val(colorbar_flip);
  CHECKBOX_colorbar_autoflip->set_int_val(colorbar_autoflip);
}

/* ------------------ UpdateColorbarList2 ------------------------ */

extern "C" void UpdateColorbarList2(void){
  if(LIST_colorbar2!=NULL)LIST_colorbar2->set_int_val(selectedcolorbar_index2);
}

/* ------------------ AddColorbarList2 ------------------------ */

extern "C" void AddColorbarList2(int index, char *label){
  LIST_colorbar2->add_item(index,label);
}

/* ------------------ SetColorbarListIndex ------------------------ */

extern "C" void SetColorbarListIndex(int val){
  if(LIST_colorbar2!=NULL)LIST_colorbar2->set_int_val(val);
}

/* ------------------ GetColorbarListIndex ------------------------ */

extern "C" int GetColorbarListIndex(void){
  return LIST_colorbar2->get_int_val();
}

/* ------------------ UpdateAxisLabelsSmooth ------------------------ */

extern "C" void UpdateAxisLabelsSmooth(void){
  CHECKBOX_axislabels_smooth->set_int_val(axislabels_smooth);
}

/* ------------------ UpdateTransparency ------------------------ */

extern "C" void UpdateTransparency(void){
  if(CHECKBOX_transparentflag!=NULL)CHECKBOX_transparentflag->set_int_val(use_transparency_data);
}

/* ------------------ ExtremeCB ------------------------ */

extern "C" void ExtremeCB(int var){
  colorbardata *cbi;
  unsigned char *rgb_nodes;
  int i;

  switch(var){
    case COLORBAR_EXTREME:
      if(show_extreme_mindata==1){
        if(SPINNER_down_red!=NULL)SPINNER_down_red->enable();
        if(SPINNER_down_green!=NULL)SPINNER_down_green->enable();
        if(SPINNER_down_blue!=NULL)SPINNER_down_blue->enable();
      }
      else{
        if(SPINNER_down_red!=NULL)SPINNER_down_red->disable();
        if(SPINNER_down_green!=NULL)SPINNER_down_green->disable();
        if(SPINNER_down_blue!=NULL)SPINNER_down_blue->disable();
      }
      if(show_extreme_maxdata==1){
        if(SPINNER_up_red!=NULL)SPINNER_up_red->enable();
        if(SPINNER_up_green!=NULL)SPINNER_up_green->enable();
        if(SPINNER_up_blue!=NULL)SPINNER_up_blue->enable();
      }
      else{
        if(SPINNER_up_red!=NULL)SPINNER_up_red->disable();
        if(SPINNER_up_green!=NULL)SPINNER_up_green->disable();
        if(SPINNER_up_blue!=NULL)SPINNER_up_blue->disable();
      }
      if(colorbartype<0||colorbartype>=ncolorbars)return;
      cbi = colorbarinfo + colorbartype;
      RemapColorbar(cbi);
      UpdateRGBColors(COLORBAR_INDEX_NONE);
      updatemenu=1;
      break;
    case COLORBAR_EXTREME_RGB:
      if(colorbartype<0||colorbartype>=ncolorbars)return;
      cbi = colorbarinfo + colorbartype;

      rgb_nodes=rgb_above_max;
      for(i=0;i<3;i++){
        rgb_nodes[i]=cb_up_rgb[i];
      }
      rgb_nodes=rgb_below_min;
      for(i=0;i<3;i++){
        rgb_nodes[i]=cb_down_rgb[i];
      }
      RemapColorbar(cbi);
      UpdateRGBColors(COLORBAR_INDEX_NONE);
      break;
    default:
      break;
  }
}

/* ------------------ UpdateExtremeVals ------------------------ */

extern "C" void UpdateExtremeVals(void){
  unsigned char *rgb_local;

  rgb_local = rgb_below_min;
  if(SPINNER_down_red!=NULL)SPINNER_down_red->set_int_val(  (int)(rgb_local[0]));
  if(SPINNER_down_green!=NULL)SPINNER_down_green->set_int_val(  (int)(rgb_local[1]));
  if(SPINNER_down_blue!=NULL)SPINNER_down_blue->set_int_val(  (int)(rgb_local[2]));

  rgb_local = rgb_above_max;
  if(SPINNER_up_red!=NULL)SPINNER_up_red->set_int_val(  (int)(rgb_local[0]));
  if(SPINNER_up_green!=NULL)SPINNER_up_green->set_int_val(  (int)(rgb_local[1]));
  if(SPINNER_up_blue!=NULL)SPINNER_up_blue->set_int_val(  (int)(rgb_local[2]));
}

/* ------------------ UpdateExtreme ------------------------ */

extern "C" void UpdateExtreme(void){
  if(CHECKBOX_show_extreme_mindata!=NULL){
    CHECKBOX_show_extreme_mindata->set_int_val(show_extreme_mindata);
  }
  if(CHECKBOX_show_extreme_maxdata!=NULL){
    CHECKBOX_show_extreme_maxdata->set_int_val(show_extreme_maxdata);
  }
  ExtremeCB(COLORBAR_EXTREME);
}
