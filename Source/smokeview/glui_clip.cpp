#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_motion.h"

GLUI *glui_clip=NULL;

GLUI_RadioGroup *radio_clip=NULL;

GLUI_Spinner *SPINNER_clip_xmax=NULL, *SPINNER_clip_xmin=NULL;
GLUI_Spinner *SPINNER_clip_ymax=NULL, *SPINNER_clip_ymin=NULL;
GLUI_Spinner *SPINNER_clip_zmax=NULL, *SPINNER_clip_zmin=NULL;

GLUI_Checkbox *CHECKBOX_clip_xmin=NULL, *CHECKBOX_clip_xmax=NULL;
GLUI_Checkbox *CHECKBOX_clip_ymin=NULL, *CHECKBOX_clip_ymax=NULL;
GLUI_Checkbox *CHECKBOX_clip_zmin=NULL, *CHECKBOX_clip_zmax=NULL;
GLUI_Checkbox *CHECKBOX_clip_rotate = NULL;
GLUI_Checkbox *CHECKBOX_clip_show_rotation_center2 = NULL;

GLUI_Panel *PANEL_clip_lower=NULL, *PANEL_clip_upper=NULL, *PANEL_clip=NULL,*panel_wrapup=NULL;
GLUI_Panel *PANEL_clipx=NULL, *PANEL_clipX=NULL;
GLUI_Panel *PANEL_clipy=NULL, *PANEL_clipY=NULL;
GLUI_Panel *PANEL_clipz=NULL, *PANEL_clipZ=NULL;
GLUI_Panel *PANEL_rotation_center = NULL;

GLUI_RadioButton *RADIOBUTTON_clip_1a=NULL;
GLUI_RadioButton *RADIOBUTTON_clip_1b=NULL;
GLUI_RadioButton *RADIOBUTTON_clip_1c=NULL;

GLUI_Button *BUTTON_clip_1=NULL;
GLUI_Button *BUTTON_clip_2=NULL;

#define CLIP_xlower 0
#define CLIP_ylower 1
#define CLIP_zlower 2
#define CLIP_xupper 3
#define CLIP_yupper 4
#define CLIP_zupper 5
#define CLIP_ROTATE 6
#define CLIP_SHOW_ROTATE2 7

#define CLIP_all 12

#define SPINNER_xlower 13
#define SPINNER_ylower 14
#define SPINNER_zlower 15
#define SPINNER_xupper 16
#define SPINNER_yupper 17
#define SPINNER_zupper 18

#define INI_VALS -1
#define DEFAULT_VALS -2

#define CLIP_CLOSE 99
#define SAVE_SETTINGS_CLIP 98
#define CLIP_MESH 80

/* ------------------ GLUIUpdateShowRotationCenter2 ------------------------ */

extern "C" void GLUIUpdateShowRotationCenter2(void){
  if(CHECKBOX_clip_show_rotation_center2!=NULL)CHECKBOX_clip_show_rotation_center2->set_int_val(show_rotation_center);
}

/* ------------------ ClipCB ------------------------ */

void ClipCB(int var){
  meshdata *mesh0;
  float dx, dy, dz;

  mesh0 = global_scase.meshescoll.meshinfo;
  dx = MAX(0.1, mesh0->boxeps_fds[0]);
  dy = MAX(0.1, mesh0->boxeps_fds[1]);
  dz = MAX(0.1, mesh0->boxeps_fds[2]);

  glutPostRedisplay();
  switch(var){
  case CLIP_ROTATE:
    if(clip_rotate==0){
      UpdateGluiRotateAbout(ROTATE_ABOUT_FDS_CENTER);
    }
    else{
      UpdateGluiRotateAbout(ROTATE_ABOUT_CLIPPING_CENTER);
    }
    break;
  case CLIP_SHOW_ROTATE2:
    GLUIUpdateShowRotationCenter();
    break;
  case CLIP_MESH:
    if(global_scase.clip_mesh == 0){
      SetClipControls(DEFAULT_VALS);
    }
    else{
      SetClipControls(global_scase.clip_mesh);
    }
    break;
  case SAVE_SETTINGS_CLIP:
    WriteIni(LOCAL_INI, NULL);
    break;
  case CLIP_CLOSE:
    GLUIHideClip();
    break;
  case CLIP_xlower:
    if(clipinfo.clip_xmin == 0)SPINNER_clip_xmin->disable();
    if(clipinfo.clip_xmin == 1)SPINNER_clip_xmin->enable();
    updatefacelists = 1;
    break;
  case CLIP_ylower:
    if(clipinfo.clip_ymin == 0)SPINNER_clip_ymin->disable();
    if(clipinfo.clip_ymin == 1)SPINNER_clip_ymin->enable();
    updatefacelists = 1;
    break;
  case CLIP_zlower:
    if(clipinfo.clip_zmin == 0)SPINNER_clip_zmin->disable();
    if(clipinfo.clip_zmin == 1)SPINNER_clip_zmin->enable();
    updatefacelists = 1;
    break;
  case CLIP_xupper:
    if(clipinfo.clip_xmax == 0)SPINNER_clip_xmax->disable();
    if(clipinfo.clip_xmax == 1)SPINNER_clip_xmax->enable();
    updatefacelists = 1;
    break;
  case CLIP_yupper:
    if(clipinfo.clip_ymax == 0)SPINNER_clip_ymax->disable();
    if(clipinfo.clip_ymax == 1)SPINNER_clip_ymax->enable();
    updatefacelists = 1;
    break;
  case CLIP_zupper:
    if(clipinfo.clip_zmax == 0)SPINNER_clip_zmax->disable();
    if(clipinfo.clip_zmax == 1)SPINNER_clip_zmax->enable();
    updatefacelists = 1;
    break;
  case CLIP_all:
    updatefacelists = 1;
    UpdateClipPlanes();
    if(clip_mode != CLIP_OFF){
      int i;

      for(i = 0;i < 6;i++){
        ClipCB(i);
      }
      CHECKBOX_clip_xmin->enable();
      CHECKBOX_clip_ymin->enable();
      CHECKBOX_clip_zmin->enable();
      CHECKBOX_clip_xmax->enable();
      CHECKBOX_clip_ymax->enable();
      CHECKBOX_clip_zmax->enable();
      global_scase.updatefaces = 1;
    }
    else{
      SPINNER_clip_xmin->disable();
      SPINNER_clip_ymin->disable();
      SPINNER_clip_zmin->disable();
      SPINNER_clip_xmax->disable();
      SPINNER_clip_ymax->disable();
      SPINNER_clip_zmax->disable();

      CHECKBOX_clip_xmin->disable();
      CHECKBOX_clip_ymin->disable();
      CHECKBOX_clip_zmin->disable();
      CHECKBOX_clip_xmax->disable();
      CHECKBOX_clip_ymax->disable();
      CHECKBOX_clip_zmax->disable();
      global_scase.updatefaces = 1;
    }
 // draw both sides of blockage faces if clipping is on or if the case has removeable blokcages
    if(clip_mode != CLIP_OFF || global_scase.have_removable_obsts == 1){
      show_bothsides_blockages = 1;
    }
    else{
      show_bothsides_blockages = 0;
    }
    break;
  case SPINNER_xlower:
    if(clipinfo.xmin < xbar0ORIG) {
      clipinfo.xmin = xbar0ORIG - dx;
      SPINNER_clip_xmin->set_float_val(clipinfo.xmin);
    }
    updatefacelists = 1;
    break;
  case SPINNER_xupper:
    if(clipinfo.xmax > xbarORIG) {
      clipinfo.xmax = xbarORIG + dx;
      SPINNER_clip_xmax->set_float_val(clipinfo.xmax);
    }
    updatefacelists = 1;
    break;
  case SPINNER_ylower:
    if(clipinfo.ymin < ybar0ORIG) {
      clipinfo.ymin = ybar0ORIG - dy;
      SPINNER_clip_ymin->set_float_val(clipinfo.ymin);
    }
    updatefacelists = 1;
    break;
  case SPINNER_yupper:
    if(clipinfo.ymax > ybarORIG) {
      clipinfo.ymax = ybarORIG + dy;
      SPINNER_clip_ymax->set_float_val(clipinfo.ymax);
    }
    updatefacelists = 1;
    break;
  case SPINNER_zlower:
    if(clipinfo.zmin < zbar0ORIG) {
      clipinfo.zmin = zbar0ORIG - dz;
      SPINNER_clip_zmin->set_float_val(clipinfo.zmin);
    }
    updatefacelists = 1;
    break;
  case SPINNER_zupper:
    if(clipinfo.zmax > zbarORIG) {
      clipinfo.zmax = zbarORIG + dz;
      SPINNER_clip_zmax->set_float_val(clipinfo.zmax);
    }
    updatefacelists = 1;
    break;
  default:
    assert(FFALSE);
    break;
  }
  switch(var){
  case SPINNER_xlower:
  case SPINNER_xupper:
  case SPINNER_ylower:
  case SPINNER_yupper:
  case SPINNER_zlower:
  case SPINNER_zupper:
  case CLIP_xlower:
  case CLIP_ylower:
  case CLIP_zlower:
  case CLIP_xupper:
  case CLIP_yupper:
  case CLIP_zupper:
  case CLIP_all:
    camera_current->clip_mode = clip_mode;
    camera_current->clip_xmin = clipinfo.clip_xmin;
    camera_current->clip_xmax = clipinfo.clip_xmax;
    camera_current->clip_ymin = clipinfo.clip_ymin;
    camera_current->clip_ymax = clipinfo.clip_ymax;
    camera_current->clip_zmin = clipinfo.clip_zmin;
    camera_current->clip_zmax = clipinfo.clip_zmax;
    camera_current->xmin = clipinfo.xmin;
    camera_current->xmax = clipinfo.xmax;
    camera_current->ymin = clipinfo.ymin;
    camera_current->ymax = clipinfo.ymax;
    camera_current->zmin = clipinfo.zmin;
    camera_current->zmax = clipinfo.zmax;
    break;
  case CLIP_CLOSE:
  case CLIP_ROTATE:
  case CLIP_SHOW_ROTATE2:
  case SAVE_SETTINGS_CLIP:
    break;
  default:
    assert(FFALSE);
    break;
  }
  if(glui_rotation_index==ROTATE_ABOUT_CLIPPING_CENTER)GLUIUpdateRotationIndex(ROTATE_ABOUT_CLIPPING_CENTER);
}

/* ------------------ SetClipControls ------------------------ */

void SetClipControls(int val){
  int i;

  for(i = 0;i < 6;i++){
    ClipCB(i);
  }
  if(val == DEFAULT_VALS){
    clipinfo.xmin = xclip_min;
    clipinfo.ymin = yclip_min;
    clipinfo.zmin = zclip_min;
    clipinfo.xmax = xclip_max;
    clipinfo.ymax = yclip_max;
    clipinfo.zmax = zclip_max;
  }
  if(val >= 1 && val <= global_scase.meshescoll.nmeshes){
    meshdata *meshi;
    float *xplt, *yplt, *zplt;

    float dxclip, dyclip, dzclip;

    dxclip = (xbarORIG - xbar0ORIG) / 1000.0;
    dyclip = (ybarORIG - ybar0ORIG) / 1000.0;
    dzclip = (zbarORIG - zbar0ORIG) / 1000.0;

    meshi = global_scase.meshescoll.meshinfo + val - 1;

    xplt = meshi->xplt_fds;
    yplt = meshi->yplt_fds;
    zplt = meshi->zplt_fds;

    clipinfo.xmin = xplt[0] - dxclip;
    clipinfo.ymin = yplt[0] - dyclip;
    clipinfo.zmin = zplt[0] - dzclip;
    clipinfo.xmax = xplt[meshi->ibar] + dxclip;
    clipinfo.ymax = yplt[meshi->jbar] + dyclip;
    clipinfo.zmax = zplt[meshi->kbar] + dzclip;
  }
  SPINNER_clip_xmin->set_float_val(clipinfo.xmin);
  SPINNER_clip_ymin->set_float_val(clipinfo.ymin);
  SPINNER_clip_zmin->set_float_val(clipinfo.zmin);
  SPINNER_clip_xmax->set_float_val(clipinfo.xmax);
  SPINNER_clip_ymax->set_float_val(clipinfo.ymax);
  SPINNER_clip_zmax->set_float_val(clipinfo.zmax);
}

/* ------------------ GLUIClipSetup ------------------------ */

extern "C" void GLUIClipSetup(int main_window){
  if(glui_clip!=NULL){
    glui_clip->close();
    glui_clip=NULL;
  }
  glui_clip = GLUI_Master.create_glui("Clipping",0,dialogX0,dialogY0);
  glui_clip->hide();

  PANEL_clip = glui_clip->add_panel("",GLUI_PANEL_NONE);
  PANEL_clip_lower = glui_clip->add_panel_to_panel(PANEL_clip,_("Clip lower"));
  PANEL_clipx = glui_clip->add_panel_to_panel(PANEL_clip_lower,"X",GLUI_PANEL_NONE);
  SPINNER_clip_xmin=glui_clip->add_spinner_to_panel(PANEL_clipx,"X",GLUI_SPINNER_FLOAT,&clipinfo.xmin,SPINNER_xlower,ClipCB);
  glui_clip->add_column_to_panel(PANEL_clipx,false);
  CHECKBOX_clip_xmin=glui_clip->add_checkbox_to_panel(PANEL_clipx,"",&clipinfo.clip_xmin,CLIP_xlower,ClipCB);

  PANEL_clipy = glui_clip->add_panel_to_panel(PANEL_clip_lower,"Y",GLUI_PANEL_NONE);
  SPINNER_clip_ymin=glui_clip->add_spinner_to_panel(PANEL_clipy,"Y",GLUI_SPINNER_FLOAT,&clipinfo.ymin,SPINNER_ylower,ClipCB);
  glui_clip->add_column_to_panel(PANEL_clipy,false);
  CHECKBOX_clip_ymin=glui_clip->add_checkbox_to_panel(PANEL_clipy,"",&clipinfo.clip_ymin,CLIP_ylower,ClipCB);

  PANEL_clipz = glui_clip->add_panel_to_panel(PANEL_clip_lower,"Z",GLUI_PANEL_NONE);
  SPINNER_clip_zmin=glui_clip->add_spinner_to_panel(PANEL_clipz,"Z",GLUI_SPINNER_FLOAT,&clipinfo.zmin,SPINNER_zlower,ClipCB);
  glui_clip->add_column_to_panel(PANEL_clipz,false);
  CHECKBOX_clip_zmin=glui_clip->add_checkbox_to_panel(PANEL_clipz,"",&clipinfo.clip_zmin,CLIP_zlower,ClipCB);

  radio_clip = glui_clip->add_radiogroup_to_panel(PANEL_clip,&clip_mode,CLIP_all,ClipCB);
  RADIOBUTTON_clip_1a=glui_clip->add_radiobutton_to_group(radio_clip,_("Clipping disabled"));
  RADIOBUTTON_clip_1b=glui_clip->add_radiobutton_to_group(radio_clip,_("Clip blockages and data"));
  RADIOBUTTON_clip_1c=glui_clip->add_radiobutton_to_group(radio_clip,_("Clip blockages"));
  RADIOBUTTON_clip_1c=glui_clip->add_radiobutton_to_group(radio_clip,_("Clip data"));
  assert(CLIP_MAX==3);

  glui_clip->add_column_to_panel(PANEL_clip, false);

  PANEL_clip_upper = glui_clip->add_panel_to_panel(PANEL_clip,_("Clip upper"));

  PANEL_clipX = glui_clip->add_panel_to_panel(PANEL_clip_upper,"X",GLUI_PANEL_NONE);
  SPINNER_clip_xmax=glui_clip->add_spinner_to_panel(PANEL_clipX,"X",GLUI_SPINNER_FLOAT,&clipinfo.xmax,SPINNER_xupper,ClipCB);
  glui_clip->add_column_to_panel(PANEL_clipX,false);
  CHECKBOX_clip_xmax=glui_clip->add_checkbox_to_panel(PANEL_clipX,"",&clipinfo.clip_xmax,CLIP_xupper,ClipCB);

  PANEL_clipY = glui_clip->add_panel_to_panel(PANEL_clip_upper,"Y",GLUI_PANEL_NONE);
  SPINNER_clip_ymax=glui_clip->add_spinner_to_panel(PANEL_clipY,"Y",GLUI_SPINNER_FLOAT,&clipinfo.ymax,SPINNER_yupper,ClipCB);
  glui_clip->add_column_to_panel(PANEL_clipY,false);
  CHECKBOX_clip_ymax=glui_clip->add_checkbox_to_panel(PANEL_clipY,"",&clipinfo.clip_ymax,CLIP_yupper,ClipCB);

  PANEL_clipZ = glui_clip->add_panel_to_panel(PANEL_clip_upper,"Z",GLUI_PANEL_NONE);
  SPINNER_clip_zmax=glui_clip->add_spinner_to_panel(PANEL_clipZ,"Z",GLUI_SPINNER_FLOAT,&clipinfo.zmax,SPINNER_zupper,ClipCB);
  glui_clip->add_column_to_panel(PANEL_clipZ,false);
  CHECKBOX_clip_zmax=glui_clip->add_checkbox_to_panel(PANEL_clipZ,"",&clipinfo.clip_zmax,CLIP_zupper,ClipCB);

  PANEL_rotation_center = glui_clip->add_panel_to_panel(PANEL_clip, "rotation center");
  CHECKBOX_clip_rotate = glui_clip->add_checkbox_to_panel(PANEL_rotation_center, "center of clipping planes", &clip_rotate, CLIP_ROTATE, ClipCB);
  CHECKBOX_clip_show_rotation_center2 = glui_clip->add_checkbox_to_panel(PANEL_rotation_center, "Show", &show_rotation_center, CLIP_SHOW_ROTATE2, ClipCB);

  panel_wrapup = glui_clip->add_panel_to_panel(PANEL_clip,"",GLUI_PANEL_NONE);

  glui_clip->add_column_to_panel(panel_wrapup,false);

  BUTTON_clip_1=glui_clip->add_button_to_panel(panel_wrapup,_("Save settings"),SAVE_SETTINGS_CLIP,ClipCB);

  glui_clip->add_column_to_panel(panel_wrapup,false);

  BUTTON_clip_2=glui_clip->add_button_to_panel(panel_wrapup,_("Close"),CLIP_CLOSE,ClipCB);
#ifdef pp_CLOSEOFF
  BUTTON_clip_2->disable();
#endif

  if(updateclipvals==1){
    SetClipControls(INI_VALS);  // clip vals from ini file
  }
  else{
    if(global_scase.clip_mesh==0){
      SetClipControls(DEFAULT_VALS);  // clip vals from global scene
    }
    else{
      SetClipControls(global_scase.clip_mesh);  // clip vals from mesh clip_mesh
    }
  }

  glui_clip->set_main_gfx_window( main_window );
}

/* ------------------ GLUIHideClip ------------------------ */

extern "C" void GLUIHideClip(void){
  GLUICloseRollouts(glui_clip);
}

/* ------------------ GLUIShowClip ------------------------ */

extern "C" void GLUIShowClip(void){
  if(glui_clip != NULL){
    glui_clip->show();
  }
}

/* ------------------ GLUIUpdateClip ------------------------ */

extern "C" void GLUIUpdateClip(void){
  if(CHECKBOX_clip_xmin!=NULL&&CHECKBOX_clip_ymin!=NULL&&CHECKBOX_clip_zmin!=NULL&&
     CHECKBOX_clip_xmax!=NULL&&CHECKBOX_clip_ymax!=NULL&&CHECKBOX_clip_zmax!=NULL){

// set clipping min and max clipping plane checkboxes

    CHECKBOX_clip_xmin->set_int_val(clipinfo.clip_xmin);
    CHECKBOX_clip_ymin->set_int_val(clipinfo.clip_ymin);
    CHECKBOX_clip_zmin->set_int_val(clipinfo.clip_zmin);
    CHECKBOX_clip_xmax->set_int_val(clipinfo.clip_xmax);
    CHECKBOX_clip_ymax->set_int_val(clipinfo.clip_ymax);
    CHECKBOX_clip_zmax->set_int_val(clipinfo.clip_zmax);

// set clipping min and max clipping plane values

    SPINNER_clip_xmin->set_float_val(clipinfo.xmin);
    SPINNER_clip_xmax->set_float_val(clipinfo.xmax);
    SPINNER_clip_ymin->set_float_val(clipinfo.ymin);
    SPINNER_clip_ymax->set_float_val(clipinfo.ymax);
    SPINNER_clip_zmin->set_float_val(clipinfo.zmin);
    SPINNER_clip_zmax->set_float_val(clipinfo.zmax);

    if(radio_clip!=NULL)radio_clip->set_int_val(clip_mode);
    ClipCB(CLIP_all);
  }
}

/* ------------------ GLUIUpdateClipAll ------------------------ */

extern "C" void GLUIUpdateClipAll(void){
  ClipCB(CLIP_all);
  radio_clip->set_int_val(clip_mode);
}
