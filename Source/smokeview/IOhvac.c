#include "options.h"
#ifdef pp_HVAC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "IOobjects.h"

/* ------------------ GetDamperDir ------------------------ */

int GetDamperDir(float *xyz){
  float eyedir[3];
  // 0 y
  // 1 x
  // 2 z
  //cos(angle) = dir1 .dot. dir2 /(norm(dir1)*norm(dir2))
  eyedir[0] = fds_eyepos[0] - xyz[0];
  eyedir[1] = fds_eyepos[1] - xyz[1];
  eyedir[2] = fds_eyepos[2] - xyz[2];
  NORMALIZE3(eyedir);
  if(eyedir[0]<MIN(eyedir[1],eyedir[2]))return 1;
  if(eyedir[1]<MIN(eyedir[0],eyedir[2]))return 0;
  return 2;
}

/* ------------------ DrawHVACDamper ------------------------ */

void DrawHVACDamper(float *xyz, float diam, int state){
  unsigned char color_active[3]   = {0, 255, 0};
  unsigned char color_inactive[3] = {255, 0, 0};
  unsigned char color2[3]         = {0, 0, 0};
  unsigned char *color;

  color2[0] = CLAMP(255 * foregroundcolor[0], 0, 255);
  color2[1] = CLAMP(255 * foregroundcolor[1], 0, 255);
  color2[2] = CLAMP(255 * foregroundcolor[2], 0, 255);
  if(state == 0){
    color = color_active;
  }
  else{
    color = color_inactive;
  }
  float cyl_diam, cyl_height;

  cyl_diam = diam / 4.0;
  cyl_height = 3.0 * diam;
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);
  glRotatef(45.0, 0.0, 1.0, 0.0);
  glTranslatef(0.0, 0.0, -cyl_height/2.0);
  DrawDisk(cyl_diam, cyl_height, color2);
  glPopMatrix();
}

/* ------------------ DrawHVACAircoil ------------------------ */

void DrawHVACAircoil(float *xyz, float size, float diam, int state){
  unsigned char color_active[3] = {0, 255, 0};
  unsigned char color_inactive[3] = {255, 0, 0};
  unsigned char *color;

  if(state == 0){
    color = color_active;
  }
  else{
    color = color_inactive;
  }

  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);
  glLineWidth(2.0);
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

void DrawHVACFan(float *xyz, float size, float diam, int state){
  int i;
  unsigned char color_active[3] = {0, 255, 0};
  unsigned char color_inactive[3] = {255, 0, 0};
  unsigned char *color;

  if(state == 0){
    color = color_active;
  }
  else{
    color = color_inactive;
  }

  if(hvac_circ_x == NULL||hvac_circ_y==NULL){
    FREEMEMORY(hvac_circ_x);
    FREEMEMORY(hvac_circ_y);
    NewMemory((void **)&hvac_circ_x,2*HVAC_NCIRC*sizeof(float));
    NewMemory((void **)&hvac_circ_y,2*HVAC_NCIRC*sizeof(float));
    for(i=0;i<2*HVAC_NCIRC;i++){
      float arg;

      arg = 2.0*PI*(float)i/(float)(HVAC_NCIRC-1);
      hvac_circ_x[i] = cos(arg);
      hvac_circ_y[i] = sin(arg);
    }
  }
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);
  glLineWidth(2.0);
  glScalef(size,size,size);
  glColor3fv(foregroundcolor);
  glBegin(GL_LINES);
  for(i=0;i<HVAC_NCIRC-1;i++){
    float x, y, xp1, yp1;

    x   = hvac_circ_x[i];
    xp1 = hvac_circ_x[i+1];
    y   = hvac_circ_y[i];
    yp1 = hvac_circ_y[i+1];
    glVertex3f(x,   0.0, y);
    glVertex3f(xp1, 0.0, yp1);
  }
  int ibeg, iend;
  float x0, y0;

  int ii;
  for(ii = 0;ii <3;ii++){
    if(ii == 0){
      ibeg = HVAC_NCIRC / 12;
      x0 = 0.0;
      y0 = -1.0;
    }
    else if(ii==1){
      ibeg = 5*HVAC_NCIRC/12;
      x0 = sqrt(3.0)/2.0;
      y0 = 0.5;
    }
    else{
      ibeg = HVAC_NCIRC / 12 + 2*HVAC_NCIRC/3;
      x0 = -sqrt(3.0)/ 2.0;
      y0 = 0.5;
    }
    iend = ibeg + HVAC_NCIRC / 3;

    for(i = ibeg;i < iend;i++){
      float x, y, xp1, yp1;

      x   = hvac_circ_x[i]     + x0;
      xp1 = hvac_circ_x[i + 1] + x0;
      y   = hvac_circ_y[i]     + y0;
      yp1 = hvac_circ_y[i + 1] + y0;
      glVertex3f(x, 0.0, y);
      glVertex3f(xp1, 0.0, yp1);
    }
  }
  glEnd();
  glPopMatrix();
}

/* ------------------ DrawHVACFilter ------------------------ */

void DrawHVACFilter(float *xyz, float size){
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  glLineWidth(2.0);
  glScalef(size,size,size);
  glBegin(GL_LINES);
  glVertex3f(0.0, -0.5, -1.0);
  glVertex3f(0.0, 0.5,  -1.0);
  glVertex3f(0.0, -0.5, -0.5);
  glVertex3f(0.0, 0.5,  0.5);
  glVertex3f(0.0, -0.5, 0.0);
  glVertex3f(0.0, 0.5,  0.0);
  glVertex3f(0.0, -0.5, 0.5);
  glVertex3f(0.0, 0.5,  0.5);
  glVertex3f(0.0, -0.5, 1.0);
  glVertex3f(0.0, 0.5,  1.0);
  glVertex3f(0.0, -0.5,-1.0);
  glVertex3f(0.0, -0.5, 1.0);
  glVertex3f(0.0, 0.5, -1.0);
  glVertex3f(0.0, 0.5,  1.0);
  glEnd();
  glPopMatrix();
}

/* ------------------ GetHVACDuctID ------------------------ */

hvacductdata *GetHVACDuctID(char *duct_name){
  int i;

  for(i = 0;i < nhvacductinfo;i++){
    hvacductdata *ducti;

    ducti = hvacductinfo + i;
    if(strcmp(ducti->duct_name, duct_name) == 0)return ducti;
  }
  return NULL;
}

/* ------------------ GetHVACNodeID ------------------------ */

hvacnodedata *GetHVACNodeID(char *node_name){
  int i;

  for(i = 0;i < nhvacnodeinfo;i++){
    hvacnodedata *nodei;

    nodei = hvacnodeinfo + i;
    if(strcmp(nodei->duct_name, node_name) == 0)return nodei;
  }
  return NULL;
}

/* ------------------ DrawHVAC ------------------------ */

void DrawHVAC(hvacdata *hvaci) {
  int i;
  unsigned char uc_color[3];

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);

  // draw ducts
  
  glLineWidth(hvaci->duct_width);
  glBegin(GL_LINES);
  uc_color[0] = CLAMP(hvaci->duct_color[0], 0, 255);
  uc_color[1] = CLAMP(hvaci->duct_color[1], 0, 255);
  uc_color[2] = CLAMP(hvaci->duct_color[2], 0, 255);
  glColor3ubv(uc_color);
  for (i = 0; i < nhvacductinfo; i++) {
    hvacductdata *hvacducti;
    hvacnodedata *node_from, *node_to;
    float* xyz0, * xyz1;

    hvacducti = hvacductinfo + i;
    if(strcmp(hvaci->network_name, hvacducti->network_name) != 0)continue;

    node_from = hvacnodeinfo + hvacducti->node_id_from;
    node_to   = hvacnodeinfo + hvacducti->node_id_to;
    if (node_from == NULL || node_to == NULL)continue;
    xyz0 = node_from->xyz;
    xyz1 = node_to->xyz;
    glVertex3f(xyz0[0], xyz0[1], xyz0[2]);
    if(hvac_metro_view == 1){
      glVertex3f(xyz1[0], xyz0[1], xyz0[2]);
      glVertex3f(xyz1[0], xyz0[1], xyz0[2]);
      glVertex3f(xyz1[0], xyz1[1], xyz0[2]);
      glVertex3f(xyz1[0], xyz1[1], xyz0[2]);
    }
    glVertex3f(xyz1[0], xyz1[1], xyz1[2]);
  }
  glEnd();
  if (hvaci->show_duct_labels == 1|| hvaci->show_components==1) {
    for (i = 0; i < nhvacductinfo; i++) {
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];
      char label[256];

      ducti = hvacductinfo + i;
      if(strcmp(hvaci->network_name, ducti->network_name) != 0)continue;

      strcpy(label, "");
      if(hvaci->show_components == 1)strcat(label, ducti->c_component);
      if (hvaci->show_duct_labels == 1) {
        if (hvaci->show_components == 1)strcat(label, ":");
        strcat(label, ducti->duct_name);
      }
      node_from = hvacnodeinfo + ducti->node_id_from;
      node_to   = hvacnodeinfo + ducti->node_id_to;
      if (node_from == NULL || node_to == NULL)continue;
      xyz[0] = (node_from->xyz[0] + node_to->xyz[0])/2.0;
      xyz[1] = (node_from->xyz[1] + node_to->xyz[1])/2.0;
      xyz[2] = (node_from->xyz[2] + node_to->xyz[2])/2.0;
      Output3Text(foregroundcolor, xyz[0], xyz[1], xyz[2], label);
    }
    for(i = 0; i < nhvacductinfo; i++) {
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];

      ducti = hvacductinfo + i;
      if(strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
      node_from = hvacnodeinfo + ducti->node_id_from;
      node_to   = hvacnodeinfo + ducti->node_id_to;
      if (node_from == NULL || node_to == NULL)continue;
      xyz[0] = (node_from->xyz[0] + node_to->xyz[0])/2.0;
      xyz[1] = (node_from->xyz[1] + node_to->xyz[1])/2.0;
      xyz[2] = (node_from->xyz[2] + node_to->xyz[2])/2.0;
      float size;
      int state;

      size = xyzmaxdiff / 80.0;
      state = 0;
     // DrawHVACDamper(xyz, size, state);
      // DrawHVACFilter(xyz, size);
     //  DrawHVACAircoil(xyz, 2.0*size, size, state);
     //  DrawHVACFan(xyz, 2.0*size, size, state);
     }
  }

  // draw nodes
  glPointSize(hvaci->node_size);
  glBegin(GL_POINTS);
  uc_color[0] = CLAMP(hvaci->node_color[0], 0, 255);
  uc_color[1] = CLAMP(hvaci->node_color[1], 0, 255);
  uc_color[2] = CLAMP(hvaci->node_color[2], 0, 255);
  glColor3ubv(uc_color);
  for (i = 0; i < nhvacnodeinfo; i++) {
    hvacnodedata *nodei;

    nodei = hvacnodeinfo + i;
    if(strcmp(hvaci->network_name, nodei->network_name) != 0)continue;

    glVertex3fv(nodei->xyz);
  }
  glEnd();

  if (hvaci->show_node_labels == 1|| hvaci->show_filters==1) {
    for (i = 0; i < nhvacnodeinfo; i++) {
      hvacnodedata* nodei;
      char label[256];

      nodei = hvacnodeinfo + i;
      strcpy(label, "");
      if(hvaci->show_filters == 1)strcat(label, nodei->c_filter);
      if(hvaci->show_node_labels == 1){
        if(hvaci->show_filters == 1&&nodei->filter== HVAC_FILTER_YES)strcat(label, ":");
        strcat(label, nodei->node_name);
      }
      Output3Text(foregroundcolor, nodei->xyz[0], nodei->xyz[1], nodei->xyz[2], label);
    }
  }

  glPopMatrix();
}

/* ------------------ DrawHVACS ------------------------ */

void DrawHVACS(void){
  int i;

  for(i=0; i<nhvacinfo; i++){
    hvacdata *hvaci;

    hvaci = hvacinfo + i;
    if(hvaci->display==0)continue;
    DrawHVAC(hvaci);
  }
}

#endif
