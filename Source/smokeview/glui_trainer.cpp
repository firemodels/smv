#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "glui_motion.h"

#define LOAD_SMOKE 100
#define LOAD_TEMP 101
#define LOAD_OXY 102

#define MOVETYPE 200
#define TRAINERPATH 201
#define TRAINER_PAUSE 212
#define TRAINER_LEFTRIGHT_INOUT 221
#define TRAINER_UPDOWN 223
#define TRAINER_AZ_ELEV 231

#define TRAINERVIEWPOINTS 300
#define MENU_OUTLINEVIEW -104

GLUI *glui_trainer=NULL;

GLUI_Checkbox *CHECKBOX_pause=NULL;

GLUI_Listbox *LIST_trainerpath=NULL,*LIST_viewpoint=NULL;

GLUI_Panel *PANEL_smokeview=NULL;
GLUI_Panel *PANEL_explore=NULL;
GLUI_Panel *PANEL_manual=NULL;//, *PANEL_automatic=NULL;
GLUI_Panel *PANEL_move=NULL,*PANEL_rotate2=NULL;

GLUI_Button *BUTTON_smoke3d=NULL, *BUTTON_temp=NULL, *BUTTON_oxy=NULL, *BUTTON_unload=NULL;

GLUI_Translation *TRANSLATE_updown=NULL,*TRANSLATE_leftright_inout=NULL;
GLUI_Translation *TRANSLATE_az_elev=NULL;

GLUI_StaticText *STATIC_alert=NULL;

GLUI *glui_alert=NULL;

/* ------------------ GLUISetViewListManual ------------------------ */

extern "C" void GLUISetViewListManual(void){
  if(trainer_viewpoints!=-2){
    LIST_viewpoint->set_int_val(-2);
  }
}

/* ------------------ GLUIShowAlert ------------------------ */

extern "C" void GLUIShowAlert(void){
  if(glui_alert!=NULL)glui_alert->show();
}

/* ------------------ GLUIHideAlert ------------------------ */

extern "C" void GLUIHideAlert(void){
  if(glui_alert!=NULL)glui_alert->hide();
}

/* ------------------ GLUIAlertSetup ------------------------ */

extern "C" void GLUIAlertSetup(int main_window){
  if(glui_alert!=NULL){
    glui_alert->close();
    glui_alert=NULL;
  }
  glui_alert = GLUI_Master.create_glui("",0,screenWidth/2,screenHeight/2);
  glui_alert->hide();
  STATIC_alert = glui_alert->add_statictext("Loading smoke and fire data");
}

/* ------------------ GLUIHideTrainer ------------------------ */

extern "C" void GLUIHideTrainer(void){
  GLUICloseRollouts(glui_trainer);
  showtrainer_dialog=0;
}

/* ------------------ GLUIShowTrainer ------------------------ */

extern "C" void GLUIShowTrainer(void){
  if(glui_trainer!=NULL){
    glui_trainer->show();
    showtrainer_dialog=1;
    updatemenu=1;
  }
}

/* ------------------ GLUIUpdateTrainerMoves ------------------------ */

extern "C" void GLUIUpdateTrainerMoves(void){
  float *eye_xyz;
  float *az, *elev;

  az = camera_current->az_elev;
  elev = camera_current->az_elev+1;

  eye_xyz = camera_current->eye;

  if(TRANSLATE_leftright_inout!=NULL){
    TRANSLATE_leftright_inout->set_x(eye_xyz[0]);
    TRANSLATE_leftright_inout->set_y(eye_xyz[1]);
    TRANSLATE_leftright_inout->set_speed(1.0/(float)screenWidth);
  }

  if(TRANSLATE_updown!=NULL){
    TRANSLATE_updown->set_x(eye_xyz[2]);
    TRANSLATE_updown->set_speed(1.0/(float)screenHeight);
  }

  if(TRANSLATE_az_elev!=NULL){
    TRANSLATE_az_elev->set_x(*az);
    TRANSLATE_az_elev->set_speed(180.0/(float)screenHeight);
    TRANSLATE_az_elev->set_y(*elev);
  }
}

/* ------------------ TrainerCB ------------------------ */

void TrainerCB(int var){
  int i;

  switch(var){
  case TRAINER_PAUSE:
    stept = trainer_pause;
    Keyboard('t', FROM_SMOKEVIEW);
    break;
  case TRAINERVIEWPOINTS:
    ResetDefaultMenu(trainer_viewpoints);
    break;
  case TRAINERPATH:
    TrainerCB(MOVETYPE);
    if(trainer_viewpoints != -2){
      LIST_viewpoint->set_int_val(-2);
    }
    switch(trainer_path){
    case -1:
      if(trainer_path_old != -1){
        trainer_pause = 0;
        CHECKBOX_pause->set_int_val(trainer_pause);
        TourMenu(MENU_TOUR_MANUAL);
        rotation_type = ROTATION_2AXIS;
        HandleRotationType(ROTATION_2AXIS);
        from_glui_trainer = 1;
        trainee_location = 0;
      }
      TrainerCB(MOVETYPE);
      break;
    case -2:
      break;
    default:
      if(rotation_type != EYE_CENTERED){
        rotation_type = EYE_CENTERED;
        HandleRotationType(ROTATION_2AXIS);
      }
      for(i = 0;i < global_scase.tourcoll.ntourinfo;i++){
        tourdata *touri;

        touri = global_scase.tourcoll.tourinfo + i;
        touri->display = 0;
      }
      viewtourfrompath = 1;
      TourMenu(trainer_path);
    }
    trainer_path_old = trainer_path;
    break;
  case MOVETYPE:
    rotation_type = ROTATION_2AXIS;
    HandleRotationType(ROTATION_2AXIS);
    SetViewPoint(RESTORE_EXTERIOR_VIEW);
    break;
  case LOAD_SMOKE:
    TrainerViewMenu(MENU_TRAINER_smoke);
    break;
  case LOAD_TEMP:
    // kind of a hack, having to put in code seg twice, but this is required to get data chopping to work
    if(slicebounds != NULL&&slicefile_labelindex != -1){
      if(glui_setslicechopmin == 1 || glui_setslicechopmax == 1){
        SliceBounds2Glui(slicefile_labelindex);
      }
    }
    TrainerViewMenu(MENU_TRAINER_temp);
    UpdateChopColors();
    if(slicebounds != NULL&&slicefile_labelindex != -1){
      if(glui_setslicechopmin == 1 || glui_setslicechopmax == 1){
        SliceBounds2Glui(slicefile_labelindex);
      }
    }
    UpdateChopColors();
    colorbar_flip = 1;
    ColorbarMenu(COLORBAR_FLIP);
    break;
  case LOAD_OXY:
    // kind of a hack, having to put in code seg twice, but this is required to get data chopping to work
    if(slicebounds != NULL&&slicefile_labelindex != -1){
      if(glui_setslicechopmin == 1 || glui_setslicechopmax == 1){
        SliceBounds2Glui(slicefile_labelindex);
      }
    }
    TrainerViewMenu(MENU_TRAINER_oxy);
    UpdateChopColors();
    if(slicebounds != NULL&&slicefile_labelindex != -1){
      if(glui_setslicechopmin == 1 || glui_setslicechopmax == 1){
        SliceBounds2Glui(slicefile_labelindex);
      }
    }
    UpdateChopColors();
    colorbar_flip = 0;
    ColorbarMenu(COLORBAR_FLIP);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ RotateCB ------------------------ */

void RotateCB(int var){

  float *eye_xyz, *az, *elev;

  eye_xyz = camera_current->eye;
  az = camera_current->az_elev;
  elev = camera_current->az_elev + 1;


  if(rotation_type != ROTATION_2AXIS){
    rotation_type = ROTATION_2AXIS;
    HandleRotationType(ROTATION_2AXIS);
    SetViewPoint(RESTORE_EXTERIOR_VIEW);
  }

  if(trainer_viewpoints != -2){
    LIST_viewpoint->set_int_val(-2);
  }
  if(trainer_path != -1){
    LIST_trainerpath->set_int_val(-1);
    TrainerCB(TRAINERPATH);
  }
  switch(var){
  case TRAINER_AZ_ELEV:
    *az = TRANSLATE_az_elev->get_x();
    *elev = -TRANSLATE_az_elev->get_y();
    break;
  case TRAINER_LEFTRIGHT_INOUT:
    eye_xyz[0] = TRANSLATE_leftright_inout->get_x();
    eye_xyz[1] = TRANSLATE_leftright_inout->get_y();
    break;
  case TRAINER_UPDOWN:
    eye_xyz[2] = TRANSLATE_updown->get_y();
    break;
  default:
    assert(FFALSE);
    break;
  }
  camera_current->dirty = 1;
}

/* ------------------ GLUITrainerSetup ------------------------ */

extern "C" void GLUITrainerSetup(int main_window){

  if(glui_trainer!=NULL){
    glui_trainer->close();
    glui_trainer=NULL;
  }
  if(glui_trainer!=NULL)glui_trainer->close();
  glui_trainer = GLUI_Master.create_glui("Demonstrator",0,screenWidth+12,0);
  if(showgluitrainer==0)glui_trainer->hide();

  glui_trainer->set_main_gfx_window( main_window );
  PANEL_smokeview = glui_trainer->add_panel("Data");
  BUTTON_smoke3d = glui_trainer->add_button_to_panel(PANEL_smokeview,"Smoke/Fire",LOAD_SMOKE,TrainerCB);
  if(AnySmoke()==0)BUTTON_smoke3d->disable();
  BUTTON_temp = glui_trainer->add_button_to_panel(PANEL_smokeview,"Temperature",LOAD_TEMP,TrainerCB);
  if(AnySlices("TEMPERATURE")==0)BUTTON_temp->disable();
  BUTTON_oxy = glui_trainer->add_button_to_panel(PANEL_smokeview,"Oxygen",LOAD_OXY,TrainerCB);
  if(AnySlices("oxygen")==0&&AnySlices("oxygen VOLUME FRACTION")==0){
    BUTTON_oxy->disable();
  }

  PANEL_explore = glui_trainer->add_panel("Explore",true);

  trainer_path=-1;
  LIST_trainerpath = glui_trainer->add_listbox_to_panel(PANEL_explore,"Path:",&trainer_path,TRAINERPATH,TrainerCB);
  {
    int i;
    LIST_trainerpath->add_item(-1,"Manual");
    LIST_trainerpath->add_item(-2,"-");
    for(i=0;i<global_scase.tourcoll.ntourinfo;i++){
      tourdata *touri;

      touri = global_scase.tourcoll.tourinfo + i;
      LIST_trainerpath->add_item(i,touri->menulabel);
    }
  }

  trainer_viewpoints = -2;
  LIST_viewpoint = glui_trainer->add_listbox_to_panel(PANEL_explore,"Viewpoint:",&trainer_viewpoints,TRAINERVIEWPOINTS,TrainerCB);
  {
    cameradata *ca;

    for(ca=camera_list_first.next;ca->next!=NULL;ca=ca->next){
      if(strcmp(ca->name,"external")!=0){
        LIST_viewpoint->add_item(ca->view_id, ca->name);
      }
    }
  }
  LIST_viewpoint->set_int_val(trainer_viewpoints);
  CHECKBOX_pause = glui_trainer->add_checkbox_to_panel(PANEL_explore,"Pause",&trainer_pause,TRAINER_PAUSE,TrainerCB);

  PANEL_move = glui_trainer->add_panel_to_panel(PANEL_explore,"Move",false);
  TRANSLATE_leftright_inout = glui_trainer->add_translation_to_panel(PANEL_move,"Horizontal",
    GLUI_TRANSLATION_XY,trainer_xzy,TRAINER_LEFTRIGHT_INOUT, RotateCB);
  glui_trainer->add_column_to_panel(PANEL_move,false);

  TRANSLATE_updown = glui_trainer->add_translation_to_panel(PANEL_move,"Vertical",
    GLUI_TRANSLATION_Y,trainer_xzy+2,TRAINER_UPDOWN, RotateCB);
  glui_trainer->add_column_to_panel(PANEL_move,false);

  TRANSLATE_az_elev = glui_trainer->add_translation_to_panel(PANEL_move,"Rotate",
    GLUI_TRANSLATION_XY,trainer_ab,TRAINER_AZ_ELEV, RotateCB);

  GLUIUpdateTrainerMoves();

  TrainerCB(MOVETYPE);
  TrainerCB(TRAINERVIEWPOINTS);

}
