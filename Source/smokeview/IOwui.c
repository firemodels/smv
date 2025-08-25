#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "IOobjects.h"

#define ijnode2(i,j) ((nxcell+1)*(j) + (i))

#define FORTWUIREAD(var,size) FSEEK(WUIFILE,4,SEEK_CUR);\
                           returncode=fread(var,4,size,WUIFILE);\
                           FSEEK(WUIFILE,4,SEEK_CUR)

/* ------------------ GenerateTerrainGeom ------------------------ */

void GenerateTerrainGeom(float **vertices_arg, unsigned int **indices_arg, int *nindices_arg){
  geomlistdata *terrain;
  int i, sizeof_indices, sizeof_vertices, sizeof_tvertices, terrain_nindices_local;
  float terrain_xmin, terrain_xmax, terrain_ymin, terrain_ymax;

  if(global_scase.geominfo->geomlistinfo==NULL)return;
  terrain = global_scase.geominfo->geomlistinfo - 1;

  sizeof_vertices  = 9*terrain->nverts*sizeof(float);
  sizeof_tvertices = 2*terrain->nverts*sizeof(float);
  NewMemory((void **)&terrain_vertices, sizeof_vertices);
  NewMemory((void **)&terrain_tvertices, sizeof_tvertices);
  terrain_xmin = terrain->verts[0].xyz[0];
  terrain_xmax = terrain_xmin;
  terrain_ymin = terrain->verts[0].xyz[1];
  terrain_ymax = terrain_ymin;
  for(i = 1; i<terrain->nverts; i++){
    vertdata *verti;
    float *xyz;

    verti = terrain->verts+i;
    xyz = verti->xyz;
    terrain_xmin = MIN(terrain_xmin, xyz[0]);
    terrain_xmax = MAX(terrain_xmax, xyz[0]);
    terrain_ymin = MIN(terrain_ymin, xyz[1]);
    terrain_ymax = MAX(terrain_ymax, xyz[1]);
  }
  for(i = 0; i<terrain->nverts; i++){
    vertdata *verti;
    surfdata *geomsurf;
    float *xyz;

    verti = terrain->verts+i;
    xyz = verti->xyz;

    memcpy(terrain_vertices + 9*i,     xyz,              3*sizeof(float));
    memcpy(terrain_vertices + 9*i + 3, verti->vert_norm, 3*sizeof(float));
    terrain_tvertices[2*i+0] = (xyz[0]-terrain_xmin)/(terrain_xmax-terrain_xmin);
    terrain_tvertices[2*i+1] = (xyz[1]-terrain_ymin)/(terrain_ymax-terrain_ymin);
    geomsurf = verti->triangles[0]->geomsurf;
    if(geomsurf!=NULL){
      memcpy(terrain_vertices + 9*i + 6, geomsurf->color, 3*sizeof(float));
    }
    else{
      terrain_vertices[9*i+6] = 0.0;
      terrain_vertices[9*i+7] = 0.0;
      terrain_vertices[9*i+8] = 1.0;
    }
  }

  sizeof_indices = 3*terrain->ntriangles*sizeof(unsigned int);
  NewMemory((void **)&terrain_indices, sizeof_indices);
  NewMemory((void **)&terrain_colors, 3*terrain->ntriangles*sizeof(float));
  for(i = 0; i<terrain->ntriangles; i++){
    tridata *trii;
    int i0, i1, i2;
    surfdata *geomsurf;

    trii = terrain->triangles+i;
    geomsurf = trii->geomsurf;

    i0 = trii->verts[0]-terrain->verts;
    i1 = trii->verts[1]-terrain->verts;
    i2 = trii->verts[2]-terrain->verts;
    terrain_indices[3*i+0] = i0;
    terrain_indices[3*i+1] = i1;
    terrain_indices[3*i+2] = i2;
    if(geomsurf!=NULL){
      memcpy(terrain_colors + 3*i, geomsurf->color, 3*sizeof(float));
    }
    else{
      terrain_colors[3*i+0] = 0.0;
      terrain_colors[3*i+1] = 0.0;
      terrain_colors[3*i+2] = 1.0;
    }
  }
  terrain_nindices_local = 3*terrain->ntriangles;
  terrain_nfaces = terrain->ntriangles;
  *vertices_arg = terrain_vertices;
  *indices_arg = terrain_indices;
  *nindices_arg = terrain_nindices_local;
}

/* ------------------ InDomain ------------------------ */

int InDomain(float *v1, float *v2, float *v3){
  int inside1 = 0, inside2 = 0, inside3 = 0;
  float vmid[3];

  if(v1[0]>=xbar0FDS&&v1[0]<=xbarFDS&&
     v1[1]>=ybar0FDS&&v1[1]<=ybarFDS&&
     v1[2]>=zbar0FDS&&v1[2]<=zbarFDS)inside1=1;

  if(v2[0]>=xbar0FDS&&v2[0]<=xbarFDS&&
     v2[1]>=ybar0FDS&&v2[1]<=ybarFDS&&
     v2[2]>=zbar0FDS&&v2[2]<=zbarFDS)inside2 = 1;

  if(v3[0]>=xbar0FDS&&v3[0]<=xbarFDS&&
     v3[1]>=ybar0FDS&&v3[1]<=ybarFDS&&
     v3[2]>=zbar0FDS&&v3[2]<=zbarFDS)inside3 = 1;
  if(inside1==1&&inside2==1&&inside3==1)return 1; // all points are inside
  if(inside1==0&&inside2==0&&inside3==0)return 0; // all points are outside
  vmid[0] = (v1[0]+v2[0]+v3[0])/3.0;
  vmid[1] = (v1[1]+v2[1]+v3[1])/3.0;
  vmid[2] = (v1[2]+v2[2]+v3[2])/3.0;
  if(vmid[0]>=xbar0FDS&&vmid[0]<=xbarFDS&&
     vmid[1]>=ybar0FDS&&vmid[1]<=ybarFDS&&
     vmid[2]>=zbar0FDS&&vmid[2]<=zbarFDS)return 1; // center is inside
  return 0;                                        // center is outside
}

/* ------------------ HaveTerrainTexture ------------------------ */

int HaveTerrainTexture(int *draw_surfaceptr){
  int draw_texture = 0, draw_surface = 1;
  int i;

  for(i = 0; i < global_scase.terrain_texture_coll.nterrain_textures; i++){
    texturedata *texti;

    texti = global_scase.terrain_texture_coll.terrain_textures + i;
    if(texti->loaded == 1 && texti->display == 1){
      draw_texture = 1;
      if(texti->is_transparent == 0)draw_surface = 0; // don't draw a surface if we are drawing a texture
    }
  }
  if(draw_surfaceptr!=NULL)*draw_surfaceptr = draw_surface;
  return draw_texture;
}

/* ------------------ GetNTerrainTexturesLoaded ------------------------ */

int GetNTerrainTexturesLoaded(void){
  int count, i, opaque_texture_index = -1;

  for(i = 0; i < global_scase.terrain_texture_coll.nterrain_textures; i++){
    texturedata *texti;

    texti = global_scase.terrain_texture_coll.terrain_textures + i;
    if(texti->loaded == 1 && texti->display == 1 && texti->is_transparent == 0){
      opaque_texture_index = i;
      break;
    }
  }

  count = 0;
  for(i = -1; i<global_scase.terrain_texture_coll.nterrain_textures; i++){
    texturedata *texti;

    if(i==-1){
      if(opaque_texture_index==-1)continue;
      texti = global_scase.terrain_texture_coll.terrain_textures+opaque_texture_index;
    }
    else{
      if(i==opaque_texture_index)continue;
      texti = global_scase.terrain_texture_coll.terrain_textures+i;
    }
    if(texti->loaded==0||texti->display==0)continue;
    count++;
  }
  return count;
}

/* ------------------ DrawTerrainGeom ------------------------ */

void DrawTerrainGeom(int option){
  int i;
  float terrain_shininess = 100.0;
  float terrain_specular[4] = {0.8, 0.8, 0.8, 1.0};
  float neutral_color[4] = {0.91, 0.91, 0.76, 1.0};
  int draw_surface = 1, draw_texture=0;
  int showgeom_inside_domain_local;

  draw_texture = HaveTerrainTexture(&draw_surface);
  if(terrain_nindices<=0)return;
  if(hide_scene == 1 && mouse_down == 1){
    DrawGeomBoundingBox(foregroundcolor);
    return;
  }

  showgeom_inside_domain_local = showgeom_inside_domain;
  if(drawing_boundary_files==1)showgeom_inside_domain_local = 0; // hide terrain within FDS domain if drawing boundary files

  if(option==DRAW_TRANSPARENT&&draw_texture==0)return;

  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);
  ENABLE_LIGHTING;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &terrain_shininess);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, rgbterrain);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, terrain_specular);
  glEnable(GL_COLOR_MATERIAL);

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);

  if(option==DRAW_OPAQUE){

    //*** surface

    if(show_faces_shaded==1&&draw_surface==1){
      if(show_texture_1dimage==1){
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, terrain_colorbar_id);
      }
      glBegin(GL_TRIANGLES);

      // surface
      for(i = 0; i<terrain_nindices/3; i++){
        float *v1, *v2, *v3;
        float v1o[3], v2o[3], v3o[3];
        float *c1;
        float *n1, *n2, *n3;
        int j;
        unsigned int *ind;
        int inside_domain=0, outside_domain=1;

        ind = terrain_indices+3*i;

        c1 = terrain_colors+3*i;

        v1 = terrain_vertices+9*ind[0];
        v2 = terrain_vertices+9*ind[1];
        v3 = terrain_vertices+9*ind[2];
        for(j = 0; j<3; j++){
          v1o[j] = v1[j];
          v2o[j] = v2[j];
          v3o[j] = v3[j];
          if(j==2){
            v1o[j] = terrain_zmin+geom_vert_exag*(v1o[j]-terrain_zmin);
            v2o[j] = terrain_zmin+geom_vert_exag*(v2o[j]-terrain_zmin);
            v3o[j] = terrain_zmin+geom_vert_exag*(v3o[j]-terrain_zmin);
          }
        }

        if(showgeom_inside_domain_local==0||showgeom_outside_domain==0){
          inside_domain = InDomain(v1, v2, v3);
          outside_domain = 1-inside_domain;
        }

        if(showgeom_inside_domain_local==0&&inside_domain==1)continue;
        if(showgeom_outside_domain==0&&outside_domain==1)continue;

        n1 = v1+3;
        if(terrain_showonly_top==1&&n1[2]<0.0)continue;

        if(show_texture_1dimage==0)glColor3fv(c1);
        glNormal3fv(n1);
        if(show_texture_1dimage==1){
          float texture_z;

          texture_z = (v1[2]-terrain_zmin)/(terrain_zmax-terrain_zmin);
          glTexCoord1f(texture_z);
        }
        glVertex3fv(v1o);

        n2 = v2+3;
        glNormal3fv(n2);
        if(show_texture_1dimage==1){
          float texture_z;

          texture_z = (v2[2]-terrain_zmin)/(terrain_zmax-terrain_zmin);
          glTexCoord1f(texture_z);
        }
        glVertex3fv(v2o);

        n3 = v3+3;
        glNormal3fv(n3);
        if(show_texture_1dimage==1){
          float texture_z;

          texture_z = (v3[2]-terrain_zmin)/(terrain_zmax-terrain_zmin);
          glTexCoord1f(texture_z);
        }
        glVertex3fv(v3o);
      }
      glEnd();
      if(show_texture_1dimage == 1)glDisable(GL_TEXTURE_1D);
    }

#define FDS_OFFSET 0.005

    //*** edges

    if(use_cfaces==0&&show_faces_outline==1){
      glPushMatrix();
      glTranslatef(0.0, 0.0, geom_dz_offset);
      glLineWidth(geom_linewidth);
      glBegin(GL_LINES);

      // lines
      unsigned char outlinecolor_uc[4];
      if(visGrid != 0){
        outlinecolor_uc[0] = (unsigned char)glui_outlinecolor[0];
        outlinecolor_uc[1] = (unsigned char)glui_outlinecolor[1];
        outlinecolor_uc[2] = (unsigned char)glui_outlinecolor[2];
        outlinecolor_uc[3] = (unsigned char)glui_outlinecolor[3];
      }
      else{
        outlinecolor_uc[0] = 0;
        outlinecolor_uc[1] = 0;
        outlinecolor_uc[2] = 0;
        outlinecolor_uc[3] = 255;
      }
      glColor4ubv(outlinecolor_uc);
      for(i = 0; i<terrain_nfaces; i++){
        int j;
        int inside_domain=0, outside_domain=1;

        float *v1, *v2, *v3;
        float v1o[3], v2o[3], v3o[3];
        float *n1;
        unsigned int *ind;

        ind = terrain_indices+3*i;

        v1 = terrain_vertices+9*ind[0];
        v2 = terrain_vertices+9*ind[1];
        v3 = terrain_vertices+9*ind[2];

        if(showgeom_inside_domain_local==0||showgeom_outside_domain==0){
          inside_domain = InDomain(v1, v2, v3);
          outside_domain = 1-inside_domain;
        }

        if(showgeom_inside_domain_local==0&&inside_domain==1)continue;
        if(showgeom_outside_domain==0&&outside_domain==1)continue;

        n1 = v1+3;
        if(terrain_showonly_top==1&&n1[2]<0.0)continue;

        for(j = 0; j<3; j++){
          v1o[j] = v1[j];
          v2o[j] = v2[j];
          v3o[j] = v3[j];
          if(j==2){
            v1o[j] = terrain_zmin+geom_vert_exag*(v1o[j]-terrain_zmin);
            v2o[j] = terrain_zmin+geom_vert_exag*(v2o[j]-terrain_zmin);
            v3o[j] = terrain_zmin+geom_vert_exag*(v3o[j]-terrain_zmin);
          }
        }

        glVertex3fv(v1o);
        glVertex3fv(v2o);

        glVertex3fv(v2o);
        glVertex3fv(v3o);

        glVertex3fv(v3o);
        glVertex3fv(v1o);
      }
      glEnd();
      glPopMatrix();
    }

    //*** vertices

    if(show_geom_verts==1){
      glPushMatrix();
      glTranslatef(0.0, 0.0, geom_dz_offset);
      glPointSize(geom_pointsize);
      glBegin(GL_POINTS);

      // points
      glColor3fv(foregroundcolor);
      for(i = 0; i<terrain_nfaces; i++){
        int j;
        int inside_domain=0, outside_domain=1;

        float *v1, *v2, *v3;
        float v1o[3], v2o[3], v3o[3];
        float *n1;
        unsigned int *ind;

        ind = terrain_indices+3*i;

        v1 = terrain_vertices+9*ind[0];
        v2 = terrain_vertices+9*ind[1];
        v3 = terrain_vertices+9*ind[2];

        if(showgeom_inside_domain_local==0||showgeom_outside_domain==0){
          inside_domain = InDomain(v1, v2, v3);
          outside_domain = 1-inside_domain;
        }

        if(showgeom_inside_domain_local==0&&inside_domain==1)continue;
        if(showgeom_outside_domain==0&&outside_domain==1)continue;

        n1 = v1+3;
        if(terrain_showonly_top==1&&n1[2]<0.0)continue;

        for(j = 0; j<3; j++){
          v1o[j] = v1[j];
          v2o[j] = v2[j];
          v3o[j] = v3[j];
          if(j==2){
            v1o[j] = terrain_zmin+geom_vert_exag*(v1o[j]-terrain_zmin);
            v2o[j] = terrain_zmin+geom_vert_exag*(v2o[j]-terrain_zmin);
            v3o[j] = terrain_zmin+geom_vert_exag*(v3o[j]-terrain_zmin);
          }
        }

        glVertex3fv(v1o);
        glVertex3fv(v2o);
        glVertex3fv(v3o);
      }
      glEnd();
      glPopMatrix();
    }

    //*** bottom side of top surface

    if(terrain_showonly_top==1&&(show_faces_shaded==1||draw_texture==1)){
      glBegin(GL_TRIANGLES);

      // surface
      for(i = 0; i<terrain_nindices/3; i++){
        float *v1, *v2, *v3;
        float v1o[3], v2o[3], v3o[3];
        float *n1, *n2, *n3;
        int j;
        unsigned int *ind;
        float n1n[3], n2n[3], n3n[3];
        int inside_domain=0, outside_domain=1;

        ind = terrain_indices+3*i;


        v1 = terrain_vertices+9*ind[0];
        v2 = terrain_vertices+9*ind[1];
        v3 = terrain_vertices+9*ind[2];

        for(j = 0; j<3; j++){
          v1o[j] = v1[j];
          v2o[j] = v2[j];
          v3o[j] = v3[j];
          if(j==2){
            v1o[j] = terrain_zmin+geom_vert_exag*(v1o[j]-terrain_zmin);
            v2o[j] = terrain_zmin+geom_vert_exag*(v2o[j]-terrain_zmin);
            v3o[j] = terrain_zmin+geom_vert_exag*(v3o[j]-terrain_zmin);
          }
        }

        if(showgeom_inside_domain_local==0||showgeom_outside_domain==0){
          inside_domain = InDomain(v1, v2, v3);
          outside_domain = 1-inside_domain;
        }

        if(showgeom_inside_domain_local==0&&inside_domain==1)continue;
        if(showgeom_outside_domain==0&&outside_domain==1)continue;

        n1 = v1+3;
        if(n1[2]<0.0)continue;
        n1n[0] = -n1[0];
        n1n[1] = -n1[1];
        n1n[2] = -n1[2];
        glColor4fv(neutral_color);

        glNormal3fv(n1n);
        glVertex3fv(v1o);

        n3 = v3+3;
        n3n[0] = -n3[0];
        n3n[1] = -n3[1];
        n3n[2] = -n3[2];
        glNormal3fv(n3n);
        glVertex3fv(v3o);

        n2 = v2+3;
        n2n[0] = -n2[0];
        n2n[1] = -n2[1];
        n2n[2] = -n2[2];
        glNormal3fv(n2n);
        glVertex3fv(v2o);
      }
      glEnd();
    }

    //*** draw sides in a neutral color

    if(terrain_showonly_top==0&&(show_faces_shaded==1||draw_texture==1)){
      glBegin(GL_TRIANGLES);

      // surface
      for(i = 0; i<terrain_nindices/3; i++){
        float *v1, *v2, *v3;
        float v1o[3], v2o[3], v3o[3];
        int j;
        float *n1, *n2, *n3;
        unsigned int *ind;
        int inside_domain=0, outside_domain=1;

        ind = terrain_indices+3*i;

        v1 = terrain_vertices+9*ind[0];
        v2 = terrain_vertices+9*ind[1];
        v3 = terrain_vertices+9*ind[2];

        for(j = 0; j<3; j++){
          v1o[j] = v1[j];
          v2o[j] = v2[j];
          v3o[j] = v3[j];
          if(j==2){
            v1o[j] = terrain_zmin+geom_vert_exag*(v1o[j]-terrain_zmin);
            v2o[j] = terrain_zmin+geom_vert_exag*(v2o[j]-terrain_zmin);
            v3o[j] = terrain_zmin+geom_vert_exag*(v3o[j]-terrain_zmin);
          }
        }

        if(showgeom_inside_domain_local==0||showgeom_outside_domain==0){
          inside_domain = InDomain(v1, v2, v3);
          outside_domain = 1-inside_domain;
        }

        if(showgeom_inside_domain_local==0&&inside_domain==1)continue;
        if(showgeom_outside_domain==0&&outside_domain==1)continue;

        n1 = v1+3;
        if(n1[2]>=0.0)continue;
        glColor4fv(neutral_color);
        glNormal3fv(n1);
        glVertex3fv(v1o);

        n2 = v2+3;
        glNormal3fv(n2);
        glVertex3fv(v2o);

        n3 = v3+3;
        glNormal3fv(n3);
        glVertex3fv(v3o);
      }
      glEnd();
    }
  }
  else{
    // draw texture

    int ii;
    int opaque_texture_index = -1;

    for(i = 0; i<global_scase.terrain_texture_coll.nterrain_textures; i++){
      texturedata *texti;

      texti = global_scase.terrain_texture_coll.terrain_textures+i;
      if(texti->loaded==1&&texti->display==1&&texti->is_transparent==0){
        opaque_texture_index = i;
        break;
      }
    }

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_2D);

    int count = 0;
    int is_transparent=0;
    TransparentOff();
    for(ii = -1; ii<global_scase.terrain_texture_coll.nterrain_textures; ii++){
      float dz;
      texturedata *texti;

      // draw opaque texture first
      if(ii==-1){
        if(opaque_texture_index==-1)continue;
        texti = global_scase.terrain_texture_coll.terrain_textures+opaque_texture_index;
      }
      else{
        if(ii==opaque_texture_index)continue;
        texti = global_scase.terrain_texture_coll.terrain_textures+ii;
      }
      if(texti->loaded==0||texti->display==0)continue;
      dz = SCALE2FDS((float)(count)*FDS_OFFSET);
      count++;

      if(texti->is_transparent==1){
        if(is_transparent==0){
          is_transparent = 1;
          TransparentOn();
        }
      }
      else{
        if(is_transparent==1){
          is_transparent = 0;
          TransparentOff();
        }
      }
      glBindTexture(GL_TEXTURE_2D, texti->name);

      glBegin(GL_TRIANGLES);

      for(i = 0; i<terrain_nindices/3; i++){
        float *v1, *v2, *v3;
        float v1o[3], v2o[3], v3o[3];
        float *n1, *n2, *n3;
        float *t1, *t2, *t3;
        int j;
        unsigned int *ind;
        int inside_domain=0, outside_domain=1;

        ind = terrain_indices+3*i;

        v1 = terrain_vertices+9*ind[0];
        v2 = terrain_vertices+9*ind[1];
        v3 = terrain_vertices+9*ind[2];

        for(j = 0; j<3; j++){
          v1o[j] = v1[j];
          v2o[j] = v2[j];
          v3o[j] = v3[j];
          if(j==2){
            v1o[j] = terrain_zmin+geom_vert_exag*(v1o[j]-terrain_zmin);
            v2o[j] = terrain_zmin+geom_vert_exag*(v2o[j]-terrain_zmin);
            v3o[j] = terrain_zmin+geom_vert_exag*(v3o[j]-terrain_zmin);
          }
        }

        if(showgeom_inside_domain_local==0||showgeom_outside_domain==0){
          inside_domain = InDomain(v1, v2, v3);
          outside_domain = 1-inside_domain;
        }

        if(showgeom_inside_domain_local==0&&inside_domain==1)continue;
        if(showgeom_outside_domain==0&&outside_domain==1)continue;

        t1 = terrain_tvertices+2*ind[0];
        n1 = v1+3;
        if(n1[2]<0.0)continue;
        glNormal3fv(n1);
        glTexCoord2fv(t1);
        glVertex3f(v1o[0], v1o[1], v1o[2]+dz);

        t2 = terrain_tvertices+2*ind[1];
        n2 = v2+3;
        glTexCoord2fv(t2);
        glNormal3fv(n2);
        glVertex3f(v2o[0], v2o[1], v2o[2]+dz);

        t3 = terrain_tvertices+2*ind[2];
        n3 = v3+3;
        glTexCoord2fv(t3);
        glNormal3fv(n3);
        glVertex3f(v3o[0], v3o[1], v3o[2]+dz);
      }
      glEnd();
    }
    glDisable(GL_TEXTURE_2D);
    if(is_transparent==1)TransparentOff();
  }

  glPopMatrix();
  glDisable(GL_COLOR_MATERIAL);
  DISABLE_LIGHTING;
}

/* ------------------ DrawNorth ------------------------ */

void DrawNorth(void){
  glPushMatrix();
  glTranslatef(northangle_position[0], northangle_position[1], northangle_position[2]);
  glRotatef(-global_scase.northangle, 0.0, 0.0, 1.0);
  glBegin(GL_LINES);
  glColor3fv(foregroundcolor);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.1, 0.0);
  glEnd();
  glPopMatrix();
}

  /*
typedef struct {
  float xyz[3];
  float trunk_diam;
  float tree_height;
  float base_diam;
  float base_height;
*/

/* ------------------ DrawTrees ------------------------ */

void DrawTrees(void){
  int i;

 ENABLE_LIGHTING;

 glEnable(GL_COLOR_MATERIAL);

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
  for(i=0;i<global_scase.ntreeinfo;i++){
    treedata *treei;
    float crown_height;
    int state;

    treei = global_scase.treeinfo + i;

    state=0;
    if(showtime==1&&global_times!=NULL){
      assert(itimes>=0);
      if(treei->time_char>0.0&&global_times[itimes]>treei->time_char)state=1;
      if(treei->time_complete>0.0&&global_times[itimes]>treei->time_complete)state=2;
    }

    glPushMatrix();
    glTranslatef(treei->xyz[0],treei->xyz[1],treei->xyz[2]);

    switch(state){
      case 0:
        glColor4fv(trunccolor);
        DrawDisk(treei->trunk_diam,treei->base_height,trunccolor_uc);

        crown_height=treei->tree_height-treei->base_height;
        glTranslatef(0.0,0.0,treei->base_height);
        glColor4fv(treecolor);
        DrawCone(treei->base_diam,crown_height,treecolor_uc);
        break;
      case 1:
        glColor4fv(treecharcolor);
        DrawDisk(treei->trunk_diam,treei->base_height,trunccolor_uc);

        crown_height=treei->tree_height-treei->base_height;
        glTranslatef(0.0,0.0,treei->base_height);
        DrawCone(treei->base_diam,crown_height,treecolor_uc);
        break;
      case 2:
        glColor4fv(treecharcolor);
        DrawDisk(treei->trunk_diam,treei->base_height,trunccolor_uc);
        crown_height=treei->tree_height-treei->base_height;
        glTranslatef(0.0,0.0,treei->base_height);
        DrawCone(treei->trunk_diam,crown_height,trunccolor_uc);
        break;
      default:
        assert(FFALSE);
        break;
    }
    glPopMatrix();


  }
  glPopMatrix();


  glDisable(GL_COLOR_MATERIAL);

}

/* ------------------ GetZCellVal ------------------------ */

float GetZCellVal(meshdata *meshi,float xval, float yval, float *zval_offset, int *loc){
  int imesh;
  int meshstart=-1;

  if(meshi==NULL)meshstart=0;
  if(zval_offset!=NULL)*zval_offset=0.0;
  for(imesh=meshstart;imesh<global_scase.meshescoll.nmeshes;imesh++){
    meshdata *meshj;
    float *xplt, *yplt;
    int ibar, jbar;

    if(imesh==-1){
      meshj=meshi;
    }
    else{
      meshj=global_scase.meshescoll.meshinfo+imesh;
      if(meshi==meshj)continue;
    }
    xplt = meshj->xplt_fds;
    yplt = meshj->yplt_fds;
    ibar = meshj->ibar;
    jbar = meshj->jbar;
    if(xplt[0]<=xval&&xval<=xplt[ibar]&&yplt[0]<=yval&&yval<=yplt[jbar]){
      float dx, dy;
      terraindata *terri;
      int ival, jval;
      int nxcell;
      float zval;
      float *zcell;
      float zval_return;

      dx = xplt[1]-xplt[0];
      dy = yplt[1]-yplt[0];
      ival = (xval-xplt[0])/dx;
      if(ival>=ibar)ival=ibar-1;
      jval = (yval-yplt[0])/dy;
      if(jval>=jbar)jval=jbar-1;
      terri=meshj->terrain;
      nxcell = terri->ibar;
      zcell = terri->zcell;
      zval = zcell[IJCELL2(ival,jval)];
      *loc=1;
      zval_return = zterrain_min+vertical_factor*(zval-zterrain_min);
      if(zval_offset!=NULL){
        *zval_offset=(vertical_factor-1.0)*(zval-zterrain_min);
      }
      return zval_return;
    }
  }
  *loc=0;
  return 0.0;
}

/* ------------------ GetZCellValOffset ------------------------ */

float GetZCellValOffset(meshdata *meshi,float xval, float yval, int *loc){
  int imesh;
  int meshstart=-1;

  if(meshi==NULL)meshstart=0;

  for(imesh=meshstart;imesh<global_scase.meshescoll.nmeshes;imesh++){
    meshdata *meshj;
    float *xplt, *yplt;
    int ibar, jbar;

    if(imesh==-1){
      meshj=meshi;
    }
    else{
      meshj=global_scase.meshescoll.meshinfo+imesh;
      if(meshi==meshj)continue;
    }

// convert xval and yval to "user" units
    xval = SMV2FDS_X(xval);
    yval = SMV2FDS_Y(yval);

    xplt = meshj->xplt_fds;
    yplt = meshj->yplt_fds;
    ibar = meshj->ibar;
    jbar = meshj->jbar;
    if(xplt[0]<=xval&&xval<=xplt[ibar]&&yplt[0]<=yval&&yval<=yplt[jbar]){
      terraindata *terri;
      int ival, jval;
      float dx, dy;
      int nxcell;
      float *zcell,zval;
      float zvaloffset;

      dx = xplt[1]-xplt[0];
      dy = yplt[1]-yplt[0];
      ival = (xval-xplt[0])/dx;
      if(ival>=ibar)ival=ibar-1;
      jval = (yval-yplt[0])/dy;
      if(jval>=jbar)jval=jbar-1;
      terri=meshj->terrain;
      nxcell = terri->ibar;
      zcell = terri->zcell;
      zval = zcell[IJCELL2(ival,jval)];
      *loc=1;
      zvaloffset = zterrain_min+vertical_factor*(zval-zterrain_min)-zval;

    // convert zoffset back to smokeview/scaled units

      zvaloffset = SCALE2SMV(zvaloffset);
      return zvaloffset;
    }
  }
  *loc=0;
  return 0.0;
}

/* ------------------ UpdateTerrainColors ------------------------ */

void UpdateTerrainColors(void){
  int i;

  for(i=0;i<MAXRGB;i++){
    float f1;

    f1 = (float)i/(float)(MAXRGB-1);
    rgbterrain[4*i  ]=((1.0-f1)*terrain_rgba_zmin[0] + f1*terrain_rgba_zmax[0])/255.0;
    rgbterrain[4*i+1]=((1.0-f1)*terrain_rgba_zmin[1] + f1*terrain_rgba_zmax[1])/255.0;
    rgbterrain[4*i+2]=((1.0-f1)*terrain_rgba_zmin[2] + f1*terrain_rgba_zmax[2])/255.0;
    rgbterrain[4*i+3]=1.0;
  }
}

/* ------------------ GetZTerrain ------------------------ */

float GetZTerrain(float x, float y){
  int loc;
  float zterrain;

  zterrain = GetZCellVal(NULL, x, y, NULL, &loc);
  return zterrain;
}

/* ------------------ ComputeTerrainNormalsManual ------------------------ */

void ComputeTerrainNormalsManual(void){
  int imesh;

  for(imesh=0;imesh<global_scase.meshescoll.nmeshes;imesh++){
    meshdata *meshi;
    terraindata *terri;
    float *znode;
    int j;
    int nycell;

    meshi = global_scase.meshescoll.meshinfo + imesh;
    terri = meshi->terrain;
    if(terri==NULL)continue;
    znode = terri->znode;
    nycell = meshi->jbar;

    for(j=0;j<=terri->jbar;j++){
      int i;
      float *yplt;

      yplt = terri->yplt+j;
      for(i=0;i<=terri->ibar;i++){
        float dzdx, dzdy, sum, znormal3[3];
        unsigned char *uc_znormal;
        float *xplt;

     //     i      j    k
     //     1      0    dzdx
     //     0      1    dzdy

     //     -dzdx -dzdy 1

        xplt = terri->xplt+i;
        if(i==0){
          dzdx = (znode[IJ2(i+1,j)]  -znode[IJ2(i,j)])/(xplt[1]-xplt[0]);
        }
        else if(i==terri->ibar){
          dzdx = (znode[IJ2(i, j)]-znode[IJ2(i-1, j)])/(xplt[0]-terri->xplt[-1]);
        }
        else{
          float dx1, dx3;
          float dz1, dz3;

          dx1 = (xplt[0]-xplt[-1]);
          dx3 = (xplt[1]-xplt[0]);
          dz1 = (znode[IJ2(i,j)]  -znode[IJ2(i-1,j)]);
          dz3 = (znode[IJ2(i+1, j)]-znode[IJ2(i, j)]);
          dzdx = ((dz1/dx1)*dx3+(dz3/dx3)*dx1)/(dx1+dx3);
        }

        if(j==0){
          dzdy = (znode[IJ2(i, j+1)]-znode[IJ2(i, j)])/(yplt[1]-yplt[0]);
        }
        else if(j==terri->jbar){
          dzdy = (znode[IJ2(i, j)]-znode[IJ2(i, j-1)])/(yplt[0]-yplt[-1]);
        }
        else{
          float dy1, dy3;
          float dz1, dz3;

          dy1 = (yplt[0]-yplt[-1]);
          dy3 = (yplt[1]-yplt[0]);
          dz1 = (znode[IJ2(i, j)]-znode[IJ2(i, j-1)]);
          dz3 = (znode[IJ2(i, j+1)]-znode[IJ2(i, j)]);
          dzdy = ((dz1/dy1)*dy3+(dz3/dy3)*dy1)/(dy1+dy3);
        }

        uc_znormal = terri->uc_znormal + IJ2(i,j);
        znormal3[0] = -dzdx;
        znormal3[1] = -dzdy;
        znormal3[2] = 1.0;

        sum  = znormal3[0]*znormal3[0];
        sum += znormal3[1]*znormal3[1];
        sum += 1.0;
        sum = sqrt(sum);
        znormal3[0]/=sum;
        znormal3[1]/=sum;
        znormal3[2]/=sum;
        *uc_znormal = GetNormalIndex(global_scase.wui_sphereinfo, znormal3);
      }
    }
  }
}

/* ------------------ ComputeTerrainNormalsAuto ------------------------ */

void ComputeTerrainNormalsAuto(void){
  int imesh;
  float zmin, zmax;

  for(imesh=0;imesh<global_scase.meshescoll.nmeshes;imesh++){
    meshdata *meshi;
    terraindata *terri;
    int j;
    float dx, dy;
    float *znode, *znode_offset;
    int nycell;
    unsigned char *uc_znormal;

    meshi = global_scase.meshescoll.meshinfo + imesh;

    terri = meshi->terrain;

    dx = terri->xplt[1]-terri->xplt[0];
    dy = terri->yplt[1]-terri->yplt[0];

    znode = terri->znode;
    znode_offset = terri->znode_offset;
    nycell = terri->jbar;

    for(j=0;j<=terri->jbar;j++){
      int i;
      float ynode;

      ynode = terri->yplt[j];

      for(i=0;i<=terri->ibar;i++){
        float xnode;
        int count, loc1, loc2, loc3, loc4;
        float val1, val2, val3, val4;
        float val1_offset, val2_offset, val3_offset, val4_offset;
        float valx1a, valx2a, valx3a, valx4a;
        float valx1b, valx2b, valx3b, valx4b;
        float valy1a, valy2a, valy3a, valy4a;
        float valy1b, valy2b, valy3b, valy4b;
        float zval, zval_offset;
        float zvalxa, zvalxb;
        float zvalya, zvalyb;
        float dxa, dxb, dya, dyb;
        float dzdx, dzdy;
        float sum;
        float znormal3[3];
        float denom;

        xnode = terri->xplt[i];

        val1 =  GetZCellVal(meshi,xnode-dx/2.0,ynode-dy/2.0,&val1_offset,&loc1);
        val2 =  GetZCellVal(meshi,xnode+dx/2.0,ynode-dy/2.0,&val2_offset,&loc2);
        val3 =  GetZCellVal(meshi,xnode+dx/2.0,ynode+dy/2.0,&val3_offset,&loc3);
        val4 =  GetZCellVal(meshi,xnode-dx/2.0,ynode+dy/2.0,&val4_offset,&loc4);
        count = loc1 + loc2 + loc3 + loc4;

        zval = (val1*loc1 + val2*loc2 + val3*loc3 + val4*loc4)/(float)MAX(1,count);

        znode[IJ2(i,j)]=zval;
        zval_offset = SCALE2SMV((val1_offset*loc1 + val2_offset*loc2 + val3_offset*loc3 + val4_offset*loc4))/(float)MAX(1,count);

        *znode_offset++=zval_offset;

 // compute (f(x+dx,y) - f(x-dx,y))/(2*dx)

        valx1a =  GetZCellVal(meshi,xnode-dx-dx/2.0,ynode-dy/2.0,NULL,&loc1);
        valx2a =  GetZCellVal(meshi,xnode-dx+dx/2.0,ynode-dy/2.0,NULL,&loc2);
        valx3a =  GetZCellVal(meshi,xnode-dx+dx/2.0,ynode+dy/2.0,NULL,&loc3);
        valx4a =  GetZCellVal(meshi,xnode-dx-dx/2.0,ynode+dy/2.0,NULL,&loc4);
        count = loc1 + loc2 + loc3 + loc4;
        zvalxa = valx1a*loc1 + valx2a*loc2 + valx3a*loc3 + valx4a*loc4;
        if(count==0){
          zvalxa = zval;
          dxa = 0.0;
        }
        else{
          zvalxa /= (float)count;
          dxa = dx;
        }
        valx1b =  GetZCellVal(meshi,xnode+dx-dx/2.0,ynode-dy/2.0,NULL,&loc1);
        valx2b =  GetZCellVal(meshi,xnode+dx+dx/2.0,ynode-dy/2.0,NULL,&loc2);
        valx3b =  GetZCellVal(meshi,xnode+dx+dx/2.0,ynode+dy/2.0,NULL,&loc3);
        valx4b =  GetZCellVal(meshi,xnode+dx-dx/2.0,ynode+dy/2.0,NULL,&loc4);
        count = loc1 + loc2 + loc3 + loc4;
        zvalxb = valx1b*loc1 + valx2b*loc2 + valx3b*loc3 + valx4b*loc4;
        if(count==0){
          zvalxb = zval;
          dxb = 0.0;
        }
        else{
          zvalxb /= (float)count;
          dxb = dx;
        }
        denom = dxa+dxb;
        if(denom==0.0){
          dzdx=1.0;
        }
        else{
          dzdx = (zvalxb - zvalxa)/denom;
        }

 // compute (f(x,y+dy) - f(x,y-dy))/(2*dy)

        valy1a =  GetZCellVal(meshi,xnode-dx/2.0,ynode-dy-dy/2.0,NULL,&loc1);
        valy2a =  GetZCellVal(meshi,xnode+dx/2.0,ynode-dy-dy/2.0,NULL,&loc2);
        valy3a =  GetZCellVal(meshi,xnode+dx/2.0,ynode-dy+dy/2.0,NULL,&loc3);
        valy4a =  GetZCellVal(meshi,xnode-dx/2.0,ynode-dy+dy/2.0,NULL,&loc4);
        count = loc1 + loc2 + loc3 + loc4;
        zvalya = valy1a*loc1 + valy2a*loc2 + valy3a*loc3 + valy4a*loc4;
        if(count==0){
          zvalya = zval;
          dya = 0.0;
        }
        else{
          zvalya /= (float)count;
          dya = dy;
        }
        valy1b =  GetZCellVal(meshi,xnode-dx/2.0,ynode+dy-dy/2.0,NULL,&loc1);
        valy2b =  GetZCellVal(meshi,xnode+dx/2.0,ynode+dy-dy/2.0,NULL,&loc2);
        valy3b =  GetZCellVal(meshi,xnode+dx/2.0,ynode+dy+dy/2.0,NULL,&loc3);
        valy4b =  GetZCellVal(meshi,xnode-dx/2.0,ynode+dy+dy/2.0,NULL,&loc4);
        count = loc1 + loc2 + loc3 + loc4;
        zvalyb = valy1b*loc1 + valy2b*loc2 + valy3b*loc3 + valy4b*loc4;
        if(count==0){
          zvalyb = zval;
          dyb = 0.0;
        }
        else{
          zvalyb /= (float)count;
          dyb = dy;
        }
        denom = dya + dyb;
        if(denom==0.0){
          dzdy=1.0;
        }
        else{
          dzdy = (zvalyb - zvalya)/denom;
        }

     //     i  j  k
     //     1  0 dzdx
     //     0  1 dzdy

     //     -dzdx -dzdy 1

        //znormal = terri->znormal + 3*ijnode2(i,j);
        uc_znormal = terri->uc_znormal + IJ2(i,j);
        znormal3[0] = -dzdx;
        znormal3[1] = -dzdy;
        znormal3[2] = 1.0;

        sum  = znormal3[0]*znormal3[0];
        sum += znormal3[1]*znormal3[1];
        sum += znormal3[2]*znormal3[2];
        sum = sqrt(sum);
        znormal3[0]/=sum;
        znormal3[1]/=sum;
        znormal3[2]/=sum;
        *uc_znormal = GetNormalIndex(global_scase.wui_sphereinfo, znormal3);
      }
    }
  }

  zmin = global_scase.meshescoll.meshinfo->terrain->znode[0];
  zmax = zmin;
  for(imesh=0;imesh<global_scase.meshescoll.nmeshes;imesh++){
    meshdata *meshi;
    terraindata *terri;
    int i;

    meshi = global_scase.meshescoll.meshinfo + imesh;
    terri = meshi->terrain;

    for(i=0;i<(terri->ibar+1)*(terri->jbar+1);i++){
      float *znode;

      znode = terri->znode+i;
      zmin = MIN(zmin,*znode);
      zmax = MAX(zmax,*znode);
    }
  }
}


/* ------------------ GetTerrainData ------------------------ */

int GetTerrainData(char *file, terraindata *terri){
  FILE *WUIFILE;
  float zmin_cutoff;
  int ibp1, jbp1, ijbar[2];
  float *xplt, *yplt, *z_terrain;
  int returncode = 1;
  int nvalues, i;

#ifdef _DEBUG
  printf("reading terrain data mesh: %i\n", (int)(terri-global_scase.terraininfo));
#endif
  WUIFILE = FOPEN(file, "rb");
  if(WUIFILE==NULL)return 1;

//    WRITE(LU_TERRAIN(NM)) REAL(M%ZS-1._EB, FB)
//    WRITE(LU_TERRAIN(NM)) IBP1, JBP1
//    WRITE(LU_TERRAIN(NM)) (M%XPLT(I), I = 0, IBAR)
//    WRITE(LU_TERRAIN(NM)) (M%YPLT(J), J = 0, JBAR)
//    WRITE(LU_TERRAIN(NM)) Z_TERRAIN

  FORTWUIREAD(&zmin_cutoff, 1);
  zmin_cutoff = global_scase.zbar0;
  zmin_cutoff -= 0.1;
  terri->zmin_cutoff = zmin_cutoff;
  FORTWUIREAD(ijbar, 2);
  ibp1 = ijbar[0];
  jbp1 = ijbar[1];

  NewMemory((void **)&xplt, ibp1*sizeof(float));
  NewMemory((void **)&yplt, jbp1*sizeof(float));
  terri->xplt = xplt;
  terri->yplt = yplt;
  FORTWUIREAD(xplt, ibp1);
  FORTWUIREAD(yplt, jbp1);

  if(terri->znode==NULL){
    NewMemory((void **)&z_terrain, ibp1*jbp1*sizeof(float));
    terri->znode = z_terrain;
  }
  else{
    z_terrain = terri->znode;
  }
  FORTWUIREAD(z_terrain, ibp1*jbp1);

  for(i = 0, nvalues = 0; i<ibp1*jbp1; i++){
    if(z_terrain[i]>zmin_cutoff)nvalues++;
  }
  terri->nvalues = nvalues;
  if(returncode!=0)returncode = 0;
  fclose(WUIFILE);
  return returncode;
}

/* ------------------ InitTerrainZNode ------------------------ */

void InitTerrainZNode(meshdata *meshi, terraindata *terri, float xmin, float xmax, int nx, float ymin, float ymax, int ny,
                       int allocate_memory){
  float dx, dy;
  float *x, *y;
  int i;

  if(meshi!=NULL){
    meshi->terrain = terri;
    meshi->nznodes = (nx+1)*(ny+1);
    if(allocate_memory==1){
      meshi->nznodes = (nx+1)*(ny+1);
      NewMemory((void **)&meshi->znodes_complete, (nx+1)*(ny+1)*sizeof(float));
    }
  }

  if(terri==NULL)return;

  terri->xmin = xmin;
  terri->xmax = xmax;
  terri->ymin = ymin;
  terri->ymax = ymax;
  terri->jbar = ny;
  nx = ABS(nx);
  terri->ibar = nx;

  if(allocate_memory==1){
    terri->xplt=NULL;
    terri->yplt=NULL;
    terri->zcell=NULL;
    terri->znode=NULL;
    terri->uc_znormal=NULL;
    terri->ter_texture=NULL;

    NewMemory((void **)&terri->xplt,(nx+1)*sizeof(float));
    NewMemory((void **)&terri->yplt,(ny+1)*sizeof(float));
    NewMemory((void **)&terri->zcell,nx*ny*sizeof(float));
    NewMemory((void **)&terri->znode,(nx+1)*(ny+1)*sizeof(float));
    NewMemory((void **)&terri->znode_offset,(nx+1)*(ny+1)*sizeof(float));
    NewMemory((void **)&terri->znode_scaled,(nx+1)*(ny+1)*sizeof(float));
    NewMemory((void **)&terri->uc_znormal,(nx+1)*(ny+1)*sizeof(unsigned char));
  }

  x = terri->xplt;
  y = terri->yplt;
  dx = (xmax-xmin)/nx;
  dy = (ymax-ymin)/ny;
  for(i=0;i<nx;i++){
    x[i] = xmin + dx*i;
  }
  x[nx] = xmax;

  for(i=0;i<ny;i++){
    y[i] = ymin + dy*i;
  }
  y[ny] = ymax;

  if(terri->file!=NULL&&terri->defined==0){
    GetTerrainData(terri->file, terri);
    terri->defined = 1;
  }
}

/* ------------------ DrawTerrainOBST ------------------------ */

void DrawTerrainOBST(terraindata *terri, int flag){
  float *znode;
  unsigned char *uc_znormal;
  int nycell;
  int i, j;
  float *x, *y;
  float terrain_color[4];
  float terrain_shininess=100.0;
  float terrain_specular[4]={0.8,0.8,0.8,1.0};
  float zcut;

#define ZOFFSET 0.001

  terrain_color[0]=0.47843;
  terrain_color[1]=0.45882;
  terrain_color[2]=0.18824;
  terrain_color[3]=1.0;

  zcut = terri->zmin_cutoff;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);

  ENABLE_LIGHTING;
  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&terrain_shininess);
  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,rgbterrain);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,terrain_specular);
  glEnable(GL_COLOR_MATERIAL);

  glBegin(GL_TRIANGLES);
  uc_znormal = terri->uc_znormal;
  znode = terri->znode;
  nycell = terri->jbar;
  x = terri->xplt;
  y = terri->yplt;
  glColor4fv(terrain_color);
  for(j=0;j<terri->jbar;j++){
    int jp1;

    jp1 = j + 1;

    for(i=0;i<terri->ibar;i++){
      unsigned char *uc_zn1, *uc_zn2, *uc_zn3, *uc_zn4;
      int ip1;
      float zval1, zval2, zval3, zval4;
      float *zn1, *zn2, *zn3, *zn4;
      int skip123, skip134;

      ip1 = i + 1;

      zval1 = znode[IJ2(i, j)];
      zval2 = znode[IJ2(ip1, j)];
      zval3 = znode[IJ2(ip1, jp1)];
      zval4 = znode[IJ2(i, jp1)];

      if(zval1<zcut&&zval2<zcut&&zval3<zcut&&zval4<zcut)continue;

      skip123 = 0;
      skip134 = 0;
      if(zval1<zcut||zval2<zcut||zval3<zcut)skip123=1;
      if(zval1<zcut||zval3<zcut||zval4<zcut)skip134=1;

      zval1 += ZOFFSET;
      zval2 += ZOFFSET;
      zval3 += ZOFFSET;
      zval4 += ZOFFSET;

      uc_zn1 = uc_znormal+IJ2(i,j);
      zn1 = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn1));

      uc_zn2 = uc_znormal+IJ2(ip1, j);
      zn2 = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn2));

      uc_zn3 = uc_znormal+IJ2(ip1, jp1);
      zn3 = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn3));

      uc_zn4 = uc_znormal+IJ2(i, jp1);
      zn4 = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn4));

      if(flag==TERRAIN_TOP_SIDE||flag==TERRAIN_BOTH_SIDES){
        if(skip123==0){
          glNormal3fv(zn1);
          glVertex3f(x[i], y[j], zval1);

          glNormal3fv(zn2);
          glVertex3f(x[i+1], y[j], zval2);

          glNormal3fv(zn3);
          glVertex3f(x[i+1], y[j+1], zval3);
        }

        if(skip134==0){
          glNormal3fv(zn1);
          glVertex3f(x[i], y[j], zval1);

          glNormal3fv(zn3);
          glVertex3f(x[i+1], y[j+1], zval3);

          glNormal3fv(zn4);
          glVertex3f(x[i], y[j+1], zval4);
        }
      }

      if(flag==TERRAIN_BOTTOM_SIDE||flag==TERRAIN_BOTH_SIDES){
        if(skip134==0){
          glNormal3fv(zn1);
          glVertex3f(x[i], y[j], zval1);

          glNormal3fv(zn4);
          glVertex3f(x[i], y[j+1], zval4);

          glNormal3fv(zn3);
          glVertex3f(x[i+1], y[j+1], zval3);
        }

        if(skip123==0){
          glNormal3fv(zn1);
          glVertex3f(x[i], y[j], zval1);

          glNormal3fv(zn3);
          glVertex3f(x[i+1], y[j+1], zval3);

          glNormal3fv(zn2);
          glVertex3f(x[i+1], y[j], zval2);
        }
      }
    }
  }
  glEnd();

  glDisable(GL_COLOR_MATERIAL);
  DISABLE_LIGHTING;

  if(show_terrain_normals==1||show_terrain_grid==1){
    glBegin(GL_LINES);
    uc_znormal = terri->uc_znormal;
    znode = terri->znode;
    nycell = terri->jbar;
    x = terri->xplt;
    y = terri->yplt;
    glColor3f(0.0, 0.0, 0.0);
    if(show_terrain_grid==1){
      for(j = 0; j<terri->jbar; j++){
        for(i = 0; i<terri->ibar; i++){
          float zval11, zval13, zval33, zval31;

          zval11 = znode[IJ2(i,     j)]+ZOFFSET;
          zval31 = znode[IJ2(i+1,   j)]+ZOFFSET;
          zval33 = znode[IJ2(i+1, j+1)]+ZOFFSET;
          zval13 = znode[IJ2(i,   j+1)]+ZOFFSET;

          glVertex3f(x[i],   y[j],   zval11);
          glVertex3f(x[i+1], y[j],   zval31);

          glVertex3f(x[i+1], y[j],   zval31);
          glVertex3f(x[i+1], y[j+1], zval33);

          glVertex3f(x[i+1], y[j+1], zval33);
          glVertex3f(x[i],   y[j+1], zval13);

          glVertex3f(x[i],   y[j+1], zval13);
          glVertex3f(x[i],   y[j],   zval11);
        }
      }
    }
    if(show_terrain_normals==1){
      for(j = 0; j<terri->jbar; j+=MAX(ABS(terrain_normal_skip),1)){
        for(i = 0; i<terri->ibar; i+= MAX(ABS(terrain_normal_skip), 1)){
          unsigned char *uc_zn;
          float zval11, *zn;

          zval11 = znode[IJ2(i,     j)]+ZOFFSET;

          uc_zn = uc_znormal+IJ2(i, j);
          zn = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn));

          glVertex3f(x[i], y[j], zval11);
          glVertex3f(x[i]  +terrain_normal_length*zn[0],
                     y[j]  +terrain_normal_length*zn[1],
                     zval11+terrain_normal_length*zn[2]);
        }
      }
    }
    glEnd();
  }
  glPopMatrix();

}

/* ------------------ DrawTerrainOBSTSides ------------------------ */

void DrawTerrainOBSTSides(meshdata *meshi){
  float *znode;
  int nycell;
  int i, j;
  float *x, *y;
  float terrain_color[4];
  terraindata *terri;
  int ibar, jbar;
  float zcutoff;

  terri = meshi->terrain;
  if(terri!=NULL){
    zcutoff = terri->zmin_cutoff;
  }
  else{
    zcutoff = meshi->zplt_fds[0]-0.1;
  }

  terrain_color[0] = 0.47843;
  terrain_color[1] = 0.45882;
  terrain_color[2] = 0.18824;
  terrain_color[3] = 1.0;

  glPushMatrix();
  glScalef(SCALE2SMV(mscale[0]), SCALE2SMV(mscale[1]), vertical_factor*SCALE2SMV(mscale[2]));
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);

  ENABLE_LIGHTING;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &global_scase.color_defs.block_shininess);

  glEnable(GL_COLOR_MATERIAL);
  glColor4fv(terrain_color);
  glBegin(GL_TRIANGLES);
  znode = NULL;
  if(terri!=NULL)znode = terri->znode;
  nycell = meshi->jbar;
  ibar   = meshi->ibar;
  jbar   = meshi->jbar;
  x      = meshi->xplt_fds;
  y      = meshi->yplt_fds;

  if(meshi->is_extface[0]==1 && znode!=NULL){
    float zij, zijp1;

    i = 0;
    for(j = 0; j<jbar; j++){
      zij   = znode[IJ2(i, j)];
      zijp1 = znode[IJ2(i, j+1)];
      if(zij>zcutoff){
        glVertex3f(x[i], y[j],   global_scase.zbar0);
        glVertex3f(x[i], y[j],   zij);
        glVertex3f(x[i], y[j+1], global_scase.zbar0);
      }

      if(zij<zcutoff&&zijp1<zcutoff)continue;
      if(zij<zcutoff)zij = zijp1;
      if(zijp1<zcutoff)zijp1 = zij;
      glVertex3f(x[i], y[j+1], global_scase.zbar0);
      glVertex3f(x[i], y[j],   zij);
      glVertex3f(x[i], y[j+1], zijp1);
    }
  }
  if(meshi->is_extface[1]==1 && znode!=NULL){
    float zij, zijp1;

    i = ibar;
    for(j = 0; j<jbar; j++){
      zij   = znode[IJ2(i, j)];
      zijp1 = znode[IJ2(i, j+1)];

      if(zijp1>zcutoff){
        glVertex3f(x[i], y[j],   global_scase.zbar0);
        glVertex3f(x[i], y[j+1], global_scase.zbar0);
        glVertex3f(x[i], y[j+1], zijp1);
      }

      if(zij<zcutoff&&zijp1<zcutoff)continue;
      if(zij<zcutoff)zij = zijp1;
      if(zijp1<zcutoff)zijp1 = zij;
      glVertex3f(x[i], y[j],   global_scase.zbar0);
      glVertex3f(x[i], y[j+1], zijp1);
      glVertex3f(x[i], y[j],   zij);
    }
  }
  if(meshi->is_extface[2]==1 && znode!=NULL){
    float zij, zip1j;

    j = 0;
    for(i = 0; i<ibar; i++){
      zij   = znode[IJ2(i, j)];
      zip1j = znode[IJ2(i+1, j)];
      if(zip1j>zcutoff){
        glVertex3f(x[i],   y[j], global_scase.zbar0);
        glVertex3f(x[i+1], y[j], global_scase.zbar0);
        glVertex3f(x[i+1], y[j], zip1j);
      }

      if(zij<zcutoff&&zip1j<zcutoff)continue;
      if(zij<zcutoff)zij = zip1j;
      if(zip1j<zcutoff)zip1j = zij;
      glVertex3f(x[i],   y[j], global_scase.zbar0);
      glVertex3f(x[i+1], y[j], zip1j);
      glVertex3f(x[i],   y[j], zij);
    }
  }
  if(meshi->is_extface[3]==1 && znode!=NULL){
    float zij, zip1j;

    j = jbar;
    for(i = 0; i<ibar; i++){
      zij   = znode[IJ2(i, j)];
      zip1j = znode[IJ2(i+1, j)];
      if(zip1j>zcutoff){
        glVertex3f(x[i],   y[j], global_scase.zbar0);
        glVertex3f(x[i+1], y[j], zip1j);
        glVertex3f(x[i+1], y[j], global_scase.zbar0);
      }

      if(zij<zcutoff&&zip1j<zcutoff)continue;
      if(zij<zcutoff)zij = zip1j;
      if(zip1j<zcutoff)zip1j = zij;
      glVertex3f(x[i],   y[j], global_scase.zbar0);
      glVertex3f(x[i],   y[j], zij);
      glVertex3f(x[i+1], y[j], zip1j);
    }
  }
  if(meshi->is_extface[4]==1){
    for(i = 0; i<ibar; i++){
      glVertex3f(x[0],    y[0],    meshi->zplt_fds[0]);
      glVertex3f(x[ibar], y[jbar], meshi->zplt_fds[0]);
      glVertex3f(x[ibar], y[0],    meshi->zplt_fds[0]);

      glVertex3f(x[0],    y[0],    meshi->zplt_fds[0]);
      glVertex3f(x[0],    y[jbar], meshi->zplt_fds[0]);
      glVertex3f(x[ibar], y[jbar], meshi->zplt_fds[0]);
    }
  }
  glEnd();

  glDisable(GL_COLOR_MATERIAL);
  DISABLE_LIGHTING;

  glPopMatrix();
}

/* ------------------ DrawTerrainOBSTTexture ------------------------ */

void DrawTerrainOBSTTexture(terraindata *terri){
  float *znode;
  unsigned char *uc_znormal;
  int nxcell,nycell;
  int i, j;
  float *x, *y;
  float terrain_color[4];
  float zcut;

  zcut = terri->zmin_cutoff;


  terrain_color[0]=1.0;
  terrain_color[1]=1.0;
  terrain_color[2]=1.0;
  terrain_color[3]=1.0;

  glPushMatrix();
  glScalef(SCALE2SMV(mscale[0]),SCALE2SMV(mscale[1]),vertical_factor*SCALE2SMV(mscale[2]));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);

  ENABLE_LIGHTING;
  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,&global_scase.color_defs.block_shininess);
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,global_scase.terrain_texture_coll.terrain_textures[iterrain_textures].name);

  glEnable(GL_COLOR_MATERIAL);
  glColor4fv(terrain_color);
  glBegin(GL_TRIANGLES);
  uc_znormal = terri->uc_znormal;
  znode = terri->znode;
  nxcell = terri->ibar;
  nycell = terri->jbar;
  x = terri->xplt;
  y = terri->yplt;
  for(j=0;j<terri->jbar;j+=terrain_skip){
    int jp1;
    float ty,typ1;
    unsigned char *uc_zn1, *uc_zn2, *uc_zn3, *uc_zn4;

    jp1 = j + terrain_skip;
    if(jp1>terri->jbar)jp1=terri->jbar;
    ty = (y[j]-ybar0ORIG)/(ybarORIG-ybar0ORIG);
    typ1 = (y[jp1]-ybar0ORIG)/(ybarORIG-ybar0ORIG);

    for(i=0;i<terri->ibar;i+=terrain_skip){
      float *zn1, *zn2, *zn3, *zn4;
      float zval1, zval2, zval3, zval4;
      int ip1;
      float tx,txp1;
      int skip123=0, skip134=0;

      ip1 = i + terrain_skip;
      if(ip1>terri->ibar)ip1=terri->ibar;

      tx = (x[i]-xbar0ORIG)/(xbarORIG-xbar0ORIG);
      txp1 = (x[ip1]-xbar0ORIG)/(xbarORIG-xbar0ORIG);

      uc_zn1 = uc_znormal+ijnode2(i,j);
      zn1 = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn1));
      zval1 = znode[IJ2(i, j)];

      uc_zn2 = uc_znormal+ijnode2(ip1, j);
      zn2 = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn2));
      zval2 = znode[IJ2(ip1, j)];

      uc_zn3 = uc_znormal+ijnode2(ip1, jp1);
      zn3 = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn3));
      zval3 = znode[IJ2(ip1, jp1)];

      uc_zn4 = uc_znormal+ijnode2(i, jp1);
      zn4 = GetNormalVectorPtr(global_scase.wui_sphereinfo, (unsigned int)(*uc_zn4));
      zval4 = znode[IJ2(i, jp1)];

      if(zval1<zcut&&zval2<zcut&&zval3<zcut&&zval4<zcut)continue;

      skip123 = 0;
      skip134 = 0;
      if(zval1<zcut||zval2<zcut||zval3<zcut)skip123=1;
      if(zval1<zcut||zval3<zcut||zval4<zcut)skip134=1;

      if(skip123==0){
        glNormal3fv(zn1);
        glTexCoord2f(tx,ty);
        glVertex3f(x[i],y[j],zval1);


        glNormal3fv(zn2);
        glTexCoord2f(txp1,ty);
        glVertex3f(x[ip1],y[j],zval2);


        glNormal3fv(zn3);
        glTexCoord2f(txp1,typ1);
        glVertex3f(x[ip1],y[jp1],zval3);
      }

      if(skip134==0){
        glNormal3fv(zn1);
        glTexCoord2f(tx,ty);
        glVertex3f(x[i],y[j],zval1);

        glNormal3fv(zn3);
        glTexCoord2f(txp1,typ1);
        glVertex3f(x[ip1],y[jp1],zval3);

        glNormal3fv(zn4);
        glTexCoord2f(tx,typ1);
        glVertex3f(x[i],y[jp1],zval4);
      }

      if(terrain_showonly_top==1){
        if(skip134==0){
          glNormal3fv(zn1);
          glTexCoord2f(tx,ty);
          glVertex3f(x[i],y[j],zval1);

          glNormal3fv(zn4);
          glTexCoord2f(tx,typ1);
          glVertex3f(x[i],y[jp1],zval4);

          glNormal3fv(zn3);
          glTexCoord2f(txp1,typ1);
          glVertex3f(x[ip1],y[jp1],zval3);
        }

        if(skip123==0){
          glNormal3fv(zn1);
          glTexCoord2f(tx,ty);
          glVertex3f(x[i],y[j],zval1);

          glNormal3fv(zn3);
          glTexCoord2f(txp1,typ1);
          glVertex3f(x[ip1],y[jp1],zval3);

          glNormal3fv(zn2);
          glTexCoord2f(txp1,ty);
          glVertex3f(x[ip1],y[j],zval2);
        }
      }
    }
  }
  glEnd();

  glDisable(GL_TEXTURE_2D);

  glDisable(GL_COLOR_MATERIAL);
  DISABLE_LIGHTING;

  glPopMatrix();
}

/* ------------------ GetTerrainSize ------------------------ */

int GetTerrainSize(char *file, float *xmin, float *xmax, int *nx, float *ymin, float *ymax, int *ny, int *times_local){
  FILE *WUIFILE;
  int one;
  float xyminmax[4];
  int nxy[2];
  size_t returncode;
  int version;
  float time_local;
  int nchanges;
  int nt = 0;

  WUIFILE = FOPEN(file, "rb");
  if(WUIFILE == NULL)return 1;

  FSEEK(WUIFILE, 4, SEEK_CUR);fread(&one, 4, 1, WUIFILE);FSEEK(WUIFILE, 4, SEEK_CUR);

  FORTWUIREAD(&version, 1);
  FORTWUIREAD(xyminmax, 4);
  *xmin = xyminmax[0];
  *xmax = xyminmax[1];
  *ymin = xyminmax[2];
  *ymax = xyminmax[3];

  FORTWUIREAD(nxy, 2);
  *nx = nxy[0];
  *ny = nxy[1];

  FSEEK(WUIFILE, 16 + 5 * (*nx)*(*ny), SEEK_CUR); // skip over zelev and state

  for(;;){

    FORTWUIREAD(&time_local, 1);
    if(returncode == 0)break;

    FORTWUIREAD(&nchanges, 1);
    if(returncode == 0)break;

    if(nchanges > 0)FSEEK(WUIFILE, 16 + 5 * nchanges, SEEK_CUR);

    nt++;

  }
  *times_local = nt;

  fclose(WUIFILE);

  return 0;
}

/* ------------------ GetTerrainElev ------------------------ */

float GetTerrainElev(meshdata *meshi, int index){
  for(;;){
    if(meshi==NULL)return global_scase.zbar0-2.0;
    if(meshi->terrain==NULL||meshi->terrain->znode[index]<global_scase.zbar0){
      meshi = meshi->nabors[MDOWN];
    }
    else{
      return meshi->terrain->znode[index];
    }
  }
}

/* ------------------ UpdateTerrain ------------------------ */

void UpdateTerrain(int allocate_memory){
  if(global_scase.auto_terrain==1||global_scase.manual_terrain==1){
    int i;

    if(global_scase.manual_terrain==0){
      global_scase.nterraininfo = global_scase.meshescoll.nmeshes;
      if(allocate_memory==1&&global_scase.manual_terrain==0){
        NewMemory((void **)&global_scase.terraininfo, global_scase.nterraininfo*sizeof(terraindata));
        for(i = 0; i<global_scase.nterraininfo; i++){
          terraindata *terri;

          terri = global_scase.terraininfo+i;
          terri->defined = 0;
        }
      }
    }

    for(i=0;i<global_scase.meshescoll.nmeshes;i++){
      meshdata *meshi;
      terraindata *terri;
      float xmin, xmax, ymin, ymax;
      int nx, ny;

      meshi=global_scase.meshescoll.meshinfo + i;
      if(global_scase.manual_terrain==1){
        terri = meshi->terrain;
      }
      else{
        terri = global_scase.terraininfo + i;
        terri->file = NULL;
      }

      nx = meshi->ibar;
      ny = meshi->jbar;
      xmin = meshi->xplt_fds[0];
      xmax = meshi->xplt_fds[nx];
      ymin = meshi->yplt_fds[0];
      ymax = meshi->yplt_fds[ny];

      InitTerrainZNode(meshi, terri, xmin, xmax, nx, ymin, ymax, ny, allocate_memory);
    }
    if(global_scase.manual_terrain==0){ // slow
      ComputeTerrainNormalsAuto();
    }
    if(global_scase.manual_terrain==1){
      ComputeTerrainNormalsManual();
    }
  }
  if(allocate_memory==1){
    int i;

    for(i = 0; i<global_scase.meshescoll.nmeshes; i++){
      meshdata *meshi;
      int ii;

      meshi = global_scase.meshescoll.meshinfo+i;
      // compute elevations for terrain in each mesh
      // really only need to do this for one mesh in a column but computation isquick and doesn't take a lot of space
      // so doing it for all meshes keeps code simpler
      for(ii = 0; ii<meshi->nznodes; ii++){
        meshi->znodes_complete[ii] = GetTerrainElev(meshi, ii);
      }
    }
    for(i=0; i<global_scase.slicecoll.nsliceinfo; i++){
      slicedata *slicei;
      meshdata *meshi;
      float zmin, zmax;
      float agl;
      int ii;

      slicei = global_scase.slicecoll.sliceinfo + i;
      if(slicei->slice_filetype!=SLICE_TERRAIN)continue;
      meshi = global_scase.meshescoll.meshinfo + slicei->blocknumber;
      zmin = meshi->zplt_fds[0];
      zmax = meshi->zplt_fds[meshi->kbar];
      agl = slicei->above_ground_level;
      for(ii = 0; ii<meshi->nznodes; ii++){
        float zslice, zterrain;

        zterrain = meshi->znodes_complete[ii];
        zslice   = zterrain+agl;
        if(zterrain>=global_scase.zbar0&&zslice>=zmin&&zslice<=zmax){
          slicei->have_agl_data = 1;
          break;
        }
      }
    }
    CheckMemory;
  }
  if(global_scase.nterraininfo>0){
    int imesh;

    for(imesh=0;imesh<global_scase.meshescoll.nmeshes;imesh++){
      meshdata *meshi;
      terraindata *terri;
      float *znode, *znode_scaled;
      int i, j;
      float mesh_zmin, mesh_zmax;
      float t_zmin, t_zmax;

      meshi=global_scase.meshescoll.meshinfo + imesh;
      terri = meshi->terrain;
      if(terri==NULL)continue;
      terri->terrain_mesh = meshi;
      znode = terri->znode;
      znode_scaled = terri->znode_scaled;
      t_zmin=1.0;
      t_zmax=0.0;
      mesh_zmin = meshi->zplt_fds[0];
      mesh_zmax = meshi->zplt_fds[meshi->kbar];

      for(j=0;j<=terri->jbar;j++){
        for(i=0;i<=terri->ibar;i++){
          *znode_scaled = FDS2SMV_Z(*znode);
          if(*znode>=mesh_zmin&&*znode<=mesh_zmax){
            if(t_zmin>t_zmax){
              t_zmin = *znode;
              t_zmax = *znode;
            }
            else{
              t_zmin = MIN(*znode, t_zmin);
              t_zmax = MAX(*znode, t_zmax);
            }
          }
          znode++;
          znode_scaled++;
        }
      }
      if(t_zmin<t_zmax){
        for(j=0; j<4; j++){
          meshi->verts[   3*j+2] = t_zmin;
          meshi->verts[12+3*j+2] = t_zmax;
        }
      }
    }
  }
}

/* ------------------ HaveTerrainSlice ------------------------ */

int HaveTerrainSlice(void){
  int i;

  for(i=0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicedata *slicei;

    slicei = global_scase.slicecoll.sliceinfo + i;

    if(slicei->loaded==1&&slicei->slice_filetype==SLICE_TERRAIN)return 1;

  }
  return 0;
}

/* ------------------ UpdateTerrainOptions ------------------------ */

void UpdateTerrainOptions(void){
  if(global_scase.nterraininfo>0||global_scase.auto_terrain==1){
    visOpenVents=0;
    visDummyVents=0;
    updatemenu=1;
  }
}
