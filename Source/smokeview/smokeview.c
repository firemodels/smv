#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "glew.h"
#include GLUT_H

#include "smokeviewvars.h"
#include "glui_motion.h"
#include "IOscript.h"
#include "paths.h"

#ifdef WIN32
#include <direct.h>
#endif

/* ------------------ _Sniff_Errors ------------------------ */
#ifdef pp_SNIFF_ERROR
void _Sniff_Errors(const char *whereat, const char *file, int line){
  int error;

  while((error=glGetError())!=GL_NO_ERROR){
    char *glu_error;

    glu_error=(char *)gluErrorString((unsigned int)error);
    fprintf(stderr,"\n*** Error: OpenGL error:%s, %s \n file:%s line: %i\n",
      glu_error,whereat,file,line);
  }
}
#endif

/* ------------------ UpdateLights ------------------------ */

void UpdateLights(float *pos1, float *pos2){
  int i;
  GLfloat ambientlight2[4], diffuselight2[4], specularlight2[4];

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);

  if(use_light0==1||use_light1==1){
    float num_lights;

    num_lights = use_light0 + use_light1;
    for(i = 0;i<3;i++){
      ambientlight2[i] = ambientlight[i]/(float)num_lights;
      diffuselight2[i] = diffuselight[i]/(float)num_lights;
      specularlight2[i] = specularlight[i]/(float)num_lights;
    }
    ambientlight2[3] = 1.0;
    diffuselight2[3] = 1.0;
  }

  if(use_light0==1){
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuselight2);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientlight2);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularlight2);
    glLightfv(GL_LIGHT0, GL_POSITION, pos1);
    glEnable(GL_LIGHT0);
  }
  else{
    glDisable(GL_LIGHT0);
  }

  if(use_light1==1){
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuselight2);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambientlight2);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specularlight2);
    glLightfv(GL_LIGHT1, GL_POSITION, pos2);
    glEnable(GL_LIGHT1);
  }
  else{
    glDisable(GL_LIGHT1);
  }
}

/* ------------------ AntiAliasLine ------------------------ */

void AntiAliasLine(int flag){
  if(antialiasflag==1){
    if(flag==1){
      glEnable(GL_LINE_SMOOTH);
      glEnable(GL_BLEND);
      glEnable(GL_POINT_SMOOTH);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glHint(GL_LINE_SMOOTH_HINT,GL_DONT_CARE);
    }
    if(flag==0){
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_POINT_SMOOTH);
      glDisable(GL_BLEND);
    }
  }
}

/* ------------------ TransparentOn ------------------------ */

void TransparentOn(void){
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

/* ------------------ TransparentOff ------------------------ */

void TransparentOff(void){
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}


/* ------------------ SetViewPoint ------------------------ */

void SetViewPoint(int option){
  in_external=0;
  switch(option){
    int rotation_type_save;
    int projection_type_save;

  case RESTORE_EXTERIOR_VIEW_ZOOM:
    break;
  case RESTORE_EXTERIOR_VIEW:
    in_external=1;
    rotation_type_save = camera_current->rotation_type;
    projection_type_save = camera_current->projection_type;
    CopyCamera(camera_current,camera_external);
    camera_current->rotation_type=rotation_type_save;
    camera_current->projection_type=projection_type_save;
    if(camera_current->projection_type==PROJECTION_ORTHOGRAPHIC){
      camera_current->eye[1]=camera_current->isometric_y;
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
  if(rotation_type==ROTATION_3AXIS){
    float azimuth, elevation,axis[3];
    float quat_temp[4];
    float x, y, z;

    azimuth = camera_current->az_elev[0]*DEG2RAD;
    elevation = camera_current->az_elev[1]*DEG2RAD;

    x = cos(azimuth);
    y = sin(azimuth);
    z = cos(elevation);

    axis[0]=0.0;
    axis[1]=0.0;
    axis[2]=1.0;

    AngleAxis2Quat(azimuth,axis,quat_temp);

    axis[0]=x;
    axis[1]=y;
    axis[2]=0.0;

    AngleAxis2Quat(acos(z),axis,quat_general);

    MultQuat(quat_temp,quat_general,quat_general);

    Quat2Rot(quat_general,quat_rotation);
  }
  if(option==RESTORE_EXTERIOR_VIEW_ZOOM)camera_current->zoom=zooms[zoomindex];
  zoom=camera_current->zoom;
  GLUIUpdateZoom();
}

/* ------------------ InitVolrenderScript ------------------------ */

void InitVolrenderScript(char *prefix, char *tour_label, int startframe, int skipframe){
  scriptfiledata *sfd;
  FILE *script_stream;

  if(volrender_scriptname==NULL){
    int len;

    len = strlen(global_scase.fdsprefix)+strlen("_volrender.ssf")+1;
    NewMemory((void **)&volrender_scriptname,(unsigned int)(len));
    STRCPY(volrender_scriptname,global_scase.fdsprefix);
    STRCAT(volrender_scriptname,"_volrender.ssf");
  }

  sfd = InsertScriptFile(volrender_scriptname);
  if(sfd!=NULL)default_script=sfd;
  script_stream=fopen(volrender_scriptname,"w");
  if(script_stream!=NULL){
    fprintf(script_stream,"RENDERDIR\n");
    fprintf(script_stream," .\n");
    if(tour_label!=NULL&&strcmp(tour_label,"Manual")!=0){
      fprintf(script_stream,"LOADTOUR\n");
      fprintf(script_stream," %s\n",tour_label);
    }
    fprintf(script_stream,"VOLSMOKERENDERALL\n");
    fprintf(script_stream," %i %i\n",skipframe,startframe);
    fprintf(script_stream," %s\n",prefix);
    runscript=1;
    fclose(script_stream);
  }
}

/* ------------------ DisplayVersionInfo ------------------------ */

void DisplayVersionInfo(char *progname){
  PRINTVERSION(progname);
  if(global_scase.fds_version!=NULL){
    PRINTF("FDS Build        : %s\n",global_scase.fds_githash);
  }
  char *smv_progname = GetBinPath();
  PRINTF("Smokeview path   : %s\n",smv_progname);
  FREEMEMORY(smv_progname);
  if(verbose_output == 1){
#ifdef pp_COMPRESS
    if(smokezippath!=NULL)PRINTF("Smokezip path    : %s\n",smokezippath);
#endif
    if(global_scase.texturedir!=NULL)PRINTF("Texture directory: %s\n",global_scase.texturedir);
  }
  char *smv_bindir = GetSmvRootDir();
  if(smv_bindir){
    PRINTF("Root directory   : %s\n", smv_bindir);
  }
  char *global_ini_path = GetSystemIniPath();
  if(global_ini_path != NULL && FileExistsOrig(global_ini_path) == 1){
    PRINTF("Global ini       : %s\n", global_ini_path);
  }
  else{
    PRINTF("Global ini       : not found\n");
  }
  FREEMEMORY(global_ini_path);
  char *user_ini_path = GetUserIniPath();
  if(user_ini_path != NULL && FileExistsOrig(user_ini_path) == 1){
    PRINTF("User ini         : %s\n", user_ini_path);
  }
  FREEMEMORY(user_ini_path);

  char *objectfile = GetSmvRootFile("objects.svo");
  if(objectfile != NULL && FileExistsOrig(objectfile) == 1){
    PRINTF("objects.svo      : %s\n", objectfile);
  }
  else{
    PRINTF("objects.svo      : not found\n");
  }
  FREEMEMORY(objectfile);

  char fullini_filename[256];
  strcpy(fullini_filename, "");
  char *smokeview_scratchdir = GetUserConfigDir();
  char *caseini_filename = CasePathCaseIni(&global_scase);
  if(caseini_filename != NULL){
    if(FileExistsOrig(caseini_filename) == 1){
      char cwdpath[1000];
      GETCWD(cwdpath, 1000);
      strcpy(fullini_filename, cwdpath);
      strcat(fullini_filename, dirseparator);
      strcat(fullini_filename, caseini_filename);
    }
    else if(smokeview_scratchdir!=NULL){
      strcpy(fullini_filename, smokeview_scratchdir);
      strcat(fullini_filename, caseini_filename);
      if(FileExistsOrig(fullini_filename)==0)strcpy(fullini_filename, "");
    }
  }
  FREEMEMORY(caseini_filename);
  FREEMEMORY(smokeview_scratchdir);
  if(smv_filename != NULL || show_version == 0){
    if(strlen(fullini_filename) > 0){
      PRINTF("Casename ini     : %s\n", fullini_filename);
    }
    else{
      PRINTF("Casename ini     : not found\n");
    }
  }
  FREEMEMORY(smv_progname);
  FREEMEMORY(smv_bindir);
}

/* ------------------ IsFDSRunning ------------------------ */

int IsFDSRunning(FILE_SIZE *last_size){
  FILE_SIZE file_size;

  char *stepcsv_filename = CasePathStepCsv(&global_scase);
  file_size = GetFileSizeSMV(stepcsv_filename);
  FREEMEMORY(stepcsv_filename);
  if(file_size != *last_size){
    *last_size = file_size;
    return 1;
  }
  return 0;
}

/* ------------------ BuildGbndFile ------------------------ */

int BuildGbndFile(int file_type){
  char *stepcsv_filename = CasePathStepCsv(&global_scase);
  switch(file_type){
    case BOUND_SLICE:
      if(FileExistsOrig(slice_gbnd_filename)==0)return 1;
      if(IsFileNewer(stepcsv_filename, slice_gbnd_filename)==1)return 1;
      break;
    case BOUND_PATCH:
      if(FileExistsOrig(patch_gbnd_filename)==0)return 1;
      if(IsFileNewer(stepcsv_filename, patch_gbnd_filename)==1)return 1;
      break;
    case BOUND_PLOT3D:
      if(FileExistsOrig(plot3d_gbnd_filename)==0)return 1;
      if(IsFileNewer(stepcsv_filename, plot3d_gbnd_filename)==1)return 1;
      break;
    default:
      assert(FFALSE);
      break;
  }
  FREEMEMORY(stepcsv_filename);
  return 0;
}

/* ------------------ SMV_EXIT ------------------------ */

void SMV_EXIT(int code){
#ifdef _DEBUG_VS
  char buffer[255];

  printf("smokeview exiting\n");
  fgets(buffer, 255, stdin);
#endif
  exit(code);
}

/* ------------------ StartTimer ------------------------ */

void StartTimer(float *timerptr){
  float timer;

  START_TIMER(timer);
  *timerptr = timer;
}
