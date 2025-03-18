#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#if defined(WIN32)
#include <windows.h>
#endif
#include GLU_H

#include "datadefs.h"
#include "smokeviewvars.h"
#include "IOobjects.h"
#include "getdata.h"

#include "readobject.h"
#include "readgeom.h"

static float *cos_long = NULL, *sin_long = NULL, *cos_lat = NULL, *sin_lat = NULL;
static float *sphere_coords = NULL;
static float specular[4] = {0.4,0.4,0.4,1.0};
unsigned char *rgbimage = NULL;
int rgbsize = 0;

#define IN_INTERVAL(IVAL) \
  if(time_local>=times_local[(IVAL)]&&time_local<=times_local[(IVAL)+1]){\
    if(time_local-times_local[(IVAL)]<times_local[(IVAL)+1]-time_local){\
      devicei->val=devicei->vals[(IVAL)];\
      *valid=devicei->valids[(IVAL)];\
    }\
    else{\
      devicei->val=devicei->vals[(IVAL)+1];\
      *valid=devicei->valids[(IVAL)+1];\
    }\
    devicei->ival=(IVAL);\
    return devicei->val;\
  }

/* ----------------------- GetSmokeSensors ----------------------------- */

void GetSmokeSensors(void){
  int doit, i;
  int width, height;

  width = screenWidth;
  height = screenHeight;

  doit=0;
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;
    char *label;

    devicei = global_scase.devicecoll.deviceinfo + i;
    label = devicei->object->label;
    if(STRCMP(label,"smokesensor")!=0)continue;
    doit=1;
    break;
  }
  if(doit==0)return;

  if(rgbimage==NULL||rgbsize!=width*height){
    if(rgbimage!=NULL){
      FREEMEMORY(rgbimage);
    }
    rgbsize=width*height;
    NewMemory( (void **)&rgbimage,3*rgbsize*sizeof(GLubyte));
  }
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0,0,width,height, GL_RGB, GL_UNSIGNED_BYTE, rgbimage);

  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;
    char *label;
    int row, col;
    int index,val;

    devicei = global_scase.devicecoll.deviceinfo + i;
    label = devicei->object->label;


    if(STRCMP(label,"smokesensor")!=0)continue;

    col = devicei->screenijk[0];
    row = devicei->screenijk[1];

    if(col<0||col>width-1||row<0||row>height-1){
      val=-1;
    }
    else{
      unsigned char *rgbpixel;

      index=row*width+col;
      rgbpixel = rgbimage + 3 * index;
      val=TOBW(rgbpixel);
    }
    devicei->visval=val;
  }
}

/* ----------------------- GetScreenCoords ----------------------------- */

void GetScreenCoords(float *xyz, int *screen){
  double mv_setup[16], projection_setup[16];
  GLint viewport_setup[4];
  double dscreen[3];

  glGetDoublev(GL_MODELVIEW_MATRIX, mv_setup);
  glGetDoublev(GL_PROJECTION_MATRIX, projection_setup);
  glGetIntegerv(GL_VIEWPORT, viewport_setup);

  gluProject(xyz[0], xyz[1], xyz[2], mv_setup, projection_setup, viewport_setup, dscreen, dscreen+1, dscreen+2);

  screen[0] = dscreen[0];
  screen[1] = dscreen[1];
  screen[2] = dscreen[2];
}

/* ----------------------- GetScreenRGB ----------------------------- */

void GetScreenRGB(float *xyz, int *rgbcolor){
  int screen[3];
  int width = 1, height = 1;
  unsigned char color[3];

  GetScreenCoords(xyz, screen);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(screen[0], screen[1], width, height, GL_RED,   GL_UNSIGNED_BYTE, color);
  glReadPixels(screen[0], screen[1], width, height, GL_GREEN, GL_UNSIGNED_BYTE, color+1);
  glReadPixels(screen[0], screen[1], width, height, GL_BLUE,  GL_UNSIGNED_BYTE, color+2);
  rgbcolor[0] = color[0];
  rgbcolor[1] = color[1];
  rgbcolor[2] = color[2];
}

/* ----------------------- RGBTest ----------------------------- */

void RGBTest(void){
  int rgbcolor[3];
  int dr, dg, db;
  int max_err;

  GetScreenRGB(rgb_test_xyz, rgbcolor);
  dr = ABS(rgbcolor[0]-rgb_test_rgb[0]);
  dg = ABS(rgbcolor[1]-rgb_test_rgb[1]);
  db = ABS(rgbcolor[2]-rgb_test_rgb[2]);
  max_err = MAX(dr, dg);
  max_err = MAX(max_err, db);
  if(max_err<=rgb_test_delta){
    printf("found color (%i,%i,%i) at (%f,%f,%f) - within bounds\n",
      rgbcolor[0], rgbcolor[1], rgbcolor[2],
      rgb_test_xyz[0], rgb_test_xyz[1], rgb_test_xyz[2]);
  }
  else{
    printf("***Error: found color (%i,%i,%i) at (%f,%f,%f) was expecting (%i,%i,%i)\n",
      rgbcolor[0], rgbcolor[1], rgbcolor[2],
      rgb_test_xyz[0], rgb_test_xyz[1], rgb_test_xyz[2],
      rgb_test_rgb[0], rgb_test_rgb[1], rgb_test_rgb[2]);
  }
  use_lighting = 0;
}

/* ----------------------- HaveSmokeSensor ----------------------------- */

int HaveSmokeSensor(void){
  int i;

  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devicei;

    devicei = global_scase.devicecoll.deviceinfo+i;
    if(STRCMP(devicei->object->label, "smokesensor")==0)return 1;
  }
  return 0;
}


/* ----------------------- GetPoint2BoxDist ----------------------------- */

float GetPoint2BoxDist(float boxmin[3], float boxmax[3], float p1[3],
                       float p2orig[3]){
  int i;
  float tt;
  int doit = 0;
  float dx, dy, dz;
  float xx, yy, zz;
  float p2[3];

  // box - xmin, ymin, zmin, xmax, ymax, zmax

  // if p1 is outside of box then return Dist(p1,p2)

  for(i = 0; i < 3; i++){
    if(p1[i] < boxmin[i]) return Dist(p1, p2orig);
    if(p1[i] > boxmax[i]) return Dist(p1, p2orig);
    p2[i] = p2orig[i];
  }

  // if p1 and p2 are both inside box then return Dist(p1,p2)

  for(i = 0; i < 3; i++){
    if(p2[i] < boxmin[i]){
      doit = 1;
      break;
    }
    if(p2[i] > boxmax[i]){
      doit = 1;
      break;
    }
  }
  if(doit == 0) return Dist(p1, p2);

  dx = p2[0] - p1[0];
  dy = p2[1] - p1[1];
  dz = p2[2] - p1[2];

  if(p1[0] >= boxmin[0] && boxmin[0] >= p2[0]){
    if(dx != 0.0){
      tt = (boxmin[0] - p1[0]) / dx;
      xx = boxmin[0];
      yy = p1[1] + tt * dy;
      zz = p1[2] + tt * dz;
      if(boxmin[1] <= yy && yy <= boxmax[1] && boxmin[2] <= zz &&
          zz <= boxmax[2]){
        p2[0] = xx;
        p2[1] = yy;
        p2[2] = zz;
        return Dist(p1, p2);
      }
    }
  }
  if(p1[0] <= boxmax[0] && boxmax[0] <= p2[0]){
    if(dx != 0.0){
      tt = (boxmax[0] - p1[0]) / dx;
      xx = boxmax[0];
      yy = p1[1] + tt * dy;
      zz = p1[2] + tt * dz;
      if(boxmin[1] <= yy && yy <= boxmax[1] && boxmin[2] <= zz &&
          zz <= boxmax[2]){
        p2[0] = xx;
        p2[1] = yy;
        p2[2] = zz;
        return Dist(p1, p2);
      }
    }
  }
  if(p1[1] >= boxmin[1] && boxmin[1] >= p2[1]){
    if(dy != 0.0){
      tt = (boxmin[1] - p1[1]) / dy;
      xx = p1[0] + tt * dx;
      yy = boxmin[1];
      zz = p1[2] + tt * dz;
      if(boxmin[0] <= xx && xx <= boxmax[0] && boxmin[2] <= zz &&
          zz <= boxmax[2]){
        p2[0] = xx;
        p2[1] = yy;
        p2[2] = zz;
        return Dist(p1, p2);
      }
    }
  }
  if(p1[1] <= boxmax[1] && boxmax[1] <= p2[1]){
    if(dy != 0.0){
      tt = (boxmax[1] - p1[1]) / dy;
      xx = p1[0] + tt * dx;
      yy = boxmax[1];
      zz = p1[2] + tt * dz;
      if(boxmin[0] <= xx && xx <= boxmax[0] && boxmin[2] <= zz &&
          zz <= boxmax[2]){
        p2[0] = xx;
        p2[1] = yy;
        p2[2] = zz;
        return Dist(p1, p2);
      }
    }
  }
  if(p1[2] >= boxmin[2] && boxmin[2] >= p2[2]){
    if(dz != 0.0){
      tt = (boxmin[2] - p1[2]) / dz;
      xx = p1[0] + tt * dx;
      yy = p1[1] + tt * dy;
      zz = boxmin[2];
      if(boxmin[0] <= xx && xx <= boxmax[0] && boxmin[1] <= yy &&
          yy <= boxmax[1]){
        p2[0] = xx;
        p2[1] = yy;
        p2[2] = zz;
        return Dist(p1, p2);
      }
    }
  }
  if(p1[2] <= boxmax[2] && boxmax[2] <= p2[2]){
    if(dz != 0.0){
      tt = (boxmax[2] - p1[2]) / dz;
      xx = p1[0] + tt * dx;
      yy = p1[1] + tt * dy;
      zz = boxmin[2];
      if(boxmin[0] <= xx && xx <= boxmax[0] && boxmin[1] <= yy &&
          yy <= boxmax[1]){
        p2[0] = xx;
        p2[1] = yy;
        p2[2] = zz;
        return Dist(p1, p2);
      }
    }
  }
  assert(FFALSE);
  return Dist(p1, p2);
}


/* ----------------------- GetDeviceScreenCoords ----------------------------- */

void GetDeviceScreenCoords(void){
  double mv_setup[16], projection_setup[16];
  GLint viewport_setup[4];
  int i;
  int doit;

  doit=0;
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;
    char *label;

    devicei = global_scase.devicecoll.deviceinfo + i;
    label = devicei->object->label;
    if(STRCMP(label,"smokesensor")==0){
      doit=1;
      break;
    }
  }
  if(doit==0)return;

  glGetDoublev(GL_MODELVIEW_MATRIX,mv_setup);
  glGetDoublev(GL_PROJECTION_MATRIX,projection_setup);
  glGetIntegerv(GL_VIEWPORT, viewport_setup);
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    float *xyz;
    double d_ijk[3];
    devicedata *devicei;
    int *ijk;
    char *label;
    meshdata *device_mesh;

    devicei = global_scase.devicecoll.deviceinfo + i;
    label = devicei->object->label;

    if(STRCMP(label,"smokesensor")!=0)continue;
    xyz = devicei->xyz;
    device_mesh = devicei->device_mesh;
    devicei->eyedist = GetPoint2BoxDist(device_mesh->boxmin,device_mesh->boxmax,xyz,fds_eyepos);
    ijk = devicei->screenijk;
    gluProject(xyz[0],xyz[1],xyz[2],mv_setup,projection_setup,viewport_setup,d_ijk,d_ijk+1,d_ijk+2);
    ijk[0] = d_ijk[0];
    ijk[1] = d_ijk[1];
    ijk[2] = d_ijk[2];
  }
}

/* ----------------------- DrawDevicesVal ----------------------------- */

void DrawDevicesVal(void){
  devicedata *devicei;
  int i;
  float *xyz, *xyznorm;
  float white[3]={1.0,1.0,1.0};
  float black[3]={0.0,0.0,0.0};
  int doit=0;

  if(fontindex==SCALED_FONT)ScaleFont3D();
  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
  if(active_smokesensors==1&&show_smokesensors!=SMOKESENSORS_HIDDEN){
    GetDeviceScreenCoords();
  }
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicei = global_scase.devicecoll.deviceinfo + i;

    if(devicei->object->visible==0||devicei->show == 0)continue;
    xyz = devicei->xyz;
    xyznorm = devicei->xyznorm;
    if(active_smokesensors==1&&show_smokesensors!=SMOKESENSORS_HIDDEN&&STRCMP(devicei->object->label,"smokesensor")==0){
      char label[256];
      float val;
      int ival;

      if(doit==0){
        glBlendFunc(GL_ONE,GL_ZERO);
        doit=1;
      }
      switch(show_smokesensors){
        case SMOKESENSORS_0255:
          sprintf(label,"%i",devicei->visval);
          break;
        case SMOKESENSORS_01:
          val = devicei->visval/255.0;
          sprintf(label,"%.2f",val);
          TrimZeros(label);
          break;
        case SMOKESENSORS_SCALED:
        case SMOKESENSORS_0INF:
          ival = devicei->visval;
          if(ival==255){
            strcpy(label,"Inf");
          }
          else{
            float light_extinct;

            val = ival/255.0;
            light_extinct = -log(val)/devicei->eyedist;
            val = smoke3d_cvis/light_extinct;
            if(val<10.0){
              sprintf(label,"%.1f",val);
            }
            else{
              sprintf(label,"%.0f",val);
            }
            TrimZeros(label);
          }
          break;
        default:
          assert(FFALSE);
          break;
      }
      if(devicei->visval>128){
        Output3Text(black,xyz[0]+0.2*xyznorm[0],xyz[1]+0.2*xyznorm[1],xyz[2]+0.2*xyznorm[2],label);
      }
      else{
        Output3Text(white,xyz[0]+0.2*xyznorm[0],xyz[1]+0.2*xyznorm[1],xyz[2]+0.2*xyznorm[2],label);
      }
    }
  }
  if(doit==1){
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  }
  glPopMatrix();
}

/* ----------------------- GetDeviceVal ----------------------------- */

float GetDeviceVal(float time_local, devicedata *devicei, int *valid){
  int nvals;
  int ival;
  float *times_local;

  nvals = devicei->nvals;
  ival = devicei->ival;
  times_local = devicei->times;

  if(nvals == 0 || times_local == NULL){
    *valid = 0;
    return 0.0;
  }
  IN_INTERVAL(ival);
  if(ival < nvals - 1){
    IN_INTERVAL(ival + 1);
  }
  if(ival > 0){
    IN_INTERVAL(ival - 1);
  }

  if(time_local <= times_local[0]){
    devicei->val = devicei->vals[0];
    devicei->ival = 0;
    *valid = devicei->valids[0];
  }
  else if(time_local >= times_local[nvals - 1]){
    devicei->val = devicei->vals[nvals - 1];
    devicei->ival = nvals - 2;
    *valid = devicei->valids[nvals - 1];
  }
  else{
    int low, mid, high;

    low = 0;
    high = nvals - 1;

    while(high - low > 1){
      mid = (low + high) / 2;
      if(time_local > times_local[mid]){
        low = mid;
      }
      else{
        high = mid;
      }
    }
    devicei->ival = low;
    devicei->val = devicei->vals[low];
    *valid = devicei->valids[low];
  }

  return devicei->val;
}

/* ----------------------- GetVDeviceVel ----------------------------- */

void GetVDeviceVel(float time_local, vdevicedata *vdevicei, float *vel, float *angle_local, float *dvel, float *dangle, int *velocity_type){
  float uvel=0.0, vvel=0.0, wvel=0.0;
  devicedata *udev, *vdev, *wdev;
  int validu=1,validv=1,validw=1;

  udev = vdevicei->udev;
  vdev = vdevicei->vdev;
  wdev = vdevicei->wdev;

  *velocity_type=VEL_INVALID;
  if(udev!=NULL){
    uvel=GetDeviceVal(time_local,udev,&validu);
  }
  if(vdev!=NULL){
    vvel= GetDeviceVal(time_local,vdev,&validv);
  }
  if(wdev!=NULL){
    wvel= GetDeviceVal(time_local,wdev,&validw);
  }
  if(validu==1&&validv==1&&validw==1){
    vel[0]=uvel;
    vel[1]=vvel;
    vel[2]=wvel;
    *velocity_type=VEL_CARTESIAN;
  }
  if(vdevicei->veldev!=NULL&&vdevicei->angledev!=NULL){
    float  velocity,  ang;
    float dvelocity=0.0, dang=0.0;
    int  valid_velocity=0,  valid_angle=0;
    int dvalid_velocity=0, dvalid_angle=0;

    velocity= GetDeviceVal(time_local,vdevicei->veldev,&valid_velocity);
    if(vdevicei->sd_veldev!=NULL){
      dvelocity= GetDeviceVal(time_local,vdevicei->sd_veldev,&dvalid_velocity);
      if(dvalid_velocity==0)dvelocity=0.0;
    }
    ang= GetDeviceVal(time_local,vdevicei->angledev,&valid_angle);
    if(vdevicei->sd_angledev!=NULL){
      dang= GetDeviceVal(time_local,vdevicei->sd_angledev,&dvalid_angle);
      if(dvalid_angle==0)dang=0.0;
    }
    if(valid_velocity==1&&valid_angle==1){
      vel[0]=velocity;
      dvel[0]=dvelocity;
      angle_local[0]=ang;
      dangle[0]=dang;
      *velocity_type=VEL_POLAR;
    }
  }
}

/* ----------------------- GetDeviceColor ----------------------------- */

unsigned char *GetDeviceColor(devicedata *devicei, unsigned char *colorval,float valmin, float valmax){
  float val;
  int valid,colorindex;
  float *rgb_local;

  if(devicei==NULL||valmax<=valmin)return NULL;
  val= GetDeviceVal(global_times[itimes],devicei,&valid);
  if(valid!=1)return NULL;
  val = (val-valmin)/(valmax-valmin);
  colorindex=CLAMP(255*val,0,255);
  rgb_local=current_colorbar->colorbar_rgb+3*colorindex;
  colorval[0]=255*rgb_local[0];
  colorval[1]=255*rgb_local[1];
  colorval[2]=255*rgb_local[2];
  return colorval;
}

/* ----------------------- OutputDeviceVal ----------------------------- */

void OutputDeviceVal(devicedata *devicei){
  char label[1000];
  float val;
  int valid;

  if(fontindex==SCALED_FONT)ScaleFont3D();
  val= GetDeviceVal(global_times[itimes],devicei,&valid);
  if(valid==1){
    f_units *unitclass;
    char *unit;
    char valuelabel[100];

    unitclass = GetUnitClass(devicei->unit);
    unit=devicei->unit;
    if(unitclass!=NULL){
      char *unit_type;
      f_unit *funit;

      funit = unitclass->units+unitclass->unit_index;
      unit = funit->unit;
      unit_type = unitclass->unitclass;
      val = GetUnitVal(unit_type, val, 1);
    }
    strcpy(label, "");
    sprintf(valuelabel, "%.1f",val);
    if(showdevice_id==1&&strcmp(devicei->deviceID,"null")!=0&&strlen(devicei->deviceID)>0){
      strcat(label, devicei->deviceID);
      strcat(label, ": ");
    }
    if(showdevice_type == 1){
      strcat(label, devicei->quantity);
      strcat(label, " ");
    }
    strcat(label, valuelabel);
    strcat(label, " ");
    if(showdevice_unit == 1)strcat(label, unit);
    Output3Text(foregroundcolor,0.0,0.0,0.0,label);
  }
  else{
    sprintf(label,"not available");
    Output3Text(foregroundcolor,0.0,0.0,0.0,label);
  }
}

#define GLVERTEX2F(x,y) \
if(orientation==WINDROSE_XY){\
  glVertex3f((x), (y), 0.0);\
}\
else if(orientation==WINDROSE_XZ){\
  glVertex3f((x), 0.0, (y));\
}\
else{\
  glVertex3f(0.0, (x), (y));\
}

/* ----------------------- DrawWindRose ----------------------------- */

void DrawWindRose(windrosedata *wr,int orientation){
  int itheta,icirc;
  float *xyz;
  histogramdata *hist;
  float dtheta,maxr;

  if(wr==NULL)return;
  xyz = wr->xyz;
  hist = wr->histogram+orientation;
  if(scale_windrose==WINDROSE_LOCALSCALE){
    maxr = hist->bucket_maxr;
  }
  else{
    maxr = maxr_windrose;
  }
  ENABLE_LIGHTING;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, global_scase.color_defs.block_ambient2);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  glEnable(GL_COLOR_MATERIAL);

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(xyz[0]-xbar0ORIG, xyz[1]-ybar0ORIG, xyz[2]-zbar0ORIG);

  dtheta = DEG2RAD*360.0/(float)hist->ntheta;
  glBegin(GL_TRIANGLES);
  for(itheta = 0;itheta<hist->ntheta;itheta++){
    int ir;
    float theta, theta2;
    float rval, rval2;

    theta  = ((float)itheta-0.5)*dtheta;
    theta2 = ((float)itheta+0.5)*dtheta;
    rval = 0.0;
    for(ir = 0;ir<hist->nr;ir++){
      int color_index;
      float drval;
      int k, nk;
      float dk;
      float angle_offset;

      //  (rval,theta2)     (rval2,theta2)
      //  (rval,theta)     (rval2,theta)
      //   d05              0.05
      //   radius_windrose (maxr/ntotal)

      color_index  = CLAMP(255*(float)(ir+0.5)/(float)hist->nr, 0, 255);
      drval  = radius_windrose*hist->buckets_polar[ir+  itheta*hist->nr]/maxr;
      rval2 = rval + drval;

      nk = RAD2DEG*(theta2-theta);
      dk = (theta2-theta)/(float)nk;
      angle_offset = 0.0;
      if(windstate_windrose == WINDROSE_HEADING)angle_offset = PI;

      for(k = 0;k<nk;k++){
        float angle1, angle2;
        float x11, x12, x21, x22;
        float y11, y12, y21, y22;

        angle1 = theta +     (float)k*dk + angle_offset;
        angle2 = theta + (float)(k+1)*dk + angle_offset;

        x11 = rval*cos(angle1);
        x12 = rval2*cos(angle1);
        x21 = rval*cos(angle2);
        x22 = rval2*cos(angle2);
        y11 = rval*sin(angle1);
        y12 = rval2*sin(angle1);
        y21 = rval*sin(angle2);
        y22 = rval2*sin(angle2);

        glColor3fv(rgb_slice+4*color_index);
        GLVERTEX2F(x11, y11);
        GLVERTEX2F(x12, y12);
        GLVERTEX2F(x22, y22);

        GLVERTEX2F(x11, y11);
        GLVERTEX2F(x22, y22);
        GLVERTEX2F(x12, y12);

        GLVERTEX2F(x11, y11);
        GLVERTEX2F(x22, y22);
        GLVERTEX2F(x21, y21);

        GLVERTEX2F(x11, y11);
        GLVERTEX2F(x21, y21);
        GLVERTEX2F(x22, y22);
      }
      rval = rval2;
    }
  }
  glEnd();

  if(showref_windrose==1){
    unsigned char uc_foregroundcolor[4];

    uc_foregroundcolor[0] = 255 * foregroundcolor[0];
    uc_foregroundcolor[1] = 255 * foregroundcolor[1];
    uc_foregroundcolor[2] = 255 * foregroundcolor[2];
    uc_foregroundcolor[3] = 255 * foregroundcolor[3];

    if(orientation == WINDROSE_XZ)glRotatef(90.0, 1.0, 0.0, 0.0);
    if(orientation == WINDROSE_YZ)glRotatef(90.0, 0.0, 1.0, 0.0);
    glTranslatef(0.0,0.0,0.001);
    glLineWidth(2.0);
    for(icirc = 1;icirc<100;icirc++){
      float scalei,scalei_normalized,diameter;
      char scale_percen[256];

      scalei=(float)icirc*(float)scale_increment_windrose/100.0;
      if(scalei > (float)scale_max_windrose/100.0)continue;
      scalei_normalized = scalei/(maxr/hist->ntotal);
      if(scalei_normalized>1.0)break;
      diameter = 2.0*radius_windrose*scalei_normalized;
      DrawCircle(diameter, uc_foregroundcolor, &windrose_circ);
      if(showlabels_windrose == 1){
        sprintf(scale_percen, "%.0f%s", 100.0*scalei, "%");
        Output3Text(foregroundcolor, 0.01 + diameter / 2.0, 0.0, 0.0, scale_percen);
      }
    }
    glTranslatef(0.0, 0.0, -0.002);
    for(icirc = 1;icirc<100;icirc++){
      float scalei, scalei_normalized, diameter;

      scalei=(float)icirc*(float)scale_increment_windrose/100.0;
      if(scalei > (float)scale_max_windrose/100.0)continue;
      scalei_normalized = scalei /(maxr/hist->ntotal);
      if(scalei_normalized>1.0)break;
      diameter = 2.0*radius_windrose*scalei_normalized;
      DrawCircle(diameter, uc_foregroundcolor, &windrose_circ);
    }
  }
  glPopMatrix();
  DISABLE_LIGHTING;
}

/* ----------------------- DrawWindRosesDevices ----------------------------- */

void DrawWindRosesDevices(void){
  int i;

  if(windrose_xy_vis==0&&windrose_xz_vis==0&&windrose_yz_vis==0)return;
  for(i = 0;i<global_scase.devicecoll.nvdeviceinfo;i++){
    vdevicedata *vdevi;
    windrosedata *wr;
    int itime;


    vdevi = global_scase.devicecoll.vdeviceinfo + i;
    if(vdevi->display==0||vdevi->unique==0)continue;
    itime = 0;
    if(global_times!=NULL)itime = CLAMP(itimes, 0, vdevi->nwindroseinfo-1);
    wr = vdevi->windroseinfo+itime;
    if(windrose_xy_vis==1)DrawWindRose(wr, WINDROSE_XY);
    if(windrose_xz_vis==1)DrawWindRose(wr, WINDROSE_XZ);
    if(windrose_yz_vis==1)DrawWindRose(wr, WINDROSE_YZ);
  }
}

/* ----------------------- DrawLine ----------------------------- */

void DrawLine(float *xyz1, float *xyz2, unsigned char *rgbcolor){
  glBegin(GL_LINES);
  if(rgbcolor != NULL)glColor3ubv(rgbcolor);
  glVertex3fv(xyz1);
  glVertex3fv(xyz2);
  glEnd();
}

/* ----------------------- DrawTargetNorm ----------------------------- */

void DrawTargetNorm(void){
  int i;
  devicedata *devicei;
  float *xyz, *xyznorm;

  if(global_scase.isZoneFireModel==1&&global_scase.hasSensorNorm==1&&visSensor==1&&visSensorNorm==1){
    glPushMatrix();
    glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
    glBegin(GL_LINES);
    glColor4fv(sensornormcolor);

    for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
      float xyz2[3];

      devicei = global_scase.devicecoll.deviceinfo + i;

      if(devicei->object->visible == 0 || devicei->show == 0)continue;
      if(STRCMP(devicei->object->label,"sensor")==0&&visSensor==0)continue;
      if(global_scase.isZoneFireModel==1&&STRCMP(devicei->object->label,"target")==0&&visSensor==0)continue;
      xyz = devicei->xyz;
      xyznorm = devicei->xyznorm;
      glVertex3fv(xyz);
      xyz2[0]=xyz[0]+devicenorm_length*xyznorm[0];
      xyz2[1]=xyz[1]+devicenorm_length*xyznorm[1];
      xyz2[2]=xyz[2]+devicenorm_length*xyznorm[2];
      glVertex3fv(xyz2);
    }
    glEnd();
    glPopMatrix();
  }
}

/* ----------------------- DrawArcDisk ----------------------------- */

void DrawArcDisk(float angle, float diameter, float height, unsigned char *rgbcolor){
  int i, iarc;

  if(cos_lat == NULL)InitSphere(NLAT, NLONG);

  iarc = NLONG*angle / 360.0 + 0.5;
  if(iarc < 2)iarc = 2;
  if(iarc > NLONG)iarc = NLONG;

  if(object_outlines == 0){
    glBegin(GL_QUADS);
    if(rgbcolor != NULL)glColor3ubv(rgbcolor);

    for(i = 0;i < iarc;i++){
      glNormal3f(cos_long[i], sin_long[i], 0.0);
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, 0.0); // 1

      glNormal3f(cos_long[i + 1], sin_long[i + 1], 0.0);
      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, 0.0); // 2

      glNormal3f(cos_long[i + 1], sin_long[i + 1], 0.0);
      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, height); // 3

      glNormal3f(cos_long[i], sin_long[i], 0.0);
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, height); // 4
    }

    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(diameter*cos_long[0] / 2.0, diameter*sin_long[0] / 2.0, 0.0); // 1
    glVertex3f(diameter*cos_long[0] / 2.0, diameter*sin_long[0] / 2.0, height); // 1
    glVertex3f(0.0, 0.0, height);

    glNormal3f(sin_long[iarc - 1], -cos_long[iarc - 1], 0.0);
    glVertex3f(0.0, 0.0, height);
    glVertex3f(diameter*cos_long[iarc] / 2.0, diameter*sin_long[iarc] / 2.0, height); // 1
    glVertex3f(diameter*cos_long[iarc] / 2.0, diameter*sin_long[iarc] / 2.0, 0.0); // 1
    glVertex3f(0.0, 0.0, 0.0);
    glEnd();

    glBegin(GL_TRIANGLES);
    if(rgbcolor != NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0, 0.0, -1.0);
    for(i = 0;i < iarc;i++){
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, 0.0);
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, 0.0);
    }
    glNormal3f(0.0, 0.0, 1.0);
    for(i = 0;i < iarc;i++){
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, height);
      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, height);
      glVertex3f(0.0, 0.0, height);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor != NULL)glColor3ubv(rgbcolor);

    for(i = 0;i < iarc;i++){
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, 0.0); // 1
      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, 0.0); // 2

      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, 0.0); // 2
      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, height); // 3

      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, height); // 3
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, height); // 4

      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, height); // 4
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, 0.0); // 1
    }

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(diameter*cos_long[0] / 2.0, diameter*sin_long[0] / 2.0, 0.0); // 1

    glVertex3f(diameter*cos_long[0] / 2.0, diameter*sin_long[0] / 2.0, 0.0); // 1
    glVertex3f(diameter*cos_long[0] / 2.0, diameter*sin_long[0] / 2.0, height); // 1

    glVertex3f(diameter*cos_long[0] / 2.0, diameter*sin_long[0] / 2.0, height); // 1
    glVertex3f(0.0, 0.0, height);

    glVertex3f(0.0, 0.0, height);
    glVertex3f(0.0, 0.0, 0.0);

    glVertex3f(0.0, 0.0, height);
    glVertex3f(diameter*cos_long[iarc] / 2.0, diameter*sin_long[iarc] / 2.0, height); // 1

    glVertex3f(diameter*cos_long[iarc] / 2.0, diameter*sin_long[iarc] / 2.0, height); // 1
    glVertex3f(diameter*cos_long[iarc] / 2.0, diameter*sin_long[iarc] / 2.0, 0.0); // 1

    glVertex3f(diameter*cos_long[iarc] / 2.0, diameter*sin_long[iarc] / 2.0, 0.0); // 1
    glVertex3f(0.0, 0.0, 0.0);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, height);
    glEnd();

    glBegin(GL_LINES);
    if(rgbcolor != NULL)glColor3ubv(rgbcolor);

    for(i = 0;i < iarc;i++){
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, 0.0);
      glVertex3f(0.0, 0.0, 0.0);

      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, 0.0);

      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, 0.0);
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, 0.0);
    }
    for(i = 0;i < iarc;i++){
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, height);
      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, height);

      glVertex3f(diameter*cos_long[i + 1] / 2.0, diameter*sin_long[i + 1] / 2.0, height);
      glVertex3f(0.0, 0.0, height);

      glVertex3f(0.0, 0.0, height);
      glVertex3f(diameter*cos_long[i] / 2.0, diameter*sin_long[i] / 2.0, height);
    }
    glEnd();
  }

}

/* ----------------------- DrawArc ----------------------------- */

void DrawArc(float angle, float diameter, unsigned char *rgbcolor){
  int i, iarc;
  float *xcirc, *ycirc;

  if(object_circ.ncirc == 0)InitCircle(CIRCLE_SEGS, &object_circ);
  xcirc = object_circ.xcirc;
  ycirc = object_circ.ycirc;

  iarc = CIRCLE_SEGS*(angle + 180.0 / CIRCLE_SEGS) / 360.0;
  if(iarc < 2)iarc = 2;
  if(iarc > CIRCLE_SEGS)iarc = CIRCLE_SEGS;
  glBegin(GL_LINE_LOOP);
  if(rgbcolor != NULL)glColor3ubv(rgbcolor);
  for(i = 0;i < iarc;i++){
    glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, 0.0);
  }
  glEnd();
}

/* ----------------------- DrawCDisk ----------------------------- */

void DrawCDisk(float diameter, float height, unsigned char *rgbcolor){
  int i;
  int ncirc;
  float *xcirc, *ycirc;

  if(object_circ.ncirc == 0)InitCircle(CIRCLE_SEGS, &object_circ);
  ncirc = object_circ.ncirc;
  xcirc = object_circ.xcirc;
  ycirc = object_circ.ycirc;

  if(object_outlines == 0){
    glBegin(GL_QUADS);
    if(rgbcolor != NULL)glColor3ubv(rgbcolor);

    for(i = 0;i < ncirc;i++){
      glNormal3f(xcirc[i], ycirc[i], 0.0);
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, -height / 2.00); // 1

      glNormal3f(xcirc[i + 1], ycirc[i + 1], 0.0);
      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, -height / 2.0); // 2

      glNormal3f(xcirc[i + 1], ycirc[i + 1], 0.0);
      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, height / 2.0); // 3

      glNormal3f(xcirc[i], ycirc[i], 0.0);
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, height / 2.0); // 4
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor != NULL)glColor3ubv(rgbcolor);

    for(i = 0;i < ncirc;i++){
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, -height / 2.00); // 1
      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, -height / 2.0); // 2

      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, -height / 2.0); // 2
      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, height / 2.0); // 3

      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, height / 2.0); // 3
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, height / 2.0); // 4

      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, height / 2.0); // 4
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, -height / 2.00); // 1
    }
    glEnd();
  }

  if(object_outlines == 0){
    glBegin(GL_TRIANGLES);
    if(rgbcolor != NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0, 0.0, -1.0);
    for(i = 0;i < ncirc;i++){
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, -height / 2.0);
      glVertex3f(0.0, 0.0, -height / 2.0);
      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, -height / 2.0);
    }
    glNormal3f(0.0, 0.0, 1.0);
    for(i = 0;i < ncirc;i++){
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, height / 2.0);
      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, height / 2.0);
      glVertex3f(0.0, 0.0, height / 2.0);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor != NULL)glColor3ubv(rgbcolor);

    for(i = 0;i < ncirc;i++){
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, -height / 2.0);
      glVertex3f(0.0, 0.0, -height / 2.0);

      glVertex3f(0.0, 0.0, -height / 2.0);
      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, -height / 2.0);

      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, -height / 2.0);
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, -height / 2.0);
    }
    for(i = 0;i < ncirc;i++){
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, height / 2.0);
      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, height / 2.0);

      glVertex3f(diameter*xcirc[i + 1] / 2.0, diameter*ycirc[i + 1] / 2.0, height / 2.0);
      glVertex3f(0.0, 0.0, height / 2.0);

      glVertex3f(0.0, 0.0, height / 2.0);
      glVertex3f(diameter*xcirc[i] / 2.0, diameter*ycirc[i] / 2.0, height / 2.0);
    }
    glEnd();
  }
}

/* ----------------------- DrawTSphere ----------------------------- */

void DrawTSphere(int texture_index,float diameter, unsigned char *rgbcolor){
  texturedata *texti;

  if(texture_index<0||texture_index>global_scase.texture_coll.ntextureinfo-1){
    texti=NULL;
  }
  else{
    texti = global_scase.texture_coll.textureinfo + texture_index;
    if(texti->loaded==0||texti->display==0)texti=NULL;
  }
  if(texti!=NULL&&object_outlines==0){
    int i,j;

    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D,texti->name);

    glPushMatrix();
    glScalef(diameter/2.0,diameter/2.0,diameter/2.0);
    if(cos_lat==NULL)InitSphere(NLAT,NLONG);
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(j=0;j<NLAT;j++){
      float ti,tip1;
      float tj,tjp1;

      tj = 1.0-(float)j/NLAT;
      tjp1 = 1.0-(float)(j+1)/NLAT;
      for(i=0;i<NLONG;i++){
        float x, y, z;

        ti = 1.0-(float)i/(float)NLONG;
        tip1 = 1.0-(float)(i+1)/(float)NLONG;

        x = cos_long[i]*cos_lat[j];
        y = sin_long[i]*cos_lat[j];
        z = sin_lat[j];

        glNormal3f(x,y,z);
        glTexCoord2f(ti,tj);
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j];
        y = sin_long[i+1]*cos_lat[j];
        z = sin_lat[j];
        glNormal3f(x,y,z);
        glTexCoord2f(tip1,tj);
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j+1];
        y = sin_long[i+1]*cos_lat[j+1];
        z = sin_lat[j+1];
        glNormal3f(x,y,z);
        glTexCoord2f(tip1,tjp1);
        glVertex3f(x,y,z);

        x = cos_long[i]*cos_lat[j+1];
        y = sin_long[i]*cos_lat[j+1];
        z = sin_lat[j+1];

        glNormal3f(x,y,z);
        glTexCoord2f(ti,tjp1);
        glVertex3f(x,y,z);
      }
    }
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
  }
  else{
    DrawSphere(diameter,rgbcolor);
  }
}

/* ----------------------- DrawSphereSeg ----------------------------- */

void DrawSphereSeg(float anglemin, float anglemax, float rmin, float rmax){
  int i, j;
  float ai, aip1, aj, ajp1;
  float danglei,danglej;
  float cosi, cosip1, sini, sinip1;
  float cosj, cosjp1, sinj, sinjp1;
  float colorin[4]={1.0,0.0,0.0,1.0};
  float colorout[4]={0.0,1.0,0.0,1.0};
  float coloredge[4]={0.0,0.0,1.0,1.0};
  float colori[4]={1.0,0.0,0.0,1.0};
  float colorip1[4]={1.0,0.0,0.0,1.0};

  anglemax=0.0;

  danglei = (anglemax-anglemin)/(float)NLAT;
  danglej = 2.0*PI/(float)NLONG;

  if(object_outlines==0){
  glColor4fv(coloredge);
  glBegin(GL_QUADS);
  ai = anglemin;
  cosi = cos(ai);
  sini = sin(ai);
  for(j=0;j<NLONG;j++){
    aj = j*danglej;
    ajp1 = (j+1)*danglej;
    cosj = cos(aj);
    cosjp1 = cos(ajp1);
    sinj = sin(aj);
    sinjp1 = sin(ajp1);
    glNormal3f(-cosj,-sinj,0.0);
    glVertex3f(rmin*sini*cosj,rmin*sini*sinj,cosi*rmin);

    glNormal3f(-cosjp1,-sinjp1,0.0);
    glVertex3f(rmin*sini*cosjp1,rmin*sini*sinjp1,cosi*rmin);

    glNormal3f(-cosjp1,-sinjp1,0.0);
    glVertex3f(rmax*sini*cosjp1,rmax*sini*sinjp1,cosi*rmax);

    glNormal3f(-cosj,-sinj,0.0);
    glVertex3f(rmax*sini*cosj,rmax*sini*sinj,cosi*rmax);
  }

  memcpy(colori,colorin,4*sizeof(float));
  memcpy(colorip1,colorin,4*sizeof(float));
  for(i=0;i<NLAT;i++){
    ai = anglemin + i*danglei;
    aip1 = anglemin + (i+1)*danglei;
    cosi = cos(ai);
    cosip1 = cos(aip1);
    sini = sin(ai);
    sinip1 = sin(aip1);
    colori[1]=0.6*(float)i/(float)NLAT;
    colorip1[1]=0.6*(float)(i+1)/(float)NLAT;
    glColor4fv(colori);
    for(j=0;j<NLONG;j++){
      aj = j*danglej;
      ajp1 = (j+1)*danglej;
      cosj = cos(aj);
      cosjp1 = cos(ajp1);
      sinj = sin(aj);
      sinjp1 = sin(ajp1);
      glColor4fv(colori);
      glNormal3f(-sini*cosj,-sini*sinj,-cosi);
      glVertex3f(rmin*sini*cosj,rmin*sini*sinj,cosi*rmin);

      glColor4fv(colorip1);
      glNormal3f(-sinip1*cosj,-sinip1*sinj,-cosip1);
      glVertex3f(rmin*sinip1*cosj,rmin*sinip1*sinj,cosip1*rmin);

      glColor4fv(colorip1);
      glNormal3f(-sinip1*cosjp1,-sinip1*sinjp1,-cosip1);
      glVertex3f(rmin*sinip1*cosjp1,rmin*sinip1*sinjp1,cosip1*rmin);

      glColor4fv(colori);
      glNormal3f(-sini*cosjp1,-sini*sinjp1,-cosi);
      glVertex3f(rmin*sini*cosjp1,rmin*sini*sinjp1,cosi*rmin);
    }
  }

  memcpy(colori,colorout,4*sizeof(float));
  memcpy(colorip1,colorout,4*sizeof(float));
  for(i=0;i<NLAT;i++){
    ai = anglemin + i*danglei;
    aip1 = anglemin + (i+1)*danglei;
    cosi = cos(ai);
    cosip1 = cos(aip1);
    sini = sin(ai);
    sinip1 = sin(aip1);
    colori[2]=0.6*(float)i/(float)NLAT;
    colorip1[2]=0.6*(float)(i+1)/(float)NLAT;
    for(j=0;j<NLONG;j++){
      aj = j*danglej;
      ajp1 = (j+1)*danglej;
      cosj = cos(aj);
      cosjp1 = cos(ajp1);
      sinj = sin(aj);
      sinjp1 = sin(ajp1);

      glColor4fv(colori);
      glNormal3f(sini*cosj,sini*sinj,cosi);
      glVertex3f(rmax*sini*cosj,rmax*sini*sinj,cosi*rmax);

      glNormal3f(sini*cosjp1,sini*sinjp1,cosi);
      glVertex3f(rmax*sini*cosjp1,rmax*sini*sinjp1,cosi*rmax);

      glColor4fv(colorip1);
      glNormal3f(sinip1*cosjp1,sinip1*sinjp1,cosip1);
      glVertex3f(rmax*sinip1*cosjp1,rmax*sinip1*sinjp1,cosip1*rmax);

      glNormal3f(sinip1*cosj,sinip1*sinj,cosip1);
      glVertex3f(rmax*sinip1*cosj,rmax*sinip1*sinj,cosip1*rmax);
    }
  }
  glEnd();
  }
  else{
  ai = anglemin;
  glColor4fv(coloredge);
  glBegin(GL_LINES);
  cosi = cos(ai);
  sini = sin(ai);
  for(j=0;j<NLONG;j++){
    aj = j*danglej;
    cosj = cos(aj);
    sinj = sin(aj);
    glVertex3f(rmin*sini*cosj,rmin*sini*sinj,cosi*rmin);

    glVertex3f(rmax*sini*cosj,rmax*sini*sinj,cosi*rmax);
  }

  memcpy(colori,colorin,4*sizeof(float));
  memcpy(colorip1,colorin,4*sizeof(float));
  for(i=0;i<NLAT;i++){
    ai = anglemin + i*danglei;
    aip1 = anglemin + (i+1)*danglei;
    cosi = cos(ai);
    cosip1 = cos(aip1);
    sini = sin(ai);
    sinip1 = sin(aip1);
    colori[1]=0.6*(float)i/(float)NLAT;
    colorip1[1]=0.6*(float)(i+1)/(float)NLAT;
    glColor4fv(colori);
    for(j=0;j<NLONG;j++){
      aj = j*danglej;
      ajp1 = (j+1)*danglej;
      cosj = cos(aj);
      cosjp1 = cos(ajp1);
      sinj = sin(aj);
      sinjp1 = sin(ajp1);
      glColor4fv(colori);
      glVertex3f(rmin*sini*cosj,rmin*sini*sinj,cosi*rmin);

      glColor4fv(colorip1);
      glVertex3f(rmin*sinip1*cosj,rmin*sinip1*sinj,cosip1*rmin);

      glColor4fv(colorip1);
      glVertex3f(rmin*sinip1*cosjp1,rmin*sinip1*sinjp1,cosip1*rmin);

      glColor4fv(colori);
      glVertex3f(rmin*sini*cosjp1,rmin*sini*sinjp1,cosi*rmin);
    }
  }

  memcpy(colori,colorout,4*sizeof(float));
  memcpy(colorip1,colorout,4*sizeof(float));
  for(i=0;i<NLAT;i++){
    ai = anglemin + i*danglei;
    aip1 = anglemin + (i+1)*danglei;
    cosi = cos(ai);
    cosip1 = cos(aip1);
    sini = sin(ai);
    sinip1 = sin(aip1);
    colori[2]=0.6*(float)i/(float)NLAT;
    colorip1[2]=0.6*(float)(i+1)/(float)NLAT;
    for(j=0;j<NLONG;j++){
      aj = j*danglej;
      ajp1 = (j+1)*danglej;
      cosj = cos(aj);
      cosjp1 = cos(ajp1);
      sinj = sin(aj);
      sinjp1 = sin(ajp1);

      glColor4fv(colori);
      glVertex3f(rmax*sini*cosj,rmax*sini*sinj,cosi*rmax);

      glVertex3f(rmax*sini*cosjp1,rmax*sini*sinjp1,cosi*rmax);

      glColor4fv(colorip1);
      glVertex3f(rmax*sinip1*cosjp1,rmax*sinip1*sinjp1,cosip1*rmax);

      glVertex3f(rmax*sinip1*cosj,rmax*sinip1*sinj,cosip1*rmax);
    }
  }
  glEnd();
  }
}

/* ----------------------- DrawSphere ----------------------------- */

void DrawSphere(float diameter, unsigned char *rgbcolor){
  int i,j;

  if(cos_lat==NULL)InitSphere(NLAT,NLONG);

  glPushMatrix();
  glScalef(diameter/2.0,diameter/2.0,diameter/2.0);

  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(j=0;j<NLAT;j++){
      for(i=0;i<NLONG;i++){
        float x, y, z;

        x = cos_long[i]*cos_lat[j];
        y = sin_long[i]*cos_lat[j];
        z = sin_lat[j];

        glNormal3f(x,y,z);
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j];
        y = sin_long[i+1]*cos_lat[j];
        z = sin_lat[j];
        glNormal3f(x,y,z);
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j+1];
        y = sin_long[i+1]*cos_lat[j+1];
        z = sin_lat[j+1];
        glNormal3f(x,y,z);
        glVertex3f(x,y,z);

        x = cos_long[i]*cos_lat[j+1];
        y = sin_long[i]*cos_lat[j+1];
        z = sin_lat[j+1];

        glNormal3f(x,y,z);
        glVertex3f(x,y,z);
      }
    }
    glEnd();
  }
  else{
    glBegin(GL_LINE_LOOP);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(j=0;j<NLAT;j++){
      for(i=0;i<NLONG;i++){
        float x, y, z;

        x = cos_long[i]*cos_lat[j];
        y = sin_long[i]*cos_lat[j];
        z = sin_lat[j];
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j];
        y = sin_long[i+1]*cos_lat[j];
        z = sin_lat[j];
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j+1];
        y = sin_long[i+1]*cos_lat[j+1];
        z = sin_lat[j+1];
        glVertex3f(x,y,z);

        x = cos_long[i]*cos_lat[j+1];
        y = sin_long[i]*cos_lat[j+1];
        z = sin_lat[j+1];
        glVertex3f(x,y,z);
      }
    }
    glEnd();
  }
  glPopMatrix();
}

/* ----------------------- DrawHSphere ----------------------------- */

void DrawHSphere(float diameter, unsigned char *rgbcolor){
  int i,j;

  if(cos_lat==NULL)InitSphere(NLAT,NLONG);

  glPushMatrix();
  glScalef(diameter/2.0,diameter/2.0,diameter/2.0);

  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(j=NLAT/2;j<NLAT;j++){
      for(i=0;i<NLONG;i++){
        float x, y, z;

        x = cos_long[i]*cos_lat[j];
        y = sin_long[i]*cos_lat[j];
        z = sin_lat[j];

        glNormal3f(x,y,z);
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j];
        y = sin_long[i+1]*cos_lat[j];
        z = sin_lat[j];
        glNormal3f(x,y,z);
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j+1];
        y = sin_long[i+1]*cos_lat[j+1];
        z = sin_lat[j+1];
        glNormal3f(x,y,z);
        glVertex3f(x,y,z);

        x = cos_long[i]*cos_lat[j+1];
        y = sin_long[i]*cos_lat[j+1];
        z = sin_lat[j+1];

        glNormal3f(x,y,z);
        glVertex3f(x,y,z);
      }
    }
    glEnd();
    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(i=0;i<NLONG;i++){
      float x, y, z;

      x = cos_long[i+1];
      y = sin_long[i+1];
      z = 0.0;

      glNormal3f(0.0,0.0,-1.0);
      glVertex3f(x,y,z);

      x = cos_long[i];
      y = sin_long[i];
      z = 0.0;
      glVertex3f(x,y,z);

      x = 0.0;
      y = 0.0;
      z = 0.0;
      glVertex3f(x,y,z);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINE_LOOP);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(j=NLAT/2;j<NLAT;j++){
      for(i=0;i<NLONG;i++){
        float x, y, z;

        x = cos_long[i]*cos_lat[j];
        y = sin_long[i]*cos_lat[j];
        z = sin_lat[j];
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j];
        y = sin_long[i+1]*cos_lat[j];
        z = sin_lat[j];
        glVertex3f(x,y,z);

        x = cos_long[i+1]*cos_lat[j+1];
        y = sin_long[i+1]*cos_lat[j+1];
        z = sin_lat[j+1];
        glVertex3f(x,y,z);

        x = cos_long[i]*cos_lat[j+1];
        y = sin_long[i]*cos_lat[j+1];
        z = sin_lat[j+1];
        glVertex3f(x,y,z);
      }
    }
    glEnd();
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(i=0;i<NLONG;i++){
      float x, y, z;

      x = cos_long[i];
      y = sin_long[i];
      z = 0.0;
      glVertex3f(x,y,z);

      x = 0.0;
      y = 0.0;
      z = 0.0;
      glVertex3f(x,y,z);
    }
    glEnd();
  }
  glPopMatrix();
}

/* ----------------------- DrawHalfSphere ----------------------------- */

void DrawHalfSphere(void){
  int i, j;
  float dxFDS, dyFDS, dzFDS, diameter;
  float *c_lat, *s_lat, *c_long, *s_long;

  dxFDS = xbarFDS - xbar0FDS;
  dyFDS = ybarFDS - ybar0FDS;
  dzFDS = zbarFDS - zbar0FDS;
  diameter = sky_diam*sqrt(dxFDS*dxFDS + dyFDS*dyFDS + dzFDS*dzFDS);

  if(sphere_coords == NULL)sphere_coords = InitSphere2(nlat_hsphere, nlong_hsphere);
  c_lat  = sphere_coords;
  s_lat  = c_lat  + nlat_hsphere  + 1;
  c_long = s_lat  + nlat_hsphere  + 1;
  s_long = c_long + nlong_hsphere + 1;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
  glTranslatef(dxFDS/2.0,dyFDS/2.0,0.0);
  glScalef(diameter/2.0, diameter/2.0, diameter/2.0);
  if(mscale[0]>0.0&&mscale[1]>0.0&&mscale[2]>0.0){
    glScalef(1.0/mscale[0], 1.0/mscale[1], 1.0/mscale[2]);
  }

  if(visSkyground == 1){
    unsigned char forest_green[3]={87,108,67};

    j = nlat_hsphere / 2;
    glBegin(GL_TRIANGLES);
    glColor3ubv(forest_green);
    for(i = 0; i < nlong_hsphere; i++){
      int ip1;
      float x[2], y[2], z[2];

      ip1 = i + 1;
      x[0] = c_long[i] * c_lat[j];
      y[0] = s_long[i] * c_lat[j];
      z[0] = s_lat[j];

      x[1] = c_long[ip1] * c_lat[j];
      y[1] = s_long[ip1] * c_lat[j];
      z[1] = s_lat[j];
      glVertex3f(0.0,0.0,0.0);
      glVertex3f(x[0], y[0], z[0]);
      glVertex3f(x[1], y[1], z[1]);
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(x[1], y[1], z[1]);
      glVertex3f(x[0], y[0], z[0]);
    }
    glEnd();
  }

  int use_sky;

  use_sky = 0;
  if(nsky_texture > 0 && sky_texture != NULL && sky_texture->loaded == 1 && sky_texture->display == 1&& visSkySpheretexture==1)use_sky = 1;
  if(use_sky == 1){
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, sky_texture->name);
  }

  glBegin(GL_QUADS);
  if(use_sky == 0)glColor3f(0.0, 0.0, 1.0);
  int nlats;

  nlats = nlat_hsphere - 1 - nlat_hsphere / 2;
  for(j = nlat_hsphere/2; j < nlat_hsphere; j++){
    float r[2], g[2], b[2];
    float tj, tjp1;

    tj   = (float)(j-nlat_hsphere/2)/(float)(nlats+1);
    tjp1 = (float)(j+1-nlat_hsphere/2)/(float)(nlats+1);

    if(use_sky == 0){
      float f1, f2;

      f1 = (float)(j - nlat_hsphere / 2) / (float)nlats;
      f2 = 1.0 - f1;
      r[0] = (f1 * 32.0 + f2 * 160.0) / 256.0;
      g[0] = r[0];
      b[0] = 1.0;

      f1 = (float)(j+1 - nlat_hsphere / 2) / (float)nlats;
      f2 = 1.0 - f1;
      r[1] = (f1 * 32.0 + f2 * 160.0) / 256.0;
      g[1] = r[1];
      b[1] = 1.0;
    }
    for(i = 0; i < nlong_hsphere; i++){
      float x[4], y[4], z[4];
      int ip1;

      ip1 = i + 1;

      x[0] = c_long[i]*c_lat[j];
      y[0] = s_long[i]*c_lat[j];
      z[0] = s_lat[j];

      x[1] = c_long[ip1]*c_lat[j];
      y[1] = s_long[ip1]*c_lat[j];
      z[1] = s_lat[j];

      x[2] = c_long[ip1]*c_lat[j + 1];
      y[2] = s_long[ip1]*c_lat[j + 1];
      z[2] = s_lat[j + 1];

      x[3] = c_long[i]*c_lat[j + 1];
      y[3] = s_long[i]*c_lat[j + 1];
      z[3] = s_lat[j + 1];

      if(use_sky == 1){
        float tx[4], ty[4];

        tx[0] = (float)i/(float)nlong_hsphere;
        ty[0] = tj;

        tx[1] = (float)(ip1)/(float)nlong_hsphere;
        ty[1] = tj;

        tx[2] = (float)(ip1)/(float)nlong_hsphere;
        ty[2] = tjp1;

        tx[3] = (float)i/(float)nlong_hsphere;
        ty[3] = tjp1;

        glNormal3f(x[0], y[0], z[0]);
        glTexCoord2f(tx[0],ty[0]);
        glVertex3f(x[0], y[0], z[0]);

        glNormal3f(x[1], y[1], z[1]);
        glTexCoord2f(tx[1], ty[1]);
        glVertex3f(x[1], y[1], z[1]);

        glNormal3f(x[2], y[2], z[2]);
        glTexCoord2f(tx[2], ty[2]);
        glVertex3f(x[2], y[2], z[2]);

        glNormal3f(x[3], y[3], z[3]);
        glTexCoord2f(tx[3], ty[3]);
        glVertex3f(x[3], y[3], z[3]);

        glNormal3f(-x[0], -y[0], -z[0]);
        glTexCoord2f(tx[0], ty[0]);
        glVertex3f(x[0], y[0], z[0]);

        glNormal3f(-x[3], -y[3], -z[3]);
        glTexCoord2f(tx[3], ty[3]);
        glVertex3f(x[3], y[3], z[3]);

        glNormal3f(-x[2], -y[2], -z[2]);
        glTexCoord2f(tx[2], ty[2]);
        glVertex3f(x[2], y[2], z[2]);

        glNormal3f(-x[1], -y[1], -z[1]);
        glTexCoord2f(tx[1], ty[1]);
        glVertex3f(x[1], y[1], z[1]);
      }
      else{
        glColor3f(r[0], g[0], b[0]);
        glNormal3f(x[0], y[0], z[0]);
        glVertex3f(x[0], y[0], z[0]);

        glNormal3f(x[1], y[1], z[1]);
        glVertex3f(x[1], y[1], z[1]);

        glColor3f(r[1], g[1], b[1]);
        glNormal3f(x[2], y[2], z[2]);
        glVertex3f(x[2], y[2], z[2]);

        glNormal3f(x[3], y[3], z[3]);
        glVertex3f(x[3], y[3], z[3]);

        glColor3f(r[0], g[0], b[0]);
        glNormal3f(-x[0], -y[0], -z[0]);
        glVertex3f(x[0], y[0], z[0]);

        glColor3f(r[1], g[1], b[1]);
        glNormal3f(-x[3], -y[3], -z[3]);
        glVertex3f(x[3], y[3], z[3]);

        glNormal3f(-x[2], -y[2], -z[2]);
        glVertex3f(x[2], y[2], z[2]);

        glColor3f(r[0], g[0], b[0]);
        glNormal3f(-x[1], -y[1], -z[1]);
        glVertex3f(x[1], y[1], z[1]);
      }
    }
  }
  glEnd();
  if(use_sky == 1){
    glDisable(GL_TEXTURE_2D);
  }
  glPopMatrix();
}

/* ----------------------- DrawPoint ----------------------------- */

void DrawPoint(unsigned char *rgbcolor){
  glBegin(GL_POINTS);
  if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
  glVertex3f(0.0,0.0,0.0);
  glEnd();
}


/* ----------------------- DrawRectangle ----------------------------- */

void DrawRectangle(float width,float height, unsigned char *rgbcolor){
  glBegin(GL_LINE_LOOP);
  if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(width, 0.0, 0.0);
  glVertex3f(width, height, 0.0);
  glVertex3f(0.0, height, 0.0);
  glVertex3f(0.0, 0.0, 0.0);

  glEnd();
}

/* ----------------------- DrawFilledRectangle ----------------------------- */

void DrawFilledRectangle(float width,float height, unsigned char *rgbcolor){
  if(object_outlines==0){
    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(width, 0.0, 0.0);
    glVertex3f(width, height, 0.0);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(width, height, 0.0);
    glVertex3f(0.0, height, 0.0);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(width, height, 0.0);
    glVertex3f(width, 0.0, 0.0);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, height, 0.0);
    glVertex3f(width, height, 0.0);
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(width, 0.0, 0.0);

    glVertex3f(width, 0.0, 0.0);
    glVertex3f(width, height, 0.0);

    glVertex3f(width, height, 0.0);
    glVertex3f(0.0, 0.0, 0.0);

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, height, 0.0);

    glVertex3f(0.0, height, 0.0);
    glVertex3f(width, height, 0.0);
    glEnd();
  }
}

/* ----------------------- DrawFilledCircle ----------------------------- */

void DrawFilledCircle(float diameter,unsigned char *rgbcolor, circdata *circinfo){
  int i;
  int ncirc;
  float *xcirc, *ycirc;

  if(circinfo->ncirc==0)InitCircle(CIRCLE_SEGS,circinfo);
  ncirc = circinfo->ncirc;
  xcirc = circinfo->xcirc;
  ycirc = circinfo->ycirc;

  if(object_outlines==0){
    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(i=0;i<ncirc;i++){
      int i2;

      i2 = (i+1)%ncirc;
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
      glVertex3f(0.0,0.0,0.0);
      glVertex3f(diameter*xcirc[  i2]/2.0,diameter*ycirc[  i2]/2.0,0.0);
    }
    for(i=0;i<ncirc;i++){
      int i2;

      i2 = (i+1)%ncirc;
      glVertex3f(diameter*xcirc[  i2]/2.0,diameter*ycirc[  i2]/2.0,0.0);
      glVertex3f(0.0,0.0,0.0);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
    for(i=0;i<ncirc;i++){
      int i2;

      i2 = (i+1)%ncirc;
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
      glVertex3f(0.0,0.0,0.0);
      glVertex3f(diameter*xcirc[  i2]/2.0,diameter*ycirc[  i2]/2.0,0.0);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
    }
    glEnd();
  }
}

/* ----------------------- DrawCircle ----------------------------- */

void DrawCircle(float diameter,unsigned char *rgbcolor, circdata *circinfo){
  int i;
  float *xcirc, *ycirc;

  if(circinfo->ncirc==0)InitCircle(CIRCLE_SEGS,circinfo);
  xcirc = circinfo->xcirc;
  ycirc = circinfo->ycirc;

  glBegin(GL_LINES);
  if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
  for(i=0;i<circinfo->ncirc;i++){
    glVertex3f(diameter*xcirc[  i]/2.0, diameter*ycirc[  i]/2.0,0.0);
    glVertex3f(diameter*xcirc[i+1]/2.0, diameter*ycirc[i+1]/2.0, 0.0);
  }
  glEnd();
}

/* ----------------------- DrawCuboid ----------------------------- */

//     7--------6
//    /|       /
//   / |      / |
//  4--------5  |
//  |  |     |  |
//  |  |     |  |
//  |  3--------2
//  | /      | /
//  |/       |/
//  0--------1

void DrawCuboid(float *origin, float verts[8][3], unsigned char *rgbcolor, int draw_outline){
  if(origin!=NULL){
    glPushMatrix();
    glTranslatef(origin[0], origin[1], origin[2]);
  }
  if(draw_outline==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    // bottom face
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3fv(verts[0]);
    glVertex3fv(verts[3]);
    glVertex3fv(verts[2]);
    glVertex3fv(verts[1]);

    // top face
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3fv(verts[4]);
    glVertex3fv(verts[5]);
    glVertex3fv(verts[6]);
    glVertex3fv(verts[7]);

    // front face
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3fv(verts[0]);
    glVertex3fv(verts[1]);
    glVertex3fv(verts[5]);
    glVertex3fv(verts[4]);

    // back face
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3fv(verts[3]);
    glVertex3fv(verts[7]);
    glVertex3fv(verts[6]);
    glVertex3fv(verts[2]);

    // left face
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3fv(verts[0]);
    glVertex3fv(verts[4]);
    glVertex3fv(verts[7]);
    glVertex3fv(verts[3]);

    // right face
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3fv(verts[1]);
    glVertex3fv(verts[2]);
    glVertex3fv(verts[6]);
    glVertex3fv(verts[5]);
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glVertex3fv(verts[0]); glVertex3fv(verts[1]);
    glVertex3fv(verts[3]); glVertex3fv(verts[2]);
    glVertex3fv(verts[4]); glVertex3fv(verts[5]);
    glVertex3fv(verts[7]); glVertex3fv(verts[6]);

    glVertex3fv(verts[0]); glVertex3fv(verts[3]);
    glVertex3fv(verts[1]); glVertex3fv(verts[2]);
    glVertex3fv(verts[4]); glVertex3fv(verts[7]);
    glVertex3fv(verts[5]); glVertex3fv(verts[6]);

    glVertex3fv(verts[0]); glVertex3fv(verts[4]);
    glVertex3fv(verts[1]); glVertex3fv(verts[5]);
    glVertex3fv(verts[2]); glVertex3fv(verts[6]);
    glVertex3fv(verts[3]); glVertex3fv(verts[7]);
    glEnd();
  }
  if(origin!=NULL)glPopMatrix();
}

/* ----------------------- DrawBox2 ----------------------------- */

void DrawBox2(float *origin, float *dxyz, float *color, int draw_outline){
  if(origin!=NULL){
    glPushMatrix();
    glTranslatef(origin[0], origin[1], origin[2]);
  }
  if(draw_outline==0){
    glBegin(GL_QUADS);
    if(color!=NULL)glColor3fv(color);

    //
    //  0   dx
    // bottom face
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(0.0,     0.0,     0.0);
    glVertex3f(0.0, dxyz[1], 0.0);
    glVertex3f(dxyz[0], dxyz[1], 0.0);
    glVertex3f(dxyz[0], 0.0,     0.0);

    // top face
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, dxyz[2]);
    glVertex3f(dxyz[0], 0.0, dxyz[2]);
    glVertex3f(dxyz[0], dxyz[1], dxyz[2]);
    glVertex3f(0.0, dxyz[1], dxyz[2]);

    // front face
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(dxyz[0], 0.0, 0.0);
    glVertex3f(dxyz[0], 0.0, dxyz[2]);
    glVertex3f(0.0, 0.0, dxyz[2]);

    // back face
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, dxyz[1], 0.0);
    glVertex3f(0.0, dxyz[1], dxyz[2]);
    glVertex3f(dxyz[0], dxyz[1], dxyz[2]);
    glVertex3f(dxyz[0], dxyz[1], 0.0);

    // left face
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, dxyz[2]);
    glVertex3f(0.0, dxyz[1], dxyz[2]);
    glVertex3f(0.0, dxyz[1], 0.0);

    // right face
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(dxyz[0], 0.0, 0.0);
    glVertex3f(dxyz[0], dxyz[1], 0.0);
    glVertex3f(dxyz[0], dxyz[1], dxyz[2]);
    glVertex3f(dxyz[0], 0.0, dxyz[2]);
    glEnd();
  }
  else{
    glLineWidth(4.0);
    glBegin(GL_LINES);
    if(color!=NULL)glColor3fv(color);
    glVertex3f(0.0, 0.0,     0.0);     glVertex3f(dxyz[0], 0.0,     0.0);
    glVertex3f(0.0, dxyz[1], 0.0);     glVertex3f(dxyz[0], dxyz[1], 0.0);
    glVertex3f(0.0, dxyz[1], dxyz[2]); glVertex3f(dxyz[0], dxyz[1], dxyz[2]);
    glVertex3f(0.0, 0.0,     dxyz[2]); glVertex3f(dxyz[0], 0.0,     dxyz[2]);

    glVertex3f(0.0,     0.0, 0.0);     glVertex3f(0.0,     dxyz[1], 0.0);
    glVertex3f(dxyz[0], 0.0, 0.0);     glVertex3f(dxyz[0], dxyz[1], 0.0);
    glVertex3f(dxyz[0], 0.0, dxyz[2]); glVertex3f(dxyz[0], dxyz[1], dxyz[2]);
    glVertex3f(0.0,     0.0, dxyz[2]); glVertex3f(0.0,     dxyz[1], dxyz[2]);

    glVertex3f(0.0,     0.0,     0.0); glVertex3f(0.0,     0.0,     dxyz[2]);
    glVertex3f(0.0,     dxyz[1], 0.0); glVertex3f(0.0,     dxyz[1], dxyz[2]);
    glVertex3f(dxyz[0], dxyz[1], 0.0); glVertex3f(dxyz[0], dxyz[1], dxyz[2]);
    glVertex3f(dxyz[0], 0.0,     0.0); glVertex3f(dxyz[0], 0.0,     dxyz[2]);
    glEnd();
  }
  if(origin!=NULL)glPopMatrix();
}

/* ----------------------- DrawCubeC ----------------------------- */

void DrawCubeC(float size, unsigned char *rgbcolor){
  float s1=size/2.0;
  float verts[8][3]={
    {-s1,-s1,-s1},{s1,-s1,-s1},{s1,s1,-s1},{-s1,s1,-s1},
    {-s1,-s1, s1},{s1,-s1, s1},{s1,s1, s1},{-s1,s1, s1}
  };

  DrawCuboid(NULL,verts,rgbcolor,object_outlines);
}


/* ----------------------- DrawPrismXyz ----------------------------- */

void DrawPrismXyz(float *args, unsigned char *rgbcolor){
// 0,1,2 - origin
// 3,4,5 - size1
// 6       size2
  float verts[8][3] = {
    {0.0, 0.0, 0.0},     {args[3], 0.0, 0.0},     {args[3], args[4], 0.0},     {0.0, args[4], 0.0},
    {0.0, 0.0, args[5]}, {args[3], 0.0, args[6]}, {args[3], args[4], args[6]}, {0.0, args[4], args[5]}
  };

  DrawCuboid(args, verts, rgbcolor, object_outlines);
}

/* ----------------------- DrawBoxXyz ----------------------------- */

void DrawBoxXyz(float *args, unsigned char *rgbcolor){
  float verts[8][3] = {
    {0.0, 0.0, 0.0},     {args[3], 0.0, 0.0},     {args[3], args[4], 0.0},     {0.0, args[4], 0.0},
    {0.0, 0.0, args[5]}, {args[3], 0.0, args[5]}, {args[3], args[4], args[5]}, {0.0, args[4], args[5]}
  };
  DrawCuboid(args, verts, rgbcolor, object_outlines);
}

/* ----------------------- DrawCube ----------------------------- */

void DrawCube(float size, unsigned char *rgbcolor){
  float s2 = size/2.0;

  float verts[8][3] = {
    {-s2, -s2, -s2}, {s2, -s2, -s2}, {s2, s2, -s2}, {-s2, s2, -s2},
    {-s2, -s2,  s2}, {s2, -s2,  s2}, {s2, s2,  s2}, {-s2, s2,  s2},
  };

  DrawCuboid(NULL, verts, rgbcolor, object_outlines);
}

/* ----------------------- DrawTriBlock ----------------------------- */

void DrawTriBlock(float s, float h, unsigned char *rgbcolor){
  float sd2;
  float ny=0.0, nz=1.0, denom;

  sd2 = s/2.0;
  if(object_outlines==0){
    denom = sqrt(h*h+sd2*sd2);
    if(denom>0.0){
      ny = h/denom;
      nz = sd2/denom;
    }
    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glNormal3f( 0.0, 0.0,-1.0);
    glVertex3f( 0.0, 0.0, 0.0);  // 1
    glVertex3f( 0.0,   s, 0.0);  // 4
    glVertex3f(   s,   s, 0.0);  // 3
    glVertex3f( 0.0, 0.0, 0.0);  // 1
    glVertex3f(   s,   s, 0.0);  // 3
    glVertex3f(   s, 0.0, 0.0);  // 2

    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f( 0.0, 0.0, 0.0);  // 1
    glVertex3f( 0.0, sd2,   h);  // 6
    glVertex3f( 0.0,   s, 0.0);  // 4

    glNormal3f( 1.0, 0.0, 0.0);
    glVertex3f(   s, 0.0, 0.0);  // 2
    glVertex3f(   s,   s, 0.0);  // 3
    glVertex3f(   s, sd2,   h);  // 5

    glNormal3f( 0.0, -ny,  nz);
    glVertex3f( 0.0, 0.0, 0.0);  // 1
    glVertex3f(   s, sd2,   h);  // 5
    glVertex3f( 0.0, sd2,   h);  // 6
    glVertex3f( 0.0, 0.0, 0.0);  // 1
    glVertex3f(   s, 0.0, 0.0);  // 2
    glVertex3f(   s, sd2,   h);  // 5

    glNormal3f( 0.0,  ny,  nz);
    glVertex3f(   s,   s, 0.0);  // 3
    glVertex3f( 0.0, sd2,   h);  // 6
    glVertex3f(   s, sd2,   h);  // 5
    glVertex3f(   s,   s, 0.0);  // 3
    glVertex3f( 0.0,   s, 0.0);  // 4
    glVertex3f( 0.0, sd2,   h);  // 6

    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glVertex3f( 0.0, 0.0, 0.0);  // 1
    glVertex3f(   s, 0.0, 0.0);  // 2
    glVertex3f(   s, 0.0, 0.0);  // 2
    glVertex3f(   s,   s, 0.0);  // 3
    glVertex3f(   s,   s, 0.0);  // 3
    glVertex3f( 0.0,   s, 0.0);  // 4
    glVertex3f( 0.0,   s, 0.0);  // 4
    glVertex3f( 0.0, 0.0, 0.0);  // 1

    glVertex3f( 0.0, 0.0, 0.0);  // 1
    glVertex3f( 0.0, sd2,   h);  // 6
    glVertex3f( 0.0, sd2,   h);  // 6
    glVertex3f( 0.0,   s, 0.0);  // 4

    glVertex3f(   s, 0.0, 0.0);  // 2
    glVertex3f(   s, sd2,   h);  // 5
    glVertex3f(   s, sd2,   h);  // 5
    glVertex3f(   s,   s, 0.0);  // 3

    glVertex3f( 0.0, sd2,   h);  // 6
    glVertex3f(   s, sd2,   h);  // 5

    glEnd();
  }
}

/* ----------------------- DrawVent ----------------------------- */

void DrawVent(float width, float height, unsigned char *rgbcolor){
  float wd2, hd2, dw, dh;
  int i;
  float dslot;
#define NSLOTS 10
#define FACTOR 2

  dslot = height/(NSLOTS+NSLOTS-1+FACTOR+FACTOR);
  wd2 = width/2.0;
  hd2 = height/2.0;
  dw = dslot*FACTOR;
  dh = dw;

  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0,0.0,1.0);

    glVertex3f(-wd2,   -hd2,0.0);
    glVertex3f(-wd2+dw,-hd2,0.0);
    glVertex3f(-wd2+dw, hd2,0.0);
    glVertex3f(-wd2,    hd2,0.0);

    glVertex3f(wd2-dw,-hd2,0.0);
    glVertex3f(wd2,   -hd2,0.0);
    glVertex3f(wd2,    hd2,0.0);
    glVertex3f(wd2-dw, hd2,0.0);

    glVertex3f(-wd2+dw,-hd2,   0.0);
    glVertex3f( wd2-dw,-hd2,   0.0);
    glVertex3f( wd2-dw,-hd2+dh,0.0);
    glVertex3f(-wd2+dw,-hd2+dh,0.0);

    glVertex3f(-wd2+dw,hd2-dh,   0.0);
    glVertex3f( wd2-dw,hd2-dh,   0.0);
    glVertex3f( wd2-dw,hd2   ,0.0);
    glVertex3f(-wd2+dw,hd2   ,0.0);

    glNormal3f(0.0,0.0,-1.0);
    glVertex3f(-wd2,    hd2,0.0);
    glVertex3f(-wd2+dw, hd2,0.0);
    glVertex3f(-wd2+dw,-hd2,0.0);
    glVertex3f(-wd2,   -hd2,0.0);

    glVertex3f(wd2-dw, hd2,0.0);
    glVertex3f(wd2,    hd2,0.0);
    glVertex3f(wd2,   -hd2,0.0);
    glVertex3f(wd2-dw,-hd2,0.0);

    glVertex3f(-wd2+dw,-hd2+dh,0.0);
    glVertex3f( wd2-dw,-hd2+dh,0.0);
    glVertex3f( wd2-dw,-hd2,   0.0);
    glVertex3f(-wd2+dw,-hd2,   0.0);

    glVertex3f(-wd2+dw,hd2   ,0.0);
    glVertex3f( wd2-dw,hd2   ,0.0);
    glVertex3f( wd2-dw,hd2-dh,   0.0);
    glVertex3f(-wd2+dw,hd2-dh,   0.0);

    glNormal3f(0.0,0.0,1.0);
    for(i=0;i<NSLOTS;i++){
      float yy, yy2;

      yy = -hd2+(2*i+FACTOR+1)*dslot;
      yy2 = yy + dslot;
      glVertex3f(-wd2,yy,0.0);
      glVertex3f( wd2,yy,0.0);
      glVertex3f( wd2,yy2,0.0);
      glVertex3f(-wd2,yy2,0.0);
    }
    glNormal3f(0.0,0.0,-1.0);
    for(i=0;i<NSLOTS;i++){
      float yy, yy2;

      yy = -hd2+(2*i+FACTOR+1)*dslot;
      yy2 = yy + dslot;
      glVertex3f(-wd2,yy2,0.0);
      glVertex3f( wd2,yy2,0.0);
      glVertex3f( wd2,yy,0.0);
      glVertex3f(-wd2,yy,0.0);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glVertex3f(-wd2,   -hd2,0.0);
    glVertex3f(-wd2+dw,-hd2,0.0);

    glVertex3f(-wd2+dw,-hd2,0.0);
    glVertex3f(-wd2+dw, hd2,0.0);

    glVertex3f(-wd2+dw, hd2,0.0);
    glVertex3f(-wd2,    hd2,0.0);

    glVertex3f(-wd2,    hd2,0.0);
    glVertex3f(-wd2,   -hd2,0.0);

    glVertex3f(wd2-dw,-hd2,0.0);
    glVertex3f(wd2,   -hd2,0.0);

    glVertex3f(wd2,   -hd2,0.0);
    glVertex3f(wd2,    hd2,0.0);

    glVertex3f(wd2,    hd2,0.0);
    glVertex3f(wd2-dw, hd2,0.0);

    glVertex3f(wd2-dw, hd2,0.0);
    glVertex3f(wd2-dw,-hd2,0.0);

    glVertex3f(-wd2+dw,-hd2,   0.0);
    glVertex3f( wd2-dw,-hd2,   0.0);

    glVertex3f( wd2-dw,-hd2,   0.0);
    glVertex3f( wd2-dw,-hd2+dh,0.0);

    glVertex3f( wd2-dw,-hd2+dh,0.0);
    glVertex3f(-wd2+dw,-hd2+dh,0.0);

    glVertex3f(-wd2+dw,-hd2+dh,0.0);
    glVertex3f(-wd2+dw,-hd2,   0.0);

    glVertex3f(-wd2+dw,hd2-dh,   0.0);
    glVertex3f( wd2-dw,hd2-dh,   0.0);

    glVertex3f( wd2-dw,hd2-dh,   0.0);
    glVertex3f( wd2-dw,hd2   ,0.0);

    glVertex3f( wd2-dw,hd2   ,0.0);
    glVertex3f(-wd2+dw,hd2   ,0.0);

    glVertex3f(-wd2+dw,hd2   ,0.0);
    glVertex3f(-wd2+dw,hd2-dh,   0.0);

    glVertex3f(-wd2,    hd2,0.0);
    glVertex3f(-wd2+dw, hd2,0.0);

    glVertex3f(-wd2+dw, hd2,0.0);
    glVertex3f(-wd2+dw,-hd2,0.0);

    glVertex3f(-wd2+dw,-hd2,0.0);
    glVertex3f(-wd2,   -hd2,0.0);

    glVertex3f(-wd2,   -hd2,0.0);
    glVertex3f(-wd2,    hd2,0.0);

    glVertex3f(wd2-dw, hd2,0.0);
    glVertex3f(wd2,    hd2,0.0);

    glVertex3f(wd2,    hd2,0.0);
    glVertex3f(wd2,   -hd2,0.0);

    glVertex3f(wd2,   -hd2,0.0);
    glVertex3f(wd2-dw,-hd2,0.0);

    glVertex3f(wd2-dw,-hd2,0.0);
    glVertex3f(wd2-dw, hd2,0.0);

    glVertex3f(-wd2+dw,-hd2+dh,0.0);
    glVertex3f( wd2-dw,-hd2+dh,0.0);

    glVertex3f( wd2-dw,-hd2+dh,0.0);
    glVertex3f( wd2-dw,-hd2,   0.0);

    glVertex3f( wd2-dw,-hd2,   0.0);
    glVertex3f(-wd2+dw,-hd2,   0.0);

    glVertex3f(-wd2+dw,-hd2,   0.0);
    glVertex3f(-wd2+dw,-hd2+dh,0.0);

    glVertex3f(-wd2+dw,hd2   ,0.0);
    glVertex3f( wd2-dw,hd2   ,0.0);

    glVertex3f( wd2-dw,hd2   ,0.0);
    glVertex3f( wd2-dw,hd2-dh,   0.0);

    glVertex3f( wd2-dw,hd2-dh,   0.0);
    glVertex3f(-wd2+dw,hd2-dh,   0.0);

    glVertex3f(-wd2+dw,hd2-dh,   0.0);
    glVertex3f(-wd2+dw,hd2   ,0.0);

    for(i=0;i<NSLOTS-1;i++){
      float yy, yy2;

      yy = -hd2+(2*i+FACTOR+1)*dslot;
      yy2 = yy + dslot;
      glVertex3f(-wd2+dw,yy2,0.0);
      glVertex3f( wd2-dw,yy2,0.0);

      glVertex3f( wd2-dw,yy2,0.0);
      glVertex3f( wd2-dw,yy,0.0);

      glVertex3f( wd2-dw,yy,0.0);
      glVertex3f(-wd2+dw,yy,0.0);

      glVertex3f(-wd2+dw,yy,0.0);
      glVertex3f(-wd2+dw,yy2,0.0);
    }
    glEnd();
  }

}

/* ----------------------- DrawSquare ----------------------------- */

void DrawSquare(float size, unsigned char *rgbcolor){
  float s2;

  s2 = size/2.0;

  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0,0.0,-1.0);
    glVertex3f(-s2,-s2,0.0);  // 1
    glVertex3f(-s2, s2,0.0);  // 4
    glVertex3f( s2, s2,0.0);  // 3
    glVertex3f( s2,-s2,0.0);  // 2

    glNormal3f(0.0,0.0,1.0);
    glVertex3f( s2,-s2,0.0);  // 2
    glVertex3f( s2, s2,0.0);  // 3
    glVertex3f(-s2, s2,0.0);  // 4
    glVertex3f(-s2,-s2,0.0);  // 1
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glVertex3f(-s2,-s2,0.0);  // 1
    glVertex3f(-s2, s2,0.0);  // 4

    glVertex3f(-s2, s2,0.0);  // 4
    glVertex3f( s2, s2,0.0);  // 3

    glVertex3f( s2, s2,0.0);  // 3
    glVertex3f( s2,-s2,0.0);  // 2

    glVertex3f( s2,-s2,0.0);  // 2
    glVertex3f(-s2,-s2,0.0);  // 1
    glEnd();
  }

}

/* ----------------------- DrawRing ----------------------------- */

void DrawRing(float diam_inner, float diam_outer, float height, unsigned char *rgbcolor){
  int i;
  int ncirc;
  float *xcirc, *ycirc;

  if(object_circ.ncirc==0)InitCircle(CIRCLE_SEGS,&object_circ);
  ncirc = object_circ.ncirc;
  xcirc = object_circ.xcirc;
  ycirc = object_circ.ycirc;

  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glNormal3f(xcirc[i],ycirc[i],0.0);
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,0.0); // 1

      glNormal3f(xcirc[i+1],ycirc[i+1],0.0);
      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,0.0); // 2

      glNormal3f(xcirc[i+1],ycirc[i+1],0.0);
      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0, height); // 3

      glNormal3f(xcirc[i],ycirc[i],0.0);
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0, height); // 4

      glNormal3f(xcirc[i],ycirc[i],0.0);
      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,0.0); // 1

      glNormal3f(xcirc[i],ycirc[i],0.0);
      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0, height); // 4

      glNormal3f(xcirc[i+1],ycirc[i+1],0.0);
      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0, height); // 3

      glNormal3f(xcirc[i+1],ycirc[i+1],0.0);
      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,0.0); // 2
    }
    glNormal3f(0.0,0.0,1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,height);
      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,height);
      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,height);
      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,height);
    }
    glNormal3f(0.0,0.0,-1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,0.0);
      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,0.0);
      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,0.0);
      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,0.0);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,0.0); // 1
      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,0.0); // 2

      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,0.0); // 2
      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0, height); // 3

      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0, height); // 3
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0, height); // 4

      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0, height); // 4
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,0.0); // 1

      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,0.0); // 1
      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0, height); // 4

      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0, height); // 4
      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0, height); // 3

      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0, height); // 3
      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,0.0); // 2

      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,0.0); // 2
      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,0.0); // 1
    }
    for(i=0;i<ncirc;i++){
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,height);
      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,height);

      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,height);
      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,height);

      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,height);
      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,height);

      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,height);
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,height);
    }
    for(i=0;i<ncirc;i++){
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,0.0);
      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,0.0);

      glVertex3f(diam_inner*xcirc[  i]/2.0,diam_inner*ycirc[  i]/2.0,0.0);
      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,0.0);

      glVertex3f(diam_inner*xcirc[i+1]/2.0,diam_inner*ycirc[i+1]/2.0,0.0);
      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,0.0);

      glVertex3f(diam_outer*xcirc[i+1]/2.0,diam_outer*ycirc[i+1]/2.0,0.0);
      glVertex3f(diam_outer*xcirc[  i]/2.0,diam_outer*ycirc[  i]/2.0,0.0);
    }
    glEnd();
  }

}

/* ----------------------- RotateXYZ ----------------------------- */

void RotateXYZ(float x, float y, float z){
  float angle;
  float normxy, normxyz;

  normxy = x*x + y*y;
  normxy = sqrt(normxy);
  if(normxy < 0.00001)return;
  normxyz = x*x + y*y + z*z;
  normxyz = sqrt(normxyz);
  if(normxyz < 0.00001)return;
  angle = RAD2DEG*acos(z / normxyz);
  glRotatef(angle, -y / normxy, x / normxy, 0.0);
}

/* ----------------------- RotateEye ----------------------------- */

void RotateEye(void){
  RotateXYZ(partfacedir[0],partfacedir[1],partfacedir[2]);
}

/* ----------------------- RotateAxis ----------------------------- */

void RotateAxis(float angle, float ax, float ay, float az){
  glRotatef(angle,ax,ay,az);
}

/* ----------------------- DrawWheel ----------------------------- */

void DrawWheel(float diameter, float thickness, unsigned char *rgbcolor){
  glPushMatrix();
  glRotatef(90.0, 1.0, 0.0, 0.0);
  DrawDisk(diameter, thickness, rgbcolor);
  glPopMatrix();
}

/* ----------------------- DrawDisk ----------------------------- */

void DrawDisk(float diameter, float height, unsigned char *rgbcolor){
  int i;
  int ncirc;
  float *xcirc, *ycirc;

  if(object_circ.ncirc==0)InitCircle(CIRCLE_SEGS,&object_circ);
  ncirc = object_circ.ncirc;
  xcirc = object_circ.xcirc;
  ycirc = object_circ.ycirc;

  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glNormal3f(xcirc[i],ycirc[i],0.0);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1

      glNormal3f(xcirc[i+1],ycirc[i+1],0.0);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0); // 2

      glNormal3f(xcirc[i+1],ycirc[i+1],0.0);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height); // 3

      glNormal3f(xcirc[i],ycirc[i],0.0);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height); // 4
    }
    glEnd();

    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0,0.0,-1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
      glVertex3f(                    0.0,                    0.0,0.0);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0);
    }
    glNormal3f(0.0,0.0,1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height);
      glVertex3f(                    0.0,                    0.0, height);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0); // 2

      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0); // 2
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height); // 3

      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height); // 3
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height); // 4

      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height); // 4
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1
    }
    glEnd();

    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
      glVertex3f(                    0.0,                    0.0,0.0);

      glVertex3f(                    0.0,                    0.0,0.0);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0);

      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
    }
    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height);

      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height);
      glVertex3f(                    0.0,                    0.0, height);

      glVertex3f(                    0.0,                    0.0, height);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height);
    }
    glEnd();
  }
}

/* ----------------------- DrawPolyDisk ----------------------------- */

void DrawPolyDisk(int nsides, float diameter, float height, unsigned char *rgbcolor){
  int i;

  float x[33], y[33], xnorm[32], ynorm[32];
  float radius;
  float factor,factor2;

  if(nsides>32)nsides=32;
  if(nsides<3)nsides=3;

  factor=2.0*PI/nsides;
  factor2 = factor/2.0;

  for(i=0;i<nsides;i++){
    x[i]=cos(i*factor);
    y[i]=sin(i*factor);
    xnorm[i] = cos(factor2+i*factor);
    ynorm[i] = sin(factor2+i*factor);
  }
  x[nsides] = x[0];
  y[nsides] = y[0];

  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    radius = diameter/2.0;

    for(i=0;i<nsides;i++){
      glNormal3f(xnorm[i], ynorm[i], 0.0);
      glVertex3f(radius*x[  i],radius*y[  i],0.0); // 1

      glVertex3f(radius*x[i+1],radius*y[i+1],0.0); // 2

      glVertex3f(radius*x[i+1],radius*y[i+1], height); // 3

      glVertex3f(radius*x[  i],radius*y[  i], height); // 4
    }
    glEnd();

    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0,0.0,-1.0);
    for(i=0;i<nsides;i++){
      glVertex3f(radius*x[  i],radius*y[  i],0.0);
      glVertex3f(            0.0,            0.0,0.0);
      glVertex3f(radius*x[i+1],radius*y[i+1],0.0);
    }
    glNormal3f(0.0,0.0,1.0);
    for(i=0;i<nsides;i++){
      glVertex3f(radius*x[  i],radius*y[  i], height);
      glVertex3f(radius*x[i+1],radius*y[i+1], height);
      glVertex3f(            0.0,            0.0, height);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    radius = diameter/2.0;

    for(i=0;i<nsides;i++){
      glVertex3f(radius*x[  i],radius*y[  i],0.0); // 1
      glVertex3f(radius*x[i+1],radius*y[i+1],0.0); // 2

      glVertex3f(radius*x[i+1],radius*y[i+1],0.0); // 2
      glVertex3f(radius*x[i+1],radius*y[i+1], height); // 3

      glVertex3f(radius*x[i+1],radius*y[i+1], height); // 3
      glVertex3f(radius*x[  i],radius*y[  i], height); // 4

      glVertex3f(radius*x[  i],radius*y[  i], height); // 4
      glVertex3f(radius*x[  i],radius*y[  i],0.0); // 1
    }
    glEnd();

    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<nsides;i++){
      glVertex3f(radius*x[  i],radius*y[  i],0.0);
      glVertex3f(            0.0,            0.0,0.0);

      glVertex3f(            0.0,            0.0,0.0);
      glVertex3f(radius*x[i+1],radius*y[i+1],0.0);

      glVertex3f(radius*x[i+1],radius*y[i+1],0.0);
      glVertex3f(radius*x[  i],radius*y[  i],0.0);
    }
    for(i=0;i<nsides;i++){
      glVertex3f(radius*x[  i],radius*y[  i], height);
      glVertex3f(radius*x[i+1],radius*y[i+1], height);

      glVertex3f(radius*x[i+1],radius*y[i+1], height);
      glVertex3f(            0.0,            0.0, height);

      glVertex3f(            0.0,            0.0, height);
      glVertex3f(radius*x[  i],radius*y[  i], height);
    }
    glEnd();
  }
}

/* ----------------------- DrawHexDisk ----------------------------- */

void DrawHexDisk(float diameter, float height, unsigned char *rgbcolor){
  int i;

  float x[7]={0.866,0.0,-0.866,-0.866,0.0 ,0.866,0.866};
  float y[7]={0.5,  1.0, 0.5,  -0.5, -1.0,-0.5,  0.5};
  float xnorm[6]={0.500, -0.500, -1.0,-0.500,  0.500, 1.0};
  float ynorm[6]={0.866,  0.866,  0.0,-0.866, -0.866, 0.0};
  float radius;

  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    radius = diameter/2.0;

    for(i=0;i<6;i++){
      glNormal3f(xnorm[i], ynorm[i], 0.0);
      glVertex3f(radius*x[  i],radius*y[  i],0.0); // 1

      glVertex3f(radius*x[i+1],radius*y[i+1],0.0); // 2

      glVertex3f(radius*x[i+1],radius*y[i+1], height); // 3

      glVertex3f(radius*x[  i],radius*y[  i], height); // 4
    }
    glEnd();

    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0,0.0,-1.0);
    for(i=0;i<6;i++){
      glVertex3f(radius*x[  i],radius*y[  i],0.0);
      glVertex3f(            0.0,            0.0,0.0);
      glVertex3f(radius*x[i+1],radius*y[i+1],0.0);
    }
    glNormal3f(0.0,0.0,1.0);
    for(i=0;i<6;i++){
      glVertex3f(radius*x[  i],radius*y[  i], height);
      glVertex3f(radius*x[i+1],radius*y[i+1], height);
      glVertex3f(            0.0,            0.0, height);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    radius = diameter/2.0;

    for(i=0;i<6;i++){
      glVertex3f(radius*x[  i],radius*y[  i],0.0); // 1
      glVertex3f(radius*x[i+1],radius*y[i+1],0.0); // 2

      glVertex3f(radius*x[i+1],radius*y[i+1],0.0); // 2
      glVertex3f(radius*x[i+1],radius*y[i+1], height); // 3

      glVertex3f(radius*x[i+1],radius*y[i+1], height); // 3
      glVertex3f(radius*x[  i],radius*y[  i], height); // 4

      glVertex3f(radius*x[  i],radius*y[  i], height); // 4
      glVertex3f(radius*x[  i],radius*y[  i],0.0); // 1
    }
    glEnd();

    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<6;i++){
      glVertex3f(radius*x[  i],radius*y[  i],0.0);
      glVertex3f(            0.0,            0.0,0.0);

      glVertex3f(            0.0,            0.0,0.0);
      glVertex3f(radius*x[i+1],radius*y[i+1],0.0);

      glVertex3f(radius*x[i+1],radius*y[i+1],0.0);
      glVertex3f(radius*x[  i],radius*y[  i],0.0);
    }
    for(i=0;i<6;i++){
      glVertex3f(radius*x[  i],radius*y[  i], height);
      glVertex3f(radius*x[i+1],radius*y[i+1], height);

      glVertex3f(radius*x[i+1],radius*y[i+1], height);
      glVertex3f(            0.0,            0.0, height);

      glVertex3f(            0.0,            0.0, height);
      glVertex3f(radius*x[  i],radius*y[  i], height);
    }
    glEnd();
  }
}

/* ----------------------- DrawNotchPlate ----------------------------- */

void DrawNotchPlate(float diameter, float height, float notchheight, float direction, unsigned char *rgbcolor){
  int i;
  float diameter2;

  int ncirc;
  float *xcirc, *ycirc;

  if(object_circ.ncirc==0)InitCircle(CIRCLE_SEGS,&object_circ);
  ncirc = object_circ.ncirc;
  xcirc = object_circ.xcirc;
  ycirc = object_circ.ycirc;

  diameter2 = diameter + notchheight;

  if(object_outlines==0){
    if(cullfaces==1)glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      float xmid, ymid;

      xmid = (xcirc[i]+xcirc[i+1])/2.0;
      ymid = (ycirc[i]+ycirc[i+1])/2.0;

      glNormal3f(xcirc[i],ycirc[i],0.0);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1

      glNormal3f(xcirc[i+1],ycirc[i+1],0.0);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0); // 2

      glNormal3f(xcirc[i+1],ycirc[i+1],0.0);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height); // 3

      glNormal3f(xcirc[i],ycirc[i],0.0);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height); // 4

    // draw notch

      if(direction<0.0){
        glNormal3f(xcirc[i],ycirc[i],0.0);
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1

        glNormal3f(xcirc[i],ycirc[i],0.0);
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, 0.0-notchheight); // 4

        glNormal3f(xmid,ymid,0.0);
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0, 0.0-notchheight); // 3

        glNormal3f(xmid,ymid,0.0);
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 2
      }
      else{
      // top plate
        glNormal3f(0.0,0.0,1.0);
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,height); // 1t

        glNormal3f(0.0,0.0,1.0);
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t

        glNormal3f(0.0,0.0,1.0);
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 3t

        glNormal3f(0.0,0.0,1.0);
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,height); // 2t

      // bottom plate

        glNormal3f(0.0,0.0,-1.0);
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1b

        glNormal3f(0.0,0.0,-1.0);
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 2b

        glNormal3f(0.0,0.0,-1.0);
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 3b

        glNormal3f(0.0,0.0,-1.0);
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b

      // front plate

        glNormal3f(xcirc[i],ycirc[i],0.0);
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t-1

        glNormal3f(xcirc[i],ycirc[i],0.0);
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b-4

        glNormal3f(xmid,ymid,0.0);
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 3b-3

        glNormal3f(xmid,ymid,0.0);
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 3t-2

      // left plate

        glNormal3f(-ycirc[i],xcirc[i],0.0);
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,height); // 1t

        glNormal3f(-ycirc[i],xcirc[i],0.0);
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t

        glNormal3f(-ycirc[i],xcirc[i],0.0);
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b

        glNormal3f(-ycirc[i],xcirc[i],0.0);
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1b

      // right plate

        glNormal3f(ymid,-xmid,0.0);
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,height); // 1t

        glNormal3f(ymid,-xmid,0.0);
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 1b

        glNormal3f(ymid,-xmid,0.0);
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 4b

        glNormal3f(ymid,-xmid,0.0);
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 4t
      }
    }
    glEnd();

    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0,0.0,-1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
      glVertex3f(                    0.0,                    0.0,0.0);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0);
    }
    glNormal3f(0.0,0.0,1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height);
      glVertex3f(                    0.0,                    0.0, height);
    }
    glEnd();
    if(cullfaces==1)glEnable(GL_CULL_FACE);
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      float xmid, ymid;

      xmid = (xcirc[i]+xcirc[i+1])/2.0;
      ymid = (ycirc[i]+ycirc[i+1])/2.0;

      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0); // 2

      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0); // 2
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height); // 3

      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height); // 3
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height); // 4

      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height); // 4
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1

    // draw notch

      if(direction<0.0){
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, 0.0-notchheight); // 4

        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, 0.0-notchheight); // 4
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0, 0.0-notchheight); // 3

        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0, 0.0-notchheight); // 3
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 2

        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 2
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1
      }
      else{
      // top plate
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,height); // 1t
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t

        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 3t

        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 3t
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,height); // 2t

        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,height); // 2t
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,height); // 1t

      // bottom plate

        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1b
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 2b

        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 2b
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 3b

        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 3b
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b

        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1b

      // front plate

        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t-1
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b-4

        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b-4
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 3b-3

        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 3b-3
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 3t-2

        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 3t-2
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t-1

      // left plate

        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,height); // 1t
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t

        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, height); // 4t
        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b

        glVertex3f(diameter2*xcirc[  i]/2.0,diameter2*ycirc[  i]/2.0, 0.0); // 4b
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1b

        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0); // 1b
        glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,height); // 1t

      // right plate

        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,height); // 1t
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 1b

        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,0.0); // 1b
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 4b

        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, 0.0); // 4b
        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 4t

        glVertex3f(diameter2*xmid/2.0,diameter2*ymid/2.0, height); // 4t
        glVertex3f(diameter*xmid/2.0,diameter*ymid/2.0,height); // 1t
      }
    }
    glEnd();

    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
      glVertex3f(                    0.0,                    0.0,0.0);

      glVertex3f(                    0.0,                    0.0,0.0);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0);

      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0,0.0);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0,0.0);
    }
    for(i=0;i<ncirc;i++){
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height);
      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height);

      glVertex3f(diameter*xcirc[i+1]/2.0,diameter*ycirc[i+1]/2.0, height);
      glVertex3f(                    0.0,                    0.0, height);

      glVertex3f(                    0.0,                    0.0, height);
      glVertex3f(diameter*xcirc[  i]/2.0,diameter*ycirc[  i]/2.0, height);
    }
    glEnd();
  }
}

/* ----------------------- DrawCone ----------------------------- */

void DrawCone(float d1, float height, unsigned char *rgbcolor){
  int i;
  float factor, denom, rad;
  float hdr;
  int ncirc;
  float *xcirc, *ycirc;

  if(object_circ.ncirc==0)InitCircle(CIRCLE_SEGS,&object_circ);
  ncirc = object_circ.ncirc;
  xcirc = object_circ.xcirc;
  ycirc = object_circ.ycirc;
  if(height<=0.0)height=0.0001;

  rad = d1/2.0;
  hdr = height/rad;
  denom = 1.0/sqrt(1.0+hdr*hdr);
  factor = hdr*denom;

  if(object_outlines==0){
    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glNormal3f(factor*xcirc[i],factor*ycirc[i],denom);
      glVertex3f(rad*xcirc[  i],rad*ycirc[  i],0.0); // 1

      glNormal3f(factor*xcirc[i+1],factor*ycirc[i+1],denom);
      glVertex3f(rad*xcirc[i+1],rad*ycirc[i+1],0.0); // 2

      glNormal3f(factor*xcirc[i],factor*ycirc[i],denom);
      glVertex3f(0.0,0.0, height); // 3
    }
    glNormal3f(0.0,0.0,-1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(rad*xcirc[  i],rad*ycirc[  i],0.0);
      glVertex3f(                    0.0,                    0.0,0.0);
      glVertex3f(rad*xcirc[i+1],rad*ycirc[i+1],0.0);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glVertex3f(rad*xcirc[  i],rad*ycirc[  i],0.0); // 1
      glVertex3f(rad*xcirc[i+1],rad*ycirc[i+1],0.0); // 2

      glVertex3f(rad*xcirc[i+1],rad*ycirc[i+1],0.0); // 2
      glVertex3f(0.0,0.0, height); // 3

      glVertex3f(0.0,0.0, height); // 3
      glVertex3f(rad*xcirc[  i],rad*ycirc[  i],0.0); // 1
    }
    for(i=0;i<ncirc;i++){
      glVertex3f(rad*xcirc[  i],rad*ycirc[  i],0.0);
      glVertex3f(                    0.0,                    0.0,0.0);

      glVertex3f(                    0.0,                    0.0,0.0);
      glVertex3f(rad*xcirc[i+1],rad*ycirc[i+1],0.0);

      glVertex3f(rad*xcirc[i+1],rad*ycirc[i+1],0.0);
      glVertex3f(rad*xcirc[  i],rad*ycirc[  i],0.0);
    }
    glEnd();
  }
}

/* ----------------------- DrawTruncCone ----------------------------- */

void DrawTruncCone(float d1, float d2, float height, unsigned char *rgbcolor){
  int i;
  float dz;
  int ncirc;
  float *xcirc, *ycirc;

  if(object_circ.ncirc==0)InitCircle(CIRCLE_SEGS,&object_circ);
  ncirc = object_circ.ncirc;
  xcirc = object_circ.xcirc;
  ycirc = object_circ.ycirc;

  if(height<=0.0)height=0.0001;
  dz = -(d2-d1)/height;
  if(object_outlines==0){
    glBegin(GL_QUADS);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glNormal3f(xcirc[i],ycirc[i],dz);
      glVertex3f(d1*xcirc[  i]/2.0,d1*ycirc[  i]/2.0,0.0); // 1

      glNormal3f(xcirc[i+1],ycirc[i+1],dz);
      glVertex3f(d1*xcirc[i+1]/2.0,d1*ycirc[i+1]/2.0,0.0); // 2

      glNormal3f(xcirc[i+1],ycirc[i+1],dz);
      glVertex3f(d2*xcirc[i+1]/2.0,d2*ycirc[i+1]/2.0, height); // 3

      glNormal3f(xcirc[i],ycirc[i],dz);
      glVertex3f(d2*xcirc[  i]/2.0,d2*ycirc[  i]/2.0, height); // 4
    }
    glEnd();

    glBegin(GL_TRIANGLES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    glNormal3f(0.0,0.0,-1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(d1*xcirc[  i]/2.0,d1*ycirc[  i]/2.0,0.0);
      glVertex3f(                    0.0,                    0.0,0.0);
      glVertex3f(d1*xcirc[i+1]/2.0,d1*ycirc[i+1]/2.0,0.0);
    }
    glNormal3f(0.0,0.0,1.0);
    for(i=0;i<ncirc;i++){
      glVertex3f(d2*xcirc[  i]/2.0,d2*ycirc[  i]/2.0, height);
      glVertex3f(d2*xcirc[i+1]/2.0,d2*ycirc[i+1]/2.0, height);
      glVertex3f(                    0.0,                    0.0, height);
    }
    glEnd();
  }
  else{
    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glVertex3f(d1*xcirc[  i]/2.0,d1*ycirc[  i]/2.0,0.0); // 1
      glVertex3f(d1*xcirc[i+1]/2.0,d1*ycirc[i+1]/2.0,0.0); // 2

      glVertex3f(d1*xcirc[i+1]/2.0,d1*ycirc[i+1]/2.0,0.0); // 2
      glVertex3f(d2*xcirc[i+1]/2.0,d2*ycirc[i+1]/2.0, height); // 3

      glVertex3f(d2*xcirc[i+1]/2.0,d2*ycirc[i+1]/2.0, height); // 3
      glVertex3f(d2*xcirc[  i]/2.0,d2*ycirc[  i]/2.0, height); // 4

      glVertex3f(d2*xcirc[  i]/2.0,d2*ycirc[  i]/2.0, height); // 4
      glVertex3f(d1*xcirc[  i]/2.0,d1*ycirc[  i]/2.0,0.0); // 1
    }
    glEnd();

    glBegin(GL_LINES);
    if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

    for(i=0;i<ncirc;i++){
      glVertex3f(d1*xcirc[  i]/2.0,d1*ycirc[  i]/2.0,0.0);
      glVertex3f(                    0.0,                    0.0,0.0);

      glVertex3f(                    0.0,                    0.0,0.0);
      glVertex3f(d1*xcirc[i+1]/2.0,d1*ycirc[i+1]/2.0,0.0);

      glVertex3f(d1*xcirc[i+1]/2.0,d1*ycirc[i+1]/2.0,0.0);
      glVertex3f(d1*xcirc[  i]/2.0,d1*ycirc[  i]/2.0,0.0);

    }
    for(i=0;i<ncirc;i++){
      glVertex3f(d2*xcirc[  i]/2.0,d2*ycirc[  i]/2.0, height);
      glVertex3f(d2*xcirc[i+1]/2.0,d2*ycirc[i+1]/2.0, height);

      glVertex3f(d2*xcirc[i+1]/2.0,d2*ycirc[i+1]/2.0, height);
      glVertex3f(                    0.0,                    0.0, height);

      glVertex3f(                    0.0,                    0.0, height);
      glVertex3f(d2*xcirc[  i]/2.0,d2*ycirc[  i]/2.0, height);
    }
    glEnd();
  }
}


/* ----------------------- InitSphere ----------------------------- */

void InitSphere(int nlat, int nlong){
  float dlat, dlong;
  int i;

  FREEMEMORY(cos_lat);
  FREEMEMORY(sin_lat);
  FREEMEMORY(cos_long);
  FREEMEMORY(sin_long);
  NewMemory( (void **)&cos_lat,(nlat+1)*sizeof(float));
  NewMemory( (void **)&sin_lat,(nlat+1)*sizeof(float));
  NewMemory( (void **)&cos_long,(nlong+1)*sizeof(float));
  NewMemory( (void **)&sin_long,(nlong+1)*sizeof(float));

  dlat=PI/(float)nlat;
  for(i=0;i<=nlat;i++){
    float angle;

    angle = -PI/2.0 + i*dlat;
    cos_lat[i] = cos(angle);
    sin_lat[i] = sin(angle);
  }

  dlong=2.0*PI/(float)nlong;
  for(i=0;i<nlong;i++){
    float angle;

    angle = i*dlong;
    cos_long[i] = cos(angle);
    sin_long[i] = sin(angle);
  }
  cos_long[nlong]=cos_long[0];
  sin_long[nlong]=sin_long[0];
}

/* ----------------------- InitSphere2 ----------------------------- */

float *InitSphere2(int nlat, int nlong){
  int i;
  int ntotal;
  float *sphere;
  float *c_lat, *s_lat, *c_long, *s_long;

  ntotal = 2*(nlat + 1) + 2*(nlong + 1);
  NewMemory( (void **)&sphere, ntotal*sizeof(float));
  c_lat  = sphere;
  s_lat  = c_lat  + nlat + 1;
  c_long = s_lat  + nlat + 1;
  s_long = c_long + nlong + 1;

  c_lat[0] =  0.0;
  s_lat[0] = -1.0;
  for(i=1;i<nlat;i++){
    float angle;

    angle = -PI/2.0 + (float)i*PI/(float)nlat;
    c_lat[i] = cos(angle);
    s_lat[i] = sin(angle);
  }
  c_lat[nlat] = 0.0;
  s_lat[nlat] = 1.0;

  c_long[0] = 1.0;
  s_long[0] = 0.0;
  for(i=1;i<nlong;i++){
    float angle;

    angle = (float)i*2.0*PI/(float)nlong;
    c_long[i] = cos(angle);
    s_long[i] = sin(angle);
  }
  c_long[nlong] = 1.0;
  s_long[nlong] = 0.0;

  return sphere;
}

/* ----------------------- GetGlobalDeviceBounds ----------------------------- */

void GetGlobalDeviceBounds(int type){
  int i;
  float valmin, valmax;

  valmin = 1.0;
  valmax = 0.0;
  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devicei;
    devicei = global_scase.devicecoll.deviceinfo+i;
    if(devicei->type2==type){
      int j;
      float *vals;

      vals = devicei->vals;
      for(j = 0; j<devicei->nvals; j++){
        if(valmin>valmax){
          valmin = vals[j];
          valmax = valmin;
        }
        else{
          valmin = MIN(valmin, vals[j]);
          valmax = MAX(valmax, vals[j]);
        }
      }
    }
  }
  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devicei;
    devicei = global_scase.devicecoll.deviceinfo+i;
    if(devicei->type2==type){
      devicei->global_valmin = valmin;
      devicei->global_valmax = valmax;
    }
  }
}

/* ----------------------- DrawDevices ----------------------------- */

void DrawDevices(int mode){
  int drawobjects_as_vectors;
  int ii;

  if(select_device == 0 || show_mode != SELECTOBJECT){
    int i;

    if(object_box==1){
      if(object_outlines==0){
        ENABLE_LIGHTING;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, global_scase.color_defs.block_ambient2);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
        glEnable(GL_COLOR_MATERIAL);
      }

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
        devicedata *devicei;
        float *xyz1, *xyz2, dxyz[3];

        devicei = global_scase.devicecoll.deviceinfo+i;
        if(devicei->object->visible == 0 || devicei->show == 0)continue;
        xyz1 = devicei->xyz1;
        xyz2 = devicei->xyz2;
        dxyz[0] = xyz2[0]-xyz1[0];
        dxyz[1] = xyz2[1]-xyz1[1];
        dxyz[2] = xyz2[2]-xyz1[2];
        DrawBox2(xyz1, dxyz, foregroundcolor, OUTLINE_ONLY);
      }
      glPopMatrix();
      if(object_outlines==0){
        DISABLE_LIGHTING;
      }
    }

    for(i = 0;i < global_scase.devicecoll.ndeviceinfo;i++){
      devicedata *devicei;

      devicei = global_scase.devicecoll.deviceinfo + i;
      if(devicei->object->visible == 0 || devicei->show == 0)continue;
      if(devicei->in_zone_csv == 1)continue;
      if(devicei->plane_surface != NULL){
        int j;

        for(j = 0;j < global_scase.meshescoll.nmeshes;j++){
          DrawStaticIso(devicei->plane_surface[j], -1, 0, 2, 0, devicei->line_width);
          DrawStaticIso(devicei->plane_surface[j], 2, 0, 2, 0, devicei->line_width);
        }
        continue;
      }
    }
  }
  drawobjects_as_vectors = 0;
  if(showtime == 1 && itimes >= 0 && itimes < nglobal_times&&showvdevice_val == 1 && global_scase.devicecoll.nvdeviceinfo>0){
    unsigned char arrow_color[4];
    float arrow_color_float[4];
    int j;

    ENABLE_LIGHTING;
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, global_scase.color_defs.block_ambient2);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glEnable(GL_COLOR_MATERIAL);

    glPushMatrix();
    glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
    glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
    glPointSize(vectorpointsize);
    arrow_color[0] = 255 * foregroundcolor[0];
    arrow_color[1] = 255 * foregroundcolor[1];
    arrow_color[2] = 255 * foregroundcolor[2];
    arrow_color[3] = 255;
    glColor3ubv(arrow_color);
    for(j = 0; j < ntreedeviceinfo; j++){
      treedevicedata *treei;
      int i, first;

      treei = treedeviceinfo + j;
      if(vectortype == VECTOR_PROFILE&&treei->n < mintreesize)continue;
      first = 1;
      for(i = treei->first; i <= treei->last; i++){
        vdevicedata *vdevi;
        devicedata *devicei;
        float vel[3], angle, dvel, dangle;
        float *xyz;
        int velocity_type;
        vdevicesortdata *vdevsorti;

        vdevsorti = global_scase.devicecoll.vdevices_sorted + i;
        if(vectortype == VECTOR_PROFILE){
          if(vdevsorti->dir == XDIR && vis_xtree == 0)continue;
          if(vdevsorti->dir == YDIR && vis_ytree == 0)continue;
          if(vdevsorti->dir == ZDIR && vis_ztree == 0)continue;
        }
        else{
          if(vdevsorti->dir != ZDIR)continue;
        }

        vdevi = vdevsorti->vdeviceinfo;
        devicei = vdevi->colordev;
        if(devicei == NULL)continue;
        if(vdevi->unique == 0)continue;
        xyz = vdevi->valdev->xyz;
        GetVDeviceVel(global_times[itimes], vdevi, vel, &angle, &dvel, &dangle, &velocity_type);
        if(colordevice_val == 1){
          int type, vistype = 0;

          type = devicei->type2;
          if(type >= 0 && type < ndevicetypes)vistype = devicetypes[type]->type2vis;
          if(vistype == 1){
            unsigned char color[4], *colorptr;

            colorptr = GetDeviceColor(devicei, color, device_valmin, device_valmax);
            if(colorptr != NULL){
              arrow_color[0] = colorptr[0];
              arrow_color[1] = colorptr[1];
              arrow_color[2] = colorptr[2];
              arrow_color[3] = 255;
            }
            else{
              arrow_color[0] = 255 * foregroundcolor[0];
              arrow_color[1] = 255 * foregroundcolor[1];
              arrow_color[2] = 255 * foregroundcolor[2];
              arrow_color[3] = 255;
            }
            glColor3ubv(arrow_color);
          }
        }
        arrow_color_float[0] = (float)arrow_color[0] / 255.0;
        arrow_color_float[1] = (float)arrow_color[1] / 255.0;
        arrow_color_float[2] = (float)arrow_color[2] / 255.0;
        arrow_color_float[3] = (float)arrow_color[3] / 255.0;
        if(velocity_type == VEL_CARTESIAN){
          float xyz1_old[3], xyz2_old[3];
          float xyz1_new[3], xyz2_new[3];
          unsigned char arrow_color_old[4], *arrow_color_new;
          float dxyz[3], vec0[3] = {0.0, 0.0, 0.0}, zvec[3] = {0.0, 0.0, 1.0};
          float axis[3], speed;
          int state = 0;
          int jj;

          for(jj = 0; jj < 3; jj++){
            dxyz[jj] = 0.5*SCALE2FDS(vel[jj]) / max_dev_vel;
          }
          speed = sqrt(dxyz[0] * dxyz[0] + dxyz[1] * dxyz[1] + dxyz[2] * dxyz[2]);

          switch(vectortype){
          case VECTOR_PROFILE:
            xyz2_new[0] = xyz[0] + dxyz[0];
            xyz2_new[1] = xyz[1] + dxyz[1];
            xyz2_new[2] = xyz[2] + dxyz[2];

            xyz1_new[0] = xyz[0];
            xyz1_new[1] = xyz[1];
            xyz1_new[2] = xyz[2];

            arrow_color_new = arrow_color;
            if(first == 1){
              first = 0;
              memcpy(xyz1_old, xyz1_new, 3 * sizeof(float));
              memcpy(xyz2_old, xyz2_new, 3 * sizeof(float));
              memcpy(arrow_color_old, arrow_color_new, 4 * sizeof(unsigned char));
              continue;
            }

            //  draw triangles for following rectangle

            //   xyz1_new---------xyz2_new
            //      |       /       |
            //   xyz1_old---------xyz2_old
            glBegin(GL_TRIANGLES);
            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz1_old);
            glVertex3fv(xyz2_old);
            glColor3ubv(arrow_color_new);
            glVertex3fv(xyz2_new);

            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz1_old);
            glColor3ubv(arrow_color_new);
            glVertex3fv(xyz2_new);
            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz2_old);

            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz1_old);
            glColor3ubv(arrow_color_new);
            glVertex3fv(xyz2_new);
            glVertex3fv(xyz1_new);

            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz1_old);
            glColor3ubv(arrow_color_new);
            glVertex3fv(xyz1_new);
            glVertex3fv(xyz2_new);

            glEnd();

            memcpy(xyz1_old, xyz1_new, 3 * sizeof(float));
            memcpy(xyz2_old, xyz2_new, 3 * sizeof(float));
            memcpy(arrow_color_old, arrow_color_new, 4 * sizeof(unsigned char));
            break;
          case VECTOR_LINE:
            RotateU2V(zvec, dxyz, axis, &angle);
            glPushMatrix();
            glTranslatef(xyz[0], xyz[1], xyz[2]);
            glRotatef(RAD2DEG*angle, axis[0], axis[1], axis[2]);
            glScalef(1.0, 1.0, vector_baselength*speed);
            glColor3ubv(arrow_color);
            glBegin(GL_LINES);
            glVertex3fv(vec0);
            glVertex3fv(zvec);
            glEnd();
            glBegin(GL_POINTS);
            glVertex3fv(zvec);
            glEnd();
            glPopMatrix();
            break;
          case VECTOR_ARROW:
            RotateU2V(zvec, dxyz, axis, &angle);
            glPushMatrix();
            glTranslatef(xyz[0], xyz[1], xyz[2]);
            glRotatef(RAD2DEG*angle, axis[0], axis[1], axis[2]);
            glPushMatrix();
            glScalef(1.0, 1.0, speed*vector_baselength);
            DrawDisk(vector_basediameter, 1.0, arrow_color);
            glPopMatrix();
            glTranslatef(0.0, 0.0, speed*vector_baselength);
            DrawCone(vector_headdiameter, vector_headlength, arrow_color);
            glPopMatrix();
            break;
          case VECTOR_OBJECT:
            RotateU2V(zvec, dxyz, axis, &angle);
            glPushMatrix();
            glTranslatef(xyz[0], xyz[1], xyz[2]);
            glRotatef(RAD2DEG*angle, axis[0], axis[1], axis[2]);
            drawobjects_as_vectors = 1;
            glScalef(sensorrelsize*vector_baselength, sensorrelsize*vector_baselength, sensorrelsize*vector_baselength);
            DrawSmvObject(devicei->object, state, devicei->prop, 0, arrow_color_float, 1);
            glPopMatrix();
            break;
          default:
            assert(FFALSE);
            break;
          }
        }
        if(velocity_type == VEL_POLAR){
          float vv;
          float xyz1_old[3], xyz2_old[3];
          float xyz1_new[3], xyz2_new[3];
          unsigned char arrow_color_old[4], *arrow_color_new;
          float anglemin, anglemax, rmin, rmax;


          switch(vectortype){
          case VECTOR_PROFILE:
            //            cos(-alpha)  -sin(-alpha)  0
            //  rot(z) =  sin(-alpha)   cos(-alpha)  0
            //                0            0       1

            //            1   0        0             1 0  0
            //  rot(x) =  0 cos(90) -sin(90)    =    1 0 -1
            //            0 sin(90)  cos(90)         0 1  0

            //                   cos(alpha)  0  -sin(alpha)
            // rot(z)*rot(x) =  -sin(alpha)  0  -cos(alpha)
            //                        0      1        0

            // rot(z)*rot(x)*(0,0,vv) = (-sin(alpha)*vv,-cos(alpha)*vv,0)

            vv = SCALE2FDS(vel[0]) / max_dev_vel;
            xyz2_new[0] = xyz[0] - sin(angle*DEG2RAD)*vv;
            xyz2_new[1] = xyz[1] - cos(angle*DEG2RAD)*vv;
            xyz2_new[2] = xyz[2];

            xyz1_new[0] = xyz[0];
            xyz1_new[1] = xyz[1];
            xyz1_new[2] = xyz[2];

            arrow_color_new = arrow_color;
            if(i == treei->first){
              xyz1_old[0] = xyz1_new[0];
              xyz1_old[1] = xyz1_new[1];
              xyz1_old[2] = xyz1_new[2];

              xyz2_old[0] = xyz2_new[0];
              xyz2_old[1] = xyz2_new[1];
              xyz2_old[2] = xyz2_new[2];

              arrow_color_old[0] = arrow_color_new[0];
              arrow_color_old[1] = arrow_color_new[1];
              arrow_color_old[2] = arrow_color_new[2];
              arrow_color_old[3] = arrow_color_new[3];
              continue;
            }

            //  draw triangles for following rectangle

            //   xyz1_new---------xyz2_new
            //      |       /       |
            //   xyz1_old---------xyz2_old
            glBegin(GL_TRIANGLES);
            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz1_old);
            glVertex3fv(xyz2_old);
            glColor3ubv(arrow_color_new);
            glVertex3fv(xyz2_new);

            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz1_old);
            glColor3ubv(arrow_color_new);
            glVertex3fv(xyz2_new);
            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz2_old);

            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz1_old);
            glColor3ubv(arrow_color_new);
            glVertex3fv(xyz2_new);
            glVertex3fv(xyz1_new);

            glColor3ubv(arrow_color_old);
            glVertex3fv(xyz1_old);
            glColor3ubv(arrow_color_new);
            glVertex3fv(xyz1_new);
            glVertex3fv(xyz2_new);

            glEnd();

            xyz1_old[0] = xyz1_new[0];
            xyz1_old[1] = xyz1_new[1];
            xyz1_old[2] = xyz1_new[2];

            xyz2_old[0] = xyz2_new[0];
            xyz2_old[1] = xyz2_new[1];
            xyz2_old[2] = xyz2_new[2];

            arrow_color_old[0] = arrow_color[0];
            arrow_color_old[1] = arrow_color[1];
            arrow_color_old[2] = arrow_color[2];
            arrow_color_old[3] = arrow_color[3];
            break;
          case VECTOR_LINE:
            vv = SCALE2FDS(vel[0]) / max_dev_vel;
            glPushMatrix();
            glTranslatef(xyz[0], xyz[1], xyz[2]);
            glRotatef(-angle, 0.0, 0.0, 1.0);
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glColor3ubv(arrow_color);
            glBegin(GL_LINES);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(0.0, 0.0, vv);
            glEnd();
            glPopMatrix();
            break;
          case VECTOR_ARROW:
            vv = SCALE2FDS(vel[0]) / max_dev_vel;
            glPushMatrix();
            glTranslatef(xyz[0], xyz[1], xyz[2]);
            glRotatef(-angle, 0.0, 0.0, 1.0);
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glColor3ubv(arrow_color);

            glPushMatrix();
            glScalef(1.0, 1.0, vv*vector_baselength);
            DrawDisk(vector_basediameter*xyzmaxdiff / 10.0, 1.0, arrow_color);
            glPopMatrix();
            glTranslatef(0.0, 0.0, vv*vector_baselength);
            DrawCone(vector_headdiameter*xyzmaxdiff / 10.0, vector_headlength*xyzmaxdiff / 10.0, arrow_color);
            glPopMatrix();
            break;
          case VECTOR_OBJECT:
            vv = SCALE2FDS(vel[0]) / max_dev_vel;
            glPushMatrix();
            glTranslatef(xyz[0], xyz[1], xyz[2]);
            glRotatef(-angle, 0.0, 0.0, 1.0);
            glRotatef(90.0, 1.0, 0.0, 0.0);
            glPushMatrix();
            glScalef(1.0, 1.0, vv*vector_baselength);
            DrawDisk(vector_basediameter*xyzmaxdiff / 10.0, 1.0, arrow_color);
            glPopMatrix();

            anglemin = -dangle*DEG2RAD;
            anglemax = -dangle*DEG2RAD;
            rmin = MAX(vv - dvel, 0.0);
            rmax = vv + dvel;
            DrawSphereSeg(anglemin, anglemax, rmin, rmax);
            glPopMatrix();
            break;
          default:
            assert(FFALSE);
            break;
          }
        }
      }
    }
    glPopMatrix();
    DISABLE_LIGHTING;
  }

  glPushMatrix();
  glPushAttrib(GL_POINT_BIT | GL_LINE_BIT);
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
  for(ii = 0;ii < global_scase.devicecoll.ndeviceinfo;ii++){
    devicedata *devicei;
    int tagval;
    propdata *prop;
    int j;
    float dpsi;
    float *xyz;

    devicei = global_scase.devicecoll.deviceinfo + ii;

    if(devicei->object->visible == 0 || (devicei->prop != NULL&&devicei->prop->smv_object->visible == 0))continue;
    if(devicei->plane_surface != NULL)continue;
    if(devicei->show == 0)continue;
    if(global_scase.isZoneFireModel == 1 && STRCMP(devicei->object->label, "target") == 0 && visSensor == 0)continue;
    if(devicei->in_zone_csv == 1&&strcmp(devicei->deviceID,"TARGET")!=0)continue;
    if(global_scase.isZoneFireModel == 1 && STRCMP(devicei->deviceID, "TIME") == 0)continue;
    tagval = ii + 1;
    if(select_device == 1 && show_mode == SELECTOBJECT){

      select_device_color[0] = tagval >> (ngreenbits + nbluebits);
      select_device_color[1] = tagval >> nbluebits;
      select_device_color[2] = tagval&rgbmask[nbluebits - 1];
      select_device_color_ptr = select_device_color;
    }
    else{
      if(devicei->selected==1 && select_device == 1){
        select_device_color_ptr = select_device_color;
        select_device_color[0] = 255;
        select_device_color[1] = 0;
        select_device_color[2] = 0;
      }
      else{
        select_device_color_ptr = NULL;
      }
    }

    xyz = devicei->xyz;
    glPushMatrix();
    glTranslatef(xyz[0], xyz[1], xyz[2]);

    if(show_device_orientation == 1){
      float *xyznorm;

      xyznorm = devicei->xyznorm;
      glPushMatrix();
      glScalef(orientation_scale / 5.0, orientation_scale / 5.0, orientation_scale / 5.0);
      glBegin(GL_LINES);
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(xyznorm[0], xyznorm[1], xyznorm[2]);
      glEnd();
      glPopMatrix();
    }
    if(devicei->is_beam == 1 && showbeam_as_line == 1){
      unsigned char uc_foregroundcolor[3], uc_beamcolor[3], *bc;

      if(use_beamcolor == 1){
        uc_beamcolor[0] = beam_color[0];
        uc_beamcolor[1] = beam_color[1];
        uc_beamcolor[2] = beam_color[2];
        bc = uc_beamcolor;
      }
      else{
        uc_foregroundcolor[0] = 255 * foregroundcolor[0];
        uc_foregroundcolor[1] = 255 * foregroundcolor[1];
        uc_foregroundcolor[2] = 255 * foregroundcolor[2];
        bc = uc_foregroundcolor;
      }
      glPopMatrix();
      glLineWidth(beam_line_width);
      DrawLine(devicei->xyz1, devicei->xyz2, bc);
      glPushMatrix();
      glTranslatef(xyz[0], xyz[1], xyz[2]);
    }
    dpsi = 0.0;
    if((active_smokesensors == 1 && show_smokesensors != SMOKESENSORS_HIDDEN&&STRCMP(devicei->object->label, "smokesensor") == 0) ||
      STRCMP(devicei->object->label, "thermocouple") == 0
      ){
      float *xyznorm;

      xyznorm = devicei->xyznorm;
      xyznorm[0] = fds_eyepos[0] - devicei->xyz[0];
      xyznorm[1] = fds_eyepos[1] - devicei->xyz[1];
      xyznorm[2] = fds_eyepos[2] - devicei->xyz[2];

      GetElevAz(xyznorm, &devicei->dtheta, devicei->rotate_axis, &dpsi);
    }
    {
      float *axis;

      axis = devicei->rotate_axis;
      // the statement below causes problems in objects.svo definitions
      glRotatef(devicei->dtheta, axis[0], axis[1], axis[2]);
      glRotatef(-dpsi, 0.0, 0.0, 1.0);
    }
    if(sensorrelsize != 1.0){
      glScalef(sensorrelsize, sensorrelsize, sensorrelsize);
    }
    if(mode == SELECTOBJECT){
      glScalef(4.0, 4.0, 4.0);
    }
    prop = devicei->prop;
    if(prop != NULL){
      prop->rotate_axis = devicei->rotate_axis;
      prop->rotate_angle = devicei->dtheta;
    }
    if(devicei->nparams > 0 && prop != NULL){
      prop->nvars_indep = devicei->nparams;
      if(prop->fvals == NULL){
        prop->nvars_indep = devicei->nparams;
        NewMemory((void **)&prop->fvals, prop->nvars_indep * sizeof(float));
      }
      if(prop->vars_indep_index == NULL){
        prop->nvars_indep = devicei->nparams;
        NewMemory((void **)&prop->vars_indep_index, prop->nvars_indep * sizeof(int));
      }
      for(j = 0;j < devicei->nparams;j++){
        prop->fvals[j] = devicei->params[j];
        prop->vars_indep_index[j] = j;
      }
    }
    if(showtime == 1 && itimes >= 0 && itimes < nglobal_times&&showdevice_val == 1 && ndevicetypes>0){
      int type, vistype = 0;

      type = devicei->type2;
      if(type >= 0 && type < ndevicetypes)vistype = devicetypes[type]->type2vis;
      if(vistype == 1){
        OutputDeviceVal(devicei);
      }
    }
    if(drawobjects_as_vectors == 0){
      if(select_device==0 && showtime == 1 && itimes >= 0 && itimes < nglobal_times){
        int state;

        if(devicei->showstatelist == NULL){
          state = devicei->state0;
        }
        else{
          state = devicei->showstatelist[itimes];
        }
        if(colordevice_val == 1){
          int type, vistype = 0;

          type = devicei->type2;
          if(type >= 0 && type < ndevicetypes)vistype = devicetypes[type]->type2vis;
          if(vistype == 1)DrawSmvObject(devicei->object, state, prop, 0, NULL, 0);
        }
        else{
          int target_index;

          target_index = devicei->target_index;
          if(target_index>=0&&izonetargets!=NULL&&have_target_data==1&&vis_target_data==1){
            unsigned char color_index, target_color[4];

            color_index = izonetargets[itimes*nzone_targets+target_index];
            target_color[0] = (float)rgb_full[color_index][0]*255.0;
            target_color[1] = (float)rgb_full[color_index][1]*255.0;
            target_color[2] = (float)rgb_full[color_index][2]*255.0;
            target_color[3] = (float)rgb_full[color_index][3]*255.0;
            select_device_color_ptr = target_color;
          }
          else{
            select_device_color_ptr = NULL;
          }
          DrawSmvObject(devicei->object, state, prop, 0, NULL, 0);
        }
      }
      else{
        DrawSmvObject(devicei->object, devicei->state0, prop, 0, NULL, 0);
      }
    }
    if(devicei->nparams > 0 && prop != NULL){
      prop->nvars_indep = 0;
    }
    glPopMatrix();
  }

  glPopAttrib();
  glPopMatrix();
  DrawTargetNorm();
}

/* ----------------------- DrawSmvObject ----------------------------- */

void DrawSmvObject(sv_object *object_dev, int iframe_local, propdata *prop, int recurse_level, float *valrgb, int vis_override){
  sv_object_frame *framei;
  tokendata *toknext;
  unsigned char *rgbptr_local;
  unsigned char rgbcolor[4];
  int ii;
  sv_object *object;
  int use_material;

  if(prop != NULL){
    object = prop->smv_object;
  }
  else{
    object = object_dev;
  }
  if(object->visible == 0 && vis_override == 0)return;
  if(object == global_scase.objectscoll.std_object_defs.missing_device&&show_missing_objects == 0)return;
  if(iframe_local > object->nframes - 1 || iframe_local < 0)iframe_local = 0;
  framei = object->obj_frames[iframe_local];

  assert(framei->error == 0 || framei->error == 1);

  if(framei->error == 1){
    framei = global_scase.objectscoll.std_object_defs.error_device->obj_frames[0];
    prop = NULL;
  }

  rgbcolor[0] = 255;
  rgbcolor[1] = 0;
  rgbcolor[2] = 0;
  rgbcolor[3] = 255;
  glPushMatrix();

  // copy in default values ( :var=value in objects.svo file )

  for(ii = 0;ii < framei->ntokens;ii++){
    tokendata *toki;

    toki = framei->tokens + ii;
    if(toki->is_label == 1){
      toki->var = toki->default_val;
    }
    if(toki->is_texturefile == 1){
      strcpy(toki->string, toki->default_string);
    }
  }

  // copy values

  if(prop != NULL){
    int i;

    // copy static data from PROP line

    for(i = 0;i < prop->nvars_indep;i++){
      tokendata *toki;
      int index;

      index = prop->vars_indep_index[i];
      if(index<0 || index>framei->ntokens - 1)continue;
      toki = framei->tokens + index;
      toki->var = prop->fvals[i];
      if(prop->svals != NULL&&prop->svals[i] != NULL&&strlen(prop->svals[i]) > 0){
        strcpy(toki->string, prop->svals[i]);
      }
    }

    // copy time dependent data using variables from the class_of_... lines (only if non-uniform coloring is set)

    if(partshortlabel==NULL||strcmp(partshortlabel, "Uniform")!=0){
      for(i = 0; i<prop->nvars_dep; i++){
        tokendata *toki;
        int index;

        index = prop->vars_dep_index[i];
        if(index<0||index>framei->ntokens-1)continue;
        toki = framei->tokens+index;
        toki->var = prop->fvars_dep[i];
      }
    }

  }

  use_material = 0;
  if(select_device_color_ptr == NULL&&recurse_level == 0){
    ENABLE_LIGHTING;

    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, global_scase.color_defs.block_ambient2);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);

    glEnable(GL_COLOR_MATERIAL);
    use_material = 1;
  }
  toknext = NULL;
  for(ii = 0;;ii++){
    tokendata *toki;
#define NARGVAL 10
    float arg[NARGVAL], *argptr;
    int j;

    for(j=0;j<NARGVAL;j++){
      arg[j] = 0.0;
    }
    if(ii == 0){
      toki = framei->command_list[0];
    }
    else{
      toki = toknext;
    }
    if(toki == NULL)break;
    toknext = toki->next;

    if(select_device_color_ptr == NULL){
      rgbptr_local = rgbcolor;
    }
    else{
      rgbptr_local = select_device_color_ptr;
    }
    for(j = 0;j < toki->nvars;j++){
      tokendata *tokj;

      tokj = toki - toki->nvars + j;
      arg[j] = *(tokj->varptr);
    }
    if(toki->nvars > 0){
      argptr = (toki - 1)->varptr;
    }

    switch(toki->command){
    case SV_ADD:
    {
      float val1, val2, val_result;

      val1 = arg[0];
      val2 = arg[1];


      val_result = val1 + val2;

      *argptr = val_result;
    }
    break;
    case SV_ORIENX:
      if(arg[2] < 10.0){
        float u[3] = {1.0,0.0,0.0}, axis[3], angle;

        RotateU2V(u, arg, axis, &angle);
        glRotatef(RAD2DEG*angle, axis[0], axis[1], axis[2]);

      }
      break;
    case SV_ORIENY:
      if(arg[2] < 10.0){
        float u[3] = {0.0,1.0,0.0}, axis[3], angle;

        RotateU2V(u, arg, axis, &angle);
        glRotatef(RAD2DEG*angle, axis[0], axis[1], axis[2]);

      }
      break;
    case SV_ORIENZ:
      if(arg[2] < 10.0){
        float u[3] = {0.0,0.0,1.0}, axis[3], angle;

        RotateU2V(u, arg, axis, &angle);
        glRotatef(RAD2DEG*angle, axis[0], axis[1], axis[2]);

      }
      break;
    case SV_RANDXY:
      if(ABS(arg[0] - 1.0) < 0.01){
        float random_angle = 0.0;

        random_angle = RandAB(prop->tag_number, 0.0, 360.0);
        glRotatef(random_angle, 0.0, 0.0, 1.0);
      }
      break;
    case SV_RANDXZ:
      if(ABS(arg[0] - 1.0) < 0.01){
        float random_angle = 0.0;

        random_angle = RandAB(prop->tag_number, 0.0, 360.0);
        glRotatef(random_angle, 0.0, 1.0, 0.0);
      }
      break;
    case SV_RANDYZ:
      if(ABS(arg[0] - 1.0) < 0.01){
        float random_angle = 0.0;

        random_angle = RandAB(prop->tag_number, 0.0, 360.0);
        glRotatef(random_angle, 1.0, 0.0, 0.0);
      }
      break;
    case SV_RANDXYZ:
      if(ABS(arg[0] - 1.0) < 0.01){
        float zz, tt, rr, xx, yy, olddir[3], newdir[3], axis[3], angle;

        //    Choose z uniformly distributed in [-1,1].
        //    Choose t uniformly distributed on [0, 2*pi).
        //    Let r = sqrt(1-z^2).
        //    Let x = r * cos(t).
        //    Let y = r * sin(t).

        zz = RandAB(2 * prop->tag_number - 1, -1.0, 1.0);
        tt = RandAB(2 * prop->tag_number, 0.0, 2.0*PI);
        rr = sqrt(ABS(1.0 - zz*zz));
        xx = rr*cos(tt);
        yy = rr*sin(tt);
        olddir[0] = 1.0;
        olddir[1] = 0.0;
        olddir[2] = 0.0;
        newdir[0] = xx;
        newdir[1] = yy;
        newdir[2] = zz;
        RotateU2V(olddir, newdir, axis, &angle);
        glRotatef(RAD2DEG*angle, axis[0], axis[1], axis[2]);
      }
      break;
    case SV_INCLUDE:
    case SV_INCLUDEF:
    {
      sv_object *included_object;
      int iframe_local2;
      char *object_name;

      if(toki->included_object == NULL){
        if(toki->command == SV_INCLUDEF){
          iframe_local2 = arg[0];
        }
        else{
          iframe_local2 = 0;
        }
        object_name = (toki - 1)->string;
        included_object = GetSmvObjectType(&global_scase.objectscoll, object_name, global_scase.objectscoll.std_object_defs.missing_device);
        toki->included_frame = iframe_local2;
        toki->included_object = included_object;
      }
      else{
        iframe_local2 = toki->included_frame;
        included_object = toki->included_object;
      }
      DrawSmvObject(included_object, iframe_local2, NULL, recurse_level + 1, NULL, 0);
    }
    break;
    case SV_ABS:
      if(arg[0] < 0.0){
        *argptr = -arg[0];
      }
      else{
        *argptr = arg[0];
      }
      break;
    case SV_SUB:
    {
      float val1, val2, val_result;

      val1 = arg[0];
      val2 = arg[1];


      val_result = val1 - val2;

      *argptr = val_result;
    }
    break;
    case SV_MULT:
    {
      float val1, val2, val_result;

      val1 = arg[0];
      val2 = arg[1];


      val_result = val1*val2;

      *argptr = val_result;
    }
    break;
    case SV_DIV:
    {
      float val1, val2, val_result;

      val1 = arg[0];
      val2 = arg[1];


      if(val2 == 0.0){
        val_result = 0.0;
      }
      else{
        val_result = val1 / val2;
      }

      *argptr = val_result;
    }
    break;
    case SV_GETT:
    {
      float val_result;
      float time_val = 0.0;

      if(nglobal_times > 0){
        time_val = global_times[itimes];
      }

      val_result = time_val;

      *argptr = val_result;
    }
    break;
    case SV_MULTIADDT:
    {
      float val1, val2, val_result;
      float time_val = 0.0;

      val1 = arg[0];
      val2 = arg[1];

      if(nglobal_times > 0){
        time_val = global_times[itimes];
      }

      val_result = val1*time_val + val2;

      *argptr = val_result;
    }
    break;
    case SV_CLIP:
    {
      float val, valmin, valmax;

      val = arg[0];
      valmin = arg[1];
      valmax = arg[2];

      *argptr = CLAMP(val, valmin, valmax);
    }
    break;
    case SV_CLIPX:
    {
      clipdata objclip, *ci;

      ci = &objclip;
      ci->clip_xmin = arg[0];
      ci->xmin = arg[1];
      ci->clip_xmax = arg[2];
      ci->xmax = arg[3];
      ci->clip_ymin = -1;
      ci->clip_ymax = -1;
      ci->clip_zmin = -1;
      ci->clip_zmax = -1;
      SetClipPlanes(ci, CLIP_ON);
    }
    break;
    case SV_CLIPY:
    {
      clipdata objclip, *ci;

      ci = &objclip;
      ci->clip_ymin = arg[0];
      ci->ymin = arg[1];
      ci->clip_ymax = arg[2];
      ci->ymax = arg[3];
      ci->clip_xmin = -1;
      ci->clip_xmax = -1;
      ci->clip_zmin = -1;
      ci->clip_zmax = -1;
      SetClipPlanes(ci, CLIP_ON);
    }
    break;
    case SV_CLIPZ:
    {
      clipdata objclip, *ci;

      ci = &objclip;
      ci->clip_zmin = arg[0];
      ci->zmin = arg[1];
      ci->clip_zmax = arg[2];
      ci->zmax = arg[3];
      ci->clip_xmin = -1;
      ci->clip_xmin = -1;
      ci->clip_ymin = -1;
      ci->clip_ymax = -1;
      SetClipPlanes(ci, CLIP_ON);
    }
    break;
    case SV_CLIPOFF:
      SetClipPlanes(NULL, CLIP_OFF);
      break;
    case SV_MIRRORCLIP:
    {
      float val, valmin, valmax;
      float val_rel, valmax_rel;

      val = arg[0];
      valmin = arg[1];
      valmax = arg[2];

      valmax_rel = valmax - valmin;
      val_rel = fmod(val - valmin, 2.0*valmax_rel);
      if(val_rel < 0.0)val_rel += 2.0*valmax_rel;
      if(val_rel > valmax_rel)val_rel = 2.0*valmax_rel - val_rel;

      *argptr = val_rel + valmin;
    }
    break;
    case SV_PERIODICCLIP:
    {
      float val, valmin, valmax;
      float val_rel, valmax_rel;

      val = arg[0];
      valmin = arg[1];
      valmax = arg[2];

      val_rel = val - valmin;
      valmax_rel = valmax - valmin;

      val_rel = fmod(val_rel, valmax_rel);

      *argptr = val_rel + valmin;
    }
    break;
    case SV_GTRANSLATE:
      if(prop != NULL){
        float *axis;

        axis = prop->rotate_axis;
        glRotatef(-prop->rotate_angle, axis[0], axis[1], axis[2]);
        glTranslatef(arg[0], arg[1], arg[2]);
        glRotatef(prop->rotate_angle, axis[0], axis[1], axis[2]);
      }
      else{
        glTranslatef(arg[0], arg[1], arg[2]);
      }
      break;
    case SV_TRANSLATE:
      glTranslatef(arg[0], arg[1], arg[2]);
      break;
    case SV_OFFSETX:
      glTranslatef(arg[0], 0.0, 0.0);
      break;
    case SV_OFFSETY:
      glTranslatef(0.0, arg[0], 0.0);
      break;
    case SV_OFFSETZ:
      glTranslatef(0.0, 0.0, arg[0]);
      break;
    case SV_IF:
      if(ABS(arg[0]) <= 0.001){
        toknext = toki->elsenext;
      }
      break;
    case SV_ELSE:
    case SV_ENDIF:
      break;
    case SV_AND:
      if(ABS(arg[0]) >= 0.001&&ABS(arg[1]) >= 0.001){
        *argptr = 1.0;
      }
      else{
        *argptr = 0.0;
      }
      break;
    case SV_OR:
      if(ABS(arg[0]) >= 0.001 || ABS(arg[1]) >= 0.001){
        *argptr = 1.0;
      }
      else{
        *argptr = 0.0;
      }
      break;
    case SV_EQ:
    {
      float *to_ptr, *from_ptr;

      to_ptr = (toki - 2)->varptr;
      from_ptr = (toki - 1)->varptr;
      *to_ptr = *from_ptr;
    }
    break;
    case SV_GT:
      if(arg[0] > arg[1]){
        *argptr = 1.0;
      }
      else{
        *argptr = 0.0;
      }
      break;
    case SV_GE:
      if(arg[0] >= arg[1]){
        *argptr = 1.0;
      }
      else{
        *argptr = 0.0;
      }
      break;
    case SV_LT:
      if(arg[0] < arg[1]){
        *argptr = 1.0;
      }
      else{
        *argptr = 0.0;
      }
      break;
    case SV_LE:
      if(arg[0] <= arg[1]){
        *argptr = 1.0;
      }
      else{
        *argptr = 0.0;
      }
      break;
    case SV_ROTATEXYZ:
      RotateXYZ(arg[0], arg[1], arg[2]);
      break;
    case SV_ROTATEAXIS:
      RotateAxis(arg[0], arg[1], arg[2], arg[3]);
      break;
    case SV_ROTATEEYE:
      RotateEye();
      break;
    case SV_ROTATEX:
      glRotatef(arg[0], 1.0, 0.0, 0.0);
      break;
    case SV_ROTATEY:
      glRotatef(arg[0], 0.0, 1.0, 0.0);
      break;
    case SV_ROTATEZ:
      glRotatef(arg[0], 0.0, 0.0, 1.0);
      break;
    case SV_SCALEXYZ:
      glScalef(arg[0], arg[1], arg[2]);
      break;
    case SV_SCALEAUTO:
      glScalef(SCALE2FDS(arg[0]), SCALE2FDS(arg[0]), SCALE2FDS(arg[0]));
      break;
    case SV_SCALEGRID:
      glScalef(arg[0] * min_gridcell_size, arg[0] * min_gridcell_size, arg[0] * min_gridcell_size);
      break;
    case SV_SCALE:
      glScalef(arg[0], arg[1], arg[2]);
      break;
    case SV_DRAWCUBE:
      DrawCube(arg[0], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWCUBEC:
      DrawCubeC(arg[0], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWBOXXYZ:
      DrawBoxXyz(arg, rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWPRISMXYZ:
      DrawPrismXyz(arg, rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWSQUARE:
      DrawSquare(arg[0], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWVENT:
      DrawVent(arg[0], arg[1], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWDISK:
      DrawDisk(arg[0], arg[1], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWWHEEL:
      DrawWheel(arg[0], arg[1], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWARCDISK:
      DrawArcDisk(arg[0], arg[1], arg[2], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWCDISK:
      DrawCDisk(arg[0], arg[1], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWHEXDISK:
      DrawHexDisk(arg[0], arg[1], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWPOLYDISK:
    {
      int nsides;

      nsides = arg[0] + 0.5;
      DrawPolyDisk(nsides, arg[1], arg[2], rgbptr_local);
      rgbptr_local = NULL;
    }
    break;
    case SV_DRAWRING:
      DrawRing(arg[0], arg[1], arg[2], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWNOTCHPLATE:
      DrawNotchPlate(arg[0], arg[1], arg[2], arg[3], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWTRUNCCONE:
      DrawTruncCone(arg[0], arg[1], arg[2], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWCONE:
      DrawCone(arg[0], arg[1], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWTSPHERE:
    {
      int texture_index;

      texture_index = arg[0] + 0.5;
      DrawTSphere(texture_index, arg[1], rgbptr_local);
    }
    rgbptr_local = NULL;
    break;
    case SV_DRAWSPHERE:
      DrawSphere(arg[0], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWHSPHERE:
      DrawHSphere(arg[0], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWTRIBLOCK:
      DrawTriBlock(arg[0], arg[1], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWCIRCLE:
      DrawCircle(arg[0], rgbptr_local, &object_circ);
      rgbptr_local = NULL;
      break;
    case SV_DRAWFILLEDCIRCLE:
      DrawFilledCircle(arg[0], rgbptr_local, &object_circ);
      rgbptr_local = NULL;
      break;
    case SV_DRAWARC:
      DrawArc(arg[0], arg[1], rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_DRAWPOINT:
      DrawPoint(rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_GETTEXTUREINDEX:
    {
      char *texturefile;
      int i;
      int textureindex = 0;

      texturefile = (toki - 2)->stringptr;

      for(i = 0;i < global_scase.device_texture_list_coll.ndevice_texture_list;i++){
        if(strcmp(global_scase.device_texture_list_coll.device_texture_list[i], texturefile) == 0){
          textureindex = i;
          break;
        }
      }
      *argptr = textureindex;
    }
    break;
    case SV_SETCOLOR:
    case SV_SETRGB:
    case SV_SETRGBVAL:
      if(toki->command == SV_SETCOLOR){
        int iarg[3];
        char *stringptr;

        stringptr = (toki - 1)->string;

        color2rgb(iarg, stringptr);
        arg[0] = iarg[0];
        arg[1] = iarg[1];
        arg[2] = iarg[2];
      }
      if(valrgb != NULL&&toki->command == SV_SETRGBVAL){
        arg[0] = valrgb[0];
        arg[1] = valrgb[1];
        arg[2] = valrgb[2];
      }
      if(setbw == 1){
        float grey;

        grey = TOBW(arg);
        rgbcolor[0] = grey;
        rgbcolor[1] = grey;
        rgbcolor[2] = grey;
        rgbcolor[3] = 255;
      }
      else{
        rgbcolor[0] = arg[0];
        rgbcolor[1] = arg[1];
        rgbcolor[2] = arg[2];
        rgbcolor[3] = 255;
      }
      break;
    case SV_SETLINEWIDTH:
    {
      glLineWidth(arg[0]);
    }
    break;
    case SV_SETPOINTSIZE:
    {
      glPointSize(arg[0]);
    }
    break;
    case SV_SETBW:
    {
      rgbcolor[0] = 255 * arg[0];
      rgbcolor[1] = 255 * arg[0];
      rgbcolor[2] = 255 * arg[0];
      rgbcolor[3] = 255;
    }
    break;
    case SV_DRAWLINE:
      DrawLine(arg, arg + 3, rgbptr_local);
      rgbptr_local = NULL;
      break;
    case SV_PUSH:
      glPushMatrix();
      break;
    case SV_POP:
      glPopMatrix();
      break;
    case SV_NO_OP:
      break;
    case SV_ERR:
      break;
    default:
      assert(FFALSE);
      break;
    }
  }
  if(use_material == 1 && recurse_level == 0){
    glDisable(GL_COLOR_MATERIAL);
    DISABLE_LIGHTING;
  }

  glPopMatrix();
}

#define BUFFER_SIZE 10000

/* ----------------------- GetTokenPtr ----------------------------- */

tokendata *GetTokenPtr(char *var,sv_object_frame *frame){
  int i;

  for(i=0;i<frame->nsymbols;i++){
    int ii;
    tokendata *toki;
    char *token_var;

    ii = frame->symbols[i];
    toki = frame->tokens+ii;
    token_var = toki->tokenlabel+1;
    if(STRCMP(var,token_var)==0)return toki;
  }
  return NULL;
}
/* ----------------------- GetCSVDeviceFromLabel ----------------------------- */

devicedata *GetCSVDeviceFromLabel(char *label, int index){
  int i;

  if(strlen(label)>=4&&strncmp(label, "null", 4)==0&&index>=0&&index<global_scase.devicecoll.ndeviceinfo){
    return global_scase.devicecoll.deviceinfo+index;
  }
  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devicei;

    devicei = global_scase.devicecoll.deviceinfo+i;
    if(global_scase.nzoneinfo==0){
      if(STRCMP(devicei->labelptr, label)==0)return devicei;
    }
    else{
      if(STRCMP(devicei->csvlabel, label)==0)return devicei;
    }
  }
  return NULL;
}

/* ----------------------- GetDeviceIndexFromLabel ----------------------------- */

int GetDeviceIndexFromLabel(char *label){
  int i;

  for(i = 0;i < global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;

    devicei = global_scase.devicecoll.deviceinfo + i;
    if(STRCMP(devicei->deviceID, label) == 0)return i;
  }
  return -1;
}

/* ----------------------- GetDeviceFromLabel ----------------------------- */

devicedata *GetDeviceFromLabel(char *label,int index){
  int i;

  if(strlen(label)>=4&&strncmp(label,"null",4)==0&&index>=0&&index<global_scase.devicecoll.ndeviceinfo){
    return global_scase.devicecoll.deviceinfo + index;
  }
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devicei;

    devicei = global_scase.devicecoll.deviceinfo + i;
    if(STRCMP(devicei->deviceID,label)==0)return devicei;
  }
  return NULL;
}

#define EPSDEV 0.01

/* ------------------ CompareV2Devices ------------------------ */

int CompareV2Devices(const void *arg1, const void *arg2){
  vdevicesortdata *vdevi, *vdevj;
  float *xyzi, *xyzj;
  int diri, dirj;

  vdevi = (vdevicesortdata *)arg1;
  vdevj = (vdevicesortdata *)arg2;
  diri = vdevi->dir;
  dirj = vdevj->dir;
  xyzi = vdevi->vdeviceinfo->valdev->xyz;
  xyzj = vdevj->vdeviceinfo->valdev->xyz;
  if(diri - dirj < 0)return -1;
  if(diri - dirj > 0)return 1;
  switch(diri){
  case XDIR:
    if(xyzi[1] - xyzj[1]<-EPSDEV)return -1;
    if(xyzi[1] - xyzj[1]>EPSDEV)return 1;
    if(xyzi[2] - xyzj[2]<-EPSDEV)return -1;
    if(xyzi[2] - xyzj[2]>+EPSDEV)return 1;
    break;
  case YDIR:
    if(xyzi[0] - xyzj[0]<-EPSDEV)return -1;
    if(xyzi[0] - xyzj[0]>EPSDEV)return 1;
    if(xyzi[2] - xyzj[2]<-EPSDEV)return -1;
    if(xyzi[2] - xyzj[2]>+EPSDEV)return 1;
    break;
  case ZDIR:
    if(xyzi[0] - xyzj[0]<-EPSDEV)return -1;
    if(xyzi[0] - xyzj[0]>EPSDEV)return 1;
    if(xyzi[1] - xyzj[1]<-EPSDEV)return -1;
    if(xyzi[1] - xyzj[1]>+EPSDEV)return 1;
    break;
  default:
    assert(FFALSE);
    break;
  }
  return 0;
}

/* ------------------ CompareV3Devices ------------------------ */

int CompareV3Devices(const void *arg1, const void *arg2){
  vdevicesortdata *vdevi, *vdevj;
  float *xyzi, *xyzj;
  int diri, dirj;

  vdevi = (vdevicesortdata *)arg1;
  vdevj = (vdevicesortdata *)arg2;
  diri = vdevi->dir;
  dirj = vdevj->dir;
  xyzi = vdevi->vdeviceinfo->valdev->xyz;
  xyzj = vdevj->vdeviceinfo->valdev->xyz;
  if(diri - dirj < 0)return -1;
  if(diri - dirj > 0)return 1;
  switch(diri){
  case XDIR:
    if(xyzi[1]-xyzj[1]<-EPSDEV)return -1;
    if(xyzi[1]-xyzj[1]>+EPSDEV)return 1;
    if(xyzi[2]-xyzj[2]<-EPSDEV)return -1;
    if(xyzi[2]-xyzj[2]>+EPSDEV)return 1;
    if(xyzi[0]-xyzj[0]<-EPSDEV)return -1;
    if(xyzi[0]-xyzj[0]>EPSDEV)return 1;
    break;
  case YDIR:
    if(xyzi[0]-xyzj[0]<-EPSDEV)return -1;
    if(xyzi[0]-xyzj[0]>EPSDEV)return 1;
    if(xyzi[2]-xyzj[2]<-EPSDEV)return -1;
    if(xyzi[2]-xyzj[2]>+EPSDEV)return 1;
    if(xyzi[1]-xyzj[1]<-EPSDEV)return -1;
    if(xyzi[1]-xyzj[1]>+EPSDEV)return 1;
    break;
  case ZDIR:
    if(xyzi[0]-xyzj[0]<-EPSDEV)return -1;
    if(xyzi[0]-xyzj[0]>EPSDEV)return 1;
    if(xyzi[1]-xyzj[1]<-EPSDEV)return -1;
    if(xyzi[1]-xyzj[1]>+EPSDEV)return 1;
    if(xyzi[2]-xyzj[2]<-EPSDEV)return -1;
    if(xyzi[2]-xyzj[2]>+EPSDEV)return 1;
    break;
  default:
    assert(FFALSE);
    break;
  }
  return 0;
}

/* ------------------ CompareV3Devices ------------------------ */

int CompareZ3Devices(const void *arg1, const void *arg2){
  devicedata *devi, *devj;
  float *xyzi, *xyzj;
  char *quanti, *quantj;
  int iquant;

  devi = *(devicedata **)arg1;
  devj = *(devicedata **)arg2;
  xyzi = devi->xyz;
  xyzj = devj->xyz;
  quanti = devi->quantity;
  quantj = devj->quantity;

  iquant = strcmp(quanti, quantj);
  if(iquant<0)return -1;
  if(iquant>0)return 1;
  if(xyzi[0]-xyzj[0]<-EPSDEV)return -1;
  if(xyzi[0]-xyzj[0]>EPSDEV)return 1;
  if(xyzi[1]-xyzj[1]<-EPSDEV)return -1;
  if(xyzi[1]-xyzj[1]>+EPSDEV)return 1;
  if(xyzi[2]-xyzj[2]<-EPSDEV)return -1;
  if(xyzi[2]-xyzj[2]>+EPSDEV)return 1;
  return 0;
}

/* ------------------ CompareV3Devices ------------------------ */

int CompareZ2Devices(const void *arg1, const void *arg2){
  devicedata *devi, *devj;
  float *xyzi, *xyzj;
  char *quanti, *quantj;
  int iquant;

  devi = *(devicedata **)arg1;
  devj = *(devicedata **)arg2;
  xyzi = devi->xyz;
  xyzj = devj->xyz;
  quanti = devi->quantity;
  quantj = devj->quantity;

  iquant = strcmp(quanti, quantj);
  if(iquant<0)return -1;
  if(iquant>0)return 1;
  if(xyzi[0]-xyzj[0]<-EPSDEV)return -1;
  if(xyzi[0]-xyzj[0]>EPSDEV)return 1;
  if(xyzi[1]-xyzj[1]<-EPSDEV)return -1;
  if(xyzi[1]-xyzj[1]>+EPSDEV)return 1;
  return 0;
}

/* ----------------------- SetupZTreeDevices ----------------------------- */

void SetupZTreeDevices(void){
  int i;

  if(nztreedeviceinfo>0){
    FREEMEMORY(ztreedeviceinfo);
    FREEMEMORY(deviceinfo_sortedz);
    nztreedeviceinfo=0;
  }
  NewMemory((void **)&ztreedeviceinfo, global_scase.devicecoll.ndeviceinfo*sizeof(ztreedevicedata));
  NewMemory((void **)&deviceinfo_sortedz, global_scase.devicecoll.ndeviceinfo*sizeof(devicedata *));
  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    deviceinfo_sortedz[i] = global_scase.devicecoll.deviceinfo+i;
  }
  qsort((devicedata **)deviceinfo_sortedz, (size_t)global_scase.devicecoll.ndeviceinfo, sizeof(devicedata *), CompareZ3Devices);

  nztreedeviceinfo = 1;
  ztreedeviceinfo->first = 0;
  for(i = 1; i<global_scase.devicecoll.ndeviceinfo; i++){
    if(CompareZ2Devices(deviceinfo_sortedz+i, deviceinfo_sortedz+i-1)!=0){
      ztreedevicedata *ztreei;

      ztreei           = ztreedeviceinfo+nztreedeviceinfo-1;
      ztreei->n        = i-ztreei->first;
      ztreei->quantity = deviceinfo_sortedz[i-1]->quantity;
      ztreei->unit     = deviceinfo_sortedz[i-1]->unit;

      ztreei++;
      ztreei->first    = i;
      nztreedeviceinfo++;
    }
  }
  {
    ztreedevicedata *ztreei;

    ztreei           = ztreedeviceinfo+nztreedeviceinfo-1;
    ztreei->quantity = deviceinfo_sortedz[global_scase.devicecoll.ndeviceinfo-1]->quantity;
    ztreei->unit     = deviceinfo_sortedz[global_scase.devicecoll.ndeviceinfo-1]->unit;
    ztreei->n        = global_scase.devicecoll.ndeviceinfo-ztreei->first;
  }
  ResizeMemory((void **)&ztreedeviceinfo, nztreedeviceinfo*sizeof(ztreedevicedata));
}

/* ----------------------- SetupWindTreeDevices ----------------------------- */

void SetupWindTreeDevices(void){
  int i;
  treedevicedata *treei;

  if(global_scase.devicecoll.nvdeviceinfo==0)return;
  if(ntreedeviceinfo>0){
    FREEMEMORY(treedeviceinfo);
    ntreedeviceinfo=0;
  }

  if(nzwindtreeinfo>0){
    FREEMEMORY(zwindtreeinfo);
    nzwindtreeinfo = 0;
  }

  qsort((vdevicedata **)global_scase.devicecoll.vdevices_sorted,3*(size_t)global_scase.devicecoll.nvdeviceinfo,sizeof(vdevicesortdata), CompareV3Devices);

  ntreedeviceinfo = 1;
  for(i = 1; i < 3*global_scase.devicecoll.nvdeviceinfo; i++){
    if(CompareV2Devices(global_scase.devicecoll.vdevices_sorted+i, global_scase.devicecoll.vdevices_sorted+i-1) != 0)ntreedeviceinfo++;
  }

  NewMemory((void **)&treedeviceinfo,ntreedeviceinfo*sizeof(treedevicedata));

  ntreedeviceinfo = 1;
  treei = treedeviceinfo;
  treei->first = 0;
  for(i = 1; i < 3*global_scase.devicecoll.nvdeviceinfo; i++){
    if(CompareV2Devices(global_scase.devicecoll.vdevices_sorted + i, global_scase.devicecoll.vdevices_sorted + i - 1) != 0){
      treei->last = i-1;
      treei = treedeviceinfo + ntreedeviceinfo;
      treei->first = i;
      ntreedeviceinfo++;
    }
  }
  treei->last = 3*global_scase.devicecoll.nvdeviceinfo - 1;

  max_device_tree=0;
  for(i = 0; i < ntreedeviceinfo; i++){
    int j, n;

    treei = treedeviceinfo + i;
    n = 0;
    for(j = treei->first; j <= treei->last; j++){
      vdevicedata *vdevi;
      vdevicesortdata *vdevsorti;

      vdevsorti = global_scase.devicecoll.vdevices_sorted + j;
      vdevi = vdevsorti->vdeviceinfo;
      if(vdevi->unique != 0)n++;
    }
    treei->n = n;
    max_device_tree=MAX(max_device_tree,n);
  }

  for(i = 0; i<ntreedeviceinfo; i++){
    int j, nz;
    vdevicedata *vd;
    float *xyz = NULL;

    treei = treedeviceinfo+i;
    nz = 0;
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;

      vdevsorti = global_scase.devicecoll.vdevices_sorted + j;
      if(vdevsorti->dir==ZDIR){
        vd = vdevsorti->vdeviceinfo;
        if(vd->unique==0)continue;
        xyz = NULL;
        if(xyz==NULL&&vd->udev!=NULL)xyz = vd->udev->xyz;
        if(xyz==NULL&&vd->vdev!=NULL)xyz = vd->vdev->xyz;
        if(xyz==NULL&&vd->wdev!=NULL)xyz = vd->wdev->xyz;
        if(xyz == NULL&&vd->angledev != NULL)xyz = vd->angledev->xyz;
        if(xyz == NULL&&vd->veldev != NULL)xyz = vd->veldev->xyz;
        if(xyz!=NULL){
          treei->xyz = xyz;
          nz++;
        }
      }
    }
    if(nz>1)nzwindtreeinfo++;
    treei->nz = nz;
  }

  if(nzwindtreeinfo>0)NewMemory((void **)&zwindtreeinfo, nzwindtreeinfo*sizeof(treedevicedata *));

  nzwindtreeinfo=0;
  for(i = 0; i<ntreedeviceinfo; i++){
    int j, nz;
    vdevicedata *vd;

    treei = treedeviceinfo+i;
    nz = 0;
    for(j = treei->first; j<=treei->last; j++){
      vdevicesortdata *vdevsorti;

      vdevsorti = global_scase.devicecoll.vdevices_sorted + j;
      if(vdevsorti->dir==ZDIR){
        vd = vdevsorti->vdeviceinfo;
        if(vd->unique==0)continue;
        if(vd->udev==NULL&&vd->vdev==NULL&&vd->wdev==NULL&&
           vd->angledev == NULL&&vd->veldev == NULL)continue;
        nz++;
      }
    }
    if(nz>1)zwindtreeinfo[nzwindtreeinfo++] = treei;
  }
}

/* ----------------------- SetupZoneDevs ----------------------------- */

void SetupZoneDevs(void){
  int i;

  show_missing_objects = 0;
  for(i=0;i<global_scase.nzoneinfo;i++){
    FILE *stream;
    char *file;
    int nrows, ncols, buffer_len,ntokens;
    char *buffer=NULL, *buffer_temp, **devclabels=NULL;
    zonedata *zonei;
    int j;

    zonei = global_scase.zoneinfo + i;
    if(zonei->csv!=1)continue;
    file = zonei->file;

    stream=fopen(file,"r");
    if(stream==NULL)continue;
    buffer_len=GetRowCols(stream,&nrows,&ncols);
    buffer_len += ncols;
    if(nrows<=0||ncols<=0||buffer_len<=0){
      fclose(stream);
      continue;
    }
    buffer_len+=10;
    rewind(stream);

    NewMemory((void **)&buffer,      buffer_len);
    NewMemory((void **)&buffer_temp, buffer_len);
    NewMemory((void **)&devclabels,  ncols*sizeof(char *));
    fgets(buffer,buffer_len,stream);
    fgets(buffer,buffer_len,stream);
    ParseCSV(buffer, buffer_temp, devclabels, &ntokens);
    for(j=0;j<ntokens;j++){
      devicedata *devi;

      TrimBack(devclabels[j]);
      devclabels[j]=TrimFront(devclabels[j]);
      devi = GetDeviceFromLabel(devclabels[j],-1);
      if(devi!=NULL)devi->in_zone_csv=1;
    }
    FREEMEMORY(devclabels);
    FREEMEMORY(buffer);
    fclose(stream);
  }
}

/* ----------------------- GetDeviceLabel ----------------------------- */

char *GetDeviceLabel(char *buffer){
  char *label_present;

  label_present = strstr(buffer, "#");
  if(label_present == NULL) return NULL;
  if(strlen(label_present) <= 1){
    label_present[0] = 0;
    return NULL;
  }
  label_present[0] = 0;
  label_present++;
  label_present = TrimFront(label_present);
  TrimBack(label_present);
  if(strlen(label_present) == 0) return NULL;
  return label_present;
}

void RewindDeviceFile(FILE *stream){
#define BUFFER_LEN 255
  char buffer[BUFFER_LEN], *comma;
  int found_data = 0, buffer_len = BUFFER_LEN;

  fgets(buffer, buffer_len, stream);
  comma = strchr(buffer, ',');
  if(comma != NULL) *comma = 0;
  TrimBack(buffer);
  if(strcmp(buffer, "//HEADER") != 0){
    rewind(stream);
    return;
  }
  while(!feof(stream)){
    fgets(buffer, buffer_len, stream);
    comma = strchr(buffer, ',');
    if(comma != NULL) *comma = 0;
    TrimBack(buffer);
    if(strcmp(buffer, "//DATA") == 0){
      found_data = 1;
      break;
    }
  }
  if(found_data == 0){
    fprintf(stderr,
            "*** Warning //DATA keyword not found in spreadsheet file\n");
  }
}


/* ----------------------- ReadDeviceData ----------------------------- */

FILE_SIZE ReadDeviceData(char *file, int filetype, int loadstatus){
  FILE *stream;
  int nrows, ncols;
  int irow;
  float *vals=NULL;
  int *valids=NULL;
  int i;
  char *buffer, *buffer2, *buffer_temp;
  char **devcunits=NULL, **devclabels=NULL;
  devicedata **devices=NULL;
  int ntokens;
  int buffer_len;
  float *times_local;

// unload data

  if(loadstatus==UNLOAD){
    for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
      devicedata *devicei;

      devicei = global_scase.devicecoll.deviceinfo + i;
      if(devicei->filetype!=filetype)continue;
      FREEMEMORY(devicei->vals);
      FREEMEMORY(devicei->vals_orig);
      FREEMEMORY(devicei->valids);
    }
    for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
      devicedata *devicei;
      int j;

      devicei = global_scase.devicecoll.deviceinfo + i;
      if(devicei->filetype!=filetype||devicei->times==NULL)continue;
      times_local = devicei->times;
      FREEMEMORY(devicei->times);
      for(j=i+1;j<global_scase.devicecoll.ndeviceinfo;j++){
        devicedata *devicej;

        devicej = global_scase.devicecoll.deviceinfo + j;
        if(devicej->filetype!=filetype)continue;
        if(times_local==devicej->times)devicej->times=NULL;
      }
    }
    return 0;
  }

  // find number of rows and columns

  stream=fopen(file,"r");
  if(stream==NULL)return 0;
  RewindDeviceFile(stream);
  buffer_len=GetRowCols(stream,&nrows,&ncols);
  buffer_len += (ncols+1);
  if(nrows<=0||ncols<=0||buffer_len<=0){
    fclose(stream);
    return 0;
  }
  buffer_len+=10;
  RewindDeviceFile(stream);

  FILE_SIZE file_size;

  file_size = GetFileSizeSMV(file);

  NewMemory((void **)&buffer,      buffer_len);
  NewMemory((void **)&buffer2,     buffer_len);
  NewMemory((void **)&buffer_temp, buffer_len);
  NewMemory((void **)&vals,        ncols*sizeof(float));
  NewMemory((void **)&valids,      ncols*sizeof(int));
  NewMemory((void **)&devcunits,   ncols*sizeof(char *));
  NewMemory((void **)&devclabels,  ncols*sizeof(char *));
  NewMemory((void **)&devices,     ncols*sizeof(devicedata *));

  for(i = 0; i<ncols; i++){
    devices[i] = NULL;
  }

  fgets(buffer, buffer_len, stream);
  ParseCSV(buffer, buffer_temp, devcunits, &ntokens);
  for(i=0;i<ntokens;i++){
    TrimBack(devcunits[i]);
    devcunits[i]=TrimFront(devcunits[i]);
  }

  fgets(buffer2, buffer_len, stream);
  ParseCSV(buffer2, buffer_temp, devclabels, &ntokens);
  for(i=0;i<ntokens;i++){
    TrimBack(devclabels[i]);
    devclabels[i]=TrimFront(devclabels[i]);
  }

  NewMemory((void **)&times_local,nrows*sizeof(float));
  for(i=1;i<ntokens;i++){
    devicedata *devicei;

    devicei = GetCSVDeviceFromLabel(devclabels[i],i-1);
    devices[i]=devicei;
#ifdef _DEBUG
    if(devicei==NULL){
      fprintf(stderr,"*** Error: spreadsheet entry: %s is not present in %s\n",devclabels[i],smv_filename);
      continue;
    }
#endif
    if(devicei==NULL)continue;
    devicei->filetype=filetype;
    if(filetype==CSV_FDS)devicei->in_devc_csv=1;
    NewMemory((void **)&devicei->vals,nrows*sizeof(float));
    NewMemory((void **)&devicei->valids,nrows*sizeof(int));
    devicei->times=times_local;
    NewMemory((void **)&devicei->vals_orig,nrows*sizeof(float));
    strcpy(devicei->unit,devcunits[i]);
    devicei->nvals=nrows-2;
  }

  for(irow=2;irow<nrows;irow++){
    int icol=0;

    fgets(buffer,buffer_len,stream);
    FParseCSV(buffer,vals,valids,ncols,&ntokens);
    times_local[irow-2]=vals[icol];
    for(icol=1;icol<ncols;icol++){
      devicedata *devicei;

      devicei = devices[icol];
      if(devicei==NULL)continue;
      devicei->vals[irow-2]=vals[icol];
      devicei->vals_orig[irow-2]=vals[icol];
      devicei->valids[irow-2]=valids[icol];
    }
  }
  FREEMEMORY(buffer);
  FREEMEMORY(buffer2);
  fclose(stream);

  FREEMEMORY(vals);
  FREEMEMORY(valids);
  FREEMEMORY(devcunits);
  FREEMEMORY(devclabels)
  FREEMEMORY(devices);
  return file_size;
}


/* ----------------------- GetVDevice ----------------------------- */

vdevicedata *GetVDevice(float *xyzval){
  int j;

  for(j=0;j<global_scase.devicecoll.nvdeviceinfo;j++){
    vdevicedata *vdevj;
    float *xyzj;

    vdevj = global_scase.devicecoll.vdeviceinfo + j;

    xyzj = vdevj->valdev->xyz;
    if(ABS(xyzval[0]-xyzj[0])>EPSDEV)continue;
    if(ABS(xyzval[1]-xyzj[1])>EPSDEV)continue;
    if(ABS(xyzval[2]-xyzj[2])>EPSDEV)continue;
    return vdevj;
  }
  return NULL;
}

/* ----------------------- GetDeviceFromPosition ----------------------------- */

devicedata *GetDeviceFromPosition(float *xyzval, char *device_label, int device_type){
  int j;

  for(j=0;j<global_scase.devicecoll.ndeviceinfo;j++){
    devicedata *devj;
    float *xyz;

    devj = global_scase.devicecoll.deviceinfo + j;
    if(devj->filetype!=device_type)continue;
    xyz = devj->xyz;
    if(strcmp(devj->quantity,device_label)!=0)continue;
    if(ABS(xyz[0]-xyzval[0])>EPSDEV)continue;
    if(ABS(xyz[1]-xyzval[1])>EPSDEV)continue;
    if(ABS(xyz[2]-xyzval[2])>EPSDEV)continue;
    return devj;
  }
  return NULL;
}

/* ----------------------- update_colordevs ----------------------------- */

void UpdateColorDevices(void){
  int i;
  devicedata *colordev;

  colordev = devicetypes[devicetypes_index];

  for(i=0;i<global_scase.devicecoll.nvdeviceinfo;i++){
    vdevicedata *vdevi;

    vdevi = global_scase.devicecoll.vdeviceinfo + i;
    vdevi->colordev=NULL;
  }
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devi;
    vdevicedata *vdevi;
    devi = global_scase.devicecoll.deviceinfo + i;
    vdevi = devi->vdevice;
    if(vdevi==NULL)continue;
    if(strcmp(colordev->quantity,devi->quantity)==0){
      vdevi->colordev=devi;
    }
  }
}

/* ----------------------- IsDupDeviceLabel ----------------------------- */
#define BEFORE 0
#define AFTER 1

int IsDupDeviceLabel(int index, int direction){
  int i,i1,i2;
  devicedata *dev_index;

  if(direction==BEFORE){
    i1=0;
    i2=index;
  }
  else{
    i1=index+1;
    i2=global_scase.devicecoll.ndeviceinfo;
  }
  dev_index = global_scase.devicecoll.deviceinfo + index;
  if(index<0||index>=global_scase.devicecoll.ndeviceinfo||STRCMP(dev_index->deviceID,"null")==0||dev_index->in_devc_csv==0)return 0;

  for(i=i1;i<i2;i++){
    devicedata *devi;

    devi = global_scase.devicecoll.deviceinfo + i;
    if(STRCMP(devi->deviceID, "null")==0)continue;
    if(STRCMP(dev_index->deviceID,devi->deviceID)==0)return 1;
  }
  return 0;
}

/* ----------------------- DeviceData2WindRose ----------------------------- */

#define WINDROSE_EPS 0.001

void DeviceData2WindRose(int nr, int ntheta){
  int i;

  maxr_windrose = 0.0;
  for(i = 0; i < global_scase.devicecoll.nvdeviceinfo; i++){
    vdevicedata *vdevicei;
    windrosedata *windroseinfo;
    devicedata *udev, *vdev, *wdev;
    devicedata *angledev, *veldev;
#define MAXDEVS 1000000000
    int nvals=MAXDEVS;
    int use_uvw_dev = 0, use_angle_dev=0;
    int k;

    vdevicei = global_scase.devicecoll.vdeviceinfo + i;
    udev = vdevicei->udev;
    vdev = vdevicei->vdev;
    wdev = vdevicei->wdev;
    angledev = vdevicei->angledev;
    veldev = vdevicei->veldev;

    if(udev!=NULL||vdev!=NULL||wdev!=NULL){
      use_uvw_dev = 1;
      if(udev!=NULL)nvals = MIN(nvals, udev->nvals);
      if(vdev!=NULL)nvals = MIN(nvals, vdev->nvals);
      if(wdev!=NULL)nvals = MIN(nvals, wdev->nvals);
      if(udev!=NULL&&vdev!=NULL)windrose_xy_active = 1;
      if(udev!=NULL&&wdev!=NULL)windrose_xz_active = 1;
      if(vdev!=NULL&&wdev!=NULL)windrose_yz_active = 1;
    }
    if(angledev != NULL&&veldev != NULL){ // if have 'polar' devices then don't use cartesian
      use_uvw_dev = 0;
      use_angle_dev = 1;
      nvals = MIN(angledev->nvals, veldev->nvals);
    }

// free windrose data

    for(k = 0; k<vdevicei->nwindroseinfo; k++){
      windrosedata *windrosei;
      histogramdata *histogram;

      windrosei = vdevicei->windroseinfo+k;

      histogram = windrosei->histogram+0;
      FREEMEMORY(histogram->buckets_polar);
      if(use_uvw_dev==1){
        histogram = windrosei->histogram+1;
        FREEMEMORY(histogram->buckets_polar);
        histogram = windrosei->histogram+2;
        FREEMEMORY(histogram->buckets_polar);
      }
    }
    FREEMEMORY(vdevicei->windroseinfo);

// allocate windrose data

    vdevicei->nwindroseinfo = MAX(1,nglobal_times);
    NewMemory((void **)&windroseinfo, vdevicei->nwindroseinfo * sizeof(windrosedata));
    vdevicei->windroseinfo= windroseinfo;

    if(use_uvw_dev==1){
      float rmin, rmax;

      for(k = 0; k<vdevicei->nwindroseinfo; k++){
        windrosedata *windrosei;
        int j;

        windrosei = vdevicei->windroseinfo+k;
        windrosei->xyz = udev->xyz;

        for(j = 0; j<3; j++){
          float *times, *uvals, *vvals, tmin, tmax;
          histogramdata *histogram;

          histogram = windrosei->histogram+j;
          InitHistogramPolar(histogram, nr, ntheta, NULL, NULL);
          uvals = NULL;
          vvals = NULL;
          switch(j){
          case 0:
            if(udev!=NULL)uvals = udev->vals;
            if(vdev!=NULL)vvals = vdev->vals;
            break;
          case 1:
            if(udev!=NULL)uvals = udev->vals;
            if(wdev!=NULL)vvals = wdev->vals;
            break;
          case 2:
            if(vdev!=NULL)uvals = vdev->vals;
            if(wdev!=NULL)vvals = wdev->vals;
            break;
          default:
            assert(FFALSE);
            break;
          }
          if(udev!=NULL)times = udev->times;
          if(vdev!=NULL)times = vdev->times;
          if(wdev!=NULL)times = wdev->times;
          if(uvals==NULL||vvals==NULL)continue;
          if(global_times!=NULL&&windrose_merge_dxyzt[3]>0.1&&windrose_ttype==WINDROSE_USE_DT){
            if(windrose_merge_type==WINDROSE_STEPPED){
              float dtime;
              int itime;

              dtime = MAX(windrose_merge_dxyzt[3],1.0);
              itime = (int)(global_times[k]/dtime);
              tmin = (float)itime*dtime;
              tmax = MIN((float)(itime+1)*dtime, global_times[vdevicei->nwindroseinfo-1]);
              if(ABS(tmax-tmin)<dtime){
                tmin = (float)(itime-1)*dtime;
                tmax = global_times[vdevicei->nwindroseinfo-1];
              }
            }
            else if(windrose_merge_type==WINDROSE_SLIDING){
              tmin = global_times[k]-windrose_merge_dxyzt[3];
              tmax = global_times[k]+windrose_merge_dxyzt[3];
            }
            else{ // WINDROSE_POINT
              tmin = 0.0;
              tmax = 0.0;
              times = NULL;
            }
          }
          else if(times!=NULL&&windrose_merge_dxyzt[5]>windrose_merge_dxyzt[4]&&windrose_ttype==WINDROSE_USE_TMINMAX){
            tmin = windrose_merge_dxyzt[4];
            tmax = windrose_merge_dxyzt[5];
          }
          else{
            tmin = 0.0;
            tmax = 0.0;
            times = NULL;
          }
          if(Get2DBounds(times, uvals, vvals, nvals, tmin, tmax, &rmin, &rmax)>0){
            CopyUV2Histogram(times, uvals, vvals, nvals, tmin, tmax, rmin, rmax, histogram);
            maxr_windrose = MAX(maxr_windrose, histogram->bucket_maxr);
          }
        }
      }
    }
    if(use_angle_dev==1){
      float rmin=0.0, rmax=1.0;

      nvals = MIN(angledev->nvals, veldev->nvals);
      windrose_xy_active = 1;

      for(k = 0; k<vdevicei->nwindroseinfo; k++){
        windrosedata *windrosei;
        int j;
        histogramdata *histogram;

        windrosei = vdevicei->windroseinfo+k;
        windrosei->xyz = angledev->xyz;
        histogram = windrosei->histogram;
        InitHistogramPolar(histogram, nr, ntheta, NULL, NULL);
        if(windrose_merge_type==WINDROSE_POINT){
          GetPolarBounds(veldev->vals, nvals, &rmin, &rmax);
          CopyPolar2Histogram(veldev->vals, angledev->vals, nvals, rmin, rmax, histogram);
        }
        else{
          float *xyzi;
          int first = 1;

          xyzi = veldev->xyz;
          // find min rmin and max rmax
          for(j = 0; j<global_scase.devicecoll.nvdeviceinfo; j++){
            vdevicedata *vdevicej;
            devicedata *angledevj, *veldevj;
            float *xyzj, rminj, rmaxj;

            vdevicej = global_scase.devicecoll.vdeviceinfo+j;
            angledevj = vdevicej->angledev;
            veldevj = vdevicej->veldev;
            if(angledevj==NULL||veldevj==NULL)continue;
            nvals = MIN(angledevj->nvals, veldevj->nvals);
            xyzj = veldevj->xyz;
            if(vdevicei==vdevicej||
              (ABS(xyzi[0]-xyzj[0])<=WINDROSE_EPS+windrose_merge_dxyzt[0]&&
                ABS(xyzi[1]-xyzj[1])<=WINDROSE_EPS+windrose_merge_dxyzt[1]&&
                ABS(xyzi[2]-xyzj[2])<=WINDROSE_EPS+windrose_merge_dxyzt[2])){

              GetPolarBounds(veldev->vals, nvals, &rminj, &rmaxj);
              if(first==1){
                first = 0;
                rmin = rminj;
                rmax = rmaxj;
              }
              else{
                rmin = MIN(rminj, rmin);
                rmax = MAX(rmaxj, rmax);
              }
            }
          }
          // update windrose
          for(j = 0; j<global_scase.devicecoll.nvdeviceinfo; j++){
            vdevicedata *vdevicej;
            devicedata *angledevj, *veldevj;
            float *xyzj;

            vdevicej = global_scase.devicecoll.vdeviceinfo+j;
            angledevj = vdevicej->angledev;
            veldevj = vdevicej->veldev;
            if(angledevj==NULL||veldevj==NULL)continue;
            nvals = MIN(angledev->nvals, veldev->nvals);
            xyzj = veldevj->xyz;
            if(vdevicei==vdevicej||
              (ABS(xyzi[0]-xyzj[0])<=WINDROSE_EPS+windrose_merge_dxyzt[0]&&
                ABS(xyzi[1]-xyzj[1])<=WINDROSE_EPS+windrose_merge_dxyzt[1]&&
                ABS(xyzi[2]-xyzj[2])<=WINDROSE_EPS+windrose_merge_dxyzt[2])){

              CopyPolar2Histogram(veldevj->vals, angledevj->vals, nvals, rmin, rmax, histogram);
            }
          }
        }
        maxr_windrose = MAX(maxr_windrose, histogram->bucket_maxr);
      }
    }
  }
}

/* ----------------------- SetupDeviceData ----------------------------- */

void SetupDeviceData(void){
  float *vals=NULL;
  int *valids=NULL;
  int i;
  char **devcunits=NULL, **devclabels=NULL;
  int is_dup;

  if(global_scase.devicecoll.ndeviceinfo==0)return; // only setup device data once
  devices_setup = 1;
  FREEMEMORY(global_scase.devicecoll.vdeviceinfo);
  NewMemory((void **)&global_scase.devicecoll.vdeviceinfo,global_scase.devicecoll.ndeviceinfo*sizeof(vdevicedata));
  FREEMEMORY(global_scase.devicecoll.vdevices_sorted);
  NewMemory((void **)&global_scase.devicecoll.vdevices_sorted,3*global_scase.devicecoll.ndeviceinfo*sizeof(vdevicesortdata));
  global_scase.devicecoll.nvdeviceinfo=0;
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    vdevicedata *vdevi;
    devicedata *devi,*devj;
    float *xyzval;

    devi = global_scase.devicecoll.deviceinfo+i;
    xyzval = devi->xyz;
    devi->vdevice = NULL;

    vdevi = global_scase.devicecoll.vdeviceinfo+global_scase.devicecoll.nvdeviceinfo;
    vdevi->valdev = devi;
    vdevi->udev = NULL;
    vdevi->vdev = NULL;
    vdevi->wdev = NULL;
    vdevi->angledev = NULL;
    vdevi->veldev = NULL;
    vdevi->sd_angledev = NULL;
    vdevi->sd_veldev = NULL;
    vdevi->colordev = NULL;

    devj = GetDeviceFromPosition(xyzval, "VELOCITY", CSV_EXP);
    if(devj!=NULL){
      vdevi->veldev = devj;
      vdevi->filetype = CSV_EXP;
    }

    devj = GetDeviceFromPosition(xyzval, "SD_VELOCITY", CSV_EXP);
    if(devj!=NULL){
      vdevi->sd_veldev = devj;
      vdevi->filetype = CSV_EXP;
    }

    devj = GetDeviceFromPosition(xyzval, "ANGLE", CSV_EXP);
    if(devj!=NULL){
      vdevi->angledev = devj;
      vdevi->filetype = CSV_EXP;
    }

    devj = GetDeviceFromPosition(xyzval, "SD_ANGLE", CSV_EXP);
    if(devj!=NULL){
      vdevi->sd_angledev = devj;
      vdevi->filetype = CSV_EXP;
    }

    devj = GetDeviceFromPosition(xyzval, "U-VELOCITY", CSV_FDS);
    if(devj!=NULL){
      vdevi->udev = devj;
      vdevi->filetype = CSV_FDS;
    }

    devj = GetDeviceFromPosition(xyzval, "V-VELOCITY", CSV_FDS);
    if(devj!=NULL){
      vdevi->vdev = devj;
      vdevi->filetype = CSV_FDS;
    }

    devj = GetDeviceFromPosition(xyzval, "W-VELOCITY", CSV_FDS);
    if(devj!=NULL){
      vdevi->wdev = devj;
      vdevi->filetype = CSV_FDS;
    }

    if(vdevi->udev!=NULL||vdevi->vdev!=NULL||vdevi->wdev!=NULL||
      vdevi->angledev!=NULL||vdevi->veldev!=NULL){
      vdevi->unique=1;
      vdevi->display = 1;
      global_scase.devicecoll.nvdeviceinfo++;
    }
  }

  // look for duplicate device labels

  is_dup=0;
  for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
    devicedata *devi;

    devi = global_scase.devicecoll.deviceinfo + i;
    if(STRCMP(devi->deviceID,"null")==0)continue;
    if(IsDupDeviceLabel(i,AFTER)==1){
      is_dup=1;
      break;
    }
  }
  if(is_dup==1){
    int ii;

    fprintf(stderr,"*** Warning: Duplicate device labels: ");
    for(ii=0;ii<global_scase.devicecoll.ndeviceinfo;ii++){
      devicedata *devi;

      devi = global_scase.devicecoll.deviceinfo + ii;
      if(STRCMP(devi->deviceID,"null")==0)continue;
      if(IsDupDeviceLabel(ii,BEFORE)==0&& IsDupDeviceLabel(ii,AFTER)==1){
        fprintf(stderr," %s,",devi->deviceID);
      }
    }
    fprintf(stderr," found in %s\n",global_scase.paths.fds_filein);
  }
  for(i=0;i<global_scase.devicecoll.nvdeviceinfo;i++){
    vdevicedata *vdevi;
    int j;
    float *xyzi;

    vdevi = global_scase.devicecoll.vdeviceinfo + i;
    xyzi = vdevi->valdev->xyz;
    for(j=i+1;j<global_scase.devicecoll.nvdeviceinfo;j++){
      vdevicedata *vdevj;
      float *xyzj;

      vdevj = global_scase.devicecoll.vdeviceinfo + j;
      if(vdevj->unique==0)continue;
      xyzj = vdevj->valdev->xyz;
      if(ABS(xyzi[0]-xyzj[0])>EPSDEV)continue;
      if(ABS(xyzi[1]-xyzj[1])>EPSDEV)continue;
      if(ABS(xyzi[2]-xyzj[2])>EPSDEV)continue;
      vdevj->unique=0;
    }
  }
  max_dev_vel=-1.0;
  for(i=0;i<global_scase.devicecoll.nvdeviceinfo;i++){
    vdevicedata *vdevi;
    devicedata *devval;
    int j;

    vdevi = global_scase.devicecoll.vdeviceinfo + i;
    if(vdevi->unique==0)continue;
    devval = vdevi->valdev;
    if(vdevi->udev!=NULL)vdevi->udev->vdevice=vdevi;
    if(vdevi->vdev!=NULL)vdevi->vdev->vdevice=vdevi;
    if(vdevi->wdev!=NULL)vdevi->wdev->vdevice=vdevi;
    if(vdevi->filetype==CSV_FDS){
      devicedata *udev,*vdev,*wdev;

      udev=vdevi->udev;
      vdev=vdevi->vdev;
      wdev=vdevi->wdev;
      for(j=0;j<devval->nvals;j++){
        float uvel=0.0, vvel=0.0, wvel=0.0;
        float speed;

        if(udev!=NULL)uvel=udev->vals[j];
        if(vdev!=NULL)vvel=vdev->vals[j];
        if(wdev!=NULL)wvel=wdev->vals[j];
        speed = sqrt(uvel*uvel+vvel*vvel+wvel*wvel);
        if(speed>max_dev_vel)max_dev_vel=speed;
      }
    }
    if(vdevi->filetype==CSV_EXP){
      devicedata *veldev;

      veldev=vdevi->veldev;
      if(veldev!=NULL){
        for(j=0;j<devval->nvals;j++){
          if(veldev->valids[j]==1){
            float speed;

            speed=veldev->vals[j];
            if(speed>max_dev_vel)max_dev_vel=speed;
          }
        }
      }
    }
  }

  // find devices linked with each vdevice

  if(global_scase.devicecoll.ndeviceinfo>0){
    for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
      devicedata *devi;
      float *xyzi;
      vdevicedata *vdevj;

      devi = global_scase.devicecoll.deviceinfo + i;
      if(devi->vdevice!=NULL)continue;
      xyzi = devi->xyz;
      vdevj = GetVDevice(xyzi);
      if(vdevj!=NULL)devi->vdevice=vdevj;
    }
  }

  //setup devicetypes
  if(global_scase.devicecoll.ndeviceinfo>0){
    ndevicetypes=0;
    FREEMEMORY(devicetypes);
    NewMemory((void **)&devicetypes,global_scase.devicecoll.ndeviceinfo*sizeof(devicedata *));
    for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
      devicedata *devi;

      devi = global_scase.devicecoll.deviceinfo + i;
      devi->type2=-1;
    }
    for(i=0;i<global_scase.devicecoll.ndeviceinfo;i++){
      int j;
      devicedata *devi;

      devi = global_scase.devicecoll.deviceinfo + i;
      if(devi->type2>=0||devi->nvals==0||strlen(devi->quantity)==0)continue;
      devi->type2=ndevicetypes;
      devi->type2vis=0;
      devicetypes[ndevicetypes++]=devi;
      for(j=i+1;j<global_scase.devicecoll.ndeviceinfo;j++){
        devicedata *devj;

        devj = global_scase.devicecoll.deviceinfo + j;
        if(devj->type2<0&&strcmp(devi->quantity,devj->quantity)==0){
          devj->type2=devi->type2;
        }
      }
    }
    if(ndevicetypes>0)devicetypes[0]->type2vis=1;
  }
  for(i=0;i<global_scase.devicecoll.nvdeviceinfo;i++){
    vdevicesortdata *vdevsorti;

    vdevsorti = global_scase.devicecoll.vdevices_sorted + i;
    vdevsorti->vdeviceinfo = global_scase.devicecoll.vdeviceinfo + i;
    vdevsorti->dir = XDIR;

    vdevsorti = global_scase.devicecoll.vdevices_sorted + global_scase.devicecoll.nvdeviceinfo + i;
    vdevsorti->vdeviceinfo = global_scase.devicecoll.vdeviceinfo + i;
    vdevsorti->dir = YDIR;

    vdevsorti = global_scase.devicecoll.vdevices_sorted + 2*global_scase.devicecoll.nvdeviceinfo + i;
    vdevsorti->vdeviceinfo = global_scase.devicecoll.vdeviceinfo + i;
    vdevsorti->dir = ZDIR;
  }
  for(i = 0; i<global_scase.devicecoll.nvdeviceinfo; i++){
    vdevicedata *vdevicei;

    vdevicei = global_scase.devicecoll.vdeviceinfo+i;
    vdevicei->nwindroseinfo = 0;
    vdevicei->windroseinfo = NULL;
  }

  SetupWindTreeDevices();
  SetupZTreeDevices();
  UpdateColorDevices();

  if(viswindrose==1)DeviceData2WindRose(nr_windrose,ntheta_windrose);

  FREEMEMORY(vals);
  FREEMEMORY(valids);
  FREEMEMORY(devcunits);
  FREEMEMORY(devclabels)
}

/* ------------------ InitializeDeviceCsvData ------------------------ */

void InitializeDeviceCsvData(int flag){
  int i;
  FILE_SIZE file_size = 0;
  float total_time;

  if(flag == LOAD)printf("Loading CSV files");
  START_TIMER(total_time);
  INIT_PRINT_TIMER(device_timer);
  ReadDeviceData(NULL, CSV_FDS, UNLOAD);
  ReadDeviceData(NULL, CSV_EXP, UNLOAD);
  for(i = 0; i < global_scase.csvcoll.ncsvfileinfo; i++){
    csvfiledata *csvi;

    csvi = global_scase.csvcoll.csvfileinfo + i;
    if(strcmp(csvi->c_type, "devc") == 0)file_size += ReadDeviceData(csvi->file, CSV_FDS, flag);
    if(strcmp(csvi->c_type, "ext") == 0)file_size += ReadDeviceData(csvi->file, CSV_EXP, flag);
  }
  PRINT_TIMER(device_timer, "ReadDeviceData");
  INIT_PRINT_TIMER(setup_timer);
  if(flag==LOAD)SetupDeviceData();
  PRINT_TIMER(setup_timer, "SetupDeviceData");
  INIT_PRINT_TIMER(csv_timer);
  file_size += ReadAllCSVFiles(flag);
  PRINT_TIMER(csv_timer, "ReadAllCSVFiles");
  if(flag==LOAD){
    csv_loaded = 1;
    plot2d_show_plots = 1;
    STOP_TIMER(total_time);
    printf("\n");
    PrintFileLoadTimes(2,file_size, total_time);
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateTimes();
    ForceIdle();
  }

}

/* ----------------------- UpdateObjectUsed ----------------------------- */

void UpdateObjectUsed(void){
  int i;

  for(i = 0; i<global_scase.objectscoll.nobject_defs; i++){
    sv_object *obj_typei;

    obj_typei = global_scase.objectscoll.object_defs[i];
    obj_typei->used_by_device = 0;
  }
  for(i = 0; i<global_scase.devicecoll.ndeviceinfo; i++){
    devicedata *devicei;
    propdata *propi;
    int jj;

    devicei = global_scase.devicecoll.deviceinfo+i;
    propi = devicei->prop;
    if(propi==NULL)continue;
    for(jj = 0; jj<propi->nsmokeview_ids; jj++){
      sv_object *objectj;

      objectj = propi->smv_objects[jj];
      objectj->used_by_device = 1;
    }
  }
  for(i = 0; i<npart5prop; i++){
    partpropdata *partpropi;
    int j;

    partpropi = part5propinfo+i;
    for(j = 0; j<global_scase.npartclassinfo; j++){
      partclassdata *partclassj;
      propdata *propi;
      int jj;

      if(partpropi->class_present[j]==0)continue;
      partclassj = global_scase.partclassinfo+j;
      propi = partclassj->prop;
      if(propi==NULL)continue;
      for(jj = 0; jj<propi->nsmokeview_ids; jj++){
        sv_object *objectj;

        objectj = propi->smv_objects[jj];
        objectj->used_by_device = 1;
      }
    }
  }
}

/* ----------------------- Dist2Plane ------------------------ */

float Dist2Plane(float x, float y, float z, float xyzp[3], float xyzpn[3]){
  float return_val;
  float xyz[3];
  int i;

  xyz[0]=x;
  xyz[1]=y;
  xyz[2]=z;
  return_val=0.0;
  for(i=0;i<3;i++){
    return_val+=(xyz[i]-xyzp[i])*xyzpn[i];
  }
  return return_val;
}

/* ----------------------- InitDevicePlane ------------------------ */

void InitDevicePlane(devicedata *devicei){
  int colorindex;
  int i;
  float level=0.0;
  float xx[2], yy[2], zz[2];

/* stuff min and max grid data into a more convenient form
  assuming the following grid numbering scheme

       5-------6
     / |      /|
   /   |     / |
  4 -------7   |
  |    |   |   |
  Z    1---|---2
  |  Y     |  /
  |/       |/
  0--X-----3

  */
  if(devicei->plane_surface==NULL)return;
  if(devicei->color==NULL){
    float rgbcolor[4];

    rgbcolor[0]=1.0;
    rgbcolor[1]=0.0;
    rgbcolor[2]=0.0;
    rgbcolor[3]=1.0;
    devicei->color=GetColorPtr(&global_scase, rgbcolor);
  }
  colorindex=0;
  for(i=0;i<global_scase.meshescoll.nmeshes;i++){
    int j;
    meshdata *meshi;
    float xvert[12], yvert[12], zvert[12];
    int triangles[18];
    int nvert, ntriangles;
    int nodeindexes[8], closestnodes[18];
    float vals[8];

    InitIsoSurface(devicei->plane_surface[i],level,devicei->color,colorindex);
    devicei->plane_surface[i]->cullfaces=1;

    meshi = global_scase.meshescoll.meshinfo + i;

    xx[0]=meshi->xyz_bar0[XXX];
    xx[1]=SMV2FDS_X(meshi->xyz_bar[XXX]);

    yy[0]=meshi->xyz_bar0[YYY];
    yy[1]=SMV2FDS_Y(meshi->xyz_bar[YYY]);

    zz[0]=meshi->xyz_bar0[ZZZ];
    zz[1]=SMV2FDS_Z(meshi->xyz_bar[ZZZ]);

    for(j=0;j<8;j++){
      nodeindexes[j]=j;
    }
    vals[0]= Dist2Plane(xx[0],yy[0],zz[0],devicei->xyz,devicei->xyznorm);
    vals[1]= Dist2Plane(xx[0],yy[1],zz[0],devicei->xyz,devicei->xyznorm);
    vals[2]= Dist2Plane(xx[1],yy[1],zz[0],devicei->xyz,devicei->xyznorm);
    vals[3]= Dist2Plane(xx[1],yy[0],zz[0],devicei->xyz,devicei->xyznorm);
    vals[4]= Dist2Plane(xx[0],yy[0],zz[1],devicei->xyz,devicei->xyznorm);
    vals[5]= Dist2Plane(xx[0],yy[1],zz[1],devicei->xyz,devicei->xyznorm);
    vals[6]= Dist2Plane(xx[1],yy[1],zz[1],devicei->xyz,devicei->xyznorm);
    vals[7]= Dist2Plane(xx[1],yy[0],zz[1],devicei->xyz,devicei->xyznorm);

    xx[0]=FDS2SMV_X(meshi->xyz_bar0[XXX]);
    yy[0]=FDS2SMV_Y(meshi->xyz_bar0[YYY]);
    zz[0]=FDS2SMV_Z(meshi->xyz_bar0[ZZZ]);
    xx[1]=meshi->xyz_bar[XXX];
    yy[1]=meshi->xyz_bar[YYY];
    zz[1]=meshi->xyz_bar[ZZZ];

    GetIsoHexaHedron(xx, yy, zz, NULL, vals, NULL, nodeindexes, level,
              xvert, yvert, zvert, NULL, closestnodes, &nvert, triangles, &ntriangles, NULL, NULL);

    UpdateIsosurface(devicei->plane_surface[i], xvert, yvert, zvert, NULL,
                     closestnodes, nvert, triangles, ntriangles);
    GetNormalSurface(devicei->plane_surface[i]);
    CompressIsoSurface(devicei->plane_surface[i],1,
          global_scase.xbar0,2*global_scase.xbar,global_scase.ybar0,2*global_scase.ybar,global_scase.zbar0,global_scase.zbar);
    SmoothIsoSurface(devicei->plane_surface[i]);
  }

}
