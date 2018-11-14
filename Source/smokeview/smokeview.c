#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "glew.h"
#include GLUT_H

#include "smokeviewvars.h"

#ifdef WIN32
#include <direct.h>
#endif

/* ------------------ _Sniff_Errors ------------------------ */

void _Sniff_Errors(char *whereat, char *file, int line){
  int error;

  while((error=glGetError())!=GL_NO_ERROR){
    char *glu_error;

    glu_error=(char *)gluErrorString((unsigned int)error);
    fprintf(stderr,"\n*** Error: OpenGL error:%s, %s \n file:%s line: %i\n",
      glu_error,whereat,file,line);
  }
}

/* ------------------ UpdateLights ------------------------ */

void UpdateLights(float *pos1, float *pos2){
  int i;
  GLfloat ambientlight2[4], diffuselight2[4];

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SINGLE_COLOR);

  if(use_light0==1||use_light1==1){
    float num_lights;

    num_lights = use_light0 + use_light1;
    for(i = 0;i<3;i++){
      ambientlight2[i] = ambientlight[i]/num_lights;
      diffuselight2[i] = diffuselight[i]/num_lights;
    }
    ambientlight2[3] = 1.0;
    diffuselight2[3] = 1.0;
  }

  if(use_light0==1){
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuselight2);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientlight2);
    glLightfv(GL_LIGHT0, GL_POSITION, pos1);
    glEnable(GL_LIGHT0);
  }
  else{
    glDisable(GL_LIGHT0);
  }

  if(use_light1==1){
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuselight2);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambientlight2);
    glLightfv(GL_LIGHT1, GL_POSITION, pos2);
    glEnable(GL_LIGHT1);
  }
  else{
    glDisable(GL_LIGHT1);
  }
}

/* ------------------ AntiAliasSurface ------------------------ */

void AntiAliasSurface(int flag){
  if(antialiasflag==ON&&1==0){ // disable this routine for now
    if(flag==ON){
//      glBlendFunc( GL_SRC_ALPHA_SATURATE, GL_ONE );
      glEnable( GL_BLEND );
      glEnable( GL_POLYGON_SMOOTH );
    }
    if(flag==OFF){
      glDisable(GL_BLEND);
      glDisable( GL_POLYGON_SMOOTH );
    }
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
  case RESTORE_INTERIOR_VIEW:
    rotation_type_save = camera_current->rotation_type;
    projection_type_save = camera_current->projection_type;
    CopyCamera(camera_current,camera_internal);
    camera_current->rotation_type=rotation_type_save;
    camera_current->projection_type=projection_type_save;
    break;
  case RESTORE_SAVED_VIEW:
    CopyCamera(camera_current,camera_save);
    break;
  case 3:
  case 4:
    ASSERT(FFALSE);
    break;
  default:
    ASSERT(FFALSE);
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
  UpdateGluiZoom();
}

/* ------------------ InitVolrenderScript ------------------------ */

void InitVolrenderScript(char *prefix, char *tour_label, int startframe, int skipframe){
  scriptfiledata *sfd;
  FILE *script_stream;

  if(volrender_scriptname==NULL){
    int len;

    len = strlen(fdsprefix)+strlen("_volrender.ssf")+1;
    NewMemory((void **)&volrender_scriptname,(unsigned int)(len));
    STRCPY(volrender_scriptname,fdsprefix);
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
  PRINTVERSION(progname,prog_fullpath);
  PRINTF("\n");
  if(fds_version!=NULL){
    PRINTF("FDS Build        : %s\n",fds_githash);
  }
  if(smokeviewpath!=NULL){
    PRINTF("Smokeview        : %s\n",smokeviewpath);
  }
  if(smokezippath!=NULL){
    PRINTF("Smokezip         : %s\n",smokezippath);
  }
  if(texturedir!=NULL){
    PRINTF("Texture directory: %s\n",texturedir);
  }
}

