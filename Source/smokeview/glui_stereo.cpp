#define CPP
#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_motion.h"

GLUI *glui_stereo=NULL;

GLUI_Panel *PANEL_stereo_method=NULL;
GLUI_Panel *PANEL_stereo_options = NULL;

GLUI_RadioGroup *RADIO_stereotype=NULL;
GLUI_RadioGroup *RADIO_stereotype_frame=NULL;

GLUI_RadioButton *RADIOBUTTON_seq=NULL;
GLUI_RadioButton *RADIOBUTTON_1=NULL;
GLUI_RadioButton *RADIOBUTTON_2=NULL;
GLUI_RadioButton *RADIOBUTTON_3=NULL;
GLUI_RadioButton *RADIOBUTTON_4=NULL;
GLUI_RadioButton *RADIOBUTTON_5=NULL;

GLUI_Spinner *SPINNER_zero_parallax=NULL, *SPINNER_right_green2=NULL, *SPINNER_right_blue2=NULL;

GLUI_Button *BUTTON_stereo_1=NULL;
GLUI_Button *BUTTON_stereo_2=NULL;
GLUI_Button *BUTTON_stereo_3=NULL;

#define STEREO_CLOSE 0
#define STEREO_RESET 2
#define STEREO_SHOW  4
#define STEREO_GREEN 5
#define STEREO_BLUE  6
#define SAVE_SETTINGS_STEREO 999

/* ------------------ GLUIUpdateStereo ------------------------ */

extern "C" void GLUIUpdateStereo(void){
  if(RADIO_stereotype!=NULL){
    RADIO_stereotype->set_int_val(stereotype);
  }
  if(stereotypeOLD==3&&stereotype!=3){
    if(setbw!=setbwSAVE){
      setbw=1-setbwSAVE;
      ColorbarMenu(COLORBAR_TOGGLE_BW);
    }
  }
  else if(stereotypeOLD!=3&&stereotype==3){
    setbwSAVE=setbw;
    if(setbw==0){
      setbwSAVE=setbw;
      ColorbarMenu(COLORBAR_TOGGLE_BW);
    }
  }
}

/* ------------------ StereoCB ------------------------ */

void StereoCB(int var){

  switch(var){
  case STEREO_GREEN:
    // right_blue=1.0-right_green;
    // SPINNER_right_blue2->set_float_val(right_blue);
    break;
  case STEREO_BLUE:
    // right_green=1.0-right_blue;
    // SPINNER_right_green2->set_float_val(right_green);
    break;
  case STEREO_SHOW:
    if(stereotypeOLD != stereotype){
      GLUIUpdateStereo();
      stereotypeOLD = stereotype;
    }
    if(stereotype == STEREO_CUSTOM){
      SPINNER_right_blue2->enable();
      SPINNER_right_green2->enable();
    }
    else{
      SPINNER_right_blue2->disable();
      SPINNER_right_green2->disable();
    }
    break;
  case STEREO_RESET:
    SPINNER_zero_parallax->set_float_val(SCALE2FDS(0.25));
    break;
  case STEREO_CLOSE:
    GLUIHideStereo();
    break;
  case SAVE_SETTINGS_STEREO:
    WriteIni(LOCAL_INI, NULL);
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GLUIStereoSetup ------------------------ */

extern "C" void GLUIStereoSetup(int main_window){
  if(have_vr==1){
    glui_stereo = GLUI_Master.create_glui("Stereo/VR settings", 0, dialogX0, dialogY0);
  }
  else{
    glui_stereo = GLUI_Master.create_glui("Stereo settings", 0, dialogX0, dialogY0);
  }
  glui_stereo->hide();

  PANEL_stereo_method = glui_stereo->add_panel(_("Stereo Method"));
  RADIO_stereotype = glui_stereo->add_radiogroup_to_panel(PANEL_stereo_method,&stereotype,STEREO_SHOW,StereoCB);
  RADIOBUTTON_1=glui_stereo->add_radiobutton_to_group(RADIO_stereotype,_("Off"));
  RADIOBUTTON_seq=glui_stereo->add_radiobutton_to_group(RADIO_stereotype,_("Successive frames"));
  if(videoSTEREO==0)RADIOBUTTON_seq->disable();
  RADIOBUTTON_2=glui_stereo->add_radiobutton_to_group(RADIO_stereotype,_("Left/Right"));
  RADIOBUTTON_3=glui_stereo->add_radiobutton_to_group(RADIO_stereotype,_("Red/Blue"));
  RADIOBUTTON_4=glui_stereo->add_radiobutton_to_group(RADIO_stereotype,_("Red/Cyan"));
  RADIOBUTTON_5=glui_stereo->add_radiobutton_to_group(RADIO_stereotype,_("Custom Red/Custom Blue"));
  SPINNER_right_green2=glui_stereo->add_spinner_to_panel(PANEL_stereo_method,_("green"),GLUI_SPINNER_FLOAT,&right_green,STEREO_GREEN,StereoCB);
  SPINNER_right_blue2= glui_stereo->add_spinner_to_panel(PANEL_stereo_method, _("blue"),GLUI_SPINNER_FLOAT,&right_blue,STEREO_BLUE,StereoCB);

  SPINNER_right_green2->set_float_limits(0.0,1.0,GLUI_LIMIT_CLAMP);
  SPINNER_right_blue2->set_float_limits(0.0,1.0,GLUI_LIMIT_CLAMP);

  PANEL_stereo_options = glui_stereo->add_panel(_("Stereo Options"));
  fzero=SCALE2FDS(fzero);
  SPINNER_zero_parallax=glui_stereo->add_spinner_to_panel(PANEL_stereo_options,_("Distance to zero parallax plane (m)"),GLUI_SPINNER_FLOAT,&fzero);
  glui_stereo->add_checkbox_to_panel(PANEL_stereo_options, "Show stereo parallax",&show_parallax);
  RADIO_stereotype_frame = glui_stereo->add_radiogroup_to_panel(PANEL_stereo_options, &stereotype_frame);
  glui_stereo->add_radiobutton_to_group(RADIO_stereotype_frame,_("Left eye"));
  glui_stereo->add_radiobutton_to_group(RADIO_stereotype_frame,_("Right eye"));
  glui_stereo->add_radiobutton_to_group(RADIO_stereotype_frame,_("Both eyes"));
  //SPINNER_zero_parallax->set_float_limits(0.1*xyzmaxdiff,2.0*xyzmaxdiff,GLUI_LIMIT_CLAMP);
  StereoCB(STEREO_SHOW);
  GLUIUpdateStereo();

  BUTTON_stereo_1=glui_stereo->add_button(_("Reset"),STEREO_RESET,StereoCB);
  BUTTON_stereo_2=glui_stereo->add_button(_("Save settings"),SAVE_SETTINGS_STEREO,StereoCB);
  BUTTON_stereo_3=glui_stereo->add_button(_("Close"),STEREO_CLOSE,StereoCB);
#ifdef pp_CLOSEOFF
  BUTTON_stereo_3->disable();
#endif

  glui_stereo->set_main_gfx_window( main_window );
}

/* ------------------ GLUIHideStereo ------------------------ */

extern "C" void GLUIHideStereo(void){
  GLUICloseRollouts(glui_stereo);
}

/* ------------------ GLUIShowStereo ------------------------ */

extern "C" void GLUIShowStereo(void){
  if(glui_stereo!=NULL){
    glui_stereo->show();
  }
}
