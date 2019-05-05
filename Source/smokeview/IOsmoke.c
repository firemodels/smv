#include "options.h"
#include "glew.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include GLUT_H
#include <float.h>

#include "update.h"
#include "smokeviewvars.h"
#include "IOvolsmoke.h"
#include "compress.h"

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
  for(i = 0; i < nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    switch (smoke3di->type){
    case SOOT:
      if(smoke3di->loaded == 1)nsootloaded++;
      nsootfiles++;
      break;
    case HRRPUV:
      if(smoke3di->loaded == 1)nhrrpuvloaded++;
      nhrrpuvfiles++;
      break;
    case TEMP:
      if(smoke3di->loaded == 1)ntemploaded++;
      ntempfiles++;
      break;
    case CO2:
      if(smoke3di->loaded == 1)nco2loaded++;
      nco2files++;
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
  }
}

/* ------------------ UpdateOpacityMap ------------------------ */

void UpdateOpacityMap(void){
  int i;

  unsigned char alpha;
  int ival0, ival1, ival2, ival3;
  float dx, *xplt;
  float ods[3];

  update_opacity_map=0;

  ods[0]  = slicehrrpuv_lower;
  ods[1]  = slicehrrpuv_middle;
  ods[2]  = slicehrrpuv_upper;

  ival0 = 0;
  ival1 = slicehrrpuv_cut1;
  ival2 = slicehrrpuv_cut2;
  ival3 = 256;

  xplt = meshinfo->xplt_orig;
  dx = xplt[1] - xplt[0];

  alpha = CLAMP(255*(1.0-pow(0.5, dx/MAX(ods[0],0.001))), 0, 255);
  for(i = ival0;i<ival1;i++){
    opacity_map[i] = alpha;
  }

  alpha = CLAMP(255*(1.0-pow(0.5, dx/MAX(ods[1],0.001))), 0, 255);
  for(i = ival1;i<ival2;i++){
    opacity_map[i] = alpha;
  }

  alpha = CLAMP(255*(1.0-pow(0.5, dx/MAX(ods[2],0.001))), 0, 255);
  for(i = ival2;i<ival3;i++){
    opacity_map[i] = alpha;
  }
}

/* ------------------ AdjustAlpha ------------------------ */

unsigned char AdjustAlpha(unsigned char alpha, float factor){
  double val, rr;
  float falpha;
  float term1, term2, term3, term4;

  rr = factor;
  falpha = alpha/255.0;

  //val = 1.0 - pow(1.0-falpha,rr);
  term1 = falpha*rr;
  term2 = falpha*(rr-1.0)/2.0;
  term3 = falpha*(rr-2.0)/3.0;
  term4 = falpha*(rr-3.0)/4.0;
  val = term1*(1.0-term2*(1.0-term3*(1.0-term4)));

  val = 255*val+0.5;
  if(val>255)val = 255;
  alpha = val;
  return alpha;
}

//              alphaf_out[n]=AdjustAlpha(ALPHAIN, eye_position_fds, xp, ASPECTRATIO, NORM, NORMTYPE);

// -------------------------- ADJUSTALPHA ----------------------------------

#define ADJUSTALPHA(ALPHAIN,ASPECTRATIO,NORM,NORMTYPE) \
            alphaf_out[n]=0;\
            if(ALPHAIN==0)continue;\
            if((adjustalphaflag==2||adjustalphaflag==3)&&iblank_smoke3d!=NULL&&iblank_smoke3d[n]==SOLID)continue;\
            if(adjustalphaflag==2){\
              alphaf_out[n]=ALPHAIN;\
            }\
            else{\
              alphaf_out[n]=AdjustAlpha(ALPHAIN, ASPECTRATIO);\
            }

#ifdef pp_SMOKEDIAG
#define SMOKESKIP \
  total_triangles++;\
  if(value[0]==0&&value[1]==0&&value[2]==0&&value[3]==0)continue;\
  total_drawn_triangles++
#else
#define SMOKESKIP \
  if(value[0]==0&&value[1]==0&&value[2]==0&&value[3]==0)continue
#endif
#ifdef pp_GPUSMOKE
#define SMOKETIMER   if(smoke_timer==1)triangle_count+=2
#else
#define SMOKETIMER
#endif

// -------------------------- DRAWVERTEX ----------------------------------
#define DRAWVERTEX(XX,YY,ZZ)        \
  value[0]=alphaf_ptr[n11]; \
  value[1]=alphaf_ptr[n12]; \
  value[2]=alphaf_ptr[n22]; \
  value[3]=alphaf_ptr[n21]; \
  SMOKESKIP;\
  SMOKETIMER;\
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
    unsigned char alphabyte;\
    float alphaval;\
    int mm;\
    mm = xyzindex[node];                                 \
    alphabyte = value[mm];                               \
    if(skip_global==2){\
      alphaval=alphabyte/255.0; \
      alphaval=alphaval*(2.0-alphaval);                  \
      alphabyte=alphaval*255.0;\
    }\
    else if(skip_global==3){\
      alphaval=alphabyte/255.0;                              \
      alphaval = alphaval*(3.0-alphaval*(3.0-alphaval));\
      alphabyte = 255*alphaval; \
    }\
    colorptr=smokecolor_ptr+ivalue[mm];\
    colorptr[3]=alphabyte;                                   \
    glColor4ubv(colorptr);                                \
    glVertex3f(XX,YY,ZZ);                                \
  }

// -------------------------- DRAWVERTEXTERRAIN ----------------------------------

#define DRAWVERTEXTERRAIN(XX,YY,ZZ)        \
  value[0]=alphaf_ptr[n11]; \
  value[1]=alphaf_ptr[n12]; \
  value[2]=alphaf_ptr[n22]; \
  value[3]=alphaf_ptr[n21]; \
  SMOKESKIP;\
  z_offset[XXX]=znode_offset[m11];\
  SMOKETIMER;\
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
    unsigned char alphabyte;\
    float alphaval;\
    int mm;\
    mm = xyzindex[node];                                 \
    alphabyte = value[mm];                               \
    if(skip_global==2){\
      alphaval=alphabyte/255.0; \
      alphaval=alphaval*(2.0-alphaval);                  \
      alphabyte=alphaval*255.0;\
    }\
    else if(skip_global==3){\
      alphaval=alphabyte/255.0;                              \
      alphaval = alphaval*(3.0-alphaval*(3.0-alphaval));\
      alphabyte = 255*alphaval; \
    }\
    colorptr=smokecolor_ptr+ivalue[mm];\
    colorptr[3]=alphabyte;                                   \
    glColor4ubv(colorptr);                                \
    glVertex3f(XX,YY,ZZ+z_offset[mm]);                                \
  }

// -------------------------- DRAWVERTEXGPU ----------------------------------

#define DRAWVERTEXGPU(XX,YY,ZZ) \
  value[0]=alphaf_in[n11];\
  value[1]=alphaf_in[n12];\
  value[2]=alphaf_in[n22];\
  value[3]=alphaf_in[n21];\
  if(value[0]==0&&value[1]==0&&value[2]==0&&value[3]==0)continue;\
  SMOKETIMER;\
  if((adjustalphaflag==2||adjustalphaflag==3)&&iblank_smoke3d!=NULL){\
    if(iblank_smoke3d[n11]==SOLID)value[0]=0;\
    if(iblank_smoke3d[n12]==SOLID)value[1]=0;\
    if(iblank_smoke3d[n22]==SOLID)value[2]=0;\
    if(iblank_smoke3d[n21]==SOLID)value[3]=0;\
  }\
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
  z_offset[XXX]=znode_offset[m11];\
  z_offset[YYY]=znode_offset[m12];\
  z_offset[ZZZ]=znode_offset[m22];\
  z_offset[3]=znode_offset[m21];\
  value[0]=alphaf_in[n11];\
  value[1]=alphaf_in[n12];\
  value[2]=alphaf_in[n22];\
  value[3]=alphaf_in[n21];\
  if((adjustalphaflag==2||adjustalphaflag==3)&&iblank_smoke3d!=NULL){\
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


  /* ------------------ DrawSmokeTest ------------------------ */
#ifdef pp_SMOKETEST
void DrawSmokeTest(void){
  meshdata *meshi;
  int i;
  float dy,y;
  float *boxmin, *boxmax, zmid;
  float ymin, ymax;
  float opacity_full, opacity_slice;

  meshi = meshinfo;
  boxmin = meshi->boxmin;
  boxmax = meshi->boxmax;
  ymin = boxmin[1] + 0.1;
  ymax = boxmin[1] + smoke_test_range;
  dy = (ymax-ymin) / (float)(smoke_test_nslices - 1);
  zmid = (boxmin[2] + boxmax[2]) / 2.0;

  opacity_full = smoke_test_opacity;
  opacity_slice = 1.0 - pow(1.0 - opacity_full, 1.0/(float)smoke_test_nslices);

  TransparentOn();
  glPushMatrix();
  glScalef(SCALE2SMV(1.0),SCALE2SMV(1.0),SCALE2SMV(1.0));
  glTranslatef(-xbar0,-ybar0,-zbar0);
  glBegin(GL_TRIANGLES);
  for(i=0;i<smoke_test_nslices;i++){
    y = ymax - (float)i*dy;
    glColor4f(smoke_test_color[0],smoke_test_color[1],smoke_test_color[2],opacity_slice);
    glVertex3f(boxmin[0],y,boxmin[2]);
    glVertex3f(boxmax[0],y,boxmin[2]);
    glVertex3f(boxmax[0],y,zmid);

    glVertex3f(boxmin[0],y,boxmin[2]);
    glVertex3f(boxmax[0],y,zmid);
    glVertex3f(boxmin[0],y,zmid);
  }
  y = ymin;
  glColor4f(smoke_test_color[0], smoke_test_color[1], smoke_test_color[2], opacity_full);
  glVertex3f(boxmin[0], y, zmid);
  glVertex3f(boxmax[0], y, zmid);
  glVertex3f(boxmax[0], y, boxmax[2]);

  glVertex3f(boxmin[0], y, zmid);
  glVertex3f(boxmax[0], y, boxmax[2]);
  glVertex3f(boxmin[0], y, boxmax[2]);
  glEnd();
  glPopMatrix();
  TransparentOff();
}
#endif

  /* ------------------ GetLightLimit ------------------------ */

void GetLightLimit(float *xyz1, float *dxyz, float *xyz_light, int light_type, float *xyz2, float *length){
  float tval = TMAX, length3[3];
  int i;

  // shoot a ray from xyz1 in direction dxyz and intersect with bounding box of all meshes.  return intersection in xyz2
  // if light position is within bounding box then return its position in xyz2

  for(i = 0; i < 3; i++){
    if(dxyz[i] != 0.0){
      float tval1;

      if(dxyz[i]>0.0){
        tval1 = (boxmax_global[i] - xyz1[i]) / dxyz[i];
        if(tval1 >= 0.0&&tval1 <= tval)tval = tval1;
      }
      else{
        tval1 = (boxmin_global[i] - xyz1[i]) / dxyz[i];
        if(tval1 >= 0.0&&tval1 <= tval)tval = tval1;
      }
    }
  }
  if(tval >= TMAX) tval = 0.0;
  xyz2[0] = xyz1[0] + tval*dxyz[0];
  xyz2[1] = xyz1[1] + tval*dxyz[1];
  xyz2[2] = xyz1[2] + tval*dxyz[2];

  if(light_type == LOCAL_LIGHT){
    float dxyz2[3], dxyzlight[3];

    VEC3DIFF(dxyz2, xyz2, xyz1);
    VEC3DIFF(dxyzlight, xyz_light, xyz1);
    if(DOT3(dxyzlight,dxyzlight) < DOT3(dxyz2,dxyz2)){
      VEC3EQ(xyz2, xyz_light);
    }
  }
  VEC3DIFF(length3, xyz2, xyz1);
  *length = NORM3(length3);
}

/* ------------------ GetCellindex ------------------------ */

int GetCellindex(float *xyz, meshdata **mesh_tryptr){
  int i;
  meshdata *mesh_try=NULL;

  if(mesh_tryptr != NULL)mesh_try = *mesh_tryptr;
  for(i = -1; i < nmeshes; i++){
    meshdata *meshi;
    float *boxmin, *boxmax, *dbox;

    if(i == -1){
      if(mesh_try == NULL)continue;
      meshi = mesh_try;
    }
    else{
      meshi = meshinfo + i;
      if(meshi == mesh_try)continue;
    }
    boxmin = meshi->boxmin;
    boxmax = meshi->boxmax;
    dbox = meshi->dbox;
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
  vr = &(mesh_soot->volrenderinfo);
  if(vr->smokedataptrs ==NULL)return 0.0;
  itime = CLAMP(itime,0, vr->ntimes -1);
  if(vr->smokedataptrs[itime] == NULL)return 0.0;
  slice_vals = vr->smokedataptrs[itime];
  soot_val = slice_vals[ijk];
  return soot_val;
}

/* ------------------ InitLightFractions ------------------------ */

int InitLightFractions(meshdata *meshi, float *xyz_light, int light_type){
  // type LOCAL_LIGHT     xyz contains position of light
  // type INFINITE_LIGHT  xyz contains direction of light (infinitely far away)

  float xyz1[3],xyz2[3];
  float *xplt, *yplt, *zplt, dxyz[3];
  int ibar, jbar, kbar;
  int itime, ntimes, ntotal, ncount;
  volrenderdata *vr;
  float *light_fraction;

  vr = &(meshi->volrenderinfo);
  ntimes = vr->ntimes;

  if(update_boxbounds == 1){
    int i;

    update_boxbounds = 0;
    for(i = 0; i < nmeshes; i++){
      meshdata *meshii;
      float *boxmin, *boxmax, *xp, *yp, *zp;

      meshii = meshinfo + i;
      boxmin = meshii->boxmin;
      boxmax = meshii->boxmax;
      xp = meshii->xplt_orig;
      yp = meshii->yplt_orig;
      zp = meshii->zplt_orig;
      if(i == 0){
        VEC3EQ(boxmin_global, boxmin);
        VEC3EQ(boxmax_global, boxmax);
        max_cell_length = xp[1] - xp[0];
      }
      else{
        boxmin_global[0] = MIN(boxmin_global[0], boxmin[0]);
        boxmin_global[1] = MIN(boxmin_global[1], boxmin[1]);
        boxmin_global[2] = MIN(boxmin_global[2], boxmin[2]);
        boxmax_global[0] = MAX(boxmax_global[0], boxmax[0]);
        boxmax_global[1] = MAX(boxmax_global[1], boxmax[1]);
        boxmax_global[2] = MAX(boxmax_global[2], boxmax[2]);
      }
      max_cell_length = MIN(max_cell_length, xp[1] - xp[0]);
      max_cell_length = MIN(max_cell_length, yp[1] - yp[0]);
      max_cell_length = MIN(max_cell_length, zp[1] - zp[0]);
    }
  }
  if(light_type == INFINITE_LIGHT){
    float norm;

    VEC3EQ(dxyz, xyz_light);
    dxyz[0] = -dxyz[0];
    dxyz[1] = -dxyz[1];
    dxyz[2] = -dxyz[2];
    norm = NORM3(dxyz);
    if(norm == 0.0)return 1;
    VEC3DA(dxyz, norm);
  }

  xplt = meshi->xplt_orig;
  yplt = meshi->yplt_orig;
  zplt = meshi->zplt_orig;
  ibar = meshi->ibar;
  jbar = meshi->jbar;
  kbar = meshi->kbar;

  FREEMEMORY(meshi->light_fraction);
  FREEMEMORY(meshi->uc_light_fraction);
  ntotal = ntimes*(ibar + 1)*(jbar + 1)*(kbar + 1);
  NewMemory((void **)&meshi->light_fraction,  ntotal*sizeof(float));
  light_fraction = meshi->light_fraction;
//  NewMemory((void **)&meshi->uc_light_fraction,  ntotal*sizeof(unsigned char));

  ncount = 0;
  for(itime = 0;itime<ntimes;itime++){
    int k;

    printf("time step: %i\n", itime);

    for(k = 0; k<=kbar; k++){
      int j;

      xyz1[2] = zplt[k];
      for(j = 0; j <= jbar; j++){
        int i;

        xyz1[1] = yplt[j];
        for(i = 0; i <= ibar; i++){
          float length;
          float soot_sum, opacity, arg;
          float ddlength;
          int ii,nlength;
          meshdata *mesh_try;

          xyz1[0] = xplt[i];
          if(light_type == LOCAL_LIGHT){
            VEC3DIFF(dxyz, xyz1, xyz_light);
            NORMALIZE3(dxyz);
          }
          GetLightLimit(xyz1,dxyz,xyz_light,light_type,xyz2,&length);
          nlength = length / max_cell_length + 1;
          ddlength = 0.0;
          if(nlength>1)ddlength = length / (float)(nlength - 1);

          mesh_try = meshi;

          soot_sum = 0.0;
          if(nlength>1){
            for(ii = 0;ii < nlength;ii++){
              float xyz[3], factor;
              float soot_density;

              if(nlength == 1)break;
              factor = (float)ii / (float)(nlength - 1);

              xyz[0] = xyz1[0] * (1.0 - factor) + xyz2[0] * factor;
              xyz[1] = xyz1[1] * (1.0 - factor) + xyz2[1] * factor;
              xyz[2] = xyz1[2] * (1.0 - factor) + xyz2[2] * factor;
              soot_density = GetSootDensity(xyz, itime, &mesh_try);
              if(ii == 0||ii==nlength-1){  // trapezoidal rule
                soot_sum += soot_density;
              }
              else{
                soot_sum += 2.0*soot_density;
              }
              if(mesh_try==NULL)break;  // outside of domain
            }
          }
          arg = mass_extinct*soot_sum*ddlength/2.0;
          if(arg>9.0){ // exp(-9) ~ 0.0001
            opacity = 0.0;
          }
          else{
            opacity = exp(-arg);  // fraction of light reaching xyz
          }
          *light_fraction++ = opacity;
          ncount++;
        }
      }
    }
  }
  return 0;
}

/* ------------------ LightFractions2File ------------------------ */

void LightFractions2File(meshdata *meshi){
  char longlabel[30], shortlabel[30], unitlabel[30], slicefile[256];
  int len_longlabel, len_shortlabel, len_unitlabel, len_slicefile;
  int meshnum;
  char smvlight_file[256];
  FILE *slicesmv = NULL;
  int is1, is2, js1, js2, ks1, ks2;
  volrenderdata *vr;

  vr = &(meshi->volrenderinfo);

  strcpy(smvlight_file,fdsprefix);
  strcat(smvlight_file,"_fraction_smv.txt");
  if(meshi==meshinfo){
    slicesmv = fopen(smvlight_file, "w");
  }
  else{
    slicesmv = fopen(smvlight_file, "w");
  }
  meshnum = (int)(meshi-meshinfo)+1;
  sprintf(slicefile, "%s_frac_%i.sf", fdsprefix, meshnum);
  len_slicefile = strlen(slicefile);

  strcpy(longlabel, "light fraction");
  len_longlabel = strlen(longlabel);

  strcpy(shortlabel, "frac");
  len_shortlabel = strlen(shortlabel);

  strcpy(unitlabel, " ");
  len_unitlabel = strlen(unitlabel);

  is1 = 0;
  is2 = meshi->ibar;

  js1 = 0;
  js2 = meshi->jbar;

  ks1 = 0;
  ks2 = meshi->kbar;

  fprintf(slicesmv, "SLCF %i & %i %i %i %i %i %i\n", meshnum, is1, is2, js1, js2, ks1, ks2);
  fprintf(slicesmv, " %s\n", slicefile);
  fprintf(slicesmv, " %s\n", longlabel);
  fprintf(slicesmv, " %s\n", shortlabel);
  fprintf(slicesmv, " %s\n", unitlabel);
  fclose(slicesmv);

  FORTwriteslicedata2(slicefile, longlabel, shortlabel, unitlabel,
    &is1, &is2, &js1, &js2, &ks1, &ks2,
    meshi->light_fraction, vr->times, &vr->ntimes, len_slicefile, len_longlabel, len_shortlabel, len_unitlabel);
}

/* ------------------ InitAllLightFractions ------------------------ */

void InitAllLightFractions(float *xyz_light, int light_type){
  int i;

  update_boxbounds = 1;
  for(i=0; i<nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo+i;
    InitLightFractions(meshi, xyz_light, light_type);
    LightFractions2File(meshi);
  }
}

/* ------------------ IsSmokeComponentPresent ------------------------ */

int IsSmokeComponentPresent(smoke3ddata *smoke3di){
  int i;

  for(i = 0;i < MAXSMOKETYPES;i++){
    smoke3ddata *smoke_component;

    if(smoke3di->smokestate[i].index == -1)continue;
    smoke_component = smoke3dinfo + smoke3di->smokestate[i].index;
    if(smoke_component->loaded != 1 || smoke_component->display != 1)continue;
    if(smoke_component->frame_all_zeros[smoke_component->ismoke3d_time] == SMOKE3D_ZEROS_ALL)continue;
    return 1;
  }
  return 0;
}

#ifdef pp_GPUSMOKE
/* ------------------ IsSmokeInMesh ------------------------ */

int IsSmokeInMesh(meshdata *meshi){
  int i;

  for(i = 0;i < nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    meshdata *meshj;

    smoke3di = smoke3dinfo + i;
    if(smoke3di->loaded == 0 || smoke3di->display == 0)continue;
    meshj = meshinfo + smoke3di->blocknumber;
    if(meshj != meshi)continue;
    if(IsSmokeComponentPresent(smoke3di) == 1)return 1;
  }
  return 0;
}
#endif

#ifdef pp_GPU
#ifdef pp_GPUSMOKE

/* ------------------ InitSmoke3DTexture ------------------------ */

void InitSmoke3DTexture(meshdata *meshi){
  GLint border_size = 0;
  GLsizei nx, ny, nz;

  // define smoke texture

  if(meshi->smokealpha_ptr!=NULL){
    glActiveTexture(GL_TEXTURE0);
    if(meshi->smoke_texture_id != 0)glDeleteTextures(1, &meshi->smoke_texture_id);
    glGenTextures(1, &meshi->smoke_texture_id);
    glBindTexture(GL_TEXTURE_3D, meshi->smoke_texture_id);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    nx = meshi->ibar + 1;
    ny = meshi->jbar + 1;
    nz = meshi->kbar + 1;
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, nx, ny, nz, border_size, GL_RED, GL_FLOAT, meshi->smoke_texture_buffer);
    SNIFF_ERRORS("after smoke texture initialization");
  }

  // define fire texture

  if(meshi->fire_texture_buffer!=NULL){
    glActiveTexture(GL_TEXTURE1);
    if(meshi->fire_texture_id != 0)glDeleteTextures(1, &meshi->fire_texture_id);
    glGenTextures(1, &meshi->fire_texture_id);
    glBindTexture(GL_TEXTURE_3D, meshi->fire_texture_id);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    nx = meshi->ibar+1;
    ny = meshi->jbar+1;
    nz = meshi->kbar+1;
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, nx, ny, nz, border_size, GL_RED, GL_FLOAT, meshi->fire_texture_buffer);
    SNIFF_ERRORS("after fire texture initialization");
  }

  // define co2 texture

  if(meshi->co2_texture_buffer != NULL){
    glActiveTexture(GL_TEXTURE2);
    if(meshi->co2_texture_id != 0)glDeleteTextures(1, &meshi->co2_texture_id);
    glGenTextures(1, &meshi->co2_texture_id);
    glBindTexture(GL_TEXTURE_3D, meshi->co2_texture_id);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    nx = meshi->ibar + 1;
    ny = meshi->jbar + 1;
    nz = meshi->kbar + 1;
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, nx, ny, nz, border_size, GL_RED, GL_FLOAT, meshi->co2_texture_buffer);
    SNIFF_ERRORS("after co2 texture initialization");
  }

  glActiveTexture(GL_TEXTURE0);
}

/* ------------------ UpdateSmoke3DTexture ------------------------ */

void UpdateSmoke3DTexture(smoke3ddata *smoke3di){
  meshdata *meshi;
  GLsizei nx, ny, nz;
  int i;
  unsigned char *firecolor, *co2color;

  GLint xoffset = 0, yoffset = 0, zoffset = 0;

  meshi = meshinfo + smoke3di->blocknumber;
  InitSmoke3DTexture(meshi);

  nx = meshi->ibar + 1;
  ny = meshi->jbar + 1;
  nz = meshi->kbar + 1;

  // update smoke texture

  if(meshi->smokealpha_ptr!=NULL){
    unsigned char *cbuffer;
    float *v;

    cbuffer = meshi->smokealpha_ptr;
    if(meshi->smoke_texture_buffer==NULL){
      NewMemory((void **)&(meshi->smoke_texture_buffer), smoke3di->nchars_uncompressed * sizeof(float));
    }
    v = meshi->smoke_texture_buffer;
    for(i = 0; i<smoke3di->nchars_uncompressed; i++){
      *v++ = (float)(*cbuffer++)/255.0;
    }

    glActiveTexture(GL_TEXTURE0);
    glTexSubImage3D(GL_TEXTURE_3D, 0, xoffset, yoffset, zoffset, nx, ny, nz, GL_RED, GL_FLOAT, meshi->smoke_texture_buffer);
    SNIFF_ERRORS("after smoke texture update");
  }

  // update fire texture

  firecolor = smoke3di->smokestate[HRRPUV].color;
  if(firecolor!=NULL){
    unsigned char *cbuffer;
    float *v, factor;

    factor = hrrpuv_max_smv/255.0;
    cbuffer = firecolor;
    if(meshi->fire_texture_buffer==NULL){
      NewMemory((void **)&(meshi->fire_texture_buffer), smoke3di->nchars_uncompressed * sizeof(float));
    }
    v = meshi->fire_texture_buffer;
    for(i = 0; i<smoke3di->nchars_uncompressed; i++){
      *v++ = (float)(*cbuffer++)*factor;
    }

    glActiveTexture(GL_TEXTURE1);
    glTexSubImage3D(GL_TEXTURE_3D, 0, xoffset, yoffset, zoffset, nx, ny, nz, GL_RED, GL_FLOAT, meshi->fire_texture_buffer);
    SNIFF_ERRORS("after fire texture update");
  }

  // update co2 texture

  co2color = smoke3di->smokestate[CO2].color;
  if(co2color != NULL){
    unsigned char *cbuffer;
    float *v, factor;

    factor = 0.1 / 255.0;
    cbuffer = co2color;
    if(meshi->co2_texture_buffer==NULL){
      NewMemory((void **)&(meshi->co2_texture_buffer), smoke3di->nchars_uncompressed * sizeof(float));
    }
    v = meshi->co2_texture_buffer;
    for(i = 0; i < smoke3di->nchars_uncompressed; i++){
      *v++ = (float)(*cbuffer++)*factor;
    }

    glActiveTexture(GL_TEXTURE2);
    glTexSubImage3D(GL_TEXTURE_3D, 0, xoffset, yoffset, zoffset, nx, ny, nz, GL_RED, GL_FLOAT, meshi->co2_texture_buffer);
    SNIFF_ERRORS("after co2 texture update");
  }

  if(slicesmoke_colormap_id_defined ==-1){
    slicesmoke_colormap_id_defined = 1;
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &slicesmoke_colormap_id);
    glBindTexture(GL_TEXTURE_1D, slicesmoke_colormap_id);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef pp_GPU
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
#else
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
#endif
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, MAXSMOKERGB, 0, GL_RGBA, GL_FLOAT, rgb_slicesmokecolormap_01);
    SNIFF_ERRORS("after smoke color bar update");
  }
  glActiveTexture(GL_TEXTURE0);
}

/* ------------------ DrawSmoke3DGPUNew ------------------------ */
#ifdef pp_SMOKETEST
int DrawSmoke3DGPUNew(smoke3ddata *smoke3di, int option){
#else
int DrawSmoke3DGPUNew(smoke3ddata *smoke3di){
#endif
  int i;
  meshdata *meshi;
  float *boxmin, *boxmax;
  unsigned char *firecolor, *co2color;
  float fire_alpha;
  int count = 0;
  float grid_ratio;

  meshi = meshinfo + smoke3di->blocknumber;
  firecolor = smoke3di->smokestate[HRRPUV].color;
  co2color = smoke3di->smokestate[CO2].color;
  grid_ratio = smoke3d_delta_perp/smoke3d_delta_par_min;
  UpdateSmoke3DTexture(smoke3di);
  boxmin = meshi->boxmin;
  boxmax = meshi->boxmax;
  if(meshi->smokealpha_ptr!=NULL){
    glUniform1i(GPUnewsmoke_have_smoke, 1);
  }
  else{
    glUniform1i(GPUnewsmoke_have_smoke, 0);
  }
  if(firecolor!=NULL){
    glUniform1i(GPUnewsmoke_have_fire, 1);
  }
  else{
    glUniform1i(GPUnewsmoke_have_fire, 0);
  }
  if(co2color!=NULL){
    glUniform1i(GPUnewsmoke_have_co2, 1);
  }
  else{
    glUniform1i(GPUnewsmoke_have_co2, 0);
  }
  glUniform1i(GPUnewsmoke_smoketexture, 0);
  glUniform1i(GPUnewsmoke_firetexture,  1);
  glUniform1i(GPUnewsmoke_co2texture, 2);
  glUniform1i(GPUnewsmoke_smokecolormap,3);
  glUniform3f(GPUnewsmoke_boxmin, boxmin[0], boxmin[1], boxmin[2]);
  glUniform3f(GPUnewsmoke_boxmax, boxmax[0], boxmax[1], boxmax[2]);
  glUniform3f(GPUnewsmoke_co2_color,
    (float)co2_color_int255[0]/255.0,
    (float)co2_color_int255[1]/255.0,
    (float)co2_color_int255[2]/255.0);
  glUniform1f(GPUnewsmoke_co2_alpha, (float)smoke3di->co2_alpha/255.0);
  glUniform1f(GPUnewsmoke_co2_fraction, co2_fraction);
  glUniform1f(GPUnewsmoke_grid_ratio, grid_ratio);

  fire_alpha = CLAMP((float)smoke3di->fire_alpha/255.0,0.0,1.0);

  glUniform1f(GPUnewsmoke_hrrpuv_max_smv, hrrpuv_max_smv);
  glUniform1f(GPUnewsmoke_hrrpuv_cutoff,  global_hrrpuv_cutoff);
  glUniform1f(GPUnewsmoke_fire_alpha,     fire_alpha);

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);
  TransparentOn();
  CheckMemory;
  ASSERT(meshi->nsmokeplaneinfo>=0);
  if(meshi->nsmokeplaneinfo<0){
    ASSERT(0);
  }
  glBegin(GL_TRIANGLES);
  for(i = 0; i<meshi->nsmokeplaneinfo; i++){
    meshplanedata *spi;
    int j;

    spi = meshi->smokeplaneinfo + i;
    for(j = 0; j<spi->ntriangles; j++){
      float *v1, *v2, *v3;
      int iv1, iv2, iv3;

      iv1 = spi->triangles[3*j];
      iv2 = spi->triangles[3*j + 1];
      iv3 = spi->triangles[3*j + 2];
      v1 = spi->verts + 3*iv1;
      v2 = spi->verts + 3*iv2;
      v3 = spi->verts + 3*iv3;
      glVertex3fv(v1);
      glVertex3fv(v2);
      glVertex3fv(v3);
    }
    count += spi->ntriangles;
  }
  glEnd();
  TransparentOff();
  glPopMatrix();
  SNIFF_ERRORS("after smoke DrawSmoke3DGPUNew");
  return count;
}

#define INTERP_SMOKE3D(val) \
  val00 = MIX(dx, vv[i100], vv[i000]);\
  val10 = MIX(dx, vv[i110], vv[i010]);\
  val01 = MIX(dx, vv[i101], vv[i001]);\
  val11 = MIX(dx, vv[i111], vv[i011]);\
  val0  = MIX(dy,    val10,    val00);\
  val1  = MIX(dy,    val11,    val01);\
  val   = MIX(dz,     val1,     val0)

/* ------------------ GetNSmokeTypes ------------------------ */

int GetNSmokeTypes(smoke3ddata * smoke3di, int *have_vals){
  meshdata *valmesh;
  unsigned char *smoke, *fire, *co2;

  valmesh = meshinfo+smoke3di->blocknumber;

  smoke = valmesh->smokealpha_ptr;
  fire = smoke3di->smokestate[HRRPUV].color;
  co2 = smoke3di->smokestate[CO2].color;

  have_vals[0] = 0;
  have_vals[1] = 0;
  have_vals[2] = 0;

  if(smoke!=NULL)have_vals[0]=1;;
  if(fire!=NULL)have_vals[1]=1;
  if(co2!=NULL)have_vals[2]=1;
  return have_vals[0]+have_vals[1]+have_vals[2];
}

/* ------------------ GetSmoke3DVals ------------------------ */

void GetSmoke3DVals(float *xyz, smoke3ddata * smoke3di, float *vals, int *have_vals){
  int i, j, k;
  unsigned char *vv;
  float *xplt, *yplt, *zplt;
  float dxbar, dybar, dzbar;
  int ibar, jbar, kbar;
  int nx, ny, nz;
  float dx, dy, dz;
  float val00, val10, val01, val11;
  float val0, val1;
  float val_fraction;
  int i000, i001, i010, i011, i100, i101, i110, i111;
  int ijk;
  int iplus = 0, jplus = 0, kplus = 0;
  unsigned char *smoke, *fire, *co2;

  meshdata *valmesh;

  if(smoke_timer==1)smoke_function_count++;
  valmesh = meshinfo+smoke3di->blocknumber;

  smoke = valmesh->smokealpha_ptr;
  fire = smoke3di->smokestate[HRRPUV].color;
  co2 = smoke3di->smokestate[CO2].color;

  have_vals[0] = 0;
  have_vals[1] = 0;
  have_vals[2] = 0;

  xplt = valmesh->xplt_orig;
  yplt = valmesh->yplt_orig;
  zplt = valmesh->zplt_orig;
  ibar = valmesh->ibar;
  jbar = valmesh->jbar;
  kbar = valmesh->kbar;
  dxbar = xplt[1]-xplt[0];
  dybar = yplt[1]-yplt[0];
  dzbar = zplt[1]-zplt[0];

  nx = ibar+1;
  ny = jbar+1;
  nz = kbar+1;

  i = GETINDEX(xyz[0], xplt[0], dxbar, nx);
  j = GETINDEX(xyz[1], yplt[0], dybar, ny);
  k = GETINDEX(xyz[2], zplt[0], dzbar, nz);

  ijk = i+nx*(j+k*ny);

  if(smoke_fast_interp==0){
    dx = (xyz[0]-xplt[i])/dxbar;
    dx = CLAMP(dx, 0.0, 1.0);
    dy = (xyz[1]-yplt[j])/dybar;
    dy = CLAMP(dy, 0.0, 1.0);
    dz = (xyz[2]-zplt[k])/dzbar;
    dz = CLAMP(dz, 0.0, 1.0);

    if(i+1<=ibar)iplus = 1;
    if(j+1<=jbar)jplus = nx;
    if(k+1<=kbar)kplus = nx*ny;

    i000 = 0;                 // i,j,k
    i001 = kplus;             // i,j,k+1

    i010 = jplus;             // i,j+1,k, 
    i011 = jplus+kplus;       // i,j+1,k+1

    i100 = iplus;             // i+1,j,k
    i101 = iplus+kplus;       // i+1,j,k+1

    i110 = iplus+jplus;       // i+1,j+1,k
    i111 = iplus+jplus+kplus; // i+1,j+1,k+1
  }

  if(smoke!=NULL){
    float smokeval, ratio;

    vv = smoke+ijk;
    if(smoke_fast_interp==1){
      smokeval = *vv;
    }
    else{
      INTERP_SMOKE3D(smokeval);
    }
    val_fraction = smokeval/254.0;
    val_fraction = CLAMP(val_fraction, 0.0, 1.0);
    ratio = smoke3d_delta_perp/smoke3d_delta_par_min;
    if(ratio>=1.1)val_fraction = 1.0-pow(1-val_fraction, ratio);
    vals[0] = val_fraction;
    have_vals[0] = 1;
  }
  if(fire!=NULL){
    float fireval;

    vv = fire+ijk;
    if(smoke_fast_interp==1){
      fireval = *vv;
    }
    else{
      INTERP_SMOKE3D(fireval);
    }
    vals[1] = fireval*hrrpuv_max_smv/255.0;
    have_vals[1] = 1;
  }
  if(co2 != NULL){
    float co2val, co2max = 0.1;

    vv = co2 + ijk;
    if(smoke_fast_interp==1){
      co2val = *vv;
    }
    else{
      INTERP_SMOKE3D(co2val);
    }
    vals[2] = co2max*co2val/255.0;
    have_vals[2] = 1;
  }
}

/* ------------------ DrawSmoke3DOutline2 ------------------------ */

void DrawSmoke3DOutline2(smoke3ddata *smoke3di){
  int i;
  meshdata *meshi;

  meshi = meshinfo+smoke3di->blocknumber;

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);
  CheckMemory;
  ASSERT(meshi->nsmokeplaneinfo>=0);
  if(meshi->nsmokeplaneinfo<0){
    ASSERT(0);
  }
  glLineWidth(plane_outline_width);
  glBegin(GL_LINES);
  glColor3f(0.0,0.0,0.0);
  for(i = 0; i<meshi->nsmokeplaneinfo; i++){
    meshplanedata *spi;
    int j;

    spi = meshi->smokeplaneinfo+i;
    for(j = 0; j<spi->ntris2; j++){
      float *v1, *v2, *v3;
      int iv1, iv2, iv3;

      iv1 = spi->tris2[3*j];
      iv2 = spi->tris2[3*j+1];
      iv3 = spi->tris2[3*j+2];
      v1 = spi->verts2+3*iv1;
      v2 = spi->verts2+3*iv2;
      v3 = spi->verts2+3*iv3;
      glVertex3fv(v1);
      glVertex3fv(v2);
      glVertex3fv(v2);
      glVertex3fv(v3);
      glVertex3fv(v3);
      glVertex3fv(v1);
    }
  }
  glEnd();

  if(smoke_show_polygon==1){
    glLineWidth(2.0*plane_outline_width+2.0);
    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 1.0);
    for(i = 0; i<meshi->nsmokeplaneinfo; i++){
      meshplanedata *spi;
      int j;

      spi = meshi->smokeplaneinfo+i;
      for(j = 0; j<spi->npolys; j++){
        float *v1, *v2;
        int iv1, iv2;
        int jp1;

        iv1 = spi->polys[j];
        jp1 = j+1;
        if(j==spi->npolys-1)jp1 = 0;
        iv2 = spi->polys[jp1];
        v1 = spi->verts+3*iv1;
        v2 = spi->verts+3*iv2;
        glVertex3fv(v1);
        glVertex3fv(v2);
      }
    }
    glEnd();
  }
  glPopMatrix();
  SNIFF_ERRORS("after smoke DrawSmoke3DOutline2");
}

/* ------------------ DrawSmokeVertex ------------------------ */

#ifdef pp_SMOKETEST
void DrawSmokeVertex(smoke3ddata *smoke3di, float *v, float *t, int *hv,int option){
#else
void DrawSmokeVertex(smoke3ddata *smoke3di, float *v, float *t, int *hv){
#endif
  // SMOKE3D_FIRE_ONLY      0
  // SMOKE3D_SMOKE_ONLY     1
  // SMOKE3D_SMOKE_AND_FIRE 2
  int use_smoke = 1, index = 0;
  float sootval = 0.0, fireval = 0.0, co2val = 0.0;

  if(hv[VSOOT]==1)sootval = t[index++];
  if(hv[VFIRE]==1)fireval = t[index++];
  if(hv[VCO2]==1)co2val = t[index++];

  if(hv[VFIRE]==1){
#ifdef pp_SMOKETEST
    if(option!=SMOKE3D_SMOKE_ONLY&&fireval>global_hrrpuv_cutoff){
#else
    if(fireval>global_hrrpuv_cutoff){
#endif
      float *color, alpha, mergecolor[3];

      //      float alpha_factor;
      int color_index;

      //alpha_factor = CLAMP((fireval-global_hrrpuv_cutoff)/(global_hrrpuv_max-global_hrrpuv_cutoff),0.0,1.0);
      color_index = CLAMP(255*fireval/hrrpuv_max_smv, 0, 255);
      color = rgb_slicesmokecolormap_01+4*color_index;

      alpha = (float)smoke3di->fire_alpha/255.0;
      if(hv[VSOOT]==1&&hv[VCO2]==1){
        float f1 = 1.0, f2 = 0.0, denom, co2alpha;

        f1 = ABS(1.0-co2_fraction)*sootval;
        f2 = ABS(co2_fraction)*co2val;
        denom = f1+f2;
        if(denom>0.0){
          f1 /= denom;
          f2 /= denom;
        }
        co2alpha = (float)smoke3di->co2_alpha/255.0;
        mergecolor[0] = f2*co2_color_int255[0]/255.0+f1*color[0];
        mergecolor[1] = f2*co2_color_int255[1]/255.0+f1*color[1];
        mergecolor[2] = f2*co2_color_int255[2]/255.0+f1*color[2];
        alpha = f2*co2alpha+f1*alpha;
        color = mergecolor;
      }
      use_smoke = 0;
      glColor4f(color[0], color[1], color[2], alpha);
      glVertex3fv(v);
    }
    else{
      if(hv[VSOOT]==0){
        use_smoke = 0;
        glColor4f(0.0, 0.0, 0.0, 0.0);
        glVertex3fv(v);
      }
    }
  }
  if(use_smoke==1){
#ifdef pp_SMOKETEST
    if(option==SMOKE3D_FIRE_ONLY){
      glColor4f(0.0, 0.0, 0.0, 0.0);
    }
    else{
      glColor4f(0.0, 0.0, 0.0, sootval);
    }
#else
    glColor4f(0.0, 0.0, 0.0, sootval);
#endif
    glVertex3fv(v);
  }
}

/* ------------------ DrawSmoke3DNew ------------------------ */

#ifdef pp_SMOKETEST
int DrawSmoke3DNew(smoke3ddata *smoke3di, int option){
#else
int DrawSmoke3DNew(smoke3ddata *smoke3di){
#endif

  int i, nsmoketypes, ntriangles = 0, have_vals[3];
  meshdata *meshi;
  int nsmokeplanes;

  meshi = meshinfo+smoke3di->blocknumber;
  nsmoketypes = GetNSmokeTypes(smoke3di,have_vals);

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);
  TransparentOn();

  if(smoke_outline==1){
    glBegin(GL_LINES);
  }
  else{
    glBegin(GL_TRIANGLES);
  }
  if(smoke_subset==1){
    nsmokeplanes = MIN(smoke_num, meshi->nsmokeplaneinfo);
    nsmokeplanes = MAX(0, nsmokeplanes);
  }
  else{
    nsmokeplanes = meshi->nsmokeplaneinfo;
  }
  for(i = 0; i<nsmokeplanes; i++){
    meshplanedata *spi;
    int j;

    spi = meshi->smokeplaneinfo+i;
    if(spi->drawsmoke==1){
      for(j = 0; j<spi->ntris2; j++){
        float *v1, *v2, *v3;
        float *vals1, *vals2, *vals3;
        int iv1, iv2, iv3;

        iv1 = spi->tris2[3*j];
        iv2 = spi->tris2[3*j+1];
        iv3 = spi->tris2[3*j+2];
        v1 = spi->verts2+3*iv1;
        v2 = spi->verts2+3*iv2;
        v3 = spi->verts2+3*iv3;
        vals1 = spi->vals2+nsmoketypes*iv1;
        vals2 = spi->vals2+nsmoketypes*iv2;
        vals3 = spi->vals2+nsmoketypes*iv3;
        // don't draw the triangle if there is smoke and the smoke is all zero
        if(have_vals[0]==0||vals1[0]!=0.0||vals2[0]!=0.0||vals3[0]!=0.0){
#ifdef pp_SMOKETEST
          if(smoke_outline==1){
            DrawSmokeVertex(smoke3di, v1, vals1, have_vals, option);
            DrawSmokeVertex(smoke3di, v2, vals2, have_vals, option);
            DrawSmokeVertex(smoke3di, v2, vals2, have_vals, option);
            DrawSmokeVertex(smoke3di, v3, vals3, have_vals, option);
            DrawSmokeVertex(smoke3di, v3, vals3, have_vals, option);
            DrawSmokeVertex(smoke3di, v1, vals1, have_vals, option);
          }
          else{
            DrawSmokeVertex(smoke3di, v1, vals1, have_vals, option);
            DrawSmokeVertex(smoke3di, v2, vals2, have_vals, option);
            DrawSmokeVertex(smoke3di, v3, vals3, have_vals, option);
          }
#else
          if(smoke_outline==1){
            DrawSmokeVertex(smoke3di, v1, vals1, have_vals);
            DrawSmokeVertex(smoke3di, v2, vals2, have_vals);
            DrawSmokeVertex(smoke3di, v2, vals2, have_vals);
            DrawSmokeVertex(smoke3di, v3, vals3, have_vals);
            DrawSmokeVertex(smoke3di, v3, vals3, have_vals);
            DrawSmokeVertex(smoke3di, v1, vals1, have_vals);
          }
          else{
            DrawSmokeVertex(smoke3di, v1, vals1, have_vals);
            DrawSmokeVertex(smoke3di, v2, vals2, have_vals);
            DrawSmokeVertex(smoke3di, v3, vals3, have_vals);
          }
#endif
        }
      }
      ntriangles += spi->ntris2;
    }
  }
  glEnd();
  TransparentOff();
  glPopMatrix();
  return ntriangles;
}

#endif  
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
  float x11[3], x12[3], x22[3], x21[3];
  int n11, n12, n22, n21;
  int ipj, jpk, ipk, jmi, kmi, kmj;
  int iii, jjj, kkk;
  int slice_end, slice_beg;
  float aspectratio;
  int ssmokedir;
  unsigned char *iblank_smoke3d;
  int have_smoke;

  unsigned char *firecolor, *alphaf_in;
  float value[4], fvalue[4];

  meshdata *meshi;

  meshi = meshinfo+smoke3di->blocknumber;
  if(meshvisptr[meshi-meshinfo]==0)return;

  firecolor = smoke3di->smokestate[HRRPUV].color;

  {
    smoke3ddata *sooti = NULL;

    if(smoke3di->smokestate[SOOT].index >=0){
      sooti = smoke3dinfo+smoke3di->smokestate[SOOT].index;
    }
    if(sooti!=NULL&&sooti->display==1){
      have_smoke = 1;
    }
    else{
      have_smoke = 0;
    }
  }
  iblank_smoke3d = meshi->iblank_smoke3d;

  // meshi->hrrpuv_cutoff
  // hrrpuv_max_smv;

  meshi = meshinfo+smoke3di->blocknumber;
  if(meshvisptr[meshi-meshinfo]==0)return;
  value[0] = 255;
  value[1] = 255;
  value[2] = 255;
  value[3] = 255;

  if(nterraininfo>0){
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

  nx = smoke3di->is2+1-smoke3di->is1;
  ny = js2+1-js1;
  nz = ks2+1-ks1;
  nxy = nx*ny;

  ssmokedir = meshi->smokedir;
  skip_local = smokeskipm1+1;

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

  if(cullfaces==1)glDisable(GL_CULL_FACE);

  glUniform1i(GPU_adjustalphaflag, adjustalphaflag);
  glUniform1i(GPU_have_smoke, have_smoke);
  glUniform1i(GPU_smokecolormap, 0);
  glUniform1f(GPU_smoke3d_rthick, smoke3d_rthick);
  glUniform1f(GPU_hrrpuv_max_smv, hrrpuv_max_smv);
  glUniform1f(GPU_hrrpuv_cutoff, global_hrrpuv_cutoff);
  glUniform1f(GPU_fire_alpha, smoke3di->fire_alpha);

  TransparentOn();
  switch(ssmokedir){
    // +++++++++++++++++++++++++++++++++++ DIR 1 +++++++++++++++++++++++++++++++++++++++


  case 1:
  case -1:

    aspectratio = meshi->dx;
    glUniform1f(GPU_aspectratio, aspectratio);

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = is1;
    slice_end = is2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      i = iii;
      if(ssmokedir<0)i = is1+is2-iii-1;
      iterm = (i-smoke3di->is1);

      if(smokecullflag==1){
        x11[0] = xplt[i];
        x12[0] = xplt[i];
        x22[0] = xplt[i];
        x21[0] = xplt[i];

        x11[1] = yplt[js1];
        x12[1] = yplt[js2];
        x22[1] = yplt[js2];
        x21[1] = yplt[js1];

        x11[2] = zplt[ks1];
        x12[2] = zplt[ks1];
        x22[2] = zplt[ks2];
        x21[2] = zplt[ks2];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      constval = xplt[i]+0.001;
      for(k = ks1; k<ks2; k++){
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k+1];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;

        if(smokecullflag==1&&k!=ks2){
          x11[2] = zplt[k];
          x12[2] = zplt[k];
          x22[2] = zplt[k+1];
          x21[2] = zplt[k+1];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }
        for(j = js1; j<js2; j++){
          jterm = (j-js1)*nx;
          yy1 = yplt[j];
          y3 = yplt[j+1];
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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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

    aspectratio = meshi->dy;
    glUniform1f(GPU_aspectratio, aspectratio);


    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = js1;
    slice_end = js2;
    for(jjj = slice_beg;jjj<slice_end;jjj += skip_local){
      j = jjj;
      if(ssmokedir<0)j = js1+js2-jjj-1;
      constval = yplt[j]+0.001;
      jterm = (j-js1)*nx;

      if(smokecullflag==1){
        x11[0] = xplt[is1];
        x12[0] = xplt[is2];
        x22[0] = xplt[is2];
        x21[0] = xplt[is1];

        x11[1] = yplt[j];
        x12[1] = yplt[j];
        x22[1] = yplt[j];
        x21[1] = yplt[j];

        x11[2] = zplt[ks1];
        x12[2] = zplt[ks1];
        x22[2] = zplt[ks2];
        x21[2] = zplt[ks2];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(k = ks1; k<ks2; k++){
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k+1];

        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;

        if(smokecullflag==1){
          x11[2] = zplt[k];
          x12[2] = zplt[k];
          x22[2] = zplt[k+1];
          x21[2] = zplt[k+1];
          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(i = is1; i<is2; i++){
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i+1];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          n = iterm+jterm+kterm;
          n11 = n;            //n
          n12 = n11+1;;       //n+1
          n22 = n12+nxy;      //n+1+nxy
          n21 = n22-1;        //n+nxy

                              //        n11 = (i-is1)   + (j-js1)*nx   + (k-ks1)*nx*ny;
                              //        n12 = (i+1-is1) + (j-js1)*nx   + (k-ks1)*nx*ny;
                              //        n22 = (i+1-is1) + (j-js1)*nx   + (k+1-ks1)*nx*ny;
                              //        n21 = (i-is1)   + (j-js1)*nx   + (k+1-ks1)*nx*ny;

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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

    aspectratio = meshi->dz;
    glUniform1f(GPU_aspectratio, aspectratio);


    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = ks1;
    slice_end = ks2;
    for(kkk = slice_beg;kkk<slice_end;kkk += skip_local){
      k = kkk;
      if(ssmokedir<0)k = ks1+ks2-kkk-1;
      constval = zplt[k]+0.001;
      kterm = (k-ks1)*nxy;

      if(smokecullflag==1){
        x11[0] = xplt[is1];
        x12[0] = xplt[is2];
        x22[0] = xplt[is2];
        x21[0] = xplt[is1];

        x11[1] = yplt[js1];
        x12[1] = yplt[js1];
        x22[1] = yplt[js2];
        x21[1] = yplt[js2];

        x11[2] = zplt[k];
        x12[2] = zplt[k];
        x22[2] = zplt[k];
        x21[2] = zplt[k];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(j = js1; j<js2; j++){
        jterm = (j-js1)*nx;

        yy1 = yplt[j];
        y3 = yplt[j+1];

        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;

        if(smokecullflag==1){
          x11[1] = yplt[j];
          x12[1] = yplt[j];
          x22[1] = yplt[j+1];
          x21[1] = yplt[j+1];
          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(i = is1; i<is2; i++){
          iterm = (i-is1);
          x1 = xplt[i];
          x3 = xplt[i+1];

          xnode[0] = x1;
          xnode[1] = x3;
          xnode[2] = x3;
          xnode[3] = x1;

          n = iterm+jterm+kterm;
          n11 = n;
          n12 = n11+1;;
          n22 = n12+nx;
          n21 = n22-1;

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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

    aspectratio = meshi->dxy;
    glUniform1f(GPU_aspectratio, aspectratio);

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = nx+ny-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1+ibeg];
        x12[0] = xplt[is1+iend];
        x22[0] = xplt[is1+iend];
        x21[0] = xplt[is1+ibeg];

        x11[1] = yplt[js1+jbeg];
        x12[1] = yplt[js1+jend];
        x22[1] = yplt[js1+jend];
        x21[1] = yplt[js1+jbeg];

        x11[2] = zplt[ks1];
        x12[2] = zplt[ks1];
        x22[2] = zplt[ks2];
        x21[2] = zplt[ks2];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(k = ks1; k<ks2; k++){
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k+1];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;

        if(smokecullflag==1){
          x11[2] = zplt[k];
          x12[2] = zplt[k];
          x22[2] = zplt[k+1];
          x21[2] = zplt[k+1];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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

    aspectratio = meshi->dxy;
    glUniform1f(GPU_aspectratio, aspectratio);


    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = nx+ny-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1+ibeg];
        x12[0] = xplt[is1+iend];
        x22[0] = xplt[is1+iend];
        x21[0] = xplt[is1+ibeg];

        x11[1] = yplt[js1+jbeg];
        x12[1] = yplt[js1+jend];
        x22[1] = yplt[js1+jend];
        x21[1] = yplt[js1+jbeg];

        x11[2] = zplt[ks1];
        x12[2] = zplt[ks1];
        x22[2] = zplt[ks2];
        x21[2] = zplt[ks2];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(k = ks1; k<ks2; k++){
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k+1];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;


        if(smokecullflag==1){
          x11[2] = z1;
          x12[2] = z1;
          x22[2] = z3;
          x21[2] = z3;

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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

    aspectratio = meshi->dyz;
    glUniform1f(GPU_aspectratio, aspectratio);


    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = ny+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1];
        x12[0] = xplt[is1];
        x22[0] = xplt[is2];
        x21[0] = xplt[is2];

        x11[1] = yplt[js1+jbeg];
        x12[1] = yplt[js1+jend];
        x22[1] = yplt[js1+jend];
        x21[1] = yplt[js1+jbeg];

        x11[2] = zplt[ks1+kbeg];
        x12[2] = zplt[ks1+kend];
        x22[2] = zplt[ks1+kend];
        x21[2] = zplt[ks1+kbeg];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(i = is1; i<is2; i++){
        iterm = (i-is1);
        x1 = xplt[i];
        x3 = xplt[i+1];
        xnode[0] = x1;
        xnode[1] = x1;
        xnode[2] = x3;
        xnode[3] = x3;

        if(smokecullflag==1){
          x11[0] = xplt[i];
          x12[0] = xplt[i];
          x22[0] = xplt[i+1];
          x21[0] = xplt[i+1];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
    aspectratio = meshi->dyz;
    glUniform1f(GPU_aspectratio, aspectratio);


    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = ny+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      kmj = iii;
      if(ssmokedir<0)kmi = ny+nz-2-iii;

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

      if(smokecullflag==1){
        x11[0] = xplt[is1];
        x12[0] = xplt[is1];
        x22[0] = xplt[is2];
        x21[0] = xplt[is2];

        x11[1] = yplt[js1+jbeg];
        x12[1] = yplt[js1+jend];
        x22[1] = yplt[js1+jend];
        x21[1] = yplt[js1+jbeg];

        x11[2] = zplt[ks1+kbeg];
        x12[2] = zplt[ks1+kend];
        x22[2] = zplt[ks1+kend];
        x21[2] = zplt[ks1+kbeg];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(i = is1; i<is2; i++){
        iterm = (i-is1);
        x1 = xplt[i];
        x3 = xplt[i+1];
        xnode[0] = x1;
        xnode[1] = x1;
        xnode[2] = x3;
        xnode[3] = x3;


        if(smokecullflag==1){
          x11[0] = x1;
          x12[0] = x1;
          x22[0] = x3;
          x21[0] = x3;

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
    aspectratio = meshi->dxz;
    glUniform1f(GPU_aspectratio, aspectratio);

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = nx+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1+ibeg];
        x12[0] = xplt[is1+iend];
        x22[0] = xplt[is1+iend];
        x21[0] = xplt[is1+ibeg];

        x11[1] = yplt[js1];
        x12[1] = yplt[js1];
        x22[1] = yplt[js2];
        x21[1] = yplt[js2];

        x11[2] = zplt[ks1+kbeg];
        x12[2] = zplt[ks1+kend];
        x22[2] = zplt[ks1+kend];
        x21[2] = zplt[ks1+kbeg];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(j = js1; j<js2; j++){
        jterm = (j-js1)*nx;
        yy1 = yplt[j];
        y3 = yplt[j+1];
        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;

        if(smokecullflag==1){
          x11[1] = yplt[j];
          x12[1] = yplt[j];
          x22[1] = yplt[j+1];
          x21[1] = yplt[j+1];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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

    /* interchange y and z, j and z */
  case 9:
  case -9:
    aspectratio = meshi->dxz;
    glUniform1f(GPU_aspectratio, aspectratio);

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = nx+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1+ibeg];
        x12[0] = xplt[is1+iend];
        x22[0] = xplt[is1+iend];
        x21[0] = xplt[is1+ibeg];

        x11[1] = yplt[js1];
        x12[1] = yplt[js1];
        x22[1] = yplt[js2];
        x21[1] = yplt[js2];

        x11[2] = zplt[ks1+kbeg];
        x12[2] = zplt[ks1+kend];
        x22[2] = zplt[ks1+kend];
        x21[2] = zplt[ks1+kbeg];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(j = js1; j<js2; j++){
        jterm = (j-js1)*nx;
        yy1 = yplt[j];
        y3 = yplt[j+1];
        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;


        if(smokecullflag==1){
          x11[1] = yy1;
          x12[1] = yy1;
          x22[1] = y3;
          x21[1] = y3;

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
    ASSERT(FFALSE);
    break;
  }
  TransparentOff();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
}

#endif

#ifdef pp_GPUSMOKE

/* --------------------------  vertpdata ------------------------------------ */

typedef struct _vertpdata{
  int in_poly,in_tri, index;
  float *xyz, dist, xy2[2], norm2[2];
} vertpdata;

#define POLY_OUTSIDE 0
#define POLY_INSIDE  1
#define POLY_ONEDGE  2
#define POLY_ONNODE  3
#define POLY_EPS 0.00001

/* ------------------ PointOnPolygon ------------------------ */

int PointOnPolygon(vertpdata *vertpinfo, int nvertpinfo, float *xy2){
  int i;
  float minsum;
  int mini;

  for(i = 0;i<nvertpinfo;i++){
    vertpdata *vertpi;
    float dxy[2], *norm2, ddot2;

    vertpi = vertpinfo+i;
    dxy[0] = xy2[0]-vertpi->xy2[0];
    dxy[1] = xy2[1]-vertpi->xy2[1];
    norm2 = vertpi->norm2;
    ddot2 = norm2[0]*dxy[0]+norm2[1]*dxy[1];
    if(ddot2>POLY_EPS){
      float dv[2], t, denom;
      int ip1;
      vertpdata *vertpip1;

      ip1 = i+1;
      if(i==nvertpinfo-1)ip1 = 0;
      vertpip1 = vertpinfo+ip1;
      dv[0] = vertpip1->xy2[0]-vertpi->xy2[0];
      dv[1] = vertpip1->xy2[1]-vertpi->xy2[1];
      denom = dv[0]*dv[0]+dv[1]*dv[1];
      if(denom==0.0)continue;
      t = (dxy[0]*dv[0]+dxy[1]*dv[1])/denom;
      if(t<0.0||t>1.0)continue;
      if(t==1.0){
        xy2[0] = vertpip1->xy2[0];
        xy2[1] = vertpip1->xy2[1];
        return POLY_ONNODE;
      }
      xy2[0] = vertpi->xy2[0]+t*dv[0];
      xy2[1] = vertpi->xy2[1]+t*dv[1];
      return POLY_ONEDGE;
    }
  }
  mini = -1;
  for(i = 0;i<nvertpinfo;i++){
    vertpdata *vertpi;
    float dxy[2], *norm2, ddot2;

    vertpi = vertpinfo+i;
    dxy[0] = xy2[0]-vertpi->xy2[0];
    dxy[1] = xy2[1]-vertpi->xy2[1];
    norm2 = vertpi->norm2;
    ddot2 = norm2[0]*dxy[0]+norm2[1]*dxy[1];
    if(ddot2>POLY_EPS){
      float sum;

      sum = dxy[0]*dxy[0]+dxy[1]*dxy[1];
      if(mini==-1){
        mini = i;
        minsum = sum;
      }
      else{
        if(sum<minsum){
          mini = i;
          minsum = sum;
        }
      }
    }
  }
  if(mini>=0){
    vertpdata *vertpi;

    vertpi = vertpinfo+mini;
    xy2[0] = vertpi->xy2[0];
    xy2[1] = vertpi->xy2[1];
    return POLY_ONNODE;
  }
  return -1;
}

/* ------------------ PointInPolygon ------------------------ */

int PointInPolygon(vertpdata *vertpinfo, int nvertpinfo, float *xy2){
  int i;

  for(i = 0;i < nvertpinfo;i++){
    vertpdata *vertpi;
    float xy[2], *norm2, ddot2;

    vertpi = vertpinfo + i;
    xy[0] = xy2[0]-vertpi->xy2[0];
    xy[1] = xy2[1]-vertpi->xy2[1];
    norm2 = vertpi->norm2;
    ddot2 = norm2[0]*xy[0]+norm2[1]*xy[1];
    if(ddot2>POLY_EPS)return POLY_OUTSIDE;
  }
  return POLY_INSIDE;
}

/* ------------------ PolyTriangulate ------------------------ */

void PolyTriangulate(int flag, float *verts_in, int nverts_in, int *poly, int npoly, float del,
                     float *verts_out, int *nverts_out, 
                     int *tris_out, int *ntris_out){

  vertpdata *vertpinfo=NULL, *vert2pinfo=NULL;
  float dx, dy, dz;
  int i;
  float maxdist, maxdistx, maxdisty;
  float mindistx, mindisty;
  int maxi, maxip1, maxip2;
  float xvec[3], yvec[3], ycrossx[3];
  float xdelvec[3], ydelvec[3];
  float xyz0[3], *xyzi, *xyzip1, *xyzip2;
  int nrows, ncols;
  int nverts, ntris;
  int nverts_allocated, ntris_allocated;

  *nverts_out = 0;
  *ntris_out = 0;
  if(nverts_in == 0 || npoly == 0)return;

  NewMemory((void **)&vertpinfo, npoly * sizeof(vertpdata));

  // determine distance of each edge and longest edge

  for(i = 0;i < npoly;i++){
    float *verti, *vertip1;
    vertpdata *vertpi;
    int ip1;

    vertpi = vertpinfo + i;
    verti   = verts_in + 3*poly[i];
    vertpi->xyz = verti;

    ip1 = i + 1;
    if(i == npoly - 1)ip1 = 0;
    vertip1 = verts_in + 3*poly[ip1];

    DDIST3(verti, vertip1, vertpi->dist);
    if(i==0){
      maxdist = vertpi->dist;
      maxi = 0;
    }
    else{
      if(vertpi->dist>maxdist){
        maxdist = vertpi->dist;
        maxi = i;
      }
    }
  }

  // find x and y axis of plane containing polygon

  maxip1 = maxi+1;
  if(maxi == npoly - 1)maxip1 = 0;
  maxip2 = maxip1 + 1;
  if(maxip1 == npoly - 1)maxip2 = 0;

  xyzi   = vertpinfo[maxi].xyz;
  xyzip1 = vertpinfo[maxip1].xyz;
  xyz0[0] = xyzip1[0];                  // temporary origin
  xyz0[1] = xyzip1[1];
  xyz0[2] = xyzip1[2];
  xyzip2 = vertpinfo[maxip2].xyz;
  VEC3DIFF(xvec, xyzi, xyz0);     // xyzi - xyzip1
  NORMALIZE3(xvec);
  xdelvec[0] = del*xvec[0];
  xdelvec[1] = del*xvec[1];
  xdelvec[2] = del*xvec[2];

  VEC3DIFF(yvec, xyzip2, xyz0);   // xyzip2 - xyzip1
  NORMALIZE3(yvec);

  CROSS(ycrossx, yvec, xvec);     // yvec cross xvec
  CROSS(yvec, ycrossx, xvec);     // (yvec cross xvec ) cross xvec
  NORMALIZE3(yvec);
  ydelvec[0] = del*yvec[0];
  ydelvec[1] = del*yvec[1];
  ydelvec[2] = del*yvec[2];

  // shift origin so all poly vertices have positive coordinates (in the coordinate system defined by xvec,yvec)

  for(i = 0; i<npoly; i++){
    vertpdata *vertpi;
    float distx, disty;

    vertpi = vertpinfo+i;
    distx = PLANEDIST(xvec, xyz0, vertpi->xyz);
    disty = PLANEDIST(yvec, xyz0, vertpi->xyz);
    if(i==0){
      mindistx = distx;
      mindisty = disty;
    }
    else{
      mindistx = MIN(distx, mindistx);
      mindisty = MIN(disty, mindisty);
    }
  }
  mindistx -= 1.5*del;
  mindisty -= 1.5*del;
  xyz0[0] += (mindistx*xvec[0]+mindisty*yvec[0]);
  xyz0[1] += (mindistx*xvec[1]+mindisty*yvec[1]);
  xyz0[2] += (mindistx*xvec[2]+mindisty*yvec[2]);

  // set 2d coordinates of polygon vertices

  for(i = 0;i < npoly;i++){
    vertpdata *vertpi;
    float *xy2, distx, disty;

    vertpi = vertpinfo + i;
    distx = PLANEDIST(xvec, xyz0, vertpi->xyz);
    disty = PLANEDIST(yvec, xyz0, vertpi->xyz);
    xy2 = vertpi->xy2;
    xy2[0] = distx;
    xy2[1] = disty;
    if(i == 0){
      maxdistx = distx;
      maxdisty = disty;
    }
    else{
      maxdistx = MAX(maxdistx, distx);
      maxdisty = MAX(maxdisty, disty);
    }
  }
  maxdistx += 1.5*del;
  maxdisty += 1.5*del;

  // get 2d normals

  for(i = 0;i < npoly;i++){
    vertpdata *vertpi, *vertpip1;
    float dx, dy, norm;
    int ip1;

    ip1 = i + 1;
    if(i == npoly - 1)ip1 = 0;

    vertpi   = vertpinfo + i;
    vertpip1 = vertpinfo + ip1;
    dx = vertpip1->xy2[0] - vertpi->xy2[0];
    dy = vertpip1->xy2[1] - vertpi->xy2[1];
    norm = sqrt(dx*dx+dy*dy);
    if(norm>0.0){
      dx /= norm;
      dy /= norm;
    }
    vertpi->norm2[0] = dy;
    vertpi->norm2[1] = -dx;
  }

  nrows = MAX(1,maxdisty / del+0.5)+1;
  ncols = MAX(1,maxdistx / del+0.5)+1;

  if(flag==1){
    int ncells, nnodes;

    FREEMEMORY(vertpinfo);
    ncells = (nrows-1)*(ncols-1);
    nnodes = nrows*ncols;
    *ntris_out = 2*ncells;
    *nverts_out = nnodes;
    return;
  }

  NewMemory((void **)&vert2pinfo, nrows*ncols*sizeof(vertpdata));

  // set 2d coordinates of triangles vertices

  for(i = 0;i < nrows;i++){
    int j;
    float t;

    t = (float)i*del;
    for(j = 0;j < ncols;j++){
      float s;
      vertpdata *vertpij;

      s = (float)j*del;
      vertpij = vert2pinfo + i*ncols + j;
      vertpij->xy2[0] = s;
      vertpij->xy2[1] = t;
      vertpij->in_poly = PointInPolygon(vertpinfo, npoly, vertpij->xy2);
      vertpij->in_tri = 0;
    }
  }

  // move points next to points in the polygon to the polygon

  for(i = 0;i<nrows;i++){
    int j;

    for(j = 0;j<ncols;j++){
      vertpdata *p11;

      p11 = vert2pinfo+i*ncols+j;
      if(p11->in_poly==POLY_OUTSIDE){
        int ii, doit;

   // examine 8 points surrounding p11, if any are inside the polygon move p11 onto the polygon

        doit = 0;
        for(ii=i-1;ii<i+2;ii++){
          int jj;

          if(ii<0||ii>=nrows)continue;
          for(jj=j-1;jj<j+2;jj++){
            vertpdata *pij;

            if(jj<0||jj>=ncols)continue;
            if(ii==i&&jj==j)continue;
            pij = vert2pinfo + ii*ncols + jj;
            if(pij->in_poly!=POLY_OUTSIDE){
              doit=1;
              break;
            }
          }
          if(doit==1)break;
        }
        if(doit==1){
          int in_poly;

          in_poly = PointOnPolygon(vertpinfo, npoly, p11->xy2);
          if(in_poly>=0)p11->in_poly = in_poly;
        }
      }
    }
  }

  // count triangles

  ntris = 0;
  for(i = 0;i < nrows-1;i++){
    int j;

    for(j = 0;j < ncols-1;j++){
      vertpdata *vert11,*vert12, *vert21, *vert22;
      int npoints;

      npoints = 0;
      vert11 = vert2pinfo + i*ncols + j;
      vert12 = vert2pinfo + i*ncols + j+1;
      vert21 = vert2pinfo + (i+1)*ncols + j;
      vert22 = vert2pinfo + (i+1)*ncols + j+1;
      if(vert11->in_poly!=POLY_OUTSIDE)npoints++;
      if(vert12->in_poly!=POLY_OUTSIDE)npoints++;
      if(vert21->in_poly!=POLY_OUTSIDE)npoints++;
      if(vert22->in_poly!=POLY_OUTSIDE)npoints++;
      if(npoints <3)continue;
      if(vert11->in_poly!=POLY_OUTSIDE)vert11->in_tri = 1;
      if(vert12->in_poly!=POLY_OUTSIDE)vert12->in_tri = 1;
      if(vert21->in_poly!=POLY_OUTSIDE)vert21->in_tri = 1;
      if(vert22->in_poly!=POLY_OUTSIDE)vert22->in_tri = 1;
      ntris++;
      if(npoints==4)ntris++;
    }
  }

  if(ntris==0){
    *nverts_out = 0;
    *ntris_out = 0;
    FREEMEMORY(vert2pinfo);
    FREEMEMORY(vertpinfo);
    return;
  }

  // count_verts

  nverts = 0;
  for(i = 0; i<nrows; i++){
    int j;

    for(j = 0; j<ncols; j++){
      vertpdata *vertpij;

      vertpij = vert2pinfo+i*ncols+j;
      vertpij->index = -1;
      if(vertpij->in_poly!=POLY_OUTSIDE&&vertpij->in_tri==1){
        vertpij->index = nverts++;
      }
    }
  }

  if(nverts==0){
    *nverts_out = 0;
    FREEMEMORY(vert2pinfo);
    FREEMEMORY(vertpinfo);
    return;
  }

  nverts_allocated = nverts;
  ntris_allocated = ntris;

// define output vertex array

  nverts = 0;
  for(i = 0; i<nrows; i++){
    int j;

    for(j = 0; j<ncols; j++){
      vertpdata *vertpij;
      float di, dj;

      vertpij = vert2pinfo+i*ncols+j;
      di = vertpij->xy2[0]/del;
      dj = vertpij->xy2[1]/del;
      if(vertpij->in_poly!=POLY_OUTSIDE&&vertpij->in_tri==1){
        verts_out[3*nverts+0] = xyz0[0]+(float)dj*ydelvec[0]+(float)di*xdelvec[0];
        verts_out[3*nverts+1] = xyz0[1]+(float)dj*ydelvec[1]+(float)di*xdelvec[1];
        verts_out[3*nverts+2] = xyz0[2]+(float)dj*ydelvec[2]+(float)di*xdelvec[2];
        nverts++;
      }
    }
  }
  ASSERT(nverts==nverts_allocated);
  CheckMemory;
  
// define output triangle array

  ntris = 0;
  for(i = 0; i<nrows-1; i++){
    int j;

    for(j = 0; j<ncols-1; j++){
      vertpdata *vert11, *vert12, *vert21, *vert22;
      int i11, i12, i21, i22;
      int npoints;

      npoints = 0;
      vert11 = vert2pinfo+i*ncols+j;
      vert12 = vert2pinfo+i*ncols+j+1;
      vert21 = vert2pinfo+(i+1)*ncols+j;
      vert22 = vert2pinfo+(i+1)*ncols+j+1;
      if(vert11->in_poly!=POLY_OUTSIDE)npoints++;
      if(vert12->in_poly!=POLY_OUTSIDE)npoints++;
      if(vert21->in_poly!=POLY_OUTSIDE)npoints++;
      if(vert22->in_poly!=POLY_OUTSIDE)npoints++;
      if(npoints<3)continue;
      i11 = vert11->index;
      i21 = vert21->index;
      i12 = vert12->index;
      i22 = vert22->index;

      if(npoints==4){

        ASSERT(i11>=0&&i12>=0&&i21>=0&&i22>=0);
        tris_out[3*ntris+0] = i11;
        tris_out[3*ntris+1] = i12;
        tris_out[3*ntris+2] = i22;
        ntris++;

        tris_out[3*ntris+0] = i11;
        tris_out[3*ntris+1] = i22;
        tris_out[3*ntris+2] = i21;
        ntris++;
      }
      else{
        if(vert11->in_poly==POLY_OUTSIDE){
          ASSERT(i12>=0&&i21>=0&&i22>=0);
          tris_out[3*ntris+0] = i12;
          tris_out[3*ntris+1] = i22;
          tris_out[3*ntris+2] = i21;
        }
        else if(vert12->in_poly==POLY_OUTSIDE){
          ASSERT(i11>=0&&i21>=0&&i22>=0);
          tris_out[3*ntris+0] = i11;
          tris_out[3*ntris+1] = i22;
          tris_out[3*ntris+2] = i21;
        }
        else if(vert21->in_poly==POLY_OUTSIDE){
          ASSERT(i11>=0&&i12>=0&&i22>=0);
          tris_out[3*ntris+0] = i11;
          tris_out[3*ntris+1] = i12;
          tris_out[3*ntris+2] = i22;
        }
        else if(vert22->in_poly==POLY_OUTSIDE){
          ASSERT(i11>=0&&i12>=0&&i21>=0);
          tris_out[3*ntris+0] = i11;
          tris_out[3*ntris+1] = i12;
          tris_out[3*ntris+2] = i21;
        }
        ntris++;
      }
    }
  }
  ASSERT(ntris==ntris_allocated);
  CheckMemory;
  *nverts_out = nverts;
  *ntris_out = ntris;
  FREEMEMORY(vert2pinfo);
  FREEMEMORY(vertpinfo);
}

/* ------------------ FreeSmokeMemory ------------------------ */

void FreeSmokeMemory(meshdata *meshi){
  FREEMEMORY(meshi->smoke_verts);
  FREEMEMORY(meshi->smoke_tris);
  FREEMEMORY(meshi->smoke_vals);
  meshi->max_verts = 0;
  meshi->max_tris = 0;
}

/* ------------------ AllocateSmokeMemory ------------------------ */

void AllocateSmokeMemory(meshdata *meshi, int nverts, int ntris){
  if(nverts>meshi->max_verts){
    FREEMEMORY(meshi->smoke_verts);
    FREEMEMORY(meshi->smoke_vals);
    NewMemory((void **)&meshi->smoke_verts, 3*nverts*sizeof(float));
    NewMemory((void **)&meshi->smoke_vals,  3*nverts*sizeof(float));
    meshi->max_verts = nverts;
  }
  if(ntris>meshi->max_tris){
    FREEMEMORY(meshi->smoke_tris);
    NewMemory((void **)&meshi->smoke_tris, 3*ntris*sizeof(int));
    meshi->max_tris = ntris;
  }
}

/* ------------------ UpdateSmoke3DPlanes ------------------------ */

void UpdateSmoke3DPlanes(float delta_perp, float delta_par){
  int i;
  float *xyz0, *norm;
  float d, distmin, distmax;
  float norm_align[3];

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
  int ix[8] = {0, 0, 1, 1, 0, 0, 1, 1};
  int iy[8] = {0, 1, 1, 0, 0, 1, 1, 0};
  int iz[8] = {0, 0, 0, 0, 1, 1, 1, 1};

  // plane equation: (x-xyz0) .dot. norm = 0

  xyz0 = fds_eyepos;
  norm = fds_viewdir;

  distmin = -1.0;
  distmax = -1.0;
  for(i = 0; i<nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    meshdata *meshi;
    float xx[2], yy[2], zz[2];
    float *boxmin, *boxmax;
    float *verts, *dist;
    int j;

    smoke3di = smoke3dinfo + i;
    if(smoke3di->loaded==0||smoke3di->display==0)continue;
    if(smoke3di->primary_file==0)continue;
    if(IsSmokeComponentPresent(smoke3di)==0)continue;

    meshi = meshinfo + smoke3di->blocknumber;


    meshi->nsmokeplaneinfo = 0;
    if(smoke_mesh_aligned == 1){
      norm_align[0] = -meshi->norm[0];
      norm_align[1] = -meshi->norm[1];
      norm_align[2] = -meshi->norm[2];
      norm = norm_align;
    }
    if(use_smokebox==1){
      boxmin = smoke3di->smoke_boxmin + 3*smoke3di->ismoke3d_time;
      boxmax = smoke3di->smoke_boxmax + 3*smoke3di->ismoke3d_time;
    }
    else{
      boxmin = meshi->boxmin;
      boxmax = meshi->boxmax;
    }
    verts = meshi->verts;
    dist = meshi->vert_dists;
    xx[0] = boxmin[0];
    yy[0] = boxmin[1];
    zz[0] = boxmin[2];
    xx[1] = boxmax[0];
    yy[1] = boxmax[1];
    zz[1] = boxmax[2];
    if(meshi->nverts == 0){
      int j;

      meshi->nverts = 8;
      for(j = 0; j<8; j++){
        verts[3*j + 0] = xx[ix[j]];
        verts[3*j + 1] = yy[iy[j]];
        verts[3*j + 2] = zz[iz[j]];
      }
    }
    meshi->vert_distmin = -1.0;
    meshi->vert_distmax = -1.0;
    for(j = 0; j<8; j++){
      float *xyz, distxyz, xyzrel[3];

      xyz = meshi->verts + 3*j;
      VEC3DIFF(xyzrel, xyz, xyz0);
      distxyz = PLANEDIST(norm, xyz0, xyz);
      if(smoke_mesh_aligned==1){
        dist[j] = distxyz;
      }
      else{
        dist[j] = SIGN(distxyz)*NORM3(xyzrel);
      }
      if(dist[j]>0.0){
        meshi->vert_distmax = MAX(meshi->vert_distmax,dist[j]);
        if(meshi->vert_distmin>0.0){
          meshi->vert_distmin = MIN(meshi->vert_distmin,dist[j]);
        }
        else{
          meshi->vert_distmin = dist[j];
        }
        distmax = MAX(distmax,dist[j]);
        if(distmin>0.0){
          distmin = MIN(distmin,dist[j]);
        }
        else{
          distmin = dist[j];
        }
      }
    }
  }
  if(distmin<0.0){
    return;
  }
  UpdateGluiPlanes(distmin, distmax);
  if(plane_labels==1){
    for(i = 0;i < nmeshes;i++){
      meshdata *meshi;
      float *dist, *verts;
      int j;

      meshi = meshinfo + i;
      if(IsSmokeInMesh(meshi)==0)continue;

      dist = meshi->vert_dists;
      verts = meshi->verts;
      for(j = 0;j < 8;j++){
        Output3Val(NORMALIZE_X(verts[3 * j]), NORMALIZE_Y(verts[3 * j + 1]), NORMALIZE_Z(verts[3 * j + 2]), dist[j]);
      }
    }
  }
  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo + i;

    FREEMEMORY(meshi->smokeplaneinfo);
    meshi->nsmokeplaneinfo = 0;
  }
  for(d = distmax - delta_perp/2.0; d>distmin; d -= delta_perp){
    if(plane_single == 1)d = plane_distance;
    for(i = 0; i<nmeshes; i++){
      meshdata *meshi;

      meshi = meshinfo + i;
      if(d>meshi->vert_distmin&&d<meshi->vert_distmax)meshi->nsmokeplaneinfo++;
    }
    if(plane_single == 1)break;
  }
  for(i = 0; i<nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    if(meshi->nsmokeplaneinfo>0){
      NewMemory((void **)&meshi->smokeplaneinfo, meshi->nsmokeplaneinfo * sizeof(meshplanedata));
    }
  }
  for(i = 0; i<nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    meshdata *meshi;
    float xx[2], yy[2], zz[2];
    float *boxmin, *boxmax;
    int jj;
    float *xyz_orig;

    smoke3di = smoke3dinfo + i;
    if(smoke3di->loaded==0||smoke3di->display==0)continue;
    if(smoke3di->primary_file==0)continue;
    if(IsSmokeComponentPresent(smoke3di)==0)continue;

    meshi = meshinfo + smoke3di->blocknumber;
    
    if(smoke_exact_dist==1){
      xyz_orig = xyz0;
    }
    else{
      xyz_orig = NULL;
    }
    if(use_smokebox==1){
      boxmin = smoke3di->smoke_boxmin + 3*smoke3di->ismoke3d_time;
      boxmax = smoke3di->smoke_boxmax + 3*smoke3di->ismoke3d_time;
    }
    else{
      boxmin = meshi->boxmin;
      boxmax = meshi->boxmax;
    }

    xx[0] = boxmin[0];
    yy[0] = boxmin[1];
    zz[0] = boxmin[2];
    xx[1] = boxmax[0];
    yy[1] = boxmax[1];
    zz[1] = boxmax[2];
    jj = 0;
    for(d = distmax - delta_perp/2.0; d > distmin; d -= delta_perp){
      if(plane_single == 1)d = plane_distance;
      if(d>meshi->vert_distmin&&d<meshi->vert_distmax){
        meshplanedata *spi;
        int k;

        if(jj >= meshi->nsmokeplaneinfo)break;
        spi = meshi->smokeplaneinfo + jj;
        GetIsoBox(xx, yy, zz, xyz_orig, meshi->vert_dists, d, spi->verts, &(spi->nverts), spi->triangles, &(spi->ntriangles),spi->polys,&spi->npolys);
        spi->ntriangles /= 3;
        for(k = 0; k<spi->nverts; k++){
          NORMALIZE_XYZ(spi->verts_smv + 3 * k, spi->verts + 3 * k);
        }

        // compute normals

        for(k = 0;k < spi->ntriangles;k++){
          float *norm0, *norm1, *v, *v1, *v2, *v3;
          float vec1[3], vec2[3];

          norm0 = spi->norm0+3*k;
          norm1 = spi->norm1+3*k;
          v = spi->verts_smv;
          v1 = v + 3*spi->triangles[3*k + 0];
          v2 = v + 3*spi->triangles[3*k + 1];
          v3 = v + 3*spi->triangles[3*k + 2];
          norm0[0] = (v1[0] + v2[0] + v3[0])/3.0;
          norm0[1] = (v1[1] + v2[1] + v3[1])/3.0;
          norm0[2] = (v1[2] + v2[2] + v3[2])/3.0;
          VEC3DIFF(vec1, v2, v1);
          VEC3DIFF(vec2, v3, v1);
          CROSS(norm1, vec1, vec2);
          NORMALIZE3(norm1);
          norm1[0]+=norm0[0];
          norm1[1]+=norm0[1];
          norm1[2]+=norm0[2];
        }

        jj++;
      }
      if(plane_single == 1)break;
    }
  }
  smoke_function_count = 0;
  if(usegpu==0&&smoke_outline_type==SMOKE_TRIANGULATION){
    for(i = 0; i<nsmoke3dinfo; i++){
      smoke3ddata *smoke3di;
      meshdata *meshi;
      int j;
      int nsmoketypes;
      int have_vals[3];
      int total_verts, total_tris;
      float delta_pari;

      smoke3di = smoke3dinfo + i;
      if(smoke3di->loaded==0||smoke3di->display==0)continue;
      if(smoke3di->primary_file==0)continue;
      if(IsSmokeComponentPresent(smoke3di)==0)continue;

      meshi = meshinfo + smoke3di->blocknumber;
      delta_pari = MAX(delta_par,meshi->xplt_orig[1]-meshi->xplt_orig[0]);
      nsmoketypes = GetNSmokeTypes(smoke3di,have_vals);

      total_verts = 0;
      total_tris = 0;
      for(j = 0; j<meshi->nsmokeplaneinfo; j++){
        meshplanedata *spi;
        int nverts2, ntris2;

        spi = meshi->smokeplaneinfo+j;
        PolyTriangulate(1, spi->verts, spi->nverts, spi->polys, spi->npolys, delta_pari, NULL, &nverts2, NULL, &ntris2);
        total_verts += nverts2;
        total_tris += ntris2;
      }
      if(total_verts>meshi->max_verts||total_tris>meshi->max_tris){
        AllocateSmokeMemory(meshi, total_verts, total_tris);
      }

      for(j = 0; j<meshi->nsmokeplaneinfo; j++){
        meshplanedata *spi;

        spi = meshi->smokeplaneinfo+j;
        if(j==0){
          spi->verts2 = meshi->smoke_verts;
          spi->tris2 = meshi->smoke_tris;
          spi->vals2 = meshi->smoke_vals;
        }
        else{
          meshplanedata *spim1;

          spim1 = meshi->smokeplaneinfo+j-1;
          spi->verts2 = spim1->verts2+3*spim1->nverts2;
          spi->tris2 = spim1->tris2+3*spim1->ntris2;
          spi->vals2 = spim1->vals2+nsmoketypes*spim1->nverts2;
        }
        PolyTriangulate(0,spi->verts, spi->nverts, spi->polys, spi->npolys, delta_pari, spi->verts2, &spi->nverts2, spi->tris2, &spi->ntris2);

        if(smoke_getvals==1&&spi->nverts2>0&&nsmoketypes>0){
          int k;
          float *valsptr;
           
          valsptr = spi->vals2;
          for(k=0;k<spi->nverts2;k++){
            float *xyz, vals[3];

            xyz = spi->verts2+3*k;
            GetSmoke3DVals(xyz, smoke3di, vals, have_vals);
            if(have_vals[0]==1)*valsptr++ = vals[0];
            if(have_vals[1]==1)*valsptr++ = vals[1];
            if(have_vals[2]==1)*valsptr++ = vals[2];
          }
          spi->drawsmoke = 1;
        }
        else{
          spi->drawsmoke = 0;
        }
      }
    }
  }
}

/* ------------------ DrawSmokeDiag ------------------------ */

void DrawSmokeDiag(smoke3ddata *smoke3di){
  int i;
  meshdata *meshi;

  meshi = meshinfo+smoke3di->blocknumber;

  if(smoke_outline_type==SMOKE_TRIANGULATION){
    DrawSmoke3DOutline2(smoke3di);
    SNIFF_ERRORS("after DrawQuadSmokeOutline2");
    return;
  }
  if(smoke_outline_type==SMOKE_OUTLINE_TRIANGLE){
    glLineWidth(plane_outline_width);
    glBegin(GL_LINES);
    for(i = 0; i<meshi->nsmokeplaneinfo; i++){
      meshplanedata *spi;
      int j;

      spi = meshi->smokeplaneinfo+i;
      glColor3f(0.0, 0.0, 1.0);
      if(smoke_outline_type==SMOKE_OUTLINE_TRIANGLE){
        for(j = 0; j<spi->ntriangles; j++){
          float *v1, *v2, *v3;
          int i1, i2, i3;

          i1 = spi->triangles[3*j];
          i2 = spi->triangles[3*j+1];
          i3 = spi->triangles[3*j+2];
          v1 = spi->verts_smv+3*i1;
          v2 = spi->verts_smv+3*i2;
          v3 = spi->verts_smv+3*i3;
#ifdef XXXYYY
          printf("DrawSMokePlanes triangle: %i\n", j);
          printf("v1=%f %f %f\n", v1[0], v1[1], v1[0]);
          printf("v2=%f %f %f\n", v2[0], v2[1], v2[0]);
          printf("v3=%f %f %f\n", v3[0], v3[1], v3[0]);
#endif
          glVertex3fv(v1);
          glVertex3fv(v2);
          glVertex3fv(v2);
          glVertex3fv(v3);
          glVertex3fv(v3);
          glVertex3fv(v1);
        }
      }
      if(plane_normal == 1){
        glColor3f(1.0, 0.0, 0.0);
        for(j = 0; j < spi->ntriangles; j++){
          glVertex3fv(spi->norm0 + 3 * j);
          glVertex3fv(spi->norm1 + 3 * j);
        }
      }
    }
    glEnd();
    if(plane_solid==1){
      glColor4f(0.0, 0.0, 1.0, 0.6);
      glBegin(GL_TRIANGLES);
      for(i = 0; i<meshi->nsmokeplaneinfo; i++){
        meshplanedata *spi;
        int j;

        spi = meshi->smokeplaneinfo+i;
        for(j = 0; j<spi->ntriangles; j++){
          float *v1, *v2, *v3;
          int i1, i2, i3;

          i1 = spi->triangles[3*j];
          i2 = spi->triangles[3*j+1];
          i3 = spi->triangles[3*j+2];
          v1 = spi->verts_smv+3*i1;
          v2 = spi->verts_smv+3*i2;
          v3 = spi->verts_smv+3*i3;
          glVertex3fv(v1);
          glVertex3fv(v2);
          glVertex3fv(v3);
        }
      }
      glEnd();
    }
  }
  if(smoke_outline_type==SMOKE_OUTLINE_POLYGON){
    glLineWidth(plane_outline_width);
    glBegin(GL_LINES);
    for(i = 0; i<meshi->nsmokeplaneinfo; i++){
      meshplanedata *spi;
      int j;

      spi = meshi->smokeplaneinfo+i;
      glColor3f(0.0, 0.0, 0.0);
      for(j = 0; j<spi->npolys; j++){
        float *v1, *v2;
        int i1, i2;

        i1 = spi->polys[j];
        if(j==spi->npolys-1){
          i2 = spi->polys[0];
        }
        else{
          i2 = spi->polys[j+1];
        }
        v1 = spi->verts_smv+3*i1;
        v2 = spi->verts_smv+3*i2;
        glVertex3fv(v1);
        glVertex3fv(v2);
      }
    }
    glEnd();
  }
}


#endif

/* ------------------ DrawSmoke3d ------------------------ */

void DrawSmoke3D(smoke3ddata *smoke3di){
  int i, j, k, n;
  float constval, x1, x3, z1, z3, yy1, y3;
  int is1, is2, js1, js2, ks1, ks2;
  int ii, jj, kk;
  int ibeg, iend, jbeg, jend, kbeg, kend;
  float *znode_offset, z_offset[4];

  float *xplt, *yplt, *zplt;
  unsigned char smokealpha, *smokealpha_ptr, *smokecolor_ptr;
  int nx, ny, nz;
  unsigned char *alphaf_in, *alphaf_out, *alphaf_ptr;
  unsigned char *colorptr;
  int xyzindex1[6], xyzindex2[6], *xyzindex, node;
  float xnode[4], znode[4], ynode[4];
  int skip_local;
  int iterm, jterm, kterm, nxy;
  float x11[3], x12[3], x22[3], x21[3];
  unsigned int n11, n12, n22, n21;
  int ipj, jpk, ipk, jmi, kmi, kmj;
  int iii, jjj, kkk;
  int slice_end, slice_beg;
  float aspectratio;
  int ssmokedir;
  unsigned char *iblank_smoke3d;
#ifdef pp_SMOKEDIAG
  float merge_time, draw_time;
#endif

  unsigned char value[4];
  int ivalue[4];

  meshdata *meshi;

  meshi = meshinfo+smoke3di->blocknumber;
  if(meshvisptr[meshi-meshinfo]==0)return;

#ifdef pp_SMOKEDIAG
  START_TIMER(merge_time);
#endif
  if(meshi->smokealpha_ptr==NULL||meshi->merge_alpha==NULL||meshi->update_smoke3dcolors==1){
    meshi->update_smoke3dcolors = 0;
    MergeSmoke3D(smoke3di);
  }
#ifdef pp_SMOKEDIAG
  STOP_TIMER(merge_time);
  smoketime_merge += merge_time;
  START_TIMER(draw_time);
#endif
  smokealpha_ptr = meshi->smokealpha_ptr;
  smokecolor_ptr = meshi->smokecolor_ptr;
  value[0] = 255;
  value[1] = 255;
  value[2] = 255;
  value[3] = 255;

  if(nterraininfo>0){
    znode_offset = meshi->terrain->znode_offset;
  }

  xplt = meshi->xplt;
  yplt = meshi->yplt;
  zplt = meshi->zplt;
  iblank_smoke3d = meshi->iblank_smoke3d;
  alphaf_in = smoke3di->smokeframe_in;
  alphaf_out = smoke3di->smokeframe_out;

  switch(demo_mode){
  case 0:
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

  nx = smoke3di->is2+1-smoke3di->is1;
  ny = js2+1-js1;
  nz = ks2+1-ks1;
  nxy = nx*ny;

  ssmokedir = meshi->smokedir;
  skip_local = smokeskipm1+1;

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

  if(cullfaces==1)glDisable(GL_CULL_FACE);

  TransparentOn();
  switch(ssmokedir){

    // +++++++++++++++++++++++++++++++++++ DIR 1 +++++++++++++++++++++++++++++++++++++++

  case 1:
  case -1:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    if(adjustalphaflag!=0){

      aspectratio = meshi->dx;
      for(i = is1;i<=is2;i++){
        iterm = (i-smoke3di->is1);

        if(smokecullflag==1){
          x11[0] = xplt[i];
          x12[0] = xplt[i];
          x22[0] = xplt[i];
          x21[0] = xplt[i];

          x11[1] = yplt[js1];
          x12[1] = yplt[js2];
          x22[1] = yplt[js2];
          x21[1] = yplt[js1];

          x11[2] = zplt[ks1];
          x12[2] = zplt[ks1];
          x22[2] = zplt[ks2];
          x21[2] = zplt[ks2];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(k = ks1;k<=ks2;k++){
          kterm = (k-ks1)*nxy;

          if(smokecullflag==1&&k!=ks2){
            x11[2] = zplt[k];
            x12[2] = zplt[k];
            x22[2] = zplt[k+1];
            x21[2] = zplt[k+1];
            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(j = js1;j<=js2;j++){
            jterm = (j-js1)*nx;
            //  jterm = (j-js1)*nx;
            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, NULL, 1);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = is1;
    slice_end = is2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      i = iii;
      if(ssmokedir<0)i = is1+is2-iii-1;
      iterm = (i-smoke3di->is1);

      if(smokecullflag==1){
        x11[0] = xplt[i];
        x12[0] = xplt[i];
        x22[0] = xplt[i];
        x21[0] = xplt[i];

        x11[1] = yplt[js1];
        x12[1] = yplt[js2];
        x22[1] = yplt[js2];
        x21[1] = yplt[js1];

        x11[2] = zplt[ks1];
        x12[2] = zplt[ks1];
        x22[2] = zplt[ks2];
        x21[2] = zplt[ks2];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      constval = xplt[i]+0.001;
      for(k = ks1; k<ks2; k+=smoke3d_skip){
        int k2, koffset;

        k2 = MIN(k+smoke3d_skip,ks2);
        koffset = k2 - k;

        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k2];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;

        if(smokecullflag==1&&k!=ks2){
          x11[2] = zplt[k];
          x12[2] = zplt[k];
          x22[2] = zplt[k2];
          x21[2] = zplt[k2];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }
        for(j = js1; j<js2; j+=smoke3d_skip){
          int j2, joffset;

          j2 = MIN(j+smoke3d_skip,js2);
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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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

        }
      }
    }
    glEnd();

    break;

    // +++++++++++++++++++++++++++++++++++ DIR 2 +++++++++++++++++++++++++++++++++++++++

  case 2:
  case -2:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    if(adjustalphaflag!=0){

      aspectratio = meshi->dy;
      for(j = js1;j<=js2;j++){
        jterm = (j-js1)*nx;
        //    xp[1]=yplt[j];

        if(smokecullflag==1){
          x11[0] = xplt[is1];
          x12[0] = xplt[is2];
          x22[0] = xplt[is2];
          x21[0] = xplt[is1];

          x11[1] = yplt[j];
          x12[1] = yplt[j];
          x22[1] = yplt[j];
          x21[1] = yplt[j];

          x11[2] = zplt[ks1];
          x12[2] = zplt[ks1];
          x22[2] = zplt[ks2];
          x21[2] = zplt[ks2];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(k = ks1;k<=ks2;k++){
          kterm = (k-ks1)*nxy;

          if(smokecullflag==1&&k!=ks2){
            x11[2] = zplt[k];
            x12[2] = zplt[k];
            x22[2] = zplt[k+1];
            x21[2] = zplt[k+1];

            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(i = is1;i<=is2;i++){
            iterm = (i-is1);
            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, NULL, 2);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = js1;
    slice_end = js2;
    for(jjj = slice_beg;jjj<slice_end;jjj += skip_local){
      j = jjj;
      if(ssmokedir<0)j = js1+js2-jjj-1;
      constval = yplt[j]+0.001;
      jterm = (j-js1)*nx;

      if(smokecullflag==1){
        x11[0] = xplt[is1];
        x12[0] = xplt[is2];
        x22[0] = xplt[is2];
        x21[0] = xplt[is1];

        x11[1] = yplt[j];
        x12[1] = yplt[j];
        x22[1] = yplt[j];
        x21[1] = yplt[j];

        x11[2] = zplt[ks1];
        x12[2] = zplt[ks1];
        x22[2] = zplt[ks2];
        x21[2] = zplt[ks2];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }
      for(k = ks1; k<ks2; k+=smoke3d_skip){
        int k2, koffset;

        k2 = MIN(k+smoke3d_skip,ks2);
        koffset = k2 - k;
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k2];

        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;

        if(smokecullflag==1){
          x11[2] = zplt[k];
          x12[2] = zplt[k];
          x22[2] = zplt[k2];
          x21[2] = zplt[k2];
          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(i = is1; i<is2; i+=smoke3d_skip){
          int i2,ioffset;

          i2 = MIN(i+smoke3d_skip,is2);
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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 3 +++++++++++++++++++++++++++++++++++++++

  case 3:
  case -3:
    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    aspectratio = meshi->dz;
    if(adjustalphaflag!=0){
      for(k = ks1;k<=ks2;k++){
        kterm = (k-ks1)*nxy;

        if(smokecullflag==1){
          x11[0] = xplt[is1];
          x12[0] = xplt[is2];
          x22[0] = xplt[is2];
          x21[0] = xplt[is1];

          x11[1] = yplt[js1];
          x12[1] = yplt[js1];
          x22[1] = yplt[js2];
          x21[1] = yplt[js2];

          x11[2] = zplt[k];
          x12[2] = zplt[k];
          x22[2] = zplt[k];
          x21[2] = zplt[k];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(j = js1;j<=js2;j++){
          jterm = (j-js1)*nx;

          if(smokecullflag==1&&j!=js2){
            x11[1] = yplt[j];
            x12[1] = yplt[j];
            x22[1] = yplt[j+1];
            x21[1] = yplt[j+1];
            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(i = is1;i<=is2;i++){
            iterm = (i-is1);
            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, NULL, 3);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = ks1;
    slice_end = ks2;
    for(kkk = slice_beg;kkk<slice_end;kkk += skip_local){
      k = kkk;
      if(ssmokedir<0)k = ks1+ks2-kkk-1;
      constval = zplt[k]+0.001;
      kterm = (k-ks1)*nxy;

      if(smokecullflag==1){
        x11[0] = xplt[is1];
        x12[0] = xplt[is2];
        x22[0] = xplt[is2];
        x21[0] = xplt[is1];

        x11[1] = yplt[js1];
        x12[1] = yplt[js1];
        x22[1] = yplt[js2];
        x21[1] = yplt[js2];

        x11[2] = zplt[k];
        x12[2] = zplt[k];
        x22[2] = zplt[k];
        x21[2] = zplt[k];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(j = js1; j<js2; j+=smoke3d_skip){
        int j2,joffset;

        j2 = MIN(j+smoke3d_skip,js2);
        joffset = j2 - j;
        jterm = (j-js1)*nx;

        yy1 = yplt[j];
        y3 = yplt[j2];

        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;

        if(smokecullflag==1){
          x11[1] = yplt[j];
          x12[1] = yplt[j];
          x22[1] = yplt[j2];
          x21[1] = yplt[j2];
          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(i = is1; i<is2; i+=smoke3d_skip){
        int i2,ioffset;

        i2 = MIN(i+smoke3d_skip,is2);
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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 4 +++++++++++++++++++++++++++++++++++++++

  case 4:
  case -4:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    aspectratio = meshi->dxy;
    if(adjustalphaflag!=0){

      for(iii = 1;iii<nx+ny-2;iii += skip_local){
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


        if(smokecullflag==1){
          x11[0] = xplt[is1+ibeg];
          x12[0] = xplt[is1+iend];
          x22[0] = xplt[is1+iend];
          x21[0] = xplt[is1+ibeg];

          x11[1] = yplt[js1+jbeg];
          x12[1] = yplt[js1+jend];
          x22[1] = yplt[js1+jend];
          x21[1] = yplt[js1+jbeg];

          x11[2] = zplt[ks1];
          x12[2] = zplt[ks1];
          x22[2] = zplt[ks2];
          x21[2] = zplt[ks2];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(k = ks1;k<=ks2;k++){
          kterm = (k-ks1)*nxy;

          if(smokecullflag==1&&k!=ks2){
            x11[2] = zplt[k];
            x12[2] = zplt[k];
            x22[2] = zplt[k+1];
            x21[2] = zplt[k+1];

            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(ii = ibeg;ii<=iend;ii++){
            i = is1+ii;
            iterm = (i-is1);

            jj = ipj-ii;
            j = js1+jj;
            jterm = (j-js1)*nx;

            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, norm, 4);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = nx+ny-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1+ibeg];
        x12[0] = xplt[is1+iend];
        x22[0] = xplt[is1+iend];
        x21[0] = xplt[is1+ibeg];

        x11[1] = yplt[js1+jbeg];
        x12[1] = yplt[js1+jend];
        x22[1] = yplt[js1+jend];
        x21[1] = yplt[js1+jbeg];

        x11[2] = zplt[ks1];
        x12[2] = zplt[ks1];
        x22[2] = zplt[ks2];
        x21[2] = zplt[ks2];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(k = ks1; k<ks2; k++){
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k+1];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;

        if(smokecullflag==1){
          x11[2] = zplt[k];
          x12[2] = zplt[k];
          x22[2] = zplt[k+1];
          x21[2] = zplt[k+1];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 5 +++++++++++++++++++++++++++++++++++++++

  case 5:
  case -5:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    aspectratio = meshi->dxy;
    if(adjustalphaflag!=0){

      for(iii = 1;iii<nx+ny-2;iii += skip_local){
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

        if(smokecullflag==1){
          x11[0] = xplt[is1+ibeg];
          x12[0] = xplt[is1+iend];
          x22[0] = xplt[is1+iend];
          x21[0] = xplt[is1+ibeg];

          x11[1] = yplt[js1+jbeg];
          x12[1] = yplt[js1+jend];
          x22[1] = yplt[js1+jend];
          x21[1] = yplt[js1+jbeg];

          x11[2] = zplt[ks1];
          x12[2] = zplt[ks1];
          x22[2] = zplt[ks2];
          x21[2] = zplt[ks2];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(k = ks1;k<=ks2;k++){
          kterm = (k-ks1)*nxy;

          if(smokecullflag==1&&k!=ks2){
            x11[2] = zplt[k];
            x12[2] = zplt[k];
            x22[2] = zplt[k+1];
            x21[2] = zplt[k+1];
            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(ii = ibeg;ii<=iend;ii++){
            i = is1+ii;
            iterm = (i-is1);

            jj = ii+jmi+1-nx;
            j = js1+jj;
            jterm = (j-js1)*nx;

            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, norm, 4);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = nx+ny-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1+ibeg];
        x12[0] = xplt[is1+iend];
        x22[0] = xplt[is1+iend];
        x21[0] = xplt[is1+ibeg];

        x11[1] = yplt[js1+jbeg];
        x12[1] = yplt[js1+jend];
        x22[1] = yplt[js1+jend];
        x21[1] = yplt[js1+jbeg];

        x11[2] = zplt[ks1];
        x12[2] = zplt[ks1];
        x22[2] = zplt[ks2];
        x21[2] = zplt[ks2];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(k = ks1; k<ks2; k++){
        kterm = (k-ks1)*nxy;
        z1 = zplt[k];
        z3 = zplt[k+1];
        znode[0] = z1;
        znode[1] = z1;
        znode[2] = z3;
        znode[3] = z3;


        if(smokecullflag==1){
          x11[2] = z1;
          x12[2] = z1;
          x22[2] = z3;
          x21[2] = z3;

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 6 +++++++++++++++++++++++++++++++++++++++

  case 6:
  case -6:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    aspectratio = meshi->dyz;
    if(adjustalphaflag!=0){

      for(iii = 1;iii<ny+nz-2;iii += skip_local){
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


        if(smokecullflag==1){
          x11[0] = xplt[is1];
          x12[0] = xplt[is1];
          x22[0] = xplt[is2];
          x21[0] = xplt[is2];

          x11[1] = yplt[js1+jbeg];
          x12[1] = yplt[js1+jend];
          x22[1] = yplt[js1+jend];
          x21[1] = yplt[js1+jbeg];

          x11[2] = zplt[ks1+kbeg];
          x12[2] = zplt[ks1+kend];
          x22[2] = zplt[ks1+kend];
          x21[2] = zplt[ks1+kbeg];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(i = is1;i<=is2;i++){
          iterm = (i-is1);

          if(smokecullflag==1&&i!=is2){
            x11[0] = xplt[i];
            x12[0] = xplt[i];
            x22[0] = xplt[i+1];
            x21[0] = xplt[i+1];

            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(jj = jbeg;jj<=jend;jj++){
            j = js1+jj;
            jterm = (j-js1)*nx;

            kk = jpk-jj;
            k = ks1+kk;
            kterm = (k-ks1)*nxy;

            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, norm, 4);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = ny+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1];
        x12[0] = xplt[is1];
        x22[0] = xplt[is2];
        x21[0] = xplt[is2];

        x11[1] = yplt[js1+jbeg];
        x12[1] = yplt[js1+jend];
        x22[1] = yplt[js1+jend];
        x21[1] = yplt[js1+jbeg];

        x11[2] = zplt[ks1+kbeg];
        x12[2] = zplt[ks1+kend];
        x22[2] = zplt[ks1+kend];
        x21[2] = zplt[ks1+kbeg];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(i = is1; i<is2; i++){
        iterm = (i-is1);
        x1 = xplt[i];
        x3 = xplt[i+1];
        xnode[0] = x1;
        xnode[1] = x1;
        xnode[2] = x3;
        xnode[3] = x3;

        if(smokecullflag==1){
          x11[0] = xplt[i];
          x12[0] = xplt[i];
          x22[0] = xplt[i+1];
          x21[0] = xplt[i+1];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 7 +++++++++++++++++++++++++++++++++++++++

  case 7:
  case -7:
    aspectratio = meshi->dyz;

    if(adjustalphaflag!=0){

      for(iii = 1;iii<ny+nz-2;iii += skip_local){
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

        if(smokecullflag==1){
          x11[0] = xplt[is1];
          x12[0] = xplt[is1];
          x22[0] = xplt[is2];
          x21[0] = xplt[is2];

          x11[1] = yplt[js1+jbeg];
          x12[1] = yplt[js1+jend];
          x22[1] = yplt[js1+jend];
          x21[1] = yplt[js1+jbeg];

          x11[2] = zplt[ks1+kbeg];
          x12[2] = zplt[ks1+kend];
          x22[2] = zplt[ks1+kend];
          x21[2] = zplt[ks1+kbeg];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(i = is1;i<=is2;i++){
          iterm = (i-is1);

          if(smokecullflag==1&&i!=is2){
            x11[0] = xplt[i];
            x12[0] = xplt[i];
            x22[0] = xplt[i+1];
            x21[0] = xplt[i+1];
            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(jj = jbeg;jj<=jend;jj++){
            j = js1+jj;
            jterm = (j-js1)*nx;

            kk = jj+kmj+1-ny;
            k = ks1+kk;
            kterm = (k-ks1)*nxy;

            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, norm, 4);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = ny+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
      kmj = iii;
      if(ssmokedir<0)kmi = ny+nz-2-iii;

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

      if(smokecullflag==1){
        x11[0] = xplt[is1];
        x12[0] = xplt[is1];
        x22[0] = xplt[is2];
        x21[0] = xplt[is2];

        x11[1] = yplt[js1+jbeg];
        x12[1] = yplt[js1+jend];
        x22[1] = yplt[js1+jend];
        x21[1] = yplt[js1+jbeg];

        x11[2] = zplt[ks1+kbeg];
        x12[2] = zplt[ks1+kend];
        x22[2] = zplt[ks1+kend];
        x21[2] = zplt[ks1+kbeg];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(i = is1; i<is2; i++){
        iterm = (i-is1);
        x1 = xplt[i];
        x3 = xplt[i+1];
        xnode[0] = x1;
        xnode[1] = x1;
        xnode[2] = x3;
        xnode[3] = x3;


        if(smokecullflag==1){
          x11[0] = x1;
          x12[0] = x1;
          x22[0] = x3;
          x21[0] = x3;

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
        }
      }
    }
    glEnd();
    break;


    // +++++++++++++++++++++++++++++++++++ DIR 8 +++++++++++++++++++++++++++++++++++++++

  case 8:
  case -8:

    // ++++++++++++++++++  adjust transparency +++++++++++++++++

    aspectratio = meshi->dxz;
    if(adjustalphaflag!=0){

      for(iii = 1;iii<nx+nz-2;iii += skip_local){
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


        if(smokecullflag==1){
          x11[0] = xplt[is1+ibeg];
          x12[0] = xplt[is1+iend];
          x22[0] = xplt[is1+iend];
          x21[0] = xplt[is1+ibeg];

          x11[1] = yplt[js1];
          x12[1] = yplt[js1];
          x22[1] = yplt[js2];
          x21[1] = yplt[js2];

          x11[2] = zplt[ks1+kbeg];
          x12[2] = zplt[ks1+kend];
          x22[2] = zplt[ks1+kend];
          x21[2] = zplt[ks1+kbeg];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(j = js1;j<=js2;j++){
          jterm = (j-js1)*nx;

          if(smokecullflag==1&&j!=js2){
            x11[1] = yplt[j];
            x12[1] = yplt[j];
            x22[1] = yplt[j+1];
            x21[1] = yplt[j+1];

            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(ii = ibeg;ii<=iend;ii++){
            i = is1+ii;
            iterm = (i-is1);

            kk = ipk-ii;
            k = ks1+kk;
            kterm = (k-ks1)*nxy;

            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, norm, 4);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);
    slice_beg = 1;
    slice_end = nx+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1+ibeg];
        x12[0] = xplt[is1+iend];
        x22[0] = xplt[is1+iend];
        x21[0] = xplt[is1+ibeg];

        x11[1] = yplt[js1];
        x12[1] = yplt[js1];
        x22[1] = yplt[js2];
        x21[1] = yplt[js2];

        x11[2] = zplt[ks1+kbeg];
        x12[2] = zplt[ks1+kend];
        x22[2] = zplt[ks1+kend];
        x21[2] = zplt[ks1+kbeg];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(j = js1; j<js2; j++){
        jterm = (j-js1)*nx;
        yy1 = yplt[j];
        y3 = yplt[j+1];
        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;

        if(smokecullflag==1){
          x11[1] = yplt[j];
          x12[1] = yplt[j];
          x22[1] = yplt[j+1];
          x21[1] = yplt[j+1];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
        }
      }
    }
    glEnd();
    break;

    // +++++++++++++++++++++++++++++++++++ DIR 9 +++++++++++++++++++++++++++++++++++++++

    /* interchange y and z, j and z */
  case 9:
  case -9:
    aspectratio = meshi->dxz;
    if(adjustalphaflag!=0){

      for(iii = 1;iii<nx+nz-2;iii += skip_local){
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

        if(smokecullflag==1){
          x11[0] = xplt[is1+ibeg];
          x12[0] = xplt[is1+iend];
          x22[0] = xplt[is1+iend];
          x21[0] = xplt[is1+ibeg];

          x11[1] = yplt[js1];
          x12[1] = yplt[js1];
          x22[1] = yplt[js2];
          x21[1] = yplt[js2];

          x11[2] = zplt[ks1+kbeg];
          x12[2] = zplt[ks1+kend];
          x22[2] = zplt[ks1+kend];
          x21[2] = zplt[ks1+kbeg];

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

        for(j = js1;j<=js2;j++){
          jterm = (j-js1)*nx;

          if(smokecullflag==1&&j!=js2){
            x11[1] = yplt[j];
            x12[1] = yplt[j];
            x22[1] = yplt[j+1];
            x21[1] = yplt[j+1];
            if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
          }

          for(ii = ibeg;ii<=iend;ii++){
            i = is1+ii;
            iterm = (i-is1);

            kk = ii+kmi+1-nx;
            k = ks1+kk;
            kterm = (k-ks1)*nxy;

            n = iterm+jterm+kterm;
            ASSERT(n>=0&&n<smoke3di->nchars_uncompressed);
            smokealpha = smokealpha_ptr[n];
            ADJUSTALPHA(smokealpha, aspectratio, norm, 4);
          }
        }
      }
      alphaf_ptr = alphaf_out;
    }
    else{
      alphaf_ptr = alphaf_in;
    }

    // ++++++++++++++++++  draw triangles +++++++++++++++++

    glBegin(GL_TRIANGLES);

    slice_beg = 1;
    slice_end = nx+nz-2;
    for(iii = slice_beg;iii<slice_end;iii += skip_local){
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

      if(smokecullflag==1){
        x11[0] = xplt[is1+ibeg];
        x12[0] = xplt[is1+iend];
        x22[0] = xplt[is1+iend];
        x21[0] = xplt[is1+ibeg];

        x11[1] = yplt[js1];
        x12[1] = yplt[js1];
        x22[1] = yplt[js2];
        x21[1] = yplt[js2];

        x11[2] = zplt[ks1+kbeg];
        x12[2] = zplt[ks1+kend];
        x22[2] = zplt[ks1+kend];
        x21[2] = zplt[ks1+kbeg];

        if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
      }

      for(j = js1; j<js2; j++){
        jterm = (j-js1)*nx;
        yy1 = yplt[j];
        y3 = yplt[j+1];
        ynode[0] = yy1;
        ynode[1] = yy1;
        ynode[2] = y3;
        ynode[3] = y3;


        if(smokecullflag==1){
          x11[1] = yy1;
          x12[1] = yy1;
          x22[1] = y3;
          x21[1] = y3;

          if(RectangleInFrustum(x11, x12, x22, x21)==0)continue;
        }

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

          if(nterraininfo>0&&ABS(vertical_factor-1.0)>0.01){
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
        }
      }
    }
    glEnd();
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
  TransparentOff();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
#ifdef pp_SMOKEDIAG
  STOP_TIMER(draw_time);
  smoketime_draw += draw_time;
#endif
}

/* ------------------ DrawSmokeFrame ------------------------ */

#ifdef pp_SMOKETEST
void DrawSmokeFrame(int option){
#else
void DrawSmokeFrame(void){
#endif
  // options:
  // SMOKE3D_FIRE_ONLY      0
  // SMOKE3D_SMOKE_ONLY     1
  // SMOKE3D_SMOKE_AND_FIRE 2

#ifdef pp_GPUSMOKE
  float smoke_time = 0.0;
#endif
  int load_shaders = 0;
  int i;
  int blend_mode;
#ifdef pp_SMOKEDIAG
  int nm=0;
#endif

  triangle_count = 0;
#ifdef pp_GPUSMOKE
  if(smoke_timer == 1){
    START_TIMER(smoke_time);
  }
#endif
#ifdef pp_GPU
  if(usegpu==1){
    if(use_newsmoke==SMOKE3D_ORIG){
      LoadSmokeShaders();
      load_shaders = 1;
    }
#ifdef pp_GPUSMOKE
    else if(use_newsmoke == SMOKE3D_NEW){
      LoadNewSmokeShaders();
      load_shaders = 1;
    }
#endif
  }
#endif

  blend_mode = 0;
#ifdef pp_SMOKETEST
  if(option==SMOKE3D_FIRE_ONLY){
#else
  if(usegpu==0&&hrrpuv_max_blending==1){
#endif
    blend_mode = 1;
    glBlendEquation(GL_MAX);
  }
#ifdef pp_SMOKEDIAG
  total_triangles=0;
  total_drawn_triangles=0;
#endif
  for(i = 0; i<nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo_sorted[i];
    if(smoke3di->loaded==0||smoke3di->display==0)continue;
#ifdef pp_SMOKETEST
    switch(option){
    case SMOKE3D_FIRE_ONLY:
      if(smoke3di->type!=HRRPUV&&smoke3di->type!=TEMP)continue;
      break;
    case SMOKE3D_SMOKE_AND_FIRE:
      if(smoke3di->primary_file==0)continue;
      if(IsSmokeComponentPresent(smoke3di)==0)continue;
      break;
    case SMOKE3D_SMOKE_ONLY:
      if(smoke3di->type!=SOOT)continue;
      break;
    }
#else
    if(smoke3di->primary_file==0)continue;
    if(IsSmokeComponentPresent(smoke3di)==0)continue;
#endif
#ifdef pp_SMOKEDIAG
    nm++;
#endif
#ifdef pp_GPU
    if(usegpu==1){
#ifdef pp_GPUSMOKE
      if(use_newsmoke==SMOKE3D_NEW){
#ifdef pp_SMOKETEST
        triangle_count += DrawSmoke3DGPUNew(smoke3di,option);
#else
        triangle_count += DrawSmoke3DGPUNew(smoke3di);
#endif
      }
      else if(use_newsmoke==SMOKE3D_ORIG){
        DrawSmoke3DGPU(smoke3di);
      }
      else{
        DrawSmokeDiag(smoke3di);
      }
#else
      DrawSmoke3DGPU(smoke3di);
#endif
    }
    else{
      if(use_newsmoke==SMOKE3D_ORIG){
        DrawSmoke3D(smoke3di);
      }
#ifdef pp_GPUSMOKE
      else if(use_newsmoke==SMOKE3D_NEW){
#ifdef pp_SMOKETEST
        triangle_count += DrawSmoke3DNew(smoke3di,option);
#else
        triangle_count += DrawSmoke3DNew(smoke3di);
#endif
      }
      else{
        DrawSmokeDiag(smoke3di);
      }
#endif
    }
#else
    DrawSmoke3D(smoke3di);
#endif
  }
#ifdef pp_SMOKEDIAG
  printf("meshes: %i triangles: total=%u drawn=%u fraction skipped=%f\n",
    nm,total_triangles,total_drawn_triangles,(float)(total_triangles-total_drawn_triangles)/(float)total_triangles);
  printf("times: merge=%f draw=%f\n", smoketime_merge, smoketime_draw);
#endif
  if(blend_mode==1){
    glBlendEquation(GL_FUNC_ADD);
  }
#ifdef pp_GPU
  if(load_shaders==1){
    UnLoadShaders();
  }
#endif
#ifdef pp_GPUSMOKE
  if(smoke_timer == 1){
    float rate=-999.0;
    char label1[100],label2[100],label3[100];
    
    STOP_TIMER(smoke_time);
    if(smoke_time>0.0){
      rate = (float)triangle_count/smoke_time;
    }
#ifdef pp_GPUSMOKE
    if(use_newsmoke==SMOKE3D_NEW){
      printf("triangles=%s verts=%s time=%f triangle rate=%s/s\n",
        GetIntLabel(triangle_count, label1),
        GetIntLabel(smoke_function_count, label2),
        smoke_time,
        GetFloatLabel(rate, label3));
    }
    else{
      printf("triangles=%s time=%f triangle rate=%s/s\n",
        GetIntLabel(triangle_count, label1),
        smoke_time,
        GetFloatLabel(rate, label3));
    }
#endif
  }
#endif

  SNIFF_ERRORS("after drawsmoke");
}

/* ------------------ DrawSmokeVolFrame ------------------------ */

void DrawVolSmokeFrame(void){
#ifdef pp_GPUSMOKE
  float smoke_time = 0.0;
#endif
  int load_shaders = 0;

  triangle_count = 0;
#ifdef pp_GPUSMOKE
  if(smoke_timer==1){
    START_TIMER(smoke_time);
  }
#endif
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
#ifdef pp_GPUSMOKE
  if(smoke_timer==1){
    float rate = -999.0;
    char label1[100], label2[100], label3[100];

    STOP_TIMER(smoke_time);
    if(smoke_time>0.0){
      rate = (float)triangle_count/smoke_time;
    }
#ifdef pp_GPUSMOKE
    if(use_newsmoke==SMOKE3D_NEW){
      printf("triangles=%s verts=%s time=%f triangle rate=%s/s\n",
        GetIntLabel(triangle_count, label1),
        GetIntLabel(smoke_function_count, label2),
        smoke_time,
        GetFloatLabel(rate, label3));
    }
    else{
      printf("triangles=%s time=%f triangle rate=%s/s\n",
        GetIntLabel(triangle_count, label1),
        smoke_time,
        GetFloatLabel(rate, label3));
    }
#endif
  }
#endif

  SNIFF_ERRORS("after drawsmoke");
}

/* ------------------ SkipSmokeFrames ------------------------ */

void SkipSmokeFrames(FILE *SMOKE3DFILE, int nsteps, int fortran_skip){
  int i;

  if(nsteps==0)return;
  for(i=0;i<nsteps;i++){
    float time_local;
    int nchars[2], skip_local;

    SKIP; fread(&time_local, 4, 1, SMOKE3DFILE); SKIP;
    if(feof(SMOKE3DFILE)!=0)break;
    SKIP; fread(nchars, 4, 2, SMOKE3DFILE); SKIP;
    skip_local = ABS(nchars[1]);
    SKIP; FSEEK(SMOKE3DFILE, skip_local, SEEK_CUR); SKIP;
  }
}

/* ------------------ GetSmokeFileSize ------------------------ */

FILE *GetSmokeFileSize(char *smokefile, int fortran_skip, int version){
  FILE *SMOKE_SIZE, *SMOKE3DFILE;
  char smoke_sizefilename[1024], smoke_sizefilename2[1024];
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
  SMOKE_SIZE = fopen(smoke_sizefilename, "r");

  if(SMOKE_SIZE == NULL){
    // not .szz so try .sz
    strcpy(smoke_sizefilename, smokefile);
    strcat(smoke_sizefilename, ".sz");
    SMOKE_SIZE = fopen(smoke_sizefilename, "r");
  }
  if(SMOKE_SIZE != NULL)return SMOKE_SIZE;

  // wasn't able to read the size file so try creating a new one

  strcpy(smoke_sizefilename, smokefile);
  strcat(smoke_sizefilename, ".sz");
  SMOKE_SIZE = fopen(smoke_sizefilename, "w");
  if(SMOKE_SIZE == NULL&&smokeviewtempdir != NULL){
    strcpy(smoke_sizefilename2, smokeviewtempdir);
    strcat(smoke_sizefilename2, smoke_sizefilename);
    strcpy(smoke_sizefilename, smoke_sizefilename2);
    SMOKE_SIZE = fopen(smoke_sizefilename, "w");
  }
  if(SMOKE_SIZE == NULL)return NULL;  // can't write size file in temp directory so give up
  SMOKE3DFILE = fopen(smokefile, "rb");
  if(SMOKE3DFILE == NULL){
    fclose(SMOKE_SIZE);
    return NULL;
  }

  SKIP; fread(nxyz, 4, 8, SMOKE3DFILE); SKIP;

  if(version != 1)version = 0;
  fprintf(SMOKE_SIZE, "%i\n", version);

  for(;;){
    int nframeboth;
    size_t count;

    SKIP; count=fread(&time_local, 4, 1, SMOKE3DFILE); SKIP;
    if(count!=1||feof(SMOKE3DFILE) != 0)break;
    SKIP; fread(nchars, 4, 2, SMOKE3DFILE); SKIP;
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
    SKIP; FSEEK(SMOKE3DFILE, skip_local, SEEK_CUR); SKIP;
  }

  fclose(SMOKE3DFILE);
  fclose(SMOKE_SIZE);
  SMOKE_SIZE = fopen(smoke_sizefilename, "r");
  return SMOKE_SIZE;
}


/* ------------------ GetSmoke3DTimeStepsMin ------------------------ */

int GetSmoke3DTimeStepsMin(int smoketype){
  int i, minstep=-1;

  for(i = 0; i < nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    int nsteps, nsteps_found;

    smoke3di = smoke3dinfo + i;
    if((smoke3di->type2&smoketype) != 0){
      int fortran_skip;

      fortran_skip = 0;
      if(smoke3di->filetype==FORTRAN_GENERATED&&smoke3di->is_zlib==0)fortran_skip = 4;
      GetSmoke3DTimeSteps(fortran_skip, smoke3di->file, smoke3di->compression_type, &nsteps_found, &nsteps);
      if(minstep==-1){
        minstep = nsteps;
      }
      else{
        minstep = MIN(minstep,nsteps);
      }
    }
  }
  return minstep;
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
    if(use_tload_end == 1 && time_local > tload_end)break;
    if(iii%smoke3dframestep == 0 && (use_tload_begin == 0 || time_local >= tload_begin)){
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
  return;
}

/* ------------------ GetSmoke3DSizes ------------------------ */

int GetSmoke3DSizes(int fortran_skip, char *smokefile, int version, float **timelist_found, int **use_smokeframe,
  int *nchars_smoke_uncompressed, int **nchars_smoke_compressed_found, int **nchars_smoke_compressed_full, float *maxval, int *ntimes_found, int *ntimes_full){

  char buffer[255];
  FILE *SMOKE_SIZE = NULL;
  int nframes_found;
  float time_local, time_max, *time_found = NULL;
  int *use_smokeframe_full;
  int nch_uncompressed, nch_smoke_compressed;
  int *nch_smoke_compressed_full = NULL;
  int *nch_smoke_compressed_found = NULL;
  int iframe_local;
  int dummy;
  int ntimes_full2;
  int iii;

  SMOKE_SIZE = GetSmokeFileSize(smokefile,fortran_skip,version);
  if(SMOKE_SIZE == NULL)return 1;
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
    if(use_tload_end == 1 && time_local > tload_end)break;
    if(iii%smoke3dframestep == 0 && (use_tload_begin == 0 || time_local >= tload_begin)){
      nframes_found++;
      time_max = time_local;
    }
    iii++;
  }
  rewind(SMOKE_SIZE);
  if(nframes_found <= 0){
    *ntimes_found = 0;
    *ntimes_full = 0;
    fclose(SMOKE_SIZE);
    return 1;
  }
  *ntimes_found = nframes_found;
  *ntimes_full = iframe_local + 1;

  use_smokeframe_full=*use_smokeframe;
  time_found =*timelist_found;
  nch_smoke_compressed_full =*nchars_smoke_compressed_full;
  nch_smoke_compressed_found=*nchars_smoke_compressed_found;

  NewResizeMemory(       use_smokeframe_full, (*ntimes_full)  * sizeof(int));
  NewResizeMemory(                time_found, nframes_found   * sizeof(float));
  NewResizeMemory( nch_smoke_compressed_full, (*ntimes_full)  * sizeof(int));
  NewResizeMemory(nch_smoke_compressed_found, (*ntimes_found) * sizeof(int));

  *use_smokeframe = use_smokeframe_full;
  *timelist_found = time_found;
  *nchars_smoke_compressed_full = nch_smoke_compressed_full;
  *nchars_smoke_compressed_found = nch_smoke_compressed_found;

  fgets(buffer, 255, SMOKE_SIZE);
  ntimes_full2 = 0;
  time_max = -1000000.0;
  iii = 0;
  *maxval = -1.0;
  while(!feof(SMOKE_SIZE)){
    float maxvali;

    if(fgets(buffer, 255, SMOKE_SIZE) == NULL)break;
    ntimes_full2++;
    if(ntimes_full2 > *ntimes_full)break;
    maxvali = -1.0;
    if(version == 0){
      sscanf(buffer, "%f %i %i %f", &time_local, &nch_uncompressed, &nch_smoke_compressed, &maxvali);
    }
    else{
      int nch_light;

      nch_light = 0;
      sscanf(buffer, "%f %i %i %i %i %f", &time_local, &nch_uncompressed, &dummy, &nch_smoke_compressed, &nch_light, &maxvali);
    }
    *maxval = MAX(maxvali, *maxval);
    *nch_smoke_compressed_full++ = nch_smoke_compressed;
    *use_smokeframe_full = 0;
    if(use_tload_end == 1 && time_local > tload_end)break;
    if(time_local <= time_max){
      use_smokeframe_full++;
      continue;
    }

    if(iii%smoke3dframestep == 0 && (use_tload_begin == 0 || time_local >= tload_begin)){
      *use_smokeframe_full = 1;
      *time_found++ = time_local;
      time_max = time_local;
      *nch_smoke_compressed_found++ = nch_smoke_compressed;
    }
    use_smokeframe_full++;
    iii++;
  }
  *nchars_smoke_uncompressed = nch_uncompressed;
  fclose(SMOKE_SIZE);
  return 0;
}

/* ------------------ FreeSmoke3d ------------------------ */

void FreeSmoke3D(smoke3ddata *smoke3di){

  smoke3di->lastiframe = -999;
  FREEMEMORY(smoke3di->smokeframe_in);
  FREEMEMORY(smoke3di->smokeframe_out);
  FREEMEMORY(smoke3di->timeslist);
  FREEMEMORY(smoke3di->times);
  FREEMEMORY(smoke3di->use_smokeframe);
  FREEMEMORY(smoke3di->nchars_compressed_smoke_full);
  FREEMEMORY(smoke3di->nchars_compressed_smoke);
  FREEMEMORY(smoke3di->frame_all_zeros);
  FREEMEMORY(smoke3di->smoke_boxmin);
  FREEMEMORY(smoke3di->smoke_boxmax);
  FREEMEMORY(smoke3di->smoke_comp_all);
  FREEMEMORY(smoke3di->smokeframe_comp_list);
  FREEMEMORY(smoke3di->smokeview_tmp);
}

/* ------------------ GetSmoke3DVersion ------------------------ */

int GetSmoke3DVersion(smoke3ddata *smoke3di){

  FILE *SMOKE3DFILE = NULL, *SMOKE3D_REGFILE = NULL, *SMOKE3D_COMPFILE = NULL;
  int nxyz[8];
  char *file;
  int fortran_skip = 0;

  if(smoke3di->filetype==FORTRAN_GENERATED&&smoke3di->is_zlib==0)fortran_skip = 4;

  file = smoke3di->comp_file;
  SMOKE3D_COMPFILE = fopen(file, "rb");
  if(SMOKE3D_COMPFILE==NULL){
    file = smoke3di->reg_file;
    SMOKE3D_REGFILE = fopen(file, "rb");
  }
  if(SMOKE3D_REGFILE==NULL&&SMOKE3D_COMPFILE==NULL)return -1;
  if(SMOKE3D_COMPFILE!=NULL)SMOKE3DFILE = SMOKE3D_COMPFILE;
  if(SMOKE3D_REGFILE!=NULL)SMOKE3DFILE = SMOKE3D_REGFILE;
  smoke3di->file = file;

  SKIP;fread(nxyz, 4, 8, SMOKE3DFILE);SKIP;
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

/* ------------------ SetSmokeColorFlags ------------------------ */

void SetSmokeColorFlags(void){
  int i;

  for(i = 0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    int j;

    smoke3di = smoke3dinfo + i;
    for(j = 0;j < MAXSMOKETYPES;j++){
      smoke3di->smokestate[j].loaded = 0;
    }
  }

  for(i = 0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    int j;

    smoke3di = smoke3dinfo+i;
    for(j = 0;j < MAXSMOKETYPES;j++){
      smoke3di->smokestate[j].color = NULL;
      smoke3di->smokestate[j].index = -1;
    }
    if(smoke3di->loaded==0)continue;

    if(smoke3di->type >= 0 && smoke3di->type < MAXSMOKETYPES){
      smoke3di->smokestate[smoke3di->type].color = smoke3di->smokeframe_in;
      smoke3di->smokestate[smoke3di->type].index = i;
    }
    else{
      ASSERT(FFALSE);
    }

    for(j = 0;j<nsmoke3dinfo;j++){
      smoke3ddata *smoke3dj;
      int k;

      if(i==j)continue;
      smoke3dj = smoke3dinfo+j;
      if(smoke3dj->loaded==0)continue;
      if(smoke3di->blocknumber!=smoke3dj->blocknumber)continue;
      if(smoke3di->is1!=smoke3dj->is1)continue;
      if(smoke3di->is2!=smoke3dj->is2)continue;
      if(smoke3di->js1!=smoke3dj->js1)continue;
      if(smoke3di->js2!=smoke3dj->js2)continue;
      if(smoke3di->ks1!=smoke3dj->ks1)continue;
      if(smoke3di->ks2!=smoke3dj->ks2)continue;

      if(smoke3dj->type >= 0 && smoke3dj->type < MAXSMOKETYPES){
        smoke3di->smokestate[smoke3dj->type].color = smoke3dj->smokeframe_in;
        smoke3di->smokestate[smoke3dj->type].index = j;
      }
      else{
        ASSERT(FFALSE);
      }
      for(k = 0;k < MAXSMOKETYPES;k++){
        if(smoke3di->smokestate[k].color != NULL)smoke3dj->smokestate[k].loaded = 1;
      }
    }
  }
  for(i = 0;i<nsmoke3dinfo;i++){
    int j;
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo+i;
    if(smoke3di->loaded==0||smoke3di->display==0||smoke3di->frame_all_zeros==NULL)continue;
    for(j = 0;j<smoke3di->ntimes_full;j++){
      smoke3di->frame_all_zeros[j] = SMOKE3D_ZEROS_UNKNOWN;
    }
  }
}

/* ------------------ SmokeWrapup ------------------------ */

#define SMOKE_OPTIONS 19
void SmokeWrapup(smoke3ddata *smoke3di){
  if(smoke3di->finalize==1){
    smoke3di->finalize = 0;
    UpdateSmoke3dFileParms();
    UpdateTimes();
    Smoke3dCB(UPDATE_SMOKEFIRE_COLORS);
    smoke_render_option = RENDER_SLICE;
    Smoke3dCB(SMOKE_OPTIONS);
    IdleCB();
  }
}

/* ------------------ ReadSmoke3D ------------------------ */

FILE_SIZE ReadSmoke3D(int iframe,int ifile,int flag, int *errorcode){
  smoke3ddata *smoke3di;
  FILE *SMOKE3DFILE;
  int error;
  int ncomp_smoke_total;
  int ncomp_smoke_total_skipped;
  FILE_SIZE file_size=0;
  float read_time, total_time;
  int iii,ii,i,j;
  int nxyz[8];
  int nchars[2];
  int nframes_found=0;
  int frame_start, frame_end;
  char smoketype[100];

  float time_local;
  char compstring[128];
  meshdata *mesh_smoke3d;
  int fortran_skip=0;

#ifndef pp_FSEEK
  if(flag==RELOAD)flag = LOAD;
#endif
  START_TIMER(total_time);
  ASSERT(ifile>=0&&ifile<nsmoke3dinfo);
  smoke3di = smoke3dinfo + ifile;
  if(smoke3di->filetype==FORTRAN_GENERATED&&smoke3di->is_zlib==0)fortran_skip=4;
  mesh_smoke3d = meshinfo+smoke3di->blocknumber;

  switch(smoke3di->type){
  case HRRPUV:
    strcpy(smoketype, "hrrpuv");
    mesh_smoke3d->smoke3d_hrrpuv = smoke3di;
    break;
  case TEMP:
    strcpy(smoketype, "temperature");
    mesh_smoke3d->smoke3d_temp = smoke3di;
    break;
  case CO2:
    strcpy(smoketype, "CO2");
    mesh_smoke3d->smoke3d_co2 = smoke3di;
    break;
  case SOOT:
    strcpy(smoketype, "soot");
    mesh_smoke3d->smoke3d_soot = smoke3di;
    break;
  default:
    strcpy(smoketype, "unknown");
    ASSERT(FFALSE);
    break;
  }

  if(smoke3di->loaded==1&&flag!=RELOAD){
    FreeSmoke3D(smoke3di);
    smoke3di->loaded=0;
    smoke3di->display=0;
    smoke3di->primary_file=0;
  }

  if(flag!=RELOAD){
    FREEMEMORY(mesh_smoke3d->merge_alpha);
    FREEMEMORY(mesh_smoke3d->merge_color);
#ifdef pp_GPUSMOKE
    FREEMEMORY(mesh_smoke3d->smoke_texture_buffer);
    FREEMEMORY(mesh_smoke3d->fire_texture_buffer);
    FREEMEMORY(mesh_smoke3d->co2_texture_buffer);
#endif
  }

  if(flag==UNLOAD){
    plotstate=GetPlotState(DYNAMIC_PLOTS);
    UpdateTimes();
    Read3DSmoke3DFile=0;
    SetSmokeColorFlags();
    smoke3di->request_load = 0;

    switch(smoke3di->type){
    case HRRPUV:
      mesh_smoke3d->smoke3d_hrrpuv = NULL;
      break;
    case TEMP:
      mesh_smoke3d->smoke3d_temp = NULL;
      break;
    case CO2:
      mesh_smoke3d->smoke3d_co2 = NULL;
      break;
    case SOOT:
      mesh_smoke3d->smoke3d_soot = NULL;
      break;
    default:
      ASSERT(FFALSE);
      break;
    }

    hrrpuv_loaded=0;
    temp_loaded = 0;
    for(j=0;j<nsmoke3dinfo;j++){
      smoke3ddata *smoke3dj;

      smoke3dj = smoke3dinfo + j;
      if(smoke3dj->loaded==1){
        Read3DSmoke3DFile=1;
        if(smoke3dj->type==HRRPUV){
          hrrpuv_loaded=1;
          break;
        }
        if(smoke3dj->type == TEMP){
          temp_loaded = 1;
          break;
        }
      }
#ifdef pp_GPUSMOKE
      FreeSmokeMemory(mesh_smoke3d);
#endif
    }
    if(mesh_smoke3d->iblank_smoke3d != NULL){
      int free_iblank_smoke3d;

      free_iblank_smoke3d = 1;
      for(j = 0; j < nsmoke3dinfo; j++){
        smoke3ddata *smoke3dj;
        meshdata *meshj;

        smoke3dj = smoke3dinfo + j;
        meshj = meshinfo + smoke3dj->blocknumber;
        if(smoke3dj != smoke3di && smoke3dj->loaded == 1 && meshj == mesh_smoke3d){
          free_iblank_smoke3d = 0;
          break;
        }
      }
      if(free_iblank_smoke3d == 1){
        FREEMEMORY(mesh_smoke3d->iblank_smoke3d);
        mesh_smoke3d->iblank_smoke3d_defined = 0;
        update_makeiblank_smoke3d=1;
      }
    }
    update_makeiblank_smoke3d=1;
    UpdateSmoke3dFileParms();
    UpdateCo2Blending();
    UpdateFireCutoffs();
    return 0;
  }
  if(smoke3di->type == HRRPUV||smoke3di->type==TEMP){
    // if we are loading an HRRPUV then unload all TEMP's
    if(smoke3di->type == HRRPUV&&ntemploaded > 0)printf("unloading all smoke3d temperature files\n");

    // if we are loading a TEMP then unload all HRRPUV's
    if(smoke3di->type == TEMP&&nhrrpuvloaded > 0)printf("unloading all  smoke3d hrrpuv files\n");

    for(j = 0; j<nsmoke3dinfo; j++){
      smoke3ddata *smoke3dj;
      int error2;

      smoke3dj = smoke3dinfo + j;
      if(smoke3di != smoke3dj&&smoke3dj->loaded == 1){
        if((smoke3di->type == TEMP&&smoke3dj->type == HRRPUV) ||
          (smoke3di->type == HRRPUV&&smoke3dj->type == TEMP)){
          printf("unloading %s\n", smoke3dj->file);
          ReadSmoke3D(iframe,j, UNLOAD, &error2);
        }
      }
    }
  }

  if(smoke3di->compression_type==UNKNOWN){
    smoke3di->compression_type=GetSmoke3DVersion(smoke3di);
    UpdateSmoke3dMenuLabels();
  }

  PRINTF("loading %s(%s) ", smoke3di->file,smoketype);
  CheckMemory;
  smoke3di->request_load = 1;
  smoke3di->ntimes_old = smoke3di->ntimes;
  if(GetSmoke3DSizes(fortran_skip,smoke3di->file,smoke3di->compression_type,&smoke3di->times, &smoke3di->use_smokeframe,
                 &smoke3di->nchars_uncompressed,
                 &smoke3di->nchars_compressed_smoke,
                 &smoke3di->nchars_compressed_smoke_full,
                 &smoke3di->maxval,
                 &smoke3di->ntimes,
                 &smoke3di->ntimes_full)==1){
    ReadSmoke3D(iframe,ifile,UNLOAD,&error);
    *errorcode=1;
    fprintf(stderr,"\n*** Error: problems sizing 3d smoke data for %s\n",smoke3di->file);
    return 0;
  }
  if(smoke3di->maxval>=0.0){
    int return_flag = 0;

    if(smoke3di->type == HRRPUV&&smoke3di->maxval<=load_hrrpuv_cutoff){
      ReadSmoke3D(iframe,ifile,UNLOAD,&error);
      *errorcode=0;
      PRINTF(" - skipped (hrrpuv<%0.f)\n",load_hrrpuv_cutoff);
      return_flag=1;
    }
    if(smoke3di->type == SOOT&&smoke3di->maxval<=load_3dsmoke_cutoff){
      ReadSmoke3D(iframe,ifile,UNLOAD,&error);
      *errorcode=0;
      PRINTF(" - skipped (opacity<%0.f)\n", load_3dsmoke_cutoff);
      return_flag = 1;
    }
    if(return_flag==1){
      if(smoke3di->finalize==1){
        SmokeWrapup(smoke3di);
      }
      return 0;
    }
  }
  CheckMemory;
  if(
     NewResizeMemory(smoke3di->smokeframe_comp_list,smoke3di->ntimes_full*sizeof(unsigned char *))==0||
     NewResizeMemory(smoke3di->frame_all_zeros,     smoke3di->ntimes_full*sizeof(unsigned char))==0||
     NewResizeMemory(smoke3di->smoke_boxmin,        3*smoke3di->ntimes_full*sizeof(float))==0||
     NewResizeMemory(smoke3di->smoke_boxmax,        3*smoke3di->ntimes_full*sizeof(float))==0||
     NewResizeMemory(smoke3di->smokeframe_in,       smoke3di->nchars_uncompressed*sizeof(unsigned char))==0||
     NewResizeMemory(smoke3di->smokeview_tmp,       smoke3di->nchars_uncompressed*sizeof(unsigned char))==0||
     NewResizeMemory(smoke3di->smokeframe_out,      smoke3di->nchars_uncompressed*sizeof(unsigned char))==0||
     NewResizeMemory(mesh_smoke3d->merge_color,          4*smoke3di->nchars_uncompressed*sizeof(unsigned char))==0||
#ifdef  pp_GPUSMOKE
     NewResizeMemory(mesh_smoke3d->smoke_texture_buffer, smoke3di->nchars_uncompressed*sizeof(float)) == 0 ||
     NewResizeMemory(mesh_smoke3d->fire_texture_buffer,  smoke3di->nchars_uncompressed*sizeof(float))==0||
     NewResizeMemory(mesh_smoke3d->co2_texture_buffer,   smoke3di->nchars_uncompressed*sizeof(float)) == 0 ||
#endif
     NewResizeMemory(mesh_smoke3d->merge_alpha, smoke3di->nchars_uncompressed * sizeof(unsigned char)) == 0){
     ReadSmoke3D(iframe,ifile,UNLOAD,&error);
     *errorcode=1;
     fprintf(stderr,"\n*** Error: problems allocating memory for 3d smoke file: %s\n",smoke3di->file);
     return 0;
  }
  for(i=0;i<smoke3di->ntimes_full;i++){
    smoke3di->frame_all_zeros[i]=SMOKE3D_ZEROS_UNKNOWN;
  }

  ncomp_smoke_total=0;
  ncomp_smoke_total_skipped=0;
  for(i=0;i<smoke3di->ntimes_full;i++){
    ncomp_smoke_total+=smoke3di->nchars_compressed_smoke_full[i];
    if(smoke3di->use_smokeframe[i]==1){
      ncomp_smoke_total_skipped+=smoke3di->nchars_compressed_smoke_full[i];
    }
  }
  if(NewResizeMemory(smoke3di->smoke_comp_all,ncomp_smoke_total_skipped*sizeof(unsigned char))==0){
    ReadSmoke3D(iframe,ifile,UNLOAD,&error);
    *errorcode=1;
     fprintf(stderr,"\n*** Error: problems allocating memory for 3d smoke file: %s\n",smoke3di->file);
    return 0;
  }
  smoke3di->ncomp_smoke_total=ncomp_smoke_total_skipped;

  ncomp_smoke_total=0;
  i=0;
  for(ii=0;ii<smoke3di->ntimes_full;ii++){
    if(smoke3di->use_smokeframe[ii]==1){
      smoke3di->smokeframe_comp_list[i]=smoke3di->smoke_comp_all+ncomp_smoke_total;
      ncomp_smoke_total+=smoke3di->nchars_compressed_smoke[i];
      i++;
    }
  }

// read in data

  GetFileSizeSMV(smoke3di->file);
  SMOKE3DFILE=fopen(smoke3di->file,"rb");
  if(SMOKE3DFILE==NULL){
    ReadSmoke3D(iframe,ifile,UNLOAD,&error);
    *errorcode=1;
    return 0;
  }

  SKIP;fread(nxyz,4,8,SMOKE3DFILE);SKIP;
  file_size+=4+4*8+4;
  smoke3di->is1=nxyz[2];
  smoke3di->is2=nxyz[3];
  smoke3di->js1=nxyz[4];
  smoke3di->js2=nxyz[5];
  smoke3di->ks1=nxyz[6];
  smoke3di->ks2=nxyz[7];
  smoke3di->compression_type=nxyz[1];

  // read smoke data

  START_TIMER(read_time);
  if(iframe == ALL_FRAMES){
    if(flag == RELOAD&&smoke3di->ntimes_old > 0){
      SkipSmokeFrames(SMOKE3DFILE, smoke3di->ntimes_old, fortran_skip);
      frame_start = smoke3di->ntimes_old;
    }
    else {
      frame_start = 0;
    }
    frame_end = smoke3di->ntimes_full;
  }
  else {
    SkipSmokeFrames(SMOKE3DFILE, iframe, fortran_skip);
    frame_start = iframe;
    frame_end = iframe+1;
  }
  iii = frame_start;
  nframes_found = frame_start;
  for(i=frame_start;i<frame_end;i++){
    SKIP;fread(&time_local,4,1,SMOKE3DFILE);SKIP;
    file_size+=4+4+4;
    if(feof(SMOKE3DFILE)!=0||(use_tload_end==1&&time_local>tload_end)){
      smoke3di->ntimes_full=i;
      smoke3di->ntimes=nframes_found;
      break;
    }
    if(use_tload_begin==1&&time_local<tload_begin)smoke3di->use_smokeframe[i]=0;
    SKIP;fread(nchars,4,2,SMOKE3DFILE);SKIP;
    file_size+=4+2*4+4;
    if(feof(SMOKE3DFILE)!=0){
      smoke3di->ntimes_full=i;
      smoke3di->ntimes=nframes_found;
      break;
    }
    if(smoke3di->use_smokeframe[i]==1){
      float complevel;

      nframes_found++;
      SKIP;fread(smoke3di->smokeframe_comp_list[iii],1,smoke3di->nchars_compressed_smoke[iii],SMOKE3DFILE);SKIP;
      file_size+=4+smoke3di->nchars_compressed_smoke[iii]+4;
      iii++;
      CheckMemory;
      if(feof(SMOKE3DFILE)!=0){
        smoke3di->ntimes_full=i;
        smoke3di->ntimes=nframes_found;
        break;
      }

      complevel=40.0*(float)nchars[0]/(float)nchars[1];
      complevel=(int)complevel;
      complevel/=10.0;
      if(complevel<0.0)complevel=-complevel;
      sprintf(compstring," compression ratio: %.1f",complevel);
      TrimBack(compstring);
      TrimZeros(compstring);
    }
    else{
      SKIP;FSEEK(SMOKE3DFILE,smoke3di->nchars_compressed_smoke_full[i],SEEK_CUR);SKIP;
      if(feof(SMOKE3DFILE)!=0){
        smoke3di->ntimes_full=i;
        smoke3di->ntimes=nframes_found;
        break;
      }
    }
  }
  STOP_TIMER(read_time);

  if(SMOKE3DFILE!=NULL){
    fclose(SMOKE3DFILE);
  }

  smoke3di->loaded=1;
  smoke3di->display=1;
  SetSmokeColorFlags();
  if(smoke3di->type == HRRPUV)hrrpuv_loaded=1;
  if(smoke3di->type == TEMP)temp_loaded = 1;
  UpdateCo2Blending();
  UpdateFireCutoffs();

  Read3DSmoke3DFile=1;
  update_makeiblank_smoke3d=1;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  if(smoke3di->finalize == 1){
    SmokeWrapup(smoke3di);
  }
  STOP_TIMER(total_time);

  if(file_size>1000000){
    PRINTF(" - %.1f MB/%.1f s\n",(float)file_size/1000000.,total_time);
  }
  else{
  PRINTF(" - %.0f kB/%.1f s\n",(float)file_size/1000.,total_time);
  }
  PrintMemoryInfo;
  *errorcode = 0;
  return file_size;
}

/* ------------------ ReadSmoke3DAllMeshes ------------------------ */

void ReadSmoke3DAllMeshes(int iframe, int smoketype, int *errorcode){
  int i;

  for(i = 0; i < nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;

    smoke3di = smoke3dinfo + i;
    if(smoke3di->type != smoketype)continue;
    ReadSmoke3D(iframe, i, LOAD, errorcode);
  }
}

/* ------------------ ReadSmoke3DAllMeshesAllTimes ------------------------ */

void ReadSmoke3DAllMeshesAllTimes(int smoketype2, int *errorcode){
  int i, ntimes, itime;

  ntimes = GetSmoke3DTimeStepsMin(smoketype2);
  for(itime = 0; itime < ntimes; itime++){
    for(i = 0; i < nsmoke3dinfo; i++){
      smoke3ddata *smoke3di;

      smoke3di = smoke3dinfo + i;
      switch (smoke3di->type){
        case SOOT:
          if((smoketype2&SOOT_2)!=0)ReadSmoke3DAllMeshes(itime, SOOT, errorcode);
          break;
        case HRRPUV:
          if((smoketype2&HRRPUV_2)!=0)ReadSmoke3DAllMeshes(itime, HRRPUV, errorcode);
          break;
        case TEMP:
          if((smoketype2&TEMP_2)!=0)ReadSmoke3DAllMeshes(itime, TEMP, errorcode);
          break;
        case CO2:
          if((smoketype2&CO2_2)!=0)ReadSmoke3DAllMeshes(itime, CO2, errorcode);
          break;
      }
    }
  }

}

/* ------------------ UpdateSmoke3d ------------------------ */

void UpdateSmoke3D(smoke3ddata *smoke3di){
  int iframe_local;
  int countin;
  uLongf countout;
#ifdef pp_GPUSMOKE
  meshdata *mesh_smoke3d;
#endif

  iframe_local = smoke3di->ismoke3d_time;
  countin = smoke3di->nchars_compressed_smoke[iframe_local];
  countout=smoke3di->nchars_uncompressed;
  switch(smoke3di->compression_type){
  case RLE:
    countout = UnCompressRLE(smoke3di->smokeframe_comp_list[iframe_local],countin,smoke3di->smokeframe_in);
    break;
  case ZLIB:
    UnCompressZLIB(
      smoke3di->smokeframe_in,&countout,
      smoke3di->smokeframe_comp_list[iframe_local],countin);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }

#ifdef pp_GPUSMOKE
  mesh_smoke3d = meshinfo+smoke3di->blocknumber;
#endif

  if(
#ifdef pp_GPUSMOKE
    mesh_smoke3d->update_smokebox==1||
#endif
    smoke3di->frame_all_zeros[iframe_local] == SMOKE3D_ZEROS_UNKNOWN){
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
#ifdef pp_GPUSMOKE
    // construct a bounding box containing smoke

    if(mesh_smoke3d->update_smokebox==1||smoke3di->frame_all_zeros[iframe_local] != SMOKE3D_ZEROS_ALL){
      float *smoke_boxmin, *smoke_boxmax;
      float *xplt, *yplt, *zplt;
      int ibar, jbar, kbar;
      int nx, ny, nz;
      int k;
      int imin=-1, imax=-1, jmin=-1, jmax=-1, kmin=-1, kmax=-1;
      unsigned char *smokeptr;

      xplt = mesh_smoke3d->xplt_orig;
      yplt = mesh_smoke3d->yplt_orig;
      zplt = mesh_smoke3d->zplt_orig;

      ibar = mesh_smoke3d->ibar;
      jbar = mesh_smoke3d->jbar;
      kbar = mesh_smoke3d->kbar;

      nx = ibar+1;
      ny = jbar+1;
      nz = kbar+1;

      smoke_boxmin = smoke3di->smoke_boxmin+3*iframe_local;
      smoke_boxmax = smoke3di->smoke_boxmax+3*iframe_local;

      smokeptr = smokeframe_in;
      for(k=0;k<nz;k++){
        int j;

        for(j=0;j<ny;j++){
          int i;

          for(i=0;i<nx;i++){
            //ijk = i+nx*(j+k*ny);
            // if(smokeframe_in[ijk]!=0){
            if(*smokeptr++!=0){
              if(imin==-1){imin = i;}else{imin = MIN(i,imin);}
              if(imax==-1){imax = i;}else{imax = MAX(i,imax);}
              if(jmin==-1){jmin = j;}else{jmin = MIN(j,jmin);}
              if(jmax==-1){jmax = j;}else{jmax = MAX(j,jmax);}
              if(kmin==-1){kmin = k;}else{kmin = MIN(k,kmin);}
              if(kmax==-1){kmax = k;}else{kmax = MAX(k,kmax);}
            }
          }
        }
      }
      imin = MAX(imin-smokebox_buffer, 0);
      jmin = MAX(jmin-smokebox_buffer, 0);
      kmin = MAX(kmin-smokebox_buffer, 0);
      imax = MIN(imax+smokebox_buffer, ibar);
      jmax = MIN(jmax+smokebox_buffer, jbar);
      kmax = MIN(kmax+smokebox_buffer, kbar);
      smoke_boxmin[0] = xplt[imin];
      smoke_boxmin[1] = yplt[jmin];
      smoke_boxmin[2] = zplt[kmin];
      smoke_boxmax[0] = xplt[imax];
      smoke_boxmax[1] = yplt[jmax];
      smoke_boxmax[2] = zplt[kmax];
    }
    mesh_smoke3d->update_smokebox = 0;
#endif
  }
  ASSERT(countout==smoke3di->nchars_uncompressed);
}

/* ------------------ MergeSmoke3DColors ------------------------ */

void MergeSmoke3DColors(smoke3ddata *smoke3dset){
  int i,j;
  int i_smoke3d_cutoff;
  int fire_index = HRRPUV;
  unsigned char rgb_slicesmokecolormap_0255[4*MAXSMOKERGB];
  unsigned char rgb_sliceco2colormap_0255[4*MAXSMOKERGB];

  if(have_fire==HRRPUV){
    i_smoke3d_cutoff = 254*global_hrrpuv_cutoff/hrrpuv_max_smv;
  }
  else if(have_fire==TEMP){
    i_smoke3d_cutoff = 254*((global_temp_cutoff - global_temp_min)/(global_temp_max- global_temp_min));
  }
  else{
    i_smoke3d_cutoff = 255;
  }
  for(i=0;i<4*MAXSMOKERGB;i++){
    rgb_slicesmokecolormap_0255[i] = 255*rgb_slicesmokecolormap_01[i];
    rgb_sliceco2colormap_0255[i]   = 255*rgb_sliceco2colormap_01[i];
  }

  for(i=0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di, *smoke3d_soot;
    meshdata *mesh_smoke3d;

    smoke3di=smoke3dinfo + i;
    if(smoke3dset!=NULL&&smoke3dset!=smoke3di)continue;
    smoke3di->primary_file=0;
    if(smoke3di->loaded==0||smoke3di->display==0)continue;
    mesh_smoke3d = meshinfo+smoke3di->blocknumber;
    smoke3d_soot = mesh_smoke3d->smoke3d_soot;
    switch(smoke3di->type){
    case SOOT:
      smoke3di->primary_file=1;
      break;
    case HRRPUV:
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3di->primary_file=1;
      }
      break;
    case TEMP:
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3di->primary_file = 1;
      }
      fire_index = TEMP;
      break;
    case CO2:
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3ddata *smoke3d_hrrpuv, *smoke3d_temp;

        smoke3d_hrrpuv = mesh_smoke3d->smoke3d_hrrpuv;
        smoke3d_temp   = mesh_smoke3d->smoke3d_temp;
        if(smoke3d_hrrpuv==NULL||smoke3d_hrrpuv->loaded==0||smoke3d_hrrpuv->display==0){
          if(smoke3d_temp==NULL||smoke3d_temp->loaded==0||smoke3d_temp->display==0){
            smoke3di->primary_file = 1;
          }
        }
      }
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
  }

  for(i=0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    meshdata *mesh_smoke3d;
    unsigned char *firecolor_data,*smokecolor_data,*co2color_data;
    unsigned char *mergecolor,*mergealpha;
    unsigned char smokeval_uc[3], co2val_uc[3];
    int i_hrrpuv_offset=0;

    smoke3di = smoke3dinfo + i;
    if(smoke3dset!=NULL&&smoke3dset!=smoke3di)continue;
    if(smoke3di->loaded==0||smoke3di->primary_file==0)continue;
    mesh_smoke3d = meshinfo+smoke3di->blocknumber;
    if(IsSmokeComponentPresent(smoke3di)==0)continue;
    if(smoke3d_testsmoke==1)i_hrrpuv_offset = 254*slicehrrpuv_offset/hrrpuv_max_smv;

    if(fire_halfdepth<=0.0){
      smoke3di->fire_alpha=255;
    }
    else{
      smoke3di->fire_alpha=255*(1.0-pow(0.5,mesh_smoke3d->dx/fire_halfdepth));
    }

    if(co2_halfdepth <= 0.0){
      smoke3di->co2_alpha = 255;
    }
    else {
      smoke3di->co2_alpha = 255 * (1.0 - pow(0.5, mesh_smoke3d->dx / co2_halfdepth));
    }

//  temp and hrrpuv cannot be loaded at the same time

    ASSERT(mesh_smoke3d->smoke3d_temp==NULL||mesh_smoke3d->smoke3d_hrrpuv==NULL);

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
    ASSERT(firecolor_data!=NULL||smokecolor_data!=NULL||co2color_data!=NULL);

#ifdef pp_GPU
    if(usegpu==1)continue;
#endif
#ifdef pp_GPUSMOKE
    if(mesh_smoke3d->smoke_texture_buffer == NULL){
      NewMemory((void **)&mesh_smoke3d->smoke_texture_buffer, smoke3di->nchars_uncompressed * sizeof(float));
    }
    if(mesh_smoke3d->fire_texture_buffer == NULL){
      NewMemory((void **)&mesh_smoke3d->fire_texture_buffer, smoke3di->nchars_uncompressed * sizeof(float));
    }
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

    for(j=0;j<smoke3di->nchars_uncompressed;j++){
      unsigned char *firecolor_ptr, *smokecolor_ptr, *co2color_ptr;
      float f_smoke, f_co2, denom;
      float alpha_fire_local, alpha_smoke_local, alpha_co2_local;
      int only_fire;

// set fire color and opacity

      only_fire = 0;
      alpha_fire_local = 0.0;
      if(firecolor_data!=NULL && smokecolor_data==NULL && co2color_data==NULL){
        int fire_index;

        only_fire = 1;
        fire_index = CLAMP(firecolor_data[j]+i_hrrpuv_offset,0,254);
        firecolor_ptr = rgb_slicesmokecolormap_0255+4*fire_index;
        if(firecolor_data[j]>=i_smoke3d_cutoff){
          alpha_fire_local = smoke3di->fire_alpha;
        }
      }

// set smoke color and opacity

      f_smoke = 0.0;
      alpha_smoke_local = 0.0;
      smokecolor_ptr = smokeval_uc;
      if(smokecolor_data!=NULL){
        if(firecolor_data!=NULL && firecolor_data[j]>=i_smoke3d_cutoff){
          f_smoke = (1.0-co2_fraction);
          fire_index = CLAMP(firecolor_data[j]+i_hrrpuv_offset,0,254);
          smokecolor_ptr = rgb_slicesmokecolormap_0255+4*fire_index;
          alpha_smoke_local = smoke3di->fire_alpha;
        }
        else{
          f_smoke = 1.0;
          if(firecolor_data==NULL && co2color_data!=NULL)f_smoke=(1.0-co2_fraction);
          smokecolor_ptr = smokeval_uc;
          alpha_smoke_local = smokecolor_data[j];
        }
      }

// set co2 color and opacity

      alpha_co2_local = 0.0;
      f_co2 = 0.0;
      co2color_ptr = co2val_uc;
      if(co2color_data!=NULL){
        if(firecolor_data!=NULL && firecolor_data[j]>=i_smoke3d_cutoff){
          f_co2 = co2_fraction;
          if(smokecolor_data==NULL)f_co2 = 1.0;
          co2color_ptr = rgb_sliceco2colormap_0255+4*firecolor_data[j];
          alpha_co2_local = smoke3di->co2_alpha;
        }
        if(firecolor_data==NULL){
          f_co2 = co2_fraction;
          co2color_ptr = co2val_uc;
          if(smokecolor_data==NULL)f_co2 = 1.0;
          alpha_co2_local = co2color_data[j];
        }
      }

      denom = f_smoke + f_co2;
      if(denom>0.0){
        f_smoke /= denom;
        f_co2   /= denom;
      }

// merge color and opacity

      if(only_fire == 1){
        mergecolor[0] = firecolor_ptr[0];
        mergecolor[1] = firecolor_ptr[1];
        mergecolor[2] = firecolor_ptr[2];
        *mergealpha++ = alpha_fire_local;
      }
      else{
        mergecolor[0] = f_smoke*smokecolor_ptr[0] + f_co2*co2color_ptr[0];
        mergecolor[1] = f_smoke*smokecolor_ptr[1] + f_co2*co2color_ptr[1];
        mergecolor[2] = f_smoke*smokecolor_ptr[2] + f_co2*co2color_ptr[2];
        *mergealpha++ = f_smoke*alpha_smoke_local + f_co2*alpha_co2_local;
      }
      mergecolor+=4;
    }
  }
}

/* ------------------ MergeSmoke3DBlack ------------------------ */

void MergeSmoke3DBlack(smoke3ddata *smoke3dset){
  int i;
  int fire_index = HRRPUV;
  int total1 = 0, total2=0;

  for(i = 0; i<nsmoke3dinfo; i++){
    smoke3ddata *smoke3di, *smoke3d_soot;
    meshdata *mesh_smoke3d;

    smoke3di = smoke3dinfo+i;
    if(smoke3dset!=NULL&&smoke3dset!=smoke3di)continue;
    smoke3di->primary_file = 0;
    if(smoke3di->loaded==0||smoke3di->display==0)continue;
    mesh_smoke3d = meshinfo+smoke3di->blocknumber;
    smoke3d_soot = mesh_smoke3d->smoke3d_soot;
    switch(smoke3di->type){
    case SOOT:
      smoke3di->primary_file = 1;
      break;
    case HRRPUV:
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3di->primary_file = 1;
      }
      break;
    case TEMP:
      if(smoke3d_soot==NULL||smoke3d_soot->loaded==0||smoke3d_soot->display==0){
        smoke3di->primary_file = 1;
      }
      fire_index = TEMP;
      break;
    case CO2:
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
      break;
    default:
      ASSERT(FFALSE);
      break;
    }
  }

  for(i = 0; i<nsmoke3dinfo; i++){
    smoke3ddata *smoke3di;
    meshdata *meshi;
    unsigned char *firecolor_data, *smokecolor_data;

    smoke3di = smoke3dinfo+i;
    if(smoke3dset!=NULL&&smoke3dset!=smoke3di)continue;
    if(smoke3di->loaded==0||smoke3di->primary_file==0)continue;
    total1++;
    if(IsSmokeComponentPresent(smoke3di)==0)continue;
    total2++;
    meshi = meshinfo+smoke3di->blocknumber;

    if(fire_halfdepth<=0.0){
      smoke3di->fire_alpha = 255;
    }
    else{
      smoke3di->fire_alpha = 255*(1.0-pow(0.5, meshi->dx/fire_halfdepth));
    }
    if(co2_halfdepth<=0.0){
      smoke3di->co2_alpha = 255;
    }
    else {
      smoke3di->co2_alpha = 255*(1.0-pow(0.5, meshi->dx/co2_halfdepth));
    }
    firecolor_data = NULL;
    smokecolor_data = NULL;
    if(smoke3di->smokestate[fire_index].color!=NULL){
      firecolor_data = smoke3di->smokestate[fire_index].color;
      if(smoke3di->smokestate[fire_index].index!=-1){
        smoke3ddata *smoke3dref;

        smoke3dref = smoke3dinfo+smoke3di->smokestate[fire_index].index;
        if(smoke3dref->display==0)firecolor_data = NULL;
      }
    }
    if(smoke3di->smokestate[SOOT].color!=NULL){
      smokecolor_data = smoke3di->smokestate[SOOT].color;
      if(smoke3di->smokestate[SOOT].index!=-1){
        smoke3ddata *smoke3dref;

        smoke3dref = smoke3dinfo+smoke3di->smokestate[SOOT].index;
        if(smoke3dref->display==0)smokecolor_data = NULL;
      }
    }
#ifdef pp_GPU
    if(usegpu==1&&use_newsmoke==SMOKE3D_ORIG)continue;
#endif
    ASSERT(firecolor_data!=NULL||smokecolor_data!=NULL);
    meshi->smokecolor_ptr = firecolor_data;
    meshi->smokealpha_ptr = smokecolor_data;
  }
}

/* ------------------ MergeSmoke3D ------------------------ */

void MergeSmoke3D(smoke3ddata *smoke3dset){
  if(smoke3d_black==1||use_newsmoke==SMOKE3D_NEW){
    MergeSmoke3DBlack(smoke3dset);
    }
  else{
    MergeSmoke3DColors(smoke3dset);
  }
}

/* ------------------ UpdateSmoke3dMenuLabels ------------------------ */

void UpdateSmoke3dMenuLabels(void){
  int i;
  smoke3ddata *smoke3di;
  char meshlabel[128];

  for(i=0;i<nsmoke3dinfo;i++){
    smoke3di = smoke3dinfo + i;
    STRCPY(smoke3di->menulabel, "");
    if(nmeshes > 1){
      meshdata *mesh_smoke3d;

      mesh_smoke3d = meshinfo + smoke3di->blocknumber;
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
    case UNKNOWN:
      // compression type not determined yet
      break;
    case RLE:
      STRCAT(smoke3di->menulabel," (RLE) ");
      break;
    case ZLIB:
      STRCAT(smoke3di->menulabel," (ZLIB) ");
      break;
    default:
      ASSERT(FFALSE);
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
    n = nmeshes;
  }
  for(i = 0;i<n;i++){
    meshdata *meshi;

    meshi = meshinfo+i;
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

  update_makeiblank_smoke3d=0;
  for(i=0;i<nsmoke3dinfo;i++){
    smoke3ddata *smoke3di;
    meshdata *mesh_smoke3d;
    int ibar, jbar, kbar;
    int ijksize;

    smoke3di = smoke3dinfo + i;
    mesh_smoke3d = meshinfo + smoke3di->blocknumber;

    ibar = mesh_smoke3d->ibar;
    jbar = mesh_smoke3d->jbar;
    kbar = mesh_smoke3d->kbar;
    ijksize=(ibar+1)*(jbar+1)*(kbar+1);

    if(use_iblank==1&&smoke3di->loaded==1&&mesh_smoke3d->iblank_smoke3d==NULL){
      unsigned char *iblank_smoke3d;

      NewMemory((void **)&iblank_smoke3d, ijksize*sizeof(unsigned char));
      mesh_smoke3d->iblank_smoke3d=iblank_smoke3d;
    }
  }

  for(ic=nmeshes-1;ic>=0;ic--){
    meshdata *mesh_smoke3d;
    unsigned char *iblank_smoke3d;
    float *xplt, *yplt, *zplt;
    float dx, dy, dz;
    int nx, ny, nxy;
    int ibar, jbar, kbar;
    int ijksize;
    int j, k;

    mesh_smoke3d = meshinfo + ic;
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

#ifdef pp_SPECTRAL
/* ------------------ SpectralDensity ------------------------ */

double SpectralDensity(double  temperature, double lambda){
  double val;
  const double c1 = 1.19104e-16, c2 = 0.014387774;
  float arg1, AA;

  // temperature K
  // lambda m
  // c1  =  2*h*c^2  m^4 kg/s^3   W m^2
  // c2 - hc/kB  m K
  // where
  // c== speed of light = 299792458 m/s
  // h== plank constant = 6.62607 * 10^-34 m^2 kg/s
  // kB == boltzman constant = 1.38065*10^-23 m^2kg/(s^2 K )
  // compute: spectral_density(plank's law)=(c1/lambda^5)*(1.0/(exp(c2/(lambda*T))-1.0) = (c1/lambda^5)*A/(1-A)
  // where A=exp(-c2/(lambda*T))

  arg1 = c2/(lambda*temperature);
  AA = exp(-arg1);
  val = c1 / pow(lambda, 5.0);
  val *= (AA / (1.0 - AA));
  return val;
}

/* ------------------ GetBlackBodyColors ------------------------ */

#define NVALS 89
void GetBlackBodyColors(float tmin, float tmax, float *intensities, int n){
  int i;
  float dtemp=0.0;
  float dlambda;
  float lambda_min = 390.0;
  float lambda_max = 830.0;

  if(intensities==NULL)NewMemory((void **)&intensities, 4*n*sizeof(float));

  // https://en.wikipedia.org/wiki/CIE_1931_color_space#Color_matching_functions
  // 390 nm to 830 nm
    float xhat[NVALS] = {
      2.95E-03,7.64E-03,1.88E-02,4.20E-02,8.28E-02,1.40E-01,2.08E-01,2.69E-01,3.28E-01,3.69E-01,
      4.03E-01,4.04E-01,3.93E-01,3.48E-01,3.01E-01,2.53E-01,1.91E-01,1.28E-01,7.59E-02,3.84E-02,
      1.40E-02,3.45E-03,5.65E-03,1.56E-02,3.78E-02,7.54E-02,1.20E-01,1.76E-01,2.38E-01,3.05E-01,
      3.84E-01,4.63E-01,5.37E-01,6.23E-01,7.12E-01,8.02E-01,8.93E-01,9.72E-01,1.03E+00,1.11E+00,
      1.15E+00,1.16E+00,1.15E+00,1.11E+00,1.05E+00,9.62E-01,8.63E-01,7.60E-01,6.41E-01,5.29E-01,
      4.32E-01,3.50E-01,2.71E-01,2.06E-01,1.54E-01,1.14E-01,8.28E-02,5.95E-02,4.22E-02,2.95E-02,
      2.03E-02,1.41E-02,9.82E-03,6.81E-03,4.67E-03,3.19E-03,2.21E-03,1.52E-03,1.06E-03,7.40E-04,
      5.15E-04,3.63E-04,2.56E-04,1.81E-04,1.29E-04,9.17E-05,6.58E-05,4.71E-05,3.41E-05,2.47E-05,
      1.79E-05,1.31E-05,9.57E-06,7.04E-06,5.17E-06,3.82E-06,2.84E-06,2.11E-06,1.58E-06
    };

    float yhat[NVALS] = {
      4.08E-04,1.08E-03,2.59E-03,5.47E-03,1.04E-02,1.71E-02,2.58E-02,3.53E-02,4.70E-02,6.05E-02,
      7.47E-02,8.82E-02,1.04E-01,1.20E-01,1.41E-01,1.70E-01,2.00E-01,2.31E-01,2.68E-01,3.11E-01,
      3.55E-01,4.15E-01,4.78E-01,5.49E-01,6.25E-01,7.01E-01,7.79E-01,8.38E-01,8.83E-01,9.23E-01,
      9.67E-01,9.89E-01,9.91E-01,1.00E+00,9.94E-01,9.85E-01,9.64E-01,9.29E-01,8.78E-01,8.37E-01,
      7.87E-01,7.27E-01,6.63E-01,5.97E-01,5.28E-01,4.60E-01,3.95E-01,3.35E-01,2.75E-01,2.22E-01,
      1.78E-01,1.41E-01,1.08E-01,8.14E-02,6.03E-02,4.43E-02,3.21E-02,2.30E-02,1.63E-02,1.14E-02,
      7.80E-03,5.43E-03,3.78E-03,2.62E-03,1.80E-03,1.23E-03,8.50E-04,5.88E-04,4.10E-04,2.86E-04,
      1.99E-04,1.41E-04,9.93E-05,7.04E-05,5.02E-05,3.58E-05,2.57E-05,1.85E-05,1.34E-05,9.72E-06,
      7.07E-06,5.16E-06,3.79E-06,2.79E-06,2.06E-06,1.52E-06,1.14E-06,8.48E-07,6.35E-07
    };

    float zhat[NVALS] = {
      1.32E-02,3.42E-02,8.51E-02,1.93E-01,3.83E-01,6.57E-01,9.93E-01,1.31E+00,1.62E+00,1.87E+00,
      2.08E+00,2.13E+00,2.13E+00,1.95E+00,1.77E+00,1.58E+00,1.31E+00,1.01E+00,7.52E-01,5.55E-01,
      3.98E-01,2.91E-01,2.08E-01,1.39E-01,8.85E-02,5.82E-02,3.78E-02,2.43E-02,1.54E-02,9.75E-03,
      6.08E-03,3.77E-03,2.32E-03,1.43E-03,8.78E-04,5.41E-04,3.34E-04,2.08E-04,1.30E-04,8.18E-05,
      5.21E-05,3.35E-05,2.18E-05,1.43E-05,9.53E-06,6.43E-06,0.00E+00,0.00E+00,0.00E+00,0.00E+00,
      0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,
      0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,
      0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,
      0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00,0.00E+00
    };

  tmin += 273.15E0; // convert from C to K
  tmax += 273.15E0;
  lambda_min /= 1.0E9;
  lambda_max /= 1.0E9;
  dlambda = (lambda_max-lambda_min)/(float)(NVALS-1);
  if(n>1)dtemp = (tmax-tmin)/(float)n;
  for(i = 0;i<n;i++){
    float temp, vals[4];
    int j;

    temp = tmin+i*dtemp;

    vals[0] = 0.0;
    vals[1] = 0.0;
    vals[2] = 0.0;
    for(j = 0;j<NVALS;j++){      // trapezoidal rule
      float lambda, intensity;

      lambda = lambda_min+(float)j*dlambda;
      intensity = SpectralDensity((double)temp, (double)lambda);
      if(j==0||j==NVALS-1){
        vals[0] += xhat[j]*intensity/2.0;
        vals[1] += yhat[j]*intensity/2.0;
        vals[2] += zhat[j]*intensity/2.0;
      }
      else{
        vals[0] += xhat[j]*intensity;
        vals[1] += yhat[j]*intensity;
        vals[2] += zhat[j]*intensity;
      }
    }
    vals[0] *= dlambda;
    vals[1] *= dlambda;
    vals[2] *= dlambda;

    vals[3] = MAX3(vals[0], vals[1], vals[2]);
    if(vals[3]!=0.0){
      vals[0] /= vals[3]/255.0;
      vals[1] /= vals[3]/255.0;
      vals[2] /= vals[3]/255.0;
    }

    intensities[4*i+0] = vals[0];
    intensities[4*i+1] = vals[1];
    intensities[4*i+2] = vals[2];
    intensities[4*i+3] = vals[3];
#ifdef _DEBUG
    printf("%f %f %f %f %f\n", temp-273.15, vals[0], vals[1], vals[2], vals[3]);
#endif
  }
}
#endif
