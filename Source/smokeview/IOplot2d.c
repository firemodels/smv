#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "datadefs.h"
#include "smokeviewvars.h"

#ifdef pp_PLOT2D_NEW

/* ------------------ HaveGenDev ------------------------ */

int HaveGenDev(void){
  int i;

  for(i = 0; i<glui_plot2dinfo->ncurve_indexes; i++){
    if(glui_plot2dinfo->curve[i].index<ndeviceinfo)return 1;
  }
  return 0;
}

/* ------------------ HaveGenHrr ------------------------ */

int HaveGenHrr(void){
  int i;

  for(i = 0; i<glui_plot2dinfo->ncurve_indexes; i++){
    if(glui_plot2dinfo->curve[i].index>=ndeviceinfo)return 1;
  }
  return 0;
}

/* ------------------ DrawPlot ------------------------ */
#define AXIS_LEFT  0
#define AXIS_RIGHT 1
#define AXIS_NONE  2
void DrawGenCurve(int option, plot2ddata *plot2di, curvedata *curve, float size_factor,
              float *x, float *z, int n, float x_cur, float z_cur, float zmin, float zmax,
              char *label, int position, int axis_side, char *unit){
  float xmin, xmax, dx, dz;
  float xscale = 1.0, zscale = 1.0;
  int i, ndigits = 3;

  float *xyz0, linewidth_arg, *plot_factors;
  int *plot_color, use_plot_factors, show_title;
  char *title;

  xyz0             = plot2di->xyz;
  plot_color       = curve->color;
  linewidth_arg    = curve->linewidth;
  plot_factors     = curve->factors;
  use_plot_factors = curve->use_factors;
  title            = plot2di->plot_label;
  show_title       = plot2di->show_title;

  xmin = x[0];
  xmax = xmin;
  for(i = 1; i < n; i++){
    xmin = MIN(xmin, x[i]);
    xmax = MAX(xmax, x[i]);
    }
  if(xmax == xmin)xmax = xmin + 1.0;
  if(xmax > xmin)xscale = 1.0 / (xmax - xmin);

  if(zmax == zmin)zmax = zmin + 1.0;
  if(zmax > zmin)zscale = 1.0 / (zmax - zmin);

  dx = (xmax - xmin) / 20.0;
  dz = (zmax - zmin) / 20.0;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(SCALE2FDS(plot2d_xyz_offset[0]), SCALE2FDS(plot2d_xyz_offset[1]), SCALE2FDS(plot2d_xyz_offset[2]));

  glTranslatef(xyz0[0], xyz0[1], xyz0[2]);

  float az = camera_current->az_elev[0];
  glRotatef(-az, 0.0, 0.0, 1.0);

  float elev = camera_current->az_elev[1];
  glRotatef(-elev, 1.0, 0.0, 0.0);

  glScalef(SCALE2FDS(size_factor), SCALE2FDS(size_factor), SCALE2FDS(size_factor));
  glScalef(xscale, 1.0, zscale);
  glTranslatef(-xmin, 0.0, -zmin);
  glColor3ub((unsigned char)plot_color[0], (unsigned char)plot_color[1], (unsigned char)plot_color[2] );
  glLineWidth(linewidth_arg);
  glBegin(GL_LINES);
  if(use_plot_factors == 1){
    for(i = 0; i < n - 1; i++){
      glVertex3f(    x[i], 0.0, plot_factors[0]*z[i]   + plot_factors[1]);
      glVertex3f(x[i + 1], 0.0, plot_factors[0]*z[i+1] + plot_factors[1]);
    }
  }
  else{
    for(i = 0; i < n - 1; i++){
      glVertex3f(x[i], 0.0, z[i]);
      glVertex3f(x[i + 1], 0.0, z[i + 1]);
    }
  }
  glEnd();
  SNIFF_ERRORS("after DrawGenCurve 1");
  if(option == PLOT_ALL){
    glColor3fv(foregroundcolor);
    glLineWidth(plot2d_line_width);
    glBegin(GL_LINES);
    glVertex3f(xmin, 0.0, zmin);
    glVertex3f(xmax, 0.0, zmin);

    glVertex3f(xmax, 0.0, zmin);
    glVertex3f(xmax, 0.0, zmax);

    glVertex3f(xmax, 0.0, zmax);
    glVertex3f(xmin, 0.0, zmax);

    glVertex3f(xmin, 0.0, zmax);
    glVertex3f(xmin, 0.0, zmin);

    glVertex3f(xmax, 0.0, zmax);
    glVertex3f(xmax, 0.0, zmax);

    glVertex3f(xmax, 0.0, zmin);
    glVertex3f(xmax, 0.0, zmin);
    glEnd();
    SNIFF_ERRORS("after DrawGenCurve 2");
  }

  glColor3f(1.0, 0.0, 0.0);
  glPointSize(plot2d_point_size);
  glBegin(GL_POINTS);
  if(use_plot_factors == 1){
    glVertex3f(x_cur, 0.0, plot_factors[0] * z_cur + plot_factors[1]);
  }
  else{
    glVertex3f(x_cur, 0.0, z_cur);
  }
  glEnd();
  SNIFF_ERRORS("after DrawGenCurve 3");

  if(showd_plot2d_labels == 1){
    float dfont = (float)GetFontHeight() / ((float)screenHeight * zscale * SCALE2FDS(size_factor) * SCALE2SMV(1.0));

    if(option == PLOT_ALL){
      char c_tmin[32], c_tmax[32];

      Float2String(c_tmin, x[0], ndigits, force_fixedpoint);
      Output3Text(foregroundcolor, xmin, 0.0, zmin - dz - dfont, c_tmin);

      Float2String(c_tmax, x[n - 1], ndigits, force_fixedpoint);
      Output3Text(foregroundcolor, xmax, 0.0, zmin - dz - dfont, c_tmax);
      if(show_title==1){
        Output3Text(foregroundcolor, xmin, 0.0, zmax + 1.5*dz, title);
      }
      SNIFF_ERRORS("after DrawGenCurve 4");
    }
    if(label != NULL){
      float p2_color[3];
      char label2[64], c_zcur[32];
      char c_zmin[32], c_zmax[32];

      Float2String(c_zmin, zmin, ndigits, force_fixedpoint);
      Float2String(c_zmax, zmax, ndigits, force_fixedpoint);
      Float2String(c_zcur, z_cur, ndigits, force_fixedpoint);
      strcpy(label2, label);
      strcat(label2, "/");
      strcat(label2, c_zcur);

      p2_color[0] = (float)plot_color[0]/255.0;
      p2_color[1] = (float)plot_color[1]/255.0;
      p2_color[2] = (float)plot_color[2]/255.0;
      if(axis_side == AXIS_LEFT){
        Output3Text(p2_color, xmax + 2.0 * dx, 0.0, zmax - (0.5 + plot2d_font_spacing * (float)position) * dfont, label2);
        Output3Text(p2_color, xmax + 2.0 * dx, 0.0, zmin,  c_zmin);
        Output3Text(p2_color, xmax + 2.0 * dx, 0.0, zmax , c_zmax);
        }
      else{
        Output3TextRight(p2_color, xmin - dx, 0.0, zmax - (0.5 + plot2d_font_spacing * (float)position) * dfont, label2, 3);
        Output3TextRight(p2_color, xmin - dx, 0.0, zmin,  c_zmin, 3);
        Output3TextRight(p2_color, xmin - dx, 0.0, zmax , c_zmax, 3);
        }
      SNIFF_ERRORS("after DrawGenCurve 5");
    }
    if(unit!=NULL){
      if(axis_side == AXIS_LEFT){
        Output3Text(foregroundcolor, xmax + 2.0 * dx, 0.0, zmax - (0.5 + plot2d_font_spacing * (float)(position+1)) * dfont, unit);
      }
      else{
        Output3TextRight(foregroundcolor, xmin - dx, 0.0, zmax - (0.5 + plot2d_font_spacing * (float)(position+1)) * dfont, unit, 3);
      }
    }
  }
  glPopMatrix();
  SNIFF_ERRORS("after DrawGenCurve end");
}

/* ------------------ DrawGenPlot ------------------------ */

void DrawGenPlot(plot2ddata * plot2di){
  int i;
  char *axis_right_unit = NULL, *axis_left_unit = NULL;
  float axis_left_min   = 1.0,  axis_left_max  = 0.0;
  float axis_right_min  = 1.0,  axis_right_max = 0.0;
  int left_position  = 0;
  int right_position = 0;
  int unit_left_index=0, unit_right_index=0;

  for(i = 0; i < plot2di->ncurve_indexes; i++){
    int curve_index;
    char *unit;

    curve_index = plot2di->curve[i].index;
    if(curve_index < ndeviceinfo){
      unit = deviceinfo[curve_index].unit;
    }
    else{
      unit = hrrinfo[curve_index - ndeviceinfo].label.unit;
    }
    if(unit==NULL||strlen(unit)==0)continue;
    if(axis_right_unit == NULL){
      axis_right_unit = unit;
      continue;
    }
    if(strcmp(unit, axis_right_unit) != 0){
      axis_left_unit = unit;
      break;
    }
  }
  for(i = 0; i < plot2di->ncurve_indexes; i++){
    int curve_index;
    char *unit;

    curve_index = plot2di->curve[i].index;
    if(curve_index < ndeviceinfo){
      unit = deviceinfo[curve_index].unit;
    }
    else{
      unit = hrrinfo[curve_index - ndeviceinfo].label.unit;
    }
    if(unit == NULL || strlen(unit) == 0)continue;
    if(axis_right_unit != NULL && strcmp(axis_right_unit, unit) == 0){
      unit_right_index = i;
      continue;
    }
    if(axis_left_unit != NULL && strcmp(axis_left_unit, unit) == 0){
      unit_left_index = i;
      continue;
    }
  }

  for(i = 0; i < plot2di->ncurve_indexes; i++){
    int curve_index;
    float valmin, valmax;
    char *unit;
    curvedata *curve;

    curve_index = plot2di->curve[i].index;
    curve = plot2di->curve + curve_index;
    valmin = curve->valmin;
    valmax = curve->valmax;
    if(curve_index < ndeviceinfo){
      unit = deviceinfo[curve_index].unit;
    }
    else{
      unit = hrrinfo[curve_index - ndeviceinfo].label.unit;
    }
    if(axis_right_unit!=NULL&&strcmp(unit, axis_right_unit) == 0){
      if(axis_right_min>axis_right_max){
        axis_right_min = valmin;
        axis_right_max = valmax;
      }
      else{
        axis_right_min = MIN(axis_right_min,valmin);
        axis_right_max = MAX(axis_right_max,valmax);
      }
    }
    if(axis_left_unit!=NULL&&strcmp(unit, axis_left_unit) == 0){
      if(axis_left_min>axis_left_max){
        axis_left_min = valmin;
        axis_left_max = valmax;
      }
      else{
        axis_left_min = MIN(axis_left_min,valmin);
        axis_left_max = MAX(axis_left_max,valmax);
      }
    }
  }
  for(i = 0; i<plot2di->ncurve_indexes; i++){
    int curve_index;
    float highlight_time, highlight_val;
    int valid;
    char *unit;
    float valmin, valmax;
    int option, position, side;
    char *unit_display;
    curvedata *curve;

    if(axis_right_unit == NULL)break;
    curve_index = plot2di->curve[i].index;
    curve = plot2di->curve + curve_index;
    if(curve_index < ndeviceinfo){
      unit = deviceinfo[curve_index].unit;
    }
    else{
      unit = hrrinfo[curve_index - ndeviceinfo].label.unit;
    }
    if(unit==NULL)continue;
    unit_display = NULL;
    if(strcmp(unit, axis_right_unit) == 0){
      right_position++;
      position = right_position;
      valmin   = axis_right_min;
      valmax   = axis_right_max;
      side     = AXIS_RIGHT;
      if(unit_right_index==i)unit_display = unit;
    }
    else{
      if(axis_left_unit == NULL || strcmp(unit, axis_left_unit) != 0)continue;
      left_position++;
      position = left_position;
      valmin   = axis_left_min;
      valmax   = axis_left_max;
      side     = AXIS_LEFT;
      if(unit_left_index==i)unit_display = unit;
    }
    if(position == 1){
      option = PLOT_ALL;
    }
    else{
       option = PLOT_ONLY_DATA;
    }
    if(curve_index < ndeviceinfo){
      devicedata *devi;

      devi = deviceinfo + curve_index;
      if(global_times!=NULL){
        highlight_time = global_times[itimes];
        highlight_val = GetDeviceVal(global_times[itimes], devi, &valid);
      }
      if(devi->nvals>0){
        DrawGenCurve(option, plot2di, curve, plot2d_size_factor, devi->times, devi->vals, devi->nvals,
                     highlight_time, highlight_val, valmin, valmax,
                     devi->deviceID, position, side, unit_display);
      }
    }
    else{
      hrrdata *hrri;

      hrri = hrrinfo + curve_index - ndeviceinfo;
      if(global_times != NULL){
        int itime;

        highlight_time = global_times[itimes];
        itime = GetInterval(highlight_time, hrrinfo->vals, hrrinfo->nvals);
        itime = CLAMP(itime, 0, hrrinfo->nvals - 1);

        highlight_val = hrri->vals[itime];
      }
      if(hrri->nvals > 0){
        DrawGenCurve(option, plot2di, curve, plot2d_size_factor, hrrinfo->vals, hrri->vals, hrri->nvals,
                     highlight_time, highlight_val, valmin, valmax,
                     hrri->label.shortlabel, position, side, unit_display);
      }
    }
  }
}

  /* ------------------ DrawGenPlots ------------------------ */

void DrawGenPlots(void){
  int i;

  for(i = 0; i < nplot2dinfo;i++){
    plot2ddata *plot2di;

    plot2di = plot2dinfo + i;
    if(plot2di->show == 1){
      DrawGenPlot(plot2di);
    }
  }
}

/* ----------------------- SetupPlot2DUnitData ----------------------------- */

void SetupPlot2DUnitData(void){

  //setup deviceunits
  if(ndeviceinfo > 0){
    int i;

    ndeviceunits = 0;
    FREEMEMORY(deviceunits);
    NewMemory((void **)&deviceunits, ndeviceinfo * sizeof(devicedata *));
    for(i = 0; i < ndeviceinfo; i++){
      int j;
      devicedata *devi;
      int skip_dev;

      devi = deviceinfo + i;
      if(devi->nvals == 0 || strlen(devi->quantity) == 0 || strlen(devi->unit) == 0)continue;
      skip_dev = 0;
      for(j = 0; j < ndeviceunits; j++){
        devicedata *devj;

        devj = deviceunits[j];
        if(strcmp(devi->unit, devj->unit) == 0){
          skip_dev = 1;
          break;
        }
      }
      if(skip_dev == 1)continue;
      deviceunits[ndeviceunits++] = devi;
    }
  }
  //setup hrrunits
  if(nhrrinfo > 0){
    int i;

    nhrrunits = 0;
    FREEMEMORY(hrrunits);
    NewMemory((void **)&hrrunits, nhrrinfo * sizeof(hrrdata *));
    for(i = 0; i < nhrrinfo; i++){
      int j;
      hrrdata *hrri;
      int skip_hrr;

      hrri = hrrinfo + i;
      if(hrri->nvals == 0 || strlen(hrri->label.shortlabel) == 0 || strlen(hrri->label.unit) == 0)continue;
      if(STRCMP(hrri->label.shortlabel, "Time") == 0)continue;
      skip_hrr = 0;
      for(j = 0; j < nhrrunits; j++){
        hrrdata *hrrj;

        hrrj = hrrunits[j];
        if(strcmp(hrri->label.unit, hrrj->label.unit) == 0){
          skip_hrr = 1;
          break;
        }
      }
      if(skip_hrr == 1)continue;
      hrrunits[nhrrunits++] = hrri;
    }
  }
}

/* ------------------ UpdateCurveBounds ------------------------ */

void UpdateCurveBounds(plot2ddata * plot2di, int option){
  int i;

  for(i = 0; i < ndeviceinfo; i++){
    devicedata *devi;
    int j;
    curvedata *curve;

    devi = deviceinfo + i;
    curve = plot2di->curve + i;
    if(option == 1){
      curve->color[0] = 0;
      curve->color[1] = 0;
      curve->color[2] = 0;
      curve->linewidth = 1.0;
      curve->factors[0] = 1.0;
      curve->factors[1] = 0.0;
      curve->use_factors = 0;
      }
    if(devi->nvals > 0){
      float valmin, valmax;

      valmin = devi->vals[0];
      valmax = valmin;
      for(j = 1; j < devi->nvals; j++){
        valmin = MIN(valmin, devi->vals[j]);
        valmax = MAX(valmax, devi->vals[j]);
        }
      curve->valmin = valmin;
      curve->valmax = valmax;
      curve->usermin = valmin;
      curve->usermax = valmax;
      curve->use_usermin = 0;
      curve->use_usermax = 0;
      }
    }
  for(i = 0; i < nhrrinfo; i++){
    hrrdata *hrri;
    int j;
    curvedata *curve;

    curve = plot2di->curve + i + ndeviceinfo;
    hrri = hrrinfo + i;
    if(option == 1){
      curve->color[0] = 0;
      curve->color[1] = 0;
      curve->color[2] = 0;
      curve->linewidth = 1.0;
      curve->factors[0] = 1.0;
      curve->factors[1] = 0.0;
      curve->use_factors = 0;
      }
    if(hrri->nvals > 0){
      float valmin, valmax;

      valmin = hrri->vals[0];
      valmax = valmin;
      for(j = 1; j < hrri->nvals; j++){
        valmin = MIN(valmin, hrri->vals[j]);
        valmax = MAX(valmax, hrri->vals[j]);
        }
      curve->valmin = valmin;
      curve->valmax = valmax;
      curve->usermin = valmin;
      curve->usermax = valmax;
      curve->use_usermin = 0;
      curve->use_usermax = 0;
      }
    }
  }

/* ------------------ InitPlot2D ------------------------ */

void InitPlot2D(plot2ddata * plot2di, int plot_index){
  if(ndeviceinfo == 0 && nhrrinfo == 0)return;
  plot2di->ncurve_indexes = 0;
  plot2di->ncurve_indexes_ini = 0;
  plot2di->show = 0;
  plot2di->show_title = 0;
  plot2di->xyz[0] = xbar0FDS;
  plot2di->xyz[1] = ybar0FDS;
  plot2di->xyz[2] = zbar0FDS;
  plot2di->plot_index = plot_index;
  sprintf(plot2di->plot_label, "plot %i", plot_index);
  plot2di->curve_index = 0;
  NewMemory((void **)&(plot2di->curve), (ndeviceinfo + nhrrinfo) * sizeof(curvedata));
  UpdateCurveBounds(plot2di, 1);
  }

#endif

  /* ------------------ DrawPlot ------------------------ */

void DrawPlot(int option, float *xyz0, float factor, float *x, float *z, int n,
              float highlight_x, float highlight_y, int valid,
              float global_valmin, float global_valmax, char *quantity, char *unit){
  float xmin, xmax, zmin, zmax, dx, dz;
  float zmax_display;
  float xscale=1.0, zscale=1.0;
  float origin[3];
  int i;
  char cvalmin[20], cvalmax[20], cval[20];
  int ndigits = 3;

  origin[0] = xyz0[0];
  origin[1] = xyz0[1];
  origin[2] = xyz0[2];

  xmin = x[0];
  xmax = xmin;
  zmin = z[0];
  zmax = zmin;
  for(i = 1; i<n; i++){
    xmin = MIN(xmin, x[i]);
    xmax = MAX(xmax, x[i]);
    zmin = MIN(zmin, z[i]);
    zmax = MAX(zmax, z[i]);
  }
  if(xmax==xmin)xmax=xmin+1.0;
  if(xmax>xmin)xscale = 1.0/(xmax-xmin);

  if(global_valmin<global_valmax){
    zmin = global_valmin;
    zmax = global_valmax;
  }
  zmax_display = zmax;
  if(zmax==zmin)zmax=zmin+1.0;
  if(zmax>zmin)zscale = 1.0/(zmax-zmin);

  Float2String(cvalmin, zmin,         ndigits, force_fixedpoint);
  Float2String(cvalmax, zmax_display, ndigits, force_fixedpoint);
  Float2String(cval,     highlight_y, ndigits, force_fixedpoint);

  dx = (xmax - xmin)/20.0;
  dz = (zmax - zmin)/20.0;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(SCALE2FDS(plot2d_xyz_offset[0]), SCALE2FDS(plot2d_xyz_offset[1]), SCALE2FDS(plot2d_xyz_offset[2]));

  glTranslatef(origin[0], origin[1], origin[2]);

  float az = camera_current->az_elev[0];
  glRotatef(-az, 0.0,0.0,1.0);

  float elev = camera_current->az_elev[1];
  glRotatef(-elev, 1.0, 0.0, 0.0);

  glScalef(SCALE2FDS(factor), SCALE2FDS(factor), SCALE2FDS(factor));
  glScalef(xscale, 1.0, zscale);
  glTranslatef(-xmin, 0.0, -zmin);
  glColor3fv(foregroundcolor);
  glLineWidth(plot2d_line_width);
  glBegin(GL_LINES);
  for(i = 0; i<n-1; i++){
    glVertex3f(x[i],   0.0, z[i]);
    glVertex3f(x[i+1], 0.0, z[i+1]);
  }

  if(option == PLOT_ALL){
    glVertex3f(xmin - dx, 0.0, zmin - dz);
    glVertex3f(xmax + dx, 0.0, zmin - dz);

    glVertex3f(xmax + dx, 0.0, zmin - dz);
    glVertex3f(xmax + dx, 0.0, zmax + dz);

    glVertex3f(xmax + dx, 0.0, zmax + dz);
    glVertex3f(xmin - dx, 0.0, zmax + dz);

    glVertex3f(xmin - dx, 0.0, zmax + dz);
    glVertex3f(xmin - dx, 0.0, zmin - dz);

    glVertex3f(xmax,      0.0, zmax);
    glVertex3f(xmax + dx, 0.0, zmax);

    glVertex3f(xmax,      0.0, zmin);
    glVertex3f(xmax + dx, 0.0, zmin);
  }
  glEnd();

  float dfont = (float)GetFontHeight()/((float)screenHeight*zscale*SCALE2FDS(factor)*SCALE2SMV(1.0));

  if(option == PLOT_ALL && showd_plot2d_labels==1){
    float zmid;

    zmid = (zmax-2.0*dfont+zmin)/2.0;
    Output3Text(foregroundcolor, xmax + 2.0*dx, 0.0, zmax-0.5*dfont, cvalmax);
    Output3Text(foregroundcolor, xmax + 2.0*dx, 0.0, zmax-1.7*dfont, quantity);
    Output3Text(foregroundcolor, xmax + 2.0*dx, 0.0, zmax-2.9*dfont, unit);
    Output3Text(foregroundcolor, xmax + 2.0*dx, 0.0, zmid-0.5*dfont, cval);
    Output3Text(foregroundcolor, xmax + 2.0*dx, 0.0, zmin-0.5*dfont, cvalmin);
  }

  if(valid==1){
    glColor3f(1.0,0.0,0.0);
    glPointSize(plot2d_point_size);
    glBegin(GL_POINTS);
    glVertex3f(highlight_x, 0.0, highlight_y);
    glEnd();
  }

  glPopMatrix();
}

/* ------------------ TimeAveragePlot2DData ------------------------ */

void TimeAveragePlot2DData(float *times, float *vals, float *vals_avg, int nvals){
  int i;

  if(nvals<=0)return;
  if(times[nvals-1]<=device_time_average){
    float sum = 0.0;

    for(i = 0; i<nvals; i++){
      sum += vals[i];
    }
    sum /= (float)nvals;
    for(i = 0; i<nvals; i++){
      vals_avg[i] = sum;
    }
    return;
  }
  for(i = 0; i<nvals; i++){
    float tlower, tupper;
    int ilower, iupper;
    float sum;
    int j;
    int count;

    if(times[i]>=device_time_average/2.0&&times[i]<=times[nvals-1]-device_time_average/2.0){
      tlower = times[i]-device_time_average/2.0;
      tupper = tlower+device_time_average;
    }
    else if(times[i]<=device_time_average/2.0){
      tlower = times[0];
      tupper = tlower+device_time_average;
    }
    else{
      tupper = times[nvals-1];
      tlower = tupper-device_time_average;
    }
    for(j = i; j>=0; j--){
      ilower = j;
      if(times[j]<=tlower)break;
    }
    for(j = i; j<nvals; j++){
      iupper = j;
      if(times[j]>=tupper)break;
    }
    sum = 0.0;
    count = 0;
    for(j = ilower; j<=iupper;j++){
      if(times[j]>=tlower&&times[j]<=tupper){
        sum += vals[j];
        count++;
      }
    }
    if(count>0){
      vals_avg[i] = sum/(float)(count);
    }
    else{
      vals_avg[i] = vals[i];
    }
  }
}

/* ----------------------- DrawDevicePlots ----------------------------- */

void DrawDevicePlots(void){
  int i;

  if(vis_device_plot!=DEVICE_PLOT_HIDDEN){
    for(i = 0; i<ndeviceinfo; i++){
      devicedata *devicei;

      devicei = deviceinfo+i;
      if(vis_device_plot==DEVICE_PLOT_SHOW_SELECTED&&devicei->selected==0)continue;
      if(devicei->times==NULL||devicei->vals==NULL)continue;
      if(devicei->update_avg==1){
        devicei->update_avg = 0;
        TimeAveragePlot2DData(devicei->times, devicei->vals_orig, devicei->vals, devicei->nvals);
      }
      if(devicei->nvals>1&&devicei->type2==devicetypes_index){
        int valid;
        float highlight_time = 0.0, highlight_val = 0.0;

        valid = 0;
        if(global_times!=NULL){
          highlight_time = global_times[itimes];
          highlight_val = GetDeviceVal(global_times[itimes], devicei, &valid);
        }
        if(devicei->global_valmin>devicei->global_valmax){
          GetGlobalDeviceBounds(devicei->type2);
        }
        DrawPlot(PLOT_ALL, devicei->xyz, plot2d_size_factor, devicei->times, devicei->vals, devicei->nvals,
                 highlight_time, highlight_val, valid, devicei->global_valmin, devicei->global_valmax,
                 devicei->quantity, devicei->unit
        );
      }
    }
  }
}

/* ----------------------- DrawTreePlot ----------------------------- */

void DrawTreePlot(int first, int n){
  int j;
  int drawplot = 0;
  float *xyz = NULL;

  for(j=0;j<n;j++){
    devicedata *devicei;
    int valid, option;
    float highlight_time = 0.0, highlight_val = 0.0;

    devicei = deviceinfo_sortedz[first+j];
    if(devicei->object->visible==0)continue;
    if(devicei->times==NULL||devicei->vals==NULL)continue;
    if(devicei->nvals<=1||devicei->type2!=devicetypes_index)continue;
    drawplot++;

    if(drawplot==1){
      option = PLOT_ALL;
      xyz = devicei->xyz;
    }
    else{
      option = PLOT_ONLY_DATA;
    }
    valid = 0;
    if(global_times!=NULL){
      highlight_time = global_times[itimes];
      highlight_val = GetDeviceVal(global_times[itimes], devicei, &valid);
    }
    if(devicei->global_valmin>devicei->global_valmax){
      GetGlobalDeviceBounds(devicei->type2);
    }
    DrawPlot(option, xyz, plot2d_size_factor, devicei->times, devicei->vals, devicei->nvals,
             highlight_time, highlight_val, valid, devicei->global_valmin, devicei->global_valmax,
             devicei->quantity, devicei->unit
    );
  }
}

/* ----------------------- DrawTreeDevicePlots ----------------------------- */

void DrawTreeDevicePlots(void){
  int i;

  for(i = 0; i<nztreedeviceinfo; i++){
    ztreedevicedata *ztreei;

    ztreei = ztreedeviceinfo+i;
    DrawTreePlot(ztreei->first, ztreei->n);
  }
}
