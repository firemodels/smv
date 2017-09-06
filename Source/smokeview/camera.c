#include "options.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "smokeviewvars.h"

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

/* --------------------------- InitDefaultViews ----------------------------- */
// Initialise some default cameras to add to the camera list.
void InitDefaultViews() {
  FREEMEMORY(camera_external);
  NewMemory((void **)&camera_external,sizeof(cameradata));

  FREEMEMORY(camera_external_save);
  NewMemory((void **)&camera_external_save,sizeof(cameradata));

  FREEMEMORY(camera_ini);
  NewMemory((void **)&camera_ini,sizeof(cameradata));
  camera_ini->defined=0;

  FREEMEMORY(camera_current);
  NewMemory((void **)&camera_current,sizeof(cameradata));

  FREEMEMORY(camera_internal);
  NewMemory((void **)&camera_internal,sizeof(cameradata));

  FREEMEMORY(camera_save);
  NewMemory((void **)&camera_save,sizeof(cameradata));

  FREEMEMORY(camera_last);
  NewMemory((void **)&camera_last,sizeof(cameradata));

  {
    char name_external[32];

    strcpy(name_external,"external");
    InitCamera(camera_external,name_external);
    camera_external->view_id=EXTERNAL_LIST_ID;
  }
  if(camera_ini!=NULL&&camera_ini->defined==1){
    CopyCamera(camera_current,camera_ini);
  }
  else{
    camera_external->zoom=zoom;
    CopyCamera(camera_current,camera_external);
  }
  strcpy(camera_label,camera_current->name);
  UpdateCameraLabel();
  {
    char name_internal[32];
    strcpy(name_internal,"internal");
    InitCamera(camera_internal,name_internal);
  }
  camera_internal->eye[0]=0.5*xbar;
  camera_internal->eye[1]=0.5*ybar;
  camera_internal->eye[2]=0.5*zbar;
  camera_internal->view_id=0;
  CopyCamera(camera_save,camera_current);
  CopyCamera(camera_last,camera_current);

}

/* ------------------ AddDefaultViews ------------------------ */
// Add default views to the camera list (i.e. external and internal views).
void AddDefaultViews(void){
  cameradata *cb, *ca;

  InitDefaultViews();

  cb=&camera_list_first;
  ca=cb->next;

  cb->next=camera_external;
  camera_external->next=camera_internal;
  camera_internal->next=ca;

  ca->prev=camera_internal;
  camera_internal->prev=camera_external;
  camera_external->prev=cb;
}

/* ------------------ UpdateCameraYpos ------------------------ */

void UpdateCameraYpos(cameradata *ci){
  float local_aperture_default;
  float width;
  float asp;

  local_aperture_default=Zoom2Aperture(1.0);
  asp=(float)screenHeight/(float)screenWidth;
  width=xbar;
  if(zbar/asp>xbar){
    width=zbar/asp;
  }
  eyeyfactor = -1.10*width/2.0/tan(local_aperture_default*DEG2RAD/2.0);
  ci->eye[1]=eyeyfactor*xyzbox;
  if(viscolorbarpath==1){
    ci->eye[0]=0.7;
    ci->eye[1]=-2.25;
    ci->eye[2]=0.5;
  }
  ci->isometric_y=(eyeyfactor-1.0)*xyzbox;
}

/* ------------------ InitCamera ------------------------ */

void InitCamera(cameradata *ci,char *name){
  strcpy(ci->name,name);
  ci->rotation_index=nmeshes;
  ci->defined=1;
  ci->azimuth=0.0;
  ci->view_angle=0.0;
  ci->eye[0]=eyexfactor*xbar;
  UpdateCameraYpos(ci);
  ci->eye[2]=eyezfactor*zbar;
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
  ci->xcen=xbar/2.0;
  ci->ycen=ybar/2.0;
  ci->zcen=zbar/2.0;
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
    Update_Glui_Clip();

  }

/* ------------------ CopyCamera ------------------------ */

void CopyCamera(cameradata *to, cameradata *from){

  memcpy(to,from,sizeof(cameradata));
  if(to==camera_current){
    zoom=camera_current->zoom;
    UpdateGluiZoom();
  }
  to->dirty=1;
  if(to==camera_current&&updateclipvals==0){
    Cam2Clip(camera_current);
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
    if(ca->rotation_index>=0&&ca->rotation_index<nmeshes){
      UpdateCurrentMesh(meshinfo + ca->rotation_index);
    }
    else{
      UpdateCurrentMesh(meshinfo);
    }
    highlight_mesh = current_mesh-meshinfo;
    handle_rotation_type(EYE_CENTERED);
    UpdateMeshList1(ca->rotation_index);
    update_trainer_moves();

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
  UpdateGluiSetViewXYZ(ca->eye);
  ca->dirty=0;
}

/* ------------------ CompareCameras ------------------------ */

#define IS_EXT 0
#define IS_INT 1
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
  if(strcmp(x->name, "internal") == 0)x_state = IS_INT;

  if(strcmp(y->name,"external") == 0)y_state = IS_EXT;
  if(strcmp(y->name, "internal") == 0)y_state = IS_INT;

  if(x_state == IS_EXT){
    if(y_state == IS_EXT)return 0;
    return -1;
  }
  else if(x_state == IS_INT){
    if(y_state == IS_EXT)return 1;
    if(y_state == IS_INT)return 0;
    return -1;
  }
  else{
    if(y_state == IS_OTHER)return strcmp(x->name, y->name);
    return 1;
  }
}

/* ------------------ SortCameras ------------------------ */

void SortCameras(void){
  cameradata *ca;
  int i;

  FREEMEMORY(cameras_sorted);
  ncameras_sorted=0;
  for(ca = camera_list_first.next; ca->next != NULL; ca = ca->next){
    ncameras_sorted++;
  }
  if(ncameras_sorted == 0)return;
  NewMemory((void **)&cameras_sorted, ncameras_sorted*sizeof(cameradata *));
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
  UpdateGluiViewList();
  updatemenu=1;
  return cam;
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
