#define CPP
#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_tour.h"
#include "glui_motion.h"
#include "glui_smoke.h"
#include "readtour.h"

static int tour_hide=0;
static char tour_label[sizeof(GLUI_String)];

GLUI *glui_tour=NULL;

GLUI_Rollout *ROLLOUT_keyframe = NULL;
GLUI_Rollout *ROLLOUT_circular = NULL;

GLUI_Panel *PANEL_misc = NULL;
GLUI_Panel *PANEL_node = NULL;
GLUI_Panel *PANEL_tour = NULL;
GLUI_Panel *PANEL_settingskeyframe=NULL;
GLUI_Panel *PANEL_path=NULL;
GLUI_Panel *PANEL_tour1=NULL;
GLUI_Panel *PANEL_tour3=NULL;
GLUI_Panel *PANEL_close_tour=NULL;
GLUI_Panel *PANEL_pos=NULL;
GLUI_Panel *PANEL_tourposition=NULL;
GLUI_Panel *PANEL_tournavigate=NULL;
GLUI_Panel *PANEL_tourview=NULL;
GLUI_Panel *PANEL_tour_circular_center;
GLUI_Panel *PANEL_tour_circular_view;
GLUI_Panel *PANEL_tour4;
GLUI_Panel *PANEL_tour5;
GLUI_Panel *PANEL_tour6;
GLUI_Panel *PANEL_tour7;


GLUI_Checkbox *CHECKBOX_view1 = NULL;
GLUI_Checkbox *CHECKBOX_view2 = NULL;
#ifdef pp_TOUR_SNAP
GLUI_Checkbox *CHECKBOX_tour_snap = NULL;
#endif
GLUI_Checkbox *CHECKBOX_tour_constant_velocity = NULL;
GLUI_Checkbox *CHECKBOX_set_tour_time = NULL;
GLUI_Checkbox *CHECKBOX_showtourroute1 = NULL;
GLUI_Checkbox *CHECKBOX_showtourroute2 = NULL;
#ifdef _DEBUG
GLUI_Checkbox *CHECKBOX_showdebugtour1= NULL;
GLUI_Checkbox *CHECKBOX_showdebugtour2= NULL;
#endif
GLUI_Checkbox *CHECKBOX_showintermediate = NULL;
GLUI_Checkbox *CHECKBOX_tourhide=NULL;

GLUI_Spinner *SPINNER_tour_time = NULL;
GLUI_Spinner *SPINNER_tour_pause_time = NULL;
GLUI_Spinner *SPINNER_x = NULL;
GLUI_Spinner *SPINNER_y = NULL;
GLUI_Spinner *SPINNER_z = NULL;
GLUI_Spinner *SPINNER_viewx = NULL;
GLUI_Spinner *SPINNER_viewy = NULL;
GLUI_Spinner *SPINNER_viewz = NULL;

GLUI_Spinner *SPINNER_tour_circular_view[3];
GLUI_Spinner *SPINNER_tour_circular_center[3];
GLUI_Spinner *SPINNER_tour_circular_radius = NULL;
GLUI_Spinner *SPINNER_tour_circular_angle0 = NULL;

GLUI_Button *BUTTON_next_tour=NULL;
GLUI_Button *BUTTON_prev_tour=NULL;
GLUI_Button *BUTTON_delete_tour = NULL;

GLUI_EditText *EDIT_label=NULL;

GLUI_Listbox *LISTBOX_tour=NULL;
GLUI_Listbox *LISTBOX_avatar=NULL;

procdata toursprocinfo[2];
int ntoursprocinfo = 0;

/* ------------------ ToursRolloutCB ------------------------ */

void ToursRolloutCB(int var){
  GLUIToggleRollout(toursprocinfo, ntoursprocinfo, var);
  if(var == MODIFY_TOURS_ROLLOUT){
    if(ROLLOUT_circular->is_open == 1){
      selectedtour_index = 0;
      TourCB(TOUR_LIST);
      LISTBOX_tour->set_int_val(selectedtour_index);
    }
  }
}

/* ------------------ SetKeyframeViews ------------------------ */

void SetKeyFrameViews(float *view){
  keyframe *this_key, *first_key, *last_key;

  if(selected_tour==NULL)return;

  first_key = selected_tour->first_frame.next;
  last_key = selected_tour->last_frame.prev;
  for(this_key = first_key; this_key->next!=NULL; this_key = this_key->next){
    keyframe *next_key;

    next_key = this_key->next;
    if(view==NULL){
      if(this_key==last_key){
        float view_temp[3];
        float *this_xyz, *prev_xyz;

        this_xyz = this_key->xyz_smv;
        prev_xyz = this_key->prev->xyz_smv;

        view_temp[0] = this_xyz[0]+(this_xyz[0]-prev_xyz[0]);
        view_temp[1] = this_xyz[1]+(this_xyz[1]-prev_xyz[1]);
        view_temp[2] = this_xyz[2]+(this_xyz[2]-prev_xyz[2]);
        memcpy(this_key->view_smv, view_temp, 3*sizeof(float));
      }
      else{
        memcpy(this_key->view_smv, next_key->xyz_smv, 3*sizeof(float));
      }
    }
    else{
      memcpy(this_key->view_smv, view, 3*sizeof(float));
    }
  }
}

/* ------------------ GLUIUpdateTourState ------------------------ */

extern "C" void GLUIUpdateTourState(void){
  TourCB(SHOWTOURROUTE);
  TourCB(VIEWTOURFROMPATH);
}

/* ------------------ AddDeleteKeyframe ------------------------ */

void AddDeleteKeyframe(int flag){
  if(flag==ADD_KEYFRAME)TourCB(KEYFRAME_INSERT);
  if(flag==DELETE_KEYFRAME)TourCB(KEYFRAME_DELETE);
}

/* ------------------ GLUIUpdateTourParms ------------------------ */

extern "C" void GLUIUpdateTourParms(void){
  TourCB(KEYFRAME_tXYZ);
}

/* ------------------ GLUIAddNewTour ------------------------ */

extern "C" void GLUIAddNewTour(void){
  TourCB(TOUR_INSERT_NEW);
}

/* ------------------ GLUITourSetup ------------------------ */

extern "C" void GLUITourSetup(int main_window){

  int i;

  if(glui_tour!=NULL){
    glui_tour->close();
    glui_tour=NULL;
  }
  glui_tour = GLUI_Master.create_glui(_("Tours"),0,dialogX0,dialogY0);
  glui_tour->hide();

  ROLLOUT_circular = glui_tour->add_rollout(_("Modify circular tour"),false, MODIFY_TOURS_ROLLOUT, ToursRolloutCB);
  ADDPROCINFO(toursprocinfo, ntoursprocinfo, ROLLOUT_circular, MODIFY_TOURS_ROLLOUT, glui_tour);

  CHECKBOX_showtourroute2 = glui_tour->add_checkbox_to_panel(ROLLOUT_circular, _("Show tour"), &edittour, SHOWTOURROUTE2, TourCB);
#ifdef _DEBUG
  CHECKBOX_showdebugtour2  = glui_tour->add_checkbox_to_panel(ROLLOUT_circular, _("Show debug info"), &showdebugtour, SHOWTOURROUTE2, TourCB);
#endif
  CHECKBOX_view2 = glui_tour->add_checkbox_to_panel(ROLLOUT_circular, _("View from tour path"), &viewtourfrompath, VIEWTOURFROMPATH2, TourCB);

  SPINNER_tour_circular_radius = glui_tour->add_spinner_to_panel(ROLLOUT_circular, "radius", GLUI_SPINNER_FLOAT, &tour_circular_radius,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_angle0 = glui_tour->add_spinner_to_panel(ROLLOUT_circular, "initial angle", GLUI_SPINNER_FLOAT, &tour_circular_angle0, TOUR_CIRCULAR_UPDATE, TourCB);
  glui_tour->add_spinner_to_panel(ROLLOUT_circular, "speedup factor", GLUI_SPINNER_FLOAT, &tour_speedup_factor, TOUR_CIRCULAR_UPDATE, TourCB);

  PANEL_tour_circular_center = glui_tour->add_panel_to_panel(ROLLOUT_circular,_("center"),true);
  SPINNER_tour_circular_center[0]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"x",GLUI_SPINNER_FLOAT,tour_circular_center,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_center[1]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"y",GLUI_SPINNER_FLOAT,tour_circular_center+1,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_center[2]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"z",GLUI_SPINNER_FLOAT,tour_circular_center+2,TOUR_CIRCULAR_UPDATE,TourCB);

  PANEL_tour_circular_view = glui_tour->add_panel_to_panel(ROLLOUT_circular,_("Target"),true);
  SPINNER_tour_circular_view[0]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"x",GLUI_SPINNER_FLOAT,tour_circular_view,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_view[1]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"y",GLUI_SPINNER_FLOAT,tour_circular_view+1,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_view[2]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"z",GLUI_SPINNER_FLOAT,tour_circular_view+2,TOUR_CIRCULAR_UPDATE,TourCB);

  ROLLOUT_keyframe = glui_tour->add_rollout("Modify general tour",true,KEYFRAME_TOURS_ROLLOUT, ToursRolloutCB);
  ADDPROCINFO(toursprocinfo, ntoursprocinfo, ROLLOUT_keyframe, KEYFRAME_TOURS_ROLLOUT, glui_tour);

  PANEL_tour = glui_tour->add_panel_to_panel(ROLLOUT_keyframe,"Tour", true);

  PANEL_tour1 = glui_tour->add_panel_to_panel(PANEL_tour, "", GLUI_PANEL_NONE);

  BUTTON_prev_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Previous"), TOUR_PREVIOUS, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("Copy"), TOUR_INSERT_COPY, TourCB);
  BUTTON_delete_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Delete"), TOUR_DELETE, TourCB);
  glui_tour->add_column_to_panel(PANEL_tour1, false);
  BUTTON_next_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Next"), TOUR_NEXT, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("New"), TOUR_INSERT_NEW, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("Reverse"), TOUR_REVERSE, TourCB);
  if(global_scase.tourcoll.ntourinfo > 0){
    selectedtour_index = TOURINDEX_MANUAL;
    selectedtour_index_old = TOURINDEX_MANUAL;
    LISTBOX_tour = glui_tour->add_listbox_to_panel(PANEL_tour, "Select: ", &selectedtour_index, TOUR_LIST, TourCB);

    LISTBOX_tour->add_item(TOURINDEX_MANUAL, "Manual");
    LISTBOX_tour->add_item(-999, "-");
    for(i = 0;i < global_scase.tourcoll.ntourinfo;i++){
      tourdata *touri;

      touri = global_scase.tourcoll.tourinfo + i;
      LISTBOX_tour->add_item(i, touri->label);
    }
    LISTBOX_tour->set_int_val(selectedtour_index);
    glui_tour->add_column_to_panel(PANEL_tour1, false);
  }
  EDIT_label = glui_tour->add_edittext_to_panel(PANEL_tour, "Label:", GLUI_EDITTEXT_TEXT, tour_label, TOUR_LABEL, TourCB);
  EDIT_label->set_w(200);
  glui_tour->add_button_to_panel(PANEL_tour, _("Update label"), TOUR_UPDATELABEL, TourCB);

  PANEL_tour6 = glui_tour->add_panel_to_panel(PANEL_tour, "", GLUI_PANEL_NONE);
  PANEL_tour5 = glui_tour->add_panel_to_panel(PANEL_tour6, "");
  CHECKBOX_showtourroute1        = glui_tour->add_checkbox_to_panel(PANEL_tour5, _("Show tour"),
    &edittour, SHOWTOURROUTE1, TourCB);
#ifdef _DEBUG
  CHECKBOX_showdebugtour1 = glui_tour->add_checkbox_to_panel(PANEL_tour5, _("Show debug info"), &showdebugtour, SHOWTOURROUTE1, TourCB);
#endif
  CHECKBOX_view1                 = glui_tour->add_checkbox_to_panel(PANEL_tour5, _("View from tour"),
    &viewtourfrompath, VIEWTOURFROMPATH1, TourCB);
#ifdef pp_TOUR_SNAP
  CHECKBOX_tour_snap             = glui_tour->add_checkbox_to_panel(PANEL_tour5, _("View from current tour position"),
    &tour_snap, TOUR_SNAP, TourCB);
#endif
  CHECKBOX_tour_constant_velocity = glui_tour->add_checkbox_to_panel(PANEL_tour5, _("Constant velocity"),
    &tour_constant_velocity, TOUR_CONSTANT_VELOCITY, TourCB);

  
  glui_tour->add_column_to_panel(PANEL_tour6, false);
  PANEL_tour7 = glui_tour->add_panel_to_panel(PANEL_tour6, "");

  if(global_scase.objectscoll.navatar_types > 0){
    glui_tour->add_checkbox_to_panel(PANEL_tour7, _("Show avatar"), &show_avatar);
    LISTBOX_avatar = glui_tour->add_listbox_to_panel(PANEL_tour7, _("Avatar:"), &glui_avatar_index, TOUR_AVATAR, TourCB);
    for(i = 0;i < global_scase.objectscoll.navatar_types;i++){
      LISTBOX_avatar->add_item(i, global_scase.objectscoll.avatar_types[i]->label);
    }
    if(tourlocus_type == 0){
      glui_avatar_index = -1;
    }
    else if(tourlocus_type == 1){
      glui_avatar_index = -2;
    }
    else{
      glui_avatar_index = global_scase.objectscoll.iavatar_types;
    }
    LISTBOX_avatar->set_int_val(glui_avatar_index);
  }
  CHECKBOX_showintermediate = glui_tour->add_checkbox_to_panel(PANEL_tour7, _("Show points"), &show_path_knots);
  glui_tour->add_spinner_to_panel(PANEL_tour7, _("points"), GLUI_SPINNER_INT, &global_scase.tourcoll.tour_ntimes, VIEW_times, TourCB);

  PANEL_node = glui_tour->add_panel_to_panel(ROLLOUT_keyframe, "", GLUI_PANEL_NONE);

  PANEL_tour4 = glui_tour->add_panel_to_panel(PANEL_node, _(""), GLUI_PANEL_NONE);
  PANEL_tourposition = glui_tour->add_panel_to_panel(PANEL_tour4, _("Node"));

  CHECKBOX_set_tour_time = glui_tour->add_checkbox_to_panel(PANEL_tourposition, _("Set time"),
    &glui_set_tour_time, KEYFRAME_SET_TOUR_TIME, TourCB);
  SPINNER_tour_time = glui_tour->add_spinner_to_panel(PANEL_tourposition, "t:", GLUI_SPINNER_FLOAT, &glui_tour_time, KEYFRAME_t, TourCB);
  SPINNER_tour_pause_time = glui_tour->add_spinner_to_panel(PANEL_tourposition, "pause:", GLUI_SPINNER_FLOAT, &glui_tour_pause_time, KEYFRAME_tXYZ, TourCB);
  SPINNER_x=glui_tour->add_spinner_to_panel(PANEL_tourposition,"x:",GLUI_SPINNER_FLOAT,glui_tour_xyz,  KEYFRAME_tXYZ,TourCB);
  SPINNER_y=glui_tour->add_spinner_to_panel(PANEL_tourposition,"y:",GLUI_SPINNER_FLOAT,glui_tour_xyz+1,KEYFRAME_tXYZ,TourCB);
  SPINNER_z=glui_tour->add_spinner_to_panel(PANEL_tourposition,"z:",GLUI_SPINNER_FLOAT,glui_tour_xyz+2,KEYFRAME_tXYZ,TourCB);
  TourCB(TOUR_CONSTANT_VELOCITY);
  TourCB(KEYFRAME_tXYZ);
  glui_tour->add_column_to_panel(PANEL_tour4, false);
  PANEL_tourview = glui_tour->add_panel_to_panel(PANEL_tour4, _("View direction (target)"));
  SPINNER_viewx=glui_tour->add_spinner_to_panel(PANEL_tourview,"x",GLUI_SPINNER_FLOAT,glui_tour_view,  KEYFRAME_viewXYZ,TourCB);
  SPINNER_viewy=glui_tour->add_spinner_to_panel(PANEL_tourview,"y",GLUI_SPINNER_FLOAT,glui_tour_view+1,KEYFRAME_viewXYZ,TourCB);
  SPINNER_viewz=glui_tour->add_spinner_to_panel(PANEL_tourview,"z",GLUI_SPINNER_FLOAT,glui_tour_view+2,KEYFRAME_viewXYZ,TourCB);
  glui_tour->add_button_to_panel(PANEL_tourview, _("Set target(all nodes)"),     VIEW_ALL_NODES, TourCB);
  glui_tour->add_button_to_panel(PANEL_tourview, _("Set targets(next node)"), VIEW_NEXT_NODE, TourCB);

  PANEL_tournavigate = glui_tour->add_panel_to_panel(PANEL_node, "", GLUI_PANEL_NONE);

  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Previous"), KEYFRAME_PREVIOUS, TourCB);
  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Delete"), KEYFRAME_DELETE, TourCB);

  glui_tour->add_column_to_panel(PANEL_tournavigate, false);

  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Next"), KEYFRAME_NEXT, TourCB);
  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Insert after"), KEYFRAME_INSERT, TourCB);

  PANEL_close_tour = glui_tour->add_panel("",false);
  glui_tour->add_button_to_panel(PANEL_close_tour,_("Save"),SAVE_SETTINGS_TOUR,TourCB);
  glui_tour->add_column_to_panel(PANEL_close_tour,false);
#ifdef pp_CLOSEOFF
  GLUI_Button *BUTTON_button2 = glui_tour->add_button_to_panel(PANEL_close_tour,"Close",TOUR_CLOSE,TourCB);
  BUTTON_button2->disable();
#else
  glui_tour->add_button_to_panel(PANEL_close_tour,"Close",TOUR_CLOSE,TourCB);
#endif
  ROLLOUT_keyframe->close();

  glui_tour->set_main_gfx_window( main_window );

  TourCB(VIEW1);

  GLUIUpdateTourControls();
  update_tour_list =1;
}

/* ------------------ GLUIUpdateTourList(void) ------------------------ */

extern "C" void GLUIUpdateTourList(void){

  update_tour_list =0;
  TourCB(TOUR_LIST);
}

/* ------------------ GLUIHideTour ------------------------ */

extern "C" void GLUIHideTour(void){
  GLUICloseRollouts(glui_tour);
  showtour_dialog = 0;
}

/* ------------------ GLUIShowTour ------------------------ */

extern "C" void GLUIShowTour(void){
  showtour_dialog=1;
  if(glui_tour!=NULL)glui_tour->show();
  updatemenu=1;
}

/* ------------------ TrimVal ------------------------ */

float TrimVal(float val){
  if(ABS(val)<0.000001){
    return 0.0;
  }
  else{
    return val;
  }
}

/* ------------------ GLUIUpdateKeyframe ------------------------ */

 extern "C" void GLUIUpdateKeyframe(void){
  glui_tour_time = selected_frame->time;
  glui_set_tour_time = selected_frame->set_tour_time;
  SPINNER_tour_time->set_float_val(glui_tour_time);
  glui_tour_pause_time = selected_frame->pause_time;
  SPINNER_tour_pause_time->set_float_val(glui_tour_pause_time);
  SPINNER_x->set_float_val(glui_tour_xyz[0]);
  SPINNER_y->set_float_val(glui_tour_xyz[1]);
  SPINNER_z->set_float_val(glui_tour_xyz[2]);
  CHECKBOX_set_tour_time->set_int_val(glui_set_tour_time);
}

/* ------------------ GLUISetTourKeyframe ------------------------ */

extern "C" void GLUISetTourKeyframe(void){
  tourdata *ti;
  float *eye,*xyz_view;

  if(selected_frame==NULL)return;

  ti = selected_tour;
  if(ti==NULL)return;

  tour_hide=1-ti->display;
  if(selected_tour!=NULL)strcpy(tour_label,selected_tour->label);
  glui_avatar_index=ti->glui_avatar_index;
  TourCB(TOUR_AVATAR);
  LISTBOX_avatar->set_int_val(glui_avatar_index);
  eye = selected_frame->xyz_smv;
  xyz_view = selected_frame->view_smv;

  glui_set_tour_time = selected_frame->set_tour_time;
  glui_tour_time    = selected_frame->time;
  glui_tour_pause_time = selected_frame->pause_time;
  glui_tour_xyz[0]  = TrimVal(SMV2FDS_X(eye[0]));
  glui_tour_xyz[1]  = TrimVal(SMV2FDS_Y(eye[1]));
  glui_tour_xyz[2]  = TrimVal(SMV2FDS_Z(eye[2]));
  glui_tour_view[0] = TrimVal(SMV2FDS_X(xyz_view[0]));
  glui_tour_view[1] = TrimVal(SMV2FDS_Y(xyz_view[1]));
  glui_tour_view[2] = TrimVal(SMV2FDS_Z(xyz_view[2]));
  if(SPINNER_tour_time==NULL)return;

  {
    float time_temp;

    time_temp=glui_tour_time;
    SPINNER_tour_time->set_float_limits(selected_frame->prev->time,selected_frame->next->time);
    glui_tour_time=time_temp;
    SPINNER_tour_time->set_float_val(glui_tour_time);
  }

  CHECKBOX_set_tour_time->set_int_val(glui_set_tour_time);
  if(glui_set_tour_time == 1){
    SPINNER_tour_time->enable();
  }
  else{
    SPINNER_tour_time->disable();
  }
  SPINNER_tour_pause_time->set_float_val(glui_tour_pause_time);
  SPINNER_tour_time->set_float_val(glui_tour_time);
  SPINNER_x->set_float_val(glui_tour_xyz[0]);
  SPINNER_y->set_float_val(glui_tour_xyz[1]);
  SPINNER_z->set_float_val(glui_tour_xyz[2]);
  SPINNER_viewx->set_float_val(glui_tour_view[0]);
  SPINNER_viewy->set_float_val(glui_tour_view[1]);
  SPINNER_viewz->set_float_val(glui_tour_view[2]);
  if(CHECKBOX_tourhide!=NULL)CHECKBOX_tourhide->set_int_val(tour_hide);
  EDIT_label->set_text(tour_label);
}

/* ------------------ GLUIUpdateTourIndex ------------------------ */

extern "C" void GLUIUpdateTourIndex(void){
  update_selectedtour_index=0;
  selectedtour_index=selectedtour_index_ini;
  TourCB(TOUR_LIST);
}

/* ------------------ NextTour ------------------------ */

int NextTour(void){
  int i;

  i = selectedtour_index + 1;
  if(i > global_scase.tourcoll.ntourinfo - 1)i = 0;
  if(i >= 0 && i < global_scase.tourcoll.ntourinfo){
    selectedtour_index = i;
    selected_tour = global_scase.tourcoll.tourinfo + i;
    selected_frame = selected_tour->first_frame.next;
    return 1;
  }
  return 0;
}

/* ------------------ PrevTour ------------------------ */

int PrevTour(void){
  int i;

  i = selectedtour_index - 1;
  if(i < 0)i = global_scase.tourcoll.ntourinfo - 1;
  if(i >= 0 && i < global_scase.tourcoll.ntourinfo){
    selectedtour_index = i;
    selected_tour = global_scase.tourcoll.tourinfo + i;
    selected_frame = selected_tour->first_frame.next;
    return 1;
  }
  return 0;
}

/* ------------------ TourCB ------------------------ */

void TourCB(int var){
  keyframe *thiskey,*nextkey,*newframe;
  keyframe *lastkey;
  tourdata *thistour=NULL;
  float *xyz_view,*eye;
  int selectedtour_index_save;

  float key_xyz[3];
  float key_time_in, key_view[3];

  if(global_scase.tourcoll.ntourinfo==0&&var!=TOUR_INSERT_NEW&&var!=TOUR_INSERT_COPY&&var!=TOUR_CLOSE&&var!=SAVE_SETTINGS_TOUR){
    return;
  }
  if(selected_frame!=NULL){
    thistour=selected_tour;
  }

  switch(var){
  case TOUR_CIRCULAR_UPDATE:
    if(edittour==0){
      edittour=1;
      CHECKBOX_showtourroute1->set_int_val(edittour);
      TourCB(SHOWTOURROUTE1);
      if(tour_circular_index!=-1){
        LISTBOX_tour->set_int_val(tour_circular_index);
        TourCB(TOUR_LIST);
      }
    }
    DeleteTourFrames(global_scase.tourcoll.tourinfo);
    InitCircularTour(global_scase.tourcoll.tourinfo,ncircletournodes,UPDATE);
    TourCB(KEYFRAME_UPDATE_ALL);
    UpdateTourMenuLabels();
    CreateTourPaths();
    UpdateTimes();
    GLUICreateTourList();
    glutPostRedisplay();
    break;
  case KEYFRAME_UPDATE_ALL:
    {
      keyframe *frame;

      if(selected_tour == NULL)return;
      update_tour_path=0;
      for(frame=selected_tour->first_frame.next;frame->next!=NULL;frame=frame->next){
        glui_tour_xyz[0] = frame->xyz_fds[0];
        glui_tour_xyz[1] = frame->xyz_fds[1];
        glui_tour_xyz[2] = frame->xyz_fds[2];
        SPINNER_x->set_float_val(glui_tour_xyz[0]);
        SPINNER_y->set_float_val(glui_tour_xyz[1]);
        SPINNER_z->set_float_val(glui_tour_xyz[2]);
        TourCB(KEYFRAME_tXYZ);

        glui_tour_view[0] = tour_circular_view[0];
        glui_tour_view[1] = tour_circular_view[1];
        glui_tour_view[2] = tour_circular_view[2];
        SPINNER_viewx->set_float_val(glui_tour_view[0]);
        SPINNER_viewy->set_float_val(glui_tour_view[1]);
        SPINNER_viewz->set_float_val(glui_tour_view[2]);
        TourCB(KEYFRAME_tXYZ);

        TourCB(KEYFRAME_NEXT);
      }
      update_tour_path=1;
    }
    break;
  case TOUR_USECURRENT:
    break;
  case TOUR_NEXT:
    if(NextTour()==1){
      selected_tour->display=0;
      TOURMENU(selectedtour_index);
      GLUISetTourKeyframe();
    }
    break;
  case TOUR_PREVIOUS:
    if(PrevTour()==1){
      selected_tour->display=0;
      TOURMENU(selectedtour_index);
      GLUISetTourKeyframe();
    }
    break;
  case TOUR_CLOSE:
    GLUIHideTour();
    break;
  case SAVE_SETTINGS_TOUR:
    WriteIni(LOCAL_INI,NULL);
    break;
  case SHOWTOURROUTE:
    if(edittour==1&&selectedtour_index<0&&global_scase.tourcoll.ntourinfo>0){
      selectedtour_index=0;
      TourCB(TOUR_LIST);
    }
    edittour = 1 - edittour;
    TOURMENU(MENU_TOUR_SHOWDIALOG);
    GLUIUpdateTourControls();
    TourCB(VIEW1);
    updatemenu=0;
    break;
  case SHOWTOURROUTE1:
    TourCB(SHOWTOURROUTE);
    CHECKBOX_showtourroute2->set_int_val(edittour);
#ifdef _DEBUG
    CHECKBOX_showdebugtour2->set_int_val(showdebugtour);
#endif
    break;
  case SHOWTOURROUTE2:
    TourCB(SHOWTOURROUTE);
    CHECKBOX_showtourroute1->set_int_val(edittour);
#ifdef _DEBUG
    CHECKBOX_showdebugtour1->set_int_val(showdebugtour);
#endif
    break;
  case VIEWTOURFROMPATH1:
    TourCB(VIEWTOURFROMPATH);
    CHECKBOX_view2->set_int_val(viewtourfrompath);
    break;
  case TOUR_CONSTANT_VELOCITY:
    if(selected_tour != NULL&&tour_constant_velocity==1){
      keyframe *frame;

      for(frame = selected_tour->first_frame.next; frame->next != NULL; frame = frame->next){
        frame->set_tour_time = 0;
      }
      if(CHECKBOX_set_tour_time != NULL){
        glui_set_tour_time = 0;
        CHECKBOX_set_tour_time->set_int_val(glui_set_tour_time);
      }
      TourCB(KEYFRAME_tXYZ);
      SPINNER_tour_time->disable();
    }
    break;
  case KEYFRAME_SET_TOUR_TIME:
    TourCB(KEYFRAME_tXYZ);
    tour_constant_velocity = 1;
    if(glui_set_tour_time == 1){
      tour_constant_velocity = 0;
      SPINNER_tour_time->enable();
    }
    else{
      keyframe *frame;

      for(frame = selected_tour->first_frame.next; frame->next != NULL; frame = frame->next){
        if(frame->set_tour_time == 1){
          tour_constant_velocity = 0;
          break;
        }
      }
      SPINNER_tour_time->disable();
    }
    CHECKBOX_tour_constant_velocity->set_int_val(tour_constant_velocity);
    break;
  case VIEWTOURFROMPATH2:
    TourCB(VIEWTOURFROMPATH);
    CHECKBOX_view1->set_int_val(viewtourfrompath);
    break;
  case TOUR_SNAP:
    if(tour_snap == 1){
      if(global_times != NULL){
        tour_snap_time = global_times[itimes];
      }
      else{
        tour_snap_time = 0.0;
      }
    }
    break;
  case VIEWTOURFROMPATH:
    viewtourfrompath = 1 - viewtourfrompath;
    TOURMENU(MENU_TOUR_VIEWFROMROUTE);
    break;
  case VIEW1:
    CreateTourPaths();
    break;
  case VIEW_times:
    ReallocTourMemory(&global_scase.tourcoll);
    CreateTourPaths();
    UpdateTimes();
    break;
  case VIEW_ALL_NODES:
    SetKeyFrameViews(selected_frame->view_smv);
    GLUISetTourKeyframe();
    break;
  case VIEW_NEXT_NODE:
    SetKeyFrameViews(NULL);
    GLUISetTourKeyframe();
    break;
  case KEYFRAME_viewXYZ:
    if(selected_frame != NULL){
      if(selected_tour-global_scase.tourcoll.tourinfo==0)dirtycircletour=1;
      selected_tour->startup = 0;
      xyz_view = selected_frame->view_smv;
      FDS2SMV_XYZ(xyz_view, glui_tour_view);

      if(update_tour_path == 1)CreateTourPaths();
      selected_frame->selected = 1;
    }
    break;
  case KEYFRAME_t:
#define TOUR_EPS 0.01
    if(selected_frame != NULL && glui_set_tour_time ==1){
      float tour_tmin, tour_tmax;
      thiskey = selected_frame;
      nextkey = thiskey->next;
      lastkey = thiskey->prev;
      tour_tmin = lastkey->time + lastkey->pause_time + TOUR_EPS;
      tour_tmax = nextkey->time - TOUR_EPS;
      if(glui_tour_time < tour_tmin){
        glui_tour_time = tour_tmin;
        SPINNER_tour_time->set_float_val(glui_tour_time);
      }
      else if(glui_tour_time > tour_tmax){
        glui_tour_time = tour_tmax;
        SPINNER_tour_time->set_float_val(glui_tour_time);
      }
    }
    TourCB(KEYFRAME_tXYZ);
    break;
  case KEYFRAME_tXYZ:
    if(selected_frame != NULL){
      show_tour_hint = 0;
      if(selected_tour - global_scase.tourcoll.tourinfo == 0)dirtycircletour = 1;
      selected_tour->startup = 0;
      eye = selected_frame->xyz_smv;
      xyz_view = selected_frame->view_smv;
      selected_frame->set_tour_time = glui_set_tour_time;
      selected_frame->time          = glui_tour_time;

      FDS2SMV_XYZ(eye, glui_tour_xyz);
      memcpy(selected_frame->xyz_fds, glui_tour_xyz, 3 * sizeof(float));
      memcpy(selected_frame->xyz_smv, eye, 3 * sizeof(float));
      selected_frame->pause_time = glui_tour_pause_time;

      FDS2SMV_XYZ(xyz_view, glui_tour_view);
      selected_frame->selected = 1;
      TourCB(KEYFRAME_viewXYZ);
    }
    break;
  case KEYFRAME_NEXT:
    show_tour_hint = 0;
    if(selected_frame==NULL&&global_scase.tourcoll.tourinfo!=NULL){
      selected_frame=&(global_scase.tourcoll.tourinfo[0].first_frame);
      selected_tour=global_scase.tourcoll.tourinfo;
    }
    if(selected_frame!=NULL){
      thistour=selected_tour;
      if(selected_frame->next!=&thistour->last_frame){
        NewSelect(selected_frame->next);
      }
      else{
        NewSelect(thistour->first_frame.next);
      }
    }
    GLUISetTourKeyframe();
    break;
  case KEYFRAME_PREVIOUS:
    show_tour_hint = 0;
    if(selected_frame==NULL&&global_scase.tourcoll.tourinfo!=NULL){
      selected_frame=&(global_scase.tourcoll.tourinfo[0].last_frame);
      selected_tour=global_scase.tourcoll.tourinfo;
    }
    if(selected_frame!=NULL){
      thistour=selected_tour;
      selected_tour=thistour;
      if(selected_frame->prev!=&thistour->first_frame){
        NewSelect(selected_frame->prev);
      }
      else{
        NewSelect(thistour->last_frame.prev);
      }
    }
    GLUISetTourKeyframe();
    break;
  case KEYFRAME_INSERT:
    show_tour_hint = 0;
    if(selected_frame!=NULL){
      thistour=selected_tour;
      thiskey=selected_frame;
      nextkey=thiskey->next;
      if(nextkey==&thistour->last_frame){
        lastkey=thiskey->prev;
        key_xyz[0] = SMV2FDS_X(2*thiskey->xyz_smv[0]-lastkey->xyz_smv[0]);
        key_xyz[1] = SMV2FDS_Y(2*thiskey->xyz_smv[1]-lastkey->xyz_smv[1]);
        key_xyz[2] = SMV2FDS_Z(2*thiskey->xyz_smv[2]-lastkey->xyz_smv[2]);
        key_time_in = thiskey->time;
        thiskey->time=(thiskey->time+thiskey->pause_time+lastkey->time)/2.0;
        key_view[0] = SMV2FDS_X(2*thiskey->view_smv[0]-lastkey->view_smv[0]);
        key_view[1] = SMV2FDS_Y(2*thiskey->view_smv[1]-lastkey->view_smv[1]);
        key_view[2] = SMV2FDS_Z(2*thiskey->view_smv[2]-lastkey->view_smv[2]);
      }
      else{
        float t_avg;

        t_avg = (thiskey->time+nextkey->time)/2.0;
        GetKeyXYZ(t_avg,  thiskey, key_xyz);
        SMV2FDS_XYZ(key_xyz, key_xyz);
        key_time_in = (thiskey->time +nextkey->time)/2.0;
        GetKeyView(t_avg, thiskey, key_view);
        SMV2FDS_XYZ(key_view, key_view);
      }
      newframe = AddFrame(selected_frame, key_time_in, 0.0, key_xyz, key_view, 0);
      CreateTourPaths();
      NewSelect(newframe);
      GLUISetTourKeyframe();
    }
    break;
  case KEYFRAME_DELETE:
    show_tour_hint = 0;
    if(selected_frame!=NULL){
      selected_frame=DeleteFrame(selected_frame);
      if(selected_frame!=NULL){
        selected_frame->selected=1;
        CreateTourPaths();
      }
      else{
        if(thistour!=NULL)DeleteTour(thistour-global_scase.tourcoll.tourinfo);
      }
    }
    break;
  case TOUR_AVATAR:
    if(selected_tour->glui_avatar_index!=glui_avatar_index){
      selected_tour->glui_avatar_index=glui_avatar_index;
// hack to make touring avatar show up
      global_scase.objectscoll.avatar_types[glui_avatar_index]->visible=1;
      updatemenu=1;
    }
    if(glui_avatar_index==-1){
      tourlocus_type=0;
    }
    else if(glui_avatar_index==-2){
      tourlocus_type=1;
    }
    else{
      tourlocus_type=2;
      global_scase.objectscoll.iavatar_types=glui_avatar_index;
    }
    break;
  case TOUR_UPDATELABEL:
    if(selectedtour_index>=0&&selectedtour_index<global_scase.tourcoll.ntourinfo){
      selectedtour_index_save = selectedtour_index;
      selectedtour_index = -1;
      TourCB(TOUR_LIST);
      selectedtour_index = selectedtour_index_save;
    }
    TourCB(TOUR_LIST);
    break;
  case TOUR_LIST:
    if(selectedtour_index==-999){
      selectedtour_index=selectedtour_index_old;
      if(selectedtour_index==-999)selectedtour_index = TOURINDEX_MANUAL;
      TourCB(TOUR_LIST);
      return;
    }
    switch(selectedtour_index){
    case TOURINDEX_ALL:
      TOURMENU(MENU_TOUR_SHOWALL); // show all tours
      GLUISetTourKeyframe();
      break;
    case TOURINDEX_MANUAL:
      edittour=0;
      TOURMENU(MENU_TOUR_CLEARALL);  // reset tour vis to ini values
      if(PANEL_node != NULL)PANEL_node->disable();
      if(PANEL_tournavigate!=NULL)PANEL_tournavigate->disable();
      break;
    case TOURINDEX_DEFAULT:
      TOURMENU(MENU_TOUR_DEFAULT);  // default tour
      break;
    default:
      selected_tour=global_scase.tourcoll.tourinfo + selectedtour_index;
      selected_frame=selected_tour->first_frame.next;
      selected_tour->display=0;
      TOURMENU(selectedtour_index);
      GLUISetTourKeyframe();
      if(PANEL_node != NULL)PANEL_node->enable();
      if(PANEL_tournavigate!=NULL)PANEL_tournavigate->enable();
      TourCB(TOUR_CONSTANT_VELOCITY);
      break;
    }
    GLUIDeleteTourList();
    GLUICreateTourList();
    UpdateViewTour();
    GLUIUpdateTourControls();
    selectedtour_index_old=selectedtour_index;
    break;
  case TOUR_DELETE:
    DeleteTour(selectedtour_index);
    break;
  case TOUR_REVERSE:
    if(selectedtour_index>=0&&selectedtour_index<global_scase.tourcoll.ntourinfo){
      ReverseTour(global_scase.tourcoll.tourinfo[selectedtour_index].label);
    }
    break;
  case TOUR_INSERT_NEW:
  case TOUR_INSERT_COPY:
    if(var==TOUR_INSERT_NEW){
      thistour=AddTour(NULL);
    }
    else{
      if(selectedtour_index>=0&&selectedtour_index<global_scase.tourcoll.ntourinfo){
        char label[300];

        strcpy(label, global_scase.tourcoll.tourinfo[selectedtour_index].label);
        thistour = AddTour(label);
      }
      else{
        thistour = AddTour(NULL);
      }
    }
    if(CHECKBOX_showtourroute1 != NULL&&edittour == 0)CHECKBOX_showtourroute1->set_int_val(1);
    if(CHECKBOX_showtourroute2 != NULL&&edittour == 0)CHECKBOX_showtourroute2->set_int_val(1);
    selected_frame=thistour->first_frame.next;
    selected_tour=thistour;
    selectedtour_index = thistour - global_scase.tourcoll.tourinfo;
    selectedtour_index_old=selectedtour_index;
    GLUISetTourKeyframe();
    CreateTourPaths();
    UpdateViewTour();
    GLUIUpdateTourControls();
    selected_tour->display=0;
    TOURMENU(selectedtour_index);
    if(PANEL_node!=NULL)PANEL_node->enable();
    if(SPINNER_tour_time != NULL){
      if(glui_set_tour_time == 1){
        SPINNER_tour_time->enable();
    }
      else{
        SPINNER_tour_time->disable();
      }
    }
    updatemenu=1;
    break;
  case TOUR_LABEL:
    if(thistour!=NULL){
      strcpy(thistour->label,tour_label);
      GLUISetTourKeyframe();
      if(LISTBOX_tour!=NULL){
        LISTBOX_tour->delete_item(thistour-global_scase.tourcoll.tourinfo);
        LISTBOX_tour->add_item(thistour-global_scase.tourcoll.tourinfo,thistour->label);
      }
      UpdateTourMenuLabels();
      updatemenu=1;
    }
    break;
  case TOUR_HIDE:
    if(thistour!=NULL){
      if(tour_hide==1){
        thistour->display=1;
        TOURMENU(thistour-global_scase.tourcoll.tourinfo);
        NextTour();
        GLUISetTourKeyframe();
        thistour->display=0;
      }
      else{
        thistour->display=1;
      }
      updatemenu=1;
      GLUIDeleteTourList();
      GLUICreateTourList();
      UpdateViewTour();
      GLUIUpdateTourControls();
    }
    break;
  default:
    assert(FFALSE);
  }
}

/* ------------------ GLUIDeleteTourList ------------------------ */

extern "C" void GLUIDeleteTourList(void){
  int i;

  if(LISTBOX_tour==NULL)return;
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    LISTBOX_tour->delete_item(i);
  }
  GLUIDeleteVolTourList(); //xx comment this line if smokebot fails with seg fault
}

/* ------------------ GLUICreateTourList ------------------------ */

extern "C" void GLUICreateTourList(void){
  int i;

  if(LISTBOX_tour==NULL)return;
  for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
    tourdata *touri;
    char label[1000];

    touri = global_scase.tourcoll.tourinfo + i;
    strcpy(label,"");
    if(i==selectedtour_index)strcat(label,"*");
    if(strlen(touri->label)>0)strcat(label,touri->label);
    if(strlen(label)>0){
      LISTBOX_tour->add_item(i,label);
    }
    else{
      LISTBOX_tour->add_item(i,"error");
    }
  }
  if(selectedtour_index>=-1&&selectedtour_index<global_scase.tourcoll.ntourinfo)LISTBOX_tour->set_int_val(selectedtour_index);

  GLUICreateVolTourList(); //xx comment this line if smokebot fails with seg fault
}

/* ------------------ GLUIUpdateTourControls ------------------------ */

extern "C" void GLUIUpdateTourControls(void){
  if(BUTTON_next_tour==NULL)return;
  if(BUTTON_prev_tour==NULL)return;
  if(ROLLOUT_keyframe==NULL)return;
  if(SPINNER_x==NULL)return;
  if(CHECKBOX_showtourroute1 != NULL)CHECKBOX_showtourroute1->set_int_val(edittour);
  if(CHECKBOX_showtourroute2 != NULL)CHECKBOX_showtourroute2->set_int_val(edittour);
  if(CHECKBOX_view1!=NULL)CHECKBOX_view1->set_int_val(viewtourfrompath);
  if(CHECKBOX_view2!=NULL)CHECKBOX_view2->set_int_val(viewtourfrompath);
  if(global_scase.tourcoll.ntourinfo>1){
    BUTTON_next_tour->enable();
    BUTTON_prev_tour->enable();
  }
  else{
    BUTTON_next_tour->disable();
    BUTTON_prev_tour->disable();
  }

  if(CHECKBOX_tourhide!=NULL){
    if(viewanytours>0&&edittour==1){
      CHECKBOX_tourhide->enable();
    }
    else{
      CHECKBOX_tourhide->disable();
    }
  }
  if(selected_tour!=NULL){
    selectedtour_index = selected_tour-global_scase.tourcoll.tourinfo;
    LISTBOX_tour->set_int_val(selectedtour_index);
  }
  else{
    selectedtour_index = TOURINDEX_MANUAL;
    LISTBOX_tour->set_int_val(selectedtour_index);
  }
}
