#include "options.h"
#ifdef pp_HVAC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

/* ------------------ DrawHVAC ------------------------ */

void DrawHVAC(hvacdata *hvaci) {
  int i;
  float hvac_color[3]={0.0,0.0,0.0}, hvac_line_width=4.0, hvac_node_size=8.0;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);

  // draw ducts
  
  glColor3fv(hvac_color);
  glLineWidth(hvac_line_width);
  glBegin(GL_LINES);
  for (i = 0; i < nhvacductinfo; i++) {
    hvacductdata *hvacducti;
    hvacnodedata *node_from, *node_to;

    hvacducti = hvacductinfo + i;
    if(strcmp(hvaci->network_name, hvacducti->network_name) != 0)continue;

    node_from = hvacnodeinfo + hvacducti->node_id_from;
    node_to   = hvacnodeinfo + hvacducti->node_id_to;
    if (node_from == NULL || node_to == NULL)continue;
    glVertex3fv(node_from->xyz);
    glVertex3fv(node_to->xyz);
  }
  glEnd();
  if (hvac_show_duct_labels == 1) {
    for (i = 0; i < nhvacductinfo; i++) {
      hvacductdata *hvacducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];

      hvacducti = hvacductinfo + i;
      if(strcmp(hvaci->network_name, hvacducti->network_name) != 0)continue;

      node_from = hvacnodeinfo + hvacducti->node_id_from;
      node_to   = hvacnodeinfo + hvacducti->node_id_to;
      if (node_from == NULL || node_to == NULL)continue;
      xyz[0] = (node_from->xyz[0] + node_to->xyz[0])/2.0;
      xyz[1] = (node_from->xyz[1] + node_to->xyz[1])/2.0;
      xyz[2] = (node_from->xyz[2] + node_to->xyz[2])/2.0;
      Output3Text(foregroundcolor, xyz[0], xyz[1], xyz[2], hvacducti->duct_name);
    }
  }

  // draw nodes
  glPointSize(hvac_node_size);
  glBegin(GL_POINTS);
  for (i = 0; i < nhvacnodeinfo; i++) {
    hvacnodedata *nodei;

    nodei = hvacnodeinfo + i;
    if(strcmp(hvaci->network_name, nodei->network_name) != 0)continue;

    glVertex3fv(nodei->xyz);
  }
  glEnd();

  if (hvac_show_node_labels == 1) {
    for (i = 0; i < nhvacnodeinfo; i++) {
      hvacnodedata* nodei;

      nodei = hvacnodeinfo + i;
      Output3Text(foregroundcolor, nodei->xyz[0], nodei->xyz[1], nodei->xyz[2], nodei->node_name);
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
