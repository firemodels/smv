#include "options.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


#include "smokeviewvars.h"
#include "glui_motion.h"
#include "IOscript.h"
#include "viewports.h"

/* ------------------ Zoom2Aperture ------------------------ */

float Zoom2Aperture(float zoom0){
  float ap;
  // note tan(46*PI/360)~(W/2)/D  where W=17==monitor width
  //                                D=20==distance from eye to monitor
  // (rounded off to 45 degrees)

  ap = 2.0*RAD2DEG*atan(tan(45.0*DEG2RAD/2.0)/zoom0);
  return ap;
}

/* ------------------ Aperture2Zoom ------------------------ */

float Aperture2Zoom(float ap){
  float zoom0;
  zoom0 = tan(45.0*DEG2RAD/2.0)/tan(ap*DEG2RAD/2.0);
  return zoom0;
}

/* ------------------ InitCameraList ------------------------ */

void InitCameraList(void){
  cameradata *cb, *ca;

  if(init_camera_list==0)return;
  init_camera_list = 0;

  cb=&camera_list_first;
  ca=&camera_list_last;
  InitCamera(cb,"first");
  InitCamera(ca,"last");

  cb->prev=NULL;
  cb->next=ca;

  ca->prev=cb;
  ca->next=NULL;
}

/* ------------------ AddDefaultViewpoints ------------------------ */

void AddDefaultViewpoints(void){
  cameradata *cfirst, *clast;
  int i;

  cfirst                = &camera_list_first;
  clast                 = cfirst->next;
  cfirst->next          = camera_external;
  camera_external->prev = cfirst;
  camera_external->next = clast;
  clast->prev           = camera_external;
  for(i = 0; i<6; i++){
    cameradata *ca, *cnext;

    ca           = camera_defaults[i];
    cnext        = cfirst->next;

    cfirst->next = ca;
    cnext->prev  = ca;
    ca->prev     = cfirst;
    ca->next     = cnext;
  }
}

/* ------------------ UpdateCameraYpos ------------------------ */

void UpdateCameraYpos(cameradata *ci, int option){
  float local_aperture_default;
  float width=1.0, height=1.0, asp=1.0, offset=0.0;
  float dx, dy, dz;

  if(update_saving_viewpoint>0)return;
  local_aperture_default = Zoom2Aperture(1.0);
  if(VP_scene.width==0||VP_scene.height==0)GetViewportInfo();
  if(VP_scene.height!=0)asp = (float)VP_scene.width/(float)VP_scene.height;


  if(use_geom_factors==1&&have_geom_factors==1){
    dx = FDS2SMV_X(geom_xmax) - FDS2SMV_X(geom_xmin);
    dy = FDS2SMV_Y(geom_ymax) - FDS2SMV_Y(geom_ymin);
    dz = FDS2SMV_Z(geom_zmax) - FDS2SMV_Z(geom_zmin);
    ci->xcen = FDS2SMV_X((geom_xmin+geom_xmax)/2.0);
    ci->ycen = FDS2SMV_Y((geom_ymin+geom_ymax)/2.0);
    ci->zcen = FDS2SMV_Z((geom_zmin+geom_zmax)/2.0);
  }
  else{
    dx = global_scase.xbar;
    dy = global_scase.ybar;
    dz = global_scase.zbar;
    ci->xcen = global_scase.xbar/2.0;
    ci->ycen = global_scase.ybar/2.0;
    ci->zcen = global_scase.zbar/2.0;
  }
  switch(option){
    case 1:
      if(asp>dy/dz){
        height = dz;
        width  = height*asp;
      }
      else{
        width = dy;
      }
      offset = (dx-dy)/2.0;
      break;

    case 2:
      if(asp>dx/dz){
        height = dz;
        width  = height*asp;
      }
      else{
        width = dx;
      }
      offset = 0.0;
      break;

    case 3:
      if(asp>dx/dy){
        height = dy;
        width  = height*asp;
      }
      else{
        width = dx;
      }
      offset = (dz-dy)/2.0;
      break;
    default:
      assert(FFALSE);
      break;
  }
  eyeyfactor = -(width/2.0)/tan(local_aperture_default*DEG2RAD/2.0) - offset;
  if(option==3&&use_geom_factors==1&&have_geom_factors==1){
    eyeyfactor *= 1.4;
  }
  else{
    eyeyfactor *= 1.05;
  }

  ci->eye[0] = ci->xcen;
  ci->eye[1] = eyeyfactor;
  ci->eye[2] = ci->zcen;
  if(viscolorbarpath==1){
    ci->eye[0] = 0.7;
    ci->eye[1] = -2.25;
    ci->eye[2] = 0.5;
  }
  ci->isometric_y = (eyeyfactor-1.0)*xyzbox;
}

/* ------------------ SetCameraViewPersp ------------------------ */

void SetCameraViewPersp(cameradata *ca, int option){
  float az = 0.0, elev = 0.0;

  switch(option){
    case MENU_VIEW_XMIN:
      az = 90.0;
      break;
    case MENU_VIEW_XMAX:
      az = -90.0;
      break;
    case MENU_VIEW_YMIN:
      az = 0.0;
      break;
    case MENU_VIEW_YMAX:
      az = 180.0;
      break;
    case MENU_VIEW_ZMIN:
      elev = -90.0;
      break;
    case MENU_VIEW_ZMAX:
      elev = 90.0;
      break;
    default:
      assert(FFALSE);
      break;
  }
  ca->az_elev[0] = az;
  ca->az_elev[1] = elev;
  ca->azimuth = az;
  ca->elevation = elev;
  ca->eye[0] = ca->xcen;
  ca->eye[2] = ca->zcen;

  switch(option){
    case MENU_VIEW_XMIN:
    case MENU_VIEW_XMAX:
      UpdateCameraYpos(ca, 1);
      break;
    case MENU_VIEW_YMIN:
    case MENU_VIEW_YMAX:
      UpdateCameraYpos(ca, 2);
      break;
    case MENU_VIEW_ZMIN:
    case MENU_VIEW_ZMAX:
      UpdateCameraYpos(ca, 3);
      break;
    default:
      assert(FFALSE);
      break;
  }
}

/* ------------------ SetCameraView ------------------------ */

void SetCameraView(cameradata *ca, int option){
  if(projection_type==PROJECTION_PERSPECTIVE){
    SetCameraViewPersp(ca, option);
  }
  else{
    ScriptViewXYZMINMAXOrtho(option);
  }
}

/* ------------------ InitCamera ------------------------ */

void InitCamera(cameradata *ci,char *name){
  strcpy(ci->name,name);
  ci->rotation_index=global_scase.meshescoll.nmeshes;
  ci->defined=1;
  ci->azimuth=0.0;
  ci->view_angle=0.0;
  ci->eye[0]=eyexfactor*global_scase.xbar;
  UpdateCameraYpos(ci, 2);
  ci->eye[2]=eyezfactor*global_scase.zbar;
  ci->eye_save[0]=ci->eye[0];
  ci->eye_save[1]=ci->eye[1];
  ci->eye_save[2]=ci->eye[2];
  ci->quat_defined=0;
  ci->quaternion[0]=1.0;
  ci->quaternion[0]=0.0;
  ci->quaternion[0]=0.0;
  ci->quaternion[0]=0.0;

  ci->az_elev[0]=0.0;
  ci->az_elev[1]=0.0;
  ci->up[0]=0.0;
  ci->up[1]=0.0;
  ci->up[2]=1.0;
  ci->view[0]=0.0;
  ci->view[1]=0.0;
  ci->view[2]=0.0;
  ci->xcen=global_scase.xbar/2.0;
  ci->ycen=global_scase.ybar/2.0;
  ci->zcen=global_scase.zbar/2.0;
  ci->rotation_type=rotation_type;

  ci->azimuth=0.0;
  ci->elevation=0.0;

  ci->view_angle=0.0;
  ci->next=NULL;
  ci->prev=NULL;
  ci->view_id=-1;
  ci->zoom=1.0;
  ci->projection_type=projection_type;
  ci->dirty=0;

  Clip2Cam(ci);
}

/* ------------------ Clip2Cam ------------------------ */

  void Clip2Cam(cameradata *cam){
    cam->clip_mode=clip_mode;
    cam->clip_xmin=clipinfo.clip_xmin;
    cam->clip_ymin=clipinfo.clip_ymin;
    cam->clip_zmin=clipinfo.clip_zmin;

    cam->clip_xmax=clipinfo.clip_xmax;
    cam->clip_ymax=clipinfo.clip_ymax;
    cam->clip_zmax=clipinfo.clip_zmax;

    cam->xmin=clipinfo.xmin;
    cam->ymin=clipinfo.ymin;
    cam->zmin=clipinfo.zmin;

    cam->xmax=clipinfo.xmax;
    cam->ymax=clipinfo.ymax;
    cam->zmax=clipinfo.zmax;
  }


/* ------------------ Clip2Cam ------------------------ */

  void Cam2Clip(cameradata *cam){
    clip_mode = cam->clip_mode;
    clipinfo.clip_xmin = cam->clip_xmin;
    clipinfo.clip_ymin = cam->clip_ymin;
    clipinfo.clip_zmin = cam->clip_zmin;

    clipinfo.clip_xmax = cam->clip_xmax;
    clipinfo.clip_ymax = cam->clip_ymax;
    clipinfo.clip_zmax = cam->clip_zmax;

    clipinfo.xmin = cam->xmin;
    clipinfo.ymin = cam->ymin;
    clipinfo.zmin = cam->zmin;

    clipinfo.xmax = cam->xmax;
    clipinfo.ymax = cam->ymax;
    clipinfo.zmax = cam->zmax;
    GLUIUpdateClip();

  }

/* ------------------ CopyViewCamera ------------------------ */

void CopyViewCamera(cameradata *to, cameradata *from){

  memcpy(to,from,sizeof(cameradata));
  if(to==camera_current){
    zoom=camera_current->zoom;
    GLUIUpdateZoom();
  }
  to->dirty=1;

  Cam2Clip(to);
  if(to==camera_current&&to->quat_defined==1){
    quat_general[0]=to->quaternion[0];
    quat_general[1]=to->quaternion[1];
    quat_general[2]=to->quaternion[2];
    quat_general[3]=to->quaternion[3];
  }
}

/* ------------------ CopyCamera ------------------------ */

void CopyCamera(cameradata *to, cameradata *from){

  memcpy(to,from,sizeof(cameradata));
  if(to==camera_current){
    zoom=camera_current->zoom;
    GLUIUpdateZoom();
  }
  to->dirty=1;
  if(to == camera_current && updateclipvals == 0){
    if(clip_mode==CLIP_OFF)Cam2Clip(camera_current);
  }
  if(to==camera_current&&to->quat_defined==1){
    quat_general[0]=to->quaternion[0];
    quat_general[1]=to->quaternion[1];
    quat_general[2]=to->quaternion[2];
    quat_general[3]=to->quaternion[3];
  }
}

/* ------------------ UpdateCamera ------------------------ */

void UpdateCamera(cameradata *ca){
  if(ca==camera_current){
    rotation_type=ca->rotation_type;
    if(ca->rotation_index>=0&&ca->rotation_index<global_scase.meshescoll.nmeshes){
      UpdateCurrentMesh(global_scase.meshescoll.meshinfo + ca->rotation_index);
    }
    else{
      UpdateCurrentMesh(global_scase.meshescoll.meshinfo);
    }
    highlight_mesh = current_mesh-global_scase.meshescoll.meshinfo;
    HandleRotationType(EYE_CENTERED);
    GLUIUpdateMeshList1(ca->rotation_index);
    GLUIUpdateTrainerMoves();

    ca->clip_mode=clip_mode;
    ca->clip_xmin=clipinfo.clip_xmin;
    ca->clip_ymin=clipinfo.clip_ymin;
    ca->clip_zmin=clipinfo.clip_zmin;

    ca->xmin=clipinfo.xmin;
    ca->ymin=clipinfo.ymin;
    ca->zmin=clipinfo.zmin;

    ca->xmax=clipinfo.xmax;
    ca->ymax=clipinfo.ymax;
    ca->zmax=clipinfo.zmax;
  }
  GLUISetPosXYZSMV(ca->eye);
  ca->dirty=0;
}

/* ------------------ CompareCameras ------------------------ */

#define IS_EXT 0
#define IS_OTHER 2
int CompareCameras(const void *arg1, const void *arg2){
  cameradata *x, *y;
  int x_state=IS_OTHER, y_state=IS_OTHER;

  x = *(cameradata **)arg1;
  y = *(cameradata **)arg2;

  /*
      ext  int   other
ext    0   -1     -1
int    1    0     -1
other  1    1    strcmp
  */

  if(strcmp(x->name, "external") == 0)x_state = IS_EXT;

  if(strcmp(y->name,"external") == 0)y_state = IS_EXT;

  if(x_state == IS_EXT){
    if(y_state == IS_EXT)return 0;
    return -1;
  }
  if(y_state == IS_OTHER)return strcmp(x->name, y->name);
  return 1;
}

/* ------------------ SortCameras ------------------------ */

void SortCameras(void){
  cameradata *ca;
  int i;

  ncameras_sorted=0;
  for(ca = camera_list_first.next; ca->next != NULL; ca = ca->next){
    ncameras_sorted++;
  }
  if(ncameras_sorted == 0)return;
  NEWMEM(cameras_sorted, ncameras_sorted*sizeof(cameradata *));
  for(i=0,ca = camera_list_first.next; ca->next != NULL; ca = ca->next,i++){
    cameras_sorted[i] = ca;
  }
  qsort((cameradata **)cameras_sorted, (size_t)ncameras_sorted, sizeof(cameradata *), CompareCameras);
}

/* ------------------ InsertCamera ------------------------ */

cameradata *InsertCamera(cameradata *cb,cameradata *source, char *name){
  cameradata *cam=NULL,*ca;
  int insert = 1;

  for(ca=camera_list_first.next;ca->next!=NULL;ca=ca->next){
    if(strcmp(ca->name, name) == 0){
      cam = ca;
      insert = 0;
      if(source != NULL){
        source->view_id = cam->view_id;
        source->next = cam->next;
        source->prev = cam->prev;
      }
      break;
    }
  }

  if(cam == NULL){
    if(NewMemory((void **)&cam, sizeof(cameradata)) == 0)return NULL;
    InitCamera(cam, name);
  }
  if(source!=NULL){
    CopyCamera(cam,source);
  }
  if(insert == 1){
    strcpy(cam->name, name);
    ca = cb->next;
    cb->next = cam;
    ca->prev = cam;
    cam->prev = cb;
    cam->next = ca;
    cam->view_id = camera_max_id;
    camera_max_id++;
  }
  GLUIUpdateViewpointList();
  updatemenu=1;
  return cam;
}

/* ------------------ CompareCamerasID ------------------------ */

int CompareCamerasID(const void *arg1, const void *arg2){
  cameradata *x, *y;
  int x_state = IS_OTHER, y_state = IS_OTHER;

  x = *(cameradata **)arg1;
  y = *(cameradata **)arg2;

  /*
      ext  int   other
ext    0   -1     -1
int    1    0     -1
other  1    1    strcmp
  */

  if(strcmp(x->name, "external")==0)x_state = IS_EXT;

  if(strcmp(y->name, "external")==0)y_state = IS_EXT;

  if(x_state==IS_EXT){
    if(y_state==IS_EXT)return 0;
    return -1;
  }
  else{
    if(y_state==IS_OTHER)return x->view_id - y->view_id;
    return 1;
  }
}

/* ------------------ SortCamerasID ------------------------ */

void SortCamerasID(void){
  cameradata *ca;
  int i;

  ncameras_sorted = 0;
  for(ca = camera_list_first.next; ca->next!=NULL; ca = ca->next){
    ncameras_sorted++;
  }
  if(ncameras_sorted==0)return;
  NEWMEM(cameras_sorted, ncameras_sorted*sizeof(cameradata *));
  for(i = 0, ca = camera_list_first.next; ca->next!=NULL; ca = ca->next, i++){
    cameras_sorted[i] = ca;
  }
  qsort((cameradata **)cameras_sorted, (size_t)ncameras_sorted, sizeof(cameradata *), CompareCamerasID);
}

/* ------------------ DeleteCamera ------------------------ */

void DeleteCamera(cameradata *cam){
  cameradata *ca, *cb;

  cb=cam->prev;
  ca=cam->next;
  cb->next=ca;
  ca->prev=cb;
  FREEMEMORY(cam);
  updatemenu=1;
}

/* ------------------ GetCamera ------------------------ */

cameradata *GetCamera(char *name){
  cameradata *ca;

  for(ca=camera_list_first.next;ca->next!=NULL;ca=ca->next){
    if(strcmp(ca->name,name)==0){
      return ca;
    }
  }
  return NULL;
}

/* ------------------ GetCameraLabel ------------------------ */

char *GetCameraLabel(int index){
  cameradata *ca;

  for(ca=camera_list_first.next;ca->next!=NULL;ca=ca->next){
    if(ca->view_id==index){
      return ca->name;
    }
  }
  return NULL;
}
