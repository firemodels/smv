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
#include "interp.h"
#include "smokeviewvars.h"
#include "IOscript.h"
#include "getdata.h"

#define SLICE_HEADER_SIZE 4
#define SLICE_TRAILER_SIZE 4

#define IJK_SLICE(i,j,k)  ( ((i)-sd->is1)*sd->nslicej*sd->nslicek + ((j)-sd->js1)*sd->nslicek + ((k)-sd->ks1) )

#define SLICETEXTURE(i,j,k) \
    (sd->compression_type==UNCOMPRESSED ? \
    CLAMP((sd->qslice[ IJK_SLICE((i), (j),  (k))]-valmin)/(valmax-valmin),0.0,1.0) : \
    CLAMP(((float)sd->slicecomplevel[ IJK_SLICE((i), (j),  (k))])/255.0,0.0,1.0) \
    )

#define SLICECOLOR(index) CLAMP((int)(255.0*(sd->qslice[index]-valmin)/(valmax-valmin)),0,255)

#define FOPEN_SLICE(a,b)         fopen(a,b)
#ifdef X64
#define FSEEK_SLICE(a,b,c)       _fseeki64(a,b,c)
#define FTELL_SLICE(a)           _ftelli64(a)
#else
#define FSEEK_SLICE(a,b,c)       fseeko(a,b,c)
#define FTELL_SLICE(a)           ftello(a)
#endif
#define FREAD_SLICE(a,b,c,d)     fread(a,b,c,d)
#define FCLOSE_SLICE(a)          fclose(a)

#define FORT_SLICEREAD(var,count,STREAM) \
                           FSEEK_SLICE(STREAM,SLICE_HEADER_SIZE,SEEK_CUR);\
                           returncode=FREAD_SLICE(var,4,count,STREAM);\
                           if(returncode!=count)returncode=0;\
                           FSEEK_SLICE(STREAM,SLICE_TRAILER_SIZE,SEEK_CUR)

void DrawQuadSlice(float *v1, float *v2, float *v3, float *v4, float t1, float t2, float t3, float t4, float del, int level);
void DrawQuadVectorSlice(float *v1, float *v2, float *v3, float *v4, float del, int level);
void DrawTriangleOutlineSlice(float *v1, float *v2, float *v3, float del, int level);

// dummy change
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
                           FSEEK(RLESLICEFILE,4,SEEK_CUR)

#define GET_VAL(U,VAL,n) \
         VAL=0.0;           \
         if(U!=NULL){       \
           if(U->compression_type!=UNCOMPRESSED){\
             VAL = U->qval256[U->iqsliceframe[(n)]];\
           }                                  \
           else{                              \
             VAL = U->qslice[(n)];               \
           }                                  \
         }

#define GET_VAL_N(U,n)  ( (U)->compression_type!=UNCOMPRESSED ? (U)->qval256[(U)->iqsliceframe[(n)]] : (U)->qslice[(n)] )

#define GET_VEC_DXYZ(U,DU,n)                       \
         if(U==NULL){                              \
           DU=0.0;                                 \
         }                                         \
         else{                                     \
           if(U->compression_type==UNCOMPRESSED){  \
             DU = U->qslice[(n)];                  \
           }                                       \
           else{                                   \
             DU = U->qval256[U->iqsliceframe[(n)]];\
           }                                       \
         }

#define ADJUST_VEC_DX(dx)                       \
         if(vec_uniform_length==1){                              \
           float vecnorm; \
           vecnorm = ABS(dx);\
           if(vecnorm==0.0)vecnorm=1.0;\
           dx *= vecfactor*0.05/(vel_max*vecnorm);\
         }                                         \
         else{                                     \
           dx *= 0.05*vecfactor/vel_max;\
         }                                         \

#define ADJUST_VEC_DXYZ(dx,dy,dz)                       \
         if(vec_uniform_length==1){                              \
           float vecnorm; \
           vecnorm = sqrt(dx*dx+dy*dy+dz*dz);\
           if(vecnorm==0.0)vecnorm=1.0;\
           dx *= vecfactor*0.05/(vel_max*vecnorm);\
           dy *= vecfactor*0.05/(vel_max*vecnorm);\
           dz *= vecfactor*0.05/(vel_max*vecnorm);\
         }                                         \
         else{                                     \
           dx *= 0.05*vecfactor/vel_max;\
           dy *= 0.05*vecfactor/vel_max;\
           dz *= 0.05*vecfactor/vel_max;\
         }                                         \

#define GET_VEC_DXYZ_TERRAIN(U,DU,n)                                                 \
         if(U==NULL){                                                              \
           DU=0.0;                                                                 \
         }                                                                         \
         else{                                                                     \
           if(U->compression_type==UNCOMPRESSED){                                  \
             DU = U->qslice[n];                               \
           }                                                                       \
           else{                                                                   \
             DU = U->qval256[(int)(U->iqsliceframe[n])];\
           }                                                                       \
         }                                                                         \
         DU *= SCALE2FDS(.05*vecfactor/vel_max)

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

/* ------------------ DrawQuadSlice ------------------------ */

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

/* ------------------ DrawTriangleOutlineSlice ------------------------ */

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

/* ------------------ DrawTriangleVectorSlice ------------------------ */

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
    float vecfactor2, vel_max;

    vel_max = max_velocity;
    if(vel_max<= 0.0)vel_max = 1.0;
    vecfactor2 = 0.05*vecfactor /vel_max*xyzmaxdiff;

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
  writeslicedata(sd->file,
    sd->is1,sd->is2,sd->js1,sd->js2,sd->ks1,sd->ks2,
    sd->qslicedata,sd->times,sd->ntimes, redirect);
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
  FILE *stream, *sizestream;
  int count;

  stream = FOPEN(file, "rb");
  if(stream==NULL)return 0;

  sizestream = fopen(sizefile, "w");
  if(sizestream==NULL){
    fclose(stream);
    return 0;
  }
  count = 0;
  if(compression_type!=UNCOMPRESSED){
    float minmax[2];
    int ijkbar[6];

  // endian
  // completion (0/1)
  // fileversion (compressed format)
  // version_local  (slicef version)
  // global min max (used to perform conversion)
  // i1,i2,j1,j2,k1,k2

#define FSKIP if(compression_type==COMPRESSED_RLE)FSEEK(stream, 4, SEEK_CUR)

    FSKIP;FSEEK(stream, 4, SEEK_CUR); FSKIP;
    FSKIP;FSEEK(stream, 12, SEEK_CUR);FSKIP;
    FSKIP;fread(minmax, 4, 2, stream);FSKIP;
    FSKIP;fread(ijkbar, 4, 6, stream);FSKIP;

    fprintf(sizestream, "%i %i %i %i %i %i\n", ijkbar[0], ijkbar[1], ijkbar[2], ijkbar[3], ijkbar[4], ijkbar[5]);
    fprintf(sizestream, "%f %f\n", minmax[0], minmax[1]);
    count = 2;

    while(!feof(stream)){
      float time_local;
      int ncompressed, noriginal;

//  time
//  original frame size, compressed frame size (rle)
//  compressed frame size                      (zlib)
//  compressed buffer
      FSKIP;if(fread(&time_local, 4, 1, stream)!=1)break;FSKIP;
      if(compression_type==COMPRESSED_RLE){
        FSEEK(stream, 4, SEEK_CUR);
        fread(&noriginal, 4, 1, stream);
        fread(&ncompressed, 4, 1, stream);
        FSEEK(stream, 4, SEEK_CUR);
      }
      else{
        fread(&ncompressed, 4, 1, stream);
      }
      if(compression_type==COMPRESSED_ZLIB)fprintf(sizestream, "%f %i %i\n", time_local, ncompressed, 0);
      if(compression_type==COMPRESSED_RLE )fprintf(sizestream, "%f %i %i\n", time_local, noriginal, ncompressed);
      count++;

      FSKIP;FSEEK(stream, ncompressed, SEEK_CUR); FSKIP;
    }
  }
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
  int headersize,framesize;
  int frame_size;
  long int skip_local;
  FILEBUFFER *SLICEFILE=NULL;
  float *time_local,*slicevals;
  int error;
  int returncode=0;

  sd = sliceinfo + sd_index;
  if(sd->loaded==1)ReadSlice(sd->file,sd_index, ALL_FRAMES, NULL,  UNLOAD,SET_SLICECOLOR,&error);
  if(flag==UNLOAD){
    FREEMEMORY(sd->qslicedata);
    FREEMEMORY(sd->times);
    return 0;
  }
  if(frame_index_local==first_frame_index){
    if(sd->compression_type==UNCOMPRESSED){

      getslicesizes(sd->file, &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, tload_step,&error,
        use_tload_begin, use_tload_end, tload_begin, tload_end, &headersize, &framesize);
    }
    else if(sd->compression_type!=UNCOMPRESSED){
      if(
        GetSliceHeader(sd->comp_file,sd->size_file,sd->compression_type,
                       tload_step, use_tload_begin, use_tload_end, tload_begin, tload_end,
                       &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, &sd->ncompressed, &sd->valmin, &sd->valmax)==0){
        ReadSlice("",sd_index, ALL_FRAMES,NULL,UNLOAD,SET_SLICECOLOR,&error);
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

  if(SLICEFILE==NULL){
    SLICEFILE=FOPEN_SLICE(sd->file,"rb");
  }
  if(SLICEFILE==NULL){
    return -1;
  }

  FSEEK_SLICE(SLICEFILE,skip_local,SEEK_SET); // skip from beginning of file

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

  FORT_SLICEREAD(time_local,1,SLICEFILE);
  FORT_SLICEREAD(slicevals,frame_size,SLICEFILE);
  FCLOSE_SLICE(SLICEFILE);
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

void ReadFed(int file_index, int time_frame, float *time_value, int flag, int file_type, int *errorcode){
  feddata *fedi;
  slicedata *fed_slice,*o2,*co2,*co;
  isodata *fed_iso;
  int error_local;
  meshdata *meshi;
  int nx, ny;
  int nxy;
  int nxdata, nydata;
  int ibar, jbar, kbar;
  slicedata *slicei=NULL;

#define FEDCO(CO) ( (2.764/100000.0)*pow(1000000.0*CLAMP(CO,0.0,0.1),1.036)/60.0 )
#define FEDO2(O2)  ( exp( -(8.13-0.54*(20.9-100.0*CLAMP(O2,0.0,0.2))) )/60.0 )
#define HVCO2(CO2) (exp(0.1930*CLAMP(CO2,0.0,0.1)*100.0+2.0004)/7.1)

  ASSERT(fedinfo!=NULL);
  ASSERT(file_index>=0);
  if(file_type==FED_SLICE){

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
    ReadSlice(fed_slice->file,fedi->fed_index, ALL_FRAMES, NULL, UNLOAD,SET_SLICECOLOR,&error_local);
  }
  else if(file_type==FED_ISO){
    ReadIsoOrig(fed_iso->file,file_index,UNLOAD,&error_local);
  }

  if(flag==UNLOAD){
    update_draw_hist = 1;
    return;
  }

  // regenerate if either the FED slice or isosurface file does not exist or is older than
  // either the CO, CO2 or O2 slice files

  if(file_type==FED_SLICE){
    FILE *stream;

    stream = fopen(slicei->bound_file, "r");
    if(stream!=NULL){
      slicei->have_bound_file = 1;
      fclose(stream);
      UpdateGlobalFEDSliceBounds();
    }
    else{
      slicei->have_bound_file = 0;
      regenerate_fed = 1;
    }
  }
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

       ReadFed(file_index,time_frame, time_value, UNLOAD, file_type, errorcode);
       return;
    }
    fed_slice->is1=co->is1;
    fed_slice->is2=co->is2;
    if(fed_slice->is1!=fed_slice->is2&&fed_slice->is1==1)fed_slice->is1 = 0;
    fed_slice->js1=co->js1;
    fed_slice->js2=co->js2;
    if(fed_slice->js1!=fed_slice->js2&&fed_slice->js1==1)fed_slice->js1 = 0;
    fed_slice->ks1=co->ks1;
    fed_slice->ks2=co->ks2;
    if(fed_slice->ks1!=fed_slice->ks2&&fed_slice->ks1==1)fed_slice->ks1 = 0;
    fed_slice->nslicei=co->nslicei;
    fed_slice->nslicej=co->nslicej;
    fed_slice->nslicek=co->nslicek;
    fed_slice->volslice=co->volslice;
    fed_slice->iis1 = co->iis1;
    fed_slice->iis2 = co->iis2;
    fed_slice->jjs1 = co->jjs1;
    fed_slice->jjs2 = co->jjs2;
    fed_slice->kks1 = co->kks1;
    fed_slice->kks2 = co->kks2;
    fed_slice->plotx = co->plotx;
    fed_slice->ploty = co->ploty;
    fed_slice->plotz = co->plotz;
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
       ReadFed(file_index,time_frame,NULL,UNLOAD, file_type, errorcode);
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
    float fed_valmin = 1.0, fed_valmax = 0.0;
    fed_valmin = 0.0;
    fed_valmax = 0.0;
    for(i=1;i<fed_slice->ntimes;i++){
      int jj;
      float dt;

      if(CReadSlice_frame(i,fedi->o2_index,LOAD)<0||
         CReadSlice_frame(i,fedi->co2_index,LOAD)<0||
         CReadSlice_frame(i,fedi->co_index,LOAD)){
         ReadFed(file_index, time_frame,NULL,UNLOAD, file_type,errorcode);
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
        fed_valmin = MIN(fed_valmin, fed_frame[jj]);
        fed_valmax = MAX(fed_valmax, fed_frame[jj]);
      }
    }
    FREEMEMORY(iblank);
    if(file_type==FED_SLICE){
      FILE *stream=NULL;

      stream = fopen(slicei->bound_file, "w");
      if(stream!=NULL){
        fprintf(stream, "0.0 %f %f\n", fed_valmin, fed_valmax);
        fclose(stream);
        slicei->have_bound_file=1;
        UpdateGlobalFEDSliceBounds();
      }
      else{
        slicei->have_bound_file=0;
      }
    }
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
    ReadSlice(fed_slice->file,fedi->fed_index,ALL_FRAMES,NULL,flag,SET_SLICECOLOR,&error_local);
  }
  else{
    ReadIsoOrig(fed_iso->file,file_index,flag,&error_local);
  }
  {
    colorbardata *cb;

    cb = GetColorbar(default_fed_colorbar);
    if(cb!=NULL){
      colorbartype=cb-colorbarinfo;
      SetColorbarListEdit(colorbartype);
      SliceBoundCB(COLORBAR_LIST2);
      UpdateCurrentColorbar(cb);
    }
  }
}

/* ------------------ ReadVSlice ------------------------ */

FILE_SIZE ReadVSlice(int ivslice, int time_frame, float *time_value, int flag, int set_slicecolor, int *errorcode){
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

  if(vd->iu!=-1)sliceinfo[vd->iu].uvw = 1;
  if(vd->iv!=-1)sliceinfo[vd->iv].uvw = 1;
  if(vd->iw!=-1)sliceinfo[vd->iw].uvw = 1;
  if(flag==UNLOAD){
    if(vd->loaded==0)return 0;
    if(vd->iu!=-1){
      slicedata *u=NULL;

      u = sliceinfo + vd->iu;
      display=u->display;
      if(u->loaded==1){
        if(u->slice_filetype == SLICE_GEOM){
          return_filesize = ReadGeomData(u->patchgeom, u, UNLOAD, time_frame, time_value, errorcode);
        }
        else{
          return_filesize+=ReadSlice(u->file, vd->iu, time_frame,NULL,UNLOAD, DEFER_SLICECOLOR, errorcode);
        }
      }
      u->display=display;
      u->vloaded=0;
    }
    if(vd->iv!=-1){
      slicedata *v=NULL;

      v = sliceinfo + vd->iv;
      display=v->display;
      if(v->loaded==1){
        if(v->slice_filetype == SLICE_GEOM){
          return_filesize = ReadGeomData(v->patchgeom, v, UNLOAD, time_frame, time_value, errorcode);
        }
        else{
          return_filesize+=ReadSlice(v->file, vd->iv, time_frame,NULL,UNLOAD, DEFER_SLICECOLOR, errorcode);
        }
      }
      v->display=display;
      v->vloaded=0;
    }
    if(vd->iw!=-1){
      slicedata *w=NULL;

      w = sliceinfo + vd->iw;
      display=w->display;
      if(w->loaded==1){
        if(w->slice_filetype == SLICE_GEOM){
          return_filesize = ReadGeomData(w->patchgeom, w, UNLOAD, time_frame, time_value, errorcode);
        }
        else{
          return_filesize+=ReadSlice(w->file, vd->iw, time_frame,NULL,UNLOAD, DEFER_SLICECOLOR, errorcode);
        }
      }
      w->display=display;
      w->vloaded=0;
    }
    if(vd->ival!=-1){
      slicedata *val=NULL;

      val = sliceinfo + vd->ival;
      display=val->display;
      if(val->loaded==1){
        if(val->slice_filetype == SLICE_GEOM){
          return_filesize = ReadGeomData(val->patchgeom, val, UNLOAD, time_frame, time_value, errorcode);
        }
        else{
          return_filesize+=ReadSlice(val->file, vd->ival, time_frame,NULL,UNLOAD, set_slicecolor, errorcode);
        }
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
    update_draw_hist = 1;
    update_vectorskip = 1;
    return return_filesize;
  }
  if(vd->finalize==0)set_slicecolor = DEFER_SLICECOLOR;
  if(vd->iu!=-1){
    slicedata *u=NULL;

    u = sliceinfo + vd->iu;
    u->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->u=u;
    if(scriptoutstream==NULL||script_defer_loading==0){
      if(u->slice_filetype == SLICE_GEOM){
        return_filesize += ReadGeomData(u->patchgeom, u, LOAD, time_frame, time_value, errorcode);
      }
      else{
        return_filesize += ReadSlice(u->file, vd->iu, time_frame,time_value, flag, set_slicecolor, errorcode);
      }
      if(*errorcode!=0){
        vd->loaded = 1;
        fprintf(stderr, "*** Error: unable to load U velocity vector components in %s . Vector load aborted\n", u->file);
        ReadVSlice(ivslice, time_frame, time_value, UNLOAD, DEFER_SLICECOLOR, errorcode);
        *errorcode = 1;
        return 0;
      }
      if(u->valmin<valmin)valmin = u->valmin;
      if(u->valmax>valmax)valmax = u->valmax;
      u->display = 0;
      u->vloaded = 1;
    }
  }
  if(vd->iv!=-1){
    slicedata *v=NULL;

    v = sliceinfo + vd->iv;
    v->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->v=v;
    if(scriptoutstream==NULL||script_defer_loading==0){
      if(v->slice_filetype == SLICE_GEOM){
        return_filesize += ReadGeomData(v->patchgeom, v, LOAD, time_frame, time_value, errorcode);
      }
      else{
        return_filesize += ReadSlice(v->file, vd->iv, time_frame,time_value,flag, set_slicecolor, errorcode);
      }
      if(*errorcode!=0){
        fprintf(stderr, "*** Error: unable to load V velocity vector components in %s . Vector load aborted\n", v->file);
        vd->loaded = 1;
        ReadVSlice(ivslice, time_frame, time_value, UNLOAD, DEFER_SLICECOLOR, errorcode);
        *errorcode = 1;
        return 0;
      }

      if(v->valmin<valmin)valmin = v->valmin;
      if(v->valmax>valmax)valmax = v->valmax;
      v->display = 0;
      v->vloaded = 1;
    }
  }
  if(vd->iw!=-1){
    slicedata *w=NULL;

    w = sliceinfo + vd->iw;
    w->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->w=w;
    if(scriptoutstream==NULL||script_defer_loading==0){
      if(w->slice_filetype == SLICE_GEOM){
        return_filesize += ReadGeomData(w->patchgeom, w, LOAD, time_frame, time_value, errorcode);
      }
      else{
        return_filesize += ReadSlice(w->file, vd->iw, time_frame,time_value,flag, set_slicecolor, errorcode);
      }
      if(*errorcode!=0){
        fprintf(stderr, "*** Error: unable to load W velocity vector components in %s . Vector load aborted\n", w->file);
        vd->loaded = 1;
        ReadVSlice(ivslice, time_frame, time_value, UNLOAD, DEFER_SLICECOLOR, errorcode);
        *errorcode = 1;
        return 0;
      }

      if(w->valmin<valmin)valmin = w->valmin;
      if(w->valmax>valmax)valmax = w->valmax;
      w->display = 0;
      w->vloaded = 1;
    }
  }
  vd->vslicefile_labelindex=-1;
  if(vd->ival!=-1){
    slicedata *val=NULL;

    val = sliceinfo + vd->ival;
    val->finalize = vd->finalize;
    finalize = vd->finalize;
    vd->val=val;
    if(scriptoutstream==NULL||script_defer_loading==0){
      if(val->slice_filetype == SLICE_GEOM){
        return_filesize += ReadGeomData(val->patchgeom, val, LOAD, time_frame, time_value, errorcode);
      }
      else{
        return_filesize += ReadSlice(val->file, vd->ival, time_frame,time_value,flag, set_slicecolor, errorcode);
      }
      if(*errorcode!=0){
        fprintf(stderr, "*** Error: unable to load vector values in %s . Vector load aborted\n", val->file);
        vd->loaded = 1;
        ReadVSlice(ivslice, time_frame, time_value, UNLOAD, DEFER_SLICECOLOR, errorcode);
        *errorcode = 1;
        return 0;
      }
      slicefile_labelindex = GetSliceBoundsIndex(val);
      vd->vslicefile_labelindex = val->slicefile_labelindex;
      vd->valmin = valmin;
      vd->valmax = valmax;
      val->display = 0;
      val->vloaded = 1;
    }
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
    max_velocity = MAX(ABS(valmax),ABS(valmin));
  }
  PushVSliceLoadstack(ivslice);

  PrintMemoryInfo;
  if(finalize==1){
    updatemenu = 1;
    ForceIdle();
  }
  update_vectorskip = 1;
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
  if(sd->compression_type == COMPRESSED_RLE){
    countout = UnCompressRLE(compressed_data, countin, sd->slicecomplevel);
  }
  CheckMemory;
}

/* ------------------ GetHistogramValProc ------------------------ */

void GetHistogramValProc(histogramdata *histogram, float cdf, float *val){
  *val = GetHistogramVal(histogram, cdf);
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
        if(sd->slice_filetype == SLICE_CELL_CENTER &&
          ((k == 0 && sd->nslicek != 1) || (j == 0 && sd->nslicej != 1) || (i == 0 && sd->nslicei != 1)))continue;
        if(show_slice_in_obst == ONLY_IN_GAS){
          if(sd->slice_filetype != SLICE_CELL_CENTER&& iblank_node != NULL&&iblank_node[IJKNODE(sd->is1 + i, sd->js1 + j, sd->ks1 + k)] == SOLID)continue;
          if(sd->slice_filetype == SLICE_CELL_CENTER&& iblank_cell != NULL&&iblank_cell[IJKCELL(sd->is1 + i - 1, sd->js1 + j - 1, sd->ks1 + k - 1)] == EMBED_YES)continue;
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
  int skip;
  skip = MAX(ntimes/ histogram_nframes, 1);
  for(istep = 0; istep < ntimes; istep+=skip){
    histogramdata *histi, *histall;
    int nn;

    if(sd->compression_type != UNCOMPRESSED){
      UncompressSliceDataFrame(sd, istep);
    }
    for(nn = 0; nn < sd->nslicei*sd->nslicej*sd->nslicek; nn++){
      if(sd->compression_type != UNCOMPRESSED){
        pdata0[nn] = sd->qval256[sd->slicecomplevel[nn]];
      }
      else{
        pdata0[nn] = sd->qslicedata[n + nn];
      }
    }
    n += skip*sd->nslicei*sd->nslicej*sd->nslicek;

    // compute histogram for each timestep, histi and all time steps, histall

    histi = sd->histograms + istep + 1;
    histall = sd->histograms;
    CopyVals2Histogram(pdata0, slice_mask0, slice_weight0, nframe, histi);
    MergeHistogram(histall, histi, MERGE_BOUNDS);
  }
  FREEMEMORY(pdata0);
  FREEMEMORY(slice_mask0);
  FREEMEMORY(slice_weight0);
}

/* ------------------ GetSliceGeomHists ------------------------ */

void GetSliceGeomHists(slicedata *sd){
  if(sd->histograms != NULL)return;

  // initialize histograms

  sd->nhistograms = 1;
  NewMemory((void **)&sd->histograms, sizeof(histogramdata));
  InitHistogram(sd->histograms, NHIST_BUCKETS, NULL, NULL);

  // compute histogram for each timestep, histi and all time steps, histall

  CopyVals2Histogram(sd->patchgeom->geom_vals, NULL, NULL, sd->patchgeom->geom_nvals, sd->histograms);
}

/* ------------------ ComputeLoadedSliceHist ------------------------ */

void ComputeLoadedSliceHist(char *label){
  int i;

  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(slicei->loaded == 0)continue;
    if(label!=NULL&&strcmp(slicei->label.shortlabel, label)!=0)continue;
    if(slicei->histograms==NULL){
      if(slicei->slice_filetype==SLICE_GEOM){
        GetSliceGeomHists(slicei);
      }
      else{
        GetSliceHists(slicei);
      }
    }
  }
}

/* ------------------ MergeLoadedSliceHist ------------------------ */

void MergeLoadedSliceHist(char *label, histogramdata **histptr){
  histogramdata *hist;
  int i;

  hist = *histptr;
  if(*histptr != NULL)FreeHistogram(*histptr);
  NewMemory((void **)&hist, sizeof(histogramdata));
  *histptr = hist;

  InitHistogram(hist, NHIST_BUCKETS, NULL, NULL);
  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo + i;
    if(slicei->loaded == 0 || strcmp(slicei->label.shortlabel, label) != 0)continue;
    MergeHistogram(hist, slicei->histograms, MERGE_BOUNDS);
  }
}

/* ------------------ UpdateSliceHist ------------------------ */

void UpdateSliceHist(void){
  int i;
  int nmax;
  int is_fed = 0;

  histograms_defined = 1;
  ComputeLoadedSliceHist(NULL);
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

      dval = sb->dlg_valmax - sb->dlg_valmin;

      hist256i = hists256_slice + i;
      hist12i = hists12_slice + i;

      val256_min = sb->dlg_valmin - dval / (float)(histogram_nbuckets - 2);
      val256_max = sb->dlg_valmax + dval / (float)(histogram_nbuckets - 2);
      InitHistogram(hist256i, histogram_nbuckets, &val256_min, &val256_max);

      val12_min = sb->dlg_valmin - dval / 10.0;
      val12_max = sb->dlg_valmax + dval / 10.0;
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
        slicebounds[i].dlg_setvalmin=SET_MIN;
        slicebounds[i].dlg_valmin=0.0;
      }
      if(slicebounds[i].dlg_setvalmin!=SET_MIN){
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
        slicebounds[i].dlg_setvalmax=SET_MAX;
        slicebounds[i].dlg_valmax=3.0;
      }
      if(slicebounds[i].dlg_setvalmax!=SET_MAX){
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
    if(minflag==1)slicebounds[i].dlg_valmin=valmin;
    if(maxflag==1)slicebounds[i].dlg_valmax=valmax;
    if(minflag2==1)slicebounds[i].data_valmin=valmin_data;
    if(maxflag2==1)slicebounds[i].data_valmax=valmax_data;
  }
}

/* ------------------ SetSliceLabels ------------------------ */

void SetSliceLabels(float smin, float smax,
  slicedata *sd, patchdata *pd, int *errorcode){
  int slicetype=-1;

  ASSERT((sd != NULL && pd == NULL)||(sd == NULL && pd != NULL));
  if(sd!=NULL)slicetype = GetSliceBoundsIndexFromLabel(sd->label.shortlabel);
  if(pd != NULL)slicetype = GetSliceBoundsIndexFromLabel(pd->label.shortlabel);
  if(slicetype!=-1){
    boundsdata *sb;

    sb = slicebounds+slicetype;
    if(sd!=NULL)sb->label = &(sd->label);
    if(pd!=NULL)sb->label = &(pd->label);

    *errorcode = 0;
    GetColorbarLabels(smin, smax, nrgb, sb->colorlabels, sb->levels256);
  }
}

/* ------------------ UpdateAllSliceLabels ------------------------ */

void UpdateAllSliceLabels(int slicetype, int *errorcode){
  int i;
  float valmin, valmax;
  int setvalmin, setvalmax;
  int ii;
  slicedata *sd;

  *errorcode=0;

  setvalmin=slicebounds[slicetype].dlg_setvalmin;
  setvalmax=slicebounds[slicetype].dlg_setvalmax;
  if(setvalmin==1){
    valmin=slicebounds[slicetype].dlg_valmin;
  }
  else{
    valmin=slicebounds[slicetype].data_valmin;
    slicebounds[slicetype].dlg_valmin=valmin;
  }
  if(setvalmax==1){
    valmax=slicebounds[slicetype].dlg_valmax;
  }
  else{
    valmax=slicebounds[slicetype].data_valmax;
    slicebounds[slicetype].dlg_valmax=valmax;
  }
  for(ii=0;ii<nslice_loaded;ii++){
    i = slice_loaded_list[ii];
    sd = sliceinfo + i;
    if(sd->slicefile_labelindex == slicetype){
      SetSliceLabels(valmin, valmax, sd, NULL, errorcode);
    }
    if(*errorcode!=0)return;
  }
  SliceBounds2Glui(slicetype);
}

/* ------------------ SetSliceColors ------------------------ */

void SetSliceColors(float smin, float smax, slicedata *sd, int flag, int *errorcode){
  int slicetype;
  boundsdata *sb;

  slicetype = GetSliceBoundsIndex(sd);
  sb = slicebounds + slicetype;
  sb->label = &(sd->label);


  *errorcode = 0;
  if(sd->slice_filetype == SLICE_GEOM){
    patchdata *patchgeom;

    patchgeom = sd->patchgeom;
    GetSliceColors(patchgeom->geom_vals, patchgeom->geom_nvals, patchgeom->geom_ivals,
      smin, smax,
      nrgb_full, nrgb,
      sb->colorlabels, sb->colorvalues, sb->levels256,
      &sd->extreme_min, &sd->extreme_max, flag
    );
  }
  else{
    if(sd->qslicedata == NULL)return;
    GetSliceColors(sd->qslicedata, sd->nslicetotal, sd->slicelevel,
      smin, smax,
      nrgb_full, nrgb,
      sb->colorlabels, sb->colorvalues, sb->levels256,
      &sd->extreme_min, &sd->extreme_max, flag
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

  setvalmin=slicebounds[slicetype].dlg_setvalmin;
  setvalmax=slicebounds[slicetype].dlg_setvalmax;
  if(setvalmin==1){
    valmin=slicebounds[slicetype].dlg_valmin;
  }
  else{
    valmin=slicebounds[slicetype].data_valmin;
    slicebounds[slicetype].dlg_valmin=valmin;
  }
  if(setvalmax==1){
    valmax=slicebounds[slicetype].dlg_valmax;
  }
  else{
    valmax=slicebounds[slicetype].data_valmax;
    slicebounds[slicetype].dlg_valmax=valmax;
  }
  for(ii=0;ii<nslice_loaded;ii++){
    i = slice_loaded_list[ii];
    sd = sliceinfo + i;
    if(sd->slicefile_labelindex!=slicetype)continue;
    SetSliceColors(valmin,valmax,sd,1,errorcode);
    if(*errorcode!=0)return;
  }
  SliceBounds2Glui(slicetype);
}

/* ------------------ SliceCompare ------------------------ */

int SliceCompare( const void *arg1, const void *arg2 ){
  slicedata *slicei, *slicej;

  slicei = sliceinfo + *(int *)arg1;
  slicej = sliceinfo + *(int *)arg2;

  if(strcmp(slicei->label.longlabel,slicej->label.longlabel)<0)return -1;
  if(strcmp(slicei->label.longlabel,slicej->label.longlabel)>0)return 1;
  if(slicei->volslice==1&&slicej->volslice==0)return -1;
  if(slicei->volslice==0&&slicej->volslice==1)return 1;
  if(slicei->idir<slicej->idir)return -1;
  if(slicei->idir>slicej->idir)return 1;
  if(slicei->position_orig<slicej->position_orig)return -1;
  if(slicei->position_orig>slicej->position_orig)return 1;
  if(slicei->slice_filetype<slicej->slice_filetype)return -1;
  if(slicei->slice_filetype>slicej->slice_filetype)return 1;
  if(slicei->slcf_index<slicej->slcf_index)return -1;
  if(slicei->slcf_index>slicej->slcf_index)return 1;
  return 0;
}

/* ------------------ VSliceCompare ------------------------ */

int VSliceCompare(const void *arg1, const void *arg2){
  vslicedata *vslicei, *vslicej;

  vslicei = vsliceinfo+*(int *)arg1;
  vslicej = vsliceinfo+*(int *)arg2;
  return SliceCompare(&(vslicei->ival), &(vslicej->ival));
}

/* ------------------ UpdateSliceMenuShow ------------------------ */

void UpdateSliceMenuShow(void){
  int i;

  for(i=0;i<nsliceinfo;i++){
    slicedata *sd;

    sd = sliceinfo + i;
    sd->menu_show=1;
    sd->constant_color = NULL;
  }
}

/* ------------------ GetMSliceDir ------------------------ */

char *GetMSliceDir(multislicedata *mslicei){
  char *cdir;
  int i;
  float deltamin;

  deltamin = 0.0;
  for(i = 0; i<mslicei->nslices; i++){
    slicedata *slicei;
    meshdata *meshi;
    float delta;

    slicei = sliceinfo+mslicei->islices[i];
    meshi = meshinfo+slicei->blocknumber;
    if(slicei->idir==0){
      return slicei->cdir;
    }
    delta = meshi->dcell3[slicei->idir-1];
    if(i==0||delta<deltamin){
      if(i==0){
        deltamin = delta;
      }
      else{
        deltamin = MIN(delta, deltamin);
      }
      cdir = slicei->cdir;
    }

  }
  return cdir;
}

/* ------------------ UpdateSliceMenuLabels ------------------------ */

void UpdateSliceMenuLabels(void){
  int i;
  char label[128];
  multislicedata *mslicei;
  slicedata *sd,*sdold;

  UpdateSliceMenuShow();
  if(nsliceinfo>0){
    char *cdir;

    mslicei = multisliceinfo;
    sd = sliceinfo + sliceorderindex[0];
    cdir = GetMSliceDir(mslicei);
    STRCPY(mslicei->menulabel, cdir);
    STRCPY(sd->menulabel,mslicei->menulabel);

    STRCPY(mslicei->menulabel2,sd->label.longlabel);
    STRCAT(mslicei->menulabel2,", ");
    STRCAT(mslicei->menulabel2,sd->menulabel);

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
    if(sd->compression_type==COMPRESSED_RLE){
      STRCAT(sd->menulabel," (RLE)");
    }
    for(i=1;i<nsliceinfo;i++){
      sdold = sliceinfo + sliceorderindex[i - 1];
      sd = sliceinfo + sliceorderindex[i];
      cdir = GetMSliceDir(mslicei);
      STRCPY(sd->menulabel, cdir);
      if(NewMultiSlice(sdold,sd)==1){
        mslicei++;
        cdir = GetMSliceDir(mslicei);
        STRCPY(mslicei->menulabel, cdir);
        STRCPY(mslicei->menulabel2,sd->label.longlabel);
        STRCAT(mslicei->menulabel2,", ");
        STRCAT(mslicei->menulabel2, cdir);
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
      if(sd->compression_type==COMPRESSED_RLE){
        STRCAT(sd->menulabel," (RLE)");
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

/* ------------------ UpdateMeshSkip ------------------------ */

void UpdateMeshSkip(meshdata *meshi, int skip, int dir){
  meshdata *left, *front, *down;
  int beg, i, n;

  switch(dir){
  case 0:
    if(meshi->n_imap > 0)return;
    left = meshi->skip_nabors[MLEFT];
    beg = 0;
    if(left != NULL){
      UpdateMeshSkip(left, skip, dir);
      beg = left->imap[left->n_imap - 1] + skip - left->ibar;
      beg = beg % skip;
      if(beg == 0) beg = skip;
    }
    n = 0;
    for(i = beg; i < meshi->ibar + 1; i += skip){
      meshi->imap[n++] = i;
    }
    meshi->n_imap = n;
    break;
  case 1:
    if(meshi->n_jmap > 0)return;
    front = meshi->skip_nabors[MFRONT];
    beg = 0;
    if(front != NULL){
      UpdateMeshSkip(front, skip, dir);
      beg = front->jmap[front->n_jmap - 1] + skip - front->jbar;
      beg = beg % skip;
      if(beg == 0) beg = skip;
    }
    n = 0;
    for(i = beg; i < meshi->jbar + 1; i += skip){
      meshi->jmap[n++] = i;
    }
    meshi->n_jmap = n;
    break;
  case 2:
    if(meshi->n_kmap > 0)return;
    down = meshi->skip_nabors[MDOWN];
    beg = 0;
    if(down != NULL){
      UpdateMeshSkip(down, skip, dir);
      beg = down->kmap[down->n_kmap - 1] + skip - down->kbar;
      beg = beg % skip;
      if(beg == 0) beg = skip;
    }
    n = 0;
    for(i = beg; i < meshi->kbar + 1; i += skip){
      meshi->kmap[n++] = i;
    }
    meshi->n_kmap = n;
    break;
  default:
    ASSERT(FFALSE);
    break;
  }

}

/* ------------------ UpdateAllMeshSkips ------------------------ */

void UpdateAllMeshSkips(int skip){
  int i;

  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    meshi->n_imap = 0;
    meshi->n_jmap = 0;
    meshi->n_kmap = 0;
  }
  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    UpdateMeshSkip(meshi, skip, 0);
    UpdateMeshSkip(meshi, skip, 1);
    UpdateMeshSkip(meshi, skip, 2);
  }
}

/* ------------------ UpdateVectorSkipDefault ------------------------ */

void UpdateVectorSkipDefault(void){
  int i;

  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;
    int ii, jj, kk;

    slicei = sliceinfo + i;
    if(slicei->loaded == 0)continue;
    for(ii = slicei->is1; ii <= slicei->is2; ii++){
      slicei->imap[ii - slicei->is1] = ii;
    }
    slicei->n_imap = slicei->is2 + 1 - slicei->is1;

    for(jj = slicei->js1; jj <= slicei->js2; jj++){
      slicei->jmap[jj - slicei->js1] = jj;
    }
    slicei->n_jmap = slicei->js2 + 1 - slicei->js1;

    for(kk = slicei->ks1; kk <= slicei->ks2; kk++){
      slicei->kmap[kk - slicei->ks1] = kk;
    }
    slicei->n_kmap = slicei->ks2 + 1 - slicei->ks1;
  }
}

/* ------------------ UpdateVectorSkipNonUniform ------------------------ */

void UpdateVectorSkipNonUniform(slicedata *slicei, int factor_x, int factor_y, int factor_z){
  meshdata *slicemesh;
  int ii, jj, kk;

  if(slicei->loaded == 0)return;
  slicemesh = meshinfo + slicei->blocknumber;

  int n = 0;
  for(ii = 0; ii < slicemesh->n_imap; ii+=factor_x){
    int i;

    i = slicemesh->imap[ii];
    if(slicei->is1<=i&&i<=slicei->is2)slicei->imap[n++] = i;
  }
  slicei->n_imap = n;

  n = 0;
  for(jj = 0; jj < slicemesh->n_jmap; jj+=factor_y){
    int j;

    j = slicemesh->jmap[jj];
    if(slicei->js1<=j&&j<=slicei->js2)slicei->jmap[n++] = j;
  }
  slicei->n_jmap = n;

  n = 0;
  for(kk = 0; kk < slicemesh->n_kmap; kk+=factor_z){
    int k;

    k = slicemesh->kmap[kk];
    if(slicei->ks1<=k&&k<=slicei->ks2)slicei->kmap[n++] = k;
  }
  slicei->n_kmap = n;
}

/* ------------------ UpdateVectorSkipUniform ------------------------ */

void UpdateVectorSkipUniform(int skip){
  int i;

  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;
    meshdata *slicemesh;
    float mesh_dx, mesh_dy, mesh_dz;
    int factor_i, factor_j, factor_k;

    slicei = sliceinfo + i;
    if(slicei->loaded == 0)continue;
    slicemesh = meshinfo + slicei->blocknumber;

    mesh_dx = slicemesh->xplt_orig[1] - slicemesh->xplt_orig[0];
    mesh_dy = slicemesh->yplt_orig[1] - slicemesh->yplt_orig[0];
    mesh_dz = slicemesh->zplt_orig[1] - slicemesh->zplt_orig[0];
    factor_i = MAX(1, max_dx / mesh_dx + 0.5);
    factor_j = MAX(1, max_dy / mesh_dy + 0.5);
    factor_k = MAX(1, max_dz / mesh_dz + 0.5);

    UpdateVectorSkipNonUniform(slicei, factor_i, factor_j, factor_k);
  }
}

    /* ------------------ UpdateVectorSkip ------------------------ */


void UpdateVectorSkip(int skip){
  int i;

  if(nsliceinfo > 0){
    UpdateAllMeshSkips(skip);
  }
  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;
    meshdata *slicemesh;

    slicei = sliceinfo + i;
    if(slicei->loaded == 0)continue;
    slicemesh = meshinfo + slicei->blocknumber;
    if(slicei->imap == NULL){
      int *imap;

      NewMemory(( void ** )&imap, (slicemesh->ibar + 1) * sizeof(int));
      slicei->imap = imap;
      slicei->n_imap = 0;
    }
    if(slicei->jmap == NULL){
      int *jmap;

      NewMemory(( void ** )&jmap, (slicemesh->jbar + 1) * sizeof(int));
      slicei->jmap = jmap;
      slicei->n_jmap = 0;
    }
    if(slicei->kmap == NULL){
      int *kmap;

      NewMemory(( void ** )&kmap, (slicemesh->kbar + 1) * sizeof(int));
      slicei->kmap = kmap;
      slicei->n_kmap = 0;
    }
  }
  if(vec_uniform_spacing == 1){
    UpdateVectorSkipUniform(skip);
  }
  else{
    if(skip == 1){
      UpdateVectorSkipDefault();
    }
    else{
      for(i = 0; i < nsliceinfo; i++){
        UpdateVectorSkipNonUniform(sliceinfo + i, 1, 1, 1);
      }
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

    STRCPY(mvslicei->menulabel,  sd->cdir);
    STRCPY(mvslicei->menulabel2, sd->label.longlabel);
    STRCAT(mvslicei->menulabel2, ", ");
    STRCAT(mvslicei->menulabel2, sd->cdir);

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
      STRCPY(vsd->menulabel,sd->cdir);
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
  if(strcmp(sd->label.longlabel, sdold->label.longlabel)==0&&sd->slcf_index==sdold->slcf_index)return 0;
  return 1;
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
  meshdata *meshi;


  if(flag==FIND_DUPLICATES&&slicedup_option==SLICEDUP_KEEPALL)return 0;
  slicei = sliceinfo+mslicei->islices[ii];
  meshi = meshinfo+slicei->blocknumber;
  xyzmini = slicei->xyz_min;
  xyzmaxi = slicei->xyz_max;
  for(jj=0;jj<mslicei->nslices;jj++){ // identify duplicate slices
    slicedata *slicej;
    float *xyzminj, *xyzmaxj;
    meshdata *meshj;

    slicej = sliceinfo + mslicei->islices[jj];
    meshj = meshinfo+slicej->blocknumber;
    if(slicej==slicei||slicej->skipdup==1)continue;
    if(slicei->above_ground_level>0.0&&slicej->above_ground_level>0.0){
      if(slicei->ks1==0&&slicej->ks2==meshj->kbar)return 0;
      if(slicej->ks1==0&&slicei->ks2==meshi->kbar)return 0;
    }
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
  meshdata *meshi;


  if(vectorslicedup_option==SLICEDUP_KEEPALL)return 0;
  vslicei = vsliceinfo + mvslicei->ivslices[i];
  slicei = sliceinfo + vslicei->ival;
  meshi = meshinfo+slicei->blocknumber;
  xyzmini = slicei->xyz_min;
  xyzmaxi = slicei->xyz_max;
  for(jj=0;jj<mvslicei->nvslices;jj++){ // identify duplicate slices
    vslicedata *vslicej;
    slicedata *slicej;
    float *xyzminj, *xyzmaxj;
    meshdata *meshj;

    vslicej = vsliceinfo + mvslicei->ivslices[jj];
    slicej = sliceinfo + vslicej->ival;
    meshj = meshinfo+slicej->blocknumber;
    if(slicej==slicei||slicej->skipdup==1)continue;
    if(slicei->above_ground_level>0.0&&slicej->above_ground_level>0.0){
      if(slicei->ks1==0&&slicej->ks2==meshj->kbar)return 0;
      if(slicej->ks1==0&&slicei->ks2==meshi->kbar)return 0;
    }
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
      slicei->skipdup =0;
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
      slicei->skipdup = IsSliceDuplicate(mslicei,ii, FIND_DUPLICATES);
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
    if(slicei->slice_filetype != SLICE_CELL_CENTER&&strcmp(slicei->label.longlabel, "CARBON DIOXIDE VOLUME FRACTION") != 0)continue;
    if(slicei->slice_filetype == SLICE_CELL_CENTER&&strcmp(slicei->label.longlabel, "CARBON DIOXIDE VOLUME FRACTION(cell centered)") != 0)continue;
    fedi->co2_index = i;
    for(j = 0; j < nsliceinfo; j++){
      slicedata *slicej;

      slicej = sliceinfo + j;
      if(slicei->slice_filetype != SLICE_CELL_CENTER&&strcmp(slicej->label.longlabel, "CARBON MONOXIDE VOLUME FRACTION") != 0)continue;
      if(slicei->slice_filetype == SLICE_CELL_CENTER&&strcmp(slicej->label.longlabel, "CARBON MONOXIDE VOLUME FRACTION(cell centered)") != 0)continue;
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
      if(slicei->slice_filetype != SLICE_CELL_CENTER&&strcmp(slicej->label.longlabel, "OXYGEN VOLUME FRACTION") != 0)continue;
      if(slicei->slice_filetype == SLICE_CELL_CENTER&&strcmp(slicej->label.longlabel, "OXYGEN VOLUME FRACTION(cell centered)") != 0)continue;
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
    sd->iis1  = co2->iis1;
    sd->iis2  = co2->iis2;
    sd->jjs1  = co2->jjs1;
    sd->jjs2  = co2->jjs2;
    sd->kks1  = co2->kks1;
    sd->kks2  = co2->kks2;
    sd->plotx = co2->plotx;
    sd->ploty = co2->ploty;
    sd->plotz = co2->plotz;

    nn_slice = nsliceinfo + i;

    sd->is_fed = 1;
    sd->slcf_index = co2->slcf_index;
    sd->uvw = 0;
    sd->fedptr = fedi;
    sd->slice_filetype = co2->slice_filetype;
    if(sd->slice_filetype == SLICE_CELL_CENTER){
      SetLabels(&(sd->label), "Fractional effective dose(cell centered)", "FED", " ");
    }
    else{
      SetLabels(&(sd->label), "Fractional effective dose", "FED", " ");
    }

    sd->imap = NULL;
    sd->jmap = NULL;
    sd->kmap = NULL;
    sd->n_imap = 0;
    sd->n_jmap = 0;
    sd->n_kmap = 0;
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
    sd->nline_contours = 0;
    sd->line_contours = NULL;
    sd->menu_show = 1;
    sd->constant_color = NULL;
    sd->histograms = NULL;
    sd->nhistograms = 0;
    sd->have_bound_file = 0;

    strcpy(filename_base, fedi->co->file);
    ext = strrchr(filename_base, '.');
    *ext = 0;
    strcat(filename_base, "_fed.sf");

    filename = GetFileName(smokeview_scratchdir, filename_base, NOT_FORCE_IN_DIR);

    NewMemory((void **)&fedi->fed_slice->reg_file, strlen(filename) + 1);
    strcpy(fedi->fed_slice->reg_file, filename);

    NewMemory((void **)&sd->reg_file, strlen(filename)+1);
    strcpy(sd->reg_file, filename);

    NewMemory((void **)&sd->bound_file, strlen(filename)+4+1);
    strcpy(sd->bound_file, filename);
    strcat(sd->bound_file, ".bnd");

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
      nmemory_ids++;
      isoi->memory_id = nmemory_ids;
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
      filename = GetFileName(smokeview_scratchdir, filename_base, NOT_FORCE_IN_DIR);
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
      if((slicej->slice_filetype == SLICE_CELL_CENTER&&slicei->slice_filetype != SLICE_CELL_CENTER) ||
         (slicej->slice_filetype != SLICE_CELL_CENTER&&slicei->slice_filetype == SLICE_CELL_CENTER))continue;
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
      if((slicej->slice_filetype == SLICE_CELL_CENTER&&slicei->slice_filetype != SLICE_CELL_CENTER) ||
         (slicej->slice_filetype != SLICE_CELL_CENTER&&slicei->slice_filetype == SLICE_CELL_CENTER))continue;
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
        getslicefiledirection(is1, &is2, &iis1, &iis2, js1, &js2, ks1, &ks2, &idir, &joff, &koff,&volslice);
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
    else if(sd->compression_type!=UNCOMPRESSED){
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

      strcpy(sd->cdir,"");
      position=-999.0;
      if(sd->is1==sd->is2||(sd->js1!=sd->js2&&sd->ks1!=sd->ks2)){
        sd->idir=1;
        position = meshi->xplt_orig[is1];
        if(sd->slice_filetype==SLICE_CELL_CENTER){
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
          sprintf(sd->cdir, "X=%f", position);
        }
        else{
          sd->dplane_min = meshi->dplane_min[0];
          sd->dplane_max = meshi->dplane_max[0];
          sprintf(sd->cdir, "3D slice");
        }
      }
      if(sd->js1==sd->js2){
        sd->dplane_min = meshi->dplane_min[2];
        sd->dplane_max = meshi->dplane_max[2];

        sd->idir = 2;
        position = meshi->yplt_orig[js1];
        if(sd->slice_filetype==SLICE_CELL_CENTER){
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
        sprintf(sd->cdir,"Y=%f",position);
      }
      if(sd->ks1==sd->ks2){
        sd->dplane_min = meshi->dplane_min[3];
        sd->dplane_max = meshi->dplane_max[3];

        sd->idir = 3;
        position = meshi->zplt_orig[ks1];
        if(sd->slice_filetype==SLICE_CELL_CENTER){
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
        if(sd->slice_filetype==SLICE_TERRAIN){
          position=sd->above_ground_level;
          sprintf(sd->cdir,"AGL=%f",position);
        }
        else{
          sprintf(sd->cdir,"Z=%f",position);
        }
      }
      sd->position_orig=position;
      TrimZeros(sd->cdir);
    }
    {
      float *xplt, *yplt, *zplt;
      float *xyz_min, *xyz_max;

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
      for(i=1;i<nsliceinfo;i++){
        slicedata *sdold;

        sdold = sliceinfo + sliceorderindex[i - 1];
        sd = sliceinfo + sliceorderindex[i];
        mslicei->autoload=0;
        if(NewMultiSlice(sdold,sd)==1){
          nmultisliceinfo++;
          mslicei++;
          mslicei->nslices=0;
          mslicei->islices=NULL;
          NewMemory((void **)&mslicei->islices,sizeof(int)*nsliceinfo);
        }
        mslicei->nslices++;
        mslicei->islices[mslicei->nslices-1]=sliceorderindex[i];
      }
    }
    have_multislice = 0;
    //if(nmultisliceinfo>0&&nsliceinfo>0&&nmultisliceinfo+nfedinfo<nsliceinfo)have_multislice = 1;
    if(nmultisliceinfo>0)have_multislice = 1; // use multi slice for all cases
  }
  for(i = 0; i < nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo + i;
    slicei->mslice = NULL;
    slicei->skipdup = 0;
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
      if(ii==0){
        mslicei->slice_filetype = slicei->slice_filetype;
      }
      else{
        if(slicei->slice_filetype!=SLICE_CELL_CENTER&&slicei->slice_filetype!=SLICE_NODE_CENTER){
          mslicei->slice_filetype = slicei->slice_filetype;
        }
      }
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

  max_dx = meshinfo->xplt_orig[1] - meshinfo->xplt_orig[0];
  max_dy = meshinfo->yplt_orig[1] - meshinfo->yplt_orig[0];
  max_dz = meshinfo->zplt_orig[1] - meshinfo->zplt_orig[0];
  for(i = 1; i<nmeshes; i++){
    meshdata *meshi;
    float *xplt, *yplt, *zplt;

    meshi = meshinfo+i;
    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;
    max_dx = MAX(max_dx, xplt[1]-xplt[0]);
    max_dy = MAX(max_dy, yplt[1]-yplt[0]);
    max_dz = MAX(max_dz, zplt[1]-zplt[0]);
  }

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
#ifdef pp_GEOM_SLICE_VECTORS
    if(sdi->slice_filetype==SLICE_GEOM)continue;
#endif
    if(strncmp(sdi->label.shortlabel,"U-VEL",5)==0){
       sdi->vec_comp=1;
       continue;
    }
    if(strncmp(sdi->label.shortlabel,"V-VEL",5)==0){
      sdi->vec_comp=2;
      continue;
    }
    if(strncmp(sdi->label.shortlabel,"W-VEL",5)==0){
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
    vd->vslice_filetype=sdi->slice_filetype;
    if(vd->vslice_filetype==SLICE_CELL_CENTER){
      for(j=0;j<nsliceinfo;j++){
        slicedata *sdj;

        sdj = sliceinfo+j;
        if(sdj->slice_filetype!=SLICE_CELL_CENTER)continue;
        if(sdi->blocknumber!=sdj->blocknumber)continue;
        if(sdi->is1!=sdj->is1||sdi->is2!=sdj->is2||sdi->js1!=sdj->js1)continue;
        if(sdi->js2!=sdj->js2||sdi->ks1!=sdj->ks1||sdi->ks2!=sdj->ks2)continue;
        if(sdj->vec_comp==1)vd->iu=j;
        if(sdj->vec_comp==2)vd->iv=j;
        if(sdj->vec_comp==3)vd->iw=j;
      }
    }
    else if(vd->vslice_filetype == SLICE_GEOM){
      for(j=0;j<nsliceinfo;j++){
        slicedata *sdj;

        sdj = sliceinfo+j;
        if(sdj->slice_filetype!=SLICE_GEOM)continue;
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
        if (sdj->slice_filetype == SLICE_CELL_CENTER|| sdj->slice_filetype == SLICE_GEOM)continue;
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
      vslicei->reload = 0;
    }
  }
  have_multivslice = 0;
  //if(nvsliceinfo > 0 && nmultivsliceinfo < nvsliceinfo)have_multivslice = 1;
  if(nvsliceinfo>0)have_multivslice = 1; // use multi vslice for all cases

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
      if((slicej->slice_filetype==SLICE_CELL_CENTER&&slicei->slice_filetype!=SLICE_CELL_CENTER)||
         (slicej->slice_filetype!=SLICE_CELL_CENTER&&slicei->slice_filetype==SLICE_CELL_CENTER))continue;
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
    constval += slice_dz;

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

/* ------------------ GetSliceDataBounds ------------------------ */

void GetSliceDataBounds(slicedata *sd, float *pmin, float *pmax){
  float *pdata;
  int ndata;
  int n, i, j, k;
  int first=1;

  int istep;
  int nx, ny, nxy, ibar, jbar;
  int ntimes;
  char *iblank_node, *iblank_cell, *slice_mask0;
  meshdata *meshi;

  if(sd->slice_filetype == SLICE_GEOM){
    pdata = sd->patchgeom->geom_vals;
    ndata = sd->patchgeom->geom_nvals;
    if(pdata==NULL||ndata==0){
      *pmin = 1.0;
      *pmax = 0.0;
      return;
    }
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
  for(n=0;n<sd->nslicei*sd->nslicej*sd->nslicek;n++){
    slice_mask0[n]=0;
  }
  n=-1;
  for(i=0;i<sd->nslicei;i++){
    for(j=0;j<sd->nslicej;j++){
      for(k=0;k<sd->nslicek;k++){
        n++;
        if(sd->slice_filetype==SLICE_CELL_CENTER&&((k==0&&sd->nslicek!=1)||(j==0&&sd->nslicej!=1)||(i==0&&sd->nslicei!=1)))continue;
        if(show_slice_in_obst == ONLY_IN_GAS){
          if(sd->slice_filetype!=SLICE_CELL_CENTER&& iblank_node!=NULL){
            if(iblank_node[IJKNODE(sd->is1+i, sd->js1+j, sd->ks1+k)]==SOLID)continue;
          }
          if(sd->slice_filetype==SLICE_CELL_CENTER&& iblank_cell!=NULL){
            if(iblank_cell[IJKCELL(sd->is1+i-1, sd->js1+j-1, sd->ks1+k-1)]==EMBED_YES)continue;
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

    for(i=0;i<sd->nslicei;i++){
      for(j=0;j<sd->nslicej;j++){
        for(k=0;k<sd->nslicek;k++){
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

  /* ------------------ TimeAverageData ------------------------ */

int TimeAverageData(float *data_out, float *data_in, int ndata, int data_per_timestep, float *times_local, int ntimes_local, float average_time){

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

/* ------------------ GetSliceZlibRLEData ------------------------ */

int GetSliceZlibRLEData(char *file, int compression_type,
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


  if(compression_type==COMPRESSED_RLE){ // written out in fortran, an extra 4 bytes before and after each record
    FSEEK(stream, 4, SEEK_CUR);fread(&endian, 4, 1, stream);    FSEEK(stream, 4, SEEK_CUR);
    FSEEK(stream, 4, SEEK_CUR);fread(&completion, 4, 1, stream);fread(&fileversion, 4, 1, stream);fread(&version, 4, 1, stream);FSEEK(stream, 4, SEEK_CUR);
    FSEEK(stream, 4, SEEK_CUR);fread(minmax, 4, 2, stream);FSEEK(stream, 4, SEEK_CUR);
    FSEEK(stream, 4, SEEK_CUR);fread(ijkbar, 4, 6, stream);FSEEK(stream, 4, SEEK_CUR);
  }
  else{
    fread(&endian, 4, 1, stream);

    fread(&completion, 4, 1, stream);
    fread(&fileversion, 4, 1, stream);
    fread(&version, 4, 1, stream);
    fread(minmax, 4, 2, stream);
    fread(ijkbar, 4, 6, stream);
  }
  if(completion == 0){
    fclose(stream);
    return 0;
  }

  count = 0;
  ns = 0;
  while(!feof(stream)){
    float ttime;
    int nncomp;

    if(compression_type==COMPRESSED_RLE)FSEEK(stream, 4, SEEK_CUR);
    fread(&ttime, 4, 1, stream);
    if(compression_type==COMPRESSED_RLE)FSEEK(stream, 4, SEEK_CUR);
    if(compression_type==COMPRESSED_RLE){
      FSEEK(stream, 4, SEEK_CUR);
      FSEEK(stream, 4, SEEK_CUR); // original
      fread(&nncomp, 4, 1, stream);
      FSEEK(stream, 4, SEEK_CUR);
    }
    else{
      fread(&nncomp, 4, 1, stream);
    }
    if((count++%sliceskip != 0) || (set_tmin == 1 && ttime<tmin_local) || (set_tmax == 1 && ttime>tmax_local)){
      if(compression_type==COMPRESSED_RLE)FSEEK(stream, 4, SEEK_CUR);
      FSEEK(stream, nncomp, SEEK_CUR);
      if(compression_type==COMPRESSED_RLE)FSEEK(stream, 4, SEEK_CUR);
      continue;
    }
    times_local[ns++] = ttime;
    compindex[ns].offset = compindex[ns - 1].offset + nncomp;
    compindex[ns - 1].size = nncomp;

    if(compression_type==COMPRESSED_RLE)FSEEK(stream, 4, SEEK_CUR);
    fread(cd, 1, nncomp, stream);
    if(compression_type==COMPRESSED_RLE)FSEEK(stream, 4, SEEK_CUR);
    cd += nncomp;
    if(ns >= nsliceframes || cd - compressed_data >= ncompressed)break;
  }
  fclose(stream);
  return cd - compressed_data;
}

/* ------------------ GetSliceCompressedData ------------------------ */

int GetSliceCompressedData(char *file, int compression_type,
  int set_tmin, int set_tmax, float tmin_local, float tmax_local, int ncompressed, int sliceskip, int nsliceframes,
  float *times_local, unsigned char *compressed_data, compdata *compindex, float *valmin, float *valmax){
  int returnval;

  returnval = GetSliceZlibRLEData(file, compression_type, set_tmin, set_tmax, tmin_local, tmax_local, ncompressed, sliceskip, nsliceframes,
    times_local, compressed_data, compindex, valmin, valmax);
  return returnval;
}

#ifdef pp_GPU

/* ------------------ InitSlice3DTexture ------------------------ */

void InitSlice3DTexture(meshdata *meshi){
#ifdef pp_WINGPU
  GLint border_size = 0;
#endif
  GLsizei nx, ny, nz;

  SNIFF_ERRORS("InitSlice3DTexture: start");
  if(verbose_output==1)PRINTF("Defining 3d slice textures for %s ...", meshi->label);
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
#ifdef pp_WINGPU
  if(meshi->slice3d_texture_buffer == NULL){
    int i;

    NewMemory((void **)&meshi->slice3d_texture_buffer, nx*ny*nz * sizeof(float));
    for(i = 0; i < nx*ny*nz; i++){
      meshi->slice3d_texture_buffer[i] = 0.0;
    }
  }
  glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, nx, ny, nz, border_size, GL_RED, GL_FLOAT, meshi->slice3d_texture_buffer);
#endif
  if(meshi->slice3d_c_buffer == NULL){
    NewMemory((void **)&meshi->slice3d_c_buffer, nx*ny*nz * sizeof(float));
  }


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
  if(verbose_output==1)PRINTF("completed");
  if(verbose_output==1)PRINTF("\n");
  FFLUSH();
}
#endif

/* ------------------ GetSliceSizes ------------------------ */

void GetSliceSizes(slicedata *sd, const char *slicefilenameptr, int time_frame, int *nsliceiptr, int *nslicejptr, int *nslicekptr, int *ntimesptr, int tload_step_arg,
  int *errorptr, int settmin_s_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg, int *headersizeptr, int *framesizeptr){

  int ip1, ip2, jp1, jp2, kp1, kp2;
  int iip1, iip2;
  int nxsp, nysp, nzsp;

  float timeval, time_max;
  int idir, joff, koff, volslice;
  int count,countskip;
  FILEBUFFER *SLICEFILE=NULL;
  int ijk[6];
  int returncode=0;

  *errorptr = 0;
  *ntimesptr = 0;

  if(SLICEFILE==NULL){
    SLICEFILE = FOPEN_SLICE(slicefilenameptr, "rb");
  }
  if(SLICEFILE==NULL){
    *errorptr = 1;
    return;
  }

  *headersizeptr = 3*(4+30+4);
  FSEEK_SLICE(SLICEFILE, *headersizeptr, SEEK_CUR);

  FORT_SLICEREAD(ijk, 6, SLICEFILE);
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

  getslicefiledirection(ip1, &ip2, &iip1, &iip2, jp1, &jp2, kp1, &kp2, &idir, &joff, &koff, &volslice);
  *nsliceiptr = nxsp;
  *nslicejptr = nysp + joff;
  *nslicekptr = nzsp + koff;

  *framesizeptr = 4*(1+nxsp*nysp*nzsp)+16;

  count = -1;
  countskip = -1;
  time_max = -1000000.0;
  for(;;){
    int loadframe;

    loadframe = 0;
    FORT_SLICEREAD(&timeval, 1, SLICEFILE);
    if(returncode==0)break;
    if((settmin_s_arg!=0&&timeval<tmin_s_arg)||timeval<=time_max){
    }
    else{
      if(time_frame==ALL_FRAMES)loadframe = 1;
      time_max = timeval;
    }
    if(settmax_s_arg!=0&&timeval>tmax_s_arg){
      FCLOSE_SLICE(SLICEFILE);
      return;
    }
    FSEEK_SLICE(SLICEFILE, *framesizeptr-12, SEEK_CUR);
    if(count%tload_step_arg==0){
      countskip++;
      if(time_frame>=0&&time_frame==countskip){
        *ntimesptr = *ntimesptr+1;
        FCLOSE_SLICE(SLICEFILE);
        *errorptr = 0;
        return;
      }
    }
    else{
      loadframe = 0;
    }
    if(loadframe==1)*ntimesptr = *ntimesptr+1;
    count++;
  }
  *errorptr = 0;
  FCLOSE_SLICE(SLICEFILE);
}

/* ------------------ GetSliceData ------------------------ */

FILE_SIZE GetSliceData(slicedata *sd, const char *slicefilename, int time_frame, int *is1ptr, int *is2ptr, int *js1ptr, int *js2ptr, int *ks1ptr, int *ks2ptr, int *idirptr,
  float *qminptr, float *qmaxptr, float *qdataptr, float *timesptr, int ntimes_old_arg, int *ntimesptr,
  int tload_step_arg, int settmin_s_arg, int settmax_s_arg, float tmin_s_arg, float tmax_s_arg
){

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
  FILEBUFFER *stream=NULL;
  int returncode=0;
  float *qq;
  int nx, ny, nxy;
  int count_timeframe;

  joff = 0;
  koff = 0;
  file_size = 0;

  if(stream==NULL){
    stream = FOPEN_SLICE(slicefilename,"rb");
  }
  if(stream==NULL){
    printf(" the slice file %s does not exist\n", slicefilename);
    nsteps = 0;
    return 0;
  }

  nsteps = 0;
  FSEEK_SLICE(stream, 3*(4+30+4), SEEK_CUR);

  FORT_SLICEREAD(ijk, 6, stream);
  if(returncode==0){
    FCLOSE_SLICE(stream);
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

  getslicefiledirection(*is1ptr, is2ptr, &iis1, &iis2, *js1ptr, js2ptr, *ks1ptr, ks2ptr, idirptr, &joff, &koff, &volslice);
  sd->iis1 = *is1ptr;
  NewMemory((void **)&qq, nxsp*(nysp+joff)*(nzsp+koff)*sizeof(float));

  count = -1;
  time_max = -1000000.0;

  if(time_frame>0){
    int size;

    size = 4+4+4;                     // time
    size += (4+4*nxsp*nysp*nzsp+4);   // slice data
    size *= time_frame;               // number of steps to skip over
    FSEEK_SLICE(stream, size, SEEK_CUR);
  }
  else{
    if(*ntimesptr!=ntimes_old_arg&&ntimes_old_arg>0){
      int size;

      size = 0;
      for(i = 0; i<ntimes_old_arg; i++){
        size += 4+4+4;
        size += 4+4*nxsp*nysp*nzsp+4;
      }
      FSEEK_SLICE(stream, size, SEEK_CUR);
      nsteps = ntimes_old_arg;
    }
  }
  count_timeframe = 0;
  for(;;){
    if(time_frame>=0&&count_timeframe==1){
      count_timeframe = 1;
      break;
    }
    FORT_SLICEREAD(&timeval, 1, stream);
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
    FORT_SLICEREAD(qq, nxsp*nysp*nzsp, stream);
    if(returncode==0||nsteps>=*ntimesptr)break;
    count++;
    if(count%tload_step_arg!=0)loadframe = 0;
    if(loadframe==0)continue;
    if(koff==1){
//  qq(1:nxsp, 1:nysp, 2) = qq(1:nxsp, 1:nysp, 1)
      for(j = 0;j<nysp;j++){
        float *qqto, *qqfrom;

        qqfrom = qq + IJKNODE(0,j,0);
        qqto = qq + IJKNODE(0,j,1);
        for(i = 0;i<nxsp;i++){
//        qq[IJKNODE(i, j, 1)] = qq[IJKNODE(i, j, 0)];
          if((sd->slice_filetype==SLICE_CELL_CENTER&&i!=0&&j!=0)||
             sd->slice_filetype!=SLICE_CELL_CENTER){
            *qminptr = MIN(*qminptr, *qqfrom);
            *qmaxptr = MAX(*qmaxptr, *qqfrom);
          }
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
          if((sd->slice_filetype==SLICE_CELL_CENTER&&i!=0&&k!=0)||
              sd->slice_filetype!=SLICE_CELL_CENTER){
            *qminptr = MIN(*qminptr, *qqfrom);
            *qmaxptr = MAX(*qmaxptr, *qqfrom);
          }
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
          if((sd->slice_filetype==SLICE_CELL_CENTER&&i!=0&&j!=0)||
              sd->slice_filetype!=SLICE_CELL_CENTER){
            *qminptr = MIN(*qminptr, *qqfrom);
            *qmaxptr = MAX(*qmaxptr, *qqfrom);
          }
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
          if((sd->slice_filetype==SLICE_CELL_CENTER&&i!=0&&k!=0)||
              sd->slice_filetype!=SLICE_CELL_CENTER){
            *qminptr = MIN(*qminptr, *qqfrom);
            *qmaxptr = MAX(*qmaxptr, *qqfrom);
          }
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
            if((sd->slice_filetype==SLICE_CELL_CENTER&&i!=0&&j!=0&&k!=0)||
                sd->slice_filetype!=SLICE_CELL_CENTER){
              *qminptr = MIN(*qminptr, *qqfrom);
              *qmaxptr = MAX(*qmaxptr, *qqfrom);
            }
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

  FCLOSE_SLICE(stream);
  FREEMEMORY(qq);
  return file_size;
}

/* ------------------ GetNSliceFrames ------------------------ */

int GetNSliceFrames(char *file, float *stime_min, float *stime_max){
  int is1, is2, js1, js2, ks1, ks2, error;

  int header_size = 3*(4+30+4)+4+6*4+4;
  FILE_SIZE file_size = GetFileSizeSMV(file);

  getsliceheader(file, &is1, &is2, &js1, &js2, &ks1, &ks2, &error);
  int frame_size = 12;
  frame_size += 8+(is2+1-is1)*(js2+1-js1)*(ks2+1-ks1)*4;
  int nframes = (file_size-header_size)/frame_size;

  if(*stime_min > *stime_max&&nframes>0){
    FILE *stream = fopen(file, "rb");
    if(stream!=NULL){
      fseek(stream, header_size, SEEK_SET);
      fseek(stream, 4, SEEK_CUR); fread(stime_min, sizeof(float), 1, stream); fseek(stream, 4, SEEK_CUR);
      *stime_max = *stime_min;
      if(nframes>1){
        fseek(stream, header_size + (nframes-1)*frame_size, SEEK_SET);
        fseek(stream, 4, SEEK_CUR); fread(stime_max, sizeof(float), 1, stream); fseek(stream, 4, SEEK_CUR);
      }
    }
    if(stream!=NULL)fclose(stream);
  }
  return nframes;
}

/* ------------------ HideSlices ------------------------ */

void HideSlices(char *longlabel){
  int i;
  int len;


  if(longlabel==NULL)return;
  len = strlen(longlabel);
  for(i = 0; i<nvsliceinfo; i++){
    vslicedata *vslicei;

    vslicei = vsliceinfo+i;
    if(vslicei->display==1){
      char *label2;
      int len2;

      label2 = sliceinfo[vslicei->ival].label.longlabel;
      len2 = strlen(label2);
      if(strncmp(label2, longlabel, MIN(len, len2))!=0){
        vslicei->display = 0;
      }
    }
  }
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;

    slicei = sliceinfo+i;
    if(slicei->display==1){
      char *label2;
      int len2;

      label2 = slicei->label.longlabel;
      len2 = strlen(label2);
      if(strncmp(label2, longlabel, MIN(len, len2))!=0){
        slicei->display = 0;
      }
    }
  }
}

/* ------------------ GetSliceTimes ------------------------ */

void GetSliceTimes(char *file, float *times, int ntimes){
  FILE *stream;
  int i;
  char buffer[256];

  stream = fopen(file, "r");
  if(stream == NULL){
    for(i = 0; i<ntimes; i++){
      times[i] = (float)i;
    }
  }
  else{
    for(i = 0; i<ntimes; i++){
      fgets(buffer, 255, stream);
      sscanf(buffer, "%f", times+i);
    }
    fclose(stream);
  }
}

/* ------------------ ReadSlice ------------------------ */

FILE_SIZE ReadSlice(const char *file, int ifile, int time_frame, float *time_value, int flag, int set_slicecolor, int *errorcode){
  float *xplt_local, *yplt_local, *zplt_local, offset, qmin, qmax, read_time, total_time;
  int blocknumber, error, i, ii, headersize, framesize, flag2 = 0;
  slicedata *sd;
  int ntimes_slice_old;

  SNIFF_ERRORS("ReadSlice: start");
  SetTimeState();
  vslicedata *vd;
  meshdata *meshi;

  updatemenu = 1;
  update_plot_label = 1;
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
  slicefilenumber = ifile;
  ASSERT(slicefilenumber>=0&&slicefilenumber<nsliceinfo);
  slicefilenum = ifile;
  histograms_defined = 0;
  sd = sliceinfo+slicefilenumber;

  blocknumber = sd->blocknumber;
  meshi = meshinfo + blocknumber;

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
          SliceBounds2Glui(slicefile_labelindex);
          UpdateAllSliceColors(slicefile_labelindex, errorcode);
        }
        else{
          UpdateAllSliceLabels(slicefile_labelindex, errorcode);
        }
      }
      UpdateUnitDefs();
      update_times = 1;
      RemoveSliceLoadstack(slicefilenumber);
      update_draw_hist = 1;
      PrintMemoryInfo;
      return 0;
    }

// load entire slice file (flag=LOAD) or
// load only portion of slice file written to since last time it was loaded (flag=RELOAD)

    if(sd->compression_type == UNCOMPRESSED){
      sd->ntimes_old = sd->ntimes;
      GetSliceSizes(sd, file, time_frame, &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, tload_step, &error,
                    use_tload_begin, use_tload_end, tload_begin, tload_end, &headersize, &framesize);
    }
    else if(sd->compression_type != UNCOMPRESSED){
      if(
        GetSliceHeader(sd->comp_file, sd->size_file, sd->compression_type,
          tload_step, use_tload_begin, use_tload_end, tload_begin, tload_end,
          &sd->nslicei, &sd->nslicej, &sd->nslicek, &sd->ntimes, &sd->ncompressed, &sd->valmin, &sd->valmax) == 0){
        ReadSlice("", ifile, time_frame, time_value, UNLOAD, set_slicecolor, &error);
        *errorcode = 1;
        return 0;
      }
    }
    if(sd->nslicei != 1 && sd->nslicej != 1 && sd->nslicek != 1){
      sd->volslice = 1;
      ReadVolSlice = 1;
    }
    if(error != 0){
      ReadSlice("", ifile, time_frame, time_value, UNLOAD, set_slicecolor, &error);
      *errorcode = 1;
      return 0;
    }
    if(use_tload_begin== 0 &&use_tload_end == 0 && sd->compression_type == UNCOMPRESSED){
      if(framesize <= 0){
        fprintf(stderr, "*** Error: frame size is 0 in slice file %s . \n", file);
        error = 1;
      }
      else{
        if(time_frame==ALL_FRAMES){
          sd->ntimes = (int)(GetFileSizeSMV(file)-headersize)/framesize;
          if(tload_step>1)sd->ntimes /= tload_step;
        }
      }
    }
    if(error != 0 || sd->ntimes<1){
      ReadSlice("", ifile, time_frame, time_value, UNLOAD, set_slicecolor, &error);
      *errorcode = 1;
      return 0;
    }
    if(time_frame==ALL_FRAMES){
      PRINTF("Loading %s(%s)", file, sd->label.shortlabel);
    }
    MEMSTATUS(1, &availmemory, NULL, NULL);
    START_TIMER(read_time);
    if(sd->compression_type != UNCOMPRESSED){
      int return_code;

      return_code = NewResizeMemory(sd->qslicedata_compressed, sd->ncompressed);
      if(return_code!=0)return_code = NewResizeMemory(sd->times, sizeof(float)*sd->ntimes);
      if(return_code!=0)return_code = NewResizeMemory(sd->compindex, sizeof(compdata)*(1+sd->ntimes));
      if(return_code==0){
        ReadSlice("", ifile, time_frame, time_value, UNLOAD, set_slicecolor, &error);
        *errorcode = 1;
        return 0;
      }
      return_code=GetSliceCompressedData(sd->comp_file, sd->compression_type,
        use_tload_begin, use_tload_end, tload_begin, tload_end, sd->ncompressed, tload_step, sd->ntimes,
        sd->times, sd->qslicedata_compressed, sd->compindex, &sd->globalmin, &sd->globalmax);
      if(return_code == 0){
        ReadSlice("", ifile, time_frame, time_value, UNLOAD,  set_slicecolor, &error);
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
        ReadSlice("", ifile, time_frame, time_value, UNLOAD, set_slicecolor, &error);
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
        return_filesize = GetSliceData(sd, file, time_frame, &sd->is1, &sd->is2, &sd->js1, &sd->js2, &sd->ks1, &sd->ks2, &sd->idir,
            &qmin, &qmax, sd->qslicedata, sd->times, ntimes_slice_old, &sd->ntimes,
            tload_step, use_tload_begin, use_tload_end, tload_begin, tload_end
          );
        file_size = (int)return_filesize;
        sd->valmin_smv = qmin;
        sd->valmax_smv = qmax;
        if(sd->have_bound_file==NO){
          if(WriteFileBounds(sd->bound_file, qmin, qmax)==1){
            sd->have_bound_file = YES;
            update_slicefile_bounds = 1;
          }
        }
      }
#ifdef pp_MEMDEBUG
      ASSERT(ValidPointer(sd->qslicedata, sizeof(float)*sd->nslicei*sd->nslicej*sd->nslicek*sd->ntimes));
#endif
    }
    if(time_value!=NULL&&sd->ntimes>0){
      *time_value = sd->times[0];
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
        sd->compression_type != UNCOMPRESSED ||
        TimeAverageData(sd->qslicedata, sd->qslicedata, ndata, data_per_timestep, sd->times, ntimes_local, slice_average_interval) == 1
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
    if(sd->compression_type != UNCOMPRESSED){
      if(NewMemory((void **)&sd->slicecomplevel, sd->nsliceijk * sizeof(unsigned char)) == 0){
        ReadSlice("", ifile, time_frame, time_value, UNLOAD,  set_slicecolor, &error);
        *errorcode = 1;
        return 0;
      }
    }
    else{
      int return_code;

      return_code = NewResizeMemory(sd->slicelevel, sd->nslicetotal*sizeof(int));
      if(return_code == 0){
        ReadSlice("", ifile, time_frame, time_value, UNLOAD,  set_slicecolor, &error);
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
  }
  else{
    qmin = sd->valmin;
    qmax = sd->valmax;
  }
  CheckMemory;

  sd->loaded = 1;
  if(sd->vloaded == 0){
    sd->display = 1;
    if(sd->uvw==0)HideSlices(sd->label.longlabel);
  }

  slicefile_labelindex = GetSliceBoundsIndex(sd);
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  if(sd->finalize==1){
    int set_valmin, set_valmax;

    update_slicefile_bounds = 0; // if set to 1, temporary fix to make sure bounds are always up to date
    update_slice2device = 1;
    if(update_slicefile_bounds==1){
      update_slicefile_bounds = 0;
      GetGlobalSliceBounds(sd->label.shortlabel);
      SetLoadedSliceBounds(NULL, 0);
    }
    GetMinMax(BOUND_SLICE, sd->label.shortlabel, &set_valmin, &qmin, &set_valmax, &qmax);
    if(set_valmin==BOUND_PERCENTILE_MIN||set_valmax==BOUND_PERCENTILE_MAX){
      cpp_boundsdata *bounds;

      bounds = GetBoundsData(BOUND_SLICE);
      ComputeLoadedSliceHist(bounds->label);
      MergeLoadedSliceHist(bounds->label, &(bounds->hist));
      if(bounds->hist!=NULL&&bounds->hist->defined==1){
        if(set_valmin==BOUND_PERCENTILE_MIN){
          GetHistogramValProc(bounds->hist, percentile_level_min, &qmin);
          SetMin(BOUND_SLICE, bounds->label, BOUND_PERCENTILE_MIN, qmin);
        }
        if(set_valmax==BOUND_PERCENTILE_MAX){
          GetHistogramValProc(bounds->hist, percentile_level_max, &qmax);
          SetMax(BOUND_SLICE, bounds->label, BOUND_PERCENTILE_MAX, qmax);
        }
      }
    }
#define BOUND_PERCENTILE_DRAW          120
    SliceBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
    colorbar_slice_min = qmin;
    colorbar_slice_max = qmax;
    UpdateUnitDefs();
    UpdateTimes();
    CheckMemory;

    //*** comment out following line to prevent crash when loading a slice when particles are loaded
    //if(flag!=RESETBOUNDS)update_research_mode=1;
    if(use_set_slicecolor==0||set_slicecolor==SET_SLICECOLOR){
      if(sd->compression_type==UNCOMPRESSED){
        for(i = 0; i<nsliceinfo; i++){
          slicedata *slicei;

          slicei = sliceinfo+i;
          if(slicei->loaded==0)continue;
          if(slicei->vloaded==0&&slicei->display==0)continue;
          if(slicei->slicefile_labelindex!=slicefile_labelindex)continue;
          slicei->globalmin = qmin;
          slicei->globalmax = qmax;
          slicei->valmin = qmin;
          slicei->valmax = qmax;
          slicei->valmin_data = qmin;
          sd->valmax_data = qmax;
          for(ii = 0; ii<256; ii++){
            slicei->qval256[ii] = (qmin*(255 - ii) + qmax*ii) / 255;
          }
          SetSliceColors(qmin, qmax, slicei, 0, errorcode);
        }
      }
      else{
        boundsdata *sb;

        sb = slicebounds + slicefile_labelindex;
        sb->data_valmin = qmin;
        sb->data_valmax = qmax;

        UpdateAllSliceLabels(slicefile_labelindex, errorcode);
        MakeColorLabels(sb->colorlabels, sb->colorvalues, qmin, qmax, nrgb);
      }
    }
    CheckMemory;
#ifdef pp_MEMDEBUG
    if(sd->compression_type==UNCOMPRESSED&&sd->slice_filetype!=SLICE_GEOM){
      ASSERT(ValidPointer(sd->qslicedata, sizeof(float)*sd->nslicei*sd->nslicej*sd->nslicek*sd->ntimes));
    }
    CheckMemory;
#endif
    ForceIdle();
  }

  if(cache_slice_data == 0){
    FREEMEMORY(sd->qslicedata);
  }

  STOP_TIMER(total_time);

  if(time_frame==ALL_FRAMES&&flag != RESETBOUNDS){
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

  update_flipped_colorbar=1;

  if(colorbartype_ini == -1){
    if(strcmp(sd->label.shortlabel, "thick") == 0){
      ColorbarMenu(wallthickness_colorbar);
    }
    if(strcmp(sd->label.shortlabel, "phi") == 0){
      ColorbarMenu(levelset_colorbar);
    }
  }
  PushSliceLoadstack(slicefilenumber);

  if(sd->volslice == 1){
    meshdata *meshj;

    meshj = meshinfo + sd->blocknumber;

    meshj->slice_min[0] = SMV2FDS_X(sd->xyz_min[0]);
    meshj->slice_min[1] = SMV2FDS_Y(sd->xyz_min[1]);
    meshj->slice_min[2] = SMV2FDS_Z(sd->xyz_min[2]);

    meshj->slice_max[0] = SMV2FDS_X(sd->xyz_max[0]);
    meshj->slice_max[1] = SMV2FDS_Y(sd->xyz_max[1]);
    meshj->slice_max[2] = SMV2FDS_Z(sd->xyz_max[2]);

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
  CheckMemory;
  showall_slices=1;
  GLUTPOSTREDISPLAY;
  if(sd->finalize==1){
    update_slice_bounds = slicefilenumber;
    PrintMemoryInfo;
  }
  SNIFF_ERRORS("ReadSlice: end");
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
  valmin = sb->levels256[0];
  valmax = sb->levels256[255];
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

void DrawVolSliceCellFaceCenter(const slicedata *sd, int flag, 
                                int is1, int is2, int js1, int js2, int ks1, int ks2, int slicedir){
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

  float valmin, valmax;

  valmin = sd->valmin;
  valmax = sd->valmax;
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }

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
    plotx = is1;
    ploty = js1;
    plotz = ks1;
    //tentative fix (was iimin = plotx) to FDS issue 7266
    iimin = plotx+1;
  }

  ibar = meshi->ibar;
  jbar = meshi->jbar;

  iblank_cell = meshi->c_iblank_cell;
  iblank_embed = meshi->c_iblank_embed;

  if(cullfaces == 1)glDisable(GL_CULL_FACE);

  if(use_transparency_data == 1)TransparentOn();
  int doit;
  
  doit = 0;
  if(sd->volslice == 1 && plotx > 0 && visx_all == 1)doit = 1;
  if(sd->volslice == 0 && sd->idir == XDIR)doit = 1;
  if(slicedir>0&&slicedir!=XDIR)doit = 0;
  if(doit == 1){
    float constval;
    int maxj;
    int j;

    int plotxm1;
    plotxm1 = MAX(plotx-1, 0);
    switch(flag){
    case SLICE_CELL_CENTER:
      constval = (xplt[plotx] + xplt[plotxm1]) / 2.0;
      break;
    default:
      constval = (xplt[plotx] + xplt[plotxm1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(slice_dz);

    glBegin(GL_TRIANGLES);
    maxj = MAX(js2, js1 + 1);
    for(j = js1; j<maxj; j++){
      float yy1;
      int k;
      float y3;


      yy1 = yplt[j];
      y3 = yplt[j + 1];
      // val(i,j,k) = di*nj*nk + dj*nk + dk
      for(k = ks1; k<ks2; k++){
        int index_cell;
        int i33;
        float z1, z3;
        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(plotxm1, j, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_slice_shaded[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_slice_shaded[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(plotx, j, k)] == EMBED_YES)continue;

        index_cell = (plotx+1-incx-iimin)*sd->nslicej*sd->nslicek + (j+1-sd->js1)*sd->nslicek + k+1-sd->ks1;
        i33 = 4*SLICECOLOR(index_cell);
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
  }
  doit = 0;
  if(sd->volslice == 1 && ploty > 0 && visy_all == 1)doit = 1;
  if(sd->volslice == 0 && sd->idir == YDIR)doit = 1;
  if(slicedir>0&&slicedir!=YDIR)doit = 0;
  if(doit == 1){
    float constval;
    int i;
    int maxi;

    switch(flag){
    case SLICE_CELL_CENTER:
      constval = (yplt[ploty] + yplt[ploty - 1]) / 2.0;
      break;
    default:
      constval = (yplt[ploty] + yplt[ploty - 1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(slice_dz);

    glBegin(GL_TRIANGLES);
    maxi = MAX(is2, is1 + 1);
    for(i = is1; i<maxi; i++){
      int index_cell;
      float x1, x3;
      int k;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      for(k = ks1; k<ks2; k++){
        int i33;
        float z1, z3;
        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(i, ploty-1, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_slice_shaded[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_slice_shaded[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(i, ploty, k)] == EMBED_YES)continue;

        index_cell = (i+incx-sd->is1)*sd->nslicej*sd->nslicek + (ploty+1-incy-sd->js1)*sd->nslicek + k+1-sd->ks1;
        i33 = 4*CLAMP((int)(255.0*(sd->qslice[index_cell]-valmin)/(valmax-valmin)),0,255);
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
  }
  doit = 0;
  if(sd->volslice == 1 && plotz > 0 && visz_all == 1)doit = 1;
  if(sd->volslice == 0 && sd->idir == ZDIR)doit = 1;
  if(slicedir>0&&slicedir!=ZDIR)doit = 0;
  if(doit == 1){
    float constval;
    int i;
    int maxi;

    switch(flag){
    case SLICE_CELL_CENTER:
      constval = (zplt[plotz] + zplt[plotz - 1]) / 2.0;
      break;
    default:
      constval = (zplt[plotz] + zplt[plotz - 1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(slice_dz);

    glBegin(GL_TRIANGLES);
    maxi = MAX(is2, is1 + 1);
    for(i = is1; i<maxi; i++){
      float x1, x3;
      int j;

      x1 = xplt[i];
      x3 = xplt[i + 1];
      for(j = js1; j<js2; j++){
        int index_cell;
        int i33;
        float yy1, y3;
        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(i, j, plotz-1)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_slice_shaded[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_slice_shaded[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(i, j, plotz)] == EMBED_YES)continue;

        index_cell = (i+1-sd->is1)*sd->nslicej*sd->nslicek + (j+incy-sd->js1)*sd->nslicek + plotz+1-incz-sd->ks1;
        i33 = 4*CLAMP((int)(255.0*(sd->qslice[index_cell]-valmin)/(valmax-valmin)),0,255);
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
  }
  if(use_transparency_data == 1)TransparentOff();
  if(cullfaces == 1)glEnable(GL_CULL_FACE);
}

/* ------------------ DrawVolSliceValues ------------------------ */

void DrawVolSliceValues(slicedata *sd){
  int i, j, k, n;
  int i11;
  float constval, x1, yy1, z1;
  float vel_max;
  meshdata *meshi;
  float *xplttemp, *yplttemp, *zplttemp;
  int plotx, ploty, plotz;
  char *iblank;
  int nx, ny, nxy;
  float *rgb_ptr;

  meshi = meshinfo + sd->blocknumber;
  xplttemp = meshi->xplt;
  yplttemp = meshi->yplt;
  zplttemp = meshi->zplt;

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

  iblank = meshi->c_iblank_node;
  nx = meshi->ibar + 1;
  ny = meshi->jbar + 1;
  nxy = nx*ny;

  vel_max = max_velocity;
  if(vel_max<= 0.0)vel_max = 1.0;
  if((sd->volslice == 1 && plotx >= 0 && visx_all == 1) || (sd->volslice == 0 && sd->idir == XDIR)){
    int maxj;

    constval = xplttemp[plotx] + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    maxj = sd->js2;
    if(sd->js1 + 1 > maxj)maxj = sd->js1 + 1;
    for(j = sd->js1; j < maxj + 1; j += slice_skipy){
      n = (j - sd->js1)*sd->nslicek - slice_skipy;
      n += (plotx - sd->is1)*sd->nslicej*sd->nslicek;
      yy1 = yplttemp[j];
      for(k = sd->ks1; k < sd->ks2 + 1; k += slice_skipz){
        n += slice_skipz;
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
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(plotx, j, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_slice_values[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_slice_values[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.5)continue;

        z1 = zplttemp[k];
        float val;
        val = GET_VAL_N(sd, n);
        Output3Val(constval, yy1, z1, val);
      }
    }
    SNIFF_ERRORS("after DrawSliceValues: dir=1");
  }
  if((sd->volslice == 1 && ploty >= 0 && visy_all == 1) || (sd->volslice == 0 && sd->idir == YDIR)){
    int maxi;

    constval = yplttemp[ploty] + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1 > maxi)maxi = sd->is1 + 1;
    for(i = sd->is1; i < maxi + 1; i += slice_skipx){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - slice_skipx;
      n += (ploty - sd->js1)*sd->nslicek;

      x1 = xplttemp[i];

      for(k = sd->ks1; k < sd->ks2 + 1; k += slice_skipz){
        n += slice_skipz;
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
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(i, ploty, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.5)continue;

        z1 = zplttemp[k];
        float val;
        val = GET_VAL_N(sd, n);
        Output3Val(x1, constval, z1, val);
      }
    }
    SNIFF_ERRORS("after DrawVolSliceValues: dir=2");
  }
  if((sd->volslice == 1 && plotz >= 0 && visz_all == 1) || (sd->volslice == 0 && sd->idir == ZDIR)){
    int maxi;

    constval = zplttemp[plotz] + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    maxi = sd->is1 + sd->nslicei - 1;
    if(sd->is1 + 1 > maxi)maxi = sd->is1 + 1;
    for(i = sd->is1; i < maxi + 1; i += slice_skipx){
      n = (i - sd->is1)*sd->nslicej*sd->nslicek - slice_skipx*sd->nslicek;
      n += (plotz - sd->ks1);

      x1 = xplttemp[i];
      for(j = sd->js1; j < sd->js2 + 1; j += slice_skipy){
        n += slice_skipy*sd->nslicek;
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
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(i, j, plotz)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.5)continue;

        yy1 = yplttemp[j];
        float val;
        val = GET_VAL_N(sd, n);
        Output3Val(x1, yy1, constval, val);
      }
    }
    SNIFF_ERRORS("after DrawVolSliceValues: dir=3");
  }
}

/* ------------------ DrawVolSliceCellValues ------------------------ */

void DrawVolSliceCellFaceCenterValues(const slicedata *sd, int flag){
  float *xplt, *yplt, *zplt;
  int plotx, ploty, plotz;
  int ibar, jbar;
  char *iblank_cell, *iblank_embed;
  int incx = 0, incy = 0, incz = 0;
  int iimin;

  meshdata *meshi;

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
    //tentative fix (was iimin = plotx) to FDS issue 7266
    iimin = plotx+1;
  }

  ibar = meshi->ibar;
  jbar = meshi->jbar;

  iblank_cell = meshi->c_iblank_cell;
  iblank_embed = meshi->c_iblank_embed;

  if(cullfaces == 1)glDisable(GL_CULL_FACE);

  if((sd->volslice == 1 && plotx > 0 && visx_all == 1) || (sd->volslice == 0 && sd->idir == XDIR)){
    float constval;
    int maxj;
    int j;

    int plotxm1;
    plotxm1 = MAX(plotx-1, 0);
    switch(flag){
    case SLICE_CELL_CENTER:
      constval = (xplt[plotx] + xplt[plotxm1]) / 2.0;
      break;
    default:
      constval = (xplt[plotx] + xplt[plotxm1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(slice_dz);

    if(show_slice_values[0]==1||show_slice_values[1]==1||show_slice_values[2]==1){
      maxj = sd->js2;
      if(sd->js1 + 1>maxj){
        maxj = sd->js1 + 1;
      }
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
          int in_solid, in_gas;

          in_gas=1;
          if(iblank_cell != NULL&&iblank_cell[IJKCELL(plotxm1, j, k)] != GAS)in_gas=0;
          in_solid = 1 - in_gas;

          if(iblank_cell!=NULL){
            if(show_slice_values[IN_SOLID_GLUI]==0 && in_solid==1)continue;
            if(show_slice_values[IN_GAS_GLUI]==0   && in_gas==1)continue;
          }
          if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(plotx, j, k)] == EMBED_YES)continue;
          z1 = zplt[k];
          z3 = zplt[k + 1];
          /*
          n+1 (y1,z3) n2+1 (y3,z3)
          n (y1,z1)     n2 (y3,z1)
          */
          index_cell = (plotx+1-incx-iimin)*sd->nslicej*sd->nslicek + (j+1-sd->js1)*sd->nslicek + k+1-sd->ks1;

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
    default:
      constval = (yplt[ploty] + yplt[ploty - 1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(slice_dz);

    if(show_slice_values[0]==1||show_slice_values[1]==1||show_slice_values[2]==1){
      maxi = sd->is1 + sd->nslicei - 1;
      if(sd->is1 + 1>maxi){
        maxi = sd->is1 + 1;
      }
      for(i = sd->is1; i<maxi; i++){
        float x1, x3;
        int k;

        x1 = xplt[i];
        x3 = xplt[i + 1];
        for(k = sd->ks1; k<sd->ks2; k++){
          float val;
          int index_cell;
          float z1, z3;
          int in_solid, in_gas;

          in_gas=1;
          if(iblank_cell != NULL&&iblank_cell[IJKCELL(i, ploty-1, k)] != GAS)in_gas=0;
          in_solid = 1 - in_gas;

          if(iblank_cell!=NULL){
            if(show_slice_values[IN_SOLID_GLUI]==0 && in_solid==1)continue;
            if(show_slice_values[IN_GAS_GLUI]==0   && in_gas==1)continue;
          }
          if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(i, ploty, k)] == EMBED_YES)continue;

          index_cell = (i+incx-sd->is1)*sd->nslicej*sd->nslicek + (ploty+1-incy-sd->js1)*sd->nslicek + k+1-sd->ks1;
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
    default:
      constval = (zplt[plotz] + zplt[plotz - 1]) / 2.0;
      ASSERT(FFALSE);
      break;
    }
    constval += SCALE2SMV(slice_dz);

    if(show_slice_values[0]==1||show_slice_values[1]==1||show_slice_values[2]==1){
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
          float val;
          int index_cell;
          float yy1, y3;
          int in_solid, in_gas;

          in_gas=1;
          if(iblank_cell != NULL&&iblank_cell[IJKCELL(i, j, plotz-1)] != GAS)in_gas=0;
          in_solid = 1 - in_gas;

          if(iblank_cell!=NULL){
            if(show_slice_values[IN_SOLID_GLUI]==0 && in_solid==1)continue;
            if(show_slice_values[IN_GAS_GLUI]==0   && in_gas==1)continue;
          }
          if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJKCELL(i, j, plotz)] == EMBED_YES)continue;

          index_cell = (i+1-sd->is1)*sd->nslicej*sd->nslicek + (j+incy-sd->js1)*sd->nslicek + plotz+1-incz-sd->ks1;
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
  int i, j;
  float r11, r31, r13, r33;
  float x1, x3, yy1, y3;

  float *xplt, *yplt;
  int plotz;
  int nycell;
  meshdata *meshi;

  if(sd->have_agl_data==0)return;
  meshi = meshinfo + sd->blocknumber;
  if(meshi->in_frustum == 0)return;
  nycell = meshi->jbar;

  xplt = meshi->xplt_orig;
  yplt = meshi->yplt_orig;

  if(sd->volslice == 1){
    plotz = meshi->iplotz_all[iplotz_all];
  }
  else{
    plotz = sd->ks1;
  }
  float valmin, valmax;

  valmin = sd->valmin;
  valmax = sd->valmax;
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }
  if(cullfaces == 1)glDisable(GL_CULL_FACE);

  if(use_transparency_data == 1)TransparentOn();
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, texture_slice_colorbar_id);
  if((sd->volslice == 1 && plotz >= 0 && visz_all == 1) || (sd->volslice == 0 && sd->idir == ZDIR)){
    float z11, z31, z13, z33;
    int maxi;
    float *znode, agl_smv, zmin, zmax;
    float voffset;

    znode = meshi->znodes_complete;

    agl_smv = sd->above_ground_level;
    if(agl_offset_actual==1){
      voffset = agl_smv;
    }
    else{
      voffset = MAX(agl_smv, slice_dz);
    }

    zmin = meshi->zplt_orig[0];
    zmax = meshi->zplt_orig[meshi->kbar];
    zmin -= agl_smv;
    zmax -= agl_smv;

    glPushMatrix();
    glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),vertical_factor*SCALE2SMV(1.0));
    glTranslatef(-xbar0,-ybar0,-zbar0 + voffset);

    glBegin(GL_TRIANGLES);
    maxi = sd->is2;
    for(i = sd->is1; i<maxi; i+=slice_skip){
      int i2;

      i2 = MIN(i+slice_skip, maxi);
      if(plotz<sd->ks1)break;
      if(plotz >= sd->ks1 + sd->nslicek)break;
      x1 = xplt[i];
      x3 = xplt[i2];

      for(j = sd->js1; j<sd->js2; j += slice_skip){
        int j2;
        int draw123=0, draw134=0;

        j2 = MIN(j+slice_skip, sd->js2);
        z11 = znode[IJ2(i,  j)];
        z31 = znode[IJ2(i2, j)];
        z13 = znode[IJ2(i, j2)];
        z33 = znode[IJ2(i2, j2)];

        if(z11>=zmin&&z11<=zmax){
          draw123=1;
          draw134=1;
        }
        if(z33>=zmin&&z33<=zmax){
          draw123=1;
          draw134=1;
        }
        if(draw123==0&&z31>=zmin&&z31<=zmax)draw123=1;
        if(draw134==0&&z13>=zmin&&z13<=zmax)draw134=1;
        if(z11<zbar0||z31<zbar0||z33<zbar0)draw123=0;
        if(z11<zbar0||z33<zbar0||z13<zbar0)draw134=0;

        if(draw123==0&&draw134==0)continue;

        z11 = terrain_zmin+geom_vert_exag*(z11-terrain_zmin);
        z31 = terrain_zmin+geom_vert_exag*(z31-terrain_zmin);
        z13 = terrain_zmin+geom_vert_exag*(z13-terrain_zmin);
        z33 = terrain_zmin+geom_vert_exag*(z33-terrain_zmin);

        yy1 = yplt[j];
        y3 = yplt[j2];

        r11 = SLICETEXTURE(i,   j, sd->ks1);
        r31 = SLICETEXTURE(i2,  j, sd->ks1);
        r13 = SLICETEXTURE(i,  j2, sd->ks1);
        r33 = SLICETEXTURE(i2, j2, sd->ks1);

        if(draw123==1){
          glTexCoord1f(r11);  glVertex3f(x1, yy1, z11);
          glTexCoord1f(r31);  glVertex3f(x3, yy1, z31);
          glTexCoord1f(r33);  glVertex3f(x3,  y3, z33);
        }

        if(draw134==1){
          glTexCoord1f(r11);  glVertex3f(x1, yy1, z11);
          glTexCoord1f(r33);  glVertex3f(x3,  y3, z33);
          glTexCoord1f(r13);  glVertex3f(x1,  y3, z13);
        }
      }
    }
    glEnd();
    glPopMatrix();
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

void DrawVolSliceTexture(const slicedata *sd, int is1, int is2, int js1, int js2, int ks1, int ks2, int slicedir){
  int i, j, k;
  float r11, r31, r13, r33;
  float constval, x1, x3, yy1, y3, z1, z3;

  float *xplt, *yplt, *zplt;
  int ibar, jbar;
  int nx, ny, nxy;
  char *c_iblank_x, *c_iblank_y, *c_iblank_z;
  char *iblank_embed;
  int plotx, ploty, plotz;

  meshdata *meshi;

  float valmin, valmax;

  valmin = sd->valmin;
  valmax = sd->valmax;
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }
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
    plotx = is1;
    ploty = js1;
    plotz = ks1;
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

  int doit;

  //*** x plane slices
  doit = 0;
  if(sd->volslice == 1 && plotx >= 0 && visx_all == 1)doit = 1;
  if(sd->volslice == 0 && sd->idir == XDIR)doit = 1;
  if(slicedir>0&&slicedir!=XDIR)doit = 0;
  if(doit == 1){
    int maxj;

    constval = xplt[plotx] + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glBegin(GL_TRIANGLES);
    maxj = MAX(js2, js1 + 1);
    for(j = js1; j<maxj; j+=slice_skipy){
      float ymid;
      int j2;

      j2 = MIN(j+slice_skipy, js2);
      yy1 = yplt[j];
      y3 = yplt[j2];
      ymid = (yy1 + y3) / 2.0;

      // val(i,j,k) = di*nj*nk + dj*nk + dk
      for(k = ks1; k<ks2; k+=slice_skipz){
        float rmid, zmid;
        int k2;
        int in_solid, in_gas;

        in_gas=1;
        if(c_iblank_x!=NULL&&c_iblank_x[IJK(plotx, j, k)]!=GASGAS)in_gas=0;
        in_solid = 1 - in_gas;

        k2 = MIN(k+slice_skipz, ks2);
        if(slice_skipz==1&&slice_skipy==1){
          if(c_iblank_x!=NULL){
            if(show_slice_shaded[IN_SOLID_GLUI]==0&&in_solid==1)continue;
            if(show_slice_shaded[IN_GAS_GLUI]==0&&in_gas==1)continue;
          }
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[IJK(plotx, j, k)]==EMBED_YES)continue;
        }
        r11 = SLICETEXTURE(plotx,  j,  k);
        r31 = SLICETEXTURE(plotx, j2,  k);
        r13 = SLICETEXTURE(plotx,  j, k2);
        r33 = SLICETEXTURE(plotx, j2, k2);
        rmid = (r11 + r31 + r13 + r33) / 4.0;

        z1 = zplt[k];
        z3 = zplt[k2];
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

  //*** y plane slices

  doit = 0;
  if(sd->volslice == 1 && ploty >= 0 && visy_all == 1)doit = 1;
  if(sd->volslice == 0 && sd->idir == YDIR)doit = 1;
  if(slicedir>0&&slicedir!=YDIR)doit = 0;
  if(doit == 1){
    int maxi;
    int istart, iend;

    constval = yplt[ploty]+offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glBegin(GL_TRIANGLES);
    maxi = MAX(is1 + 1, is2);
    istart = is1;
    iend = maxi;

    for(i = istart; i<iend; i+=slice_skipx){
      float xmid;
      int kmin, kmax;
      int i2;

      i2 = MIN(i+slice_skipx, iend);

      x1 = xplt[i];
      x3 = xplt[i2];
      xmid = (x1 + x3) / 2.0;

      kmin = ks1;
      kmax = ks2;
      for(k = kmin; k<kmax; k+=slice_skipz){
        float rmid, zmid;
        int k2;
        int in_solid, in_gas;

        in_gas=1;
        if(c_iblank_y!=NULL&&c_iblank_y[IJK(i, ploty, k)]!=GASGAS)in_gas=0;
        in_solid = 1 - in_gas;

        k2 = MIN(k + slice_skipz, ks2);
        if(slice_skipx==1&&slice_skipz==1){
          if(c_iblank_y!=NULL){
            if(show_slice_shaded[IN_SOLID_GLUI]==0   && in_solid==1)continue;
            if(show_slice_shaded[IN_GAS_GLUI]==0 && in_gas==1)continue;
          }
          if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, ploty, k)] == EMBED_YES)continue;
        }
        r11 = SLICETEXTURE(i,  ploty, k);
        r31 = SLICETEXTURE(i2, ploty, k);
        r13 = SLICETEXTURE(i,  ploty, k2);
        r33 = SLICETEXTURE(i2, ploty, k2);
        rmid = (r11 + r31 + r13 + r33) / 4.0;

        z1 = zplt[k];
        z3 = zplt[k2];
        zmid = (z1 + z3) / 2.0;

        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,z3,r13)                    (x3,z3,r33)
        //                (xmid,zmid,rmid)
        //  (x1,z1,r11)                    (x3,z1,r31)
        glTexCoord1f(r11); glVertex3f(x1,    constval, z1);
        glTexCoord1f(r31); glVertex3f(x3,    constval, z1);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r31); glVertex3f(x3,    constval, z1);
        glTexCoord1f(r33); glVertex3f(x3,    constval, z3);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r33); glVertex3f(x3,    constval, z3);
        glTexCoord1f(r13); glVertex3f(x1,    constval, z3);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
        glTexCoord1f(r13); glVertex3f(x1,    constval, z3);
        glTexCoord1f(r11); glVertex3f(x1,    constval, z1);
        glTexCoord1f(rmid); glVertex3f(xmid, constval, zmid);
      }
    }
    glEnd();
  }

  //*** z plane slices

  doit = 0;
  if(sd->volslice == 1 && plotz >= 0 && visz_all == 1)doit = 1;
  if(sd->volslice == 0 && sd->idir == ZDIR)doit = 1;
  if(slicedir>0&&slicedir!=ZDIR)doit = 0;
  if(doit == 1){
    int maxi;

    constval = zplt[plotz] + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glBegin(GL_TRIANGLES);

    maxi = MAX(is1 + 1, is2);
    for(i = is1; i<maxi; i+=slice_skipx){
      float xmid;
      int i2;

      i2 = MIN(i+slice_skipx, maxi);

      x1 = xplt[i];
      x3 = xplt[i2];
      xmid = (x1 + x3) / 2.0;

      for(j = js1; j<js2; j+=slice_skipy){
        float ymid, rmid;
        int j2;
        int in_solid, in_gas;

        in_gas=1;
        if(c_iblank_z!=NULL&&c_iblank_z[IJK(i, j, plotz)] != GASGAS)in_gas=0;
        in_solid = 1 - in_gas;

        j2 = MIN(j+slice_skipy, js2);
        if(slice_skipy==1&&slice_skipx==1){
          if(c_iblank_z!=NULL){
            if(show_slice_shaded[IN_SOLID_GLUI]==0 && in_solid==1)continue;
            if(show_slice_shaded[IN_GAS_GLUI]==0   && in_gas==1)continue;
          }
          if(skip_slice_in_embedded_mesh == 1 && iblank_embed != NULL&&iblank_embed[IJK(i, j, plotz)] == EMBED_YES)continue;
        }
        r11 = SLICETEXTURE(i,   j, plotz);
        r31 = SLICETEXTURE(i2,  j, plotz);
        r13 = SLICETEXTURE(i,  j2, plotz);
        r33 = SLICETEXTURE(i2, j2, plotz);
        rmid = (r11 + r31 + r13 + r33) / 4.0;

        yy1 = yplt[j];
        y3 = yplt[j2];
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

/* ------------------ DrawVolSliceLines ------------------------ */

void DrawVolSliceLines(const slicedata *sd){
  int i, j, k;
  float constval, x1, x3, yy1, y3, z1, z3;

  float *xplt, *yplt, *zplt;
  int ibar, jbar;
  int nx, ny, nxy;
  char *c_iblank_x, *c_iblank_y, *c_iblank_z;
  char *iblank_embed;
  int plotx, ploty, plotz;
  int draw_x_slice = 0, draw_y_slice = 0, draw_z_slice = 0;
  float *slice_color, *slice_color13, *slice_color31, *slice_color33;

  meshdata *meshi;

  if(sd->volslice==1&&visx_all==0&&visy_all==0&&visz_all==0)return;
  meshi = meshinfo+sd->blocknumber;

  float valmin, valmax;

  valmin = sd->valmin;
  valmax = sd->valmax;
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }
  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  if(sd->volslice==1){
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
  nx = ibar+1;
  ny = jbar+1;
  nxy = nx*ny;

  if((sd->volslice==1&&plotx>=0&&visx_all==1)||(sd->volslice==0&&sd->idir==XDIR))draw_x_slice = 1;
  if((sd->volslice==1&&ploty>=0&&visy_all==1)||(sd->volslice==0&&sd->idir==YDIR))draw_y_slice = 1;
  if((sd->volslice==1&&plotz>=0&&visz_all==1)||(sd->volslice==0&&sd->idir==ZDIR))draw_z_slice = 1;

  if(draw_x_slice==1||draw_y_slice==1||draw_z_slice==1){
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    glColor3fv(foregroundcolor);
  }
    //*** x plane slices

  if(draw_x_slice==1){
    int maxj;

    constval = xplt[plotx]+offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    maxj = sd->js2;
    if(sd->js1+1>maxj){
      maxj = sd->js1+1;
    }
    for(j = sd->js1; j<maxj; j += slice_skipy){
      int j2;

      j2 = MIN(j+slice_skipy, sd->js2);
      yy1 = yplt[j];
      y3 = yplt[j2];

      // val(i,j,k) = di*nj*nk + dj*nk + dk
      for(k = sd->ks1; k<sd->ks2; k += slice_skipz){
        int k2, in_gas, in_solid;

        in_gas=1;
        if(c_iblank_x!=NULL&&c_iblank_x[IJK(plotx, j, k)]!=GASGAS)in_gas=0;
        in_solid = 1 - in_gas;
        k2 = MIN(k+slice_skipz, sd->ks2);
        if(slice_skipz==1&&slice_skipy==1){
          if(c_iblank_x!=NULL){
            if(show_slice_outlines[IN_SOLID_GLUI]==0&&in_solid==1)continue;
            if(show_slice_outlines[IN_GAS_GLUI]==0  &&in_gas==1)continue;
          }
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[IJK(plotx, j, k)]==EMBED_YES)continue;
        }
#define IND_SLICEX(JJ,KK) JJ*sd->nslicek + KK*slice_skipz

        if(in_gas==1&&show_slice_shaded[IN_GAS_GLUI]==1){
          slice_color   = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else if(in_solid==1&&show_slice_shaded[IN_SOLID_GLUI]==1){
          slice_color   = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else{
          int val[4];

          val[0] = SLICECOLOR(IJK_SLICE(plotx, j,  k));
          val[1] = SLICECOLOR(IJK_SLICE(plotx, j,  k2));
          val[2] = SLICECOLOR(IJK_SLICE(plotx, j2, k2));
          val[3] = SLICECOLOR(IJK_SLICE(plotx, j2, k));
          slice_color   = rgb_slice + 4 * val[0];
          slice_color13 = rgb_slice + 4 * val[1];
          slice_color33 = rgb_slice + 4 * val[2];
          slice_color31 = rgb_slice + 4 * val[3];
        }
        z1 = zplt[k];
        z3 = zplt[k2];

        /*
        n+1 (y1,z3) n2+1 (y3,z3)
        n (y1,z1)     n2 (y3,z1)
        */
        //  (yy1,z3) (y3,z3)
        //  (yy1,z1) (y3,z1)

        glColor3fv(slice_color);
        glVertex3f(constval, yy1, z1);
        glColor3fv(slice_color31);
        glVertex3f(constval, y3, z1);

        glVertex3f(constval, y3, z1);
        glColor3fv(slice_color33);
        glVertex3f(constval, y3, z3);

        glVertex3f(constval, y3, z3);
        glColor3fv(slice_color13);
        glVertex3f(constval, yy1, z3);

        glVertex3f(constval, yy1, z3);
        glColor3fv(slice_color);
        glVertex3f(constval, yy1, z1);
      }
    }
  }

  //*** y plane slices

  if(draw_y_slice==1){
    int maxi;
    int istart, iend;

    constval = yplt[ploty]+offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    maxi = sd->is1+sd->nslicei-1;
    if(sd->is1+1>maxi){
      maxi = sd->is1+1;
    }
    istart = sd->is1;
    iend = maxi;

    for(i = istart; i<iend; i += slice_skipx){
      int kmin, kmax;
      int i2;

      i2 = MIN(i+slice_skipx, iend);

      x1 = xplt[i];
      x3 = xplt[i2];

      kmin = sd->ks1;
      kmax = sd->ks2;
      for(k = kmin; k<kmax; k += slice_skipz){
        int k2;
        int in_solid, in_gas;

        in_gas = 1;
        if(c_iblank_y!=NULL&&c_iblank_y[IJK(i, ploty, k)]!=GASGAS)in_gas = 0;
        in_solid = 1 - in_gas;

        k2 = MIN(k+slice_skipz, sd->ks2);
        if(slice_skipx==1&&slice_skipz==1){
          if(c_iblank_y!=NULL){
            if(show_slice_outlines[IN_GAS_GLUI]==0  &&in_gas==1)continue;
            if(show_slice_outlines[IN_SOLID_GLUI]==0&&in_solid==1)continue;
          }
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[IJK(i, ploty, k)]==EMBED_YES)continue;
        }
        z1 = zplt[k];
        z3 = zplt[k2];

        if(in_gas==1&&show_slice_shaded[IN_GAS_GLUI]==1){
          slice_color   = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else if(in_solid==1&&show_slice_shaded[IN_SOLID_GLUI]==1){
          slice_color   = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else{
          int val[4];

          val[0] = SLICECOLOR(IJK_SLICE(i,  ploty,  k));
          val[1] = SLICECOLOR(IJK_SLICE(i,  ploty,  k2));
          val[2] = SLICECOLOR(IJK_SLICE(i2, ploty,  k2));
          val[3] = SLICECOLOR(IJK_SLICE(i2, ploty,  k));
          slice_color   = rgb_slice + 4 * val[0];
          slice_color13 = rgb_slice + 4 * val[1];
          slice_color33 = rgb_slice + 4 * val[2];
          slice_color31 = rgb_slice + 4 * val[3];
        }
        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,z3) (x3,z3)
        //  (x1,z1) (x3,z1,r31)
        glColor3fv(slice_color);
        glVertex3f(x1, constval, z1);
        glColor3fv(slice_color31);
        glVertex3f(x3, constval, z1);

        glVertex3f(x3, constval, z1);
        glColor3fv(slice_color33);
        glVertex3f(x3, constval, z3);

        glVertex3f(x3, constval, z3);
        glColor3fv(slice_color13);
        glVertex3f(x1, constval, z3);

        glVertex3f(x1, constval, z3);
        glColor3fv(slice_color);
        glVertex3f(x1, constval, z1);
      }
    }
  }

  //*** z plane slices

  if(draw_z_slice==1){
    int maxi;

    constval = zplt[plotz]+offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);

    maxi = sd->is1+sd->nslicei-1;
    if(sd->is1+1>maxi){
      maxi = sd->is1+1;
    }
    for(i = sd->is1; i<maxi; i += slice_skipx){
      int i2;
      int in_gas, in_solid;

      i2 = MIN(i+slice_skipx, sd->is2);

      x1 = xplt[i];
      x3 = xplt[i2];

      for(j = sd->js1; j<sd->js2; j += slice_skipy){
        int j2;

        j2 = MIN(j+slice_skipy, sd->js2);
        in_gas = 1;
        if(c_iblank_z!=NULL&&c_iblank_z[IJK(i, j, plotz)]!=GASGAS)in_gas = 0;
        in_solid = 1 - in_gas;
        if(slice_skipy==1&&slice_skipx==1){
          if(c_iblank_z!=NULL){
            if(show_slice_outlines[IN_GAS_GLUI]==0  &&in_gas==1)continue;
            if(show_slice_outlines[IN_SOLID_GLUI]==0&&in_solid==1)continue;
          }
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[IJK(i, j, plotz)]==EMBED_YES)continue;
        }
        yy1 = yplt[j];
        y3 = yplt[j2];

#define IND_SLICEZ(II,JJ) II*sd->nslicej*sd->nslicek + JJ*slice_skipy*sd->nslicek

        if(in_gas==1&&show_slice_shaded[IN_GAS_GLUI]==1){
          slice_color   = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else if(in_solid==1&&show_slice_shaded[IN_SOLID_GLUI]==1){
          slice_color   = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else{
          int val[4];

          val[0] = SLICECOLOR(IJK_SLICE(i,  j,  plotz));
          val[1] = SLICECOLOR(IJK_SLICE(i,  j2, plotz));
          val[2] = SLICECOLOR(IJK_SLICE(i2, j2, plotz));
          val[3] = SLICECOLOR(IJK_SLICE(i2, j,  plotz));
          slice_color   = rgb_slice + 4 * val[0];
          slice_color13 = rgb_slice + 4 * val[1];
          slice_color33 = rgb_slice + 4 * val[2];
          slice_color31 = rgb_slice + 4 * val[3];
        }
        /*
        n+nk (x1,y3)   n2+nk (x3,y3)
        n (x1,y1)      n2 (x3,y1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,y3) (x3,y3)
        //  (x1,yy1) (x3,yy1)
        glColor3fv(slice_color);
        glVertex3f(x1, yy1, constval);
        glColor3fv(slice_color31);
        glVertex3f(x3, yy1, constval);

        glVertex3f(x3, yy1, constval);
        glColor3fv(slice_color33);
        glVertex3f(x3, y3, constval);

        glVertex3f(x3, y3, constval);
        glColor3fv(slice_color13);
        glVertex3f(x1, y3, constval);

        glVertex3f(x1, y3, constval);
        glColor3fv(slice_color);
        glVertex3f(x1, yy1, constval);
      }
    }
  }
  if(draw_x_slice==1||draw_y_slice==1||draw_z_slice==1)glEnd();
}

/* ------------------ DrawVolSliceVerts ------------------------ */

void DrawVolSliceVerts(const slicedata *sd){
  int i, j, k;
  float constval, x1, x3, yy1, y3, z1, z3;

  float *xplt, *yplt, *zplt;
  int ibar, jbar;
  int nx, ny, nxy;
  char *c_iblank_x, *c_iblank_y, *c_iblank_z;
  char *iblank_embed;
  int plotx, ploty, plotz;
  int draw_x_slice = 0, draw_y_slice = 0, draw_z_slice = 0;
  float *slice_color, *slice_color13, *slice_color31, *slice_color33;

  meshdata *meshi;

  if(sd->volslice==1&&visx_all==0&&visy_all==0&&visz_all==0)return;
  meshi = meshinfo+sd->blocknumber;

  float valmin, valmax;

  valmin = sd->valmin;
  valmax = sd->valmax;
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }
  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  if(sd->volslice==1){
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
  nx = ibar+1;
  ny = jbar+1;
  nxy = nx*ny;

  if((sd->volslice==1&&plotx>=0&&visx_all==1)||(sd->volslice==0&&sd->idir==XDIR))draw_x_slice = 1;
  if((sd->volslice==1&&ploty>=0&&visy_all==1)||(sd->volslice==0&&sd->idir==YDIR))draw_y_slice = 1;
  if((sd->volslice==1&&plotz>=0&&visz_all==1)||(sd->volslice==0&&sd->idir==ZDIR))draw_z_slice = 1;

  if(draw_x_slice==1||draw_y_slice==1||draw_z_slice==1){
    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    glColor3fv(foregroundcolor);
  }
    //*** x plane slices

  if(draw_x_slice==1){
    int maxj;

    constval = xplt[plotx]+offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    maxj = sd->js2;
    if(sd->js1+1>maxj){
      maxj = sd->js1+1;
    }
    for(j = sd->js1; j<maxj; j += slice_skipy){
      int j2;

      j2 = MIN(j+slice_skipy, sd->js2);
      yy1 = yplt[j];
      y3 = yplt[j2];

      // val(i,j,k) = di*nj*nk + dj*nk + dk
      for(k = sd->ks1; k<sd->ks2; k += slice_skipz){
        int k2, in_gas, in_solid;

        in_gas = 1;
        if(c_iblank_x!=NULL&&c_iblank_x[IJK(plotx, j, k)]!=GASGAS)in_gas = 0;
        in_solid = 1-in_gas;
        k2 = MIN(k+slice_skipz, sd->ks2);
        if(slice_skipz==1&&slice_skipy==1){
          if(c_iblank_x!=NULL){
            if(show_slice_points[IN_SOLID_GLUI]==0&&in_solid==1)continue;
            if(show_slice_points[IN_GAS_GLUI]==0&&in_gas==1)continue;
          }
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[IJK(plotx, j, k)]==EMBED_YES)continue;
        }

        if(in_gas==1&&show_slice_shaded[IN_GAS_GLUI]==1){
          slice_color = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else if(in_solid==1&&show_slice_shaded[IN_SOLID_GLUI]==1){
          slice_color = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else{
          int val[4];

          val[0] = SLICECOLOR(IJK_SLICE(plotx,  j,   k));
          val[1] = SLICECOLOR(IJK_SLICE(plotx,  j,   k2));
          val[2] = SLICECOLOR(IJK_SLICE(plotx,  j2,  k2));
          val[3] = SLICECOLOR(IJK_SLICE(plotx,  j2,  k));
          slice_color   = rgb_slice + 4 * val[0];
          slice_color13 = rgb_slice + 4 * val[1];
          slice_color33 = rgb_slice + 4 * val[2];
          slice_color31 = rgb_slice + 4 * val[3];
        }
        z1 = zplt[k];
        z3 = zplt[k2];

        /*
        n+1 (y1,z3) n2+1 (y3,z3)
        n (y1,z1)     n2 (y3,z1)
        */
        //  (yy1,z3) (y3,z3)
        //  (yy1,z1) (y3,z1)

        glColor3fv(slice_color);
        glVertex3f(constval, yy1, z1);
        glColor3fv(slice_color31);
        glVertex3f(constval, y3, z1);
        glColor3fv(slice_color33);
        glVertex3f(constval, y3, z3);
        glColor3fv(slice_color13);
        glVertex3f(constval, yy1, z3);
      }
    }
  }

  //*** y plane slices

  if(draw_y_slice==1){
    int maxi;
    int istart, iend;

    constval = yplt[ploty]+offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    maxi = sd->is1+sd->nslicei-1;
    if(sd->is1+1>maxi){
      maxi = sd->is1+1;
    }
    istart = sd->is1;
    iend = maxi;

    for(i = istart; i<iend; i += slice_skipx){
      int kmin, kmax;
      int i2;

      i2 = MIN(i+slice_skipx, iend);

      x1 = xplt[i];
      x3 = xplt[i2];

      kmin = sd->ks1;
      kmax = sd->ks2;
      for(k = kmin; k<kmax; k += slice_skipz){
        int k2;
        int in_solid, in_gas;

        in_gas = 1;
        if(c_iblank_y!=NULL&&c_iblank_y[IJK(i, ploty, k)]!=GASGAS)in_gas = 0;
        in_solid = 1-in_gas;

        k2 = MIN(k+slice_skipz, sd->ks2);
        if(slice_skipx==1&&slice_skipz==1){
          if(c_iblank_y!=NULL){
            if(show_slice_points[IN_GAS_GLUI]==0&&in_gas==1)continue;
            if(show_slice_points[IN_SOLID_GLUI]==0&&in_solid==1)continue;
          }
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[IJK(i, ploty, k)]==EMBED_YES)continue;
        }
        z1 = zplt[k];
        z3 = zplt[k2];

        if(in_gas==1&&show_slice_shaded[IN_GAS_GLUI]==1){
          slice_color = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else if(in_solid==1&&show_slice_shaded[IN_SOLID_GLUI]==1){
          slice_color = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else{
          int val[4];

          val[0] = SLICECOLOR(IJK_SLICE(i,  ploty,  k));
          val[1] = SLICECOLOR(IJK_SLICE(i,  ploty,  k2));
          val[2] = SLICECOLOR(IJK_SLICE(i2, ploty,  k2));
          val[3] = SLICECOLOR(IJK_SLICE(i2, ploty,  k));
          slice_color   = rgb_slice + 4 * val[0];
          slice_color13 = rgb_slice + 4 * val[1];
          slice_color33 = rgb_slice + 4 * val[2];
          slice_color31 = rgb_slice + 4 * val[3];
        }
        /*
        n+1 (x1,z3)   n2+1 (x3,z3)
        n (x1,z1)     n2 (x3,z1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,z3) (x3,z3)
        //  (x1,z1) (x3,z1,r31)
        glColor3fv(slice_color);
        glVertex3f(x1, constval, z1);
        glColor3fv(slice_color31);
        glVertex3f(x3, constval, z1);
        glColor3fv(slice_color33);
        glVertex3f(x3, constval, z3);
        glColor3fv(slice_color13);
        glVertex3f(x1, constval, z3);
      }
    }
  }

  //*** z plane slices

  if(draw_z_slice==1){
    int maxi;

    constval = zplt[plotz]+offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);

    maxi = sd->is1+sd->nslicei-1;
    if(sd->is1+1>maxi){
      maxi = sd->is1+1;
    }
    for(i = sd->is1; i<maxi; i += slice_skipx){
      int i2;
      int in_gas, in_solid;

      i2 = MIN(i+slice_skipx, sd->is2);

      x1 = xplt[i];
      x3 = xplt[i2];

      for(j = sd->js1; j<sd->js2; j += slice_skipy){
        int j2;

        j2 = MIN(j+slice_skipy, sd->js2);
        in_gas = 1;
        if(c_iblank_z!=NULL&&c_iblank_z[IJK(i, j, plotz)]!=GASGAS)in_gas = 0;
        in_solid = 1-in_gas;
        if(slice_skipy==1&&slice_skipx==1){
          if(c_iblank_z!=NULL){
            if(show_slice_points[IN_GAS_GLUI]==0&&in_gas==1)continue;
            if(show_slice_points[IN_SOLID_GLUI]==0&&in_solid==1)continue;
          }
          if(skip_slice_in_embedded_mesh==1&&iblank_embed!=NULL&&iblank_embed[IJK(i, j, plotz)]==EMBED_YES)continue;
        }
        yy1 = yplt[j];
        y3 = yplt[j2];

        if(in_gas==1&&show_slice_shaded[IN_GAS_GLUI]==1){
          slice_color = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else if(in_solid==1&&show_slice_shaded[IN_SOLID_GLUI]==1){
          slice_color = foregroundcolor;
          slice_color13 = foregroundcolor;
          slice_color31 = foregroundcolor;
          slice_color33 = foregroundcolor;
        }
        else{
          int val[4];

          val[0] = SLICECOLOR(IJK_SLICE(i,  j,  plotz));
          val[1] = SLICECOLOR(IJK_SLICE(i,  j2, plotz));
          val[2] = SLICECOLOR(IJK_SLICE(i2, j2, plotz));
          val[3] = SLICECOLOR(IJK_SLICE(i2, j,  plotz));
          slice_color   = rgb_slice + 4 * val[0];
          slice_color13 = rgb_slice + 4 * val[1];
          slice_color33 = rgb_slice + 4 * val[2];
          slice_color31 = rgb_slice + 4 * val[3];
        }
        /*
        n+nk (x1,y3)   n2+nk (x3,y3)
        n (x1,y1)      n2 (x3,y1)

        val(i,j,k) = di*nj*nk + dj*nk + dk
        */
        //  (x1,y3) (x3,y3)
        //  (x1,yy1) (x3,yy1)
        glColor3fv(slice_color);
        glVertex3f(x1, yy1, constval);
        glColor3fv(slice_color31);
        glVertex3f(x3, yy1, constval);
        glColor3fv(slice_color33);
        glVertex3f(x3, y3, constval);
        glColor3fv(slice_color13);
        glVertex3f(x1, y3, constval);
      }
    }
  }
  if(draw_x_slice==1||draw_y_slice==1||draw_z_slice==1)glEnd();
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
  if(eye_position_smv[dir] < position_i){
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
  qsort((int *)slice_sorted_loaded_list,nslice_loaded,sizeof(int), CompareLoadedSliceList);
}

/* ------------------ GetSliceOffsetGeom ------------------------ */

int GetSliceOffsetGeom(slicedata *sd, float *xyz, float *device_xyz){
  geomdata *geomi;
  geomlistdata *geomlisti;
  float dist_min;
  float dx, dy, dz;
  int offset;
  int i;

  geomi = sd->patchgeom->geominfo;
  geomlisti = geomi->geomlistinfo-1;
  if(sd->cell_center==1){
    for(i = 0; i<geomlisti->ntriangles; i++){
      tridata *trianglei;
      float vert_avg[3];
      vertdata **verts;
      float dist2;

      trianglei = geomlisti->triangles+i;
      verts = trianglei->verts;
      VERT_AVG3(verts[0]->xyz, verts[1]->xyz, verts[2]->xyz, vert_avg);
      DIST3(vert_avg, xyz, dist2);
      if(i==0){
        dist_min = dist2;
        memcpy(device_xyz, vert_avg, 3*sizeof(float));
        offset = i;
      }
      else{
        if(dist2<dist_min){
          dist_min = dist2;
          memcpy(device_xyz, vert_avg, 3*sizeof(float));
          offset = i;
        }
      }
    }
  }
  else{
    for(i = 0; i<geomlisti->nverts; i++){
      vertdata *verti;
      float dist2;

      verti = geomlisti->verts+i;
      DIST3(verti->xyz, xyz, dist2);
      if(i==0){
        dist_min = dist2;
        memcpy(device_xyz, verti->xyz, 3*sizeof(float));
        offset = i;
      }
      else{
        if(dist2<dist_min){
          dist_min = dist2;
          memcpy(device_xyz, verti->xyz, 3*sizeof(float));
          offset = i;
        }
      }
    }
  }
  return offset;
}

/* ------------------ GetSliceOffsetReg ------------------------ */

int GetSliceOffsetReg(slicedata *sd, float *xyz, float *device_xyz){
  meshdata *slicemesh;
  float *xplt, *yplt, *zplt;
  int plotx, ploty, plotz;
  int i, j, k, ii;
  int ibar, jbar, kbar;
  int nx, ny, nz;
  int offset=0;

  memcpy(device_xyz, xyz, 3*sizeof(float));
  slicemesh = meshinfo+sd->blocknumber;
  xplt = slicemesh->xplt_orig;
  yplt = slicemesh->yplt_orig;
  zplt = slicemesh->zplt_orig;
  if(sd->volslice==0){
    plotx = sd->is1;
    ploty = sd->js1;
    plotz = sd->ks1;
  }
  else{
    plotx = slicemesh->iplotx_all[iplotx_all];
    ploty = slicemesh->iploty_all[iploty_all];
    plotz = slicemesh->iplotz_all[iplotz_all];
  }
  ibar = slicemesh->ibar;
  jbar = slicemesh->jbar;
  kbar = slicemesh->kbar;
  nx = ibar + 1;
  ny = jbar + 1;
  nz = kbar + 1;
  i=0;
  j=0;
  k=0;
  for(ii=0;ii<nx;ii++){
    if((ii!=nx-1&&xplt[ii]<=xyz[0]&&xyz[0]<xplt[ii+1])||(ii == nx-1&&xyz[0]==xplt[nx-1])){
      i=ii;
      break;
    }
  }
  for(ii=0;ii<ny;ii++){
    if((ii!=ny-1&&yplt[ii]<=xyz[1]&&xyz[1]<yplt[ii+1])||(ii == ny-1&&xyz[1]==yplt[ny-1])){
      j=ii;
      break;
    }
  }
  for(ii=0;ii<nz;ii++){
    if((ii!=nz-1&&zplt[ii]<=xyz[2]&&xyz[2]<zplt[ii+1])||(ii == nz-1&&xyz[2]==zplt[nz-1])){
      k=ii;
      break;
    }
  }
  if((sd->volslice == 0 && sd->idir == XDIR)
    || (sd->volslice == 1 && visx_all==1)
  ){
    offset = IJK_SLICE(plotx, j, k);
    device_xyz[0] = xplt[plotx];
  }
  if((sd->volslice == 0 && sd->idir == YDIR)
    || (sd->volslice == 1 && visy_all==1)
  ){
    offset = IJK_SLICE(i, ploty, k);
    device_xyz[1] = yplt[ploty];
  }
  if((sd->volslice == 0 && sd->idir == ZDIR)
    || (sd->volslice == 1 && visz_all==1)
  ){
    offset = IJK_SLICE(i, j, plotz);
    device_xyz[2] = zplt[plotz];
  }
  return offset;
}

/* ------------------ GetSliceOffset ------------------------ */

int GetSliceOffset(slicedata *sd, float *xyz, float *device_xyz){
  int offset;

  if(sd->slice_filetype!=SLICE_GEOM){
    offset = GetSliceOffsetReg(sd, xyz, device_xyz);
  }
  else{
    offset = GetSliceOffsetGeom(sd, xyz, device_xyz);
  }
  return offset;
}

/* ------------------ GetSliceVal ------------------------ */

float GetSliceVal(slicedata *slicei, int itime, int offset){
  float *qslice, sliceval;

  if(slicei->slice_filetype!=SLICE_GEOM){
    qslice = slicei->qslicedata + itime*slicei->nsliceijk;
    sliceval = qslice[offset];
  }
  else{
    patchdata *patchgeom;

    patchgeom = slicei->patchgeom;
    if(patchgeom==NULL){
      sliceval = 0.0;
    }
    else{
      sliceval = patchgeom->geom_vals[offset+itime*patchgeom->geom_ndynamics[0]];
    }
  }
  return sliceval;
}

/* ------------------ GetDevMinMax ------------------------ */

void GetDevMinMax(devicedata *devi, float *valmin, float *valmax){
  int i;

  *valmin = devi->vals[0];
  *valmax = *valmin;
  for(i = 1; i<devi->nvals; i++){
    *valmin = MIN(*valmin, devi->vals[i]);
    *valmax = MAX(*valmax, devi->vals[i]);
  }
}

/* ------------------ InSliceMesh ------------------------ */

int InSliceMesh(slicedata *slicei, float *xyz){
  float *boxmin, *boxmax;
  int dir;
  meshdata *meshi;
  int plotx, ploty, plotz;

  meshi = meshinfo + slicei->blocknumber;
  dir = slicei->idir;
  boxmin = meshi->boxmin;
  boxmax = meshi->boxmax;
  if(slicei->volslice==0){
    if(dir==XDIR){
      if(xyz[1]<boxmin[1]||xyz[1]>boxmax[1])return 0;
      if(xyz[2]<boxmin[2]||xyz[2]>boxmax[2])return 0;
      return 1;
    }
    if(dir==YDIR){
      if(xyz[0]<boxmin[0]||xyz[0]>boxmax[0])return 0;
      if(xyz[2]<boxmin[2]||xyz[2]>boxmax[2])return 0;
      return 1;
    }
    if(dir==ZDIR){
      if(xyz[0]<boxmin[0]||xyz[0]>boxmax[0])return 0;
      if(xyz[1]<boxmin[1]||xyz[1]>boxmax[1])return 0;
      return 1;
    }
  }
  else{
    if(visx_all==1){
      float dx;

      if(xyz[1]<boxmin[1]||xyz[1]>boxmax[1]||xyz[2]<boxmin[2]||xyz[2]>boxmax[2])return 0;
      plotx = meshi->iplotx_all[iplotx_all];
      dx = meshi->xplt_orig[plotx];
      if(dx<boxmin[0]||dx>boxmax[0])return 0;
      xyz[0] = dx;
      update_slicexyz = 1;
      return 1;
    }
    if(visy_all==1){
      float dy;

      if(xyz[0]<boxmin[0]||xyz[0]>boxmax[0]||xyz[2]<boxmin[2]||xyz[2]>boxmax[2])return 0;
      ploty = meshi->iploty_all[iploty_all];
      dy = meshi->yplt_orig[ploty];
      if(dy<boxmin[1]||dy>boxmax[1])return 0;
      xyz[1] = dy;
      update_slicexyz = 1;
      return 1;
    }
    if(visz_all==1){
      float dz;

      if(xyz[0]<boxmin[0]||xyz[0]>boxmax[0]||xyz[1]<boxmin[1]||xyz[1]>boxmax[1])return 0;
      plotz = meshi->iplotz_all[iplotz_all];
      dz = meshi->zplt_orig[plotz];
      if(dz<boxmin[2]||dz>boxmax[2])return 0;
      xyz[2] = dz;
      update_slicexyz = 1;
      return 1;
    }
  }
  return 1;
}


/* ------------------ Slice2Device ------------------------ */

void Slice2Device(void){
  int i;
#define NOFFSETS 5
  int offsets[NOFFSETS*NOFFSETS*NOFFSETS];

  if(vis_slice_plot==0)return;
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;
    devicedata *sdev;
    int offset;
    float xyz[3], dxyz[3];
    int ii;
    slicei = sliceinfo+i;
    sdev = &(slicei->vals2d);
    sdev->valid = 0;
    if(slicei->loaded==0||slicei->ntimes==0)continue;

    int noffsets;
    int noffset_i, noffset_j, noffset_k;

    if(average_plot2d_slice_region==1){
      float slice_xb[6];

      slice_xb[0] = MAX(slice_xyz[0] - slice_dxyz[0]/2.0, xbar0ORIG);
      slice_xb[1] = MIN(slice_xyz[0] + slice_dxyz[0]/2.0, xbarORIG);
      slice_xb[2] = MAX(slice_xyz[1] - slice_dxyz[1]/2.0, ybar0ORIG);
      slice_xb[3] = MIN(slice_xyz[1] + slice_dxyz[1]/2.0, ybarORIG);
      slice_xb[4] = MAX(slice_xyz[2] - slice_dxyz[2]/2.0, zbar0ORIG);
      slice_xb[5] = MIN(slice_xyz[2] + slice_dxyz[2]/2.0, zbarORIG);

      dxyz[0] = (slice_xb[1]-slice_xb[0])/(float)(NOFFSETS-1);
      dxyz[1] = (slice_xb[3]-slice_xb[2])/(float)(NOFFSETS-1);
      dxyz[2] = (slice_xb[5]-slice_xb[4])/(float)(NOFFSETS-1);

      noffset_i = NOFFSETS;
      noffset_j = NOFFSETS;
      noffset_k = NOFFSETS;
      if(dxyz[0]<0.0001)noffset_i = 1;
      if(dxyz[1]<0.0001)noffset_j = 1;
      if(dxyz[2]<0.0001)noffset_k = 1;
      }
    else{
      dxyz[0] = 0.0;
      dxyz[1] = 0.0;
      dxyz[2] = 0.0;

      noffset_i = 1;
      noffset_j = 1;
      noffset_k = 1;
    }

    noffsets = 0;
    for(ii=0;ii<noffset_i;ii++){
      int jj;
      
      if(noffset_i==1)xyz[0] = slice_xyz[0];
      if(noffset_i>1)xyz[0] = slice_xyz[0] - slice_dxyz[0]/2.0 + (float)ii*dxyz[0];
      for(jj=0;jj<noffset_j;jj++){
        int kk;

        if(noffset_j==1)xyz[1] = slice_xyz[1];
        if(noffset_j>1)xyz[1] = slice_xyz[1] - slice_dxyz[1]/2.0 + (float)jj*dxyz[1];
        for(kk=0;kk<noffset_k;kk++){
          int ll;
          
          if(noffset_k==1)xyz[2] = slice_xyz[2];
          if(noffset_k>1)xyz[2] = slice_xyz[2] - slice_dxyz[2]/2.0 + (float)kk*dxyz[2];
          if(InSliceMesh(slicei, xyz)==0)continue;
          offsets[noffsets++] = GetSliceOffset(slicei, xyz, sdev->xyz);
          int is_dup;

          is_dup = 0;
          for(ll=0;ll<noffsets-1;ll++){
            if(offsets[ll] == offsets[noffsets-1]){
              is_dup = 1;;
              break;
            }
          }
          if(is_dup==1)noffsets--;
        }
      }
    }
    if(noffsets==0)continue;
    offset = GetSliceOffset(slicei, slice_xyz, sdev->xyz);
    sdev->valid = 1;
    FREEMEMORY(sdev->vals);
    FREEMEMORY(sdev->vals_orig);
    NewMemory((void **)&(sdev->vals), slicei->ntimes*sizeof(float));
    NewMemory((void **)&(sdev->vals_orig), slicei->ntimes*sizeof(float));
    sdev->nvals = slicei->ntimes;
    sdev->times = slicei->times;

    int j;

    for(j = 0; j<sdev->nvals; j++){
      sdev->vals[j] = 0.0;
    }
    for(ii=0;ii<noffsets;ii++){
      offset = offsets[ii];
      for(j = 0; j<sdev->nvals; j++){
        sdev->vals[j] += GetSliceVal(slicei, j, offset);
      }
    }
    for(j = 0; j<sdev->nvals; j++){
      sdev->vals[j] /= (float)noffsets;
      sdev->vals_orig[j] = sdev->vals[j];
    }
  }
  for(i = 0; i<nslicebounds; i++){
    boundsdata *sb;
    int j;

    sb = slicebounds+i;
    sb->dev_min = 1.0;
    sb->dev_max = 0.0;

    for(j = 0; j<nsliceinfo; j++){
      slicedata *slicej;
      float valmin, valmax;

      slicej = sliceinfo+j;
      if(slicej->loaded==0||strcmp(sb->label->longlabel, slicej->label.longlabel)!=0)continue;
      if(slice_plot_bound_option==1){
        valmin = slicej->valmin_fds;
        valmax = slicej->valmax_fds;
      }
      else{
        devicedata *devicej;

        devicej = &(slicej->vals2d);
        if(devicej->valid==0)continue;
        GetDevMinMax(devicej, &valmin, &valmax);
      }
      if(sb->dev_min>sb->dev_max){
        sb->dev_min = valmin;
        sb->dev_max = valmax;
      }
      else{
        sb->dev_min = MIN(valmin, sb->dev_min);
        sb->dev_max = MAX(valmax, sb->dev_max);
      }
    }
  }
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;
    devicedata *sdev;
    float time_average;

    slicei = sliceinfo+i;
    sdev = &(slicei->vals2d);
    if(slicei->loaded==0||slicei->ntimes==0)continue;
    if(InSliceMesh(slicei, slice_xyz)==0)continue;
    time_average = plot2d_time_average;
    if(average_plot2d_slice_region==0)time_average = 0.0;
    TimeAveragePlot2DData(sdev->times, sdev->vals_orig, sdev->vals, sdev->nvals, time_average);
  }
}

/* ------------------ DrawSlicePlots ------------------------ */

void DrawSlicePlots(void){
  int i;

  if(show_plot2d_slice_position==0)return;
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;
    devicedata *devicei;

    slicei = sliceinfo+i;
    devicei = &(slicei->vals2d);
    if(slicei->loaded==0||devicei->valid==0)continue;

    glPushMatrix();
    glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),vertical_factor*SCALE2SMV(1.0));
    glTranslatef(-xbar0,-ybar0,-zbar0);
    if(average_plot2d_slice_region==0||(slice_dxyz[0]<0.001&&slice_dxyz[1]<0.001&&slice_dxyz[2]<0.001)){
      glPointSize(10.0);
      glBegin(GL_POINTS);
      glColor3f(0.0,0.0,0.0);
      glVertex3fv(devicei->xyz);
      glEnd();
    }
    else{
      float slice_xb[6];

      slice_xb[0] = MAX(devicei->xyz[0] - slice_dxyz[0]/2.0, xbar0ORIG);
      slice_xb[1] = MIN(devicei->xyz[0] + slice_dxyz[0]/2.0, xbarORIG);
      slice_xb[2] = MAX(devicei->xyz[1] - slice_dxyz[1]/2.0, ybar0ORIG);
      slice_xb[3] = MIN(devicei->xyz[1] + slice_dxyz[1]/2.0, ybarORIG);
      slice_xb[4] = MAX(devicei->xyz[2] - slice_dxyz[2]/2.0, zbar0ORIG);
      slice_xb[5] = MIN(devicei->xyz[2] + slice_dxyz[2]/2.0, zbarORIG);

      glLineWidth(gridlinewidth);
      glBegin(GL_LINES);
      glColor3fv(foregroundcolor);
      glVertex3f(slice_xb[0], slice_xb[2], slice_xb[4]);
      glVertex3f(slice_xb[0], slice_xb[2], slice_xb[5]);
      glVertex3f(slice_xb[0], slice_xb[3], slice_xb[4]);
      glVertex3f(slice_xb[0], slice_xb[3], slice_xb[5]);
      glVertex3f(slice_xb[1], slice_xb[2], slice_xb[4]);
      glVertex3f(slice_xb[1], slice_xb[2], slice_xb[5]);
      glVertex3f(slice_xb[1], slice_xb[3], slice_xb[4]);
      glVertex3f(slice_xb[1], slice_xb[3], slice_xb[5]);

      glVertex3f(slice_xb[0], slice_xb[2], slice_xb[4]);
      glVertex3f(slice_xb[0], slice_xb[3], slice_xb[4]);
      glVertex3f(slice_xb[1], slice_xb[2], slice_xb[4]);
      glVertex3f(slice_xb[1], slice_xb[3], slice_xb[4]);
      glVertex3f(slice_xb[0], slice_xb[2], slice_xb[5]);
      glVertex3f(slice_xb[0], slice_xb[3], slice_xb[5]);
      glVertex3f(slice_xb[1], slice_xb[2], slice_xb[5]);
      glVertex3f(slice_xb[1], slice_xb[3], slice_xb[5]);

      glVertex3f(slice_xb[0], slice_xb[2], slice_xb[4]);
      glVertex3f(slice_xb[1], slice_xb[2], slice_xb[4]);
      glVertex3f(slice_xb[0], slice_xb[2], slice_xb[5]);
      glVertex3f(slice_xb[1], slice_xb[2], slice_xb[5]);
      glVertex3f(slice_xb[0], slice_xb[3], slice_xb[4]);
      glVertex3f(slice_xb[1], slice_xb[3], slice_xb[4]);
      glVertex3f(slice_xb[0], slice_xb[3], slice_xb[5]);
      glVertex3f(slice_xb[1], slice_xb[3], slice_xb[5]);
      glEnd();
    }
    glPopMatrix();
  }
}

/* ------------------ SetupSlice ------------------------ */

int SetupSlice(slicedata *sd){
  if(strcmp(sd->label.shortlabel, "ccell") != 0 && sd->slicefile_labelindex != slicefile_labelindex)return 0;
  if(sd->display == 0){
    if(showvslice == 0)return 0;
    if((sd->slice_filetype == SLICE_NODE_CENTER || sd->slice_filetype == SLICE_TERRAIN) && show_node_slices_and_vectors == 0)return 0;
    if(sd->slice_filetype == SLICE_CELL_CENTER){
      if(show_cell_slices_and_vectors == 0)return 0;
    }
  }
  if(current_script_command == NULL || current_script_command->command != SCRIPT_LOADSLICERENDER){
    // don't draw slice if the global time is before the first or after the last slice time
    if(global_times[itimes] < sd->times[0])return 0;
    if(global_times[itimes] > sd->times[sd->ntimes - 1])return 0;
  }
  if(sd->slice_filetype != SLICE_GEOM){
    if(sd->compression_type != UNCOMPRESSED){
      UncompressSliceDataFrame(sd, sd->itime);
      sd->iqsliceframe = sd->slicecomplevel;
    }
    else{
      sd->iqsliceframe = sd->slicelevel + sd->itime * sd->nsliceijk;
      sd->qslice = sd->qslicedata + sd->itime * sd->nsliceijk;
    }
    sd->qsliceframe = NULL;
#ifdef pp_MEMDEBUG
    if(sd->compression_type == UNCOMPRESSED && sd->qslicedata != NULL){
      ASSERT(ValidPointer(sd->qslicedata, sizeof(float) * sd->nslicetotal));
    }
#endif
    if(sd->qslicedata != NULL)sd->qsliceframe = sd->qslicedata + sd->itime * sd->nsliceijk;
  }
  return 1;
}

/* ------------------ DrawSliceFrame ------------------------ */

void DrawSliceFrame(){
  int ii;
  int jjj, nslicemax, blend_mode;
  int draw_slice;

  if(vis_slice_plot==1){
    DrawSlicePlots();
  }

  if(use_tload_begin==1 && global_times[itimes]<tload_begin)return;
  if(use_tload_end==1   && global_times[itimes]>tload_end)return;
  SortLoadedSliceList();

  if(sortslices==1){
    if(sortslices_debug == 1){
      DrawSortSlicesDebug();
    }
    else{
      DrawSortSlices();
    }
  }
  for(ii = 0; ii<nslice_loaded; ii++){
    slicedata *sd;
    int i;
    meshdata *slicemesh;
    int orien, direction;
    int jjjj;

    i=slice_sorted_loaded_list[ii];
    sd = sliceinfo + i;
    if(SetupSlice(sd) == 0)continue;
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
      if(slicemesh->smokedir<0)direction = -1;
      switch (ABS(slicemesh->smokedir)){
      case 4:  // -45 slope slices
        visy_all = 1;
        nslicemax = nploty_list;
        orien = 1;
        slice_normal[0] = direction*slicemesh->dyDdx;
        slice_normal[1] = direction*slicemesh->dxDdx;
        break;
      case 5:  // 45 slope slices
        visx_all = 1;
        nslicemax = nplotx_list;
        orien = 1;
        slice_normal[0] = -direction*slicemesh->dyDdx;
        slice_normal[1] =  direction*slicemesh->dxDdx;
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
      default:
      ASSERT(FFALSE);
      break;
      }
      nslicemax = MAX(nslicemax, 1);
      if(slices3d_max_blending==1){
        blend_mode=1;
        glBlendEquation(GL_MAX);
      }
      if(slice_opacity_adjustment==1)ComputeOpacityCorrections(slicemesh, eye_position_smv, slice_normal);
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

      switch(sd->slice_filetype){
      case SLICE_NODE_CENTER:
        if(orien==0){
          int is2;

          if(sd->volslice==1){
            is2 = sd->is1 + sd->nslicei - 1;
          }
          else{
            is2 = sd->is2;
          }
          if(sortslices==0){
            DrawVolSliceTexture(sd, sd->is1, is2, sd->js1, sd->js2, sd->ks1, sd->ks2, 0);
          }
          SNIFF_ERRORS("after DrawVolSliceTexture");
          if(show_slice_outlines[IN_SOLID_GLUI]==1||show_slice_outlines[IN_GAS_GLUI]==1){
            DrawVolSliceLines(sd);
            SNIFF_ERRORS("after DrawVolSliceLines SLICE_NODE_CENTER");
          }
          if(show_slice_points[IN_SOLID_GLUI]==1||show_slice_points[IN_GAS_GLUI]==1){
            DrawVolSliceVerts(sd);
            SNIFF_ERRORS("after DrawVolSliceVerts SLICE_NODE_CENTER");
          }
          if(show_slice_values[IN_SOLID_GLUI]==1||show_slice_values[IN_GAS_GLUI]==1){
            DrawVolSliceValues(sd);
            SNIFF_ERRORS("after DrawVolSliceValues SLICE_NODE_CENTER");
          }
        }
#ifdef pp_WINGPU
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
        {
          int is2;

          if(sd->volslice==1){
            is2 = sd->is1 + sd->nslicei - 1;
          }
          else{
            is2 = sd->is2;
          }
          if(sortslices==0||sd->volslice==1){
            DrawVolSliceCellFaceCenter(sd, SLICE_CELL_CENTER, 
                                       sd->is1, is2, sd->js1, sd->js2, sd->ks1, sd->ks2, 0);
          }
        }
        SNIFF_ERRORS("after DrawVolSliceCellFaceCenter SLICE_CELL_CENTER");
        if(show_slice_outlines[IN_SOLID_GLUI]==1||show_slice_outlines[IN_GAS_GLUI]==1){
          DrawVolSliceLines(sd);
          SNIFF_ERRORS("after DrawVolSliceLines SLICE_CELL_CENTER");
        }
        if(show_slice_points[IN_SOLID_GLUI]==1||show_slice_points[IN_GAS_GLUI]==1){
          DrawVolSliceVerts(sd);
          SNIFF_ERRORS("after DrawVolSliceVerts SLICE_CELL_CENTER");
        }
        if(show_slice_values[IN_SOLID_GLUI]==1||show_slice_values[IN_GAS_GLUI]==1){
          DrawVolSliceCellFaceCenterValues(sd, SLICE_CELL_CENTER);
          SNIFF_ERRORS("after DrawVolSliceVerts SLICE_CELL_CENTER");
        }
        break;
      case SLICE_TERRAIN:
        DrawVolSliceTerrain(sd);
        SNIFF_ERRORS("after DrawVolSliceTerrain");
        break;
      case SLICE_GEOM:
      // don't draw slice if also a vector slice unless you want to
      //     (set show_cell_slices_and_vectors or show_node_slices_and_vectors)
        draw_slice = 0;
        if(sd->vloaded==1){
          if(sd->cell_center==1&&show_cell_slices_and_vectors==1)draw_slice=1;
          if(sd->cell_center==0&&show_node_slices_and_vectors==1)draw_slice=1;
        }
        else{
          draw_slice=1;
        }
        if(draw_slice==1){
          DrawGeomData(DRAW_TRANSPARENT, sd, sd->patchgeom, GEOM_STATIC);
          DrawGeomData(DRAW_TRANSPARENT, sd, sd->patchgeom, GEOM_DYNAMIC);
        }
        if(show_slice_values[0]==1||show_slice_values[1]==1||show_slice_values[2]==1){
          DrawGeomValues(sd, sd->patchgeom, GEOM_STATIC);
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
    }
    if(orien==1&&sd->slice_filetype==SLICE_NODE_CENTER){
      DrawVolAllSlicesTextureDiag(sd,direction);
      SNIFF_ERRORS("after DrawVolAllSlicesTextureDiag");
    }
    if(blend_mode==1){
      glBlendEquation(GL_FUNC_ADD);
    }
  }
  for (ii = 0; ii < npatchinfo; ii++) {
    patchdata *patchi;

    patchi = patchinfo + ii;
    if(patchi->boundary==0 && patchi->loaded == 1 && patchi->display == 1){
      DrawGeomData(DRAW_TRANSPARENT, NULL, patchi, GEOM_STATIC);
      DrawGeomData(DRAW_TRANSPARENT, NULL, patchi, GEOM_DYNAMIC);
    }
  }
}

/* ------------------ DrawVVolSliceCellCenter ------------------------ */

void DrawVVolSliceCellCenter(const vslicedata *vd){
  int i;
  int ii, jj, kk;
  float constval;
  slicedata *u, *v, *w, *sd;
  float vel_max;
  meshdata *meshi;
  float *xplttemp, *yplttemp, *zplttemp;
  int plotx, ploty, plotz;
  char *iblank_cell;
  int ibar, jbar;

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
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  iblank_cell = meshi->c_iblank_cell;
  vel_max = max_velocity;
  if(vel_max<= 0.0)vel_max = 1.0;
  u = vd->u;
  v = vd->v;
  w = vd->w;
  if((vd->volslice == 1 && plotx > 0 && visx_all == 1) || (vd->volslice == 0 && sd->idir == XDIR)){
    int j;
    float xhalf;
    if(plotx > 0){
      xhalf = (xplttemp[plotx] + xplttemp[plotx - 1]) / 2.0;
    }
    else{
      xhalf = xplttemp[plotx];
    }
    int plotxm1;
    plotxm1 = MAX(plotx-1, 0);

    constval = xhalf + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    glColor4fv(foregroundcolor);
    for(jj = 0; jj < sd->n_jmap; jj++){
      j = sd->jmap[jj];
      float yy1, yy2, yhalf;
      int k;

      yy1 = yplttemp[j];
      if(j + 1 <= sd->js2){
        yy2 = yplttemp[j + 1];
      }
      else{
        yy2 = yy1;
      }
      yhalf = (yy1+yy2) / 2.0;
    for(kk = 0; kk < sd->n_kmap; kk++){
      k = sd->kmap[kk];
        float zhalf, z1;
        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(plotxm1, j, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }

        z1 = zplttemp[k];
        if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

        //       n = (j-sd->js1)*sd->nslicek - 1;
        //       n += (plotx-sd->is1)*sd->nslicej*sd->nslicek;

        if(k != sd->ks2){
          int index_v;
          float dy;

          index_v = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
          GET_VEC_DXYZ(v, dy, index_v);
          ADJUST_VEC_DX(dy);
          glVertex3f(constval, yy1 - dy, zhalf);
          glVertex3f(constval, yy1 + dy, zhalf);
        }
        if(j + 1 <= sd->js2){
          int index_w;
          float dz;

          index_w = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1 + 1)*sd->nslicek + k - sd->ks1;
          GET_VEC_DXYZ(w, dz, index_w);
          ADJUST_VEC_DX(dz);
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
    for(jj = 0; jj < sd->n_jmap; jj++){
      j = sd->jmap[jj];
      float yy1, yy2, yhalf;
      int k;

      yy1 = yplttemp[j];
      if(j + 1 <= sd->js2){
        yy2 = yplttemp[j + 1];
      }
      else{
        yy2 = yy1;
      }
      yhalf = (yy1+yy2) / 2.0;
    for(kk = 0; kk < sd->n_kmap; kk++){
      k = sd->jmap[kk];
        float zhalf, z1;

        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(plotxm1, j, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }

        z1 = zplttemp[k];
        if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

        if(k != sd->ks2){
          int index_v;
          float dy;

          index_v = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1)*sd->nslicek + k - sd->ks1 + 1;
          GET_VEC_DXYZ(v, dy, index_v);
          ADJUST_VEC_DX(dy);
          glVertex3f(constval, yy1 + dy, zhalf);
        }
        if(j + 1 <= sd->js2){
          int index_w;
          float dz;

          index_w = (plotx - sd->is1)*sd->nslicej*sd->nslicek + (j - sd->js1 + 1)*sd->nslicek + k - sd->ks1;
          GET_VEC_DXYZ(w, dz, index_w);
          ADJUST_VEC_DX(dz);
          glVertex3f(constval, yhalf, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:points dir=1");
  }
  if((vd->volslice == 1 && ploty > 0 && visy_all == 1) || (vd->volslice == 0 && sd->idir == YDIR)){
    float yhalf;

    if(ploty > 0){
      yhalf = (yplttemp[ploty] + yplttemp[ploty - 1]) / 2.0;
    }
    else{
      yhalf = yplttemp[ploty];
    }

    constval = yhalf + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    glColor4fv(foregroundcolor);
    for(ii = 0; ii < sd->n_imap; ii++){
      i = sd->imap[ii];

      float x1, xhalf;
      int k;

      // n = (i-sd->is1)*sd->nslicej*sd->nslicek - 1;
      // n += (ploty-sd->js1)*sd->nslicek;

      x1 = xplttemp[i];
      if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;

      for(kk = 0; kk < sd->n_kmap; kk++){
        k = sd->kmap[kk];
        float zhalf, z1;
        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(i, ploty-1, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_slice_shaded[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_slice_shaded[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }

        z1 = zplttemp[k];
        if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

        if(k + 1 != sd->nslicek){
          int index_u;
          float dx;

          index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
          GET_VEC_DXYZ(u, dx, index_u);
          ADJUST_VEC_DX(dx);
          glVertex3f(x1 - dx, constval, zhalf);
          glVertex3f(x1 + dx, constval, zhalf);
        }
        if(i + 1 != sd->nslicei){
          int index_w;
          float dz;

          index_w = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k - sd->ks1;
          GET_VEC_DXYZ(w, dz, index_w);
          ADJUST_VEC_DX(dz);
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
    for(ii = 0; ii < sd->n_imap; ii++){
      i = sd->imap[ii];
      float x1, xhalf;
      int k;

      // n = (i-sd->is1)*sd->nslicej*sd->nslicek - 1;
      // n += (ploty-sd->js1)*sd->nslicek;

      x1 = xplttemp[i];
      if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;

      for(kk = 0; kk < sd->n_kmap; kk++){
        k = sd->kmap[kk];
        float zhalf, z1;
        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(i, ploty-1, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }

        z1 = zplttemp[k];
        if(k + 1 != sd->nslicek)zhalf = (zplttemp[k] + zplttemp[k + 1]) / 2.0;

        if(k +1 != sd->nslicek){
          int index_u;
          float dx;

          index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k + 1 - sd->ks1;
          GET_VEC_DXYZ(u, dx, index_u);
          ADJUST_VEC_DX(dx);
          glVertex3f(x1 + dx, constval, zhalf);
        }
        if(i + 1 != sd->nslicei){
          int index_w;
          float dz;

          index_w = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (ploty - sd->js1)*sd->nslicek + k - sd->ks1;
          GET_VEC_DXYZ(w, dz, index_w);
          ADJUST_VEC_DX(dz);
          glVertex3f(xhalf, constval, z1 + dz);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:points dir=2");
  }
  if((vd->volslice == 1 && plotz > 0 && visz_all == 1) || (vd->volslice == 0 && sd->idir == ZDIR)){
    float zhalf;

    if(plotz > 0){
      zhalf = (zplttemp[plotz] + zplttemp[plotz - 1]) / 2.0;
    }
    else{
      zhalf = zplttemp[plotz];
    }

    constval = zhalf + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    glColor4fv(foregroundcolor);
    for(ii = 0; ii < sd->n_imap; ii++){
      i = sd->imap[ii];
      float xhalf;
      float x1;
      int j;

      //      n = (i-sd->is1)*sd->nslicej*sd->nslicek - sd->nslicek;
      //      n += (plotz-sd->ks1);

      x1 = xplttemp[i];
      if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;
      for(jj = 0; jj < sd->n_jmap; jj++){
        j = sd->jmap[jj];
        float yhalf;
        float yy1;
        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(i, j, plotz-1)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }

        yy1 = yplttemp[j];
        if(j + 1 != sd->nslicej)yhalf = (yplttemp[j] + yplttemp[j + 1]) / 2.0;

        if(j + 1 != sd->nslicej){
          int index_u;
          float dx;

          index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j + 1 - sd->js1)*sd->nslicek;
          GET_VEC_DXYZ(u, dx, index_u);
          ADJUST_VEC_DX(dx);
          glVertex3f(x1 - dx, yhalf, constval);
          glVertex3f(x1 + dx, yhalf, constval);
        }
        if(i + 1 != sd->nslicei){
          int index_v;
          float dy;

          index_v = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j - sd->js1)*sd->nslicek;
          GET_VEC_DXYZ(v, dy, index_v);
          ADJUST_VEC_DX(dy);
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
    for(ii = 0; ii < sd->n_imap; ii++){
      i = sd->imap[ii];
      float xhalf;
      float x1;
      int j;

      //      n = (i-sd->is1)*sd->nslicej*sd->nslicek - sd->nslicek;
      //      n += (plotz-sd->ks1);

      x1 = xplttemp[i];
      if(i + 1 != sd->nslicei)xhalf = (xplttemp[i] + xplttemp[i + 1]) / 2.0;
      for(jj = 0; jj < sd->n_jmap; jj++){
        j = sd->jmap[jj];
        float yhalf;
        float yy1;
        int in_solid, in_gas;

        in_gas=1;
        if(iblank_cell != NULL&&iblank_cell[IJKCELL(i, j, plotz-1)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank_cell!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }

        yy1 = yplttemp[j];
        if(j + 1 != sd->nslicej)yhalf = (yplttemp[j] + yplttemp[j + 1]) / 2.0;

        if(j + 1 != sd->nslicej){
          int index_u;
          float dx;

          index_u = (i - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j + 1 - sd->js1)*sd->nslicek;
          GET_VEC_DXYZ(u, dx, index_u);
          ADJUST_VEC_DX(dx);
          glVertex3f(x1 + dx, yhalf, constval);
        }
        if(i + 1 != sd->nslicei){
          int index_v;
          float dy;

          index_v = (i + 1 - sd->is1)*sd->nslicej*sd->nslicek + (plotz - sd->ks1) + (j - sd->js1)*sd->nslicek;
          GET_VEC_DXYZ(v, dy, index_v);
          ADJUST_VEC_DX(dy);
          glVertex3f(xhalf, yy1 + dy, constval);
        }
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSliceCellCenter:points dir=3");
  }
}

/* ------------------ DrawVVolSliceTerrain ------------------------ */

void DrawVVolSliceTerrain(const vslicedata *vd){
  int ii, jj;
  int i;
  slicedata *u, *v, *w, *sd;
  float vel_max;
  meshdata *meshi;
  float *xplttemp, *yplttemp;
  float *znode;
  int nycell;
  int plotz;

  sd = sliceinfo + vd->ival;
  meshi = meshinfo + sd->blocknumber;
  xplttemp = meshi->xplt;
  yplttemp = meshi->yplt;
  if(vd->volslice == 1){
    plotz = meshi->iplotz_all[iplotz_all];
  }
  else{
    plotz = sd->ks1;
  }

  nycell = meshi->jbar;

  vel_max = max_velocity;
  if(vel_max<= 0.0)vel_max = 1.0;
  u = vd->u;
  v = vd->v;
  w = vd->w;

  float valmin, valmax;

  valmin = sd->valmin;
  valmax = sd->valmax;
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }

  if((vd->volslice == 1 && plotz >= 0 && visz_all == 1) || (vd->volslice == 0 && sd->idir == ZDIR)){
    float agl_smv;
    float zmin, zmax, voffset;

    xplttemp = meshi->xplt_orig;
    yplttemp = meshi->yplt_orig;
    znode = meshi->znodes_complete;

    agl_smv = sd->above_ground_level;
    if(agl_offset_actual==1){
      voffset = agl_smv;
    }
    else{
      voffset = MAX(agl_smv, slice_dz);
    }

    zmin  = meshi->zplt_orig[0];
    zmin -= agl_smv;
    zmax  = meshi->zplt_orig[meshi->kbar];
    zmax -= agl_smv;

    glPushMatrix();
    glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),vertical_factor*SCALE2SMV(1.0));
    glTranslatef(-xbar0,-ybar0,-zbar0 + voffset);

    glLineWidth(vectorlinewidth);

    if(vector_debug==0){
      glBegin(GL_LINES);
      for(ii = 0; ii <sd->n_imap; ii++){
        i = sd->imap[ii];
        float x1;
        int j;

        x1 = xplttemp[i];
        for(jj = 0; jj < sd->n_jmap; jj++){
          j = sd->jmap[jj];
          int n11;
          float z11;
          float *rgb_ptr;

          z11 = znode[IJ2(i, j)];
          if(z11<zmin || z11>zmax)continue;

          n11 = IJK_SLICE(i,j, sd->ks1);
          if(color_vector_black==0&&show_node_slices_and_vectors==0){
            int i11;

            i11 = SLICECOLOR(n11);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = foregroundcolor;
          }
          if(rgb_ptr[3] > 0.5){
            float yy1;
            float dx, dy, dz;

            yy1 = yplttemp[j];
            GET_VEC_DXYZ_TERRAIN(u, dx, n11);
            GET_VEC_DXYZ_TERRAIN(v, dy, n11);
            GET_VEC_DXYZ_TERRAIN(w, dz, n11);
            ADJUST_VEC_DXYZ(dx,dy,dz);

            glColor4fv(rgb_ptr);
            glVertex3f(x1 - dx, yy1 - dy, z11 - dz);
            glVertex3f(x1 + dx, yy1 + dy, z11 + dz);
          }
        }
      }
      glEnd();

      glPointSize(vectorpointsize);
      glBegin(GL_POINTS);
      for(ii = 0; ii < sd->n_imap; ii++){
        i = sd->imap[ii];
        float x1;
        int j;

        x1 = xplttemp[i];
        for(jj = 0; jj < sd->n_jmap; jj++){
          j = sd->jmap[jj];
          int n11;
          float z11;
          float *rgb_ptr;

          z11 = znode[IJ2(i, j)];
          if(z11<=zmin || z11>=zmax)continue;

          n11 = IJK_SLICE(i,j, sd->ks1);
          if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
            int i11;

            i11 = SLICECOLOR(n11);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = foregroundcolor;
          }
          if(rgb_ptr[3] > 0.5){
            float yy1;
            float dx, dy, dz;

            yy1 = yplttemp[j];
            GET_VEC_DXYZ_TERRAIN(u, dx, n11);
            GET_VEC_DXYZ_TERRAIN(v, dy, n11);
            GET_VEC_DXYZ_TERRAIN(w, dz, n11);
            ADJUST_VEC_DXYZ(dx,dy,dz);
            glColor4fv(rgb_ptr);
            glVertex3f(x1 + dx, yy1 + dy, z11 + dz);
          }
        }
      }
      glEnd();
    }

    if(vector_debug==1){
      int j;

      glBegin(GL_LINES);
      for(i = sd->is1; i<sd->is2; i++){
        float x1;

        x1 = xplttemp[i];

        for(j = sd->js1; j<sd->js2; j++){
          int n11;
          float z11, z22;
          float *rgb_ptr;

          z11 = znode[IJ2(i, j)];
          z22 = znode[IJ2(i, j+1)];
          if(z11<zmin || z11>zmax)continue;
          if(z22<zmin || z22>zmax)continue;
          n11 = i*sd->nslicej*sd->nslicek+j*sd->nslicek;
          if(color_vector_black==0&&show_node_slices_and_vectors==0){
            int i11;

            i11 = SLICECOLOR(n11);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = foregroundcolor;
          }
          if(rgb_ptr[3]>0.5){
            float yy1, yy2;

            yy1 = yplttemp[j];
            yy2 = yplttemp[j+1];

            glColor4fv(rgb_ptr);
            glVertex3f(x1, yy1, z11);
            glVertex3f(x1, yy2, z22);
          }
        }
      }
      for(j = sd->js1; j<sd->js2; j++){
        float yy1;

        yy1 = yplttemp[j];
        for(i = sd->is1; i<sd->is2; i++){
          float z11, z22;
          float *rgb_ptr;

          z11 = znode[IJ2(i, j)];
          z22 = znode[IJ2(i+1, j)];
          if(z11<zmin || z11>zmax)continue;
          if(z22<zmin || z22>zmax)continue;
          rgb_ptr = foregroundcolor;
          if(rgb_ptr[3]>0.5){
            float x1, x2;

            x1 = xplttemp[i];
            x2 = xplttemp[i+1];

            glColor4fv(rgb_ptr);
            glVertex3f(x1, yy1, z11);
            glVertex3f(x2, yy1, z22);
          }
        }
      }
      glEnd();
    }
    glPopMatrix();
    SNIFF_ERRORS("after DrawVVolSliceTerrain dir=3");
  }
}

/* ------------------ DrawVVolSlice ------------------------ */

void DrawVVolSlice(const vslicedata *vd){
  int i, j, k, n;
  int ii, jj, kk;
  int i11;
  float constval, x1, yy1, z1;
  slicedata *u, *v, *w, *sd;
  float dx, dy, dz;
  float vel_max;
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

  float valmin, valmax;

  valmin = sd->valmin;
  valmax = sd->valmax;
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }
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

  vel_max = max_velocity;
  if(vel_max<= 0.0)vel_max = 1.0;
  u = vd->u;
  v = vd->v;
  w = vd->w;
  if((vd->volslice == 1 && plotx >= 0 && visx_all == 1) || (vd->volslice == 0 && sd->idir == XDIR)){
    constval = xplttemp[plotx] + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    for(jj = 0; jj < sd->n_jmap; jj++){
      j = sd->jmap[jj];
      yy1 = yplttemp[j];
      for(kk = 0; kk < sd->n_kmap; kk++){
        k = sd->kmap[kk];
        n = IJK_SLICE(plotx,j,k);
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = SLICECOLOR(n);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(plotx, j, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.1)continue;

        z1 = zplttemp[k];
        GET_VEC_DXYZ(u, dx, n);
        GET_VEC_DXYZ(v, dy, n);
        GET_VEC_DXYZ(w, dz, n);
        ADJUST_VEC_DXYZ(dx,dy,dz);
        glColor4fv(rgb_ptr);
        glVertex3f(constval - dx, yy1 - dy, z1 - dz);
        glVertex3f(constval + dx, yy1 + dy, z1 + dz);
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:lines dir=1");

    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    for(jj = 0; jj < sd->n_jmap; jj++){
      j = sd->jmap[jj];
      yy1 = yplttemp[j];
      for(kk = 0; kk < sd->n_kmap; kk++){
        k = sd->kmap[kk];
        n = IJK_SLICE(plotx,j,k);
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = SLICECOLOR(n);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(plotx, j, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.1)continue;

        z1 = zplttemp[k];
        GET_VEC_DXYZ(u, dx, n);
        GET_VEC_DXYZ(v, dy, n);
        GET_VEC_DXYZ(w, dz, n);
        ADJUST_VEC_DXYZ(dx,dy,dz);
        glColor4fv(rgb_ptr);
        glVertex3f(constval + dx, yy1 + dy, z1 + dz);
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:points dir=1");
  }
  if((vd->volslice == 1 && ploty >= 0 && visy_all == 1) || (vd->volslice == 0 && sd->idir == YDIR)){
    constval = yplttemp[ploty] + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    for(ii = 0; ii < sd->n_imap; ii++){
      i = sd->imap[ii];
      x1 = xplttemp[i];
      for(kk = 0; kk < sd->n_kmap; kk++){
        k = sd->kmap[kk];
        n = IJK_SLICE(i,ploty,k);
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = SLICECOLOR(n);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(i, ploty, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.1)continue;

        z1 = zplttemp[k];
        GET_VEC_DXYZ(u, dx, n);
        GET_VEC_DXYZ(v, dy, n);
        GET_VEC_DXYZ(w, dz, n);
        ADJUST_VEC_DXYZ(dx,dy,dz);
        glColor4fv(rgb_ptr);
        glVertex3f(x1 - dx, constval - dy, z1 - dz);
        glVertex3f(x1 + dx, constval + dy, z1 + dz);
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:lines dir=2");
    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    for(ii = 0; ii < sd->n_imap; ii++){
      i = sd->imap[ii];
      x1 = xplttemp[i];
      for(kk = 0; kk < sd->n_kmap; kk++){
        k = sd->kmap[kk];
        n = IJK_SLICE(i,ploty,k);
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = SLICECOLOR(n);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(i, ploty, k)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.1)continue;

        z1 = zplttemp[k];
        GET_VEC_DXYZ(u, dx, n);
        GET_VEC_DXYZ(v, dy, n);
        GET_VEC_DXYZ(w, dz, n);
        ADJUST_VEC_DXYZ(dx,dy,dz);
        glColor4fv(rgb_ptr);
        glVertex3f(x1 + dx, constval + dy, z1 + dz);
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:points dir=2");
  }
  if((vd->volslice == 1 && plotz >= 0 && visz_all == 1) || (vd->volslice == 0 && sd->idir == ZDIR)){
    constval = zplttemp[plotz] + offset_slice*sd->sliceoffset+SCALE2SMV(slice_dz);
    glLineWidth(vectorlinewidth);
    glBegin(GL_LINES);
    for(ii = 0; ii < sd->n_imap; ii++){
      i = sd->imap[ii];
      x1 = xplttemp[i];
      for(jj = 0; jj < sd->n_jmap; jj++){
        j = sd->jmap[jj];
        n = IJK_SLICE(i,j,plotz);
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = SLICECOLOR(n);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(i, j, plotz)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.1)continue;

        yy1 = yplttemp[j];
        GET_VEC_DXYZ(u, dx, n);
        GET_VEC_DXYZ(v, dy, n);
        GET_VEC_DXYZ(w, dz, n);
        ADJUST_VEC_DXYZ(dx,dy,dz);
        glColor4fv(rgb_ptr);
        glVertex3f(x1 - dx, yy1 - dy, constval - dz);
        glVertex3f(x1 + dx, yy1 + dy, constval + dz);
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:lines dir=3");

    glPointSize(vectorpointsize);
    glBegin(GL_POINTS);
    for(ii = 0; ii < sd->n_imap; ii++){
      i = sd->imap[ii];
      x1 = xplttemp[i];
      for(jj = 0; jj < sd->n_jmap; jj++){
        j = sd->jmap[jj];
        n = IJK_SLICE(i,j,plotz);
        if(color_vector_black == 0 && show_node_slices_and_vectors == 0){
          if(sd->constant_color == NULL){
            i11 = SLICECOLOR(n);
            rgb_ptr = rgb_slice + 4 * i11;
          }
          else{
            rgb_ptr = sd->constant_color;
          }
        }
        else{
          rgb_ptr = foregroundcolor;
        }
        int in_solid, in_gas;

        in_gas=1;
        if(iblank != NULL&&iblank[IJK(i, j, plotz)] != GAS)in_gas=0;
        in_solid = 1 - in_gas;

        if(iblank!=NULL){
          if(show_vector_slice[IN_SOLID_GLUI]==0 && in_solid==1)continue;
          if(show_vector_slice[IN_GAS_GLUI]==0   && in_gas==1)continue;
        }
        if(rgb_ptr[3]<0.1)continue;

        yy1 = yplttemp[j];
        GET_VEC_DXYZ(u, dx, n);
        GET_VEC_DXYZ(v, dy, n);
        GET_VEC_DXYZ(w, dz, n);
        ADJUST_VEC_DXYZ(dx,dy,dz);
        glColor4fv(rgb_ptr);
        glVertex3f(x1 + dx, yy1 + dy, constval + dz);
      }
    }
    glEnd();
    SNIFF_ERRORS("after DrawVVolSlice:points dir=3");
  }
}

/* ------------------ DrawVSliceFrame ------------------------ */

void DrawVSliceFrame(void){
  int i;

  if(use_tload_begin==1 && global_times[itimes]<tload_begin)return;
  if(use_tload_end==1   && global_times[itimes]>tload_end)return;
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
    if(vd->vslice_filetype!=SLICE_GEOM){
      if(val->compression_type!=UNCOMPRESSED){
        UncompressSliceDataFrame(val, val->itime);
        val->iqsliceframe = val->slicecomplevel;
      }
      else{
        if(val!=NULL){
          val->iqsliceframe = val->slicelevel+val->itime*val->nsliceijk;
          val->qslice = val->qslicedata+val->itime*val->nsliceijk;
        }
      }
      if(val->qslicedata!=NULL)val->qsliceframe = val->qslicedata+val->itime*val->nsliceijk;
      if(u!=NULL){
        if(u->compression_type!=UNCOMPRESSED){
          UncompressSliceDataFrame(u, u->itime);
          u->iqsliceframe = u->slicecomplevel;
        }
        else{
          if(u!=NULL)u->iqsliceframe = u->slicelevel+u->itime*u->nsliceijk;
        }
      }
      if(v!=NULL){
        if(v->compression_type!=UNCOMPRESSED){
          UncompressSliceDataFrame(v, v->itime);
          v->iqsliceframe = v->slicecomplevel;
        }
        else{
          if(v!=NULL)v->iqsliceframe = v->slicelevel+v->itime*v->nsliceijk;
        }
      }
      if(w!=NULL){
        if(w->compression_type!=UNCOMPRESSED){
          UncompressSliceDataFrame(w, w->itime);
          w->iqsliceframe = w->slicecomplevel;
        }
        else{
          if(w!=NULL)w->iqsliceframe = w->slicelevel+w->itime*w->nsliceijk;
        }
      }
      if(u!=NULL&&u->compression_type==UNCOMPRESSED){
        u->qslice = u->qslicedata+u->itime*u->nsliceijk;
      }
      if(v!=NULL&&v->compression_type==UNCOMPRESSED){
        v->qslice = v->qslicedata+v->itime*v->nsliceijk;
      }
      if(w!=NULL&&w->compression_type==UNCOMPRESSED){
        w->qslice = w->qslicedata+w->itime*w->nsliceijk;
      }
    }

    if(vd->vslice_filetype==SLICE_TERRAIN){
      DrawVVolSliceTerrain(vd);
    }
    else if(vd->vslice_filetype==SLICE_CELL_CENTER){
      DrawVVolSliceCellCenter(vd);
    }
    else if(vd->vslice_filetype==SLICE_GEOM){
      DrawGeomVData(vd);
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
  valmin = sb->levels256[0];
  valmax = sb->levels256[255];

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
  valmin = sb->levels256[0];
  valmax = sb->levels256[255];

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


/* ------------------ ISSliceMenuDup ------------------------ */

int IsSliceMenuDup(slicemenudata *slicemenu_arg, int nslicemenuinfo_arg, char *label, int slcf_index, float position_arg){
  int i;

  for(i = 0; i<nslicemenuinfo_arg; i++){
    slicemenudata *slicemi;
    slicedata *slicei;

    slicemi = slicemenu_arg+i;
    slicei = slicemi->sliceinfo;
    if(
      slicei->slcf_index==slcf_index&&
      strcmp(slicei->label.longlabel, label)==0&&
      ABS(slicei->position_orig-position_arg)<0.001
      )return 1;
  }
  return 0;
}

/* ------------------ CompareSliceMenuInfo ------------------------ */

int CompareSliceMenuInfo(const void *arg1, const void *arg2){
  slicemenudata *sm1, *sm2;
  slicedata *sf1, *sf2;
  int compare_label;
  char *label1, *label2;

  sm1 = *(slicemenudata **)arg1;
  sm2 = *(slicemenudata **)arg2;
  sf1 = sm1->sliceinfo;
  sf2 = sm2->sliceinfo;
  label1 = sf1->label.longlabel;
  label2 = sf2->label.longlabel;
  compare_label = strcmp(label1, label2);

  if(sf1->slice_filetype>sf2->slice_filetype)return  1;
  if(sf1->slice_filetype<sf2->slice_filetype)return -1;

  if(compare_label>0)return  1;
  if(compare_label<0)return -1;

  if(sf1->idir>sf2->idir)return  1;
  if(sf1->idir<sf2->idir)return -1;

  if(sf1->position_orig-sf2->position_orig> 0.001)return  1;
  if(sf1->position_orig-sf2->position_orig<-0.001)return -1;

  return 0;
}

/* ------------------ GenerateSliceMenu ------------------------ */

void GenerateSliceMenu(int option){
  char slicemenu_filename[256];
  int i;
  FILE *stream = NULL;

  if(nsliceinfo==0)return;

  strcpy(slicemenu_filename, "");
  if(smokeview_scratchdir!=NULL){
    strcat(slicemenu_filename, smokeview_scratchdir);
    strcat(slicemenu_filename, dirseparator);
  }
  strcat(slicemenu_filename, fdsprefix);
  strcat(slicemenu_filename, ".slcf");

  // if we can't write out to the slice menu file then abort

  nslicemenuinfo = 0;
  NewMemory((void **)&slicemenuinfo, nsliceinfo*sizeof(slicemenudata));
  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;
    slicemenudata *slicemi;

    slicei = sliceinfo+i;
    if(slicei->volslice==1)continue;
    if(IsSliceMenuDup(slicemenuinfo, nslicemenuinfo, slicei->label.longlabel, slicei->slcf_index, slicei->position_orig)==1)continue;
    slicemi = slicemenuinfo+nslicemenuinfo;
    slicemi->sliceinfo = slicei;
    nslicemenuinfo++;
  }

  if(nslicemenuinfo==0){
    FREEMEMORY(slicemenuinfo);
    return;
  }
  NewMemory((void **)&slicemenu_sorted, nslicemenuinfo*sizeof(slicemenudata));
  for(i = 0; i<nslicemenuinfo; i++){
    slicemenu_sorted[i] = slicemenuinfo+i;
  }
  qsort((slicemenudata **)slicemenu_sorted, (size_t)nslicemenuinfo, sizeof(slicemenudata *), CompareSliceMenuInfo);

  if(option==1){
    int max1 = 0, max2 = 0, max3 = 0, max4 = 0;

    stream = fopen(slicemenu_filename, "w");
    if(stream==NULL)return;
    for(i = 0; i<nslicemenuinfo; i++){
      slicedata *slicei;
      slicemenudata *slicemi;
      char *quantity, cposition[25];

      slicemi = slicemenu_sorted[i];
      slicei = slicemi->sliceinfo;
      quantity = slicei->label.longlabel;
      if((int)strlen(quantity)>max2)max2 = strlen(quantity);
      sprintf(cposition, "%f", slicei->position_orig);
      TrimZeros(cposition);
      if((int)strlen(cposition)>max4)max4 = strlen(cposition);
    }

    max1 = 5;
    max2 = MAX(5, max2) + 1;
    max3 = 4;
    max4 = MAX(8, max4) + 1;
    char cform1[20], cform2[20], cform3[20], cform4[20];
    sprintf(cform1, "%s%i.%is", "%",max1,max1);/* %20.20s*/
    sprintf(cform2, "%s-%i.%is", "%", max2,max2);
    sprintf(cform3, "%s%i.%is", "%", max3,max3);
    sprintf(cform4, "%s%i.%is", "%", max4,max4);

    char format[256];
    sprintf(format, "%s, %s, %s, %s\n",cform1, cform2, cform3, cform4);

    fprintf(stream, "\n");
    fprintf(stream, format, "index", "quantity", "dir", "position");
#ifdef _DEBUG
    printf("\n");
    printf(format, "ind", "quantity", "dir", "pos");
#endif
    for(i = 0; i<nslicemenuinfo; i++){
      slicedata *slicei;
      slicemenudata *slicemi;
      char *quantity, cposition[25];
      float position;
      char index[10], cdir[10];

      slicemi = slicemenu_sorted[i];
      slicei = slicemi->sliceinfo;
      quantity = slicei->label.longlabel;
      sprintf(cdir, "%i", slicei->idir);
      position = slicei->position_orig;
      sprintf(cposition, "%f", position);
      TrimZeros(cposition);
      sprintf(index, "%i", i+1);
      fprintf(stream, format, index, quantity, cdir, cposition);
#ifdef _DEBUG
      printf(format, index, quantity, cdir, cposition);
#endif
    }
    fclose(stream);
  }
}

/* ------------------ CompareSliceX ------------------------ */

int CompareSliceX(const void *arg1, const void *arg2){
  slicedata *sf1, *sf2;

  sf1 = *(slicedata **)arg1;
  sf2 = *(slicedata **)arg2;

  if(sf1->xmin<sf2->xmin)return  -1;
  if(sf1->xmin>sf2->xmin)return   1;

  return 0;
}

/* ------------------ CompareSliceZ ------------------------ */

int CompareSliceY(const void *arg1, const void *arg2){
  slicedata *sf1, *sf2;

  sf1 = *(slicedata **)arg1;
  sf2 = *(slicedata **)arg2;

  if(sf1->ymin<sf2->ymin)return  -1;
  if(sf1->ymin>sf2->ymin)return  +1;

  return 0;
}

/* ------------------ CompareSliceZ ------------------------ */

int CompareSliceZ(const void *arg1, const void *arg2){
  slicedata *sf1, *sf2;

  sf1 = *(slicedata **)arg1;
  sf2 = *(slicedata **)arg2;

  if(sf1->zmin<sf2->zmin)return  -1;
  if(sf1->zmin>sf2->zmin)return   1;

  return 0;
}

/* ------------------ CompareSortSlices ------------------------ */

int CompareSortSlices(const void *arg1, const void *arg2){
  splitslicedata *s1, *s2;
  meshdata *m1, *m2;
  float *x1, *y1, *z1;
  float *x2, *y2, *z2;
  float dist1, dist2;

  s1 = *(splitslicedata **)arg1;
  s2 = *(splitslicedata **)arg2;
  m1 = s1->mesh;
  m2 = s2->mesh;
  x1 = m1->xplt;
  y1 = m1->yplt;
  z1 = m1->zplt;
  x2 = m2->xplt;
  y2 = m2->yplt;
  z2 = m2->zplt;
  float dx1, dy1, dz1;
  float dx2, dy2, dz2;

  dx1 = (x1[s1->is1]+x1[s1->is2])/2.0 - smv_eyepos[0];
  dy1 = (y1[s1->js1]+y1[s1->js2])/2.0 - smv_eyepos[1];
  dz1 = (z1[s1->ks1]+z1[s1->ks2])/2.0 - smv_eyepos[2];
  dist1 = sqrt(dx1*dx1+dy1*dy1+dz1*dz1);
  dx2 = (x2[s2->is1]+x2[s2->is2])/2.0 - smv_eyepos[0];
  dy2 = (y2[s2->js1]+y2[s2->js2])/2.0 - smv_eyepos[1];
  dz2 = (z2[s2->ks1]+z2[s2->ks2])/2.0 - smv_eyepos[2];
  dist2 = sqrt(dx2*dx2+dy2*dy2+dz2*dz2);

  if(dist1 < dist2)return 1;
  if(dist1 > dist2)return -1;
  return 0;
}

/* ------------------ SortSlices ------------------------ */

void SortSlices(void){
  int i;
  slicedata **slicex0, **slicey0, **slicez0;

  if(slicex==NULL)NewMemory((void **)&slicex, nsliceinfo*sizeof(slicedata *));
  if(slicey==NULL)NewMemory((void **)&slicey, nsliceinfo*sizeof(slicedata *));
  if(slicez==NULL)NewMemory((void **)&slicez, nsliceinfo*sizeof(slicedata *));
  nsplitsliceinfo = 0;
  slicex0 = slicex;
  slicey0 = slicey;
  slicez0 = slicez;
  for(i = 0;i < nmeshes;i++){
    int j, nx, ny, nz;
    meshdata *meshi;

    meshi = meshinfo + i;
    meshi->slicex = slicex0;
    meshi->slicey = slicey0;
    meshi->slicez = slicez0;
    nx = 0;
    ny = 0;
    nz = 0;
    for(j = 0;j < nsliceinfo;j++){
      slicedata *slicej;

      slicej = sliceinfo + j;

      if(slicej->loaded == 0 || slicej->blocknumber != i)continue;
      if(slicej->slice_filetype!=SLICE_NODE_CENTER&&
         slicej->slice_filetype!=SLICE_CELL_CENTER)continue;
      if(slicej->volslice == 1){
        int plotx, ploty, plotz;

        plotx = meshi->iplotx_all[iplotx_all];
        ploty = meshi->iploty_all[iploty_all];
        plotz = meshi->iplotz_all[iplotz_all];
        slicej->plotx = plotx;
        slicej->ploty = ploty;
        slicej->plotz = plotz;
        if(plotx >= 0 && visx_all == 1)slicex0[nx++] = slicej;
        if(ploty >= 0 && visy_all == 1)slicey0[ny++] = slicej;
        if(plotz >= 0 && visz_all == 1)slicez0[nz++] = slicej;
      }
      else{
        if(slicej->idir == 1)slicex0[nx++] = slicej;
        if(slicej->idir == 2)slicey0[ny++] = slicej;
        if(slicej->idir == 3)slicez0[nz++] = slicej;
      }
    }
    nsplitsliceinfo += nx*(ny+1)*(nz+1);
    nsplitsliceinfo += ny*(nx+1)*(nz+1);
    nsplitsliceinfo += nz*(nx+1)*(ny+1);
    meshi->nslicex = nx;
    meshi->nslicey = ny;
    meshi->nslicez = nz;
    slicex0 += nx;
    slicey0 += ny;
    slicez0 += nz;
  }
  if(nsplitsliceinfo==0)return;
  if(nsplitsliceinfo>nsplitsliceinfoMAX){
    FREEMEMORY(splitsliceinfo);
    NewMemory((void **)&splitsliceinfo, nsplitsliceinfo*sizeof(splitslicedata));
    FREEMEMORY(splitsliceinfoptr);
    NewMemory((void **)&splitsliceinfoptr, nsplitsliceinfo*sizeof(splitslicedata *));
    nsplitsliceinfoMAX = nsplitsliceinfo;
  }
  nsplitsliceinfo = 0;
  for(i = 0;i < nmeshes;i++){
    meshdata *meshi;
    int ii, jj, kk;
    int is1, is2, js1, js2, ks1, ks2;
    slicedata **slicexx, **sliceyy, **slicezz;

    meshi = meshinfo + i;

    slicexx = meshi->slicex;
    sliceyy = meshi->slicey;
    slicezz = meshi->slicez;

    // x slices
    for(ii=0;ii<meshi->nslicex;ii++){
      is1 = slicexx[ii]->plotx;
      is2 = is1;
      for(jj=0;jj<=meshi->nslicey;jj++){
        js1 = slicexx[ii]->jjs1;
        js2 = slicexx[ii]->jjs2;
        if(jj!=0)js1 = sliceyy[jj-1]->ploty;
        if(jj!=meshi->nslicey)js2 = sliceyy[jj]->ploty;
        for(kk=0;kk<=meshi->nslicez;kk++){
          splitslicedata *spliti;

          ks1 = slicexx[ii]->ks1;
          ks2 = slicexx[ii]->ks2;
          if(kk!=0)ks1 = slicezz[kk-1]->plotz;
          if(kk!=meshi->nslicez)ks2 = slicezz[kk]->plotz;
          if(is1==is2&&js1==js2)continue;
          if(is1==is2&&ks1==ks2)continue;
          if(js1==js2&&ks1==ks2)continue;
          spliti = splitsliceinfo + nsplitsliceinfo++;
          spliti->mesh = meshi;
          spliti->slice = slicexx[ii];
          spliti->is1 = is1;
          spliti->is2 = is2;
          spliti->js1 = js1;
          spliti->js2 = js2;
          spliti->ks1 = ks1;
          spliti->ks2 = ks2;
          spliti->splitdir = 1;
        }
      }
    }
    // y slices
    for(jj=0;jj<meshi->nslicey;jj++){
      js1 = sliceyy[jj]->ploty;
      js2 = js1;
      for(ii=0;ii<=meshi->nslicex;ii++){
        is1 = sliceyy[jj]->iis1;
        is2 = sliceyy[jj]->iis2;
        if(ii!=0)is1 = slicexx[ii-1]->plotx;
        if(ii!=meshi->nslicex)is2 = slicexx[ii]->plotx;
        for(kk=0;kk<=meshi->nslicez;kk++){
          splitslicedata *spliti;

          ks1 = sliceyy[jj]->kks1;
          ks2 = sliceyy[jj]->kks2;
          if(kk!=0)ks1 = slicezz[kk-1]->plotz;
          if(kk!=meshi->nslicez)ks2 = slicezz[kk]->plotz;
          if(is1==is2&&js1==js2)continue;
          if(is1==is2&&ks1==ks2)continue;
          if(js1==js2&&ks1==ks2)continue;
          spliti = splitsliceinfo + nsplitsliceinfo++;
          spliti->mesh = meshi;
          spliti->slice = sliceyy[jj];
          spliti->is1 = is1;
          spliti->is2 = is2;
          spliti->js1 = js1;
          spliti->js2 = js2;
          spliti->ks1 = ks1;
          spliti->ks2 = ks2;
          spliti->splitdir = 2;
        }
      }
    }
    // z slices
    for(kk=0;kk<meshi->nslicez;kk++){
      ks1 = slicezz[kk]->plotz;
      ks2 = ks1;
      for(jj=0;jj<=meshi->nslicey;jj++){
        js1 = slicezz[kk]->jjs1;
        js2 = slicezz[kk]->jjs2;
        if(jj!=0)js1 = sliceyy[jj-1]->ploty;
        if(jj!=meshi->nslicey)js2 = sliceyy[jj]->ploty;
        for(ii=0;ii<=meshi->nslicex;ii++){
          splitslicedata *spliti;

          is1 = slicezz[kk]->iis1;
          is2 = slicezz[kk]->iis2;
          if(ii!=0)is1 = slicexx[ii-1]->plotx;
          if(ii!=meshi->nslicex)is2 = slicexx[ii]->plotx;
          if(is1==is2&&js1==js2)continue;
          if(is1==is2&&ks1==ks2)continue;
          if(js1==js2&&ks1==ks2)continue;
          spliti = splitsliceinfo + nsplitsliceinfo++;
          spliti->mesh = meshi;
          spliti->slice = slicezz[kk];
          spliti->is1 = is1;
          spliti->is2 = is2;
          spliti->js1 = js1;
          spliti->js2 = js2;
          spliti->ks1 = ks1;
          spliti->ks2 = ks2;
          spliti->splitdir = 3;
        }
      }
    }
  }
  for(i=0;i<nsplitsliceinfo;i++){
    splitsliceinfoptr[i] = splitsliceinfo + i;
  }
  if(nsplitsliceinfo > 1){
    qsort(( splitslicedata ** )splitsliceinfoptr, ( size_t )nsplitsliceinfo, sizeof(splitslicedata *), CompareSortSlices);
  }
}

/* ------------------ DrawSortSlices ------------------------ */

void DrawSortSlices(void){
  int i;

  for(i = 0;i < nsplitsliceinfo;i++){
    splitslicedata *si;
    slicedata *sd;

    si = splitsliceinfoptr[i];
    sd = si->slice;
    if(SetupSlice(sd) == 0)continue;
    switch(sd->slice_filetype){
      case SLICE_NODE_CENTER:
        DrawVolSliceTexture(sd, si->is1, si->is2, si->js1, si->js2, si->ks1, si->ks2, si->splitdir);
        break;
      case SLICE_CELL_CENTER:
        DrawVolSliceCellFaceCenter(sd, SLICE_CELL_CENTER, 
                                   si->is1, si->is2, si->js1, si->js2, si->ks1, si->ks2, si->splitdir);
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
  }
}

/* ------------------ DrawSortSlicesDebug ------------------------ */

void DrawSortSlicesDebug(void){
  int i;

  if(nsplitsliceinfo==0)return;
  
  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);

  glLineWidth(4.0);
  glColor3f(0.0, 0.0, 0.0);
  glBegin(GL_LINES);
  for(i=0;i<nsplitsliceinfo;i++){
    splitslicedata *spliti;
    slicedata *sd;
    meshdata *meshi;
    float *xplt, *yplt, *zplt;
    int is1, is2, js1, js2, ks1, ks2;

    spliti = splitsliceinfoptr[i];
    xplt = spliti->mesh->xplt_orig;
    yplt = spliti->mesh->yplt_orig;
    zplt = spliti->mesh->zplt_orig;
    is1 = spliti->is1;
    is2 = spliti->is2;
    js1 = spliti->js1;
    js2 = spliti->js2;
    ks1 = spliti->ks1;
    ks2 = spliti->ks2;
    sd = spliti->slice;
    int plotx, ploty, plotz;
    meshi = spliti->mesh;

    plotx = meshi->iplotx_all[iplotx_all];
    ploty = meshi->iploty_all[iploty_all];
    plotz = meshi->iplotz_all[iplotz_all];

    if((sd->volslice==0&&sd->idir==1)||(sd->volslice == 1 && plotx >= 0 && visx_all == 1)){
      glVertex3f(xplt[is1],yplt[js1],zplt[ks1]);
      glVertex3f(xplt[is1],yplt[js2],zplt[ks1]);

      glVertex3f(xplt[is1],yplt[js2],zplt[ks1]);
      glVertex3f(xplt[is1],yplt[js2],zplt[ks2]);

      glVertex3f(xplt[is1],yplt[js2],zplt[ks2]);
      glVertex3f(xplt[is1],yplt[js1],zplt[ks2]);

      glVertex3f(xplt[is1],yplt[js1],zplt[ks2]);
      glVertex3f(xplt[is1],yplt[js1],zplt[ks1]);
    }
    if((sd->volslice==0&&sd->idir==2)||(sd->volslice == 1 && ploty >= 0 && visy_all == 1)){
      glVertex3f(xplt[is1],yplt[js1],zplt[ks1]);
      glVertex3f(xplt[is2],yplt[js1],zplt[ks1]);

      glVertex3f(xplt[is2],yplt[js1],zplt[ks1]);
      glVertex3f(xplt[is2],yplt[js1],zplt[ks2]);

      glVertex3f(xplt[is2],yplt[js1],zplt[ks2]);
      glVertex3f(xplt[is1],yplt[js1],zplt[ks2]);

      glVertex3f(xplt[is1],yplt[js1],zplt[ks2]);
      glVertex3f(xplt[is1],yplt[js1],zplt[ks1]);
    }
    if((sd->volslice == 0 && sd->idir == 3) || (sd->volslice == 1 && plotz >= 0 && visz_all == 1)){
      glVertex3f(xplt[is1],yplt[js1],zplt[ks1]);
      glVertex3f(xplt[is2],yplt[js1],zplt[ks1]);

      glVertex3f(xplt[is2],yplt[js1],zplt[ks1]);
      glVertex3f(xplt[is2],yplt[js2],zplt[ks1]);

      glVertex3f(xplt[is2],yplt[js2],zplt[ks1]);
      glVertex3f(xplt[is1],yplt[js2],zplt[ks1]);

      glVertex3f(xplt[is1],yplt[js2],zplt[ks1]);
      glVertex3f(xplt[is1],yplt[js1],zplt[ks1]);
    }
  }
  glEnd();
  for(i = 0;i < nsplitsliceinfo;i++){
    splitslicedata *spliti;
    float *xplt, *yplt, *zplt;
    int is1, is2, js1, js2, ks1, ks2;
    char label[32];
    float xmid, ymid, zmid;

    spliti = splitsliceinfoptr[i];
    xplt = spliti->mesh->xplt_orig;
    yplt = spliti->mesh->yplt_orig;
    zplt = spliti->mesh->zplt_orig;
    is1 = spliti->is1;
    is2 = spliti->is2;
    js1 = spliti->js1;
    js2 = spliti->js2;
    ks1 = spliti->ks1;
    ks2 = spliti->ks2;
    xmid = (xplt[is1] + xplt[is2]) / 2.0;
    ymid = (yplt[js1] + yplt[js2]) / 2.0;
    zmid = (zplt[ks1] + zplt[ks2]) / 2.0;
    sprintf(label, "%i", i);
    Output3Text(foregroundcolor, xmid, ymid, zmid, label);
  }
  glPopMatrix();

}
