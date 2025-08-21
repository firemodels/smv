#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "string_util.h"
#include "IOscript.h"
#include "viewports.h"

#include "colorbars.h"

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
      if(global_times!=NULL && nglobal_times>1){
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

  if(global_scase.hrrptr!=NULL&&global_times!=NULL&&vis_hrr_label==1){
    float hrr;
    int itime;

    itime = GetInterval(global_times[itimes], global_scase.timeptr->vals, global_scase.timeptr->nvals);
    hrr = global_scase.hrrptr->vals[itime];
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

  glLineWidth(global_scase.linewidth);
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
    cbi = colorbars.colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbars.colorbarinfo+colorbars.fire_colorbar_index;
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
    cbi = colorbars.colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbars.colorbarinfo+colorbars.fire_colorbar_index;
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
    glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
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
        vval_min=global_scase.hrrpuv_min;
        vval_cutoff=global_hrrpuv_cb_min;
        vval_max=global_scase.hrrpuv_max;
      }
      else{
        vval_min=global_scase.temp_min;
        vval_cutoff= global_temp_cb_min;
        vval_max= global_temp_cb_max;
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

/* ------------------ DrawColorbarPathCIELab ------------------------ */

void DrawColorbarPathCIELab(void){
  int i;
  colorbardata *cbi;

  if(show_firecolormap == 0){
    cbi = colorbars.colorbarinfo + colorbartype;
  }
  else{
    cbi = colorbars.colorbarinfo + colorbars.fire_colorbar_index;
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
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
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
/* ------------------ UpdateCurrentColorbar ------------------------ */

void UpdateCurrentColorbar(colorbardata *cb){
  current_colorbar = cb;
}

/* ------------------ UpdateColorbarOrig ------------------------ */

void UpdateColorbarOrig(void){
  int i;

  for(i = 0;i < colorbars.ncolorbars;i++){
    colorbardata *cbi;

    cbi = colorbars.colorbarinfo + i;
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
  cbi = colorbars.colorbarinfo + *i;
  cbj = colorbars.colorbarinfo + *j;
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
    cbt1 = colorbars.colorbarinfo + index_colorbar1;
    strcpy(toggle_label1,cbt1->menu_label);
  }
  strcpy(toggle_label2, "");
  if(index_colorbar2 >= 0){
    colorbardata *cbt2;

    cbt2 = colorbars.colorbarinfo + index_colorbar2;
    strcpy(toggle_label2, cbt2->menu_label);
  }
  strcpy(label_edit, "");
  if(colorbartype >= 0){
    colorbardata *cbt1;
    cbt1 = colorbars.colorbarinfo + colorbartype;
    strcpy(label_edit, cbt1->menu_label);
  }
  strcpy(label_bound, "");
  if(colorbartype >= 0){
    colorbardata *cbt1;
    cbt1 = colorbars.colorbarinfo + colorbartype;
    strcpy(label_bound, cbt1->menu_label);
  }

  FREEMEMORY(colorbar_list_sorted);
  NewMemory((void **)&colorbar_list_sorted, colorbars.ncolorbars*sizeof(int));
  FREEMEMORY(colorbar_list_inverse);
  NewMemory((void **)&colorbar_list_inverse, colorbars.ncolorbars*sizeof(int));
  for(i=0; i<colorbars.ncolorbars; i++){
    colorbardata *cbi;

    cbi = colorbars.colorbarinfo + i;
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
  qsort((colorbardata *)colorbar_list_sorted, (size_t)colorbars.ncolorbars, sizeof(int), CompareColorbars);
  for(i=0; i<colorbars.ncolorbars; i++){
    colorbar_list_inverse[colorbar_list_sorted[i]] = i;
  }

  colorbardata *cb;

  colorbars.bw_colorbar_index = -1;
  cb = GetColorbar(&colorbars, "black->white");
  if(cb != NULL)colorbars.bw_colorbar_index = cb - colorbars.colorbarinfo;

  // TODO: what if there is no "fire" colorbar
  cb = GetColorbar(&colorbars, "fire");
  colorbars.fire_colorbar_index=cb-colorbars.colorbarinfo;
  fire_colorbar=cb;

  cb = GetColorbar(&colorbars, "fire line (level set)");
  levelset_colorbar=cb-colorbars.colorbarinfo;

  colorbars.split_colorbar_index = -1;
  cb = GetColorbar(&colorbars, "split");
  split_colorbar=cb;
  if(cb != NULL)colorbars.split_colorbar_index = cb - colorbars.colorbarinfo;

  cb = GetColorbar(&colorbars, "CO2");
  colorbars.co2_colorbar_index = cb - colorbars.colorbarinfo;

  colorbartype       = colorbartype_default;
  colorbars.iso_colorbar_index = colorbartype_default;
  cb = NULL;
  if(strlen(toggle_label1)>0)cb = GetColorbar(&colorbars, toggle_label1);
  if(cb!=NULL)index_colorbar1 = cb - colorbars.colorbarinfo;

  cb = NULL;
  if(strlen(toggle_label2) > 0)cb = GetColorbar(&colorbars, toggle_label2);
  if(cb != NULL)index_colorbar2 = cb - colorbars.colorbarinfo;
  cb = NULL;
  if(strlen(label_edit) > 0)cb = GetColorbar(&colorbars, label_edit);
  if(cb != NULL)colorbartype = cb - colorbars.colorbarinfo;

  cb = NULL;
  if(strlen(label_bound) > 0)cb = GetColorbar(&colorbars, label_bound);
  if(cb != NULL)colorbartype= cb - colorbars.colorbarinfo;
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

  colorbars.ncolorbars++;
  CheckMemory;
  ResizeMemory((void **)&colorbars.colorbarinfo, colorbars.ncolorbars * sizeof(colorbardata));
  UpdateCurrentColorbar(colorbars.colorbarinfo + colorbartype);

  cb_from = colorbars.colorbarinfo + icolorbar;
  CheckMemory;

  // new colorbar

  cb_to = colorbars.colorbarinfo + colorbars.ncolorbars - 1;

  memcpy(cb_to, cb_from, sizeof(colorbardata));
  strcpy(cb_to->menu_label, cb_from->menu_label);
  strcat(cb_to->menu_label, "_copy");
  strcpy(cb_label, cb_to->menu_label);
  strcpy(cb_to->colorbar_type, "user defined");
  cb_to->interp = INTERP_LAB;
  cb_to->can_adjust = 1;
  RemapColorbar(cb_to, show_extreme_mindata, rgb_below_min,
                show_extreme_maxdata, rgb_above_max);
  UpdateColorbarDialogs();

  colorbardata *cbnew;

  cbnew = GetColorbar(&colorbars, cb_label);
  if(cbnew != NULL){
    colorbartype = cbnew - colorbars.colorbarinfo;
  }
  GLUISetColorbarListEdit(colorbartype);
  GLUISetColorbarListBound(colorbartype);
  return colorbartype;
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

  if(global_scase.hvaccoll.hvacductvar_index >= 0)*show_hvacduct_colorbar_arg = 1;
  if(global_scase.hvaccoll.hvacnodevar_index >= 0)*show_hvacnode_colorbar_arg = 1;
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

    right_hat = hcolorbar_right_pos*(float)(global_scase.nrgb - 3) / (float)(global_scase.nrgb - 2)+hcolorbar_left_pos/(float)(global_scase.nrgb-2);

    glBegin(GL_QUADS);
    for(i = 0; i < global_scase.nrgb - 2; i++){
      float *rgb_plot3d_local;
      float xleft, xright;

      rgb_plot3d_local = rgb_plot3d_contour[i];

      xleft = MIX2(i, global_scase.nrgb - 3, right_hat, hcolorbar_left_pos);
      xright = MIX2(i + 1, global_scase.nrgb - 3, right_hat, hcolorbar_left_pos);

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

      rgb_plot3d_local = rgb_plot3d_contour[global_scase.nrgb - 2];
      barmid = (hcolorbar_down_pos + hcolorbar_top_pos) / 2.0;
      i = -1;
      xright = MIX2(i + 0.5, global_scase.nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);
      xleft  = MIX2(i + 1, global_scase.nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);

      if(have_extreme_mindata == 1 || have_extreme_maxdata == 1)glEnable(GL_POLYGON_SMOOTH);

      if(show_extreme_mindata == 1 && have_extreme_mindata == 1 && rgb_plot3d_local[3] != 0.0){
        glBegin(GL_TRIANGLES);
        glColor4fv(rgb_plot3d_local);

        glVertex2f(xleft, hcolorbar_down_pos);
        glVertex2f(xleft, hcolorbar_top_pos);
        glVertex2f(xright, barmid);
        glEnd();
      }

      i = global_scase.nrgb - 2;
      xleft = MIX2(i, global_scase.nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);
      xright = MIX2(i + 0.5, global_scase.nrgb - 3, hcolorbar_right_pos, hcolorbar_left_pos);

      rgb_plot3d_local = rgb_plot3d_contour[global_scase.nrgb - 1];
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

      top_hat = vcolorbar_top_pos*(float)(global_scase.nrgb - 3) / (float)(global_scase.nrgb - 2)+vcolorbar_down_pos/(float)(global_scase.nrgb-2);

      glBegin(GL_QUADS);
      for(i = 0; i < global_scase.nrgb-2; i++){
        float *rgb_plot3d_local;
        float ybot, ytop;

        rgb_plot3d_local = rgb_plot3d_contour[i];
        ybot = MIX2(  i,global_scase.nrgb-3,top_hat,vcolorbar_down_pos);
        ytop = MIX2(i+1,global_scase.nrgb-3,top_hat,vcolorbar_down_pos);

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

        rgb_plot3d_local = rgb_plot3d_contour[global_scase.nrgb-2];
        barmid = (vcolorbar_left_pos+vcolorbar_right_pos)/2.0;
        i=-1;
        ytop = MIX2(i+0.5,global_scase.nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);
        ybot = MIX2(i+1,global_scase.nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);

        if(have_extreme_mindata==1||have_extreme_maxdata==1)glEnable(GL_POLYGON_SMOOTH);

        if(show_extreme_mindata==1&&have_extreme_mindata==1&&rgb_plot3d_local[3]!=0.0){
          glBegin(GL_TRIANGLES);
          glColor4fv(rgb_plot3d_local);

          glVertex2f(vcolorbar_left_pos,ybot);
          glVertex2f(barmid,ytop);
          glVertex2f(vcolorbar_right_pos,ybot);
          glEnd();
        }

        i=global_scase.nrgb-2;
        ybot = MIX2(i,global_scase.nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);
        ytop = MIX2(i+0.5,global_scase.nrgb-3,vcolorbar_top_pos,vcolorbar_down_pos);

        rgb_plot3d_local = rgb_plot3d_contour[global_scase.nrgb-1];
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

      GetUnitInfo(partunitlabel, &partunitclass, &partunittype);
      if(partunitclass >= 0 && partunitclass < nunitclasses && partunittype >= 0){
        partflag = 1;
        partfactor = unitclasses[partunitclass].units[partunittype].scale;
        strcpy(partunitlabel2, unitclasses[partunitclass].units[partunittype].unit);
      }
      OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, partshortlabel2);
      OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, partunitlabel2);
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

    patchi = global_scase.patchinfo + global_scase.boundarytypes[iboundarytype];
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

    up3label = global_scase.plot3dinfo[0].label[plotn - 1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass >= 0 && plot3dunitclass < nunitclasses){
      if(plot3dunittype >= 0){
        plot3dflag = 1;
        plot3dfactor = unitclasses[plot3dunitclass].units[plot3dunittype].scale;
        strcpy(unitlabel, unitclasses[plot3dunitclass].units[plot3dunittype].unit);
      }
    }
    p3label = global_scase.plot3dinfo[0].label[plotn - 1].shortlabel;
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
      Float2String(isolabel, tttval, ncolorlabel_digits, force_fixedpoint);
      ScaleFloat2String(tttval, isocolorlabel, isofactor, ncolorlabel_digits, force_fixedpoint);
      isocolorlabel_ptr = isocolorlabel;
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, isocolorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float horiz_position;
      char isocolorlabel[256];
      char *isocolorlabel_ptr = NULL;
      float val;

      horiz_position = MIX2(i, global_scase.nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
      if(iposition == i)continue;

      val = tttmin + i*isorange / (global_scase.nrgb - 2);
      ScaleFloat2String(val, isocolorlabel, isofactor, ncolorlabel_digits, force_fixedpoint);
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
      Float2String(partlabel, tttval, ncolorlabel_digits, force_fixedpoint);
      partcolorlabel_ptr = partlabel;
      if(partflag == 1){
        ScaleFloat2String(tttval, partcolorlabel, partfactor, ncolorlabel_digits, force_fixedpoint);
        partcolorlabel_ptr = partcolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, partcolorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float horiz_position;
      char partcolorlabel[256];
      char *partcolorlabel_ptr = NULL;

      horiz_position = MIX2(i, global_scase.nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
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
        val = tttmin + i*partrange / (global_scase.nrgb - 2);
        val = ScaleFloat(val, partfactor);
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
      Float2String(slicelabel, tttval, ncolorlabel_digits, force_fixedpoint);
      slicecolorlabel_ptr = slicelabel;
      if(sliceflag == 1){
        ScaleFloat2String(tttval, slicecolorlabel, slicefactor, ncolorlabel_digits, force_fixedpoint);
        slicecolorlabel_ptr = slicecolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, slicecolorlabel_ptr);
    }
    {
      for(i = 0; i < global_scase.nrgb - 1; i++){
        float horiz_position, val;
        char slicecolorlabel[256];
        char *slicecolorlabel_ptr = NULL;

        horiz_position = MIX2(i, global_scase.nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
        if(iposition == i)continue;
        if(sliceflag == 1){
          val = tttmin + i*slicerange / (global_scase.nrgb - 2);
        }
        else{
          val = sb->colorvalues[i+1];
        }
        val = ScaleFloat(val, slicefactor);
        slicecolorlabel_ptr = slicecolorlabel;
        Float2String(slicecolorlabel_ptr, val, ncolorlabel_digits, force_fixedpoint);
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
      Float2String(boundarylabel, tttval, ncolorlabel_digits, force_fixedpoint);
      boundary_colorlabel_ptr = &(boundarylabel[0]);
      if(patchflag == 1){
        ScaleFloat2String(tttval, boundary_colorlabel, patchfactor, ncolorlabel_digits, force_fixedpoint);
        boundary_colorlabel_ptr = boundary_colorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, boundary_colorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      char boundary_colorlabel[256];
      char *boundary_colorlabel_ptr = NULL;
      float horiz_position;
      float val;

      horiz_position = MIX2(i, global_scase.nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);


      if(iposition == i)continue;
      if(patchflag == 1){
        val = tttmin + i*patchrange / (global_scase.nrgb - 2);
      }
      else{
        val = colorvaluespatch[i+1];
      }
      val = ScaleFloat(val, patchfactor);
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
      Float2String(zonelabel, tttval, ncolorlabel_digits, force_fixedpoint);
      zonecolorlabel_ptr = &(zonelabel[0]);
      if(zoneflag == 1){
        ScaleFloat2String(tttval, zonecolorlabel, zonefactor, ncolorlabel_digits, force_fixedpoint);
        zonecolorlabel_ptr = zonecolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, zonecolorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float horiz_position;
      char zonecolorlabel[256];
      char *zonecolorlabel_ptr = NULL;

      horiz_position = MIX2(i, global_scase.nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
      if(iposition == i)continue;
      zonecolorlabel_ptr = &global_scase.colorlabelzone[i + 1][0];
      if(zoneflag == 1){
        float val;

        val = tttmin + (i - 1)*zonerange / (global_scase.nrgb - 2);
        ScaleFloat2String(val, zonecolorlabel, zonefactor, ncolorlabel_digits, force_fixedpoint);
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
      Float2String(p3dlabel, tttval, ncolorlabel_digits, force_fixedpoint);
      plot3dcolorlabel_ptr = p3dlabel;
      if(plot3dflag == 1){
        ScaleFloat2String(tttval, plot3dcolorlabel, plot3dfactor, ncolorlabel_digits, force_fixedpoint);
        plot3dcolorlabel_ptr = plot3dcolorlabel;
      }
      horiz_position = MIX2(global_colorbar_index, 255, hcolorbar_right_pos, hcolorbar_left_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(horiz_position, 0.0, red_color, plot3dcolorlabel_ptr);
    }
    if(visiso == 0){
      float horiz_position;

      for(i = 0; i < global_scase.nrgb - 1; i++){
        char plot3dcolorlabel[256];
        char *plot3dcolorlabel_ptr = NULL;
        float val;

        horiz_position = MIX2(i, global_scase.nrgb - 2, hcolorbar_right_pos, hcolorbar_left_pos);
        if(iposition == i)continue;
        if(plot3dflag == 1){
          val = tttmin + i*plot3drange / (global_scase.nrgb - 2);
          if(plot3dfactor != NULL)val = plot3dfactor[0]*val + plot3dfactor[1];
        }
        else{
          val = colorvaluesp3[plotn - 1][i];
        }
        plot3dcolorlabel_ptr = plot3dcolorlabel;
        Float2String(plot3dcolorlabel, val, ncolorlabel_digits, force_fixedpoint);
        OutputBarText(horiz_position, 0.0, foreground_color, plot3dcolorlabel_ptr);
      }
    }
    else{
      float horiz_position;
      float right_hat;

      right_hat = hcolorbar_right_pos*(float)(global_scase.nrgb - 3) / (float)(global_scase.nrgb - 2)+hcolorbar_left_pos/(float)(global_scase.nrgb-2);
      for(i = 0; i < global_scase.nrgb - 2; i++){
        char plot3dcolorlabel[256];
        char *plot3dcolorlabel_ptr = NULL;

        horiz_position = MIX2(i, global_scase.nrgb - 2, right_hat, hcolorbar_left_pos);

        if(iposition == i)continue;
        plot3dcolorlabel_ptr = &colorlabeliso[plotn - 1][i][0];
        if(plot3dflag == 1){
          float val;

          val = tttmin + (i - 1)*plot3drange / (global_scase.nrgb - 2);
          ScaleFloat2String(val, plot3dcolorlabel, plot3dfactor, ncolorlabel_digits, force_fixedpoint);
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

  int dohist = 0;

  GLfloat *foreground_color, *red_color;

  int showcfast_local = 0;
  int show_slice_colorbar_local = 0;
  int show_hvacduct_colorbar_local = 0;
  int show_hvacnode_colorbar_local = 0;
  char exp_factor_label[256];

  max_colorbar_label_width = GetStringWidth("123456");

  UpdateShowColorbar(&showcfast_local, &show_slice_colorbar_local,
    &show_hvacduct_colorbar_local, &show_hvacnode_colorbar_local);

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

  // -------------- isosurface left labels ------------

  if(showiso_colorbar == 1){
    float tttval, tttmin, tttmax;
    boundsdata *sb;
    float isorange;
    float *isofactor = NULL;
    int isounitclass, isounittype;
    char unitlabel[256];

    sb = isobounds + iisottype;
    strcpy(unitlabel, sb->label->unit);
    GetUnitInfo(sb->label->unit, &isounitclass, &isounittype);
    if(isounitclass >= 0 && isounitclass < nunitclasses){
      if(isounittype > 0){
        isofactor = unitclasses[isounitclass].units[isounittype].scale;
        strcpy(unitlabel, unitclasses[isounitclass].units[isounittype].unit);
      }
    }
    tttmin = iso_valmin;
    tttmax = iso_valmax;
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
      char *isocolorlabel_ptr = NULL;
      float vert_position;

      tttval = ScaleFloat(sb->levels256[valindex], isofactor);
      Floats2Strings(colorbar_labels, &tttval, 1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      isocolorlabel_ptr = colorbar_labels[0];
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, isocolorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      val = tttmin + i*isorange / (global_scase.nrgb - 2);;
      val = ScaleFloat(val, isofactor);
      colorbar_vals[i] = val;
    }
    Floats2Strings(colorbar_labels, colorbar_vals, global_scase.nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float vert_position;
      char *isocolorlabel_ptr = NULL;

      vert_position = MIX2(i, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      if(iposition == i)continue;

      isocolorlabel_ptr = colorbar_labels[i];
      OutputBarText(0.0, vert_position, foreground_color, isocolorlabel_ptr);
    }
    glPopMatrix();

  // -------------- isosurface top labels ------------

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
    float *partfactor = NULL;

    if(parttype!=0){
      int partunitclass, partunittype;

      GetUnitInfo(partunitlabel, &partunitclass, &partunittype);
      if(partunitclass>=0&&partunitclass<nunitclasses&&partunittype>=0){
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
      char *partcolorlabel_ptr = NULL;
      float vert_position;

      tttval = ScaleFloat(partlevels256_ptr[valindex], partfactor);
      Floats2Strings(colorbar_labels, &tttval, 1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      partcolorlabel_ptr = colorbar_labels[0];
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, partcolorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      val = part5propinfo[global_prop_index].partlabelvals[i + 1];
      val = ScaleFloat(val, partfactor);
      colorbar_vals[i] = val;
    }
    Floats2Strings(colorbar_labels, colorbar_vals, global_scase.nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
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
    float *slicefactor = NULL;

    char unitlabel[256];
    int sliceunitclass, sliceunittype;

    sb = slicebounds + slicefile_labelindex;
    strcpy(unitlabel, sb->label->unit);
    GetUnitInfo(sb->label->unit, &sliceunitclass, &sliceunittype);
    if(sliceunitclass >= 0 && sliceunitclass < nunitclasses){
      if(sliceunittype > 0){
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
      char *slicecolorlabel_ptr = NULL;
      float vert_position;
      int shifted_colorbar_index;

      tttval = sb->levels256[valindex];
      shifted_colorbar_index = global_colorbar_index;
      if(ABS(colorbar_shift-1.0)>0.0001){
        shifted_colorbar_index = SHIFT_VAL(global_colorbar_index, 0, 255, colorbar_shift);
      }
      tttval = ScaleFloat(tttval, slicefactor);
      Floats2Strings(colorbar_labels, &tttval, 1, ncolorlabel_digits, force_fixedpoint, force_exponential,
                     force_decimal, force_zero_pad, exp_factor_label);
      slicecolorlabel_ptr = colorbar_labels[0];
      vert_position = MIX2(shifted_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(shifted_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, slicecolorlabel_ptr);
    }
    {
      float valmin, valmax;


      if(slicefactor!=NULL){
        valmin = tttmin;
      }
      else{
        valmin = sb->colorvalues[1];
      }
      valmin = ScaleFloat(valmin, slicefactor);

      if(slicefactor!=NULL){
        valmax = tttmax;
      }
      else{
        valmax = sb->colorvalues[global_scase.nrgb-1];
      }
      valmax = ScaleFloat(valmax, slicefactor);

      for(i = 0; i<global_scase.nrgb-1; i++){
        float val;

        if(iposition==i)continue;
        if(slicefactor!=NULL){
          val = tttmin+i*slicerange/(global_scase.nrgb-2);
        }
        else{
          val = sb->colorvalues[i+1];
        }
        val = ScaleFloat(val, slicefactor);
        if(ABS(colorbar_shift-1.0)>0.0001){
          val = SHIFT_VAL(val, valmin, valmax, 1.0/colorbar_shift);
        }
        colorbar_vals[i] = val;
      }
      Floats2Strings(colorbar_labels, colorbar_vals, global_scase.nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
      for(i = 0; i < global_scase.nrgb - 1; i++){
        float vert_position;

        vert_position = MIX2(i, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
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
    glPopMatrix();
  }

  // -------------- HVAC node left labels ------------

  if(show_hvacnode_colorbar_local==1 && global_scase.hvaccoll.hvacnodevar_index>=0){
    hvacvaldata *hi;
    float tttval, tttmin, tttmax;
    float hvacrange;

    hi = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + global_scase.hvaccoll.hvacnodevar_index;
    iposition = -1;
    tttmin = hi->levels256[0];
    tttmax = hi->levels256[255];
    hvacrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-lefthvacnode*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char *hvac_colorlabel_ptr = NULL;
      float vert_position;

      // draw hvac file value selected with mouse
      tttval = hi->levels256[valindex];
      Floats2Strings(colorbar_labels, &tttval, 1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      hvac_colorlabel_ptr = colorbar_labels[0];
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, hvac_colorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      val = tttmin + i * hvacrange / (global_scase.nrgb - 2);
      colorbar_vals[i] = val;
      GetMantissaExponent(ABS(val), colorbar_exponents + i);
    }
    Floats2Strings(colorbar_labels, colorbar_vals, global_scase.nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

      if(iposition == i)continue;
      OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
    }
    glPopMatrix();
  }

  // -------------- HVAC duct left labels ------------

  if(show_hvacduct_colorbar_local==1 && global_scase.hvaccoll.hvacductvar_index>=0){
    hvacvaldata *hi;
    float tttval, tttmin, tttmax;
    float hvacrange;

    hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + global_scase.hvaccoll.hvacductvar_index;
    iposition = -1;
    tttmin = hi->levels256[0];
    tttmax = hi->levels256[255];
    hvacrange = tttmax - tttmin;
    glPushMatrix();
    glTranslatef(vcolorbar_left_pos - colorbar_label_width, -VP_vcolorbar.text_height / 2.0, 0.0);
    glTranslatef(-lefthvacduct*(colorbar_label_width + h_space), 0.0, 0.0);
    if(global_colorbar_index != -1){
      char *hvac_colorlabel_ptr = NULL;
      float vert_position;

      // draw hvac file value selected with mouse
      tttval = hi->levels256[valindex];
      Floats2Strings(colorbar_labels, &tttval, 1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      hvac_colorlabel_ptr = colorbar_labels[0];
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, hvac_colorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      val = tttmin + i * hvacrange / (global_scase.nrgb - 2);
      colorbar_vals[i] = val;
      GetMantissaExponent(ABS(val), colorbar_exponents + i);
    }
    Floats2Strings(colorbar_labels, colorbar_vals, global_scase.nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

      if(iposition == i)continue;
      OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
    }
    glPopMatrix();
  }

  // -------------- HVAC file node top labels ------------

  if(show_hvacnode_colorbar_local==1 && global_scase.hvaccoll.hvacnodevar_index>=0){
    char *slabel, *unitlabel;
    hvacvaldata *hi;

    hi = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + global_scase.hvaccoll.hvacnodevar_index;
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

  if(show_hvacduct_colorbar_local==1 && global_scase.hvaccoll.hvacductvar_index >=0){
    char *slabel, *unitlabel;
    hvacvaldata *hi;

    hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + global_scase.hvaccoll.hvacductvar_index;
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
    float *patchfactor=NULL;
    float patchrange = 0.0;

    char unitlabel[256];
    patchdata *patchi;
    int patchunitclass, patchunittype;

    patchi = global_scase.patchinfo + global_scase.boundarytypes[iboundarytype];
    strcpy(unitlabel, patchi->label.unit);
    GetUnitInfo(patchi->label.unit, &patchunitclass, &patchunittype);
    if(patchunitclass >= 0 && patchunitclass < nunitclasses){
      if(patchunittype > 0){
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
      char *boundary_colorlabel_ptr = NULL;
      float vert_position;

      // draw boundary file value selected with mouse
      tttval = ScaleFloat(boundarylevels256[valindex], patchfactor);
      Floats2Strings(colorbar_labels, &tttval, 1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      boundary_colorlabel_ptr = colorbar_labels[0];
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, boundary_colorlabel_ptr);
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float val;

      if(iposition == i)continue;
      if(patchfactor!=NULL){
        val = tttmin+i*patchrange/(global_scase.nrgb-2);
      }
      else{
        val = colorvaluespatch[i+1];
      }
      val = ScaleFloat(val, patchfactor);
      colorbar_vals[i] = val;
      GetMantissaExponent(ABS(val), colorbar_exponents + i);
    }
    Floats2Strings(colorbar_labels, colorbar_vals, global_scase.nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
    max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float vert_position;

      vert_position = MIX2(i, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

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

    patchi = global_scase.patchinfo + global_scase.boundarytypes[iboundarytype];
    strcpy(unitlabel, patchi->label.unit);
    GetUnitInfo(patchi->label.unit, &patchunitclass, &patchunittype);
    if(patchunitclass >= 0 && patchunitclass < nunitclasses){
      if(patchunittype > 0){
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
    float *zonefactor = NULL;
    float zonerange;

    char unitlabel[256];
    int zoneunitclass, zoneunittype;

    strcpy(unitlabel, (const char *)degC);
    GetUnitInfo(unitlabel, &zoneunitclass, &zoneunittype);
    if(zoneunitclass >= 0 && zoneunitclass < nunitclasses){
      if(zoneunittype > 0){
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
      char *zonecolorlabel_ptr = NULL, zonelabel[256];
      float vert_position;

      tttval = ScaleFloat(zonelevels256[valindex], zonefactor);
      Float2String(zonelabel, tttval, ncolorlabel_digits, force_fixedpoint);
      zonecolorlabel_ptr = &(zonelabel[0]);
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, zonecolorlabel_ptr);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(zonecolorlabel_ptr));
    }
    for(i = 0; i < global_scase.nrgb - 1; i++){
      float vert_position;
      char zonecolorlabel[256];
      char *zonecolorlabel_ptr = NULL;
      float val;

      vert_position = MIX2(i, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
      if(iposition == i)continue;
      if(zonefactor != NULL){
        val = tttmin + i*zonerange / (global_scase.nrgb - 2);
      }
      else{
        val = colorvalueszone[i+1];
      }
      val = ScaleFloat(val, zonefactor);
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
    float *plot3dfactor = NULL;
    float plot3drange;

    up3label = global_scase.plot3dinfo[0].label[plotn-1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass>=0&&plot3dunitclass<nunitclasses){
      if(plot3dunittype>0){
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
      char *plot3dcolorlabel_ptr = NULL;
      float vert_position;

      tttval = ScaleFloat(p3lev[valindex],plot3dfactor);
      Floats2Strings(colorbar_labels, &tttval, 1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      plot3dcolorlabel_ptr = colorbar_labels[0];
      vert_position = MIX2(global_colorbar_index, 255, vcolorbar_top_pos, vcolorbar_down_pos);
      iposition = MIX2(global_colorbar_index, 255, global_scase.nrgb - 1, 0);
      OutputBarText(0.0, vert_position, red_color, plot3dcolorlabel_ptr);
    }
    if(visiso == 0){
      for(i = 0; i < global_scase.nrgb - 1; i++){
        float val;

        if(iposition == i)continue;
        if(plot3dfactor!=NULL){
          val = tttmin + i*plot3drange / (global_scase.nrgb - 2);
        }
        else{
          val = colorvaluesp3[plotn - 1][i];
        }
        val = ScaleFloat(val, plot3dfactor);
        colorbar_vals[i] = val;
        GetMantissaExponent(ABS(val), colorbar_exponents + i);
      }
      Floats2Strings(colorbar_labels, colorbar_vals, global_scase.nrgb-1, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
      for(i = 0; i < global_scase.nrgb - 1; i++){
        float vert_position;

        vert_position = MIX2(i, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);

        if(iposition == i)continue;
        OutputBarText(0.0, vert_position, foreground_color, colorbar_labels[i]);
        max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(colorbar_labels[i]));
      }
    }
    else{
      float vert_position;

      for(i = 0; i < global_scase.nrgb - 2; i++){
        float val;

        val = tttmin + (float)(i +0.5)*plot3drange / (global_scase.nrgb - 2);
        colorbar_vals[i] = val;
      }
      Floats2Strings(colorbar_labels, colorbar_vals, global_scase.nrgb-2, ncolorlabel_digits, force_fixedpoint, force_exponential, force_decimal, force_zero_pad, exp_factor_label);
      max_colorbar_label_width = MAX(max_colorbar_label_width, GetStringWidth(exp_factor_label));
      for(i = 0; i < global_scase.nrgb - 2; i++){
        vert_position = MIX2(i+0.5, global_scase.nrgb - 2, vcolorbar_top_pos, vcolorbar_down_pos);
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

    up3label = global_scase.plot3dinfo[0].label[plotn - 1].unit;
    strcpy(unitlabel, up3label);
    GetUnitInfo(up3label, &plot3dunitclass, &plot3dunittype);
    if(plot3dunitclass >= 0 && plot3dunitclass < nunitclasses){
      if(plot3dunittype > 0){
        strcpy(unitlabel, unitclasses[plot3dunitclass].units[plot3dunittype].unit);
      }
    }
    p3label = global_scase.plot3dinfo[0].label[plotn - 1].shortlabel;
    glPushMatrix();
    glTranslatef(
      vcolorbar_left_pos - colorbar_label_width,
      vcolorbar_top_pos + v_space + vcolorbar_delta,
      0.0);
    OutputBarText(0.0, 3 * (VP_vcolorbar.text_height + v_space), foreground_color, "Plot3D");
    OutputBarText(0.0, 2 * (VP_vcolorbar.text_height + v_space), foreground_color, p3label);
    OutputBarText(0.0,     (VP_vcolorbar.text_height + v_space), foreground_color, unitlabel);
    OutputBarText(0.0, 0                                       , foreground_color, exp_factor_label);
    glPopMatrix();
  }
}
