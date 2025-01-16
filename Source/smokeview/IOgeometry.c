#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "IOscript.h"
#include "readgeom.h"

#define MAX_FRAMES 1000000
#define BUILD_GEOM_OFFSETS 0
#define GET_GEOM_OFFSETS  -1

/* ------------------ CompareFaces ------------------------ */

int CompareFaces(const void *arg1, const void *arg2){
  tridata *face1, *face2;
  int *verts1, *verts2;
  int v1[3], v2[3];

  face1 = *( tridata ** )arg1;
  face2 = *( tridata ** )arg2;
  verts1 = face1->vert_index;
  verts2 = face2->vert_index;

  v1[0] = MIN(verts1[0], MIN(verts1[1], verts1[2]));
  v1[2] = MAX(verts1[0], MAX(verts1[1], verts1[2]));
  v1[1] = verts1[0] + verts1[1] + verts1[2] - v1[0] - v1[2];

  v2[0] = MIN(verts2[0], MIN(verts2[1], verts2[2]));
  v2[2] = MAX(verts2[0], MAX(verts2[1], verts2[2]));
  v2[1] = verts2[0] + verts2[1] + verts2[2] - v2[0] - v2[2];

  if(v1[0] < v2[0])return -1;
  if(v1[0] > v2[0])return 1;

  if(v1[1] < v2[1])return -1;
  if(v1[1] > v2[1])return 1;

  if(v1[2] < v2[2])return -1;
  if(v1[2] > v2[2])return 1;
  return 0;
}

/* ------------------ CompareEdges ------------------------ */

int CompareEdges(const void *arg1, const void *arg2){
  edgedata *edge1, *edge2;
  int *v1, *v2;

  edge1 = *( edgedata ** )arg1;
  edge2 = *( edgedata ** )arg2;
  v1 = edge1->vert_index;
  v2 = edge2->vert_index;

  if(v1[0] < v2[0])return -1;
  if(v1[0] > v2[0])return 1;

  if(v1[1] < v2[1])return -1;
  if(v1[1] > v2[1])return 1;
  return 0;
}

/* ------------------ CompareVerts2 ------------------------ */

#define VERT_EPS 0.001

int CompareVerts2(const void *arg1, const void *arg2){
  vertdata *vert1, *vert2;
  float *xyz1, *xyz2;


  vert1 = *( vertdata ** )arg1;
  xyz1 = vert1->xyz;

  vert2 = *( vertdata ** )arg2;
  xyz2 = vert2->xyz;

  if(xyz1[0] < xyz2[0] - VERT_EPS)return -1;
  if(xyz1[0] > xyz2[0] + VERT_EPS)return  1;

  if(xyz1[1] < xyz2[1] - VERT_EPS)return -1;
  if(xyz1[1] > xyz2[1] + VERT_EPS)return  1;

  if(xyz1[2] < xyz2[2] - VERT_EPS)return -1;
  if(xyz1[2] > xyz2[2] + VERT_EPS)return  1;
  return 0;
}

/* ------------------ CompareEdges2 ------------------------ */

int CompareEdges2(edgedata *edge1, edgedata *edge2){
  int *v1, *v2;

  v1 = edge1->vert_index;
  v2 = edge2->vert_index;

  if(v1[0] < v2[0])return -1;
  if(v1[0] > v2[0])return 1;

  if(v1[1] < v2[1])return -1;
  if(v1[1] > v2[1])return 1;
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

void ClassifyGeom(geomdata *geomi, int *geom_frame_index){
  int i, iend;

  if(geomi->geomlistinfo == NULL)return;
  iend = geomi->ntimes;
  if(geom_frame_index != NULL)iend = 1;

  for(i = -1; i < iend; i++){
    geomlistdata *geomlisti;
    int nverts, ntriangles;
    int j;
    vertdata *vertbase;


    geomlisti = geomi->geomlistinfo + i;
    if(i != -1 && geom_frame_index != NULL)geomlisti = geomi->geomlistinfo + (*geom_frame_index);

    nverts = geomlisti->nverts;
    ntriangles = geomlisti->ntriangles;
    if(nverts == 0 || geomlisti->verts == NULL)continue;
    vertbase = geomlisti->verts;
    if(ntriangles > 0){
      int nfacelist_index;
      tridata **facelist_ptrs = NULL;

      nfacelist_index = ntriangles;
      NewMemory(( void ** )&facelist_ptrs, nfacelist_index * sizeof(tridata *));
      for(j = 0; j < nfacelist_index; j++){
        tridata *trij;
        int *vert_index;

        trij = geomlisti->triangles + j;
        trij->exterior = 1;
        facelist_ptrs[j] = trij;
        vert_index = trij->vert_index;
        vert_index[0] = trij->verts[0] - vertbase;
        vert_index[1] = trij->verts[1] - vertbase;
        vert_index[2] = trij->verts[2] - vertbase;
      }
      qsort(facelist_ptrs, nfacelist_index, sizeof(tridata *), CompareFaces);
      for(j = 1; j < nfacelist_index; j++){
        if(CompareFaces(facelist_ptrs + j, facelist_ptrs + j - 1) == 0){
          tridata *trij, *trijm1;

          trij = facelist_ptrs[j];
          trij->exterior = 0;

          trijm1 = facelist_ptrs[j - 1];
          trijm1->exterior = 0;
        }
      }
      FREEMEMORY(facelist_ptrs);
    }
    if(ntriangles > 0){
      edgedata **edgelist_ptr, *edges, *edges2;
      tridata *triangles;
      int ii;
      int ntris;
      int nedges;
      int nedgelist_index = 0;

      ntris = geomlisti->ntriangles;
      triangles = geomlisti->triangles;

      NewMemory(( void ** )&edges, 3 * ntris * sizeof(edgedata));
      NewMemory(( void ** )&edges2, 3 * ntris * sizeof(edgedata));

      nedgelist_index = 3 * ntris;
      NewMemory(( void ** )&edgelist_ptr, nedgelist_index * sizeof(edgedata *));

      for(ii = 0; ii < ntris; ii++){
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

        edgelist_ptr[3 * ii] = edges + 3 * ii;
        edgelist_ptr[3 * ii + 1] = edges + 3 * ii + 1;
        edgelist_ptr[3 * ii + 2] = edges + 3 * ii + 2;
      }


      // remove duplicate edges
      qsort(edgelist_ptr, nedgelist_index, sizeof(edgedata *), CompareEdges);
      nedges = 0;
      edges2[nedges].vert_index[0] = edgelist_ptr[nedges]->vert_index[0];
      edges2[nedges].vert_index[1] = edgelist_ptr[nedges]->vert_index[1];
      nedges++;
      for(ii = 1; ii < nedgelist_index; ii++){
        if(CompareEdges(edgelist_ptr + ii - 1, edgelist_ptr + ii) == 0)continue;
        memcpy(edges2[nedges].vert_index, edgelist_ptr[ii]->vert_index, 2 * sizeof(int));
        nedges++;
      }
      if(nedges > 0)ResizeMemory(( void ** )&edges2, nedges * sizeof(edgedata));
      geomlisti->edges = edges2;
      geomlisti->nedges = nedges;
      FREEMEMORY(edges);
      edges = edges2;

      for(ii = 0; ii < nedges; ii++){
        edges[ii].ntriangles = 0;
      }

      // count triangles associated with each edge

      for(ii = 0; ii < ntris; ii++){
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

    }
    if(nverts > 0){
      int nvertlist_index = 0;
      vertdata **vertlist_ptr, *verts;
      int ii;

      verts = geomlisti->verts;
      nvertlist_index = nverts;
      NewMemory(( void ** )&vertlist_ptr, nvertlist_index * sizeof(vertdata *));
      for(ii = 0; ii < nvertlist_index; ii++){
        vertlist_ptr[ii] = verts + ii;
      }

      qsort(vertlist_ptr, nvertlist_index, sizeof(vertdata *), CompareVerts2);
      for(ii = 0; ii < nvertlist_index; ii++){
        vertdata *vi;

        vi = verts + ii;
        vi->isdup = 0;
      }
      for(ii = 1; ii < nvertlist_index; ii++){
        if(CompareVerts2(vertlist_ptr + ii - 1, vertlist_ptr + ii) == 0){
          vertdata *v1, *v2;

          v1 = vertlist_ptr[ii];
          v2 = vertlist_ptr[ii - 1];
          v1->isdup = 1;
          v2->isdup = 1;
        }
      }
      FREEMEMORY(vertlist_ptr);
    }
  }
}

/* ------------------ ClassifyAllGeom ------------------------ */

void *ClassifyAllGeom(void *arg){
  int i;

  for(i = 0; i < global_scase.ngeominfo; i++){
    geomdata *geomi;

    geomi = global_scase.geominfo + i;
    THREADcontrol(readallgeom_threads, THREAD_LOCK);
    if(geomi->read_status != 0){
      THREADcontrol(readallgeom_threads, THREAD_UNLOCK);
      continue;
    }
    geomi->read_status = 1;
    THREADcontrol(readallgeom_threads, THREAD_UNLOCK);

    if(geomi->geomtype != GEOM_ISO){
      ClassifyGeom(geomi, NULL);
    }
    THREADcontrol(readallgeom_threads, THREAD_LOCK);
    geomi->read_status = 2;
    THREADcontrol(readallgeom_threads, THREAD_UNLOCK);
  }
  for(i = 0; i < global_scase.ncgeominfo; i++){
    geomdata *geomi;

    geomi = global_scase.cgeominfo + i;
    THREADcontrol(readallgeom_threads, THREAD_LOCK);
    if(geomi->read_status != 0){
      THREADcontrol(readallgeom_threads, THREAD_UNLOCK);
      continue;
    }
    geomi->read_status = 1;
    THREADcontrol(readallgeom_threads, THREAD_UNLOCK);

    if(geomi->geomtype != GEOM_ISO){
      ClassifyGeom(geomi, NULL);
    }
    THREADcontrol(readallgeom_threads, THREAD_LOCK);
    geomi->read_status = 2;
    THREADcontrol(readallgeom_threads, THREAD_UNLOCK);
  }
  THREAD_EXIT(readallgeom_threads);
}


/* ------------------ GetFaceInfo ------------------------ */

void GetFaceInfo(void){
  int i;

  for(i=0;i<ngeominfoptrs;i++){
    geomdata *geomi;
    geomlistdata *geomlisti;
    vertdata **verts;
    int j;
#ifndef pp_ISOFRAME
    int ndups=0,nused=0,nskinny=0;
#endif

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
#ifndef pp_ISOFRAME
      for(j=1;j<geomlisti->nverts;j++){
        if(CompareVerts(verts[j-1],verts[j])==0)ndups++;
      }
#endif
      for(j=0;j<geomlisti->ntriangles;j++){
        tridata *trii;

        trii = geomlisti->triangles + j;
        trii->verts[0]->nused++;
        trii->verts[1]->nused++;
        trii->verts[2]->nused++;
        if(GetMinAngle(trii)<=10.0){
          trii->skinny=1;
#ifndef pp_ISOFRAME
          nskinny++;
#endif
        }
        else{
          trii->skinny=0;
        }
      }
#ifndef pp_ISOFRAME
      for(j=0;j<geomlisti->nverts;j++){
        if(verts[j]->nused>0)nused++;
      }
      if(print_geominfo==1){
        PRINTF("Face/Vertex Summary\n");
        PRINTF("      Faces: %i\n", geomlisti->ntriangles);
        PRINTF(" slim faces: %i\n", nskinny);
        PRINTF("   Vertices: %i\n", geomlisti->nverts);
        PRINTF("     unused: %i\n", geomlisti->nverts-nused);
        PRINTF(" duplicates: %i\n\n", ndups);
      }
#endif
      FREEMEMORY(verts);
    }
  }
}

/* ------------------ TextureOff ------------------------ */

int TextureOff(void){
  glDisable(GL_TEXTURE_1D);
  return 0;
}

/* ------------------ TextureOn ------------------------ */

int TextureOn(GLuint texture_id,int *texture_first){
  if(*texture_first==1){
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    *texture_first=0;
  }
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, texture_id);
  return 1;
}

/* ------------------ UpdateGeomAreas ------------------------ */

void UpdateGeomAreas(void){
  int ntris;
  geomdata *geomi;
  geomlistdata *geomlisti;

  if(ngeominfoptrs==0)return;
  geomi = geominfoptrs[0];
  geomlisti = geomi->geomlistinfo-1;
  ntris = geomlisti->ntriangles;
  if(ntris>0){
    int i;

    // initialize surf values

    for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo+i;
      surfi->geom_area = 0.0;
      surfi->axis[0] = 0.0;
      surfi->axis[1] = 0.0;
      surfi->axis[2] = 0.0;
      surfi->ntris = 0;
    }

    // compute surf area and median

    for(i = 0; i<ntris; i++){
      tridata *trianglei;
      surfdata *tri_surf;

      trianglei = geomlisti->triangles + i;
      if(trianglei->geomtype!=GEOM_ISO){
        if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
        if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
        if(trianglei->exterior==0)continue;
        if(trianglei->geomtype==GEOM_GEOM&&show_faces_shaded==0)continue;
      }
      tri_surf = trianglei->geomsurf;
      if(tri_surf!=NULL){
        float *xyz0, *xyz1, *xyz2;

        tri_surf->geom_area += trianglei->area;
        tri_surf->ntris++;
        xyz0 = trianglei->verts[0]->xyz;
        xyz1 = trianglei->verts[1]->xyz;
        xyz2 = trianglei->verts[2]->xyz;
        tri_surf->axis[0] += (xyz0[0]+xyz1[0]+xyz2[0])/3.0;
        tri_surf->axis[1] += (xyz0[1]+xyz1[1]+xyz2[1])/3.0;
        tri_surf->axis[2] += (xyz0[2]+xyz1[2]+xyz2[2])/3.0;
      }
    }

    // normalize median

    for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
      surfdata *surfi;

      surfi = global_scase.surfcoll.surfinfo+i;
      if(surfi->ntris>0){
        surfi->axis[0] /= surfi->ntris;
        surfi->axis[1] /= surfi->ntris;
        surfi->axis[2] /= surfi->ntris;
      }
    }
  }
}

/* ------------------ DrawSelectGeom ------------------------ */

void DrawSelectGeom(void){
  geomdata *geomi;
  geomlistdata *geomlisti;
  int color_index = 1;

  geomi = geominfoptrs[0];
  geomlisti = geomi->geomlistinfo-1;

  switch(select_geom){
  case GEOM_PROP_VERTEX1:
  case GEOM_PROP_VERTEX2:

    if(geomlisti->nverts>0){
      int j;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      glPointSize(20);
      color_index = 1;
      glBegin(GL_POINTS);
      for(j = 0; j<geomlisti->nverts; j++){
        vertdata *verti;
        unsigned char r, g, b;

        verti = geomlisti->verts+j;
        if(verti->geomtype!=GEOM_ISO && verti->ntriangles!=0){
          GetRGB(color_index, &r, &g, &b);
          glColor3ub(r, g, b);
          glVertex3fv(verti->xyz);
        }
        color_index++;
      }
      glEnd();
      glPopMatrix();
    }
    break;
  case GEOM_PROP_TRIANGLE:
  case GEOM_PROP_SURF:
    if(geomlisti->ntriangles>0){
      int i;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      color_index = 0;
      glBegin(GL_TRIANGLES);
      for(i = 0; i<geomlisti->ntriangles; i++){
        tridata *trianglei;
        unsigned char r, g, b;

        trianglei = geomlisti->triangles+i;
        if(trianglei->geomtype!=GEOM_ISO){
          if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
          if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
          if(trianglei->exterior==0)continue;
          if(trianglei->geomtype==GEOM_GEOM&&show_faces_shaded==0)continue;
        }
        GetRGB(color_index+1, &r, &g, &b);
        color_index++;
        glColor3ub(r, g, b);
        {
          vertdata *vert0, *vert1, *vert2;

          vert0 = trianglei->verts[0];
          vert1 = trianglei->verts[1];
          vert2 = trianglei->verts[2];
          glVertex3fv(vert0->xyz);
          glVertex3fv(vert1->xyz);
          glVertex3fv(vert2->xyz);
          glVertex3fv(vert0->xyz);
          glVertex3fv(vert2->xyz);
          glVertex3fv(vert1->xyz);
        }
      }
      glEnd();
    }
  break;
  default:
  assert(FFALSE);
  break;
  }
}

/* ------------------ HaveNonTextures ------------------------ */

int HaveNonTextures(tridata **tris, int ntris){
  int i;

  if(visGeomTextures==visGeomTextures_last)return have_non_textures;
  visGeomTextures_last = visGeomTextures;
  if(visGeomTextures!=1)return 1;
  for(i = 0; i<ntris; i++){
    texturedata *ti;

    ti = tris[i]->textureinfo;
    if(ti==NULL||ti->loaded!=1)return 1;
  }
  return 0;
}

/* ------------------ DrawBoxShaded ------------------------ */

void DrawBoxShaded(float *bb, int flag, int *hidden6, float *box_color){
  float x0, x1, y0, y1, z0, z1;

  x0 = bb[0];
  x1 = bb[1];
  y0 = bb[2];
  y1 = bb[3];
  z0 = bb[4];
  z1 = bb[5];
  glColor3fv(box_color);
  glBegin(GL_TRIANGLES);

  if(flag==2||(flag==3&&hidden6[0]==0)){
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y0, z1);
    glVertex3f(x0, y1, z1);

    if(flag == 3 && hidden6[0] == 0){
      glNormal3f(1.0, 0.0, 0.0);
      glVertex3f(x0, y0, z0);
      glVertex3f(x0, y1, z0);
      glVertex3f(x0, y1, z1);
      glVertex3f(x0, y0, z0);
      glVertex3f(x0, y1, z1);
      glVertex3f(x0, y0, z1);
    }
  }

  if(flag==2||(flag==3&&hidden6[1]==0)){
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y0, z1);

    if(flag == 3 && hidden6[1] == 0){
      glNormal3f(-1.0, 0.0, 0.0);
      glVertex3f(x1, y0, z0);
      glVertex3f(x1, y1, z1);
      glVertex3f(x1, y1, z0);
      glVertex3f(x1, y0, z0);
      glVertex3f(x1, y0, z1);
      glVertex3f(x1, y1, z1);
    }
  }

  if(flag==2||(flag==3&&hidden6[2]==0)){
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z1);

    if(flag == 3 && hidden6[2] == 0){
      glNormal3f(0.0, 1.0, 0.0);
      glVertex3f(x0, y0, z0);
      glVertex3f(x1, y0, z1);
      glVertex3f(x1, y0, z0);
      glVertex3f(x0, y0, z0);
      glVertex3f(x0, y0, z1);
      glVertex3f(x1, y0, z1);
    }
  }

  if(flag==2||(flag==3&&hidden6[3]==0)){
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x0, y1, z1);
    glVertex3f(x1, y1, z1);

    if(flag == 3 && hidden6[3] == 0){
      glNormal3f(0.0, -1.0, 0.0);
      glVertex3f(x0, y1, z0);
      glVertex3f(x1, y1, z0);
      glVertex3f(x1, y1, z1);
      glVertex3f(x0, y1, z0);
      glVertex3f(x1, y1, z1);
      glVertex3f(x0, y1, z1);
    }
  }

  if(flag==2||(flag==3&&hidden6[4]==0)){
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z0);

    if(flag == 3 && hidden6[4] == 0){
      glNormal3f(0.0, 0.0, 1.0);
      glVertex3f(x1, y0, z0);
      glVertex3f(x1, y1, z0);
      glVertex3f(x0, y0, z0);
      glVertex3f(x0, y0, z0);
      glVertex3f(x1, y1, z0);
      glVertex3f(x0, y1, z0);
    }
  }

  if(flag==2||(flag==3&&hidden6[5]==0)){
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    if(flag == 3 && hidden6[5] == 0){
      glNormal3f(0.0, 0.0, -1.0);
      glVertex3f(x0, y0, z1);
      glVertex3f(x1, y1, z1);
      glVertex3f(x1, y0, z1);
      glVertex3f(x0, y0, z1);
      glVertex3f(x0, y1, z1);
      glVertex3f(x1, y1, z1);
    }
  }
  glEnd();
}

/* ------------------ DrawBoxOutline ------------------------ */

void DrawBoxOutline(float *bb, float *box_color){
  glColor3fv(box_color);
  glLineWidth(geom_linewidth);
  glBegin(GL_LINES);
    // xx
  glVertex3f(bb[0], bb[2], bb[4]);
  glVertex3f(bb[1], bb[2], bb[4]);

  glVertex3f(bb[0], bb[2], bb[5]);
  glVertex3f(bb[1], bb[2], bb[5]);

  glVertex3f(bb[0], bb[3], bb[4]);
  glVertex3f(bb[1], bb[3], bb[4]);

  glVertex3f(bb[0], bb[3], bb[5]);
  glVertex3f(bb[1], bb[3], bb[5]);

  // yy
  glVertex3f(bb[0], bb[2], bb[4]);
  glVertex3f(bb[0], bb[3], bb[4]);

  glVertex3f(bb[0], bb[2], bb[5]);
  glVertex3f(bb[0], bb[3], bb[5]);

  glVertex3f(bb[1], bb[2], bb[4]);
  glVertex3f(bb[1], bb[3], bb[4]);

  glVertex3f(bb[1], bb[2], bb[5]);
  glVertex3f(bb[1], bb[3], bb[5]);

  // zz
  glVertex3f(bb[0], bb[2], bb[4]);
  glVertex3f(bb[0], bb[2], bb[5]);

  glVertex3f(bb[0], bb[3], bb[4]);
  glVertex3f(bb[0], bb[3], bb[5]);

  glVertex3f(bb[1], bb[2], bb[4]);
  glVertex3f(bb[1], bb[2], bb[5]);

  glVertex3f(bb[1], bb[3], bb[4]);
  glVertex3f(bb[1], bb[3], bb[5]);
  glEnd();

  glPointSize(geom_pointsize);
  glBegin(GL_POINTS);
  glVertex3f(bb[0], bb[2], bb[4]);
  glEnd();
}

/* ------------------ DrawBoxMinMax ------------------------ */

void DrawBoxMinMax(float *bbmin, float *bbmax, float *box_color){
  float bb[6];

  bb[0] = bbmin[0];
  bb[2] = bbmin[1];
  bb[4] = bbmin[2];
  bb[1] = bbmax[0];
  bb[3] = bbmax[1];
  bb[5] = bbmax[2];
  DrawBoxOutline(bb, box_color);
}

/* ------------------ DrawObstBoundingBox ------------------------ */

void DrawObstBoundingBox(void){
  if(global_scase.obst_bounding_box[0]>global_scase.obst_bounding_box[1])return;
  if(global_scase.obst_bounding_box[2]>global_scase.obst_bounding_box[3])return;
  if(global_scase.obst_bounding_box[4]>global_scase.obst_bounding_box[5])return;
  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
  DrawBoxOutline(global_scase.obst_bounding_box, foregroundcolor);
  glPopMatrix();
}

/* ------------------ DrawGeomBoundingBox ------------------------ */

void DrawGeomBoundingBox(float *boundingbox_color){
  int i;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),vertical_factor*SCALE2SMV(1.0));
  glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
  for(i = 0; i<global_scase.ngeominfo; i++){
    geomdata *geomi;
    int j, have_box;

    geomi = global_scase.geominfo + i;
    if(geomi->geomtype!=GEOM_GEOM)continue;
    have_box = 0;
    for(j = 0; j<geomi->ngeomobjinfo; j++){
      geomobjdata *geomobjj;

      geomobjj = geomi->geomobjinfo+j;
      if(geomobjj->bounding_box!=NULL){
        float *box_color;

        box_color = foregroundcolor;
        if(geomobjj->color!=NULL)box_color = geomobjj->color;
        if(boundingbox_color!=NULL)box_color = boundingbox_color;
        DrawBoxOutline(geomobjj->bounding_box, box_color);
        have_box = 1;
      }
    }
    if(have_box==0){
      DrawBoxOutline(geomi->bounding_box, foregroundcolor);
    }
  }
  glPopMatrix();
}

/* ------------------ DrawGeom ------------------------ */

void DrawGeom(int flag, int timestate){
  int i;
  unsigned char black[]={0,0,0,255};
  float black01[] = {0.0, 0.0, 0.0, 255};
  float blue[]={0.0,0.0,1.0,1.0};
  float skinny_color[]={1.0,0.0,0.0,1.0};
  float *last_color=NULL;
  float last_transparent_level=-1.0;
  int ntris=0;
  tridata **tris;
  int texture_state = OFF, texture_first=1;

  if(global_scase.auto_terrain==1)return;
  if(show_geom_boundingbox==SHOW_BOUNDING_BOX_ALWAYS||geom_bounding_box_mousedown==1){
    if(flag==DRAW_OPAQUE&&timestate==GEOM_STATIC&&have_geom_triangles==1){
      DrawGeomBoundingBox(NULL);
    }
    return;
  }
  if(flag == DRAW_OPAQUE){
    ntris=nopaque_triangles;
    tris=opaque_triangles;
  }
  if(flag==DRAW_TRANSPARENT){
    ntris=ntransparent_triangles;
    tris=transparent_triangles;
  }
  if(ntris==0&&show_faces_shaded==1&&show_faces_outline==0)return;

  if(ntris>0&&timestate==GEOM_STATIC){
    float *color=NULL;
    surfdata *selected_surf;

  // draw geometry surface

    if(flag==DRAW_TRANSPARENT&&use_transparency_data==1){
      TransparentOn();
    }

    if(texture_state==OFF){
      texture_state=TextureOn(texture_iso_colorbar_id,&texture_first);
    }

    if(select_geom==GEOM_PROP_SURF&&ntris>0&&selected_geom_triangle>=0){
      tridata *selected_triangle;

      selected_triangle = tris[selected_geom_triangle];
      selected_surf = selected_triangle->geomsurf;
    }
    else{
      selected_surf = NULL;
    }
    have_non_textures = HaveNonTextures(tris, ntris);
    if(cullfaces==1)glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    ENABLE_LIGHTING;
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,iso_specular);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,iso_shininess);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,block_ambient2);
    glEnable(GL_COLOR_MATERIAL);

    glPushMatrix();
    glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),vertical_factor*SCALE2SMV(1.0));
    glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
    glBegin(GL_TRIANGLES);
    if(have_non_textures==1){
      for(i = 0; i<ntris; i++){
        tridata *trianglei;
        float transparent_level_local;
        texturedata *ti;
        int  j;
        int smooth_triangles;
        int use_select_color;

        trianglei = tris[i];
        use_select_color = 0;
        if(use_cfaces==1&&trianglei->geomtype==GEOM_GEOM)continue;
        if(select_geom==GEOM_PROP_TRIANGLE||select_geom==GEOM_PROP_SURF){
          if(trianglei->geomtype==GEOM_ISO)continue;
          if(select_geom==GEOM_PROP_TRIANGLE&&selected_geom_triangle==i)use_select_color = 1;
          if(select_geom==GEOM_PROP_SURF&&selected_surf==trianglei->geomsurf)use_select_color = 1;
        }
        if(trianglei->geomtype!=GEOM_ISO){
          if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
          if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
          if(trianglei->exterior==0)continue;
          if(trianglei->geomtype==GEOM_GEOM&&show_faces_shaded==0)continue;
          if(trianglei->geomsurf->invisible==1)continue;
        }
        else{
          if(show_iso_shaded==0)continue;
        }
        ti = trianglei->textureinfo;
        if(show_texture_1dimage==1)continue;
        if(visGeomTextures==1&&ti!=NULL&&ti->loaded==1)continue;

        if((trianglei->geomtype==GEOM_GEOM&&smooth_geom_normal==0)||
          (trianglei->geomtype==GEOM_ISO &&smooth_iso_normal==0)){
          smooth_triangles = 0;
        }
        else{
          smooth_triangles = 1;
        }
        if(hilight_skinny==1&&trianglei->skinny==1){
          color = skinny_color;
          transparent_level_local = 1.0;
        }
        else{
          if(trianglei->geomobj!=NULL&&trianglei->geomobj->color!=NULL&&trianglei->geomobj->use_geom_color==1){
            color = trianglei->geomobj->color;
            transparent_level_local = trianglei->geomobj->color[3];
          }
          else{
            color = trianglei->geomsurf->color;
            transparent_level_local = trianglei->geomsurf->transparent_level;
          }
        }
        if(geom_force_transparent==1)transparent_level_local = geom_transparency;
        if(use_select_color==1||use_surf_color==1){
          unsigned char geom_rgb_uc[4];

          if(use_surf_color==1){
            int *gcolor;

            gcolor = trianglei->geomsurf->geom_surf_color;
            geom_rgb_uc[0] = (unsigned char)gcolor[0];
            geom_rgb_uc[1] = (unsigned char)gcolor[1];
            geom_rgb_uc[2] = (unsigned char)gcolor[2];
          }
          if(use_select_color==1){
            if(select_geom==GEOM_PROP_TRIANGLE||select_geom==GEOM_PROP_SURF){
              geom_rgb_uc[0] = (unsigned char)geom_triangle_rgb[0];
              geom_rgb_uc[1] = (unsigned char)geom_triangle_rgb[1];
              geom_rgb_uc[2] = (unsigned char)geom_triangle_rgb[2];
            }
          }
          if(texture_state==ON){
            glEnd();
            texture_state = TextureOff();
            glBegin(GL_TRIANGLES);
          }
          if(geom_force_transparent==1){
            geom_rgb_uc[3] = CLAMP(255*geom_transparency, 0, 255);
          }
          else{
            geom_rgb_uc[3] = 255;
          }
          glColor4ubv(geom_rgb_uc);
          last_transparent_level = -1.0;
        }
        else{
          if(iso_opacity_change==0||trianglei->geomtype!=GEOM_ISO){
            if(color!=last_color||ABS(last_transparent_level-transparent_level_local)>0.001){
              if(texture_state==ON){
                glEnd();
                texture_state = TextureOff();
                glBegin(GL_TRIANGLES);
              }
              if(color!=NULL) glColor4f(color[0], color[1], color[2], transparent_level_local);
              last_color = color;
              last_transparent_level = transparent_level_local;
            }
          }
        }

        if(smooth_triangles==0){
          glNormal3fv(trianglei->tri_norm);
          for(j = 0; j<3; j++){
            vertdata *vertj;

            vertj = trianglei->verts[j];
            glVertex3fv(vertj->xyz);
          }
        }
        else{
          for(j = 0; j<3; j++){
            vertdata *vertj;
            float v1[3], v2[3];
            float *vnorm, *vpos;
            float factor;
            float transparent_level_local_new;
            geomlistdata *geomlisti = NULL;
            float *vertvals = NULL;
            float texture_val;

            vertj = trianglei->verts[j];
            geomlisti = trianglei->geomlisti;
            if(geomlisti!=NULL)vertvals = geomlisti->vertvals;
            if(show_iso_color==1&&vertvals!=NULL){
              int vertj_index;
              float vertval;
              int colorbar_index;

              vertj_index = vertj-trianglei->geomlisti->verts;
              vertval = vertvals[vertj_index];
              texture_val = CLAMP((vertval-iso_valmin)/(iso_valmax-iso_valmin), 0.0, 1.0);
              colorbar_index = CLAMP((int)(255.0*texture_val), 0, 255);
              color = rgb_iso+4*colorbar_index;
            }
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
              factor = ABS(DOT3(v1, v2));
              transparent_level_local_new = CLAMP(transparent_level_local, 0.0, 1.0);
              if(factor!=0.0&&transparent_level_local<1.0)transparent_level_local_new = 1.0-pow(1.0-transparent_level_local, 1.0/factor);
              if(show_iso_color==1&&vertvals!=NULL&&trianglei->geomtype!=GEOM_GEOM){
                if(texture_state==OFF){
                  glEnd();
                  texture_state = TextureOn(texture_iso_colorbar_id, &texture_first);
                  glBegin(GL_TRIANGLES);
                }
                glTexCoord1f(texture_val);
              }
              else{
                if(texture_state==ON){
                  glEnd();
                  texture_state = TextureOff();
                  glBegin(GL_TRIANGLES);
                }
                glColor4f(color[0], color[1], color[2], transparent_level_local_new);
              }
            }
            glNormal3fv(trianglei->vert_norm+3*j);
            glVertex3fv(vertj->xyz);
          }
        }
      }
    }
    glEnd();
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
          if(trianglei->exterior==0)continue;
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
          {
            float tx[3], ty[3];

            for(j = 0; j < 3; j++){
              float *tvertj;

              tvertj = trianglei->tverts + 2 * j;
              tx[j] = tvertj[0];
              ty[j] = tvertj[1];
            }
            // textures on a sphere are periodic
            if(trianglei->geomobj!=NULL && trianglei->geomobj->texture_mapping==TEXTURE_SPHERICAL){
              if(MAX(tx[0],tx[2])>0.8&&tx[1]<0.2){
                tx[1]+=1.0;
              }
              if(MAX(tx[0],tx[1])>0.8&&tx[2]<0.2){
                tx[2]+=1.0;
              }
              if(MAX(tx[1],tx[2])>0.8&&tx[0]<0.2){
                tx[0]+=1.0;
              }
            }
            for(j = 0; j < 3; j++){
              vertdata *vertj;

              vertj = trianglei->verts[j];
              glNormal3fv(vertj->vert_norm);
              glTexCoord2f(tx[j],ty[j]);
              glVertex3fv(vertj->xyz);
            }
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
    if(geomi->is_terrain==1)continue;
    if(geomi->geomtype!=GEOM_GEOM&&geomi->geomtype!=GEOM_ISO)continue;
    if(timestate==GEOM_STATIC||geomi->ntimes==0){
      geomlisti = geomi->geomlistinfo-1;
    }
    else{
      geomlisti = geomi->geomlistinfo+geomi->itime;
    }

    if(show_surf_axis==1){
      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      glLineWidth(glui_surf_axis_width);
      glBegin(GL_LINES);
      for(i = 0; i<global_scase.surfcoll.nsurfinfo;  i++){
        surfdata *surfi;
        float *axis;
        float x0, y0, z0;
        float x1, y1, z1;

        surfi = global_scase.surfcoll.surfinfo+i;
        if(surfi->ntris==0)continue;
        axis = surfi->axis;

        x0 = axis[0];
        x1 = x0+glui_surf_axis_length;
        y0 = axis[1];
        y1 = y0+glui_surf_axis_length;
        z0 = axis[2];
        z1 = z0+glui_surf_axis_length;

        glColor3f(1.0,0.0,0.0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        Output3Text(foregroundcolor, x1,y0,z0, "X");

        glColor3f(0.0, 1.0, 0.0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        Output3Text(foregroundcolor, x0, y1, z0, "Y");

        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
        Output3Text(foregroundcolor, x0, y0, z1, "Z");
      }
      glEnd();
      for(i = 0; i<global_scase.surfcoll.nsurfinfo; i++){
        surfdata *surfi;
        float *axis;
        float x0, y0, z0;
        float x1, y1, z1;

        surfi = global_scase.surfcoll.surfinfo+i;
        if(surfi->ntris==0)continue;
        axis = surfi->axis;

        x0 = axis[0];
        x1 = x0+glui_surf_axis_length;
        y0 = axis[1];
        y1 = y0+glui_surf_axis_length;
        z0 = axis[2];
        z1 = z0+glui_surf_axis_length;

        Output3Text(foregroundcolor, x1, y0, z0, "X");
        Output3Text(foregroundcolor, x0, y1, z0, "Y");
        Output3Text(foregroundcolor, x0, y0, z1, "Z");
      }
      glPopMatrix();
    }

    // draw geometry (faces) outline

    last_color=NULL;
    if(geomlisti->ntriangles>0){
      float line_offset;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
      glTranslatef(0.0, 0.0, geom_dz_offset);
      if(geomi->geomtype==GEOM_ISO){
        glLineWidth(isolinewidth);
        line_offset = iso_outline_offset;
      }
      else{
        glLineWidth(geom_linewidth);
        line_offset = geom_norm_offset;
      }
      glBegin(GL_LINES);

      unsigned char outlinecolor_uc[4];
      if(geomi->geomtype != GEOM_ISO){
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
      }
      glColor4ubv(outlinecolor_uc);
      for(j=0;j<geomlisti->ntriangles;j++){
        tridata *trianglei;

        trianglei = geomlisti->triangles+j;
        if(trianglei->geomtype!=GEOM_ISO){
          if(show_faces_outline==0)continue;
          if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
          if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
          if(trianglei->exterior==0)continue;
          if(trianglei->geomtype == GEOM_GEOM&&show_faces_outline == 0)continue;
        }
        else{
          if(show_iso_outline == 0)continue;
        }

        if(geomi->geomtype == GEOM_ISO){
          if(show_iso_shaded == 1){
            color = black01;
          }
          else{
            color = trianglei->geomsurf->color;
          }
          if(last_color != color){
            glColor3fv(color);
            last_color = color;
          }
        }
        {
          float vert2a[3], vert2b[3], vert2c[3];
          float     *xyz0,     *xyz1,     *xyz2;
          float    *norm0,    *norm1,    *norm2;
          vertdata *vert0,    *vert1,    *vert2;
          int k;

          vert0 = trianglei->verts[0];
          vert1 = trianglei->verts[1];
          vert2 = trianglei->verts[2];

          xyz0  = vert0->xyz;
          xyz1  = vert1->xyz;
          xyz2  = vert2->xyz;

          norm0 = vert0->vert_norm;
          norm1 = vert1->vert_norm;
          norm2 = vert2->vert_norm;

          for(k=0;k<3;k++){
            vert2a[k] = xyz0[k] + line_offset*norm0[k];
            vert2b[k] = xyz1[k] + line_offset*norm1[k];
            vert2c[k] = xyz2[k] + line_offset*norm2[k];
          }

          glVertex3fv(vert2a);
          glVertex3fv(vert2b);

          glVertex3fv(vert2b);
          glVertex3fv(vert2c);

          glVertex3fv(vert2c);
          glVertex3fv(vert2a);
        }
      }
      glEnd();
      glPopMatrix();
    }

    // draw geometry verts

    last_color=NULL;
    if(geomlisti->nverts>0){
      float line_offset;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
      glTranslatef(0.0, 0.0, geom_dz_offset);
      glPointSize(geom_pointsize);
      glBegin(GL_POINTS);
      if(geomi->geomtype==GEOM_ISO){
        line_offset = iso_outline_offset;
      }
      else{
        line_offset = geom_norm_offset;
      }
      for(j=0;j<geomlisti->nverts;j++){
        vertdata *verti;
        int use_select_color;

        verti = geomlisti->verts+j;
        use_select_color=0;
        if(select_geom==GEOM_PROP_VERTEX1||select_geom==GEOM_PROP_VERTEX2){
          if(verti->geomtype==GEOM_ISO||verti->ntriangles==0)continue;
          if(selected_geom_vertex1==j)use_select_color = 1;
          if(selected_geom_vertex2==j)use_select_color = 2;
        }
        else{ // draw vertices normally if vertices are not being selected
          if(verti->geomtype==GEOM_GEOM&&show_geom_verts == 0)continue;
          if(verti->geomtype==GEOM_ISO&&show_iso_points == 0)continue;
          if(verti->ntriangles==0)continue;
        }
        if(use_select_color==1){
          unsigned char geom_vertex1_rgb_uc[3];

          geom_vertex1_rgb_uc[0] = (unsigned char)geom_vertex1_rgb[0];
          geom_vertex1_rgb_uc[1] = (unsigned char)geom_vertex1_rgb[1];
          geom_vertex1_rgb_uc[2] = (unsigned char)geom_vertex1_rgb[2];
          glColor3ubv(geom_vertex1_rgb_uc);
          last_color = NULL;
        }
        else if(use_select_color==2){
          unsigned char geom_vertex2_rgb_uc[3];

          geom_vertex2_rgb_uc[0] = (unsigned char)geom_vertex2_rgb[0];
          geom_vertex2_rgb_uc[1] = (unsigned char)geom_vertex2_rgb[1];
          geom_vertex2_rgb_uc[2] = (unsigned char)geom_vertex2_rgb[2];
          glColor3ubv(geom_vertex2_rgb_uc);
          last_color = NULL;
        }
        else{
          color = verti->triangles[0]->geomsurf->color;
          if(last_color!=color){
            glColor3fv(color);
            last_color = color;
          }
        }
        {
          float vert2a[3];
          int k;

          for(k=0;k<3;k++){
            vert2a[k] = verti->xyz[k] + line_offset*verti->vert_norm[k];
          }

          glVertex3fv(vert2a);
        }
      }
      glEnd();
      glPopMatrix();
    }

    // draw geometry normal vectors

    int doit = 0;
    if(geomlisti->ntriangles > 0){
      if(show_geom_normal==1&&smooth_geom_normal==0)doit = 1;
      if(show_iso_normal == 1&&smooth_iso_normal==0)doit = 1;
    }
    if(doit==1){  // draw faceted normals
      glPushMatrix();
      glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0,-global_scase.ybar0,-global_scase.zbar0);
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

      glPointSize(geom_pointsize);  // draw verts at end of vector
      glBegin(GL_POINTS);
      glColor4ubv(black);
      for(j=0;j<geomlisti->ntriangles;j++){
        float *v1, *v2, *v3;
        float *xyznorm;
        tridata *trianglei;
        float xyz1[3], xyz2[3];

        trianglei = geomlisti->triangles+j;
        if(trianglei->outside_domain == 0 && showgeom_inside_domain == 0)continue;
        if(trianglei->outside_domain == 1 && showgeom_outside_domain == 0)continue;
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
    doit = 0;
    if(geomlisti->ntriangles > 0){
      if(show_geom_normal == 1 && smooth_geom_normal == 1)doit = 1;
      if(show_iso_normal == 1 && smooth_iso_normal == 1)doit = 1;
    }
    if(doit==1){  // draw smooth normals
      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
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
      glPointSize(geom_pointsize);  // draw verts at end of vector
      glBegin(GL_POINTS);
      glColor4ubv(black);
      for(j = 0; j < geomlisti->ntriangles; j++){
        tridata *trianglei;
        int k;

        trianglei = geomlisti->triangles + j;
        if(trianglei->outside_domain == 0 && showgeom_inside_domain == 0)continue;
        if(trianglei->outside_domain == 1 && showgeom_outside_domain == 0)continue;
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
          norm[0] += trianglek->area*tri_normk[0];
          norm[1] += trianglek->area*tri_normk[1];
          norm[2] += trianglek->area*tri_normk[2];
        }
        ReduceToUnit(norm);
        vertj->vert_norm[0] = norm[0];
        vertj->vert_norm[1] = norm[1];
        vertj->vert_norm[2] = norm[2];
      }
      else{
        for(k = 0; k<vertj->ntriangles; k++){
          tridata *trianglek;
          float *tri_normk;
          float lengthi, lengthk;

          trianglek = vertj->triangles[k];
          if(trianglek->exterior == 0)continue;
          tri_normk = trianglek->tri_norm;
          lengthk = NORM3(tri_normk);
          lengthi = NORM3(tri_normi);
          if(lengthk > 0.0&&lengthi > 0.0){
            norm[0] += trianglek->area*tri_normk[0];
            norm[1] += trianglek->area*tri_normk[1];
            norm[2] += trianglek->area*tri_normk[2];
          }
        }
        ReduceToUnit(norm);
        vertj->vert_norm[0] = norm[0];
        vertj->vert_norm[1] = norm[1];
        vertj->vert_norm[2] = norm[2];
      }
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
  int ntimes_max=0;

  GetGeomInfoPtrs(0);
  updating_triangles = 1;
  if(cancel_update_triangles==1){
    updating_triangles = 0;
    return;
  }

  if(update==GEOM_UPDATE_NORMALS){
    UpdateGeomNormals();
    updating_triangles = 0;
    return;
  }
  for(j=0;j<ngeominfoptrs;j++){
    geomdata *geomi;

    geomi = geominfoptrs[j];
    if(geomi->loaded==0||geomi->display==0)continue;
    if(geomi->geomtype != GEOM_GEOM&&geomi->geomtype!=GEOM_ISO)continue;
    ntimes_max=MAX(ntimes_max,geomi->ntimes);
  }
  if(cancel_update_triangles==1){
    updating_triangles = 0;
    return;
  }

  for(ii=-1;ii<ntimes_max;ii++){
    geomlistdata *geomlisti;
    int ntriangles;
    tridata **triangles;

    for(j=0;j<ngeominfoptrs;j++){
      geomdata *geomi;
      float *xyzptr[3];
      float *xyznorm;
      int i;

      if(cancel_update_triangles==1){
        updating_triangles = 0;
        return;
      }
      geomi = geominfoptrs[j];
      if(ii>geomi->ntimes-1)continue;
      if(geomi->loaded==0||geomi->display==0)continue;
      if(geomi->geomtype!=GEOM_GEOM && geomi->geomtype!=GEOM_ISO)continue;

      if(ii==-1||geomi->currentframe==NULL){
        geomlisti = geomi->geomlistinfo + ii;
      }
      else{
        geomlisti = geomi->currentframe;
      }
      if(geomlisti-geomi->geomlistinfo!=ii)continue;

      for(i=0;i<geomlisti->ntriangles;i++){
        tridata *trianglei;
        vertdata **verts;

        trianglei = geomlisti->triangles+i;
        verts = trianglei->verts;

        xyzptr[0] = verts[0]->xyz;
        xyzptr[1] = verts[1]->xyz;
        xyzptr[2] = verts[2]->xyz;
        xyznorm = trianglei->tri_norm;
        GetTriangleNormal(xyzptr[0],xyzptr[1],xyzptr[2],xyznorm,&trianglei->area);
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

      ntriangles = 3*geomlisti->ntriangles;

      // allocate triangle pointers

      FREEMEMORY(geomlisti->connected_triangles);
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
      if(large_case==0)SmoothGeomNormals(geomlisti,geomi->geomtype);
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

      if(cancel_update_triangles==1){
        updating_triangles = 0;
        FREEMEMORY(surface_verts);
        FREEMEMORY(match_verts);
        return;
      }
      nsurface_verts = 0;
      for(j = 0; j<ngeominfoptrs; j++){
        geomlistdata *geomlisti;
        int  i;
        geomdata *geomj;

        geomj = geominfoptrs[j];
        if(geomj->geomtype != GEOM_GEOM&&geomj->geomtype!=GEOM_ISO)continue;
        geomlisti = geomj->geomlistinfo+ii;
        if(geomlisti->verts != NULL){
          for(i = 0; i < geomlisti->nverts; i++){
            vertdata *verti;

            verti = geomlisti->verts + i;
            verti->on_mesh_boundary = OnMeshBoundary(verti->xyz);
            if(verti->on_mesh_boundary == 1)nsurface_verts++;
          }
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
          if(geomlisti->verts != NULL){
            for(i = 0; i < geomlisti->nverts; i++){
              vertdata *verti;

              verti = geomlisti->verts + i;
              if(verti->on_mesh_boundary == 1){
                if(isurf < nsurface_verts){
                  surface_verts[isurf] = verti;
                  match_verts[isurf] = -1;
                  isurf++;
                }
              }
            }
          }
        }

        // average normals

        if(large_case==0){
          for(iii = 0; iii < nsurface_verts; iii++){
            int jjj;
            vertdata *verti;
            float *xyzi, *normi;
            float avgnorm[3];

            if(match_verts[iii] >= 0)continue;
            verti = surface_verts[iii];
            xyzi = verti->xyz;
            normi = verti->vert_norm;
            avgnorm[0] = normi[0];
            avgnorm[1] = normi[1];
            avgnorm[2] = normi[2];
            match_verts[iii] = iii;
            for(jjj = iii + 1; jjj < nsurface_verts; jjj++){
              vertdata *vertj;
              float *xyzj, *normj;

              if(match_verts[jjj] >= 0)continue;
              vertj = surface_verts[jjj];
              xyzj = vertj->xyz;
              normj = vertj->vert_norm;
#define POINTEPS 0.001
              if(ABS(xyzi[0] - xyzj[0]) < POINTEPS && ABS(xyzi[1] - xyzj[1]) < POINTEPS && ABS(xyzi[2] - xyzj[2]) < POINTEPS){
                match_verts[jjj] = iii;
                avgnorm[0] += normj[0];
                avgnorm[1] += normj[1];
                avgnorm[2] += normj[2];
              }
            }
            ReduceToUnit(avgnorm);
            for(jjj = iii; jjj < nsurface_verts; jjj++){
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
  updating_triangles = 0;
}

/* ------------------ GetGeomDataSizeFixed ------------------------ */

int GetGeomDataSizeFixed(patchdata *patchi, int *nvars, int time_frame, int *geom_offsets, int *geom_offset_flag, int *max_buffer_size, int *error){
  int ntimes_local = 0;
  FILE_SIZE file_size;
  int frame_size, header_size;
  int nvars_per_frame;
  int i;

  *error = 0;
  file_size = GetFileSizeSMV(patchi->reg_file);
  if(max_buffer_size!=NULL)*max_buffer_size = 0;
  if(file_size == 0)return 0;

  header_size = 2*(4 + 4 + 4);
  frame_size  = 12; // time
  frame_size += 24; // nval1, nval2, nval3, nval4
  nvars_per_frame = patchi->geominfo->geomlistinfo_0->ntriangles;
  if(max_buffer_size!=NULL)*max_buffer_size = nvars_per_frame;
  frame_size += 8 + nvars_per_frame * sizeof(float); // data
  if(frame_size > 0){
    ntimes_local = (file_size - header_size) / frame_size;
  }
  if(geom_offset_flag != NULL && *geom_offset_flag == GET_GEOM_OFFSETS){
    *nvars = ntimes_local * nvars_per_frame;
  }
  else{
    *nvars = nvars_per_frame;
    ntimes_local = 1;
    if(geom_offsets != NULL)geom_offsets[0] = header_size;
  }
  if(geom_offsets!=NULL&&geom_offset_flag != NULL && *geom_offset_flag == BUILD_GEOM_OFFSETS){
    for(i = 0;i < ntimes_local;i++){
      geom_offsets[i] = header_size + i * frame_size;
    }
  }
  return ntimes_local;
}

/* ------------------ GetGeomDataSize ------------------------ */

int GetGeomDataSize(char *filename, int *nvars, int time_frame, int *cvals_offsets, int *cvals_sizes, int *geom_offsets, int *geom_offset_flag, int *max_buffer_size, int *error){

  float time;
  int one, version;
  int nvert_s, nvert_d, nface_s, nface_d;
  FILE_m *stream=NULL;
  int nvars_local, ntimes_local;
  int iframe;
  int geom_offset_index=0, geom_offset = 0, frame_start;
  int is_compressed=0;
  int count_read;

  *error=1;
  *nvars = 0;
  if(filename==NULL)return 0;
  if(cvals_sizes != NULL)is_compressed = 1;
  stream = fopen_b(filename,NULL,0,"rb");
  if(stream == NULL){
    if(is_compressed == 1){
      printf(" The compressed boundary file %s failed to open\n", filename);
    }
    else{
      printf(" The boundary file %s failed to open\n", filename);
    }
  }

  *error = 0;
  if(max_buffer_size != NULL)*max_buffer_size = 0;

  if(is_compressed==1){
    // OUTPUT
    // 1
    // completion (0/1)
    // fileversion (compressed format)
    // min max (used to perform conversion)
    // for each time step
    // time
    // nval1,nval2,nval3,nval4
    // ncompressed
    // compressed_1,...,compressed_ncompressed
    fseek_m(stream, 20, SEEK_CUR);
  }
  else{
    FORTREAD_m(&one, 4, 1, stream);
    FORTREAD_m(&version, 4, 1, stream);
  }

  geom_offset = 0;
  ntimes_local = 0;
  nvars_local = 0;
  if(geom_offset_flag==NULL||*geom_offset_flag==BUILD_GEOM_OFFSETS||time_frame==ALL_FRAMES){
    frame_start = 0;
  }
  else{
    frame_start = time_frame;
    if(geom_offsets!=NULL)fseek_m(stream, geom_offsets[time_frame], SEEK_CUR);
  }
  int count = 0;
  for(iframe=frame_start;;iframe++){
    int nvals[4], nskip;

    if(geom_offset_flag!=NULL&&*geom_offset_flag==BUILD_GEOM_OFFSETS)geom_offsets[geom_offset_index] = geom_offset;
    if(is_compressed==1){
      count_read = fread_m(&time, 4, 1, stream);
      geom_offset += 4;
    }
    else{
      FORTREAD_m(&time, 4, 1, stream);
      geom_offset += (4+4+4);
    }
    if(count_read != 1)break;
    if(is_compressed==1){
      int ncvals;
      int ntotal;

      fread_m(nvals, 4, 4, stream);
      if(max_buffer_size != NULL){
        ntotal = nvals[0] + nvals[1] + nvals[2] + nvals[3];
        *max_buffer_size = MAX(*max_buffer_size, ntotal);
      }
      geom_offset += 16;
      fread_m(&ncvals, 4, 1, stream);
      if(cvals_offsets != NULL){
        if(count == 0){
          cvals_offsets[count] = 0;
        }
        else{
          cvals_offsets[count] = cvals_offsets[count-1] + cvals_sizes[count-1];
        }
        cvals_sizes[count] = ncvals;
        count++;
      }
      geom_offset += 4;
      nvars_local += ncvals;
      fseek_m(stream, ncvals, SEEK_CUR);
      geom_offset += ncvals;
    }
    else{
      FORTREAD_m(nvals, 4, 4, stream);
      geom_offset += (4+4*4+4);
      if(count_read != 4)break;
      nvert_s = nvals[0];
      nface_s = nvals[1];
      nvert_d = nvals[2];
      nface_d = nvals[3];
      nskip = 0;
      if(nvert_s>0)nskip += 4 + 4*nvert_s + 4;
      if(nface_s>0)nskip += 4 + 4*nface_s + 4;
      if(nvert_d>0)nskip += 4 + 4*nvert_d + 4;
      if(nface_d>0)nskip += 4 + 4*nface_d + 4;
      geom_offset += nskip;
    }
    if(time_frame==ALL_FRAMES||time_frame==iframe){
      if(is_compressed == 0){
        nvars_local += nvert_s + nvert_d + nface_s + nface_d;
      }
      ntimes_local++;
    }
    geom_offset_index++;
    if(geom_offset_flag!=NULL&&*geom_offset_flag==GET_GEOM_OFFSETS&&time_frame==iframe)break;
    if(is_compressed == 0){
      if(fseek_m(stream, nskip, SEEK_CUR) != 0)break;
    }
  }
  fclose_b(stream);
  *nvars = nvars_local;
  if(geom_offset_flag!=NULL&&*geom_offset_flag==BUILD_GEOM_OFFSETS)*geom_offset_flag = geom_offset_index;
  return ntimes_local;
}

/* ------------------ GetGeomData------------------------ */

FILE_SIZE GetGeomData(patchdata *patchi, char *filename, int load_flag, int ntimes, int nvals, float *times, int *nstatics, int *ndynamics, float *vals,
                      int time_frame, float *time_value, int *geom_offsets, int *error){
  FILE_SIZE file_size;

  int one, nvars;
  int nvert_s, ntri_s, nvert_d, ntri_d;
  int version;
  int count;
  float time;
  int iframe, frame_start, frame_stop;
  char *ext=NULL;
  int is_compressed = 0;
  float valminmax[2];
  unsigned char *cvals;

  //***format (uncompressed)
  // 1
  // version
  // for each time step:
  // time
  // nvert_static, ntri_static, nvert_dynamic, ntri_dynamic
  // if(nvert_static>0) vals_1, ...vals_nvert_static
  // if(ntri_static>0)  vals_1, ...vals_ntri_static
  // if(nvert_dynamic>0)vals_1, ...vals_nvert_dynamic
  // if(ntri_dynamic>0) vals_1, ...vals_ntri_dynamic

  //*** compressed (compressed)
  // 1
  // completion (0/1)
  // fileversion (compressed format)
  // min max (used to perform conversion)
  // for each time step
  // time
  // ncompressed
  // compressed_1,...,compressed_ncompressed

  FILE_m *stream;
  int count_read;

#ifdef pp_BOUNDFRAME
  if(patchi != NULL&&load_flag!=UNLOAD){
    patchi->frameinfo = FRAMELoadData(patchi->frameinfo, patchi->file, load_flag, time_frame, FORTRAN_FILE, GetGeomDataFrameInfo);
    update_frame = 1;
  }
#endif

  cvals = (unsigned char *)vals;
  *error = 1;
  if(filename==NULL)return 0;
  ext = strrchr(filename, '.');
  if(ext != NULL && strcmp(ext, ".svz") == 0)is_compressed = 1;
#ifdef pp_BOUNDFRAME
  if(patchi!=NULL && patchi->frameinfo!=NULL && patchi->frameinfo->bufferinfo!=NULL){
    bufferdata *bufferinfo;

    bufferinfo = patchi->frameinfo->bufferinfo;
    stream     = fopen_b(patchi->reg_file, bufferinfo->buffer, bufferinfo->nbuffer, "rb");
    file_size  = patchi->frameinfo->filesize;
  }
  else{
    stream    = fopen_b(filename, NULL, 0, "rb");
    file_size = GetFileSizeSMV(filename);
  }
#else
  stream = fopen_b(filename, NULL, 0, "rb");
  file_size = GetFileSizeSMV(filename);
#endif
  if(stream == NULL){
    if(is_compressed == 1){
      printf(" The compressed boundary file %s failed to open\n", filename);
    }
    else{
    printf(" The boundary file %s failed to open\n", filename);
    }
    return 0;
  }

  *error = 0;
  if(is_compressed == 1){
    int completion;

    fread_m(&one, 4, 1, stream);
    fread_m(&completion, 4, 1, stream);
    fread_m(&version, 4, 1, stream);
    fread_m(valminmax, 4, 2, stream);
  }
  else{
    FORTREAD_m(&one, 4, 1, stream);
    FORTREAD_m(&version, 4, 1, stream);
  }
  nvars = 0;
  count = 0;
  if(time_frame==ALL_FRAMES){
    frame_start = 0;
    frame_stop = ntimes;
  }
  else{
    frame_start = time_frame;
    frame_stop = time_frame+1;
    if(geom_offsets!=NULL)fseek_m(stream, geom_offsets[time_frame], SEEK_CUR);
  }
  for(iframe = frame_start; iframe<frame_stop; iframe++){
    int nvals_local[4];
    int ncompressed;

    if(is_compressed == 1){
      fread_m(&time, 4, 1, stream);
    }
    else{
      FORTREAD_m(&time, 4, 1, stream);
      if(count_read!=1)break;
    }
    if(time_frame==ALL_FRAMES||time_frame==iframe)times[count] = time;
    if(is_compressed == 1){
      fread_m(nvals_local, 4, 4, stream);
      fread_m(&ncompressed, 4, 1, stream);
    }
    else{
      FORTREAD_m(nvals_local, 4, 4, stream);
      if(count_read != 4)break;
    }
    nvert_s = nvals_local[0];
    ntri_s  = nvals_local[1];
    nvert_d = nvals_local[2];
    ntri_d  = nvals_local[3];
    if(time_frame==ALL_FRAMES||time_frame==iframe)nstatics[count] = nvert_s+ntri_s;

    if(is_compressed == 1){
      if(ncompressed>0){
        fread_m(cvals, 1, ncompressed, stream);
        cvals += ncompressed;
      }
    }
    else{
      if(nvert_s > 0){
        if(time_frame == ALL_FRAMES || time_frame == iframe){
          FORTREAD_m(vals + nvars, 4, nvert_s, stream);
          if(count_read!=nvert_s)break;
          nvars += nvert_s;
        }
        else{
          fseek_m(stream, 4 + 4 * nvert_s + 4, SEEK_CUR);
        }
      }
      if(ntri_s > 0){
        if(time_frame == ALL_FRAMES || time_frame == iframe){
          FORTREAD_m(vals + nvars, 4, ntri_s, stream);
          if(count_read!=ntri_s)break;
          nvars += ntri_s;
        }
        else{
          fseek_m(stream, (4 + 4 * ntri_s + 4), SEEK_CUR);
        }
      }
    }
    if(time_frame == ALL_FRAMES || time_frame == iframe)ndynamics[count] = nvert_d + ntri_d;
    if(is_compressed == 0){
      if(nvert_d > 0){
        if(time_frame == ALL_FRAMES || time_frame == iframe){
          FORTREAD_m(vals + nvars, 4, nvert_d, stream);
          if(count_read!=nvert_d)break;
          nvars += nvert_d;
        }
        else{
          fseek_m(stream, 4 + 4 * nvert_d + 4, SEEK_CUR);
        }
      }
      if(ntri_d > 0){
        if(time_frame == ALL_FRAMES || time_frame == iframe){
          FORTREAD_m(vals + nvars, 4, ntri_d, stream);
          if(count_read!=ntri_d)break;
          nvars += ntri_d;
        }
        else{
          fseek_m(stream, 4 + 4 * ntri_d + 4, SEEK_CUR);
        }
      }
    }
    if(time_frame==iframe&&time_value!=NULL){
      if(times != NULL){
        *time_value = times[count];
      }
      else{
        *time_value = 0.0;
      }
      break;
    }
    if(time_frame==ALL_FRAMES)count++;
  }
#ifndef pp_BOUNDFRAME
  fclose_m(stream);
#endif
  return file_size;
}

/* ------------------ ReadGeomData ------------------------ */

FILE_SIZE ReadGeomData(patchdata *patchi, slicedata *slicei, int load_flag, int time_frame, float *time_value, int flag, int *errorcode){
  int ntimes_local;
  int i;
  int nvals=0;
  int n;
  int error;
  FILE_SIZE return_filesize = 0;
  float total_time;
  int *geom_offsets = NULL, geom_offset_flag;
  int *cvals_offsets = NULL, *cvals_sizes = NULL;
  int max_buffer_size = 0;
  unsigned char *cbuffer = NULL;

  if(patchi->structured == YES)return 0;

  START_TIMER(total_time);

  patchi->loaded = 0;
  patchi->display = 0;
  if(slicei != NULL){
    slicei->loaded = 0;
    slicei->display = 0;
    slicei->vloaded = 0;
    slicei->ntimes = 0;
    slicei->times = NULL;
  }
  patchi->bounds.defined = 0;
  have_boundary_arrival = 0;

  FREEMEMORY(patchi->geom_nstatics);
  FREEMEMORY(patchi->geom_ndynamics);
  FREEMEMORY(patchi->geom_ivals_static_offset);
  FREEMEMORY(patchi->geom_ivals_dynamic_offset);
  FREEMEMORY(patchi->geom_vals_static_offset);
  FREEMEMORY(patchi->geom_vals_dynamic_offset);
  FREEMEMORY(patchi->geom_vals);
  FREEMEMORY(patchi->geom_ivals);
  FREEMEMORY(patchi->geom_times);
  FREEMEMORY(patchi->geom_times_map);
#ifdef pp_BOUNDFRAME
  if(load_flag != RELOAD){
    FRAMEFree(patchi->frameinfo);
    patchi->frameinfo = NULL;
  }
#endif

  if(load_flag==UNLOAD){
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    if(patchi->boundary==1)UpdateBoundaryType();
    UpdateUnitDefs();
    UpdateTimes();
    PrintMemoryInfo;
    return 0;
  }
  if(patchi->skip == 1)return 0;

  char *ext = NULL;
  ext = strrchr(patchi->file, '.');
  if(ext != NULL && strcmp(ext, ".svz") == 0){
    patchi->is_compressed = 1;
    NewMemory((void **)&cvals_offsets, MAX_FRAMES * sizeof(int));
    NewMemory((void **)&cvals_sizes,   MAX_FRAMES * sizeof(int));
  }
  if(time_value != NULL){
    NewMemory((void **)&geom_offsets, MAX_FRAMES * sizeof(int));
    geom_offset_flag = BUILD_GEOM_OFFSETS;
  }
  else{
    geom_offset_flag = GET_GEOM_OFFSETS;
  }

  if(flag==1&&patchi->is_compressed==0){
    ntimes_local = GetGeomDataSizeFixed(patchi, &nvals, time_frame, geom_offsets, &geom_offset_flag, &max_buffer_size, &error);
  }
  else{
    ntimes_local = GetGeomDataSize(patchi->file, &nvals, time_frame, cvals_offsets, cvals_sizes, geom_offsets, &geom_offset_flag, &max_buffer_size, &error);
  }
  if(max_buffer_size > 0){
    NewMemory((void **)&cbuffer, max_buffer_size);
    patchi->cbuffer = cbuffer;
  }
  else{
    patchi->cbuffer = NULL;
  }
  patchi->cbuffer_size = max_buffer_size;
  if(time_value!=NULL){
    if(geom_offset_flag>0){
      ResizeMemory((void **)&geom_offsets, geom_offset_flag*sizeof(int));
      if(patchi!=NULL)patchi->geom_offsets = geom_offsets;
      if(slicei!=NULL)slicei->geom_offsets = geom_offsets;

    }
    else if(geom_offset_flag==0){
      FREEMEMORY(geom_offsets);
    }
    else{
      if(patchi!=NULL)geom_offsets = patchi->geom_offsets;
      if(slicei!=NULL)geom_offsets = slicei->geom_offsets;
    }
  }
  patchi->cvals_offsets = cvals_offsets;
  patchi->cvals_sizes   = cvals_sizes;


  if(ntimes_local>0){
    NewMemory((void **)&patchi->geom_nstatics,             ntimes_local*sizeof(int));
    NewMemory((void **)&patchi->geom_ndynamics,            ntimes_local*sizeof(int));
    NewMemory((void **)&patchi->geom_times,                ntimes_local*sizeof(float));
    NewMemory((void **)&patchi->geom_times_map,            ntimes_local*sizeof(unsigned char));
    NewMemory((void **)&patchi->geom_ivals_static_offset,  ntimes_local*sizeof(int));
    NewMemory((void **)&patchi->geom_ivals_dynamic_offset, ntimes_local*sizeof(int));
    NewMemory((void **)&patchi->geom_vals_static_offset,   ntimes_local*sizeof(int));
    NewMemory((void **)&patchi->geom_vals_dynamic_offset,  ntimes_local*sizeof(int));
  }
  if(nvals>0){
    if(patchi->is_compressed==1){
      patchi->is_compressed = 1;
      NewMemory((void **)&patchi->geom_vals, nvals*sizeof(char));
    }
    else{
      NewMemory((void **)&patchi->geom_vals, nvals * sizeof(float));
      NewMemory((void **)&patchi->geom_ivals, nvals * sizeof(char));
    }
  }
  int filesize;

  if(current_script_command==NULL||NOT_LOADRENDER){
    PRINTF("Loading %s(%s)", patchi->file, patchi->label.shortlabel);
  }
  filesize=GetGeomData(patchi, patchi->file, load_flag, ntimes_local, nvals, patchi->geom_times,
    patchi->geom_nstatics, patchi->geom_ndynamics, patchi->geom_vals, time_frame, time_value, geom_offsets, &error);
  MakeTimesMap(patchi->geom_times, &patchi->geom_times_map, ntimes_local);

  return_filesize += filesize;
  if(error == 1){
    patchi->loaded = 0;
    patchi->display = 0;
    return return_filesize;
  }

  if(FileExistsOrig(patchi->bound_file)==0){
    float *vals, valmin, valmax;

    vals = patchi->geom_vals;
    if(strcmp(patchi->label.shortlabel, "t_a") == 0){
      valmin = 1.0;
      valmax = 0.0;
      for(i = 0;i < nvals;i++){
        if(valmin > valmax){
          if(vals[i] < TOA_LIMIT){
            valmin = vals[i];
            valmax = valmin;
          }
        }
        else{
          valmin = MIN(vals[i], valmin);
          if(vals[i] < TOA_LIMIT)valmax = MAX(vals[i], valmax);
        }
      }
    }
    else{
      valmin = vals[0];
      valmax = valmin;
      for(i = 1;i < nvals;i++){
        valmin = MIN(vals[i], valmin);
        valmax = MAX(vals[i], valmax);
      }
    }
    WriteFileBounds(patchi->bound_file, valmin, valmax);
  }
  patchi->ngeom_times = ntimes_local;
  patchi->geom_nvals = nvals;
  patchi->geom_ivals_static_offset[0] = 0;
  patchi->geom_ivals_dynamic_offset[0] = patchi->geom_ivals_static_offset[0] + patchi->geom_nstatics[0];
  for(i = 1;i<ntimes_local;i++){
    patchi->geom_ivals_static_offset[i]  = patchi->geom_ivals_dynamic_offset[i-1] + patchi->geom_ndynamics[i-1];
    patchi->geom_ivals_dynamic_offset[i] = patchi->geom_ivals_static_offset[i]    + patchi->geom_nstatics[i];
  }

  patchi->geom_vals_static_offset[0]  = 0;
  patchi->geom_vals_dynamic_offset[0] = patchi->geom_vals_static_offset[0] + patchi->geom_nstatics[0];
  for(i = 1; i<ntimes_local; i++){
    patchi->geom_vals_static_offset[i]  = patchi->geom_vals_dynamic_offset[i-1] + patchi->geom_ndynamics[i-1];
    patchi->geom_vals_dynamic_offset[i] = patchi->geom_vals_static_offset[i]    + patchi->geom_nstatics[i];
  }

  patchi->loaded = 1;
  patchi->display = 1;

  if(slicei == NULL){
    if(colorlabelpatch != NULL){
      for(n = 0; n < MAXRGB; n++){
        FREEMEMORY(colorlabelpatch[n]);
      }
      FREEMEMORY(colorlabelpatch);
    }
    if(NewMemory((void **)&colorlabelpatch, MAXRGB * sizeof(char *)) == 0){
      ReadGeomData(patchi, NULL, UNLOAD, time_frame, time_value,  0, &error);
      return 0;
    }
    for(n = 0; n < MAXRGB; n++){
      colorlabelpatch[n] = NULL;
    }
    for(n = 0; n < global_scase.nrgb; n++){
      if(NewMemory((void **)&colorlabelpatch[n], 11) == 0){
        ReadGeomData(patchi, NULL, UNLOAD, time_frame, time_value, 0, &error);
        return 0;
      }
    }
    int set_valmin, set_valmax;
    float valmin, valmax;
    char *label;

    label = patchi->label.shortlabel;

    GLUIGetMinMax(BOUND_PATCH, label, &set_valmin, &valmin, &set_valmax, &valmax);
    int convert = 0;
    if(patchi->patch_filetype != PATCH_GEOMETRY_BOUNDARY && patchi->patch_filetype != PATCH_GEOMETRY_SLICE)convert = 0;
    GetBoundaryColors3(patchi, patchi->geom_vals, 0, patchi->geom_nvals, patchi->geom_ivals,
      &valmin, &valmax,
      global_scase.nrgb, colorlabelpatch, colorvaluespatch, boundarylevels256,
      &patchi->extreme_min, &patchi->extreme_max, convert);
    if(cache_boundary_data==0){
      FREEMEMORY(patchi->geom_vals);
    }
  }
  else{
    int slicetype;
    boundsdata *sb;
    float qmin, qmax;

    slicetype = GetSliceBoundsIndex(slicei);
    sb = slicebounds + slicetype;
    sb->label = &(slicei->label);

    HideSlices(slicei->label.longlabel);
    slicei->loaded = 1;
    slicei->display = 1;
    slicei->ntimes = patchi->ngeom_times;
    slicei->times = patchi->geom_times;

    UpdateLoadedLists();
    GetSliceDataBounds(slicei, &qmin, &qmax);
    slicei->globalmin_slice = qmin;
    slicei->globalmax_slice = qmax;
    slicei->valmin_slice    = qmin;
    slicei->valmax_slice    = qmax;
    if(slice_average_flag==1){
      int data_per_timestep, nvals2, ntimes;
      float *times, **qvalptrs;

      show_slice_average = 1;
      nvals2 = slicei->patchgeom->geom_nvals;
      times = patchi->geom_times;
      ntimes = patchi->ngeom_times;
      data_per_timestep = nvals2/ntimes;
#ifdef pp_SLICEFRAME
      qvalptrs = (float **)slicei->frameinfo->frameptrs;
#else
      NewMemory((void **)&qvalptrs, ntimes*sizeof(float *));
      for(i = 0; i < ntimes; i++){
        qvalptrs[i] = slicei->patchgeom->geom_vals + i*data_per_timestep;
      }
#endif
      if(TimeAverageData(qvalptrs, qvalptrs, nvals2, data_per_timestep, times, ntimes, slice_average_interval)==1){
        show_slice_average = 0;
      }
    }
    slicei->valmin_slice    = qmin;
    slicei->valmax_slice    = qmax;
    slicei->globalmin_slice = qmin;
    slicei->globalmax_slice = qmax;
    for(i = 0; i < 256; i++){
      slicei->qval256[i] = (qmin*(255 - i) + qmax*i) / 255;
    }
    UpdateSliceBounds();
    slicefile_labelindex = GetSliceBoundsIndexFromLabel(patchi->label.shortlabel);
    UpdateAllSliceColors(slicefile_labelindex, errorcode);
    list_slice_index = slicefile_labelindex;
    SliceBounds2Glui(slicefile_labelindex);

    GetSliceColors(patchi->geom_vals, patchi->geom_nvals, patchi->geom_ivals,
      glui_slicemin, glui_slicemax, nrgb_full, global_scase.nrgb,
      sb->colorlabels, sb->colorvalues, sb->levels256,
      &slicei->extreme_min, &slicei->extreme_max, 1
    );
  }
  if(patchi->boundary == 1){
    iboundarytype = GetBoundaryType(patchi);
  }
  else{
    slicefile_labelindex = GetSliceBoundsIndexFromLabel(patchi->label.shortlabel);
  }
#ifdef pp_RECOMPUTE_DEBUG
  int recompute = 0;
#endif
  if(current_script_command!=NULL||(slicei==NULL&&patchi->finalize==1)||(slicei!=NULL&&slicei->finalize==1)){
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    if(patchi->boundary==1)UpdateBoundaryType();
    cpp_boundsdata *bounds;
    int bound_type;

    if(patchi->boundary==1){
      bound_type = BOUND_PATCH;
      from_read_boundary = 1;
      int HaveBoundaryArrival(void);
      have_boundary_arrival = HaveBoundaryArrival();
    }
    else{
      bound_type = BOUND_SLICE;
    }

    bounds = GLUIGetBoundsData(bound_type);
    INIT_PRINT_TIMER(geom_bounds_timer);
    int bound_update = 0;
    if(force_bound_update == 1 || current_script_command != NULL)bound_update = 1;
    if(patchi->boundary == 1){
      if(bound_update==1||patch_bounds_defined==0 || BuildGbndFile(BOUND_PATCH) == 1){
        GetGlobalPatchBounds(1,DONOT_SET_MINMAX_FLAG);
        SetLoadedPatchBounds(NULL, 0);
        GLUIPatchBoundsCPP_CB(BOUND_DONTUPDATE_COLORS);
#ifdef pp_RECOMPUTE_DEBUG
        recompute = 1;
#endif
      }
    }
    else{
      if(bound_update==1||slice_bounds_defined==0|| BuildGbndFile(BOUND_SLICE) ==1){
        GetGlobalSliceBounds(1, DONOT_SET_MINMAX_FLAG);
        SetLoadedSliceBounds(NULL, 0);
#ifdef pp_RECOMPUTE_DEBUG
        recompute = 1;
#endif
      }
    }
    if(bounds->set_valmin==BOUND_SET_MIN||bounds->set_valmax==BOUND_SET_MAX){
      if(patchi->boundary==1){
      }
      else{
        int set_valmin, set_valmax;
        float valmin_dlg, valmax_dlg;

        GLUIGetMinMax(BOUND_SLICE, bounds->label, &set_valmin, &valmin_dlg, &set_valmax, &valmax_dlg);
      }
    }
    PRINT_TIMER(geom_bounds_timer, "update boundary bounds");

    INIT_PRINT_TIMER(geom_color_timer);
//*** don't think the following is needed
//    if(patchi->boundary==1){
//      GLUIPatchBoundsCPP_CB(BOUND_UPDATE_COLORS);
//    }
//    else{
//      GLUIHVACSliceBoundsCPP_CB(BOUND_UPDATE_COLORS);
//    }
    PRINT_TIMER(geom_color_timer, "update boundary colors");

    UpdateUnitDefs();
    UpdateTimes();
    force_redisplay = 1;
    UpdateFrameNumber(1);
  }
  stept = 1;
  force_redisplay = 1;
  updatemenu = 1;
  STOP_TIMER(total_time);
  if(current_script_command==NULL||NOT_LOADRENDER){
    PRINTF(" - %.1f MB/%.1f s\n", (float)return_filesize/1000000., total_time);
  }
  PrintMemoryInfo;
#ifdef pp_RECOMPUTE_DEBUG
  if(recompute == 1)printf("***recomputing bounds\n");
#endif
  return return_filesize;
}


/* ------------------ SetupReadAllGeom ------------------------ */

void SetupReadAllGeom(void){
  int i;

  for(i = 0; i<global_scase.ngeominfo; i++){
    geomdata *geomi;

    geomi = global_scase.geominfo+i;
    geomi->read_status = 0;
  }
  for(i = 0; i<global_scase.ncgeominfo; i++){
    geomdata *geomi;

    geomi = global_scase.cgeominfo+i;
    geomi->read_status = 0;
  }
}

/* ------------------ UpdateGeomTriangles ------------------------ */

void UpdateGeomTriangles(geomdata *geomi, int geom_type){
  geomlistdata *geomlisti;
  tridata **connected_triangles=NULL;
  int ntris, nverts, nconnected_triangles = 0;
  int j;

  if(geomi == NULL || geomi->display == 0 || geomi->loaded == 0)return;
  if(geom_type == GEOM_STATIC){
    geomlisti = geomi->geomlistinfo - 1;
  }
  else{
    geomlisti = geomi->geomlistinfo + geomi->itime;
  }

  // initialize

  ntris = geomlisti->ntriangles;
  nverts = geomlisti->nverts;

  for(j = 0; j < nverts; j++){
    vertdata *vert;

    vert = geomlisti->verts + j;
    vert->ntriangles = 0;
    vert->itriangle = 0;
  }

  // compute normal vector for each triangle

  for(j = 0; j < ntris; j++){
    tridata *trianglei;
    vertdata **verts;

    trianglei = geomlisti->triangles + j;
    verts = trianglei->verts;
    verts[0]->ntriangles++;
    verts[1]->ntriangles++;
    verts[2]->ntriangles++;
    GetTriangleNormal(verts[0]->xyz, verts[1]->xyz, verts[2]->xyz, trianglei->vert_norm, &trianglei->area);
  }

  // allocate memory for total number of connected triangles

  nconnected_triangles = 3 * ntris;
  if(nconnected_triangles > 0){
    NewMemory((void **)&connected_triangles, nconnected_triangles * sizeof(tridata *));
    geomlisti->connected_triangles = connected_triangles;
  }

  // associate assign triangle to each vertex

  for(j = 0; j < nverts; j++){
    vertdata *vert;

    vert = geomlisti->verts + j;
    vert->triangles = connected_triangles;
    connected_triangles += vert->ntriangles;
  }
  for(j = 0; j < ntris; j++){
    tridata *trianglei;
    vertdata **verts;

    trianglei = geomlisti->triangles + j;
    verts = trianglei->verts;
    verts[0]->triangles[verts[0]->itriangle++] = trianglei;
    verts[1]->triangles[verts[1]->itriangle++] = trianglei;
    verts[2]->triangles[verts[2]->itriangle++] = trianglei;
  }

  // average normals for each vertex

  for(j = 0; j < nverts; j++){
    vertdata *vert;
    float *vert_norm;

    vert = geomlisti->verts + j;
    vert_norm = vert->vert_norm;
    if(vert->ntriangles > 0){
      int k;

      vert_norm[0] = 0.0;
      vert_norm[1] = 0.0;
      vert_norm[2] = 0.0;
      for(k = 0; k < vert->ntriangles; k++){
        tridata *tri;

        tri = vert->triangles[k];
        vert_norm[0] += tri->area * tri->vert_norm[0];
        vert_norm[1] += tri->area * tri->vert_norm[1];
        vert_norm[2] += tri->area * tri->vert_norm[2];
      }
      ReduceToUnit(vert_norm);
    }
    else{
      vert_norm[0] = 0.0;
      vert_norm[1] = 0.0;
      vert_norm[2] = 1.0;
    }
  }
  geomlisti->norms_defined = 1;
}

/* ------------------ ReadAllGeom ------------------------ */

void *ReadAllGeom(void *arg){
  int i;

  for(i=0;i<global_scase.ngeominfo;i++){
    geomdata *geomi;

    geomi = global_scase.geominfo + i;
    THREADcontrol(readallgeom_threads, THREAD_LOCK);
    if(geomi->read_status!=0){
      THREADcontrol(readallgeom_threads, THREAD_UNLOCK);
      continue;
    }
    geomi->read_status = 1;
    THREADcontrol(readallgeom_threads, THREAD_UNLOCK);

    ReadGeom(geomi, NULL, 0, LOAD, GEOM_GEOM, NULL);
    THREADcontrol(readallgeom_threads, THREAD_LOCK);
    geomi->read_status = 2;
    THREADcontrol(readallgeom_threads, THREAD_UNLOCK);
  }
  for(i = 0; i<global_scase.ncgeominfo; i++){
    geomdata *geomi;

    geomi = global_scase.cgeominfo+i;
    THREADcontrol(readallgeom_threads, THREAD_LOCK);
    if(geomi->read_status!=0){
      THREADcontrol(readallgeom_threads, THREAD_UNLOCK);
      continue;
    }
    geomi->read_status = 1;
    THREADcontrol(readallgeom_threads, THREAD_UNLOCK);

    ReadGeom(geomi, NULL, 0, LOAD, GEOM_CGEOM, NULL);
    UpdateGeomTriangles(geomi, GEOM_STATIC);
    THREADcontrol(readallgeom_threads, THREAD_LOCK);
    geomi->read_status = 2;
    THREADcontrol(readallgeom_threads, THREAD_UNLOCK);
  }
  THREAD_EXIT(readallgeom_threads);
}

/* ------------------ UpdateAllGeomTriangles ------------------------ */

void UpdateAllGeomTriangles(void){
  int i;

  for(i = 0; i<global_scase.ngeominfo; i++){
    geomdata *geomi;

    geomi = global_scase.geominfo+i;
    UpdateGeomTriangles(geomi, GEOM_STATIC);
  }
}


/* ------------------ ReadGeom0 ------------------------ */

FILE_SIZE ReadGeom0(geomdata *geomi, int load_flag, int type, int *geom_frame_index){
  FILE_m *stream;
  int one=1;
  int ntimes_local;
  int version;
  int nvertfacesvolumes[3];
  int nfloat_vals, nint_vals;
  int iframe, icount;
  FILE_SIZE return_filesize;
  int count_read;

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
  unsigned char *filebuffer=NULL;
  int nfilebuffer=0;

  filebuffer  = NULL;
  nfilebuffer = 0;
#ifdef pp_ISOFRAME
  if(geomi->frameinfo != NULL && geomi->frameinfo->bufferinfo != NULL){
    filebuffer  = geomi->frameinfo->bufferinfo->buffer;
    nfilebuffer = geomi->frameinfo->bufferinfo->nbuffer;
  }
#endif
  // header
  // one
  // version
  // nfloats
  // nfloat vals
  // nints
  // nint vals


  stream = fopen_b(geomi->file, filebuffer, nfilebuffer, "rb");
  if(stream==NULL)return 0;

  FORTREAD_m(&one,4,1,stream);
  FORTREAD_m(&version, 4, 1, stream);
  return_filesize = 2*(4+4+4);

  FORTREAD_m(&nfloat_vals, 4, 1, stream);
  return_filesize += (4+4+4);

  if(nfloat_vals>0){
    fseek_m(stream, 4+nfloat_vals*4+4, SEEK_CUR);
    return_filesize += 4+nfloat_vals*4+4;
  }

  FORTREAD_m(&nint_vals, 4, 1, stream);
  return_filesize += (4+4+4);

  if(nint_vals>0){
    fseek_m(stream, 4+nint_vals*4+4, SEEK_CUR);
    return_filesize += 4+nint_vals*4+4;
  }

  geomi->ntimes=ntimes_local;
  geomi->itime=0;
  NewMemoryMemID((void **)&geomi->geomlistinfo_0,(ntimes_local+1)*sizeof(geomlistdata),geomi->memory_id);
  geomi->geomlistinfo=geomi->geomlistinfo_0+1;
  NewMemoryMemID((void **)&geomi->times,ntimes_local*sizeof(float),geomi->memory_id);

  // frame
  // time
  // nverts nfaces
  // xyz 3*nverts
  // ijk 3*ntris
  // surf_ind ntris
  icount=-1;
  for(iframe=-1;iframe<ntimes_local;){
    float times_local[2];
    geomlistdata *geomlisti;
    int nverts, ntris;
    int  skipframe;
    vertdata *verts=NULL;

    geomlisti = geomi->geomlistinfo+iframe;
    InitGeomlist(geomlisti);
    skipframe = 0;

    times_local[0] = 0.0;
    if(iframe>=0){
      FORTREAD_m(times_local, 4, 2, stream);
      if(count_read != 2)break;
      return_filesize += 4+4+4;

      icount++;
      if(geom_frame_index == NULL){
        if(use_tload_begin == 1 && times_local[0]     < global_scase.tload_begin)skipframe = 1;
        if(use_tload_end   == 1 && times_local[0]     > global_scase.tload_end)skipframe = 1;
        if(tload_step      >  1 && icount%tload_step != 0)skipframe = 1;
        if(skipframe == 0)geomi->times[iframe] = times_local[0];
      }
      else{
        if(iframe!=*geom_frame_index)skipframe = 1;
        geomi->times[iframe] = times_local[0];
        if(skipframe == 0)geomi->currentframe = geomlisti;
      }
    }
    FORTREAD_m(nvertfacesvolumes, 4, 2, stream);
    if(count_read != 2)break;
    return_filesize += (4+8+4);

    nverts=nvertfacesvolumes[0];
    ntris=nvertfacesvolumes[1];
    if(skipframe==1){
      int file_offset = 0;
      if(nverts>0)file_offset += 4+3*nverts*4+4;
      if(ntris>0)file_offset += (4+3*ntris*4+4)+(4+ntris*4+4);
      if(file_offset>0)fseek_m(stream, file_offset, SEEK_CUR);
    }
    if(skipframe==0&&nverts>0){
      int ii;
      float *xyz=NULL;
      float *zORIG;

#ifndef pp_ISOFRAME
      NewMemoryMemID((void **)&xyz,3*nverts*sizeof(float),    geomi->memory_id);
#endif
      NewMemoryMemID((void **)&verts,nverts*sizeof(vertdata), geomi->memory_id);
      NewMemoryMemID((void **)&zORIG, nverts*sizeof(float),   geomi->memory_id);
      geomlisti->zORIG = zORIG;
      geomlisti->verts = verts;
      geomlisti->nverts=nverts;

#ifdef pp_ISOFRAME
      FORTREAD_mv((void **)&xyz, 4, 3*nverts, stream);
#else
      FORTREAD_m(xyz, 4, 3*nverts, stream);
#endif
      if(count_read != 3 * nverts)break;
      return_filesize += 4+3*nverts*4+4;

      for(ii=0;ii<nverts;ii++){
        memcpy(verts[ii].xyz, xyz + 3*ii, 3*sizeof(float));
        zORIG[ii] = xyz[3 * ii+2];
      }
#ifndef pp_ISOFRAME
      FREEMEMORY(xyz);
#endif
    }
    if(verts!=NULL&&skipframe==0&&ntris>0){
      int *surf_ind=NULL,*ijk=NULL;
      int ii;
      int offset=0;
      tridata *triangles;

      NewMemoryMemID((void **)&triangles,ntris*sizeof(tridata), geomi->memory_id);
#ifndef pp_ISOFRAME
      NewMemoryMemID((void **)&ijk,3*ntris*sizeof(int),         geomi->memory_id);
      NewMemoryMemID((void **)&surf_ind,ntris*sizeof(int),      geomi->memory_id);
#endif
      geomlisti->triangles=triangles;
      geomlisti->ntriangles=ntris;

#ifdef pp_ISOFRAME
      FORTREAD_mv((void **)&ijk, 4, 3*ntris, stream);
#else
      FORTREAD_m(ijk, 4, 3*ntris, stream);
#endif
      if(count_read != 3 * ntris)break;
      return_filesize += 4+3*ntris*4+4;

#ifdef pp_ISOFRAME
      FORTREAD_mv((void **)&surf_ind, 4, ntris, stream);
#else
      FORTREAD_m(surf_ind, 4, ntris, stream);
#endif
      if(count_read != ntris)break;
      return_filesize += 4+ntris*4+4;

      if(type==GEOM_ISO)offset=global_scase.surfcoll.nsurfinfo;
      for(ii=0;ii<ntris;ii++){
        surfdata *surfi;

        triangles[ii].verts[0]=verts+ijk[3*ii]-1;
        triangles[ii].verts[1]=verts+ijk[3*ii+1]-1;
        triangles[ii].verts[2]=verts+ijk[3*ii+2]-1;

        surfi = global_scase.surfcoll.surfinfo+CLAMP(surf_ind[ii]+offset, global_scase.surfcoll.nsurfinfo+1, global_scase.surfcoll.nsurfinfo+MAX_ISO_COLORS);
        triangles[ii].geomsurf=surfi;
        if(geomi->file2_tris!=NULL){
          triangles[ii].geomobj = geomi->geomobjinfo + geomi->file2_tris[ii] - 1;
        }
        else{
          triangles[ii].geomobj = NULL;
        }
        surfi->used_by_geom = 1;
        triangles[ii].textureinfo=NULL;
      }
#ifndef pp_ISOFRAME
      FREEMEMORY(ijk);
      FREEMEMORY(surf_ind);
#endif
    }

    if(skipframe==0||geom_frame_index!=NULL){
      // add decimation code here
      iframe++;
    }
    if(geom_frame_index==NULL&&use_tload_end == 1 && (times_local[0] > global_scase.tload_end))break;
  }
  geomi->loaded = 1;
  geomi->display=1;
  fclose_b(stream);
  return return_filesize;
}

/* ------------------ InMesh ------------------------ */

int InMesh(float *xyz){
  int i;

  for(i = 0;i < global_scase.meshescoll.nmeshes;i++){
    meshdata *meshi;
    float *boxmin, *boxmax;

    meshi = global_scase.meshescoll.meshinfo + i;
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

FILE_SIZE ReadGeom2(geomdata *geomi, int load_flag, int type){
  FILE *stream;
  int one=1;
  int returncode=0;
  int ntimes_local;
  int i;
  vertdata *verts=NULL;
  tridata *triangles;
  int version;
  int nvertfacesvolumes[3];
  int nheaders[3], nfloat_vals, nint_vals, first_frame_all;
  FILE_SIZE return_filesize = 0;
  float *bounding_box;

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

  bounding_box = geomi->bounding_box;
  bounding_box[0] = 1.0;
  bounding_box[1] = 0.0;
  bounding_box[2] = 1.0;
  bounding_box[3] = 0.0;
  bounding_box[4] = 1.0;
  bounding_box[5] = 0.0;

  ReadGeomHeader(geomi,NULL,&ntimes_local);
  if(ntimes_local<0)return 0;
  stream = fopen(geomi->file,"rb");
  if(stream==NULL)return 0;

  FSEEK(stream,4,SEEK_CUR);fread(&one,4,1,stream);FSEEK(stream,4,SEEK_CUR);

  FORTREAD(&version,4,1,stream);
  return_filesize += 2*(4+4+4);

  FORTREAD(nheaders,4,3,stream);
  return_filesize += 4+3*4+4;

  nfloat_vals=nheaders[0];
  nint_vals=nheaders[1];
  first_frame_all=nheaders[2];

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

    if(first_frame_all==0&&i==-1)continue;

    FORTREADBR(&time_local,1,stream);
    return_filesize += 4+4+4;

    if(i>=0)geomi->times[i]=time_local;

    FORTREADBR(nvertfacesvolumes,3,stream);
    return_filesize += 4+3*4+4;

    nverts=nvertfacesvolumes[0];
    ntris=nvertfacesvolumes[1];
    nvolumes=nvertfacesvolumes[2];
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

#define XMIN 0
#define XMAX 1
#define YMIN 2
#define YMAX 3
#define ZMIN 4
#define ZMAX 5
      for(ii=0;ii<nverts;ii++){
        float *xyz_in, *xyz_out;

        xyz_in     = xyz+3*ii;
        xyz_out    = verts[ii].xyz;
        xyz_out[0] = xyz_in[0];
        xyz_out[1] = xyz_in[1];
        xyz_out[2] = xyz_in[2];
        zORIG[ii]  = xyz_in[2];
        if(bounding_box[XMIN]>bounding_box[XMAX]){
          bounding_box[XMIN] = xyz_in[0];
          bounding_box[XMAX] = xyz_in[0];
        }
        else{
          bounding_box[XMIN] = MIN(bounding_box[XMIN], xyz_in[0]);
          bounding_box[XMAX] = MAX(bounding_box[XMAX], xyz_in[0]);
        }
        if(bounding_box[YMIN]>bounding_box[YMAX]){
          bounding_box[YMIN] = xyz_in[1];
          bounding_box[YMAX] = xyz_in[1];
        }
        else{
          bounding_box[YMIN] = MIN(bounding_box[YMIN], xyz_in[1]);
          bounding_box[YMAX] = MAX(bounding_box[YMAX], xyz_in[1]);
        }
        if(bounding_box[ZMIN]>bounding_box[ZMAX]){
          bounding_box[ZMIN] = xyz_in[2];
          bounding_box[ZMAX] = xyz_in[2];
        }
        else{
          bounding_box[ZMIN] = MIN(bounding_box[ZMIN], xyz_in[2]);
          bounding_box[ZMAX] = MAX(bounding_box[ZMAX], xyz_in[2]);
        }
      }
      FREEMEMORY(xyz);
    }
    bounding_box[XMAX] = MAX(bounding_box[XMAX], bounding_box[XMIN]+0.001);
    bounding_box[YMAX] = MAX(bounding_box[YMAX], bounding_box[YMIN]+0.001);
    bounding_box[ZMAX] = MAX(bounding_box[ZMAX], bounding_box[ZMIN]+0.001);
    if(ntris>0){
      int *surf_ind=NULL,*ijk=NULL;
      int *locations=NULL, *geom_ind=NULL;
      float *texture_coords=NULL;
      int ii;

      NewMemoryMemID((void **)&triangles,ntris*sizeof(tridata),geomi->memory_id);
      NewMemory((void **)&ijk,3*ntris*sizeof(int));
      NewMemory((void **)&surf_ind,ntris*sizeof(int));
      if(geomi->geomtype==GEOM_CGEOM){
        NewMemory((void **)&locations, ntris*sizeof(int));
        NewMemory((void **)&geom_ind,  ntris*sizeof(int));
        texture_coords = NULL;
      }
      else{
        NewMemory((void **)&texture_coords,6*ntris*sizeof(float));
      }
      geomlisti->triangles  = triangles;
      geomlisti->ntriangles = ntris;
      if(geomi->geomtype==GEOM_CGEOM){
        FORTREADBR(ijk, 3*ntris, stream);
        return_filesize += 4+3*ntris*4+4;

        FORTREADBR(locations, ntris, stream);
        return_filesize += 4+ntris*4+4;

        FORTREADBR(surf_ind, ntris, stream);
        return_filesize += 4+ntris*4+4;

        FORTREADBR(geom_ind, ntris, stream);
        return_filesize += 4+ntris*4+4;
      }
      else{
        FORTREADBR(ijk, 3*ntris, stream);
        return_filesize += 4+3*ntris*4+4;

        FORTREADBR(surf_ind, ntris, stream);
        return_filesize += 4+ntris*4+4;

        FORTREADBR(texture_coords, 6*ntris, stream);
        return_filesize += 4+6*ntris*4+4;
      }

      // compute texture coordinates

      if(verts!=NULL&&geomi->is_terrain==1){
        float xmin, xmax, ymin, ymax, zmin, zmax;

        xmin = verts[0].xyz[0];
        xmax = xmin;
        ymin = verts[0].xyz[1];
        ymax = ymin;
        zmin = verts[0].xyz[2];
        zmax = zmin;

        for(ii=1;ii<nverts;ii++){
          float *xyz;

          xyz = verts[ii].xyz;
          xmin = MIN(xmin,xyz[0]);
          xmax = MAX(xmax,xyz[0]);
          ymin = MIN(ymin,xyz[1]);
          ymax = MAX(ymax,xyz[1]);
          zmin = MIN(zmin,xyz[2]);
          zmax = MAX(zmax,xyz[2]);
        }
        if(zmax>zmin&&xmax>xmin){
          float xratio, yratio, zratio;

          xratio = (xmax-xmin)/(xbarORIG-xbar0ORIG);
          yratio = (ymax-ymin)/(ybarORIG-ybar0ORIG);
          zratio = (zmax-zmin)/(zbarORIG-zbar0ORIG);

          geomyfactor = MAX(xratio,zratio);
          geomyfactor = MAX(1.0,geomyfactor);

          geomzfactor = MAX(xratio, yratio);
          geomzfactor = MAX(1.0, geomzfactor);

          geom_xmin = xmin;
          geom_xmax = xmax;
          geom_ymin = ymin;
          geom_ymax = ymax;
          geom_zmin = zmin;
          geom_zmax = zmax;
          have_geom_factors = 1;
        }

        if(texture_coords!=NULL&&verts!=NULL&&global_scase.terrain_texture_coll.terrain_textures!=NULL){
          float xfactor, yfactor;

          xfactor = 1.0;
          yfactor = 1.0;
          if(ABS(xmax-xmin)>0.0001)xfactor = 1.0/(xmax-xmin);
          if(ABS(ymax-ymin)>0.0001)yfactor = 1.0/(ymax-ymin);
          for(ii=0;ii<ntris;ii++){
            float *text_coords;
            int *tri_ind;
            float *xy;
            vertdata *vert;

            text_coords = texture_coords + 6*ii;
            tri_ind = ijk + 3*ii;

            vert = verts+tri_ind[0]-1;
            xy = vert->xyz;
            text_coords[0] = (xy[0]-xmin)*xfactor;
            text_coords[1] = (xy[1]-ymin)*yfactor;

            vert = verts+tri_ind[1]-1;
            xy = vert->xyz;
            text_coords[2] = (xy[0]-xmin)*xfactor;
            text_coords[3] = (xy[1]-ymin)*yfactor;

            vert = verts+tri_ind[2]-1;
            xy = vert->xyz;
            text_coords[4] = (xy[0]-xmin)*xfactor;
            text_coords[5] = (xy[1]-ymin)*yfactor;
          }
        }
      }
      else if(verts!=NULL&&geomi->geomtype!=GEOM_CGEOM&&geomi->geomobjinfo!=NULL&&geomi->geomobjinfo->texture_mapping==TEXTURE_SPHERICAL){
        for(ii = 0; ii<ntris; ii++){
          float *text_coords;
          int *tri_ind;
          float *xy;
          vertdata *vert;

#define XYZ2AZ(x,y)     CLAMP(((atan2((y),(x))+PI)/(2.0*PI)), 0.0, 1.0)
#define XYZ2ELEV(x,y,z) CLAMP(( (PI/2.0+atan2( (z), sqrt( (x)*(x)+(y)*(y) ) )) /PI ), 0.0, 1.0)

          text_coords = texture_coords+6*ii;
          tri_ind = ijk+3*ii;

          vert = verts+tri_ind[0]-1;
          xy = vert->xyz;
          text_coords[0] = XYZ2AZ(xy[0],xy[1]);
          text_coords[1] = XYZ2ELEV(xy[0], xy[1], xy[2]);

          vert = verts+tri_ind[1]-1;
          xy = vert->xyz;
          text_coords[2] = XYZ2AZ(xy[0], xy[1]);
          text_coords[3] = XYZ2ELEV(xy[0], xy[1], xy[2]);

          vert = verts+tri_ind[2]-1;
          xy = vert->xyz;
          text_coords[4] = XYZ2AZ(xy[0], xy[1]);
          text_coords[5] = XYZ2ELEV(xy[0], xy[1], xy[2]);
        }
      }
      else if(verts!=NULL&&geomi->geomtype!=GEOM_CGEOM&&geomi->geomobjinfo!=NULL&&geomi->geomobjinfo->texture_mapping==TEXTURE_RECTANGULAR){
        for(ii = 0; ii<ntris; ii++){
          float *text_coords;
          int *tri_ind;
          float *xy;
          vertdata *vert;

#define XYZ2X(x) ((x)-bounding_box[XMIN])/(bounding_box[XMAX]-bounding_box[XMIN])
#define XYZ2Y(y) ((y)-bounding_box[YMIN])/(bounding_box[YMAX]-bounding_box[YMIN])
          text_coords = texture_coords+6*ii;
          tri_ind = ijk+3*ii;

          vert = verts+tri_ind[0]-1;
          xy = vert->xyz;
          text_coords[0] = XYZ2X(xy[0]);
          text_coords[1] = XYZ2Y(xy[1]);

          vert = verts+tri_ind[1]-1;
          xy = vert->xyz;
          text_coords[2] = XYZ2X(xy[0]);
          text_coords[3] = XYZ2Y(xy[1]);

          vert = verts+tri_ind[2]-1;
          xy = vert->xyz;
          text_coords[4] = XYZ2X(xy[0]);
          text_coords[5] = XYZ2Y(xy[1]);
        }
      }

      CheckMemory;
     // assert(geomi->ngeomobj_offsets<=0 || ntris==geomi->ngeomobj_offsets);
      if(texture_coords!=NULL&&verts != NULL){
        for(ii = 0;ii < ntris;ii++){
          surfdata *surfi;
          int k;

          for(k = 0;k < 3;k++){
            triangles[ii].verts[k] = verts + ijk[3 * ii + k] - 1;
          }

          if(geomi->geomtype != GEOM_CGEOM){
            for(k = 0;k < 6;k++){
              triangles[ii].tverts[k] = texture_coords[6 * ii + k];
            }
          }
          GetTriangleNormal(triangles[ii].verts[0]->xyz, triangles[ii].verts[1]->xyz, triangles[ii].verts[2]->xyz,
            triangles[ii].tri_norm, NULL);

          CheckMemory;
          triangles[ii].geomtype = type;
          switch(type){
          case GEOM_CGEOM:
            surfi = global_scase.surfcoll.surfinfo + CLAMP(surf_ind[ii], 0, global_scase.surfcoll.nsurfinfo - 1);
            triangles[ii].insolid = locations[ii];
            triangles[ii].geomobj = global_scase.geominfo->geomobjinfo + geom_ind[ii] - 1;
            break;
          case GEOM_GEOM:
          case GEOM_ISO:
            surfi = global_scase.surfcoll.surfinfo + CLAMP(surf_ind[ii], 0, global_scase.surfcoll.nsurfinfo - 1);
            if(type == GEOM_ISO)surfi += global_scase.surfcoll.nsurfinfo;
            triangles[ii].insolid = surf_ind[ii];
            if(geomi->file2_tris != NULL){
              triangles[ii].geomobj = geomi->geomobjinfo + geomi->file2_tris[ii] - 1;
            }
            else{
              if(geomi->ngeomobj_offsets > 0 && ii < geomi->ngeomobj_offsets && geomi->geomobj_offsets != NULL){
                triangles[ii].geomobj = geomi->geomobjinfo + geomi->geomobj_offsets[ii];
              }
              else{
                triangles[ii].geomobj = geomi->geomobjinfo;
              }
            }
            break;
          case GEOM_SLICE:
          case GEOM_BOUNDARY:
            surfi = global_scase.surfcoll.surfinfo;
            triangles[ii].insolid = 0;
            break;
          default:
            assert(FFALSE);
            break;
          }
          if(geomi->geomtype == GEOM_GEOM)surfi->used_by_geom = 1;
          triangles[ii].geomsurf = surfi;
          if(terrain_textures != NULL && geomi->is_terrain == 1){
            triangles[ii].textureinfo = terrain_textures;
          }
          else{
            triangles[ii].textureinfo = surfi->textureinfo;
          }
          triangles[ii].outside_domain = OutSideDomain(triangles[ii].verts);
        }
      }
      FREEMEMORY(ijk);
      FREEMEMORY(surf_ind);
      FREEMEMORY(texture_coords);
    }
    if(nvolumes>0){
      FSEEK(stream, 4 + 4*nvolumes*sizeof(int) + 4, SEEK_CUR);
      FSEEK(stream, 4 + nvolumes*sizeof(int) + 4,   SEEK_CUR);
    }
    if(geomi->have_cface_normals==CFACE_NORMALS_YES){
      int ncface_normals;

      FORTREADBR(&ncface_normals, 1, stream);
      return_filesize += 4+1*4+4;
      if(ncface_normals>0){
        float *cface_normals;

        NewMemory((void **)&cface_normals, 6*ncface_normals*sizeof(float));
        geomi->cface_normals = cface_normals;

        FORTREADBR(cface_normals, 6*ncface_normals, stream);
        return_filesize += 4+6*ncface_normals*4+4;
      }
      geomi->ncface_normals = ncface_normals;
    }
  }
  geomi->loaded=1;
  geomi->display=1;
  stept = 1;
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

/* ------------------ CancelUpdateTriangles ------------------------ */

void CancelUpdateTriangles(void){
  cancel_update_triangles = 1;
  THREADcontrol(triangles_threads, THREAD_JOIN);
  cancel_update_triangles = 0;
}

/* ------------------ ReadGeom ------------------------ */

FILE_SIZE ReadGeom(geomdata *geomi, unsigned char *buffer, int nbuffer, int load_flag, int type, int *geom_frame_index){
  FILE_m *stream;
  int version;
  int one=0;
  int count_read;
  FILE_SIZE return_filesize=0;

  if(geomi->file == NULL && buffer==NULL)return 0;
  stream = fopen_b(geomi->file, buffer, nbuffer, "rb");
  if(stream == NULL)return 0;
  FORTREAD_m(&one,4,1,stream);
  FORTREAD_m(&version, 4, 1, stream);
  if(count_read!=1)fclose_b(stream);
  fclose_b(stream);
  return_filesize = 2*(4+4+4);

  if(version<=1){
    return_filesize+=ReadGeom0(geomi,load_flag,type,geom_frame_index);
  }
  else{
    return_filesize += ReadGeom2(geomi,load_flag,type);
  }
  PrintMemoryInfo;
  return return_filesize;
}

/* ------------------ UpdatePatchGeomTriangles ------------------------ */

void UpdatePatchGeomTriangles(patchdata *patchi, int geom_type){
  geomdata *geomi;

  if(patchi->patch_filetype!=PATCH_GEOMETRY_BOUNDARY)return;
  geomi = patchi->geominfo;
  UpdateGeomTriangles(geomi, geom_type);
}

/* ------------------ Tri2Verts ------------------------ */

void Tri2Verts(tridata *triangles, int ntriangles, vertdata *verts, int nverts){
  int i;

  for(i = 0; i<nverts; i++){
    vertdata *verti;

    verti = verts+i;
    verti->triangle1 = NULL;
  }
  for(i = 0; i<ntriangles; i++){
    tridata *trii;
    vertdata *vert1, *vert2, *vert3;

    trii = triangles+i;
    vert1 = verts+trii->vert_index[0];
    vert2 = verts+trii->vert_index[1];
    vert3 = verts+trii->vert_index[2];
    vert1->triangle1 = trii;
    vert2->triangle1 = trii;
    vert3->triangle1 = trii;
  }
}

/* ------------------ DrawGeomVData ------------------------ */

void DrawGeomVData(vslicedata *vd){
  patchdata *patchi, *patchu=NULL, *patchv=NULL, *patchw=NULL;
  unsigned char *ivals;
  int i, geom_type=GEOM_STATIC;
  int cell_center, nvals;
  int vert2tri;

  patchi = vd->val->patchgeom;
  vert2tri = patchi->geom_vert2tri;
  cell_center = vd->val->cell_center;
  ivals = patchi->geom_ival_static;
  if(vd->u!=NULL)patchu = vd->u->patchgeom;
  if(vd->v!=NULL)patchv = vd->v->patchgeom;
  if(vd->w!=NULL)patchw = vd->w->patchgeom;
  if(
    (patchi->patch_filetype==PATCH_GEOMETRY_SLICE&&(
    show_vector_slice[IN_CUTCELL_GLUI]==1||
    show_vector_slice[IN_SOLID_GLUI]==1||
    show_vector_slice[IN_GAS_GLUI]==1))
    ){
    for(i = 0; i<1; i++){
      geomdata *geomi;
      geomlistdata *geomlisti;
      int nverts, ntris, j;

      geomi = patchi->geominfo;
      if(geomi==NULL||geomi->display==0||geomi->loaded==0)continue;
      if(geom_type==GEOM_STATIC){
        geomlisti = geomi->geomlistinfo-1;
      }
      else{
        geomlisti = geomi->geomlistinfo+geomi->itime;
      }
      if(cell_center==0&&vert2tri==0){
        patchi->geom_vert2tri = 1;
        Tri2Verts(geomlisti->triangles, geomlisti->ntriangles, geomlisti->verts, geomlisti->nverts);
      }

      ntris = geomlisti->ntriangles;
      nverts = geomlisti->nverts;
      if(cell_center==1){ // set according to whether vector slice is cell centered or node centered
        nvals = ntris;
      }
      else{
        nvals = nverts;
      }
      if(ntris==0||nverts==0)continue;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      if(global_scase.auto_terrain==1)glTranslatef(0.0, 0.0, SCALE2FDS(0.01));

      glLineWidth(vectorlinewidth);
      glBegin(GL_LINES);
      for(j = 0; j<nvals; j+=vectorskip){
        float *xyz1, *xyz2, *xyz3, xyz[3], *xyzptr;
        int color_index;
        float *color;
        tridata *trianglei;
        vertdata *verti;
        int insolid;
        float du, dv, dw;

        insolid = -1;
        if(cell_center==1){
          trianglei = geomlisti->triangles+j;
          insolid = trianglei->insolid&3;
        }
        else{
          verti = geomlisti->verts+j;
          if(verti->triangle1!=NULL)insolid = verti->triangle1->insolid&3;
        }
        if(insolid>=0){
          if(insolid==IN_CUTCELL&&show_vector_slice[IN_CUTCELL_GLUI]==0)continue;
          if(insolid==IN_SOLID  &&show_vector_slice[IN_SOLID_GLUI]==0)continue;
          if(insolid==IN_GAS    &&show_vector_slice[IN_GAS_GLUI]==0)continue;
        }

        color = NULL;
        if((cell_center==1&&show_cell_slices_and_vectors==1)||(cell_center==0&&show_node_slices_and_vectors==1)){
          if(insolid==IN_CUTCELL   &&show_slice_shaded[IN_CUTCELL_GLUI]==1)color = foregroundcolor;
          if(insolid==IN_SOLID_GLUI&&show_slice_shaded[IN_SOLID_GLUI]==1)color = foregroundcolor;
          if(insolid==IN_GAS_GLUI  &&show_slice_shaded[IN_GAS_GLUI]==1)color = foregroundcolor;
        }
        if(color==NULL){
          color_index = ivals[j];
          color = rgb_patch+4*color_index;
        }
        if(cell_center==1){
          xyz1 = trianglei->verts[0]->xyz;
          xyz2 = trianglei->verts[1]->xyz;
          xyz3 = trianglei->verts[2]->xyz;
          xyz[0] = (xyz1[0]+xyz2[0]+xyz3[0])/3.0;
          xyz[1] = (xyz1[1]+xyz2[1]+xyz3[1])/3.0;
          xyz[2] = (xyz1[2]+xyz2[2]+xyz3[2])/3.0;
          xyzptr = xyz;
        }
        else{
          xyzptr = verti->xyz;
        }

#define GET_VEC_GEOM_DXYZ(PATCHU,DU,n)                  \
         if(PATCHU==NULL||PATCHU->geom_ival_static==NULL){                              \
           DU=0.0;                                 \
         }                                         \
         else{                                     \
           DU = PATCHU->geom_val_static[(n)];                    \
         }

#define ADJUST_VEC_DXYZ(dx,dy,dz)                       \
         if(vec_uniform_length==1){                              \
           float vecnorm; \
           vecnorm = sqrt(dx*dx+dy*dy+dz*dz);\
           if(vecnorm==0.0)vecnorm=1.0;\
           dx *= vecfactor*0.05/(max_velocity*vecnorm);\
           dy *= vecfactor*0.05/(max_velocity*vecnorm);\
           dz *= vecfactor*0.05/(max_velocity*vecnorm);\
         }                                         \
         else{                                     \
           dx *= 0.05*vecfactor/max_velocity;\
           dy *= 0.05*vecfactor/max_velocity;\
           dz *= 0.05*vecfactor/max_velocity;\
         }                                         \

        GET_VEC_GEOM_DXYZ(patchu, du, j);
        GET_VEC_GEOM_DXYZ(patchv, dv, j);
        GET_VEC_GEOM_DXYZ(patchw, dw, j);
        ADJUST_VEC_DXYZ(du, dv, dw);

        glColor3f(color[0], color[1], color[2]);
        glVertex3f(xyzptr[0]-du/2.0, xyzptr[1]-dv/2.0, xyzptr[2]-dw/2.0);
        glVertex3f(xyzptr[0]+du/2.0, xyzptr[1]+dv/2.0, xyzptr[2]+dw/2.0);
      }
      glEnd();

      glPointSize(vectorpointsize);
      glBegin(GL_POINTS);
      for(j = 0; j<nvals; j+=vectorskip){
        float *xyz1, *xyz2, *xyz3, xyz[3], *xyzptr;
        int color_index;
        float *color;
        tridata *trianglei;
        vertdata *verti;
        int insolid;
        float du, dv, dw;

        insolid = -1;
        if(cell_center==1){
          trianglei = geomlisti->triangles+j;
          insolid = trianglei->insolid&3;
        }
        else{
          verti = geomlisti->verts+j;
          if(verti->triangle1!=NULL)insolid = verti->triangle1->insolid&3;
        }
        if(insolid>=0){
          if(insolid==IN_CUTCELL&&show_vector_slice[IN_CUTCELL_GLUI]==0)continue;
          if(insolid==IN_SOLID  &&show_vector_slice[IN_SOLID_GLUI]==0)continue;
          if(insolid==IN_GAS    &&show_vector_slice[IN_GAS_GLUI]==0)continue;
        }

        color = NULL;
        if((cell_center==1&&show_cell_slices_and_vectors==1)||(cell_center==0&&show_node_slices_and_vectors==1)){
          if(insolid==IN_CUTCELL   &&show_slice_shaded[IN_CUTCELL_GLUI]==1)color = foregroundcolor;
          if(insolid==IN_SOLID_GLUI&&show_slice_shaded[IN_SOLID_GLUI]==1)color = foregroundcolor;
          if(insolid==IN_GAS_GLUI  &&show_slice_shaded[IN_GAS_GLUI]==1)color = foregroundcolor;
        }
        if(color==NULL){
          color_index = ivals[j];
          color = rgb_patch+4*color_index;
        }
        if(cell_center==1){
          xyz1 = trianglei->verts[0]->xyz;
          xyz2 = trianglei->verts[1]->xyz;
          xyz3 = trianglei->verts[2]->xyz;
          xyz[0] = (xyz1[0]+xyz2[0]+xyz3[0])/3.0;
          xyz[1] = (xyz1[1]+xyz2[1]+xyz3[1])/3.0;
          xyz[2] = (xyz1[2]+xyz2[2]+xyz3[2])/3.0;
          xyzptr = xyz;
        }
        else{
          xyzptr = verti->xyz;
        }

        GET_VEC_GEOM_DXYZ(patchu, du, j);
        GET_VEC_GEOM_DXYZ(patchv, dv, j);
        GET_VEC_GEOM_DXYZ(patchw, dw, j);
        ADJUST_VEC_DXYZ(du, dv, dw);

        glColor3f(color[0], color[1], color[2]);
        glVertex3f(xyzptr[0]+du/2.0, xyzptr[1]+dv/2.0, xyzptr[2]+dw/2.0);
      }
      glEnd();
      glPopMatrix();
    }
  }
}

  /* ------------------ DrawGeomData ------------------------ */

#define GEOMVAL(index) ( patchi->is_compressed==0 ? vals[(index)] : (float)cvals[(index)] )
#define GEOMTEXTURE(index, vmin, vmax) ( \
        CLAMP( (vals[(index)]-vmin)/(vmax-vmin),0.0,1.0) \
        )

//#define GEOMTEXTURE(index, vmin, vmax) ( \
//        patchi->is_compressed==0 ? \
//        CLAMP( (vals[(index)]-vmin)/(vmax-vmin),0.0,1.0) : \
//        CLAMP( (float)cvals[(index)]/255.0,0.0,1.0) \
//        )
#define GEOMTEXTURE2(geomval, vmin, vmax) ( \
        patchi->is_compressed==0 ? \
        CLAMP( (geomval-vmin)/(vmax-vmin),0.0,1.0) : \
        CLAMP( (float)geomval/255.0,0.0,1.0) \
        )

void DrawGeomData(int flag, slicedata *sd, patchdata *patchi, int geom_type){
  int i;
  unsigned char *ivals, *cvals;
  int is_ccell = 0;
  float *vals=NULL;

  int set_valmin, set_valmax;
  char *label;
  float ttmin, ttmax;

  label = patchi->label.shortlabel;
  GLUIGetOnlyMinMax(BOUND_PATCH, label, &set_valmin, &ttmin, &set_valmax, &ttmax);

  float rvals[3];
  float valmin, valmax;
  if(sd!=NULL){
    valmin = sd->valmin_slice;
    valmax = sd->valmax_slice;
    if(valmin>=valmax){
      valmin = 0.0;
      valmax = 1.0;
    }
  }

  if(strcmp(patchi->label.shortlabel, "ccell")==0)is_ccell = 1;
  if(geom_type==GEOM_STATIC){
    vals = patchi->geom_vals + patchi->geom_vals_static_offset[patchi->geom_itime];
    cvals = patchi->cbuffer;
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
      int ntris, j, enable_lighting;

      geomi = patchi->geominfo;
      if(geomi == NULL || geomi->display == 0 || geomi->loaded == 0)continue;
      if(geom_type == GEOM_STATIC){
        geomlisti = geomi->geomlistinfo - 1;
      }
      else{
        geomlisti = geomi->geomlistinfo + geomi->itime;
      }
      if(patchi->patch_filetype==PATCH_GEOMETRY_BOUNDARY){
        enable_lighting = 1;
      }
      else{
        enable_lighting = 0;
      }
      if(geomlisti->norms_defined==0&&enable_lighting==1){
        UpdatePatchGeomTriangles(patchi, geom_type);
      }
      tridata *triangles;
      int is_time_arrival = 0;

      if(strcmp(patchi->label.shortlabel, "t_a") == 0)is_time_arrival = 1;
      triangles = geomlisti->triangles;
      ntris     = geomlisti->ntriangles;
      if(ntris == 0)continue;

      if(is_ccell==0&&flag == DRAW_TRANSPARENT&&use_transparency_data == 1 && patchi->patch_filetype == PATCH_GEOMETRY_SLICE)TransparentOn();

      glEnable(GL_NORMALIZE);
      glShadeModel(GL_SMOOTH);
      if(enable_lighting==1){
        ENABLE_LIGHTING;
      }
      else{
        DISABLE_LIGHTING;
      }
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glEnable(GL_TEXTURE_1D);
      glBindTexture(GL_TEXTURE_1D, texture_slice_colorbar_id);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,            iso_specular);
      glMaterialf(GL_FRONT_AND_BACK,  GL_SHININESS,           iso_shininess);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, block_ambient2);
      glEnable(GL_COLOR_MATERIAL);

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0+boundaryoffset);
      if(global_scase.auto_terrain==1)glTranslatef(0.0, 0.0, slice_dz);
      glBegin(GL_TRIANGLES);
      if(
        vals!=NULL &&
       ( (patchi->patch_filetype!=PATCH_GEOMETRY_BOUNDARY&&smooth_iso_normal == 0) ||
         patchi->patch_filetype==PATCH_GEOMETRY_BOUNDARY)

       ){
        for(j = 0; j < ntris; j++){
          float *xyzptr[3];
          tridata *trianglei;
          int ival;

          trianglei = triangles + j;
          if(patchi->patch_filetype==PATCH_GEOMETRY_BOUNDARY){
            rvals[0] = GEOMVAL(j);
            if(is_time_arrival == 1 && rvals[0] > TOA_LIMIT)continue;
            rvals[0] = GEOMTEXTURE2(rvals[0], ttmin, ttmax);
            ival = CLAMP(rvals[0] * 255.0, 0, 255);
            if(rgb_patch[4*ival + 3] == 0.0)continue;
          }
          else if(patchi->patch_filetype==PATCH_GEOMETRY_SLICE){
            rvals[0] = GEOMTEXTURE(j, valmin, valmax);
            ival = CLAMP(rvals[0] * 255.0, 0, 255);
            if(rgb_slice[4*ival + 3] == 0.0)continue;
          }
          else{
            rvals[0] = (float)ivals[j]/255.0;
          }
          rvals[1] = rvals[0];
          rvals[2] = rvals[0];
          if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            int insolid;

            insolid = trianglei->insolid & 3;
            if(insolid == IN_CUTCELL && show_slice_shaded[IN_CUTCELL_GLUI] == 0)continue;
            if(insolid == IN_SOLID   && show_slice_shaded[IN_SOLID_GLUI] == 0)continue;
            if(insolid == IN_GAS     && show_slice_shaded[IN_GAS_GLUI] == 0)continue;
          }
          else if(trianglei->geomtype == GEOM_BOUNDARY){
            if(show_boundary_shaded == 0)continue;
          }

          xyzptr[0] = trianglei->verts[0]->xyz;
          xyzptr[1] = trianglei->verts[1]->xyz;
          xyzptr[2] = trianglei->verts[2]->xyz;

          if(lighting_on==1)glNormal3fv(trianglei->tri_norm);
          glTexCoord1f(rvals[0]);
          glVertex3fv(xyzptr[0]);

          glTexCoord1f(rvals[1]);
          glVertex3fv(xyzptr[1]);

          glTexCoord1f(rvals[2]);
          glVertex3fv(xyzptr[2]);

          if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            glTexCoord1f(rvals[0]);
            glVertex3fv(xyzptr[0]);

            glTexCoord1f(rvals[2]);
            glVertex3fv(xyzptr[2]);

            glTexCoord1f(rvals[1]);
            glVertex3fv(xyzptr[1]);
          }
        }
      }
      else{
        for(j = 0; j < ntris; j++){
          float *xyzptr[3];
          float *xyznorm[3];
          tridata *trianglei;

          trianglei = triangles+j;

          if(patchi->structured == NO&&patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            int insolid;

            insolid = trianglei->insolid & 3;
            if(insolid == IN_CUTCELL && show_slice_shaded[IN_CUTCELL_GLUI] == 0)continue;
            if(insolid == IN_SOLID   && show_slice_shaded[IN_SOLID_GLUI] == 0)continue;
            if(insolid == IN_GAS     && show_slice_shaded[IN_GAS_GLUI] == 0)continue;
          }
          else if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY){
            if(show_boundary_shaded == 0)continue;
          }

          if(sd==NULL||sd->cell_center==1){
            if(sd!=NULL){
              rvals[0] = GEOMTEXTURE(j, valmin, valmax);
            }
            else{
              rvals[0] = (float)ivals[j]/255.0;
            }
            rvals[1] = rvals[0];
            rvals[2] = rvals[0];
          }
          else{
            rvals[0] = GEOMTEXTURE(trianglei->vert_index[0], valmin, valmax);
            rvals[1] = GEOMTEXTURE(trianglei->vert_index[1], valmin, valmax);
            rvals[2] = GEOMTEXTURE(trianglei->vert_index[2], valmin, valmax);
          }

          xyzptr[0] = trianglei->verts[0]->xyz;
          xyzptr[1] = trianglei->verts[1]->xyz;
          xyzptr[2] = trianglei->verts[2]->xyz;

          xyznorm[0] = trianglei->verts[0]->vert_norm;
          xyznorm[1] = trianglei->verts[1]->vert_norm;
          xyznorm[2] = trianglei->verts[2]->vert_norm;

          if(lighting_on==1)glNormal3fv(xyznorm[0]);
          glTexCoord1f(rvals[0]);
          glVertex3fv(xyzptr[0]);

          if(lighting_on==1)glNormal3fv(xyznorm[1]);
          glTexCoord1f(rvals[1]);
          glVertex3fv(xyzptr[1]);

          if(lighting_on==1)glNormal3fv(xyznorm[2]);
          glTexCoord1f(rvals[2]);
          glVertex3fv(xyzptr[2]);

          if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            glTexCoord1f(rvals[0]);
            glVertex3fv(xyzptr[0]);

            glTexCoord1f(rvals[2]);
            glVertex3fv(xyzptr[2]);

            glTexCoord1f(rvals[1]);
            glVertex3fv(xyzptr[1]);
          }
        }
      }
      glEnd();
      glPopMatrix();
      glDisable(GL_COLOR_MATERIAL);
      if(enable_lighting==1){
        DISABLE_LIGHTING;
      }
      glDisable(GL_TEXTURE_1D);
      if(is_ccell==0&&flag == DRAW_TRANSPARENT&&use_transparency_data == 1 && patchi->patch_filetype == PATCH_GEOMETRY_SLICE)TransparentOff();
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
      tridata *triangles;

      triangles = geomlisti->triangles;
      ntris = geomlisti->ntriangles;
      if(ntris == 0)continue;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY){
        glLineWidth(geomboundary_linewidth);
      }
      else if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
        glLineWidth(vectorlinewidth);
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
          float *color0, *color1, *color2;

          trianglei = triangles + j;
          if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
            int insolid, insolid_glui=-1;

            insolid = trianglei->insolid & 3;
            if(insolid>=0&&insolid<3)insolid_glui = insolid;
            if(insolid == IN_CUTCELL && show_slice_outlines[IN_CUTCELL_GLUI] == 0)continue;
            if(insolid == IN_SOLID   && show_slice_outlines[IN_SOLID_GLUI] == 0)continue;
            if(insolid == IN_GAS     && show_slice_outlines[IN_GAS_GLUI] == 0)continue;

            if(insolid_glui!=-1&&slice_edgetypes[insolid_glui] == OUTLINE_POLYGON){
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
            int insolid, insolid_glui = -1;

            insolid = trianglei->insolid & 3;
            if(insolid>=0&&insolid<3)insolid_glui = insolid;
            if(insolid_glui!=-1&&boundary_edgetype==OUTLINE_POLYGON){
              int insolid4, insolid8, insolid16;

              insolid4 = trianglei->insolid&4;
              if(insolid4  ==  4)show_edge1 = 0;

              insolid8 = trianglei->insolid&8;
              if(insolid8  ==  8)show_edge2 = 0;

              insolid16 = trianglei->insolid&16;
              if(insolid16 == 16)show_edge3 = 0;
            }
            if(show_boundary_shaded==1){
              draw_foreground=1;
            }
            else{
              draw_foreground=0;
            }
          }
          if(draw_foreground == 1){
             color0 = foregroundcolor;
             color1 = foregroundcolor;
             color2 = foregroundcolor;
          }
          else{
            if(sd==NULL||sd->cell_center==1){
              color0 = rgb_patch+4*ivals[j];
              color1 = color0;
              color2 = color0;
            }
            else{
              color0 = rgb_patch+4*ivals[trianglei->vert_index[0]];
              color1 = rgb_patch+4*ivals[trianglei->vert_index[1]];
              color2 = rgb_patch+4*ivals[trianglei->vert_index[2]];
            }
          }

          xyzptr[0] = trianglei->verts[0]->xyz;
          xyzptr[1] = trianglei->verts[1]->xyz;
          xyzptr[2] = trianglei->verts[2]->xyz;

          if(show_edge1==1){
            glColor3fv(color0);
            glVertex3fv(xyzptr[0]);
            glVertex3fv(xyzptr[1]);
          }

          if(show_edge2==1){
            glColor3fv(color1);
            glVertex3fv(xyzptr[1]);
            glVertex3fv(xyzptr[2]);
          }

          if(show_edge3==1){
            glColor3fv(color2);
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
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY){
        glPointSize(geomboundary_pointsize);
      }
      else if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
        glPointSize(vectorpointsize);
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
        draw_foreground = 0;

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

  /* ------------------ DrawGeomValues ------------------------ */

void DrawGeomValues(slicedata *sd, patchdata *patchi, int geom_type){
  int i;
  unsigned char *ivals;
  int cell_center;
  float *vals;

  if(geom_type==GEOM_STATIC){
    ivals = patchi->geom_ival_static;
    vals = patchi->geom_val_static;
  }
  else{
    ivals = patchi->geom_ival_dynamic;
    vals = patchi->geom_val_dynamic;
  }
  cell_center = sd->cell_center;


  // show values

  if(
    patchi->patch_filetype==PATCH_GEOMETRY_SLICE&&(
    show_slice_values[IN_CUTCELL_GLUI]==1||
    show_slice_values[IN_SOLID_GLUI]==1||
    show_slice_values[IN_GAS_GLUI]==1)
    ){

    for(i = 0; i<1; i++){
      geomdata *geomi;
      geomlistdata *geomlisti;
      int ntris, nverts;
      int j;
      float *color;
      int nvals;

      geomi = patchi->geominfo;
      if(geomi==NULL||geomi->display==0||geomi->loaded==0)continue;
      if(geom_type==GEOM_STATIC){
        geomlisti = geomi->geomlistinfo-1;
      }
      else{
        geomlisti = geomi->geomlistinfo+geomi->itime;
      }

      if(patchi->geom_vert2tri==0){
        patchi->geom_vert2tri = 1;
        Tri2Verts(geomlisti->triangles, geomlisti->ntriangles, geomlisti->verts, geomlisti->nverts);
      }
      ntris = geomlisti->ntriangles;
      nverts = geomlisti->nverts;
      if(ntris==0||nverts==0||patchi->patch_filetype!=PATCH_GEOMETRY_SLICE)continue;
      nvals = ntris;
      if(cell_center==0)nvals = nverts;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      for(j = 0; j<nvals; j++){
        tridata *trianglei;
        vertdata *verti;
        int draw_foreground;

        int insolid;

        insolid = -1;
        if(cell_center==1){
          trianglei = geomlisti->triangles+j;
          insolid = trianglei->insolid&3;
        }
        else{
          verti = geomlisti->verts+j;
          if(verti->triangle1!=NULL)insolid = verti->triangle1->insolid&3;
        }
        if(insolid>=0){
          if(insolid==IN_CUTCELL&&show_slice_values[IN_CUTCELL_GLUI]==0)continue;
          if(insolid==IN_SOLID  &&show_slice_values[IN_SOLID_GLUI]==0)continue;
          if(insolid==IN_GAS    &&show_slice_values[IN_GAS_GLUI]==0)continue;
        }
        if(show_slice_shaded[IN_CUTCELL_GLUI]==1||
           show_slice_shaded[IN_SOLID_GLUI]==1||
           show_slice_shaded[IN_GAS_GLUI]==1){
          draw_foreground = 1;
        }
        else{
          draw_foreground = 0;
        }
        if(draw_foreground==1){
          glColor4fv(foregroundcolor);
        }
        else{
          int color_index;

          color_index = ivals[j];
          color = rgb_patch+4*color_index;
          glColor3fv(color);
        }

        float xmid[3], *xyz1, *xyz2, *xyz3;
        float val;

        if(cell_center==1){
          val = vals[j];
          xyz1 = trianglei->verts[0]->xyz;
          xyz2 = trianglei->verts[1]->xyz;
          xyz3 = trianglei->verts[2]->xyz;
          xmid[0] = (xyz1[0]+xyz2[0]+xyz3[0])/3.0;
          xmid[1] = (xyz1[1]+xyz2[1]+xyz3[1])/3.0;
          xmid[2] = (xyz1[2]+xyz2[2]+xyz3[2])/3.0;
          Output3Val(xmid[0], xmid[1], xmid[2], val);
        }
        else{
          float val1;
          int ival1;

          ival1 = verti-geomlisti->verts;
          xyz1 = verti->xyz;
          val1 = vals[ival1];
          Output3Val(xyz1[0], xyz1[1], xyz1[2], val1);
        }
      }
      glPopMatrix();
    }
  }
}

/* ------------------ DrawCGeom ------------------------ */

void DrawCGeom(int flag, geomdata *cgeom){
  int i;
  geomdata *geomi;

  if(HaveTerrainTexture(NULL) == 1)return;
  if(show_geom_boundingbox==SHOW_BOUNDING_BOX_ALWAYS||geom_bounding_box_mousedown==1){
    if(flag==DRAW_OPAQUE&&have_geom_triangles==1){
      DrawGeomBoundingBox(NULL);
    }
    return;
  }

  // draw surfaces

  geomi = cgeom;
  if(show_faces_shaded==1&&(geomi!=NULL&&geomi->display==1&&geomi->loaded==1)){
    for(i=0;i<1;i++){
      geomlistdata *geomlisti;
      int ntris, j;

      geomlisti = geomi->geomlistinfo-1;
      ntris = geomlisti->ntriangles;
      if(ntris==0)continue;
      if(geom_force_transparent==1&&flag!=DRAW_TRANSPARENT)continue;

      if(flag==DRAW_TRANSPARENT&&use_transparency_data==1)TransparentOn();

      glEnable(GL_NORMALIZE);
      glShadeModel(GL_SMOOTH);
      ENABLE_LIGHTING;
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, iso_specular);
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, iso_shininess);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, block_ambient2);
      glEnable(GL_COLOR_MATERIAL);

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      glBegin(GL_TRIANGLES);
      for(j = 0; j<ntris; j++){
        float *color, *xyzptr[3];
        tridata *trianglei;
        float transparent_level_local;
        float skinny_color[] = {1.0,0.0,0.0,1.0};

        trianglei = geomlisti->triangles+j;

        if(trianglei->outside_domain==0&&showgeom_inside_domain==0)continue;
        if(trianglei->outside_domain==1&&showgeom_outside_domain==0)continue;
        if(trianglei->exterior==0)continue;
        if(trianglei->geomtype==GEOM_CGEOM&&show_faces_shaded==0)continue;
        if(trianglei->geomsurf->invisible==1)continue;

        xyzptr[0] = trianglei->verts[0]->xyz;
        xyzptr[1] = trianglei->verts[1]->xyz;
        xyzptr[2] = trianglei->verts[2]->xyz;

        if(hilight_skinny==1&&trianglei->skinny==1){
          color = skinny_color;
          transparent_level_local = 1.0;
        }
        else{
          if(trianglei->geomobj!=NULL&&trianglei->geomobj->color!=NULL&&trianglei->geomobj->use_geom_color==1){
            color = trianglei->geomobj->color;
            transparent_level_local = trianglei->geomobj->color[3];
          }
          else{
            color = trianglei->geomsurf->color;
            transparent_level_local = trianglei->geomsurf->transparent_level;
          }
        }
        if(geom_force_transparent==1)transparent_level_local = geom_transparency;
        if(have_boundary_arrival == 0){
          if(flag==DRAW_TRANSPARENT&&transparent_level_local>=1.0)continue;
          if(flag!=DRAW_TRANSPARENT&&transparent_level_local<1.0)continue;
        }

        if(lighting_on==1)glNormal3fv(trianglei->tri_norm);
        glColor4f(color[0], color[1], color[2], transparent_level_local);
        glVertex3fv(xyzptr[0]);
        glVertex3fv(xyzptr[1]);
        glVertex3fv(xyzptr[2]);

        glVertex3fv(xyzptr[0]);
        glVertex3fv(xyzptr[2]);
        glVertex3fv(xyzptr[1]);
      }
      glEnd();
      glPopMatrix();
      glDisable(GL_COLOR_MATERIAL);
      DISABLE_LIGHTING;
      if(flag==DRAW_TRANSPARENT&&use_transparency_data==1)TransparentOff();
    }
  }

  // draw lines

  if((show_cface_normals==1||show_faces_outline==1)&&(geomi!=NULL&&geomi->display==1&&geomi->loaded==1)){
    for(i = 0; i<1; i++){
      geomlistdata *geomlisti;
      int ntris;
      int j;

      geomlisti = geomi->geomlistinfo-1;

      ntris = geomlisti->ntriangles;
      if(ntris==0)continue;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      glTranslatef(0.0, 0.0, geom_dz_offset);
      glLineWidth(geom_linewidth);
      glBegin(GL_LINES);
      if(show_faces_outline==1){
        for(j = 0; j<ntris; j++){
          float *xyzptr[3];
          tridata *trianglei;
          int show_edge1 = 1, show_edge2 = 1, show_edge3 = 1;
          float *norm0, *norm1, *norm2;
          float vert2a[3], vert2b[3], vert2c[3];
          int k;

          trianglei = geomlisti->triangles+j;
          if(geom_cface_type==1){
            int insolid4, insolid8, insolid16;

            insolid4 = trianglei->insolid&4;
            if(insolid4==4)show_edge1 = 0;

            insolid8 = trianglei->insolid&8;
            if(insolid8==8)show_edge2 = 0;

            insolid16 = trianglei->insolid&16;
            if(insolid16==16)show_edge3 = 0;
          }
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

          norm0 = trianglei->verts[0]->vert_norm;
          norm1 = trianglei->verts[1]->vert_norm;
          norm2 = trianglei->verts[2]->vert_norm;

          xyzptr[0] = trianglei->verts[0]->xyz;
          xyzptr[1] = trianglei->verts[1]->xyz;
          xyzptr[2] = trianglei->verts[2]->xyz;

          for(k = 0; k<3; k++){
            vert2a[k] = xyzptr[0][k]+geom_norm_offset*norm0[k];
            vert2b[k] = xyzptr[1][k]+geom_norm_offset*norm1[k];
            vert2c[k] = xyzptr[2][k]+geom_norm_offset*norm2[k];
          }
          if(show_edge1==1){
            glVertex3fv(vert2a);
            glVertex3fv(vert2b);
          }

          if(show_edge2==1){
            glVertex3fv(vert2b);
            glVertex3fv(vert2c);
          }

          if(show_edge3==1){
            glVertex3fv(vert2c);
            glVertex3fv(vert2a);
          }
        }
      }
      glEnd();
      glPopMatrix();
    }
    if(show_cface_normals==1&&geomi->geomtype==GEOM_CGEOM){
      int j;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      glLineWidth(geom_linewidth);
      glBegin(GL_LINES);
      for(j = 0; j<geomi->ncface_normals; j++){
        float *v1, *v2;

        v1 = geomi->cface_normals + 6*j;
        v2 = v1 + 3;
        glColor4fv(foregroundcolor);
        glVertex3fv(v1);
        glVertex3fv(v2);
      }
      glEnd();
      glPopMatrix();
    }
  }

  // draw points

  if(show_geom_verts&&(geomi!=NULL&&geomi->display==1&&geomi->loaded==1)){
    for(i = 0; i<1; i++){
      geomlistdata *geomlisti;
      int ntris;
      int j;

      geomlisti = geomi->geomlistinfo-1;

      ntris = geomlisti->ntriangles;
      if(ntris==0)continue;

      glPushMatrix();
      glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
      glTranslatef(-global_scase.xbar0, -global_scase.ybar0, -global_scase.zbar0);
      glTranslatef(0.0, 0.0, geom_dz_offset);
      glPointSize(geom_pointsize);
      glBegin(GL_POINTS);
      for(j = 0; j<ntris; j++){
        float *xyzptr[3];
        tridata *trianglei;

        trianglei = geomlisti->triangles+j;
        glColor4fv(foregroundcolor);

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


/* ------------------ GetGeomInfoPtrs ------------------------ */

void GetGeomInfoPtrs(int flag){
  int i, hide_geom = 0;
  geomdata **gptr = NULL;

  if(flag==1){
    int count;

    count = global_scase.nisoinfo+global_scase.ngeominfo;
    if(count>0){
      NewMemory((void **)&gptr, count*sizeof(geomdata *));
    }
    geominfoptrs = gptr;
    return;
  }

  if(updating_triangles==1)return;

  gptr = geominfoptrs;
  hide_geom = 0;
  for(i = 0;i < global_scase.npatchinfo;i++){
    patchdata *patchi;

    patchi = global_scase.patchinfo + i;
    if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY && patchi->loaded == 1 && patchi->display == 1){
      hide_geom = 1;
      break;
    }
  }
  if(show_geom_bndf==1)hide_geom = 0;

  // count size of geominfoptrs array

  ngeominfoptrs=0;
  for(i=0;i<global_scase.ngeominfo;i++){
    geomdata *geomi;

    geomi = global_scase.geominfo + i;
    // hide geometry if we are displaying a boundary file over top of it
    if(geomi->loaded==1&&geomi->display==1&&geomi->geomtype==GEOM_GEOM&&hide_geom==0)ngeominfoptrs++;
  }
  for(i=0;i<global_scase.nisoinfo;i++){
    isodata *isoi;
    geomdata *geomi;

    isoi = global_scase.isoinfo + i;
    if(isoi->loaded==0||isoi->display==0)continue;
    geomi = isoi->geominfo;
    if(geomi==NULL)continue;
    if(geomi->loaded==0||geomi->display==0)continue;
    ngeominfoptrs++;
  }

  // put pointers into geominfoptrs array

  for(i=0;i<global_scase.ngeominfo;i++){
    geomdata *geomi;

    geomi = global_scase.geominfo + i;
    if(geomi->loaded==1&&geomi->display==1&&geomi->geomtype==GEOM_GEOM&&hide_geom == 0)*gptr++=geomi;
  }
  for(i=0;i<global_scase.nisoinfo;i++){
    isodata *isoi;
    geomdata *geomi;

    isoi = global_scase.isoinfo + i;
    if(isoi->loaded==0||isoi->display==0)continue;
    geomi = isoi->geominfo;
    if(geomi==NULL||geomi->loaded==0||geomi->display==0)continue;
    *gptr++=geomi;
  }
}

/* ------------------ ShowHideSortGeometry ------------------------ */

void ShowHideSortGeometry(int sort_geom, float *mm){
  int i;
  int count_transparent,count_opaque;
  int itime;
  int *showlevels=NULL;
  int iter;

  if(loaded_isomesh!=NULL)showlevels=loaded_isomesh->showlevels;

  have_geom_triangles = 0;
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

      if(global_scase.auto_terrain==1&&i==0)continue;
      if(geomi->is_terrain==1)continue;
      have_geom_triangles = 1;
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
        if(itime==1&&geomi->geomtype==GEOM_ISO){
          if(use_tload_begin==1&&global_times[itimes]<global_scase.tload_begin)continue;
          if(use_tload_end==1&&global_times[itimes]>global_scase.tload_end)continue;
        }

        for(j = 0; j < geomlisti->ntriangles; j++){
          tridata *tri;
          float xyz[3];
          float *xyz1, *xyz2, *xyz3;
          float xyzeye[3];
          int isurf;
          int is_opaque;

          is_opaque = 0;
          tri = geomlisti->triangles+j;
          if(hilight_skinny==1&&tri->skinny==1)is_opaque = 1;
          if(tri->geomobj!=NULL&&tri->geomobj->use_geom_color==1){
            if(tri->geomobj->color[3]>=1.0)is_opaque = 1;
          }
          else{
            if(tri->geomsurf!=NULL&&tri->geomsurf->transparent_level>=1.0)is_opaque = 1;
          }
          if(geom_force_transparent == 1)is_opaque = 0;
          isurf = tri->geomsurf - global_scase.surfcoll.surfinfo - global_scase.surfcoll.nsurfinfo - 1;
          tri->geomlisti = geomlisti;
          if((geomi->geomtype==GEOM_ISO&&showlevels != NULL&&showlevels[isurf] == 0) || (tri->geomsurf!=NULL&&tri->geomsurf->transparent_level <= 0.0)){
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
          if(iter==0&&sort_geom == 1){
            xyz1 = tri->verts[0]->xyz;
            xyz2 = tri->verts[1]->xyz;
            xyz3 = tri->verts[2]->xyz;
            xyz[0] = FDS2SMV_X((xyz1[0] + xyz2[0] + xyz3[0]) / 3.0);
            xyz[1] = FDS2SMV_Y((xyz1[1] + xyz2[1] + xyz3[1]) / 3.0);
            xyz[2] = FDS2SMV_Z((xyz1[2] + xyz2[2] + xyz3[2]) / 3.0);

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
      NEWMEM(alltriangles, (count_opaque + count_transparent) * sizeof(tridata **));
      transparent_triangles = alltriangles;
      opaque_triangles = alltriangles + count_transparent;
    }
  }
  ntransparent_triangles = count_transparent;
  nopaque_triangles = count_opaque;
  if(sort_geom==1&&ntransparent_triangles>0){
    qsort((isotri **)transparent_triangles, (size_t)ntransparent_triangles, sizeof(tridata **), CompareTransparentTriangles);
  }
}
