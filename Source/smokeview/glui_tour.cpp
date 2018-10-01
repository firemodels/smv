#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "update.h"
#include "smokeviewvars.h"

static int viewtype1=REL_VIEW;
static int viewtype2=REL_VIEW;
static float tour_ttt;
static float tour_view_xyz[3]={0.0,0.0,0.0}, tour_elev_path=0.0;
static int tour_hide=0;
static float tour_zoom=1.0;
static char tour_label[sizeof(GLUI_String)];

GLUI *glui_tour=NULL;

GLUI_Rollout *ROLLOUT_keyframe = NULL;
GLUI_Rollout *ROLLOUT_settings = NULL;
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

GLUI_Checkbox *CHECKBOX_snap = NULL;
GLUI_Checkbox *CHECKBOX_view1 = NULL;
GLUI_Checkbox *CHECKBOX_view2 = NULL;
GLUI_Checkbox *CHECKBOX_showtourroute1 = NULL;
GLUI_Checkbox *CHECKBOX_showtourroute2 = NULL;
GLUI_Checkbox *CHECKBOX_showintermediate = NULL;
GLUI_Checkbox *CHECKBOX_tourhide=NULL;

GLUI_Spinner *SPINNER_t = NULL;
GLUI_Spinner *SPINNER_x = NULL;
GLUI_Spinner *SPINNER_y = NULL;
GLUI_Spinner *SPINNER_z = NULL;
GLUI_Spinner *SPINNER_viewx = NULL;
GLUI_Spinner *SPINNER_viewy = NULL;
GLUI_Spinner *SPINNER_viewz = NULL;
GLUI_Spinner *SPINNER_tourzoom=NULL;

GLUI_Spinner *SPINNER_tour_circular_view[3];
GLUI_Spinner *SPINNER_tour_circular_center[3];
GLUI_Spinner *SPINNER_tour_circular_radius=NULL;

GLUI_Button *BUTTON_next_tour=NULL;
GLUI_Button *BUTTON_prev_tour=NULL;
GLUI_Button *BUTTON_delete_tour = NULL;
GLUI_EditText *EDIT_label=NULL;

GLUI_Listbox *LISTBOX_tour=NULL;
GLUI_Listbox *LISTBOX_avatar=NULL;

#define TOUR_CLOSE 99
#define KEYFRAME_tXYZ 1
#define KEYFRAME_INSERT 2
#define KEYFRAME_DELETE 3
#define KEYFRAME_PREVIOUS 4
#define KEYFRAME_NEXT 5
#define SAVE_SETTINGS 7
#define SHOWTOURROUTE 8
#define VIEWTOURFROMPATH 9
#define VIEWTOURFROMPATH1 41
#define VIEWTOURFROMPATH2 42
#define SHOWTOURROUTE1 39
#define SHOWTOURROUTE2 40
#define TOUR_INSERT_NEW 32
#define TOUR_INSERT_COPY 33
#define TOUR_PREVIOUS 17
#define TOUR_NEXT 18
#define TOUR_DELETE 34
#define TOUR_LABEL 19
#define TOUR_HIDE 20
#define KEYFRAME_viewXYZ 22
#define VIEWSNAP 23
#define TOUR_LIST 24
#define TOUR_AVATAR 31
#define VIEW1 26
#define VIEW2 30
#define VIEW_times 27
#define TOUR_UPDATELABEL 28
#define TOUR_USECURRENT 29
#define TOUR_REVERSE 35
#define TOUR_CIRCULAR_UPDATE 36
#define KEYFRAME_UPDATE_ALL 38

#define TOURMENU(f) callfrom_tourglui=1;TourMenu(f);callfrom_tourglui=0;

#define SETTINGS_TOURS_ROLLOUT 0
#define KEYFRAME_TOURS_ROLLOUT 1
#define MODIFY_TOURS_ROLLOUT 2

procdata toursprocinfo[3];
int ntoursprocinfo = 0;

/* ------------------ ToursRolloutCB ------------------------ */

void ToursRolloutCB(int var){
  ToggleRollout(toursprocinfo, ntoursprocinfo, var);
  if(var == MODIFY_TOURS_ROLLOUT){
    if(ROLLOUT_circular->is_open == 1){
      selectedtour_index = 0;
      TourCB(TOUR_LIST);
      LISTBOX_tour->set_int_val(selectedtour_index);
    }
  }
}


/* ------------------ UpdateTourState ------------------------ */

extern "C" void UpdateTourState(void){
  TourCB(SHOWTOURROUTE);
  TourCB(VIEWTOURFROMPATH);
  TourCB(VIEWSNAP);
}

/* ------------------ AddDeleteKeyframe ------------------------ */

void AddDeleteKeyframe(int flag){
  if(flag==ADD_KEYFRAME)TourCB(KEYFRAME_INSERT);
  if(flag==DELETE_KEYFRAME)TourCB(KEYFRAME_DELETE);
}


/* ------------------ UpdateEditTour ------------------------ */

extern "C" void UpdateEditTour(void){
  TourCB(SHOWTOURROUTE);
}

/* ------------------ UpdateTourParms ------------------------ */

extern "C" void UpdateTourParms(void){
  TourCB(KEYFRAME_tXYZ);
}

/* ------------------ AddNewTour ------------------------ */

extern "C" void AddNewTour(void){
  TourCB(TOUR_INSERT_NEW);
}

/* ------------------ GluiTourSetup ------------------------ */

extern "C" void GluiTourSetup(int main_window){

  int i;

  update_glui_tour=0;
  if(glui_tour!=NULL){
    glui_tour->close();
    glui_tour=NULL;
  }
  glui_tour = GLUI_Master.create_glui(_("Tours"),0,0,0);
  glui_tour->hide();

  ROLLOUT_circular = glui_tour->add_rollout(_("Modify circular tour"),false, MODIFY_TOURS_ROLLOUT, ToursRolloutCB);
  ADDPROCINFO(toursprocinfo, ntoursprocinfo, ROLLOUT_circular, MODIFY_TOURS_ROLLOUT);

  CHECKBOX_showtourroute2 = glui_tour->add_checkbox_to_panel(ROLLOUT_circular, _("Show path/nodes"), &edittour, SHOWTOURROUTE2, TourCB);
  CHECKBOX_view2 = glui_tour->add_checkbox_to_panel(ROLLOUT_circular, _("View from tour path"), &viewtourfrompath, VIEWTOURFROMPATH2, TourCB);

  SPINNER_tour_circular_radius=glui_tour->add_spinner_to_panel(ROLLOUT_circular, "radius", GLUI_SPINNER_FLOAT, &tour_circular_radius,TOUR_CIRCULAR_UPDATE,TourCB);

  PANEL_tour_circular_center = glui_tour->add_panel_to_panel(ROLLOUT_circular,_("center"),true);
  SPINNER_tour_circular_center[0]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"x",GLUI_SPINNER_FLOAT,tour_circular_center,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_center[1]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"y",GLUI_SPINNER_FLOAT,tour_circular_center+1,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_center[2]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"z",GLUI_SPINNER_FLOAT,tour_circular_center+2,TOUR_CIRCULAR_UPDATE,TourCB);

  PANEL_tour_circular_view = glui_tour->add_panel_to_panel(ROLLOUT_circular,_("target"),true);
  SPINNER_tour_circular_view[0]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"x",GLUI_SPINNER_FLOAT,tour_circular_view,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_view[1]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"y",GLUI_SPINNER_FLOAT,tour_circular_view+1,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_view[2]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"z",GLUI_SPINNER_FLOAT,tour_circular_view+2,TOUR_CIRCULAR_UPDATE,TourCB);

  ROLLOUT_keyframe = glui_tour->add_rollout("Modify general tour",true,KEYFRAME_TOURS_ROLLOUT, ToursRolloutCB);
  ADDPROCINFO(toursprocinfo, ntoursprocinfo, ROLLOUT_keyframe, KEYFRAME_TOURS_ROLLOUT);

  PANEL_tour = glui_tour->add_panel_to_panel(ROLLOUT_keyframe,"Tour", true);

  PANEL_tour1 = glui_tour->add_panel_to_panel(PANEL_tour, "", GLUI_PANEL_NONE);

  BUTTON_prev_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Previous"), TOUR_PREVIOUS, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("Copy"), TOUR_INSERT_COPY, TourCB);
  BUTTON_delete_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Delete"), TOUR_DELETE, TourCB);
  glui_tour->add_column_to_panel(PANEL_tour1, false);
  BUTTON_next_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Next"), TOUR_NEXT, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("New"), TOUR_INSERT_NEW, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("Reverse"), TOUR_REVERSE, TourCB);

  if(ntourinfo > 0){
    selectedtour_index = TOURINDEX_MANUAL;
    selectedtour_index_old = TOURINDEX_MANUAL;
    LISTBOX_tour = glui_tour->add_listbox_to_panel(PANEL_tour, "Select: ", &selectedtour_index, TOUR_LIST, TourCB);

    LISTBOX_tour->add_item(TOURINDEX_MANUAL, "Manual");
    LISTBOX_tour->add_item(-999, "-");
    for(i = 0;i < ntourinfo;i++){
      tourdata *touri;

      touri = tourinfo + i;
      LISTBOX_tour->add_item(i, touri->label);
    }
    LISTBOX_tour->set_int_val(selectedtour_index);
    glui_tour->add_column_to_panel(PANEL_tour1, false);
  }
  EDIT_label = glui_tour->add_edittext_to_panel(PANEL_tour, "Label:", GLUI_EDITTEXT_TEXT, tour_label, TOUR_LABEL, TourCB);
  EDIT_label->set_w(200);
  glui_tour->add_button_to_panel(PANEL_tour, _("Update label"), TOUR_UPDATELABEL, TourCB);

  CHECKBOX_showtourroute1 = glui_tour->add_checkbox_to_panel(ROLLOUT_keyframe, _("Show path/nodes"), &edittour, SHOWTOURROUTE1, TourCB);
  CHECKBOX_view1 = glui_tour->add_checkbox_to_panel(ROLLOUT_keyframe, _("View from tour path"), &viewtourfrompath, VIEWTOURFROMPATH1, TourCB);

  PANEL_node = glui_tour->add_panel_to_panel(ROLLOUT_keyframe, "", GLUI_PANEL_NONE);

  PANEL_tourposition = glui_tour->add_panel_to_panel(PANEL_node, _("Node"));

  SPINNER_t = glui_tour->add_spinner_to_panel(PANEL_tourposition, "t:", GLUI_SPINNER_FLOAT, &tour_ttt, KEYFRAME_tXYZ, TourCB);
  SPINNER_t->disable();
  SPINNER_x=glui_tour->add_spinner_to_panel(PANEL_tourposition,"x:",GLUI_SPINNER_FLOAT,tour_xyz,KEYFRAME_tXYZ,TourCB);
  SPINNER_y=glui_tour->add_spinner_to_panel(PANEL_tourposition,"y:",GLUI_SPINNER_FLOAT,tour_xyz+1,KEYFRAME_tXYZ,TourCB);
  SPINNER_z=glui_tour->add_spinner_to_panel(PANEL_tourposition,"z:",GLUI_SPINNER_FLOAT,tour_xyz+2,KEYFRAME_tXYZ,TourCB);

  PANEL_tourview = glui_tour->add_panel_to_panel(PANEL_node, _("Target"));
  SPINNER_tourzoom = glui_tour->add_spinner_to_panel(PANEL_tourview, _("Zoom:"), GLUI_SPINNER_FLOAT, &tour_zoom, KEYFRAME_tXYZ, TourCB);
  SPINNER_viewx=glui_tour->add_spinner_to_panel(PANEL_tourview,"x",GLUI_SPINNER_FLOAT,tour_view_xyz,KEYFRAME_viewXYZ,TourCB);
  SPINNER_viewy=glui_tour->add_spinner_to_panel(PANEL_tourview,"y",GLUI_SPINNER_FLOAT,tour_view_xyz+1,KEYFRAME_viewXYZ,TourCB);
  SPINNER_viewz=glui_tour->add_spinner_to_panel(PANEL_tourview,"z",GLUI_SPINNER_FLOAT,tour_view_xyz+2,KEYFRAME_viewXYZ,TourCB);

  PANEL_tournavigate = glui_tour->add_panel_to_panel(PANEL_node, "", GLUI_PANEL_NONE);

  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Previous"), KEYFRAME_PREVIOUS, TourCB);
  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Delete"), KEYFRAME_DELETE, TourCB);

  glui_tour->add_column_to_panel(PANEL_tournavigate, false);

  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Next"), KEYFRAME_NEXT, TourCB);
  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Insert after"), KEYFRAME_INSERT, TourCB);

  ROLLOUT_settings = glui_tour->add_rollout(_("Settings"), true, SETTINGS_TOURS_ROLLOUT, ToursRolloutCB);
  ADDPROCINFO(toursprocinfo, ntoursprocinfo, ROLLOUT_settings, SETTINGS_TOURS_ROLLOUT);

  PANEL_path = glui_tour->add_panel_to_panel(ROLLOUT_settings, _("Duration"), true);

  glui_tour->add_spinner_to_panel(PANEL_path, _("start time"), GLUI_SPINNER_FLOAT, &view_tstart, VIEW_times, TourCB);
  glui_tour->add_spinner_to_panel(PANEL_path, _("stop time:"), GLUI_SPINNER_FLOAT, &view_tstop, VIEW_times, TourCB);
  glui_tour->add_spinner_to_panel(PANEL_path, _("points"), GLUI_SPINNER_INT, &view_ntimes, VIEW_times, TourCB);

  PANEL_misc = glui_tour->add_panel_to_panel(ROLLOUT_settings, "Misc", true);
  CHECKBOX_snap = glui_tour->add_checkbox_to_panel(PANEL_misc, _("View from selected keyframe"), &keyframe_snap, VIEWSNAP, TourCB);
  CHECKBOX_showintermediate = glui_tour->add_checkbox_to_panel(PANEL_misc, _("Show intermediate path nodes"), &show_path_knots);
#ifdef _DEBUG
  glui_tour->add_checkbox_to_panel(PANEL_misc, _("Anti-alias tour path line"), &tour_antialias);
#endif
  if(navatar_types > 0){
    glui_tour->add_checkbox_to_panel(PANEL_misc, _("Show avatar"), &show_tourlocus);
    LISTBOX_avatar = glui_tour->add_listbox_to_panel(PANEL_misc, _("Avatar:"), &glui_avatar_index, TOUR_AVATAR, TourCB);
    for(i = 0;i < navatar_types;i++){
      LISTBOX_avatar->add_item(i, avatar_types[i]->label);
    }
    if(tourlocus_type == 0){
      glui_avatar_index = -1;
    }
    else if(tourlocus_type == 1){
      glui_avatar_index = -2;
    }
    else{
      glui_avatar_index = iavatar_types;
    }
    LISTBOX_avatar->set_int_val(glui_avatar_index);
  }

  PANEL_close_tour = glui_tour->add_panel("",false);
  glui_tour->add_button_to_panel(PANEL_close_tour,_("Save settings"),SAVE_SETTINGS,TourCB);
  glui_tour->add_column_to_panel(PANEL_close_tour,false);
  glui_tour->add_button_to_panel(PANEL_close_tour,"Close",TOUR_CLOSE,TourCB);

  ROLLOUT_keyframe->close();
  ROLLOUT_settings->close();

  glui_tour->set_main_gfx_window( main_window );

  TourCB(VIEW1);

  UpdateTourControls();
  update_tour_list =1;
}

/* ------------------ UpdateTourList(void) ------------------------ */

extern "C" void UpdateTourList(void){

  update_tour_list =0;
  TourCB(TOUR_LIST);
}

/* ------------------ HideGluiTour ------------------------ */

extern "C" void HideGluiTour(void){
  if(glui_tour!=NULL)glui_tour->hide();
  showtour_dialog=0;
  updatemenu=1;
}

/* ------------------ ShowGluiTour ------------------------ */

extern "C" void ShowGluiTour(void){
  showtour_dialog=1;
  if(glui_tour!=NULL)glui_tour->show();
  updatemenu=1;
}

/* ------------------ TrimVal ------------------------ */

extern "C" float TrimVal(float val){
  if(ABS(val)<0.000001){
    return 0.0;
  }
  else{
    return val;
  }
}

/* ------------------ UpdateGluiKeyframe ------------------------ */

extern "C" void UpdateGluiKeyframe(void){
  tour_ttt = selected_frame->disp_time;
  SPINNER_t->set_float_val(tour_ttt);
  SPINNER_x->set_float_val(tour_xyz[0]);
  SPINNER_y->set_float_val(tour_xyz[1]);
  SPINNER_z->set_float_val(tour_xyz[2]);
}

/* ------------------ SetGluiTourKeyframe ------------------------ */

extern "C" void SetGluiTourKeyframe(void){
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
  eye = selected_frame->nodeval.eye;
  xyz_view = selected_frame->nodeval.xyz_view_abs;

  tour_ttt = selected_frame->disp_time;
  tour_xyz[0] = TrimVal(DENORMALIZE_X(eye[0]));
  tour_xyz[1] = TrimVal(DENORMALIZE_Y(eye[1]));
  tour_xyz[2] = TrimVal(DENORMALIZE_Z(eye[2]));
  tour_view_xyz[0] = TrimVal(DENORMALIZE_X(xyz_view[0]));
  tour_view_xyz[1] = TrimVal(DENORMALIZE_Y(xyz_view[1]));
  tour_view_xyz[2] = TrimVal(DENORMALIZE_Z(xyz_view[2]));
  viewtype1=selected_frame->viewtype;
  viewtype2=1-viewtype1;
  tour_zoom=selected_frame->nodeval.zoom;
  tour_elev_path=selected_frame->nodeval.elev_path;

  if(SPINNER_t==NULL)return;

  {
    float time_temp;

    time_temp=tour_ttt;
    SPINNER_t->set_float_limits(selected_frame->prev->disp_time,selected_frame->next->disp_time);
    tour_ttt=time_temp;
    SPINNER_t->set_float_val(tour_ttt);
  }

  SPINNER_t->set_float_val(tour_ttt);
  SPINNER_x->set_float_val(tour_xyz[0]);
  SPINNER_y->set_float_val(tour_xyz[1]);
  SPINNER_z->set_float_val(tour_xyz[2]);
  SPINNER_tourzoom->set_float_val(tour_zoom);
  SPINNER_viewx->set_float_val(tour_view_xyz[0]);
  SPINNER_viewy->set_float_val(tour_view_xyz[1]);
  SPINNER_viewz->set_float_val(tour_view_xyz[2]);
  if(CHECKBOX_tourhide!=NULL)CHECKBOX_tourhide->set_int_val(tour_hide);
  EDIT_label->set_text(tour_label);
}

/* ------------------ UpdateTourIndex ------------------------ */

extern "C" void UpdateTourIndex(void){
  update_selectedtour_index=0;
  selectedtour_index=selectedtour_index_ini;
  TourCB(TOUR_LIST);
}

/* ------------------ NextTour ------------------------ */

int NextTour(void){
  int i;

  i = selectedtour_index + 1;
  if(i > ntourinfo - 1)i = 0;
  if(i >= 0 && i < ntourinfo){
    selectedtour_index = i;
    selected_tour = tourinfo + i;
    selected_frame = selected_tour->first_frame.next;
    return 1;
  }
  return 0;
}

/* ------------------ PrevTour ------------------------ */

int PrevTour(void){
  int i;

  i = selectedtour_index - 1;
  if(i < 0)i = ntourinfo - 1;
  if(i >= 0 && i < ntourinfo){
    selectedtour_index = i;
    selected_tour = tourinfo + i;
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
  float key_time_in, key_az_path, key_view[3], key_zoom;
  float key_elev_path;

  if(ntourinfo==0&&var!=TOUR_INSERT_NEW&&var!=TOUR_INSERT_COPY&&var!=TOUR_CLOSE&&var!=SAVE_SETTINGS){
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
    DeleteTourFrames(tourinfo);
    InitCircularTour(tourinfo,ncircletournodes,UPDATE);
    TourCB(KEYFRAME_UPDATE_ALL);
    UpdateTourMenuLabels();
    CreateTourPaths();
    UpdateTimes();
    CreateTourList();
    glutPostRedisplay();
    break;
  case KEYFRAME_UPDATE_ALL:
    {
      keyframe *frame;

      if(selected_tour == NULL)return;
      update_tour_path=0;
      for(frame=selected_tour->first_frame.next;frame->next!=NULL;frame=frame->next){
        tour_xyz[0] = frame->eye[0];
        tour_xyz[1] = frame->eye[1];
        tour_xyz[2] = frame->eye[2];
        SPINNER_x->set_float_val(tour_xyz[0]);
        SPINNER_y->set_float_val(tour_xyz[1]);
        SPINNER_z->set_float_val(tour_xyz[2]);
        TourCB(KEYFRAME_tXYZ);

        tour_view_xyz[0] = tour_circular_view[0];
        tour_view_xyz[1] = tour_circular_view[1];
        tour_view_xyz[2] = tour_circular_view[2];
        SPINNER_viewx->set_float_val(tour_view_xyz[0]);
        SPINNER_viewy->set_float_val(tour_view_xyz[1]);
        SPINNER_viewz->set_float_val(tour_view_xyz[2]);
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
      SetGluiTourKeyframe();
    }
    break;
  case TOUR_PREVIOUS:
    if(PrevTour()==1){
      selected_tour->display=0;
      TOURMENU(selectedtour_index);
      SetGluiTourKeyframe();
    }
    break;
  case TOUR_CLOSE:
    HideGluiTour();
    break;
  case SAVE_SETTINGS:
    WriteIni(LOCAL_INI,NULL);
    break;
  case SHOWTOURROUTE:
    if(edittour==1&&selectedtour_index<0&&ntourinfo>0){
      selectedtour_index=0;
      TourCB(TOUR_LIST);
    }
    edittour = 1 - edittour;
    TOURMENU(MENU_TOUR_SHOWDIALOG);
    UpdateTourControls();
    TourCB(VIEW1);
    updatemenu=0;
    break;
  case SHOWTOURROUTE1:
    TourCB(SHOWTOURROUTE);
    CHECKBOX_showtourroute2->set_int_val(edittour);
    break;
  case SHOWTOURROUTE2:
    TourCB(SHOWTOURROUTE);
    CHECKBOX_showtourroute1->set_int_val(edittour);
    break;
  case VIEWSNAP:
    if(viewtourfrompath==1&&keyframe_snap==1){
      viewtourfrompath=0;
      CHECKBOX_view1->set_int_val(viewtourfrompath);
      CHECKBOX_view2->set_int_val(viewtourfrompath);
    }
    TourCB(VIEWTOURFROMPATH);
    updatemenu=0;
    break;
  case VIEWTOURFROMPATH1:
    TourCB(VIEWTOURFROMPATH);
    CHECKBOX_view2->set_int_val(viewtourfrompath);
    break;
  case VIEWTOURFROMPATH2:
    TourCB(VIEWTOURFROMPATH);
    CHECKBOX_view1->set_int_val(viewtourfrompath);
    break;
  case VIEWTOURFROMPATH:
    if(viewtourfrompath==1&&keyframe_snap==1){
      keyframe_snap=0;
      CHECKBOX_snap->set_int_val(keyframe_snap);
    }
    viewtourfrompath = 1 - viewtourfrompath;
    TOURMENU(MENU_TOUR_VIEWFROMROUTE);
    break;
  case VIEW2:
    viewtype1=1-viewtype2;
    viewtype2=1-viewtype1;
    TourCB(VIEW1);
    break;
  case VIEW1:
    viewtype2 = 1 - viewtype1;
    if(viewtype1==1&&edittour==1){
      if(selected_frame!=NULL){
        XYZView2AzElev(selected_frame);
      }
    }
    if(selected_frame!=NULL){
      selected_frame->viewtype=viewtype1;
    }
    CreateTourPaths();
    break;
  case VIEW_times:
    ReallocTourMemory();
    CreateTourPaths();
    UpdateTimes();
    break;
  case KEYFRAME_viewXYZ:
    if(selected_frame!=NULL){
      if(selected_tour-tourinfo==0)dirtycircletour=1;
      selected_tour->startup=0;
      xyz_view = selected_frame->nodeval.xyz_view_abs;
      NORMALIZE_XYZ(xyz_view,tour_view_xyz);

      if(update_tour_path==1)CreateTourPaths();
      selected_frame->selected=1;
    }
    break;
  case KEYFRAME_tXYZ:
    if(selected_frame!=NULL){
      show_tour_hint = 0;
      if(selected_tour-tourinfo==0)dirtycircletour=1;
      selected_tour->startup=0;
      eye = selected_frame->nodeval.eye;
      xyz_view = selected_frame->nodeval.xyz_view_abs;

      if(tour_constant_vel==0){
        selected_frame->noncon_time=tour_ttt;
        selected_frame->disp_time=tour_ttt;
      }
      NORMALIZE_XYZ(eye,tour_xyz);
      if(viewtype1==REL_VIEW){
      }
      selected_frame->nodeval.elev_path=tour_elev_path;

      selected_frame->viewtype=viewtype1;
      selected_frame->nodeval.zoom=tour_zoom;
      NORMALIZE_XYZ(xyz_view,tour_view_xyz);
      if(update_tour_path==1)CreateTourPaths();
      selected_frame->selected=1;
      if(viewtype1==ABS_VIEW){
        TourCB(KEYFRAME_viewXYZ);
      }
    }
    break;
  case KEYFRAME_NEXT:
    show_tour_hint = 0;
    if(selected_frame==NULL&&tourinfo!=NULL){
      selected_frame=&(tourinfo[0].first_frame);
      selected_tour=tourinfo;
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
    SetGluiTourKeyframe();
    break;
  case KEYFRAME_PREVIOUS:
    show_tour_hint = 0;
    if(selected_frame==NULL&&tourinfo!=NULL){
      selected_frame=&(tourinfo[0].last_frame);
      selected_tour=tourinfo;
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
    SetGluiTourKeyframe();
    break;
  case KEYFRAME_INSERT:
    show_tour_hint = 0;
    if(selected_frame!=NULL){
      thistour=selected_tour;
      thiskey=selected_frame;
      nextkey=thiskey->next;
      if(nextkey==&thistour->last_frame){
        lastkey=thiskey->prev;
        key_xyz[0]=DENORMALIZE_X(2*thiskey->nodeval.eye[0]-lastkey->nodeval.eye[0]);
        key_xyz[1]=DENORMALIZE_Y(2*thiskey->nodeval.eye[1]-lastkey->nodeval.eye[1]);
        key_xyz[2]=DENORMALIZE_Z(2*thiskey->nodeval.eye[2]-lastkey->nodeval.eye[2]);
        key_az_path = (2*thiskey->az_path-lastkey->az_path);
        key_elev_path=(2*thiskey->nodeval.elev_path-lastkey->nodeval.elev_path);
        key_time_in = thiskey->noncon_time;
        thiskey->noncon_time=(thiskey->noncon_time+lastkey->noncon_time)/2.0;
        key_view[0]=DENORMALIZE_X(2*thiskey->nodeval.xyz_view_abs[0]-lastkey->nodeval.xyz_view_abs[0]);
        key_view[1]=DENORMALIZE_Y(2*thiskey->nodeval.xyz_view_abs[1]-lastkey->nodeval.xyz_view_abs[1]);
        key_view[2]=DENORMALIZE_Z(2*thiskey->nodeval.xyz_view_abs[2]-lastkey->nodeval.xyz_view_abs[2]);
        key_zoom = (2*thiskey->nodeval.zoom - lastkey->nodeval.zoom);
        viewtype1=thiskey->viewtype;
        viewtype2=1-viewtype1;
      }
      else{
        key_xyz[0]=DENORMALIZE_X((thiskey->nodeval.eye[0]+nextkey->nodeval.eye[0])/2.0);
        key_xyz[1]=DENORMALIZE_Y((thiskey->nodeval.eye[1]+nextkey->nodeval.eye[1])/2.0);
        key_xyz[2]=DENORMALIZE_Z((thiskey->nodeval.eye[2]+nextkey->nodeval.eye[2])/2.0);
        key_az_path = (thiskey->az_path+nextkey->az_path)/2.0;
        key_elev_path=(thiskey->nodeval.elev_path+nextkey->nodeval.elev_path)/2.0;
        key_time_in = (thiskey->noncon_time+nextkey->noncon_time)/2.0;
        key_view[0]=DENORMALIZE_X((thiskey->nodeval.xyz_view_abs[0]+nextkey->nodeval.xyz_view_abs[0])/2.0);
        key_view[1]=DENORMALIZE_Y((thiskey->nodeval.xyz_view_abs[1]+nextkey->nodeval.xyz_view_abs[1])/2.0);
        key_view[2]=DENORMALIZE_Z((thiskey->nodeval.xyz_view_abs[2]+nextkey->nodeval.xyz_view_abs[2])/2.0);
        key_zoom = (thiskey->nodeval.zoom + nextkey->nodeval.zoom)/2.0;
        if(thiskey->viewtype==REL_VIEW&&nextkey->viewtype==REL_VIEW){
          viewtype1=REL_VIEW;
        }
        else{
          viewtype1=ABS_VIEW;
          if(thiskey->viewtype==ABS_VIEW){
            //DENORMALIZE_XYZ(key_view,thiskey->nodeval.xyz_view_abs);
            key_elev_path = thiskey->nodeval.elev_path;
          }
          if(thiskey->viewtype==REL_VIEW&&nextkey->viewtype==ABS_VIEW){
            //DENORMALIZE_XYZ(key_view,nextkey->nodeval.xyz_view_abs);
            key_elev_path = nextkey->nodeval.elev_path;
          }
        }
        viewtype2=1-viewtype1;
      }
      newframe=AddFrame(selected_frame,key_time_in,key_xyz,key_az_path,key_elev_path,viewtype1,key_zoom,key_view);
      CreateTourPaths();
      NewSelect(newframe);
      SetGluiTourKeyframe();
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
        if(thistour!=NULL)DeleteTour(thistour-tourinfo);
      }
    }
    break;
  case TOUR_AVATAR:
    if(selected_tour->glui_avatar_index!=glui_avatar_index){
      selected_tour->glui_avatar_index=glui_avatar_index;
// hack to make touring avatar show up
      avatar_types[glui_avatar_index]->visible=1;
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
      iavatar_types=glui_avatar_index;
    }
    break;
  case TOUR_UPDATELABEL:
    if(selectedtour_index>=0&&selectedtour_index<ntourinfo){
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
      SetGluiTourKeyframe();
      break;
    case TOURINDEX_MANUAL:
      edittour=0;
      TOURMENU(MENU_TOUR_CLEARALL);  // reset tour vis to ini values
      if(PANEL_node != NULL)PANEL_node->disable();
      if(PANEL_tournavigate!=NULL)PANEL_tournavigate->disable();
      if (SPINNER_t != NULL)SPINNER_t->disable();
      break;
    case TOURINDEX_DEFAULT:
      TOURMENU(MENU_TOUR_DEFAULT);  // default tour
      break;
    default:
      selected_tour=tourinfo + selectedtour_index;
      selected_frame=selected_tour->first_frame.next;
      selected_tour->display=0;
      TOURMENU(selectedtour_index);
      SetGluiTourKeyframe();
      if(PANEL_node != NULL)PANEL_node->enable();
      if(PANEL_tournavigate!=NULL)PANEL_tournavigate->enable();
      if (SPINNER_t != NULL)SPINNER_t->disable();
      break;
    }
    DeleteTourList();
    CreateTourList();
    UpdateViewTour();
    UpdateTourControls();
    selectedtour_index_old=selectedtour_index;
    break;
  case TOUR_DELETE:
    DeleteTour(selectedtour_index);
    break;
  case TOUR_REVERSE:
    if(selectedtour_index>=0&&selectedtour_index<ntourinfo){
      ReverseTour(tourinfo[selectedtour_index].label);
    }
    break;
  case TOUR_INSERT_NEW:
  case TOUR_INSERT_COPY:
    if(var==TOUR_INSERT_NEW){
      thistour=AddTour(NULL);
    }
    else{
      if(selectedtour_index>=0&&selectedtour_index<ntourinfo){
        char label[300];

        strcpy(label, tourinfo[selectedtour_index].label);
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
    selectedtour_index = thistour - tourinfo;
    selectedtour_index_old=selectedtour_index;
    SetGluiTourKeyframe();
    CreateTourPaths();
    UpdateViewTour();
    UpdateTourControls();
    selected_tour->display=0;
    TOURMENU(selectedtour_index);
    updatemenu=1;
    break;
  case TOUR_LABEL:
    if(thistour!=NULL){
      strcpy(thistour->label,tour_label);
      SetGluiTourKeyframe();
      if(LISTBOX_tour!=NULL){
        LISTBOX_tour->delete_item(thistour-tourinfo);
        LISTBOX_tour->add_item(thistour-tourinfo,thistour->label);
      }
      UpdateTourMenuLabels();
      updatemenu=1;
    }
    break;
  case TOUR_HIDE:
    if(thistour!=NULL){
      if(tour_hide==1){
        thistour->display=1;
        TOURMENU(thistour-tourinfo);
        NextTour();
        SetGluiTourKeyframe();
        thistour->display=0;
      }
      else{
        thistour->display=1;
      }
      updatemenu=1;
      DeleteTourList();
      CreateTourList();
      UpdateViewTour();
      UpdateTourControls();
    }
    break;
  default:
    ASSERT(FFALSE);
  }
}

/* ------------------ DeleteTourList ------------------------ */

extern "C" void DeleteTourList(void){
  int i;

  if(LISTBOX_tour==NULL)return;
  for(i=0;i<ntourinfo;i++){
    LISTBOX_tour->delete_item(i);
  }
  DeleteVolTourList(); //xx comment this line if smokebot fails with seg fault
}

/* ------------------ CreateTourList ------------------------ */

extern "C" void CreateTourList(void){
  int i;

  if(LISTBOX_tour==NULL)return;
  for(i=0;i<ntourinfo;i++){
    tourdata *touri;
    char label[1000];

    touri = tourinfo + i;
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
  if(selectedtour_index>=-1&&selectedtour_index<ntourinfo)LISTBOX_tour->set_int_val(selectedtour_index);

  CreateVolTourList(); //xx comment this line if smokebot fails with seg fault
}

/* ------------------ UpdateTourControls ------------------------ */

extern "C" void UpdateTourControls(void){

  if(BUTTON_next_tour==NULL)return;
  if(BUTTON_prev_tour==NULL)return;
  if(ROLLOUT_keyframe==NULL)return;
  if(SPINNER_x==NULL)return;
  if(CHECKBOX_showtourroute1 != NULL)CHECKBOX_showtourroute1->set_int_val(edittour);
  if(CHECKBOX_showtourroute2 != NULL)CHECKBOX_showtourroute2->set_int_val(edittour);
  if(CHECKBOX_view1!=NULL)CHECKBOX_view1->set_int_val(viewtourfrompath);
  if(CHECKBOX_view2!=NULL)CHECKBOX_view2->set_int_val(viewtourfrompath);
  if(ntourinfo>1){
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
    selectedtour_index = selected_tour-tourinfo;
    LISTBOX_tour->set_int_val(selectedtour_index);
  }
  else{
    selectedtour_index = TOURINDEX_MANUAL;
    LISTBOX_tour->set_int_val(selectedtour_index);
  }
}
