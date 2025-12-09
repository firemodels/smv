#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "IOobjects.h"

#include "readhvac.h"

#define HVAC_MAXCELLS global_scase.hvaccoll.hvac_maxcells
#define HVAC_N_DUCTS  global_scase.hvaccoll.hvac_n_ducts

#define HVACVALINDEX(itime, iduct, icell) (itime)*HVAC_MAXCELLS*HVAC_N_DUCTS +  (iduct)*HVAC_MAXCELLS + (icell)

unsigned char hvac_off_color[3] = {0, 255, 0}, hvac_on_color[3] = {255, 0, 0};
unsigned char *hvac_color_states[2] = {hvac_off_color, hvac_on_color};
#define HVACCOLORCONV(v,min,max) max < min ? 0 : CLAMP(255*((v)-min)/(max-min),0,255)

#define NODE_XYZ 0.1


/* ------------------ UpdateHVACDuctColorLabels ------------------------ */

void UpdateHVACDuctColorLabels(int index){
  hvacvaldata *hi;
  int set_valmin, set_valmax;
  float valmin, valmax;

  hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + index;
  GLUIGetMinMax(BOUND_HVACDUCT, hi->label.shortlabel, &set_valmin, &valmin, &set_valmax, &valmax);
  GetColorbarLabels(valmin, valmax, global_scase.nrgb, hi->colorlabels, hi->levels256);
}

/* ------------------ UpdateHVACColorNodeLabels ------------------------ */

void UpdateHVACNodeColorLabels(int index){
  hvacvaldata *hi;
  int set_valmin, set_valmax;
  float valmin, valmax;

  hi = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + index;
  GLUIGetMinMax(BOUND_HVACNODE, hi->label.shortlabel, &set_valmin, &valmin, &set_valmax, &valmax);
  GetColorbarLabels(valmin, valmax, global_scase.nrgb, hi->colorlabels, hi->levels256);
}

/* ------------------ UpdateAllHVACColorLabels ------------------------ */

void UpdateAllHVACColorLabels(void){
  int i;

  for(i = 0; i < global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars; i++){
    hvacvaldata *hi;
    int set_valmin, set_valmax;
    float valmin, valmax;

    hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + i;
    GLUIGetMinMax(BOUND_HVACDUCT, hi->label.shortlabel, &set_valmin, &valmin, &set_valmax, &valmax);
    GetColorbarLabels(hi->valmin, hi->valmax, global_scase.nrgb, hi->colorlabels, hi->levels256);
  }
  for(i = 0; i < global_scase.hvaccoll.hvacnodevalsinfo->n_node_vars; i++){
    hvacvaldata *hi;
    int set_valmin, set_valmax;
    float valmin, valmax;

    hi = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + i;
    GLUIGetMinMax(BOUND_HVACNODE, hi->label.shortlabel, &set_valmin, &valmin, &set_valmax, &valmax);
    GetColorbarLabels(valmin, valmax, global_scase.nrgb, hi->colorlabels, hi->levels256);
  }
}

/* ------------------ ReadHVACData ------------------------ */

void ReadHVACData(int flag){
  float total_time;
  FILE_SIZE file_size;

  if(flag == LOAD){
    START_TIMER(total_time);
  }
  ReadHVACData0(&global_scase.hvaccoll,flag, &file_size);
  if(flag == LOAD){
    UpdateAllHVACColorLabels();
    GetGlobalHVACNodeBounds(1);
    GetGlobalHVACDuctBounds(1);
    GLUIUpdateHVACDuctType();
    GLUISetValTypeIndex(BOUND_HVACDUCT, 0);

    STOP_TIMER(total_time);
    PRINTF("Loading %s\n", global_scase.hvaccoll.hvacductvalsinfo->file);
    if(file_size > 1000000000){
      PRINTF("Loaded %.1f GB/%.1f s\n", (float)file_size / 1000000000., total_time);
    }
    else if(file_size > 1000000){
      PRINTF("Loaded %.1f MB/%.1f s\n", (float)file_size / 1000000., total_time);
    }
    else{
      PRINTF("Loaded %.0f KB/%.1f s\n", (float)file_size / 1000., total_time);
    }
    global_scase.hvaccoll.hvacductvalsinfo->loaded = 1;
  }
  if(flag == BOUNDS_ONLY){
    ReadHVACData(UNLOAD);
  }
}

/* ------------------ GetHVACState ------------------------ */

int GetHVACDuctState(hvacductdata *ducti){
  int i, n, *states;
  float *times;
  float current_time;

  if(global_times==NULL)return HVAC_STATE_INACTIVE;
  times = ducti->act_times;
  states = ducti->act_states;
  n = ducti->nact_times;
  if(n==0||times==NULL)return HVAC_STATE_INACTIVE;

  current_time = GetTime();

  if(current_time < times[0])return HVAC_STATE_INACTIVE;
  for(i = 0;i < n - 1;i++){
    if(current_time >= times[i] && current_time < times[i + 1])return states[i];
  }
  return states[n - 1];
}

/* ------------------ GetDuctDir ------------------------ */

int GetDuctDir(float *xyz){
  float eyedir[3];

  //cos(angle) = dir1 .dot. dir2 /(norm(dir1)*norm(dir2))
  eyedir[0] = ABS(xyz[0] - fds_eyepos[0]);
  eyedir[1] = ABS(xyz[1] - fds_eyepos[1]);
  eyedir[2] = ABS(xyz[2] - fds_eyepos[2]);
  if(eyedir[0]>MAX(eyedir[1],eyedir[2]))return 1;
  if(eyedir[1]>MAX(eyedir[0],eyedir[2]))return 0;
  return 2;
}

/* ------------------ DrawHVACDamper ------------------------ */

void DrawHVACDamper(hvacductdata *ducti, float *xyz, float diam, int state){
  float cyl_diam, cyl_height;
  unsigned char color2[3] = {0, 0, 0};
  unsigned char *color;
  float axis[3];
  float u[3] = {0.0, 0.0, 1.0}, *v, angle;

  color = hvac_color_states[state];
  color2[0] = CLAMP(255 * foregroundcolor[0], 0, 255);
  color2[1] = CLAMP(255 * foregroundcolor[1], 0, 255);
  color2[2] = CLAMP(255 * foregroundcolor[2], 0, 255);

  cyl_diam = diam / 4.0;
  cyl_height = 3.0 * diam;
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);

  if(hvac_metro_view == 1){
    v = ducti->normal_metro;
  }
  else{
    v = ducti->normal;
  }

  RotateU2V(u, v, axis, &angle);
  angle *= 180.0 / 3.14159;
  if(state == 0){
    glRotatef(-45.0, 0.0, 1.0, 0.0);
  }
  else{
    glRotatef(-90.0, 0.0, 1.0, 0.0);
  }

  glRotatef(angle, axis[0], axis[1], axis[2]);

  glTranslatef(0.0, 0.0, -cyl_height/2.0);
  DrawDisk(cyl_diam, cyl_height, color2);
  glPopMatrix();
}

/* ------------------ DrawHVACAircoil ------------------------ */

void DrawHVACAircoil(hvacductdata *ducti, float *xyz, float size, float diam, int state){
  unsigned char *color;
  float axis[3];
  float u[3] = {1.0, 0.0, 0.0}, *v, angle;

  color = hvac_color_states[state];

  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);

  if(hvac_metro_view == 1){
    v = ducti->normal_metro;
  }
  else{
    v = ducti->normal;
  }
  RotateU2V(u, v, axis, &angle);
  angle *= 180.0 / 3.14159;
  glRotatef(angle, axis[0], axis[1], axis[2]);

  glLineWidth(2.0);
  glColor3fv(foregroundcolor);
  glScalef(size, size, size);
  glBegin(GL_LINES);
  glVertex3f(-1.0,  0.0,  0.0);
  glVertex3f(-0.75, 0.0, -0.5);

  glVertex3f(-0.75, 0.0, -0.5);
  glVertex3f(-0.25, 0.0,  0.5);

  glVertex3f(-0.25, 0.0,  0.5);
  glVertex3f( 0.25, 0.0, -0.5);

  glVertex3f(0.25, 0.0, -0.5);
  glVertex3f(0.75, 0.0,  0.5);

  glVertex3f(0.75, 0.0, 0.5);
  glVertex3f(1.0 , 0.0, 0.0);
  glEnd();
  glPopMatrix();
}

/* ------------------ DrawHVACFan ------------------------ */

void DrawHVACFan(hvacductdata *ducti, float *xyz, float size, float diam, int state){
  int i;
  unsigned char *color;
  float axis[3];
  float u[3] = {1.0, 0.0, 0.0}, *v, angle;

  color = hvac_color_states[state];
  if(hvac_circ_x == NULL||hvac_circ_y==NULL){
    FREEMEMORY(hvac_circ_x);
    FREEMEMORY(hvac_circ_y);
    NewMemory((void **)&hvac_circ_x,2*HVAC_NCIRC*sizeof(float));
    NewMemory((void **)&hvac_circ_y,2*HVAC_NCIRC*sizeof(float));
    for(i=0;i<HVAC_NCIRC;i++){
      float arg;
      float r, xx, yy;

      arg = 2.0*PI*(float)i/(float)(HVAC_NCIRC-1);
      hvac_circ_x[i] = cos(arg);
      hvac_circ_y[i] = sin(arg);
      r = sin(2.0 * arg);
      xx = r * cos(arg);
      yy = r * sin(arg);
      hvac_circ_x[i + HVAC_NCIRC] = xx;
      hvac_circ_y[i + HVAC_NCIRC] = yy;
    }
  }
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);
  glLineWidth(2.0);

  if(hvac_metro_view == 1){
    v = ducti->normal_metro;
  }
  else{
    v = ducti->normal;
  }
  RotateU2V(u, v, axis, &angle);
  angle *= 180.0 / 3.14159;
  glRotatef(angle, axis[0], axis[1], axis[2]);
  glRotatef(90, 0.0,0.0,1.0);
  if(state==1&&global_times != NULL){
    float angle2, time2;

    time2 = global_times[itimes];
    angle2 = 360.0*time2 / 10.0;
    glRotatef(angle2, 0.0, 1.0, 0.0);
  }

  glScalef(size,size,size);
  glColor3fv(foregroundcolor);
  glBegin(GL_LINES);
  for(i=0;i<2*HVAC_NCIRC-1;i++){
    float x, y, xp1, yp1;

    if(i == HVAC_NCIRC - 1)continue;
    x   = hvac_circ_x[i];
    xp1 = hvac_circ_x[i+1];
    y   = hvac_circ_y[i];
    yp1 = hvac_circ_y[i+1];
    glVertex3f(x,   0.0, y);
    glVertex3f(xp1, 0.0, yp1);
  }
  glEnd();
  glPopMatrix();
}

/* ------------------ DrawHVACFilter ------------------------ */

void DrawHVACFilter(hvacductdata *ducti, float *xyz, float size){
  float axis[3];
  float u[3] = {0.0, 1.0, 0.0}, *v, angle;

  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  if(ducti!=NULL){
    if(hvac_metro_view == 1){
      v = ducti->normal_metro;
    }
    else{
      v = ducti->normal;
    }
    RotateU2V(u, v, axis, &angle);
    angle *= 180.0 / 3.14159;
    glRotatef(angle, axis[0], axis[1], axis[2]);
  }
  glRotatef(90.0, 0.0, 0.0, 1.0);
  glLineWidth(2.0);
  glScalef(size,size,size);
  glBegin(GL_LINES);
  glVertex3f(0.0, -0.5, -1.0);
  glVertex3f(0.0,  0.5, -1.0);

  glVertex3f(0.0, -0.5, -0.5);
  glVertex3f(0.0,  0.5, -0.5);

  glVertex3f(0.0, -0.5,  0.0);
  glVertex3f(0.0,  0.5,  0.0);

  glVertex3f(0.0, -0.5,  0.5);
  glVertex3f(0.0,  0.5,  0.5);

  glVertex3f(0.0, -0.5,  1.0);
  glVertex3f(0.0,  0.5,  1.0);

  glVertex3f(0.0, -0.5, -1.0);
  glVertex3f(0.0, -0.5,  1.0);

  glVertex3f(0.0,  0.5, -1.0);
  glVertex3f(0.0,  0.5,  1.0);
  glEnd();
  glPopMatrix();
}

/* ------------------ DrawHVAC ------------------------ */

void DrawHVAC(hvacdata *hvaci){
  int i, frame_index=0;
  unsigned char uc_color[3];
  unsigned char hvac_cell_color[3] = {128, 128, 128};
  float *last_color = NULL;

  if((global_scase.hvaccoll.hvacductvar_index >= 0||global_scase.hvaccoll.hvacnodevar_index>=0)&&global_times!=NULL){
    frame_index = GetTimeInterval(GetTime(), global_scase.hvaccoll.hvacductvalsinfo->times, global_scase.hvaccoll.hvacductvalsinfo->ntimes);
  }

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);

  // draw ducts

  float valmin, valmax;
  int set_valmin, set_valmax;
  hvacvaldata *ductvar;
  if(global_times != NULL && global_scase.hvaccoll.hvacductvar_index>=0){
    ductvar = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + global_scase.hvaccoll.hvacductvar_index;
    GLUIGetOnlyMinMax(BOUND_HVACDUCT, ductvar->label.shortlabel, &set_valmin, &valmin, &set_valmax, &valmax);
  }

  glLineWidth(hvaci->duct_width);
  glBegin(GL_LINES);
  if(global_scase.hvaccoll.hvacductvar_index < 0||global_times==NULL){
    uc_color[0] = CLAMP(hvaci->duct_color[0], 0, 255);
    uc_color[1] = CLAMP(hvaci->duct_color[1], 0, 255);
    uc_color[2] = CLAMP(hvaci->duct_color[2], 0, 255);
    glColor3ubv(uc_color);
  }
  for(i = 0; i < global_scase.hvaccoll.nhvacductinfo; i++){
    hvacductdata *ducti;
    hvacnodedata *node_from, *node_to;

    ducti = global_scase.hvaccoll.hvacductinfo + i;
    if(hvac_show_networks==1&&strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
    if(hvac_show_connections==1&&ducti->connect != NULL && ducti->connect->display == 0)continue;

    node_from = global_scase.hvaccoll.hvacnodeinfo + ducti->node_id_from;
    node_to = global_scase.hvaccoll.hvacnodeinfo + ducti->node_id_to;
    if(node_from == NULL || node_to == NULL)continue;
    float *xyzs;
    int nxyzs, j, *cell_index;

    if(hvac_metro_view == 1){
      xyzs       = ducti->xyz_met_cell;
      cell_index = ducti->cell_met;
      nxyzs      = ducti->nxyz_met_cell-1;
    }
    else{
      xyzs       = ducti->xyz_reg_cell;
      cell_index = ducti->cell_reg;
      nxyzs      = ducti->nxyz_reg_cell-1;
    }
    if(global_times != NULL && global_scase.hvaccoll.hvacductvar_index >= 0){
      for(j = 0;j < nxyzs;j++){
        float *xyz, *this_color;
        int cell, index;
        unsigned char ival;

        xyz = xyzs + 3 * j;
        cell = cell_index[j];

        index = HVACVALINDEX(frame_index, i, cell);

        ival = HVACCOLORCONV(ductvar->vals[index], valmin, valmax);
        this_color = rgb_full[ival];
        if(this_color != last_color){
          last_color = this_color;
          glColor3fv(this_color);
        }
        glVertex3fv(xyz);
        glVertex3fv(xyz+3);
      }
    }
    else{
      for(j = 0;j < nxyzs;j++){
        float *xyz;

        xyz = xyzs + 3 * j;
        glVertex3fv(xyz);
        glVertex3fv(xyz+3);
      }
    }
  }
  glEnd();
  SNIFF_ERRORS("after hvac duct lines");

  if(hvac_cell_view==1){
    glColor3ubv(hvac_cell_color);
    glPointSize(hvaci->cell_node_size);
    glBegin(GL_POINTS);
    for(i = 0; i < global_scase.hvaccoll.nhvacductinfo; i++){
      hvacductdata *ducti;
      float *xyzs;
      int nxyzs, j;

      ducti = global_scase.hvaccoll.hvacductinfo + i;
      if(ducti->nduct_cells <= 1)continue;

      if(hvac_metro_view == 1){
        xyzs       = ducti->xyz_met_cell;
        nxyzs      = ducti->nxyz_met_cell-1;
      }
      else{
        xyzs       = ducti->xyz_reg_cell;
        nxyzs      = ducti->nxyz_reg_cell-1;
      }
      for(j = 1;j < nxyzs-1;j++){
        float *xyz;

        xyz = xyzs + 3 * j;
        glVertex3fv(xyz);
      }
    }
    glEnd();
    SNIFF_ERRORS("after hvac duct points");
  }
  if(hvaci->show_duct_labels == 1){
    for(i = 0; i < global_scase.hvaccoll.nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];
      char label[256];
      float offset;

      ducti = global_scase.hvaccoll.hvacductinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
      if(hvac_show_connections == 1 && ducti->connect != NULL && ducti->connect->display == 0)continue;

      strcpy(label, ducti->duct_name);
      node_from = global_scase.hvaccoll.hvacnodeinfo + ducti->node_id_from;
      node_to   = global_scase.hvaccoll.hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      if(hvac_metro_view==1){
        memcpy(xyz, ducti->xyz_label_metro, 3*sizeof(float));
      }
      else{
        memcpy(xyz, ducti->xyz_label,       3*sizeof(float));
      }
      offset = 0.01/xyzmaxdiff;
      Output3Text(foregroundcolor, xyz[0]+offset, xyz[1]+offset, xyz[2]+offset, label);
    }
    SNIFF_ERRORS("after hvac duct labels");
  }
  if(hvaci->show_component == DUCT_COMPONENT_TEXT){
    for(i = 0; i < global_scase.hvaccoll.nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];
      char label[256];

      ducti = global_scase.hvaccoll.hvacductinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
      if(hvac_show_connections == 1 && ducti->connect != NULL && ducti->connect->display == 0)continue;

      strcpy(label, "");
      strcat(label, ducti->c_component);
      node_from = global_scase.hvaccoll.hvacnodeinfo + ducti->node_id_from;
      node_to   = global_scase.hvaccoll.hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      if(hvac_metro_view == 1){
        memcpy(xyz, ducti->xyz_symbol_metro, 3*sizeof(float));
      }
      else{
        memcpy(xyz, ducti->xyz_symbol,       3*sizeof(float));
      }
      xyz[2] += 0.01 / xyzmaxdiff;
      Output3Text(foregroundcolor, xyz[0], xyz[1], xyz[2]+0.01/xyzmaxdiff, label);
    }
    SNIFF_ERRORS("after hvac duct component text");
  }
  if(hvaci->show_component == DUCT_COMPONENT_SYMBOLS){
    for(i = 0; i < global_scase.hvaccoll.nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float *xyz;

      ducti = global_scase.hvaccoll.hvacductinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
      if(hvac_show_connections == 1 && ducti->connect != NULL && ducti->connect->display == 0)continue;
      node_from = global_scase.hvaccoll.hvacnodeinfo + ducti->node_id_from;
      node_to   = global_scase.hvaccoll.hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      float size;
      int state;

      state = GetHVACDuctState(ducti);
      size  = xyzmaxdiff / 40.0;
      size *= hvaci->component_size;
      if(hvac_metro_view == 1){
        xyz = ducti->xyz_symbol_metro;
      }
      else{
        xyz = ducti->xyz_symbol;
      }
      switch(ducti->component){
      case HVAC_NONE:
        break;
      case HVAC_FAN:
        DrawHVACFan(ducti, xyz, 0.75*2.0*size, 0.75*size, state);
        break;
      case HVAC_AIRCOIL:
        DrawHVACAircoil(ducti, xyz, 2.0*size, size, state);
        break;
      case HVAC_DAMPER:
        DrawHVACDamper(ducti, xyz, size, state);
        break;
      default:
        assert(FFALSE);
        break;
      }
    }
    SNIFF_ERRORS("after hvac component symbols");
  }

  // draw nodes
  glPointSize(hvaci->node_size);
  glBegin(GL_POINTS);
  if(global_scase.hvaccoll.hvacnodevar_index < 0 || global_times == NULL){
    uc_color[0] = CLAMP(hvaci->node_color[0], 0, 255);
    uc_color[1] = CLAMP(hvaci->node_color[1], 0, 255);
    uc_color[2] = CLAMP(hvaci->node_color[2], 0, 255);
    glColor3ubv(uc_color);
  }
  for(i = 0; i < global_scase.hvaccoll.nhvacnodeinfo; i++){
    hvacnodedata *nodei;

    nodei = global_scase.hvaccoll.hvacnodeinfo + i;
    if(hvac_show_networks == 1 && strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
    if(hvac_show_connections == 1 && nodei->connect != NULL && nodei->connect->display == 0)continue;
    if(global_times != NULL && global_scase.hvaccoll.hvacnodevar_index >= 0){
      hvacvaldata *nodevar;
      unsigned char ival;

      //        duct: i
      //        time: itime
      //   var index: hvacductvar_index
      nodevar = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + global_scase.hvaccoll.hvacnodevar_index;
      ival = HVACCOLORCONV(nodevar->vals[frame_index + i * global_scase.hvaccoll.hvacnodevalsinfo->ntimes], nodevar->valmin, nodevar->valmax);
      glColor3fv(rgb_full[ival]);
    }
    glVertex3fv(nodei->xyz);
  }
  glEnd();
  SNIFF_ERRORS("after hvac duct nodes");

  uc_color[0] = CLAMP(hvaci->node_color[0], 0, 255);
  uc_color[1] = CLAMP(hvaci->node_color[1], 0, 255);
  uc_color[2] = CLAMP(hvaci->node_color[2], 0, 255);
  glColor3ubv(uc_color);
  if(hvaci->show_node_labels == 1){
    for(i = 0; i < global_scase.hvaccoll.nhvacnodeinfo; i++){
      hvacnodedata* nodei;
      char label[256];
      float offset;

      nodei = global_scase.hvaccoll.hvacnodeinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      if(hvac_show_connections == 1 && nodei->connect != NULL && nodei->connect->display == 0)continue;
      offset = 0.01/xyzmaxdiff;
      strcpy(label, nodei->node_name);
      Output3Text(foregroundcolor, nodei->xyz[0]+offset, nodei->xyz[1]+offset, nodei->xyz[2]+offset, label);
    }
    SNIFF_ERRORS("after hvac node labels");
  }
  if(hvaci->show_filters == NODE_FILTERS_LABELS){
    for(i = 0; i < global_scase.hvaccoll.nhvacnodeinfo; i++){
      hvacnodedata *nodei;
      char label[256];
      float offset;

      nodei = global_scase.hvaccoll.hvacnodeinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      if(hvac_show_connections == 1 && nodei->connect != NULL && nodei->connect->display == 0)continue;
      strcpy(label, nodei->c_filter);
      offset = 0.01 / xyzmaxdiff;
      Output3Text(foregroundcolor, nodei->xyz[0]+offset, nodei->xyz[1] + offset, nodei->xyz[2] + offset, label);
    }
    SNIFF_ERRORS("after hvac node filter labels");
  }
  if(hvaci->show_filters == NODE_FILTERS_SYMBOLS){
    for(i = 0; i < global_scase.hvaccoll.nhvacnodeinfo; i++){
      hvacnodedata *nodei;
      float size;

      size  = xyzmaxdiff / 25.0;
      size *= hvaci->filter_size;
      nodei = global_scase.hvaccoll.hvacnodeinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      if(hvac_show_connections == 1 && nodei->connect != NULL && nodei->connect->display == 0)continue;
      if(nodei->filter == HVAC_FILTER_NO)continue;
      DrawHVACFilter(nodei->duct, nodei->xyz, size);
    }
    SNIFF_ERRORS("after hvac node filter symbols");
  }
  glPopMatrix();
}

/* ------------------ DrawHVACS ------------------------ */

void DrawHVACS(void){
  int i;

  for(i=0; i<global_scase.hvaccoll.nhvacinfo; i++){
    hvacdata *hvaci;

    hvaci = global_scase.hvaccoll.hvacinfo + i;
    if(hvac_show_networks==1&&hvaci->display==0)continue;
    DrawHVAC(hvaci);
  }
}
