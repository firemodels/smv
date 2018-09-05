#include "options.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H

#include "smokeviewvars.h"

/* ------------------ Slerp ------------------------ */

void Slerp(float *p0, float *p1, float t, float *pout){
  float cosangle,sinangle,denom,angle,factor1,factor2;

  denom = NORM3(p0)*NORM3(p1);
  if(denom==0.0){
    pout[0]=p0[0];
    pout[1]=p0[1];
    pout[2]=p0[2];
    return;
  }
  cosangle = CLAMP(DOT3(p0,p1)/denom,-1.0,1.0);
  angle = acos(cosangle);
  sinangle = sin(angle);
  if(sinangle == 0.0){
    factor1 = (1.0 - t);
    factor2 = t;
  }
  else{
    factor1 = sin((1.0 - t)*angle) / sinangle;
    factor2 = sin(t*angle) / sinangle;
  }
  pout[0]=factor1*p0[0]+factor2*p1[0];
  pout[1]=factor1*p0[1]+factor2*p1[1];
  pout[2]=factor1*p0[2]+factor2*p1[2];
}

/* ----------------------- DrawTetraOutline ----------------------------- */

void DrawTetraOutline(float *v1, float *v2, float *v3, float *v4, unsigned char *rgbcolor){
  glBegin(GL_LINES);
  if(rgbcolor!=NULL)glColor3ubv(rgbcolor);
  glVertex3fv(v1);
  glVertex3fv(v2);
  glVertex3fv(v2);
  glVertex3fv(v3);
  glVertex3fv(v3);
  glVertex3fv(v1);
  glVertex3fv(v1);
  glVertex3fv(v4);
  glVertex3fv(v2);
  glVertex3fv(v4);
  glVertex3fv(v3);
  glVertex3fv(v4);
  glEnd();
}

/* ----------------------- DrawFilledTetra ----------------------------- */

void DrawFilledTetra(float *v1, float *v2, float *v3, float *v4, unsigned char *rgbcolor){
  float diff1[3],diff2[3],cross[3];

  glBegin(GL_TRIANGLES);
  if(rgbcolor!=NULL)glColor3ubv(rgbcolor);

  VEC3DIFF(diff1,v1,v2);
  VEC3DIFF(diff2,v4,v2);
  CROSS(cross,diff1,diff2);
  glNormal3f(cross[0],cross[1],cross[2]);
  glVertex3fv(v1);
  glVertex3fv(v2);
  glVertex3fv(v4);

  VEC3DIFF(diff1,v1,v4);
  VEC3DIFF(diff2,v2,v4);
  CROSS(cross,diff1,diff2);
  glNormal3f(cross[0],cross[1],cross[2]);
  glVertex3fv(v1);
  glVertex3fv(v4);
  glVertex3fv(v2);

  VEC3DIFF(diff1,v2,v3);
  VEC3DIFF(diff2,v4,v3);
  CROSS(cross,diff1,diff2);
  glNormal3f(cross[0],cross[1],cross[2]);
  glVertex3fv(v2);
  glVertex3fv(v3);
  glVertex3fv(v4);

  VEC3DIFF(diff1,v2,v4);
  VEC3DIFF(diff2,v3,v4);
  CROSS(cross,diff1,diff2);
  glNormal3f(cross[0],cross[1],cross[2]);
  glVertex3fv(v2);
  glVertex3fv(v4);
  glVertex3fv(v3);

  VEC3DIFF(diff1,v4,v1);
  VEC3DIFF(diff2,v3,v4);
  CROSS(cross,diff1,diff2);
  glNormal3f(cross[0],cross[1],cross[2]);
  glVertex3fv(v1);
  glVertex3fv(v4);
  glVertex3fv(v3);

  VEC3DIFF(diff1,v1,v3);
  VEC3DIFF(diff2,v4,v3);
  CROSS(cross,diff1,diff2);
  glNormal3f(cross[0],cross[1],cross[2]);
  glVertex3fv(v1);
  glVertex3fv(v3);
  glVertex3fv(v4);

  VEC3DIFF(diff1,v1,v3);
  VEC3DIFF(diff2,v2,v3);
  CROSS(cross,diff1,diff2);
  glNormal3f(cross[0],cross[1],cross[2]);
  glVertex3fv(v1);
  glVertex3fv(v3);
  glVertex3fv(v2);

  VEC3DIFF(diff1,v1,v2);
  VEC3DIFF(diff2,v3,v2);
  CROSS(cross,diff1,diff2);
  glNormal3f(cross[0],cross[1],cross[2]);
  glVertex3fv(v1);
  glVertex3fv(v2);
  glVertex3fv(v3);
  glEnd();
}

/* ----------------------- DrawFilled2Tetra ----------------------------- */

void DrawFilled2Tetra(float *v1, float *v2, float *v3, float *v4,
                     unsigned char *rgb0color,
                     unsigned char *rgb1color,
                     unsigned char *rgb2color,
                     unsigned char *rgb3color,
                     int *vis_plane
                     ){
  float diff1[3],diff2[3],cross[3];

  glBegin(GL_TRIANGLES);
  if(vis_plane[0]==1){
     if(rgb0color!=NULL)glColor3ubv(rgb0color);
    VEC3DIFF(diff1,v1,v2);
    VEC3DIFF(diff2,v4,v2);
    CROSS(cross,diff2,diff1);
    glNormal3f(cross[0],cross[1],cross[2]);
    glVertex3fv(v1);
    glVertex3fv(v2);
    glVertex3fv(v4);

    VEC3DIFF(diff1,v1,v4);
    VEC3DIFF(diff2,v2,v4);
    CROSS(cross,diff2,diff1);
    glNormal3f(cross[0],cross[1],cross[2]);
    glVertex3fv(v1);
    glVertex3fv(v4);
    glVertex3fv(v2);
  }

  if(vis_plane[1]==1){
    if(rgb1color!=NULL)glColor3ubv(rgb1color);
    VEC3DIFF(diff1,v2,v3);
    VEC3DIFF(diff2,v4,v3);
    CROSS(cross,diff2,diff1);
    glNormal3f(cross[0],cross[1],cross[2]);
    glVertex3fv(v2);
    glVertex3fv(v3);
    glVertex3fv(v4);

    VEC3DIFF(diff1,v2,v4);
    VEC3DIFF(diff2,v3,v4);
    CROSS(cross,diff2,diff1);
    glNormal3f(cross[0],cross[1],cross[2]);
    glVertex3fv(v2);
    glVertex3fv(v4);
    glVertex3fv(v3);
  }

  if(vis_plane[2]==1){
    if(rgb2color!=NULL)glColor3ubv(rgb2color);
    VEC3DIFF(diff1,v4,v1);
    VEC3DIFF(diff2,v3,v4);
    CROSS(cross,diff2,diff1);
    glNormal3f(cross[0],cross[1],cross[2]);
    glVertex3fv(v1);
    glVertex3fv(v4);
    glVertex3fv(v3);

    VEC3DIFF(diff1,v1,v3);
    VEC3DIFF(diff2,v4,v3);
    CROSS(cross,diff2,diff1);
    glNormal3f(cross[0],cross[1],cross[2]);
    glVertex3fv(v1);
    glVertex3fv(v3);
    glVertex3fv(v4);
  }

  if(vis_plane[3]==1){
    if(rgb3color!=NULL)glColor3ubv(rgb3color);
    VEC3DIFF(diff1,v1,v3);
    VEC3DIFF(diff2,v2,v3);
    CROSS(cross,diff2,diff1);
    glNormal3f(cross[0],cross[1],cross[2]);
    glVertex3fv(v1);
    glVertex3fv(v3);
    glVertex3fv(v2);

    VEC3DIFF(diff1,v1,v2);
    VEC3DIFF(diff2,v3,v2);
    CROSS(cross,diff2,diff1);
    glNormal3f(cross[0],cross[1],cross[2]);
    glVertex3fv(v1);
    glVertex3fv(v2);
    glVertex3fv(v3);
  }
  glEnd();
}

/* ------------------ CompareFloats ------------------------ */

int CompareFloats( const void *arg1, const void *arg2 ){
  float x, y;
  x=*(float *)arg1;
  y=*(float *)arg2;
  if( x< y)return -1;
  if( x> y)return 1;
  return 0;
}

/* ------------------ RemoveDupFloats ------------------------ */

void RemoveDupFloats(float **valsptr, int *nvals,int *ivals, float dval_min){
  int nv;
  int i,ii;
  float *vals,valmid;

  *ivals=0;
  if(*nvals==0)return;
  nv = *nvals;
  vals = *valsptr;
  qsort( (float *)vals, (size_t)nv, sizeof( float ), CompareFloats );
  ii=1;
  for(i=1;i<nv;i++){
    if(ABS(vals[i]-vals[i-1])<=dval_min)continue;
    vals[ii]=vals[i];
    ii++;
  }
  valmid=(vals[0]+vals[*nvals-1])/2.0;
  if(*nvals!=ii){
    *nvals=ii;
    ResizeMemory((void **)&vals,*nvals*sizeof(float));
    *valsptr=vals;
  }
  for(i=1;i<*nvals;i++){
    if(vals[i-1]<=valmid&&valmid<=vals[i]){
      *ivals=i;
      break;
    }
  }
}

/* ------------------ ClosestNodeIndex ------------------------ */

int ClosestNodeIndex(float val,float *vals,int nvals, float eps){
  int j;

  if(val<vals[0])return -1;
  if(val>vals[nvals-1])return -1;
  for(j=0;j<nvals-1;j++){
    if(vals[j] <= val&&val <= vals[j + 1]){
      if(ABS(vals[j] - val) < ABS(vals[j + 1] - val))return j;
      return j+1;
    }
  }
  return nvals-1;
}

/* ------------------ UpdatePlotxyzAll ------------------------ */

void UpdatePlotxyzAll(void){
  int i;
  float *xp, *yp, *zp;
  float dxyz_min=100000.0;

  for(i = 0;i < nmeshes;i++){
    meshdata *meshi;
    float *xplt, *yplt, *zplt, *dxyz;

    meshi = meshinfo + i;
    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;
    dxyz = meshi->dxyz;
    dxyz[0] = ABS(xplt[1] - xplt[0]);
    dxyz[1] = ABS(yplt[1] - yplt[0]);
    dxyz[2] = ABS(zplt[1] - zplt[0]);
  }

  FREEMEMORY(plotx_all);
  FREEMEMORY(ploty_all);
  FREEMEMORY(plotz_all);
  nplotx_all=0;
  nploty_all=0;
  nplotz_all=0;
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    nplotx_all+=(meshi->ibar+1);
    nploty_all+=(meshi->jbar+1);
    nplotz_all+=(meshi->kbar+1);
  }
  NewMemory((void **)&plotx_list, nplotx_all*sizeof(int));
  for(i=0;i<nplotx_all;i++){
    plotx_list[i] = 0;
  }
  nplotx_list =  0;

  NewMemory((void **)&ploty_list, nploty_all*sizeof(int));
  for(i=0;i<nploty_all;i++){
    ploty_list[i] = 1;
  }
  nploty_list = 0;

  NewMemory((void **)&plotz_list, nplotz_all*sizeof(int));
  for(i=0;i<nplotz_all;i++){
    plotz_list[i] = 0;
  }
  nplotz_list = 0;

  NewMemory((void **)&plotx_all,nplotx_all*sizeof(float));
  NewMemory((void **)&ploty_all,nploty_all*sizeof(float));
  NewMemory((void **)&plotz_all,nplotz_all*sizeof(float));
  xp = plotx_all;
  yp = ploty_all;
  zp = plotz_all;
  for(i=0;i<nmeshes;i++){
    int j;
    meshdata *meshi;

    meshi = meshinfo + i;
    for(j=0;j<meshi->ibar+1;j++){
      *xp++ = meshi->xplt[j];
    }
    for(j=0;j<meshi->jbar+1;j++){
      *yp++ = meshi->yplt[j];
    }
    for(j=0;j<meshi->kbar+1;j++){
      *zp++ = meshi->zplt[j];
    }
    for(j=1;j<meshi->ibar+1;j++){
      float dxyz;

      dxyz = meshi->xplt[j]-meshi->xplt[j-1];
      dxyz_min = MIN(dxyz_min,dxyz);
    }
    for(j=1;j<meshi->jbar+1;j++){
      float dxyz;

      dxyz = meshi->yplt[j]-meshi->yplt[j-1];
      dxyz_min = MIN(dxyz_min,dxyz);
    }
    for(j=1;j<meshi->kbar+1;j++){
      float dxyz;

      dxyz = meshi->zplt[j]-meshi->zplt[j-1];
      dxyz_min = MIN(dxyz_min,dxyz);
    }
  }
  dxyz_min/=10.0;
  RemoveDupFloats(&plotx_all,&nplotx_all,&iplotx_all,dxyz_min);
  RemoveDupFloats(&ploty_all,&nploty_all,&iploty_all,dxyz_min);
  RemoveDupFloats(&plotz_all,&nplotz_all,&iplotz_all,dxyz_min);
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;
    int j;

    meshi = meshinfo + i;
    NewMemory((void **)&meshi->iplotx_all,nplotx_all*sizeof(int));
    NewMemory((void **)&meshi->iploty_all,nploty_all*sizeof(int));
    NewMemory((void **)&meshi->iplotz_all,nplotz_all*sizeof(int));

    for(j=0;j<nplotx_all;j++){
      float val;
      int ival;

      meshi->iplotx_all[j]=-1;
      val = plotx_all[j];
        ival = ClosestNodeIndex(val,meshi->xplt,meshi->ibar+1,dxyz_min);
      if(ival<0)continue;
      meshi->iplotx_all[j]=ival;
    }
    for(j=0;j<nploty_all;j++){
      float val;
      int ival;

      meshi->iploty_all[j]=-1;
      val = ploty_all[j];
      ival = ClosestNodeIndex(val,meshi->yplt,meshi->jbar+1,dxyz_min);
      if(ival<0)continue;
      meshi->iploty_all[j]=ival;
    }
    for(j=0;j<nplotz_all;j++){
      float val;
      int ival;

      meshi->iplotz_all[j]=-1;
      val = plotz_all[j];
      ival = ClosestNodeIndex(val,meshi->zplt,meshi->kbar+1,dxyz_min);
      if(ival<0)continue;
      meshi->iplotz_all[j]=ival;
    }
  }
  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;
    int ival;

    meshi = meshinfo+i;
    ival = ClosestNodeIndex(xbar/2.0, meshi->xplt, meshi->ibar+1, dxyz_min);
    if(ival<0)continue;
    iplotx_all = ival;
  }
  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;
    int ival;

    meshi = meshinfo+i;
    ival = ClosestNodeIndex(ybar/2.0, meshi->yplt, meshi->jbar+1, dxyz_min);
    if(ival<0)continue;
    iploty_all = ival;
  }
  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;
    int ival;

    meshi = meshinfo+i;
    ival = ClosestNodeIndex(zbar/2.0, meshi->zplt, meshi->kbar+1, dxyz_min);
    if(ival<0)continue;
    iplotz_all = ival;
  }

  nplotx_list = 0;
  for(i=0;i<nplotx_all;i++){
    plotx_list[i] = i;
    nplotx_list++;
  }

  nploty_list = 0;
  for(i = 0;i<nploty_all;i++){
    ploty_list[i] = i;
    nploty_list++;
  }

  nplotz_list = 0;
  for(i = 0;i<nplotz_all;i++){
    plotz_list[i] = i;
    nplotz_list++;
  }
}

#define MESHEPS 0.001

/* ------------------ GetMesh ------------------------ */

meshdata *GetMesh(float *xyz, meshdata *guess){
  int i;

  for(i=-1;i<nmeshes;i++){
    meshdata *meshi;
    int ibar, jbar, kbar;
    float *xplt, *yplt, *zplt;

    if(i == -1){
      if(guess == NULL)continue;
      meshi = guess;
    }
    else{
      meshi = meshinfo + i;
    }

    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;

    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;

    if(
      xplt[0]<=xyz[0]&&xyz[0]<xplt[ibar]&&
      yplt[0]<=xyz[1]&&xyz[1]<yplt[jbar]&&
      zplt[0]<=xyz[2]&&xyz[2]<zplt[kbar]){
        return meshi;
    }
  }
  return NULL;
}

/* ------------------ OnMeshBoundary ------------------------ */

int OnMeshBoundary(float *xyz){
  int i;

  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;
    int ibar, jbar, kbar;
    float *xplt, *yplt, *zplt;

    meshi = meshinfo+i;

    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;

    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;

    if(xyz[0]<xplt[0]-MESHEPS||xyz[0]>xplt[ibar]+MESHEPS)continue;
    if(xyz[1]<yplt[0]-MESHEPS||xyz[1]>yplt[jbar]+MESHEPS)continue;
    if(xyz[2]<zplt[0]-MESHEPS||xyz[2]>zplt[kbar]+MESHEPS)continue;

    // pt on xmin face

    if(ABS(xplt[0]-xyz[0])<=MESHEPS&&
      yplt[0]-MESHEPS<=xyz[1]&&xyz[1]<=yplt[jbar]+MESHEPS&&
      zplt[0]-MESHEPS<=xyz[2]&&xyz[2]<=zplt[kbar]+MESHEPS)return 1;

    // pt on xmax face

    if(ABS(xplt[ibar]-xyz[0])<=MESHEPS&&
      yplt[0]-MESHEPS<=xyz[1]&&xyz[1]<=yplt[jbar]+MESHEPS&&
      zplt[0]-MESHEPS<=xyz[2]&&xyz[2]<=zplt[kbar]+MESHEPS)return 1;

    // pt on ymin face

    if(ABS(yplt[0]-xyz[1])<=MESHEPS&&
      xplt[0]-MESHEPS<=xyz[0]&&xyz[0]<=xplt[ibar]+MESHEPS&&
      zplt[0]-MESHEPS<=xyz[2]&&xyz[2]<=zplt[kbar]+MESHEPS)return 1;

    // pt on ymax face

    if(ABS(yplt[jbar]-xyz[1])<=MESHEPS&&
      xplt[0]-MESHEPS<=xyz[0]&&xyz[0]<=xplt[ibar]+MESHEPS&&
      zplt[0]-MESHEPS<=xyz[2]&&xyz[2]<=zplt[kbar]+MESHEPS)return 1;

    // pt on zmin face

    if(ABS(zplt[0]-xyz[2])<=MESHEPS&&
      xplt[0]-MESHEPS<=xyz[0]&&xyz[0]<=xplt[ibar]+MESHEPS&&
      yplt[0]-MESHEPS<=xyz[1]&&xyz[1]<=yplt[jbar]+MESHEPS)return 1;

    // pt on zmax face

    if(ABS(zplt[kbar]-xyz[2])<=MESHEPS&&
      xplt[0]-MESHEPS<=xyz[0]&&xyz[0]<=xplt[ibar]+MESHEPS&&
      yplt[0]-MESHEPS<=xyz[1]&&xyz[1]<=yplt[jbar]+MESHEPS)return 1;
  }
  return 0;
}

/* ------------------ GetMeshNoFail ------------------------ */

meshdata *GetMeshNoFail(float *xyz){
  int i;

  for(i=0;i<nmeshes;i++){
    meshdata *meshi;
    int ibar, jbar, kbar;
    float *xplt, *yplt, *zplt;

    meshi = meshinfo+i;

    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;

    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;

    if(
      xplt[0]<=xyz[0]&&xyz[0]<xplt[ibar]&&
      yplt[0]<=xyz[1]&&xyz[1]<yplt[jbar]&&
      zplt[0]<=xyz[2]&&xyz[2]<zplt[kbar]){
      return meshi;
    }
  }
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;
    int ibar, jbar, kbar;
    float *xplt, *yplt, *zplt;

    meshi = meshinfo+i;

    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;

    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;

    if(
      xplt[0]<=xyz[0]+MESHEPS&&xyz[0]-MESHEPS<=xplt[ibar]&&
      yplt[0]<=xyz[1]+MESHEPS&&xyz[1]-MESHEPS<=yplt[jbar]&&
      zplt[0]<=xyz[2]+MESHEPS&&xyz[2]-MESHEPS<=zplt[kbar]){
      return meshi;
    }
  }
  return meshinfo;
}

/* ------------------ ExtractFrustum ------------------------ */

void ExtractFrustum(void){

/* code from:  http://www.crownandcutlass.com/features/technicaldetails/frustum.html */
   float   proj[16];
   float   modl[16];
   float   clip[16];
   float   t;

   /* Get the current PROJECTION matrix from OpenGL */
   glGetFloatv( GL_PROJECTION_MATRIX, proj );

   /* Get the current MODELVIEW matrix from OpenGL */
   glGetFloatv( GL_MODELVIEW_MATRIX, modl );

   /* Combine the two matrices (multiply projection by modelview) */
   clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
   clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
   clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
   clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

   clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
   clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
   clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
   clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

   clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
   clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
   clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
   clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

   clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
   clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
   clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
   clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

   /* Extract the numbers for the RIGHT plane */
   frustum[0][0] = clip[ 3] - clip[ 0];
   frustum[0][1] = clip[ 7] - clip[ 4];
   frustum[0][2] = clip[11] - clip[ 8];
   frustum[0][3] = clip[15] - clip[12];

   /* Normalize the result */
   t = sqrt( frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2] );
   if(t != 0.0){
     frustum[0][0] /= t;
     frustum[0][1] /= t;
     frustum[0][2] /= t;
     frustum[0][3] /= t;
   }

   /* Extract the numbers for the LEFT plane */
   frustum[1][0] = clip[ 3] + clip[ 0];
   frustum[1][1] = clip[ 7] + clip[ 4];
   frustum[1][2] = clip[11] + clip[ 8];
   frustum[1][3] = clip[15] + clip[12];

   /* Normalize the result */
   t = sqrt( frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2] );
   if(t != 0.0){
     frustum[1][0] /= t;
     frustum[1][1] /= t;
     frustum[1][2] /= t;
     frustum[1][3] /= t;
   }

   /* Extract the BOTTOM plane */
   frustum[2][0] = clip[ 3] + clip[ 1];
   frustum[2][1] = clip[ 7] + clip[ 5];
   frustum[2][2] = clip[11] + clip[ 9];
   frustum[2][3] = clip[15] + clip[13];

   /* Normalize the result */
   t = sqrt( frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2] );
   if(t != 0.0){
     frustum[2][0] /= t;
     frustum[2][1] /= t;
     frustum[2][2] /= t;
     frustum[2][3] /= t;
   }

   /* Extract the TOP plane */
   frustum[3][0] = clip[ 3] - clip[ 1];
   frustum[3][1] = clip[ 7] - clip[ 5];
   frustum[3][2] = clip[11] - clip[ 9];
   frustum[3][3] = clip[15] - clip[13];

   /* Normalize the result */
   t = sqrt( frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2] );
   if(t != 0.0){
     frustum[3][0] /= t;
     frustum[3][1] /= t;
     frustum[3][2] /= t;
     frustum[3][3] /= t;
   }

   /* Extract the FAR plane */
   frustum[4][0] = clip[ 3] - clip[ 2];
   frustum[4][1] = clip[ 7] - clip[ 6];
   frustum[4][2] = clip[11] - clip[10];
   frustum[4][3] = clip[15] - clip[14];

   /* Normalize the result */
   t = sqrt( frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2] );
   if(t != 0.0){
     frustum[4][0] /= t;
     frustum[4][1] /= t;
     frustum[4][2] /= t;
     frustum[4][3] /= t;
   }

   /* Extract the NEAR plane */
   frustum[5][0] = clip[ 3] + clip[ 2];
   frustum[5][1] = clip[ 7] + clip[ 6];
   frustum[5][2] = clip[11] + clip[10];
   frustum[5][3] = clip[15] + clip[14];

   /* Normalize the result */
   t = sqrt( frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2] );
   if(t != 0.0){
     frustum[5][0] /= t;
     frustum[5][1] /= t;
     frustum[5][2] /= t;
     frustum[5][3] /= t;
   }
}

/* ------------------ FDSPointInFrustum ------------------------ */

int FDSPointInFrustum(float *xyz){
  int i;
  float xyz_smv[3];

  xyz_smv[0] = NORMALIZE_X(xyz[0]);
  xyz_smv[1] = NORMALIZE_Y(xyz[1]);
  xyz_smv[2] = NORMALIZE_Z(xyz[2]);

  for(i = 0; i<6; i++){
    if(DOT3(frustum[i], xyz_smv)+frustum[i][3]<=0)return 0;
  }
  return 1;
}

/* ------------------ PointInFrustum ------------------------ */

int PointInFrustum(float *xyz){
  int i;

  for(i = 0; i<6; i++){
    if(DOT3(frustum[i], xyz)+frustum[i][3]<=0)return 0;
  }
  return 1;
}

/* ------------------ PointInTriangle ------------------------ */

int PointInTriangle(float *v1, float *v2, float *v3){
  if(PointInFrustum(v1)==1)return 1;
  if(PointInFrustum(v2)==1)return 1;
  if(PointInFrustum(v3)==1)return 1;
  return 0;
}

/* ------------------ BoxInFrustum ------------------------ */

int BoxInFrustum(float *xx, float *yy, float *zz){
  int i;
  float xyz[3];

  for(i=0;i<2;i++){
    int j;

    xyz[0] = xx[i];
    for(j=0;j<2;j++){
      int k;

      xyz[1] = yy[j];
      for(k=0;k<2;k++){
        xyz[2] = zz[k];
        if(PointInFrustum(xyz)==1)return 1;
      }
    }
  }
  return 0;
}
/* ------------------ RectangleInFrustum ------------------------ */

int RectangleInFrustum( float *x11, float *x12, float *x22, float *x21){
   int p;

   for( p = 0; p < 6; p++ ){
      if( frustum[p][0]*x11[0] + frustum[p][1]*x11[1] + frustum[p][2]*x11[2] + frustum[p][3] > 0 )continue;
      if( frustum[p][0]*x12[0] + frustum[p][1]*x12[1] + frustum[p][2]*x12[2] + frustum[p][3] > 0 )continue;
      if( frustum[p][0]*x22[0] + frustum[p][1]*x22[1] + frustum[p][2]*x22[2] + frustum[p][3] > 0 )continue;
      if( frustum[p][0]*x21[0] + frustum[p][1]*x21[1] + frustum[p][2]*x21[2] + frustum[p][3] > 0 )continue;
      return 0;
   }
   return 1;
}


/* ------------------ MatMultMat ------------------------ */

void MatMultMat(float *m1, float *m2, float *m3){
  int i, j, k;
  int ij;

  for(i=0;i<4;i++){
    for(j=0;j<4;j++){
      ij = i+4*j;
      m3[ij]=0.0;
      for(k=0;k<4;k++){
        m3[ij]+=m1[i+4*k]*m2[k+4*j];
      }
    }
  }
}

/* ------------------ GetInverse ------------------------ */

void GetInverse(float *m, float *mi){
  int i,j;
  float *v,*vi;

  /*

  assume m is a 4x4 matrix parttioned as

  q00 q01 q02 v0
  q10 q11 q12 v1
  q20 q21 q22 v2
    0   0   0  a

  where v=(vi) and Q=(qij) is orthogonal ( Q*transpose(Q) = I )

  then inverse(m) =     transpose(Q)   -transpose(Q)*v/a
                            0                 1/a

  note:  m_ij = m[i+4*j]
  */

  v=m+12;   /* fourth column of m */
  vi=mi+12; /* fourth column of inverse(m) */
  for(i=0;i<3;i++){  /* compute transpose */
    for(j=0;j<3;j++){
      mi[i+4*j]=m[j+4*i];
    }
    mi[3+4*j]=0.0;
  }
  vi[3]=1.0/v[3];
  vi[0]=-(mi[0]*v[0]+mi[4]*v[1]+ mi[8]*v[2])*vi[3];
  vi[1]=-(mi[1]*v[0]+mi[5]*v[1]+ mi[9]*v[2])*vi[3];
  vi[2]=-(mi[2]*v[0]+mi[6]*v[1]+mi[10]*v[2])*vi[3];
}

/* ------------------ GetScreenMapping ------------------------ */

void GetScreenMapping(float *xyz0, float *screen_perm){
  GLdouble xyz[3];
  int viewport[4];
  GLdouble screen0[3];
  GLdouble screen[3];
  GLdouble modelview[16];
  GLdouble projection[16];
  int set;
  GLdouble maxvals[3];
  int min_index;

#define SETSCREEN(i1,i2,i3,dscreen)\
  if(i1==0)set=0;\
  if(set==0&&ABS((dscreen)[i1])>MAX( ABS((dscreen)[i2]) , ABS((dscreen)[i3]) ) ){\
    (dscreen)[i1]=SIGN((dscreen)[i1]);\
    (dscreen)[i2]=0.0;\
    (dscreen)[i3]=0.0;\
    set=1;\
  }

#define MAXABS3(x) (MAX(ABS((x)[0]),MAX(ABS((x)[1]),ABS((x)[2]))))

  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  VEC3EQ(xyz,xyz0);
  gluProject(xyz[0],xyz[1],xyz[2],modelview,projection,viewport,screen0,screen0+1,screen0+2);

  VEC3EQ(xyz,xyz0);
  xyz[0]+=0.1;
  gluProject(xyz[0],xyz[1],xyz[2],modelview,projection,viewport,screen,screen+1,screen+2);
  VEC3DIFF(screen_perm,screen,screen0);
  maxvals[0] = MAXABS3(screen_perm);

  VEC3EQ(xyz,xyz0);
  xyz[1]+=0.1;
  gluProject(xyz[0],xyz[1],xyz[2],modelview,projection,viewport,screen,screen+1,screen+2);
  VEC3DIFF(screen_perm+3,screen,screen0);
  maxvals[1] = MAXABS3(screen_perm+3);

  VEC3EQ(xyz,xyz0);
  xyz[2]+=0.1;
  gluProject(xyz[0],xyz[1],xyz[2],modelview,projection,viewport,screen,screen+1,screen+2);
  VEC3DIFF(screen_perm+6,screen,screen0);
  maxvals[2] = MAXABS3(screen_perm+6);

#ifdef _DEBUG
  PRINTF("%f %f %f\n",screen_perm[0],screen_perm[1],screen_perm[2]);
  PRINTF("%f %f %f\n",screen_perm[3],screen_perm[4],screen_perm[5]);
  PRINTF("%f %f %f\n",screen_perm[6],screen_perm[7],screen_perm[8]);
  PRINTF("\n");
#endif

  if(maxvals[0]<MIN(maxvals[1],maxvals[2])){
    min_index=0;
  }
  else if(maxvals[1]<MIN(maxvals[0],maxvals[2])){
    min_index=1;
  }
  else{
    min_index=2;
  }

  if(min_index==0){
    screen_perm[0]=0.0;
    screen_perm[1]=0.0;
    screen_perm[2]=0.0;
  }
  else{
    SETSCREEN(0,1,2,screen_perm);
    SETSCREEN(1,0,2,screen_perm);
    SETSCREEN(2,0,1,screen_perm);
  }

  if(min_index==1){
    screen_perm[3]=0.0;
    screen_perm[4]=0.0;
    screen_perm[5]=0.0;
  }
  else{
    SETSCREEN(0,1,2,screen_perm+3);
    SETSCREEN(1,0,2,screen_perm+3);
    SETSCREEN(2,0,1,screen_perm+3);
  }

  if(min_index==2){
    screen_perm[6]=0.0;
    screen_perm[7]=0.0;
    screen_perm[8]=0.0;
  }
  else{
    SETSCREEN(0,1,2,screen_perm+6);
    SETSCREEN(1,0,2,screen_perm+6);
    SETSCREEN(2,0,1,screen_perm+6);
  }
#ifdef _DEBUG
  PRINTF("%f %f %f\n",screen_perm[0],screen_perm[1],screen_perm[2]);
  PRINTF("%f %f %f\n",screen_perm[3],screen_perm[4],screen_perm[5]);
  PRINTF("%f %f %f\n",screen_perm[6],screen_perm[7],screen_perm[8]);
  PRINTF("\n");
#endif
}

/* ------------------ GetInterval ------------------------ */

int GetInterval(float val, float *array, int n){
  int low, mid, high;

  if(val<array[0])return -1;
  if(val>array[n-1])return -1;

  low=0;
  high=n-1;
  while(high-low>1){
    mid=(low+high)/2;
    if(val>array[mid]){
      low=mid;
    }
    else{
      high=mid;
    }
  }
  ASSERT(low<n)
  return low;
}

/* ------------------ GetNewPos ------------------------ */

void GetNewPos(float *oldpos, float dx, float dy, float dz,float local_speed_factor){
  oldpos[0] += dx;
  oldpos[1] += dy;
  oldpos[2] += dz;
  from_glui_trainer=0;
}

/* ------------------ GetBlockageDistance ------------------------ */

float GetBlockageDistance(float x, float y, float z){
  int i;
  meshdata *meshi;
  float *xplt, *yplt, *zplt;
  float xmin, xmax, ymin, ymax, zmin, zmax;
  int ibar, jbar, kbar, nx, nxy;
  int ii, jj, kk;
  int ijknode,ijkcell;
  float view_height;
  char *iblank_cell;

  for(i=0;i<nmeshes;i++){
    meshi = meshinfo+i;

    iblank_cell = meshi->c_iblank_cell;

    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;
    nx = ibar+1;
    nxy = (ibar+1)*(jbar+1);

    xplt = meshi->xplt_orig;
    yplt = meshi->yplt_orig;
    zplt = meshi->zplt_orig;

    xmin = xplt[0];
    xmax = xplt[ibar];
    if(x<xmin||x>xmax)continue;

    ymin = yplt[0];
    ymax = yplt[jbar];
    if(y<ymin||y>ymax)continue;

    zmin = zplt[0];
    zmax = zplt[kbar];
    if(z<zmin||z>zmax)continue;

    ii = GetInterval(x,xplt,ibar+1);
    jj = GetInterval(y,yplt,jbar+1);
    kk = GetInterval(z,zplt,kbar+1);
    if(ii!=-1&&jj!=-1&&kk!=-1){
      ijkcell=IJKCELL(ii,jj,kk);
      if(iblank_cell[ijkcell]==SOLID)return 0.0;
      ijknode=IJKNODE(ii,jj,kk);
      view_height = meshi->block_zdist[ijknode];
      if(view_height==0.0)return 0.0;
      view_height += (z-zplt[kk]);
      return view_height;
    }
  }
  return -1.0;
}

/* ------------------ MakeIBlankCarve ------------------------ */

int MakeIBlankCarve(void){
  int i, j;
  int ibar, jbar, kbar;
  int ijksize;
  int nx, ny, nz, nxy;
  char *ib_embed;

  n_embedded_meshes=0;
  for(i=0;i<nmeshes;i++){
    meshdata *meshi;

    meshi = meshinfo+i;
    meshi->c_iblank_embed=NULL;
  }
  if(nmeshes==1)return 0;


  for(i=0;i<nmeshes;i++){
    meshdata *meshi;
    int n_embedded;

    meshi = meshinfo+i;
    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;
    nx = ibar + 1;
    ny = jbar + 1;
    nz = kbar + 1;
    nxy = nx*ny;
    ijksize=(ibar+1)*(jbar+1)*(kbar+1);

    meshi->c_iblank_embed=NULL;

    // check to see if there are any embedded meshes

    n_embedded=0;
    for(j=0;j<nmeshes;j++){
      meshdata *meshj;

      if(i==j)continue;
      meshj = meshinfo + j;
      if(
        meshi->boxmin[0]<=meshj->boxmin[0]&&meshj->boxmax[0]<=meshi->boxmax[0]&&
        meshi->boxmin[1]<=meshj->boxmin[1]&&meshj->boxmax[1]<=meshi->boxmax[1]&&
        meshi->boxmin[2]<=meshj->boxmin[2]&&meshj->boxmax[2]<=meshi->boxmax[2]
      ){
        n_embedded++;
        n_embedded_meshes++;
      }
    }
    if(n_embedded==0)continue;

    ib_embed=NULL;
    if(use_iblank==1){
      if(NewMemory((void **)&ib_embed,ijksize*sizeof(char))==0)return 1;
    }
    meshi->c_iblank_embed=ib_embed;
    if(ib_embed==NULL)continue;
    for(j=0;j<ijksize;j++){
      ib_embed[j]=EMBED_NO;
    }
    for(j=0;j<nmeshes;j++){
      meshdata *meshj;
      int i1, i2, jj1, j2, k1, k2;
      int ii, jj, kk;
      float *xplt, *yplt, *zplt;

      if(i==j)continue;
      meshj = meshinfo + j;
      // meshj is embedded inside meshi
      if(
        meshi->boxmin[0]>meshj->boxmin[0]||meshj->boxmax[0]>meshi->boxmax[0]||
        meshi->boxmin[1]>meshj->boxmin[1]||meshj->boxmax[1]>meshi->boxmax[1]||
        meshi->boxmin[2]>meshj->boxmin[2]||meshj->boxmax[2]>meshi->boxmax[2]
      )continue;

      xplt = meshi->xplt_orig;
      yplt = meshi->yplt_orig;
      zplt = meshi->zplt_orig;
      k2 = 0;
      for(ii=0;ii<nx;ii++){
        if(xplt[ii]<=meshj->boxmin[0]&&meshj->boxmin[0]<xplt[ii+1]){
          i1=ii;
          break;
        }
      }
      for(ii=0;ii<nx;ii++){
        if(xplt[ii]<meshj->boxmax[0]&&meshj->boxmax[0]<=xplt[ii+1]){
          i2=ii;
          break;
        }
      }
      for(jj=0;jj<ny;jj++){
        if(yplt[jj]<=meshj->boxmin[1]&&meshj->boxmin[1]<yplt[jj+1]){
          jj1=jj;
          break;
        }
      }
      for(jj=0;jj<ny;jj++){
        if(yplt[jj]<meshj->boxmax[1]&&meshj->boxmax[1]<=yplt[jj+1]){
          j2=jj;
          break;
        }
      }
      for(kk=0;kk<nz;kk++){
        if(zplt[kk]<=meshj->boxmin[2]&&meshj->boxmin[2]<zplt[kk+1]){
          k1=kk;
          break;
        }
      }
      for(kk=0;kk<nz;kk++){
        if(zplt[kk]<meshj->boxmax[2]&&meshj->boxmax[2]<=zplt[kk+1]){
          k2=kk;
          break;
        }
      }

      for(kk=k1;kk<=k2;kk++){
        for(jj=jj1;jj<=j2;jj++){
          for(ii=i1;ii<=i2;ii++){
            ib_embed[IJKNODE(ii,jj,kk)]=EMBED_YES;
          }
        }
      }
    }
  }
  return 0;
}

/* ------------------ MakeIBlank ------------------------ */

int MakeIBlank(void){
  int ig;

  if(use_iblank==0)return 0;
  for(ig=0;ig<nmeshes;ig++){
    meshdata *meshi;
    int nx, ny, nxy, ibarjbar;
    int ibar,jbar,kbar;
    float *fblank_cell=NULL;
    char *iblank_node=NULL,*iblank_cell=NULL,*c_iblank_x=NULL,*c_iblank_y=NULL,*c_iblank_z=NULL;
    int ii,ijksize;
    int i,j,k;

    meshi = meshinfo+ig;

    if(meshi->nbptrs==0)continue;
    ibar = meshi->ibar;
    jbar = meshi->jbar;
    kbar = meshi->kbar;
    ijksize=(ibar+1)*(jbar+1)*(kbar+1);

    if(NewMemory((void **)&iblank_node,ijksize*sizeof(char))==0)return 1;
    if(NewMemory((void **)&iblank_cell,ibar*jbar*kbar*sizeof(char))==0)return 1;
    if(NewMemory((void **)&fblank_cell,ibar*jbar*kbar*sizeof(float))==0)return 1;
    if(NewMemory((void **)&c_iblank_x,ijksize*sizeof(char))==0)return 1;
    if(NewMemory((void **)&c_iblank_y,ijksize*sizeof(char))==0)return 1;
    if(NewMemory((void **)&c_iblank_z,ijksize*sizeof(char))==0)return 1;

    meshi->c_iblank_node0=iblank_node;
    meshi->c_iblank_cell0=iblank_cell;
    meshi->f_iblank_cell0=fblank_cell;
    meshi->c_iblank_x0=c_iblank_x;
    meshi->c_iblank_y0=c_iblank_y;
    meshi->c_iblank_z0=c_iblank_z;

    for(i=0;i<ibar*jbar*kbar;i++){
      iblank_cell[i]=GAS;
    }
    for(i=0;i<ijksize;i++){
      iblank_node[i]=GAS;
      c_iblank_x[i]=GAS;
      c_iblank_y[i]=GAS;
      c_iblank_z[i]=GAS;
    }

    nx = ibar+1;
    ny = jbar+1;
    nxy = nx*ny;
    ibarjbar = ibar*jbar;

    for(ii=0;ii<meshi->nbptrs;ii++){
      blockagedata *bc;

      bc=meshi->blockageinfoptrs[ii];
      for(k = bc->ijk[KMIN]; k < bc->ijk[KMAX]; k++){
        for(j = bc->ijk[JMIN]; j < bc->ijk[JMAX]; j++){
          int ijk;

          ijk = IJKCELL(bc->ijk[IMIN], j, k);
          for(i = bc->ijk[IMIN]; i < bc->ijk[IMAX]; i++){
            iblank_cell[ijk++] = SOLID;
          }
        }
      }
    }
    if(fblank_cell!=NULL){
      for(ii=0;ii<ibar*jbar*kbar;ii++){
        fblank_cell[ii]=iblank_cell[ii];
      }
    }
    if(meshi->nbptrs>0){
      for(k = 0; k < kbar + 1; k++){
        for(j = 0; j < jbar + 1; j++){
          int ijk,ijknode;

          //#define IJKNODE(i,j,k) ((i)+(j)*nx+(k)*nxy)
          //#define IJKCELL(i,j,k) ((i)+ (j)*ibar+(k)*ibar*jbar)
          ijk = IJKCELL(-1, j - 1, k - 1);
          ijknode = IJKNODE(0, j, k);
          for(i = 0; i < ibar + 1; i++){
            int test;
            int ijk2;

            test = 0;
//            if(i != 0 && j != 0 && k != 0)         test += iblank_cell[IJKCELL(i - 1, j - 1, k - 1)];
            if(i != 0 && j != 0 && k != 0)         test += iblank_cell[ijk];

//            if(i != ibar&&j != 0 && k != 0)        test += iblank_cell[IJKCELL(i, j - 1, k - 1)];
            if(i != ibar&&j != 0 && k != 0)        test += iblank_cell[ijk+1];

//            if(i != 0 && j != jbar&&k != 0)        test += iblank_cell[IJKCELL(i - 1,     j, k - 1)];
            if(i != 0 && j != jbar&&k != 0)        test += iblank_cell[ijk+ibar];

//            if(i != ibar&&j != jbar&&k != 0)       test += iblank_cell[IJKCELL(    i,     j, k - 1)];
            if(i != ibar&&j != jbar&&k != 0)       test += iblank_cell[ijk+1+ibar];

            ijk2 = ijk + ibarjbar;
            //            if(i != 0 && j != 0 && k != kbar)      test += iblank_cell[IJKCELL(i - 1, j - 1,     k)];
            if(i != 0 && j != 0 && k != kbar)      test += iblank_cell[ijk2];

//            if(i != ibar&&j != 0 && k != kbar)     test += iblank_cell[IJKCELL(i, j - 1, k)];
            if(i != ibar&&j != 0 && k != kbar)     test += iblank_cell[ijk2+1];

//            if(i != 0 && j != jbar&&k != kbar)     test += iblank_cell[IJKCELL(i - 1,     j,     k)];
            if(i != 0 && j != jbar&&k != kbar)     test += iblank_cell[ijk2+ibar];

//            if(i != ibar&&j != jbar&&k != kbar)    test += iblank_cell[IJKCELL(i, j, k)];
            if(i != ibar&&j != jbar&&k != kbar)    test += iblank_cell[ijk2+1+ibar];

//          if(test==0)iblank_node[IJKNODE(i,j,k)]=0;
            if(test == 0)iblank_node[ijknode] = 0;
            ijk++;
            ijknode++;
          }
        }
      }
    }

    for(j=0;j<jbar;j++){
      for(k=0;k<kbar;k++){
        int ijknode, ijkcell;

        ijkcell = IJKCELL(0, j, k);
        ijknode = IJKNODE(0, j, k);
//        c_iblank_x[IJKNODE(0,j,k)]   =2*iblank_cell[IJKCELL(0,j,k)];
        c_iblank_x[ijknode] = 2 * iblank_cell[ijkcell];
        for(i = 1; i<ibar; i++){
          ijknode++;
          ijkcell++;
//          c_iblank_x[IJKNODE(i, j, k)] = iblank_cell[IJKCELL(i - 1, j, k)] + iblank_cell[IJKCELL(i, j, k)];
          c_iblank_x[ijknode] = iblank_cell[ijkcell-1] + iblank_cell[ijkcell];
        }
        ijknode++;
        ijkcell++;
//        c_iblank_x[IJKNODE(ibar, j, k)] = 2 * iblank_cell[IJKCELL(ibar - 1, j, k)];
        c_iblank_x[ijknode] = 2 * iblank_cell[ijkcell-1];
      }
    }
    for(i=0;i<ibar;i++){
      for(k=0;k<kbar;k++){
        int ijkcell, ijknode;

        ijkcell = IJKCELL(i, 0, k);
        ijknode = IJKNODE(i, 0, k);
//        c_iblank_y[IJKNODE(i,0,k)]=2*iblank_cell[IJKCELL(i,0,k)];
        c_iblank_y[ijknode] = 2 * iblank_cell[ijkcell];
        for(j = 1; j<jbar; j++){
          ijkcell += ibar;
          ijknode += nx;
//          c_iblank_y[IJKNODE(i,j,k)]=iblank_cell[IJKCELL(i,j-1,k)]+iblank_cell[IJKCELL(i,j,k)];
          c_iblank_y[ijknode] = iblank_cell[ijkcell-ibar] + iblank_cell[ijkcell];
        }
        ijkcell += ibar;
        ijknode += nx;
        //        c_iblank_y[IJKNODE(i,jbar,k)]=2*iblank_cell[IJKCELL(i,jbar-1,k)];
        c_iblank_y[ijknode] = 2 * iblank_cell[ijkcell-ibar];
      }
    }

    for(i=0;i<ibar;i++){
      for(j=0;j<jbar;j++){
        int ijkcell, ijknode;

        ijkcell = IJKCELL(i, j, 0);
        ijknode = IJKNODE(i, j, 0);
//        c_iblank_z[IJKNODE(i,j,0)]=2*iblank_cell[IJKCELL(i,j,0)];
        c_iblank_z[ijknode]=2*iblank_cell[ijkcell];
        for(k=1;k<kbar;k++){
          ijkcell+=ibarjbar;
          ijknode+=nxy;
//          c_iblank_z[IJKNODE(i,j,k)]=iblank_cell[IJKCELL(i,j,k-1)]+iblank_cell[IJKCELL(i,j,k)];
          c_iblank_z[ijknode]=iblank_cell[ijkcell-ibar*jbar]+iblank_cell[ijkcell];
        }
        ijkcell+=ibarjbar;
        ijknode+=nxy;
//        c_iblank_z[IJKNODE(i,j,kbar)]=2*iblank_cell[IJKCELL(i,j,kbar-1)];
        c_iblank_z[ijknode]=2*iblank_cell[ijkcell-ibar*jbar];
      }
    }
  }
  LOCK_IBLANK
  for(ig = 0; ig < nmeshes; ig++){
    meshdata *meshi;

    meshi = meshinfo + ig;
    meshi->c_iblank_node = meshi->c_iblank_node0;
    meshi->c_iblank_cell = meshi->c_iblank_cell0;
    meshi->f_iblank_cell = meshi->f_iblank_cell0;
    meshi->c_iblank_x = meshi->c_iblank_x0;
    meshi->c_iblank_y = meshi->c_iblank_y0;
    meshi->c_iblank_z = meshi->c_iblank_z0;
  }
  UNLOCK_IBLANK

  return 0;
}

/* ------------------ InitClip ------------------------ */

void InitClip(void){
  clipdata *ci;

  clip_mode_last=-1;

  ci = &clipinfo;
  ci->clip_xmin=0;
  ci->clip_ymin=0;
  ci->clip_zmin=0;
  ci->clip_xmax=0;
  ci->clip_ymax=0;
  ci->clip_zmax=0;
  ci->xmin=0.0;
  ci->ymin=0.0;
  ci->zmin=0.0;
  ci->xmax=0.0;
  ci->ymax=0.0;
  ci->zmax=0.0;

  ci = &colorbar_clipinfo;
  ci->clip_xmin=1;
  ci->clip_ymin=1;
  ci->clip_zmin=1;
  ci->clip_xmax=1;
  ci->clip_ymax=1;
  ci->clip_zmax=1;
  ci->xmin=DENORMALIZE_X(2.0);
  ci->ymin=DENORMALIZE_X(2.0);
  ci->zmin=DENORMALIZE_Y(2.0);
  ci->xmax=DENORMALIZE_Y(2.0);
  ci->ymax=DENORMALIZE_Z(2.0);
  ci->zmax=DENORMALIZE_Z(2.0);

  clip_i=0;
  clip_j=0;
  clip_k=0;
  clip_I=0;
  clip_J=0;
  clip_K=0;

  stepclip_xmin=0,stepclip_ymin=0,stepclip_zmin=0;
  stepclip_xmax=0,stepclip_ymax=0,stepclip_zmax=0;
}


/* ------------------ VolumeTetrahedron ------------------------ */

float VolumeTetrahedron(float *v1, float *v2, float *v3, float *v4){
  float v2d[3], v3d[3], v4d[3], vcross[3];

  VEC3DIFF(v2d,v2,v1);
  VEC3DIFF(v3d,v3,v1);
  VEC3DIFF(v4d,v4,v1);
  CROSS(vcross,v2d,v3d);
  return DOT3(v4d,vcross)/6.0;
}

/* ----------------------- InitTetraClipInfo ----------------------------- */

void InitTetraClipInfo(clipdata *ci,float *v1, float *v2, float *v3, float *v4){
  float v1d[3], v2d[3];
  GLdouble *clipvals;
  float vol;

  //     v4
  //     .  .
  //     .   v2
  //     .  /  \         v4           v4          v4             v3
  //     ./     \       /  \         /  \        /  \          /   \
  //    v1-------v3    v1---v3      v3---v2     v2---v1       v1---v2

  vol = VolumeTetrahedron(v1,v2,v3,v4);

  clipvals = ci->clipvals;
  ci->option=TETRA_CLIPPLANES;

  VEC3DIFF(v1d,v1,v3);
  VEC3DIFF(v2d,v4,v3);
  CROSS(clipvals,v1d,v2d);
  if(vol>0.0){
    VEC3MA(clipvals,-1.0);
  }
  NORMALIZE3(clipvals);
  clipvals[3]=-DOT3(clipvals,v3);
  clipvals+=4;

  VEC3DIFF(v1d,v3,v2);
  VEC3DIFF(v2d,v4,v2);
  CROSS(clipvals,v1d,v2d);
  if(vol>0.0){
    VEC3MA(clipvals,-1.0);
  }
  NORMALIZE3(clipvals);
  clipvals[3]=-DOT3(clipvals,v2);
  clipvals+=4;

  VEC3DIFF(v1d,v2,v1);
  VEC3DIFF(v2d,v4,v1);
  CROSS(clipvals,v1d,v2d);
  if(vol>0.0){
    VEC3MA(clipvals,-1.0);
  }
  NORMALIZE3(clipvals);
  clipvals[3]=-DOT3(clipvals,v1);
  clipvals+=4;

  VEC3DIFF(v1d,v3,v1);
  VEC3DIFF(v2d,v2,v1);
  CROSS(clipvals,v1d,v2d);
  if(vol>0.0){
    VEC3MA(clipvals,-1.0);
  }
  NORMALIZE3(clipvals);
  clipvals[3]=-DOT3(clipvals,v1);
}

/* ----------------------- InitBoxClipInfo ----------------------------- */

void InitBoxClipInfo(clipdata *ci,float xmin, float xmax, float ymin, float ymax, float zmin, float zmax){
  ci->option=BOX_CLIPPLANES;
  ci->clip_xmin=1;
  ci->clip_xmax=1;
  ci->clip_ymin=1;
  ci->clip_ymax=1;
  ci->clip_zmin=1;
  ci->clip_zmax=1;
  ci->xmin=xmin;
  ci->xmax=xmax;
  ci->ymin=ymin;
  ci->ymax=ymax;
  ci->zmin=zmin;
  ci->zmax=zmax;
}


/* ----------------------- MergeMax ----------------------------- */

float MergeMax(int opti, float vali, int optj, float valj){
  if(opti==0&&optj==0)return vali;
  if(opti==1&&optj==0)return vali;
  if(opti==0&&optj==1)return valj;
  return MAX(vali,valj);
}

/* ----------------------- MergeMin ----------------------------- */

float MergeMin(int opti, float vali, int optj, float valj){
  if(opti==0&&optj==0)return vali;
  if(opti==1&&optj==0)return vali;
  if(opti==0&&optj==1)return valj;
  return MIN(vali,valj);
}

/* ----------------------- MergeClipPlanes ----------------------------- */

void MergeClipPlanes(clipdata *ci, clipdata *cj){
  ci->xmin = MergeMax(ci->clip_xmin,ci->xmin,cj->clip_xmin,cj->xmin);
  ci->ymin = MergeMax(ci->clip_ymin,ci->ymin,cj->clip_ymin,cj->ymin);
  ci->zmin = MergeMax(ci->clip_zmin,ci->zmin,cj->clip_zmin,cj->zmin);
  ci->xmax = MergeMin(ci->clip_xmax,ci->xmax,cj->clip_xmax,cj->xmax);
  ci->ymax = MergeMin(ci->clip_ymax,ci->ymax,cj->clip_ymax,cj->ymax);
  ci->zmax = MergeMin(ci->clip_zmax,ci->zmax,cj->clip_zmax,cj->zmax);

  ci->clip_xmin |= cj->clip_xmin;
  ci->clip_xmax |= cj->clip_xmax;
  ci->clip_ymin |= cj->clip_ymin;
  ci->clip_ymax |= cj->clip_ymax;
  ci->clip_zmin |= cj->clip_zmin;
  ci->clip_zmax |= cj->clip_zmax;
}

/* ----------------------- SetClipPlanes ----------------------------- */

void SetClipPlanes(clipdata *ci, int option){

  if(ci==NULL||option==CLIP_OFF){
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
    clipon=0;
    return;
  }
  clipon=1;

  if(ci->option==TETRA_CLIPPLANES){
    glClipPlane(GL_CLIP_PLANE0,ci->clipvals);
    glEnable(GL_CLIP_PLANE0);

    glClipPlane(GL_CLIP_PLANE1,ci->clipvals+4);
    glEnable(GL_CLIP_PLANE1);

    glClipPlane(GL_CLIP_PLANE2,ci->clipvals+8);
    glEnable(GL_CLIP_PLANE2);

    glClipPlane(GL_CLIP_PLANE3,ci->clipvals+12);
    glEnable(GL_CLIP_PLANE3);

    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
    return;
  }

  if(ci->clip_xmin==1){
    GLdouble clipplane[4];

    clipplane[0]=1.0;
    clipplane[1]=0.0;
    clipplane[2]=0.0;
    if(option==CLIP_ON_DENORMAL)clipplane[3]=-ci->xmin;
    if(option==CLIP_ON)clipplane[3]=-NORMALIZE_X(ci->xmin);
    glClipPlane(GL_CLIP_PLANE0,clipplane);
    glEnable(GL_CLIP_PLANE0);
  }
  else{
    glDisable(GL_CLIP_PLANE0);
  }

  if(ci->clip_xmax==1){
    GLdouble clipplane[4];

    clipplane[0]=-1.0;
    clipplane[1]=0.0;
    clipplane[2]=0.0;
    if(option==CLIP_ON_DENORMAL)clipplane[3]=ci->xmax;
    if(option==CLIP_ON)clipplane[3]=NORMALIZE_X(ci->xmax);
    glClipPlane(GL_CLIP_PLANE3,clipplane);
    glEnable(GL_CLIP_PLANE3);
  }
  else{
    glDisable(GL_CLIP_PLANE3);
  }

  if(ci->clip_ymin==1){
    GLdouble clipplane[4];

    clipplane[0]=0.0;
    clipplane[1]=1.0;
    clipplane[2]=0.0;
    if(option==CLIP_ON_DENORMAL)clipplane[3]=-ci->ymin;
    if(option==CLIP_ON)clipplane[3]=-NORMALIZE_Y(ci->ymin);
    glClipPlane(GL_CLIP_PLANE1,clipplane);
    glEnable(GL_CLIP_PLANE1);
  }
  else{
    glDisable(GL_CLIP_PLANE1);
  }

  if(ci->clip_ymax==1){
    GLdouble clipplane[4];

    clipplane[0]=0.0;
    clipplane[1]=-1.0;
    clipplane[2]=0.0;
    if(option==CLIP_ON_DENORMAL)clipplane[3]=ci->ymax;
    if(option==CLIP_ON)clipplane[3]=NORMALIZE_Y(ci->ymax);
    glClipPlane(GL_CLIP_PLANE4,clipplane);
    glEnable(GL_CLIP_PLANE4);
  }
  else{
    glDisable(GL_CLIP_PLANE4);
  }

  if(ci->clip_zmin==1){
    GLdouble clipplane[4];

    clipplane[0]=0.0;
    clipplane[1]=0.0;
    clipplane[2]=1.0;
    if(option==CLIP_ON_DENORMAL)clipplane[3]=-ci->zmin;
    if(option==CLIP_ON)clipplane[3]=-NORMALIZE_Z(ci->zmin);
    glClipPlane(GL_CLIP_PLANE2,clipplane);
    glEnable(GL_CLIP_PLANE2);
  }
  else{
    glDisable(GL_CLIP_PLANE2);
  }

  if(ci->clip_zmax==1){
    GLdouble clipplane[4];

    clipplane[0]=0.0;
    clipplane[1]=0.0;
    clipplane[2]=-1.0;
    if(option==CLIP_ON_DENORMAL)clipplane[3]=ci->zmax;
    if(option==CLIP_ON)clipplane[3]=NORMALIZE_Z(ci->zmax);
    glClipPlane(GL_CLIP_PLANE5,clipplane);
    glEnable(GL_CLIP_PLANE5);
  }
  else{
    glDisable(GL_CLIP_PLANE5);
  }
}

