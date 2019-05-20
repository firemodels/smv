#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "update.h"
#include "smokeviewvars.h"

GLUI *glui_shooter=NULL;

GLUI_Rollout *ROLLOUT_shooter_frame=NULL;
GLUI_Rollout *ROLLOUT_shooter_misc=NULL;
GLUI_Rollout *ROLLOUT_shooter_velocity = NULL;

GLUI_Panel *PANEL_shooter_frameA=NULL;
GLUI_Panel *PANEL_shooter_frameB=NULL;
GLUI_Panel *PANEL_shooter_frameC=NULL;
GLUI_Panel *PANEL_shooter_frameD=NULL;
GLUI_Panel *PANEL_shooter_frameE=NULL;
GLUI_Panel *PANEL_shooter_frameF=NULL;
GLUI_Panel *PANEL_shooter_frameG=NULL;
GLUI_Panel *PANEL_shooter_frameH=NULL;
GLUI_Panel *PANEL_shooter_win = NULL;

GLUI_RadioGroup *RADIO_shooter_vel_type=NULL;

GLUI_RadioButton *RADIOBUTTON_plot3dtype=NULL;
GLUI_RadioButton *RADIOBUTTON_shooter_1=NULL;

GLUI_Spinner *SPINNER_shooter_x=NULL;
GLUI_Spinner *SPINNER_shooter_y=NULL;
GLUI_Spinner *SPINNER_shooter_z=NULL;
GLUI_Spinner *SPINNER_shooter_u=NULL;
GLUI_Spinner *SPINNER_shooter_v=NULL;
GLUI_Spinner *SPINNER_shooter_w=NULL;
GLUI_Spinner *SPINNER_shooter_dx=NULL;
GLUI_Spinner *SPINNER_shooter_dy=NULL;
GLUI_Spinner *SPINNER_shooter_dz=NULL;
GLUI_Spinner *SPINNER_shooter_nparts=NULL;
GLUI_Spinner *SPINNER_shooter_fps=NULL;
GLUI_Spinner *SPINNER_shooter_veldir=NULL;
GLUI_Spinner *SPINNER_shooter_v_inf=NULL;
GLUI_Spinner *SPINNER_shooter_u0=NULL;
GLUI_Spinner *SPINNER_shooter_z0=NULL;
GLUI_Spinner *SPINNER_shooter_p=NULL;
GLUI_Spinner *SPINNER_shooter_duration=NULL;
GLUI_Spinner *SPINNER_shooter_history=NULL;
GLUI_Spinner *SPINNER_shooter_1=NULL;

GLUI_Button *BUTTON_shooter_loadplot3d=NULL;
GLUI_Button *BUTTON_shooter_1=NULL;
GLUI_Button *BUTTON_shooter_2=NULL;
GLUI_Button *BUTTON_shooter_3=NULL;

GLUI_Checkbox *CHECKBOX_shooter_1=NULL;
GLUI_Checkbox *CHECKBOX_shooter_2=NULL;
GLUI_Checkbox *CHECKBOX_shooter_3=NULL;

GLUI_Listbox *LIST_shooter_times=NULL;


#define SHOOTER_VEL_TYPE 101
#define SHOOTER_APPLY 102
#define SHOOTER_DURATION 103
#define SHOOTER_FPS 104
#define SHOOTER_NPARTS 105
#define SHOOTER_HISTORY 106
#define SHOOTER_XYZ 107
#define SHOOTER_DXYZ 108
#define SHOOTER_VEL 109
#define SHOOTER_SHOW 110
#define SHOOTER_FIRSTFRAME 111
#define SHOOTER_TIME 112
#define SHOOTER_LOADPLOT3D 113
#define SHOOTER_UVW 114
#define SHOOTER_TERMINAL_VEL 115

#define SAVE_SETTINGS 900
#define SHOOTER_CLOSE 901

#define START_SHOOTER_ROLLOUT 0
#define BACKGROUND_SHOOTER_ROLLOUT 1
#define MISC_SHOOTER_ROLLOUT 2

procdata shooterprocinfo[3];
int nshooterprocinfo = 0;

/* ------------------ ShooterRolloutCB ------------------------ */

void ShooterRolloutCB(int var){
  ToggleRollout(shooterprocinfo, nshooterprocinfo, var);
}

/* ------------------ HideGluiShooter ------------------------ */

extern "C" void HideGluiShooter(void){
  CloseRollouts(glui_shooter);
}

/* ------------------ ShowGluiShooter ------------------------ */

extern "C" void ShowGluiShooter(void){
  if(glui_shooter!=NULL){
    glui_shooter->show();
    updatemenu=1;
  }
}

/* ------------------ AllocateShooter ------------------------ */

int  AllocateShooter(void){
  int mem_points, mem_frames;

  FREEMEMORY(shootpointinfo);
  FREEMEMORY(shoottimeinfo);

  mem_points = max_shooter_points * sizeof(shootpointdata);
  mem_frames = nshooter_frames * sizeof(shoottimedata);

  PRINTF("shooter point memory requirements\n");
  PRINTF("max_shooter_points=%i mem=%i\n", max_shooter_points, mem_points);
  PRINTF("nshooter_frames=%i mem=%i\n", nshooter_frames, mem_frames);

  if(mem_points <= 0 || mem_frames <= 0 ||
#ifdef _DEBUG
    mem_points >= 2000000000 || mem_frames > 2000000000 ||
#endif
    NewMemory((void **)&shootpointinfo, mem_points) == 0 ||
    NewMemory((void **)&shoottimeinfo, mem_frames) == 0){
    FREEMEMORY(shootpointinfo);
    FREEMEMORY(shoottimeinfo);
    shooter_active = 0;
    PRINTF("shooter point memory allocation failed\n");
    return 1;
  }
  return 0;

}

/* ------------------ ShooterCB ------------------------ */

void ShooterCB(int var){
  float ang;
  if(shooter_firstframe == 1){
    ResetItimes0();
  }
  switch(var){
  case SHOOTER_LOADPLOT3D:
    PRINTF("Loading PLOT3D data at time: %f\n", plot3dtimelist[shooter_itime]);
    Plot3DListMenu(shooter_itime);
    ShooterCB(SHOOTER_APPLY);
    break;
  case SHOOTER_TIME:
    break;
  case SHOOTER_FIRSTFRAME:
    break;
  case SHOOTER_SHOW:
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    UpdateTimes();
    break;
  case SHOOTER_TERMINAL_VEL:
    if(shooter_v_inf < 0.0){
      shooter_v_inf = 0.0;
      SPINNER_shooter_v_inf->set_float_val(shooter_v_inf);
    }
    if(shooter_cont_update == 1){
      ShooterCB(SHOOTER_APPLY);
    }
    break;
  case SHOOTER_VEL:
    ang = DEG2RAD*shooter_veldir;
    shooter_velz = 0.0;
    shooter_velx = shooter_u0*cos(ang);
    shooter_vely = shooter_u0*sin(ang);
    if(shooter_cont_update == 1){
      ShooterCB(SHOOTER_APPLY);
    }
    break;
  case SHOOTER_UVW:
  case SHOOTER_XYZ:
    if(shooter_active == 1){
      InitShooterData();
    }
    if(shooter_cont_update == 1){
      ShooterCB(SHOOTER_APPLY);
    }
    break;
  case SHOOTER_DXYZ:
    if(shooter_active == 1){
      InitShooterData();
    }
    if(shooter_cont_update == 1){
      ShooterCB(SHOOTER_APPLY);
    }
    break;
  case SHOOTER_NPARTS:
    if(shooter_nparts < 1 && SPINNER_shooter_nparts != NULL){
      shooter_nparts = 1;
      SPINNER_shooter_nparts->set_int_val(shooter_nparts);
    }
    if(shooter_cont_update == 1){
      ShooterCB(SHOOTER_APPLY);
    }
    break;
  case SHOOTER_FPS:
    if(shooter_fps < 1 && SPINNER_shooter_fps != NULL){
      shooter_fps = 1;
      SPINNER_shooter_fps->set_int_val(shooter_fps);
    }
    if(shooter_active == 1){
      InitShooterData();
    }
    if(shooter_cont_update == 1){
      ShooterCB(SHOOTER_APPLY);
    }
    break;
  case SHOOTER_HISTORY:
    break;
  case SHOOTER_DURATION:
    if(shooter_duration < 1.0&&SPINNER_shooter_duration != NULL){
      shooter_duration = 1.0;
      SPINNER_shooter_duration->set_float_val(shooter_duration);
    }
    if(shooter_cont_update == 1){
      ShooterCB(SHOOTER_APPLY);
    }
    break;
  case SHOOTER_APPLY:
    nshooter_frames = (int)(shooter_duration*shooter_fps);
    max_shooter_points = nshooter_frames*shooter_nparts;

    if(AllocateShooter() == 0){
      SolveShooterData();
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      UpdateTimes();
    }
    break;
  case SHOOTER_VEL_TYPE:
    if(shooter_vel_type == 1){
      SPINNER_shooter_u0->enable();
      SPINNER_shooter_z0->enable();
      SPINNER_shooter_p->enable();
      SPINNER_shooter_veldir->enable();
      if(LIST_shooter_times != NULL)LIST_shooter_times->disable();
      if(BUTTON_shooter_loadplot3d != NULL)BUTTON_shooter_loadplot3d->disable();
    }
    else{
      SPINNER_shooter_u0->disable();
      SPINNER_shooter_z0->disable();
      SPINNER_shooter_p->disable();
      SPINNER_shooter_veldir->disable();
      if(LIST_shooter_times != NULL)LIST_shooter_times->enable();
      if(BUTTON_shooter_loadplot3d != NULL)BUTTON_shooter_loadplot3d->enable();
    }
    if(shooter_cont_update == 1){
      ShooterCB(SHOOTER_APPLY);
    }
    break;
  case SAVE_SETTINGS:
    WriteIni(LOCAL_INI, NULL);
    break;
  case SHOOTER_CLOSE:
    HideGluiShooter();
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
}

/* ------------------ GluiShooterSetup ------------------------ */

extern "C" void GluiShooterSetup(int main_window){

  update_glui_shooter=0;
  if(glui_shooter!=NULL){
    glui_shooter->close();
    glui_shooter=NULL;
  }
  glui_shooter = GLUI_Master.create_glui(_("Particle tracking"),0,0,0 );
  glui_shooter->hide();

  ROLLOUT_shooter_frame = glui_shooter->add_rollout(_("Starting locations/velocities"), true, START_SHOOTER_ROLLOUT, ShooterRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_shooter_frame, glui_shooter);
  ADDPROCINFO(shooterprocinfo, nshooterprocinfo, ROLLOUT_shooter_frame, START_SHOOTER_ROLLOUT, glui_shooter);

  PANEL_shooter_frameE=glui_shooter->add_panel_to_panel(ROLLOUT_shooter_frame,_("Positions"),false);

  PANEL_shooter_frameA=glui_shooter->add_panel_to_panel(PANEL_shooter_frameE,_("Center"));
  glui_shooter->add_column_to_panel(PANEL_shooter_frameE,false);
  PANEL_shooter_frameB=glui_shooter->add_panel_to_panel(PANEL_shooter_frameE,_("Size"));

  SPINNER_shooter_x=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameA,"x",GLUI_SPINNER_FLOAT,shooter_xyz,SHOOTER_XYZ,ShooterCB);
  SPINNER_shooter_x->set_float_limits(xbar0,xbarORIG);

  SPINNER_shooter_y=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameA,"y",GLUI_SPINNER_FLOAT,shooter_xyz+1,SHOOTER_XYZ,ShooterCB);
  SPINNER_shooter_y->set_float_limits(ybar0,ybarORIG);

  SPINNER_shooter_z=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameA,"z",GLUI_SPINNER_FLOAT,shooter_xyz+2,SHOOTER_XYZ,ShooterCB);
  SPINNER_shooter_z->set_float_limits(zbar0,zbarORIG);

  SPINNER_shooter_dx=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameB,"dx",GLUI_SPINNER_FLOAT,shooter_dxyz,SHOOTER_DXYZ,ShooterCB);
  SPINNER_shooter_dx->set_float_limits(0.0,xbarORIG-xbar0);

  SPINNER_shooter_dy=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameB,"dy",GLUI_SPINNER_FLOAT,shooter_dxyz+1,SHOOTER_DXYZ,ShooterCB);
  SPINNER_shooter_dy->set_float_limits(0.0,ybarORIG-ybar0);

  SPINNER_shooter_dz=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameB,"dz",GLUI_SPINNER_FLOAT,shooter_dxyz+2,SHOOTER_DXYZ,ShooterCB);
  SPINNER_shooter_dz->set_float_limits(0.0,zbarORIG-zbar0);

  PANEL_shooter_frameF=glui_shooter->add_panel_to_panel(ROLLOUT_shooter_frame,_("Velocities"));
  SPINNER_shooter_u=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameF,"u",GLUI_SPINNER_FLOAT,shooter_uvw,SHOOTER_UVW,ShooterCB);
  SPINNER_shooter_v=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameF,"v",GLUI_SPINNER_FLOAT,shooter_uvw+1,SHOOTER_UVW,ShooterCB);
  SPINNER_shooter_w=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameF,"w",GLUI_SPINNER_FLOAT,shooter_uvw+2,SHOOTER_UVW,ShooterCB);

  ROLLOUT_shooter_velocity = glui_shooter->add_rollout(_("Background velocity field"), false, BACKGROUND_SHOOTER_ROLLOUT, ShooterRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_shooter_velocity, glui_shooter);
  ADDPROCINFO(shooterprocinfo, nshooterprocinfo, ROLLOUT_shooter_velocity,BACKGROUND_SHOOTER_ROLLOUT, glui_shooter);

  RADIO_shooter_vel_type=glui_shooter->add_radiogroup_to_panel(ROLLOUT_shooter_velocity,&shooter_vel_type,SHOOTER_VEL_TYPE,ShooterCB);
  RADIOBUTTON_plot3dtype=glui_shooter->add_radiobutton_to_group(RADIO_shooter_vel_type,"PLOT3D");
  RADIOBUTTON_shooter_1=glui_shooter->add_radiobutton_to_group(RADIO_shooter_vel_type,_("Power law"));
  if(nplot3dtimelist>0&&plot3dtimelist!=NULL){
  }
  else{
    shooter_vel_type=1;
    RADIO_shooter_vel_type->set_int_val(shooter_vel_type);
    RADIOBUTTON_plot3dtype->disable();
  }

  if(nplot3dtimelist>0&&plot3dtimelist!=NULL){
    int i;

    PANEL_shooter_frameC=glui_shooter->add_panel_to_panel(ROLLOUT_shooter_velocity,"PLOT3D");
    BUTTON_shooter_loadplot3d=glui_shooter->add_button_to_panel(PANEL_shooter_frameC,_("Load"),SHOOTER_LOADPLOT3D,ShooterCB);
    LIST_shooter_times = glui_shooter->add_listbox_to_panel(PANEL_shooter_frameC,_("Time:"),&shooter_itime,SHOOTER_TIME,ShooterCB);
    for(i=0;i<nplot3dtimelist;i++){
      char label[255];

      sprintf(label,"%f",plot3dtimelist[i]);
      TrimZeros(label);
      LIST_shooter_times->add_item(i,label);
    }
  }

  PANEL_shooter_frameD=glui_shooter->add_panel_to_panel(ROLLOUT_shooter_velocity,_("Power law"));
  SPINNER_shooter_u0=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameD,_("reference velocity, U0 (m/s)"),GLUI_SPINNER_FLOAT,&shooter_u0,SHOOTER_VEL,ShooterCB);
  SPINNER_shooter_z0=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameD,_("reference elevation, Z0 (m)"),GLUI_SPINNER_FLOAT,&shooter_z0,SHOOTER_VEL,ShooterCB);
  SPINNER_shooter_p=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameD,_("decay, p"),GLUI_SPINNER_FLOAT,&shooter_p,SHOOTER_VEL,ShooterCB);
  SPINNER_shooter_veldir=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameD,_("velocity direction (deg)"),GLUI_SPINNER_FLOAT,&shooter_veldir,SHOOTER_VEL,ShooterCB);
  SPINNER_shooter_veldir->set_float_limits(-180.0,180.0);

  ROLLOUT_shooter_misc = glui_shooter->add_rollout("Misc", false, MISC_SHOOTER_ROLLOUT, ShooterRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_shooter_misc, glui_shooter);
  ADDPROCINFO(shooterprocinfo, nshooterprocinfo, ROLLOUT_shooter_misc, MISC_SHOOTER_ROLLOUT, glui_shooter);

  PANEL_shooter_frameG = glui_shooter->add_panel_to_panel(ROLLOUT_shooter_misc, "", false);
  glui_shooter->add_column_to_panel(ROLLOUT_shooter_misc,false);
  PANEL_shooter_frameH=glui_shooter->add_panel_to_panel(ROLLOUT_shooter_misc,"",false);

  CHECKBOX_shooter_1=glui_shooter->add_checkbox_to_panel(PANEL_shooter_frameG,_("Show particles"),&visShooter,SHOOTER_SHOW,ShooterCB);
  CHECKBOX_shooter_2=glui_shooter->add_checkbox_to_panel(PANEL_shooter_frameG,_("Update continuously"),&shooter_cont_update);
  CHECKBOX_shooter_3=glui_shooter->add_checkbox_to_panel(PANEL_shooter_frameG,_("Show only first frame"),&shooter_firstframe,SHOOTER_FIRSTFRAME,ShooterCB);
  SPINNER_shooter_v_inf=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameG,_("terminal velocity"),GLUI_SPINNER_FLOAT,&shooter_v_inf,SHOOTER_TERMINAL_VEL,ShooterCB);
  BUTTON_shooter_1=glui_shooter->add_button_to_panel(PANEL_shooter_frameG,_("Compute tracks"),SHOOTER_APPLY,ShooterCB);

  SPINNER_shooter_1=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameH,_("Particle size"),GLUI_SPINNER_FLOAT,&shooterpointsize);
  SPINNER_shooter_nparts=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameH,_("number of particles"),GLUI_SPINNER_INT,&shooter_nparts,SHOOTER_NPARTS,ShooterCB);

  SPINNER_shooter_fps=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameH,_("frames per second"),GLUI_SPINNER_INT,&shooter_fps,SHOOTER_FPS,ShooterCB);

  SPINNER_shooter_duration=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameH,_("duration (s)"),GLUI_SPINNER_FLOAT,&shooter_duration,SHOOTER_DURATION,ShooterCB);
 // SPINNER_shooter_history=glui_shooter->add_spinner_to_panel(PANEL_shooter_frameH,"history (s)",
 //   GLUI_SPINNER_FLOAT,&shooter_history,SHOOTER_HISTORY,ShooterCB);
 // SPINNER_shooter_history->disable();

  ShooterCB(SHOOTER_NPARTS);
  ShooterCB(SHOOTER_FPS);
  ShooterCB(SHOOTER_DURATION);

  PANEL_shooter_win=glui_shooter->add_panel("",GLUI_PANEL_NONE);

  BUTTON_shooter_2=glui_shooter->add_button_to_panel(PANEL_shooter_win,_("Save settings"),SAVE_SETTINGS,ShooterCB);
  glui_shooter->add_column_to_panel(PANEL_shooter_win,false);
  BUTTON_shooter_3=glui_shooter->add_button_to_panel(PANEL_shooter_win,_("Close"),SHOOTER_CLOSE,ShooterCB);

  ShooterCB(SHOOTER_VEL_TYPE);
  ShooterCB(SHOOTER_VEL);

  glui_shooter->set_main_gfx_window( main_window );
}
