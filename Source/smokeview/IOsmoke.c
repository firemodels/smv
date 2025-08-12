#include "options.h"
#include "glew.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H
#include <float.h>

#include "smokeviewvars.h"
#include "glui_smoke.h"
#include "IOvolsmoke.h"
#include "compress.h"
#include "getdata.h"
#include "readsmoke.h"
#include "readsmvfile.h"

typedef FILE MFILE;
#define MFILE                   FILE
#define SKIP_SMOKE              FSEEK( SMOKE3DFILE, fortran_skip, SEEK_CUR)
#define FREAD_SMOKE(a,b,c,d)    fread(a,b,c,d)
#define FREADPTR_SMOKE(a,b,c,d) fread(a,b,c,d)
#define FEOF_SMOKE(a)           feof(a)
#define FSEEK_SMOKE(a,b,c)      fseek(a,b,c)
#define FCLOSE_SMOKE(a)         fclose(a)

#define SKIP FSEEK( SMOKE3DFILE, fortran_skip, SEEK_CUR)

int cull_count=0;
int smoke_function_count;

/* ------------------ UpdateSmoke3dFileParms ------------------------ */

void UpdateSmoke3dFileParms(void){
  int i;

  nsootfiles = 0;
  nsootloaded = 0;
  nhrrpuvfiles = 0;
  nhrrpuvloaded = 0;
  ntempfiles = 0;
  ntemploaded = 0;
  nco2files = 0;
  nco2loaded = 0;
  for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->type==SOOT_index){
      if(smoke3di->loaded==1)nsootloaded++;
      nsootfiles++;
    }
    else if(smoke3di->type==HRRPUV_index){
      if(smoke3di->loaded==1)nhrrpuvloaded++;
      nhrrpuvfiles++;
    }
    else if(smoke3di->type==TEMP_index){
      if(smoke3di->loaded==1)ntemploaded++;
      ntempfiles++;
    }
    else if(smoke3di->type==CO2_index){
      if(smoke3di->loaded==1)nco2loaded++;
      nco2files++;
    }
  }
}

// -------------------------- ADJUSTALPHA ----------------------------------

// alpha correction done in alpha_map (different map for each direction, x, y, z, xy, xz, yz)
#define ADJUSTALPHA(ALPHAIN) \
            alphaf_out[n]=0;\
            if(ALPHAIN==0)continue;\
            if(iblank_smoke3d!=NULL&&iblank_smoke3d[n]==SOLID)continue;\
            alphaf_out[n] = ALPHAIN

// -------------------------- DRAWVERTEX ----------------------------------

#define DRAWVERTEX(XX,YY,ZZ)        \
  value[0] = smokealpha_map[alphaf_ptr[n11]]; \
  value[1] = smokealpha_map[alphaf_ptr[n12]]; \
  value[2] = smokealpha_map[alphaf_ptr[n22]]; \
  value[3] = smokealpha_map[alphaf_ptr[n21]]; \
  if(is_firenode != NULL){\
    if(is_firenode[n11] == 1)value[0] = firealpha_map[alphaf_ptr[n11]]; \
    if(is_firenode[n12] == 1)value[1] = firealpha_map[alphaf_ptr[n12]]; \
    if(is_firenode[n22] == 1)value[2] = firealpha_map[alphaf_ptr[n22]]; \
    if(is_firenode[n21] == 1)value[3] = firealpha_map[alphaf_ptr[n21]]; \
  }\
  if(value[0]==0&&value[1]==0&&value[2]==0&&value[3]==0)continue;\
  ivalue[0]=n11<<2;  \
  ivalue[1]=n12<<2;  \
  ivalue[2]=n22<<2;  \
  ivalue[3]=n21<<2;  \
  if(ABS(value[0]-value[2])<ABS(value[1]-value[3])){     \
    xyzindex=xyzindex1;                                  \
  }                                                      \
  else{                                                  \
    xyzindex=xyzindex2;                                  \
  }                                                      \
  for(node=0;node<6;node++){                             \
    int mm;\
    mm = xyzindex[node];                                 \
    colorptr=smokecolor_ptr+ivalue[mm];\
    colorptr[3]=(unsigned char)value[mm];                                   \
    glColor4ubv(colorptr);                                \
    glVertex3f(XX,YY,ZZ);                                \
  }

// -------------------------- DRAWVERTEXTERRAIN ----------------------------------

#define DRAWVERTEXTERRAIN(XX,YY,ZZ)        \
  value[0] = smokealpha_map[alphaf_ptr[n11]]; \
  value[1] = smokealpha_map[alphaf_ptr[n12]]; \
  value[2] = smokealpha_map[alphaf_ptr[n22]]; \
  value[3] = smokealpha_map[alphaf_ptr[n21]]; \
  if(is_firenode != NULL){\
    if(is_firenode[n11] == 1)value[0] = firealpha_map[alphaf_ptr[n11]]; \
    if(is_firenode[n12] == 1)value[1] = firealpha_map[alphaf_ptr[n12]]; \
    if(is_firenode[n22] == 1)value[2] = firealpha_map[alphaf_ptr[n22]]; \
    if(is_firenode[n21] == 1)value[3] = firealpha_map[alphaf_ptr[n21]]; \
  }\
  if(value[0]==0&&value[1]==0&&value[2]==0&&value[3]==0)continue;\
  if(znode_offset==NULL)continue;\
  z_offset[XXX]=znode_offset[m11];\
  z_offset[YYY]=znode_offset[m12];\
  z_offset[ZZZ]=znode_offset[m22];\
  z_offset[3]=znode_offset[m21];\
  ivalue[0]=n11<<2;  \
  ivalue[1]=n12<<2;  \
  ivalue[2]=n22<<2;  \
  ivalue[3]=n21<<2;  \
  if(ABS(value[0]-value[2])<ABS(value[1]-value[3])){     \
    xyzindex=xyzindex1;                                  \
  }                                                      \
  else{                                                  \
    xyzindex=xyzindex2;                                  \
  }                                                      \
  for(node=0;node<6;node++){                             \
    int mm;\
    mm = xyzindex[node];                                 \
    colorptr=smokecolor_ptr+ivalue[mm];\
    colorptr[3]=(unsigned char)value[mm];                                   \
    glColor4ubv(colorptr);                                \
    glVertex3f(XX,YY,ZZ+z_offset[mm]);                                \
  }

// -------------------------- DRAWVERTEXGPU ----------------------------------

#define DRAWVERTEXGPU(XX,YY,ZZ) \
  value[0] = smokealpha_map[alphaf_in[n11]]; \
  value[1] = smokealpha_map[alphaf_in[n12]]; \
  value[2] = smokealpha_map[alphaf_in[n22]]; \
  value[3] = smokealpha_map[alphaf_in[n21]]; \
  if(is_firenode != NULL){\
    if(is_firenode[n11] == 1)value[0] = firealpha_map[alphaf_in[n11]]; \
    if(is_firenode[n12] == 1)value[1] = firealpha_map[alphaf_in[n12]]; \
    if(is_firenode[n22] == 1)value[2] = firealpha_map[alphaf_in[n22]]; \
    if(is_firenode[n21] == 1)value[3] = firealpha_map[alphaf_in[n21]]; \
  }\
  if(iblank_smoke3d!=NULL){\
    if(iblank_smoke3d[n11]==SOLID)value[0]=0;\
    if(iblank_smoke3d[n12]==SOLID)value[1]=0;\
    if(iblank_smoke3d[n22]==SOLID)value[2]=0;\
    if(iblank_smoke3d[n21]==SOLID)value[3]=0;\
  }\
  if(value[0]==0&&value[1]==0&&value[2]==0&&value[3]==0)continue;\
  if(firecolor==NULL&&value[0]==0&&value[1]==0&&value[2]==0&&value[3]==0)continue;\
  if(ABS(value[0]-value[2])<ABS(value[1]-value[3])){     \
    xyzindex=xyzindex1;                                  \
  }                                                      \
  else{                                                  \
    xyzindex=xyzindex2;                                  \
  }                                                      \
  if(firecolor!=NULL){\
    fvalue[0]=firecolor[n11];\
    fvalue[1]=firecolor[n12];\
    fvalue[2]=firecolor[n22];\
    fvalue[3]=firecolor[n21];\
  }\
  else{\
    fvalue[0]=0.0;\
    fvalue[1]=0.0;\
    fvalue[2]=0.0;\
    fvalue[3]=0.0;\
  }\
  for(node=0;node<6;node++){                             \
    int mm;\
    mm = xyzindex[node];                                 \
    glVertexAttrib1f(GPU_hrr, fvalue[mm]); \
    glVertexAttrib1f(GPU_smokealpha, value[mm]); \
    glVertex3f(XX, YY, ZZ);                                \
  }

  // -------------------------- DRAWVERTEXGPUTERRAIN ----------------------------------

#define DRAWVERTEXGPUTERRAIN(XX,YY,ZZ) \
  if(znode_offset==NULL)continue;\
  z_offset[XXX]=znode_offset[m11];\
  z_offset[YYY]=znode_offset[m12];\
  z_offset[ZZZ]=znode_offset[m22];\
  z_offset[3]=znode_offset[m21];\
  value[0] = smokealpha_map[alphaf_in[n11]]; \
  value[1] = smokealpha_map[alphaf_in[n12]]; \
  value[2] = smokealpha_map[alphaf_in[n22]]; \
  value[3] = smokealpha_map[alphaf_in[n21]]; \
  if(is_firenode != NULL){\
    if(is_firenode[n11] == 1)value[0] = firealpha_map[alphaf_in[n11]]; \
    if(is_firenode[n12] == 1)value[1] = firealpha_map[alphaf_in[n12]]; \
    if(is_firenode[n22] == 1)value[2] = firealpha_map[alphaf_in[n22]]; \
    if(is_firenode[n21] == 1)value[3] = firealpha_map[alphaf_in[n21]]; \
  }\
  if(iblank_smoke3d!=NULL){\
    if(iblank_smoke3d[n11]==SOLID)value[0]=0;\
    if(iblank_smoke3d[n12]==SOLID)value[1]=0;\
    if(iblank_smoke3d[n22]==SOLID)value[2]=0;\
    if(iblank_smoke3d[n21]==SOLID)value[3]=0;\
  }\
  if(value[0]==0&&value[1]==0&&value[2]==0&&value[3]==0)continue;\
  if(ABS(value[0]-value[2])<ABS(value[1]-value[3])){     \
    xyzindex=xyzindex1;                                  \
  }                                                      \
  else{                                                  \
    xyzindex=xyzindex2;                                  \
  }                                                        \
  if(firecolor!=NULL){\
    fvalue[0]=firecolor[n11];\
    fvalue[1]=firecolor[n12];\
    fvalue[2]=firecolor[n22];\
    fvalue[3]=firecolor[n21];\
  }\
  else{\
    fvalue[0]=0.0;\
    fvalue[1]=0.0;\
    fvalue[2]=0.0;\
    fvalue[3]=0.0;\
  }\
  for(node=0;node<6;node++){                             \
    int mm;\
    mm = xyzindex[node];                                 \
    glVertexAttrib1f(GPU_smokealpha,(float)value[mm]);\
    glVertexAttrib1f(GPU_hrr,(float)fvalue[mm]);\
    glVertex3f(XX,YY,ZZ+z_offset[mm]);                                \
  }


/* ------------------ GetCellindex ------------------------ */

int GetCellindex(float *xyz, meshdata **mesh_tryptr){
  int i;
  meshdata *mesh_try=NULL;

  if(mesh_tryptr != NULL)mesh_try = *mesh_tryptr;
  for(i = -1; i < global_scase.meshescoll.nmeshes; i++){
    meshdata *meshi;
    float *boxmin, *boxmax, *dbox;

    if(i == -1){
      if(mesh_try == NULL)continue;
      meshi = mesh_try;
    }
    else{
      meshi = global_scase.meshescoll.meshinfo + i;
      if(meshi == mesh_try)continue;
    }
    boxmin = meshi->boxmin_fds;
    boxmax = meshi->boxmax_fds;
    dbox   = meshi->dbox_fds;
    if(boxmin[0] <= xyz[0] && xyz[0] <= boxmax[0] &&
       boxmin[1] <= xyz[1] && xyz[1] <= boxmax[1] &&
       boxmin[2] <= xyz[2] && xyz[2] <= boxmax[2]){
      int nx, ny, nxy, ijk;
      int ix, iy, iz;
      int ibar, jbar, kbar;

      ibar = meshi->ibar;
      jbar = meshi->jbar;
      kbar = meshi->kbar;
      nx = ibar + 1;
      ny = jbar + 1;
      nxy = nx*ny;

      ix = ibar*(xyz[0] - boxmin[0]) / dbox[0];
      ix = CLAMP(ix, 0, ibar);

      iy = jbar*(xyz[1] - boxmin[1]) / dbox[1];
      iy = CLAMP(iy, 0, jbar);

      iz = kbar*(xyz[2] - boxmin[2]) / dbox[2];
      iz = CLAMP(iz, 0, kbar);

      ijk = IJKNODE(ix, iy, iz);
      if(mesh_tryptr!=NULL)*mesh_tryptr = meshi;
      return ijk;
    }
  }
  if(mesh_tryptr!=NULL)*mesh_tryptr = NULL;
  return -1;

}

/* ------------------ GetSootDensity ------------------------ */

float GetSootDensity(float *xyz, int itime, meshdata **mesh_try){
  int ijk;
  meshdata *mesh_soot;
  float soot_val=0.0, *slice_vals;
  volrenderdata *vr;

  ijk = GetCellindex(xyz, mesh_try);
  if(mesh_try == NULL || *mesh_try == NULL|| ijk<0)return 0.0;
  mesh_soot = *mesh_try;
  if(mesh_soot->c_iblank_node != NULL&&mesh_soot->c_iblank_node[ijk] == SOLID){
    return 1000000.0;
  }
  vr = mesh_soot->volrenderinfo;
  if(vr->smokedataptrs ==NULL)return 0.0;
  itime = CLAMP(itime,0, vr->ntimes -1);
  if(vr->smokedataptrs[itime] == NULL)return 0.0;
  slice_vals = vr->smokedataptrs[itime];
  soot_val = slice_vals[ijk];
  return soot_val;
}

/* ------------------ IsSmokeComponentPresent ------------------------ */

int IsSmokeComponentPresent(smoke3ddata *smoke3di){
  int i;

  for(i = 0;i < global_scase.smoke3dcoll.nsmoke3dtypes;i++){
    smoke3ddata *smoke_component;

    if(smoke3di->smokestate[i].index == -1)continue;
    smoke_component = global_scase.smoke3dcoll.smoke3dinfo + smoke3di->smokestate[i].index;
    if(smoke_component->loaded != 1 || smoke_component->display != 1)continue;
    if(smoke_component->frame_all_zeros[smoke_component->ismoke3d_time] == SMOKE3D_ZEROS_ALL)continue;
    return 1;
  }
  return 0;
}

/* ------------------ IsSmokeLoaded ------------------------ */

int IsSmokeLoaded(smv_case *scase){
  int i;

  for(i = 0;i < scase->smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = scase->smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->is_smoke==1 && smoke3di->loaded==1 && smoke3di->display==1)return 1;
  }
  return 0;
}

#ifdef pp_GPU
  /* ------------------ DrawSmoke3DGPU ------------------------ */

void DrawSmoke3DGPU(smoke3ddata *smoke3di){
  int i, j, k, n;
  float constval, x1, x3, z1, z3, yy1, y3;
  int is1, is2, js1, js2, ks1, ks2;
  int ii, jj, kk;
  int ibeg, iend, jbeg, jend, kbeg, kend;

  float *xplt, *yplt, *zplt;
  float *znode_offset, z_offset[4];

  int nx, ny, nz;
  int xyzindex1[6], xyzindex2[6], *xyzindex, node;
  float xnode[4], znode[4], ynode[4];
  int skip_local;
  int iterm, jterm, kterm, nxy;
  int n11, n12, n22, n21;
  int iii, jjj, kkk;
  int slice_end, slice_beg;
  int ssmokedir;
  unsigned char *iblank_smoke3d, *is_firenode;
  int have_smoke_local;
  int have_fire_local;

  unsigned char *firecolor, *alphaf_in;
  float value[4], fvalue[4];

  meshdata *meshi;

  meshi = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;
  if(meshvisptr[meshi-global_scase.meshescoll.meshinfo]==0)return;

  if(HRRPUV_index >= 0 && smoke3di->smokestate[HRRPUV_index].index >= 0){
    firecolor = smoke3di->smokestate[HRRPUV_index].color;
  }
  else if(TEMP_index >= 0 && smoke3di->smokestate[TEMP_index].index >= 0){
    firecolor = smoke3di->smokestate[TEMP_index].color;
  }
  else{
    firecolor = NULL;
  }

  smoke3ddata *sooti = NULL;
  have_smoke_local = 0;
  if(SOOT_index >= 0 && smoke3di->smokestate[SOOT_index].index>=0) {
    sooti = global_scase.smoke3dcoll.smoke3dinfo+smoke3di->smokestate[SOOT_index].index;
    if(sooti != NULL && sooti->display == 1)have_smoke_local = 1;
  }

  smoke3ddata *firei = NULL;
  have_fire_local = 0;
  if(HRRPUV_index >= 0 && smoke3di->smokestate[HRRPUV_index].index >= 0) {
    firei = global_scase.smoke3dcoll.smoke3dinfo + smoke3di->smokestate[HRRPUV_index].index;
    if(firei != NULL && firei->display == 1)have_fire_local = 1;
  }
  if(have_fire_local==0 && TEMP_index >= 0 && smoke3di->smokestate[TEMP_index].index >= 0) {
    firei = global_scase.smoke3dcoll.smoke3dinfo + smoke3di->smokestate[TEMP_index].index;
    if(firei != NULL && firei->display == 1)have_fire_local = 1;
  }

  iblank_smoke3d = meshi->iblank_smoke3d;

  // meshi->global_hrrpuv_cb_min
  // global_hrrpuv_max;

  meshi = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;
  if(meshvisptr[meshi-global_scase.meshescoll.meshinfo]==0)return;
  value[0] = 255;
  value[1] = 255;
  value[2] = 255;
  value[3] = 255;

  znode_offset = NULL;
  if(global_scase.nterraininfo>0){
    znode_offset = meshi->terrain->znode_offset;
  }

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  alphaf_in = smoke3di->smokeframe_in;

  is1 = smoke3di->is1;
  is2 = smoke3di->is2;
  js1 = smoke3di->js1;
  js2 = smoke3di->js2;
  ks1 = smoke3di->ks1;
  ks2 = smoke3di->ks2;
  if(smoke3d_kmax>0){
    ks2 = CLAMP(ks2, 1, smoke3d_kmax);
    ks2 = CLAMP(ks2, 1, smoke3di->ks2);
  }

  nx = smoke3di->is2+1-smoke3di->is1;
  ny = js2+1-js1;
  nz = ks2+1-ks1;
  nxy = nx*ny;

  ssmokedir = meshi->smokedir;
  skip_local = smoke3d_frame_inc;

  xyzindex1[0] = 0;
  xyzindex1[1] = 1;
  xyzindex1[2] = 2;
  xyzindex1[3] = 0;
  xyzindex1[4] = 2;
  xyzindex1[5] = 3;

  xyzindex2[0] = 0;
  xyzindex2[1] = 1;
  xyzindex2[2] = 3;
  xyzindex2[3] = 1;
  xyzindex2[4] = 2;
  xyzindex2[5] = 3;

  is_firenode = meshi->is_firenode;

  if(cullfaces==1)glDisable(GL_CULL_FACE);

  glUniform1f(GPU_emission_factor, emission_factor);
  glUniform1i(GPU_use_fire_alpha, use_fire_alpha);
  glUniform1i(GPU_have_smoke, have_smoke_local);
  glUniform1i(GPU_have_fire, have_fire_local);
  glUniform1i(GPU_force_alpha_opaque, force_alpha_opaque);
  glUniform1i(GPU_smokecolormap, 2);
  if(smoke3di->type == TEMP_index && TEMP_index >= 0) {
    glUniform1f(GPU_global_hrrpuv_max, global_scase.temp_max);
    glUniform1f(GPU_global_hrrpuv_cb_min, global_temp_cb_min);
  }
  else{
    glUniform1f(GPU_global_hrrpuv_max, global_scase.hrrpuv_max);
    glUniform1f(GPU_global_hrrpuv_cb_min, global_hrrpuv_cb_min);
  }
  glUniform1f(GPU_fire_alpha, smoke3di->fire_alpha);

  TransparentOn();
  switch(ssmokedir){
    unsigned char *smokealpha_map, *firealpha_map;

    // +++++++++++++++++++++++++++++++++++ DIR 1 +++++++++++++++++++++++++++++++++++++++


  case 1:
  case -1:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_X];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_X];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = is1;
    slice_end = is2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      i = iii;
      if(ssmokedir<0)i = is1+is2-iii-1;
      iterm = (i-smoke3di->is1);
      constval = xplt[i]+0.001;
      for(k = ks1; k<ks2; k+=smoke3d_skipz){
        int k2;

        k2 = MIN(k+smoke3d_skipz, ks2);
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k2];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;
        for(j = js1; j<js2; j+=smoke3d_skipy){
          int j2;

          j2 = MIN(j+smoke3d_skipy, js2);
          jterm = (j-js1)*nx;
          yy1 = yplt[j];
          y3 = yplt[j2];
          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          n = iterm+jterm+kterm;

          n11 = n;              //n
          n12 = n11+nx;       //n+nx
          n22 = n12+nxy;      //n+nx+nxy
          n21 = n22-nx;       //n+nxy

          //        n11 = (i-is1)   + (j-js1)*nx   + (k-ks1)*nx*ny;
          //        n12 = (i-is1)   + (j+1-js1)*nx + (k-ks1)*nx*ny;
          //        n22 = (i-is1)   + (j+1-js1)*nx + (k+1-ks1)*nx*ny;
          //        n21 = (i-is1)   + (j-js1)*nx   + (k+1-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+nx;
            m22 = m12;
            m21 = m22-nx;
            DRAWVERTEXGPUTERRAIN(constval, ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEXGPU(constval, ynode[mm], znode[mm])
          }

        }
      }
    }
    glEnd();

    break;

    // +++++++++++++++++++++++++++++++++++ DIR 2 +++++++++++++++++++++++++++++++++++++++

  case 2:
  case -2:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_Y];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_Y];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = js1;
    slice_end = js2;
    for(jjj = slice_beg;jjj<slice_end;jjj += skip_local){
      j = jjj;
      if(ssmokedir<0)j = js1+js2-jjj-1;
      constval = yplt[j]+0.001;
      jterm = (j-js1)*nx;
      for(k = ks1; k<ks2; k+=smoke3d_skipz){
        int k2;

        k2 = MIN(k+smoke3d_skipz, ks2);
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k2];

        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;
        for(i = is1; i<is2; i+=smoke3d_skipx){
          int i2;

          i2 = MIN(i+smoke3d_skipx, is2);
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i2];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          n = iterm+jterm+kterm;
          n11 = n;            //n
          n12 = n11+1;        //n+1
          n22 = n12+nxy;      //n+1+nxy
          n21 = n22-1;        //n+nxy

          //        n11 = (i-is1)   + (j-js1)*nx   + (k-ks1)*nx*ny;
          //        n12 = (i+1-is1) + (j-js1)*nx   + (k-ks1)*nx*ny;
          //        n22 = (i+1-is1) + (j-js1)*nx   + (k+1-ks1)*nx*ny;
          //        n21 = (i-is1)   + (j-js1)*nx   + (k+1-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+1;
            m22 = m12;
            m21 = m22-1;
            DRAWVERTEXGPUTERRAIN(xnode[mm], constval, znode[mm])
          }
          else{
            DRAWVERTEXGPU(xnode[mm], constval, znode[mm])
          }

        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 3 +++++++++++++++++++++++++++++++++++++++

  case 3:
  case -3:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_Z];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_Z];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = ks1;
    slice_end = ks2;
    for(kkk = slice_beg;kkk<slice_end;kkk += skip_local){
      k = kkk;
      if(ssmokedir<0)k = ks1+ks2-kkk-1;
      constval = zplt[k]+0.001;
      kterm = (k-ks1)*nxy;
      for(j = js1; j<js2; j+=smoke3d_skipy){
        int j2;

        j2 = MIN(j+smoke3d_skipy, js2);
        jterm = (j-js1)*nx;

        yy1 = yplt[j];
        y3 = yplt[j2];

        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;
        for(i = is1; i<is2; i+=smoke3d_skipx){
          int i2;

          i2 = MIN(i+smoke3d_skipx, is2);
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i2];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          n = iterm+jterm+kterm;
          n11 = n;
          n12 = n11+1;
          n22 = n12+nx;
          n21 = n22-1;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+1;
            m22 = m12+nx;
            m21 = m22-1;
            DRAWVERTEXGPUTERRAIN(xnode[mm], ynode[mm], constval)
          }
          else{
            DRAWVERTEXGPU(xnode[mm], ynode[mm], constval)
          }
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 4 +++++++++++++++++++++++++++++++++++++++

  case 4:
  case -4:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_XY];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_XY];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = nx+ny-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int ipj;

      ipj = iii;
      if(ssmokedir<0)ipj = nx+ny-2-iii;
      ibeg = 0;
      jbeg = ipj;
      if(jbeg>ny-1){
        jbeg = ny-1;
        ibeg = ipj-jbeg;
      }
      iend = nx-1;
      jend = ipj-iend;
      if(jend<0){
        jend = 0;
        iend = ipj-jend;
      }
      for(k = ks1; k<ks2; k+=smoke3d_skipz){
        int k2;

        k2 = MIN(k+smoke3d_skipz, ks2);
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k2];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;
        for(ii = ibeg;ii<iend;ii++){
          i = is1+ii;
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i+1];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          jj = ipj-ii;
          j = js1+jj;
          jterm = (j-js1)*nx;

          yy1 = yplt[j];
          y3 = yplt[j-1];

          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          n11 = iterm+jterm+kterm;
          n12 = n11-nx+1;
          n22 = n12+nxy;
          n21 = n11+nxy;

          //        n11 = (j-js1)*nx   + (i-is1)   + (k-ks1)*nx*ny;
          //        n12 = (j-1-js1)*nx + (i+1-is1) + (k-ks1)*nx*ny;
          //        n22 = (j-1-js1)*nx + (i+1-is1) + (k+1-ks1)*nx*ny;
          //        n21 = (j-js1)*nx   + (i-is1)   + (k+1-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11-nx+1;
            m22 = m12;
            m21 = m11-nx;
            DRAWVERTEXGPUTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEXGPU(xnode[mm], ynode[mm], znode[mm])
          }
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 5 +++++++++++++++++++++++++++++++++++++++

  case 5:
  case -5:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_XY];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_XY];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = nx+ny-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int jmi;

      jmi = iii;
      if(ssmokedir<0)jmi = nx+ny-2-iii;

      ibeg = 0;
      jbeg = ibeg-nx+1+jmi;
      if(jbeg<0){
        jbeg = 0;
        ibeg = jbeg+nx-1-jmi;
      }
      iend = nx-1;
      jend = iend+jmi+1-nx;
      if(jend>ny-1){
        jend = ny-1;
        iend = jend+nx-1-jmi;
      }
      for(k = ks1; k<ks2; k+=smoke3d_skipz){
        int k2;

        k2 = MIN(k+smoke3d_skipz, ks2);
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k2];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;
        for(ii = ibeg;ii<iend;ii++){
          i = is1+ii;
          iterm = (i-is1);

          jj = ii+jmi+1-nx;
          j = js1+jj;
          jterm = (j-js1)*nx;


          yy1 = yplt[j];
          y3 = yplt[j+1];

          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          x1 = xplt[i];
          x3 = xplt[i+1];
          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;


          n11 = jterm+iterm+kterm;
          n12 = n11+nx+1;
          n22 = n12+nxy;
          n21 = n11+nxy;

          //    n11 = (j-js1)*nx + (i-is1) + (k-ks1)*nx*ny;
          //    n12 = (j+1-js1)*nx + (i+1-is1) + (k-ks1)*nx*ny;
          //    n22 = (j+1-js1)*nx + (i+1-is1) + (k+1-ks1)*nx*ny;
          //    n21 = (j-js1)*nx + (i-is1) + (k+1-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+nx+1;
            m22 = m12;
            m21 = m11;
            DRAWVERTEXGPUTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEXGPU(xnode[mm], ynode[mm], znode[mm])
          }
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 6 +++++++++++++++++++++++++++++++++++++++

  case 6:
  case -6:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_YZ];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_YZ];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = ny+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int jpk;

      jpk = iii;
      if(ssmokedir<0)jpk = ny+nz-2-iii;
      jbeg = 0;
      kbeg = jpk;
      if(kbeg>nz-1){
        kbeg = nz-1;
        jbeg = jpk-kbeg;
      }
      jend = ny-1;
      kend = jpk-jend;
      if(kend<0){
        kend = 0;
        jend = jpk-kend;
      }
      for(i = is1; i<is2; i+=smoke3d_skipx){
        int i2;

        i2 = MIN(i+smoke3d_skipx, is2);
        iterm = (i-is1);
        x1 = xplt[i];
        x3 = xplt[i2];
        xnode[0] = x1;
        xnode[1] = x1;
        xnode[2] = x3;
        xnode[3] = x3;
        for(jj = jbeg;jj<jend;jj++){
          j = js1+jj;
          jterm = (j-js1)*nx;
          yy1 = yplt[j];
          y3 = yplt[j+1];

          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          kk = jpk-jj;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          z1 = zplt[k];
          z3 = zplt[k-1];

          znode[0] = z1;
          znode[1] = z3;
          znode[2] = z3;
          znode[3] = z1;

          n11 = iterm+jterm+kterm;
          n12 = n11+nx-nxy;
          n22 = n12+1;
          n21 = n22-nx+nxy;

          //        n11 = (i-is1)   + (j-js1)*nx   + (k-ks1)*nx*ny;
          //        n12 = (i-is1)   + (j+1-js1)*nx + (k-1-ks1)*nx*ny;
          //        n22 = (i+1-is1) + (j+1-js1)*nx + (k-1-ks1)*nx*ny;
          //        n21 = (i+1-is1) + (j-js1)*nx   + (k-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+nx;
            m22 = m12+1;
            m21 = m22-nx;
            DRAWVERTEXGPUTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEXGPU(xnode[mm], ynode[mm], znode[mm])
          }
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 7 +++++++++++++++++++++++++++++++++++++++

  case 7:
  case -7:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_YZ];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_YZ];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = ny+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int kmj;

      kmj = iii;
      if(ssmokedir<0)kmj = ny+nz-2-iii;

      jbeg = 0;
      kbeg = jbeg-ny+1+kmj;
      if(kbeg<0){
        kbeg = 0;
        jbeg = kbeg+ny-1-kmj;
      }
      jend = ny-1;
      kend = jend+kmj+1-ny;
      if(kend>nz-1){
        kend = nz-1;
        jend = kend+ny-1-kmj;
      }
      for(i = is1; i<is2; i+=smoke3d_skipx){
        int i2;

        i2 = MIN(i+smoke3d_skipx, is2);
        iterm = (i-is1);
        x1 = xplt[i];
        x3 = xplt[i2];
        xnode[0] = x1;
        xnode[1] = x1;
        xnode[2] = x3;
        xnode[3] = x3;
        for(jj = jbeg;jj<jend;jj++){
          j = js1+jj;
          jterm = (j-js1)*nx;

          kk = jj+kmj+1-ny;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;


          z1 = zplt[k];
          z3 = zplt[k+1];

          znode[0] = z1;
          znode[1] = z3;
          znode[2] = z3;
          znode[3] = z1;

          yy1 = yplt[j];
          y3 = yplt[j+1];
          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;


          n11 = jterm+iterm+kterm;
          n12 = n11+nxy+nx;
          n22 = n12+1;
          n21 = n22-nx-nxy;

          //    n11 = (i-is1)   + (j-js1)*nx    + (k-ks1)*nx*ny;
          //    n12 = (i-is1)   + (j+1-js1)*nx  + (k+1-ks1)*nx*ny;
          //    n22 = (i+1-is1) + (j+1-js1)*nx  + (k+1-ks1)*nx*ny;
          //    n21 = (i+1-is1) + (j-js1)*nx    + (k-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+nx;
            m22 = m12+1;
            m21 = m22-nx;
            DRAWVERTEXGPUTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEXGPU(xnode[mm], ynode[mm], znode[mm])
          }
        }
      }
    }
    glEnd();
    break;


    // +++++++++++++++++++++++++++++++++++ DIR 8 +++++++++++++++++++++++++++++++++++++++

  case 8:
  case -8:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_XZ];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_XZ];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = nx+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int ipk;

      ipk = iii;
      if(ssmokedir<0)ipk = nx+nz-2-iii;
      ibeg = 0;
      kbeg = ipk;
      if(kbeg>nz-1){
        kbeg = nz-1;
        ibeg = ipk-kbeg;
      }
      iend = nx-1;
      kend = ipk-iend;
      if(kend<0){
        kend = 0;
        iend = ipk-kend;
      }
      for(j = js1; j<js2; j+=smoke3d_skipy){
        int j2;

        j2 = MIN(j+smoke3d_skipy, js2);
        jterm = (j-js1)*nx;
        yy1 = yplt[j];
        y3 = yplt[j2];
        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;
        for(ii = ibeg;ii<iend;ii++){
          i = is1+ii;
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i+1];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          kk = ipk-ii;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          z1 = zplt[k];
          z3 = zplt[k-1];

          znode[0] = z1;
          znode[1] = z3;
          znode[2] = z3;
          znode[3] = z1;

          n11 = iterm+jterm+kterm;
          n12 = n11+1-nxy;
          n22 = n12+nx;
          n21 = n22-1+nxy;

          //        n11 = (i-is1)   + (j-js1)*nx   + (k-ks1)*nx*ny;
          //        n12 = (i+1-is1) + (j-js1)*nx   + (k-1-ks1)*nx*ny;
          //        n22 = (i+1-is1) + (j+1-js1)*nx + (k-1-ks1)*nx*ny;
          //        n21 = (i-is1)   + (j+1-js1)*nx + (k-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+1;
            m22 = m12+nx;
            m21 = m22-1;
            DRAWVERTEXGPUTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEXGPU(xnode[mm], ynode[mm], znode[mm])
          }
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 9 +++++++++++++++++++++++++++++++++++++++

  case 9:
  case -9:

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_XZ];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_XZ];

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = nx+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int kmi;

      kmi = iii;
      if(ssmokedir<0)kmi = nx+nz-2-iii;

      ibeg = 0;
      kbeg = ibeg-nx+1+kmi;
      if(kbeg<0){
        kbeg = 0;
        ibeg = kbeg+nx-1-kmi;
      }
      iend = nx-1;
      kend = iend+kmi+1-nx;
      if(kend>nz-1){
        kend = nz-1;
        iend = kend+nx-1-kmi;
      }
      for(j = js1; j<js2; j+=smoke3d_skipy){
        int j2;

        j2 = MIN(j+smoke3d_skipy, js2);
        jterm = (j-js1)*nx;
        yy1 = yplt[j];
        y3 = yplt[j2];
        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;
        for(ii = ibeg;ii<iend;ii++){
          i = is1+ii;
          iterm = (i-is1);

          kk = ii+kmi+1-nx;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;


          z1 = zplt[k];
          z3 = zplt[k+1];

          znode[0] = z1;
          znode[1] = z3;
          znode[2] = z3;
          znode[3] = z1;

          x1 = xplt[i];
          x3 = xplt[i+1];
          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;


          n11 = jterm+iterm+kterm;
          n12 = n11+nxy+1;
          n22 = n12+nx;
          n21 = n22-1-nxy;

          //    n11 = (i-is1)   + (j-js1)*nx   + (k-ks1)*nx*ny;
          //    n12 = (i+1-is1) + (j-js1)*nx   + (k+1-ks1)*nx*ny;
          //    n22 = (i+1-is1) + (j+1-js1)*nx + (k+1-ks1)*nx*ny;
          //    n21 = (i-is1)   + (j+1-js1)*nx + (k-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+1;
            m22 = m12+nx;
            m21 = m22-1;
            DRAWVERTEXGPUTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEXGPU(xnode[mm], ynode[mm], znode[mm])
          }
        }
      }
    }
    glEnd();
    break;
  default:
    assert(FFALSE);
    break;
  }
  TransparentOff();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
}

#endif

/* ------------------ InitAlphas ------------------------ */

void InitAlphas(unsigned char *smokealphanew, unsigned char *firealphanew, float base_extinct, int use_smoke_density, float maxval, float new_extinct, float base_dx, float new_dx){
  int i;

  if(base_extinct<=0.01){
    base_extinct = 1.0;
    new_extinct = 1.0;
  }
  smokealphanew[0] = 0;
  firealphanew[0]  = 0;
  if(force_alpha_opaque==1){
    memset(smokealphanew+1, 254, 254);
    memset(firealphanew+1,  254, 254);
    return;
  }
  if(smoke3d_frame_inc != 1)new_dx *= (float)smoke3d_frame_inc;
  if(use_smoke_density == 1){;
    for(i = 1; i < 255; i++){
      float soot_density, soot_opacity;
      int soot_alpha, fire_alpha;

      soot_density     = maxval*(float)i/255.0;
      soot_opacity     = 254.0*(1.0 - exp(-new_extinct*new_dx*soot_density));
      soot_alpha       = CLAMP(soot_opacity+0.5, 0, 254);
      smokealphanew[i] = (unsigned char)soot_alpha;
      fire_alpha       = CLAMP(emission_factor*soot_opacity+0.5, 0, 254);
      firealphanew[i]  = (unsigned char)fire_alpha;
    }
  }
  else{
    for(i = 1; i<255; i++){
      float soot_density, soot_opacity;
      int soot_alpha, fire_alpha;

      soot_density     = -log(1.0-(float)i/254.0)/(base_extinct*base_dx);
      soot_opacity     = 254.0*(1.0-exp(-new_extinct*new_dx*soot_density))+0.5;
      soot_alpha       = CLAMP(soot_opacity+0.5, 0, 254);
      smokealphanew[i] = (unsigned char)soot_alpha;
      fire_alpha       = CLAMP(emission_factor*soot_opacity+0.5, 0, 254);
      firealphanew[i]  = (unsigned char)fire_alpha;
    }
  }
}

/* ------------------ UpdateSmokeAlphas ------------------------ */

void UpdateSmokeAlphas(void){
  int i;

  for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    float dists[6];
    meshdata *smoke_mesh;
    int j;
    float dx;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    if(smoke3di->extinct<0.0)continue;
    smoke_mesh = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;
    dx = smoke_mesh->dxyz_orig[0];
    dists[ALPHA_X]  = dx;
    dists[ALPHA_Y]  = smoke_mesh->dxyz_orig[1];
    dists[ALPHA_Z]  = smoke_mesh->dxyz_orig[2];
    dists[ALPHA_XY] = smoke_mesh->dxyDdx*dx;
    dists[ALPHA_YZ] = smoke_mesh->dyzDdx*dx;
    dists[ALPHA_XZ] = smoke_mesh->dxzDdx*dx;
    for(j=0;j<6;j++){
      float maxval;

      assert(
             (smoke3di->soot_density_loaded == 1 && smoke3di->maxvals!=NULL) ||
             (smoke3di->soot_density_loaded == 0 && smoke3di->maxvals==NULL)
            );
      maxval = smoke3di->maxval;
      if(smoke3di->soot_density_loaded == 1 && smoke3di->maxvals!=NULL)maxval = smoke3di->maxvals[smoke3di->ismoke3d_time];
      InitAlphas(smoke3di->alphas_smokedir[j], smoke3di->alphas_firedir[j], smoke3di->extinct, smoke3di->soot_density_loaded, maxval, glui_smoke3d_extinct, smoke_mesh->dxyz_orig[0], dists[j]);
    }
  }
}

/* ------------------ DrawSmoke3D ------------------------ */

int DrawSmoke3D(smoke3ddata *smoke3di){
  int i, j, k, n;
  float constval, x1, x3, z1, z3, yy1, y3;
  int is1, is2, js1, js2, ks1, ks2;
  int ii, jj, kk;
  int ibeg, iend, jbeg, jend, kbeg, kend;
  float *znode_offset=NULL, z_offset[4];

  float *xplt, *yplt, *zplt;
  unsigned char *smokealpha_ptr, *smokecolor_ptr;
  int nx, ny, nz;
  unsigned char *alphaf_out, *alphaf_ptr;
  unsigned char *colorptr;
  int xyzindex1[6], xyzindex2[6], *xyzindex, node;
  float xnode[4], znode[4], ynode[4];
  int skip_local;
  int iterm, jterm, kterm, nxy;
  unsigned int n11, n12, n22, n21;
  int iii, jjj, kkk;
  int slice_end, slice_beg;
  int ssmokedir;
  unsigned char *iblank_smoke3d, *is_firenode;

  unsigned char value[4];
  int ivalue[4];               int nsmoke_triangles = 0;

  meshdata *meshi;

  meshi = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;
  if(meshvisptr[meshi-global_scase.meshescoll.meshinfo]==0)return 0;

  if(meshi->smokealpha_ptr==NULL||meshi->merge_alpha==NULL||meshi->update_smoke3dcolors==1){
    meshi->update_smoke3dcolors = 0;
    MergeSmoke3D(smoke3di);
  }
  smokealpha_ptr = meshi->smokealpha_ptr;
  smokecolor_ptr = meshi->smokecolor_ptr;
  value[0] = 255;
  value[1] = 255;
  value[2] = 255;
  value[3] = 255;

  if(global_scase.nterraininfo>0&&meshi->terrain!=NULL){
    znode_offset = meshi->terrain->znode_offset;
  }

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  iblank_smoke3d = meshi->iblank_smoke3d;
  alphaf_out = smoke3di->smokeframe_out;

  switch(demo_mode){
  case 0:
  case 5:
    is1 = smoke3di->is1;
    is2 = smoke3di->is2;
    break;
  case 1:
    is1 = (smoke3di->is1+smoke3di->is2)/2;
    is2 = is1+1;
    if(is1<smoke3di->is1)is1 = smoke3di->is1;
    if(is2>smoke3di->is2)is2 = smoke3di->is2;
    break;
  default:
    is1 = (smoke3di->is1+smoke3di->is2)/2-demo_mode;
    is2 = is1+2*demo_mode;
    if(is1<smoke3di->is1)is1 = smoke3di->is1;
    if(is2>smoke3di->is2)is2 = smoke3di->is2;
    break;
  }
  js1 = smoke3di->js1;
  js2 = smoke3di->js2;
  ks1 = smoke3di->ks1;
  ks2 = smoke3di->ks2;
  if(smoke3d_imax > 0 && is2 > smoke3d_imax)is2 = smoke3d_imax;
  if(smoke3d_jmax > 0 && js2 > smoke3d_jmax)js2 = smoke3d_jmax;
  if(smoke3d_kmax > 0 && ks2 > smoke3d_kmax)ks2 = smoke3d_kmax;

  nx = smoke3di->is2+1-smoke3di->is1;
  ny = js2+1-js1;
  nz = ks2+1-ks1;
  nxy = nx*ny;

  ssmokedir = meshi->smokedir;
  skip_local = smoke3d_frame_inc;

  xyzindex1[0] = 0;
  xyzindex1[1] = 1;
  xyzindex1[2] = 2;
  xyzindex1[3] = 0;
  xyzindex1[4] = 2;
  xyzindex1[5] = 3;

  xyzindex2[0] = 0;
  xyzindex2[1] = 1;
  xyzindex2[2] = 3;
  xyzindex2[3] = 1;
  xyzindex2[4] = 2;
  xyzindex2[5] = 3;

  is_firenode = meshi->is_firenode;

  if(cullfaces==1)glDisable(GL_CULL_FACE);

  TransparentOn();
  switch(ssmokedir){
    unsigned char *smokealpha_map, *firealpha_map;

    // +++++++++++++++++++++++++++++++++++ DIR 1 +++++++++++++++++++++++++++++++++++++++

  case 1:
  case -1:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_X];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_X];
    for(i = is1;i<=is2;i++){
      if(demo_mode == 5 && ssmokedir == 1) {
        i+=2;
        if(i > is2) continue;
      }
      iterm = (i - smoke3di->is1);
      for(k = ks1; k<=ks2; k++){
        kterm = (k-ks1)*nxy;
        for(j = js1;j<=js2;j++){
          jterm = (j-js1)*nx;
          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++
    glBegin(GL_TRIANGLES);
    slice_beg = is1;
    slice_end = is2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      if(demo_mode == 5 && ssmokedir == 1){
        iii+=2;
        if(iii>=slice_end)continue;
      }
      i = iii;
      if(ssmokedir<0)i = is1+is2-iii-1;
      iterm = (i-smoke3di->is1);
      constval = xplt[i]+0.001;
      for(k = ks1; k<ks2; k+=smoke3d_skipz){
        int k2, koffset;

        k2 = MIN(k+smoke3d_skipz,ks2);
        koffset = k2 - k;

        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k2];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;
        for(j = js1; j<js2; j+=smoke3d_skipy){
          int j2, joffset;

          j2 = MIN(j+smoke3d_skipy,js2);
          joffset = j2 - j;

          jterm = (j-js1)*nx;
          yy1 = yplt[j];
          y3 = yplt[j2];
          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          n = iterm+jterm+kterm;

          n11 = n;                    //n
          n12 = n11+joffset*nx;       //n+nx
          n22 = n12+koffset*nxy;      //n+nx+nxy
          n21 = n22-joffset*nx;       //n+nxy

          //        n11 = (i-is1)   + (j  -js1)*nx + (k  -ks1)*nx*ny;
          //        n12 = (i-is1)   + (j+1-js1)*nx + (k  -ks1)*nx*ny;
          //        n22 = (i-is1)   + (j+1-js1)*nx + (k+1-ks1)*nx*ny;
          //        n21 = (i-is1)   + (j  -js1)*nx + (k+1-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+joffset*nx;
            m22 = m12;
            m21 = m22-joffset*nx;
            DRAWVERTEXTERRAIN(constval, ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEX(constval, ynode[mm], znode[mm])
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();

    break;

    // +++++++++++++++++++++++++++++++++++ DIR 2 +++++++++++++++++++++++++++++++++++++++

  case 2:
  case -2:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_Y];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_Y];
    for(j = js1;j<=js2;j++){
      jterm = (j-js1)*nx;
      for(k = ks1;k<=ks2;k++){
        kterm = (k-ks1)*nxy;
        for(i = is1;i<=is2;i++){
          iterm = (i-is1);
          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = js1;
    slice_end = js2;
    for(jjj = slice_beg;jjj<slice_end;jjj += skip_local){
      j = jjj;
      if(ssmokedir<0)j = js1+js2-jjj-1;
      constval = yplt[j]+0.001;
      jterm = (j-js1)*nx;
      for(k = ks1; k<ks2; k+=smoke3d_skipz){
        int k2, koffset;

        k2 = MIN(k+smoke3d_skipz,ks2);
        koffset = k2 - k;
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k2];

        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;
        for(i = is1; i<is2; i+=smoke3d_skipx){
          int i2,ioffset;

          i2 = MIN(i+smoke3d_skipx,is2);
          ioffset = i2 - i;
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i2];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          n = iterm+jterm+kterm;
          n11 = n;                //n
          n12 = n11+ioffset;      //n+1
          n22 = n12+koffset*nxy;  //n+1+nxy
          n21 = n22-ioffset;      //n+nxy

          //        n11 = (i-is1)   + (j-js1)*nx   + (k-ks1)*nx*ny;
          //        n12 = (i+1-is1) + (j-js1)*nx   + (k-ks1)*nx*ny;
          //        n22 = (i+1-is1) + (j-js1)*nx   + (k+1-ks1)*nx*ny;
          //        n21 = (i-is1)   + (j-js1)*nx   + (k+1-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+ioffset;
            m22 = m12;
            m21 = m22-ioffset;
            DRAWVERTEXTERRAIN(xnode[mm], constval, znode[mm])
          }
          else{
            DRAWVERTEX(xnode[mm], constval, znode[mm])
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 3 +++++++++++++++++++++++++++++++++++++++

  case 3:
  case -3:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_Z];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_Z];
    for(k = ks1;k<=ks2;k++){
      kterm = (k-ks1)*nxy;
      for(j = js1;j<=js2;j++){
        jterm = (j-js1)*nx;
        for(i = is1;i<=is2;i++){
          iterm = (i-is1);
          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = ks1;
    slice_end = ks2;
    for(kkk = slice_beg;kkk<slice_end;kkk += skip_local){
      k = kkk;
      if(ssmokedir<0)k = ks1+ks2-kkk-1;
      constval = zplt[k]+0.001;
      kterm = (k-ks1)*nxy;
      for(j = js1; j<js2; j+=smoke3d_skipy){
        int j2,joffset;

        j2 = MIN(j+smoke3d_skipy,js2);
        joffset = j2 - j;
        jterm = (j-js1)*nx;

        yy1 = yplt[j];
        y3 = yplt[j2];

        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;
        for(i = is1; i<is2; i+=smoke3d_skipx){
          int i2,ioffset;

          i2 = MIN(i+smoke3d_skipx,is2);
          ioffset = i2 - i;
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i2];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          n = iterm+jterm+kterm;
          n11 = n;
          n12 = n11+ioffset;
          n22 = n12+joffset*nx;
          n21 = n22-ioffset;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+ioffset;
            m22 = m12+joffset*nx;
            m21 = m22-ioffset;
            DRAWVERTEXTERRAIN(xnode[mm], ynode[mm], constval)
          }
          else{
            DRAWVERTEX(xnode[mm], ynode[mm], constval)
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 4 +++++++++++++++++++++++++++++++++++++++

  case 4:
  case -4:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_XY];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_XY];
    for(iii = 1;iii<nx+ny-2;iii += skip_local){
      int ipj;

      ipj = iii;
      if(ssmokedir<0)ipj = nx+ny-2-iii;
      ibeg = 0;
      jbeg = ipj;
      if(jbeg>ny-1){
        jbeg = ny-1;
        ibeg = ipj-jbeg;
      }
      iend = nx-1;
      jend = ipj-iend;
      if(jend<0){
        jend = 0;
        iend = ipj-jend;
      }
      for(k = ks1;k<=ks2;k++){
        kterm = (k-ks1)*nxy;
        for(ii = ibeg;ii<=iend;ii++){
          i = is1+ii;
          iterm = (i-is1);

          jj = ipj-ii;
          j = js1+jj;
          jterm = (j-js1)*nx;

          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = nx+ny-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int ipj;

      ipj = iii;
      if(ssmokedir<0)ipj = nx+ny-2-iii;
      ibeg = 0;
      jbeg = ipj;
      if(jbeg>ny-1){
        jbeg = ny-1;
        ibeg = ipj-jbeg;
      }
      iend = nx-1;
      jend = ipj-iend;
      if(jend<0){
        jend = 0;
        iend = ipj-jend;
      }
      for(k = ks1; k<ks2; k++){
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k+1];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;
        for(ii = ibeg;ii<iend;ii++){
          i = is1+ii;
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i+1];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          jj = ipj-ii;
          j = js1+jj;
          jterm = (j-js1)*nx;

          yy1 = yplt[j];
          y3 = yplt[j-1];

          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          n11 = iterm+jterm+kterm;
          n12 = n11-nx+1;
          n22 = n12+nxy;
          n21 = n11+nxy;

          //        n11 = (j  -js1)*nx + (i  -is1) + (k  -ks1)*nx*ny;
          //        n12 = (j-1-js1)*nx + (i+1-is1) + (k  -ks1)*nx*ny;
          //        n22 = (j-1-js1)*nx + (i+1-is1) + (k+1-ks1)*nx*ny;
          //        n21 = (j  -js1)*nx + (i  -is1) + (k+1-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11-nx+1;
            m22 = m12;
            m21 = m11;
            DRAWVERTEXTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEX(xnode[mm], ynode[mm], znode[mm])
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 5 +++++++++++++++++++++++++++++++++++++++

  case 5:
  case -5:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_XY];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_XY];
    for(iii = 1;iii<nx+ny-2;iii += skip_local){
      int jmi;

      jmi = iii;
      if(ssmokedir<0)jmi = nx+ny-2-iii;

      ibeg = 0;
      jbeg = ibeg-nx+1+jmi;
      if(jbeg<0){
        jbeg = 0;
        ibeg = jbeg+nx-1-jmi;
      }
      iend = nx-1;
      jend = iend+jmi+1-nx;
      if(jend>ny-1){
        jend = ny-1;
        iend = jend+nx-1-jmi;
      }
      for(k = ks1;k<=ks2;k++){
        kterm = (k-ks1)*nxy;
        for(ii = ibeg;ii<=iend;ii++){
          i = is1+ii;
          iterm = (i-is1);

          jj = ii+jmi+1-nx;
          j = js1+jj;
          jterm = (j-js1)*nx;

          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = nx+ny-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int jmi;

      jmi = iii;
      if(ssmokedir<0)jmi = nx+ny-2-iii;

      ibeg = 0;
      jbeg = ibeg-nx+1+jmi;
      if(jbeg<0){
        jbeg = 0;
        ibeg = jbeg+nx-1-jmi;
      }
      iend = nx-1;
      jend = iend+jmi+1-nx;
      if(jend>ny-1){
        jend = ny-1;
        iend = jend+nx-1-jmi;
      }
      for(k = ks1; k<ks2; k++){
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k+1];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;
        for(ii = ibeg;ii<iend;ii++){
          i = is1+ii;
          iterm = (i-is1);

          jj = ii+jmi+1-nx;
          j = js1+jj;
          jterm = (j-js1)*nx;

          yy1 = yplt[j];
          y3 = yplt[j+1];

          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          x1 = xplt[i];
          x3 = xplt[i+1];
          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;


          n11 = jterm+iterm+kterm;
          n12 = n11+nx+1;
          n22 = n12+nxy;
          n21 = n11+nxy;

          //    n11 = (j  -js1)*nx + (i  -is1) + (k  -ks1)*nx*ny;
          //    n12 = (j+1-js1)*nx + (i+1-is1) + (k  -ks1)*nx*ny;
          //    n22 = (j+1-js1)*nx + (i+1-is1) + (k+1-ks1)*nx*ny;
          //    n21 = (j  -js1)*nx + (i  -is1) + (k+1-ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+nx+1;
            m22 = m12;
            m21 = m11-nx;
            DRAWVERTEXTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEX(xnode[mm], ynode[mm], znode[mm])
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 6 +++++++++++++++++++++++++++++++++++++++

  case 6:
  case -6:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_YZ];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_YZ];
    for(iii = 1;iii<ny+nz-2;iii += skip_local){
      int jpk;

      jpk = iii;
      if(ssmokedir<0)jpk = ny+nz-2-iii;
      jbeg = 0;
      kbeg = jpk;
      if(kbeg>nz-1){
        kbeg = nz-1;
        jbeg = jpk-kbeg;
      }
      jend = ny-1;
      kend = jpk-jend;
      if(kend<0){
        kend = 0;
        jend = jpk-kend;
      }
      for(i = is1;i<=is2;i++){
        iterm = (i-is1);
        for(jj = jbeg;jj<=jend;jj++){
          j = js1+jj;
          jterm = (j-js1)*nx;

          kk = jpk-jj;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = ny+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int jpk;

      jpk = iii;
      if(ssmokedir<0)jpk = ny+nz-2-iii;
      jbeg = 0;
      kbeg = jpk;
      if(kbeg>nz-1){
        kbeg = nz-1;
        jbeg = jpk-kbeg;
      }
      jend = ny-1;
      kend = jpk-jend;
      if(kend<0){
        kend = 0;
        jend = jpk-kend;
      }
      for(i = is1; i<is2; i++){
        iterm = (i-is1);
        x1 = xplt[i];
        x3 = xplt[i+1];
        xnode[0] = x1;
        xnode[1] = x1;
        xnode[2] = x3;
        xnode[3] = x3;
        for(jj = jbeg;jj<jend;jj++){
          j = js1+jj;
          jterm = (j-js1)*nx;
          yy1 = yplt[j];
          y3 = yplt[j+1];

          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          kk = jpk-jj;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          z1 = zplt[k];
          z3 = zplt[k-1];

          znode[0] = z1;
          znode[1] = z3;
          znode[2] = z3;
          znode[3] = z1;

          n11 = iterm+jterm+kterm;
          n12 = n11+nx-nxy;
          n22 = n12+1;
          n21 = n22-nx+nxy;

          //        n11 = (i  -is1) + (j  -js1)*nx + (k  -ks1)*nx*ny;
          //        n12 = (i  -is1) + (j+1-js1)*nx + (k-1-ks1)*nx*ny;
          //        n22 = (i+1-is1) + (j+1-js1)*nx + (k-1-ks1)*nx*ny;
          //        n21 = (i+1-is1) + (j  -js1)*nx + (k  -ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+nx;
            m22 = m12+1;
            m21 = m22-nx;
            DRAWVERTEXTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEX(xnode[mm], ynode[mm], znode[mm])
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 7 +++++++++++++++++++++++++++++++++++++++

  case 7:
  case -7:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_YZ];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_YZ];
    for(iii = 1;iii<ny+nz-2;iii += skip_local){
      int kmj;

      kmj = iii;
      if(ssmokedir<0)kmj = ny+nz-2-iii;

      jbeg = 0;
      kbeg = jbeg-ny+1+kmj;
      if(kbeg<0){
        kbeg = 0;
        jbeg = kbeg+ny-1-kmj;
      }
      jend = ny-1;
      kend = jend+kmj+1-ny;
      if(kend>nz-1){
        kend = nz-1;
        jend = kend+ny-1-kmj;
      }
      for(i = is1;i<=is2;i++){
        iterm = (i-is1);
        for(jj = jbeg;jj<=jend;jj++){
          j = js1+jj;
          jterm = (j-js1)*nx;

          kk = jj+kmj+1-ny;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = ny+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int kmj;

      kmj = iii;
      if(ssmokedir<0)kmj = ny+nz-2-iii;

      jbeg = 0;
      kbeg = jbeg-ny+1+kmj;
      if(kbeg<0){
        kbeg = 0;
        jbeg = kbeg+ny-1-kmj;
      }
      jend = ny-1;
      kend = jend+kmj+1-ny;
      if(kend>nz-1){
        kend = nz-1;
        jend = kend+ny-1-kmj;
      }
      for(i = is1; i<is2; i++){
        iterm = (i-is1);
        x1 = xplt[i];
        x3 = xplt[i+1];
        xnode[0] = x1;
        xnode[1] = x1;
        xnode[2] = x3;
        xnode[3] = x3;
        for(jj = jbeg;jj<jend;jj++){
          j = js1+jj;
          jterm = (j-js1)*nx;

          kk = jj+kmj+1-ny;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          z1 = zplt[k];
          z3 = zplt[k+1];

          znode[0] = z1;
          znode[1] = z3;
          znode[2] = z3;
          znode[3] = z1;

          yy1 = yplt[j];
          y3 = yplt[j+1];
          ynode[0] = yy1;
          ynode[1] = y3;
          ynode[2] = y3;
          ynode[3] = yy1;

          n11 = jterm+iterm+kterm;
          n12 = n11+nxy+nx;
          n22 = n12+1;
          n21 = n22-nx-nxy;

          //    n11 = (i  -is1) + (j  -js1)*nx  + (k  -ks1)*nx*ny;
          //    n12 = (i  -is1) + (j+1-js1)*nx  + (k+1-ks1)*nx*ny;
          //    n22 = (i+1-is1) + (j+1-js1)*nx  + (k+1-ks1)*nx*ny;
          //    n21 = (i+1-is1) + (j  -js1)*nx  + (k  -ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+nx;
            m22 = m12+1;
            m21 = m22-nx;
            DRAWVERTEXTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEX(xnode[mm], ynode[mm], znode[mm])
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 8 +++++++++++++++++++++++++++++++++++++++

  case 8:
  case -8:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_XZ];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_XZ];
    for(iii = 1;iii<nx+nz-2;iii += skip_local){
      int ipk;

      ipk = iii;
      if(ssmokedir<0)ipk = nx+nz-2-iii;
      ibeg = 0;
      kbeg = ipk;
      if(kbeg>nz-1){
        kbeg = nz-1;
        ibeg = ipk-kbeg;
      }
      iend = nx-1;
      kend = ipk-iend;
      if(kend<0){
        kend = 0;
        iend = ipk-kend;
      }
      for(j = js1;j<=js2;j++){
        jterm = (j-js1)*nx;
        for(ii = ibeg;ii<=iend;ii++){
          i = is1+ii;
          iterm = (i-is1);

          kk = ipk-ii;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = nx+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int ipk;

      ipk = iii;
      if(ssmokedir<0)ipk = nx+nz-2-iii;
      ibeg = 0;
      kbeg = ipk;
      if(kbeg>nz-1){
        kbeg = nz-1;
        ibeg = ipk-kbeg;
      }
      iend = nx-1;
      kend = ipk-iend;
      if(kend<0){
        kend = 0;
        iend = ipk-kend;
      }
      for(j = js1; j<js2; j++){
        jterm = (j-js1)*nx;
        yy1 = yplt[j];
        y3 = yplt[j+1];
        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;
        for(ii = ibeg;ii<iend;ii++){
          i = is1+ii;
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i+1];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          kk = ipk-ii;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          z1 = zplt[k];
          z3 = zplt[k-1];

          znode[0] = z1;
          znode[1] = z3;
          znode[2] = z3;
          znode[3] = z1;

          n11 = iterm+jterm+kterm;
          n12 = n11+1-nxy;
          n22 = n12+nx;
          n21 = n22-1+nxy;

          //        n11 = (i  -is1) + (j  -js1)*nx + (k  -ks1)*nx*ny;
          //        n12 = (i+1-is1) + (j  -js1)*nx + (k-1-ks1)*nx*ny;
          //        n22 = (i+1-is1) + (j+1-js1)*nx + (k-1-ks1)*nx*ny;
          //        n21 = (i  -is1) + (j+1-js1)*nx + (k  -ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+1;
            m22 = m12+nx;
            m21 = m22-1;
            DRAWVERTEXTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEX(xnode[mm], ynode[mm], znode[mm])
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 9 +++++++++++++++++++++++++++++++++++++++

  case 9:
  case -9:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    smokealpha_map = smoke3di->alphas_smokedir[ALPHA_XZ];
    firealpha_map  = smoke3di->alphas_firedir[ALPHA_XZ];
    for(iii = 1;iii<nx+nz-2;iii += skip_local){
      int kmi;

      kmi = iii;
      if(ssmokedir<0)kmi = nx+nz-2-iii;

      ibeg = 0;
      kbeg = ibeg-nx+1+kmi;
      if(kbeg<0){
        kbeg = 0;
        ibeg = kbeg+nx-1-kmi;
      }
      iend = nx-1;
      kend = iend+kmi+1-nx;
      if(kend>nz-1){
        kend = nz-1;
        iend = kend+nx-1-kmi;
      }
      for(j = js1;j<=js2;j++){
        jterm = (j-js1)*nx;
        for(ii = ibeg;ii<=iend;ii++){
          i = is1+ii;
          iterm = (i-is1);

          kk = ii+kmi+1-nx;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          n = iterm+jterm+kterm;
          assert(n>=0&&n<smoke3di->nchars_uncompressed);
          ADJUSTALPHA(smokealpha_ptr[n]);
        }
      }
    }
    alphaf_ptr = alphaf_out;

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = nx+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      int kmi;

      kmi = iii;
      if(ssmokedir<0)kmi = nx+nz-2-iii;

      ibeg = 0;
      kbeg = ibeg-nx+1+kmi;
      if(kbeg<0){
        kbeg = 0;
        ibeg = kbeg+nx-1-kmi;
      }
      iend = nx-1;
      kend = iend+kmi+1-nx;
      if(kend>nz-1){
        kend = nz-1;
        iend = kend+nx-1-kmi;
      }
      for(j = js1; j<js2; j++){
        jterm = (j-js1)*nx;
        yy1 = yplt[j];
        y3 = yplt[j+1];
        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;
        for(ii = ibeg;ii<iend;ii++){
          i = is1+ii;
          iterm = (i-is1);

          kk = ii+kmi+1-nx;
          k = ks1+kk;
          kterm = (k-ks1)*nxy;

          z1 = zplt[k];
          z3 = zplt[k+1];

          znode[0] = z1;
          znode[1] = z3;
          znode[2] = z3;
          znode[3] = z1;

          x1 = xplt[i];
          x3 = xplt[i+1];
          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;


          n11 = jterm+iterm+kterm;
          n12 = n11+nxy+1;
          n22 = n12+nx;
          n21 = n22-1-nxy;

          //    n11 = (i  -is1) + (j  -js1)*nx + (k  -ks1)*nx*ny;
          //    n12 = (i+1-is1) + (j  -js1)*nx + (k+1-ks1)*nx*ny;
          //    n22 = (i+1-is1) + (j+1-js1)*nx + (k+1-ks1)*nx*ny;
          //    n21 = (i  -is1) + (j+1-js1)*nx + (k  -ks1)*nx*ny;

          if(global_scase.nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
            int m11, m12, m22, m21;

            m11 = iterm+jterm;
            m12 = m11+1;
            m22 = m12+nx;
            m21 = m22-1;
            DRAWVERTEXTERRAIN(xnode[mm], ynode[mm], znode[mm])
          }
          else{
            DRAWVERTEX(xnode[mm], ynode[mm], znode[mm])
          }
          nsmoke_triangles += 2;
        }
      }
    }
    glEnd();
    break;
  default:
    assert(FFALSE);
    break;
  }
  TransparentOff();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
  return nsmoke_triangles;
}

/* ------------------ GetFireMinMax ------------------------ */

void GetFireMinMax(char *cb_label, float *firemin, float *firemax, float *firemin_cb, float *firemax_cb, float *i_min, float *i_max){
  if(have_fire==HRRPUV_index){
    *firemin_cb = global_hrrpuv_cb_min;
    *firemax_cb = global_hrrpuv_cb_max;
    *firemin    = global_scase.hrrpuv_min;
    *firemax    = global_scase.hrrpuv_max;
    strcpy(cb_label, "HRRPUV");
  }
  else if(have_fire==TEMP_index){
    *firemin_cb = global_temp_cb_min;
    *firemax_cb = global_temp_cb_max;
    *firemin    = global_scase.temp_min;
    *firemax    = global_scase.temp_max;
    strcpy(cb_label, "Temperature");
  }
  else{
    *firemin_cb = 0.0;
    *firemax_cb = 1.0;
    *firemin    = 0.0;
    *firemax    = 1.0;
    strcpy(cb_label, "");
    return;
  }
  float denom;

  denom = *firemax - *firemin;
  if(denom == 0.0)denom = 1.0;
  *i_min = (*firemin_cb - *firemin) / denom;
  *i_min = CLAMP(*i_min, 0.0, 1.0);
  *i_max = (*firemax_cb - *firemin) / denom;
  *i_max = CLAMP(*i_max, 0.0, 1.0);
}

/* ------------------ DrawSmoke3DColorMap ------------------------ */

#define CB_SPACE 0.005

void DrawSmoke3DColorMap(void){
  int i;
  float yleft,  yright;
  float yleft2, yright2;
  float ybot, ytop;
  float *fire_cb;
  char label[32];

  fire_cb = colorbars.colorbarinfo[colorbars.fire_colorbar_index].colorbar_rgb;
  yleft   = FDS2SMV_X(xbarFDS);
  yleft  += 0.05;
  yright  = yleft  + 0.1;
  yleft2  = yright + 0.075;
  yright2 = yleft2 + 0.1;

  glBegin(GL_QUADS);
  for(i = 0; i < 255; i++){
    float *rgb;

    ybot = (float)i/255.0;
    ytop = (float)(i+1)/255.0;

    rgb = rgb_slicesmokecolormap_01 + 4 * i;
    glColor4fv(rgb);
    glVertex3f(yleft,  0.0, ybot);
    glVertex3f(yright, 0.0, ybot);
    glVertex3f(yright, 0.0, ytop);
    glVertex3f(yleft,  0.0, ytop);
  }
  if(use_fire_colormap==1){
    float fire_color_local[3];

    fire_color_local[0] = fire_color_int255[0]/255.0;
    fire_color_local[1] = fire_color_int255[1]/255.0;
    fire_color_local[2] = fire_color_int255[2]/255.0;
    glColor3fv(fire_color_local);
  }
  for(i = 0; i < 255; i++){

    ybot = (float)i / 255.0;
    ytop = (float)(i + 1) / 255.0;

    if(use_fire_colormap==1)glColor3fv(fire_cb + 3*i);
    glVertex3f(yleft2,  0.0, ybot);
    glVertex3f(yright2, 0.0, ybot);
    glVertex3f(yright2, 0.0, ytop);
    glVertex3f(yleft2,  0.0, ytop);
  }
  if(update_fire_histogram == 1){
    glColor3f(0.0,0.0,1.0);
    for(i = 0; i < 255; i++){

      ybot = (float)i / 255.0;
      ytop = (float)(i + 1) / 255.0;

      glVertex3f(yright2, 0.0, ybot);
      glVertex3f(yright2+smoke3d_firevals[i]/3.0, 0.0, ybot);
      glVertex3f(yright2+smoke3d_firevals[i]/3.0, 0.0, ytop);
      glVertex3f(yright2, 0.0, ytop);
    }
  }
  glEnd();

  float firemin_cb, firemax_cb;
  float firemin,    firemax;
  float imin,       imax;
  char cb_label[32];

  GetFireMinMax(cb_label, &firemin, &firemax, &firemin_cb, &firemax_cb, &imin, &imax);

  sprintf(label, "%f", firemin);
  TrimZeros(label);
  Output3Text(foregroundcolor, yright + CB_SPACE, 0.0, 0.0, label);

  sprintf(label, "%f", firemin_cb);
  TrimZeros(label);
  Output3Text(foregroundcolor, yright + CB_SPACE, 0.0, imin, label);

  sprintf(label, "%f", firemax_cb);
  TrimZeros(label);
  Output3Text(foregroundcolor, yright + CB_SPACE, 0.0, imax, label);

  sprintf(label, "%f", firemax);
  TrimZeros(label);
  Output3Text(foregroundcolor, yright + CB_SPACE, 0.0, 1.0, label);

  Output3Text(foregroundcolor, yleft, 0.0, 1.0175,  cb_label);
  Output3Text(foregroundcolor, yleft, 0.0, 1.005 , "color");
  glBegin(GL_LINES);
  glVertex3f(yleft2,  0.0, (float)global_cb_max_index/255.0);
  glVertex3f(yright,  0.0,  imax);
  glVertex3f(yleft2,  0.0, (float)global_cb_min_index / 255.0);
  glVertex3f(yright,  0.0,  imin);
  glEnd();
}

/* ------------------ DrawSmokeFrame ------------------------ */

void DrawSmokeFrame(void){
  // options:
  // SMOKE3D_FIRE_ONLY      0
  // SMOKE3D_SMOKE_ONLY     1
  // SMOKE3D_SMOKE_AND_FIRE 2

  int load_shaders = 0;
  int i;
  int blend_mode;
  int nsmoke_triangles=0;
  int usegpu_local;

  if(use_tload_begin==1 && global_times[itimes]<global_scase.tload_begin)return;
  if(use_tload_end==1   && global_times[itimes]>global_scase.tload_end)return;

  usegpu_local = usegpu;
  triangle_count = 0;
#ifdef pp_GPU
  if(usegpu_local == 1) {
    LoadSmokeShaders();
    load_shaders = 1;
  }
#endif

  float smoke3d_timer;
  START_TIMER(smoke3d_timer);
  blend_mode = 0;
  if(usegpu_local==0&&hrrpuv_max_blending==1){
    blend_mode = 1;
    glBlendEquation(GL_MAX);
  }
  for(i = 0; i<global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    meshdata *smokemesh;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo_sorted[i];
    if(smoke3di->loaded==0||smoke3di->display==0)continue;
    if(smoke3di->primary_file==0)continue;
    IF_NOT_USEMESH_CONTINUE(USEMESH_DRAW,smoke3di->blocknumber);
    smokemesh = global_scase.meshescoll.meshinfo + smoke3di->blocknumber;
    if(smokemesh->in_frustum == 0)continue;
    if(smokemesh->datavis == 0)continue;
    if(IsSmokeComponentPresent(smoke3di)==0)continue;
    if(smoke3d_use_skip==1){
      if(smoke3di->smokeframe_loaded==NULL){
        NewMemory((void **)&smoke3di->smokeframe_loaded, smoke3di->ntimes_full*sizeof(int));
        int j;
        for(j=0;j<smoke3di->ntimes_full;j++){
          smoke3di->smokeframe_loaded[j] = 0;
        }
        for(j=smoke3d_start_frame;j<smoke3di->ntimes_full;j+=smoke3d_skip_frame){
          smoke3di->smokeframe_loaded[j] = 1;
        }
      }
      if(smoke3di->smokeframe_loaded!=NULL&&smoke3di->smokeframe_loaded[smoke3di->ismoke3d_time]==0)continue;
    }
#ifdef pp_GPU
    if(usegpu_local == 1) {
      DrawSmoke3DGPU(smoke3di);
      nsmoke_triangles = -1;
    }
    else{
      nsmoke_triangles += DrawSmoke3D(smoke3di);
    }
#else
    nsmoke_triangles += DrawSmoke3D(smoke3di);
#endif
  }
  STOP_TIMER(smoke3d_timer);
  if(nsmoke_triangles>0&&show_trirates==1&&smoke3d_timer>0.0){
    float tri_fps = -1.0;

    printf("3D smoke ");
    tri_fps = (float)nsmoke_triangles / smoke3d_timer;
    if(nsmoke_triangles > 1000000000){
      PRINTF("triangles: %.1f G, ", (float)nsmoke_triangles/1000000000.0);
    }
    else if(nsmoke_triangles > 1000000){
      PRINTF("triangles: %.1f M, ", (float)nsmoke_triangles/1000000.0);
    }
    else if(nsmoke_triangles >1000){
      PRINTF("triangles: %.1f K, ", (float)nsmoke_triangles/1000.0);
    }
    else{
      PRINTF("triangles: %i, ", nsmoke_triangles);
    }
    if(tri_fps > 1000000000){
      PRINTF("rate: %.1f Gtri/s, ", (float)tri_fps/1000000000.);
    }
    else if(tri_fps > 1000000){
      PRINTF("rate: %.1f Mtri/s, ", (float)tri_fps/1000000.);
    }
    else{
      PRINTF("rate: %.0f Ktri/s, ", (float)tri_fps/1000.);
    }
    printf("time: %.1f s,", smoke3d_timer);
    printf("\n");
  }
  if(blend_mode==1){
    glBlendEquation(GL_FUNC_ADD);
  }
#ifdef pp_GPU
  if(load_shaders==1){
    UnLoadShaders();
  }
#endif
  if(have_fire != NO_FIRE && show_smoke3d_colorbar == 1){
    DrawSmoke3DColorMap();
  }
  SNIFF_ERRORS("after drawsmoke");
}

/* ------------------ DrawSmokeVolFrame ------------------------ */

void DrawVolSmokeFrame(void){
  int load_shaders = 0;

  if(use_tload_begin==1&&global_times[itimes]<global_scase.tload_begin)return;
  if(use_tload_end==1&&global_times[itimes]>global_scase.tload_end)return;
  triangle_count = 0;
  CheckMemory;
  if(smoke3dVoldebug==1){
    DrawSmoke3dVolDebug();
  }
#ifdef pp_GPU
  if(usegpu==1){
    LoadVolsmokeShaders();
    load_shaders = 1;
  }
#endif
#ifdef pp_GPU
  if(usegpu==1){
    //  glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    SNIFF_ERRORS("before DrawSmoke3dGpuVol");
    DrawSmoke3DGPUVol();
    SNIFF_ERRORS("after DrawSmoke3dGpuVol");
    //  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  }
  else{
    DrawSmoke3DVol();
    SNIFF_ERRORS("after DrawSmoke3dVol");
  }
#else
  DrawSmoke3DVol();
#endif
#ifdef pp_GPU
  if(load_shaders==1){
    UnLoadShaders();
  }
#endif
  SNIFF_ERRORS("after drawsmoke");
}

/* ------------------ SkipSmokeFrames ------------------------ */

void SkipSmokeFrames(MFILE *SMOKE3DFILE, smoke3ddata *smoke3di, int nsteps){
  int i;
  int skip_local = 0;

  if(nsteps==0)return;
  skip_local = 4+8*4+4;            // header
  for(i = 0; i<nsteps; i++){
    skip_local += 4+4+4;           // time
    skip_local += 4+2*4+4;         // size
    skip_local += 4+smoke3di->nchars_compressed_smoke[i]+4;
  }
  FSEEK_SMOKE(SMOKE3DFILE, skip_local, SEEK_SET);
}

/* ------------------ GetSmokeFileSize ------------------------ */

FILE *GetSmokeFileSize(char *smokefile, int fortran_skip, int version){
  FILE *SMOKE_SIZE;
  MFILE *SMOKE3DFILE;
  char smoke_sizefilename[1024];
  int lentext;
  char *textptr;
  int nxyz[8];
  int nchars[2];
  float time_local;
  int skip_local;

  // try .sz
  strcpy(smoke_sizefilename, smokefile);
  lentext = strlen(smoke_sizefilename);
  if(lentext > 4){
    textptr = smoke_sizefilename + lentext - 4;
    if(strcmp(textptr, ".svz") == 0){
      smoke_sizefilename[lentext - 4] = 0;
    }
  }
  strcat(smoke_sizefilename, ".szz");
  SMOKE_SIZE = FOPEN_2DIR(smoke_sizefilename, "r");

  if(SMOKE_SIZE == NULL){
    // not .szz so try .sz
    strcpy(smoke_sizefilename, smokefile);
    strcat(smoke_sizefilename, ".sz");
    SMOKE_SIZE = FOPEN_2DIR(smoke_sizefilename, "r");
  }
  if(SMOKE_SIZE != NULL)return SMOKE_SIZE;

  // wasn't able to read the size file so try creating a new one

  strcpy(smoke_sizefilename, smokefile);
  strcat(smoke_sizefilename, ".sz");
  SMOKE_SIZE = FOPEN_2DIR(smoke_sizefilename, "w");
  if(SMOKE_SIZE == NULL){
    printf("***error: was not able to read the size file for %s\n", smokefile);
    printf("          and was not able to create a new size file: %s\n", smoke_sizefilename);
    return NULL;  // can't write size file in temp directory so give up
  }
  SMOKE3DFILE = fopen(smokefile, "rb");
  if(SMOKE3DFILE == NULL){
    fclose(SMOKE_SIZE);
    return NULL;
  }

  SKIP_SMOKE; FREAD_SMOKE(nxyz, 4, 8, SMOKE3DFILE); SKIP_SMOKE;

  if(version != 1)version = 0;
  fprintf(SMOKE_SIZE, "%i\n", version);

  for(;;){
    int nframeboth;
    size_t count;

    SKIP_SMOKE; count=FREAD_SMOKE(&time_local, 4, 1, SMOKE3DFILE); SKIP_SMOKE;
    if(count!=1||FEOF_SMOKE(SMOKE3DFILE) != 0)break;
    SKIP_SMOKE; FREAD_SMOKE(nchars, 4, 2, SMOKE3DFILE); SKIP_SMOKE;
    if(version == 0){ // uncompressed data
      fprintf(SMOKE_SIZE, "%f %i %i\n", time_local, nchars[0], nchars[1]);
    }
    else{  // compressed data
      int nlightdata;

      // time, nframeboth, ncompressed_rle, ncompressed_zlib, nlightdata
      // ncompessed_zlib and nlightdata are negative if there is radiance data present

      if(nchars[1] < 0){  // light data present
        nframeboth = nchars[0];
        nlightdata = -nchars[0] / 2;
        fprintf(SMOKE_SIZE, "%f %i %i %i %i \n", time_local, nframeboth, -1, nchars[1], nlightdata);
      }
      else{
        nframeboth = nchars[0];
        nlightdata = 0;
        fprintf(SMOKE_SIZE, "%f %i %i %i %i \n", time_local, nframeboth, -1, nchars[1], nlightdata);
      }
    }
    skip_local = ABS(nchars[1]);
    SKIP_SMOKE; FSEEK_SMOKE(SMOKE3DFILE, skip_local, SEEK_CUR); SKIP_SMOKE;
  }

  FCLOSE_SMOKE(SMOKE3DFILE);
  fclose(SMOKE_SIZE);
  SMOKE_SIZE = FOPEN_2DIR(smoke_sizefilename, "r");
  return SMOKE_SIZE;
}

/* ------------------ GetSmoke3DTimeSteps ------------------------ */

void GetSmoke3DTimeSteps(int fortran_skip, char *smokefile, int version, int *ntimes_found, int *ntimes_full){
  char buffer[255];
  FILE *SMOKE_SIZE = NULL;
  int nframes_found;
  float time_local, time_max;
  int iframe_local;
  int iii;

  *ntimes_found = 0;
  *ntimes_full = 0;

  SMOKE_SIZE = GetSmokeFileSize(smokefile,fortran_skip,version);
  if(SMOKE_SIZE == NULL)return;

  nframes_found = 0;
  iframe_local = -1;
  time_max = -1000000.0;
  fgets(buffer, 255, SMOKE_SIZE);
  iii = 0;
  while(!feof(SMOKE_SIZE)){
    if(fgets(buffer, 255, SMOKE_SIZE) == NULL)break;
    sscanf(buffer, "%f", &time_local);
    iframe_local++;
    if(time_local <= time_max)continue;
    if(use_tload_end == 1 && time_local > global_scase.tload_end)break;
    if(iii%tload_step == 0 && (use_tload_begin == 0 || time_local >= global_scase.tload_begin)){
      nframes_found++;
      time_max = time_local;
    }
    iii++;
  }
  if(nframes_found > 0){
    *ntimes_found = nframes_found;
    *ntimes_full = iframe_local + 1;
  }
  fclose(SMOKE_SIZE);
}

/* ------------------ GetSmokeNFrames ------------------------ */

int GetSmokeNFrames(int type, float *tmin, float *tmax){
  FILE *stream;
  char buffer[255];
  int i, nframes;
  char size_file[256];

  nframes = 0;
  *tmin = 1.0;
  *tmax = 0.0;
  for(i = 0;i < global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    int nf;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->type == SOOT_index   && (type&1) == 0)continue;
    if(smoke3di->type == HRRPUV_index && (type&2) == 0)continue;
    if(smoke3di->type == TEMP_index   && (type&4) == 0)continue;
    if(smoke3di->type == CO2_index    && (type&8) == 0)continue;
    strcpy(size_file, smoke3di->file);
    strcat(size_file,".sz");
    stream = FOPEN_2DIR(size_file, "r");
    if(stream == NULL)continue;
    nf = 0;
    fgets(buffer, 255, stream);
    while(!feof(stream)){
      float t;
      if(fgets(buffer, 255, stream) == NULL)break;
      sscanf(buffer, "%f", &t);
      if(*tmin>*tmax){
        *tmin = t;
        *tmax = t;
      }
      else{
        *tmin = MIN(*tmin, t);
        *tmax = MAX(*tmax, t);
      }
      nf++;
    }
    nframes = MAX(nf, nframes);
    fclose(stream);
  }
  return nframes;
}

/* ------------------ GetSmokeFrameStatus ------------------------ */

int GetSmokeFrameStatus(float time_restart, float time_before, float time_now, int *mode){
  if(*mode==2)return 1;
  if(*mode == 0){
    if(time_now < time_restart){
      return 1;
    }
    else{
      *mode = 1;
      return 0;
    }
  }
  else{
    if(time_now < time_before){
      *mode = 2;
      return 1;
    }
    else{
      return 0;
    }
  }
}

/* ------------------ MakeTimesMap ------------------------ */

void MakeTimesMap(float *times, unsigned char **times_map_ptr, int n){
  int i, mode;
  float t_restart;
  unsigned char *times_map;
  int simple = 1;

  if(n <= 0)return;
  if(*times_map_ptr == NULL){
    NewMemory((void **)&times_map, n);
  }
  else{
    times_map = *times_map_ptr;
  }

  mode = 1;
  times_map[n - 1] = 1;
  for(i = n - 2;i >= 0;i--){
    if(mode == 1){
      if(times[i] >= times[i + 1]){
        t_restart = times[i + 1];
        mode = 0;
        simple = 0;
      }
      times_map[i] = mode;
      continue;
    }
// to remove near identical time points
//#define RESTART_EPS 0.0005
//    if(times[i] < t_restart-RESTART_EPS)mode = 1;
    if(times[i] < t_restart)mode = 1;
    times_map[i] = mode;
  }
  if(simple == 1)FREEMEMORY(times_map);
  *times_map_ptr = times_map;
}

/* ------------------ GetSmoke3DSizes ------------------------ */

int GetSmoke3DSizes(smoke3ddata *smoke3di, int fortran_skip, char *smokefile, int version, unsigned char **times_map_ptr, float **times_ptr, int **use_smokeframe,
  int *nchars_smoke_uncompressed, int **nchars_smoke_compressed_found, int **nchars_smoke_compressed_full, float **maxvals_ptr, float *maxval, int *ntimes_found, int *ntimes_full){
  char buffer[255];
  FILE *SMOKE_SIZE = NULL;
  int nframes_found;
  float time_local, *times = NULL;
  unsigned char *times_map = NULL;
  int *use_smokeframe_full;
  int nch_uncompressed, nch_smoke_compressed;
  int *nch_smoke_compressed_full = NULL;
  int *nch_smoke_compressed_found = NULL;
  int iframe_local;
  int dummy;
  int ntimes_full2;
  int iii;
  int first = 1;
  int nchars_first;
  float time_last;
  float *maxvals=NULL;

  if(smokefile==NULL){
    printf("***error: smokefile pointer is NULL\n");
    return 1;
  }
  SMOKE_SIZE = GetSmokeFileSize(smokefile,fortran_skip,version);
  if(SMOKE_SIZE == NULL){
    printf("***warning: failed to open 3D smoke size file: %s\n", smokefile);
    return 1;
  }

  nframes_found = 0;
  iframe_local = -1;
  fgets(buffer, 255, SMOKE_SIZE);
  iii = 0;
  while(!feof(SMOKE_SIZE)){
    int nchars;

    if(fgets(buffer, 255, SMOKE_SIZE) == NULL)break;
    sscanf(buffer, "%f %i", &time_local, &nchars);
    if(first == 1){
      nchars_first = nchars;
      first = 0;
    }
    else{
      if(nchars != nchars_first){
        fprintf(stderr, "\n");
        fprintf(stderr, "   ***error: file %s is corrupted after t=%f\n", smokefile, time_last);
        fprintf(stderr, "             frame size expected: %i frame size found %i\n", nchars_first, nchars);
        break; // file is corrupted, nchars should be the same for each time step
      }
    }
    time_last = time_local;
    iframe_local++;
    if(use_tload_end == 1 && time_local > global_scase.tload_end)break;
    if(iii%tload_step == 0 && (use_tload_begin == 0 || time_local >= global_scase.tload_begin)){
      nframes_found++;
    }
    iii++;
  }
  rewind(SMOKE_SIZE);
  if(nframes_found <= 0){
    *ntimes_found = 0;
    *ntimes_full = 0;
    fclose(SMOKE_SIZE);
    printf("***warning: nnsmoke frames found=%i <= 0\n",nframes_found);
    return 1;
  }
  *ntimes_found = nframes_found;
  *ntimes_full = iframe_local + 1;

  use_smokeframe_full=*use_smokeframe;
  times                     = *times_ptr;
  times_map                 = *times_map_ptr;
  nch_smoke_compressed_full = *nchars_smoke_compressed_full;
  nch_smoke_compressed_found= *nchars_smoke_compressed_found;
  if(load_smoke_density == 1 && smoke3di->is_smoke_density == 1)maxvals = *maxvals_ptr;

  NewResizeMemory(       use_smokeframe_full, (*ntimes_full)  * sizeof(int));
  NewResizeMemory(                     times, nframes_found   * sizeof(float));
  NewResizeMemory(                 times_map, nframes_found   * sizeof(char));
  NewResizeMemory( nch_smoke_compressed_full, (*ntimes_full)  * sizeof(int));
  NewResizeMemory(nch_smoke_compressed_found, (*ntimes_found) * sizeof(int));
  if(load_smoke_density == 1 && smoke3di->is_smoke_density == 1){
    NewResizeMemory(maxvals, (*ntimes_full) * sizeof(float));
  }

  *use_smokeframe                = use_smokeframe_full;
  *times_ptr                     = times;
  *times_map_ptr                 = times_map;
  *nchars_smoke_compressed_full  = nch_smoke_compressed_full;
  *nchars_smoke_compressed_found = nch_smoke_compressed_found;
  if(load_smoke_density == 1 && smoke3di->is_smoke_density == 1){
    *maxvals_ptr = maxvals;
  }

  fgets(buffer, 255, SMOKE_SIZE);
  ntimes_full2 = 0;
  iii = 0;
  *maxval = -1.0;
  nch_uncompressed = 0;
  int count=0;
  while(!feof(SMOKE_SIZE)){
    float maxvali;
    float maxval_density=0.0;
    int nch_smoke_density=0;

    if(fgets(buffer, 255, SMOKE_SIZE) == NULL)break;
    ntimes_full2++;
    if(ntimes_full2 > *ntimes_full)break;
    maxvali = -1.0;
    if(version == 0){
      sscanf(buffer, "%f %i %i %f %i %f", &time_local, &nch_uncompressed, &nch_smoke_compressed, &maxvali, &nch_smoke_density, &maxval_density);
    }
    else{
      int nch_light;

      nch_light = 0;
      sscanf(buffer, "%f %i %i %i %i %f", &time_local, &nch_uncompressed, &dummy, &nch_smoke_compressed, &nch_light, &maxvali);
    }
    *maxval = MAX(maxvali, *maxval);
    if(load_smoke_density == 1 && smoke3di->is_smoke_density == 1){
      *maxvals++ = maxval_density;
      *nch_smoke_compressed_full++ = nch_smoke_density;
    }
    else{
      *nch_smoke_compressed_full++ = nch_smoke_compressed;
    }
    *use_smokeframe_full = 0;
    if(use_tload_end == 1 && time_local > global_scase.tload_end)break;

    if(iii%tload_step == 0 && (use_tload_begin == 0 || time_local >= global_scase.tload_begin)){
      *use_smokeframe_full = 1;
      *times++ = time_local;
      count++;
      if(load_smoke_density == 1 && smoke3di->is_smoke_density == 1){
        *nch_smoke_compressed_found++ = nch_smoke_density;
      }
      else{
        *nch_smoke_compressed_found++ = nch_smoke_compressed;
      }
    }
    use_smokeframe_full++;
    iii++;
  }
  MakeTimesMap(smoke3di->times, &smoke3di->times_map, count);
  *nchars_smoke_uncompressed = nch_uncompressed;
  fclose(SMOKE_SIZE);
  return 0;
}

/* ------------------ GetSmoke3DVersion ------------------------ */

int GetSmoke3DVersion2(smoke3ddata *smoke3di){
  FILE *SMOKE3DFILE = NULL, *SMOKE3D_REGFILE = NULL, *SMOKE3D_COMPFILE = NULL;
  int nxyz[8];
  char *file;
  int fortran_skip = 0;

  if(smoke3di->filetype==FORTRAN_GENERATED&&smoke3di->is_zlib==0)fortran_skip = 4;

  file = smoke3di->comp_file;
  if(file!=NULL)SMOKE3D_COMPFILE = fopen(file, "rb");
  if(SMOKE3D_COMPFILE==NULL){
    file = smoke3di->reg_file;
    SMOKE3D_REGFILE = fopen(file, "rb");
  }
  if(SMOKE3D_REGFILE==NULL&&SMOKE3D_COMPFILE==NULL)return -1;
  if(SMOKE3D_COMPFILE!=NULL)SMOKE3DFILE = SMOKE3D_COMPFILE;
  if(SMOKE3D_REGFILE!=NULL)SMOKE3DFILE = SMOKE3D_REGFILE;
  smoke3di->file = file;

  SKIP;fread(nxyz, 4, 8, SMOKE3DFILE); SKIP;
  {
    float time_local;
    int nchars[2];

    SKIP;fread(&time_local, 4, 1, SMOKE3DFILE);SKIP;
    if(feof(SMOKE3DFILE)==0){
      SKIP;fread(nchars, 4, 2, SMOKE3DFILE);SKIP;
    }
  }

  fclose(SMOKE3DFILE);

  return nxyz[1];
}

/* ------------------ GetSmoke3DVersion ------------------------ */

int GetSmoke3DVersion(smoke3ddata *smoke3di){
  if(smoke3d_compression_type == COMPRESSED_UNKNOWN){
    smoke3d_compression_type = GetSmoke3DVersion2(smoke3di);
  }
  else if(smoke3d_compression_type == COMPRESSED_RLE){
    smoke3di->file = smoke3di->reg_file;
  }
  else{
    smoke3di->file = smoke3di->comp_file;
    smoke3d_compression_type = COMPRESSED_ZLIB;
  }
  return smoke3d_compression_type;
}

/* ------------------ SetSmokeColorFlags ------------------------ */

void SetSmokeColorFlags(smoke3d_collection *smoke3dcoll_arg){
  int i;

  for(i = 0;i<smoke3dcoll_arg->nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    int j;

    smoke3di = smoke3dcoll_arg->smoke3dinfo + i;
#ifdef pp_SMOKE3D_FORCE
    if(smoke3di->dummy == 1)continue;
#endif
    for(j = 0;j < smoke3dcoll_arg->nsmoke3dtypes;j++){
      smoke3di->smokestate[j].loaded = 0;
    }
  }

  for(i = 0;i<smoke3dcoll_arg->nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    int j;

    smoke3di = smoke3dcoll_arg->smoke3dinfo+i;
#ifdef pp_SMOKE3D_FORCE
    if(smoke3di->dummy == 1)continue;
#endif
    for(j = 0;j <smoke3dcoll_arg->nsmoke3dtypes;j++){
      smoke3di->smokestate[j].color = NULL;
      smoke3di->smokestate[j].index = -1;
    }
    if(smoke3di->loaded==0)continue;

    if(smoke3di->type >= 0 && smoke3di->type < smoke3dcoll_arg->nsmoke3dtypes){
      smoke3di->smokestate[smoke3di->type].color = smoke3di->smokeframe_in;
      smoke3di->smokestate[smoke3di->type].index = i;
    }
    else{
      assert(FFALSE);
    }

    for(j = 0;j<smoke3dcoll_arg->nsmoke3dinfo;j++){
      smoke3ddata *smoke3dj;
      int k;

      if(i==j)continue;
      smoke3dj = smoke3dcoll_arg->smoke3dinfo+j;
#ifdef pp_SMOKE3D_FORCE
      if(smoke3dj->dummy == 1)continue;
#endif
      if(smoke3dj->loaded==0)continue;
      if(smoke3di->blocknumber!=smoke3dj->blocknumber)continue;
      if(smoke3di->is1!=smoke3dj->is1)continue;
      if(smoke3di->is2!=smoke3dj->is2)continue;
      if(smoke3di->js1!=smoke3dj->js1)continue;
      if(smoke3di->js2!=smoke3dj->js2)continue;
      if(smoke3di->ks1!=smoke3dj->ks1)continue;
      if(smoke3di->ks2!=smoke3dj->ks2)continue;

      if(smoke3dj->type >= 0 && smoke3dj->type < smoke3dcoll_arg->nsmoke3dtypes){
        smoke3di->smokestate[smoke3dj->type].color = smoke3dj->smokeframe_in;
        smoke3di->smokestate[smoke3dj->type].index = j;
      }
      else{
        assert(FFALSE);
      }
      for(k = 0;k < smoke3dcoll_arg->nsmoke3dtypes;k++){
        if(smoke3di->smokestate[k].color != NULL)smoke3dj->smokestate[k].loaded = 1;
      }
    }
  }
}

/* ------------------ UpdateLoadedSmoke ------------------------ */

void UpdateLoadedSmoke(int *h_loaded, int *t_loaded){
  int j;

  *h_loaded = 0;
  *t_loaded = 0;
  for(j = 0; j<global_scase.smoke3dcoll.nsmoke3dinfo; j++){
    smoke3ddata *smoke3dj;

    smoke3dj = global_scase.smoke3dcoll.smoke3dinfo+j;
    if(smoke3dj->loaded==1&&smoke3dj->type==HRRPUV_index){
      *h_loaded = 1;
      break;
    }
  }
  for(j = 0; j<global_scase.smoke3dcoll.nsmoke3dinfo; j++){
    smoke3ddata *smoke3dj;

    smoke3dj = global_scase.smoke3dcoll.smoke3dinfo+j;
    if(smoke3dj->loaded==1&&smoke3dj->type==TEMP_index){
      *t_loaded = 1;
      break;
    }
  }
}

/* ------------------ SmokeWrapup ------------------------ */

#define SMOKE_OPTIONS 19
void SmokeWrapup(void){
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  stept = 1;
  SetSmokeColorFlags(&global_scase.smoke3dcoll);
  UpdateLoadedSmoke(&hrrpuv_loaded,&temp_loaded);
  UpdateSmoke3dFileParms();
  UpdateTimes();
  GLUISmoke3dCB(UPDATE_SMOKEFIRE_COLORS);
  GLUISmoke3dCB(USE_FIRE_COLORMAP);

  smoke_render_option = RENDER_SLICE;
  update_fire_alpha = 1;
  have_fire  = HaveFireLoaded();
  have_smoke = HaveSootLoaded();
  GLUISmoke3dCB(USE_OPACITY_MULTIPLIER);
  ForceIdle();
}

#define READSMOKE3D_CONTINUE_ON 0
#define READSMOKE3D_RETURN      1

/* ------------------ SetupSmoke3D ------------------------ */

int SetupSmoke3D(smoke3ddata *smoke3di, int load_flag, int iframe_arg, int *errorcode_arg){
  meshdata *mesh_smoke3d;
  int ncomp_smoke_total_local;
  int ncomp_smoke_total_skipped_local;
  int i, j, ii;
  int fortran_skip = 0;
  int error_local;

  mesh_smoke3d = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;
  smoke3ddata *smokefileptr;

  if(load_flag == UNLOAD){
    smokefileptr = NULL;
  }
  else {
    smokefileptr = smoke3di;
  }
  if(smoke3di->extinct>0.0){
    mesh_smoke3d->smoke3d_soot = smokefileptr;
  }
  else{
    if(smoke3di->type==HRRPUV_index){
      mesh_smoke3d->smoke3d_hrrpuv = smokefileptr;
    }
    else if(smoke3di->type==TEMP_index){
      mesh_smoke3d->smoke3d_temp = smokefileptr;
    }
    else if(smoke3di->type==CO2_index){
      mesh_smoke3d->smoke3d_co2 = smokefileptr;
    }
  }

  smoke3di->request_load = 1;
  if(smoke3di->filetype==FORTRAN_GENERATED&&smoke3di->is_zlib==0)fortran_skip = 4;

  if(smoke3di->loaded==1&&load_flag!=RELOAD){
    FreeSmoke3D(&global_scase, smoke3di);
    smoke3di->loaded = 0;
    smoke3di->display = 0;
    smoke3di->primary_file = 0;
  }

  if(load_flag!=RELOAD){
    FREEMEMORY(mesh_smoke3d->merge_alpha);
    FREEMEMORY(mesh_smoke3d->merge_color);
  }

  if(load_flag==UNLOAD){
    smoke3di->request_load = 0;
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    UpdateTimes();
    SetSmokeColorFlags(&global_scase.smoke3dcoll);
    update_fire_alpha = 1;

    smoke3di->soot_density_loaded = 0;
    if(smoke3di->type==HRRPUV_index)mesh_smoke3d->smoke3d_hrrpuv = NULL;
    if(smoke3di->type==TEMP_index)mesh_smoke3d->smoke3d_temp = NULL;
    if(smoke3di->type==SOOT_index)mesh_smoke3d->smoke3d_co2 = NULL;
    if(smoke3di->type==CO2_index)mesh_smoke3d->smoke3d_soot = NULL;
    UpdateLoadedSmoke(&hrrpuv_loaded,&temp_loaded);

    if(mesh_smoke3d->iblank_smoke3d!=NULL){
      int free_iblank_smoke3d_local;

      free_iblank_smoke3d_local = 1;
      for(j = 0; j<global_scase.smoke3dcoll.nsmoke3dinfo; j++){
        smoke3ddata *smoke3dj;
        meshdata *meshj;

        smoke3dj = global_scase.smoke3dcoll.smoke3dinfo+j;
        meshj = global_scase.meshescoll.meshinfo+smoke3dj->blocknumber;
        if(smoke3dj!=smoke3di && smoke3dj->loaded==1&&meshj==mesh_smoke3d){
          free_iblank_smoke3d_local = 0;
          break;
        }
      }
      if(free_iblank_smoke3d_local==1){
        FREEMEMORY(mesh_smoke3d->iblank_smoke3d);
        mesh_smoke3d->iblank_smoke3d_defined = 0;
      }
    }
    UpdateSmoke3dFileParms();
    return READSMOKE3D_RETURN;
  }
  if(smoke3di->type==HRRPUV_index||smoke3di->type==TEMP_index){
    // if we are loading an HRRPUV then unload all TEMP's
    if(smoke3di->type==HRRPUV_index&&ntemploaded>0)printf("unloading all smoke3d temperature files\n");

    // if we are loading a TEMP then unload all HRRPUV's
    if(smoke3di->type==TEMP_index&&nhrrpuvloaded>0)printf("unloading all  smoke3d hrrpuv files\n");

    for(j = 0; j<global_scase.smoke3dcoll.nsmoke3dinfo; j++){
      smoke3ddata *smoke3dj;
      int error2_local;

      smoke3dj = global_scase.smoke3dcoll.smoke3dinfo+j;
      if(smoke3di!=smoke3dj&&smoke3dj->loaded==1){
        if((smoke3di->type==TEMP_index&&smoke3dj->type==HRRPUV_index)||
          (smoke3di->type==HRRPUV_index&&smoke3dj->type==TEMP_index)){
          printf("unloading %s\n", smoke3dj->file);
          SetupSmoke3D(smoke3dj, UNLOAD, iframe_arg, &error2_local);
        }
      }
    }
  }

  if(smoke3di->compression_type==COMPRESSED_UNKNOWN){
    smoke3di->compression_type = GetSmoke3DVersion(smoke3di);
    update_smoke3dmenulabels = 1;
  }
  if(iframe_arg==ALL_SMOKE_FRAMES)PRINTF("Loading %s(%s)", smoke3di->file, smoke3di->label.shortlabel);
  CheckMemory;
  smoke3di->ntimes_old = smoke3di->ntimes;
  if(GetSmoke3DSizes(smoke3di, fortran_skip, smoke3di->file, smoke3di->compression_type, &smoke3di->times_map, &smoke3di->times, &smoke3di->use_smokeframe,
    &smoke3di->nchars_uncompressed,
    &smoke3di->nchars_compressed_smoke,
    &smoke3di->nchars_compressed_smoke_full,
    &smoke3di->maxvals,
    &smoke3di->maxval,
    &smoke3di->ntimes,
    &smoke3di->ntimes_full)==1){
    SetupSmoke3D(smoke3di, UNLOAD, iframe_arg, &error_local);
    *errorcode_arg = 1;
    fprintf(stderr, "\n*** Error: problems sizing 3d smoke data for %s\n", smoke3di->file);
    return READSMOKE3D_RETURN;
  }
  smoke3di->skip_smoke = 0;
  smoke3di->skip_fire  = 0;
  CheckMemory;
  if(
    NewResizeMemory(smoke3di->smokeframe_comp_list, smoke3di->ntimes_full*sizeof(unsigned char *))==0||
    NewResizeMemory(smoke3di->frame_all_zeros, smoke3di->ntimes_full*sizeof(unsigned char))==0||
    NewResizeMemory(smoke3di->smoke_boxmin, 3*smoke3di->ntimes_full*sizeof(float))==0||
    NewResizeMemory(smoke3di->smoke_boxmax, 3*smoke3di->ntimes_full*sizeof(float))==0||
    NewResizeMemory(smoke3di->smokeframe_in, smoke3di->nchars_uncompressed*sizeof(unsigned char))==0||
    NewResizeMemory(smoke3di->smokeview_tmp, smoke3di->nchars_uncompressed*sizeof(unsigned char))==0||
    NewResizeMemory(smoke3di->smokeframe_out, smoke3di->nchars_uncompressed*sizeof(unsigned char))==0||
    NewResizeMemory(mesh_smoke3d->merge_color, 4*smoke3di->nchars_uncompressed*sizeof(unsigned char))==0||
    NewResizeMemory(mesh_smoke3d->is_firenode, smoke3di->nchars_uncompressed * sizeof(unsigned char)) == 0 ||
    NewResizeMemory(mesh_smoke3d->merge_alpha, smoke3di->nchars_uncompressed*sizeof(unsigned char))==0){
      SetupSmoke3D(smoke3di, UNLOAD, iframe_arg, &error_local);
    *errorcode_arg = 1;
    fprintf(stderr, "\n*** Error: problems allocating memory for 3d smoke file: %s\n", smoke3di->file);
    return READSMOKE3D_RETURN;
  }
  for(i = 0; i<smoke3di->ntimes_full; i++){
    smoke3di->frame_all_zeros[i] = SMOKE3D_ZEROS_UNKNOWN;
  }
  memset(mesh_smoke3d->is_firenode, 0, smoke3di->nchars_uncompressed);

  ncomp_smoke_total_local = 0;
  ncomp_smoke_total_skipped_local = 0;
  for(i = 0; i<smoke3di->ntimes_full; i++){
    ncomp_smoke_total_local += smoke3di->nchars_compressed_smoke_full[i];
    if(smoke3di->use_smokeframe[i]==1){
      ncomp_smoke_total_skipped_local += smoke3di->nchars_compressed_smoke_full[i];
    }
  }
  smoke3di->ncomp_smoke_total = ncomp_smoke_total_skipped_local;
  if(NewResizeMemory(smoke3di->smoke_comp_all, ncomp_smoke_total_skipped_local*sizeof(unsigned char))==0){
    SetupSmoke3D(smoke3di, UNLOAD, iframe_arg, &error_local);
    *errorcode_arg = 1;
    fprintf(stderr, "\n*** Error: problems allocating memory for 3d smoke file: %s\n", smoke3di->file);
    return READSMOKE3D_RETURN;
  }

  ncomp_smoke_total_local = 0;
  i = 0;
  for(ii = 0; ii<smoke3di->ntimes_full; ii++){
    if(smoke3di->use_smokeframe[ii]==1){
      smoke3di->smokeframe_comp_list[i] = smoke3di->smoke_comp_all+ncomp_smoke_total_local;
      ncomp_smoke_total_local += smoke3di->nchars_compressed_smoke[i];
      i++;
    }
  }
  return READSMOKE3D_CONTINUE_ON;
}

/* ------------------ ReadSmoke3D ------------------------ */

FILE_SIZE ReadSmoke3D(int time_frame,int ifile_arg,int load_flag, int first_time, int *errorcode_arg){
  smoke3ddata *smoke3di;
  FILE_SIZE file_size_local=0;
  float total_time;
  int error_local;
  MFILE *SMOKE3DFILE;
  float read_time_local;
  int iii;
  int nframes_found_local=0;
  int frame_start_local, frame_end_local;
  int nchars_local[2];
  int i;
  float time_local;
  char compstring_local[128];
  int fortran_skip=0;

  GLUTPOSTREDISPLAY;
  SetTimeState();
  update_smokefire_colors = 1;
#ifndef pp_FSEEK
  if(load_flag==RELOAD)load_flag = LOAD;
#endif
  START_TIMER(total_time);
  assert(ifile_arg>=0&&ifile_arg<global_scase.smoke3dcoll.nsmoke3dinfo);
  smoke3di = global_scase.smoke3dcoll.smoke3dinfo + ifile_arg;
  if(smoke3di->filetype==FORTRAN_GENERATED&&smoke3di->is_zlib==0)fortran_skip=4;
  update_fire_histogram_now = 1;

  if(first_time == FIRST_TIME){
    if(SetupSmoke3D(smoke3di, load_flag,time_frame, errorcode_arg)==READSMOKE3D_RETURN){
      return 0;
    }
  }
  if(smoke3di->smokeframe_comp_list==NULL)return 0;

  IF_NOT_USEMESH_RETURN0(smoke3di->loaded, smoke3di->blocknumber);
  char *file;
  file = smoke3di->file;
  if(load_smoke_density == 1 && smoke3di->is_smoke_density == 1)file = smoke3di->smoke_density_file;
  SMOKE3DFILE=fopen(file,"rb");
  if(SMOKE3DFILE==NULL){
    SetupSmoke3D(smoke3di,UNLOAD, time_frame, &error_local);
    *errorcode_arg =1;
    return 0;
  }

  int nxyz_local[8];

  SKIP_SMOKE;FREAD_SMOKE(nxyz_local,4,8,SMOKE3DFILE);SKIP_SMOKE;
  file_size_local +=4+4*8+4;
  smoke3di->is1=nxyz_local[2];
  smoke3di->is2=nxyz_local[3];
  smoke3di->js1=nxyz_local[4];
  smoke3di->js2=nxyz_local[5];
  smoke3di->ks1=nxyz_local[6];
  smoke3di->ks2=nxyz_local[7];
  smoke3di->compression_type=nxyz_local[1];

  // read smoke data
  START_TIMER(read_time_local);
  if(time_frame==ALL_SMOKE_FRAMES){
    if(load_flag== RELOAD&&smoke3di->ntimes_old > 0){
      SkipSmokeFrames(SMOKE3DFILE, smoke3di, smoke3di->ntimes_old);
      frame_start_local = smoke3di->ntimes_old;
    }
    else{
      frame_start_local = 0;
    }
    frame_end_local = smoke3di->ntimes_full;
  }
  else{
    SkipSmokeFrames(SMOKE3DFILE, smoke3di, time_frame);
    frame_start_local = time_frame;
    frame_end_local = time_frame+1;
  }
  iii = frame_start_local;
  nframes_found_local = frame_start_local;
  for(i=frame_start_local;i<frame_end_local;i++){
    SKIP_SMOKE;FREAD_SMOKE(&time_local,4,1,SMOKE3DFILE);SKIP_SMOKE;
    file_size_local +=4+4+4;
    if(FEOF_SMOKE(SMOKE3DFILE)!=0||(use_tload_end==1&&time_local>global_scase.tload_end)){
      smoke3di->ntimes_full=i;
      smoke3di->ntimes=nframes_found_local;
      break;
    }
    if(use_tload_begin==1&&time_local<global_scase.tload_begin)smoke3di->use_smokeframe[i]=0;
    SKIP_SMOKE;FREAD_SMOKE(nchars_local,4,2,SMOKE3DFILE); SKIP_SMOKE;
    file_size_local += 4+2*4+4;
    if(FEOF_SMOKE(SMOKE3DFILE)!=0){
      smoke3di->ntimes_full=i;
      smoke3di->ntimes=nframes_found_local;
      break;
    }
    if(smoke3di->use_smokeframe[i]==1){
      float complevel_local;

      nframes_found_local++;
      SKIP_SMOKE;FREAD_SMOKE(smoke3di->smokeframe_comp_list[iii],1,smoke3di->nchars_compressed_smoke[iii],SMOKE3DFILE); SKIP_SMOKE;
      if(smoke3di->compression_type==COMPRESSED_RLE){
        if(AllZeroRLE(smoke3di->smokeframe_comp_list[iii],smoke3di->nchars_compressed_smoke[iii])==1){
          smoke3di->frame_all_zeros[iii] = SMOKE3D_ZEROS_ALL;
        }
        else{
          smoke3di->frame_all_zeros[iii] = SMOKE3D_ZEROS_SOME;
        }
      }
      else{
        smoke3di->frame_all_zeros[iii] = SMOKE3D_ZEROS_UNKNOWN;
      }
      file_size_local +=4+smoke3di->nchars_compressed_smoke[iii]+4;
      iii++;
      CheckMemory;
      if(FEOF_SMOKE(SMOKE3DFILE)!=0){
        smoke3di->ntimes_full=i;
        smoke3di->ntimes=nframes_found_local;
        break;
      }

      complevel_local=40.0*(float)nchars_local[0]/(float)nchars_local[1];
      complevel_local=(int)complevel_local;
      complevel_local/=10.0;
      if(complevel_local<0.0)complevel_local =-complevel_local;
      sprintf(compstring_local," compression ratio: %.1f",complevel_local);
      TrimBack(compstring_local);
      TrimZeros(compstring_local);
    }
    else{
      SKIP_SMOKE;FSEEK_SMOKE(SMOKE3DFILE,smoke3di->nchars_compressed_smoke_full[i],SEEK_CUR);SKIP_SMOKE;
      if(FEOF_SMOKE(SMOKE3DFILE)!=0){
        smoke3di->ntimes_full=i;
        smoke3di->ntimes=nframes_found_local;
        break;
      }
    }
  }
  STOP_TIMER(read_time_local);
  if(SMOKE3DFILE != NULL){
    FCLOSE_SMOKE(SMOKE3DFILE);
  }

  if(load_smoke_density == 1 && smoke3di->is_smoke_density == 1)smoke3di->soot_density_loaded = 1;
  smoke3di->loaded=1;
  smoke3di->display=1;

  if(smoke3di->finalize == 1){
    SmokeWrapup();
  }
  STOP_TIMER(total_time);
  if(time_frame==ALL_SMOKE_FRAMES){
    if(file_size_local>1000000){
      PRINTF(" - %.1f MB/%.1f s", (float)file_size_local/1000000., total_time);
    }
    else{
      PRINTF(" - %.0f kB/%.1f s", (float)file_size_local/1000., total_time);
    }
    char max_label[256];

    Float2String(max_label, smoke3di->maxval, 4, 0);
    PRINTF(" - max: %s\n", max_label);
    PrintMemoryInfo;
  }
  if(smoke3di->extinct>0.0){
    SOOT_index = GetSmoke3DType(&global_scase, smoke3di->label.shortlabel);
    global_scase.update_smoke_alphas = 1;
#define SMOKE_EXTINCT 95
    GLUISmoke3dCB(SMOKE_EXTINCT);
  }
  *errorcode_arg = 0;
  return file_size_local;
}

/* ------------------ ReadSmoke3DAllMeshes ------------------------ */

void ReadSmoke3DAllMeshes(int iframe, int smoketype, int *errorcode){
  int i;

  for(i = 0; i < global_scase.smoke3dcoll.nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    int first_time;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->type != smoketype)continue;
    if(iframe==0){
      first_time = FIRST_TIME;
    }
    else{
      first_time = LATER_TIME;
    }
    ReadSmoke3D(iframe, i, LOAD, first_time, errorcode);
  }
}

/* ------------------ UpdateSmoke3d ------------------------ */

int UpdateSmoke3D(smoke3ddata *smoke3di){
  int iframe_local;
  int countin;
  uLongf countout;

  iframe_local = smoke3di->ismoke3d_time;
  countin = smoke3di->nchars_compressed_smoke[iframe_local];
  countout=smoke3di->nchars_uncompressed;
  switch(smoke3di->compression_type){
    unsigned char *buffer_in;

  case COMPRESSED_RLE:
    buffer_in = smoke3di->smokeframe_comp_list[iframe_local];
    countout = UnCompressRLE(buffer_in,countin,smoke3di->smokeframe_in);
    if(update_fire_histogram == 1 && smoke3di->is_fire==1 && smoke3di->histtimes != NULL && smoke3di->histtimes[iframe_local] == 0){
      int i;
      unsigned char *vals;

      vals = smoke3di->smokeframe_in;
      smoke3di->histtimes[iframe_local] = 1;
      for(i = 0; i < countout; i++){
        smoke3d_firecounts[vals[i]]++;
      }
      smoke3d_firecounts[0] = 0;

      smoke3d_firevals[256] = 0.0;
      for(i = 0; i < 256; i++){
        smoke3d_firevals[i] = (float)smoke3d_firecounts[i];
        smoke3d_firevals[256] = MAX(smoke3d_firevals[256], smoke3d_firevals[i]);
      }
      float denom = 1.0;
      if(smoke3d_firevals[256] > 0.0)denom = smoke3d_firevals[256];
      for(i = 0; i < 256; i++){
        smoke3d_firevals[i] /= denom;
      }
    }
    CheckMemory;
    break;
  case COMPRESSED_ZLIB:
    UnCompressZLIB(smoke3di->smokeframe_in,&countout,smoke3di->smokeframe_comp_list[iframe_local],countin);
    break;
  default:
    assert(FFALSE);
    break;
  }
  CheckMemory;
  if(smoke3di->frame_all_zeros[iframe_local] == SMOKE3D_ZEROS_UNKNOWN){
    int i;
    unsigned char *smokeframe_in;

    smokeframe_in = smoke3di->smokeframe_in;
    smoke3di->frame_all_zeros[iframe_local] = SMOKE3D_ZEROS_ALL;

    for(i=0;i<smoke3di->nchars_uncompressed;i++){
      if(smokeframe_in[i]!=0){
        smoke3di->frame_all_zeros[iframe_local]= SMOKE3D_ZEROS_SOME;
        break;
      }
    }
  }
  //assert(countout==smoke3di->nchars_uncompressed);
  return 1;
}

/* ------------------ MergeSmoke3DColors ------------------------ */

void MergeSmoke3DColors(smoke3ddata *smoke3dset){
  int i,j;
  int i_smoke3d_cutoff, i_co2_cutoff;
  int fire_index;
  unsigned char rgb_slicesmokecolormap_0255[4*MAXSMOKERGB];
  unsigned char rgb_sliceco2colormap_0255[4*MAXSMOKERGB];
  int first, last;

  first = 0;
  last = global_scase.smoke3dcoll.nsmoke3dinfo-1;
  if(smoke3dset != NULL){
    first = smoke3dset - global_scase.smoke3dcoll.smoke3dinfo;
    last = first;
  }
  if(have_fire==HRRPUV_index){
    i_smoke3d_cutoff = 254*global_hrrpuv_cb_min/global_scase.hrrpuv_max;
    i_co2_cutoff     = 254*(MAX(0.0,global_hrrpuv_cb_min))/global_scase.hrrpuv_max;
  }
  else if(have_fire==TEMP_index){
    i_smoke3d_cutoff = 254*((global_temp_cb_min - global_scase.temp_min)/(global_temp_cb_max- global_scase.temp_min));
    i_co2_cutoff     = 254*((MAX(0.0,global_temp_cb_min - global_scase.temp_min))/(global_temp_cb_max- global_scase.temp_min));
  }
  else{
    i_smoke3d_cutoff = 255;
    i_co2_cutoff = 255;
  }
  for(i=0;i<4*MAXSMOKERGB;i++){
    rgb_slicesmokecolormap_0255[i] = 255*rgb_slicesmokecolormap_01[i];
    rgb_sliceco2colormap_0255[i]   = 255*rgb_sliceco2colormap_01[i];
  }

  for(i=first;i<=last;i++){
    smoke3ddata *smoke3di, *smoke3d_soot;
    meshdata *mesh_smoke3d;

    smoke3di=global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3dset!=NULL&&smoke3dset!=smoke3di)continue;
    smoke3di->primary_file=0;
    if(smoke3di->loaded==0||smoke3di->display==0)continue;
    if(smoke3di->is_fire == 1  && smoke3di->skip_fire == 1)continue;
    if(smoke3di->is_smoke == 1 && smoke3di->skip_smoke == 1)continue;
    mesh_smoke3d = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;
    smoke3d_soot = mesh_smoke3d->smoke3d_soot;
    if(smoke3di->type==SOOT_index){
      smoke3di->primary_file = 1;
    }
    else if(smoke3di->type==HRRPUV_index){
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3di->primary_file = 1;
      }
    }
    else if(smoke3di->type==TEMP_index){
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3di->primary_file = 1;
      }
    }
    else if(smoke3di->type==CO2_index){
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3ddata *smoke3d_hrrpuv, *smoke3d_temp;

        smoke3d_hrrpuv = mesh_smoke3d->smoke3d_hrrpuv;
        smoke3d_temp = mesh_smoke3d->smoke3d_temp;
        if(smoke3d_hrrpuv==NULL||smoke3d_hrrpuv->loaded==0||smoke3d_hrrpuv->display==0){
          if(smoke3d_temp==NULL||smoke3d_temp->loaded==0||smoke3d_temp->display==0){
            smoke3di->primary_file = 1;
          }
        }
      }
    }
    for(j = 0; j<256; j++){
      float co2j, co2max=0.1;
      float tempj, tempmax=1200.0, tempmin=20.0;

      co2j = co2max*(float)j/255.0;
      smoke3di->co2_alphas[j] = 255.0*(1.0-pow(0.5,  (mesh_smoke3d->dxyz_orig[0]/co2_halfdepth)*(co2j/co2max)));
      tempj = tempmin + (tempmax-20.0)*(float)j/255.0;
      smoke3di->fire_alphas[j] = 255.0*(1.0-pow(0.5, (mesh_smoke3d->dxyz_orig[0]/fire_halfdepth)*((tempj-tempmin)/tempmax)));
    }
  }

  for(i=first;i<=last;i++){
    smoke3ddata *smoke3di;
    meshdata *mesh_smoke3d;
    unsigned char *firecolor_data,*smokecolor_data,*co2color_data;
    unsigned char *mergecolor,*mergealpha;
    unsigned char smokeval_uc[3], co2val_uc[3];

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3dset!=NULL&&smoke3dset!=smoke3di)continue;
    if(smoke3di->loaded==0||smoke3di->primary_file==0)continue;
    if(smoke3di->is_fire == 1  && smoke3di->skip_fire == 1)continue;
    if(smoke3di->is_smoke == 1 && smoke3di->skip_smoke == 1)continue;
    mesh_smoke3d = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;
    if(IsSmokeComponentPresent(smoke3di)==0)continue;

    if(fire_halfdepth<=0.0){
      smoke3di->fire_alpha=255;
    }
    else{
      smoke3di->fire_alpha=255*(1.0-pow(0.5,mesh_smoke3d->dxyz_orig[0]/fire_halfdepth));
    }

    if(co2_halfdepth <= 0.0){
      smoke3di->co2_alpha = 255;
    }
    else{
      smoke3di->co2_alpha = 255 * (1.0 - pow(0.5, mesh_smoke3d->dxyz_orig[0]/co2_halfdepth));
    }

//  temp and hrrpuv cannot be loaded at the same time

    assert(mesh_smoke3d->smoke3d_temp==NULL||mesh_smoke3d->smoke3d_hrrpuv==NULL);

// set up fire data

    firecolor_data=NULL;
    if(mesh_smoke3d->smoke3d_temp!=NULL){
      if(mesh_smoke3d->smoke3d_temp->loaded==1&&mesh_smoke3d->smoke3d_temp->display==1){
        firecolor_data = mesh_smoke3d->smoke3d_temp->smokeframe_in;
      }
    }
    if(mesh_smoke3d->smoke3d_hrrpuv!=NULL){
      if(mesh_smoke3d->smoke3d_hrrpuv->loaded==1&&mesh_smoke3d->smoke3d_hrrpuv->display==1){
        firecolor_data = mesh_smoke3d->smoke3d_hrrpuv->smokeframe_in;
      }
    }

// set up smoke data

    smokecolor_data = NULL;
    if(mesh_smoke3d->smoke3d_soot!=NULL){
      if(mesh_smoke3d->smoke3d_soot->loaded==1&&mesh_smoke3d->smoke3d_soot->display==1){
        smokecolor_data = mesh_smoke3d->smoke3d_soot->smokeframe_in;
      }
    }

// set up co2 data

    co2color_data = NULL;
    if(mesh_smoke3d->smoke3d_co2!=NULL){
      if(mesh_smoke3d->smoke3d_co2->loaded==1&&mesh_smoke3d->smoke3d_co2->display==1){
        co2color_data = mesh_smoke3d->smoke3d_co2->smokeframe_in;
      }
    }
    assert(firecolor_data!=NULL||smokecolor_data!=NULL||co2color_data!=NULL);

#ifdef pp_GPU
    if(usegpu==1)continue;
#endif
    if(mesh_smoke3d->merge_color == NULL){
      NewMemory((void **)&mesh_smoke3d->merge_color,4*smoke3di->nchars_uncompressed*sizeof(unsigned char));
    }
    if(mesh_smoke3d->merge_alpha==NULL){
      NewMemory((void **)&mesh_smoke3d->merge_alpha,smoke3di->nchars_uncompressed*sizeof(unsigned char));
    }

    mergecolor= mesh_smoke3d->merge_color;
    mergealpha= mesh_smoke3d->merge_alpha;
    mesh_smoke3d->smokecolor_ptr = mergecolor;
    mesh_smoke3d->smokealpha_ptr = mergealpha;

    smokeval_uc[0] = (unsigned char)smoke_color_int255[0];
    smokeval_uc[1] = (unsigned char)smoke_color_int255[1];
    smokeval_uc[2] = (unsigned char)smoke_color_int255[2];

    co2val_uc[0] = (unsigned char)co2_color_int255[0];
    co2val_uc[1] = (unsigned char)co2_color_int255[1];
    co2val_uc[2] = (unsigned char)co2_color_int255[2];

    unsigned char *is_firenode;

    is_firenode = global_scase.meshescoll.meshinfo[smoke3di->blocknumber].is_firenode;
    for(j=0;j<smoke3di->nchars_uncompressed;j++){
      unsigned char *firecolor_ptr=NULL, *smokecolor_ptr=NULL, *co2color_ptr=NULL;
       float alpha_fire_local, alpha_smoke_local, alpha_co2_local;

// set fire color and opacity

      alpha_fire_local = 0.0;
      if(firecolor_data!=NULL){
        fire_index = CLAMP(firecolor_data[j],0,254);
        firecolor_ptr = rgb_slicesmokecolormap_0255+4*fire_index;
        if(firecolor_data[j]>=i_smoke3d_cutoff){
          alpha_fire_local = smoke3di->fire_alphas[firecolor_data[j]];
        }
      }

// set smoke color and opacity

      alpha_smoke_local = 0.0;
      if(is_firenode!=NULL)is_firenode[j] = 0;
      smokecolor_ptr = smokeval_uc;
      if(smokecolor_data!=NULL){
        if(firecolor_data!=NULL && firecolor_data[j]>=i_smoke3d_cutoff){
          is_firenode[j] = 1;
          fire_index = CLAMP(firecolor_data[j],0,254);
          smokecolor_ptr = rgb_slicesmokecolormap_0255+4*fire_index;
          if(use_fire_alpha==1){
            alpha_smoke_local = smoke3di->fire_alpha;
          }
          else{
            alpha_smoke_local = CLAMP(smokecolor_data[j], 0, 255);
          }
        }
        else{
          smokecolor_ptr = smokeval_uc;
          alpha_smoke_local = smokecolor_data[j];
        }
      }

// set co2 color and opacity

      alpha_co2_local = 0.0;
      co2color_ptr = co2val_uc;
      if(co2color_data!=NULL){
        if(firecolor_data!=NULL){
          if(firecolor_data[j]>=i_co2_cutoff){
            co2color_ptr = rgb_sliceco2colormap_0255+4*co2color_data[j];
            alpha_co2_local = smoke3di->co2_alphas[co2color_data[j]];
          }
        }
        else{
          alpha_co2_local = smoke3di->co2_alphas[co2color_data[j]];
        }
      }

// merge color and opacity

      if(firecolor_data!=NULL && co2color_data==NULL && smokecolor_data==NULL){
        mergecolor[0] = firecolor_ptr[0];
        mergecolor[1] = firecolor_ptr[1];
        mergecolor[2] = firecolor_ptr[2];
        *mergealpha = alpha_fire_local;
      }
      else if(firecolor_data!=NULL && co2color_data!=NULL && smokecolor_data==NULL){
        float f1=0.0, f2=0.0, denom;

        denom = alpha_fire_local+alpha_co2_local;
        if(denom>0.0){
          f1 = alpha_fire_local/denom;
          f2 = alpha_co2_local/denom;
        }
        mergecolor[0] = f1*firecolor_ptr[0] + f2*co2color_ptr[0];
        mergecolor[1] = f1*firecolor_ptr[1] + f2*co2color_ptr[1];
        mergecolor[2] = f1*firecolor_ptr[2] + f2*co2color_ptr[2];
        *mergealpha = alpha_fire_local + alpha_co2_local - alpha_fire_local*alpha_co2_local/255.0;
      }
      else{
        float f1=0.0, f2=0.0, denom;

        denom = alpha_smoke_local+alpha_co2_local;
        if(denom>0.0){
          f1 = alpha_smoke_local/denom;
          f2 = alpha_co2_local/denom;
        }
        mergecolor[0] = f1*smokecolor_ptr[0] + f2*co2color_ptr[0];
        mergecolor[1] = f1*smokecolor_ptr[1] + f2*co2color_ptr[1];
        mergecolor[2] = f1*smokecolor_ptr[2] + f2*co2color_ptr[2];
        *mergealpha = alpha_smoke_local + alpha_co2_local - alpha_smoke_local*alpha_co2_local/255.0;
      }
      mergecolor+=4;
      mergealpha++;
    }
  }
}

/* ------------------ MergeSmoke3DBlack ------------------------ */

void MergeSmoke3DBlack(smoke3ddata *smoke3dset){
  int i;
  int fire_index;
  int first, last;

  first = 0;
  last = global_scase.smoke3dcoll.nsmoke3dinfo-1;
  if(smoke3dset != NULL){
    first = smoke3dset - global_scase.smoke3dcoll.smoke3dinfo;
    last = first;
  }

  fire_index = HRRPUV_index;
  for(i = first; i<=last; i++){
    smoke3ddata *smoke3di, *smoke3d_soot;
    meshdata *mesh_smoke3d;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    if(smoke3dset!=NULL&&smoke3dset!=smoke3di)continue;
    smoke3di->primary_file = 0;
    if(smoke3di->loaded==0||smoke3di->display==0)continue;
    mesh_smoke3d = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;
    smoke3d_soot = mesh_smoke3d->smoke3d_soot;
    if(smoke3di->type==SOOT_index){
      smoke3di->primary_file = 1;
    }
    else if(smoke3di->type==HRRPUV_index){
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3di->primary_file = 1;
      }
    }
    else if(smoke3di->type==TEMP_index){
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3di->primary_file = 1;
      }
      fire_index = TEMP_index;
    }
    else if(smoke3di->type==CO2_index){
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3ddata *smoke3d_hrrpuv, *smoke3d_temp;

        smoke3d_hrrpuv = mesh_smoke3d->smoke3d_hrrpuv;
        smoke3d_temp = mesh_smoke3d->smoke3d_temp;
        if(smoke3d_hrrpuv==NULL||smoke3d_hrrpuv->loaded==0||smoke3d_hrrpuv->display==0){
          if(smoke3d_temp==NULL||smoke3d_temp->loaded==0||smoke3d_temp->display==0){
            smoke3di->primary_file = 1;
          }
        }
      }
    }
  }

  for(i = first; i <= last; i++){
    smoke3ddata *smoke3di;
    meshdata *meshi;
    unsigned char *firecolor_data, *smokecolor_data;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo+i;
    if(smoke3dset!=NULL&&smoke3dset!=smoke3di)continue;
    if(smoke3di->loaded==0||smoke3di->primary_file==0)continue;
    if(IsSmokeComponentPresent(smoke3di)==0)continue;
    meshi = global_scase.meshescoll.meshinfo+smoke3di->blocknumber;

    if(fire_halfdepth<=0.0){
      smoke3di->fire_alpha = 255;
    }
    else{
      smoke3di->fire_alpha = 255*(1.0-pow(0.5, meshi->dxyz_orig[0]/fire_halfdepth));
    }
    if(co2_halfdepth<=0.0){
      smoke3di->co2_alpha = 255;
    }
    else{
      smoke3di->co2_alpha = 255*(1.0-pow(0.5, meshi->dxyz_orig[0]/co2_halfdepth));
    }
    firecolor_data = NULL;
    smokecolor_data = NULL;
    if(smoke3di->smokestate[fire_index].color!=NULL){
      firecolor_data = smoke3di->smokestate[fire_index].color;
      if(smoke3di->smokestate[fire_index].index!=-1){
        smoke3ddata *smoke3dref;

        smoke3dref = global_scase.smoke3dcoll.smoke3dinfo+smoke3di->smokestate[fire_index].index;
        if(smoke3dref->display==0)firecolor_data = NULL;
      }
    }
    if(smoke3di->smokestate[SOOT_index].color!=NULL){
      smokecolor_data = smoke3di->smokestate[SOOT_index].color;
      if(smoke3di->smokestate[SOOT_index].index!=-1){
        smoke3ddata *smoke3dref;

        smoke3dref = global_scase.smoke3dcoll.smoke3dinfo+smoke3di->smokestate[SOOT_index].index;
        if(smoke3dref->display==0)smokecolor_data = NULL;
      }
    }
#ifdef pp_GPU
    if(usegpu==1)continue;
#endif
    assert(firecolor_data!=NULL||smokecolor_data!=NULL);
    meshi->smokecolor_ptr = firecolor_data;
    meshi->smokealpha_ptr = smokecolor_data;
  }
}

/* ------------------ MergeSmoke3D ------------------------ */

void MergeSmoke3D(smoke3ddata *smoke3dset){
  INIT_PRINT_TIMER(merge_smoke_time);
  if(smoke3d_black==1){
    MergeSmoke3DBlack(smoke3dset);
    }
  else{
    MergeSmoke3DColors(smoke3dset);
  }
  PRINT_TIMER(merge_smoke_time, "MergeSmoke3D");
}

/* ------------------ MergeSmoke3DAll ------------------------ */

void MergeSmoke3DAll(void){
  int i;

  for(i = 0;i < global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    if(smoke3di->loaded == 0 || smoke3di->display == 0)continue;
    assert(smoke3di->timeslist != NULL);
    if(smoke3di->timeslist==NULL)continue;
    smoke3di->ismoke3d_time = smoke3di->timeslist[itimes];
    if(IsSmokeComponentPresent(smoke3di) == 0)continue;
    if(smoke3di->ismoke3d_time != smoke3di->lastiframe){
      smoke3di->lastiframe = smoke3di->ismoke3d_time;
      UpdateSmoke3D(smoke3di);
    }
    MergeSmoke3D(smoke3di);
  }
}

/* ------------------ UpdateSmoke3dMenuLabels ------------------------ */

void UpdateSmoke3dMenuLabels(void){
  int i;
  smoke3ddata *smoke3di;
  char meshlabel[128];

  for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    STRCPY(smoke3di->menulabel, "");
    if(global_scase.meshescoll.nmeshes > 1){
      meshdata *mesh_smoke3d;

      mesh_smoke3d = global_scase.meshescoll.meshinfo + smoke3di->blocknumber;
      sprintf(meshlabel, "%s", mesh_smoke3d->label);
      STRCAT(smoke3di->menulabel, meshlabel);
    }
    else{
      STRCAT(smoke3di->menulabel,smoke3di->label.longlabel);
    }
    if(showfiles==1){
      if(strcmp(smoke3di->menulabel, "") != 0)STRCAT(smoke3di->menulabel, ", ");
      STRCAT(smoke3di->menulabel,smoke3di->file);
    }
    switch(smoke3di->compression_type){
    case COMPRESSED_UNKNOWN:
      // compression type not determined yet
      break;
    case COMPRESSED_RLE: // 3d smoke files are rle compressed by default
      break;
    case COMPRESSED_ZLIB:
      STRCAT(smoke3di->menulabel," (ZLIB) ");
      break;
    default:
      assert(FFALSE);
      break;
    }
  }
}

#define ALLMESHES 0
#define LOWERMESHES 1

/* ------------------ InMeshSmoke ------------------------ */

int InMeshSmoke(float x, float y, float z, int nm, int flag){
  int i;
  int n;

  if(flag==LOWERMESHES){
    n = nm;
  }
  else{
    n = global_scase.meshescoll.nmeshes;
  }
  for(i = 0;i<n;i++){
    meshdata *meshi;

    meshi = global_scase.meshescoll.meshinfo+i;
    if(flag==ALLMESHES&&i==nm)continue;
    if(meshi->iblank_smoke3d==NULL)continue;

    if(x<meshi->xplt[0]||x>meshi->xplt[meshi->ibar])continue;
    if(y<meshi->yplt[0]||y>meshi->yplt[meshi->jbar])continue;
    if(z<meshi->zplt[0]||z>meshi->zplt[meshi->kbar])continue;
    return i;
  }
  return -1;
}

/* ------------------ MakeIBlankSmoke3D ------------------------ */

void MakeIBlankSmoke3D(void){
  int i, ii;
  int ic;

  for(i=0;i<global_scase.smoke3dcoll.nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    meshdata *mesh_smoke3d;
    int ibar, jbar, kbar;
    int ijksize;

    smoke3di = global_scase.smoke3dcoll.smoke3dinfo + i;
    mesh_smoke3d = global_scase.meshescoll.meshinfo + smoke3di->blocknumber;

    ibar = mesh_smoke3d->ibar;
    jbar = mesh_smoke3d->jbar;
    kbar = mesh_smoke3d->kbar;
    ijksize=(ibar+1)*(jbar+1)*(kbar+1);

    if(global_scase.use_iblank==1&&smoke3di->loaded==1&&mesh_smoke3d->iblank_smoke3d==NULL){
      unsigned char *iblank_smoke3d;

      NewMemory((void **)&iblank_smoke3d, ijksize*sizeof(unsigned char));
      mesh_smoke3d->iblank_smoke3d=iblank_smoke3d;
    }
  }

  for(ic=global_scase.meshescoll.nmeshes-1;ic>=0;ic--){
    meshdata *mesh_smoke3d;
    unsigned char *iblank_smoke3d;
    float *xplt, *yplt, *zplt;
    float dx, dy, dz;
    int nx, ny, nxy;
    int ibar, jbar, kbar;
    int ijksize;
    int j, k;

    mesh_smoke3d = global_scase.meshescoll.meshinfo + ic;
    iblank_smoke3d = mesh_smoke3d->iblank_smoke3d;
   // if(iblank_smoke3d==NULL||mesh_smoke3d->iblank_smoke3d_defined==1)continue;
    if(iblank_smoke3d==NULL)continue;
    mesh_smoke3d->iblank_smoke3d_defined = 1;

    xplt=mesh_smoke3d->xplt;
    yplt=mesh_smoke3d->yplt;
    zplt=mesh_smoke3d->zplt;
    dx = xplt[1]-xplt[0];
    dy = yplt[1]-yplt[0];
    dz = zplt[1]-zplt[0];

    ibar = mesh_smoke3d->ibar;
    jbar = mesh_smoke3d->jbar;
    kbar = mesh_smoke3d->kbar;
    ijksize=(ibar+1)*(jbar+1)*(kbar+1);
    nx = ibar + 1;
    ny = jbar + 1;
    nxy = nx*ny;

    for(ii=0;ii<ijksize;ii++){
      *iblank_smoke3d++=GAS;
    }

    iblank_smoke3d=mesh_smoke3d->iblank_smoke3d;

    for(i=0;i<=mesh_smoke3d->ibar;i++){
      for(j=0;j<=mesh_smoke3d->jbar;j++){
        for(k=0;k<=mesh_smoke3d->kbar;k++){
          float x, y, z;
          int ijk;

          ijk = IJKNODE(i, j, k);
          x = xplt[i];
          y = yplt[j];
          z = zplt[k];
          if(InMeshSmoke(x,y,z,ic-1,LOWERMESHES)>=0)iblank_smoke3d[ijk]=SOLID;
        }
      }
    }

    for(ii=0;ii<mesh_smoke3d->nbptrs;ii++){
      blockagedata *bc;

      bc = mesh_smoke3d->blockageinfoptrs[ii];
      if(bc->invisible==1||bc->hidden==1||bc->nshowtime!=0)continue;
      for(i=bc->ijk[IMIN];i<=bc->ijk[IMAX];i++){
        for(j=bc->ijk[JMIN];j<=bc->ijk[JMAX];j++){
          for(k=bc->ijk[KMIN];k<=bc->ijk[KMAX];k++){
            int ijk;

            ijk = IJKNODE(i, j, k);
            iblank_smoke3d[ijk]=SOLID;
          }
        }
      }
    }

    for(j=0;j<=jbar;j++){
      for(k=0;k<=kbar;k++){
        float x, y, z;
        int ijk;

        ijk = IJKNODE(0, j, k);
        x = xplt[0];
        y = yplt[j];
        z = zplt[k];
        if(InMeshSmoke(x-dx,y,z,ic,ALLMESHES)<0){
          iblank_smoke3d[ijk]=SOLID;
        }
        else{
          iblank_smoke3d[ijk]=GAS;
        }

        ijk = IJKNODE(ibar,j,k);
        x = xplt[ibar];
        y = yplt[j];
        z = zplt[k];
        if(InMeshSmoke(x+dx,y,z,ic,ALLMESHES)<0){
          iblank_smoke3d[ijk]=SOLID;
        }
        else{
          iblank_smoke3d[ijk]=GAS;
        }
      }
    }

    for(i=0;i<=ibar;i++){
      for(k=0;k<=kbar;k++){
        float x, y, z;
        int ijk;

        ijk = IJKNODE(i, 0, k);
        x = xplt[i];
        y = yplt[0];
        z = zplt[k];
        if(InMeshSmoke(x,y-dy,z,ic,ALLMESHES)<0){
          iblank_smoke3d[ijk]=SOLID;
        }
        else{
          iblank_smoke3d[ijk]=GAS;
        }

        ijk = IJKNODE(i,jbar,k);
        x = xplt[i];
        y = yplt[jbar];
        z = zplt[k];
        if(InMeshSmoke(x,y+dy,z,ic,ALLMESHES)<0){
          iblank_smoke3d[ijk]=SOLID;
        }
        else{
          iblank_smoke3d[ijk]=GAS;
        }
      }
    }

    for(i=0;i<=ibar;i++){
      for(j=0;j<=jbar;j++){
        float x, y, z;
        int ijk;

        ijk = IJKNODE(i, j, 0);
        x = xplt[i];
        y = yplt[j];
        z = zplt[0];
        if(InMeshSmoke(x,y,z-dz,ic,ALLMESHES)<0){
          iblank_smoke3d[ijk]=SOLID;
        }
        else{
          iblank_smoke3d[ijk]=GAS;
        }

        ijk = IJKNODE(i,j,kbar);
        x = xplt[i];
        y = yplt[j];
        z = zplt[kbar];
        if(InMeshSmoke(x,y,z+dz,ic,ALLMESHES)<0){
          iblank_smoke3d[ijk]=SOLID;
        }
        else{
          iblank_smoke3d[ijk]=GAS;
        }
      }
    }
  }
}
