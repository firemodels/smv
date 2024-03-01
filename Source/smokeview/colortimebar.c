#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOscript.h"

/* ------------------ UpdateTimeLabels ------------------------ */

void UpdateTimeLabels(void){
  float time0;

  time0 = timeoffset;
  if(global_times!=NULL)time0 = timeoffset+global_times[itimes];
  if(current_script_command!=NULL&&IS_LOADRENDER){
    time0 = current_script_command->fval4;
  }

  if(vishmsTimelabel==1){
    int hour, min, sec, sec10;
    char sign[2];

    if(time0<0){
      strcpy(sign, "-");
      time0 = ABS(time0);
    }
    else{
      strcpy(sign, " ");
    }
    hour = time0/3600;
    min = (int)(time0/60.0-60.0*hour);
    sec10 = (int)(10*(time0-60.0*min-3600.0*hour));
    sec = sec10/10;
    sec10 = sec10-10*sec;
    sprintf(timelabel, "  %s%i:%.2i:%.2i.%i", sign, hour, min, sec, sec10);
  }
  else{
    float dt;
    char timeval[30], *timevalptr;

    if(current_script_command!=NULL&&IS_LOADRENDER){
      dt = current_script_command->fval5;
    }
    else{
      if(nglobal_times>1){
        dt = global_times[1]-global_times[0];
      }
      else{
        dt = 0.0;
      }
    }
    if(dt<0.0)dt = -dt;
    timevalptr = Time2TimeLabel(time0, dt, timeval, force_fixedpoint);
    strcpy(timelabel, "");
    if(visFrameTimelabel==1)strcat(timelabel, "Time: ");
    strcat(timelabel, timevalptr);
  }

  {
    int itime_val;

    if(current_script_command!=NULL&&IS_LOADRENDER){
      itime_val = script_itime;
    }
    else{
      itime_val = itimes;
    }

    if(visFrameTimelabel==1){
      sprintf(framelabel, "Frame: %i", itime_val);
    }
    else{
      sprintf(framelabel, "%i", itime_val);
    }
  }

  if(hrrptr!=NULL&&global_times!=NULL&&vis_hrr_label==1){
    float hrr;
    int itime;

    itime = GetInterval(global_times[itimes], timeptr->vals, timeptr->nvals);
    hrr = hrrptr->vals[itime];
    if(hrr<1.0){
      sprintf(hrrlabel,"HRR: %4.1f W",hrr*1000.0);
    }
    else if(hrr>1000.0){
      sprintf(hrrlabel,"HRR: %4.1f MW",hrr/1000.0);
    }
    else{
      sprintf(hrrlabel,"HRR: %4.1f kW",hrr);
    }
  }
  else{
    strcpy(hrrlabel, "");
  }
}

/* ------------------ DrawTimebar ------------------------ */

void DrawTimebar(float xleft, float xright, float ybot, float ytop){
  float xxright;

  if(xright<=xleft)return;
  DISABLE_LIGHTING;

  glLineWidth(linewidth);
  glBegin(GL_LINE_LOOP);
  glColor4fv(timebarcolor);
  glVertex2f(xleft,ybot);
  glVertex2f(xright,ybot);
  glVertex2f(xright,ytop);
  glVertex2f(xleft,ytop);
  glEnd();

  xxright = xright;
  if(current_script_command!=NULL&&IS_LOADRENDER){
    float factor, time_min, time_max, time_now;

    time_min = current_script_command->fval2;
    time_max = current_script_command->fval3;
    if(time_max>time_min){
      time_now = current_script_command->fval4;
      factor = CLAMP((time_now-time_min)/(time_max-time_min), 0.0, 1.0);
      xxright = (1.0-factor)*xleft+factor*xright;
    }
  }
  else{
// draw time bar proportional to time
    float factor=0.0, dtime=0.0;

    if(nglobal_times>1){
      dtime = global_times[nglobal_times-1] - global_times[0];
    }
    if(dtime!=0.0&&nglobal_times>1){
      factor = CLAMP((global_times[itimes] - global_times[0])/dtime, 0.0, 1.0);
    }
    xxright = xleft*(1.0-factor) + xright*factor;
  }
  glBegin(GL_POLYGON);
  glColor4fv(timebarcolor);
  glVertex2f(xleft,ybot);
  glVertex2f(xxright,ybot);
  glVertex2f(xxright,ytop);
  glVertex2f(xleft,ytop);
  glEnd();
}

/* ------------------ DrawSelectColorbar ------------------------ */

void DrawSelectColorbar(void){
  int i;
  colorbardata *cbi;

  if(show_firecolormap==0){
    cbi = colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbarinfo+fire_colorbar_index;
  }

  glPointSize(20.0f);
  glBegin(GL_POINTS);
  for(i=0;i<cbi->nnodes;i++){
    unsigned char *rrgb, r, g, b;

    GetRGB(i+1, &r, &g, &b);
    glColor3ub(r, g, b);

    rrgb=cbi->node_rgb+3*i;
    glVertex3f(rrgb[0]/255.0,rrgb[1]/255.0,rrgb[2]/255.0);
  }
  glEnd();
}

/* ------------------ DrawColorbarPathRGB ------------------------ */

void DrawColorbarPathRGB(void){
  int i;
  colorbardata *cbi;
  int ncolors;

  if(show_firecolormap==0){
    cbi = colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbarinfo+fire_colorbar_index;
  }
  glPointSize(5.0);
  glBegin(GL_POINTS);
  for(i=0;i<255;i++){
    float *rgbi;

    rgbi=cbi->colorbar_rgb+3*i;
    glColor3fv(rgbi);
    glVertex3fv(rgbi);
  }
  glEnd();

  glPointSize(10.0);
  glBegin(GL_POINTS);
  for(i=0;i<cbi->nnodes;i++){
    unsigned char *rrgb;

    rrgb=cbi->node_rgb+3*i;
    glColor3ubv(rrgb);
    glVertex3f(rrgb[0]/255.0,rrgb[1]/255.0,rrgb[2]/255.0);
  }
#define PLEFT2 -0.1
#define PRIGHT2 1.1

  glEnd();

  // draw rgb color axes

  glLineWidth(5.0);
  glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0);
  glVertex3f( PLEFT2,PLEFT2,PLEFT2);
  glVertex3f(PRIGHT2,PLEFT2,PLEFT2);

  glColor3f(0.0,1.0,0.0);
  glVertex3f(PLEFT2, PLEFT2,PLEFT2);
  glVertex3f(PLEFT2,PRIGHT2,PLEFT2);

  glColor3f(0.0,0.0,1.0);
  glVertex3f(PLEFT2,PLEFT2, PLEFT2);
  glVertex3f(PLEFT2,PLEFT2,PRIGHT2);

  glEnd();
  Output3Text(foregroundcolor, PRIGHT2, PLEFT2,  PLEFT2,  "R");
  Output3Text(foregroundcolor, PLEFT2,  PRIGHT2, PLEFT2,  "G");
  Output3Text(foregroundcolor, PLEFT2,  PLEFT2,  PRIGHT2, "B");

  if(colorbarpoint>=0&&colorbarpoint<cbi->nnodes){
    unsigned char *rgbleft;

    rgbleft = cbi->node_rgb+3*colorbarpoint;

    glPointSize(20.0);
    glBegin(GL_POINTS);
    glColor3ubv(rgbleft);
    glVertex3f(rgbleft[0]/255.0,rgbleft[1]/255.0,rgbleft[2]/255.0);
    glEnd();
  }

  {
    float xdenorm, ydenorm, zdenorm;

    glPointSize(10.0);
    glBegin(GL_POINTS);
    for(i=0;i<cbi->nnodes;i++){
      float *rgbi;
      float dzpoint;

      rgbi = cbi->colorbar_rgb+3*cbi->node_index[i];
      dzpoint = (float)cbi->node_index[i]/255.0;
      glColor3fv(rgbi);
      glVertex3f(1.5,0.0,dzpoint);
    }
    glEnd();

    xdenorm = SMV2FDS_X(1.55);
    ydenorm = SMV2FDS_Y(0.0);
    if(fontindex==SCALED_FONT)ScaleFont3D();
    glPushMatrix();
    glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
    glTranslatef(-xbar0,-ybar0,-zbar0);
    int skip = 1;
    if(cbi->nnodes > 16)skip = cbi->nnodes / 16;
    for(i=0;i<cbi->nnodes;i+=skip){
      char cbuff[1024];
      float dzpoint;

      dzpoint = (float)cbi->node_index[i]/255.0;
      zdenorm = SMV2FDS_Z(dzpoint);
      sprintf(cbuff,"%i",(int)cbi->node_index[i]);
      Output3Text(foregroundcolor, xdenorm,ydenorm,zdenorm, cbuff);
    }
    glPopMatrix();
    glLineWidth(5.0);
    if(colorbarpoint>=0&&colorbarpoint<cbi->nnodes){
      float *rgbi;
      float dzpoint;

      glPointSize(20.0);
      glBegin(GL_POINTS);
      rgbi = cbi->colorbar_rgb+3*cbi->node_index[colorbarpoint];
      dzpoint = (float)cbi->node_index[colorbarpoint]/255.0;
      glColor3fv(rgbi);
      glVertex3f(1.5,0.0,dzpoint);
      glEnd();
    }
    if(show_firecolormap!=0){
      char vvlabel[255];
      float vval_min, vval_cutoff, vval_max;

      if(have_fire==HRRPUV_index&&smoke_render_option==RENDER_SLICE){
        vval_min=global_hrrpuv_min;
        vval_cutoff=global_hrrpuv_cutoff;
        vval_max=global_hrrpuv_max;
      }
      else{
        vval_min=global_temp_min;
        vval_cutoff= global_temp_cutoff;
        vval_max= global_temp_max;
      }
      sprintf(vvlabel,"%4.0f",vval_min);
      Output3Text(foregroundcolor, 1.0,0.0,0.0,vvlabel);

      sprintf(vvlabel,"%4.0f",vval_cutoff);
      Output3Text(foregroundcolor, 1.0,0.0,(vval_cutoff-vval_min)/(vval_max-vval_min),vvlabel);

      sprintf(vvlabel,"%4.0f",vval_max);
      Output3Text(foregroundcolor, 1.0,0.0,1.0,vvlabel);
    }

    if(show_firecolormap!=0){
      ncolors=MAXSMOKERGB-1;
    }
    else{
      ncolors=MAXRGB-1;
    }
    glBegin(GL_TRIANGLES);
    for(i=1;i<ncolors;i++){
      float *rgbi;
      float zbot, ztop;
      float black[3] = {0.0, 0.0, 0.0};

      if(show_firecolormap!=0){
        rgbi=rgb_volsmokecolormap+4*i;
      }
      else{
        rgbi=cbi->colorbar_rgb+3*i;
      }
      if(show_Lab_dist_bars == 1 && ncolors == 255 && cbi->dist_ind[i] == 1){
        rgbi = black;
      }
      glColor3fv(rgbi);
      zbot=(float)i/(float)ncolors;
      ztop=(float)(i+1)/(float)ncolors;

      glVertex3f(1.1,0.0,zbot);
      glVertex3f(1.3,0.0,zbot);
      glVertex3f(1.3,0.0,ztop);

      glVertex3f(1.1,0.0,zbot);
      glVertex3f(1.3,0.0,ztop);
      glVertex3f(1.3,0.0,zbot);

      glVertex3f(1.1,0.0,zbot);
      glVertex3f(1.3,0.0,ztop);
      glVertex3f(1.1,0.0,ztop);

      glVertex3f(1.1,0.0,zbot);
      glVertex3f(1.1,0.0,ztop);
      glVertex3f(1.3,0.0,ztop);

      glVertex3f(1.2,-0.1,zbot);
      glVertex3f(1.2, 0.1,zbot);
      glVertex3f(1.2, 0.1,ztop);

      glVertex3f(1.2,-0.1,zbot);
      glVertex3f(1.2, 0.1,ztop);
      glVertex3f(1.2, 0.1,zbot);

      glVertex3f(1.2,-0.1,zbot);
      glVertex3f(1.2, 0.1,ztop);
      glVertex3f(1.2,-0.1,ztop);

      glVertex3f(1.2,-0.1,zbot);
      glVertex3f(1.2,-0.1,ztop);
      glVertex3f(1.2, 0.1,ztop);
    }
    glEnd();
  }
}

/* ------------------ Rgb2Lab ------------------------ */

void Rgb2Lab(unsigned char *rgb_arg, float *lab){
  float frgb_arg[3];

  frgb_arg[0] = (float)rgb_arg[0];
  frgb_arg[1] = (float)rgb_arg[1];
  frgb_arg[2] = (float)rgb_arg[2];
  FRgb2Lab(frgb_arg, lab);
}

/* ------------------ Rgbf2Lab ------------------------ */

void Rgbf2Lab(float *rgbf_arg, float *lab){
  float frgb_arg[3];

  frgb_arg[0] = rgbf_arg[0] * 255.0;
  frgb_arg[1] = rgbf_arg[1] * 255.0;
  frgb_arg[2] = rgbf_arg[2] * 255.0;
  FRgb2Lab(frgb_arg, lab);
}

/* ------------------ Lab2XYZ ------------------------ */

void Lab2XYZ(float *xyz, float *lab){
  xyz[0] = lab[0] / 100.0;
  xyz[1] = (lab[1] + 87.9) / 183.28;
  xyz[2] = (lab[2] + 126.39) / 211.11;
}

/* ------------------ DrawColorbarPathCIELab ------------------------ */

void DrawColorbarPathCIELab(void){
  int i;
  colorbardata *cbi;

  if(show_firecolormap == 0){
    cbi = colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbarinfo + fire_colorbar_index;
  }
  glPointSize(5.0);
  glBegin(GL_POINTS);
  for(i = 0; i < 255; i++){
    float *rgbi, xyz[3];

    rgbi = cbi->colorbar_rgb + 3 * i;
    glColor3fv(rgbi);
    Lab2XYZ(xyz, cbi->colorbar_lab + 3*i);
    glVertex3f(xyz[1], xyz[2], xyz[0]);
  }
  glEnd();

#ifdef _DEBUG
  for(i = 7; i < 255; i += 8){
    float xyz1[3], xyz2[3];
    char label[32];

    sprintf(label, "%.2f", cbi->colorbar_dist[i]-cbi->colorbar_dist[i-7]);
    Lab2XYZ(xyz2, cbi->colorbar_lab + 3 * i);
    Lab2XYZ(xyz1, cbi->colorbar_lab + 3 * (i + 1 - 8));
    xyz1[0] = (xyz1[0] + xyz2[0]) / 2.0;
    xyz1[1] = (xyz1[1] + xyz2[1]) / 2.0;
    xyz1[2] = (xyz1[2] + xyz2[2]) / 2.0;
    Output3Text(foregroundcolor, xyz1[1], xyz1[2], xyz1[0], label);
  }
#endif

  glPointSize(10.0);
  glBegin(GL_POINTS);
  for(i = 0; i < 256; i+=8){
    float *rgbi, xyz[3];

    rgbi = cbi->colorbar_rgb + 3 * i;
    glColor3fv(rgbi);
    Lab2XYZ(xyz, cbi->colorbar_lab + 3*i);
    glVertex3f(xyz[1],xyz[2],xyz[0]);
  }
  glEnd();

  glColor3fv(foregroundcolor);
  glLineWidth(5.0);
  glBegin(GL_LINES);
  glVertex3f(PLEFT2, PLEFT2, PLEFT2);
  glVertex3f(PRIGHT2, PLEFT2, PLEFT2);

  glVertex3f(PLEFT2, PLEFT2, PLEFT2);
  glVertex3f(PLEFT2, PRIGHT2, PLEFT2);

  glVertex3f(PLEFT2, PLEFT2, PLEFT2);
  glVertex3f(PLEFT2, PLEFT2, PRIGHT2);
  glEnd();
  Output3Text(foregroundcolor, PRIGHT2, PLEFT2,  PLEFT2,  "a*");
  Output3Text(foregroundcolor, PLEFT2,  PRIGHT2, PLEFT2,  "b*");
  Output3Text(foregroundcolor, PLEFT2,  PLEFT2,  PRIGHT2, "L*");

  glPointSize(10.0);
  glBegin(GL_POINTS);
  for(i = 0;i < cbi->nnodes;i++){
    float *rgbi;
    float dzpoint;

    rgbi = cbi->colorbar_rgb + 3 * cbi->node_index[i];
    dzpoint = (float)cbi->node_index[i] / 255.0;
    glColor3fv(rgbi);
    glVertex3f(1.5, 0.0, dzpoint);
  }
  glEnd();

  float xdenorm, ydenorm, zdenorm;
  xdenorm = SMV2FDS_X(1.55);
  ydenorm = SMV2FDS_Y(0.0);
  if(fontindex == SCALED_FONT)ScaleFont3D();
  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);
  int skip = 1;
  if(cbi->nnodes > 16)skip = cbi->nnodes / 16;
  for(i = 0;i < cbi->nnodes;i+=skip){
    char cbuff[1024];
    float dzpoint;

    dzpoint = (float)cbi->node_index[i] / 255.0;
    zdenorm = SMV2FDS_Z(dzpoint);
    sprintf(cbuff, "%i", (int)cbi->node_index[i]);
    Output3Text(foregroundcolor, xdenorm, ydenorm, zdenorm, cbuff);
  }
  glPopMatrix();

  int ncolors;
  if(show_firecolormap!=0){
    ncolors=MAXSMOKERGB-1;
  }
  else{
    ncolors=MAXRGB-1;
  }
  glBegin(GL_TRIANGLES);
  for(i=1;i<ncolors;i++){
    float *rgbi;
    float zbot, ztop;
    float black[3] = {0.0,0.0,0.0};

    if(show_firecolormap!=0){
      rgbi=rgb_volsmokecolormap+4*i;
    }
    else{
      rgbi=cbi->colorbar_rgb+3*i;
    }
    if(show_Lab_dist_bars==1&&ncolors == 255&&cbi->dist_ind[i]==1){
      rgbi = black;
    }
    glColor3fv(rgbi);
    zbot=(float)i/(float)ncolors;
    ztop=(float)(i+1)/(float)ncolors;

    glVertex3f(1.1,0.0,zbot);
    glVertex3f(1.3,0.0,zbot);
    glVertex3f(1.3,0.0,ztop);

    glVertex3f(1.1,0.0,zbot);
    glVertex3f(1.3,0.0,ztop);
    glVertex3f(1.3,0.0,zbot);

    glVertex3f(1.1,0.0,zbot);
    glVertex3f(1.3,0.0,ztop);
    glVertex3f(1.1,0.0,ztop);

    glVertex3f(1.1,0.0,zbot);
    glVertex3f(1.1,0.0,ztop);
    glVertex3f(1.3,0.0,ztop);

    glVertex3f(1.2,-0.1,zbot);
    glVertex3f(1.2, 0.1,zbot);
    glVertex3f(1.2, 0.1,ztop);

    glVertex3f(1.2,-0.1,zbot);
    glVertex3f(1.2, 0.1,ztop);
    glVertex3f(1.2, 0.1,zbot);

    glVertex3f(1.2,-0.1,zbot);
    glVertex3f(1.2, 0.1,ztop);
    glVertex3f(1.2,-0.1,ztop);

    glVertex3f(1.2,-0.1,zbot);
    glVertex3f(1.2,-0.1,ztop);
    glVertex3f(1.2, 0.1,ztop);
  }
  glEnd();
}

/* ------------------ GetColorbar ------------------------ */

colorbardata *GetColorbar(char *menu_label){
  int i;

  for(i=0;i<ncolorbars;i++){
    colorbardata *cb;

    cb = colorbarinfo + i;
    if(strcmp(cb->menu_label,menu_label)==0)return cb;
  }
  return NULL;
}

/* ------------------ UpdateCurrentColorbar ------------------------ */
#define FILE_UPDATE 6
void UpdateCurrentColorbar(colorbardata *cb){
  int jj=0,fed_loaded=0;

  current_colorbar = cb;
  if(current_colorbar != NULL&&strcmp(current_colorbar->menu_label, "FED") == 0){
    is_fed_colorbar = 1;
  }
  else{
    is_fed_colorbar = 0;
  }
  for(jj=0;jj<nslice_loaded;jj++){
    slicedata *slicej;
    int j;

    j = slice_loaded_list[jj];
    slicej = sliceinfo + j;
    if(slicej->display==0)continue;
    if(slicej->is_fed==1){
      fed_loaded=1;
      break;
    }
  }
  if(is_fed_colorbar==1&&fed_loaded==1)GLUISliceBoundCB(FILE_UPDATE);
}

/* ------------------ AdjustColorBar ------------------------ */

void AdjustColorBar(colorbardata *cbi){
  int i;

  cbi->node_dist[0] = 0.0;
  for(i = 1;i < cbi->nnodes;i++){
    unsigned char *rgb1_local, *rgb2_local;
    float lab1[3], lab2[3], dist;

    rgb2_local = cbi->node_rgb + 3*i;
    rgb1_local = rgb2_local - 3;
    Rgb2Lab(rgb1_local, lab1);
    Rgb2Lab(rgb2_local, lab2);
    float dx, dy, dz;

    DDIST3(lab1, lab2, dist);
    cbi->node_dist[i] = cbi->node_dist[i - 1] + dist;
  }

  float total_dist;
  int nnodes;

  total_dist = cbi->node_dist[cbi->nnodes - 1];
  nnodes = cbi->node_index[cbi->nnodes - 1];

  if(total_dist > 0.0){
    for(i = 1;i < cbi->nnodes - 1;i++){
      int inode;

      inode = nnodes * (cbi->node_dist[i] / total_dist);
      cbi->node_index[i] = inode;
    }
  }
  cbi->adjusted = 1;
  update_colorbar_dialog = 1;
}

/* ------------------ IsColorbarSplit ------------------------ */

int IsColorbarSplit(colorbardata *cbi){
  int i;

  for(i = 0;i < cbi->nnodes - 1;i++){
    if(cbi->node_index[i] + 1 == cbi->node_index[i + 1])return 1;
    if(cbi->node_index[i]     == cbi->node_index[i + 1])return 1;
  }
  return 0;
}

/* ------------------ AdjustColorBarLab ------------------------ */

void AdjustColorBarLab(colorbardata *cbi){
  if(cbi->can_adjust==0||IsColorbarSplit(cbi)==1)return;
  AdjustColorBar(cbi);
}

/* ------------------ FRgb2Lab ------------------------ */

void FRgb2Lab(float *rgb_arg, float *lab){

  // Convert RGB values to XYZ
  float var_R = rgb_arg[0] / 255.0f;
  float var_G = rgb_arg[1] / 255.0f;
  float var_B = rgb_arg[2] / 255.0f;

  if(var_R > 0.04045f) {
    var_R = pow((var_R + 0.055f) / 1.055f, 2.4f);
  }
  else {
    var_R /= 12.92f;
  }
  if(var_G > 0.04045f) {
    var_G = pow((var_G + 0.055f) / 1.055f, 2.4f);
  }
  else {
    var_G /= 12.92f;
  }
  if(var_B > 0.04045f) {
    var_B = pow((var_B + 0.055f) / 1.055f, 2.4f);
  }
  else {
    var_B /= 12.92f;
  }

  var_R *= 100.0f;
  var_G *= 100.0f;
  var_B *= 100.0f;

  float X = var_R * 0.4124f + var_G * 0.3576f + var_B * 0.1805f;
  float Y = var_R * 0.2126f + var_G * 0.7152f + var_B * 0.0722f;
  float Z = var_R * 0.0193f + var_G * 0.1192f + var_B * 0.9505f;

  // Convert XYZ to CIELAB
  float var_X = X / 95.047f;
  float var_Y = Y / 100.0f;
  float var_Z = Z / 108.883f;

  if(var_X > 0.008856f) {
    var_X = pow(var_X, 1.0f / 3.0f);
  }
  else {
    var_X = (7.787f * var_X) + (16.0f / 116.0f);
  }
  if(var_Y > 0.008856f) {
    var_Y = pow(var_Y, 1.0f / 3.0f);
  }
  else {
    var_Y = (7.787f * var_Y) + (16.0f / 116.0f);
  }
  if(var_Z > 0.008856f) {
    var_Z = pow(var_Z, 1.0f / 3.0f);
  }
  else {
    var_Z = (7.787f * var_Z) + (16.0f / 116.0f);
  }

  lab[0] = (116.0f * var_Y) - 16.0f;
  lab[1] = 500.0f * (var_X - var_Y);
  lab[2] = 200.0f * (var_Y - var_Z);
}

/* ------------------ Rgb2Dist ------------------------ */

void Rgb2Dist(colorbardata *cbi){
  int i;

  float total_dist, *colorbar_dist;
  int jstart, *dist_ind;

  colorbar_dist = cbi->colorbar_dist;
  dist_ind   = cbi->dist_ind;

  colorbar_dist[0]     = 0.0;
  for(i = 1;i < 256;i++){
    float dist_lab, lab2[3], *rgb1f, *rgb2f, lab1[3];
    float dx, dy, dz;

    rgb1f = cbi->colorbar_rgb + 3*(i - 1);
    rgb2f = cbi->colorbar_rgb + 3*i;
    Rgbf2Lab(rgb1f, lab1);
    Rgbf2Lab(rgb2f, lab2);
    DDIST3(lab1, lab2, dist_lab);
    colorbar_dist[i] = colorbar_dist[i - 1] + dist_lab;
  }
  total_dist = colorbar_dist[255];

  dist_ind[0] = 1;
  dist_ind[255] = 1;
  for(i=1;i<255;i++){
    dist_ind[i] = 0;
  }
  jstart = 0;
  for(i = 1;i < 16;i++){
    float val;
    int j;

    val = (float)i* total_dist / 16.0;
    for(j=jstart;j<255;j++){
      if(colorbar_dist[j]<=val&&val<= colorbar_dist[j+1]){
        dist_ind[j] = 1;
        jstart = j;
        break;
      }
    }
  }
}

/* ------------------ Lab2Rgb ------------------------ */

void Lab2Rgb(unsigned char *rgb_arg, float *frgb_arg, float *lab){
  float L, a, b;

  L = lab[0];
  a = lab[1];
  b = lab[2];

  // Convert CIELAB to XYZ
  float var_Y = (L + 16.0f) / 116.0f;
  float var_X = a / 500.0f + var_Y;
  float var_Z = var_Y - b / 200.0f;

  if(pow(var_Y, 3.0f) > 0.008856f) {
    var_Y = pow(var_Y, 3.0f);
  }
  else {
    var_Y = (var_Y - 16.0f / 116.0f) / 7.787f;
  }
  if(pow(var_X, 3.0f) > 0.008856f) {
    var_X = pow(var_X, 3.0f);
  }
  else {
    var_X = (var_X - 16.0f / 116.0f) / 7.787f;
  }
  if(pow(var_Z, 3.0f) > 0.008856f) {
    var_Z = pow(var_Z, 3.0f);
  }
  else {
    var_Z = (var_Z - 16.0f / 116.0f) / 7.787f;
  }

  float X = var_X * 0.95047f;
  float Y = var_Y;
  float Z = var_Z * 1.08883f;

  // Convert XYZ to RGB
  float var_R =  X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
  float var_G = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
  float var_B =  X * 0.0557f - Y * 0.2040f + Z * 1.0570f;

  if(var_R > 0.0031308f) {
    var_R = 1.055f * pow(var_R, 1.0f / 2.4f) - 0.055f;
  }
  else {
    var_R *= 12.92f;
  }
  if(var_G > 0.0031308f) {
    var_G = 1.055f * pow(var_G, 1.0f / 2.4f) - 0.055f;
  }
  else {
    var_G *= 12.92f;
  }
  if(var_B > 0.0031308f) {
    var_B = 1.055f * pow(var_B, 1.0f / 2.4f) - 0.055f;
  }
  else {
    var_B *= 12.92f;
  }

  frgb_arg[0] = CLAMP(var_R * 255.0f, 0.0, 255.0f);
  frgb_arg[1] = CLAMP(var_G * 255.0f, 0.0, 255.0f);
  frgb_arg[2] = CLAMP(var_B * 255.0f, 0.0, 255.0f);
  rgb_arg[0] = (unsigned char)CLAMP(frgb_arg[0] + 0.5, 0, 255);
  rgb_arg[1] = (unsigned char)CLAMP(frgb_arg[1] + 0.5, 0, 255);
  rgb_arg[2] = (unsigned char)CLAMP(frgb_arg[2] + 0.5, 0, 255);
}

// matches following website
// http://colormine.org/convert/rgb-to-lab

/* ------------------ CheckLab ------------------------ */
void CheckLab(void){
  int i, diff;
  int hist[256];
  float sum=0.0;
  float *labxyz;
  unsigned char *labrgb, *lab_check_rgb255;

  for(i = 0;i < 256;i++){
    hist[i] = 0;
  }

  NewMemory((void **)&lab_check_xyz, 3 * 17*17*17 * sizeof(float));
  NewMemory((void **)&lab_check_rgb255, 3 * 17*17*17);
  labxyz = lab_check_xyz;
  labrgb = lab_check_rgb255;
  for(i = 0; i < 256; i++){
    int j;

    printf("i=%i\n", i);
    for(j = 0; j < 256; j++){
      int k;

      for(k = 0; k < 256; k++){
        unsigned char rgbval[3], rgbnew[3];
        float lab[3], lab2[3], dist2, frgb[3];

        rgbval[0] = (unsigned char)k;
        rgbval[1] = (unsigned char)j;
        rgbval[2] = (unsigned char)i;
        Rgb2Lab(rgbval, lab);
        Lab2Rgb(rgbnew, frgb, lab);
        Rgb2Lab(rgbnew, lab2);
        diff = ABS(rgbval[0] - rgbnew[0]);
        diff = MAX(diff, ABS(rgbval[1] - rgbnew[1]));
        diff = MAX(diff, ABS(rgbval[2] - rgbnew[2]));
        dist2 = ABS(lab2[0]-lab[0]);
        dist2 = MAX(dist2, ABS(lab2[1] - lab[1]));
        dist2 = MAX(dist2, ABS(lab2[2] - lab[2]));
        sum += dist2;
        hist[diff]++;
      }
    }
  }
  for(i = 0; i <= 256; i+=16){
    int j;

    for(j = 0; j <= 256; j+=16){
      int k;

      for(k = 0; k<=256; k+=16){
        unsigned char rgbval[3];
        float lab[3];

        rgbval[0] = MIN(( unsigned char )k,255);
        rgbval[1] = MIN(( unsigned char )j,255);
        rgbval[2] = MIN(( unsigned char )i,255);
        Rgb2Lab(rgbval, lab);
        memcpy(labxyz, lab, 3 * sizeof(float));
        memcpy(labrgb, rgbval, 3);
        labxyz += 3;
        labrgb += 3;
      }
    }
  }
  for(i = 0;i < 256;i++){
    printf("%i ", hist[i]);
  }
  printf("\n");
  printf("lab avg diff=%f\n", sum / (float)(256 * 256 * 256));
  FREEMEMORY(lab_check_xyz);
  FREEMEMORY(lab_check_rgb255);
}

/* ------------------ GetColorDist ------------------------ */

void GetColorDist(colorbardata *cbi, int option, float *min, float *max){
  int i;

  for(i = 1; i < 255;i++){
    cbi->colorbar_dist_delta[i - 1] = cbi->colorbar_dist[i] - cbi->colorbar_dist[i - 1];
  }
  *min = cbi->colorbar_dist_delta[0];
  *max = *min;
  for(i = 1; i < 255 - 1; i++){
    *min = MIN(*min, cbi->colorbar_dist_delta[i]);
    *max = MAX(*max, cbi->colorbar_dist_delta[i]);
  }
}

/* ------------------ RemapColorbar ------------------------ */

unsigned char SetAlpha(unsigned char *node_rgb){
  if(
    (node_rgb[0] ==   0 && node_rgb[1] ==   1 && node_rgb[2] ==   2) ||
    (node_rgb[0] == 253 && node_rgb[1] == 254 && node_rgb[2] == 255)
    ){
    return 0;
  }
  return 255;
}

/* ------------------ RemapColorbar ------------------------ */

void RemapColorbar(colorbardata *cbi){
  int i;
  float *colorbar_rgb;
  unsigned char *node_rgb;
  unsigned char *colorbar_alpha;
  float *colorbar_lab;
  int interp;

  interp = cbi->interp;
  CheckMemory;
  colorbar_rgb   = cbi->colorbar_rgb;
  node_rgb       = cbi->node_rgb;
  colorbar_lab   = cbi->colorbar_lab;
  colorbar_alpha = cbi->colorbar_alpha;

  AdjustColorBarLab(cbi);

  for(i=0;i<cbi->node_index[0];i++){
    colorbar_rgb[0+3*i] = node_rgb[0]/255.0;
    colorbar_rgb[1+3*i] = node_rgb[1]/255.0;
    colorbar_rgb[2+3*i] = node_rgb[2]/255.0;
    colorbar_alpha[i]   = SetAlpha(node_rgb);
  }
  for(i=0;i<cbi->nnodes-1;i++){
    int i1,i2,j;
    float lab1[3], lab2[3];

    i1 = cbi->node_index[i];
    i2 = cbi->node_index[i+1];
    if(i2==i1)continue;
    node_rgb = cbi->node_rgb+3*i;
    Rgb2Lab(node_rgb,   lab1);
    Rgb2Lab(node_rgb+3, lab2);
    for(j=i1;j<i2;j++){
      float factor;

      factor = (float)(j-i1)/(float)(i2-i1);
      float *labj;

      labj  = colorbar_lab + 3*j;
      labj[0]=MIX(factor,lab2[0],lab1[0]);
      labj[1]=MIX(factor,lab2[1],lab1[1]);
      labj[2]=MIX(factor,lab2[2],lab1[2]);
      if(interp==INTERP_LAB){
        unsigned char rgb_val[3];
        float frgb[3];

        Lab2Rgb(rgb_val, frgb, labj);
        colorbar_rgb[0+3*j] = frgb[0]/255.0;
        colorbar_rgb[1+3*j] = frgb[1]/255.0;
        colorbar_rgb[2+3*j] = frgb[2]/255.0;
      }
      else{
        colorbar_rgb[0+3*j]=MIX(factor,node_rgb[3],node_rgb[0])/255.0;
        colorbar_rgb[1+3*j]=MIX(factor,node_rgb[4],node_rgb[1])/255.0;
        colorbar_rgb[2+3*j]=MIX(factor,node_rgb[5],node_rgb[2])/255.0;
      }
      colorbar_alpha[j]=SetAlpha(node_rgb);
    }
  }
  node_rgb = cbi->node_rgb+3*(cbi->nnodes-1);
  for(i=cbi->node_index[cbi->nnodes-1];i<256;i++){
    colorbar_rgb[0+3*i] = node_rgb[0]/255.0;
    colorbar_rgb[1+3*i] = node_rgb[1]/255.0;
    colorbar_rgb[2+3*i] = node_rgb[2]/255.0;
    colorbar_alpha[i]   = SetAlpha(node_rgb);
  }
  if(show_extreme_mindata==1){
    colorbar_rgb[0]=rgb_below_min[0];
    colorbar_rgb[1]=rgb_below_min[1];
    colorbar_rgb[2]=rgb_below_min[2];
  }
  if(show_extreme_maxdata==1){
    colorbar_rgb[0+3*255]=rgb_above_max[0];
    colorbar_rgb[1+3*255]=rgb_above_max[1];
    colorbar_rgb[2+3*255]=rgb_above_max[2];
  }
  Rgb2Dist(cbi);
  CheckMemory;
}

/* ------------------ ReadCSVColorbar ------------------------ */

void ReadCSVColorbar(colorbardata *cbptr, char *dir, char *file, char *colorbar_type, int type){
  FILE *stream;
  int i,n=0;
  char fullfile[1024];
  char buffer[255];
  char *field1;
  int have_name = 0;

  if(file == NULL || strlen(file) == 0)return;
  if(dir == NULL  || strlen(dir) == 0)return;
  strcpy(fullfile, dir);
  strcat(fullfile, dirseparator);
  strcat(fullfile, file);
  stream = fopen(fullfile, "r");
  if(stream == NULL)return;
  if(fgets(buffer, 255, stream) == NULL){
    fclose(stream);
    return;
  }

  strcpy(cbptr->menu_label, file);
  cbptr->nodehilight = 0;

  field1 = strtok(buffer, ",");
  if(field1 != NULL && isalpha(field1[0])!=0){
    char *field2;

    have_name = 1;
    field2 = strtok(NULL, ",");
    if(field2 != NULL){
      strcpy(cbptr->menu_label, field2);
      TrimBack(cbptr->menu_label);
    }
  }
  rewind(stream);
  if(have_name == 1){
    fgets(buffer, 255, stream);
    TrimBack(buffer);
  }
  for(;;){
    if(fgets(buffer, 255, stream) == NULL)break;
    n++;
  }
  rewind(stream);
  int *rgbs, *rgbscopy;
  NewMemory((void **)&rgbs, 3 * n * sizeof(int));
  rgbscopy = rgbs;

  if(have_name == 1){
    fgets(buffer, 255, stream);
    TrimBack(buffer);
  }
  for(i=0;i<n;i++){
    char *crgb;

    if(fgets(buffer, 255, stream) == NULL)break;
    TrimBack(buffer);
    crgb = strtok(buffer, ",");
    sscanf(crgb, "%i", rgbscopy);
    crgb = strtok(NULL, ",");
    sscanf(crgb, "%i", rgbscopy+1);
    crgb = strtok(NULL, ",");
    sscanf(crgb, "%i", rgbscopy+2);
    cbptr->node_rgb[3 * i + 0] = (unsigned char)CLAMP(rgbscopy[0], 0, 255);
    cbptr->node_rgb[3 * i + 1] = (unsigned char)CLAMP(rgbscopy[1], 0, 255);
    cbptr->node_rgb[3 * i + 2] = (unsigned char)CLAMP(rgbscopy[2], 0, 255);
    cbptr->node_index[i] = i;
    strcpy(cbptr->colorbar_type, colorbar_type);
    cbptr->type = type;
    rgbscopy += 3;
  }
  cbptr->nnodes = n;
  fclose(stream);
}

/* ------------------ InitDefaultColorbars ------------------------ */

void UpdateColorbarOrig(void){
  int i;

  for(i = 0;i < ncolorbars;i++){
    colorbardata *cbi;

    cbi = colorbarinfo + i;
    cbi->nnodes_orig = cbi->nnodes;
    memcpy(cbi->node_index_orig, cbi->node_index, cbi->nnodes*sizeof(int));
    memcpy(cbi->node_rgb_orig,   cbi->node_rgb,   cbi->nnodes*sizeof(int));
  }
}

/* ------------------ RevertColorbar ------------------------ */

void RevertColorBar(colorbardata *cbi){
  cbi->nnodes = cbi->nnodes_orig;
  memcpy(cbi->node_index, cbi->node_index_orig, cbi->nnodes*sizeof(int));
  memcpy(cbi->node_rgb,   cbi->node_rgb_orig,   cbi->nnodes*sizeof(int));
}

/* ------------------ CompareColorbars ------------------------ */

int CompareColorbars(const void *arg1, const void *arg2){
  colorbardata *cbi, *cbj;
  int *i, *j;

  i = (int *)arg1;
  j = (int *)arg2;
  cbi = colorbarinfo + *i;
  cbj = colorbarinfo + *j;
  if(cbi->type<cbj->type)return -1;
  if(cbi->type>cbj->type)return 1;
  return STRCMP(cbi->menu_label, cbj->menu_label);
}

/* ------------------ SortColorBars ------------------------ */

void SortColorBars(void){
  int i;
  char label_bound[255], label_edit[255];
  char toggle_label1[255], toggle_label2[255];

  strcpy(toggle_label1, "");
  if(index_colorbar1 >= 0){
    colorbardata *cbt1;
    cbt1 = colorbarinfo + index_colorbar1;
    strcpy(toggle_label1,cbt1->menu_label);
  }
  strcpy(toggle_label2, "");
  if(index_colorbar2 >= 0){
    colorbardata *cbt2;

    cbt2 = colorbarinfo + index_colorbar2;
    strcpy(toggle_label2, cbt2->menu_label);
  }
  strcpy(label_edit, "");
  if(colorbartype >= 0){
    colorbardata *cbt1;
    cbt1 = colorbarinfo + colorbartype;
    strcpy(label_edit, cbt1->menu_label);
  }
  strcpy(label_bound, "");
  if(colorbartype >= 0){
    colorbardata *cbt1;
    cbt1 = colorbarinfo + colorbartype;
    strcpy(label_bound, cbt1->menu_label);
  }

  FREEMEMORY(colorbar_list_sorted);
  NewMemory((void **)&colorbar_list_sorted, ncolorbars*sizeof(int));
  FREEMEMORY(colorbar_list_inverse);
  NewMemory((void **)&colorbar_list_inverse, ncolorbars*sizeof(int));
  for(i=0; i<ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbarinfo + i;
    cbi->type = CB_OTHER;
    if(strcmp(cbi->colorbar_type, "rainbow")==0)cbi->type      = CB_RAINBOW;
    if(strcmp(cbi->colorbar_type, "linear")==0)cbi->type       = CB_LINEAR;
    if(strcmp(cbi->colorbar_type, "divergent")==0)cbi->type    = CB_DIVERGENT;
    if(strcmp(cbi->colorbar_type, "circular")==0)cbi->type     = CB_CIRCULAR;
    if(strcmp(cbi->colorbar_type, "deprecated")==0)cbi->type   = CB_DEPRECATED;
    if(strcmp(cbi->colorbar_type, "original") == 0)cbi->type   = CB_ORIGINAL;
    if(strcmp(cbi->colorbar_type, "user defined")==0)cbi->type = CB_USER;
    colorbar_list_sorted[i] = i;
  }
  qsort((colorbardata *)colorbar_list_sorted, (size_t)ncolorbars, sizeof(int), CompareColorbars);
  for(i=0; i<ncolorbars; i++){
    colorbar_list_inverse[colorbar_list_sorted[i]] = i;
  }

  colorbardata *cb;

  bw_colorbar_index = -1;
  cb = GetColorbar("black->white");
  if(cb != NULL)bw_colorbar_index = cb - colorbarinfo;

  cb = GetColorbar("fire");
  fire_colorbar_index=cb-colorbarinfo;
  fire_colorbar=cb;

  cb = GetColorbar("fire line (level set)");
  levelset_colorbar=cb-colorbarinfo;

  split_colorbar_index = -1;
  cb = GetColorbar("split");
  split_colorbar=cb;
  if(cb != NULL)split_colorbar_index = cb - colorbarinfo;

  cb = GetColorbar("CO2");
  co2_colorbar_index = cb - colorbarinfo;

  colorbartype       = colorbartype_default;
  iso_colorbar_index = colorbartype_default;
  cb = NULL;
  if(strlen(toggle_label1)>0)cb = GetColorbar(toggle_label1);
  if(cb!=NULL)index_colorbar1 = cb - colorbarinfo;

  cb = NULL;
  if(strlen(toggle_label2) > 0)cb = GetColorbar(toggle_label2);
  if(cb != NULL)index_colorbar2 = cb - colorbarinfo;
  cb = NULL;
  if(strlen(label_edit) > 0)cb = GetColorbar(label_edit);
  if(cb != NULL)colorbartype = cb - colorbarinfo;

  cb = NULL;
  if(strlen(label_bound) > 0)cb = GetColorbar(label_bound);
  if(cb != NULL)colorbartype= cb - colorbarinfo;
}

/* ------------------ UpdateColorbarDialogs ------------------------ */

void UpdateColorbarDialogs(void){
  SortColorBars();
  GLUIUpdateColorbarListEdit(1, CB_DELETE);
  GLUIUpdateColorbarListBound(1);
  GLUIUpdateColorbarListEdit(2, CB_DELETE);
  GLUIUpdateColorbarListEdit(3, CB_DELETE);
  GLUIUpdateColorbarListBound(2);
  GLUIUpdateColorbarListBound(3);
  GLUIUpdateColorbarBound();
  GLUIUpdateColorbarEdit();
  GLUIColorbarCB(COLORBAR_LIST);
}

/* ------------------ AddColorbar ------------------------ */

int AddColorbar(int icolorbar){
  colorbardata *cb_to, *cb_from;
  char cb_label[255];

  ncolorbars++;
  CheckMemory;
  ResizeMemory((void **)&colorbarinfo, ncolorbars * sizeof(colorbardata));
  UpdateCurrentColorbar(colorbarinfo + colorbartype);

  cb_from = colorbarinfo + icolorbar;
  CheckMemory;

  // new colorbar

  cb_to = colorbarinfo + ncolorbars - 1;

  memcpy(cb_to, cb_from, sizeof(colorbardata));
  strcpy(cb_to->menu_label, cb_from->menu_label);
  strcat(cb_to->menu_label, "_copy");
  strcpy(cb_label, cb_to->menu_label);
  strcpy(cb_to->colorbar_type, "user defined");
  cb_to->interp = INTERP_LAB;
  cb_to->can_adjust = 1;
  RemapColorbar(cb_to);
  UpdateColorbarDialogs();

  colorbardata *cbnew;

  cbnew = GetColorbar(cb_label);
  if(cbnew != NULL){
    colorbartype = cbnew - colorbarinfo;
  }
  GLUISetColorbarListEdit(colorbartype);
  GLUISetColorbarListBound(colorbartype);
  return colorbartype;
}

/* ------------------ InitDefaultColorbars ------------------------ */

void InitDefaultColorbars(int nini){
  int i;
  colorbardata *cbi;
  int ncolorbars_dirlist;

  ndefaultcolorbars = 0;

  filelistdata *linear_filelist=NULL, *circular_filelist=NULL, *rainbow_filelist=NULL, *divergent_filelist = NULL;
  filelistdata *user_filelist = NULL, *colorbars_dirlist = NULL;

  ncolorbars_dirlist  = GetFileListSize(colorbars_dir,           "*",     DIR_MODE);
  nuser_filelist      = GetFileListSize(colorbars_user_dir,      "*.csv", FILE_MODE);
  nlinear_filelist    = GetFileListSize(colorbars_linear_dir,    "*.csv", FILE_MODE);
  ncircular_filelist  = GetFileListSize(colorbars_circular_dir,  "*.csv", FILE_MODE);
  nrainbow_filelist   = GetFileListSize(colorbars_rainbow_dir,   "*.csv", FILE_MODE);
  ndivergent_filelist = GetFileListSize(colorbars_divergent_dir, "*.csv", FILE_MODE);

  MakeFileList(colorbars_dir,           "*",     ncolorbars_dirlist,  NO, &colorbars_dirlist,  DIR_MODE);
  MakeFileList(colorbars_user_dir,      "*.csv", nuser_filelist,      NO, &user_filelist,      FILE_MODE);
  MakeFileList(colorbars_linear_dir,    "*.csv", nlinear_filelist,    NO, &linear_filelist,    FILE_MODE);
  MakeFileList(colorbars_circular_dir,  "*.csv", ncircular_filelist,  NO, &circular_filelist,  FILE_MODE);
  MakeFileList(colorbars_rainbow_dir,   "*.csv", nrainbow_filelist,   NO, &rainbow_filelist,   FILE_MODE);
  MakeFileList(colorbars_divergent_dir, "*.csv", ndivergent_filelist, NO, &divergent_filelist, FILE_MODE);

  ndefaultcolorbars+=nlinear_filelist + ncircular_filelist + nrainbow_filelist + ndivergent_filelist + nuser_filelist;

  ndefaultcolorbars+=19;

  FREEMEMORY(colorbarinfo);
  ncolorbars=ndefaultcolorbars;
  NewMemory((void **)&colorbarinfo,(ncolorbars+nini)*sizeof(colorbardata));
  NewMemory(( void ** )&colorbarcopyinfo, (ncolorbars + nini) * sizeof(colorbardata));
  UpdateCurrentColorbar(colorbarinfo + colorbartype);

  for(i=0;i<ncolorbars;i++){
    cbi = colorbarinfo + i;
    cbi->can_adjust = 1;
    cbi->adjusted   = 0;
  }
  update_colorbar_dialog = 0;

  cbi = colorbarinfo;

  // rainbow colorbar

  strcpy(cbi->menu_label,"Rainbow");
  cbi->nnodes=5;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=0;
  cbi->node_rgb[2]=255;

  cbi->node_index[1]=64;
  cbi->node_rgb[3]=0;
  cbi->node_rgb[4]=192;
  cbi->node_rgb[5]=192;

  cbi->node_index[2]=128;
  cbi->node_rgb[6]=0;
  cbi->node_rgb[7]=255;
  cbi->node_rgb[8]=0;

  cbi->node_index[3]=192;
  cbi->node_rgb[9]=192;
  cbi->node_rgb[10]=192;
  cbi->node_rgb[11]=0;

  cbi->node_index[4]=255;
  cbi->node_rgb[12]=255;
  cbi->node_rgb[13]=0;
  cbi->node_rgb[14]=0;
  strcpy(cbi->colorbar_type, "rainbow");
  cbi++;

  // original rainbow colorbar

  strcpy(cbi->menu_label, "Rainbow_orig");
  cbi->nnodes=5;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=0;
  cbi->node_rgb[2]=255;

  cbi->node_index[1]=64;
  cbi->node_rgb[3]=0;
  cbi->node_rgb[4]=255;
  cbi->node_rgb[5]=255;

  cbi->node_index[2]=128;
  cbi->node_rgb[6]=0;
  cbi->node_rgb[7]=255;
  cbi->node_rgb[8]=0;

  cbi->node_index[3]=192;
  cbi->node_rgb[9]=255;
  cbi->node_rgb[10]=255;
  cbi->node_rgb[11]=0;

  cbi->node_index[4]=255;
  cbi->node_rgb[12]=255;
  cbi->node_rgb[13]=0;
  cbi->node_rgb[14]=0;
  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // Rainbow 2 colorbar

  strcpy(cbi->menu_label,"Rainbow 2");
  cbi->nnodes=12;
  cbi->nodehilight=0;
 
  cbi->node_index[0]=0;	
  cbi->node_rgb[0]=4;	
  cbi->node_rgb[1]=0;	
  cbi->node_rgb[2]=108;
  
  cbi->node_index[1]=20;	
  cbi->node_rgb[3]=6;	
  cbi->node_rgb[4]=3;	
  cbi->node_rgb[5]=167;
  
  cbi->node_index[2]=60;	
  cbi->node_rgb[6]=24;	
  cbi->node_rgb[7]=69;	
  cbi->node_rgb[8]=240;
  
  cbi->node_index[3]=70;	
  cbi->node_rgb[9]=31;	
  cbi->node_rgb[10]=98;	
  cbi->node_rgb[11]=214;
  
  cbi->node_index[4]=80;	
  cbi->node_rgb[12]=5;	
  cbi->node_rgb[13]=125;	
  cbi->node_rgb[14]=170;
  
  cbi->node_index[5]=96;	
  cbi->node_rgb[15]=48;	
  cbi->node_rgb[16]=155;	
  cbi->node_rgb[17]=80;
  
  cbi->node_index[6]=112;	
  cbi->node_rgb[18]=82;	
  cbi->node_rgb[19]=177;	
  cbi->node_rgb[20]=8;
  
  cbi->node_index[7]=163;	
  cbi->node_rgb[21]=240;	
  cbi->node_rgb[22]=222;	
  cbi->node_rgb[23]=3;
  
  cbi->node_index[8]=170;	
  cbi->node_rgb[24]=249;	
  cbi->node_rgb[25]=214;	
  cbi->node_rgb[26]=7;
  
  cbi->node_index[9]=200;	
  cbi->node_rgb[27]=252;	
  cbi->node_rgb[28]=152;	
  cbi->node_rgb[29]=22;
  
  cbi->node_index[10]=230;	
  cbi->node_rgb[30]=254;	
  cbi->node_rgb[31]=67;	
  cbi->node_rgb[32]=13;
  
  cbi->node_index[11]=255;	
  cbi->node_rgb[33]=215;	
  cbi->node_rgb[34]=5;	
  cbi->node_rgb[35]=13;
  strcpy(cbi->colorbar_type, "deprecated");
  cbi++;

  // yellow/red

  strcpy(cbi->menu_label,"yellow->red");
  cbi->nnodes=2;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=255;
  cbi->node_rgb[1]=255;
  cbi->node_rgb[2]=0;

  cbi->node_index[1]=255;
  cbi->node_rgb[3]=255;
  cbi->node_rgb[4]=0;
  cbi->node_rgb[5]=0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // blue/green/red

  strcpy(cbi->menu_label,"blue->green->red");
  cbi->nnodes=3;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=0;
  cbi->node_rgb[2]=255;

  cbi->node_index[1]=128;
  cbi->node_rgb[3]=0;
  cbi->node_rgb[4]=255;
  cbi->node_rgb[5]=0;

  cbi->node_index[2]=255;
  cbi->node_rgb[6]=255;
  cbi->node_rgb[7]=0;
  cbi->node_rgb[8]=0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // blue/yellow/white

  strcpy(cbi->menu_label, "blue->yellow->white");
  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0]  =   0;
  cbi->node_rgb[0]    =   0;
  cbi->node_rgb[1]    = 151;
  cbi->node_rgb[2]    = 255;

  cbi->node_index[1]  = 113;
  cbi->node_rgb[3]    = 255;
  cbi->node_rgb[4]    =   0;
  cbi->node_rgb[5]    =   0;

  cbi->node_index[2]  = 212;
  cbi->node_rgb[6]    = 255;
  cbi->node_rgb[7]    = 255;
  cbi->node_rgb[8]    =   0;

  cbi->node_index[3]  = 255;
  cbi->node_rgb[9]    = 255;
  cbi->node_rgb[10]   = 255;
  cbi->node_rgb[11]   = 255;
  strcpy(cbi->colorbar_type, "deprecated");
  cbi++;

  // blue->red split

  strcpy(cbi->menu_label,"blue->red split");
  cbi->nnodes=4;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=0;
  cbi->node_rgb[2]=255;

  cbi->node_index[1]=127;
  cbi->node_rgb[3]=0;
  cbi->node_rgb[4]=255;
  cbi->node_rgb[5]=255;

  cbi->node_index[2]=128;
  cbi->node_rgb[6]=255;
  cbi->node_rgb[7]=255;
  cbi->node_rgb[8]=0;

  cbi->node_index[3]=255;
  cbi->node_rgb[9]=255;
  cbi->node_rgb[10]=0;
  cbi->node_rgb[11]=0;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "divergent");
  cbi++;

  // black->white

  strcpy(cbi->menu_label,"black->white");

  cbi->nnodes=2;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=0;
  cbi->node_rgb[2]=0;

  cbi->node_index[1]=255;
  cbi->node_rgb[3] =255;
  cbi->node_rgb[4]=255;
  cbi->node_rgb[5]=255;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // FED

  strcpy(cbi->menu_label,"FED");

  cbi->nnodes=6;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=96;
  cbi->node_rgb[1]=96;
  cbi->node_rgb[2]=255;

  cbi->node_index[1]=26; // 0.295276,0.307087
  cbi->node_rgb[3]=96;
  cbi->node_rgb[4]=96;
  cbi->node_rgb[5]=255;

  cbi->node_index[2]=26;
  cbi->node_rgb[6]=255;
  cbi->node_rgb[7]=255;
  cbi->node_rgb[8]=0;

  cbi->node_index[3]=85; // 0.992126,1.003937
  cbi->node_rgb[9]=255;
  cbi->node_rgb[10]=255;
  cbi->node_rgb[11]=0;

  cbi->node_index[4]=85;
  cbi->node_rgb[12]=255;
  cbi->node_rgb[13]=155;
  cbi->node_rgb[14]=0;

  cbi->node_index[5]=255;
  cbi->node_rgb[15]=255;
  cbi->node_rgb[16]=155;
  cbi->node_rgb[17]=0;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // fire (original)

  strcpy(cbi->menu_label,"fire");

  cbi->nnodes=4;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=0;
  cbi->node_rgb[2]=0;

  cbi->node_index[1]=127;
  cbi->node_rgb[3]=0;
  cbi->node_rgb[4]=0;
  cbi->node_rgb[5]=0;

  cbi->node_index[2]=128;
  cbi->node_rgb[6]=255;
  cbi->node_rgb[7]=128;
  cbi->node_rgb[8]=0;

  cbi->node_index[3]=255;
  cbi->node_rgb[9]=255;
  cbi->node_rgb[10]=128;
  cbi->node_rgb[11]=0;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // fire 2

  strcpy(cbi->menu_label,"fire 2");

  cbi->nnodes=10;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=0;
  cbi->node_rgb[2]=0;

  cbi->node_index[1]=127;
  cbi->node_rgb[3]=38;
  cbi->node_rgb[4]=0;
  cbi->node_rgb[5]=0;

  cbi->node_index[2]=128;
  cbi->node_rgb[6]=219;
  cbi->node_rgb[7]=68;
  cbi->node_rgb[8]=21;

  cbi->node_index[3]=160;
  cbi->node_rgb[9]=255;
  cbi->node_rgb[10]=125;
  cbi->node_rgb[11]=36;

  cbi->node_index[4]=183;
  cbi->node_rgb[12]=255;
  cbi->node_rgb[13]=157;
  cbi->node_rgb[14]=52;

  cbi->node_index[5]=198;
  cbi->node_rgb[15]=255;
  cbi->node_rgb[16]=170;
  cbi->node_rgb[17]=63;

  cbi->node_index[6]=214;
  cbi->node_rgb[18]=255;
  cbi->node_rgb[19]=198;
  cbi->node_rgb[20]=93;

  cbi->node_index[7]=229;
  cbi->node_rgb[21]=255;
  cbi->node_rgb[22]=208;
  cbi->node_rgb[23]=109;

  cbi->node_index[8]=244;
  cbi->node_rgb[24]=255;
  cbi->node_rgb[25]=234;
  cbi->node_rgb[26]=161;

  cbi->node_index[9]=255;
  cbi->node_rgb[27]=255;
  cbi->node_rgb[28]=255;
  cbi->node_rgb[29]=238;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // fire 3

  strcpy(cbi->menu_label, "fire 3");

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 108;
  cbi->node_rgb[3] = 255;
  cbi->node_rgb[4] = 127;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 156;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 255;
  cbi->node_rgb[9] = 255;
  cbi->node_rgb[10] = 255;
  cbi->node_rgb[11] = 255;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // cool

  strcpy(cbi->menu_label, "cool");

  cbi->nnodes = 7;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 90;
  cbi->node_rgb[3] = 64;
  cbi->node_rgb[4] = 64;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 110;
  cbi->node_rgb[6] = 155;
  cbi->node_rgb[7] = 35;
  cbi->node_rgb[8] = 33;

  cbi->node_index[3] = 120;
  cbi->node_rgb[9] = 108;
  cbi->node_rgb[10] = 19;
  cbi->node_rgb[11] = 43;

  cbi->node_index[4] = 130;
  cbi->node_rgb[12] = 208;
  cbi->node_rgb[13] = 93;
  cbi->node_rgb[14] = 40;

  cbi->node_index[5] = 160;
  cbi->node_rgb[15] = 255;
  cbi->node_rgb[16] = 178;
  cbi->node_rgb[17] = 0;

  cbi->node_index[6] = 255;
  cbi->node_rgb[18] = 255;
  cbi->node_rgb[19] = 255;
  cbi->node_rgb[20] = 255;
  strcpy(cbi->colorbar_type, "deprecated");
  cbi++;

  // fire line (level set)

  strcpy(cbi->menu_label,"fire line (level set)");

  cbi->nnodes=6;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=1;
  cbi->node_rgb[2]=2;

  cbi->node_index[1]=120;
  cbi->node_rgb[3]=0;
  cbi->node_rgb[4]=1;
  cbi->node_rgb[5]=2;

  cbi->node_index[2]=120;
  cbi->node_rgb[6]=255;
  cbi->node_rgb[7]=0;
  cbi->node_rgb[8]=0;

  cbi->node_index[3]=136;
  cbi->node_rgb[9]=255;
  cbi->node_rgb[10]=0;
  cbi->node_rgb[11]=0;

  cbi->node_index[4]=136;
  cbi->node_rgb[12]=64;
  cbi->node_rgb[13]=64;
  cbi->node_rgb[14]=64;

  cbi->node_index[5]=255;
  cbi->node_rgb[15]=64;
  cbi->node_rgb[16]=64;
  cbi->node_rgb[17]=64;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // fire line (wall thickness)

  wallthickness_colorbar=cbi-colorbarinfo;
  strcpy(cbi->menu_label,"fire line (wall thickness)");

  cbi->nnodes=4;
  cbi->nodehilight=0;

  cbi->node_index[0]=0;
  cbi->node_rgb[0]=0;
  cbi->node_rgb[1]=0;
  cbi->node_rgb[2]=0;

  cbi->node_index[1]=32;
  cbi->node_rgb[3]=0;
  cbi->node_rgb[4]=0;
  cbi->node_rgb[5]=0;

  cbi->node_index[2]=32;
  cbi->node_rgb[6]=253;
  cbi->node_rgb[7]=254;
  cbi->node_rgb[8]=255;

  cbi->node_index[3]=255;
  cbi->node_rgb[9]=253;
  cbi->node_rgb[10]=254;
  cbi->node_rgb[11]=255;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // split

  strcpy(cbi->menu_label, "split");

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_index[1] = 127;
  cbi->node_index[2] = 128;
  cbi->node_index[3] = 255;
  for(i = 0; i < 12; i++){
    cbi->node_rgb[i] = colorsplit[i];
  }

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // Methanol

  strcpy(cbi->menu_label, "Methanol");

  cbi->nnodes = 4;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 9;
  cbi->node_rgb[1] = 190;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 192;
  cbi->node_rgb[3] = 9;
  cbi->node_rgb[4] = 190;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 200;
  cbi->node_rgb[6] = 9;
  cbi->node_rgb[7] = 190;
  cbi->node_rgb[8] = 255;

  cbi->node_index[3] = 255;
  cbi->node_rgb[9] = 9;
  cbi->node_rgb[10] = 190;
  cbi->node_rgb[11] = 255;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // Propane

  strcpy(cbi->menu_label, "Propane");

  cbi->nnodes = 5;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 0;

  cbi->node_index[1] = 140;
  cbi->node_rgb[3] = 235;
  cbi->node_rgb[4] = 120;
  cbi->node_rgb[5] = 0;

  cbi->node_index[2] = 160;
  cbi->node_rgb[6] = 250;
  cbi->node_rgb[7] = 180;
  cbi->node_rgb[8] = 0;

  cbi->node_index[3] = 190;
  cbi->node_rgb[9] = 252;
  cbi->node_rgb[10] = 248;
  cbi->node_rgb[11] = 70;

  cbi->node_index[4] = 255;
  cbi->node_rgb[12] = 255;
  cbi->node_rgb[13] = 255;
  cbi->node_rgb[14] = 255;

  cbi->can_adjust = 0;
  strcpy(cbi->colorbar_type, "original");
  cbi++;

  // CO2

  strcpy(cbi->menu_label, "CO2");

  cbi->nnodes = 3;
  cbi->nodehilight = 0;

  cbi->node_index[0] = 0;
  cbi->node_rgb[0] = 0;
  cbi->node_rgb[1] = 0;
  cbi->node_rgb[2] = 255;

  cbi->node_index[1] = 192;
  cbi->node_rgb[3] = 0;
  cbi->node_rgb[4] = 0;
  cbi->node_rgb[5] = 255;

  cbi->node_index[2] = 255;
  cbi->node_rgb[6] = 255;
  cbi->node_rgb[7] = 255;
  cbi->node_rgb[8] = 255;
  strcpy(cbi->colorbar_type, "original");
  cbi->can_adjust = 0;
  cbi++;

  for(i = 0;i < nlinear_filelist;i++){
    ReadCSVColorbar(cbi, colorbars_linear_dir,  linear_filelist[i].file,      "linear",    CB_LINEAR);
    cbi->can_adjust = 1;
    cbi++;
  }
  for(i = 0;i < ncircular_filelist;i++){
    ReadCSVColorbar(cbi, colorbars_circular_dir,  circular_filelist[i].file,  "circular",  CB_CIRCULAR);
    cbi->can_adjust = 1;
    cbi++;
  }
  for(i = 0;i < nrainbow_filelist;i++){
    ReadCSVColorbar(cbi, colorbars_rainbow_dir,  rainbow_filelist[i].file,    "rainbow",   CB_RAINBOW);
    cbi->can_adjust = 1;
    cbi++;
  }
  for(i = 0;i < ndivergent_filelist;i++){
    ReadCSVColorbar(cbi, colorbars_divergent_dir, divergent_filelist[i].file, "divergent", CB_DIVERGENT);
    cbi->can_adjust = 1;
    cbi++;
  }
  for(i = 0;i < nuser_filelist;i++){
    ReadCSVColorbar(cbi, colorbars_user_dir, user_filelist[i].file,           "user defined",      CB_USER);
    cbi++;
  }

  // construct colormaps from color node info

  for(i=0;i<ndefaultcolorbars;i++){
    cbi = colorbarinfo + i;

    if(cbi->can_adjust==1){
      cbi->interp = INTERP_LAB;
    }
    else{
      cbi->interp = INTERP_RGB;
    }
    RemapColorbar(cbi);
    memcpy(cbi->node_rgb_orig, cbi->node_rgb, 3 * cbi->nnodes * sizeof(unsigned char));
  }
  UpdateColorbarDialogs();

  for(i = 0;i < ncolorbars;i++){
    cbi = colorbarinfo + i;
    cbi->interp = INTERP_LAB;
    if(cbi->can_adjust==1){
      AdjustColorBar(cbi);
    }
  }
  memcpy(colorbarcopyinfo, colorbarinfo, ncolorbars * sizeof(colorbardata));
}

/* ------------------ DrawHorizontalColorbarReg ------------------------ */

void DrawHorizontalColorbarReg(void){
  int i;

  glBegin(GL_QUADS);
  for(i = 0; i < nrgb_full - 1; i++){
    float *rgb_cb, *rgb_cb2;
    float xx, xx2;
    int cbd;
    int i3;

    rgb_cb = rgb_full[i];

    cbd = hcolorbar_down_pos;

    xx  = MIX2(i,     255, hcolorbar_right_pos, hcolorbar_left_pos);
    xx2 = MIX2(i + 1, 255, hcolorbar_right_pos, hcolorbar_left_pos);
    i3 = i + 1;
    if(i == nrgb_full - 2)i3 = i;
    rgb_cb2 = rgb_full[i3];

    glColor4fv(rgb_cb2);
    glVertex2f(xx2, cbd);
    glVertex2f(xx2, hcolorbar_top_pos);

    glColor4fv(rgb_cb);
    glVertex2f(xx, hcolorbar_top_pos);
    glVertex2f(xx, cbd);
  }
  glEnd();
}

/* ------------------ DrawVerticalColorbarReg ------------------------ */

void DrawVerticalColorbarReg(void){
  int i;

  glBegin(GL_QUADS);
  for(i = 0; i < nrgb_full - 1; i++){
    float *rgb_cb, *rgb_cb2;
    float yy, yy2;
    int i3;

    rgb_cb = rgb_full[i];

    yy = MIX2(i, 255, vcolorbar_top_pos, vcolorbar_down_pos);
    yy2 = MIX2(i + 1, 255, vcolorbar_top_pos, vcolorbar_down_pos);
    i3 = i + 1;
    if(i == nrgb_full - 2)i3 = i;
    rgb_cb2 = rgb_full[i3];

    if(rgb_cb[3]>0.0 && rgb_cb2[3]>0.0){
      glColor4fv(rgb_cb);
      glVertex2f(vcolorbar_left_pos, yy);
      glVertex2f(vcolorbar_right_pos, yy);

      glColor4fv(rgb_cb2);
      glVertex2f(vcolorbar_right_pos, yy2);
      glVertex2f(vcolorbar_left_pos, yy2);
    }
  }
  glEnd();
}

#define COLORBAR_PART     0
#define COLORBAR_SLICE    1
#define COLORBAR_ISO      2
#define COLORBAR_BOUNDARY 3
#define COLORBAR_PLOT3D   4
#define COLORBAR_ZONE     5
#define COLORBAR_HVACDUCT 6
#define COLORBAR_HVACNODE 7

/* ------------------ UpdateShowSliceColorbar ------------------------ */

void UpdateShowColorbar(int *showcfast_arg, int *show_slice_colorbar_arg, 
  int *show_hvacduct_colorbar_arg, int *show_hvacnode_colorbar_arg){
  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;

  if(hvacductvar_index >= 0)*show_hvacduct_colorbar_arg = 1;
  if(hvacnodevar_index >= 0)*show_hvacnode_colorbar_arg = 1;
  if(showzone==1&&zonecolortype==ZONETEMP_COLOR)showcfast_local = 1;
  if(showslice==1||(showcfast_local==0&&showvslice==1&&vslicecolorbarflag==1))show_slice_colorbar_local = 1;
  if(show_slice_colorbar_local==1&&showcfast_local==1&&strcmp(slicebounds[slicefile_labelindex].label->shortlabel, "TEMP")==0)show_slice_colorbar_local=0;
  *showcfast_arg = showcfast_local;
  *show_slice_colorbar_arg = show_slice_colorbar_local;
}

/* ------------------ CountColorbars ------------------------ */

int CountColorbars(void){
  int count = 0;
  int i;
  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;
  int show_hvacduct_colorbar_local = 0;
  int show_hvacnode_colorbar_local = 0;

  UpdateShowColorbar(&showcfast_local, &show_slice_colorbar_local, 
    &show_hvacduct_colorbar_local, &show_hvacnode_colorbar_local);

  for(i=0;i< N_COLORBARS;i++){
    hcolorbar_vis[i]=-1;
  }
  if(show_hvacduct_colorbar_local == 1){
    hcolorbar_vis[COLORBAR_HVACDUCT] = count + 2;
    count++;
  }
  if(show_hvacnode_colorbar_local == 1){
    hcolorbar_vis[COLORBAR_HVACNODE] = count + 2;
    count++;
  }
  if(showsmoke==1){
    hcolorbar_vis[COLORBAR_PART]=count+2;
    count++;
  }
  if(show_slice_colorbar_local==1){
    hcolorbar_vis[COLORBAR_SLICE]=count+2;
    count++;
  }
  if(showiso_colorbar == 1){
    hcolorbar_vis[COLORBAR_ISO]=count+2;
    count++;
  }
  if(showpatch == 1 && wall_cell_color_flag == 0){
    hcolorbar_vis[COLORBAR_BOUNDARY]=count+2;
    count++;
  }
  if(showplot3d == 1){
    hcolorbar_vis[COLORBAR_PLOT3D]=count+2;
    count++;
  }
  if(showcfast_local==1){
    hcolorbar_vis[COLORBAR_ZONE]=count+2;
    count++;
  }
  return count;
}

/* ------------------ DrawHorizontalColorbars ------------------------ */

void DrawHorizontalColorbars(void){
  int doit=0;
  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;
  int show_hvacduct_colorbar_local = 0;
  int show_hvacnode_colorbar_local = 0;

  UpdateShowColorbar(&showcfast_local, &show_slice_colorbar_local, 
    & show_hvacduct_colorbar_local, &show_hvacnode_colorbar_local);

  CountColorbars();

  if(vis_colorbar==hcolorbar_vis[COLORBAR_SLICE]){
    if(show_slice_colorbar_local == 1)doit=1;
  }
  else if(show_hvacduct_colorbar_local==1 || show_hvacnode_colorbar_local == 1){
    doit = 1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_BOUNDARY]){
    if(showpatch == 1 && wall_cell_color_flag == 0)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_PLOT3D]){
    if(showplot3d==1)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_ZONE]){
    if(showcfast_local==1)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_PART]){
    if(showsmoke==1&&parttype!=0)doit=1;
  }
  else if(vis_colorbar==hcolorbar_vis[COLORBAR_ISO]){
    if(showiso_colorbar)doit=1;
  }
  else{
    doit=1;
  }
  if(doit==0)return;

  SNIFF_ERRORS("before horizontal colorbar");
  CheckMemory;

    // -------------- draw plot3d colorbars ------------

  if(showplot3d == 1 && contour_type == STEPPED_CONTOURS){
    int i;
    float right_hat;

    right_hat = hcolorbar_right_pos*(float)(nrgb - 3) / (float)(nrgb - 2)+hcolorbar_left_pos/(float)(nrgb-2);

    glBegin(GL_QUADS);
    for(i = 0; i < nrgb - 2; i++){
      float *rgb_plot3d_local;
      float xleft, xright;

      rgb_plot3d_local = rgb_plot3d_contour[i];

      xleft = MIX2(i, nrgb - 3, right_hat, hcolorbar_left_pos);
      xright = MIX2(i + 1, nrgb - 3, right_hat, hcolorbar_left_pos);

      if(rgb_plot3d_local[3] != 0.0){
        glColor4fv(rgb_plot3d_local);
        glVertex2f(xleft, hcolorbar_down_pos);
        glVertex2f(xright, hcolorbar_down_pos);
        glVertex2f(xright, hcolorbar_top_pos);
        glVertex2f(xleft, hcolorbar_top_pos);
      }
    }
    glEnd();
    if(show_extreme_mindata == 1 || show_extreme_maxdata == 1){
      float barmid;
      float *rgb_plot3d_local;
      float xleft, xright;

      rgb_plot3d_local = rgb_plot3d_contour[nrgb - 2];
      barmid = (hcolorbar_down_pos + hcolorbar_top_pos) / 2.0;
      i = -1;
      xright = MIX2(i + 0.5, nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);
      xleft  = MIX2(i + 1, nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);

      if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glEnable(GL_POLYGON_SMOOTH);

      if(show_extreme_mindata == 1 && have_extreme_mindata == 1 && rgb_plot3d_local[3] != 0.0){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_plot3d_local);

        glVertex2f(xleft, hcolorbar_down_pos);
        glVertex2f(xleft, hcolorbar_top_pos);
        glVertex2f(xright, barmid);
        glEnd();
      }

      i = nrgb - 2;
      xleft = MIX2(i, nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);
      xright = MIX2(i + 0.5, nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);

      rgb_plot3d_local = rgb_plot3d_contour[nrgb - 1];
      if(show_extreme_maxdata == 1 && have_extreme_maxdata == 1 && rgb_plot3d_local[3] != 0.0){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_plot3d_local);
        glVertex2f(xleft, hcolorbar_down_pos);
        glVertex2f(xleft, hcolorbar_top_pos);
        glVertex2f(xright, barmid);
        glEnd();
      }
      if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glDisable(GL_POLYGON_SMOOTH);
    }
  }
  else{

    // -------------- draw all other colorbars ------------

    DrawHorizontalColorbarReg();
  }
  if(show_extreme_mindata == 1 || show_extreme_maxdata == 1){
    float barmid;

    barmid = (hcolorbar_top_pos + hcolorbar_down_pos) / 2.0;

    if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glEnable(GL_POLYGON_SMOOTH);

    if(show_extreme_mindata == 1 && have_extreme_mindata == 1){
      glBegin(GL_TRIANGLES);
      glColor4fv(rgb_full[0]);

      glVertex2f(hcolorbar_left_pos, hcolorbar_down_pos);
      glVertex2f(hcolorbar_left_pos, hcolorbar_top_pos);
      glVertex2f(hcolorbar_left_pos - 0.866*hcolorbar_delta,barmid);
      glEnd();
    }

    if(show_extreme_maxdata == 1 && have_extreme_maxdata == 1){
      glBegin(GL_TRIANGLES);
      glColor4fv(rgb_full[nrgb_full - 1]);
      glVertex2f(hcolorbar_right_pos, hcolorbar_top_pos);
      glVertex2f(hcolorbar_right_pos, hcolorbar_down_pos);
      glVertex2f(hcolorbar_right_pos + 0.866*hcolorbar_delta, barmid);
      glEnd();
    }
    if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glDisable(GL_POLYGON_SMOOTH);
  }
}

/* ------------------ DrawVerticalColorbars ------------------------ */

void DrawVerticalColorbars(void){
  int i;
  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;
  int show_hvacduct_colorbar_local = 0;
  int show_hvacnode_colorbar_local = 0;

  UpdateShowColorbar(&showcfast_local, &show_slice_colorbar_local, 
    &show_hvacduct_colorbar_local, &show_hvacnode_colorbar_local);

  // -------------- compute columns where left labels will occur ------------

  if(showiso_colorbar==1||
    (showsmoke==1&&parttype!=0)|| show_slice_colorbar_local==1||
    (showpatch==1&&wall_cell_color_flag==0)||
    showcfast_local==1||show_hvacduct_colorbar_local==1||show_hvacnode_colorbar_local == 1 ||
    showplot3d==1){

    SNIFF_ERRORS("before vertical colorbar");
    CheckMemory;

    // -------------- draw plot3d colorbars ------------

    if(showplot3d==1&&contour_type==STEPPED_CONTOURS){
      float top_hat;

      top_hat = vcolorbar_top_pos*(float)(nrgb - 3) / (float)(nrgb - 2)+vcolorbar_down_pos/(float)(nrgb-2);

      glBegin(GL_QUADS);
      for(i = 0; i < nrgb-2; i++){
        float *rgb_plot3d_local;
        float ybot, ytop;

        rgb_plot3d_local = rgb_plot3d_contour[i];
        ybot = MIX2(  i,nrgb-3,top_hat,vcolorbar_down_pos);
        ytop = MIX2(i+1,nrgb-3,top_hat,vcolorbar_down_pos);

        if(rgb_plot3d_local[3]!=0.0){
          glColor4fv(rgb_plot3d_local);
          glVertex2f((float)vcolorbar_left_pos, ybot);
          glVertex2f(vcolorbar_right_pos,ybot);

          glVertex2f(vcolorbar_right_pos,ytop);
          glVertex2f(vcolorbar_left_pos, ytop);
        }
      }
      glEnd();
      if(show_extreme_mindata==1||show_extreme_maxdata==1){
        float barmid;
        float *rgb_plot3d_local;
        float ybot, ytop;

        rgb_plot3d_local = rgb_plot3d_contour[nrgb-2];
        barmid = (vcolorbar_left_pos+vcolorbar_right_pos)/2.0;
        i=-1;
        ytop = MIX2(i+0.5,nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);
        ybot = MIX2(i+1,nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);

        if(have_extreme_mindata==1||have_extreme_maxdata==1)glEnable(GL_POLYGON_SMOOTH);

        if(show_extreme_mindata==1&&have_extreme_mindata==1&&rgb_plot3d_local[3]!=0.0){
          glBegin(GL_TRIANGLES);
          glColor4fv(rgb_plot3d_local);

          glVertex2f(vcolorbar_left_pos,ybot);
          glVertex2f(barmid,ytop);
          glVertex2f(vcolorbar_right_pos,ybot);
          glEnd();
        }

        i=nrgb-2;
        ybot = MIX2(i,nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);
        ytop = MIX2(i+0.5,nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);

        rgb_plot3d_local = rgb_plot3d_contour[nrgb-1];
        if(show_extreme_maxdata==1&&have_extreme_maxdata==1&&rgb_plot3d_local[3]!=0.0){
          glBegin(GL_TRIANGLES);
          glColor4fv(rgb_plot3d_local);
          glVertex2f(vcolorbar_left_pos, ybot);
          glVertex2f(vcolorbar_right_pos,ybot);
          glVertex2f(barmid, ytop);
          glEnd();
        }
        if(have_extreme_mindata==1||have_extreme_maxdata==1)glDisable(GL_POLYGON_SMOOTH);
      }
    }
    else{

      // -------------- draw all other colorbars ------------
      DrawVerticalColorbarReg();
    }
    if(show_extreme_mindata==1||show_extreme_maxdata==1){
      float barmid;

      barmid=(vcolorbar_right_pos+vcolorbar_left_pos)/2.0;

      if(have_extreme_mindata==1||have_extreme_maxdata==1)glEnable(GL_POLYGON_SMOOTH);

      if(show_extreme_mindata==1&&have_extreme_mindata==1){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_full[0]);

        glVertex2f( vcolorbar_left_pos, vcolorbar_down_pos);
        glVertex2f(            barmid, vcolorbar_down_pos-0.866*vcolorbar_delta);
        glVertex2f(vcolorbar_right_pos, vcolorbar_down_pos);
        glEnd();
      }

      if(show_extreme_maxdata==1&&have_extreme_maxdata==1){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_full[nrgb_full-1]);
        glVertex2f(vcolorbar_right_pos, vcolorbar_top_pos);
        glVertex2f(            barmid, vcolorbar_top_pos+0.866*vcolorbar_delta);
        glVertex2f( vcolorbar_left_pos, vcolorbar_top_pos);
        glEnd();
      }
      if(have_extreme_mindata==1||have_extreme_maxdata==1)glDisable(GL_POLYGON_SMOOTH);
    }
  }
}

/* ------------------ DrawHorizontalColorbarRegLabels ------------------------ */

void DrawHorizontalColorbarRegLabels(void){
  int i;
  int iposition;

  int sliceflag = 0;
  float *slicefactor = NULL;
  float slicefactor2[2];
  float *isofactor = NULL;

  int plot3dflag = 0;
  float *plot3dfactor = NULL;
  float plot3dfactor2[2];
  float plot3drange;

  int patchflag = 0;
  int zoneflag = 0;
  float *patchfactor = NULL;
  float *zonefactor = NULL;
  float patchrange = 0.0;
  float zonerange;

  int partflag = 0;
  float *partfactor = NULL;
  float partrange = 0.0;

  int type_label_left, type_label_down;
  int axis_label_left, axis_label_down;

  int fed_slice = 0;

  GLfloat *foreground_color, *red_color;

  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;
  int show_hvacduct_colorbar_local = 0;
  int show_hvacnode_colorbar_local = 0;

  UpdateShowColorbar(&showcfast_local, &show_slice_colorbar_local, 
    &show_hvacduct_colorbar_local, &show_hvacnode_colorbar_local);

  foreground_color = &(foregroundcolor[0]);
  red_color = &(redcolor[0]);

  type_label_left = hcolorbar_delta+hcolorbar_right_pos+h_space;
  type_label_down = 1.5*VP_vcolorbar.text_height;
  axis_label_left = -colorbar_label_width/4;
  axis_label_down = hcolorbar_down_pos-(VP_vcolorbar.text_height + v_space);

  if(showiso_colorbar==1||
    (showsmoke == 1 && parttype != 0) || show_slice_colorbar_local == 1 ||
    (showpatch == 1 && wall_cell_color_flag == 0) ||
    showcfast_local==1 || showplot3d == 1){

    SNIFF_ERRORS("before colorbar");
    CheckMemory;
    if(show_slice_colorbar_local==1){
      boundsdata *sb;

      sb = slicebounds + slicefile_labelindex;

      if(strcmp(sb->label->shortlabel, "FED") ==  0&& current_colorbar != NULL){
        strcpy(default_fed_colorbar, current_colorbar->menu_label);
        if(strcmp(current_colorbar->menu_label, "FED") == 0){
          fed_slice = 1;
          if(strcmp(sb->colorlabels[1], "0.00") != 0 || strcmp(sb->colorlabels[nrgb - 1], "3.00") != 0)fed_slice = 0;
        }
      }
    }
  }

  // -------------- particle file top labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_PART]&&showsmoke == 1){
    char partunitlabel2[256], partshortlabel2[256];

    strcpy(partshortlabel2, "");
    strcpy(partunitlabel2, "");

    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);

    if(parttype != 0){
      if(showsmoke == 1)OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Part");
    }
    if(parttype == -1){
      strcpy(partshortlabel2, "temp");
      strcpy(partunitlabel2, (const char *)degC);
    }
    else if(parttype == -2){
      strcpy(partshortlabel2, "HRRPUV");
      strcpy(partunitlabel2, "kW/m3");
    }
    else{
      if(partshortlabel != NULL)strcpy(partshortlabel2, partshortlabel);
      if(partunitlabel != NULL)strcpy(partunitlabel2, partunitlabel);
    }
    if(parttype != 0){
      int partunitclass, partunittype;

      GetUnitInfo(partunitlabel2, &partunitclass, &partunittype);
      if(partunitclass >= 0 && partunitclass < nunitclasses && partunittype >= 0){
        partflag = 1;
        partfactor = unitclasses[partunitclass].units[partunittype].scale;
        strcpy(partunitlabel, unitclasses[partunitclass].units[partunittype].unit);
      }
      OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, partshortlabel);
      OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, partunitlabel);
    }
    glPopMatrix();
  }

  // -------------- slice file top labels ------------
 if(vis_colorbar==hcolorbar_vis[COLORBAR_SLICE]&&show_slice_colorbar_local==1){
    char unitlabel[256];
    int sliceunitclass, sliceunittype;
    boundsdata *sb;

    sb = slicebounds + slicefile_labelindex;
    strcpy(unitlabel, sb->label->unit);
    GetUnitInfo(sb->label->unit, &sliceunitclass, &sliceunittype);
    if(sliceunitclass >= 0 && sliceunitclass < nunitclasses){
      if(sliceunittype > 0){
        sliceflag = 1;
        slicefactor = unitclasses[sliceunitclass].units[sliceunittype].scale;
        strcpy(unitlabel, unitclasses[sliceunitclass].units[sliceunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Slice");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, sb->label->shortlabel);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    if(strcmp(unitlabel, "ppm") == 0 && slicefactor != NULL){
      slicefactor2[0] = *slicefactor;
      slicefactor2[1] = 0.0;
      slicefactor = slicefactor2;
    }
    glPopMatrix();
  }

  // -------------- isosurface top labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_ISO]&&showiso_colorbar == 1){
    char unitlabel[256];
    boundsdata *sb;

    sb = isobounds + iisottype;
    strcpy(unitlabel, sb->label->unit);
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Iso");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, sb->label->shortlabel);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    glPopMatrix();
  }

  // -------------- boundary file top labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_BOUNDARY]&&showpatch == 1 && wall_cell_color_flag == 0){
    char unitlabel[256];
    patchdata *patchi;
    int patchunitclass, patchunittype;

    patchi = patchinfo + boundarytypes[iboundarytype];
    strcpy(unitlabel, patchi->label.unit);
    GetUnitInfo(patchi->label.unit, &patchunitclass, &patchunittype);
    if(patchunitclass >= 0 && patchunitclass < nunitclasses){
      if(patchunittype > 0){
        patchflag = 1;
        patchfactor = unitclasses[patchunitclass].units[patchunittype].scale;
        strcpy(unitlabel, unitclasses[patchunitclass].units[patchunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);

    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Bndry");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, patchi->label.shortlabel);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    glPopMatrix();
  }

  // -------------- plot3d top labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_PLOT3D]&&showplot3d == 1){
    char *p3label;
    char *up3label;
    char unitlabel[256];
    int plot3dunitclass, plot3dunittype;

    up3label = plot3dinfo[0].label[plotn - 1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass >= 0 && plot3dunitclass < nunitclasses){
      if(plot3dunittype > 0){
        plot3dflag = 1;
        plot3dfactor = unitclasses[plot3dunitclass].units[plot3dunittype].scale;
        strcpy(unitlabel, unitclasses[plot3dunitclass].units[plot3dunittype].unit);
      }
    }
    p3label = plot3dinfo[0].label[plotn - 1].shortlabel;
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Plot3D");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, p3label);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    if(strcmp(unitlabel, "ppm") == 0 && plot3dfactor != NULL){
      plot3dfactor2[0] = *plot3dfactor;
      plot3dfactor2[1] = 0.0;
      plot3dfactor = plot3dfactor2;
    }
    glPopMatrix();
  }

  if(vis_colorbar==hcolorbar_vis[COLORBAR_ZONE]&&showcfast_local == 1){
    char unitlabel[256];
    int zoneunitclass, zoneunittype;

    strcpy(unitlabel, (const char *)degC);
    GetUnitInfo(unitlabel, &zoneunitclass, &zoneunittype);
    if(zoneunitclass >= 0 && zoneunitclass < nunitclasses){
      if(zoneunittype > 0){
        zoneflag = 1;
        zonefactor = unitclasses[zoneunitclass].units[zoneunittype].scale;
        strcpy(unitlabel, unitclasses[zoneunitclass].units[zoneunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(type_label_left, type_label_down, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Zone");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, "Temp");
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    glPopMatrix();
    SNIFF_ERRORS("After ZONE labels");
  }

  // -------------- isosurface left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_ISO]&&showiso_colorbar == 1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float isorange;

    sb = isobounds + iisottype;
    tttmin = iso_valmin;
    tttmax = iso_valmax;
    int i;
    for(i = 0;i < 256;i++){
      float f1;

      f1 = (float)i / 255.0;
      sb->levels256[i] = tttmin * (1.0-f1) + f1 * tttmax;
    }
    isorange = tttmax - tttmin;
    iposition = -1;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char isocolorlabel[256], isolabel[256];
      char *isocolorlabel_ptr = NULL;
      float horiz_position;

      tttval = sb->levels256[valindex];
      Num2String(isolabel, tttval);
      isocolorlabel_ptr = isolabel;
      ScaleFloat2String(tttval, isocolorlabel, isofactor);
      isocolorlabel_ptr = isocolorlabel;
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, isocolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float horiz_position;
      char isocolorlabel[256];
      char *isocolorlabel_ptr = NULL;
      float val;

      horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
      if(iposition == i)continue;
      isocolorlabel_ptr = &(sb->colorlabels[i + 1][0]);

      val = tttmin + i*isorange / (nrgb - 2);
      ScaleFloat2String(val, isocolorlabel, isofactor);
      isocolorlabel_ptr = isocolorlabel;
      OutputBarText(horiz_position, 0.0, foreground_color, isocolorlabel_ptr);
    }
    glPopMatrix();
  }

  // -------------- particle left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_PART]&&showsmoke == 1 && parttype != 0){
    float *partlevels256_ptr;
    float tttval, tttmin, tttmax;

    partlevels256_ptr = partlevels256;
    if(global_prop_index>= 0 &&global_prop_index < npart5prop){
      partlevels256_ptr = part5propinfo[global_prop_index].ppartlevels256;
    }

    iposition = -1;
    tttmin = partlevels256_ptr[0];
    tttmax = partlevels256_ptr[255];
    partrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char partcolorlabel[256], *partcolorlabel_ptr = NULL, partlabel[256];
      float horiz_position;

      tttval = partlevels256_ptr[valindex];
      Num2String(partlabel, tttval);
      partcolorlabel_ptr = partlabel;
      if(partflag == 1){
        ScaleFloat2String(tttval, partcolorlabel, partfactor);
        partcolorlabel_ptr = partcolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, partcolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float horiz_position;
      char partcolorlabel[256];
      char *partcolorlabel_ptr = NULL;

      horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
      if(iposition == i)continue;
      if(global_prop_index>= 0 &&global_prop_index < npart5prop){
        float val;

        val = part5propinfo[global_prop_index].partlabelvals[i + 1];
        Float2String(partcolorlabel, val, ncolorlabel_digits, force_fixedpoint);
        partcolorlabel_ptr = partcolorlabel;
      }
      else{
        if(colorlabelpart != NULL){
          partcolorlabel_ptr = &colorlabelpart[i + 1][0];
        }
        else{
          partcolorlabel_ptr = NULL;
        }
      }
      if(partflag == 1){
        float val;

        partcolorlabel_ptr = partcolorlabel;
        val = tttmin + i*partrange / (nrgb - 2);
        val = ScaleFloat2Float(val, partfactor);
        Float2String(partcolorlabel_ptr, val, ncolorlabel_digits, force_fixedpoint);
      }
      OutputBarText(horiz_position, 0.0, foreground_color, partcolorlabel_ptr);
    }
    glPopMatrix();
  }

  // -------------- slice left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_SLICE]&&show_slice_colorbar_local==1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float slicerange;

    sb = slicebounds + slicefile_labelindex;
    tttmin = sb->levels256[0];
    tttmax = sb->levels256[255];
    slicerange = tttmax - tttmin;
    iposition = -1;
    glPushMatrix();
    glTranslatef(axis_label_left,axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char slicelabel[256], slicecolorlabel[256];
      char *slicecolorlabel_ptr = NULL;
      float horiz_position;

      tttval = sb->levels256[valindex];
      Num2String(slicelabel, tttval);
      slicecolorlabel_ptr = slicelabel;
      if(sliceflag == 1){
        ScaleFloat2String(tttval, slicecolorlabel, slicefactor);
        slicecolorlabel_ptr = slicecolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, slicecolorlabel_ptr);
    }
    if(fed_slice == 1){
      for(i = 0; i < nrgb - 1; i++){
        float horiz_position;

        horiz_position = MIX2(0.0, 3.0, hcolorbar_right_pos, hcolorbar_left_pos);
        OutputBarText(horiz_position, 0.0, foreground_color, "0.00");

        horiz_position = MIX2(0.3, 3.0, hcolorbar_right_pos, hcolorbar_left_pos);
        OutputBarText(horiz_position, 0.0, foreground_color, "0.30");

        horiz_position = MIX2(1.0, 3.0, hcolorbar_right_pos, hcolorbar_left_pos);
        OutputBarText(horiz_position, 0.0, foreground_color, "1.00");

        horiz_position = MIX2(3.0, 3.0, hcolorbar_right_pos, hcolorbar_left_pos);
        OutputBarText(horiz_position, 0.0, foreground_color, "3.00");
      }
    }
    else{
      for(i = 0; i < nrgb - 1; i++){
        float horiz_position;
        char slicecolorlabel[256];
        char *slicecolorlabel_ptr = NULL;

        horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
        if(iposition == i)continue;
        slicecolorlabel_ptr = &(sb->colorlabels[i + 1][0]);
        if(sliceflag == 1){
          float val;

          val = tttmin + i*slicerange / (nrgb - 2);
          ScaleFloat2String(val, slicecolorlabel, slicefactor);
          slicecolorlabel_ptr = slicecolorlabel;
          Float2String(slicecolorlabel_ptr, val, ncolorlabel_digits, force_fixedpoint);
        }
        else{
          slicecolorlabel_ptr = slicecolorlabel;
          Float2String(slicecolorlabel_ptr, sb->colorvalues[i+1], ncolorlabel_digits, force_fixedpoint);
        }
        OutputBarText(horiz_position, 0.0, foreground_color, slicecolorlabel_ptr);
      }
    }
    glPopMatrix();
  }

  // -------------- boundary left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_BOUNDARY]&&showpatch == 1 && wall_cell_color_flag == 0){
    float tttval, tttmin, tttmax;

    iposition = -1;
    tttmin = boundarylevels256[0];
    tttmax = boundarylevels256[255];
    patchrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char boundary_colorlabel[256], boundarylabel[256], *boundary_colorlabel_ptr = NULL;
      float horiz_position;

      // draw boundary file value selected with mouse
      tttval = boundarylevels256[valindex];
      Num2String(boundarylabel, tttval);
      boundary_colorlabel_ptr = &(boundarylabel[0]);
      if(patchflag == 1){
        ScaleFloat2String(tttval, boundary_colorlabel, patchfactor);
        boundary_colorlabel_ptr = boundary_colorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0,horiz_position, red_color, boundary_colorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      char boundary_colorlabel[256];
      char *boundary_colorlabel_ptr = NULL;
      float horiz_position;
      float val;

      horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);

      if(iposition == i)continue;
      if(patchflag == 1){
        val = tttmin + i*patchrange / (nrgb - 2);
      }
      else{
        val = colorvaluespatch[i+1];
      }
      val = ScaleFloat2Float(val, patchfactor);
      Float2String(boundary_colorlabel, val, ncolorlabel_digits, force_fixedpoint);
      boundary_colorlabel_ptr = boundary_colorlabel;
      OutputBarText(horiz_position, 0.0, foreground_color, boundary_colorlabel_ptr);
    }
    glPopMatrix();
  }

  // -------------- zone left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_ZONE]&&showcfast_local == 1){
    float tttval, tttmin, tttmax;

    iposition = -1;
    tttmin = zonelevels256[0];
    tttmax = zonelevels256[255];
    zonerange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char zonecolorlabel[256], *zonecolorlabel_ptr = NULL, zonelabel[256];
      float horiz_position;

      tttval = zonelevels256[valindex];
      Num2String(zonelabel, tttval);
      zonecolorlabel_ptr = &(zonelabel[0]);
      if(zoneflag == 1){
        ScaleFloat2String(tttval, zonecolorlabel, zonefactor);
        zonecolorlabel_ptr = zonecolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, zonecolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float horiz_position;
      char zonecolorlabel[256];
      char *zonecolorlabel_ptr = NULL;

      horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
      if(iposition == i)continue;
      zonecolorlabel_ptr = &colorlabelzone[i + 1][0];
      if(zoneflag == 1){
        float val;

        val = tttmin + (i - 1)*zonerange / (nrgb - 2);
        ScaleFloat2String(val, zonecolorlabel, zonefactor);
        zonecolorlabel_ptr = zonecolorlabel;
      }
      OutputBarText(horiz_position, 0.0, foreground_color, zonecolorlabel_ptr);
    }
    SNIFF_ERRORS("after zone left labels");
    glPopMatrix();
  }

  // -------------- plot3d left labels ------------

  if(vis_colorbar==hcolorbar_vis[COLORBAR_PLOT3D]&&showplot3d == 1){
    float *p3lev;
    float tttval, tttmin, tttmax;

    iposition = -1;
    p3lev = p3levels256[plotn - 1];
    tttmin = p3lev[0];
    tttmax = p3lev[255];
    plot3drange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(axis_label_left, axis_label_down, 0.0);
    if(global_colorbar_index != -1){
      char plot3dcolorlabel[256], p3dlabel[256], *plot3dcolorlabel_ptr = NULL;
      float horiz_position;

      tttval = p3lev[valindex];
      Num2String(p3dlabel, tttval);
      plot3dcolorlabel_ptr = p3dlabel;
      if(plot3dflag == 1){
        ScaleFloat2String(tttval, plot3dcolorlabel, plot3dfactor);
        plot3dcolorlabel_ptr = plot3dcolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, plot3dcolorlabel_ptr);
    }
    if(visiso == 0){
      float horiz_position;

      for(i = 0; i < nrgb - 1; i++){
        char plot3dcolorlabel[256];
        char *plot3dcolorlabel_ptr = NULL;
        float val;

        horiz_position = MIX2(i, nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
        if(iposition == i)continue;
        if(plot3dflag == 1){
          val = tttmin + i*plot3drange / (nrgb - 2);
        }
        else{
          val = colorvaluesp3[plotn - 1][i];
        }
        plot3dcolorlabel_ptr = plot3dcolorlabel;
        ScaleFloat2String(val, plot3dcolorlabel, plot3dfactor);
        Float2String(plot3dcolorlabel, val, ncolorlabel_digits, force_fixedpoint);
        OutputBarText(horiz_position, 0.0, foreground_color, plot3dcolorlabel_ptr);
      }
    }
    else{
      float horiz_position;
      float right_hat;

      right_hat = hcolorbar_right_pos*(float)(nrgb - 3) / (float)(nrgb - 2)+hcolorbar_left_pos/(float)(nrgb-2);
      for(i = 0; i < nrgb - 2; i++){
        char plot3dcolorlabel[256];
        char *plot3dcolorlabel_ptr = NULL;

        horiz_position = MIX2(i, nrgb - 2, right_hat, hcolorbar_left_pos);

        if(iposition == i)continue;
        plot3dcolorlabel_ptr = &colorlabeliso[plotn - 1][i][0];
        if(plot3dflag == 1){
          float val;

          val = tttmin + (i - 1)*plot3drange / (nrgb - 2);
          ScaleFloat2String(val, plot3dcolorlabel, plot3dfactor);
          plot3dcolorlabel_ptr = plot3dcolorlabel;
        }
        if(isolevelindex == i || isolevelindex2 == i){
          OutputBarText(horiz_position, 0.0, red_color, plot3dcolorlabel_ptr);
        }
        else{
          OutputBarText(horiz_position, 0.0, foreground_color, plot3dcolorlabel_ptr);
        }
      }
    }
    glPopMatrix();
  }
}

/* ------------------ DrawVerticalColorbarRegLabels ------------------------ */

void DrawVerticalColorbarRegLabels(void){
  int i;
  int ileft = 0;
  int leftzone, leftsmoke, leftslice, leftpatch, leftiso;
  int lefthvacduct, lefthvacnode;
  int iposition;

  int sliceflag = 0;
  float *slicefactor = NULL;
  float slicefactor2[2];
  float *isofactor = NULL;

  int plot3dflag = 0;
  float *plot3dfactor = NULL;
  float plot3dfactor2[2];
  float plot3drange;

  int patchflag = 0;
  int zoneflag = 0;
  float *patchfactor = NULL;
  float *zonefactor = NULL;
  float patchrange = 0.0;
  float zonerange;

  int partflag = 0;
  float *partfactor = NULL;
  int dohist = 0;

  int fed_slice = 0;
  float colorbar_max, colorbar_eps;

  GLfloat *foreground_color, *red_color;

  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;
  int show_hvacduct_colorbar_local = 0;
  int show_hvacnode_colorbar_local = 0;
  char exp_factor_label[256];

  max_colorbar_label_width = GetStringWidth("123456");

  UpdateShowColorbar(&showcfast_local, &show_slice_colorbar_local, 
    &show_hvacduct_colorbar_local, &show_hvacnode_colorbar_local);

  colorbar_eps = pow(10.0, -ncolorlabel_digits);

  // -------------- compute columns where left labels will occur ------------

  lefthvacnode = 0;
  lefthvacduct = 0;
  leftsmoke    = 0;
  leftslice    = 0;
  leftpatch    = 0;
  leftiso      = 0;
  ileft        = 0;
  if(showiso_colorbar == 1)leftiso = ileft++;
  if(showsmoke == 1&&parttype != 0)leftsmoke = ileft++;
  if(show_slice_colorbar_local == 1){
    leftslice = ileft++;
  }
  if(showpatch == 1 && wall_cell_color_flag == 0)leftpatch = ileft++;
  if(show_hvacnode_colorbar_local == 1)lefthvacnode = ileft++;
  if(show_hvacduct_colorbar_local == 1)lefthvacduct = ileft++;
  leftzone = ileft++;

  foreground_color = &(foregroundcolor[0]);
  red_color = &(redcolor[0]);
  if(showiso_colorbar == 1 ||
    (showsmoke == 1 && parttype != 0) || show_slice_colorbar_local == 1 ||
    (showpatch == 1 && wall_cell_color_flag == 0) ||
    showcfast_local==1 ||
    showplot3d == 1){

    SNIFF_ERRORS("before colorbar");
    CheckMemory;
    if(show_slice_colorbar_local==1){
      boundsdata *sb;

      sb = slicebounds + slicefile_labelindex;

      if(strcmp(sb->label->shortlabel, "FED") == 0){
        if(current_colorbar != NULL){
          strcpy(default_fed_colorbar, current_colorbar->menu_label);
          if(strcmp(current_colorbar->menu_label, "FED") == 0){
            fed_slice = 1;
            if(strcmp(sb->colorlabels[1], "0.00") != 0 || strcmp(sb->colorlabels[nrgb - 1], "3.00") != 0)fed_slice = 0;
          }
        }
      }
    }
  }

  // -------------- isosurface left labels ------------

  if(showiso_colorbar == 1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float isorange;

    sb = isobounds + iisottype;
    tttmin = iso_valmin;
    tttmax = iso_valmax;
    int i;
    for(i = 0;i < 256;i++){
      float f1;

      f1 = (float)i / 255.0;
      sb->levels256[i] = tttmin * (1.0-f1) + f1 * tttmax;
    }
    isorange = tttmax - tttmin;
    iposition = -1;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftiso*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char isocolorlabel[256], isolabel[256];
      char *isocolorlabel_ptr = NULL;
      float vert_position;

      tttval = sb->levels256[valindex];
      Num2String(isolabel, tttval);
      isocolorlabel_ptr = isolabel;
      ScaleFloat2String(tttval, isocolorlabel, isofactor);
      isocolorlabel_ptr = isocolorlabel;
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, isocolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;
      char isocolorlabel[256];
      char *isocolorlabel_ptr = NULL;
      float val;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      if(iposition == i)continue;
      isocolorlabel_ptr = &(sb->colorlabels[i + 1][0]);

      val = tttmin + i*isorange / (nrgb - 2);
      ScaleFloat2String(val, isocolorlabel, isofactor);
      isocolorlabel_ptr = isocolorlabel;
      OutputBarText(0.0, vert_position, foreground_color, isocolorlabel_ptr);
    }
    glPopMatrix();
  }

  // -------------- isosurface top labels ------------

  if(showiso_colorbar == 1){
    char unitlabel[256];
    boundsdata *sb;

    sb = isobounds + iisottype;
    strcpy(unitlabel, sb->label->unit);
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    glTranslatef(-leftiso*(colorbar_label_width + h_space), 0.0, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Iso");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, sb->label->shortlabel);
    OutputBarText(0.0, (VP_vcolorbar.text_height + v_space),     foreground_color, unitlabel);
    glPopMatrix();
  }

  // -------------- particle left labels ------------
  if(showsmoke == 1 && parttype != 0){
    float *partlevels256_ptr;
    float tttval;

    if(parttype!=0){
      int partunitclass, partunittype;

      GetUnitInfo(partunitlabel, &partunitclass, &partunittype);
      if(partunitclass>=0&&partunitclass<nunitclasses&&partunittype>=0){
        partflag = 1;
        partfactor = unitclasses[partunitclass].units[partunittype].scale;
      }
    }
    partlevels256_ptr = partlevels256;
    if(global_prop_index>= 0 &&global_prop_index < npart5prop){
      partlevels256_ptr = part5propinfo[global_prop_index].ppartlevels256;
    }

    iposition = -1;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftsmoke*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char partcolorlabel[256], *partcolorlabel_ptr = NULL, partlabel[256];
      float vert_position;

      tttval = partlevels256_ptr[valindex];
      Num2String(partlabel, tttval);
      partcolorlabel_ptr = partlabel;
      if(partflag == 1){
        ScaleFloat2String(tttval, partcolorlabel, partfactor);
        partcolorlabel_ptr = partcolorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, partcolorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      val = part5propinfo[global_prop_index].partlabelvals[i + 1];
      val = ScaleFloat2Float(val, partfactor);
      colorbar_vals[i] = val;
    }
    Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      if(iposition == i)continue;
      OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
    }
    glPopMatrix();
  }

  // -------------- particle file top labels ------------

  if(showsmoke==1&&parttype!=0){
    char partunitlabel2[256], partshortlabel2[256];

    strcpy(partshortlabel2, "");
    strcpy(partunitlabel2, "");

    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    if(dohist == 1)glTranslatef(colorbar_label_width / 2.0, 0.0, 0.0);

    if(parttype != 0){
      if(showsmoke == 1)OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Part");
    }
    if(parttype == -1){
      strcpy(partshortlabel2, "temp");
      strcpy(partunitlabel2, (const char *)degC);
    }
    else if(parttype == -2){
      strcpy(partshortlabel2, "HRRPUV");
      strcpy(partunitlabel2, "kW/m3");
    }
    else{
      if(partshortlabel != NULL)strcpy(partshortlabel2, partshortlabel);
      if(partunitlabel != NULL)strcpy(partunitlabel2,   partunitlabel);
    }
    if(parttype != 0){
      int partunitclass, partunittype;

      GetUnitInfo(partunitlabel, &partunitclass, &partunittype);
      if(partunitclass >= 0 && partunitclass < nunitclasses){
        if(partunittype >= 0){
          partflag = 1;
          partfactor = unitclasses[partunitclass].units[partunittype].scale;
          strcpy(partunitlabel2, unitclasses[partunitclass].units[partunittype].unit);
        }
      }
      OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space),     foreground_color, partshortlabel);
      OutputBarText(0.0, 1 * (VP_vcolorbar.text_height + v_space),     foreground_color, partunitlabel2);
      OutputBarText(0.0, 0 * (VP_vcolorbar.text_height + v_space),     foreground_color, exp_factor_label);
    }
    glPopMatrix();
  }

  // -------------- slice left labels ------------

  if(show_slice_colorbar_local==1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float slicerange;

    char unitlabel[256];
    int sliceunitclass, sliceunittype;

    sb = slicebounds + slicefile_labelindex;
    strcpy(unitlabel, sb->label->unit);
    GetUnitInfo(sb->label->unit, &sliceunitclass, &sliceunittype);
    if(sliceunitclass >= 0 && sliceunitclass < nunitclasses){
      if(sliceunittype > 0){
        sliceflag = 1;
        slicefactor = unitclasses[sliceunitclass].units[sliceunittype].scale;
        strcpy(unitlabel, unitclasses[sliceunitclass].units[sliceunittype].unit);
      }
    }

    tttmin = sb->levels256[0];
    tttmax = sb->levels256[255];
    slicerange = tttmax - tttmin;
    iposition = -1;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftslice*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char slicelabel[256], slicecolorlabel[256];
      char *slicecolorlabel_ptr = NULL;
      float vert_position;
      int shifted_colorbar_index;

      tttval = sb->levels256[valindex];
      shifted_colorbar_index = global_colorbar_index;
      if(ABS(colorbar_shift-1.0)>0.0001){
        shifted_colorbar_index = SHIFT_VAL(global_colorbar_index, 0, 255, colorbar_shift);
      }
      Float2String(slicelabel, tttval, ncolorlabel_digits, force_fixedpoint);
      slicecolorlabel_ptr = slicelabel;
      if(sliceflag == 1){
        ScaleFloat2String(tttval, slicecolorlabel, slicefactor);
        slicecolorlabel_ptr = slicecolorlabel;
      }
      vert_position = MIX2(shifted_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(shifted_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, slicecolorlabel_ptr);
    }
    if(fed_slice == 1){
      for(i = 0; i < nrgb - 1; i++){
        float vert_position;

        vert_position = MIX2(0.0, 3.0, vcolorbar_top_pos, vcolorbar_down_pos);
        OutputBarText(0.0, vert_position, foreground_color, "0.00");

        vert_position = MIX2(0.3, 3.0, vcolorbar_top_pos, vcolorbar_down_pos);
        OutputBarText(0.0, vert_position, foreground_color, "0.30");

        vert_position = MIX2(1.0, 3.0, vcolorbar_top_pos, vcolorbar_down_pos);
        OutputBarText(0.0, vert_position, foreground_color, "1.00");

        vert_position = MIX2(3.0, 3.0, vcolorbar_top_pos, vcolorbar_down_pos);
        OutputBarText(0.0, vert_position, foreground_color, "3.00");
      }
    }
    else{
      float valmin, valmax;


      if(sliceflag==1){
        valmin = tttmin;
      }
      else{
        valmin = sb->colorvalues[1];
      }
      valmin = ScaleFloat2Float(valmin, slicefactor);

      if(sliceflag==1){
        valmax = tttmax;
      }
      else{
        valmax = sb->colorvalues[nrgb-1];
      }
      valmax = ScaleFloat2Float(valmax, slicefactor);
      colorbar_max = MAX(ABS(valmax), ABS(valmin));
      colorbar_max = MAX(colorbar_max, colorbar_eps);

      for(i = 0; i<nrgb-1; i++){
        float val;

        if(iposition==i)continue;
        if(sliceflag==1){
          val = tttmin+i*slicerange/(nrgb-2);
        }
        else{
          val = sb->colorvalues[i+1];
        }
        val = ScaleFloat2Float(val, slicefactor);
        if(ABS(colorbar_shift-1.0)>0.0001){
          val = SHIFT_VAL(val, valmin, valmax, 1.0/colorbar_shift);
        }
        colorbar_vals[i] = val;
      }
      Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
      for(i = 0; i < nrgb - 1; i++){
        float vert_position;

        vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
        if(iposition == i)continue;
        OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
        max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
      }
    }
    glPopMatrix();
  }

  // -------------- slice file top labels ------------

  if(show_slice_colorbar_local==1){
    char unitlabel[256];
    int sliceunitclass, sliceunittype;
    boundsdata *sb;

    sb = slicebounds + slicefile_labelindex;
    strcpy(unitlabel, sb->label->unit);
    GetUnitInfo(sb->label->unit, &sliceunitclass, &sliceunittype);
    if(sliceunitclass >= 0 && sliceunitclass < nunitclasses){
      if(sliceunittype > 0){
        sliceflag = 1;
        slicefactor = unitclasses[sliceunitclass].units[sliceunittype].scale;
        strcpy(unitlabel, unitclasses[sliceunitclass].units[sliceunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    glTranslatef(-leftslice*(colorbar_label_width + h_space), 0.0, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Slice");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, sb->label->shortlabel);
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    if(strcmp(unitlabel, "ppm") == 0 && slicefactor != NULL){
      slicefactor2[0] = *slicefactor;
      slicefactor2[1] = 0.0;
      slicefactor = slicefactor2;
    }
    glPopMatrix();
  }

  // -------------- HVAC node left labels ------------

  if(show_hvacnode_colorbar_local==1 && hvacnodevar_index>=0){
    hvacvaldata *hi;
    float tttval, tttmin, tttmax;
    float hvacrange;

    hi = hvacnodevalsinfo->node_vars + hvacnodevar_index;
    iposition = -1;
    tttmin = hi->levels256[0];
    tttmax = hi->levels256[255];
    hvacrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-lefthvacnode*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char hvaclabel[256], *hvac_colorlabel_ptr = NULL;
      float vert_position;

      // draw hvac file value selected with mouse
      tttval = hi->levels256[valindex];
      Num2String(hvaclabel, tttval);
      hvac_colorlabel_ptr = &(hvaclabel[0]);
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, hvac_colorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      val = tttmin + i * hvacrange / (nrgb - 2);
      colorbar_vals[i] = val;
      GetMantissaExponent(ABS(val), colorbar_exponents + i);
    }
    Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

      if(iposition == i)continue;
      OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
    }
    glPopMatrix();
  }

  // -------------- HVAC duct left labels ------------

  if(show_hvacduct_colorbar_local==1 && hvacductvar_index>=0){
    hvacvaldata *hi;
    float tttval, tttmin, tttmax;
    float hvacrange;

    hi = hvacductvalsinfo->duct_vars + hvacductvar_index;
    iposition = -1;
    tttmin = hi->levels256[0];
    tttmax = hi->levels256[255];
    hvacrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-lefthvacduct*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char hvaclabel[256], *hvac_colorlabel_ptr = NULL;
      float vert_position;

      // draw hvac file value selected with mouse
      tttval = hi->levels256[valindex];
      Num2String(hvaclabel, tttval);
      hvac_colorlabel_ptr = &(hvaclabel[0]);
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, hvac_colorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      val = tttmin + i * hvacrange / (nrgb - 2);
      colorbar_vals[i] = val;
      GetMantissaExponent(ABS(val), colorbar_exponents + i);
    }
    Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

      if(iposition == i)continue;
      OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
    }
    glPopMatrix();
  }

  // -------------- HVAC file node top labels ------------

  if(show_hvacnode_colorbar_local==1 && hvacnodevar_index>=0){
    char *slabel, *unitlabel;
    hvacvaldata *hi;

    hi = hvacnodevalsinfo->node_vars + hvacnodevar_index;
    slabel = hi->label.shortlabel;
    unitlabel = hi->label.unit;

    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    glTranslatef(-lefthvacnode*(colorbar_label_width + h_space), 0.0, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "HVAC");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, "node");
    OutputBarText(0.0, 1 * (VP_vcolorbar.text_height + v_space), foreground_color, slabel);
    OutputBarText(0.0, 0 * (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0,-1 * (VP_vcolorbar.text_height + v_space), foreground_color, exp_factor_label);
    glPopMatrix();
  }

  // -------------- HVAC file duct top labels ------------

  if(show_hvacduct_colorbar_local==1 && hvacductvar_index >=0){
    char *slabel, *unitlabel;
    hvacvaldata *hi;

    hi = hvacductvalsinfo->duct_vars + hvacductvar_index;
    slabel = hi->label.shortlabel;
    unitlabel = hi->label.unit;

    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    glTranslatef(-lefthvacduct*(colorbar_label_width + h_space), 0.0, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "HVAC");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, "duct");
    OutputBarText(0.0, 1 * (VP_vcolorbar.text_height + v_space), foreground_color, slabel);
    OutputBarText(0.0, 0 * (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0,-1 * (VP_vcolorbar.text_height + v_space), foreground_color, exp_factor_label);
    glPopMatrix();
  }

  // -------------- boundary left labels ------------

  if(showpatch == 1 && wall_cell_color_flag == 0){
    float tttval, tttmin, tttmax;

    char unitlabel[256];
    patchdata *patchi;
    int patchunitclass, patchunittype;

    patchi = patchinfo + boundarytypes[iboundarytype];
    strcpy(unitlabel, patchi->label.unit);
    GetUnitInfo(patchi->label.unit, &patchunitclass, &patchunittype);
    if(patchunitclass >= 0 && patchunitclass < nunitclasses){
      if(patchunittype > 0){
        patchflag = 1;
        patchfactor = unitclasses[patchunitclass].units[patchunittype].scale;
        strcpy(unitlabel, unitclasses[patchunitclass].units[patchunittype].unit);
      }
    }

    iposition = -1;
    tttmin = boundarylevels256[0];
    tttmax = boundarylevels256[255];
    patchrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftpatch*(colorbar_label_width + h_space), 0.0, 0.0);
    if(dohist == 1)glTranslatef(colorbar_label_width / 2.0, 0.0, 0.0);
    if(global_colorbar_index != -1){
      char boundary_colorlabel[256], boundarylabel[256], *boundary_colorlabel_ptr = NULL;
      float vert_position;

      // draw boundary file value selected with mouse
      tttval = boundarylevels256[valindex];
      Num2String(boundarylabel, tttval);
      boundary_colorlabel_ptr = &(boundarylabel[0]);
      if(patchflag == 1){
        ScaleFloat2String(tttval, boundary_colorlabel, patchfactor);
        boundary_colorlabel_ptr = boundary_colorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, boundary_colorlabel_ptr);
    }
    for(i = 0; i < nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      if(patchflag==1){
        val = tttmin+i*patchrange/(nrgb-2);
      }
      else{
        val = colorvaluespatch[i+1];
      }
      val = ScaleFloat2Float(val, patchfactor);
      colorbar_vals[i] = val;
      GetMantissaExponent(ABS(val), colorbar_exponents + i);
    }
    Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

      if(iposition == i)continue;
      OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
    }
    glPopMatrix();
  }

  // -------------- boundary file top labels ------------

  if(showpatch == 1 && wall_cell_color_flag == 0){
    char unitlabel[256];
    patchdata *patchi;
    int patchunitclass, patchunittype;

    patchi = patchinfo + boundarytypes[iboundarytype];
    strcpy(unitlabel, patchi->label.unit);
    GetUnitInfo(patchi->label.unit, &patchunitclass, &patchunittype);
    if(patchunitclass >= 0 && patchunitclass < nunitclasses){
      if(patchunittype > 0){
        patchflag = 1;
        patchfactor = unitclasses[patchunitclass].units[patchunittype].scale;
        strcpy(unitlabel, unitclasses[patchunitclass].units[patchunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    glTranslatef(-leftpatch*(colorbar_label_width + h_space), 0.0, 0.0);
    if(dohist == 1)glTranslatef(colorbar_label_width / 2.0, 0.0, 0.0);

    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Bndry");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, patchi->label.shortlabel);
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    glPopMatrix();
  }

  // -------------- zone left labels ------------

  if(showcfast_local==1){
    float tttval, tttmin, tttmax;

    char unitlabel[256];
    int zoneunitclass, zoneunittype;

    strcpy(unitlabel, (const char *)degC);
    GetUnitInfo(unitlabel, &zoneunitclass, &zoneunittype);
    if(zoneunitclass >= 0 && zoneunitclass < nunitclasses){
      if(zoneunittype > 0){
        zoneflag = 1;
        zonefactor = unitclasses[zoneunitclass].units[zoneunittype].scale;
        strcpy(unitlabel, unitclasses[zoneunitclass].units[zoneunittype].unit);
      }
    }

    iposition = -1;
    tttmin = zonelevels256[0];
    tttmax = zonelevels256[255];
    zonerange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-leftzone*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char zonecolorlabel[256], *zonecolorlabel_ptr = NULL, zonelabel[256];
      float vert_position;

      tttval = zonelevels256[valindex];
      Num2String(zonelabel, tttval);
      zonecolorlabel_ptr = &(zonelabel[0]);
      if(zoneflag == 1){
        ScaleFloat2String(tttval, zonecolorlabel, zonefactor);
        zonecolorlabel_ptr = zonecolorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, zonecolorlabel_ptr);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(zonecolorlabel_ptr));
    }
    for(i = 0; i < nrgb - 1; i++){
      float vert_position;
      char zonecolorlabel[256];
      char *zonecolorlabel_ptr = NULL;
      float val;

      vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      if(iposition == i)continue;
      if(zoneflag == 1){
        val = tttmin + (i - 1)*zonerange / (nrgb - 2);
      }
      else{
        val = colorvalueszone[i+1];
      }
      val = ScaleFloat2Float(val, zonefactor);
      Float2String(zonecolorlabel, val, ncolorlabel_digits, force_fixedpoint);
      zonecolorlabel_ptr = zonecolorlabel;
      OutputBarText(0.0, vert_position, foreground_color, zonecolorlabel_ptr);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(zonecolorlabel_ptr));
    }
    strcpy(exp_factor_label, "");
    SNIFF_ERRORS("after zone left labels");
    glPopMatrix();
  }

  // -------------- zone top labels ------------

  if(showcfast_local==1){
    char unitlabel[256];
    int zoneunitclass, zoneunittype;

    strcpy(unitlabel, (const char *)degC);
    GetUnitInfo(unitlabel, &zoneunitclass, &zoneunittype);
    if(zoneunitclass >= 0 && zoneunitclass < nunitclasses){
      if(zoneunittype > 0){
        zoneflag = 1;
        zonefactor = unitclasses[zoneunitclass].units[zoneunittype].scale;
        strcpy(unitlabel, unitclasses[zoneunitclass].units[zoneunittype].unit);
      }
    }
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, vcolorbar_top_pos + v_space + vcolorbar_delta, 0.0);
    glTranslatef(-leftzone*(colorbar_label_width + h_space), 0.0, 0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Zone");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, "Temp");
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    glPopMatrix();
    SNIFF_ERRORS("After ZONE labels");
  }

  // -------------- plot3d left labels ------------

  if(showplot3d == 1){
    float *p3lev;
    float tttval, tttmin, tttmax;
    char *up3label;
    char unitlabel[256];
    int plot3dunitclass, plot3dunittype;

    up3label = plot3dinfo[0].label[plotn-1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass>=0&&plot3dunitclass<nunitclasses){
      if(plot3dunittype>0){
        plot3dflag = 1;
        plot3dfactor = unitclasses[plot3dunitclass].units[plot3dunittype].scale;
        strcpy(unitlabel, unitclasses[plot3dunitclass].units[plot3dunittype].unit);
      }
    }

    iposition = -1;
    p3lev = p3levels256[plotn - 1];
    tttmin = p3lev[0];
    tttmax = p3lev[255];
    plot3drange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    if(global_colorbar_index != -1){
      char plot3dcolorlabel[256], p3dlabel[256], *plot3dcolorlabel_ptr = NULL;
      float vert_position;

      tttval = p3lev[valindex];
      Num2String(p3dlabel, tttval);
      plot3dcolorlabel_ptr = p3dlabel;
      if(plot3dflag == 1){
        ScaleFloat2String(tttval, plot3dcolorlabel, plot3dfactor);
        plot3dcolorlabel_ptr = plot3dcolorlabel;
      }
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, plot3dcolorlabel_ptr);
    }
    if(visiso == 0){
      for(i = 0; i < nrgb - 1; i++){
        float val;

        if(iposition == i)continue;
        if(plot3dflag == 1){
          val = tttmin + i*plot3drange / (nrgb - 2);
        }
        else{
          val = colorvaluesp3[plotn - 1][i];
        }
        val = ScaleFloat2Float(val, plot3dfactor);
        colorbar_vals[i] = val;
        GetMantissaExponent(ABS(val), colorbar_exponents + i);
      }
      Floats2Strings(colorbar_labels, colorbar_vals, nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
      for(i = 0; i < nrgb - 1; i++){
        float vert_position;

        vert_position = MIX2(i, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

        if(iposition == i)continue;
        OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
        max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
      }
    }
    else{
      float vert_position;

      for(i = 0; i < nrgb - 2; i++){
        float val;

        val = tttmin + (float)(i +0.5)*plot3drange / (nrgb - 2);
        colorbar_vals[i] = val;
      }
      Floats2Strings(colorbar_labels, colorbar_vals, nrgb-2, ncolorlabel_digits, force_fixedpoint, force_exponential, exp_factor_label);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
      for(i = 0; i < nrgb - 2; i++){
        vert_position = MIX2(i+0.5, nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
        if(iposition == i)continue;
        if(isolevelindex == i || isolevelindex2 == i){
          OutputBarText(0.0, vert_position, red_color, colorbar_labels[i]);
        }
        else{
          OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
        }
      }
    }
    glPopMatrix();
  }
  // -------------- plot3d top labels ------------

  if(showplot3d == 1){
    char *p3label;
    char *up3label;
    char unitlabel[256];
    int plot3dunitclass, plot3dunittype;

    up3label = plot3dinfo[0].label[plotn - 1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass >= 0 && plot3dunitclass < nunitclasses){
      if(plot3dunittype > 0){
        plot3dflag = 1;
        plot3dfactor = unitclasses[plot3dunitclass].units[plot3dunittype].scale;
        strcpy(unitlabel, unitclasses[plot3dunitclass].units[plot3dunittype].unit);
      }
    }
    p3label = plot3dinfo[0].label[plotn - 1].shortlabel;
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Plot3D");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, p3label);
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    if(strcmp(unitlabel, "ppm") == 0 && plot3dfactor != NULL){
      plot3dfactor2[0] = *plot3dfactor;
      plot3dfactor2[1] = 0.0;
      plot3dfactor = plot3dfactor2;
    }
    glPopMatrix();
  }
}

