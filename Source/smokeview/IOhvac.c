#include "options.h"
#ifdef pp_HVAC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"

/* ------------------ GetHVACNode ------------------------ */

hvacnodedata *GetHVACNode(hvacdata *hvaci, int node_id){
  int i;

  for(i=0; i<hvaci->n_nodes; i++){
    hvacnodedata *nodei;

    nodei = hvaci->nodeinfo + i;
    if (nodei->node_id == node_id)return nodei;
  }
  return NULL;
}

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
  for (i = 0; i < hvaci->n_ducts; i++) {
    hvacductdata *hvacducti;
    hvacnodedata *node_from, *node_to;

    hvacducti = hvaci->ductinfo + i;

    node_from = hvacducti->node_from;
    node_to   = hvacducti->node_to;
    if (node_from == NULL || node_to == NULL)continue;
    glVertex3fv(node_from->xyz);
    glVertex3fv(node_to->xyz);
  }
  glEnd();

  // draw nodes
  glPointSize(hvac_node_size);
  glBegin(GL_POINTS);
  for (i = 0; i < hvaci->n_nodes; i++) {
    hvacnodedata *nodei;

    nodei = hvaci->nodeinfo + i;

    if (nodei->use_node == 0)continue;
    glVertex3fv(nodei->xyz);
  }
  glEnd();

  glPopMatrix();
}

/* ------------------ DrawHVACS ------------------------ */

void DrawHVACS() {
  int i;

  for(i=0; i<nhvacinfo; i++){
    hvacdata *hvaci;

    hvaci = hvacinfo + i;
    if(hvaci->display==0)continue;
    DrawHVAC(hvaci);
  }
}

#endif
