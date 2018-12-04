#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "smv_endian.h"
#include "update.h"
#include "smokeviewvars.h"
#ifdef pp_ISOTIME
#include GLUT_H
#endif

vertdata *vert_list;
edgedata *edge_list;
tridata *triangle_list;
tetdata *volume_list;

/* ------------------ CalcTriNormal ------------------------ */

void CalcTriNormal(float *v1, float *v2, float *v3, float *norm){
  float u[3], v[3];
  int i;

  for(i=0;i<3;i++){
    u[i]=v2[i]-v1[i];
    v[i]=v3[i]-v1[i];
  }
  /*
     i   j  k
     ux uy uz
     vx vy vz
  */
  norm[0]=u[1]*v[2]-u[2]*v[1];
  norm[1]=u[2]*v[0]-u[0]*v[2];
  norm[2]=u[0]*v[1]-u[1]*v[0];
  ReduceToUnit(norm);
}

/* ----------------------- CompareVerts ----------------------------- */

int CompareVerts( const void *arg1, const void *arg2 ){
  vertdata *verti, *vertj;
  float *xyzi, *xyzj;

  verti = (vertdata *)arg1;
  vertj = (vertdata *)arg2;
  xyzi = verti->xyz;
  xyzj = vertj->xyz;

  if(xyzi[0]<xyzj[0])return -1;
  if(xyzi[0]>xyzj[0])return 1;
  if(xyzi[1]<xyzj[1])return -1;
  if(xyzi[1]>xyzj[1])return 1;
  if(xyzi[2]<xyzj[2])return -1;
  if(xyzi[2]>xyzj[2])return 1;
  return 0;
}

/* ------------------ DistXY ------------------------ */

float DistXY(float *x, float *y){
  float r1, r2, r3;

  r1 = x[0]-y[0];
  r2 = x[1]-y[1];
  r3 = x[2]-y[2];
  return sqrt(r1*r1+r2*r2+r3*r3);
}

/* ------------------ GetAngle ------------------------ */

float GetAngle(float d1, float d2, float d3){
  float angle_local;
  float arg;
  float denom;


//         v1
//        /  \
//       d3   d2
//      /      \
//    v2---d1---v3

//       d2^2 + d3^2 - d1^2
// arg = ------------------
//           2*d2*d3

// d2==0.0 ==> d3==d1 ==> arg=0.0
// d3==0.0 ==> d2==d1 ==> arg=0.0


  denom = 2.0*d2*d3;
  if(ABS(denom) > 0.0){
    arg = CLAMP((d2*d2 + d3*d3 - d1*d1) / denom,-1.0,1.0);
    angle_local = acos(arg)*RAD2DEG;
  }
  else{
    angle_local = acos(0.0)*RAD2DEG;
  }
  return angle_local;
}

/* ------------------ GetMinAngle ------------------------ */

float GetMinAngle(tridata *trii){
  float minangle;
  float d1, d2, d3;
  float *xyz1, *xyz2, *xyz3;
  float angle1, angle2, angle3;

  xyz1 = trii->verts[0]->xyz;
  xyz2 = trii->verts[1]->xyz;
  xyz3 = trii->verts[2]->xyz;
  d1 = DistXY(xyz1,xyz2);
  d2 = DistXY(xyz1,xyz3);
  d3 = DistXY(xyz2,xyz3);
  angle1 = GetAngle(d1,d2,d3);
  angle2 = GetAngle(d2,d1,d3);
  angle3 = GetAngle(d3,d1,d2);
  minangle = angle1;
  if(angle2<minangle)minangle=angle2;
  if(angle3<minangle)minangle=angle3;
  return minangle;
}

/* ------------------ GetFaceInfo ------------------------ */

void GetFaceInfo(void){
  int i;

  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;
    geomlistdata *geomlisti;
    vertdata **verts;
    int j;
    int ndups=0,nused=0,nskinny=0;

    geomi = geominfoptrs[i];
    geomlisti = geomi->geomlistinfo;

    if(geomlisti->nverts>0){
      NewMemory((void **)&verts,geomlisti->nverts*sizeof(vertdata *));
      for(j=0;j<geomlisti->nverts;j++){
        verts[j]=geomlisti->verts+j;
        verts[j]->nused=0;
      }
      for(j=0;j<geomlisti->ntriangles;j++){
        tridata *trii;

        trii = geomlisti->triangles + j;
        trii->verts[0]->nused=0;
        trii->verts[1]->nused=0;
        trii->verts[2]->nused=0;
      }
      qsort(verts,geomlisti->nverts,sizeof(vertdata *),CompareVerts);
      for(j=1;j<geomlisti->nverts;j++){
        if(CompareVerts(verts[j-1],verts[j])==0)ndups++;
      }
      for(j=0;j<geomlisti->ntriangles;j++){
        tridata *trii;

        trii = geomlisti->triangles + j;
        trii->verts[0]->nused++;
        trii->verts[1]->nused++;
        trii->verts[2]->nused++;
        if(GetMinAngle(trii)<=10.0){
          trii->skinny=1;
          nskinny++;
        }
        else{
          trii->skinny=0;
        }
      }
      for(j=0;j<geomlisti->nverts;j++){
        if(verts[j]->nused>0)nused++;
      }
#ifdef pp_GEOMPRINT
      PRINTF("Face/Vertex Summary\n");
      PRINTF("      Faces: %i\n",geomlisti->ntriangles);
      PRINTF(" slim faces: %i\n",nskinny);
      PRINTF("   Vertices: %i\n",geomlisti->nverts);
      PRINTF("     unused: %i\n",geomlisti->nverts-nused);
      PRINTF(" duplicates: %i\n\n",ndups);
#endif
      FREEMEMORY(verts);
    }
  }
}

/* ------------------ DrawGeomDiag ------------------------ */

void DrawGeomDiag(void){
  int i;

    glPushMatrix();
    glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
    glTranslatef(-xbar0, -ybar0, -zbar0);
    glBegin(GL_TRIANGLES);
    for(i = 0; i < ngeomdiaginfo; i++){
      geomdiagdata *geomdiagi;
      geomlistdata *geomframe;
      int ntriangles;
      int j;

      geomdiagi = geomdiaginfo + i;
      geomframe = geomdiagi->geom->geomlistinfo_0;
      ntriangles = geomframe->ntriangles;
      for(j = 0; j < ntriangles; j++){
        tridata *trianglej;

        trianglej = geomframe->triangles + j;
        glVertex3fv(trianglej->verts[0]->xyz);
        glVertex3fv(trianglej->verts[1]->xyz);
        glVertex3fv(trianglej->verts[2]->xyz);
      }
    }
    glEnd();
    glPopMatrix();
}

  /* ------------------ GetGeomZBounds ------------------------ */

void GetGeomZBounds(float *zmin, float *zmax){
  int j;
  int first = 1;

  for(j = 0; j < ngeominfoptrs; j++){
    geomdata *geomi;
    int iend, ii;

    geomi = geominfoptrs[j];
    if(geomi->loaded == 0 || geomi->display == 0)continue;
    if(geomi->geomtype != GEOM_GEOM&&geomi->geomtype != GEOM_ISO)continue;

    iend = geomi->ntimes;
    if(geomi->currentframe != NULL)iend = 1;

    for(ii = -1; ii < iend; ii++){
      geomlistdata *geomlisti;
      int k;

      if(ii == -1 || geomi->currentframe == NULL){
        geomlisti = geomi->geomlistinfo + ii;
      }
      else{
        geomlisti = geomi->currentframe;
      }
      for(k = 0; k < geomlisti->nverts; k++){
        float zval;
        vertdata *vertk;

        vertk = geomlisti->verts + k;
        zval = vertk->xyz[2];
        if(first == 1){
          *zmin = zval;
          *zmax = zval;
          first = 0;
        }
        else{
          *zmin = MIN(*zmin, zval);
          *zmax = MAX(*zmax, zval);
        }
      }
    }
  }
}

  /* ------------------ DrawGeom ------------------------ */

void DrawGeom(int flag, int timestate){
  int i;
  float black[]={0.0,0.0,0.0,1.0};
  float blue[]={0.0,0.0,1.0,1.0};
  float green[]={0.0,1.0,0.0,1.0};
  float cyan[] = { 0.0,1.0,1.0,1.0 };
  float magenta[] = { 1.0,0.0,1.0,1.0 };
  float skinny_color[]={1.0,0.0,0.0,1.0};
  float *last_color=NULL;
  float last_transparent_level=-1.0;
  int ntris;
  tridata **tris;

  if(flag == DRAW_OPAQUE){
    ntris=nopaque_triangles;
    tris=opaque_triangles;
  }
  if(flag==DRAW_TRANSPARENT){
    ntris=ntransparent_triangles;
    tris=transparent_triangles;
  }

  if(ntris>0&&timestate==GEOM_STATIC){
    float *color;

  // draw geometry surface

    if(flag==DRAW_TRANSPARENT&&use_transparency_data==1)TransparentOn();

#ifdef pp_TISO
    if(usetexturebar==1&&timestate==GEOM_DYNAMIC){
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glEnable(GL_TEXTURE_1D);
      glBindTexture(GL_TEXTURE_1D, texture_iso_colorbar_id);
    }
#endif

    if(cullfaces == 1)glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    ENABLE_LIGHTING;
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,iso_specular);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,iso_shininess);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,block_ambient2);
    glEnable(GL_COLOR_MATERIAL);

    glPushMatrix();
    glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
    glTranslatef(-xbar0,-ybar0,-zbar0);
    glBegin(GL_TRIANGLES);
    for(i=0;i<ntris;i++){
      tridata *trianglei;
      float transparent_level_local;
      texturedata *ti;
      int  j;
      int smooth_triangles;

      trianglei = tris[i];
      if(trianglei->geomtype!=GEOM_ISO){
        if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
        if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
        if(trianglei->exterior==1&&show_faces_exterior==0)continue;
        if(trianglei->exterior==0&&show_faces_interior==0)continue;
        if(trianglei->geomtype == GEOM_GEOM&&show_faces_shaded == 0)continue;
      }
      else{
        if(show_iso_shaded == 0)continue;
      }

      ti = trianglei->textureinfo;
      if(show_texture_1dimage==1)continue;
      if(visGeomTextures==1&&ti!=NULL&&ti->loaded==1)continue;

      if((trianglei->geomtype==GEOM_GEOM&&smooth_geom_normal==0)||
        (trianglei->geomtype==GEOM_ISO &&smooth_iso_normal==0)){
        smooth_triangles = 0;
      }
      else{
        smooth_triangles=1;
      }
      if(hilight_skinny==1&&trianglei->skinny==1){
        color=skinny_color;
        transparent_level_local=1.0;
      }
      else{
        color = trianglei->geomsurf->color;
        transparent_level_local=trianglei->geomsurf->transparent_level;
      }
      if(geom_force_transparent == 1)transparent_level_local = geom_transparency;
      if(iso_opacity_change==0||trianglei->geomtype!=GEOM_ISO){
        if(color!=last_color||ABS(last_transparent_level-transparent_level_local)>0.001){
          glColor4f(color[0], color[1], color[2], transparent_level_local);
          last_color = color;
          last_transparent_level = transparent_level_local;
        }
      }

      if(smooth_triangles==0){
        glNormal3fv(trianglei->tri_norm);
        for(j=0;j<3;j++){
          vertdata *vertj;

          vertj = trianglei->verts[j];
          glVertex3fv(vertj->xyz);
        }
      }
      else{
        for(j=0;j<3;j++){
          vertdata *vertj;
          float v1[3], v2[3];
          float *vnorm, *vpos;
          float factor;
          float transparent_level_local_new;
#ifdef pp_TISO
          geomlistdata *geomlisti=NULL;
          float *vertvals=NULL;
          float texture_val;
#endif

          vertj = trianglei->verts[j];
#ifdef pp_TISO
          geomlisti = trianglei->geomlisti;
          if(geomlisti!=NULL)vertvals=geomlisti->vertvals;
          if(show_iso_color==1&&vertvals!=NULL){
            int vertj_index;
            float vertval;
            int colorbar_index;

            vertj_index = vertj - trianglei->geomlisti->verts;
            vertval = vertvals[vertj_index];
            texture_val = CLAMP((vertval-iso_valmin)/(iso_valmax-iso_valmin),0.0,1.0);
            colorbar_index = CLAMP((int)texture_val,0,255);
            color = iso_colorbar->colorbar+4*colorbar_index;
          }
#endif
          if(iso_opacity_change==1&&trianglei->geomtype==GEOM_ISO){
          // v1 = xyz - fds_eyepos vector from eye to vertex
          // v2 = vert_norm        normal vector from vertex
          // v1 .dot. v2 is cos(angle) between vectors
          // alphanew = 1.0 - (1.0-alpha)**(1.0/(v1 .dot. v2)
            vnorm = trianglei->vert_norm+3*j;
            vpos = vertj->xyz;
            v1[0] = vpos[0]-fds_eyepos[0];
            v1[1] = vpos[1]-fds_eyepos[1];
            v1[2] = vpos[2]-fds_eyepos[2];
            v2[0] = vnorm[0];
            v2[1] = vnorm[1];
            v2[2] = vnorm[2];
            NORMALIZE3(v1);
            NORMALIZE3(v2);
            factor = ABS(DOT3(v1,v2));
            transparent_level_local_new = CLAMP(transparent_level_local,0.0,1.0);
            if(factor!=0.0&&transparent_level_local<1.0)transparent_level_local_new = 1.0 - pow(1.0-transparent_level_local,1.0/factor);
#ifdef pp_TISO
            if(usetexturebar==1&&show_iso_color==1&&vertvals!=NULL){
              glTexCoord1f(texture_val);
            }
            else{
              glColor4f(color[0], color[1], color[2], transparent_level_local_new);
            }
#else
            glColor4f(color[0], color[1], color[2], transparent_level_local_new);
#endif
          }
          glNormal3fv(trianglei->vert_norm+3*j);
          glVertex3fv(vertj->xyz);
        }
      }
    }
    glEnd();
#ifdef pp_TISO
    if(usetexturebar==1&&timestate==GEOM_DYNAMIC){
      glDisable(GL_TEXTURE_1D);
    }
#endif

    if(visGeomTextures == 1 || show_texture_1dimage == 1){
      texturedata *lasttexture;

      if(show_texture_1dimage == 1){
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glEnable(GL_TEXTURE_1D);
        glBindTexture(GL_TEXTURE_1D, terrain_colorbar_id);
      }
      else{
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glEnable(GL_TEXTURE_2D);
      }

      lasttexture=NULL;
      glBegin(GL_TRIANGLES);
      for(i=0;i<ntris;i++){
        tridata *trianglei;
        texturedata *texti;
        int j;

        trianglei = tris[i];
        if(trianglei->geomtype!=GEOM_ISO){
          if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
          if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
          if(trianglei->exterior==1&&show_faces_exterior==0)continue;
          if(trianglei->exterior==0&&show_faces_interior==0)continue;
        }
        else{
          if(show_iso_outline == 0)continue;
        }

        if(show_texture_1dimage == 1){
          for(j = 0; j < 3; j++){
            vertdata *vertj;
            float *xyz, texture_z;

            vertj = trianglei->verts[j];
            xyz = vertj->xyz;
            //       znew = terrain_zmin + geom_vert_exag*(zold-terrain_zmin);
            //    zmaxnew = terrain_zmin + geom_vert_exag*(terrain_zmax-terrain_zmin)
            //       zold = terrain_zmin + (znew-terrain_zmin)/geom_vert_exag

            texture_z = (xyz[2] - terrain_zmin)/(geom_vert_exag*(terrain_zmax-terrain_zmin));

            glNormal3fv(vertj->vert_norm);
            glTexCoord1f(texture_z);
            glVertex3fv(xyz);
          }
        }
        else{
          texti = trianglei->textureinfo;
          if(texti == NULL || texti->loaded != 1)continue;
          if(lasttexture != texti){
            glEnd();
            glBindTexture(GL_TEXTURE_2D, texti->name);
            glBegin(GL_TRIANGLES);
            lasttexture = texti;
          }
          for(j = 0; j < 3; j++){
            vertdata *vertj;
            float *tvertj;

            vertj = trianglei->verts[j];
            tvertj = trianglei->tverts + 2 * j;
            glNormal3fv(vertj->vert_norm);
            glTexCoord2fv(tvertj);
            glVertex3fv(vertj->xyz);
          }
        }
      }
      glEnd();
      if(show_texture_1dimage == 1){
        glDisable(GL_TEXTURE_1D);
      }
      else{
        glDisable(GL_TEXTURE_2D);
      }
    }

    glDisable(GL_COLOR_MATERIAL);
    DISABLE_LIGHTING;
    glPopMatrix();
    if(flag==DRAW_TRANSPARENT){
      if(use_transparency_data==1)TransparentOff();
      return;
    }
    if(cullfaces==1)glEnable(GL_CULL_FACE);
  }

  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;
    geomlistdata *geomlisti;
    int j;
    float *color;

    geomi = geominfoptrs[i];
    if(geomi->loaded==0||geomi->display==0)continue;
    if(geomi->geomtype!=GEOM_GEOM&&geomi->geomtype!=GEOM_ISO)continue;
    if(timestate==GEOM_STATIC||geomi->ntimes==0){
      geomlisti = geomi->geomlistinfo-1;
    }
    else{
      geomlisti = geomi->geomlistinfo+geomi->itime;
    }

    if(geomlisti->nvolumes > 0 && show_volumes_solid == 1){

      // draw volume solid

      last_color = NULL;
      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-xbar0, -ybar0, -zbar0);

      glEnable(GL_NORMALIZE);
      glShadeModel(GL_SMOOTH);
      ENABLE_LIGHTING;
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, iso_specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, iso_shininess);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, block_ambient2);
      glEnable(GL_COLOR_MATERIAL);

      last_color = NULL;
      glBegin(GL_TRIANGLES);
      for(j = 0; j < geomlisti->nvolumes; j++){
        tetdata *volumei;
        float *xyzptr[4];
        int *exterior;
        //
        //             0
        //            /  \
        //           /   .3
        //             .   \
        //         / .      \
        //         1--------2
        //
        int facelist[12] = {0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2};
        int k;

        volumei = geomlisti->volumes + j;
        exterior = volumei->exterior;
        xyzptr[0] = volumei->verts[0]->xyz;
        xyzptr[1] = volumei->verts[1]->xyz;
        xyzptr[2] = volumei->verts[2]->xyz;
        xyzptr[3] = volumei->verts[3]->xyz;


        color = volumei->matl->color;
        if(last_color != color){
          glColor3fv(color);
          last_color = color;
        }

        for(k = 0; k < 4; k++){
          int kk;
          float *v0, *v1, *v2;
          float v1m0[3], v2m0[3], v2m1[3], vcross[3];
          float v0delta[3], v1delta[3], v2delta[3];

          if(show_volumes_exterior == 0 && exterior[k] == 1)continue;
          if(show_volumes_interior == 0 && exterior[k] == 0)continue;
          v0 = xyzptr[facelist[3 * k]];
          v1 = xyzptr[facelist[3 * k + 1]];
          v2 = xyzptr[facelist[3 * k + 2]];
          VEC3DIFF(v1m0, v1, v0);
          VEC3DIFF(v2m0, v2, v0);
          VEC3DIFF(v2m1, v2, v1);
          CROSS(vcross, v1m0, v2m0);

          for(kk = 0; kk < 3; kk++){
            v0delta[kk] = v0[kk] + face_factor*v1m0[kk] + face_factor*v2m0[kk];
            v1delta[kk] = v1[kk] - face_factor*v1m0[kk] + face_factor*v2m1[kk];
            v2delta[kk] = v2[kk] - face_factor*v2m0[kk] - face_factor*v2m1[kk];
          }
          glNormal3fv(vcross);
          glVertex3fv(v0delta);
          glVertex3fv(v1delta);
          glVertex3fv(v2delta);
        }
      }
      glEnd();
      glDisable(GL_COLOR_MATERIAL);
      DISABLE_LIGHTING;
      glPopMatrix();
    }

      // draw volume outline

    if(geomlisti->nvolumes > 0 && show_volumes_outline == 1){
      last_color = NULL;
      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-xbar0, -ybar0, -zbar0);
      glDisable(GL_COLOR_MATERIAL);
      DISABLE_LIGHTING;
      glLineWidth(20.0);
      glBegin(GL_LINES);
      for(j=0;j<geomlisti->nvolumes;j++){
        tetdata *volumei;
        float *xyzptr[4];
        int *exterior;
        //
        //             0
        //            /  \
        //           /   .3
        //             .   \
        //         / .      \
        //         1--------2
        //
        int facelist[12]={0,1,2, 0,2,3, 0,3,1, 1,3,2};
        int k;

        volumei = geomlisti->volumes+j;
        exterior = volumei->exterior;
        xyzptr[0] = volumei->verts[0]->xyz;
        xyzptr[1] = volumei->verts[1]->xyz;
        xyzptr[2] = volumei->verts[2]->xyz;
        xyzptr[3] = volumei->verts[3]->xyz;

        for(k=0;k<4;k++){
          if(show_volumes_exterior == 0 && exterior[k] == 1)continue;
          if(show_volumes_interior == 0 && exterior[k] == 0)continue;
          if(show_volumes_solid==1){
             color=black;
          }
          else{
            color = volumei->matl->color;
          }
          if(last_color!=color){
            glColor3fv(color);
            last_color=color;
          }
          glVertex3fv(xyzptr[facelist[3*k]]);
          glVertex3fv(xyzptr[facelist[3*k+1]]);
          glVertex3fv(xyzptr[facelist[3*k+1]]);
          glVertex3fv(xyzptr[facelist[3*k+2]]);
          glVertex3fv(xyzptr[facelist[3*k+2]]);
          glVertex3fv(xyzptr[facelist[3*k+0]]);
        }
      }
      glEnd();

      glPopMatrix();
    }

    // draw geometry (faces) outline

    last_color=NULL;
    if(geomlisti->ntriangles>0){
      glPushMatrix();
      glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
      glTranslatef(-xbar0,-ybar0,-zbar0);
      glLineWidth(geom_linewidth);
      glBegin(GL_LINES);
      for(j=0;j<geomlisti->ntriangles;j++){
        float *xyzptr[3];
        float *xyznorm;
        tridata *trianglei;

        trianglei = geomlisti->triangles+j;
        if(trianglei->geomtype!=GEOM_ISO){
          if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
          if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
          if(trianglei->exterior==1&&show_faces_exterior==0)continue;
          if(trianglei->exterior==0&&show_faces_interior==0)continue;
          if(trianglei->geomtype == GEOM_GEOM&&show_faces_outline == 0)continue;
        }
        else{
          if(show_iso_outline == 0)continue;
        }

        xyznorm=trianglei->tri_norm;
        glNormal3fv(xyznorm);

        xyzptr[0] = trianglei->verts[0]->xyz;
        xyzptr[1] = trianglei->verts[1]->xyz;
        xyzptr[2] = trianglei->verts[2]->xyz;

        if(show_iso_shaded==1){
          color = black;
        }
        else{
          color = trianglei->geomsurf->color;
        }
        if(last_color!=color){
          glColor3fv(color);
          last_color=color;
        }
        {
          int ind[6] = {0, 1, 1, 2, 2, 0};
          int k;

          for(k = 0; k < 6; k++){
            float *xyzval, *pknorm, xyzval2[3];
            vertdata *pk;

            pk = trianglei->verts[ind[k]];
            pknorm = pk->vert_norm;
            xyzval = xyzptr[ind[k]];

            VEC3EQ(xyzval2, pknorm);
            VEC3MA(xyzval2, geom_outline_offset);
            VEC3ADD(xyzval2, xyzval2, xyzval);
            glVertex3fv(xyzval2);
          }
        }
      }
      glEnd();
      glPopMatrix();
    }

    // draw geometry verts

    last_color=NULL;
    if(geomlisti->nverts>0){
      glPushMatrix();
      glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
      glTranslatef(-xbar0,-ybar0,-zbar0);
      glPointSize(6.0);
      glBegin(GL_POINTS);
      for(j=0;j<geomlisti->nverts;j++){
        vertdata *verti;

        verti = geomlisti->verts+j;
        if(verti->geomtype == GEOM_GEOM&&show_geom_verts == 0)continue;
        if(verti->geomtype == GEOM_ISO&&show_iso_points == 0)continue;
        if(verti->ntriangles==0)continue;
        color = verti->triangles[0]->geomsurf->color;
        if(last_color!=color){
          glColor3fv(color);
          last_color=color;
        }
        glVertex3fv(verti->xyz);
      }
      glEnd();
      glPopMatrix();
    }

    // draw geometry normal vectors

    if(geomlisti->ntriangles>0){  // draw faceted normals
      glPushMatrix();
      glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
      glTranslatef(-xbar0,-ybar0,-zbar0);
      glLineWidth(geom_linewidth);
      glBegin(GL_LINES);
      glColor3fv(blue);
      for(j=0;j<geomlisti->ntriangles;j++){
        float *v1, *v2, *v3;
        float *xyznorm;
        tridata *trianglei;
        float xyz1[3], xyz2[3];

        trianglei = geomlisti->triangles+j;
        if(trianglei->outside_domain == 0 && showgeom_inside_domain == 0)continue;
        if(trianglei->outside_domain == 1 && showgeom_outside_domain == 0)continue;
        if(trianglei->exterior==1&&show_faces_exterior==0)continue;
        if(trianglei->exterior==0)continue;

        if(trianglei->geomtype==GEOM_GEOM&&(show_geom_normal==0||smooth_geom_normal==1))continue;
        if(trianglei->geomtype == GEOM_ISO &&(show_iso_normal == 0||smooth_iso_normal==1))continue;

        xyznorm=trianglei->tri_norm;

        v1 = trianglei->verts[0]->xyz;
        v2 = trianglei->verts[1]->xyz;
        v3 = trianglei->verts[2]->xyz;

        xyz1[0] = (v1[0] + v2[0] + v3[0])/3.0;
        xyz1[1] = (v1[1] + v2[1] + v3[1])/3.0;
        xyz1[2] = (v1[2] + v2[2] + v3[2])/3.0;
        xyz2[0] = xyz1[0] + SCALE2FDS(geom_vecfactor)*xyznorm[0];
        xyz2[1] = xyz1[1] + SCALE2FDS(geom_vecfactor)*xyznorm[1];
        xyz2[2] = xyz1[2] + SCALE2FDS(geom_vecfactor)*xyznorm[2];

        glVertex3fv(xyz1);
        glVertex3fv(xyz2);
      }
      glEnd();

      glPointSize(6.0);  // draw verts at end of vector
      glBegin(GL_POINTS);
      glColor3fv(black);
      for(j=0;j<geomlisti->ntriangles;j++){
        float *v1, *v2, *v3;
        float *xyznorm;
        tridata *trianglei;
        float xyz1[3], xyz2[3];

        trianglei = geomlisti->triangles+j;
        if(trianglei->outside_domain == 0 && showgeom_inside_domain == 0)continue;
        if(trianglei->outside_domain == 1 && showgeom_outside_domain == 0)continue;
        if(trianglei->exterior==1&&show_faces_exterior==0)continue;
        if(trianglei->exterior==0)continue;
        if(trianglei->geomtype == GEOM_GEOM && (show_geom_normal == 0 || smooth_geom_normal == 1))continue;
        if(trianglei->geomtype == GEOM_ISO&&(show_iso_normal == 0||smooth_iso_normal==1))continue;

        xyznorm=trianglei->tri_norm;

        v1 = trianglei->verts[0]->xyz;
        v2 = trianglei->verts[1]->xyz;
        v3 = trianglei->verts[2]->xyz;

        xyz1[0] = (v1[0] + v2[0] + v3[0])/3.0;
        xyz1[1] = (v1[1] + v2[1] + v3[1])/3.0;
        xyz1[2] = (v1[2] + v2[2] + v3[2])/3.0;
        xyz2[0] = xyz1[0] + SCALE2FDS(geom_vecfactor)*xyznorm[0];
        xyz2[1] = xyz1[1] + SCALE2FDS(geom_vecfactor)*xyznorm[1];
        xyz2[2] = xyz1[2] + SCALE2FDS(geom_vecfactor)*xyznorm[2];

        glVertex3fv(xyz2);
      }
      glEnd();
      glPopMatrix();
    }
    if(geomlisti->ntriangles > 0){  // draw smooth normals
      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-xbar0, -ybar0, -zbar0);
      glLineWidth(geom_linewidth);
      glBegin(GL_LINES);
      glColor3fv(blue);
      for(j = 0; j < geomlisti->ntriangles; j++){
        tridata *trianglei;
        int k;

        trianglei = geomlisti->triangles + j;
        if(trianglei->geomtype!=GEOM_ISO){
          if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
          if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
          if(trianglei->exterior==1&&show_faces_exterior==0)continue;
          if(trianglei->exterior==0)continue;
          if(trianglei->geomtype==GEOM_GEOM&&(show_geom_normal==0||smooth_geom_normal==0))continue;
        }
        else{
          if(show_iso_normal==0||smooth_iso_normal==0)continue;
        }

        for(k = 0; k < 3; k++){
          float *pk;
          float *pknorm;
          float xyz2[3];
          vertdata *vertk;

          vertk = trianglei->verts[k];
          pk = vertk->xyz;
          pknorm = trianglei->vert_norm+3*k;
          xyz2[0] = pk[0] + SCALE2FDS(geom_vecfactor)*pknorm[0];
          xyz2[1] = pk[1] + SCALE2FDS(geom_vecfactor)*pknorm[1];
          xyz2[2] = pk[2] + SCALE2FDS(geom_vecfactor)*pknorm[2];
          glVertex3fv(pk);
          glVertex3fv(xyz2);
        }
      }
      glEnd();
      glPointSize(6.0);  // draw verts at end of vector
      glBegin(GL_POINTS);
      glColor3fv(black);
      for(j = 0; j < geomlisti->ntriangles; j++){
        tridata *trianglei;
        int k;

        trianglei = geomlisti->triangles + j;
        if(trianglei->outside_domain == 0 && showgeom_inside_domain == 0)continue;
        if(trianglei->outside_domain == 1 && showgeom_outside_domain == 0)continue;
        if(trianglei->exterior==1&&show_faces_exterior==0)continue;
        if(trianglei->exterior==0)continue;
        if(trianglei->geomtype == GEOM_GEOM && (show_geom_normal == 0 || smooth_geom_normal == 0))continue;
        if(trianglei->geomtype == GEOM_ISO && (show_iso_normal == 0 || smooth_iso_normal == 0))continue;

        for(k = 0; k < 3; k++){
          float *pk;
          float *pknorm;
          float xyz2[3];
          vertdata *vertk;

          vertk = trianglei->verts[k];
          pk = vertk->xyz;
          pknorm = trianglei->vert_norm+3*k;
          xyz2[0] = pk[0] + SCALE2FDS(geom_vecfactor)*pknorm[0];
          xyz2[1] = pk[1] + SCALE2FDS(geom_vecfactor)*pknorm[1];
          xyz2[2] = pk[2] + SCALE2FDS(geom_vecfactor)*pknorm[2];
          glVertex3fv(xyz2);
        }
      }
      glEnd();
      glPopMatrix();
    }

    // geometry diagnostics

    if(geomlisti->nedges>0 && (highlight_edge0 == 1||highlight_edge1 == 1 || highlight_edge2 == 1 || highlight_edgeother == 1)){
      int ii;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-xbar0, -ybar0, -zbar0);
      glLineWidth(geom_linewidth);
      glBegin(GL_LINES);

      for(ii = 0; ii < geomlisti->nedges; ii++){
        edgedata *edgei;
        float *xyz0, *xyz1;
        vertdata *v0, *v1;

        edgei = geomlisti->edges + ii;
        v0 = geomlisti->verts + edgei->vert_index[0];
        xyz0 = v0->xyz;
        v1 = geomlisti->verts + edgei->vert_index[1];
        xyz1 = v1->xyz;
        if((highlight_edge0==1&&edgei->ntriangles==0)||
           (highlight_edge1==1&&edgei->ntriangles==1)||
           (highlight_edge2==1&&edgei->ntriangles==2)||
           (highlight_edgeother==1&&edgei->ntriangles>2)){
          if(edgei->ntriangles > 2){
            glColor3fv(cyan);
          }
          else if(edgei->ntriangles == 1){
            glColor3fv(green);
          }
          else{
            glColor3fv(blue);
          }
          glVertex3fv(xyz0);
          glVertex3fv(xyz1);
        }
      }

      glEnd();
      glPopMatrix();

    }
    if(geomlisti->nverts>0 && highlight_vertexdup == 1){
      int ii;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-xbar0, -ybar0, -zbar0);
      glPointSize(5.0);
      glBegin(GL_POINTS);

      glColor3fv(magenta);

      for(ii = 0; ii < geomlisti->nverts; ii++){
        vertdata *verti;

        verti = geomlisti->verts + ii;
        if(verti->isdup == 1){
          float *xyz;

          xyz = verti->xyz;
          glVertex3fv(xyz);
        }
      }

      glEnd();
      glPopMatrix();
    }
  }
}

/* ------------------ SmoothGeomNormals ------------------------ */

void SmoothGeomNormals(geomlistdata *geomlisti, int geomtype){
  int i;
  float zmin, *zORIG;

  if(geomlisti->nverts > 0){
    zORIG = geomlisti->zORIG;
    zmin = zORIG[0];
    for(i = 1; i < geomlisti->nverts; i++){
      zmin = MIN(zmin, zORIG[i]);
    }
  }

  for(i = 0; i < geomlisti->ntriangles; i++){
    tridata *trianglei;
    int j;
    float *tri_normi;

    trianglei = geomlisti->triangles + i;
    tri_normi = trianglei->tri_norm;
    for(j = 0; j<3; j++){
      vertdata *vertj;
      int k;
      float *norm;

      vertj = trianglei->verts[j];
      norm = trianglei->vert_norm + 3 * j;
      if(vertj->ntriangles>0){
        norm[0] = 0.0;
        norm[1] = 0.0;
        norm[2] = 0.0;
      }
      else{
        norm[0] = 0.0;
        norm[1] = 0.0;
        norm[2] = 1.0;
      }
      if(geomtype==GEOM_ISO){
        for(k = 0; k<vertj->ntriangles; k++){
          tridata *trianglek;
          float *tri_normk;

          trianglek = vertj->triangles[k];
          tri_normk = trianglek->tri_norm;
          norm[0]+=tri_normk[0];
          norm[1]+=tri_normk[1];
          norm[2]+=tri_normk[2];
        }
      }
      else{
        for(k = 0; k<vertj->ntriangles; k++){
          tridata *trianglek;
          float *tri_normk, cosang;
          float lengthi, lengthk;

          trianglek = vertj->triangles[k];
          if(trianglek->exterior == 0)continue;
          tri_normk = trianglek->tri_norm;
          lengthk = NORM3(tri_normk);
          lengthi = NORM3(tri_normi);
          if(lengthk > 0.0&&lengthi > 0.0){
            cosang = DOT3(tri_normk, tri_normi) / (lengthi*lengthk);
            if(use_max_angle == 0 || cosang > cos_geom_max_angle){ // smooth using all triangles if an isosurface
              norm[0]+=tri_normk[0];
              norm[1]+=tri_normk[1];
              norm[2]+=tri_normk[2];
            }
          }
        }
      }
      ReduceToUnit(norm);
    }
  }
  for(i = 0; i < geomlisti->nverts; i++){
    vertdata *verti;
    float *xyz;

    verti = geomlisti->verts + i;
    xyz = verti->xyz;
    xyz[2] = zmin + geom_vert_exag*(zORIG[i] - zmin);
  }
}

/* ------------------ UpdateGeomNormals ------------------------ */

void UpdateGeomNormals(void){
  int j, ii;

  for(j = 0; j < ngeominfoptrs; j++){
    geomdata *geomi;
    int iend;

    geomi = geominfoptrs[j];
    if(geomi->loaded == 0 || geomi->display == 0)continue;
    if(geomi->geomtype != GEOM_GEOM&&geomi->geomtype != GEOM_ISO)continue;

    iend = geomi->ntimes;
    if(geomi->currentframe != NULL)iend = 1;

    for(ii = -1; ii < iend; ii++){
      geomlistdata *geomlisti;

      if(ii == -1 || geomi->currentframe == NULL){
        geomlisti = geomi->geomlistinfo + ii;
      }
      else{
        geomlisti = geomi->currentframe;
      }
      SmoothGeomNormals(geomlisti,geomi->geomtype);
    }
  }
}

/* ------------------ UpdateTriangles ------------------------ */

void UpdateTriangles(int flag,int update){
  int j, ii, ntimes;

  if(update==GEOM_UPDATE_NORMALS){
    UpdateGeomNormals();
    return;
  }
  for(j=0;j<ngeominfoptrs;j++){
    geomdata *geomi;
    float *xyzptr[3];
    float *xyznorm;
    int i;
    int iend;

    geomi = geominfoptrs[j];
    if(geomi->loaded==0||geomi->display==0)continue;
    if(geomi->geomtype != GEOM_GEOM&&geomi->geomtype!=GEOM_ISO)continue;

    iend = geomi->ntimes;
    if(geomi->currentframe != NULL)iend = 1;

    for(ii=-1;ii<iend;ii++){
      geomlistdata *geomlisti;
      int ntriangles;
      tridata **triangles;

      if(ii==-1||geomi->currentframe==NULL){
        geomlisti = geomi->geomlistinfo + ii;
      }
      else{
        geomlisti = geomi->currentframe;
      }
      for(i=0;i<geomlisti->ntriangles;i++){
        tridata *trianglei;
        vertdata **verts;

        trianglei = geomlisti->triangles+i;
        verts = trianglei->verts;

        xyzptr[0] = verts[0]->xyz;
        xyzptr[1] = verts[1]->xyz;
        xyzptr[2] = verts[2]->xyz;
        xyznorm = trianglei->tri_norm;
        CalcTriNormal(xyzptr[0],xyzptr[1],xyzptr[2],xyznorm);
      }
      for(i=0;i<geomlisti->nverts;i++){
        vertdata *verti;

        verti = geomlisti->verts + i;
        verti->ntriangles=0;
        verti->itriangle=0;
        verti->on_mesh_boundary = 0;
      }
      for(i=0;i<geomlisti->ntriangles;i++){
        tridata *trianglei;

        trianglei = geomlisti->triangles+i;
        trianglei->verts[0]->ntriangles++;
        trianglei->verts[1]->ntriangles++;
        trianglei->verts[2]->ntriangles++;
      }

      // count number of triangles

      ntriangles = 0;
      for(i = 0; i<geomlisti->nverts; i++){
        vertdata *verti;

        verti = geomlisti->verts + i;
        ntriangles += verti->ntriangles;
      }

      // allocate triangle pointers

      FREEMEMORY(geomlisti->triangleptrs);
      if(ntriangles>0){
        NewMemoryMemID((void **)&triangles, ntriangles*sizeof(tridata *), geomi->memory_id);
        geomlisti->triangleptrs = triangles;
      }

      // assign triangle pointers to verts

      for(i = 0; i<geomlisti->nverts; i++){
        vertdata *verti;

        verti = geomlisti->verts + i;
        if(verti->ntriangles>0){
          verti->triangles = triangles;
          triangles += verti->ntriangles;
        }
      }
      for(i=0;i<geomlisti->ntriangles;i++){
        tridata *trianglei;
        vertdata *verti;

        trianglei = geomlisti->triangles+i;
        verti = trianglei->verts[0];
        verti->triangles[verti->itriangle++]=trianglei;

        verti = trianglei->verts[1];
        verti->triangles[verti->itriangle++]=trianglei;

        verti = trianglei->verts[2];
        verti->triangles[verti->itriangle++]=trianglei;
      }
      SmoothGeomNormals(geomlisti,geomi->geomtype);
    }
  }

  // smooth normals at mesh boundaries

  if(ngeominfoptrs>0){
    vertdata **surface_verts = NULL;
    int *match_verts = NULL;

    if(flag == GEOM_STATIC){
      ntimes = 0;
    }
    else{
      ntimes = geominfoptrs[0]->ntimes;
    }
    for(ii = -1; ii<ntimes; ii++){
      int nsurface_verts;

  // identify and count verts on mesh surfaces

      nsurface_verts = 0;
      for(j = 0; j<ngeominfoptrs; j++){
        geomlistdata *geomlisti;
        int  i;
        geomdata *geomj;

        geomj = geominfoptrs[j];
        if(geomj->geomtype != GEOM_GEOM&&geomj->geomtype!=GEOM_ISO)continue;
        geomlisti = geomj->geomlistinfo+ii;
        for(i = 0; i<geomlisti->nverts; i++){
          vertdata *verti;

          verti = geomlisti->verts+i;
          verti->on_mesh_boundary = OnMeshBoundary(verti->xyz);
          if(verti->on_mesh_boundary==1)nsurface_verts++;
        }
      }

  // copy surface verts into an array

      if(nsurface_verts>0){
        int isurf,iii;

        isurf = 0;
        FREEMEMORY(surface_verts);
        FREEMEMORY(match_verts);
        NewMemory((void **)&surface_verts, nsurface_verts*sizeof(vertdata *));
        NewMemory((void **)&match_verts, nsurface_verts*sizeof(int));
        for(j = 0; j<ngeominfoptrs; j++){
          geomlistdata *geomlisti;
          int  i;
          geomdata *geomj;

          geomj = geominfoptrs[j];
          if(geomj->geomtype != GEOM_GEOM&&geomj->geomtype != GEOM_ISO)continue;
          geomlisti = geomj->geomlistinfo + ii;
          for(i = 0; i<geomlisti->nverts; i++){
            vertdata *verti;

            verti = geomlisti->verts+i;
            if(verti->on_mesh_boundary==1){
              if(isurf<nsurface_verts){
                surface_verts[isurf] = verti;
                match_verts[isurf] = -1;
                isurf++;
              }
            }
          }
        }

        // average normals

        for(iii = 0; iii<nsurface_verts; iii++){
          int jjj;
          vertdata *verti;
          float *xyzi, *normi;
          float avgnorm[3];

          if(match_verts[iii]>=0)continue;
          verti = surface_verts[iii];
          xyzi = verti->xyz;
          normi = verti->vert_norm;
          avgnorm[0] = normi[0];
          avgnorm[1] = normi[1];
          avgnorm[2] = normi[2];
          match_verts[iii] = iii;
          for(jjj = iii+1; jjj<nsurface_verts; jjj++){
            vertdata *vertj;
            float *xyzj, *normj;

            if(match_verts[jjj]>=0)continue;
            vertj = surface_verts[jjj];
            xyzj = vertj->xyz;
            normj = vertj->vert_norm;
#define POINTEPS 0.001
            if(ABS(xyzi[0]-xyzj[0])<POINTEPS&&ABS(xyzi[1]-xyzj[1])<POINTEPS&&ABS(xyzi[2]-xyzj[2])<POINTEPS){
              match_verts[jjj] = iii;
              avgnorm[0] += normj[0];
              avgnorm[1] += normj[1];
              avgnorm[2] += normj[2];
            }
          }
          ReduceToUnit(avgnorm);
          for(jjj = iii; jjj<nsurface_verts; jjj++){
            if(match_verts[jjj] == match_verts[iii]){
              vertdata *vertj;
              float *normj;

              vertj = surface_verts[jjj];
              normj = vertj->vert_norm;
              normj[0] = avgnorm[0];
              normj[1] = avgnorm[1];
              normj[2] = avgnorm[2];
            }
          }
        }
      }
    }
    FREEMEMORY(surface_verts);
    FREEMEMORY(match_verts);
  }

  // update cache

  if(0==1){   // don't execute this code yet
    int nverts_max=0, ntriangles_max=0;
    float *vertnormals=NULL, *trinormals = NULL;

    nverts_max = 0;
    ntriangles_max = 0;
    for(j = 0; j<ngeominfoptrs; j++){
      geomdata *geomi;
      int ii;
      FILE *stream = NULL;

      geomi = geominfoptrs[j];
      if(geomi->geomtype!=GEOM_ISO||geomi->cache_defined==1)continue;

      stream = fopen(geomi->topo_file, "wb");
      if(stream==NULL)continue;
      for(ii = 0; ii<geomi->ntimes; ii++){
        geomlistdata *geomlisti;
        int ntriangles, nverts;
        int jj;

        geomlisti = geomi->geomlistinfo+ii;
        fwrite(geomi->times+ii, sizeof(float), 1, stream);

        ntriangles = geomlisti->ntriangles;
        if(ntriangles>ntriangles_max){
          FREEMEMORY(trinormals);
          ntriangles_max = ntriangles+100;
          NewMemory((void **)&trinormals, 3*ntriangles_max*sizeof(float));
        }

        fwrite(&ntriangles, sizeof(int), 1, stream);
        if(ntriangles>0){
          float *trinormals_copy;

          trinormals_copy = trinormals;
          for(jj = 0; jj<ntriangles; jj++){
            tridata *trianglei;
            float *tri_norm;

            trianglei = geomlisti->triangles+jj;
            tri_norm = trianglei->tri_norm;
            *trinormals_copy++ = *tri_norm++;
            *trinormals_copy++ = *tri_norm++;
            *trinormals_copy++ = *tri_norm++;
          }
          fwrite(trinormals, sizeof(float), 3*ntriangles, stream);
        }

        nverts = geomlisti->nverts;
        if(nverts>nverts_max){
          FREEMEMORY(vertnormals);
          nverts_max = nverts+100;
          NewMemory((void **)&vertnormals, 3*nverts_max*sizeof(float));
        }
        fwrite(&nverts, sizeof(int), 1, stream);
        if(nverts>0){
          float *vertnormals_copy;

          vertnormals_copy = vertnormals;
          for(jj = 0; jj<nverts; jj++){
            vertdata *verti;
            float *vert_norm;

            verti = geomlisti->verts+jj;
            vert_norm = verti->vert_norm;
            *vertnormals_copy++ = *vert_norm++;
            *vertnormals_copy++ = *vert_norm++;
            *vertnormals_copy++ = *vert_norm++;
          }
          fwrite(vertnormals, sizeof(float), 3*nverts, stream);
        }
      }
      geomi->cache_defined = 1;
      fclose(stream);
      stream = NULL;
    }
    FREEMEMORY(vertnormals);
    FREEMEMORY(trinormals);
  }
}

#define FORTREAD(var,count,STREAM) FSEEK(STREAM,4,SEEK_CUR);\
                           returncode=fread(var,4,count,STREAM);\
                           if(returncode!=count)returncode=0;\
                           if(endianswitch==1&&returncode!=0)EndianSwitch(var,count);\
                           FSEEK(STREAM,4,SEEK_CUR)

#define FORTREADBR(var,count,STREAM) FORTREAD(var,(count),STREAM);if(returncode==0)break;

/* ------------------ ReadGeomHeader0 ------------------------ */

void ReadGeomHeader0(geomdata *geomi, int *geom_frame_index, int *ntimes_local){
  FILE *stream;
  int one=0,endianswitch=0;
  int nvertfaces[2];
  float times_local[2];
  int nt;
  int returncode;
  int version;
  int nfloat_vals, nint_vals;
  int *int_vals;
  float *float_vals;
  int nverts=0, ntris=0;
  int icount;

  stream = fopen(geomi->file,"rb");
  if(stream==NULL){
    *ntimes_local=-1;
    return;
  }
  FSEEK(stream,4,SEEK_CUR);fread(&one,4,1,stream);FSEEK(stream,4,SEEK_CUR);
  if(one!=1)endianswitch=1;
  FORTREAD(&version,1,stream);

// floating point header

  FORTREAD(&nfloat_vals,1,stream);
  if(nfloat_vals>0){
    NewMemoryMemID((void **)&float_vals,nfloat_vals*sizeof(float),geomi->memory_id);
    FORTREAD(float_vals,nfloat_vals,stream);
    geomi->float_vals=float_vals;
    geomi->nfloat_vals=nfloat_vals;
  }

  // integer header

  FORTREAD(&nint_vals,1,stream);
  if(nint_vals>0){
    NewMemoryMemID((void **)&int_vals,nint_vals*sizeof(float),geomi->memory_id);
    FORTREAD(int_vals,nint_vals,stream);
    geomi->int_vals=int_vals;
    geomi->nint_vals=nint_vals;
  }

// static verts

  FORTREAD(nvertfaces,2,stream);
  nverts=nvertfaces[0];
  ntris=nvertfaces[1];

  // static vertices

  if(nverts>0){
    FSEEK(stream,4+3*nverts*4+4,SEEK_CUR);
  }

  // static triangles

  if(ntris>0){
    FSEEK(stream,4+3*ntris*4+4,SEEK_CUR);
    FSEEK(stream,4+ntris*4+4,SEEK_CUR);
  }

  nt=0;
  icount=-1;
  for(;;){
    FORTREADBR(times_local,2,stream);
    icount++;
    FORTREADBR(nvertfaces,2,stream);
    nverts=nvertfaces[0];
    ntris=nvertfaces[1];

    // dynamic vertices

    if(nverts>0){
      FSEEK(stream,4+3*nverts*4+4,SEEK_CUR);
    }

    // dynamic faces

    if(ntris>0){
      FSEEK(stream,4+3*ntris*4+4,SEEK_CUR);
      FSEEK(stream,4+ntris*4+4,SEEK_CUR);
    }

    if(geom_frame_index==NULL){
      if(use_tload_begin == 1 && times_local[0] < tload_begin)continue;
      if(use_tload_skip == 1 && tload_skip>1 && icount%tload_skip!=0)continue;
      if(use_tload_end == 1 && times_local[0] > tload_end)break;
    }
    nt++;
  }
  *ntimes_local=nt;
  fclose(stream);
}

/* ------------------ ReadGeomHeader2 ------------------------ */

void ReadGeomHeader2(geomdata *geomi, int *ntimes_local){
  FILE *stream;
  int one=0,endianswitch=0;
  int nvertfacesvolumes[3];
  int nt;
  int returncode;
  int version;
  int nverts=0, ntris=0, nvolumes=0;
  int first_frame_static;
  int header[3];
  float time_local;

  stream = fopen(geomi->file,"rb");
  if(stream==NULL){
    *ntimes_local=-1;
    return;
  }
  FSEEK(stream,4,SEEK_CUR);fread(&one,4,1,stream);FSEEK(stream,4,SEEK_CUR);
  if(one!=1)endianswitch=1;
  FORTREAD(&version,1,stream);

  FORTREAD(header,3,stream);
  first_frame_static=header[2];

  nt=0;
  if(first_frame_static==1)nt=-1;
  for(;;){
    FORTREADBR(&time_local,1,stream);
    FORTREADBR(nvertfacesvolumes,3,stream);
    nverts=nvertfacesvolumes[0];
    ntris=nvertfacesvolumes[1];
    nvolumes=nvertfacesvolumes[2];

    // vertices

    if(nverts>0){
      FSEEK(stream,4+3*nverts*4+4,SEEK_CUR); // skip vertices
    }

    // faces

    if(ntris>0){
      FSEEK(stream,4+3*ntris*4+4,SEEK_CUR); // skip triangles
      FSEEK(stream,4+ntris*4+4,SEEK_CUR);   // skip surf
      FSEEK(stream,4+6*ntris*4+4,SEEK_CUR); // skip textures
    }

    // volumes

    if(nvolumes>0){
      FSEEK(stream,4+4*nvolumes*4+4,SEEK_CUR); // skip volumes
      FSEEK(stream,4+nvolumes*4+4,SEEK_CUR);   // skip matl
    }
    nt++;
  }
  *ntimes_local=nt;
  fclose(stream);
}

/* ------------------ ReadGeomHeader ------------------------ */

void ReadGeomHeader(geomdata *geomi, int *geom_frame_index, int *ntimes_local){
  FILE *stream;
  int version;
  int returncode;
  int one=0,endianswitch=0;

  stream = fopen(geomi->file,"rb");
  if(stream==NULL){
    *ntimes_local=-1;
    return;
  }
  FSEEK(stream,4,SEEK_CUR);fread(&one,4,1,stream);FSEEK(stream,4,SEEK_CUR);
  if(one!=1)endianswitch=1;
  FORTREAD(&version,1,stream);
  fclose(stream);

  if(version<=1){
    ReadGeomHeader0(geomi, geom_frame_index, ntimes_local);
  }
  else{
    ReadGeomHeader2(geomi, ntimes_local);
  }
}

/* ------------------ GetGeomDataHeader ------------------------ */

void GetGeomDataHeader(char *file, int *ntimes_local, int *nvals){
  FILE *stream;
  int one=1,endianswitch=0;
  int nface_static,nface_dynamic;
  float time_local;
  int nt,nv;
  int returncode;

  stream = fopen(file,"r");
  if(stream==NULL){
    *ntimes_local=-1;
    return;
  }
  FSEEK(stream,4,SEEK_CUR);fread(&one,4,1,stream);FSEEK(stream,4,SEEK_CUR);
  if(one!=1)endianswitch=1;
  nt=-1;
  nv=0;
  for(;;){
    FORTREADBR(&time_local,1,stream);
    FORTREADBR(&nface_static,1,stream);
    if(nface_static!=0)FSEEK(stream,4+nface_static*4+4,SEEK_CUR);
    FORTREADBR(&nface_dynamic,1,stream);
    if(nface_dynamic!=0)FSEEK(stream,4+nface_dynamic*4+4,SEEK_CUR);
    nt++;
    nv+=(nface_static+nface_dynamic);
  }
  *ntimes_local=nt;
  *nvals=nv;
  fclose(stream);
}

/* ------------------ ReadAllGeom ------------------------ */

void ReadAllGeom(void){
  int i, errorcode;

  for(i=0;i<ngeominfo;i++){
    geomdata *geomi;

    geomi = geominfo + i;
    ReadGeom(geomi,LOAD,GEOM_GEOM,NULL,&errorcode);
  }
  for(i = 0; i < ngeomdiaginfo; i++){
    geomdiagdata *geomdiagi;

    geomdiagi = geomdiaginfo + i;
    ReadGeom(geomdiagi->geom, LOAD, GEOM_GEOM, NULL, &errorcode);
  }
}

/* ------------------ InitGeomlist ------------------------ */

void InitGeomlist(geomlistdata *geomlisti){
  geomlisti->verts = NULL;
#ifdef pp_TISO
  geomlisti->vertvals = NULL;
#endif
  geomlisti->triangles = NULL;
  geomlisti->triangleptrs = NULL;
  geomlisti->volumes = NULL;
  geomlisti->nverts = 0;
  geomlisti->ntriangles = 0;
  geomlisti->nvolumes = 0;
}

/* ------------------ ReadGeom0 ------------------------ */

FILE_SIZE ReadGeom0(geomdata *geomi, int load_flag, int type, int *geom_frame_index, int *errorcode){
  FILE *stream;
  int one=1, endianswitch=0;
  int returncode;
  int ntimes_local;
  int version;
  int nvertfacesvolumes[3];
  int nfloat_vals, nint_vals;
  int iframe, icount;
  FILE_SIZE return_filesize;

  FreeAllMemory(geomi->memory_id);
  geomi->geomlistinfo = NULL;
  geomi->currentframe = NULL;
  geomi->nfloat_vals=0;
  geomi->nint_vals=0;

  if(load_flag==UNLOAD){
    geomi->loaded=0;
    geomi->display=0;
    return 0;
  }

  ReadGeomHeader(geomi,geom_frame_index,&ntimes_local);
  if(ntimes_local<0)return 0;
  stream = fopen(geomi->file,"rb");
  if(stream==NULL)return 0;

  FSEEK(stream,4,SEEK_CUR);fread(&one,4,1,stream);FSEEK(stream,4,SEEK_CUR);
  if(one!=1)endianswitch=1;

  FORTREAD(&version,1,stream);
  return_filesize = 2*(4+4+4);

  FORTREAD(&nfloat_vals,1,stream);
  return_filesize += (4+4+4);

  if(nfloat_vals>0){
    FSEEK(stream, 4+nfloat_vals*4+4, SEEK_CUR);
    return_filesize += 4+nfloat_vals*4+4;
  }

  FORTREAD(&nint_vals,1,stream);
  return_filesize += (4+4+4);

  if(nint_vals>0){
    FSEEK(stream, 4+nint_vals*4+4, SEEK_CUR);
    return_filesize += 4+nint_vals*4+4;
  }

  geomi->ntimes=ntimes_local;
  geomi->itime=0;
  NewMemoryMemID((void **)&geomi->geomlistinfo_0,(ntimes_local+1)*sizeof(geomlistdata),geomi->memory_id);
  geomi->geomlistinfo=geomi->geomlistinfo_0+1;
  NewMemoryMemID((void **)&geomi->times,ntimes_local*sizeof(float),geomi->memory_id);

  icount=-1;
  for(iframe=-1;iframe<ntimes_local;){
    float times_local[2];
    geomlistdata *geomlisti;
    int nverts, ntris;
    int  skipframe;
    vertdata *verts;

    geomlisti = geomi->geomlistinfo+iframe;
    InitGeomlist(geomlisti);
    skipframe = 0;

    if(iframe>=0){

      FORTREADBR(times_local,2,stream);
      return_filesize += 4+4+4;

      icount++;
      if(geom_frame_index == NULL){
        if(use_tload_begin == 1 && times_local[0] < tload_begin)skipframe = 1;
        if(use_tload_skip == 1 && tload_skip>1 && icount%tload_skip != 0)skipframe = 1;
        if(use_tload_end == 1 && times_local[0] > tload_end)skipframe = 1;
        if(skipframe == 0)geomi->times[iframe] = times_local[0];
      }
      else{
        if(iframe!=*geom_frame_index)skipframe = 1;
        geomi->times[iframe] = times_local[0];
        if(skipframe == 0)geomi->currentframe = geomlisti;
      }
    }

    FORTREADBR(nvertfacesvolumes,2,stream);
    return_filesize += (4+8+4);

    nverts=nvertfacesvolumes[0];
    ntris=nvertfacesvolumes[1];
    if(skipframe==1){
      int file_offset = 0;
      if(nverts>0)file_offset += 4+3*nverts*4+4;
      if(ntris>0)file_offset += (4+3*ntris*4+4)+(4+ntris*4+4);
      if(file_offset>0)FSEEK(stream, file_offset, SEEK_CUR);
    }
    if(skipframe==0&&nverts>0){
      int ii;
      float *xyz=NULL;
      float *zORIG;

      NewMemory((void **)&xyz,3*nverts*sizeof(float));
      NewMemoryMemID((void **)&verts,nverts*sizeof(vertdata),geomi->memory_id);
      NewMemory((void **)&zORIG, nverts*sizeof(float));
      geomlisti->zORIG = zORIG;
      geomlisti->verts = verts;
      geomlisti->nverts=nverts;

      FORTREADBR(xyz,3*nverts,stream);
      return_filesize += 4+3*nverts*4+4;

      for(ii=0;ii<nverts;ii++){
        verts[ii].xyz[0]=xyz[3*ii];
        verts[ii].xyz[1]=xyz[3*ii+1];
        verts[ii].xyz[2]=xyz[3*ii+2];
        zORIG[ii] = xyz[3 * ii+2];
      }
      FREEMEMORY(xyz);
    }
    if(skipframe==0&&ntris>0){
      int *surf_ind=NULL,*ijk=NULL;
      int ii;
      int offset=0;
      tridata *triangles;

      NewMemoryMemID((void **)&triangles,ntris*sizeof(tridata),geomi->memory_id);
      NewMemory((void **)&ijk,3*ntris*sizeof(int));
      NewMemory((void **)&surf_ind,ntris*sizeof(int));
      geomlisti->triangles=triangles;
      geomlisti->ntriangles=ntris;

      FORTREADBR(ijk,3*ntris,stream);
      return_filesize += 4+3*ntris*4+4;

      FORTREADBR(surf_ind,ntris,stream);
      return_filesize += 4+ntris*4+4;

      if(type==GEOM_ISO)offset=nsurfinfo;
      for(ii=0;ii<ntris;ii++){
        surfdata *surfi;

        triangles[ii].verts[0]=verts+ijk[3*ii]-1;
        triangles[ii].verts[1]=verts+ijk[3*ii+1]-1;
        triangles[ii].verts[2]=verts+ijk[3*ii+2]-1;

        surfi = surfinfo+CLAMP(surf_ind[ii]+offset, nsurfinfo+1, nsurfinfo+MAX_ISO_COLORS);
        triangles[ii].geomsurf=surfi;
        triangles[ii].textureinfo=NULL;
      }
      FREEMEMORY(ijk);
      FREEMEMORY(surf_ind);
    }

    if(skipframe==0||geom_frame_index!=NULL){
      // add decimation code here
      iframe++;
    }
    if(geom_frame_index==NULL&&use_tload_end == 1 && times_local[0] > tload_end)break;
  }
  geomi->loaded = 1;
  geomi->display=1;
  fclose(stream);
  return return_filesize;
}

/* ------------------ InMesh ------------------------ */

int InMesh(float *xyz){
  int i;

  for(i = 0;i < nmeshes;i++){
    meshdata *meshi;
    float *boxmin, *boxmax;

    meshi = meshinfo + i;
    boxmin = meshi->boxmin;
    boxmax = meshi->boxmax;
    if(xyz[0]<boxmin[0] || xyz[0]>boxmax[0])continue;
    if(xyz[1]<boxmin[1] || xyz[1]>boxmax[1])continue;
    if(xyz[2]<boxmin[2] || xyz[2]>boxmax[2])continue;
    return 1;
  }
  return 0;
}

/* ------------------ OutSideDomain ------------------------ */

int OutSideDomain(vertdata **verts){
  if(InMesh(verts[0]->xyz) == 0 &&
    InMesh(verts[1]->xyz) == 0 &&
    InMesh(verts[2]->xyz) == 0)return 1;
  return 0;
}

/* ------------------ ReadGeom2 ------------------------ */

FILE_SIZE ReadGeom2(geomdata *geomi, int load_flag, int type, int *errorcode){
  FILE *stream;
  int one=1, endianswitch=0;
  int returncode;
  int ntimes_local;
  int i;
  vertdata *verts;
  tridata *triangles;
  tetdata *volumes;
  int version;
  int nvertfacesvolumes[3];
  int nheaders[3], nfloat_vals, nint_vals, first_frame_static;
  FILE_SIZE return_filesize = 0;

  FreeAllMemory(geomi->memory_id);
  geomi->geomlistinfo=NULL;
  geomi->currentframe = NULL;
  geomi->nfloat_vals=0;
  geomi->nint_vals=0;

  if(load_flag==UNLOAD){
    geomi->loaded=0;
    geomi->display=0;
    return 0;
  }

  ReadGeomHeader(geomi,NULL,&ntimes_local);
  if(ntimes_local<0)return 0;
  stream = fopen(geomi->file,"rb");
  if(stream==NULL)return 0;

  FSEEK(stream,4,SEEK_CUR);fread(&one,4,1,stream);FSEEK(stream,4,SEEK_CUR);
  if(one!=1)endianswitch=1;

  FORTREAD(&version,1,stream);
  return_filesize += 2*(4+4+4);

  FORTREAD(nheaders,3,stream);
  return_filesize += 4+3*4+4;

  nfloat_vals=nheaders[0];
  nint_vals=nheaders[1];
  first_frame_static=nheaders[2];

  if(nfloat_vals>0)FSEEK(stream,4+nfloat_vals*4+4,SEEK_CUR);
  if(nint_vals>0)FSEEK(stream,4+nint_vals*4+4,SEEK_CUR);

  geomi->ntimes=ntimes_local;
  geomi->itime=0;
  NewMemoryMemID((void **)&geomi->geomlistinfo_0,(ntimes_local+1)*sizeof(geomlistdata),geomi->memory_id);
  geomi->geomlistinfo=geomi->geomlistinfo_0+1;
  if(ntimes_local>0)NewMemoryMemID((void **)&geomi->times,ntimes_local*sizeof(float),geomi->memory_id);

  for(i=-1;i<ntimes_local;i++){
    float time_local;
    geomlistdata *geomlisti;
    int nverts, ntris, nvolumes;

    geomlisti = geomi->geomlistinfo+i;
    InitGeomlist(geomlisti);

    if(first_frame_static==0&&i==-1)continue;

    FORTREADBR(&time_local,1,stream);
    return_filesize += 4+4+4;

    if(i>=0)geomi->times[i]=time_local;

    FORTREADBR(nvertfacesvolumes,3,stream);
    return_filesize += 4+3*4+4;

    nverts=nvertfacesvolumes[0];
    ntris=nvertfacesvolumes[1];
    nvolumes=nvertfacesvolumes[2];
    if(nvolumes>0)have_volume=1;

    if(nverts>0){
      int ii;
      float *xyz=NULL;
      float *zORIG;

      NewMemory((void **)&xyz,3*nverts*sizeof(float));
      NewMemory((void **)&zORIG,nverts*sizeof(float));
      NewMemoryMemID((void **)&verts,nverts*sizeof(vertdata),geomi->memory_id);
      geomlisti->verts=verts;
      geomlisti->zORIG=zORIG;
      geomlisti->nverts=nverts;

      FORTREADBR(xyz,3*nverts,stream);
      return_filesize += 4+3*nverts*4+4;

      for(ii=0;ii<nverts;ii++){
        verts[ii].xyz[0]=xyz[3*ii];
        verts[ii].xyz[1]=xyz[3*ii+1];
        verts[ii].xyz[2]=xyz[3*ii+2];
        zORIG[ii] = xyz[3*ii+2];
      }
      FREEMEMORY(xyz);
    }
    if(ntris>0){
      int *surf_ind=NULL,*ijk=NULL;
      float *texture_coords=NULL;
      int ii;

      NewMemoryMemID((void **)&triangles,ntris*sizeof(tridata),geomi->memory_id);
      NewMemory((void **)&ijk,3*ntris*sizeof(int));
      NewMemory((void **)&surf_ind,ntris*sizeof(int));
      NewMemory((void **)&texture_coords,6*ntris*sizeof(float));
      geomlisti->triangles=triangles;
      geomlisti->ntriangles=ntris;

      FORTREADBR(ijk,3*ntris,stream);
      return_filesize += 4+3*ntris*4+4;

      FORTREADBR(surf_ind,ntris,stream);
      return_filesize += 4+ntris*4+4;

      FORTREADBR(texture_coords,6*ntris,stream);
      return_filesize += 4+6*ntris*4+4;

      CheckMemory;
      for(ii=0;ii<ntris;ii++){
        surfdata *surfi;
        int k;

        for(k=0;k<3;k++){
          triangles[ii].verts[k]=verts+ijk[3*ii+k]-1;
        }

        for(k=0;k<6;k++){
          triangles[ii].tverts[k]=texture_coords[6*ii+k];
        }

        switch(type){
        case GEOM_GEOM:
        case GEOM_ISO:
          surfi=surfinfo + CLAMP(surf_ind[ii],0,nsurfinfo-1);
          if(type==GEOM_ISO)surfi+=nsurfinfo;
          triangles[ii].insolid = surf_ind[ii];
          break;
        case GEOM_SLICE:
        case GEOM_BOUNDARY:
          surfi=surfinfo;
          triangles[ii].insolid = 0;
          break;
        }
        triangles[ii].geomsurf=surfi;
        triangles[ii].textureinfo=surfi->textureinfo;
        triangles[ii].outside_domain = OutSideDomain(triangles[ii].verts);
      }

      FREEMEMORY(ijk);
      FREEMEMORY(surf_ind);
      FREEMEMORY(texture_coords);
    }
    if(nvolumes>0){
      int ii;
      int *ijk;
      int *matl_ind=NULL;

      NewMemoryMemID((void **)&volumes,nvolumes*sizeof(tetdata),geomi->memory_id);
      geomlisti->volumes=volumes;
      NewMemory((void **)&ijk,4*nvolumes*sizeof(int));

      FORTREADBR(ijk,4*nvolumes,stream);
      return_filesize += 4+4*nvolumes*4+4;

      for(ii=0;ii<nvolumes;ii++){
        int k;

        for(k=0;k<4;k++){
          volumes[ii].verts[k]=verts+ijk[4*ii+k]-1;
        }
      }
      FREEMEMORY(ijk);
      NewMemory((void **)&matl_ind,nvolumes*sizeof(int));

      FORTREADBR(matl_ind,nvolumes,stream);
      return_filesize += 4+nvolumes*4+4;

      for(ii=0;ii<nvolumes;ii++){
        matldata *matli;
        int index;

        index = CLAMP(matl_ind[ii],0,nmatlinfo-1);
        matli=matlinfo + index;
        volumes[ii].matl=matli;
      }
      FREEMEMORY(matl_ind);
      geomlisti->nvolumes=nvolumes;
    }
  }
  geomi->loaded=1;
  geomi->display=1;
  fclose(stream);
  return return_filesize;
}

/* ------------------ ReorderFace ------------------------ */

void ReorderFace(int *faces){
  int face_temp[5];

  if(faces[0]<=MIN(faces[1], faces[2]))return;
  face_temp[0]=faces[0];
  face_temp[1]=faces[1];
  face_temp[2]=faces[2];
  face_temp[3]=faces[0];
  face_temp[4]=faces[1];
  if(faces[1]<=MIN(faces[0],faces[2])){
    VEC3EQ(faces,face_temp+1);
    return;
  }
  VEC3EQ(faces,face_temp+2);
}

/* ------------------ CompareVerts2 ------------------------ */

#define VERT_EPS 0.001

int CompareVerts2(const void *arg1, const void *arg2){
  vertdata *vert1, *vert2;
  float *xyz1, *xyz2;

  vert1 = vert_list + *(int *)arg1;
  xyz1 = vert1->xyz;

  vert2 = vert_list + *(int *)arg2;
  xyz2 = vert2->xyz;

  if(xyz1[0]<xyz2[0] - VERT_EPS)return -1;
  if(xyz1[0]>xyz2[0] + VERT_EPS)return  1;

  if(xyz1[1]<xyz2[1] - VERT_EPS)return -1;
  if(xyz1[1]>xyz2[1] + VERT_EPS)return  1;

  if(xyz1[2]<xyz2[2] - VERT_EPS)return -1;
  if(xyz1[2]>xyz2[2] + VERT_EPS)return  1;
  return 0;
}

/* ------------------ CompareEdges ------------------------ */

int CompareEdges(const void *arg1, const void *arg2){
  edgedata *edge1, *edge2;
  int *v1, *v2;

  edge1 = edge_list + *(int *)arg1;
  edge2 = edge_list + *(int *)arg2;
  v1 = edge1->vert_index;
  v2 = edge2->vert_index;

  if(v1[0]<v2[0])return -1;
  if(v1[0]>v2[0])return 1;

  if(v1[1]<v2[1])return -1;
  if(v1[1]>v2[1])return 1;
  return 0;
}

/* ------------------ CompareEdges2 ------------------------ */

int CompareEdges2(edgedata *edge1, edgedata *edge2){
  int *v1, *v2;

  v1 = edge1->vert_index;
  v2 = edge2->vert_index;

  if(v1[0]<v2[0])return -1;
  if(v1[0]>v2[0])return 1;

  if(v1[1]<v2[1])return -1;
  if(v1[1]>v2[1])return 1;
  return 0;
}

/* ------------------ CompareFaces ------------------------ */

int CompareFaces(const void *arg1, const void *arg2){
  tridata *face1, *face2;
  int *verts1, *verts2;
  int v1[3], v2[3];

  face1 = triangle_list + *(int *)arg1;
  face2 = triangle_list + *(int *)arg2;
  verts1 = face1->vert_index;
  verts2 = face2->vert_index;

  v1[0] = MIN(verts1[0], MIN(verts1[1], verts1[2]));
  v1[2] = MAX(verts1[0], MAX(verts1[1], verts1[2]));
  v1[1] = verts1[0]+verts1[1]+verts1[2]-v1[0]-v1[2];

  v2[0] = MIN(verts2[0], MIN(verts2[1], verts2[2]));
  v2[2] = MAX(verts2[0], MAX(verts2[1], verts2[2]));
  v2[1] = verts2[0]+verts2[1]+verts2[2]-v2[0]-v2[2];

  if(v1[0]<v2[0])return -1;
  if(v1[0]>v2[0])return 1;

  if(v1[1]<v2[1])return -1;
  if(v1[1]>v2[1])return 1;

  if(v1[2]<v2[2])return -1;
  if(v1[2]>v2[2])return 1;
  return 0;
}

/* ------------------ CompareVolumeFaces ------------------------ */

int CompareVolumeFaces(const void *arg1, const void *arg2){
  int face1, face2;
  tetdata *vol1, *vol2;
  int *verts1, *verts2;
  int v1[3], v2[3];

  face1=*(int *)arg1;
  face2=*(int *)arg2;
  vol1 = volume_list + face1/4;
  vol2 = volume_list + face2/4;
  face1 %= 4;
  face2 %= 4;
  verts1 = vol1->faces+3*face1;
  verts2 = vol2->faces+3*face2;

  v1[1]=MIN(verts1[1],verts1[2]);
  v1[2]=MAX(verts1[1],verts1[2]);

  v2[1]=MIN(verts2[1],verts2[2]);
  v2[2]=MAX(verts2[1],verts2[2]);

  if(verts1[0]<verts2[0])return -1;
  if(verts1[0]>verts2[0])return 1;

  if(v1[1]<v2[1])return -1;
  if(v1[1]>v2[1])return 1;

  if(v1[2]<v2[2])return -1;
  if(v1[2]>v2[2])return 1;
  return 0;
}

/* ------------------ GetEdge ------------------------ */

edgedata *GetEdge(edgedata *edges, int nedges, int iv1, int iv2){
  int iresult;
  edgedata ei, *elow, *emid, *ehigh;
  int low, mid, high;
  int ilow, ihigh;

  ei.vert_index[0] = MIN(iv1, iv2);
  ei.vert_index[1] = MAX(iv1, iv2);

  elow = edges;
  ehigh = edges + nedges - 1;

  ilow = CompareEdges2(&ei, elow);
  if(ilow < 0)return NULL;
  if(ilow == 0)return elow;

  ihigh = CompareEdges2(&ei, ehigh);
  if(ihigh > 0)return NULL;
  if(ihigh == 0)return ehigh;

  low = 0;
  high = nedges - 1;
  while(high - low > 1){
    mid = (low + high) / 2;
    emid = edges + mid;
    iresult = CompareEdges2(&ei, emid);
    if(iresult == 0)return emid;
    if(iresult > 0){
      low = mid;
    }
    else{
      high = mid;
    }
  }
  return NULL;
}

/* ------------------ ClassifyGeom ------------------------ */

void ClassifyGeom(geomdata *geomi,int *geom_frame_index){
  int i, iend;

  iend = geomi->ntimes;
  if(geom_frame_index!=NULL)iend=1;

  for(i = -1; i<iend; i++){
    geomlistdata *geomlisti;
    int nverts, nvolumes, ntriangles;
    int j;
    vertdata *vertbase;

    geomlisti = geomi->geomlistinfo+i;
    if(i!=-1&&geom_frame_index!=NULL)geomlisti = geomi->geomlistinfo+(*geom_frame_index);

    nverts=geomlisti->nverts;
    nvolumes=geomlisti->nvolumes;
    ntriangles = geomlisti->ntriangles;
    if(nverts==0||geomlisti->verts==NULL)continue;
    vertbase = geomlisti->verts;
    for(j=0;j<nvolumes;j++){
      tetdata *tetrai;
      int *vert_index;
      vertdata **verts;
      int *faces;

      tetrai = geomlisti->volumes+j;
      vert_index = tetrai->vert_index;
      verts = tetrai->verts;
      faces = tetrai->faces;
      tetrai->exterior[0]=1;
      tetrai->exterior[1]=1;
      tetrai->exterior[2]=1;
      tetrai->exterior[3]=1;
      vert_index[0] = verts[0]-vertbase;
      vert_index[1] = verts[1]-vertbase;
      vert_index[2] = verts[2]-vertbase;
      vert_index[3] = verts[3]-vertbase;

      faces[0]=vert_index[0];
      faces[1]=vert_index[1];
      faces[2]=vert_index[2];
      ReorderFace(faces);

      faces[3]=vert_index[0];
      faces[4]=vert_index[2];
      faces[5]=vert_index[3];
      ReorderFace(faces+3);

      faces[6]=vert_index[0];
      faces[7]=vert_index[3];
      faces[8]=vert_index[1];
      ReorderFace(faces+6);

      faces[9]=vert_index[1];
      faces[10]=vert_index[3];
      faces[11]=vert_index[2];
      ReorderFace(faces+9);
    }
    if(nvolumes>0){
      int *facelist_index=NULL,nfacelist_index;

      nfacelist_index=4*nvolumes;
      volume_list=geomlisti->volumes;
      NewMemory((void **)&facelist_index,4*nfacelist_index*sizeof(int));
      for(j=0;j<nfacelist_index;j++){
        facelist_index[j]=j;
      }
      qsort(facelist_index,nfacelist_index,sizeof(int), CompareVolumeFaces);
      for(j=1;j<nfacelist_index;j++){
        int face1, face2;
        tetdata *vol1, *vol2;
        int *verts1, *verts2;

        face1=facelist_index[j-1];
        face2=facelist_index[j];
        vol1 = volume_list + face1/4;
        vol2 = volume_list + face2/4;
        face1 %= 4;
        face2 %= 4;
        verts1 = vol1->faces+3*face1;
        verts2 = vol2->faces+3*face2;
        if(verts1[0]!=verts2[0])continue;
        if(MIN(verts1[1],verts1[2])!=MIN(verts2[1],verts2[2]))continue;
        if(MAX(verts1[1],verts1[2])!=MAX(verts2[1],verts2[2]))continue;
        vol1->exterior[face1]=0;
        vol2->exterior[face2]=0;
      }

      FREEMEMORY(facelist_index);
    }
    if(ntriangles > 0){
      int *facelist_index = NULL, nfacelist_index;

      nfacelist_index = ntriangles;
      triangle_list = geomlisti->triangles;
      NewMemory((void **)&facelist_index, nfacelist_index*sizeof(int));
      for(j = 0; j < nfacelist_index; j++){
        tridata *trij;
        int *vert_index;

        trij = geomlisti->triangles + j;
        trij->exterior = 1;
        facelist_index[j] = j;
        vert_index = trij->vert_index;
        vert_index[0] = trij->verts[0] - vertbase;
        vert_index[1] = trij->verts[1] - vertbase;
        vert_index[2] = trij->verts[2] - vertbase;
      }
      qsort(facelist_index, nfacelist_index, sizeof(int), CompareFaces);
      for(j = 1; j < nfacelist_index; j++){
        if(CompareFaces(facelist_index + j, facelist_index + j - 1) == 0){
          tridata *trij, *trijm1;

          trij = geomlisti->triangles + facelist_index[j];
          trij->exterior = 0;

          trijm1 = geomlisti->triangles + facelist_index[j - 1];
          trijm1->exterior = 0;
         }
      }

      FREEMEMORY(facelist_index);
    }
    if(ntriangles > 0){
      edgedata *edges, *edges2;
      tridata *triangles;
      int ii;
      int ntris;
      int nedges, ntri0, ntri1, ntri2, ntri_other;
      int *edgelist_index, nedgelist_index = 0;

      ntris = geomlisti->ntriangles;
      triangles = geomlisti->triangles;

      NewMemory((void **)&edges, 3 * ntris * sizeof(edgedata));
      NewMemory((void **)&edges2, 3 * ntris * sizeof(edgedata));

      nedgelist_index = 3 * ntris;
      NewMemory((void **)&edgelist_index, nedgelist_index * sizeof(int));
      for(ii = 0; ii < nedgelist_index; ii++){
        edgelist_index[ii] = ii;
      }

      for(ii = 0; ii<ntris; ii++){
        int i0, i1, i2;

        i0 = triangles[ii].vert_index[0];
        i1 = triangles[ii].vert_index[1];
        i2 = triangles[ii].vert_index[2];

        edges[3 * ii].vert_index[0] = MIN(i0, i1);
        edges[3 * ii].vert_index[1] = MAX(i0, i1);

        edges[3 * ii + 1].vert_index[0] = MIN(i1, i2);
        edges[3 * ii + 1].vert_index[1] = MAX(i1, i2);

        edges[3 * ii + 2].vert_index[0] = MIN(i2, i0);
        edges[3 * ii + 2].vert_index[1] = MAX(i2, i0);
      }


      // remove duplicate edges
      edge_list = edges;
      qsort(edgelist_index, nedgelist_index, sizeof(int), CompareEdges);
      nedges = 0;
      edges2[nedges].vert_index[0] = edges[edgelist_index[nedges]].vert_index[0];
      edges2[nedges].vert_index[1] = edges[edgelist_index[nedges]].vert_index[1];
      nedges++;
      for(ii = 1; ii < nedgelist_index; ii++){
        int jj;

        if(CompareEdges(edgelist_index + ii - 1, edgelist_index + ii)==0)continue;
        jj = edgelist_index[ii];
        edges2[nedges].vert_index[0] = edges[jj].vert_index[0];
        edges2[nedges].vert_index[1] = edges[jj].vert_index[1];
        nedges++;
      }
      if(nedges>0)ResizeMemory((void **)&edges2, nedges * sizeof(edgedata));
      geomlisti->edges = edges2;
      geomlisti->nedges = nedges;
      edge_list = edges2;
      FREEMEMORY(edges);
      edges = edges2;

      for(ii = 0; ii < nedges; ii++){
        edges[ii].ntriangles = 0;
      }

      // count triangles associated with each edge

      for(ii = 0; ii<ntris; ii++){
        edgedata *edgei;
        int *vi;

        vi = triangles[ii].vert_index;
        edgei = GetEdge(edges, nedges, vi[0], vi[1]);
        if(edgei != NULL)edgei->ntriangles++;
        edgei = GetEdge(edges, nedges, vi[1], vi[2]);
        if(edgei != NULL)edgei->ntriangles++;
        edgei = GetEdge(edges, nedges, vi[2], vi[0]);
        if(edgei != NULL)edgei->ntriangles++;
      }

      ntri0 = 0;
      ntri1 = 0;
      ntri2 = 0;
      ntri_other = 0;
      for(ii = 0; ii < nedges; ii++){
        edgedata *edgei;

        edgei = edges + ii;
        switch (edgei->ntriangles){
        case 0:
          ntri0++;
          break;
        case 1:
          ntri1++;
          break;
        case 2:
          ntri2++;
          break;
        default:
          ntri_other++;
          break;
        }
      }
#ifdef XXX
//      printf("\n\nedges\n");
//      printf("                       total: %i\n", nedges);
//      printf("        0 connected triangle: %i\n", ntri0);
//      printf("        1 connected triangle: %i\n", ntri1);
//      printf("        2 connected triangle: %i\n", ntri2);
//      printf("3 or more connected triangle: %i\n", ntri_other);      FREEMEMORY(edgelist_index);
#endif
    }
    if(nverts > 0){
      int *vertlist_index, nvertlist_index = 0;
      vertdata *verts;
      int ii, ndups;

      verts = geomlisti->verts;
      nvertlist_index = nverts;
      NewMemory((void **)&vertlist_index, nvertlist_index * sizeof(int));
      for(ii = 0; ii < nvertlist_index; ii++){
        vertlist_index[ii] = ii;
      }
      vert_list = verts;
      qsort(vertlist_index, nvertlist_index, sizeof(int), CompareVerts2);
      for(ii = 0; ii < nvertlist_index; ii++){
        vertdata *vi;

        vi = verts + ii;
        vi->isdup = 0;
      }
      for(ii = 1; ii < nvertlist_index; ii++){
        if(CompareVerts2(vertlist_index + ii - 1, vertlist_index + ii) == 0){
          vertdata *v1, *v2;
          int jj1, jj2;

          jj1 = vertlist_index[ii];
          jj2 = vertlist_index[ii - 1];

          v1 = verts + jj1;
          v2 = verts + jj2;
          v1->isdup = 1;
          v2->isdup = 1;
        }
      }
      ndups = 0;
      for(ii = 0; ii < nvertlist_index; ii++){
        vertdata *vi;

        vi = verts + ii;
        if(vi->isdup == 1)ndups++;
      }
#ifdef XXX
//      printf("\nvertices\n");
//      printf("\n   total: %i\n", nverts);
//      printf("duplicates: %i\n", ndups);
//      printf("  (eps=%f m)\n", VERT_EPS);
#endif
        FREEMEMORY(vertlist_index);
    }
  }
}

/* ------------------ ReadGeom ------------------------ */

FILE_SIZE ReadGeom(geomdata *geomi, int load_flag, int type, int *geom_frame_index, int *errorcode){
  FILE *stream;
  int version;
  int returncode;
  int one=0,endianswitch=0;
  FILE_SIZE return_filesize=0;
#ifdef pp_ISOTIME
  float time1, time2;
#endif

  if(geomi->file==NULL)return 0;
  stream = fopen(geomi->file,"rb");
  if(stream==NULL)return 0;
  FSEEK(stream,4,SEEK_CUR);fread(&one,4,1,stream);FSEEK(stream,4,SEEK_CUR);
  if(one!=1)endianswitch=1;
  FORTREAD(&version,1,stream);
  fclose(stream);
  return_filesize = 2*(4+4+4);

#ifdef pp_ISOTIME
  START_TIMER(time1);
#endif
  if(version<=1){
    return_filesize+=ReadGeom0(geomi,load_flag,type,geom_frame_index,errorcode);
  }
  else{
    return_filesize += ReadGeom2(geomi,load_flag,type,errorcode);
  }
#ifdef pp_ISOTIME
  STOP_TIMER(time1);
  START_TIMER(time2);
#endif
  if(load_flag==LOAD&&geomi->geomtype!=GEOM_ISO)ClassifyGeom(geomi,geom_frame_index);
#ifdef pp_ISOTIME
  STOP_TIMER(time2);
  printf("\niso load time=%f\n",time1);
  printf("\niso classify time=%f\n",time2);
#endif
  return return_filesize;
}

/* ------------------ DrawGeomData ------------------------ */

void DrawGeomData(int flag, patchdata *patchi, int geom_type){
  int i;
  unsigned char *ivals;

  if(geom_type==GEOM_STATIC){
    ivals = patchi->geom_ival_static;
  }
  else{
    ivals = patchi->geom_ival_dynamic;
  }

  // draw surfaces

  if(
    (patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY&&show_boundary_shaded == 1)||
    (patchi->patch_filetype == PATCH_GEOMETRY_SLICE &&(
     show_slice_shaded[IN_CUTCELL_GLUI]==1||
     show_slice_shaded[IN_SOLID_GLUI]==1||
     show_slice_shaded[IN_GAS_GLUI] == 1))
     ){
    for(i = 0; i < 1; i++){
      geomdata *geomi;
      geomlistdata *geomlisti;
      int ntris;
      int j;
      float *color;

      geomi = patchi->geominfo;
      if(geomi == NULL || geomi->display == 0 || geomi->loaded == 0)continue;
      if(geom_type == GEOM_STATIC){
        geomlisti = geomi->geomlistinfo - 1;
      }
      else{
        geomlisti = geomi->geomlistinfo + geomi->itime;
      }

      ntris = geomlisti->ntriangles;
      if(ntris == 0)continue;

      if(flag == DRAW_TRANSPARENT&&use_transparency_data == 1 && patchi->patch_filetype == PATCH_GEOMETRY_SLICE)TransparentOn();

      glEnable(GL_NORMALIZE);
      glShadeModel(GL_SMOOTH);
      if(patchi->patch_filetype==PATCH_GEOMETRY_BOUNDARY){
        ENABLE_LIGHTING;
      }
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, iso_specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, iso_shininess);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, block_ambient2);
      glEnable(GL_COLOR_MATERIAL);

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-xbar0, -ybar0, -zbar0);
      glBegin(GL_TRIANGLES);
      if(smooth_iso_normal == 0){
        for(j = 0; j < ntris; j++){
          float *xyzptr[3];
          float *xyznorm;
          tridata *trianglei;
          int color_index;

          trianglei = geomlisti->triangles + j;

          xyznorm = trianglei->tri_norm;
          glNormal3fv(xyznorm);

          color_index = ivals[j];
          color = rgb_patch + 4 * color_index;
          if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            int insolid;

            insolid = trianglei->insolid & 3;
            if(insolid == IN_CUTCELL && show_slice_shaded[IN_CUTCELL_GLUI] == 0)continue;
            if(insolid == IN_SOLID   && show_slice_shaded[IN_SOLID_GLUI] == 0)continue;
            if(insolid == IN_GAS     && show_slice_shaded[IN_GAS_GLUI] == 0)continue;
            glColor4f(color[0], color[1], color[2], transparent_level);
          }
          else if(trianglei->geomtype == GEOM_BOUNDARY){
            if(show_boundary_shaded == 0)continue;
            glColor4f(color[0], color[1], color[2], transparent_level);
          }
          else{
            glColor3fv(color);
          }

          xyzptr[0] = trianglei->verts[0]->xyz;
          xyzptr[1] = trianglei->verts[1]->xyz;
          xyzptr[2] = trianglei->verts[2]->xyz;

          glVertex3fv(xyzptr[0]);
          glVertex3fv(xyzptr[1]);
          glVertex3fv(xyzptr[2]);

          if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            glVertex3fv(xyzptr[0]);
            glVertex3fv(xyzptr[2]);
            glVertex3fv(xyzptr[1]);
          }
        }
      }
      else{
        for(j = 0; j < ntris; j++){
          float *xyzptr[3];
          float *xyznorm[3];
          tridata *trianglei;
          int color_index;

          trianglei = geomlisti->triangles + j;

          color_index = ivals[j];
          color = rgb_patch + 4 * color_index;
          if(patchi->structured == NO&&patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            int insolid;

            insolid = trianglei->insolid & 3;
            if(insolid == IN_CUTCELL && show_slice_shaded[IN_CUTCELL_GLUI] == 0)continue;
            if(insolid == IN_SOLID   && show_slice_shaded[IN_SOLID_GLUI] == 0)continue;
            if(insolid == IN_GAS     && show_slice_shaded[IN_GAS_GLUI] == 0)continue;
            glColor4f(color[0], color[1], color[2], transparent_level);
          }
          else if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY){
            if(show_boundary_shaded == 0)continue;
            glColor4f(color[0], color[1], color[2], transparent_level);
          }
          else{
            glColor3fv(color);
          }

          xyzptr[0] = trianglei->verts[0]->xyz;
          xyzptr[1] = trianglei->verts[1]->xyz;
          xyzptr[2] = trianglei->verts[2]->xyz;

          xyznorm[0] = trianglei->verts[0]->vert_norm;
          xyznorm[1] = trianglei->verts[1]->vert_norm;
          xyznorm[2] = trianglei->verts[2]->vert_norm;

          glNormal3fv(xyznorm[0]);
          glVertex3fv(xyzptr[0]);

          glNormal3fv(xyznorm[1]);
          glVertex3fv(xyzptr[1]);

          glNormal3fv(xyznorm[2]);
          glVertex3fv(xyzptr[2]);

          if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            glNormal3fv(xyznorm[0]);
            glVertex3fv(xyzptr[0]);

            glNormal3fv(xyznorm[1]);
            glVertex3fv(xyzptr[2]);

            glNormal3fv(xyznorm[2]);
            glVertex3fv(xyzptr[1]);
          }
        }
      }
      glEnd();
      glPopMatrix();
      glDisable(GL_COLOR_MATERIAL);
      if(patchi->patch_filetype==PATCH_GEOMETRY_BOUNDARY){
        DISABLE_LIGHTING;
      }
      if(flag == DRAW_TRANSPARENT&&use_transparency_data == 1 && patchi->patch_filetype == PATCH_GEOMETRY_SLICE)TransparentOff();
    }
  }

  // draw lines

  if(
    (patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY&&show_boundary_outline == 1)||
    (patchi->patch_filetype == PATCH_GEOMETRY_SLICE &&(
     show_slice_outlines[IN_CUTCELL_GLUI]==1||
     show_slice_outlines[IN_SOLID_GLUI]==1||
     show_slice_outlines[IN_GAS_GLUI] == 1))
       ){
    for(i = 0; i < 1; i++){
      geomdata *geomi;
      geomlistdata *geomlisti;
      int ntris;
      int j;

      geomi = patchi->geominfo;
      if(geomi == NULL || geomi->display == 0 || geomi->loaded == 0)continue;
      if(geom_type == GEOM_STATIC){
        geomlisti = geomi->geomlistinfo - 1;
      }
      else{
        geomlisti = geomi->geomlistinfo + geomi->itime;
      }

      ntris = geomlisti->ntriangles;
      if(ntris == 0)continue;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-xbar0, -ybar0, -zbar0);
      if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY){
        glLineWidth(geomboundary_linewidth);
      }
      else if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
        glLineWidth(geomslice_linewidth);
      }
      else{
        glLineWidth(geom_linewidth);
      }
      glBegin(GL_LINES);
        for(j = 0; j < ntris; j++){
          float *xyzptr[3];
          tridata *trianglei;
          int show_edge1=1, show_edge2=1, show_edge3 = 1;
          int draw_foreground=1;

          trianglei = geomlisti->triangles + j;
          if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            int insolid, insolid_glui=-1;

            insolid = trianglei->insolid & 3;
            if(insolid>=0&&insolid<3)insolid_glui = insolid;
            if(insolid == IN_CUTCELL && show_slice_outlines[IN_CUTCELL_GLUI] == 0)continue;
            if(insolid == IN_SOLID   && show_slice_outlines[IN_SOLID_GLUI] == 0)continue;
            if(insolid == IN_GAS     && show_slice_outlines[IN_GAS_GLUI] == 0)continue;

            if(insolid_glui!=-1&&slice_edgetypes[insolid_glui] == 0){
              int insolid4, insolid8, insolid16;

              insolid4 = trianglei->insolid&4;
              if(insolid4  ==  4)show_edge1 = 0;

              insolid8 = trianglei->insolid&8;
              if(insolid8  ==  8)show_edge2 = 0;

              insolid16 = trianglei->insolid&16;
              if(insolid16 == 16)show_edge3 = 0;
            }
            if(show_slice_shaded[IN_CUTCELL_GLUI]==1||
               show_slice_shaded[IN_SOLID_GLUI]==1||
               show_slice_shaded[IN_GAS_GLUI] == 1){
              draw_foreground=1;
            }
            else{
              draw_foreground=0;
            }
          }
          if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY&&show_boundary_outline == 1){
            if(show_boundary_shaded==1){
              draw_foreground=1;
            }
            else{
              draw_foreground=0;
            }
          }
          if(draw_foreground == 1){
             glColor4fv(foregroundcolor);
          }
          else{
            int color_index;
            float *color;

            color_index = ivals[j];
            color = rgb_patch + 4 * color_index;
            glColor3fv(color);
          }

          xyzptr[0] = trianglei->verts[0]->xyz;
          xyzptr[1] = trianglei->verts[1]->xyz;
          xyzptr[2] = trianglei->verts[2]->xyz;

          if(show_edge1==1){
            glVertex3fv(xyzptr[0]);
            glVertex3fv(xyzptr[1]);
          }

          if(show_edge2==1){
            glVertex3fv(xyzptr[1]);
            glVertex3fv(xyzptr[2]);
          }

          if(show_edge3==1){
            glVertex3fv(xyzptr[2]);
            glVertex3fv(xyzptr[0]);
          }
        }
      glEnd();
      glPopMatrix();
    }
  }

  // draw points

  if(
    (patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY&&show_boundary_points == 1)||
    (patchi->patch_filetype == PATCH_GEOMETRY_SLICE &&(
     show_slice_points[IN_CUTCELL_GLUI]==1||
     show_slice_points[IN_SOLID_GLUI]==1||
     show_slice_points[IN_GAS_GLUI] == 1))
       ){
    for(i = 0; i < 1; i++){
      geomdata *geomi;
      geomlistdata *geomlisti;
      int ntris;
      int j;
      float *color;

      geomi = patchi->geominfo;
      if(geomi == NULL || geomi->display == 0 || geomi->loaded == 0)continue;
      if(geom_type == GEOM_STATIC){
        geomlisti = geomi->geomlistinfo - 1;
      }
      else{
        geomlisti = geomi->geomlistinfo + geomi->itime;
      }

      ntris = geomlisti->ntriangles;
      if(ntris == 0)continue;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-xbar0, -ybar0, -zbar0);
      if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY){
        glPointSize(geomboundary_pointsize);
      }
      else if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
        glPointSize(geomslice_pointsize);
      }
      else{
        glPointSize(geom_pointsize);
      }
      glBegin(GL_POINTS);
      for(j = 0; j < ntris; j++){
        float *xyzptr[3];
        tridata *trianglei;
        int draw_foreground;

        trianglei = geomlisti->triangles + j;

        if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
          int insolid;

          insolid = trianglei->insolid & 3;
          if(insolid == IN_CUTCELL && show_slice_points[IN_CUTCELL_GLUI] == 0)continue;
          if(insolid == IN_SOLID   && show_slice_points[IN_SOLID_GLUI] == 0)continue;
          if(insolid == IN_GAS     && show_slice_points[IN_GAS_GLUI] == 0)continue;
          if(show_slice_shaded[IN_CUTCELL_GLUI]==1||
             show_slice_shaded[IN_SOLID_GLUI]==1||
             show_slice_shaded[IN_GAS_GLUI] == 1){
            draw_foreground=1;
          }
          else{
            draw_foreground=0;
          }
        }
        if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY&&show_boundary_points == 1){
            if(show_boundary_shaded==1){
              draw_foreground=1;
            }
            else{
              draw_foreground=0;
            }
        }
        if(draw_foreground==1){
          glColor4fv(foregroundcolor);
        }
        else{
          int color_index;

          color_index = ivals[j];
          color = rgb_patch + 4 * color_index;
          glColor3fv(color);
        }

        xyzptr[0] = trianglei->verts[0]->xyz;
        xyzptr[1] = trianglei->verts[1]->xyz;
        xyzptr[2] = trianglei->verts[2]->xyz;

        glVertex3fv(xyzptr[0]);
        glVertex3fv(xyzptr[1]);
        glVertex3fv(xyzptr[2]);
      }
      glEnd();
      glPopMatrix();
    }
  }

}

/* ------------------ CompareTransparentTriangles ------------------------ */

int CompareTransparentTriangles(const void *arg1, const void *arg2){
  tridata *tri, *trj;

  tri = *(tridata **)arg1;
  trj = *(tridata **)arg2;

  if(tri->distance<trj->distance)return 1;
  if(tri->distance>trj->distance)return -1;
  return 0;
}

/* ------------------ GetGeomInfoPtrs ------------------------ */

void GetGeomInfoPtrs(geomdata ***geominfoptrs_local,int *ngeominfoptrs_local){
  geomdata **gptr;
  int i, count = 0, hide_geom = 0;

  hide_geom = 0;
  for(i = 0;i < npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY && patchi->loaded == 1 && patchi->display == 1){
      hide_geom = 1;
      break;
    }
  }

  count=0;
  for(i=0;i<ngeominfo;i++){
    geomdata *geomi;

    geomi = geominfo + i;
    if(geomi->loaded==0||geomi->display==0)continue;
    if(geomi->geomtype!=GEOM_GEOM)continue;
    if(hide_geom==1&&geomi->geomtype==GEOM_GEOM)continue;
    count++;
  }
  for(i=0;i<nisoinfo;i++){
    isodata *isoi;
    geomdata *geomi;

    isoi = isoinfo + i;
    if(isoi->loaded==0||isoi->display==0)continue;
    geomi = isoi->geominfo;
    if(geomi==NULL)continue;
    if(geomi->loaded==0||geomi->display==0)continue;
    count++;
  }
  *ngeominfoptrs_local=count;
  if(count>0){
    NewMemory((void **)&gptr,count*sizeof(geomdata *));
  }
  else{
    *geominfoptrs_local=NULL;
    return;
  }
  *geominfoptrs_local=gptr;
  for(i=0;i<ngeominfo;i++){
    geomdata *geomi;

    geomi = geominfo + i;
    if(geomi->loaded==0||geomi->display==0)continue;
    if(geomi->geomtype!=GEOM_GEOM)continue;
    if(hide_geom == 1 && geomi->geomtype == GEOM_GEOM)continue;
    *gptr++=geomi;
  }
  for(i=0;i<nisoinfo;i++){
    isodata *isoi;
    geomdata *geomi;

    isoi = isoinfo + i;
    if(isoi->loaded==0||isoi->display==0)continue;
    geomi = isoi->geominfo;
    if(geomi->loaded==0||geomi->display==0)continue;
    *gptr++=geomi;
  }
}

/* ------------------ ShowHideSortGeometry ------------------------ */

void ShowHideSortGeometry(float *mm){
  int i;
  int count_transparent,count_opaque;
  int itime;
  int *showlevels=NULL;
  int iter;

  if(loaded_isomesh!=NULL)showlevels=loaded_isomesh->showlevels;

  for(iter = 0; iter < 2; iter++){
    CheckMemory;
    count_transparent = 0;
    count_opaque = 0;
    ntransparent_triangles = count_transparent;
    nopaque_triangles = count_opaque;
    for(i = 0; i < ngeominfoptrs; i++){
      geomdata *geomi;

      geomi = geominfoptrs[i];

      // reject unwanted geometry

      if( (geomi->fdsblock == NOT_FDSBLOCK && geomi->geomtype!=GEOM_ISO)|| geomi->patchactive == 1)continue;
      for(itime = 0; itime < 2; itime++){
        geomlistdata *geomlisti;
        int j;

        if(itime == 0){
          geomlisti = geomi->geomlistinfo - 1;
        }
        else{
          if(geomi->ntimes == 0)continue;
          geomlisti = geomi->geomlistinfo + geomi->itime;
          if(geomi->currentframe != NULL)geomlisti = geomi->currentframe;
        }

        for(j = 0; j < geomlisti->ntriangles; j++){
          tridata *tri;
          float xyz[3];
          float *xyz1, *xyz2, *xyz3;
          float xyzeye[3];
          int isurf;
          int is_opaque;

          is_opaque = 0;
          tri = geomlisti->triangles + j;
          if(hilight_skinny == 1 && tri->skinny == 1)is_opaque = 1;
          if(tri->geomsurf->transparent_level >= 1.0)is_opaque = 1;
          if(geom_force_transparent == 1)is_opaque = 0;
          isurf = tri->geomsurf - surfinfo - nsurfinfo - 1;
#ifdef pp_TISO
          tri->geomlisti = geomlisti;
#endif
          if((geomi->geomtype==GEOM_ISO&&showlevels != NULL&&showlevels[isurf] == 0) || tri->geomsurf->transparent_level <= 0.0){
            continue;
          }
          if(iter == 1){
             tri->geomtype = geomi->geomtype;
             tri->verts[0]->geomtype = geomi->geomtype;
             tri->verts[1]->geomtype = geomi->geomtype;
             tri->verts[2]->geomtype = geomi->geomtype;
          }
          if(is_opaque == 1){
            if(iter==1)opaque_triangles[count_opaque] = tri;
            count_opaque++;
            if(iter==0)continue;
          }
          else{
            if(iter==1)transparent_triangles[count_transparent] = tri;
            count_transparent++;
          }
          if(iter==0&&sort_geometry == 1){
            xyz1 = tri->verts[0]->xyz;
            xyz2 = tri->verts[1]->xyz;
            xyz3 = tri->verts[2]->xyz;
            xyz[0] = NORMALIZE_X((xyz1[0] + xyz2[0] + xyz3[0]) / 3.0);
            xyz[1] = NORMALIZE_Y((xyz1[1] + xyz2[1] + xyz3[1]) / 3.0);
            xyz[2] = NORMALIZE_Z((xyz1[2] + xyz2[2] + xyz3[2]) / 3.0);

            xyzeye[0] = mm[0] * xyz[0] + mm[4] * xyz[1] + mm[8] * xyz[2] + mm[12];
            xyzeye[1] = mm[1] * xyz[0] + mm[5] * xyz[1] + mm[9] * xyz[2] + mm[13];
            xyzeye[2] = mm[2] * xyz[0] + mm[6] * xyz[1] + mm[10] * xyz[2] + mm[14];
            xyzeye[0] /= mscale[0];
            xyzeye[1] /= mscale[1];
            xyzeye[2] /= mscale[2];
            tri->distance = xyzeye[0] * xyzeye[0] + xyzeye[1] * xyzeye[1] + xyzeye[2] * xyzeye[2];
            CheckMemory;
          }
        }
      }
    }
    if(iter == 0){
      CheckMemory;
      if(count_transparent == 0 && count_opaque == 0)return;
      FREEMEMORY(alltriangles);
      NewMemory((void **)&alltriangles, (count_opaque + count_transparent)*sizeof(tridata **));
      transparent_triangles = alltriangles;
      opaque_triangles = alltriangles + count_transparent;
    }
  }
  ntransparent_triangles = count_transparent;
  nopaque_triangles = count_opaque;
  if(sort_geometry==1&&ntransparent_triangles>0){
    qsort((isotri **)transparent_triangles, (size_t)ntransparent_triangles, sizeof(tridata **), CompareTransparentTriangles);
  }
}

/* ------------------ InitGeom ------------------------ */

void InitGeom(geomdata *geomi,int geomtype, int fdsblock){
  geomi->file=NULL;
  geomi->topo_file = NULL;
  geomi->cache_defined = 0;
  geomi->display=0;
  geomi->loaded=0;
  geomi->geomlistinfo_0=NULL;
  geomi->surf=NULL;
  geomi->geomlistinfo=NULL;
  geomi->currentframe = NULL;
  geomi->times=NULL;
  geomi->ntimes=0;
  geomi->times=NULL;
  geomi->timeslist=NULL;
  geomi->float_vals=NULL;
  geomi->int_vals=NULL;
  geomi->nfloat_vals=0;
  geomi->nint_vals=0;
  geomi->geomtype = geomtype;
  geomi->fdsblock = fdsblock;
}
/* ------------------ RotateU2V ------------------------ */

void RotateU2V(float *u, float *v, float *axis, float *angle){
  float sum, cosangle, normu, normv;

  /*
  i  j  k
  ux uy uz
  vx vy vz
  */

  CROSS(axis, u, v);
  sum = NORM3(axis);
  normu = NORM3(u);
  normv = NORM3(v);
  if(sum>0.0&&normu>0.0&&normv>0.0){
    axis[0] /= sum;
    axis[1] /= sum;
    axis[2] /= sum;
    cosangle = CLAMP(DOT3(u, v) / (normu*normv), -1.0, 1.0);
    *angle = acos(cosangle);
  }
  else{
    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;
    *angle = 0.0;
  }
}

/* ------------------ AngleAxis2Quat ------------------------ */

void AngleAxis2Quat(float angle, float *axis, float *quat){
  float sum;
  float cosang, sinang;

  // angle is in radians
  // axis is a vector

  sum = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);

  if(sum>0.0){
    cosang = cos(angle / 2.0);
    sinang = sin(angle / 2.0);

    quat[0] = cosang;
    quat[1] = axis[0] * sinang / sum;
    quat[2] = axis[1] * sinang / sum;
    quat[3] = axis[2] * sinang / sum;
  }
  else{
    quat[0] = 1.0;
    quat[1] = 0.0;
    quat[2] = 0.0;
    quat[3] = 0.0;
  }
}

/* ------------------ Quat2Rot------------------ */

void Quat2Rot(float quat[4], float rot[16]){
  float w=0.0, x=0.0, y=0.0, z=0.0, sum;

  sum = sqrt(quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2] + quat[3] * quat[3]);
  if(sum!=0.0){
    w = quat[0]/sum;
    x = quat[1]/sum;
    y = quat[2]/sum;
    z = quat[3]/sum;
  }

  rot[0] = 1.0 - 2.0*y*y - 2.0*z*z;
  rot[1] = 2.0*x*y + 2.0*w*z;
  rot[2] = 2.0*x*z - 2.0*w*y;
  rot[3] = 0.0;

  rot[4] = 2.0*x*y - 2.0*w*z;
  rot[5] = 1.0 - 2.0*x*x - 2.0*z*z;
  rot[6] = 2.0*y*z + 2.0*w*x;
  rot[7] = 0.0;

  rot[8] = 2.0*x*z + 2.0*w*y;
  rot[9] = 2.0*y*z - 2.0*w*x;
  rot[10] = 1.0 - 2.0*x*x - 2.0*y*y;
  rot[11] = 0.0;

  rot[12] = 0.0;
  rot[13] = 0.0;
  rot[14] = 0.0;
  rot[15] = 1.0;
}

/* ------------------ MultQuat ------------------------ */

void MultQuat(float x[4], float y[4], float z[4]){
  float z2[4];

  z2[0] = x[0] * y[0] - x[1] * y[1] - x[2] * y[2] - x[3] * y[3];
  z2[1] = x[0] * y[1] + x[1] * y[0] + x[2] * y[3] - x[3] * y[2];
  z2[2] = x[0] * y[2] - x[1] * y[3] + x[2] * y[0] + x[3] * y[1];
  z2[3] = x[0] * y[3] + x[1] * y[2] - x[2] * y[1] + x[3] * y[0];
  z[0] = z2[0];
  z[1] = z2[1];
  z[2] = z2[2];
  z[3] = z2[3];
}

/* ------------------ XYZ2AzElev ------------------------ */

void XYZ2AzElev(float *xyz, float *azimuth, float *elevation){
  float norm3;

  // x = ||xyz||cos(az)*cos(elev)
  // y = ||xyz||sin(az)*cos(elev)
  // z = ||xyz||sin(elev)
  // elev=asin(z/||xyz||)
  // az=atan(y/x)
  norm3 = NORM3(xyz);
  if(norm3>0.00001&&ABS(xyz[2] / norm3) <= 1.0){
    *elevation = RAD2DEG*asin(xyz[2] / norm3);
  }
  else{
    *elevation = 0.0;
  }
  *azimuth = RAD2DEG*atan2(xyz[1], xyz[0]);
}


