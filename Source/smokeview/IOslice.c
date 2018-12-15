#include "options.h"
#include "glew.h"
#include <stdio.h>
#ifdef WIN32
#include <share.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include GLUT_H

#include "compress.h"
#include "smv_endian.h"
#include "update.h"
#include "interp.h"
#include "smokeviewvars.h"

void DrawQuadSlice(float *v1, float *v2, float *v3, float *v4, float t1, float t2, float t3, float t4, float del, int level);
void DrawQuadVectorSlice(float *v1, float *v2, float *v3, float *v4, float del, int level);
void DrawTriangleOutlineSlice(float *v1, float *v2, float *v3, float del, int level);

// dummy change
int endianswitch;
float gslice_valmin, gslice_valmax, *gslicedata;
meshdata *gslice_valmesh;
slicedata *gslice_u, *gslice_v, *gslice_w;
slicedata *gslice;

#ifdef WIN32
#define FOPEN(file,mode) _fsopen(file,mode,_SH_DENYNO)
#else
#define FOPEN(file,mode) fopen(file,mode)
#endif

#define FORTRLESLICEREAD(var,size) FSEEK(RLESLICEFILE,4,SEEK_CUR);\
                           returncode=fread(var,4,size,RLESLICEFILE);\
                           if(endianswitch==1)EndianSwitch(var,size);\
                           FSEEK(RLESLICEFILE,4,SEEK_CUR)

#define GET_SLICE_COLOR(color,index) \
     if(sd->constant_color==NULL){\
       int i11;\
       i11 = sd->iqsliceframe[(index)];\
       color = rgb_slice + 4*i11;\
     }\
     else{\
       color = sd->constant_color;\
     }

#define GET_VAL(U,VAL,n) \
         VAL=0.0;           \
         if(U!=NULL){       \
           if(U->compression_type==COMPRESSED_ZLIB){\
             VAL = U->qval256[U->iqsliceframe[(n)]];\
           }                                  \
           else{                              \
             VAL = U->qslice[(n)];               \
           }                                  \
         }

#define GET_VAL_N(U,n)  ( (U)->compression_type==COMPRESSED_ZLIB ? (U)->qval256[(U)->iqsliceframe[(n)]] : (U)->qslice[(n)] )

#define GET_VEC_DXYZ(U,DU,n) \
         if(U==NULL){       \
           DU=0.0;           \
         }\
         else{\
           if(U->compression_type==COMPRESSED_ZLIB){\
             DU=U->qval256[U->iqsliceframe[(n)]];\
           }                                  \
           else{                              \
             DU = U->qslice[(n)];               \
           }                                  \
         }                                   \
         DU *= 0.05*vecfactor/vrange

#define GET_VEC_DXYZ_TERRAIN(U,DU) \
         if(U==NULL){\
           DU=0.0;\
         }\
         else{       \
           if(U->compression_type==COMPRESSED_ZLIB){\
             DU=U->qval256[(int)(f1*U->iqsliceframe[n1]+f2*U->iqsliceframe[n2])];\
           }                                  \
           else{                              \
             DU=f1*U->qslice[n1]+f2*U->qslice[n2];               \
           }                                  \
         }                                   \
         DU *= 0.05*vecfactor/vrange

/* ------------------ Get3DSliceVal ------------------------ */

float Get3DSliceVal(slicedata *sd, float *xyz){
  int i, j, k;
  float *xplt, *yplt, *zplt;
  float dxbar, dybar, dzbar;
  int ibar, jbar, kbar;
  int nx, ny, nz;
  int slice_ny, slice_nz;
  float dx, dy, dz;
  float val000, val100, val010, val110;
  float val001, val101, val011, val111;
  float val00, val10, val01, val11;
  float val0, val1;
  float val;
  int ijk, ip1 = 0, jp1 = 0, kp1 = 0, *ijk_min, *ijk_max;

  meshdata *valmesh;

  valmesh = meshinfo + sd->blocknumber;

  xplt = valmesh->xplt_orig;
  yplt = valmesh->yplt_orig;
  zplt = valmesh->zplt_orig;
  ibar = valmesh->ibar;
  jbar = valmesh->jbar;
  kbar = valmesh->kbar;
  dxbar = xplt[1] - xplt[0];
  dybar = yplt[1] - yplt[0];
  dzbar = zplt[1] - zplt[0];

  nx = ibar + 1;
  ny = jbar + 1;
  nz = kbar + 1;
  slice_ny = sd->ijk_max[1] - sd->ijk_min[1] + 1;
  slice_nz = sd->ijk_max[2] - sd->ijk_min[2] + 1;

  i = GETINDEX(xyz[0], xplt[0], dxbar, nx);
  j = GETINDEX(xyz[1], yplt[0], dybar, ny);
  k = GETINDEX(xyz[2], zplt[0], dzbar, nz);

  // val(i,j,k) = di*nj*nk + dj*nk + dk
  ijk_min = sd->ijk_min;
  ijk_max = sd->ijk_max;

  ijk = (i - ijk_min[0])*slice_nz*slice_ny + (j - ijk_min[1])*slice_nz + (k - ijk_min[2]);

  dx = (xyz[0] - xplt[i]) / dxbar;
  dx = CLAMP(dx, 0.0, 1.0);
  dy = (xyz[1] - yplt[j]) / dybar;
  dy = CLAMP(dy, 0.0, 1.0);
  dz = (xyz[2] - zplt[k]) / dzbar;
  dz = CLAMP(dz, 0.0, 1.0);


  // ijk
  if(i <= ijk_max[0])ip1 = slice_nz*slice_ny;
  if(j <= ijk_max[1])jp1 = slice_nz;
  if(k <= ijk_max[2])kp1 = 1;

  val000 = (float)GET_VAL_N(sd, ijk);     // i,j,k
  val001 = (float)GET_VAL_N(sd, ijk + kp1); // i,j,k+1

                                            // ijk + slice_nz
  val010 = (float)GET_VAL_N(sd, ijk + jp1);     // i,j+1,k
  val011 = (float)GET_VAL_N(sd, ijk + jp1 + kp1); // i,j+1,k+1

                                                  // ijk + slice_nz*slice_ny
  val100 = (float)GET_VAL_N(sd, ijk + ip1);     // i+1,j,k
  val101 = (float)GET_VAL_N(sd, ijk + ip1 + kp1); // i+1,j,k+1

                                                  // ijk + slice_nz + slice_nz*slice_ny
  val110 = (float)GET_VAL_N(sd, ijk + ip1 + jp1);   // i+1,j+1,k
  val111 = (float)GET_VAL_N(sd, ijk + ip1 + jp1 + kp1); // i+1,j+1,k+1

  val00 = MIX(dx, val100, val000);
  val10 = MIX(dx, val110, val010);
  val01 = MIX(dx, val101, val001);
  val11 = MIX(dx, val111, val011);
  val0 = MIX(dy, val10, val00);
  val1 = MIX(dy, val11, val01);

  val = MIX(dz, val1, val0);
  return val;
}

/* ------------------ GetSliceTextureIndex ------------------------ */

float GetSliceTextureIndex(float *xyz){
  int i, j, k;
  float *vv;
  float *xplt, *yplt, *zplt;
  float dxbar, dybar, dzbar;
  int ibar, jbar, kbar;
  int nx, ny, nz;
  int slice_ny, slice_nz;
  float dx, dy, dz;
  float val000, val100, val010, val110;
  float val001, val101, val011, val111;
  float val00, val10, val01, val11;
  float val0, val1;
  float val, val_fraction;
  int ijk;
  int iplus = 0, jplus = 0, kplus = 0, *ijk_min, *ijk_max;

  float *slicedata0;
  float valmin, valmax;
  meshdata *valmesh;

  slicedata0 = gslicedata;
  valmin = gslice_valmin;
  valmax = gslice_valmax;
  valmesh = gslice_valmesh;

  xplt = valmesh->xplt_orig;
  yplt = valmesh->yplt_orig;
  zplt = valmesh->zplt_orig;
  ibar = valmesh->ibar;
  jbar = valmesh->jbar;
  kbar = valmesh->kbar;
  dxbar = xplt[1] - xplt[0];
  dybar = yplt[1] - yplt[0];
  dzbar = zplt[1] - zplt[0];

  nx = ibar + 1;
  ny = jbar + 1;
  nz = kbar + 1;
  slice_ny = gslice->ijk_max[1] - gslice->ijk_min[1] + 1;
  slice_nz = gslice->ijk_max[2] - gslice->ijk_min[2] + 1;

  i = GETINDEX(xyz[0], xplt[0], dxbar, nx);
  j = GETINDEX(xyz[1], yplt[0], dybar, ny);
  k = GETINDEX(xyz[2], zplt[0], dzbar, nz);

  // val(i,j,k) = di*nj*nk + dj*nk + dk
  ijk_min = gslice->ijk_min;
  ijk_max = gslice->ijk_max;
  ijk = (i - ijk_min[0])*slice_nz*slice_ny + (j - ijk_min[1])*slice_nz + (k - ijk_min[2]);

  dx = (xyz[0] - xplt[i]) / dxbar;
  dx = CLAMP(dx, 0.0, 1.0);
  dy = (xyz[1] - yplt[j]) / dybar;
  dy = CLAMP(dy, 0.0, 1.0);
  dz = (xyz[2] - zplt[k]) / dzbar;
  dz = CLAMP(dz, 0.0, 1.0);

  vv = slicedata0 + ijk;
  if(i + 1 <= ijk_max[0])iplus = slice_nz*slice_ny;
  if(j + 1 <= ijk_max[1])jplus = slice_nz;
  if(k + 1 <= ijk_max[2])kplus = 1;

  val000 = (float)vv[0]; // i,j,k
  val001 = (float)vv[kplus]; // i,j,k+1

  val010 = (float)vv[jplus]; // i,j+1,k
  val011 = (float)vv[jplus + kplus]; // i,j+1,k+1

  val100 = (float)vv[iplus]; // i+1,j,k
  val101 = (float)vv[iplus + kplus]; // i+1,j,k+1

  val110 = (float)vv[iplus + jplus]; // i+1,j+1,k
  val111 = (float)vv[iplus + jplus + kplus]; // i+1,j+1,k+1

  val00 = MIX(dx, val100, val000);
  val10 = MIX(dx, val110, val010);
  val01 = MIX(dx, val101, val001);
  val11 = MIX(dx, val111, val011);
  val0 = MIX(dy, val10, val00);
  val1 = MIX(dy, val11, val01);

  val = MIX(dz, val1, val0);
  val_fraction = (val - valmin) / (valmax - valmin);
  val_fraction = CLAMP(val_fraction, 0.0, 1.0);
  return val_fraction;
}

/* ------------------ DrawTriangleSlice ------------------------ */

void DrawTriangleSlice(float *v1, float *v2, float *v3, float t1, float t2, float t3, float del, int level){

  float d12, d13, d23;
  float v12[3], v13[3], v23[3];
  float dx, dy, dz;
  float t12, t13, t23;

  if(level == 0){
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, texture_slice_colorbar_id);
    glBegin(GL_TRIANGLES);
  }
  DIST3(v1, v2, d12);
  DIST3(v1, v3, d13);
  DIST3(v2, v3, d23);
  if(d12 <= del&&d13 <= del&&d23 < del){
    glTexCoord1f(t1);
    glVertex3fv(v1);

    glTexCoord1f(t2);
    glVertex3fv(v2);

    glTexCoord1f(t3);
    glVertex3fv(v3);
  }
  else{
    if(d12 <= MIN(d13, d23)){
      VERT_AVG2(v1, v3, v13);
      t13 = GetSliceTextureIndex(v13);
      VERT_AVG2(v2, v3, v23);
      t23 = GetSliceTextureIndex(v23);

      DrawTriangleSlice(v3, v13, v23, t3, t13, t23, del, level + 1);
      DrawQuadSlice(v13, v1, v2, v23, t13, t1, t2, t23, del, level + 1);
    }
    else if(d13 <= MIN(d12, d23)){
      VERT_AVG2(v1, v2, v12);
      t12 = GetSliceTextureIndex(v12);
      VERT_AVG2(v2, v3, v23);
      t23 = GetSliceTextureIndex(v23);

      DrawTriangleSlice(v12, v2, v23, t12, t2, t23, del, level + 1);
      DrawQuadSlice(v1, v12, v23, v3, t1, t12, t23, t3, del, level + 1);
    }
    else{ // d23<=MIN(d12,d13)
      VERT_AVG2(v1, v2, v12);
      t12 = GetSliceTextureIndex(v12);
      VERT_AVG2(v1, v3, v13);
      t13 = GetSliceTextureIndex(v13);

      DrawTriangleSlice(v1, v12, v13, t1, t12, t13, del, level + 1);
      DrawQuadSlice(v12, v2, v3, v13, t12, t2, t3, t13, del, level + 1);
    }
  }
  if(level == 0){
    glEnd();
    glDisable(GL_TEXTURE_1D);
  }
}

/* ------------------ DrawQuad ------------------------ */

void DrawQuadSlice(float *v1, float *v2, float *v3, float *v4, float t1, float t2, float t3, float t4, float del, int level){
  float d13, d24;
  float dx, dy, dz;

  if(level == 0){
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, texture_slice_colorbar_id);
    glBegin(GL_TRIANGLES);
  }
  DIST3(v1, v3, d13);
  DIST3(v2, v4, d24);
  if(d13 < d24){
    DrawTriangleSlice(v1, v2, v3, t1, t2, t3, del, level + 1);
    DrawTriangleSlice(v1, v3, v4, t1, t3, t4, del, level + 1);
  }
  else{
    DrawTriangleSlice(v1, v2, v4, t1, t2, t4, del, level + 1);
    DrawTriangleSlice(v2, v3, v4, t2, t3, t4, del, level + 1);
  }
  if(level == 0){
    glEnd();
    glDisable(GL_TEXTURE_1D);
  }
}

/* ------------------ DrawQuadOutlineSlice ------------------------ */

void DrawQuadOutlineSlice(float *v1, float *v2, float *v3, float *v4, float del, int level){
  float d13, d24;
  float dx, dy, dz;

  if(level == 0){
    glBegin(GL_LINES);
  }
  DIST3(v1, v3, d13);
  DIST3(v2, v4, d24);
  if(d13 < d24){
    DrawTriangleOutlineSlice(v1, v2, v3, del, level + 1);
    DrawTriangleOutlineSlice(v1, v3, v4, del, level + 1);
  }
  else{
    DrawTriangleOutlineSlice(v1, v2, v4, del, level + 1);
    DrawTriangleOutlineSlice(v2, v3, v4, del, level + 1);
  }
  if(level == 0){
    glEnd();
  }
}

/* ------------------ DrawTriangleOutline ------------------------ */

void DrawTriangleOutlineSlice(float *v1, float *v2, float *v3, float del, int level){
  float d12, d13, d23;
  float v12[3], v13[3], v23[3];
  float dx, dy, dz;

  if(level == 0){
    glBegin(GL_LINES);
  }
  DIST3(v1, v2, d12);
  DIST3(v1, v3, d13);
  DIST3(v2, v3, d23);
  if(d12 <= del&&d13 <= del&&d23 < del){
    glVertex3fv(v1);
    glVertex3fv(v2);

    glVertex3fv(v2);
    glVertex3fv(v3);

    glVertex3fv(v3);
    glVertex3fv(v1);
  }
  else{
    if(d12 <= MIN(d13, d23)){
      VERT_AVG2(v1, v3, v13);
      VERT_AVG2(v2, v3, v23);

      DrawTriangleOutlineSlice(v3, v13, v23, del, level + 1);
      DrawQuadOutlineSlice(v13, v1, v2, v23, del, level + 1);
    }
    else if(d13 <= MIN(d12, d23)){
      VERT_AVG2(v1, v2, v12);
      VERT_AVG2(v2, v3, v23);

      DrawTriangleOutlineSlice(v12, v2, v23, del, level + 1);
      DrawQuadOutlineSlice(v1, v12, v23, v3, del, level + 1);
    }
    else{ // d23<=MIN(d12,d13)
      VERT_AVG2(v1, v2, v12);
      VERT_AVG2(v1, v3, v13);

      DrawTriangleOutlineSlice(v1, v12, v13, del, level + 1);
      DrawQuadOutlineSlice(v12, v2, v3, v13, del, level + 1);
    }
  }
  if(level == 0){
    glEnd();
  }
}

/* ------------------ DrawTriangleVector ------------------------ */

void DrawTriangleVectorSlice(float *v1, float *v2, float *v3, float del, int level){

  float d12, d13, d23;
  float v12[3], v13[3], v23[3], vavg[3];
  float dx, dy, dz;
  float tavg;
  int tavg_index;
  float *rgb_ptr;

  if(level == 0){
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
  }
  DIST3(v1, v2, d12);
  DIST3(v1, v3, d13);
  DIST3(v2, v3, d23);
  if(d12 <= del&&d13 <= del&&d23 < del){
    float vecfactor2, vrange;

    vrange = velocity_range;
    if(vrange <= 0.0)vrange = 1.0;
    vecfactor2 = 0.05*vecfactor / vrange*xyzmaxdiff;

    VERT_AVG3(v1, v2, v3, vavg);
    dx = Get3DSliceVal(gslice_u, vavg)*vecfactor2;
    dy = Get3DSliceVal(gslice_v, vavg)*vecfactor2;
    dz = Get3DSliceVal(gslice_w, vavg)*vecfactor2;
    if(gslice->constant_color != NULL){
      rgb_ptr = gslice->constant_color;
    }
    else{
      tavg = GetSliceTextureIndex(vavg);
      tavg_index = CLAMP(tavg * 255, 0, 255);
      rgb_ptr = rgb_slice + 4 * tavg_index;
    }
    glColor4fv(rgb_ptr);
    glVertex3f(vavg[0] - dx, vavg[1] - dy, vavg[2] - dz);
    glVertex3f(vavg[0] + dx, vavg[1] + dy, vavg[2] + dz);
  }
  else{
    if(d12 <= MIN(d13, d23)){
      VERT_AVG2(v1, v3, v13);
      VERT_AVG2(v2, v3, v23);

      DrawTriangleVectorSlice(v3, v13, v23, del, level + 1);
      DrawQuadVectorSlice(v13, v1, v2, v23, del, level + 1);
    }
    else if(d13 <= MIN(d12, d23)){
      VERT_AVG2(v1, v2, v12);
      VERT_AVG2(v2, v3, v23);

      DrawTriangleVectorSlice(v12, v2, v23, del, level + 1);
      DrawQuadVectorSlice(v1, v12, v23, v3, del, level + 1);
    }
    else{ // d23<=MIN(d12,d13)
      VERT_AVG2(v1, v2, v12);
      VERT_AVG2(v1, v3, v13);

      DrawTriangleVectorSlice(v1, v12, v13, del, level + 1);
      DrawQuadVectorSlice(v12, v2, v3, v13, del, level + 1);
    }
  }
  if(level == 0)glEnd();
}

/* ------------------ DrawQuadVectorSlice ------------------------ */

void DrawQuadVectorSlice(float *v1, float *v2, float *v3, float *v4, float del, int level){
  float d13, d24;
  float dx, dy, dz;

  if(level == 0){
    glBegin(GL_LINE);
  }
  DIST3(v1, v3, d13);
  DIST3(v2, v4, d24);
  if(d13 < d24){
    DrawTriangleVectorSlice(v1, v2, v3, del, level + 1);
    DrawTriangleVectorSlice(v1, v3, v4, del, level + 1);
  }
  else{
    DrawTriangleVectorSlice(v1, v2, v4, del, level + 1);
    DrawTriangleVectorSlice(v2, v3, v4, del, level + 1);
  }
  if(level == 0){
    glEnd();
  }
}

/* ------------------ PushSliceLoadstack ------------------------ */

void PushSliceLoadstack(int sliceindex){
  int i;

  if(islice_loadstack < nslice_loadstack){
    for(i = 0; i < islice_loadstack; i++){
      if(slice_loadstack[i] == sliceindex)return;
    }
    slice_loadstack[islice_loadstack++] = sliceindex;
  }
}

/* ------------------ RemoveSliceLoadstack ------------------------ */

void RemoveSliceLoadstack(int sliceindex){
  int i;

  for(i = islice_loadstack - 1; i >= 0; i--){
    if(slice_loadstack[i] == sliceindex){
      int j;

      for(j = i; j < islice_loadstack - 1; j++){
        slice_loadstack[j] = slice_loadstack[j + 1];
      }
      islice_loadstack--;
      break;
    }
  }
}

/* ------------------ LastSliceLoadstack ------------------------ */

int LastSliceLoadstack(void){
  int return_val;

  if(islice_loadstack - 1 >= 0 && islice_loadstack - 1 < nslice_loadstack){
    return_val = slice_loadstack[islice_loadstack - 1];
  }
  else{
    return_val = -1;
  }
  return return_val;
}

/* ------------------ PushVSliceLoadstack ------------------------ */

void PushVSliceLoadstack(int vsliceindex){
  int i;

  if(ivslice_loadstack < nvslice_loadstack){
    for(i = 0; i < ivslice_loadstack; i++){
      if(vslice_loadstack[i] == vsliceindex)return;
    }
    vslice_loadstack[ivslice_loadstack++] = vsliceindex;
  }
}

/* ------------------ RemoveVSliceLoadstack ------------------------ */

void RemoveVSliceLoadstack(int vsliceindex){
  int i;

  for(i = ivslice_loadstack - 1; i >= 0; i--){
    if(vslice_loadstack[i] == vsliceindex){
      int j;

      for(j = i; j < ivslice_loadstack - 1; j++){
        vslice_loadstack[j] = vslice_loadstack[j + 1];
      }
      ivslice_loadstack--;
      break;
    }
  }
}

/* ------------------ LastVSliceLoadstack ------------------------ */

int LastVSliceLoadstack(void){
  int return_val;

  if(ivslice_loadstack - 1 >= 0 && ivslice_loadstack - 1 < nvslice_loadstack){
    return_val = vslice_loadstack[ivslice_loadstack - 1];
  }
  else{
    return_val = -1;
  }
  return return_val;
}

/* ------------------ OutSlicefile ------------------------ */

void OutSlicefile(slicedata *sd){
  FORTwriteslicedata(sd->file,
    &sd->is1,&sd->is2,&sd->js1,&sd->js2,&sd->ks1,&sd->ks2,
    sd->qslicedata,sd->times,&sd->ntimes, &redirect, strlen(sd->file));
}


//*** header
// endian
// completion (0/1)
// fileversion (compressed format)
// version  (slicef version)
// global min max (used to perform conversion)
// i1,i2,j1,j2,k1,k2


//*** frame
// time, compressed frame size                        for each frame
// compressed buffer

/* ------------------ MakeSliceSizefile ------------------------ */

int MakeSliceSizefile(char *file, char *sizefile, int compression_type){
  int endian_fromfile;
  float minmax[2];
  int ijkbar[6];
  FILE *stream, *sizestream;
  float time_local;
  int ncompressed;
  int count;

  stream = FOPEN(file, "rb");
  if(stream==NULL)return 0;

  sizestream = fopen(sizefile, "w");
  if(sizestream==NULL){
    fclose(stream);
    return 0;
  }
  count = 0;
  if(compression_type==COMPRESSED_ZLIB){
    fread(&endian_fromfile, 4, 1, stream);
    FSEEK(stream, 12, SEEK_CUR);
    fread(minmax, 4, 2, stream);
    fread(ijkbar, 4, 6, stream);

    fprintf(sizestream, "%i %i %i %i %i %i\n", ijkbar[0], ijkbar[1], ijkbar[2], ijkbar[3], ijkbar[4], ijkbar[5]);
    fprintf(sizestream, "%f %f\n", minmax[0], minmax[1]);
    count = 2;

    while(!feof(stream)){
      fread(&time_local, 4, 1, stream);
      fread(&ncompressed, 4, 1, stream);
      fprintf(sizestream, "%f %i\n", time_local, ncompressed);
      count++;
      FSEEK(stream, ncompressed, SEEK_CUR);
    }
  }
  //  endian
  //  fileversion, slice version
  //  global min max (used to perform conversion)
  //  i1,i2,j1,j2,k1,k2


  //  *** frame
  // time
  //  compressed frame size                        for each frame
  // compressed buffer

  fclose(stream);
  fclose(sizestream);
  return count;

}

/* ------------------ GetSliceHeader0 ------------------------ */

int GetSliceHeader0(char *comp_file, char *size_file, int compression_type, int *i1, int *i2, int *jj1, int *j2, int *k1, int *k2, int *slice3d){
  FILE *stream;
  char buffer[255];

  stream = FOPEN(size_file, "r");
  if(stream == NULL){
    if(MakeSliceSizefile(comp_file, size_file, compression_type) == 0)return 0;
    stream = FOPEN(size_file, "r");
    if(stream == NULL)return 0;
  }

  if(fgets(buffer, 255, stream) == NULL){
    fclose(stream);
    return 0;
  }
  sscanf(buffer, "%i %i %i %i %i %i", i1, i2, jj1, j2, k1, k2);
  if(*i1 == *i2 || *jj1 == *j2 || *k1 == *k2){
    *slice3d = 0;
  }
  else{
    *slice3d = 1;
  }
  fclose(stream);
  return 1;
}

/* ------------------ GetSliceHeader ------------------------ */

int GetSliceHeader(char *comp_file, char *size_file, int compression_type,
  int framestep, int set_tmin, int set_tmax, float tmin_local, float tmax_local,
  int *nx, int *ny, int *nz, int *nsteps, int *ntotal, float *valmin, float *valmax){
  FILE *stream;
  int i1, i2, jj1, j2, k1, k2;
  float time_local;
  int ncompressed;
  int count;
  char buffer[256];
  int ncompressed_rle, ncompressed_zlib;

  stream = FOPEN(size_file, "r");
  if(stream == NULL){
    if(MakeSliceSizefile(comp_file, size_file, compression_type) == 0)return 0;
    stream = fopen(size_file, "r");
    if(stream == NULL)return 0;
  }

  if(fgets(buffer, 255, stream) == NULL){
    fclose(stream);
    return 0;
  }
  sscanf(buffer, "%i %i %i %i %i %i", &i1, &i2, &jj1, &j2, &k1, &k2);
  *nx = i2 + 1 - i1;
  *ny = j2 + 1 - jj1;
  *nz = k2 + 1 - k1;
  if(fgets(buffer, 255, stream) == NULL){
    fclose(stream);
    return 0;
  }
  sscanf(buffer, "%f %f", valmin, valmax);

  count = 0;
  *nsteps = 0;
  *ntotal = 0;
  while(!feof(stream)){

    if(fgets(buffer, 255, stream) == NULL)break;
    sscanf(buffer, "%f %i %i", &time_local, &ncompressed_zlib, &ncompressed_rle);
    if(compression_type == COMPRESSED_ZLIB){
      ncompressed = ncompressed_zlib;
    }
    else{
      ncompressed = ncompressed_rle;
    }
    if(count++%framestep != 0)continue;
    if(set_tmin == 1 && time_local < tmin_local)continue;
    if(set_tmax == 1 && time_local > tmax_local)continue;
    (*nsteps)++;
    *ntotal += ncompressed;
  }
  fclose(stream);
  return 2 + *nsteps;
}

/* ------------------ CReadSlice_frame ------------------------ */

int CReadSlice_frame(int frame_index_local,int sd_index,int flag){
  slicedata *sd;
  int slicefilelen;
  int headersize,framesize;
  int frame_size;
  long int skip_local;
  FILE *SLICEFILE;
  float *time_local,*slicevals;
  int error;
  int returncode;

  sd = sliceinfo + sd_index;
  if(sd->loaded==1)ReadSlice(sd->file,sd_index,UNLOAD,SET_SLICECOLOR,&error);
  if(flag==UNLOAD){
    FREEMEMORY(sd->qslicedata);
    FREEMEMORY(sd->times);
    return 0;
  }
  slicefilelen = strlen(sd->file);
  if(frame_index_local==first_frame_index){
    if(sd->compression_type==UNCOMPRESSED){

      FORTgetslicesizes(sd->file, &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, &sliceframestep,&error,
        &settmin_s, &settmax_s, &tmin_s, &tmax_s, &headersize, &framesize,
        slicefilelen);
    }
    else if(sd->compression_type==COMPRESSED_ZLIB){
      if(
        GetSliceHeader(sd->comp_file,sd->size_file,sd->compression_type,
                       sliceframestep,settmin_s,settmax_s,tmin_s,tmax_s,
                       &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, &sd->ncompressed, &sd->valmin, &sd->valmax)==0){
        ReadSlice("",sd_index,UNLOAD,SET_SLICECOLOR,&error);
        return -1;
      }
    }
  }
  skip_local =           (HEADER_SIZE+30        +TRAILER_SIZE); // long label
  skip_local +=          (HEADER_SIZE+30        +TRAILER_SIZE); // short label
  skip_local +=          (HEADER_SIZE+30        +TRAILER_SIZE); // unit label
  skip_local +=          (HEADER_SIZE+6*4        +TRAILER_SIZE); // is1, is2, js1, js2, ks1, ks2

  frame_size = sd->nslicei*sd->nslicej*sd->nslicek;
  skip_local += frame_index_local*(HEADER_SIZE + 4 + TRAILER_SIZE); //
  skip_local += frame_index_local*(HEADER_SIZE + frame_size*4 + TRAILER_SIZE); //

  SLICEFILE=fopen(sd->file,"rb");
  if(SLICEFILE==NULL){
    return -1;
  }

  FSEEK(SLICEFILE,skip_local,SEEK_SET); // skip from beginning of file

  if(frame_index_local==first_frame_index){
    if(NewMemory((void **)&sd->qslicedata,2*frame_size*sizeof(float))==0||
       NewMemory((void **)&sd->times,sizeof(float))==0){
      return -1;
    }
  }
  slicevals=sd->qslicedata;
  if(frame_index_local%2!=0){
    slicevals+=frame_size;
  }
  time_local=sd->times;

  FORTREAD(time_local,1,SLICEFILE);
  FORTREAD(slicevals,frame_size,SLICEFILE);
  fclose(SLICEFILE);
  return 0;
}

/* ------------------ OutputFedCSV ------------------------ */

void OutputFedCSV(void){
  FILE *AREA_STREAM=NULL;
  char *fed_area_file=NULL,fed_area_file_base[1024];
  int i;

  if(fed_areas == NULL)return;
  sprintf(fed_area_file_base, "%s_s%04i_fedarea.csv", fdsprefix, fed_seqnum++);
  fed_area_file = fed_area_file_base;
  AREA_STREAM = fopen(fed_area_file, "w");
  if(AREA_STREAM == NULL)return;

  fprintf(AREA_STREAM, "\"step\",\"0.0->0.3\",\"0.3->1.0\",\"1.0->3.0\",\"3.0->\"\n");
  for(i = 1; i < hists256_slice->ntotal; i++){
    int *areas;

    areas = fed_areas + 4 * CLAMP(i, 1, nhists256_slice);

    fprintf(AREA_STREAM,"%f,%i,%i,%i,%i\n",hists256_slice->time,areas[0],areas[1],areas[2],areas[3]);
  }
  fclose(AREA_STREAM);
}

/* ------------------ ReadFed ------------------------ */

void ReadFed(int file_index, int flag, int file_type, int *errorcode){
  feddata *fedi;
  slicedata *fed_slice,*o2,*co2,*co;
  isodata *fed_iso;
  int error_local;
  meshdata *meshi;
  int nx, ny;
  int nxy;
  int nxdata, nydata;
  int ibar, jbar, kbar;

#define FEDCO(CO) ( (2.764/100000.0)*pow(1000000.0*CLAMP(CO,0.0,0.1),1.036)/60.0 )
#define FEDO2(O2)  ( exp( -(8.13-0.54*(20.9-100.0*CLAMP(O2,0.0,0.2))) )/60.0 )
#define HVCO2(CO2) (exp(0.1930*CLAMP(CO2,0.0,0.1)*100.0+2.0004)/7.1)

  ASSERT(fedinfo!=NULL);
  ASSERT(file_index>=0);
  if(file_type==FED_SLICE){
    slicedata *slicei;

    ASSERT(file_index<nsliceinfo);
    slicei = sliceinfo + file_index;
    fedi = slicei->fedptr;
  }
  else if(file_type==FED_ISO){
    isodata *isoi;

    ASSERT(file_index<nisoinfo);
    isoi = isoinfo + file_index;
    fedi = isoi->fedptr;
  }
  else{
    return;
  }
  o2=fedi->o2;
  co2=fedi->co2;
  co=fedi->co;
  fed_slice=fedi->fed_slice;
  fed_iso=fedi->fed_iso;
  meshi = meshinfo + fed_slice->blocknumber;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  kbar = meshi->kbar;

  nx = meshi->ibar+1;
  ny = meshi->jbar+1;
  nxy = nx*ny;

  switch(fed_slice->idir){
    case XDIR:
      nxdata = co->js2 + 1 - co->js1;
      nydata = co->ks2 + 1 - co->ks1;
      break;
    case YDIR:
      nxdata = co->is2 + 1 - co->is1;
      nydata = co->ks2 + 1 - co->ks1;
      break;
    case ZDIR:
      nxdata = co->is2 + 1 - co->is1;
      nydata = co->js2 + 1 - co->js1;
      break;
    default:
      ASSERT(FFALSE);
      break;
  }

  if(file_type==FED_SLICE){
    ReadSlice(fed_slice->file,fedi->fed_index,UNLOAD,SET_SLICECOLOR,&error_local);
  }
  else if(file_type==FED_ISO){
    ReadIsoOrig(fed_iso->file,file_index,UNLOAD,&error_local);
  }

  if(flag==UNLOAD)return;

  // regenerate if either the FED slice or isosurface file does not exist or is older than
  // either the CO, CO2 or O2 slice files

  if(regenerate_fed==1||
     (file_type==FED_SLICE&&(IsFileNewer(fed_slice->file,o2->file)!=1||
       IsFileNewer(fed_slice->file,co2->file)!=1||
       IsFileNewer(fed_slice->file,co->file)!=1))||
       (file_type==FED_ISO&&(IsFileNewer(fed_iso->file,o2->file)!=1||
       IsFileNewer(fed_iso->file,co2->file)!=1||
       IsFileNewer(fed_iso->file,co->file)!=1))){
    int i,j,k;
    int frame_size;
    float *fed_frame,*fed_framem1;
    float *o2_frame1,*o2_frame2;
    float *co2_frame1,*co2_frame2;
    float *co_frame1,*co_frame2;
    float *times;

    char *iblank;

    NewMemory((void **)&iblank,nxdata*nydata*sizeof(char));
    for(j = 0; j<nxdata-1; j++){
      for(k = 0; k<nydata-1; k++){
        iblank[j+k*(nxdata-1)] = 0;
      }
    }
    switch(fed_slice->idir){
      case XDIR:
        if(meshi->c_iblank_x!=NULL){
          for(j = 0; j<nxdata-1; j++){
            for(k = 0; k<nydata-1; k++){
              iblank[j+k*(nxdata-1)] = meshi->c_iblank_x[IJKNODE(fed_slice->is1, fed_slice->js1+j, fed_slice->ks1+k)];
            }
          }
        }
        break;
      case YDIR:
        if(meshi->c_iblank_y!=NULL){
          for(i = 0; i<nxdata-1; i++){
            for(k = 0; k<nydata-1; k++){
              iblank[i+k*(nxdata-1)] = meshi->c_iblank_y[IJKNODE(fed_slice->is1+i, fed_slice->js1, fed_slice->ks1+k)];
            }
          }
        }
        break;
      case ZDIR:
        if(meshi->c_iblank_z!=NULL){
          for(i = 0; i<nxdata-1; i++){
            for(j = 0; j<nydata-1; j++){
              iblank[i+j*(nxdata-1)] = meshi->c_iblank_z[IJKNODE(fed_slice->is1+i, fed_slice->js1+j, fed_slice->ks1)];
            }
          }
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    PRINTF("\n");
    PRINTF("generating FED slice data\n");
    if(CReadSlice_frame(0,fedi->o2_index,LOAD)<0||
       CReadSlice_frame(0,fedi->co2_index,LOAD)<0||
       CReadSlice_frame(0,fedi->co_index,LOAD)<0){

       ReadFed(file_index,UNLOAD, file_type, errorcode);
       return;
    }

    fed_slice->is1=co->is1;
    fed_slice->is2=co->is2;
    fed_slice->js1=co->js1;
    fed_slice->js2=co->js2;
    fed_slice->ks1=co->ks1;
    fed_slice->ks2=co->ks2;
    fed_slice->nslicei=co->nslicei;
    fed_slice->nslicej=co->nslicej;
    fed_slice->nslicek=co->nslicek;
    fed_slice->volslice=co->volslice;
    if(fed_slice->volslice==1){
      if(fed_slice->nslicei!=fed_slice->is2+1-fed_slice->is1)fed_slice->is2=fed_slice->nslicei+fed_slice->is1-1;
      if(fed_slice->nslicej!=fed_slice->js2+1-fed_slice->js1)fed_slice->js2=fed_slice->nslicej+fed_slice->js1-1;
      if(fed_slice->nslicek!=fed_slice->ks2+1-fed_slice->ks1)fed_slice->ks2=fed_slice->nslicek+fed_slice->ks1-1;
    }
    fed_slice->ntimes=MIN(co->ntimes,co2->ntimes);
    fed_slice->ntimes=MIN(fed_slice->ntimes,o2->ntimes);
    frame_size = fed_slice->nslicei*fed_slice->nslicej*fed_slice->nslicek;
    fed_slice->nslicetotal=frame_size*fed_slice->ntimes;

    if(NewMemory((void **)&fed_slice->qslicedata,sizeof(float)*frame_size*fed_slice->ntimes)==0||
       NewMemory((void **)&fed_slice->times,sizeof(float)*fed_slice->ntimes)==0
       ){
       ReadFed(file_index,UNLOAD, file_type, errorcode);
      *errorcode=-1;
    }
    times=fed_slice->times;
    fed_frame=fed_slice->qslicedata;

    o2_frame1=o2->qslicedata;
    o2_frame2=o2_frame1+frame_size;

    co2_frame1=co2->qslicedata;
    co2_frame2=co2_frame1+frame_size;

    co_frame1=co->qslicedata;
    co_frame2=co_frame1+frame_size;

    times[0]=co2->times[0];
    for(i=0;i<frame_size;i++){
      fed_frame[i]=0.0;
    }
    for(i=1;i<fed_slice->ntimes;i++){
      int jj;
      float dt;

      if(CReadSlice_frame(i,fedi->o2_index,LOAD)<0||
         CReadSlice_frame(i,fedi->co2_index,LOAD)<0||
         CReadSlice_frame(i,fedi->co_index,LOAD)){
         ReadFed(file_index, UNLOAD, file_type,errorcode);
         return;
      }

      times[i]=co2->times[0];
      PRINTF("generating FED time=%.2f\n",times[i]);
      dt = (times[i]-times[i-1]);

      fed_framem1=fed_frame;
      fed_frame+=frame_size;
      for(jj=0;jj<frame_size;jj++){
        float val1, val2;
        float fed_co_val, fed_o2_val;

        val1=FEDCO(co_frame1[jj])*HVCO2(co2_frame1[jj]);
        val2=FEDCO(co_frame2[jj])*HVCO2(co2_frame2[jj]);
        fed_co_val = (val1+val2)*dt/2.0;

        val1=FEDO2(o2_frame1[jj]);
        val2=FEDO2(o2_frame2[jj]);
        fed_o2_val = (val1+val2)*dt/2.0;

        fed_frame[jj] = fed_framem1[jj] + fed_co_val + fed_o2_val;
      }
    }
    FREEMEMORY(iblank);
    OutSlicefile(fed_slice);
    if(fed_slice->volslice==1){
      float *xplt, *yplt, *zplt;
      char *iblank_cell;
      char longlabel[50],shortlabel[50],unitlabel[50];
      char *isofile;
      int error_local2;
      int reduce_triangles=1;
      int nz;

      strcpy(longlabel,"Fractional effective dose");
      strcpy(shortlabel,"FED");
      strcpy(unitlabel," ");

      xplt = meshi->xplt;
      yplt = meshi->yplt;
      zplt = meshi->zplt;
      ibar = meshi->ibar;
      jbar = meshi->jbar;
      kbar = meshi->kbar;
      nx = ibar + 1;
      ny = jbar + 1;
      nz = kbar + 1;
      isofile=fed_iso->file;

      iblank_cell = meshi->c_iblank_cell;

      CCIsoHeader(isofile,longlabel,shortlabel,unitlabel,fed_iso->levels,&fed_iso->nlevels,&error_local2);
      PRINTF("generating FED isosurface\n");
      for(i=0;i<fed_slice->ntimes;i++){
        float *vals;

        vals = fed_slice->qslicedata + i*frame_size;
        PRINTF("outputting isotime time=%.2f\n",times[i]);

//    C_val(i,j,k) = i*nj*nk + j*nk + k
// Fort_val(i,j,k) = i + j*ni + k*ni*nj

        CCIsoSurface2File(isofile, times+i, vals, iblank_cell,
                  fed_iso->levels, &fed_iso->nlevels,
                  xplt, &nx,  yplt, &ny, zplt, &nz,
                  &reduce_triangles, &error_local2);
      }
    }
    FREEMEMORY(fed_slice->qslicedata);
    FREEMEMORY(fed_slice->times);
    CReadSlice_frame(0,fedi->o2_index,UNLOAD);
    CReadSlice_frame(0,fedi->co2_index,UNLOAD);
    CReadSlice_frame(0,fedi->co_index,UNLOAD);
  }
  if(file_type==FED_SLICE){
    ReadSlice(fed_slice->file,fedi->fed_index,flag,SET_SLICECOLOR,&error_local);
  }
  else{
    ReadIsoOrig(fed_iso->file,file_index,flag,&error_local);
  }
  {
    colorbardata *cb;

#define COLORBAR_LIST2 112

    cb = GetColorbar(default_fed_colorbar);
    if(cb!=NULL){
      colorbartype=cb-colorbarinfo;
      SetColorbarListIndex(colorbartype);
      SliceBoundCB(COLORBAR_LIST2);
      UpdateCurrentColorbar(cb);
    }
  }
}

/* ------------------ ReadVSlice ------------------------ */

FILE_SIZE ReadVSlice(int ivslice, int flag, int *errorcode){
  vslicedata *vd;
  float valmin, valmax;
  int display;
  int i;
  FILE_SIZE return_filesize=0;
  int finalize = 0;

  valmin = 1000000000.0;
  valmax = -valmin;
  vd = vsliceinfo + ivslice;
  vd->u=NULL;
  vd->v=NULL;
  vd->w=NULL;
  vd->val=NULL;
  int set_slicecolor = SET_SLICECOLOR;

  if(flag==UNLOAD){
    if(vd->loaded==0)return 0;
    if(vd->iu!=-1){
      slicedata *u=NULL;

      u = sliceinfo + vd->iu;
      display=u->display;
      if(u->loaded==1){
        return_filesize+=ReadSlice(u->file, vd->iu, UNLOAD, SET_SLICECOLOR, errorcode);
      }
      u->display=display;
      u->vloaded=0;
    }
    if(vd->iv!=-1){
      slicedata *v=NULL;

      v = sliceinfo + vd->iv;
      display=v->display;
      if(v->loaded==1){
        return_filesize+=ReadSlice(v->file, vd->iv, UNLOAD, SET_SLICECOLOR, errorcode);
      }
      v->display=display;
      v->vloaded=0;
    }
    if(vd->iw!=-1){
      slicedata *w=NULL;

      w = sliceinfo + vd->iw;
      display=w->display;
      if(w->loaded==1){
        return_filesize+=ReadSlice(w->file, vd->iw, UNLOAD, SET_SLICECOLOR, errorcode);
      }
      w->display=display;
      w->vloaded=0;
    }
    if(vd->ival!=-1){
      slicedata *val=NULL;

      val = sliceinfo + vd->ival;
      display=val->display;
      if(val->loaded==1){
        return_filesize+=ReadSlice(val->file, vd->ival, UNLOAD, SET_SLICECOLOR, errorcode);
      }
      val->display=display;
      val->vloaded=0;
    }
    RemoveVSliceLoadstack(ivslice);
    vd->loaded=0;
    vd->display=0;
    showvslice=0;
    updatemenu=1;
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    return return_filesize;
  }
  if(vd->finalize==0)set_slicecolor = DEFER_SLICECOLOR;
  if(vd->iu!=-1){
    slicedata *u=NULL;

    u = sliceinfo + vd->iu;
    u->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->u=u;
    return_filesize+=ReadSlice(u->file,vd->iu,flag, set_slicecolor,errorcode);
    if(*errorcode!=0){
      vd->loaded=1;
      fprintf(stderr,"*** Error: unable to load U velocity vector components in %s . Vector load aborted\n",u->file);
      ReadVSlice(ivslice,UNLOAD,errorcode);
      *errorcode=1;
      return 0;
    }
    if(u->valmin<valmin)valmin=u->valmin;
    if(u->valmax>valmax)valmax=u->valmax;
    u->display=0;
    u->reload=0;
    u->vloaded=1;
  }
  if(vd->iv!=-1){
    slicedata *v=NULL;

    v = sliceinfo + vd->iv;
    v->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->v=v;
    return_filesize+=ReadSlice(v->file,vd->iv,flag, set_slicecolor,errorcode);
    if(*errorcode!=0){
      fprintf(stderr,"*** Error: unable to load V velocity vector components in %s . Vector load aborted\n",v->file);
      vd->loaded=1;
      ReadVSlice(ivslice,UNLOAD,errorcode);
      *errorcode=1;
      return 0;
    }

    if(v->valmin<valmin)valmin=v->valmin;
    if(v->valmax>valmax)valmax=v->valmax;
    v->display=0;
    v->reload=0;
    v->vloaded=1;
  }
  if(vd->iw!=-1){
    slicedata *w=NULL;

    w = sliceinfo + vd->iw;
    w->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->w=w;
    return_filesize+=ReadSlice(w->file,vd->iw,flag, set_slicecolor,errorcode);
    if(*errorcode!=0){
      fprintf(stderr,"*** Error: unable to load W velocity vector components in %s . Vector load aborted\n",w->file);
      vd->loaded=1;
      ReadVSlice(ivslice,UNLOAD,errorcode);
      *errorcode=1;
      return 0;
    }

    if(w->valmin<valmin)valmin=w->valmin;
    if(w->valmax>valmax)valmax=w->valmax;
    w->display=0;
    w->reload=0;
    w->vloaded=1;
  }
  vd->vslicefile_labelindex=-1;
  if(vd->ival!=-1){
    slicedata *val=NULL;

    val = sliceinfo + vd->ival;
    val->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->val=val;
    return_filesize+=ReadSlice(val->file,vd->ival,flag,set_slicecolor,errorcode);
    if(*errorcode!=0){
      fprintf(stderr,"*** Error: unable to load vector values in %s . Vector load aborted\n",val->file);
      vd->loaded=1;
      ReadVSlice(ivslice,UNLOAD,errorcode);
      *errorcode=1;
      return 0;
    }
    slicefile_labelindex= GetSliceBoundsIndex(val);
    vd->vslicefile_labelindex=val->slicefile_labelindex;
    vd->valmin=valmin;
    vd->valmax=valmax;
    val->display=0;
    val->vloaded=1;
    val->reload=0;
  }
  vd->display=1;
  vd->loaded=1;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  updatemenu=1;
  if(finalize==1){
    UpdateTimes();

    valmax = -100000.0;
    valmin = 100000.0;
    for(i = 0;i<nvsliceinfo;i++){
      vslicedata *vslicei;

      vslicei = vsliceinfo+i;
      if(vslicei->loaded==0)continue;
      if(vslicei->iu!=-1){
        slicedata *u = NULL;

        u = sliceinfo+vslicei->iu;
        valmin = MIN(u->valmin, valmin);
        valmax = MAX(u->valmax, valmax);
      }
      if(vslicei->iv!=-1){
        slicedata *v = NULL;

        v = sliceinfo+vslicei->iv;
        valmin = MIN(v->valmin, valmin);
        valmax = MAX(v->valmax, valmax);
      }
      if(vslicei->iw!=-1){
        slicedata *w = NULL;

        w = sliceinfo+vslicei->iw;
        valmin = MIN(w->valmin, valmin);
        valmax = MAX(w->valmax, valmax);
      }
    }
    velocity_range = valmax-valmin;
  }
  PushVSliceLoadstack(ivslice);

  PrintMemoryInfo;
  if(finalize==1)IdleCB();
  return return_filesize;
}

/* ------------------ UpdateSliceFilenum ------------------------ */

void UpdateSliceFilenum(void){
  slicedata *sd;
  int i;
  int ii;

  for(ii=0;ii<nslice_loaded;ii++){
    i = slice_loaded_list[ii];
    sd = sliceinfo+i;
    if(sd->display==0||slicefile_labelindex!=sd->slicefile_labelindex)continue;
    slicefilenum=i;
    break;
  }
}

/* ------------------ UncompressSliceDataFrame ------------------------ */

void UncompressSliceDataFrame(slicedata *sd, int iframe_local){
  unsigned int countin;
  uLongf countout;
  unsigned char *compressed_data;

  compressed_data = sd->qslicedata_compressed + sd->compindex[iframe_local].offset;
  countin = sd->compindex[iframe_local].size;
  countout = sd->nsliceijk;

  if(sd->compression_type == COMPRESSED_ZLIB){
    UnCompressZLIB(sd->slicecomplevel, &countout, compressed_data, countin);
  }
}

/* ------------------ GetSliceHists ------------------------ */

void GetSliceHists(slicedata *sd){
  int ndata;
  int n, i;
  int nframe;
  float *pdata0;

  int istep;
  int nx, ny, nxy, ibar, jbar;
  int ntimes;
  char *iblank_node, *iblank_cell, *slice_mask0;
  meshdata *meshi;
  float *slice_weight0;
  float *xplt, *yplt, *zplt;

  if(sd->histograms != NULL)return;
  meshi = meshinfo + sd->blocknumber;
  iblank_node = meshi->c_iblank_node;
  iblank_cell = meshi->c_iblank_cell;
  xplt = meshi->xplt_orig;
  yplt = meshi->yplt_orig;
  zplt = meshi->zplt_orig;

  ibar = meshi->ibar;
  jbar = meshi->jbar;
  nx = ibar + 1;
  ny = jbar + 1;
  nxy = nx*ny;

  ndata = sd->nslicetotal;

  nframe = sd->nslicei*sd->nslicej*sd->nslicek;
  NewMemory((void **)&slice_mask0, sd->nslicei*sd->nslicej*sd->nslicek);
  NewMemory((void **)&slice_weight0, sd->nslicei*sd->nslicej*sd->nslicek*sizeof(float));
  n = -1;
  for(i = 0; i < sd->nslicei; i++){
    int j;
    float dx;
    int i1, i1p1;

    i1 = MIN(sd->is1+i,sd->is2-2);
    i1p1 = i1+1;
    dx = xplt[i1p1] - xplt[i1];
    if(dx <= 0.0)dx = 1.0;

    for(j = 0; j < sd->nslicej; j++){
      int k;
      float dy;
      int j1, j1p1;

      j1 = MIN(sd->js1+j,sd->js2-2);
      j1p1 = j1+1;
      dy = yplt[j1p1] - yplt[j1];
      if(dy <= 0.0)dy = 1.0;

      for(k = 0; k < sd->nslicek; k++){
        float dz;
        int k1, k1p1;

        k1 = MIN(sd->ks1+k,sd->ks2-2);
        k1p1 = k1+1;
        dz = zplt[k1p1] - zplt[k1];
        if(dz <= 0.0)dz = 1.0;

        n++;
        slice_mask0[n] = 0;
        slice_weight0[n] = dx*dy*dz;
        if(sd->slicefile_type == SLICE_CELL_CENTER &&
          ((k == 0 && sd->nslicek != 1) || (j == 0 && sd->nslicej != 1) || (i == 0 && sd->nslicei != 1)))continue;
        if(show_slice_in_obst == ONLY_IN_GAS){
          if(sd->slicefile_type != SLICE_CELL_CENTER&& iblank_node != NULL&&iblank_node[IJKNODE(sd->is1 + i, sd->js1 + j, sd->ks1 + k)] == SOLID)continue;
          if(sd->slicefile_type == SLICE_CELL_CENTER&& iblank_cell != NULL&&iblank_cell[IJKCELL(sd->is1 + i - 1, sd->js1 + j - 1, sd->ks1 + k - 1)] == EMBED_YES)continue;
        }
        slice_mask0[n] = 1;
      }
    }
  }

  ntimes = ndata / sd->nsliceijk;

  // initialize histograms

  sd->nhistograms = ntimes + 1;
  NewMemory((void **)&sd->histograms, sd->nhistograms * sizeof(histogramdata));
  NewMemory((void **)&pdata0, sd->nslicei*sd->nslicej*sd->nslicek * sizeof(float));
  for(i = 0; i < sd->nhistograms; i++){
    InitHistogram(sd->histograms + i, NHIST_BUCKETS, NULL, NULL);
  }

  n = 0;
  for(istep = 0; istep < ntimes; istep++){
    histogramdata *histi, *histall;
    int nn;

    if(sd->compression_type == COMPRESSED_ZLIB){
      UncompressSliceDataFrame(sd, istep);
    }
    for(nn = 0; nn < sd->nslicei*sd->nslicej*sd->nslicek; nn++){
      if(sd->compression_type == COMPRESSED_ZLIB){
        pdata0[nn] = sd->qval256[sd->slicecomplevel[nn]];
      }
      else{
        pdata0[nn] = sd->qslicedata[n + nn];
      }
    }
    n += sd->nslicei*sd->nslicej*sd->nslicek;

    // compute histogram for each timestep, histi and all time steps, histall

    histi = sd->histograms + istep + 1;
    histall = sd->histograms;
    CopyVals2Histogram(pdata0, slice_mask0, slice_weight0, nframe, histi);
    MergeHistogram(histall, histi, MERGE_BOUNDS);
  }
  FREEMEMORY(slice_mask0);
  FREEMEMORY(slice_weight0);
  FREEMEMORY(pdata0);
}

/* ------------------ GetAllSliceHists ------------------------ */

void GetAllSliceHists(void){
  int ii;

  for(ii = 0; ii < nslice_loaded; ii++){
    slicedata *sdi;
    int i;

    i = slice_loaded_list[ii];
    sdi = sliceinfo + i;
    if(sdi->histograms == NULL)GetSliceHists(sdi);
  }
}

/* ------------------ UpdateSliceHist ------------------------ */

void UpdateSliceHist(void){
  int i;
  int nmax;
  int is_fed = 0;

  histograms_defined = 1;
  GetAllSliceHists();
  if(hists256_slice != NULL){
    for(i = 0; i < nhists256_slice; i++){
      FreeHistogram(hists256_slice + i);
    }
    FREEMEMORY(hists256_slice);
  }

  if(hists12_slice != NULL){
    for(i = 0; i < nhists256_slice; i++){
      FreeHistogram(hists12_slice + i);
    }
    FREEMEMORY(hists12_slice);
  }

  nmax = 0;
  for(i = 0; i < nslice_loaded; i++){
    slicedata *slicei;

    slicei = sliceinfo + slice_loaded_list[i];
    if(slicei->slicefile_labelindex != slicefile_labelindex)continue;
    nmax = MAX(nmax, slicei->nhistograms);
  }

  nhists256_slice = nmax;
  if(nhists256_slice > 0){
    boundsdata *sb;
    float maxval;

    sb = slicebounds + slicefile_labelindex;
    NewMemory((void **)&hists256_slice, nhists256_slice * sizeof(histogramdata));
    NewMemory((void **)&hists12_slice, nhists256_slice * sizeof(histogramdata));
    for(i = 0; i < nhists256_slice; i++){
      histogramdata *hist256i;
      histogramdata *hist12i;
      float val12_min, val12_max, dval;
      float val256_min, val256_max;

      dval = sb->valmax - sb->valmin;

      hist256i = hists256_slice + i;
      hist12i = hists12_slice + i;

      val256_min = sb->valmin - dval / (float)(histogram_nbuckets - 2);
      val256_max = sb->valmax + dval / (float)(histogram_nbuckets - 2);
      InitHistogram(hist256i, histogram_nbuckets, &val256_min, &val256_max);

      val12_min = sb->valmin - dval / 10.0;
      val12_max = sb->valmax + dval / 10.0;
      InitHistogram(hist12i, 12, &val12_min, &val12_max);
    }
    for(i = 0; i < nslice_loaded; i++){
      slicedata *slicei;
      int j;

      slicei = sliceinfo + slice_loaded_list[i];
      if(slicei->slicefile_labelindex != slicefile_labelindex)continue;
      if(slicei->is_fed == 1)is_fed = 1;
      for(j = 0; j < MIN(slicei->nhistograms,nhists256_slice); j++){
        histogramdata *hist256j, *hist12j, *histj;

        histj = slicei->histograms + j;
        hist256j = hists256_slice + j;
        hist12j = hists12_slice + j;
        MergeHistogram(hist256j, histj, KEEP_BOUNDS);
        MergeHistogram(hist12j, histj, KEEP_BOUNDS);
      }
    }
    for(i = 0; i < nslice_loaded; i++){
      slicedata *slicei;
      int j;

      slicei = sliceinfo + slice_loaded_list[i];
      if(slicei->slicefile_labelindex != slicefile_labelindex)continue;
      if(slicei->is_fed == 0)continue;
      for(j = 0; j < MIN(slicei->nhistograms, nhists256_slice); j++){
        histogramdata *hist256j;

        hist256j = hists256_slice + j;
        hist256j->time_defined = 1;
        hist256j->time = slicei->times[MIN(j,slicei->ntimes-1)];
      }
      break;
    }
    for(i = 0; i < nhists256_slice; i++){
      histogramdata *hist256i;
      int j;

      hist256i = hists256_slice + i;
      maxval = (float)hist256i->buckets[0] / (float)hist256i->ntotal;
      for(j = 1; j < histogram_nbuckets; j++){
        float val;

        val = (float)hist256i->buckets[j] / (float)hist256i->ntotal;
        maxval = MAX(val, maxval);
      }
      hist256i->bucket_maxval = maxval;
    }
    FREEMEMORY(fed_areas);
    if(is_fed == 1){
      NewMemory((void **)&fed_areas, 4*nhists256_slice * sizeof(int));
      for(i = 0; i < nhists256_slice; i++){
        int *fed_areasi;
        float hist0p0, hist0p3, hist1p0, hist3p0;

        hist0p0 = GetHistogramCDF(hists256_slice + i, 0.0);
        hist0p3 = GetHistogramCDF(hists256_slice + i, 0.3);
        hist1p0 = GetHistogramCDF(hists256_slice + i, 1.0);
        hist3p0 = GetHistogramCDF(hists256_slice + i, 3.0);

        fed_areasi = fed_areas + 4 * i;
        fed_areasi[0] = 100 * (hist0p3 - hist0p0);
        fed_areasi[1] = 100 * (hist1p0 - hist0p3);
        fed_areasi[2] = 100 * (hist3p0 - hist1p0);
        fed_areasi[3] = 100 * (1.0     - hist3p0);
      }
    }
  }

}

/* ------------------ UpdateSliceBounds ------------------------ */

void UpdateSliceBounds(void){
  int i, j;
  float valmin, valmax;
  float valmin_data, valmax_data;
  int minflag, maxflag;
  int minflag2, maxflag2;
  int jj;

  for(i=0;i<nslicebounds;i++){
    minflag=0; maxflag=0;
    minflag2=0; maxflag2=0;
    for(jj=0;jj<nslice_loaded;jj++){
      slicedata *slicej;

      j = slice_loaded_list[jj];
      slicej = sliceinfo + j;
      if(slicej->slicefile_labelindex!=i)continue;
      if(is_fed_colorbar==1&&slicej->is_fed==1){
        slicebounds[i].setvalmin=SET_MIN;
        slicebounds[i].valmin=0.0;
      }
      if(slicebounds[i].setvalmin!=SET_MIN){
        if(minflag==0){
          valmin=slicej->valmin;
          minflag=1;
        }
        else{
          if(sliceinfo[j].valmin<valmin)valmin=slicej->valmin;
        }
      }
      if(minflag2==0){
        valmin_data=sliceinfo[j].valmin_data;
        minflag2=1;
      }
      else{
        if(slicej->valmin_data<valmin_data)valmin_data=slicej->valmin_data;
      }
    }
    for(jj=0;jj<nslice_loaded;jj++){
      slicedata *slicej;

      j = slice_loaded_list[jj];
      slicej = sliceinfo + j;
      if(slicej->slicefile_labelindex!=i)continue;
      if(is_fed_colorbar==1&&slicej->is_fed==1){
        slicebounds[i].setvalmax=SET_MAX;
        slicebounds[i].valmax=3.0;
      }
      if(slicebounds[i].setvalmax!=SET_MAX){
        if(maxflag==0){
          valmax=sliceinfo[j].valmax;
          maxflag=1;
        }
        else{
          if(sliceinfo[j].valmax>valmax)valmax=slicej->valmax;
        }
      }
      if(maxflag2==0){
        valmax_data=slicej->valmax_data;
        maxflag2=1;
      }
      else{
        if(slicej->valmax_data>valmax_data)valmax_data=slicej->valmax_data;
      }
    }
    if(minflag==1)slicebounds[i].valmin=valmin;
    if(maxflag==1)slicebounds[i].valmax=valmax;
    if(minflag2==1)slicebounds[i].valmin_data=valmin_data;
    if(maxflag2==1)slicebounds[i].valmax_data=valmax_data;
  }
}

/* ------------------ SetSliceLabels ------------------------ */

void SetSliceLabels(float smin, float smax,
  slicedata *sd, patchdata *pd, int *errorcode){
  char *scale;
  int slicetype;
  boundsdata *sb;

  ASSERT((sd != NULL && pd == NULL)||(sd == NULL && pd != NULL));
  if(sd!=NULL)slicetype = GetSliceBoundsIndexFromLabel(sd->label.shortlabel);
  if(pd != NULL)slicetype = GetSliceBoundsIndexFromLabel(pd->label.shortlabel);
  sb = slicebounds + slicetype;
  if(sd!=NULL)sb->label = &(sd->label);
  if(pd != NULL)sb->label = &(pd->label);

  *errorcode = 0;
  scale = sb->scale;
  GetSliceLabels(smin, smax, nrgb, sb->colorlabels, &scale, &sb->fscale, sb->levels256);
}

/* ------------------ UpdateAllSliceLabels ------------------------ */

void UpdateAllSliceLabels(int slicetype, int *errorcode){
  int i;
  float valmin, valmax;
  int setvalmin, setvalmax;
  int ii;
  slicedata *sd;

  *errorcode=0;

  setvalmin=slicebounds[slicetype].setvalmin;
  setvalmax=slicebounds[slicetype].setvalmax;
  if(setvalmin==1){
    valmin=slicebounds[slicetype].valmin;
  }
  else{
    valmin=slicebounds[slicetype].valmin_data;
    slicebounds[slicetype].valmin=valmin;
  }
  if(setvalmax==1){
    valmax=slicebounds[slicetype].valmax;
  }
  else{
    valmax=slicebounds[slicetype].valmax_data;
    slicebounds[slicetype].valmax=valmax;
  }
  for(ii=0;ii<nslice_loaded;ii++){
    i = slice_loaded_list[ii];
    sd = sliceinfo + i;
    if(sd->slicefile_labelindex == slicetype){
      SetSliceLabels(valmin, valmax, sd, NULL, errorcode);
    }
    if(*errorcode!=0)return;
  }
  SetSliceBounds(slicetype);
  UpdateGlui();
}

/* ------------------ SetSliceColors ------------------------ */

void SetSliceColors(float smin, float smax,
  slicedata *sd, int *errorcode){
  char *scale;
  int slicetype;
  boundsdata *sb;

  slicetype = GetSliceBoundsIndex(sd);
  sb = slicebounds + slicetype;
  sb->label = &(sd->label);


  *errorcode = 0;
  scale = sb->scale;
  if(sd->slicefile_type == SLICE_GEOM){
    patchdata *patchgeom;

    patchgeom = sd->patchgeom;
    GetSliceColors(patchgeom->geom_vals, patchgeom->geom_nvals, patchgeom->geom_ivals,
      smin, smax,
      nrgb_full, nrgb,
      sb->colorlabels, &scale, &sb->fscale, sb->levels256,
      &sd->extreme_min, &sd->extreme_max
    );
  }
  else{
    if(sd->qslicedata == NULL)return;
    GetSliceColors(sd->qslicedata, sd->nslicetotal, sd->slicelevel,
      smin, smax,
      nrgb_full, nrgb,
      sb->colorlabels, &scale, &sb->fscale, sb->levels256,
      &sd->extreme_min, &sd->extreme_max
    );
  }
}

/* ------------------ UpdateAllSliceColors ------------------------ */

void UpdateAllSliceColors(int slicetype, int *errorcode){
  int i;
  float valmin, valmax;
  int setvalmin, setvalmax;
  int ii;
  slicedata *sd;

  *errorcode=0;

  setvalmin=slicebounds[slicetype].setvalmin;
  setvalmax=slicebounds[slicetype].setvalmax;
  if(setvalmin==1){
    valmin=slicebounds[slicetype].valmin;
  }
  else{
    valmin=slicebounds[slicetype].valmin_data;
    slicebounds[slicetype].valmin=valmin;
  }
  if(setvalmax==1){
    valmax=slicebounds[slicetype].valmax;
  }
  else{
    valmax=slicebounds[slicetype].valmax_data;
    slicebounds[slicetype].valmax=valmax;
  }
  for(ii=0;ii<nslice_loaded;ii++){
    i = slice_loaded_list[ii];
    sd = sliceinfo + i;
    if(sd->slicefile_labelindex!=slicetype)continue;
    SetSliceColors(valmin,valmax,sd,errorcode);
    if(*errorcode!=0)return;
  }
  SetSliceBounds(slicetype);
  UpdateGlui();
}

/* ------------------ SliceCompare ------------------------ */

int SliceCompare( const void *arg1, const void *arg2 ){
  slicedata *slicei, *slicej;

  slicei = sliceinfo + *(int *)arg1;
  slicej = sliceinfo + *(int *)arg2;

  if(slicei->menu_show>slicej->menu_show)return -1;
  if(slicei->menu_show<slicej->menu_show)return 1;
  if(slicei->mesh_type<slicej->mesh_type)return -1;
  if(slicei->mesh_type>slicej->mesh_type)return 1;
  if(slicei->slicefile_type<slicej->slicefile_type)return -1;
  if(slicei->slicefile_type>slicej->slicefile_type)return 1;

  if( strncmp(slicei->label.longlabel,"VE",2)==0){
    if(
      strncmp(slicej->label.longlabel,"U-",2)==0||
      strncmp(slicej->label.longlabel,"V-",2)==0||
      strncmp(slicej->label.longlabel,"W-",2)==0){
      return -1;
    }
  }
  if(strncmp(slicej->label.longlabel,"VE",2)==0){
    if(
      strncmp(slicei->label.longlabel,"U-",2)==0||
      strncmp(slicei->label.longlabel,"V-",2)==0||
      strncmp(slicei->label.longlabel,"W-",2)==0){
      return 1;
    }
  }
  if(strcmp(slicei->label.longlabel,slicej->label.longlabel)<0)return -1;
  if(strcmp(slicei->label.longlabel,slicej->label.longlabel)>0)return 1;
  if(slicei->volslice>slicej->volslice)return -1;
  if(slicei->volslice<slicej->volslice)return 1;
  if(slicei->idir<slicej->idir)return -1;
  if(slicei->idir>slicej->idir)return 1;
  if(slicei->volslice==0){
    float slice_eps, delta_slice;

    slice_eps = MAX(slicei->delta_orig, slicej->delta_orig);
    delta_slice = slicei->position_orig-slicej->position_orig;
    if(delta_slice<-slice_eps)return -1;
    if(delta_slice>slice_eps)return 1;
  }
  if(slicei->blocknumber<slicej->blocknumber)return -1;
  if(slicei->blocknumber>slicej->blocknumber)return 1;
  return 0;
}

/* ------------------ VSliceCompare ------------------------ */

int VSliceCompare( const void *arg1, const void *arg2 ){
  slicedata *slicei, *slicej;
  vslicedata *vslicei, *vslicej;
  float delta_orig;

  vslicei = vsliceinfo + *(int *)arg1;
  vslicej = vsliceinfo + *(int *)arg2;
  slicei = sliceinfo + vslicei->ival;
  slicej = sliceinfo + vslicej->ival;

  if(slicei->mesh_type<slicej->mesh_type)return -1;
  if(slicei->mesh_type>slicej->mesh_type)return 1;

  if( strncmp(slicei->label.longlabel,"VE",2)==0){
    if(
      strncmp(slicej->label.longlabel,"U-",2)==0||
      strncmp(slicej->label.longlabel,"V-",2)==0||
      strncmp(slicej->label.longlabel,"W-",2)==0){
      return -1;
    }
  }
  if(strncmp(slicej->label.longlabel,"VE",2)==0){
    if(
      strncmp(slicei->label.longlabel,"U-",2)==0||
      strncmp(slicei->label.longlabel,"V-",2)==0||
      strncmp(slicei->label.longlabel,"W-",2)==0){
      return 1;
    }
  }
  if(strcmp(slicei->label.longlabel,slicej->label.longlabel)<0)return -1;
  if(strcmp(slicei->label.longlabel,slicej->label.longlabel)>0)return 1;
  if(slicei->volslice<slicej->volslice)return -1;
  if(slicei->volslice>slicej->volslice)return 1;
  if(slicei->idir<slicej->idir)return -1;
  if(slicei->idir>slicej->idir)return 1;
  delta_orig = MAX(slicei->delta_orig,slicej->delta_orig);
  if(slicei->position_orig+delta_orig<slicej->position_orig)return -1;
  if(slicei->position_orig-delta_orig>slicej->position_orig)return 1;
  if(slicei->blocknumber<slicej->blocknumber)return -1;
  if(slicei->blocknumber>slicej->blocknumber)return 1;
  return 0;
}

/* ------------------ UpdateSliceMenuShow ------------------------ */

void UpdateSliceMenuShow(void){
  int i;

  for(i=0;i<nsliceinfo;i++){
    meshdata *slicemesh;
    slicedata *sd;

    sd = sliceinfo + i;
    slicemesh = meshinfo + sd->blocknumber;
    sd->menu_show=1;
    if(show_evac_slices==0&&slicemesh->mesh_type!=0){
      sd->menu_show=0;
    }
    if(strcmp(sd->label.longlabel,"Direction")==0&&constant_evac_coloring==1){
      sd->constant_color=direction_color_ptr;
    }
    else{
      sd->constant_color=NULL;
    }
  }
}

/* ------------------ UpdateSliceMenuLabels ------------------------ */

void UpdateSliceMenuLabels(void){
  int i;
  char label[128];
  multislicedata *mslicei;
  slicedata *sd,*sdold;

  UpdateSliceMenuShow();
  if(nsliceinfo>0){
    mslicei = multisliceinfo;
    sd = sliceinfo + sliceorderindex[0];
    STRCPY(mslicei->menulabel,sd->slicedir);
    STRCPY(sd->menulabel,mslicei->menulabel);

    STRCPY(mslicei->menulabel2,sd->label.longlabel);
    STRCAT(mslicei->menulabel2,", ");
    STRCAT(mslicei->menulabel2,sd->menulabel);

    if(nmeshes>1){
      meshdata *meshi;
      meshdata *slicemesh;

      slicemesh = meshinfo + sd->blocknumber;
      sprintf(label,", %s",slicemesh->label);
      STRCAT(sd->menulabel,label);
      meshi = meshinfo + sd->blocknumber;
      if(nevac>0){
        if(meshi->mesh_type==0){
          strcpy(label,", FDS mesh");
        }
        else{
          strcpy(label,", Evacuation mesh");
        }
        STRCAT(mslicei->menulabel2,label);
        STRCAT(mslicei->menulabel,label);
      }
    }
    if(showfiles==1){
      STRCAT(sd->menulabel,", ");
      STRCAT(sd->menulabel,sd->file);
    }
    if(sd->compression_type==COMPRESSED_ZLIB){
      STRCAT(sd->menulabel," (ZLIB)");
    }
    for(i=1;i<nsliceinfo;i++){
      meshdata *meshi;

      sdold = sliceinfo + sliceorderindex[i - 1];
      sd = sliceinfo + sliceorderindex[i];
      STRCPY(sd->menulabel,sd->slicedir);
      if(NewMultiSlice(sdold,sd)==1){
        mslicei++;
        STRCPY(mslicei->menulabel,sd->menulabel);
        STRCPY(mslicei->menulabel2,sd->label.longlabel);
        STRCAT(mslicei->menulabel2,", ");
        STRCAT(mslicei->menulabel2,sd->menulabel);
        meshi = meshinfo + sd->blocknumber;
        if(nevac>0){
          if(meshi->mesh_type==0){
            strcpy(label,", FDS mesh");
          }
          else{
            strcpy(label,", Evacuation mesh");
          }
          STRCAT(mslicei->menulabel2,label);
          STRCAT(mslicei->menulabel,label);
        }
      }
      if(nmeshes>1){
        meshdata *slicemesh;

        slicemesh = meshinfo + sd->blocknumber;
        sprintf(label,", %s",slicemesh->label);
        STRCAT(sd->menulabel,label);
      }
      if(showfiles==1){
        STRCAT(sd->menulabel,", ");
        STRCAT(sd->menulabel,sd->file);
      }
      if(sd->compression_type==COMPRESSED_ZLIB){
        STRCAT(sd->menulabel," (ZLIB)");
      }
    }
    for(i=0;i<nsliceinfo;i++){
      sd = sliceinfo + i;
      STRCPY(sd->menulabel2,sd->label.longlabel);
      STRCAT(sd->menulabel2,", ");
      STRCAT(sd->menulabel2,sd->menulabel);
    }
  }
}

/* ------------------ UpdateVsliceMenulabels ------------------------ */

void UpdateVsliceMenuLabels(void){
  int i;
  slicedata *sd, *sdold;
  vslicedata *vsd, *vsdold;
  multivslicedata *mvslicei;
  char label[128];


  if(nvsliceinfo>0){
    mvslicei = multivsliceinfo;
    vsd = vsliceinfo + vsliceorderindex[0];
    sd = sliceinfo + vsd->ival;

    STRCPY(mvslicei->menulabel,sd->slicedir);
    STRCPY(mvslicei->menulabel2,sd->label.longlabel);
    STRCAT(mvslicei->menulabel2,", ");
    STRCAT(mvslicei->menulabel2,sd->slicedir);

    STRCPY(vsd->menulabel,mvslicei->menulabel);
    STRCPY(vsd->menulabel2,mvslicei->menulabel2);
    if(nmeshes>1){
      meshdata *slicemesh;

      slicemesh = meshinfo + sd->blocknumber;
      sprintf(label,", %s",slicemesh->label);
      STRCAT(vsd->menulabel,label);
    }
    if(showfiles==1){
      STRCAT(vsd->menulabel,", ");
      STRCAT(vsd->menulabel,sd->file);
    }
    for(i=1;i<nvsliceinfo;i++){
      vsdold = vsliceinfo + vsliceorderindex[i - 1];
      sdold = sliceinfo + vsdold->ival;
      vsd = vsliceinfo + vsliceorderindex[i];
      sd = sliceinfo + vsd->ival;
      STRCPY(vsd->menulabel,sd->slicedir);
      if(NewMultiSlice(sdold,sd)==1){
        mvslicei++;
        STRCPY(mvslicei->menulabel,vsd->menulabel);
        STRCPY(mvslicei->menulabel2,sd->label.longlabel);
        STRCAT(mvslicei->menulabel2,", ");
        STRCAT(mvslicei->menulabel2,mvslicei->menulabel);
      }
      if(nmeshes>1){
        meshdata *slicemesh;

        slicemesh = meshinfo + sd->blocknumber;
        sprintf(label,", %s",slicemesh->label);
        STRCAT(vsd->menulabel,label);
      }
      if(showfiles==1){
        STRCAT(vsd->menulabel,", ");
        STRCAT(vsd->menulabel,sd->file);
      }
    }
    for(i=0;i<nvsliceinfo;i++){
      vsd = vsliceinfo + vsliceorderindex[i];
      sd = sliceinfo + vsd->ival;
      STRCPY(vsd->menulabel2,sd->label.longlabel);
      STRCAT(vsd->menulabel2,", ");
      STRCAT(vsd->menulabel2,vsd->menulabel);
    }
  }
}

/* ------------------ NewMultiSlice ------------------------ */

int NewMultiSlice(slicedata *sdold,slicedata *sd){
#ifdef pp_SLICE_USE_ID
    float same=0;
#endif

  if(sdold->volslice!=sd->volslice)return 1;
  if(sd->volslice==0){
    float delta_orig;
    float delta_scaled;

  // sd->delta is in FDS physical units
  // sd->xmin/xmax etc are in Smokeview scaled units
  // convert from physical to scaled units using xyzmaxdiff
    delta_orig = 1.5*MAX(sdold->delta_orig,sd->delta_orig);
    delta_scaled = SCALE2SMV(delta_orig);
#ifdef pp_SLICE_USE_ID
      if(use_new_slice_menus==0||sd->slcf_index==0){
        if(
        ABS(sd->xmin-sdold->xmin)<delta_scaled&&ABS(sd->xmax-sdold->xmax)<delta_scaled&&         // test whether two slices are identical
        ABS(sd->ymin-sdold->ymin)<delta_scaled&&ABS(sd->ymax-sdold->ymax)<delta_scaled&&
        ABS(sd->zmin-sdold->zmin)<delta_scaled&&ABS(sd->zmax-sdold->zmax)<delta_scaled
        )same=1;
      }
      else{
        if(sd->slcf_index==sdold->slcf_index)same=1;
      }
#endif
    if(
#ifdef pp_SLICE_USE_ID
      same==1&&
#else
      ABS(sd->xmin-sdold->xmin)<delta_scaled&&ABS(sd->xmax-sdold->xmax)<delta_scaled&&         // test whether two slices are identical
      ABS(sd->ymin-sdold->ymin)<delta_scaled&&ABS(sd->ymax-sdold->ymax)<delta_scaled&&
      ABS(sd->zmin-sdold->zmin)<delta_scaled&&ABS(sd->zmax-sdold->zmax)<delta_scaled&&
#endif
      sd->blocknumber==sdold->blocknumber
        ){
      return 1;
    }

    if(strcmp(sd->label.shortlabel,sdold->label.shortlabel)!=0
      ||sd->idir!=sdold->idir
#ifdef pp_SLICE_USE_ID
      ||(use_new_slice_menus==1&&sd->slcf_index!=0&&sd->slcf_index!=sdold->slcf_index)
      ||((use_new_slice_menus==0||sd->slcf_index==0)&&ABS(sd->position_orig-sdold->position_orig)>delta_orig)
#else
      ||ABS(sd->position_orig-sdold->position_orig)>delta_orig
#endif
      ||sd->mesh_type!=sdold->mesh_type
      ||sd->slicefile_type!=sdold->slicefile_type
        ){
      return 1;
    }
  }
  else{
    if(strcmp(sd->label.shortlabel,sdold->label.shortlabel)!=0
      ||sd->mesh_type!=sdold->mesh_type||sd->slicefile_type!=sdold->slicefile_type
        ){
      return 1;
    }
  }
  return 0;
}

/* ------------------ GetGSliceParams ------------------------ */

void GetGSliceParams(void){
  int i;

  for(i = 0; i < npatchinfo;i++){
    int ii1, ii2, jj1, jj2, kk1, kk2;
    patchdata *patchi;
    meshdata *meshi;

    patchi = patchinfo + i;
    meshi = meshinfo + patchi->blocknumber;
    strcpy(patchi->gslicedir, "");
    if(patchi->structured == YES)continue;
    ii1 = patchi->ijk[0];
    ii2 = patchi->ijk[1];
    jj1 = patchi->ijk[2];
    jj2 = patchi->ijk[3];
    kk1 = patchi->ijk[4];
    kk2 = patchi->ijk[5];
    if(ii1 >= 0 && ii2 >= 0 && jj1 >= 0 && jj2 >= 0 && kk1 >= 0 && kk2 >= 0){
      float *grid, position;

      if(ABS(ii1 - ii2) < MIN(ABS(jj1 - jj2), ABS(kk1 - kk2))){
        grid = meshi->xplt_orig;
        ii2=MAX(ii1-1,0);
        position = (grid[ii1] + grid[ii2]) / 2.0;
        sprintf(patchi->gslicedir, "X=%f", position);
      }
      else if(ABS(jj1 - jj2) < MIN(ABS(ii1 - ii2), ABS(kk1 - kk2))){
        grid = meshi->yplt_orig;
        jj2=MAX(jj1-1,0);
        position = (grid[jj1] + grid[jj2]) / 2.0;
        sprintf(patchi->gslicedir, "Y=%f", position);
      }
      else{
        grid = meshi->zplt_orig;
        kk2=MAX(kk1-1,0);
        position = (grid[kk1] + grid[kk2]) / 2.0;
        sprintf(patchi->gslicedir, "Z=%f", position);
      }
      TrimZeros(patchi->gslicedir);
      if(patchi->filetype_label != NULL&&strcmp(patchi->filetype_label, "INCLUDE_GEOM") == 0){
        char geomlabel[256];

        strcpy(geomlabel, " - ");
        strcat(geomlabel, patchi->gslicedir);
        AppendLabels(&(patchi->label),geomlabel);
      }
    }
    else{
      strcpy(patchi->gslicedir, patchi->label.longlabel);
    }
  }
}

/* ------------------ MaxDiff3B ------------------------ */

float MaxDiff3B(float *xyz1, float *xyz2, int idir){
  float maxdiff = -1.0;

  if(idir!=1)maxdiff = MAX(maxdiff, ABS(xyz1[0]-xyz2[0]));
  if(idir!=2)maxdiff = MAX(maxdiff, ABS(xyz1[1]-xyz2[1]));
  if(idir!=3)maxdiff = MAX(maxdiff, ABS(xyz1[2]-xyz2[2]));
  return maxdiff;
}

/* ------------------ IsSliceDuplicate ------------------------ */

#define SLICEEPS 0.001
#define COUNT_DUPLICATES 1
#define FIND_DUPLICATES 0
int IsSliceDuplicate(multislicedata *mslicei, int ii, int flag){
  int jj;
  float *xyzmini, *xyzmaxi;
  slicedata *slicei;

  if(flag==FIND_DUPLICATES&&slicedup_option==SLICEDUP_KEEPALL)return 0;
  slicei = sliceinfo+mslicei->islices[ii];
  xyzmini = slicei->xyz_min;
  xyzmaxi = slicei->xyz_max;
  for(jj=0;jj<mslicei->nslices;jj++){ // identify duplicate slices
    slicedata *slicej;
    float *xyzminj, *xyzmaxj;

    slicej = sliceinfo + mslicei->islices[jj];
    if(slicej==slicei||slicej->skip==1)continue;
    xyzminj = slicej->xyz_min;
    xyzmaxj = slicej->xyz_max;
    if(slicei->patchgeom==NULL){
      if(MAXDIFF3(xyzmini, xyzminj)<SLICEEPS&&MAXDIFF3(xyzmaxi, xyzmaxj)<SLICEEPS){
        if(flag==COUNT_DUPLICATES)return 1;
        if(slicedup_option==SLICEDUP_KEEPFINE  &&slicei->dplane_min>slicej->dplane_min-SLICEEPS)return 1;
        if(slicedup_option==SLICEDUP_KEEPCOARSE&&slicei->dplane_max<slicej->dplane_max+SLICEEPS)return 1;
      }
    }
    else{
      if(MaxDiff3B(xyzmini, xyzminj,slicei->idir)<SLICEEPS&&MaxDiff3B(xyzmaxi, xyzmaxj,slicei->idir)<SLICEEPS){
        if(flag==COUNT_DUPLICATES)return 1;
        if(slicedup_option==SLICEDUP_KEEPFINE  &&slicei->dplane_min>slicej->dplane_min-SLICEEPS)return 1;
        if(slicedup_option==SLICEDUP_KEEPCOARSE&&slicei->dplane_max<slicej->dplane_max+SLICEEPS)return 1;
      }
    }
  }
  return 0;
}

/* ------------------ IsVectorSliceDuplicate ------------------------ */

int IsVectorSliceDuplicate(multivslicedata *mvslicei, int i){
  int jj;
  float *xyzmini, *xyzmaxi;
  slicedata *slicei;
  vslicedata *vslicei;


  if(vectorslicedup_option==SLICEDUP_KEEPALL)return 0;
  vslicei = vsliceinfo + mvslicei->ivslices[i];
  slicei = sliceinfo + vslicei->ival;
  xyzmini = slicei->xyz_min;
  xyzmaxi = slicei->xyz_max;
  for(jj=0;jj<mvslicei->nvslices;jj++){ // identify duplicate slices
    vslicedata *vslicej;
    slicedata *slicej;
    float *xyzminj, *xyzmaxj;

    vslicej = vsliceinfo + mvslicei->ivslices[jj];
    slicej = sliceinfo + vslicej->ival;
    if(slicej==slicei||slicej->skip==1)continue;
    xyzminj = slicej->xyz_min;
    xyzmaxj = slicej->xyz_max;
    if(MAXDIFF3(xyzmini, xyzminj) < SLICEEPS&&MAXDIFF3(xyzmaxi, xyzmaxj) < SLICEEPS){
      if(vectorslicedup_option==SLICEDUP_KEEPFINE  &&slicei->dplane_min>slicej->dplane_min-SLICEEPS)return 1;
      if(vectorslicedup_option==SLICEDUP_KEEPCOARSE&&slicei->dplane_max<slicej->dplane_max+SLICEEPS)return 1;
    }
  }
  return 0;
}

/* ------------------ CountSliceDups ------------------------ */

int CountSliceDups(void){
  int i, count;

  count = 0;
  for(i = 0; i < nmultisliceinfo; i++){
    int ii;
    multislicedata *mslicei;

    mslicei = multisliceinfo + i;
    for(ii = 0; ii < mslicei->nslices; ii++){
      count += IsSliceDuplicate(mslicei, ii, COUNT_DUPLICATES);
    }
  }
  return count;
}

/* ------------------ UpdateSliceDups ------------------------ */

void UpdateSliceDups(void){
  int i;

  for(i=0;i<nmultisliceinfo;i++){
    int ii;
    multislicedata *mslicei;

    mslicei = multisliceinfo + i;
    for(ii=0;ii<mslicei->nslices;ii++){
      slicedata *slicei;

      slicei = sliceinfo + mslicei->islices[ii];
      slicei->skip=0;
    }
  }
  // look for duplicate slices
  for(i=0;i<nmultisliceinfo;i++){
    int ii;
    multislicedata *mslicei;

    mslicei = multisliceinfo + i;
    for(ii=0;ii<mslicei->nslices;ii++){
      slicedata *slicei;

      slicei = sliceinfo + mslicei->islices[ii];
      slicei->skip = IsSliceDuplicate(mslicei,ii, FIND_DUPLICATES);
    }
  }
}

/* ------------------ UpdateVSliceDups ------------------------ */

void UpdateVSliceDups(void){
  int ii;

  for(ii=0;ii<nvsliceinfo;ii++){
    vslicedata *vslicei;

    vslicei = vsliceinfo + ii;
    vslicei->skip = 0;
  }
  for(ii = 0; ii < nmultivsliceinfo; ii++){
    multivslicedata *mvslicei;
    int i;

    mvslicei = multivsliceinfo + ii;
    for(i = 0; i < mvslicei->nvslices; i++){
      vslicedata *vslicei;

      vslicei = vsliceinfo + mvslicei->ivslices[i];
      vslicei->skip = IsVectorSliceDuplicate(mvslicei,i);
    }
  }
 }

/* ------------------ UpdateFedinfo ------------------------ */

void UpdateFedinfo(void){
  int i;
  int nfediso = 0, ifediso = 0;
  FILE *stream_fedsmv = NULL;

  nfedinfo = 0;
  if(smokediff == 1)return;
  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;
    feddata *fedi;
    int j;

    fedi = fedinfo + nfedinfo;
    slicei = sliceinfo + i;

    fedi->co = NULL;
    fedi->co2 = NULL;
    fedi->o2 = NULL;
    fedi->fed_slice = NULL;
    fedi->fed_iso = NULL;
    fedi->co_index = -1;
    fedi->co2_index = -1;
    fedi->o2_index = -1;
    fedi->fed_index = -1;
    fedi->loaded = 0;
    fedi->display = 0;
    if(slicei->slicefile_type != SLICE_CELL_CENTER&&strcmp(slicei->label.longlabel, "CARBON DIOXIDE VOLUME FRACTION") != 0)continue;
    if(slicei->slicefile_type == SLICE_CELL_CENTER&&strcmp(slicei->label.longlabel, "CARBON DIOXIDE VOLUME FRACTION(cell centered)") != 0)continue;
    fedi->co2_index = i;
    for(j = 0; j < nsliceinfo; j++){
      slicedata *slicej;

      slicej = sliceinfo + j;
      if(slicei->slicefile_type != SLICE_CELL_CENTER&&strcmp(slicej->label.longlabel, "CARBON MONOXIDE VOLUME FRACTION") != 0)continue;
      if(slicei->slicefile_type == SLICE_CELL_CENTER&&strcmp(slicej->label.longlabel, "CARBON MONOXIDE VOLUME FRACTION(cell centered)") != 0)continue;
      if(slicei->blocknumber != slicej->blocknumber)continue;
      if(slicei->is1 != slicej->is1 || slicei->is2 != slicej->is2)continue;
      if(slicei->js1 != slicej->js1 || slicei->js2 != slicej->js2)continue;
      if(slicei->ks1 != slicej->ks1 || slicei->ks2 != slicej->ks2)continue; // skip if not the same size and in the same mesh
      fedi->co_index = j;
      break;
    }
    if(fedi->co_index == -1)continue;
    for(j = 0; j < nsliceinfo; j++){
      slicedata *slicej;

      slicej = sliceinfo + j;
      if(slicei->slicefile_type != SLICE_CELL_CENTER&&strcmp(slicej->label.longlabel, "OXYGEN VOLUME FRACTION") != 0)continue;
      if(slicei->slicefile_type == SLICE_CELL_CENTER&&strcmp(slicej->label.longlabel, "OXYGEN VOLUME FRACTION(cell centered)") != 0)continue;
      if(slicei->blocknumber != slicej->blocknumber)continue;
      if(slicei->is1 != slicej->is1 || slicei->is2 != slicej->is2)continue;
      if(slicei->js1 != slicej->js1 || slicei->js2 != slicej->js2)continue;
      if(slicei->ks1 != slicej->ks1 || slicei->ks2 != slicej->ks2)continue; // skip if not the same size and in the same mesh
      fedi->o2_index = j;
      break;
    }
    if(fedi->o2_index == -1)continue;
    fedi->fed_index = nsliceinfo + nfedinfo;
    if(sliceinfo[fedi->co_index].volslice == 1)nfediso++;
    nfedinfo++;
  }
  if(nfedinfo == 0){
    FREEMEMORY(fedinfo);
    return;
  }
  else{
    nsliceinfo += nfedinfo;
    ResizeMemory((void **)&fedinfo, nfedinfo * sizeof(feddata));
    ResizeMemory((void **)&sliceinfo, nsliceinfo * sizeof(slicedata));
    ResizeMemory((void **)&vsliceinfo, 3 * nsliceinfo * sizeof(vslicedata));
    ResizeMemory((void **)&sliceinfo, nsliceinfo * sizeof(slicedata));
    ResizeMemory((void **)&fedinfo, nsliceinfo * sizeof(feddata));
    ResizeMemory((void **)&slice_loadstack, nsliceinfo * sizeof(int));
    ResizeMemory((void **)&vslice_loadstack, nsliceinfo * sizeof(int));
    ResizeMemory((void **)&subslice_menuindex, nsliceinfo * sizeof(int));
    ResizeMemory((void **)&msubslice_menuindex, nsliceinfo*sizeof(int));
    ResizeMemory((void **)&subvslice_menuindex, nsliceinfo * sizeof(int));
    ResizeMemory((void **)&msubvslice_menuindex, nsliceinfo*sizeof(int));
    ResizeMemory((void **)&mslice_loadstack, nsliceinfo * sizeof(int));
    ResizeMemory((void **)&mvslice_loadstack, nsliceinfo * sizeof(int));
    if(nfediso > 0){
      nisoinfo += nfediso;
      if(nisoinfo == nfediso){
        NewMemory((void **)&isoinfo, nisoinfo * sizeof(isodata));
        NewMemory((void **)&isotypes, nisoinfo * sizeof(int));
      }
      else{
        ResizeMemory((void **)&isoinfo, nisoinfo * sizeof(isodata));
        ResizeMemory((void **)&isotypes, nisoinfo * sizeof(int));
      }
    }
  }
  if(nfedinfo > 0 && fed_filename != NULL){
    stream_fedsmv = fopen(fed_filename, "w");
  }
  for(i = 0; i < nfedinfo; i++){ // define sliceinfo for fed slices
    slicedata *sd;
    slicedata *co2;
    int nn_slice;
    feddata *fedi;
    char *filename, filename_base[1024], *ext;

    sd = sliceinfo + nsliceinfo + (i - nfedinfo);
    fedi = fedinfo + i;
    fedi->co = sliceinfo + fedi->co_index;
    fedi->o2 = sliceinfo + fedi->o2_index;
    fedi->co2 = sliceinfo + fedi->co2_index;
    fedi->fed_slice = sliceinfo + fedi->fed_index;
    fedi->fed_iso = NULL;

    co2 = fedi->co2;
    sd->file = NULL;
    sd->is1 = co2->is1;
    sd->is2 = co2->is2;
    sd->js1 = co2->js1;
    sd->js2 = co2->js2;
    sd->ks1 = co2->ks1;
    sd->ks2 = co2->ks2;
    sd->finalize = 1;

    nn_slice = nsliceinfo + i;

    sd->is_fed = 1;
    sd->fedptr = fedi;
    sd->slicefile_type = co2->slicefile_type;
    if(sd->slicefile_type == SLICE_CELL_CENTER){
      SetLabels(&(sd->label), "Fractional effective dose(cell centered)", "FED", " ");
    }
    else{
      SetLabels(&(sd->label), "Fractional effective dose", "FED", " ");
    }
    sd->reg_file = NULL;
    sd->comp_file = NULL;
    sd->compression_type = co2->compression_type;
    sd->vol_file = co2->vol_file;
    sd->size_file = NULL;
    sd->slicelabel = NULL;
    sd->above_ground_level = co2->above_ground_level;
    sd->seq_id = nn_slice;
    sd->autoload = 0;
    sd->display = 0;
    sd->loaded = 0;
    sd->qslicedata = NULL;
    sd->compindex = NULL;
    sd->slicecomplevel = NULL;
    sd->qslicedata_compressed = NULL;
    sd->volslice = fedi->co->volslice;
    sd->times = NULL;
    sd->slicelevel = NULL;
    sd->iqsliceframe = NULL;
    sd->qsliceframe = NULL;
    sd->timeslist = NULL;
    sd->blocknumber = co2->blocknumber;
    sd->vloaded = 0;
    sd->reload = 0;
    sd->nline_contours = 0;
    sd->line_contours = NULL;
    sd->menu_show = 1;
    sd->constant_color = NULL;
    sd->mesh_type = co2->mesh_type;
    sd->histograms = NULL;
    sd->nhistograms = 0;

    strcpy(filename_base, fedi->co->file);
    ext = strrchr(filename_base, '.');
    *ext = 0;
    strcat(filename_base, "_fed.sf");
    filename = GetFileName(smokeviewtempdir, filename_base, tempdir_flag);
    NewMemory((void **)&fedi->fed_slice->reg_file, strlen(filename) + 1);
    strcpy(sd->reg_file, filename);
    FREEMEMORY(filename);
    sd->file = sd->reg_file;
    if(stream_fedsmv != NULL){
      fprintf(stream_fedsmv, "SLCF %i %s %i %i %i %i %i %i\n", sd->blocknumber + 1, "%", sd->is1, sd->is2, sd->js1, sd->js2, sd->ks1, sd->ks2);
      fprintf(stream_fedsmv, " %s\n", sd->reg_file);
      fprintf(stream_fedsmv, " %s\n", sd->label.longlabel);
      fprintf(stream_fedsmv, " %s\n", sd->label.shortlabel);
      fprintf(stream_fedsmv, " %s\n", sd->label.unit);
    }
    if(sd->volslice == 1){
      isodata *isoi;
      int nn_iso, ii;
      float **colorlevels;

      isoi = isoinfo + nisoinfo - nfediso + ifediso;
      fedi->fed_iso = isoi;
      isoi->tfile = NULL;
      isoi->is_fed = 1;
      isoi->fedptr = fedi;
      nn_iso = nisoinfo - nfediso + ifediso + 1;
      isoi->seq_id = nn_iso;
      isoi->autoload = 0;
      isoi->blocknumber = sd->blocknumber;
      isoi->loaded = 0;
      isoi->display = 0;
      isoi->dataflag = 0;
      isoi->geomflag = 0;
      isoi->levels = NULL;
      SetLabels(&(isoi->surface_label), "Fractional effective dose", "FED", " ");

      isoi->nlevels = 3;
      NewMemory((void **)&(isoi->levels), 3 * sizeof(float));
      NewMemory((void **)&colorlevels, 3 * sizeof(float *));
      isoi->colorlevels = colorlevels;
      for(ii = 0; ii < 3; ii++){
        colorlevels[ii] = NULL;
      }
      isoi->levels[0] = 0.3;
      isoi->levels[1] = 1.0;
      isoi->levels[2] = 3.0;
      SetLabelsIso(&(isoi->surface_label), "Fractional effective dose", "FED", " ", isoi->levels, isoi->nlevels);
      isoi->normaltable = NULL;
      isoi->color_label.longlabel = NULL;
      isoi->color_label.shortlabel = NULL;
      isoi->color_label.unit = NULL;
      isoi->geominfo = NULL;

      strcpy(filename_base, fedi->co->file);
      ext = strrchr(filename_base, '.');
      *ext = 0;
      strcat(filename_base, "_fed.iso");
      filename = GetFileName(smokeviewtempdir, filename_base, tempdir_flag);
      NewMemory((void **)&isoi->reg_file, strlen(filename) + 1);
      strcpy(isoi->reg_file, filename);
      FREEMEMORY(filename);
      isoi->file = isoi->reg_file;
      if(stream_fedsmv != NULL){
        fprintf(stream_fedsmv, "ISOF\n");
        fprintf(stream_fedsmv, " %s\n", isoi->surface_label.longlabel);
        fprintf(stream_fedsmv, " %s\n", isoi->surface_label.shortlabel);
        fprintf(stream_fedsmv, " %s\n", isoi->surface_label.unit);
        fprintf(stream_fedsmv, " %s\n", isoi->reg_file);
      }

      NewMemory((void **)&isoi->size_file, strlen(isoi->file) + 3 + 1);
      STRCPY(isoi->size_file, isoi->file);
      STRCAT(isoi->size_file, ".sz");

      ifediso++;
    }
  }
  if(stream_fedsmv != NULL)fclose(stream_fedsmv);
  if(nfediso > 0)UpdateIsoMenuLabels();
}

/* ------------------ UpdateSliceDirCount ------------------------ */

void UpdateSliceDirCount(void){
  int i, j;

  for(i = 0; i < nmultisliceinfo; i++){
    multislicedata *mslicei;

    mslicei = multisliceinfo + i;
    mslicei->ndirxyz[0] = 0;
    mslicei->ndirxyz[1] = 0;
    mslicei->ndirxyz[2] = 0;
    mslicei->ndirxyz[3] = 0;
  }
  for(i = 0; i < nmultisliceinfo; i++){
    multislicedata *mslicei;
    slicedata *slicei;

    mslicei = multisliceinfo + i;
    slicei = sliceinfo + mslicei->islices[0];
    if(slicei->idir < 1)continue;
    if(slicei->volslice == 1)continue;
    for(j = 0; j < nmultisliceinfo; j++){
      multislicedata *mslicej;
      slicedata *slicej;

      mslicej = multisliceinfo + j;
      slicej = sliceinfo + mslicej->islices[0];
      if(slicej->idir < 1)continue;
      if(slicej->volslice == 1)continue;
      if(strcmp(slicej->label.longlabel, slicei->label.longlabel) != 0)continue;
      if((slicej->slicefile_type == SLICE_CELL_CENTER&&slicei->slicefile_type != SLICE_CELL_CENTER) ||
        (slicej->slicefile_type != SLICE_CELL_CENTER&&slicei->slicefile_type == SLICE_CELL_CENTER))continue;
      mslicei->ndirxyz[slicej->idir]++;
    }
  }
  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo + i;
    slicei->ndirxyz[0] = 0;
    slicei->ndirxyz[1] = 0;
    slicei->ndirxyz[2] = 0;
    slicei->ndirxyz[3] = 0;
  }
  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei, *slicej;

    slicei = sliceinfo + i;
    if(slicei->idir < 1)continue;
    if(slicei->volslice == 1)continue;
    for(j = 0; j < nsliceinfo; j++){
      slicej = sliceinfo + j;
      if(slicej->idir < 1)continue;
      if(slicej->volslice == 1)continue;
      if(strcmp(slicej->label.longlabel, slicei->label.longlabel) != 0)continue;
      //if(slicej->cellcenter!=slicei->cellcenter)continue;
      if((slicej->slicefile_type == SLICE_CELL_CENTER&&slicei->slicefile_type != SLICE_CELL_CENTER) ||
        (slicej->slicefile_type != SLICE_CELL_CENTER&&slicei->slicefile_type == SLICE_CELL_CENTER))continue;
      slicei->ndirxyz[slicej->idir]++;
    }
  }
}

/* ------------------ GetSliceParams ------------------------ */

void GetSliceParams(void){
  int i;
  int error;

  int build_cache=0;
  FILE *stream;

  if(IfFirstLineBlank(sliceinfo_filename)==1){
    build_cache=1;
    stream=fopen(sliceinfo_filename,"w");
  }
  else{
    build_cache=0;
    stream=fopen(sliceinfo_filename,"r");
  }

  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;
    int is1, is2, js1, js2, ks1, ks2;
    int iis1, iis2;
    int ni, nj, nk;

    sd = sliceinfo + i;

#ifdef _DEBUG
    if(nsliceinfo>100&&(i%100==0||i==nsliceinfo-1)){
      PRINTF("    obtaining parameters from %i'st slice file\n",i+1);
    }
#endif

    if(sd->compression_type==UNCOMPRESSED){
      int doit_anyway;

      doit_anyway=0;
      if(build_cache==0&&stream!=NULL){
        int seq=-1;

        while(seq!=sd->seq_id){
          char buffer[255];

          if(fgets(buffer,255,stream)==NULL){
            doit_anyway=1;
            break;
          }
          sscanf(buffer,"%i %i %i %i %i %i %i %i %i %i %i",&seq,&is1,&is2,&js1,&js2,&ks1,&ks2,&ni,&nj,&nk,&sd->volslice);
        }
        error=0;
      }
      if(build_cache==1||stream==NULL||doit_anyway==1){
        int idir, joff, koff, volslice;

        is1=sd->is1;
        is2=sd->is2;
        js1=sd->js1;
        js2=sd->js2;
        ks1=sd->ks1;
        ks2=sd->ks2;
        FORTgetslicefiledirection(&is1, &is2, &iis1, &iis2, &js1, &js2, &ks1, &ks2, &idir, &joff, &koff,&volslice);
        if(volslice == 1){
          is1 = iis1;
          is2 = iis2;
        }
        ni = is2+1-is1;
        nj = js2+1-js1;
        nk = ks2+1-ks1;
        sd->volslice = volslice;
        error = 0;
        if(stream!=NULL&&doit_anyway==0)fprintf(stream,"%i %i %i %i %i %i %i %i %i %i %i\n",sd->seq_id,is1,is2,js1,js2,ks1,ks2,ni,nj,nk,sd->volslice);
      }
    }
    else if(sd->compression_type==COMPRESSED_ZLIB){
      error=0;
      if(GetSliceHeader0(sd->comp_file,sd->size_file,sd->compression_type,&is1,&is2,&js1,&js2,&ks1,&ks2, &sd->volslice)==0)error=1;
      ni = is2 + 1 - is1;
      nj = js2 + 1 - js1;
      nk = ks2 + 1 - ks1;
    }
    if(error==0){
      sd->is1=is1;
      sd->is2=is2;
      sd->js1=js1;
      sd->js2=js2;
      sd->ks1=ks1;
      sd->ks2=ks2;
      sd->nslicei=ni;
      sd->nslicej=nj;
      sd->nslicek=nk;
    }
  }
  UpdateFedinfo();
  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;
    int is1, is2, js1, js2, ks1, ks2;
    meshdata *meshi;

    sd = sliceinfo + i;
    meshi = meshinfo + sd->blocknumber;

    is1 = sd->is1;
    is2=sd->is2;
    js1=sd->js1;
    js2=sd->js2;
    ks1=sd->ks1;
    ks2=sd->ks2;
    if(error==0){
      float position;

      sd->idir=-1;

      strcpy(sd->slicedir,"");
      position=-999.0;
      if(sd->is1==sd->is2||(sd->js1!=sd->js2&&sd->ks1!=sd->ks2)){
        sd->idir=1;
        position = meshi->xplt_orig[is1];
        if(sd->slicefile_type==SLICE_CELL_CENTER){
          float *xp;

          is2=is1-1;
          if(is2<0)is2=0;
          xp = meshi->xplt_orig;
          position = (xp[is1]+xp[is2])/2.0;
        }
        if(is1>0){
          sd->delta_orig=(meshi->xplt_orig[is1]-meshi->xplt_orig[is1-1])/2.0;
        }
        else{
          sd->delta_orig=(meshi->xplt_orig[is1+1]-meshi->xplt_orig[is1])/2.0;
        }
        if(sd->volslice==0){
          sd->dplane_min = meshi->dplane_min[1];
          sd->dplane_max = meshi->dplane_max[1];
          sprintf(sd->slicedir, "X=%f", position);
        }
        else{
          sd->dplane_min = meshi->dplane_min[0];
          sd->dplane_max = meshi->dplane_max[0];
          sprintf(sd->slicedir, "3D slice");
        }
      }
      if(sd->js1==sd->js2){
        sd->dplane_min = meshi->dplane_min[2];
        sd->dplane_max = meshi->dplane_max[2];

        sd->idir = 2;
        position = meshi->yplt_orig[js1];
        if(sd->slicefile_type==SLICE_CELL_CENTER){
          float *yp;

          js2=js1-1;
          if(js2<0)js2=0;
          yp = meshi->yplt_orig;
          position = (yp[js1]+yp[js2])/2.0;
        }
        if(js1>0){
          sd->delta_orig=(meshi->yplt_orig[js1]-meshi->yplt_orig[js1-1])/2.0;
        }
        else{
          sd->delta_orig=(meshi->yplt_orig[js1+1]-meshi->yplt_orig[js1])/2.0;
        }
        sprintf(sd->slicedir,"Y=%f",position);
      }
      if(sd->ks1==sd->ks2){
        sd->dplane_min = meshi->dplane_min[3];
        sd->dplane_max = meshi->dplane_max[3];

        sd->idir = 3;
        position = meshi->zplt_orig[ks1];
        if(sd->slicefile_type==SLICE_CELL_CENTER){
          float *zp;

          ks2=ks1-1;
          if(ks2<0)ks2=0;
          zp = meshi->zplt_orig;
          position = (zp[ks1]+zp[ks2])/2.0;
        }
        if(ks1>0){
          sd->delta_orig=(meshi->zplt_orig[ks1]-meshi->zplt_orig[ks1-1])/2.0;
        }
        else{
          sd->delta_orig=(meshi->zplt_orig[ks1+1]-meshi->zplt_orig[ks1])/2.0;
        }
        if(sd->slicefile_type==SLICE_TERRAIN){
          position=sd->above_ground_level;
          sprintf(sd->slicedir,"AGL=%f",position);
        }
        else{
          sprintf(sd->slicedir,"Z=%f",position);
        }
      }
      sd->position_orig=position;
      TrimZeros(sd->slicedir);
    }
    {
      float *xplt, *yplt, *zplt;
      float *xyz_min, *xyz_max;

      sd->mesh_type=meshi->mesh_type;
      xplt = meshi->xplt;
      yplt = meshi->yplt;
      zplt = meshi->zplt;
      sd->xmin = xplt[sd->is1];
      sd->xmax = xplt[sd->is2];
      sd->ymin = yplt[sd->js1];
      sd->ymax = yplt[sd->js2];
      sd->zmin = zplt[sd->ks1];
      sd->zmax = zplt[sd->ks2];
      xyz_min = sd->xyz_min;
      xyz_max = sd->xyz_max;
      if(sd->is_fed==0){
        if(sd->volslice==1){
          xyz_min[0] = xplt[sd->ijk_min[0]];
          xyz_max[0] = xplt[sd->ijk_max[0]];
          xyz_min[1] = yplt[sd->ijk_min[1]];
          xyz_max[1] = yplt[sd->ijk_max[1]];
          xyz_min[2] = zplt[sd->ijk_min[2]];
          xyz_max[2] = zplt[sd->ijk_max[2]];
        }
        else{
          xyz_min[0] = sd->xmin;
          xyz_max[0] = sd->xmax;
          xyz_min[1] = sd->ymin;
          xyz_max[1] = sd->ymax;
          xyz_min[2] = sd->zmin;
          xyz_max[2] = sd->zmax;
        }
      }
      else{
        float *xyz_min2, *xyz_max2;

        xyz_min2 = sd->fedptr->co2->xyz_min;
        xyz_max2 = sd->fedptr->co2->xyz_max;
        xyz_min[0] = xyz_min2[0];
        xyz_max[0] = xyz_max2[0];
        xyz_min[1] = xyz_min2[1];
        xyz_max[1] = xyz_max2[1];
        xyz_min[2] = xyz_min2[2];
        xyz_max[2] = xyz_max2[2];
      }
    }
  }
  if(stream!=NULL)fclose(stream);
  if(nsliceinfo>0){
    FREEMEMORY(sliceorderindex);
    NewMemory((void **)&sliceorderindex,sizeof(int)*nsliceinfo);
    for(i=0;i<nsliceinfo;i++){
      sliceorderindex[i]=i;
    }
    qsort( (int *)sliceorderindex, (size_t)nsliceinfo, sizeof(int), SliceCompare );

    for(i=0;i<nmultisliceinfo;i++){
      multislicedata *mslicei;

      mslicei = multisliceinfo + i;
      FREEMEMORY(mslicei->islices);
    }
    FREEMEMORY(multisliceinfo);
    nmultisliceinfo=0;

    NewMemory((void **)&multisliceinfo,sizeof(multislicedata)*nsliceinfo);

    {
      multislicedata *mslicei;
      slicedata *sd;

      nmultisliceinfo=1;
      mslicei = multisliceinfo;
      mslicei->islices=NULL;
      NewMemory((void **)&mslicei->islices,sizeof(int)*nsliceinfo);
      mslicei->nslices=1;
      sd = sliceinfo + sliceorderindex[0];
      mslicei->islices[0] = sliceorderindex[0];
      mslicei->mslicefile_labelindex=sd->slicefile_labelindex;//check  'type'
      for(i=1;i<nsliceinfo;i++){
        slicedata *sdold;

        sdold = sliceinfo + sliceorderindex[i - 1];
        sd = sliceinfo + sliceorderindex[i];
        mslicei->autoload=0;
        if(NewMultiSlice(sdold,sd)==1){
          nmultisliceinfo++;
          mslicei++;
          mslicei->nslices=0;
          mslicei->mslicefile_labelindex=sd->slicefile_labelindex;//check 'type'
          mslicei->mesh_type=sd->mesh_type;
          mslicei->islices=NULL;
          NewMemory((void **)&mslicei->islices,sizeof(int)*nsliceinfo);
        }
        mslicei->nslices++;
        mslicei->islices[mslicei->nslices-1]=sliceorderindex[i];
      }
    }
  }
  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo + i;
    slicei->mslice = NULL;
    slicei->skip = 0;
  }
  UpdateSliceDups();
  nslicedups = CountSliceDups();
  for(i = 0; i < nmultisliceinfo; i++){
    int ii;
    multislicedata *mslicei;

    mslicei = multisliceinfo + i;
    for(ii = 0; ii < mslicei->nslices; ii++){
      slicedata *slicei;

      slicei = sliceinfo + mslicei->islices[ii];
      ASSERT(slicei->mslice == NULL);
      slicei->mslice = mslicei;
    }
  }
  UpdateSliceMenuLabels();
  UpdateSliceDirCount();
}

/* ------------------ GetSliceParams2 ------------------------ */

void GetSliceParams2(void){
  int i;

  trainer_temp_n=0;
  trainer_oxy_n=0;
  if(nmultisliceinfo>0){
    FREEMEMORY(trainer_temp_indexes);
    FREEMEMORY(trainer_oxy_indexes);
    NewMemory((void **)&trainer_temp_indexes,nmultisliceinfo*sizeof(int));
    NewMemory((void **)&trainer_oxy_indexes,nmultisliceinfo*sizeof(int));
  }
  for(i=0;i<nmultisliceinfo;i++){
    multislicedata *mslicei;

    mslicei = multisliceinfo + i;
    if(mslicei->autoload==1){
      slicedata *slicei;
      char *longlabel;

      slicei = sliceinfo + mslicei->islices[0];
      longlabel = slicei->label.longlabel;

      if(STRCMP(longlabel,"TEMPERATURE")==0){
        trainer_temp_indexes[trainer_temp_n++]=i;
      }
      if(STRCMP(longlabel,"OXYGEN")==0||STRCMP(longlabel,"OXYGEN VOLUME FRACTION")==0){
        trainer_oxy_indexes[trainer_oxy_n++]=i;
      }
    }
  }
}

/* ------------------ UpdateVSlices ------------------------ */

void UpdateVSlices(void){
  int i;

#ifdef _DEBUG
  PRINTF("  updating vector slices\n");
#endif
  GetSliceParams();

  /* update vector slices */

  nvsliceinfo=0;
  for(i=0;i<nsliceinfo;i++){
    slicedata *sdi;

    sdi = sliceinfo+i;
    sdi->vec_comp=0;
    if(STRNCMP(sdi->label.shortlabel,"U-VEL",5)==0){
       sdi->vec_comp=1;
       continue;
    }
    if(STRNCMP(sdi->label.shortlabel,"V-VEL",5)==0){
      sdi->vec_comp=2;
      continue;
    }
    if(STRNCMP(sdi->label.shortlabel,"W-VEL",5)==0){
      sdi->vec_comp=3;
      continue;
    }
  }
  for(i=0;i<nsliceinfo;i++){
    slicedata *sdi;
    vslicedata *vd;
    int j;
#ifdef _DEBUG
    if(nsliceinfo>100&&(i%100==0||i==nsliceinfo-1)){
      PRINTF("    examining %i'st slice file for vectors\n",i+1);
    }
#endif
    vd = vsliceinfo + nvsliceinfo;
    sdi = sliceinfo+i;
    vd->iu=-1;
    vd->iv=-1;
    vd->iw=-1;
    vd->ival=i;
    vd->vslicefile_labelindex=sdi->slicefile_labelindex;
    vd->vslicefile_type=sdi->slicefile_type;
    if(vd->vslicefile_type==SLICE_CELL_CENTER){
      for(j=0;j<nsliceinfo;j++){
        slicedata *sdj;

        sdj = sliceinfo+j;
        if(sdj->slicefile_type!=SLICE_CELL_CENTER)continue;
        if(sdi->blocknumber!=sdj->blocknumber)continue;
        if(sdi->is1!=sdj->is1||sdi->is2!=sdj->is2||sdi->js1!=sdj->js1)continue;
        if(sdi->js2!=sdj->js2||sdi->ks1!=sdj->ks1||sdi->ks2!=sdj->ks2)continue;
        if(sdj->vec_comp==1)vd->iu=j;
        if(sdj->vec_comp==2)vd->iv=j;
        if(sdj->vec_comp==3)vd->iw=j;
      }
    }
    else if(vd->vslicefile_type == SLICE_GEOM){
      for(j=0;j<nsliceinfo;j++){
        slicedata *sdj;

        sdj = sliceinfo+j;
        if(sdj->slicefile_type!=SLICE_GEOM)continue;
        if(sdi->blocknumber!=sdj->blocknumber)continue;
        if(sdi->is1!=sdj->is1||sdi->is2!=sdj->is2||sdi->js1!=sdj->js1)continue;
        if(sdi->js2!=sdj->js2||sdi->ks1!=sdj->ks1||sdi->ks2!=sdj->ks2)continue;
        if(sdj->vec_comp==1)vd->iu=j;
        if(sdj->vec_comp==2)vd->iv=j;
        if(sdj->vec_comp==3)vd->iw=j;
      }
    }
    else{
      for (j = 0; j < nsliceinfo; j++) {
        slicedata *sdj;

        sdj = sliceinfo + j;
        if (sdj->slicefile_type == SLICE_CELL_CENTER|| sdj->slicefile_type == SLICE_GEOM)continue;
        if (sdi->blocknumber != sdj->blocknumber)continue;
        if (sdi->is1 != sdj->is1 || sdi->is2 != sdj->is2 || sdi->js1 != sdj->js1)continue;
        if (sdi->js2 != sdj->js2 || sdi->ks1 != sdj->ks1 || sdi->ks2 != sdj->ks2)continue;
        if (sdj->vec_comp == 1)vd->iu = j;
        if (sdj->vec_comp == 2)vd->iv = j;
        if (sdj->vec_comp == 3)vd->iw = j;
      }
    }
    if(vd->iu!=-1||vd->iv!=-1||vd->iw!=-1){
      vd->finalize = 1;
      vd->display=0;
      vd->loaded=0;
      vd->volslice=sdi->volslice;
      nvsliceinfo++;
    }
  }
#ifdef _DEBUG
  PRINTF("    %i vector slices found\n",nvsliceinfo);
#endif
  if(nvsliceinfo>0){
    vslicedata *vsd;
    multivslicedata *mvslicei;

    FREEMEMORY(vsliceorderindex);
    NewMemory((void **)&vsliceorderindex,sizeof(int)*nvsliceinfo);
    for(i=0;i<nvsliceinfo;i++){
      vsliceorderindex[i]=i;
    }
    qsort( (int *)vsliceorderindex, (size_t)nvsliceinfo, sizeof(int), VSliceCompare );

    for(i=0;i<nmultivsliceinfo;i++){
      mvslicei = multivsliceinfo + i;
      FREEMEMORY(mvslicei->ivslices);
    }
    FREEMEMORY(multivsliceinfo);
    nmultivsliceinfo=0;

    NewMemory((void **)&multivsliceinfo,sizeof(multislicedata)*nvsliceinfo);

    nmultivsliceinfo=1;
    mvslicei = multivsliceinfo;
    mvslicei->ivslices=NULL;
    NewMemory((void **)&mvslicei->ivslices,sizeof(int)*nvsliceinfo);
    mvslicei->nvslices=1;
    vsd = vsliceinfo + vsliceorderindex[0];
    mvslicei->ivslices[0] = vsliceorderindex[0];
    mvslicei->mvslicefile_labelindex=sliceinfo[vsd->ival].slicefile_labelindex;
    for(i=1;i<nvsliceinfo;i++){
      slicedata *sd, *sdold;
      vslicedata *vsdold;

      vsdold = vsliceinfo + vsliceorderindex[i - 1];
      sdold = sliceinfo + vsdold->ival;
      vsd = vsliceinfo + vsliceorderindex[i];
      sd = sliceinfo + vsd->ival;
      if(NewMultiSlice(sdold,sd)==1){
        nmultivsliceinfo++;
        mvslicei++;
        mvslicei->nvslices=0;
        mvslicei-> mvslicefile_labelindex=sd->slicefile_labelindex;
        mvslicei->ivslices=NULL;
        NewMemory((void **)&mvslicei->ivslices,sizeof(int)*nvsliceinfo);
      }
      mvslicei->nvslices++;
      mvslicei->ivslices[mvslicei->nvslices-1]=vsliceorderindex[i];
    }

    // define sequence id's for auto file loading

    for(i=0;i<nvsliceinfo;i++){
      vslicedata *vslicei;
      slicedata *sliceval;
      int seq_id;

      vslicei = vsliceinfo + i;
      sliceval = sliceinfo + vslicei->ival;
      seq_id=-1;
      if(vslicei->ival>=0)seq_id = sliceval->seq_id;
      vslicei->seq_id=seq_id;
      vslicei->autoload=0;
      vslicei->skip = 0;
    }
  }

  UpdateVSliceDups();

  for(i = 0; i<nmultivsliceinfo; i++){
    multivslicedata *mvslicei;

    mvslicei = multivsliceinfo + i;
    mvslicei->ndirxyz[0]=0;
    mvslicei->ndirxyz[1]=0;
    mvslicei->ndirxyz[2]=0;
    mvslicei->ndirxyz[3]=0;
  }
  for(i=0;i<nmultivsliceinfo;i++){
    multivslicedata *mvslicei;
    slicedata *slicei;
    int j;

    mvslicei = multivsliceinfo + i;
    slicei = sliceinfo + mvslicei->ivslices[0];
    if(slicei->idir<1)continue;
    if(slicei->volslice==1)continue;
    for(j=0;j<nmultivsliceinfo;j++){
      multivslicedata *mvslicej;
      slicedata *slicej;

      mvslicej = multivsliceinfo + j;
      slicej = sliceinfo + mvslicej->ivslices[0];
      if(slicej->idir<1)continue;
      if(slicej->volslice==1)continue;
      if(strcmp(slicej->label.longlabel,slicei->label.longlabel)!=0)continue;
      if((slicej->slicefile_type==SLICE_CELL_CENTER&&slicei->slicefile_type!=SLICE_CELL_CENTER)||
         (slicej->slicefile_type!=SLICE_CELL_CENTER&&slicei->slicefile_type==SLICE_CELL_CENTER))continue;
      mvslicei->ndirxyz[slicej->idir]++;
    }
  }

  UpdateVsliceMenuLabels();
}

/* ------------------ UpdateVSliceBoundIndexes ------------------------ */

void UpdateVSliceBoundIndexes(void){
  int i;

  for(i=0;i<nvsliceinfo;i++){
    vslicedata *vd;
    slicedata *val;

    vd = vsliceinfo+i;
    val = sliceinfo + vd->ival;
    vd->vslicefile_labelindex= GetSliceBoundsIndex(val);
  }
}

/* ------------------ UpdateSliceContours ------------------------ */

void UpdateSliceContours(int slice_type_index, float line_min, float line_max, int nline_values){
  int j;
  boundsdata *sb;
  int contours_gen=0;
  float dval;

  dval=0.0;
  if(nline_values>1){
    dval=(line_max-line_min)/(float)(nline_values-1);
  }

  sb = slicebounds + slice_type_index;
  for(j=0;j<nsliceinfo;j++){
    slicedata *sd;
    meshdata *meshi;
    int nx, ny, nz;
    int ibar, jbar, kbar;
    float *xplt, *yplt, *zplt;
    int slice_type_j;
    float constval;
    int i;

    sd = sliceinfo + j;
    if(sd->loaded==0)continue;

    slice_type_j = GetSliceBoundsIndex(sd);
    if(slice_type_j!=slice_type_index)continue;
    if(sd->qslicedata==NULL){
      fprintf(stderr,"*** Error: data not available from %s to generate contours\n",sd->reg_file);
      continue;
    }
    PRINTF("generating contours for %s\n",sd->file);
    contours_gen=1;

    for(i=0;i<nline_values;i++){
      int val_index;
      float val;
      float valmin, valmax;

      valmin = sb->levels256[0];
      valmax = sb->levels256[255];

      val=line_min + i*dval;
      val_index=255;
      if(val<valmin){
        val_index=0;
      }
      else if(valmax>valmin&&val>=valmin&&val<=valmax){
        val_index=255*(val-valmin)/(valmax-valmin);
      }
      else if(val>valmax){
        val_index=255;
      }
      if(val_index<0)val_index=0;
      if(val_index>255)val_index=255;
      sd->rgb_slice_ptr[i]=&rgb_full[val_index][0];
    }
    meshi = meshinfo + sd->blocknumber;

    xplt=meshi->xplt;
    yplt=meshi->yplt;
    zplt=meshi->zplt;
    ibar=meshi->ibar;
    jbar=meshi->jbar;
    kbar=meshi->kbar;
    nx = ibar + 1;
    ny = jbar + 1;
    nz = kbar + 1;

    switch(sd->idir){
      case XDIR:
      constval = xplt[sd->is1]+offset_slice*sd->sliceoffset;
      break;
      case YDIR:
      constval = yplt[sd->js1]+offset_slice*sd->sliceoffset;
      break;
      case ZDIR:
      constval = zplt[sd->ks1]+offset_slice*sd->sliceoffset;
      break;
      default:
        constval = 0.0;
        ASSERT(FFALSE);
        break;
    }
    constval += SCALE2SMV(sd->sliceoffset_fds);

    FreeContours(sd->line_contours,sd->nline_contours);
    sd->nline_contours=sd->ntimes;
    if(slice_contour_type==SLICE_LINE_CONTOUR){
      InitLineContoursI(&sd->line_contours,sd->rgb_slice_ptr,sd->nline_contours,constval,sd->idir,line_min,line_max,nline_values);
    }
    else{
      InitContours(&sd->line_contours,sd->rgb_slice_ptr,sd->nline_contours,constval,sd->idir,line_min,line_max,nline_values-1);
    }
    for(i=0;i<sd->nline_contours;i++){
      float *vals;
      contour *ci;

      vals = sd->qslicedata + i*sd->nsliceijk;
      ci = sd->line_contours+i;
      if(slice_contour_type==SLICE_LINE_CONTOUR){
        PRINTF("updating line contour: %i of %i\n",i+1,sd->nline_contours);
        switch(sd->idir){
          case XDIR:
            GetLineContours(yplt,zplt,ny,nz,vals,NULL,line_min, line_max,ci);
            break;
          case YDIR:
            GetLineContours(xplt,zplt,nx,nz,vals,NULL,line_min,line_max,ci);
          break;
          case ZDIR:
            GetLineContours(xplt,yplt,nx,ny,vals,NULL,line_min,line_max,ci);
          break;
          default:
            ASSERT(FFALSE);
            break;
        }
      }
      else{
        PRINTF("updating stepped contour: %i of %i\n",i+1,sd->nline_contours);
        switch(sd->idir){
          case XDIR:
            GetContours(yplt,zplt,jbar+1,kbar+1,vals,NULL,ci->levels,DONT_GET_AREAS,DATA_FORTRAN,ci);
            break;
          case YDIR:
            GetContours(xplt,zplt,ibar+1,kbar+1,vals,NULL,ci->levels,DONT_GET_AREAS,DATA_FORTRAN,ci);
            break;
          case ZDIR:
            GetContours(xplt,yplt,ibar+1,jbar+1,vals,NULL,ci->levels,DONT_GET_AREAS,DATA_FORTRAN,ci);
            break;
          default:
            ASSERT(FFALSE);
            break;
        }
      }
    }
  }
  if(contours_gen==0){
    fprintf(stderr,"*** Error: no slice files of type %s are currently loaded\n",sb->shortlabel);
  }
}

/* ------------------ UpdateSliceBoundIndexes ------------------------ */

void UpdateSliceBoundIndexes(void){
  int i;

  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;

    sd = sliceinfo+i;
    sd->slicefile_labelindex= GetSliceBoundsIndex(sd);
  }
}

/* ------------------ GetSliceBoundsIndex ------------------------ */

int GetSliceBoundsIndex(const slicedata *sd){
  int i;

  for(i=0;i<nslicebounds;i++){
    boundsdata *boundi;

    boundi = slicebounds + i;
    if(strcmp(sd->label.shortlabel,boundi->shortlabel)==0)return i;
  }
  return -1;
}

/* ------------------ GetSliceBoundsIndexFromLabel ------------------------ */

int GetSliceBoundsIndexFromLabel(char *label){
  int i;

  for(i = 0;i < nslicebounds;i++){
    boundsdata *boundi;

    boundi = slicebounds + i;
    if(strcmp(label, boundi->shortlabel) == 0)return i;
  }
  return -1;
}


/* ------------------ UpdateSliceBoundLabels ------------------------ */

void UpdateSliceBoundLabels(){
  int i;

  for(i=0;i<nsliceinfo;i++){
    int j;
    boundsdata *sb;
    slicedata *sd;

    sd = sliceinfo + i;
    j = GetSliceBoundsIndex(sd);
    sb = slicebounds + j;
    sb->label=&(sd->label);
  }
}

/* ------------------ SetSliceBounds ------------------------ */

void SetSliceBounds(int slicetype){
  if(slicetype>=0&&slicetype<nslicebounds){
    slice_line_contour_min=slicebounds[slicetype].line_contour_min;
    slice_line_contour_max=slicebounds[slicetype].line_contour_max;
    slice_line_contour_num=slicebounds[slicetype].line_contour_num;
    slicemin=slicebounds[slicetype].valmin;
    slicemax=slicebounds[slicetype].valmax;
    setslicemin=slicebounds[slicetype].setvalmin;
    setslicemax=slicebounds[slicetype].setvalmax;
    slicechopmin=slicebounds[slicetype].chopmin;
    slicechopmax=slicebounds[slicetype].chopmax;
    setslicechopmin=slicebounds[slicetype].setchopmin;
    setslicechopmax=slicebounds[slicetype].setchopmax;
    slicemin_unit = (unsigned char *)slicebounds[slicetype].label->unit;
    slicemax_unit = slicemin_unit;

    memcpy(&glui_slicebounds, slicebounds + slicetype, sizeof(bounddata));
    UpdateGluiSliceUnits();
  }
}

/* ------------------ GetSliceDataBounds ------------------------ */

void GetSliceDataBounds(slicedata *sd, float *pmin, float *pmax){
  float *pdata;
  int ndata;
  int n, i, j, k;
  int first=1;

  int istep;
  int nx, ny, nxy, ibar, jbar;
  int ntimes;
  //int iimin, iimax, jjmin, jjmax, kkmin, kkmax;
  char *iblank_node, *iblank_cell, *slice_mask0;
  meshdata *meshi;


  if(sd->slicefile_type == SLICE_GEOM){
    pdata = sd->patchgeom->geom_vals;
    ndata = sd->patchgeom->geom_nvals;
    *pmin = pdata[0];
    *pmax = pdata[0];
    for (i = 0; i < ndata; i++) {
      *pmin = MIN(*pmin, pdata[i]);
      *pmax = MAX(*pmax, pdata[i]);
    }
    return;
  }
  meshi = meshinfo + sd->blocknumber;
  iblank_node = meshi->c_iblank_node;
  iblank_cell = meshi->c_iblank_cell;

  ibar = meshi->ibar;
  jbar = meshi->jbar;
  nx = ibar+1;
  ny = jbar+1;
  nxy = nx*ny;

  pdata = sd->qslicedata;
  ndata = sd->nslicetotal;

  NewMemory((void **)&slice_mask0,sd->nslicei*sd->nslicej*sd->nslicek);
  n=-1;
  for(k=0;k<sd->nslicek;k++){
    for(j=0;j<sd->nslicej;j++){
      char  *ib_node, *ib_cell;

      ib_node = iblank_node +  IJKNODE(sd->is1,   sd->js1+j,   sd->ks1+k);
      ib_cell = iblank_cell +  IJKCELL(sd->is1-1, sd->js1+j-1, sd->ks1+k-1);
      for(i=0;i<sd->nslicei;i++){
        n++;
        slice_mask0[n]=0;
        if(sd->slicefile_type==SLICE_CELL_CENTER&&((k==0&&sd->nslicek!=1)||(j==0&&sd->nslicej!=1)||(i==0&&sd->nslicei!=1)))continue;
        if(show_slice_in_obst == ONLY_IN_GAS){
          if(sd->slicefile_type!=SLICE_CELL_CENTER&& iblank_node!=NULL){
//            if(iblank_node[IJKNODE(sd->is1+i, sd->js1+j, sd->ks1+k)]==SOLID)continue;
            if(ib_node[i]==SOLID)continue;
          }
          if(sd->slicefile_type==SLICE_CELL_CENTER&& iblank_cell!=NULL){
//            if(iblank_cell[IJKCELL(sd->is1+i-1, sd->js1+j-1, sd->ks1+k-1)]==EMBED_YES)continue;
            if(ib_cell[i]==EMBED_YES)continue;
          }
        }
        slice_mask0[n]=1;
      }
    }
  }

  n=-1;
  ntimes = ndata/sd->nsliceijk;

  for(istep=0;istep<ntimes;istep++){
    int n0;

    n0 = -1;

    for(k=0;k<sd->nslicek;k++){
      for(j=0;j<sd->nslicej;j++){
        for(i=0;i<sd->nslicei;i++){
          n++;
          n0++;
          // 0 blocked
          // 1 partially blocked
          // 2 unblocked
          if(slice_mask0[n0]==0)continue;
          if(first==1){
            *pmin=pdata[n];
            *pmax=pdata[n];
            first=0;
          }
          else{
            *pmin=MIN(*pmin,pdata[n]);
            *pmax=MAX(*pmax,pdata[n]);
          }
        }
      }
    }
    if(first==1){
      *pmin=0.0;
      *pmax=1.0;
    }
  }
  FREEMEMORY(slice_mask0);
}

/* ------------------ AdjustSliceBounds ------------------------ */

void AdjustSliceBounds(const slicedata *sd, float *pmin, float *pmax){

    int nsmall, nbig, *buckets=NULL, n, level, total, alpha05;
    float dp;
    float *pdata;
    int ndata;
    float ppmin;

    if(sd->slicefile_type == SLICE_GEOM){
      pdata = sd->patchgeom->geom_vals;
      ndata = sd->patchgeom->geom_nvals;
    }
    else {
      pdata = sd->qslicedata;
      ndata = sd->nslicetotal;
    }

#define EPS_BUCKET 0.0001
    if(setslicemin==PERCENTILE_MIN||setslicemax==PERCENTILE_MAX){
      float abs_diff, denom;

      abs_diff = ABS(*pmax-*pmin);
      denom = MAX(ABS(*pmax), ABS(*pmin));
      if(abs_diff<EPS_BUCKET||abs_diff<EPS_BUCKET*denom)abs_diff = 0.0;
      dp = abs_diff/NBUCKETS;
      nsmall=0;
      nbig=NBUCKETS;
      if(NewMemory((void **)&buckets,NBUCKETS*sizeof(int))==0){
        fprintf(stderr,"*** Error: Unable to allocate memory in getdatabounds\n");
        return;
      }

      for(n=0;n<NBUCKETS;n++){
        buckets[n]=0;
      }
      for(n=0;n<ndata;n++){
        level=0;
        if(dp!=0.0f){
          level = (int)((pdata[n] - *pmin)/dp);
        }
        if(level<0){
          level=0;
        }
        if(level>NBUCKETS-1){
          level=NBUCKETS-1;
        }
        buckets[level]++;
      }
      alpha05 = (int)(.01f*ndata);
      total = 0;
      for(n=0;n<NBUCKETS;n++){
        total += buckets[n];
        if(total>alpha05){
          nsmall=n;
          break;
        }
      }
      total = 0;
      for(n=NBUCKETS;n>0;n--){
        total += buckets[n-1];
        if(total>alpha05){
          nbig=n;
          break;
        }
      }
      FreeMemory(buckets);
      ppmin = *pmin;
      if(setslicemin==PERCENTILE_MIN)*pmin = ppmin + nsmall*dp;
      if(setslicemax==PERCENTILE_MAX)*pmax = ppmin + (nbig+1)*dp;

    }
    if(axislabels_smooth==1){
      SmoothLabel(pmin,pmax,nrgb);
    }

}

/* ------------------ AverageSliceData ------------------------ */

int AverageSliceData(float *data_out, float *data_in, int ndata, int data_per_timestep, float *times_local, int ntimes_local, float average_time){

#define IND(itime,ival) ((itime)*data_per_timestep + (ival))
  float *datatemp = NULL;
  int below, above, naverage;
  float average_timed2;
  int i, j, k;

  if(data_in == NULL || data_out == NULL)return 1;
  if(ndata < data_per_timestep || data_per_timestep < 1 || ntimes_local < 1 || average_time < 0.0)return 1;
  if(ndata != data_per_timestep*ntimes_local)return 1;

  average_timed2 = average_time / 2.0;

  NewMemory((void **)&datatemp, ndata * sizeof(float));
  for(i = 0; i < ndata; i++){
    datatemp[i] = 0.0;
  }
  for(i = 0; i < ntimes_local; i++){
    PRINTF("averaging time=%.2f\n", times_local[i]);
    below = 0;
    for(j = i - 1; j >= 0; j--){
      if(times_local[i] - times_local[j] > average_timed2){
        below = j + 1;
        break;
      }
    }
    above = ntimes_local - 1;
    for(j = i + 1; j < ntimes_local; j++){
      if(times_local[j] - times_local[i] > average_timed2){
        above = j - 1;
        break;
      }
    }
    naverage = above + 1 - below;
    for(k = 0; k < data_per_timestep; k++){
      for(j = below; j <= above; j++){
        datatemp[IND(i, k)] += data_in[IND(j, k)];
      }
    }
    for(k = 0; k < data_per_timestep; k++){
      datatemp[IND(i, k)] /= (float)naverage;
    }
  }
  for(i = 0; i < ndata; i++){
    data_out[i] = datatemp[i];
  }
  FREEMEMORY(datatemp);
  return 0;
}

//*** header
// endian
// completion (0/1)
// fileversion (compressed format)
// version  (slicef version)
// global min max (used to perform conversion)
// i1,i2,j1,j2,k1,k2


//*** frame
// time, compressed frame size                        for each frame
// compressed buffer

/* ------------------ GetSlicecZlibData ------------------------ */

int GetSlicecZlibData(char *file,
  int set_tmin, int set_tmax, float tmin_local, float tmax_local, int ncompressed, int sliceskip, int nsliceframes,
  float *times_local, unsigned char *compressed_data, compdata *compindex, float *valmin, float *valmax){
  FILE *stream;
  int count, ns;
  unsigned char *cd;
  int endian;
  float minmax[2];
  int fileversion, version;
  int completion;
  int ijkbar[6];

  cd = compressed_data;
  compindex[0].offset = 0;

  stream = FOPEN(file, "rb");
  if(stream == NULL)return 0;

  // read header

  fread(&endian, 4, 1, stream);
  fread(&completion, 4, 1, stream);
  if(completion == 0){
    fclose(stream);
    return 0;
  }

  fread(&fileversion, 4, 1, stream);
  if(endian != 1)fileversion = IntSwitch(fileversion);

  fread(&version, 4, 1, stream);
  if(endian != 1)version = IntSwitch(version);

  fread(minmax, 4, 2, stream);
  if(endian != 1){
    minmax[0] = FloatSwitch(minmax[0]);
    minmax[1] = FloatSwitch(minmax[1]);
  }

  fread(ijkbar, 4, 6, stream);
  if(endian != 1){
    ijkbar[0] = IntSwitch(ijkbar[0]);
    ijkbar[1] = IntSwitch(ijkbar[1]);
    ijkbar[2] = IntSwitch(ijkbar[2]);
    ijkbar[3] = IntSwitch(ijkbar[3]);
    ijkbar[4] = IntSwitch(ijkbar[4]);
    ijkbar[5] = IntSwitch(ijkbar[5]);
  }

  count = 0;
  ns = 0;
  while(!feof(stream)){
    float ttime;
    int nncomp;

    fread(&ttime, 4, 1, stream);
    fread(&nncomp, 4, 1, stream);
    if((count++%sliceskip != 0) || (set_tmin == 1 && ttime<tmin_local) || (set_tmax == 1 && ttime>tmax_local)){
      FSEEK(stream, nncomp, SEEK_CUR);
      continue;
    }
    times_local[ns++] = ttime;
    compindex[ns].offset = compindex[ns - 1].offset + nncomp;
    compindex[ns - 1].size = nncomp;

    fread(cd, 1, nncomp, stream);
    cd += nncomp;
    if(ns >= nsliceframes || cd - compressed_data >= ncompressed)break;
  }
  fclose(stream);
  return cd - compressed_data;
}

/* ------------------ GetSliceCompressedData ------------------------ */

int GetSliceCompressedData(char *file,
  int set_tmin, int set_tmax, float tmin_local, float tmax_local, int ncompressed, int sliceskip, int nsliceframes,
  float *times_local, unsigned char *compressed_data, compdata *compindex, float *valmin, float *valmax){
  int returnval;

  returnval = GetSlicecZlibData(file, set_tmin, set_tmax, tmin_local, tmax_local, ncompressed, sliceskip, nsliceframes,
    times_local, compressed_data, compindex, valmin, valmax);
  return returnval;
}

#ifdef pp_GPU

/* ------------------ InitSlice3DTexture ------------------------ */

void InitSlice3DTexture(meshdata *meshi){
  GLint border_size = 0;
  GLsizei nx, ny, nz;

  PRINTF("Defining 3d slice textures for %s ...", meshi->label);
  FFLUSH();

  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &meshi->slice3d_texture_id);
  glBindTexture(GL_TEXTURE_3D, meshi->slice3d_texture_id);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  nx = meshi->ibar + 1;
  ny = meshi->jbar + 1;
  nz = meshi->kbar + 1;
  if(meshi->slice3d_texture_buffer == NULL){
    int i;

    NewMemory((void **)&meshi->slice3d_texture_buffer, nx*ny*nz * sizeof(float));
    for(i = 0; i < nx*ny*nz; i++){
      meshi->slice3d_texture_buffer[i] = 0.0;
    }
  }
  if(meshi->slice3d_c_buffer == NULL){
    NewMemory((void **)&meshi->slice3d_c_buffer, nx*ny*nz * sizeof(float));
  }
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, nx, ny, nz, border_size, GL_RED, GL_FLOAT, meshi->slice3d_texture_buffer);


  if(slice3d_colormap_id_defined == -1){
    slice3d_colormap_id_defined = 1;
    glActiveTexture(GL_TEXTURE4);
    glGenTextures(1, &slice3d_colormap_id);
    glBindTexture(GL_TEXTURE_1D, slice3d_colormap_id);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, rgb_slice);
  }

  glActiveTexture(GL_TEXTURE0);
  PRINTF("completed");
  PRINTF("\n");
  FFLUSH();
}
#endif

/* ------------------ GetSliceFileDirection ------------------------ */

void GetSliceFileDirection(int is1, int *is2ptr, int *iis1ptr, int *iis2ptr, int js1, int *js2ptr, int ks1, int *ks2ptr, int *idirptr, int *joffptr, int *koffptr, int *volsliceptr){
  int nxsp, nysp, nzsp;
  int imin;

  nxsp = *is2ptr+1-is1;
  nysp = *js2ptr+1-js1;
  nzsp = *ks2ptr+1-ks1;
  *joffptr = 0;
  *koffptr = 0;
  *volsliceptr = 0;
  *iis1ptr = is1;
  *iis2ptr = *is2ptr;
  if(is1!=*is2ptr&&js1!=*js2ptr&&ks1!=*ks2ptr){
    *idirptr = 1;
    *is2ptr = is1;
    *volsliceptr = 1;
    return;
  }
  imin = MIN(nxsp, nysp);
  imin = MIN(imin, nzsp);
  if(nxsp==imin){
    *idirptr = 1;
    *is2ptr = is1;
  }
  else if(nysp==imin){
    *idirptr = 2;
    *js2ptr = js1;
  }
  else{
    *idirptr = 3;
    *ks2ptr = ks1;
  }
  if(is1==*is2ptr&&js1==*js2ptr){
    *idirptr = 1;
    *joffptr = 1;
  }
  else if(is1==*is2ptr&&ks1==*ks2ptr){
    *idirptr = 1;
    *koffptr = 1;
  }
  else if(js1==*js2ptr&&ks1==*ks2ptr){
    *idirptr = 2;
    *koffptr = 1;
  }
}

/* ------------------ GetSliceSizes ------------------------ */

#ifdef pp_CSLICE
void GetSliceSizes(char *slicefilenameptr, int *nsliceiptr, int *nslicejptr, int *nslicekptr, int *ntimesptr, int sliceframestep_arg,
  int *errorptr, int settmin_s_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg, int *headersizeptr, int *framesizeptr){

  int ip1, ip2, jp1, jp2, kp1, kp2;
  int iip1, iip2;
  int nxsp, nysp, nzsp;

  float timeval, time_max;
  int idir, joff, koff, volslice;
  int count;
  FILE *SLICEFILE;
  int ijk[6];
  int loadframe;
  int returncode;

  *errorptr = 0;
  *ntimesptr = 0;

  SLICEFILE = fopen(slicefilenameptr, "rb");
  if(SLICEFILE==NULL){
    *errorptr = 1;
    return;
  }

  *headersizeptr = 3*(4+30+4);
  fseek(SLICEFILE, *headersizeptr, SEEK_CUR);

  FORTREAD(ijk, 6, SLICEFILE);
  ip1 = ijk[0];
  ip2 = ijk[1];
  jp1 = ijk[2];
  jp2 = ijk[3];
  kp1 = ijk[4];
  kp2 = ijk[5];
  *headersizeptr += 4+6*4+4;

  nxsp = ip2 + 1 - ip1;
  nysp = jp2 + 1 - jp1;
  nzsp = kp2 + 1 - kp1;

  GetSliceFileDirection(ip1, &ip2, &iip1, &iip2, jp1, &jp2, kp1, &kp2, &idir, &joff, &koff, &volslice);
  *nsliceiptr = nxsp;
  *nslicejptr = nysp + joff;
  *nslicekptr = nzsp + koff;

  *framesizeptr = 4*(1+nxsp*nysp*nzsp)+16;

  count = -1;
  time_max = -1000000.0;
  for(;;){
    FORTREAD(&timeval, 1, SLICEFILE);
    if(returncode==0)break;
    if((settmin_s_arg!=0&&timeval<tmin_s_arg)||timeval<=time_max){
      loadframe = 0;
    }
    else{
      loadframe = 1;
      time_max = timeval;
    }
    if(settmax_s_arg!=0&&timeval>tmax_s_arg){
      fclose(SLICEFILE);
      return;
    }
    fseek(SLICEFILE, *framesizeptr-12, SEEK_CUR);
    count = count+1;
    if(count%sliceframestep_arg!=0)loadframe = 0;
    // if(error.ne.0)exit
    if(loadframe==1)*ntimesptr = *ntimesptr+1;
  }
  *errorptr = 0;
  fclose(SLICEFILE);
}

/* ------------------ GetSliceData ------------------------ */

FILE_SIZE GetSliceData(char *slicefilename, int *is1ptr, int *is2ptr, int *js1ptr, int *js2ptr, int *ks1ptr, int *ks2ptr, int *idirptr, 
  float *qminptr, float *qmaxptr, float *qdataptr, float *timesptr, int ntimes_old_arg, int *ntimesptr,
  int sliceframestep_arg, int settmin_s_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg){

  int i, j, k;
  int nsteps;
  int ip1, ip2, jp1, jp2, kp1, kp2;
  int nxsp, nysp, nzsp;
  int istart, irowstart;
  float timeval, time_max;
  int loadframe;
  int ii, kk;
  int joff, koff, volslice;
  int count;
  int iis1, iis2;
  int ijk[6];
  int file_size;
  FILE *stream;
  int returncode;
  float *qq;
  int nx, ny, nxy;

  joff = 0;
  koff = 0;
  file_size = 0;

  stream = fopen(slicefilename, "rb");
  if(stream==NULL){
    printf(" the slice file %s does not exist\n", slicefilename);
    nsteps = 0;
    return 0;
  }

  nsteps = 0;
  fseek(stream, 3*(4+30+4), SEEK_CUR);

  FORTREAD(ijk, 6, stream);
  if(returncode==0){
    fclose(stream);
    return file_size;
  }
  ip1 = ijk[0];
  ip2 = ijk[1];
  jp1 = ijk[2];
  jp2 = ijk[3];
  kp1 = ijk[4];
  kp2 = ijk[5];
  file_size = 6*4;
  *is1ptr = ip1;
  *is2ptr = ip2;
  *js1ptr = jp1;
  *js2ptr = jp2;
  *ks1ptr = kp1;
  *ks2ptr = kp2;

  nxsp = *is2ptr+1-*is1ptr;
  nysp = *js2ptr+1-*js1ptr;
  nzsp = *ks2ptr+1-*ks1ptr;
  
  nx = nxsp;
  ny = nysp;
  nxy = nx*ny;

  GetSliceFileDirection(*is1ptr, is2ptr, &iis1, &iis2, *js1ptr, js2ptr, *ks1ptr, ks2ptr, idirptr, &joff, &koff, &volslice);

  NewMemory((void **)&qq, nxsp*(nysp+joff)*(nzsp+koff)*sizeof(float));

  count = -1;
  time_max = -1000000.0;
  if(*ntimesptr!=ntimes_old_arg&&ntimes_old_arg>0){
    int size;

    size = 0;
    for(i = 0; i<ntimes_old_arg; i++){
      size += 4+4+4;
      size += 4+4*nxsp*nysp*nzsp+4;
    }
    fseek(stream, size, SEEK_CUR);
    nsteps = ntimes_old_arg;
  }
  for(;;){
    FORTREAD(&timeval, 1, stream);
    if(returncode==0)break;
    file_size = file_size+4;
    if((settmin_s_arg!=0&&timeval<tmin_s_arg)||timeval<=time_max){
      loadframe = 0;
    }
    else{
      loadframe = 1;
      time_max = timeval;
    }
    if(settmax_s_arg!=0&&timeval>tmax_s_arg)break;
    //    read(lu11, iostat = error)(((qq(i, j, k), i = 1, nxsp), j = 1, nysp), k = 1, nzsp)
    FORTREAD(qq, nxsp*nysp*nzsp, stream);
    if(returncode==0||nsteps>=*ntimesptr)break;
    count++;
    if(count%sliceframestep_arg!=0)loadframe = 0;
    if(loadframe==0)continue;
    if(koff==1){
//  qq(1:nxsp, 1:nysp, 2) = qq(1:nxsp, 1:nysp, 1)
      for(j = 0;j<nysp;j++){
        float *qqto, *qqfrom;

        qqfrom = qq + IJKNODE(0,j,0);
        qqto = qq + IJKNODE(0,j,1);
        for(i = 0;i<nxsp;i++){
//        qq[IJKNODE(i, j, 1)] = qq[IJKNODE(i, j, 0)];
          *qqto++ = *qqfrom++;
        }
      }
    }
    else if(joff==1){
//  qq(1:nxsp, 2, 1:nzsp) = qq(1:nxsp, 1, 1:nzsp)
      for(k = 0;k<nzsp;k++){
        float *qqto, *qqfrom;

        qqfrom = qq + IJKNODE(0,0,k);
        qqto = qq + IJKNODE(0,1,k);
        for(i = 0;i<nxsp;i++){
//        qq[IJKNODE(i, 1, k)] = qq[IJKNODE(i, 0, k)];
          *qqto++ = *qqfrom++;
        }
      }
    }
    timesptr[nsteps] = timeval;
    nsteps = nsteps+1;
    file_size += 4*nxsp*nysp*nzsp;

    if(*idirptr==3){
      float *qqto, *qqfrom;

      istart = (nsteps-1)*nxsp*nysp;
      for(i = 0; i<nxsp; i++){
        irowstart = i*nysp;
        ii = istart+irowstart;
        qqto = qdataptr+ii;
        qqfrom = qq + IJKNODE(i, 0, 0);
//      qdata(ii+1:ii+nysp) = qq(i, 1:nysp, 1)
        for(j = 0;j<nysp;j++){
//        qdataptr[ii+j] = qq[IJKNODE(i, j, 0)];
          *qqto++ = *qqfrom;
          qqfrom += nx;
        }
      }
    }
    else if(*idirptr==2){
      float *qqto, *qqfrom;

      istart = (nsteps-1)*nxsp*(nzsp+koff);
      for(i = 0; i<nxsp; i++){
        irowstart = i*(nzsp+koff);
        kk = istart+irowstart;
//      qdata(kk+1:kk+nzsp+koff) = qq(i, 1, 1:nzsp+koff)
        qqto = qdataptr+kk;
        qqfrom = qq + IJKNODE(i, 0, 0);
        for(k = 0;k<nzsp+koff;k++){
//        qdataptr[kk+k] = qq[IJKNODE(i, 0, k)];
          *qqto++ = *qqfrom;
          qqfrom += nxy;
        }
      }
    }
    else{
      float *qqto, *qqfrom;

      istart = (nsteps-1)*(nysp+joff)*(nzsp+koff)*nxsp;
      for(i = 0; i<nxsp; i++){
        for(j = 0; j<nysp+joff; j++){
          irowstart = i*nysp*(nzsp+koff)+j*(nzsp+koff);
          kk = istart+irowstart;
//        qdata(kk+1:kk+nzsp+koff) = qq(i, j, 1:nzsp+koff)
          qqto = qdataptr+kk;
          qqfrom = qq + IJKNODE(i, j, 0);
          for(k = 0;k<nzsp+koff;k++){
//          qdataptr[kk+k] = qq[IJKNODE(i, j, k)];
            *qqto++ = *qqfrom;
            qqfrom += nxy;
          }
        }
      }
    }
  }
  *ks2ptr += koff;
  *js2ptr += joff;
  *ntimesptr = nsteps;

  {
    int nvals;
    float *qq;

    if(*idirptr==3){
      nvals = nsteps*nxsp*nysp;
    }
    else if(*idirptr==2){
      nvals = nsteps*nxsp*(nzsp+koff);
    }
    else{
      nvals = nsteps*(nysp+joff)*(nzsp+koff)*nxsp;
    }
    qq = qdataptr;
    for(i = 0;i<nvals;i++){
      *qminptr = MIN(*qminptr, *qq);
      *qmaxptr = MAX(*qmaxptr, *qq);
      qq++;
    }
  }
  fclose(stream);
  FREEMEMORY(qq);
  return file_size;
}
#endif

  /* ------------------ ReadSlice ------------------------ */

FILE_SIZE ReadSlice(char *file, int ifile, int flag, int set_slicecolor, int *errorcode){
  float *xplt_local, *yplt_local, *zplt_local, offset, qmin, qmax, read_time, total_time;
  int blocknumber, error, i, ii, headersize, framesize, flag2 = 0;
  slicedata *sd;
  int ntimes_slice_old;

  vslicedata *vd;
  meshdata *meshi;

  FILE_SIZE return_filesize=0;
  int file_size=0;
#ifdef pp_memstatus
  unsigned int availmemory;
#endif

#ifndef pp_FSEEK
  if(flag==RELOAD)flag = LOAD;
#endif
  CheckMemory;
  START_TIMER(total_time);
  *errorcode = 0;
  error = 0;
  show_slice_average = 0;
  blocknumber = sliceinfo[ifile].blocknumber;
  meshi = meshinfo + blocknumber;

  slicefilenumber = ifile;
  slicefilenum = ifile;
  histograms_defined = 0;

  ASSERT(slicefilenumber >= 0 && slicefilenumber<nsliceinfo);
  sd = sliceinfo + slicefilenumber;
  if(flag != RESETBOUNDS){
    if(sd->loaded == 0 && flag == UNLOAD)return 0;
    sd->display = 0;
#ifdef pp_MEMDEBUG
    if(sd->qslicedata != NULL){
      ASSERT(ValidPointer(sd->qslicedata, sizeof(float)*sd->nslicetotal));
    }
#endif

// free memory buffers

    if(flag!=RELOAD){
      if(sd->qslicedata != NULL){
        FreeMemory(sd->qslicedata);
        sd->qslicedata = NULL;
      }
      FREEMEMORY(sd->times);
      FREEMEMORY(sd->slicelevel);
      FREEMEMORY(sd->compindex);
      FREEMEMORY(sd->qslicedata_compressed);
      FREEMEMORY(sd->slicecomplevel);

      if(sd->histograms!=NULL){
        for(i = 0; i<sd->nhistograms; i++){
          FreeHistogram(sd->histograms+i);
        }
        FREEMEMORY(sd->histograms);
      }
    }

    slicefilenum = ifile;

// reset slice variables to an unloaded state

    if(flag == UNLOAD){
      update_flipped_colorbar = 1;
      sd->ntimes_old = 0;
      sd->ntimes = 0;
      updatemenu = 1;
      sd->loaded = 0;
      sd->vloaded = 0;
      sd->display = 0;
      plotstate = GetPlotState(DYNAMIC_PLOTS);
      ReadVolSlice = 0;
      for(ii = 0; ii<nslice_loaded; ii++){
        slicedata *sdi;

        i = slice_loaded_list[ii];
        sdi = sliceinfo + i;
        if(sdi->volslice == 1)ReadVolSlice = 1;
      }
      for(ii = 0; ii<nslice_loaded; ii++){
        slicedata *sdi;

        i = slice_loaded_list[ii];
        sdi = sliceinfo + i;
        if(sdi->slicefile_labelindex == slicefile_labelindex){
          slicefilenum = i;
          flag2 = 1;
          break;
        }
      }
      if(flag2 == 0){
        for(ii = 0; ii<nslice_loaded; ii++){
          slicedata *sdi;

          i = slice_loaded_list[ii];
          sdi = sliceinfo + i;
          if(sdi->slicefile_labelindex != slicefile_labelindex){
            slicefilenum = i;
            flag2 = 1;
            break;
          }
        }
      }
      if(flag2 == 0){
        slicefilenum = 0;
        slicefile_labelindex = 0;
      }

      for(i = 0; i<nvsliceinfo; i++){
        vd = vsliceinfo + i;
        if(vd->iu == ifile)vd->u = NULL;
        if(vd->iv == ifile)vd->v = NULL;
        if(vd->iw == ifile)vd->w = NULL;
        if(vd->u == NULL&&vd->v == NULL&&vd->w == NULL){
          vd->loaded = 0;
          vd->display = 0;
        }
        if(vd->ival == ifile){
          vd->val = NULL;
          vd->loaded = 0;
          vd->display = 0;
        }
      }
      if(use_set_slicecolor == 0 || set_slicecolor == SET_SLICECOLOR){
        if(sd->compression_type == UNCOMPRESSED){
          UpdateSliceBounds();
          list_slice_index = slicefile_labelindex;
          SetSliceBounds(slicefile_labelindex);
          UpdateAllSliceColors(slicefile_labelindex, errorcode);
        }
        else{
          UpdateAllSliceLabels(slicefile_labelindex, errorcode);
        }
      }

      UpdateGlui();
      UpdateUnitDefs();
      UpdateTimes();
      RemoveSliceLoadstack(slicefilenumber);
      return 0;
    }

// load entire slice file (flag=LOAD) or
// load only portion of slice file written to since last time it was loaded (flag=RELOAD)

    if(sd->compression_type == UNCOMPRESSED){
      sd->ntimes_old = sd->ntimes;
#ifdef pp_CSLICE
      if(use_cslice==1){
        GetSliceSizes(file, &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, sliceframestep, &error,
          settmin_s, settmax_s, tmin_s, tmax_s, &headersize, &framesize);
      }
      else{
        FORTgetslicesizes(file, &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, &sliceframestep, &error,
          &settmin_s, &settmax_s, &tmin_s, &tmax_s, &headersize, &framesize,
          strlen(file));
      }
#else
      FORTgetslicesizes(file, &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, &sliceframestep, &error,
        &settmin_s, &settmax_s, &tmin_s, &tmax_s, &headersize, &framesize,
        strlen(file));
#endif
    }
    else if(sd->compression_type == COMPRESSED_ZLIB){
      if(
        GetSliceHeader(sd->comp_file, sd->size_file, sd->compression_type,
          sliceframestep, settmin_s, settmax_s, tmin_s, tmax_s,
          &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, &sd->ncompressed, &sd->valmin, &sd->valmax) == 0){
        ReadSlice("", ifile, UNLOAD, set_slicecolor, &error);
        *errorcode = 1;
        return 0;
      }
    }
    if(sd->nslicei != 1 && sd->nslicej != 1 && sd->nslicek != 1){
      sd->volslice = 1;
      ReadVolSlice = 1;
    }
    if(error != 0){
      ReadSlice("", ifile, UNLOAD, set_slicecolor, &error);
      *errorcode = 1;
      return 0;
    }
    if(settmax_s == 0 && settmin_s == 0 && sd->compression_type == UNCOMPRESSED){
      if(framesize <= 0){
        fprintf(stderr, "*** Error: frame size is 0 in slice file %s . \n", file);
        error = 1;
      }
      else{
        sd->ntimes = (int)(GetFileSizeSMV(file) - headersize) / framesize;
        if(sliceframestep>1)sd->ntimes /= sliceframestep;
      }
    }
    if(error != 0 || sd->ntimes<1){
      ReadSlice("", ifile, UNLOAD, set_slicecolor, &error);
      *errorcode = 1;
      return 0;
    }
    PRINTF("Loading %s(%s)", file,sd->label.shortlabel);
    MEMSTATUS(1, &availmemory, NULL, NULL);
    START_TIMER(read_time);
    if(sd->compression_type == COMPRESSED_ZLIB){
      int return_code;

      return_code = NewResizeMemory(sd->qslicedata_compressed, sd->ncompressed);
      if(return_code!=0)return_code = NewResizeMemory(sd->times, sizeof(float)*sd->ntimes);
      if(return_code!=0)return_code = NewResizeMemory(sd->compindex, sizeof(compdata)*(1+sd->ntimes));
      if(return_code==0){
        ReadSlice("", ifile, UNLOAD, set_slicecolor, &error);
        *errorcode = 1;
        return 0;
      }
      return_code=GetSliceCompressedData(sd->comp_file,
        settmin_s, settmax_s, tmin_s, tmax_s, sd->ncompressed, sliceframestep, sd->ntimes,
        sd->times, sd->qslicedata_compressed, sd->compindex, &sd->globalmin, &sd->globalmax);
      if(return_code == 0){
        ReadSlice("", ifile, UNLOAD, set_slicecolor, &error);
        *errorcode = 1;
        return 0;
      }
      file_size = sd->ncompressed;
      return_filesize = (FILE_SIZE)file_size;
    }
    else{
      int return_val;

      return_val = NewResizeMemory(sd->qslicedata, sizeof(float)*(sd->nslicei+1)*(sd->nslicej+1)*(sd->nslicek+1)*sd->ntimes);
      if(return_val!=0)return_val = NewResizeMemory(sd->times, sizeof(float)*sd->ntimes);

      if(return_val == 0){
        *errorcode = 1;
        ReadSlice("", ifile, UNLOAD, set_slicecolor, &error);
        return 0;
      }
#ifdef pp_MEMDEBUG
      ASSERT(ValidPointer(sd->qslicedata, sizeof(float)*sd->nslicei*sd->nslicej*sd->nslicek*sd->ntimes));
#endif

      ntimes_slice_old = 0;
      if(flag==RELOAD){
        ntimes_slice_old = sd->ntimes_old;
        qmin = sd->globalmin;
        qmax = sd->globalmax;
      }
      else{
        qmin = 1.0e30;
        qmax = -1.0e30;
      }
      if(sd->ntimes > ntimes_slice_old){
#ifdef pp_CSLICE
        if(use_cslice==1){
          return_filesize =
            GetSliceData(file, &sd->is1, &sd->is2, &sd->js1, &sd->js2, &sd->ks1, &sd->ks2, &sd->idir,
              &qmin, &qmax, sd->qslicedata, sd->times, ntimes_slice_old, &sd->ntimes,
              sliceframestep, settmin_s, settmax_s, tmin_s, tmax_s);
          file_size = (int)return_filesize;
        }
        else{
          FORTgetslicedata(file,
            &sd->is1, &sd->is2, &sd->js1, &sd->js2, &sd->ks1, &sd->ks2, &sd->idir,
            &qmin, &qmax, sd->qslicedata, sd->times, &ntimes_slice_old, &sd->ntimes, &sliceframestep,
            &settmin_s, &settmax_s, &tmin_s, &tmax_s, &file_size, strlen(file));
          return_filesize = (FILE_SIZE)file_size;
        }
#else
        FORTgetslicedata(file,
          &sd->is1, &sd->is2, &sd->js1, &sd->js2, &sd->ks1, &sd->ks2, &sd->idir,
          &qmin, &qmax, sd->qslicedata, sd->times, &ntimes_slice_old, &sd->ntimes, &sliceframestep,
          &settmin_s, &settmax_s, &tmin_s, &tmax_s, &file_size, strlen(file));
          return_filesize = (FILE_SIZE)file_size;
#endif
      }
#ifdef pp_MEMDEBUG
      ASSERT(ValidPointer(sd->qslicedata, sizeof(float)*sd->nslicei*sd->nslicej*sd->nslicek*sd->ntimes));
#endif
    }
    STOP_TIMER(read_time);

    if(slice_average_flag == 1){
      int data_per_timestep;
      int ndata;
      int ntimes_local;

      data_per_timestep = sd->nslicei*sd->nslicej*sd->nslicek;
      ntimes_local = sd->ntimes;
      ndata = data_per_timestep*ntimes_local;
      show_slice_average = 1;

      if(
        sd->compression_type == COMPRESSED_ZLIB ||
        AverageSliceData(sd->qslicedata, sd->qslicedata, ndata, data_per_timestep, sd->times, ntimes_local, slice_average_interval) == 1
        ){
        show_slice_average = 0; // averaging failed
      }
    }

    /*  initialize slice data */

    sd->nslicetotal = 0;
    sd->nsliceijk = 0;
    if(sd->ntimes == 0)return 0;

    /* estimate the slice offset, the distance to move a slice so
    that it does not "interfere" with an adjacent block */

    blocknumber = sliceinfo[ifile].blocknumber;
    xplt_local = meshinfo[blocknumber].xplt;
    yplt_local = meshinfo[blocknumber].yplt;
    zplt_local = meshinfo[blocknumber].zplt;

    xslicemid = (xplt_local[sd->is1] + xplt_local[sd->is2]) / 2.0;
    yslicemid = (yplt_local[sd->js1] + yplt_local[sd->js2]) / 2.0;
    zslicemid = (zplt_local[sd->ks1] + zplt_local[sd->ks2]) / 2.0;

    sd->sliceoffset = 0.0;

    switch (sd->idir){
    case XDIR:
      offset = sliceoffset_factor*(xplt_local[1] - xplt_local[0]);
      if(InBlockage(meshi, xslicemid - offset, yslicemid, zslicemid) == 1){
        sd->sliceoffset = offset;
      }
      if(InBlockage(meshi, xslicemid + offset, yslicemid, zslicemid) == 1){
        sd->sliceoffset = -offset;
      }
      sd->nslicex = sd->js2 + 1 - sd->js1;
      sd->nslicey = sd->ks2 + 1 - sd->ks1;
      break;
    case YDIR:
      offset = sliceoffset_factor*(yplt_local[1] - yplt_local[0]);
      if(InBlockage(meshi, xslicemid, yslicemid - offset, zslicemid) == 1){
        sd->sliceoffset = offset;
      }
      if(InBlockage(meshi, xslicemid, yslicemid + offset, zslicemid) == 1){
        sd->sliceoffset = -offset;
      }
      sd->nslicex = sd->is2 + 1 - sd->is1;
      sd->nslicey = sd->ks2 + 1 - sd->ks1;
      break;
    case ZDIR:
      offset = sliceoffset_factor*(zplt_local[1] - zplt_local[0]);
      if(InBlockage(meshi, xslicemid, yslicemid, zslicemid - offset) == 1){
        sd->sliceoffset = offset;
      }
      if(InBlockage(meshi, xslicemid, yslicemid, zslicemid + offset) == 1){
        sd->sliceoffset = -offset;
      }
      sd->nslicex = sd->is2 + 1 - sd->is1;
      sd->nslicey = sd->js2 + 1 - sd->js1;
      break;
    default:
      ASSERT(FFALSE);
      break;
    }

    sd->nsliceijk = sd->nslicei*sd->nslicej*sd->nslicek;
    sd->nslicetotal = sd->ntimes*sd->nsliceijk;
    if(sd->compression_type == COMPRESSED_ZLIB){
      if(NewMemory((void **)&sd->slicecomplevel, sd->nsliceijk * sizeof(unsigned char)) == 0){
        ReadSlice("", ifile, UNLOAD, set_slicecolor, &error);
        *errorcode = 1;
        return 0;
      }
    }
    else{
      int return_code;

      return_code = NewResizeMemory(sd->slicelevel, sd->nslicetotal*sizeof(int));
      if(return_code == 0){
        ReadSlice("", ifile, UNLOAD, set_slicecolor, &error);
        *errorcode = 1;
        return 0;
      }
    }

#ifdef pp_MEMDEBUG
    if(sd->compression_type == UNCOMPRESSED){
      ASSERT(ValidPointer(sd->qslicedata, sizeof(float)*sd->nslicetotal));
    }
#endif
  }  /* RESETBOUNDS */

     // convert slice data into color indices

  if(sd->compression_type == UNCOMPRESSED){
    GetSliceDataBounds(sd, &qmin, &qmax);
  }
  else{
    qmin = sd->valmin;
    qmax = sd->valmax;
  }
  sd->globalmin = qmin;
  sd->globalmax = qmax;
  if(sd->compression_type == UNCOMPRESSED){
    AdjustSliceBounds(sd, &qmin, &qmax);
  }
  sd->valmin = qmin;
  sd->valmax = qmax;
  sd->valmin_data = qmin;
  sd->valmax_data = qmax;
  for(i = 0; i<256; i++){
    sd->qval256[i] = (qmin*(255 - i) + qmax*i) / 255;
  }
  CheckMemory;

  if(sd->slicefile_type == SLICE_CELL_CENTER){
    usetexturebar = 0;
  }
  sd->loaded = 1;
  if(sd->vloaded == 0)sd->display = 1;
  slicefile_labelindex = GetSliceBoundsIndex(sd);
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  if(sd->finalize==1){
    UpdateUnitDefs();
    UpdateTimes();
    CheckMemory;

    if(use_set_slicecolor==0||set_slicecolor==SET_SLICECOLOR){
      if(sd->compression_type==UNCOMPRESSED){
        UpdateSliceBounds();
        UpdateAllSliceColors(slicefile_labelindex, errorcode);
        list_slice_index = slicefile_labelindex;
        SetSliceBounds(slicefile_labelindex);
      }
      else{
        slicebounds[slicefile_labelindex].valmin_data = qmin;
        slicebounds[slicefile_labelindex].valmax_data = qmax;
        UpdateAllSliceLabels(slicefile_labelindex, errorcode);
      }
    }
    CheckMemory;

    UpdateSliceList(list_slice_index);
    CheckMemory;
    UpdateSliceListIndex(slicefilenum);
    CheckMemory;
    UpdateGlui();
    CheckMemory;
#ifdef pp_MEMDEBUG
    if(sd->compression_type==UNCOMPRESSED){
      ASSERT(ValidPointer(sd->qslicedata, sizeof(float)*sd->nslicei*sd->nslicej*sd->nslicek*sd->ntimes));
    }
    CheckMemory;
#endif
    IdleCB();
  }

  exportdata = 1;
  if(exportdata == 0){
    FREEMEMORY(sd->qslicedata);
  }

  STOP_TIMER(total_time);


  if(flag != RESETBOUNDS){
    if(file_size>1000000000){
      PRINTF(" - %.1f GB/%.1f s\n", (float)file_size / 1000000000., total_time);
    }
    else if(file_size>1000000){
      PRINTF(" - %.1f MB/%.1f s\n", (float)file_size / 1000000., total_time);
    }
    else{
      PRINTF(" - %.0f KB/%.1f s\n", (float)file_size / 1000., total_time);
    }
  }

  if(update_fire_line == 0 && strcmp(sd->label.shortlabel, "Fire line") == 0){
    update_fire_line = 1;
  }
  update_flipped_colorbar=1;

  if(colorbartype_ini == -1){
    if(strcmp(sd->label.shortlabel, "thick") == 0){
      ColorbarMenu(wallthickness_colorbar);
    }
    if(strcmp(sd->label.shortlabel, "phifield") == 0){
      ColorbarMenu(levelset_colorbar);
    }
  }
  PushSliceLoadstack(slicefilenumber);

  if(sd->volslice == 1){
    meshdata *meshj;

    meshj = meshinfo + sd->blocknumber;

    meshj->slice_min[0] = DENORMALIZE_X(sd->xyz_min[0]);
    meshj->slice_min[1] = DENORMALIZE_Y(sd->xyz_min[1]);
    meshj->slice_min[2] = DENORMALIZE_Z(sd->xyz_min[2]);

    meshj->slice_max[0] = DENORMALIZE_X(sd->xyz_max[0]);
    meshj->slice_max[1] = DENORMALIZE_Y(sd->xyz_max[1]);
    meshj->slice_max[2] = DENORMALIZE_Z(sd->xyz_max[2]);

#ifdef pp_GPU
    if(gpuactive == 1){
      InitSlice3DTexture(meshj);
    }
#endif
  }
  else{
    meshdata *meshj;

    meshj = meshinfo + sd->blocknumber;
    meshj->slice_min[0] = 1.0;
    meshj->slice_min[1] = 0.0;
    meshj->slice_min[2] = 1.0;
    meshj->slice_max[0] = 0.0;
    meshj->slice_max[1] = 1.0;
    meshj->slice_max[2] = 0.0;
  }

  // define histogram data structures if visible

  if(histogram_show_graph==1||histogram_show_numbers==1){
    update_slice_hists=1;
  }
  if(sd->is_fed == 1){
    update_slice_hists = 1;
    histogram_nbuckets = 255;
    histogram_show_numbers=0;
  }
  showall_slices=1;
  glutPostRedisplay();
  return return_filesize;
}

/* ------------------ UpdateSlice3DTexture ------------------------ */

void UpdateSlice3DTexture(meshdata *meshi, slicedata *slicei, float *valdata){
  GLint xoffset = 0, yoffset = 0, zoffset = 0;
  GLsizei nx, ny, nz, nxy;
  int slice_ny, slice_nz;
  int i, j, k;
  float *cbuffer;
  int *ijk_min, *ijk_max;
  int kindex;
  int slice_nyz;


  nx = meshi->ibar + 1;
  ny = meshi->jbar + 1;
  nz = meshi->kbar + 1;
  ijk_min = slicei->ijk_min;
  ijk_max = slicei->ijk_max;

  slice_ny = ijk_max[1] - ijk_min[1] + 1;
  slice_nz = ijk_max[2] - ijk_min[2] + 1;
  slice_nyz = slice_ny*slice_nz;

  nxy = nx*ny;
  for(k = ijk_min[2],kindex=0; k<ijk_max[2]+1; k++,kindex++){
    int jindex;
    
    for(j = ijk_min[1],jindex=0; j < ijk_max[1] + 1; j++,jindex+=slice_nz){
      float *v;

      cbuffer = meshi->slice3d_c_buffer+ IJKNODE(ijk_min[0], j, k);
      v = valdata + jindex + kindex;
      for(i = ijk_min[0]; i<ijk_max[0]+1; i++){
        *cbuffer++ = *v;
        v+=slice_nyz;
      }
    }
  }

  cbuffer = meshi->slice3d_c_buffer;
  glActiveTexture(GL_TEXTURE0);
  glTexSubImage3D(GL_TEXTURE_3D, 0, xoffset, yoffset, zoffset, nx, ny, nz, GL_RED, GL_FLOAT, cbuffer);
}

/* ------------------ DrawGSliceDataGpu ------------------------ */

void DrawGSliceDataGpu(slicedata *slicei){
  meshdata *meshi;
  int j;
  boundsdata *sb;
  float valmin, valmax;
  float *boxmin, *boxmax;
  float *verts;
  int *triangles;

  if(slicei->loaded == 0 || slicei->display == 0 || slicei->volslice == 0)return;

  meshi = meshinfo + slicei->blocknumber;
  verts = meshi->gsliceinfo.verts;
  triangles = meshi->gsliceinfo.triangles;

  if(meshi->gsliceinfo.nverts == 0 || meshi->gsliceinfo.ntriangles == 0)return;

  UpdateSlice3DTexture(meshi, slicei, slicei->qsliceframe);
  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);

  if(cullfaces == 1)glDisable(GL_CULL_FACE);
  if(use_transparency_data == 1)TransparentOn();


  sb = slicebounds + slicefile_labelindex;
  valmin = sb->levels256[0] * sb->fscale;
  valmax = sb->levels256[255] * sb->fscale;
  boxmin = meshi->boxmin;
  boxmax = meshi->boxmax;

  glUniform1i(GPU3dslice_valtexture, 0);
  glUniform1i(GPU3dslice_colormap, 4);
  glUniform1f(GPU3dslice_val_min, valmin);
  glUniform1f(GPU3dslice_val_max, valmax);
  glUniform1f(GPU3dslice_transparent_level, transparent_level);
  glUniform3f(GPU3dslice_boxmin, boxmin[0], boxmin[1], boxmin[2]);
  glUniform3f(GPU3dslice_boxmax, boxmax[0], boxmax[1], boxmax[2]);
  glBegin(GL_TRIANGLES);

  for(j = 0; j < meshi->gsliceinfo.ntriangles; j++){
    float *xyz1, *xyz2, *xyz3;

    xyz1 = verts + 3*triangles[3*j];
    xyz2 = verts + 3*triangles[3*j + 1];
    xyz3 = verts + 3*triangles[3*j + 2];
    glVertex3fv(xyz1);
    glVertex3fv(xyz2);
    glVertex3fv(xyz3);
  }
  glEnd();
  if(use_transparency_data == 1)TransparentOff();
  if(cullfaces == 1)glEnable(GL_CULL_FACE);
  glPopMatrix();
}

/* ------------------ DrawVolSliceCellFaceCenter ------------------------ */

void DrawVolSliceCellFaceCenter(const slicedata *sd, int flag){
  float *xplt, *yplt, *zplt;
  int plotx, ploty, plotz;
  int ibar, jbar;
  char *iblank_cell, *iblank_embed;
  int incx = 0, incy = 0, incz = 0;
  int iimin;

  meshdata *meshi;

  float *rgb_ptr;

  rgb_ptr = rgb_slice;

  meshi = meshinfo + sd->blocknumber;

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  if(sd->volslice == 1){
    plotx = meshi->iplotx_all[iplotx_all];
    ploty = meshi->iploty_all[iploty_all];
    plotz = meshi->iplotz_all[iplotz_all];
    incx = 1;
    incy = 1;
    incz = 1;
    iimin = 0;
  }
  else{
    plotx = sd->is1;
    ploty = sd->js1;
    plotz = sd->ks1;
    iimin = plotx;
  }

  ibar = meshi->ibar;
  jbar = meshi->jbar;

  iblank_cell = meshi->c_iblank_cell;
  iblank_embed = meshi->c_iblank_embed;

  if(cullfaces == 1)glDisable(GL_CULL_FACE);

  if(use_transparency_data == 1)TransparentOn();
  if((sd->volslice == 1 && plotx > 0 && visx_all == 1) || (sd->volslice == 0 && sd->idir == XDIR)){
    float constval;
    int maxj;
    int j;

    switch(flag){
    case SLICE_CELL_CENTER:
      constval = (xplt[plotx] + xplt[plotx - 1]) / 2.0;
      break;
    case SLICE_FACE_CENTER:
      constval = xplt[plotx - 1];
      break;
    default:
      constval = (xplt[plotx] + xplt[plotx - 1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(sd->sliceoffset_fds);

    glBegin(GL_TRIANGLES);
    maxj = sd->js2;
    if(sd->js1 + 1>maxj){
      maxj = sd->js1 + 1;
    }
    for(j = sd->js1; j<maxj; j++){
      float yy1;
      int k;
      float y3;

      yy1 = yplt[j];
      y3 = yplt[j + 1];
      // val(i,j,k) = di*nj*nk + dj*nk + dk
      for(k = sd->ks1; k<sd->ks2; k++){
        int index_cell;
        int i33;
        float z1, z3;

        if(show_slice_in_obst == ONLY_IN_SOLID && iblank_cell != NULL&&iblank_cell[IJKCELL(plotx-1, j, k)] == GAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && iblank_cell != NULL&&iblank_cell[IJKCELL(plotx-1, j, k)] != GAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(plotx, j, k)] == EMBED_YES)continue;

        index_cell = (plotx + 1 - incx - iimin)*sd->nslicej*sd->nslicek + (j + 1 - sd->js1)*sd->nslicek + k + 1 - sd->ks1;

        i33 = 4 * sd->iqsliceframe[index_cell];
        z1 = zplt[k];
        z3 = zplt[k + 1];
        /*
        n+1 (y1,z3) n2+1 (y3,z3)
        n (y1,z1)     n2 (y3,z1)
        */

        glColor4fv(&rgb_ptr[i33]);
        glVertex3f(constval, yy1, z1);
        glVertex3f(constval, y3, z1);
        glVertex3f(constval, y3, z3);

        glVertex3f(constval, yy1, z1);
        glVertex3f(constval, y3, z3);
        glVertex3f(constval, yy1, z3);
      }
    }
    glEnd();
    if(cell_center_text == 1){
      for(j = sd->js1; j<maxj; j++){
        float yy1, y3;
        int k;

        yy1 = yplt[j];
        y3 = yplt[j + 1];
        // val(i,j,k) = di*nj*nk + dj*nk + dk
        for(k = sd->ks1; k<sd->ks2; k++){
          float val;
          int index_cell;
          float z1, z3;

          if(show_slice_in_obst == ONLY_IN_SOLID && iblank_cell != NULL&&iblank_cell[IJKCELL(plotx-1, j, k)] == GAS)continue;
          if(show_slice_in_obst == ONLY_IN_GAS   && iblank_cell != NULL&&iblank_cell[IJKCELL(plotx-1, j, k)] != GAS)continue;
          if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(plotx, j, k)] == EMBED_YES)continue;
          z1 = zplt[k];
          z3 = zplt[k + 1];
          /*
          n+1 (y1,z3) n2+1 (y3,z3)
          n (y1,z1)     n2 (y3,z1)
          */
          index_cell = (plotx + 1 - incx - iimin)*sd->nslicej*sd->nslicek + (j + 1 - sd->js1)*sd->nslicek + k + 1 - sd->ks1;

          GET_VAL(sd, val, index_cell);
          Output3Val(constval, (yy1 + y3) / 2.0, (z1 + z3) / 2.0, val);
        }
      }
    }
  }
  if((sd->volslice == 1 && ploty > 0 && visy_all == 1) || (sd->volslice == 0 && sd->idir == YDIR)){
    float constval;
    int i;
    int maxi;

    switch(flag){
    case SLICE_CELL_CENTER:
      constval = (yplt[ploty] + yplt[ploty - 1]) / 2.0;
      break;
    case SLICE_FACE_CENTER:
      constval = yplt[ploty - 1];
      break;
    default:
      constval = (yplt[ploty] + yplt[ploty - 1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(sd->sliceoffset_fds);

    glBegin(GL_TRIANGLES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1>maxi){
      maxi = sd->is1 + 1;
    }
    for(i = sd->is1; i<maxi; i++){
      int index_cell;
      float x1, x3;
      int k;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      for(k = sd->ks1; k<sd->ks2; k++){
        int i33;
        float z1, z3;

        if(show_slice_in_obst == ONLY_IN_SOLID && iblank_cell != NULL&&iblank_cell[IJKCELL(i, ploty-1, k)] == GAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && iblank_cell != NULL&&iblank_cell[IJKCELL(i, ploty-1, k)] != GAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(i, ploty, k)] == EMBED_YES)continue;
        index_cell = (i + incx - sd->is1)*sd->nslicej*sd->nslicek + (ploty + 1 - incy - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
        i33 = 4 * sd->iqsliceframe[index_cell];
        z1 = zplt[k];
        z3 = zplt[k + 1];
        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        glColor4fv(&rgb_ptr[i33]);
        glVertex3f(x1, constval, z1);
        glVertex3f(x3, constval, z1);
        glVertex3f(x3, constval, z3);

        glVertex3f(x1, constval, z1);
        glVertex3f(x3, constval, z3);
        glVertex3f(x1, constval, z3);
      }
    }
    glEnd();
    if(cell_center_text == 1){
      for(i = sd->is1; i<maxi; i++){
        float x1, x3;
        int k;

        x1 = xplt[i];
        x3 = xplt[i + 1];
        for(k = sd->ks1; k<sd->ks2; k++){
          float val;
          int index_cell;
          float z1, z3;

          if(show_slice_in_obst == ONLY_IN_SOLID && iblank_cell != NULL&&iblank_cell[IJKCELL(i, ploty-1, k)] == GAS)continue;
          if(show_slice_in_obst == ONLY_IN_GAS   && iblank_cell != NULL&&iblank_cell[IJKCELL(i, ploty-1, k)] != GAS)continue;
          if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(i, ploty, k)] == EMBED_YES)continue;

          index_cell = (i + incx - sd->is1)*sd->nslicej*sd->nslicek + (ploty + 1 - incy - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
          z1 = zplt[k];
          z3 = zplt[k + 1];
          /*
          n+1 (x1,z3)   n2+1 (x3,z3)
          n (x1,z1)     n2 (x3,z1)

          val(i,j,k) = di*nj*nk + dj*nk + dk
          */
          GET_VAL(sd, val, index_cell);
          Output3Val((x1 + x3) / 2.0, constval, (z1 + z3) / 2.0, val);
        }
      }
    }
  }
  if((sd->volslice == 1 && plotz > 0 && visz_all == 1) || (sd->volslice == 0 && sd->idir == ZDIR)){
    float constval;
    int i;
    int maxi;

    switch(flag){
    case SLICE_CELL_CENTER:
      constval = (zplt[plotz] + zplt[plotz - 1]) / 2.0;
      break;
    case SLICE_FACE_CENTER:
      constval = zplt[plotz - 1];
      break;
    default:
      constval = (zplt[plotz] + zplt[plotz - 1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(sd->sliceoffset_fds);

    glBegin(GL_TRIANGLES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1>maxi){
      maxi = sd->is1 + 1;
    }
    for(i = sd->is1; i<maxi; i++){
      float x1, x3;
      int j;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      for(j = sd->js1; j<sd->js2; j++){
        int index_cell;
        int i33;
        float yy1, y3;

        if(show_slice_in_obst == ONLY_IN_SOLID && iblank_cell != NULL&&iblank_cell[IJKCELL(i, j, plotz-1)] == GAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && iblank_cell != NULL&&iblank_cell[IJKCELL(i, j, plotz-1)] != GAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(i, j, plotz)] == EMBED_YES)continue;

        index_cell = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (j + incy - sd->js1)*sd->nslicek + plotz + 1 - incz - sd->ks1;
        i33 = 4 * sd->iqsliceframe[index_cell];
        yy1 = yplt[j];
        y3 = yplt[j + 1];
        /*
        n+nk (x1,y3)   n2+nk (x3,y3)
        n (x1,y1)      n2 (x3,y1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        glColor4fv(&rgb_ptr[i33]);
        glVertex3f(x1, yy1, constval);
        glVertex3f(x3, yy1, constval);
        glVertex3f(x3, y3, constval);

        glVertex3f(x1, yy1, constval);
        glVertex3f(x3, y3, constval);
        glVertex3f(x1, y3, constval);
      }
    }
    glEnd();
    if(cell_center_text == 1){
      for(i = sd->is1; i<maxi; i++){
        float x1, x3;
        int j;

        x1 = xplt[i];
        x3 = xplt[i + 1];
        for(j = sd->js1; j<sd->js2; j++){
          float val;
          int index_cell;
          float yy1, y3;

          if(show_slice_in_obst == ONLY_IN_SOLID && iblank_cell != NULL&&iblank_cell[IJKCELL(i, j, plotz-1)] == GAS)continue;
          if(show_slice_in_obst == ONLY_IN_GAS   && iblank_cell != NULL&&iblank_cell[IJKCELL(i, j, plotz-1)] != GAS)continue;
          if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(i, j, plotz)] == EMBED_YES)continue;

          index_cell = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (j + incy - sd->js1)*sd->nslicek + plotz + 1 - incz - sd->ks1;
          yy1 = yplt[j];
          y3 = yplt[j + 1];
          /*
          n+nk (x1,y3)   n2+nk (x3,y3)
          n (x1,y1)      n2 (x3,y1)

          val(i,j,k) = di*nj*nk + dj*nk + dk
          */
          GET_VAL(sd, val, index_cell);
          Output3Val((x1 + x3) / 2.0, (yy1 + y3) / 2.0, constval, val);
        }
      }
    }
  }
  if(use_transparency_data == 1)TransparentOff();
  if(cullfaces == 1)glEnable(GL_CULL_FACE);

}

/* ------------------ DrawVolSliceTerrain ------------------------ */

void DrawVolSliceTerrain(const slicedata *sd){
  int i, j, k, n, n2;
  float r11, r31, r13, r33;
  float constval, x1, x3, yy1, y3, z1, z3;

  float *xplt, *yplt, *zplt;
  int plotx, ploty, plotz;
  int ibar, jbar;
  int nx, ny, nxy;
  char *iblank_x, *iblank_y, *iblank_z;
  terraindata *terri;
  int nycell;
  char *iblank_embed;

  meshdata *meshi;

  meshi = meshinfo + sd->blocknumber;

  terri = meshi->terrain;
  if(terri == NULL)return;
  nycell = terri->ny;

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  if(sd->volslice == 1){
    plotx = meshi->iplotx_all[iplotx_all];
    ploty = meshi->iploty_all[iploty_all];
    plotz = meshi->iplotz_all[iplotz_all];
  }
  else{
    plotx = sd->is1;
    ploty = sd->js1;
    plotz = sd->ks1;
  }
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  iblank_x = meshi->c_iblank_x;
  iblank_y = meshi->c_iblank_y;
  iblank_z = meshi->c_iblank_z;
  iblank_embed = meshi->c_iblank_embed;
  nx = ibar + 1;
  ny = jbar + 1;
  nxy = nx*ny;

  if(cullfaces == 1)glDisable(GL_CULL_FACE);

  if(use_transparency_data == 1)TransparentOn();
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, texture_slice_colorbar_id);
  if((sd->volslice == 1 && plotx >= 0 && visx_all == 1) || (sd->volslice == 0 && sd->idir == XDIR)){
    int maxj;

    constval = xplt[plotx] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glBegin(GL_TRIANGLES);
    maxj = sd->js2;
    if(sd->js1 + 1>maxj){
      maxj = sd->js1 + 1;
    }
    for(j = sd->js1; j<maxj; j++){
      float ymid;

      n = (j - sd->js1)*sd->nslicek - 1;
      n += (plotx - sd->is1)*sd->nslicej*sd->nslicek;
      n2 = n + sd->nslicek;
      yy1 = yplt[j];
      y3 = yplt[j + 1];
      ymid = (yy1 + y3) / 2.0;

      // val(i,j,k) = di*nj*nk + dj*nk + dk
      for(k = sd->ks1; k<sd->ks2; k++){
        float rmid, zmid;

        n++; n2++;
        if(iblank_x != NULL&&iblank_x[IJK(plotx, j, k)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(plotx, j, k)] == EMBED_YES)continue;
        r11 = (float)sd->iqsliceframe[n] / 255.0;
        r31 = (float)sd->iqsliceframe[n2] / 255.0;
        r13 = (float)sd->iqsliceframe[n + 1] / 255.0;
        r33 = (float)sd->iqsliceframe[n2 + 1] / 255.0;
        rmid = (r11 + r31 + r13 + r33) / 4.0;

        z1 = zplt[k];
        z3 = zplt[k + 1];
        zmid = (z1 + z3) / 2.0;

        /*
        n+1 (y1,z3) n2+1 (y3,z3)
        n (y1,z1)     n2 (y3,z1)
        */
        //  (yy1,z3,r13)                    (y3,z3,r33)
        //                (ymid,zmid,rmid)
        //  (yy1,z1,r11)                    (y3,z1,r31)
        glTexCoord1f(r11); glVertex3f(constval, yy1, z1);
        glTexCoord1f(r31); glVertex3f(constval, y3, z1);
        glTexCoord1f(rmid); glVertex3f(constval, ymid, zmid);
        glTexCoord1f(r31); glVertex3f(constval, y3, z1);
        glTexCoord1f(r33); glVertex3f(constval, y3, z3);
        glTexCoord1f(rmid); glVertex3f(constval, ymid, zmid);
        glTexCoord1f(r33); glVertex3f(constval, y3, z3);
        glTexCoord1f(r13); glVertex3f(constval, yy1, z3);
        glTexCoord1f(rmid); glVertex3f(constval, ymid, zmid);
        glTexCoord1f(r13); glVertex3f(constval, yy1, z3);
        glTexCoord1f(r11); glVertex3f(constval, yy1, z1);
        glTexCoord1f(rmid); glVertex3f(constval, ymid, zmid);
      }
    }
    glEnd();
  }
  if((sd->volslice == 1 && ploty >= 0 && visy_all == 1) || (sd->volslice == 0 && sd->idir == YDIR)){
    int maxi;

    constval = yplt[ploty] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glBegin(GL_TRIANGLES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1>maxi){
      maxi = sd->is1 + 1;
    }
    for(i = sd->is1; i<maxi; i++){
      float xmid;

      n = (i - sd->is1)*sd->nslicej*sd->nslicek - 1;
      n += (ploty - sd->js1)*sd->nslicek;
      n2 = n + sd->nslicej*sd->nslicek;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      xmid = (x1 + x3) / 2.0;

      for(k = sd->ks1; k<sd->ks2; k++){
        float rmid, zmid;

        n++; n2++;
        if(iblank_y != NULL&&iblank_y[IJK(i, ploty, k)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, ploty, k)] == EMBED_YES)continue;
        r11 = (float)sd->iqsliceframe[n] / 255.0;
        r31 = (float)sd->iqsliceframe[n2] / 255.0;
        r13 = (float)sd->iqsliceframe[n + 1] / 255.0;
        r33 = (float)sd->iqsliceframe[n2 + 1] / 255.0;
        rmid = (r11 + r31 + r13 + r33) / 4.0;

        z1 = zplt[k];
        z3 = zplt[k + 1];
        zmid = (z1 + z3) / 2.0;

        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,z3,r13)                    (x3,z3,r33)
        //                (xmid,zmid,rmid)
        //  (x1,z1,r11)                    (x3,z1,r31)
        glTexCoord1f(r11); glVertex3f(x1, constval, z1);
        glTexCoord1f(r31); glVertex3f(x3, constval, z1);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r31); glVertex3f(x3, constval, z1);
        glTexCoord1f(r33); glVertex3f(x3, constval, z3);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r33); glVertex3f(x3, constval, z3);
        glTexCoord1f(r13); glVertex3f(x1, constval, z3);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r13); glVertex3f(x1, constval, z3);
        glTexCoord1f(r11); glVertex3f(x1, constval, z1);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
      }
    }
    glEnd();
  }
  if((sd->volslice == 1 && plotz >= 0 && visz_all == 1) || (sd->volslice == 0 && sd->idir == ZDIR)){
    float z11, z31, z13, z33, zmid;
    int maxi;
    float *znode, zoffset;

    znode = terri->znode_scaled;
    constval = zplt[plotz] + offset_slice*sd->sliceoffset + 0.001+SCALE2SMV(sd->sliceoffset_fds);
    zoffset = SCALE2SMV(sd->above_ground_level);
    glBegin(GL_TRIANGLES);
    maxi = MAX(sd->is1 + sd->nslicei - 1, sd->is1 + 1);
    for(i = sd->is1; i<maxi; i++){
      float xmid;

      if(plotz<sd->ks1)break;
      if(plotz >= sd->ks1 + sd->nslicek)break;
      x1 = xplt[i];
      x3 = xplt[i + 1];
      xmid = (x1 + x3) / 2.0;

      for(j = sd->js1; j<sd->js2; j++){
        float ymid, rmid;
        int n11, n31, n13, n33;

        z11 = znode[IJ2(i, j)] + zoffset;
        z31 = znode[IJ2(i + 1, j)] + zoffset;
        z13 = znode[IJ2(i, j + 1)] + zoffset;
        z33 = znode[IJ2(i + 1, j + 1)] + zoffset;
        zmid = (z11 + z31 + z13 + z33) / 4.0;

        if(iblank_z != NULL&&iblank_z[IJK(i, j, plotz)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, j, plotz)] == EMBED_YES)continue;

        n11 = (i - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1)*sd->nslicek;
        r11 = Interp3DSliceIndex(sd->iqsliceframe, zplt, meshi->kbar, n11, constval) / 255.0;

        n31 = n11 + sd->nslicej*sd->nslicek;
        r31 = Interp3DSliceIndex(sd->iqsliceframe, zplt, meshi->kbar, n31, constval) / 255.0;

        n13 = n11 + sd->nslicek;
        r13 = Interp3DSliceIndex(sd->iqsliceframe, zplt, meshi->kbar, n13, constval) / 255.0;

        n33 = n13 + sd->nslicej*sd->nslicek;
        r33 = Interp3DSliceIndex(sd->iqsliceframe, zplt, meshi->kbar, n33, constval) / 255.0;

        rmid = (r11 + r31 + r13 + r33) / 4.0;

        yy1 = yplt[j];
        y3 = yplt[j + 1];
        ymid = (yy1 + y3) / 2.0;

        /*
        n+nk (x1,y3)   n2+nk (x3,y3)
        n (x1,y1)      n2 (x3,y1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,y3,r13,z13)                    (x3,y3,r33,z33)
        //                (xmid,ymid,rmid,zmid)
        //  (x1,yy1,r11,z11)                    (x3,yy1,r31,z31)

        glTexCoord1f(r11); glVertex3f(x1, yy1, z11);
        glTexCoord1f(r31); glVertex3f(x3, yy1, z31);
        glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);

        glTexCoord1f(r31); glVertex3f(x3, yy1, z31);
        glTexCoord1f(r33); glVertex3f(x3, y3, z33);
        glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);

        glTexCoord1f(r33); glVertex3f(x3, y3, z33);
        glTexCoord1f(r13); glVertex3f(x1, y3, z13);
        glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);

        glTexCoord1f(r13); glVertex3f(x1, y3, z13);
        glTexCoord1f(r11); glVertex3f(x1, yy1, z11);
        glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
      }
    }
    glEnd();
  }
  glDisable(GL_TEXTURE_1D);
  if(use_transparency_data == 1)TransparentOff();
  if(cullfaces == 1)glEnable(GL_CULL_FACE);

}

#define VALIJK(i,j,k) ( ((i))*sd->nslicej*sd->nslicek + ((j))*sd->nslicek+ (k)    )

/* ------------------ DrawVolAllSlicesTextureDiag ------------------------ */

void DrawVolAllSlicesTextureDiag(const slicedata *sd, int direction){
  int i, j, k, n, n2;
  float r11, r31, r13, r33;
  float x1, x3, yy1, y3, z1, z3;

  float *xplt, *yplt, *zplt;
  int ibar, jbar;
  int nx, ny, nxy;
  char *c_iblank_x, *c_iblank_y;
  char *iblank_embed;

  meshdata *meshi;

  if(sd->volslice == 1 && visx_all == 0 && visy_all == 0 && visz_all == 0)return;
  meshi = meshinfo+sd->blocknumber;

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  c_iblank_x = meshi->c_iblank_x;
  c_iblank_y = meshi->c_iblank_y;
  iblank_embed = meshi->c_iblank_embed;
  nx = ibar + 1;
  ny = jbar + 1;
  nxy = nx*ny;

  if(cullfaces == 1)glDisable(GL_CULL_FACE);
  if(use_transparency_data == 1)TransparentOn();
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, texture_slice_colorbar_id);
  glBegin(GL_TRIANGLES);

  if(visx_all==1){
    int icol, icol2;
    int nx, ny;

    nx = sd->nslicei;
    ny = sd->nslicej;

    for(icol2 = 1;icol2<nx+ny-2;icol2++){
      int iimin;

      icol = icol2;
      if(direction<0)icol = nx + ny - 2 - icol2;
      iimin = nx-icol-1;
      if(icol>=nx-1)iimin = 0;
      // icol + i - j = nx - 1
      for(i = iimin; i<nx-1; i++){
        float xmid, ymid;
        int ijk;

        j = icol+i+1-nx;
        if(j>ny-2)break;

        n = VALIJK(i, j, -1);
        n2 = VALIJK(i+1, j+1, -1);

        // val(i,j,k) = di*nj*nk + dj*nk + dk
        x1 = xplt[i];
        x3 = xplt[i+1];
        xmid = (x1+x3)/2.0;
        yy1 = yplt[j];
        y3 = yplt[j+1];
        ymid = (yy1 + y3)/2.0;

        ijk = IJK(i, j, -1);
        for(k = sd->ks1; k<sd->ks2; k++){
          float rmid, zmid;

          n++; n2++;ijk+=nxy;
          if(show_slice_in_obst==ONLY_IN_SOLID && c_iblank_x != NULL&&c_iblank_x[ijk]==GASGAS)continue;
          if(show_slice_in_obst==ONLY_IN_GAS   && c_iblank_x != NULL&&c_iblank_x[ijk]!=GASGAS)continue;
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[ijk]==EMBED_YES)continue;
          r11 = (float)sd->iqsliceframe[n] / 255.0;
          r31 = (float)sd->iqsliceframe[n2] / 255.0;
          r13 = (float)sd->iqsliceframe[n + 1] / 255.0;
          r33 = (float)sd->iqsliceframe[n2 + 1] / 255.0;
          rmid = (r11 + r31 + r13 + r33) / 4.0;

          z1 = zplt[k];
          z3 = zplt[k+1];
          zmid = (z1 + z3) / 2.0;
        /*
        n+1 (y1,z3) n2+1 (y3,z3)
        n (y1,z1)     n2 (y3,z1)
        */
        //  (yy1,z3,r13)                    (y3,z3,r33)
        //                (ymid,zmid,rmid)
        //  (yy1,z1,r11)                    (y3,z1,r31)
          glTexCoord1f(r11); glVertex3f(x1, yy1, z1);
          glTexCoord1f(r31); glVertex3f(x3, y3, z1);
          glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
          glTexCoord1f(r31); glVertex3f(x3, y3, z1);
          glTexCoord1f(r33); glVertex3f(x3, y3, z3);
          glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
          glTexCoord1f(r33); glVertex3f(x3, y3, z3);
          glTexCoord1f(r13); glVertex3f(x1, yy1, z3);
          glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
          glTexCoord1f(r13); glVertex3f(x1, yy1, z3);
          glTexCoord1f(r11); glVertex3f(x1, yy1, z1);
          glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
        }
      }
    }
  }
  if(visy_all==1){
    int jrow, jrow2;
    int nx, ny;

    nx = sd->nslicei;
    ny = sd->nslicej;

    for(jrow2 = 1;jrow2<nx+ny-2;jrow2++){
      int iimin;
      int ijk;

      jrow = jrow2;
      if(direction<0)jrow = nx + ny - 2 - jrow2;
      iimin = 0;
      if(jrow>=ny)iimin = jrow-(ny-1);
      // i + j = jrow
      for(i = iimin; i<nx-1; i++){
        float xmid, ymid, zmid, rmid;

        j = jrow-i;
        if(j<=0)break;

        n = VALIJK(i, j, -1);
        n2 = VALIJK(i+1, j-1, -1);

        // val(i,j,k) = di*nj*nk + dj*nk + dk
        x1 = xplt[i];
        x3 = xplt[i+1];
        xmid = (x1 + x3)/2.0;
        yy1 = yplt[j];
        y3 = yplt[j-1];
        ymid = (yy1 + y3)/2.0;
        ijk = IJK(i,j,-1);

        for(k = sd->ks1; k<sd->ks2; k++){
          n++; n2++; ijk+=nxy;
          if(show_slice_in_obst==ONLY_IN_SOLID && c_iblank_y!=NULL&&c_iblank_y[ijk]==GASGAS)continue;
          if(show_slice_in_obst==ONLY_IN_GAS   && c_iblank_y!=NULL&&c_iblank_y[ijk]!=GASGAS)continue;
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[ijk]==EMBED_YES)continue;
          r11 = (float)sd->iqsliceframe[n] / 255.0;
          r31 = (float)sd->iqsliceframe[n2] / 255.0;
          r13 = (float)sd->iqsliceframe[n + 1] / 255.0;
          r33 = (float)sd->iqsliceframe[n2 + 1] / 255.0;
          rmid = (r11 + r31 + r13 + r33) / 4.0;

          z1 = zplt[k];
          z3 = zplt[k + 1];
          zmid = (z1 + z3) / 2.0;

        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,z3,r13)                    (x3,z3,r33)
        //                (xmid,zmid,rmid)
        //  (x1,z1,r11)                    (x3,z1,r31)
          glTexCoord1f(r11); glVertex3f(x1, yy1, z1);
          glTexCoord1f(r31); glVertex3f(x3, y3, z1);
          glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
          glTexCoord1f(r31); glVertex3f(x3, y3, z1);
          glTexCoord1f(r33); glVertex3f(x3, y3, z3);
          glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
          glTexCoord1f(r33); glVertex3f(x3, y3, z3);
          glTexCoord1f(r13); glVertex3f(x1, yy1, z3);
          glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
          glTexCoord1f(r13); glVertex3f(x1, yy1, z3);
          glTexCoord1f(r11); glVertex3f(x1, yy1, z1);
          glTexCoord1f(rmid); glVertex3f(xmid, ymid, zmid);
        }
      }
    }
  }
  glEnd();
  glDisable(GL_TEXTURE_1D);
  if(use_transparency_data == 1)TransparentOff();
  if(cullfaces == 1)glEnable(GL_CULL_FACE);
}

/* ------------------ DrawVolSliceTexture ------------------------ */

void DrawVolSliceTexture(const slicedata *sd){
  int i, j, k, n, n2;
  float r11, r31, r13, r33;
  float constval, x1, x3, yy1, y3, z1, z3;

  float *xplt, *yplt, *zplt;
  int ibar, jbar;
  int nx, ny, nxy;
  char *c_iblank_x, *c_iblank_y, *c_iblank_z;
  char *iblank_embed;
  int plotx, ploty, plotz;

  meshdata *meshi;

  if(sd->volslice == 1 && visx_all == 0 && visy_all == 0 && visz_all == 0)return;
  meshi = meshinfo + sd->blocknumber;

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  if(sd->volslice == 1){
    plotx = meshi->iplotx_all[iplotx_all];
    ploty = meshi->iploty_all[iploty_all];
    plotz = meshi->iplotz_all[iplotz_all];
  }
  else{
    plotx = sd->is1;
    ploty = sd->js1;
    plotz = sd->ks1;
  }
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  c_iblank_x = meshi->c_iblank_x;
  c_iblank_y = meshi->c_iblank_y;
  c_iblank_z = meshi->c_iblank_z;
  iblank_embed = meshi->c_iblank_embed;
  nx = ibar + 1;
  ny = jbar + 1;
  nxy = nx*ny;

  if(cullfaces == 1)glDisable(GL_CULL_FACE);
  if(use_transparency_data == 1)TransparentOn();
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, texture_slice_colorbar_id);

  if((sd->volslice == 1 && plotx >= 0 && visx_all == 1) || (sd->volslice == 0 && sd->idir == XDIR)){
    int maxj;

    constval = xplt[plotx] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glBegin(GL_TRIANGLES);
    maxj = sd->js2;
    if(sd->js1 + 1>maxj){
      maxj = sd->js1 + 1;
    }
    for(j = sd->js1; j<maxj; j++){
      float ymid;

      n = (j - sd->js1)*sd->nslicek - 1;
      n += (plotx - sd->is1)*sd->nslicej*sd->nslicek;
      n2 = n + sd->nslicek;
      yy1 = yplt[j];
      y3 = yplt[j + 1];
      ymid = (yy1 + y3) / 2.0;

      // val(i,j,k) = di*nj*nk + dj*nk + dk
      for(k = sd->ks1; k<sd->ks2; k++){
        float rmid, zmid;

        n++; n2++;
        if(show_slice_in_obst == ONLY_IN_SOLID && c_iblank_x != NULL&&c_iblank_x[IJK(plotx, j, k)] == GASGAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && c_iblank_x != NULL&&c_iblank_x[IJK(plotx, j, k)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(plotx, j, k)] == EMBED_YES)continue;
        r11 = (float)sd->iqsliceframe[n] / 255.0;
        r31 = (float)sd->iqsliceframe[n2] / 255.0;
        r13 = (float)sd->iqsliceframe[n + 1] / 255.0;
        r33 = (float)sd->iqsliceframe[n2 + 1] / 255.0;
        rmid = (r11 + r31 + r13 + r33) / 4.0;

        z1 = zplt[k];
        z3 = zplt[k + 1];
        zmid = (z1 + z3) / 2.0;

        /*
        n+1 (y1,z3) n2+1 (y3,z3)
        n (y1,z1)     n2 (y3,z1)
        */
        //  (yy1,z3,r13)                    (y3,z3,r33)
        //                (ymid,zmid,rmid)
        //  (yy1,z1,r11)                    (y3,z1,r31)
        glTexCoord1f(r11); glVertex3f(constval, yy1, z1);
        glTexCoord1f(r31); glVertex3f(constval, y3, z1);
        glTexCoord1f(rmid); glVertex3f(constval, ymid, zmid);
        glTexCoord1f(r31); glVertex3f(constval, y3, z1);
        glTexCoord1f(r33); glVertex3f(constval, y3, z3);
        glTexCoord1f(rmid); glVertex3f(constval, ymid, zmid);
        glTexCoord1f(r33); glVertex3f(constval, y3, z3);
        glTexCoord1f(r13); glVertex3f(constval, yy1, z3);
        glTexCoord1f(rmid); glVertex3f(constval, ymid, zmid);
        glTexCoord1f(r13); glVertex3f(constval, yy1, z3);
        glTexCoord1f(r11); glVertex3f(constval, yy1, z1);
        glTexCoord1f(rmid); glVertex3f(constval, ymid, zmid);
      }
    }
    glEnd();
  }
  if((sd->volslice == 1 && ploty >= 0 && visy_all == 1) || (sd->volslice == 0 && sd->idir == YDIR)){
    int maxi;

    constval = yplt[ploty]+offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glBegin(GL_TRIANGLES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1>maxi){
      maxi = sd->is1 + 1;
    }
    for(i = sd->is1; i<maxi; i++){
      float xmid;

      n = (i - sd->is1)*sd->nslicej*sd->nslicek - 1;
      n += (ploty - sd->js1)*sd->nslicek;
      n2 = n + sd->nslicej*sd->nslicek;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      xmid = (x1 + x3) / 2.0;

      for(k = sd->ks1; k<sd->ks2; k++){
        float rmid, zmid;

        n++; n2++;
        if(show_slice_in_obst == ONLY_IN_SOLID && c_iblank_y != NULL&&c_iblank_y[IJK(i, ploty, k)] == GASGAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && c_iblank_y != NULL&&c_iblank_y[IJK(i, ploty, k)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, ploty, k)] == EMBED_YES)continue;
        r11 = (float)sd->iqsliceframe[n] / 255.0;
        r31 = (float)sd->iqsliceframe[n2] / 255.0;
        r13 = (float)sd->iqsliceframe[n + 1] / 255.0;
        r33 = (float)sd->iqsliceframe[n2 + 1] / 255.0;
        rmid = (r11 + r31 + r13 + r33) / 4.0;

        z1 = zplt[k];
        z3 = zplt[k + 1];
        zmid = (z1 + z3) / 2.0;

        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,z3,r13)                    (x3,z3,r33)
        //                (xmid,zmid,rmid)
        //  (x1,z1,r11)                    (x3,z1,r31)
        glTexCoord1f(r11); glVertex3f(x1, constval, z1);
        glTexCoord1f(r31); glVertex3f(x3, constval, z1);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r31); glVertex3f(x3, constval, z1);
        glTexCoord1f(r33); glVertex3f(x3, constval, z3);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r33); glVertex3f(x3, constval, z3);
        glTexCoord1f(r13); glVertex3f(x1, constval, z3);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r13); glVertex3f(x1, constval, z3);
        glTexCoord1f(r11); glVertex3f(x1, constval, z1);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
      }
    }
    glEnd();
  }
  if((sd->volslice == 1 && plotz >= 0 && visz_all == 1) || (sd->volslice == 0 && sd->idir == ZDIR)){
    int maxi;

    constval = zplt[plotz] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glBegin(GL_TRIANGLES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1>maxi){
      maxi = sd->is1 + 1;
    }
    for(i = sd->is1; i<maxi; i++){
      float xmid;

      n = (i - sd->is1)*sd->nslicej*sd->nslicek - sd->nslicek;
      n += (plotz - sd->ks1);
      n2 = n + sd->nslicej*sd->nslicek;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      xmid = (x1 + x3) / 2.0;

      for(j = sd->js1; j<sd->js2; j++){
        float ymid, rmid;

        n += sd->nslicek;
        n2 += sd->nslicek;
        if(show_slice_in_obst == ONLY_IN_SOLID && c_iblank_z != NULL&&c_iblank_z[IJK(i, j, plotz)] == GASGAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && c_iblank_z != NULL&&c_iblank_z[IJK(i, j, plotz)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, j, plotz)] == EMBED_YES)continue;
        r11 = (float)sd->iqsliceframe[n] / 255.0;
        r31 = (float)sd->iqsliceframe[n2] / 255.0;
        r13 = (float)sd->iqsliceframe[n + sd->nslicek] / 255.0;
        r33 = (float)sd->iqsliceframe[n2 + sd->nslicek] / 255.0;
        rmid = (r11 + r31 + r13 + r33) / 4.0;

        yy1 = yplt[j];
        y3 = yplt[j + 1];
        ymid = (yy1 + y3) / 2.0;

        /*
        n+nk (x1,y3)   n2+nk (x3,y3)
        n (x1,y1)      n2 (x3,y1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,y3,r13)                    (x3,y3,r33)
        //                (xmid,ymid,rmid)
        //  (x1,yy1,r11)                    (x3,yy1,r31)
        glTexCoord1f(r11); glVertex3f(x1, yy1, constval);
        glTexCoord1f(r31); glVertex3f(x3, yy1, constval);
        glTexCoord1f(rmid); glVertex3f(xmid, ymid, constval);
        glTexCoord1f(r31); glVertex3f(x3, yy1, constval);
        glTexCoord1f(r33); glVertex3f(x3, y3, constval);
        glTexCoord1f(rmid); glVertex3f(xmid, ymid, constval);
        glTexCoord1f(r33); glVertex3f(x3, y3, constval);
        glTexCoord1f(r13); glVertex3f(x1, y3, constval);
        glTexCoord1f(rmid); glVertex3f(xmid, ymid, constval);
        glTexCoord1f(r13); glVertex3f(x1, y3, constval);
        glTexCoord1f(r11); glVertex3f(x1, yy1, constval);
        glTexCoord1f(rmid); glVertex3f(xmid, ymid, constval);
      }
    }
    glEnd();
  }
  glDisable(GL_TEXTURE_1D);
  if(use_transparency_data == 1)TransparentOff();
  if(cullfaces == 1)glEnable(GL_CULL_FACE);
}

/* ------------------ DrawVolAllSlicesDiag ------------------------ */

void DrawVolAllSlicesDiag(const slicedata *sd, int direction){
  int i, j, k, n, n2;
  int i11, i31, i13, i33;
  float x1, x3, yy1, y3, z1, z3;

  float *xplt, *yplt, *zplt;
  int ibar, jbar;
  int nx, ny, nxy;
  char *iblank_x, *iblank_y;
  char *iblank_embed;

  meshdata *meshi;

  float *rgb_ptr;

  rgb_ptr = rgb_slice;

  meshi = meshinfo + sd->blocknumber;

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  iblank_x = meshi->c_iblank_x;
  iblank_y = meshi->c_iblank_y;
  iblank_embed = meshi->c_iblank_embed;
  nx = ibar + 1;
  ny = jbar + 1;
  nxy = nx*ny;

  if(cullfaces == 1)glDisable(GL_CULL_FACE);

  if(use_transparency_data == 1)TransparentOn();
  if(visx_all == 1){
    int icol, icol2;
    int nx, ny;

    nx = sd->nslicei;
    ny = sd->nslicej;

    glBegin(GL_TRIANGLES);
    for(icol2=1;icol2<nx+ny-2;icol2++){
      int iimin;

      icol = icol2;
      if(direction<0)icol = nx + ny - 2 - icol2;
      iimin = nx-icol - 1;
      if(icol>=nx-1)iimin = 0;
      // icol + i - j = nx - 1
      for(i = iimin; i<nx-1; i++){
      j = icol + i + 1 - nx;
      if(j>ny-2)break;

      n = VALIJK(i,j,-1);
      n2 = VALIJK(i+1,j+1,-1);

      // val(i,j,k) = di*nj*nk + dj*nk + dk
      x1 = xplt[i];
      x3 = xplt[i + 1];
      yy1 = yplt[j];
      y3 = yplt[j+1];

      for(k = sd->ks1; k<sd->ks2; k++){
        n++; n2++;
        if(show_slice_in_obst == ONLY_IN_SOLID && iblank_y != NULL&&iblank_x[IJK(i, j, k)] == GASGAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && iblank_y != NULL&&iblank_x[IJK(i, j, k)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(sd->is1, j, k)] == EMBED_YES)continue;
        i11 = 4 * sd->iqsliceframe[n];
        i31 = 4 * sd->iqsliceframe[n2];
        i13 = 4 * sd->iqsliceframe[n + 1];
        i33 = 4 * sd->iqsliceframe[n2 + 1];
        z1 = zplt[k];
        z3 = zplt[k + 1];
        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        if(ABS(i11 - i33)<ABS(i13 - i31)){
          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, z1);
          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, y3, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, z3);

          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, z3);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, yy1, z3);
        }
        else{
          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, z1);
          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, y3, z1);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, yy1, z3);

          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, y3, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, z3);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, yy1, z3);
        }
      }
    }
    }
    glEnd();
  }
  if(visy_all == 1){
    int jrow, jrow2;
    int nx, ny;

    nx = sd->nslicei;
    ny = sd->nslicej;

    glBegin(GL_TRIANGLES);
    for(jrow2=1;jrow2<nx+ny-2;jrow2++){
      int iimin;

      jrow = jrow2;
      if(direction<0)jrow = nx + ny - 2 - jrow2;
      iimin = 0;
      if(jrow>=ny)iimin = jrow - (ny-1);
      // i + j = jrow
      for(i = iimin; i<nx-1; i++){
      j = jrow - i;
      if(j<=0)break;

      n = VALIJK(i,j,-1);
      n2 = VALIJK(i+1,j-1,-1);

      // val(i,j,k) = di*nj*nk + dj*nk + dk
      x1 = xplt[i];
      x3 = xplt[i + 1];
      yy1 = yplt[j];
      y3 = yplt[j-1];

      for(k = sd->ks1; k<sd->ks2; k++){
        n++; n2++;
        if(show_slice_in_obst == ONLY_IN_SOLID && iblank_y != NULL&&iblank_y[IJK(i, j, k)] == GASGAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && iblank_y != NULL&&iblank_y[IJK(i, j, k)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, sd->js1, k)] == EMBED_YES)continue;
        i11 = 4 * sd->iqsliceframe[n];
        i31 = 4 * sd->iqsliceframe[n2];
        i13 = 4 * sd->iqsliceframe[n + 1];
        i33 = 4 * sd->iqsliceframe[n2 + 1];
        z1 = zplt[k];
        z3 = zplt[k + 1];
        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        if(ABS(i11 - i33)<ABS(i13 - i31)){
          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, z1);
          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, y3, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, z3);

          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, z3);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, yy1, z3);
        }
        else{
          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, z1);
          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, y3, z1);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, yy1, z3);

          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, y3, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, z3);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, yy1, z3);
        }
      }
    }
    }
    glEnd();
  }
  if(use_transparency_data == 1)TransparentOff();
  if(cullfaces == 1)glEnable(GL_CULL_FACE);

}

/* ------------------ DrawVolSlice ------------------------ */

void DrawVolSlice(const slicedata *sd){
  int i, j, k, n, n2;
  int i11, i31, i13, i33;
  float constval, x1, x3, yy1, y3, z1, z3;

  float *xplt, *yplt, *zplt;
  int plotx, ploty, plotz;
  int ibar, jbar;
  int nx, ny, nxy;
  char *iblank_x, *iblank_y, *iblank_z;
  char *iblank_embed;

  meshdata *meshi;

  float *rgb_ptr;

  rgb_ptr = rgb_slice;

  meshi = meshinfo + sd->blocknumber;

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  if(sd->volslice == 1){
    plotx = meshi->iplotx_all[iplotx_all];
    ploty = meshi->iploty_all[iploty_all];
    plotz = meshi->iplotz_all[iplotz_all];
  }
  else{
    plotx = sd->is1;
    ploty = sd->js1;
    plotz = sd->ks1;
  }
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  iblank_x = meshi->c_iblank_x;
  iblank_y = meshi->c_iblank_y;
  iblank_z = meshi->c_iblank_z;
  iblank_embed = meshi->c_iblank_embed;
  nx = ibar + 1;
  ny = jbar + 1;
  nxy = nx*ny;

  if(cullfaces == 1)glDisable(GL_CULL_FACE);

  if(use_transparency_data == 1)TransparentOn();
  if((sd->volslice == 1 && plotx >= 0 && visx_all == 1) || (sd->volslice == 0 && sd->idir == XDIR)){
    int maxj;
    float *opacity_adjustments;

    opacity_adjustments = meshi->opacity_adjustments;
    constval = xplt[plotx]+offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glBegin(GL_TRIANGLES);
    maxj = MAX(sd->js1 + 1, sd->js2);
    for(j = sd->js1; j<maxj; j++){
      n = (j - sd->js1)*sd->nslicek - 1;
      n += (plotx - sd->is1)*sd->nslicej*sd->nslicek;
      n2 = n + sd->nslicek;
      yy1 = yplt[j];
      y3 = yplt[j + 1];
      // val(i,j,k) = di*nj*nk + dj*nk + dk
      for(k = sd->ks1; k<sd->ks2; k++){
        float *rgb11, *rgb13, *rgb33, *rgb31;
        float  alpha11, alpha13, alpha33, alpha31;
        int ijk;

        n++; n2++;
        if(show_slice_in_obst == ONLY_IN_SOLID && iblank_x != NULL&&iblank_x[IJK(plotx, j, k)] == GASGAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && iblank_x != NULL&&iblank_x[IJK(plotx, j, k)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(plotx, j, k)] == EMBED_YES)continue;
        i11 = 4 * sd->iqsliceframe[n];
        i31 = 4 * sd->iqsliceframe[n2];
        i13 = 4 * sd->iqsliceframe[n + 1];
        i33 = 4 * sd->iqsliceframe[n2 + 1];
        z1 = zplt[k];
        z3 = zplt[k + 1];
        /*
        n+1 (y1,z3)     n2+1 (y3,z3)
        n   (y1,z1)     n2   (y3,z1)
        */

        rgb11 = rgb_ptr + i11;
        rgb13 = rgb_ptr + i13;
        rgb33 = rgb_ptr + i33;
        rgb31 = rgb_ptr + i31;
        if(slice_opacity_adjustment==1){
          float opacity;
#define POW 0.5
#define OPMIN 0.1
          ijk = IJK(plotx, j, k);
          opacity = CLAMP(transparent_level*pow((float)i11/1024.0,POW),0.0,1.0);
          if(opacity<OPMIN)opacity = 0.0;
          alpha11 = 1.0-pow(1.0-opacity,opacity_adjustments[ijk]);

          ijk = IJK(plotx,j+1,k);
          opacity = CLAMP(transparent_level*pow((float)i13/1024.0,POW),0.0,1.0);
          if(opacity<OPMIN)opacity = 0.0;
          alpha13 = 1.0-pow(1.0-opacity,opacity_adjustments[ijk]);

          ijk = IJK(plotx,j+1,k+1);
          opacity = CLAMP(transparent_level*pow((float)i33/1024.0,POW),0.0,1.0);
          if(opacity<OPMIN)opacity = 0.0;
          alpha33 = 1.0-pow(1.0-opacity,opacity_adjustments[ijk]);

          ijk = IJK(plotx,j,k+1);
          opacity = CLAMP(transparent_level*pow((float)i31/1024.0,POW),0.0,1.0);
          if(opacity<OPMIN)opacity = 0.0;
          alpha31 = 1.0-pow(1.0-opacity,opacity_adjustments[ijk]);
        }
        else{
          alpha11 = rgb11[3];
          alpha13 = rgb13[3];
          alpha33 = rgb33[3];
          alpha31 = rgb31[3];
        }
        if(ABS(i11 - i33)<ABS(i13 - i31)){
          glColor4f(rgb11[0],rgb11[1],rgb11[2],alpha11); glVertex3f(constval, yy1, z1);
          glColor4f(rgb31[0],rgb31[1],rgb31[2],alpha31); glVertex3f(constval, y3, z1);
          glColor4f(rgb33[0],rgb33[1],rgb33[2],alpha33); glVertex3f(constval, y3, z3);

          glColor4f(rgb11[0],rgb11[1],rgb11[2],alpha11); glVertex3f(constval, yy1, z1);
          glColor4f(rgb33[0],rgb33[1],rgb33[2],alpha33); glVertex3f(constval, y3, z3);
          glColor4f(rgb13[0],rgb13[1],rgb13[2],alpha13); glVertex3f(constval, yy1, z3);
        }
        else{
          glColor4f(rgb11[0],rgb11[1],rgb11[2],alpha11); glVertex3f(constval, yy1, z1);
          glColor4f(rgb31[0],rgb31[1],rgb31[2],alpha31); glVertex3f(constval, y3, z1);
          glColor4f(rgb13[0],rgb13[1],rgb13[2],alpha13); glVertex3f(constval, yy1, z3);

          glColor4f(rgb31[0],rgb31[1],rgb31[2],alpha31); glVertex3f(constval, y3, z1);
          glColor4f(rgb33[0],rgb33[1],rgb33[2],alpha33); glVertex3f(constval, y3, z3);
          glColor4f(rgb13[0],rgb13[1],rgb13[2],alpha13); glVertex3f(constval, yy1, z3);
        }
      }
    }
    glEnd();
  }
  if((sd->volslice == 1 && ploty >= 0 && visy_all == 1) || (sd->volslice == 0 && sd->idir == YDIR)){
    int maxi;

    constval = yplt[ploty] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glBegin(GL_TRIANGLES);
    maxi = MAX(sd->is1 + sd->nslicei - 1, sd->is1 + 1);
    for(i = sd->is1; i<maxi; i++){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - 1;
      n += (ploty - sd->js1)*sd->nslicek;
      n2 = n + sd->nslicej*sd->nslicek;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      for(k = sd->ks1; k<sd->ks2; k++){
        n++; n2++;
        if(show_slice_in_obst == ONLY_IN_SOLID && iblank_y != NULL&&iblank_y[IJK(i, ploty, k)] == GASGAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && iblank_y != NULL&&iblank_y[IJK(i, ploty, k)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, sd->js1, k)] == EMBED_YES)continue;
        i11 = 4 * sd->iqsliceframe[n];
        i31 = 4 * sd->iqsliceframe[n2];
        i13 = 4 * sd->iqsliceframe[n + 1];
        i33 = 4 * sd->iqsliceframe[n2 + 1];
        z1 = zplt[k];
        z3 = zplt[k + 1];
        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        if(ABS(i11 - i33)<ABS(i13 - i31)){
          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, constval, z1);
          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, constval, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, constval, z3);

          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, constval, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, constval, z3);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, constval, z3);
        }
        else{
          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, constval, z1);
          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, constval, z1);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, constval, z3);

          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, constval, z1);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, constval, z3);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, constval, z3);
        }
      }
    }
    glEnd();
  }
  // i*nj*nk + j*nk + k
  if((sd->volslice == 1 && plotz >= 0 && visz_all == 1) || (sd->volslice == 0 && sd->idir == ZDIR)){
    int maxi;

    constval = zplt[plotz] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glBegin(GL_TRIANGLES);
    maxi = MAX(sd->is1 + sd->nslicei - 1, sd->is1 + 1);
    for(i = sd->is1; i<maxi; i++){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - sd->nslicek;
      n += (plotz - sd->ks1);
      n2 = n + sd->nslicej*sd->nslicek;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      for(j = sd->js1; j<sd->js2; j++){
        n += sd->nslicek;
        n2 += sd->nslicek;
        if(show_slice_in_obst == ONLY_IN_SOLID && iblank_z != NULL&&iblank_z[IJK(i, j, plotz)] == GASGAS)continue;
        if(show_slice_in_obst == ONLY_IN_GAS   && iblank_z != NULL&&iblank_z[IJK(i, j, plotz)] != GASGAS)continue;
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, j, plotz)] == EMBED_YES)continue;
        i11 = 4 * sd->iqsliceframe[n];
        i31 = 4 * sd->iqsliceframe[n2];
        i13 = 4 * sd->iqsliceframe[n + sd->nslicek];
        i33 = 4 * sd->iqsliceframe[n2 + sd->nslicek];
        yy1 = yplt[j];
        y3 = yplt[j + 1];
        /*
        n+nk (x1,y3)   n2+nk (x3,y3)
        n (x1,y1)      n2 (x3,y1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        if(ABS(i11 - i33)<ABS(i13 - i31)){
          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, constval);
          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, yy1, constval);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, constval);

          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, constval);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, constval);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, y3, constval);
        }
        else{
          glColor4fv(&rgb_ptr[i11]); glVertex3f(x1, yy1, constval);
          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, yy1, constval);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, y3, constval);

          glColor4fv(&rgb_ptr[i31]); glVertex3f(x3, yy1, constval);
          glColor4fv(&rgb_ptr[i33]); glVertex3f(x3, y3, constval);
          glColor4fv(&rgb_ptr[i13]); glVertex3f(x1, y3, constval);
        }
      }
    }
    glEnd();
  }
  if(use_transparency_data == 1)TransparentOff();
  if(cullfaces == 1)glEnable(GL_CULL_FACE);

}

/* ------------------ ComputeOpacityCorrections ------------------------ */

void ComputeOpacityCorrections(meshdata *meshi, float *xyz0, float *normal){
  int k, nx, ny, nz;
  float *opacity_adjustments;
  float *xplt, *yplt, *zplt;

  nx = meshi->ibar+1;
  ny = meshi->jbar+1;
  nz = meshi->kbar+1;
  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  opacity_adjustments = meshi->opacity_adjustments;
  if(opacity_adjustments==NULL){
    NewMemory((void **)&opacity_adjustments, nx*ny*nz*sizeof(float));
    meshi->opacity_adjustments = opacity_adjustments;
  }
  for(k = 0;k<nz;k++){
    int j;
    float xyz[3];

    xyz[2] = zplt[k]-xyz0[2];

    for(j = 0;j<ny;j++){
      int i;

      xyz[1] = yplt[j]-xyz0[1];

      for(i = 0;i<nx;i++){
        float dist, xyzproj;

        xyz[0] = xplt[i]-xyz0[0];

        dist = NORM3(xyz);
        xyzproj = DOT3(xyz, normal);
        // alphahat = 1.0 - (1.0-alpha)^(xhat/x)
        *opacity_adjustments++ = dist/ABS(xyzproj);
      }
    }
  }
}

/* ------------------ CompareLoadedSliceList ------------------------ */

int CompareLoadedSliceList( const void *arg1, const void *arg2 ){
  slicedata *slicei, *slicej;
  float position_i, position_j;
  int dir;

  slicei = sliceinfo + *(int *)arg1;
  slicej = sliceinfo + *(int *)arg2;
  if(slicei->idir < slicej->idir)return -1;
  if(slicei->idir > slicej->idir)return 1;
  dir = slicei->idir - 1;
  position_i = slicei->xyz_min[dir];
  position_j = slicej->xyz_min[dir];
  if(eye_position_fds[dir] < position_i){
    if(position_i > position_j)return -1;
    if(position_i < position_j)return 1;
  }
  else{
    if(position_i > position_j)return 1;
    if(position_i < position_j)return -1;
  }
  return 0;
}

/* ------------------ SortLoadedSliceList ------------------------ */

void SortLoadedSliceList(void){
  int i;

  for(i=0;i<nslice_loaded;i++){
    slice_sorted_loaded_list[i] = slice_loaded_list[i];
  }
  if(sort_slices==1){
    qsort((int *)slice_sorted_loaded_list,nslice_loaded,sizeof(int), CompareLoadedSliceList);
  }
}

/* ------------------ DrawSliceFrame ------------------------ */

void DrawSliceFrame(){
  int ii;
  int jjj, nslicemax, blend_mode;

  SortLoadedSliceList();

  for(ii=0;ii<nslice_loaded;ii++){
    slicedata *sd;
    int i;
    meshdata *slicemesh;
    int orien, direction;
    int jjjj;

    i=slice_sorted_loaded_list[ii];
    sd = sliceinfo + i;
    if(sd->slicefile_labelindex!=slicefile_labelindex)continue;
    if(sd->display==0){
      if(showvslice==0)continue;
      if(sd->slicefile_type==SLICE_NODE_CENTER&&show_node_slices_and_vectors==0)continue;
      if(sd->slicefile_type==SLICE_CELL_CENTER||sd->slicefile_type==SLICE_FACE_CENTER){
        if(show_cell_slices_and_vectors==0)continue;
      }
    }
    if(sd->times[0]>global_times[itimes])continue;
    if(sd->slicefile_type != SLICE_GEOM){
      if(sd->compression_type==COMPRESSED_ZLIB){
        UncompressSliceDataFrame(sd,sd->itime);
        sd->iqsliceframe=sd->slicecomplevel;
      }
      else{
        sd->iqsliceframe = sd->slicelevel + sd->itime*sd->nsliceijk;
        sd->qslice = sd->qslicedata + sd->itime*sd->nsliceijk;
      }
      sd->qsliceframe=NULL;
#ifdef pp_MEMDEBUG
      if(sd->compression_type==UNCOMPRESSED){
        ASSERT(ValidPointer(sd->qslicedata,sizeof(float)*sd->nslicetotal));
      }
#endif
      if(sd->qslicedata!= NULL)sd->qsliceframe = sd->qslicedata + sd->itime*sd->nsliceijk;
    }
    orien = 0;
    direction = 1;
    blend_mode = 0;
    if(sd->volslice==1&&showall_3dslices==1){
      float slice_normal[3];

      visx_all = 0;
      visy_all = 0;
      visz_all = 0;
      slice_normal[0] = 0.0;
      slice_normal[1] = 0.0;
      slice_normal[2] = 0.0;
      slicemesh = meshinfo+sd->blocknumber;
      if(show_sort_labels==1){
        float *mid;
        char label[100];

        mid = slicemesh->boxmiddle_scaled;
        sprintf(label, "%i %f", ii,slicemesh->eyedist);
        Output3Text(foregroundcolor, mid[0], mid[1], mid[2], label);
      }
      if(slicemesh->smokedir<0)direction = -1;
      switch (ABS(slicemesh->smokedir)){
      case 4:  // -45 slope slices
        visy_all = 1;
        nslicemax = nploty_list;
        orien = 1;
        slice_normal[0] = direction*slicemesh->dy;
        slice_normal[1] = direction*slicemesh->dx;
        break;
      case 5:  // 45 slope slices
        visx_all = 1;
        nslicemax = nplotx_list;
        orien = 1;
        slice_normal[0] = -direction*slicemesh->dy;
        slice_normal[1] = direction*slicemesh->dx;
        break;
        // x direction
      case 1:
      case 8:
      case 9:
      visx_all = 1;
      nslicemax = nplotx_list;
      slice_normal[0] = direction;
      break;
      // y direction
      case 2:
      case 6:
      case 7:
      visy_all = 1;
      nslicemax = nploty_list;
      slice_normal[1] = direction;
      break;
      case 3:
      visz_all = 1;
      nslicemax = nplotz_list;
      slice_normal[2] = direction;
      break;
      }
      nslicemax = MAX(nslicemax, 1);
      if(slices3d_max_blending==1){
        blend_mode=1;
        glBlendEquation(GL_MAX);
      }
      if(slice_opacity_adjustment==1)ComputeOpacityCorrections(slicemesh, eye_position_fds, slice_normal);
    }
    else{
      blend_mode = 0;
      nslicemax = 1;
    }
    for(jjjj = 0;jjjj<nslicemax;jjjj++){
      jjj = jjjj;
      if(direction<0)jjj = nslicemax - 1 - jjjj;
      if(sd->volslice==1&&showall_3dslices==1){
        if(visx_all==1&&plotx_list!=NULL&&nplotx_list>0){
          iplotx_all = plotx_list[jjj];
        }
        if(visy_all==1&&ploty_list!=NULL&&nploty_list>0){
          iploty_all = ploty_list[jjj];
        }
        if(visz_all==1&&plotz_list!=NULL&&nplotz_list>0){
          iplotz_all = plotz_list[jjj];
        }
      }

      if(vis_slice_contours==1&&sd->line_contours!=NULL){
        if(slice_contour_type==SLICE_LINE_CONTOUR){
          DrawLineContours(sd->line_contours+sd->itime, slice_line_contour_width);
          SNIFF_ERRORS("after DrawLineContours");
        }
        else{
          DrawContours(sd->line_contours+sd->itime);
          SNIFF_ERRORS("after DrawContours");
        }
        continue;
      }

      switch(sd->slicefile_type){
      case SLICE_NODE_CENTER:
        if(orien==0){
          if(usetexturebar!=0){
            DrawVolSliceTexture(sd);
            SNIFF_ERRORS("after DrawVolSliceTexture");
          }
          else{
            DrawVolSlice(sd);
            SNIFF_ERRORS("after DrawVolSlice");
          }
        }
#ifdef pp_GPU
        if(sd->volslice==1&&(vis_gslice_data==1)){
          if(usegpu==1){
            Load3DSliceShaders();
            SNIFF_ERRORS("after Load3DSliceShaders");
            DrawGSliceDataGpu(sd);
            SNIFF_ERRORS("after DrawGSliceDataGpu");
            UnLoadShaders();
            SNIFF_ERRORS("after UnLoad3DSliceShaders");
          }
          else{
            DrawGSliceData(sd);
            SNIFF_ERRORS("after DrawGSliceData");
          }
        }
#else
        if(sd->volslice==1&&vis_gslice_data==1){
          DrawGSliceData(sd);
        }
#endif
        break;
      case SLICE_CELL_CENTER:
        DrawVolSliceCellFaceCenter(sd, SLICE_CELL_CENTER);
        SNIFF_ERRORS("after DrawVolSliceCellFaceCenter SLICE_CELL_CENTER");
        break;
      case SLICE_FACE_CENTER:
        DrawVolSliceCellFaceCenter(sd, SLICE_FACE_CENTER);
        SNIFF_ERRORS("after DrawVolSliceCellFaceCenter SLICE_FACE_CENTER");
        break;
      case SLICE_TERRAIN:
        DrawVolSliceTerrain(sd);
        SNIFF_ERRORS("after DrawVolSliceTerrain");
        break;
      case SLICE_GEOM:
        DrawGeomData(DRAW_TRANSPARENT, sd->patchgeom, GEOM_STATIC);
        DrawGeomData(DRAW_TRANSPARENT, sd->patchgeom, GEOM_DYNAMIC);
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
    }
    if(orien==1&&sd->slicefile_type==SLICE_NODE_CENTER){
      if(usetexturebar!=0){
        DrawVolAllSlicesTextureDiag(sd,direction);
        SNIFF_ERRORS("after DrawVolAllSlicesTextureDiag");
      }
      else{
        DrawVolAllSlicesDiag(sd,direction);
        SNIFF_ERRORS("after DrawVolAllSlicesDiag");
      }
    }
    if(blend_mode==1){
      glBlendEquation(GL_FUNC_ADD);
    }
  }
  for (ii = 0; ii < npatchinfo; ii++) {
    patchdata *patchi;

    patchi = patchinfo + ii;
    if(patchi->boundary==0 && patchi->loaded == 1 && patchi->display == 1){
      DrawGeomData(DRAW_TRANSPARENT, patchi, GEOM_STATIC);
      DrawGeomData(DRAW_TRANSPARENT, patchi, GEOM_DYNAMIC);
    }
  }
}

/* ------------------ DrawVVolSliceCellCenter ------------------------ */

void DrawVVolSliceCellCenter(const vslicedata *vd){
  int i;
  float constval;
  slicedata *u, *v, *w, *sd;
  float vrange;
  meshdata *meshi;
  float *xplttemp, *yplttemp, *zplttemp;
  int plotx, ploty, plotz;

  sd = sliceinfo + vd->ival;
  meshi = meshinfo + sd->blocknumber;
  xplttemp = meshi->xplt;
  yplttemp = meshi->yplt;
  zplttemp = meshi->zplt;
  if(vd->volslice == 1){
    plotx = meshi->iplotx_all[iplotx_all];
    ploty = meshi->iploty_all[iploty_all];
    plotz = meshi->iplotz_all[iplotz_all];
  }
  else{
    plotx = sd->is1;
    ploty = sd->js1;
    plotz = sd->ks1;
  }

  vrange = velocity_range;
  if(vrange <= 0.0)vrange = 1.0;
  u = vd->u;
  v = vd->v;
  w = vd->w;
  if((vd->volslice == 1 && plotx > 0 && visx_all == 1) || (vd->volslice == 0 && sd->idir == XDIR)){
    int j;
    int maxj;
    float xhalf;

    if(plotx > 0){
      xhalf = (xplttemp[plotx] + xplttemp[plotx - 1]) / 2.0;
    }
    else{
      xhalf = xplttemp[plotx];
    }

    constval = xhalf + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    glColor4fv(foregroundcolor);
    maxj = sd->js2;
    if(sd->js1 + 1 > maxj)maxj = sd->js1 + 1;
    for(j = sd->js1; j < maxj; j++){
      float yy1, yhalf;
      int k;

      yy1 = yplttemp[j];
      if(j != maxj)yhalf = (yplttemp[j] + yplttemp[j + 1]) / 2.0;
      for(k = sd->ks1; k < sd->ks2; k++){
        float zhalf, z1;

        z1 = zplttemp[k];
        if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

        //       n = (j-sd->js1)*sd->nslicek - 1;
        //       n += (plotx-sd->is1)*sd->nslicej*sd->nslicek;


        if(k != sd->ks2){
          int index_v;
          float dy;

          index_v = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
          GET_VEC_DXYZ(v, dy, index_v);
          glVertex3f(constval, yy1 - dy, zhalf);
          glVertex3f(constval, yy1 + dy, zhalf);
        }
        if(j != maxj){
          int index_w;
          float dz;

          index_w = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1 + 1)*sd->nslicek + k - sd->ks1;
          GET_VEC_DXYZ(w, dz, index_w);
          glVertex3f(constval, yhalf, z1 - dz);
          glVertex3f(constval, yhalf, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:lines dir=1");

    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    glColor4fv(foregroundcolor);
    for(j = sd->js1; j < maxj; j++){
      float yy1, yhalf;
      int k;

      yy1 = yplttemp[j];
      if(j != maxj)yhalf = (yplttemp[j] + yplttemp[j + 1]) / 2.0;
      for(k = sd->ks1; k < sd->ks2; k++){
        float zhalf, z1;

        z1 = zplttemp[k];
        if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

        if(k != sd->ks2){
          int index_v;
          float dy;

          index_v = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1)*sd->nslicek + k - sd->ks1 + 1;
          GET_VEC_DXYZ(v, dy, index_v);
          glVertex3f(constval, yy1 + dy, zhalf);
        }
        if(j != maxj){
          int index_w;
          float dz;

          index_w = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1 + 1)*sd->nslicek + k - sd->ks1;
          GET_VEC_DXYZ(w, dz, index_w);
          glVertex3f(constval, yhalf, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:points dir=1");

    if(cell_center_text == 1){
      for(j = sd->js1; j < maxj; j++){
        float yy1, yhalf;
        int k;

        yy1 = yplttemp[j];
        if(j != maxj)yhalf = (yplttemp[j] + yplttemp[j + 1]) / 2.0;
        for(k = sd->ks1; k < sd->ks2; k++){
          float zhalf, z1;

          z1 = zplttemp[k];
          if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

          if(k != sd->ks2){
            int index_v;
            float val;

            index_v = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1)*sd->nslicek + k - sd->ks1 + 1;
            GET_VAL(v, val, index_v);
            Output3Val(constval, yy1, zhalf, val);
          }
          if(j != maxj){
            int index_w;
            float val;

            index_w = (plotx - sd->is1)*sd->nslicej*sd->nslicek;
            index_w += (j + 1 - sd->js1)*sd->nslicek + k - sd->ks1;
            GET_VAL(w, val, index_w);
            Output3Val(constval, yhalf, z1, val);
          }
        }
      }
    }
  }
  if((vd->volslice == 1 && ploty > 0 && visy_all == 1) || (vd->volslice == 0 && sd->idir == YDIR)){
    int maxi;
    float yhalf;

    if(ploty > 0){
      yhalf = (yplttemp[ploty] + yplttemp[ploty - 1]) / 2.0;
    }
    else{
      yhalf = yplttemp[ploty];
    }

    constval = yhalf + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1 > maxi)maxi = sd->is1 + 1;
    glBegin(GL_LINES);
    glColor4fv(foregroundcolor);
    for(i = sd->is1; i < maxi; i++){
      float x1, xhalf;
      int k;

      // n = (i-sd->is1)*sd->nslicej*sd->nslicek - 1;
      // n += (ploty-sd->js1)*sd->nslicek;

      x1 = xplttemp[i];
      if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;

      for(k = sd->ks1; k < sd->ks2; k++){
        float zhalf, z1;

        z1 = zplttemp[k];
        if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

        if(k + 1 != sd->nslicek){
          int index_u;
          float dx;

          index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
          GET_VEC_DXYZ(u, dx, index_u);
          glVertex3f(x1 - dx, constval, zhalf);
          glVertex3f(x1 + dx, constval, zhalf);
        }
        if(i + 1 != sd->nslicei){
          int index_w;
          float dz;

          index_w = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k - sd->ks1;
          GET_VEC_DXYZ(w, dz, index_w);
          glVertex3f(xhalf, constval, z1 - dz);
          glVertex3f(xhalf, constval, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:lines dir=2");
    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    glColor4fv(foregroundcolor);
    for(i = sd->is1; i < maxi; i++){
      float x1, xhalf;
      int k;

      // n = (i-sd->is1)*sd->nslicej*sd->nslicek - 1;
      // n += (ploty-sd->js1)*sd->nslicek;

      x1 = xplttemp[i];
      if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;

      for(k = sd->ks1; k < sd->ks2; k++){
        float zhalf, z1;

        z1 = zplttemp[k];
        if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

        if(k +1 != sd->nslicek){
          int index_u;
          float dx;

          index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
          GET_VEC_DXYZ(u, dx, index_u);
          glVertex3f(x1 + dx, constval, zhalf);
        }
        if(i + 1 != sd->nslicei){
          int index_w;
          float dz;

          index_w = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k - sd->ks1;
          GET_VEC_DXYZ(w, dz, index_w);
          glVertex3f(xhalf, constval, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:points dir=2");

    if(cell_center_text == 1){
      for(i = sd->is1; i < maxi; i++){
        float x1, xhalf;
        int k;

        // n = (i-sd->is1)*sd->nslicej*sd->nslicek - 1;
        // n += (ploty-sd->js1)*sd->nslicek;

        x1 = xplttemp[i];
        if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;

        for(k = sd->ks1; k < sd->ks2 + 1; k++){
          float zhalf, z1;

          z1 = zplttemp[k];
          if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

          if(k != sd->ks2){
            int index_u;
            float val;

            index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
            GET_VAL(u, val, index_u);
            Output3Val(x1, constval, zhalf, val);
          }
          if(i != sd->is2){
            int index_w;
            float val;

            index_w = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k - sd->ks1;
            GET_VAL(w, val, index_w);
            Output3Val(xhalf, constval, z1, val);
          }
        }
      }
    }
  }
  if((vd->volslice == 1 && plotz > 0 && visz_all == 1) || (vd->volslice == 0 && sd->idir == ZDIR)){
    int maxi;
    float zhalf;

    if(plotz > 0){
      zhalf = (zplttemp[plotz] + zplttemp[plotz - 1]) / 2.0;
    }
    else{
      zhalf = zplttemp[plotz];
    }

    constval = zhalf + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1 > maxi)maxi = sd->is1 + 1;
    glBegin(GL_LINES);
    glColor4fv(foregroundcolor);
    for(i = sd->is1; i < maxi; i++){
      float xhalf;
      float x1;
      int j;

      //      n = (i-sd->is1)*sd->nslicej*sd->nslicek - sd->nslicek;
      //      n += (plotz-sd->ks1);

      x1 = xplttemp[i];
      if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;
      for(j = sd->js1; j < sd->js2; j++){
        float yhalf;
        float yy1;

        yy1 = yplttemp[j];
        if(j + 1 != sd->nslicej)yhalf = (yplttemp[j] + yplttemp[j + 1]) / 2.0;

        if(j + 1 != sd->nslicej){
          int index_u;
          float dx;

          index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j + 1 - sd->js1)*sd->nslicek;
          GET_VEC_DXYZ(u, dx, index_u);
          glVertex3f(x1 - dx, yhalf, constval);
          glVertex3f(x1 + dx, yhalf, constval);
        }
        if(i + 1 != sd->nslicei){
          int index_v;
          float dy;

          index_v = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j - sd->js1)*sd->nslicek;
          GET_VEC_DXYZ(v, dy, index_v);
          glVertex3f(xhalf, yy1 - dy, constval);
          glVertex3f(xhalf, yy1 + dy, constval);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:lines dir=3");

    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    glColor4fv(foregroundcolor);
    for(i = sd->is1; i < maxi; i++){
      float xhalf;
      float x1;
      int j;

      //      n = (i-sd->is1)*sd->nslicej*sd->nslicek - sd->nslicek;
      //      n += (plotz-sd->ks1);

      x1 = xplttemp[i];
      if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;
      for(j = sd->js1; j < sd->js2; j++){
        float yhalf;
        float yy1;

        yy1 = yplttemp[j];
        if(j + 1 != sd->nslicej)yhalf = (yplttemp[j] + yplttemp[j + 1]) / 2.0;

        if(j + 1 != sd->nslicej){
          int index_u;
          float dx;

          index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j + 1 - sd->js1)*sd->nslicek;
          GET_VEC_DXYZ(u, dx, index_u);
          glVertex3f(x1 + dx, yhalf, constval);
        }
        if(i + 1 != sd->nslicei){
          int index_v;
          float dy;

          index_v = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j - sd->js1)*sd->nslicek;
          GET_VEC_DXYZ(v, dy, index_v);
          glVertex3f(xhalf, yy1 + dy, constval);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:points dir=3");

    if(cell_center_text == 1){
      for(i = sd->is1; i < maxi + 1; i++){
        float xhalf;
        float x1;
        int j;

        //      n = (i-sd->is1)*sd->nslicej*sd->nslicek - sd->nslicek;
        //      n += (plotz-sd->ks1);

        x1 = xplttemp[i];
        if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;
        for(j = sd->js1; j < sd->js2 + 1; j++){
          float yhalf;
          float yy1;

          yy1 = yplttemp[j];
          if(j + 1 != sd->nslicej)yhalf = (yplttemp[j] + yplttemp[j + 1]) / 2.0;

          if(j != sd->js2){
            int index_u;
            float val;

            index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j + 1 - sd->js1)*sd->nslicek;
            GET_VAL(u, val, index_u);
            Output3Val(x1, yhalf, constval, val);
          }
          if(i != sd->is2){
            int index_v;
            float val;

            index_v = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j - sd->js1)*sd->nslicek;
            GET_VAL(v, val, index_v);
            Output3Val(xhalf, yy1, constval, val);
          }
        }
      }
    }
  }
}

/* ------------------ DrawVVolSliceTerrain ------------------------ */

void DrawVVolSliceTerrain(const vslicedata *vd){
  int i, j, k, n;
  int i11;
  float constval, x1, yy1, z1;
  slicedata *u, *v, *w, *sd;
  float dx, dy, dz;
  float vrange;
  meshdata *meshi;
  float *xplttemp, *yplttemp, *zplttemp;
  char *iblank;
  int nx, ny, nxy;
  float *rgb_ptr;
  terraindata *terri;
  float *znode;
  int nycell;
  int plotx, ploty, plotz;

  sd = sliceinfo + vd->ival;
  meshi = meshinfo + sd->blocknumber;
  xplttemp = meshi->xplt;
  yplttemp = meshi->yplt;
  zplttemp = meshi->zplt;
  if(vd->volslice == 1){
    plotx = meshi->iplotx_all[iplotx_all];
    ploty = meshi->iploty_all[iploty_all];
    plotz = meshi->iplotz_all[iplotz_all];
  }
  else{
    plotx = sd->is1;
    ploty = sd->js1;
    plotz = sd->ks1;
  }
  iblank = meshi->c_iblank_node;
  nx = meshi->ibar + 1;
  ny = meshi->jbar + 1;
  nxy = nx*ny;

  terri = meshi->terrain;
  if(terri == NULL)return;
  znode = terri->znode_scaled;
  nycell = terri->ny;

  vrange = velocity_range;
  if(vrange <= 0.0)vrange = 1.0;
  u = vd->u;
  v = vd->v;
  w = vd->w;
  if((vd->volslice == 1 && plotx >= 0 && visx_all == 1) || (vd->volslice == 0 && sd->idir == XDIR)){
    int maxj;

    constval = xplttemp[plotx] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    maxj = sd->js2;
    if(sd->js1 + 1 > maxj)maxj = sd->js1 + 1;
    for(j = sd->js1; j < maxj + 1; j += vectorskip){
      n = (j - sd->js1)*sd->nslicek - vectorskip;
      n += (plotx - sd->is1)*sd->nslicej*sd->nslicek;
      yy1 = yplttemp[j];
      for(k = sd->ks1; k < sd->ks2 + 1; k += vectorskip){
        n += vectorskip;
        i11 = sd->iqsliceframe[n];
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          rgb_ptr = rgb_slice + 4 * i11;
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(iblank[IJK(plotx, j, k)] == GAS&&rgb_ptr[3] > 0.5){
          z1 = zplttemp[k];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(constval - dx, yy1 - dy, z1 - dz);
          glVertex3f(constval + dx, yy1 + dy, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceTerrain:lines dir=1");

    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    for(j = sd->js1; j < maxj + 1; j += vectorskip){
      n = (j - sd->js1)*sd->nslicek - vectorskip;
      n += (plotx - sd->is1)*sd->nslicej*sd->nslicek;
      yy1 = yplttemp[j];
      for(k = sd->ks1; k < sd->ks2 + 1; k += vectorskip){
        n += vectorskip;
        i11 = sd->iqsliceframe[n];
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          rgb_ptr = rgb_slice + 4 * i11;
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(iblank[IJK(plotx, j, k)] == GAS&&rgb_ptr[3] > 0.5){
          z1 = zplttemp[k];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(constval + dx, yy1 + dy, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceTerrain:points dir=1");
  }
  if((vd->volslice == 1 && ploty >= 0 && visy_all == 1) || (vd->volslice == 0 && sd->idir == YDIR)){
    int maxi;

    constval = yplttemp[ploty] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1 > maxi)maxi = sd->is1 + 1;
    for(i = sd->is1; i < maxi + 1; i += vectorskip){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - vectorskip;
      n += (ploty - sd->js1)*sd->nslicek;

      x1 = xplttemp[i];

      for(k = sd->ks1; k < sd->ks2 + 1; k += vectorskip){
        n += vectorskip;
        i11 = sd->iqsliceframe[n];
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          rgb_ptr = rgb_slice + 4 * i11;
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(iblank[IJK(i, ploty, k)] == GAS&&rgb_ptr[3] > 0.5){
          z1 = zplttemp[k];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(x1 - dx, constval - dy, z1 - dz);
          glVertex3f(x1 + dx, constval + dy, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceTerrain:lines dir=2");
    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    for(i = sd->is1; i < maxi + 1; i += vectorskip){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - vectorskip;
      n += (ploty - sd->js1)*sd->nslicek;

      x1 = xplttemp[i];

      for(k = sd->ks1; k < sd->ks2 + 1; k += vectorskip){
        n += vectorskip;
        i11 = sd->iqsliceframe[n];
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          rgb_ptr = rgb_slice + 4 * i11;
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(iblank[IJK(i, ploty, k)] == GAS&&rgb_ptr[3] > 0.5){
          z1 = zplttemp[k];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(x1 + dx, constval + dy, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceTerrain:points dir=2");
  }
  if((vd->volslice == 1 && plotz >= 0 && visz_all == 1) || (vd->volslice == 0 && sd->idir == ZDIR)){
    float zmax;
    int maxi;

    zmax = zplttemp[meshi->kbar];
    constval = zplttemp[plotz] + offset_slice*sd->sliceoffset - znode[0]+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1 > maxi)maxi = sd->is1 + 1;
    for(i = sd->is1; i < maxi + 1; i += vectorskip){
      x1 = xplttemp[i];
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - vectorskip*sd->nslicek;
      n += (plotz - sd->ks1);
      for(j = sd->js1; j < sd->js2 + 1; j += vectorskip){
        int n11;
        float z11;
        int ij2;

        n += vectorskip*sd->nslicek;
        ij2 = IJ2(i, j);
        z11 = MIN(zmax, constval + znode[ij2]);
        n11 = i*sd->nslicej*sd->nslicek + j*sd->nslicek;
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          rgb_ptr = rgb_slice + 4 *Interp3DSliceIndex(sd->iqsliceframe, meshi->zplt, meshi->kbar, n11, constval);
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(rgb_ptr[3] > 0.5){
          float f1, f2;
          int k1, k2;
          int n1, n2;

          GetZInterpFactors(meshi->zplt, meshi->kbar, z11, &k1, &k2, &f1, &f2);
          n1 = n11 + k1;
          n2 = n11 + k2;
          yy1 = yplttemp[j];
          GET_VEC_DXYZ_TERRAIN(u, dx);
          GET_VEC_DXYZ_TERRAIN(v, dy);
          GET_VEC_DXYZ_TERRAIN(w, dz);

          glColor4fv(rgb_ptr);
          glVertex3f(x1 - dx, yy1 - dy, z11 - dz);
          glVertex3f(x1 + dx, yy1 + dy, z11 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceTerrain:lines dir=3");

    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    for(i = sd->is1; i < maxi + 1; i += vectorskip){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - vectorskip*sd->nslicek;
      n += (plotz - sd->ks1);

      x1 = xplttemp[i];
      for(j = sd->js1; j < sd->js2 + 1; j += vectorskip){
        int n11;
        float z11;
        int ij2;

        n += vectorskip*sd->nslicek;

        ij2 = IJ2(i, j);
        z11 = MIN(constval + znode[ij2], zmax);
        n11 = i*sd->nslicej*sd->nslicek + j*sd->nslicek;
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          rgb_ptr = rgb_slice + 4 *Interp3DSliceIndex(sd->iqsliceframe, meshi->zplt, meshi->kbar, n11, constval);
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(rgb_ptr[3] > 0.5){
          float f1, f2;
          int k1, k2;
          int n1, n2;

          GetZInterpFactors(meshi->zplt, meshi->kbar, z11, &k1, &k2, &f1, &f2);
          n1 = n11 + k1;
          n2 = n11 + k2;
          yy1 = yplttemp[j];
          GET_VEC_DXYZ_TERRAIN(u, dx);
          GET_VEC_DXYZ_TERRAIN(v, dy);
          GET_VEC_DXYZ_TERRAIN(w, dz);
          glColor4fv(rgb_ptr);
          glVertex3f(x1 + dx, yy1 + dy, z11 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceTerrain:points dir=3");
  }
}

/* ------------------ DrawVVolSlice ------------------------ */

void DrawVVolSlice(const vslicedata *vd){
  int i, j, k, n;
  int i11;
  float constval, x1, yy1, z1;
  slicedata *u, *v, *w, *sd;
  float dx, dy, dz;
  float vrange;
  meshdata *meshi;
  float *xplttemp, *yplttemp, *zplttemp;
  int plotx, ploty, plotz;
  char *iblank;
  int nx, ny, nxy;
  float *rgb_ptr;

  sd = sliceinfo + vd->ival;
  meshi = meshinfo + sd->blocknumber;
  xplttemp = meshi->xplt;
  yplttemp = meshi->yplt;
  zplttemp = meshi->zplt;
  if(vd->volslice == 1){
    plotx = meshi->iplotx_all[iplotx_all];
    ploty = meshi->iploty_all[iploty_all];
    plotz = meshi->iplotz_all[iplotz_all];
  }
  else{
    plotx = sd->is1;
    ploty = sd->js1;
    plotz = sd->ks1;
  }

  iblank = meshi->c_iblank_node;
  nx = meshi->ibar + 1;
  ny = meshi->jbar + 1;
  nxy = nx*ny;

  vrange = velocity_range;
  if(vrange <= 0.0)vrange = 1.0;
  u = vd->u;
  v = vd->v;
  w = vd->w;
  if((vd->volslice == 1 && plotx >= 0 && visx_all == 1) || (vd->volslice == 0 && sd->idir == XDIR)){
    int maxj;

    constval = xplttemp[plotx] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    maxj = sd->js2;
    if(sd->js1 + 1 > maxj)maxj = sd->js1 + 1;
    for(j = sd->js1; j < maxj + 1; j += vectorskip){
      n = (j - sd->js1)*sd->nslicek - vectorskip;
      n += (plotx - sd->is1)*sd->nslicej*sd->nslicek;
      yy1 = yplttemp[j];
      for(k = sd->ks1; k < sd->ks2 + 1; k += vectorskip){
        n += vectorskip;
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = sd->iqsliceframe[n];
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(show_slice_in_obst == GAS_AND_SOLID || iblank == NULL ||
           (show_slice_in_obst==ONLY_IN_GAS     && iblank[IJK(plotx, j, k)] == GAS  &&rgb_ptr[3] > 0.5) ||
           (show_slice_in_obst == ONLY_IN_SOLID && iblank[IJK(plotx, j, k)] == SOLID&&rgb_ptr[3] > 0.5)
           ){
          z1 = zplttemp[k];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(constval - dx, yy1 - dy, z1 - dz);
          glVertex3f(constval + dx, yy1 + dy, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:lines dir=1");

    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    maxj = sd->js2;
    if(sd->js1 + 1 > maxj)maxj = sd->js1 + 1;
    for(j = sd->js1; j < maxj + 1; j += vectorskip){
      n = (j - sd->js1)*sd->nslicek - vectorskip;
      n += (plotx - sd->is1)*sd->nslicej*sd->nslicek;
      yy1 = yplttemp[j];
      for(k = sd->ks1; k < sd->ks2 + 1; k += vectorskip){
        n += vectorskip;
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = sd->iqsliceframe[n];
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(show_slice_in_obst==GAS_AND_SOLID||iblank==NULL||
          (show_slice_in_obst==ONLY_IN_GAS   && iblank[IJK(plotx, j, k)]==GAS  &&rgb_ptr[3]>0.5)||
          (show_slice_in_obst==ONLY_IN_SOLID && iblank[IJK(plotx, j, k)]==SOLID&&rgb_ptr[3]>0.5)
          ){
          z1 = zplttemp[k];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(constval + dx, yy1 + dy, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:points dir=1");
  }
  if((vd->volslice == 1 && ploty >= 0 && visy_all == 1) || (vd->volslice == 0 && sd->idir == YDIR)){
    int maxi;

    constval = yplttemp[ploty] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1 > maxi)maxi = sd->is1 + 1;
    for(i = sd->is1; i < maxi + 1; i += vectorskip){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - vectorskip;
      n += (ploty - sd->js1)*sd->nslicek;

      x1 = xplttemp[i];

      for(k = sd->ks1; k < sd->ks2 + 1; k += vectorskip){
        n += vectorskip;
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = sd->iqsliceframe[n];
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(show_slice_in_obst == GAS_AND_SOLID || iblank == NULL ||
           (show_slice_in_obst==ONLY_IN_GAS     && iblank[IJK(i, ploty, k)] == GAS  &&rgb_ptr[3] > 0.5) ||
           (show_slice_in_obst == ONLY_IN_SOLID && iblank[IJK(i, ploty, k)] == SOLID&&rgb_ptr[3] > 0.5)
           ){
          z1 = zplttemp[k];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(x1 - dx, constval - dy, z1 - dz);
          glVertex3f(x1 + dx, constval + dy, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:lines dir=2");
    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    for(i = sd->is1; i < maxi + 1; i += vectorskip){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - vectorskip;
      n += (ploty - sd->js1)*sd->nslicek;

      x1 = xplttemp[i];

      for(k = sd->ks1; k < sd->ks2 + 1; k += vectorskip){
        n += vectorskip;
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = sd->iqsliceframe[n];
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(show_slice_in_obst==GAS_AND_SOLID||iblank==NULL||
          (show_slice_in_obst==ONLY_IN_GAS   && iblank[IJK(i, ploty, k)]==GAS  &&rgb_ptr[3]>0.5)||
          (show_slice_in_obst==ONLY_IN_SOLID && iblank[IJK(i, ploty, k)]==SOLID&&rgb_ptr[3]>0.5)
          ){
          z1 = zplttemp[k];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(x1 + dx, constval + dy, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:points dir=2");
  }
  if((vd->volslice == 1 && plotz >= 0 && visz_all == 1) || (vd->volslice == 0 && sd->idir == ZDIR)){
    int maxi;

    constval = zplttemp[plotz] + offset_slice*sd->sliceoffset+SCALE2SMV(sd->sliceoffset_fds);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1 > maxi)maxi = sd->is1 + 1;
    for(i = sd->is1; i < maxi + 1; i += vectorskip){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - vectorskip*sd->nslicek;
      n += (plotz - sd->ks1);

      x1 = xplttemp[i];
      for(j = sd->js1; j < sd->js2 + 1; j += vectorskip){
        n += vectorskip*sd->nslicek;
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = sd->iqsliceframe[n];
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(show_slice_in_obst == GAS_AND_SOLID || iblank == NULL ||
           (show_slice_in_obst==ONLY_IN_GAS     && iblank[IJK(i, j, plotz)] == GAS  &&rgb_ptr[3] > 0.5) ||
           (show_slice_in_obst == ONLY_IN_SOLID && iblank[IJK(i, j, plotz)] == SOLID&&rgb_ptr[3] > 0.5)
           ){
          yy1 = yplttemp[j];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(x1 - dx, yy1 - dy, constval - dz);
          glVertex3f(x1 + dx, yy1 + dy, constval + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:lines dir=3");

    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    for(i = sd->is1; i < sd->is1 + sd->nslicei; i += vectorskip){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - vectorskip*sd->nslicek;
      n += (plotz - sd->ks1);

      x1 = xplttemp[i];
      for(j = sd->js1; j < sd->js2 + 1; j += vectorskip){
        n += vectorskip*sd->nslicek;
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = sd->iqsliceframe[n];
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        if(show_slice_in_obst==GAS_AND_SOLID||iblank==NULL||
          (show_slice_in_obst==ONLY_IN_GAS   && iblank[IJK(i, j, plotz)]==GAS  &&rgb_ptr[3]>0.5)||
          (show_slice_in_obst==ONLY_IN_SOLID && iblank[IJK(i, j, plotz)]==SOLID&&rgb_ptr[3]>0.5)
          ){
          yy1 = yplttemp[j];
          GET_VEC_DXYZ(u, dx, n);
          GET_VEC_DXYZ(v, dy, n);
          GET_VEC_DXYZ(w, dz, n);
          glColor4fv(rgb_ptr);
          glVertex3f(x1 + dx, yy1 + dy, constval + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:points dir=3");
  }
}

/* ------------------ DrawVSliceFrame ------------------------ */

void DrawVSliceFrame(void){
  int i;

  for(i=0;i<nvsliceinfo;i++){
    vslicedata *vd;
    slicedata *u, *v, *w, *val;

    vd = vsliceinfo + i;
    if(vd->loaded==0||vd->display==0||sliceinfo[vd->ival].slicefile_labelindex!=slicefile_labelindex)continue;
    val = vd->val;
    if(val==NULL)continue;
    u = vd->u;
    v = vd->v;
    w = vd->w;
    if(u==NULL&&v==NULL&&w==NULL)continue;
    if(sliceinfo[vd->ival].times[0]>global_times[itimes])continue;
#define VAL val
    if(VAL->compression_type==COMPRESSED_ZLIB){
      UncompressSliceDataFrame(VAL,VAL->itime);
      VAL->iqsliceframe=VAL->slicecomplevel;
    }
    else{
      if(VAL!=NULL)VAL->iqsliceframe = VAL->slicelevel + VAL->itime*VAL->nsliceijk;
    }
    if(VAL->qslicedata!=NULL)VAL->qsliceframe = VAL->qslicedata + VAL->itime*VAL->nsliceijk;
#undef VAL
#define VAL u
    if(VAL!=NULL){
      if(VAL->compression_type==COMPRESSED_ZLIB){
        UncompressSliceDataFrame(VAL,VAL->itime);
        VAL->iqsliceframe=VAL->slicecomplevel;
      }
      else{
        if(VAL!=NULL)VAL->iqsliceframe = VAL->slicelevel + VAL->itime*VAL->nsliceijk;
      }
    }
#undef VAL
#define VAL v
    if(VAL!=NULL){
      if(VAL->compression_type==COMPRESSED_ZLIB){
        UncompressSliceDataFrame(VAL,VAL->itime);
        VAL->iqsliceframe=VAL->slicecomplevel;
      }
      else{
        if(VAL!=NULL)VAL->iqsliceframe = VAL->slicelevel + VAL->itime*VAL->nsliceijk;
      }
    }
#undef VAL
#define VAL w
    if(VAL!=NULL){
      if(VAL->compression_type==COMPRESSED_ZLIB){
        UncompressSliceDataFrame(VAL,VAL->itime);
        VAL->iqsliceframe=VAL->slicecomplevel;
      }
      else{
        if(VAL!=NULL)VAL->iqsliceframe = VAL->slicelevel + VAL->itime*VAL->nsliceijk;
      }
    }
    if(u!=NULL&&u->compression_type==UNCOMPRESSED){
      u->qslice = u->qslicedata + u->itime*u->nsliceijk;
    }
    if(v!=NULL&&v->compression_type==UNCOMPRESSED){
      v->qslice = v->qslicedata + v->itime*v->nsliceijk;
    }
    if(w!=NULL&&w->compression_type==UNCOMPRESSED){
      w->qslice = w->qslicedata + w->itime*w->nsliceijk;
    }

    if(vd->vslicefile_type==SLICE_TERRAIN){
      DrawVVolSliceTerrain(vd);
    }
    else if(vd->vslicefile_type==SLICE_CELL_CENTER){
        DrawVVolSliceCellCenter(vd);
    }
    else{
      DrawVVolSlice(vd);
    }
    if(vd->volslice==1&&vis_gslice_data==1){
      DrawVGSliceData(vd);
      SNIFF_ERRORS("after DrawVGSliceData");
    }
  }
}

/* ------------------ UpdateGslicePlanes ------------------------ */

void UpdateGslicePlanes(void){
  int i;
  float *norm;
  float *xyz0;

  /* stuff min and max grid data into a more convenient form
  assuming the following grid numbering scheme

       5-------6
     / |      /|
   /   |     / |
  4 -------7   |
  |    |   |   |
  Z    1---|---2
  |  Y     |  /
  |/       |/
  0--X-----3

  */
  int ix[8]={0,0,1,1,0,0,1,1};
  int iy[8]={0,1,1,0,0,1,1,0};
  int iz[8]={0,0,0,0,1,1,1,1};

// plane equation: (x-xyz0) .dot. norm = 0
  norm=gslice_norm;
  xyz0 = gslice_xyz;


  for(i=0;i<nmeshes;i++){
    meshdata *meshi;
    int j;
    float vals[8],xx[2],yy[2],zz[2];
    float *xyzmin, *xyzmax;
    float level;

    meshi = meshinfo + i;

    xyzmin = meshi->slice_min;
    xyzmax = meshi->slice_max;
    if(xyzmin[0]>xyzmax[0])continue;
    xx[0]=xyzmin[0];
    yy[0]=xyzmin[1];
    zz[0]=xyzmin[2];
    xx[1]=xyzmax[0];
    yy[1]=xyzmax[1];
    zz[1]=xyzmax[2];
    for(j=0;j<8;j++){
      float xyz[3];

      xyz[0] = xx[ix[j]];
      xyz[1] = yy[iy[j]];
      xyz[2] = zz[iz[j]];
      vals[j] = PLANEDIST(norm,xyz0,xyz);
    }
    level=0.0;
    GetIsoBox(xx,yy,zz,NULL,vals,level,meshi->gsliceinfo.verts,&meshi->gsliceinfo.nverts,meshi->gsliceinfo.triangles,&meshi->gsliceinfo.ntriangles,NULL,NULL);
    meshi->gsliceinfo.ntriangles/=3;
  }
}

/* ------------------ DrawGSliceOutline ------------------------ */

void DrawGSliceOutline(void){
  int i;
  float zero[3]={0.0,0.0,0.0};

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);

  glColor3fv(foregroundcolor);

  if(show_gslice_triangles==1){
    glBegin(GL_LINES);
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      int j;
      int *triangles;
      float *verts;

      meshi = meshinfo + i;
      verts = meshi->gsliceinfo.verts;
      triangles = meshi->gsliceinfo.triangles;

      if(meshi->gsliceinfo.nverts==0||meshi->gsliceinfo.ntriangles==0)continue;
      for(j=0;j<meshi->gsliceinfo.ntriangles;j++){
        float *xyz1, *xyz2, *xyz3;

        xyz1 = verts + 3*triangles[3*j];
        xyz2 = verts + 3*triangles[3*j+1];
        xyz3 = verts + 3*triangles[3*j+2];

        glVertex3fv(xyz1);
        glVertex3fv(xyz2);
        glVertex3fv(xyz2);
        glVertex3fv(xyz3);
        glVertex3fv(xyz3);
        glVertex3fv(xyz1);
      }
    }
    glEnd();
  }

  if(show_gslice_triangulation==1){
    for(i=0;i<nmeshes;i++){
      meshdata *meshi;
      int j;
      float del;
      int *triangles;
      float *verts;

      meshi = meshinfo + i;
      verts = meshi->gsliceinfo.verts;
      triangles = meshi->gsliceinfo.triangles;

      del = meshi->cellsize;
      del *= del;
      del /= 4.0;
      if(meshi->gsliceinfo.nverts==0||meshi->gsliceinfo.ntriangles==0)continue;
      for(j=0;j<meshi->gsliceinfo.ntriangles;j++){
        float *xyz1, *xyz2, *xyz3;

        xyz1 = verts + 3*triangles[3*j];
        xyz2 = verts + 3*triangles[3*j+1];
        xyz3 = verts + 3*triangles[3*j+2];
        DrawTriangleOutlineSlice(xyz1,xyz2,xyz3,del,0);
      }
    }
  }
  // draw normal vector

  if(show_gslice_normal==1||show_gslice_normal_keyboard==1){
    glColor3f(1.0,0.0,0.0);
    glPushMatrix();
    glTranslatef(gslice_xyz[0],gslice_xyz[1],gslice_xyz[2]);
    glBegin(GL_LINES);
    glVertex3fv(zero);
    glVertex3fv(gslice_norm);
    glEnd();
    glPointSize(20.0);
    glBegin(GL_POINTS);
    glVertex3fv(zero);
    glVertex3fv(gslice_norm);
    glEnd();
    glPopMatrix();
  }

  glPopMatrix();
}

/* ------------------ DrawGSliceData ------------------------ */

void DrawGSliceData(slicedata *slicei){
  meshdata *meshi;
  int j;
  boundsdata *sb;
  float valmin, valmax;
  float del;
  int *triangles;
  float *verts;

  if(slicei->loaded==0||slicei->display==0||slicei->volslice==0)return;

  meshi = meshinfo + slicei->blocknumber;
  verts = meshi->gsliceinfo.verts;
  triangles = meshi->gsliceinfo.triangles;

  if(meshi->gsliceinfo.nverts==0||meshi->gsliceinfo.ntriangles==0)return;
  del = meshi->cellsize;
  del *= del;
  del /= 4.0;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);

  if(cullfaces==1)glDisable(GL_CULL_FACE);
  if(use_transparency_data==1)TransparentOn();

  sb=slicebounds+slicefile_labelindex;
  valmin = sb->levels256[0]*sb->fscale;
  valmax = sb->levels256[255]*sb->fscale;

  gslicedata=slicei->qsliceframe;
  gslice_valmin=valmin;
  gslice_valmax=valmax;
  gslice_valmesh=meshi;
  gslice=slicei;

  for(j=0;j<meshi->gsliceinfo.ntriangles;j++){
    float *xyz1, *xyz2, *xyz3;
    float t1, t2, t3;

    xyz1 = verts + 3*triangles[3*j];
    xyz2 = verts + 3*triangles[3*j+1];
    xyz3 = verts + 3*triangles[3*j+2];
    t1 = GetSliceTextureIndex(xyz1);
    t2 = GetSliceTextureIndex(xyz2);
    t3 = GetSliceTextureIndex(xyz3);

    DrawTriangleSlice(xyz1,xyz2,xyz3,t1,t2,t3,del,0);
  }
  if(use_transparency_data==1)TransparentOff();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
  glPopMatrix();
}

/* ------------------ DrawVGSliceData ------------------------ */

void DrawVGSliceData(vslicedata *vslicei){
  meshdata *meshi;
  int j;
  boundsdata *sb;
  float valmin, valmax;
  float del;
  slicedata *slicei;
  float *verts;
  int *triangles;

  slicei = sliceinfo + vslicei->ival;

  if(slicei->loaded==0/*||slicei->display==0*/||slicei->volslice==0)return;

  meshi = meshinfo + slicei->blocknumber;
  verts = meshi->gsliceinfo.verts;
  triangles = meshi->gsliceinfo.triangles;

  if(meshi->gsliceinfo.nverts==0||meshi->gsliceinfo.ntriangles==0)return;
  del = meshi->cellsize;
  del *= del;
  del /= 4.0;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);

  if(cullfaces==1)glDisable(GL_CULL_FACE);
  if(use_transparency_data==1)TransparentOn();

  sb=slicebounds+slicefile_labelindex;
  valmin = sb->levels256[0]*sb->fscale;
  valmax = sb->levels256[255]*sb->fscale;

  gslicedata=slicei->qsliceframe;
  gslice_valmin=valmin;
  gslice_valmax=valmax;
  gslice_valmesh=meshi;
  gslice=slicei;

  gslice_u=vslicei->u;
  gslice_v=vslicei->v;
  gslice_w=vslicei->w;

  for(j=0;j<meshi->gsliceinfo.ntriangles;j++){
    float *xyz1, *xyz2, *xyz3;

    xyz1 = verts + 3*triangles[3*j];
    xyz2 = verts + 3*triangles[3*j+1];
    xyz3 = verts + 3*triangles[3*j+2];

    DrawTriangleVectorSlice(xyz1,xyz2,xyz3,del,0);
  }
  if(use_transparency_data==1)TransparentOff();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
  glPopMatrix();
}

/* ------------------ InitSliceData ------------------------ */

void InitSliceData(void){
  FILE *fileout;
  char datafile[1024];
  int i, j, k, ii;
  float *xplt, *yplt, *zplt;
  slicedata *sd;
  meshdata *meshi;
  char *ext;
  char flabel[256];

  for(ii = 0; ii < nslice_loaded; ii++){
    i = slice_loaded_list[ii];
    sd = sliceinfo + i;
    if(sd->display == 0 || sd->slicefile_labelindex != slicefile_labelindex)continue;
    if(sd->times[0] > global_times[itimes])continue;

    strcpy(datafile, sd->file);
    ext = strstr(datafile, ".");
    if(ext != NULL){
      ext[0] = 0;
    }
    sprintf(flabel, "%i", itimes);
    TrimBack(flabel);
    strcat(datafile, "_sf_");
    strcat(datafile, flabel);
    strcat(datafile, ".csv");
    fileout = fopen(datafile, "w");
    if(fileout == NULL)continue;
    fprintf(fileout, "%s\n", sd->label.longlabel);
    fprintf(fileout, "%s\n", sd->label.unit);
    meshi = meshinfo + sd->blocknumber;

    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;
    fprintf(fileout, "%f, %f, %f, %f, %f, %f\n",
      xplt[sd->is1], xplt[sd->is2],
      yplt[sd->js1], yplt[sd->js2],
      zplt[sd->ks1], zplt[sd->ks2]);


    switch (sd->idir){
    case XDIR:
      fprintf(fileout, "%i\n", sd->ks2 + 1 - sd->ks1);
      for(k = sd->ks1; k <= sd->ks2; k++){
        if(k != sd->ks2)fprintf(fileout, "%f, ", zplt[k]);
        if(k == sd->ks2)fprintf(fileout, "%f ", zplt[k]);
      }
      fprintf(fileout, "\n");
      fprintf(fileout, "%i\n", sd->js2 + 1 - sd->js1);
      for(j = sd->js1; j <= sd->js2; j++){
        if(j != sd->js2)fprintf(fileout, "%f, ", yplt[j]);
        if(j == sd->js2)fprintf(fileout, "%f ", yplt[j]);
      }
      fprintf(fileout, "\n");
      break;
    case YDIR:
      fprintf(fileout, "%i\n", sd->ks2 + 1 - sd->ks1);
      for(k = sd->ks1; k <= sd->ks2; k++){
        if(k != sd->ks2)fprintf(fileout, "%f, ", zplt[k]);
        if(k == sd->ks2)fprintf(fileout, "%f ", zplt[k]);
      }
      fprintf(fileout, "\n");
      fprintf(fileout, "%i\n", sd->is2 + 1 - sd->is1);
      for(i = sd->is1; i <= sd->is2; i++){
        if(i != sd->is2)fprintf(fileout, "%f, ", xplt[i]);
        if(i == sd->is2)fprintf(fileout, "%f ", xplt[i]);
      }
      fprintf(fileout, "\n");
      break;
    case ZDIR:
      fprintf(fileout, "%i\n", sd->js2 + 1 - sd->js1);
      for(j = sd->js1; j <= sd->js2; j++){
        if(j != sd->js2)fprintf(fileout, "%f, ", yplt[j]);
        if(j == sd->js2)fprintf(fileout, "%f ", yplt[j]);
      }
      fprintf(fileout, "\n");
      fprintf(fileout, "%i\n", sd->is2 + 1 - sd->is1);
      for(i = sd->is1; i <= sd->is2; i++){
        if(i != sd->is2)fprintf(fileout, "%f, ", xplt[i]);
        if(i == sd->is2)fprintf(fileout, "%f ", xplt[i]);
      }
      fprintf(fileout, "\n");
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
    fclose(fileout);
    fileout = NULL;

  }
}

/* ------------------ GetSliceVal ------------------------ */

float GetSliceVal(slicedata *sd, unsigned char ival){
  float returnval;

  returnval = (sd->valmax*ival + sd->valmin*(255-ival))/255.0;
  return returnval;
}

/* ------------------ SliceData2Hist ------------------------ */

void SliceData2Hist(slicedata *sd, float *xyz, float *dxyz, float time, float dtime, histogramdata *histogram){
  int i,j,k,t;
  int imin, imax, jmin, jmax, kmin, kmax, tmin, tmax;
  int ntimes;
  float *times, *xplt, *yplt, *zplt;
  int ibar, jbar, kbar;
  meshdata *meshi;
  int nvals,ival;
  float *vals;

  meshi = meshinfo+sd->blocknumber;
  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  kbar = meshi->kbar;


  times = sd->times;
  ntimes = sd->ntimes;
  tmin = GetInterval(time-dtime, times, ntimes);
  tmax = GetInterval(time+dtime, times, ntimes);
  imin = GetInterval(xyz[0]-dxyz[0], xplt, ibar+1);
  imax = GetInterval(xyz[0]+dxyz[0], xplt, ibar+1);
  jmin = GetInterval(xyz[1]-dxyz[1], yplt, jbar+1);
  jmax = GetInterval(xyz[1]+dxyz[1], yplt, jbar+1);
  kmin = GetInterval(xyz[2]-dxyz[2], zplt, kbar+1);
  kmax = GetInterval(xyz[2]+dxyz[2], zplt, kbar+1);

  nvals = (tmax+1-tmin)*(imax+1-imin)*(jmax+1-jmin)*(kmax+1-kmin);
  NewMemory((void **)&vals, nvals*sizeof(float));

  // val(i,j,k) = di*nj*nk + dj*nk + dk

//#define SLICEVAL(i,j,k) qslice[(i-sd->is1)*sd->nslicej*sd->nslicek + (j-sd->js1)*sd->nslicek + (k-sd->ks1)]
  ival = 0;
  for(t = tmin; t<=tmax; t++){
    float *qslice;

    qslice = sd->qslicedata+t*sd->nsliceijk;
    for(i = imin; i<=imax; i++){
      float *qslicei;

      qslicei = qslice+(i-sd->is1)*sd->nslicej*sd->nslicek;
      for(j = jmin; j<=jmax; j++){
        float *qslicej;

        qslicej = qslicei+(j-sd->js1)*sd->nslicek;
        for(k = kmin; k<=kmax; k++){
          float *qslicek;

          qslicek = qslicej+(k-sd->ks1);
          vals[ival++] = *qslicek;
        }
      }
    }
  }
  InitHistogram(histogram, NHIST_BUCKETS, NULL, NULL);
  CopyVals2Histogram(vals, NULL, NULL, nvals, histogram);
  FREEMEMORY(vals);
}

