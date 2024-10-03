#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include GLUT_H

#include "smokeviewvars.h"
#include "compress.h"
#include "IOscript.h"
#include "getdata.h"

#define FIRST_TIME 1

#define IJKBF(i,j) ((i)*ncol+(j))
#define BOUNDCONVERT(index, valmin, valmax) (patchi->compression_type==UNCOMPRESSED ? \
               ( valmin == valmax ? 0.0 : (patchvals[index]-valmin)/(valmax-valmin) ) : \
               (float)cpatchvals[index]/255 \
               )

/* ------------------ OutputBoundaryData ------------------------ */

void OutputBoundaryData(char *csvfile, patchdata *patchi, int first_time, float *csvtime){
  int iframe;
  float *vals;
  float *xplt, *yplt, *zplt;
  FILE *csvstream=NULL;
  int max_frame;
  char *patchfile;
  meshdata *meshi;

  patchfile = patchi->file;
  meshi = meshinfo + patchi->blocknumber;

  if(patchout_tmin > patchout_tmax)return;
  if(first_time== FIRST_TIME){
    csvstream = FOPEN_2DIR(csvfile, "w");
  }
  else{
    csvstream = FOPEN_2DIR(csvfile, "a");
  }
  if(csvstream==NULL)return;
  if(first_time==FIRST_TIME){
    fprintf(csvstream,"%s\n",patchfile);
    fprintf(csvstream,"time interval:,%f,%f\n",patchout_tmin,patchout_tmax);
    fprintf(csvstream,"region:,%f,%f,%f,%f,%f,%f\n\n",patchout_xmin,patchout_xmax,patchout_ymin,patchout_ymax,patchout_zmin,patchout_zmax);
  }

  vals = meshi->patchval;
  xplt = meshi->xplt_orig;
  yplt = meshi->yplt_orig;
  zplt = meshi->zplt_orig;

  if(csvtime == NULL){
    max_frame = meshi->maxtimes_boundary;
  }
  else{
    max_frame = 1;
  }

  for(iframe=0;iframe<max_frame;iframe++){
    int ipatch;
    float pt;

    if(csvtime == NULL){
      pt = meshi->patch_times[iframe];
    }
    else{
      pt = *csvtime;
    }
    if(pt<patchout_tmin||pt>patchout_tmax){
      vals+=meshi->npatchsize;
      continue;
    }


    for(ipatch=0;ipatch<patchi->npatches;ipatch++){
      int i1, i2, j1, j2, k1, k2;
      int imin, imax, jmin, jmax, kmin, kmax;
      int i, j, k;
      patchfacedata *pfi;

      pfi = patchi->patchfaceinfo + ipatch;

      i1 = pfi->ib[0];
      i2 = pfi->ib[1];
      j1 = pfi->ib[2];
      j2 = pfi->ib[3];
      k1 = pfi->ib[4];
      k2 = pfi->ib[5];
      if(patchout_xmin<patchout_xmax&&(patchout_xmax<xplt[i1]||patchout_xmin>xplt[i2]))continue;
      if(patchout_ymin<patchout_ymax&&(patchout_ymax<yplt[j1]||patchout_ymin>yplt[j2]))continue;
      if(patchout_zmin<patchout_zmax&&(patchout_zmax<zplt[k1]||patchout_zmin>zplt[k2]))continue;
      imin=i1;
      imax=i2;
      jmin=j1;
      jmax=j2;
      kmin=k1;
      kmax=k2;
      for(i=0;i<meshi->ibar;i++){
        if(xplt[i]<=patchout_xmin&&patchout_xmin<=xplt[i+1])imin=i;
        if(xplt[i]<=patchout_xmax&&patchout_xmax<=xplt[i+1])imax=i;
      }
      for(j=0;j<meshi->jbar;j++){
        if(yplt[j]<=patchout_ymin&&patchout_ymin<=yplt[j+1])jmin=j;
        if(yplt[j]<=patchout_ymax&&patchout_ymax<=yplt[j+1])jmax=j;
      }
      for(k=0;k<meshi->kbar;k++){
        if(zplt[k]<=patchout_zmin&&patchout_zmin<=zplt[k+1])kmin=k;
        if(zplt[k]<=patchout_zmax&&patchout_zmax<=zplt[k+1])kmax=k;
      }

      fprintf(csvstream,"time:,%f,patch %i, of, %i\n",pt,ipatch+1,patchi->npatches);
      fprintf(csvstream,"region:,%i,%i,%i,%i,%i,%i\n",i1,i2,j1,j2,k1,k2);
      fprintf(csvstream,",%f,%f,%f,%f,%f,%f\n\n",xplt[i1],xplt[i2],yplt[j1],yplt[j2],zplt[k1],zplt[k2]);
      if(i1==i2){
        for(k=k1;k<=k2;k++){
          int out;

          if(k==k1){
            fprintf(csvstream,"Z\\Y,");
            for(j=jmin;j<=jmax;j++){
              fprintf(csvstream,"%f,",yplt[j]);
            }
            fprintf(csvstream,"\n");
          }
          if(k>=kmin&&k<=kmax){
            fprintf(csvstream,"%f,",zplt[k]);
          }

          out=0;
          for(j=j1;j<=j2;j++){
            if(k>=kmin&&k<=kmax&&j>=jmin&&j<=jmax){
              fprintf(csvstream,"%f,",*vals);
              out=1;
            }
            vals++;
          }
          if(out==1)fprintf(csvstream,"\n");
        }
      }
      else if(j1==j2){
        for(k=k1;k<=k2;k++){
          int out;

          if(k==k1){
            fprintf(csvstream,"Z\\X,");
            for(i=imin;i<=imax;i++){
              fprintf(csvstream,"%f,",xplt[i]);
            }
            fprintf(csvstream,"\n");
          }
          if(k>=kmin&&k<=kmax){
            fprintf(csvstream,"%f,",zplt[k]);
          }

          out=0;
          for(i=i1;i<=i2;i++){
            if(k>=kmin&&k<=kmax&&i>=imin&&i<=imax){
              fprintf(csvstream,"%f,",*vals);
              out=1;
            }
            vals++;
          }
          if(out==1)fprintf(csvstream,"\n");
        }
      }
      else{
        for(j=j1;j<j2;j++){
          int out;

          if(j==j1){
            fprintf(csvstream,"Y\\X,");
            for(i=imin;i<=imax;i++){
              fprintf(csvstream,"%f,",xplt[i]);
            }
            fprintf(csvstream,"\n");
          }
          if(j>=jmin&&j<=jmax){
            fprintf(csvstream,"%f,",yplt[j]);
          }

          out=0;
          for(i=i1;i<=i2;i++){
            if(i>=imin&&i<=imax&&j>=jmin&&j<=jmax){
              fprintf(csvstream,"%f,",*vals);
              out=1;
            }
            vals++;
          }
          if(out==1)fprintf(csvstream,"\n");
        }
      }
    }
  }
  fclose(csvstream);

}

/* ------------------ GetBoundaryIndex ------------------------ */

int GetBoundaryIndex(const patchdata *patchi){
  int j;

  for(j = 0; j < nboundarytypes; j++){
    patchdata *patchi2;

    patchi2 = patchinfo + boundarytypes[j];
    if(strcmp(patchi->label.shortlabel, patchi2->label.shortlabel) == 0)return boundarytypes[j];
  }
  return -1;
}

/* ------------------ InitVentColors ------------------------ */

void InitVentColors(void){
  int i;

  nventcolors = 0;
  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;
    int j;

    meshi = meshinfo + i;
    for(j = 0; j<meshi->nvents; j++){
      ventdata *venti;

      venti = meshi->ventinfo + j;
      if(venti->vent_id>nventcolors)nventcolors = venti->vent_id;
    }
  }
  nventcolors++;
  NewMemory((void **)&ventcolors, nventcolors*sizeof(float *));
  for(i = 0; i < nventcolors; i++){
    ventcolors[i] = NULL;
  }
  ventcolors[0] = surfinfo->color;
  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;
    int j;

    meshi = meshinfo + i;
    for(j = 0; j < meshi->nvents; j++){
      ventdata *venti;
      int vent_id;

      venti = meshi->ventinfo + j;
      vent_id = CLAMP(venti->vent_id, 1, nventcolors - 1);
      if(venti->useventcolor == 1){
        ventcolors[vent_id] = venti->color;
      }
      else{
        ventcolors[vent_id] = venti->surf[0]->color;
      }
    }
    for(j = 0; j < meshi->ncvents; j++){
      cventdata *cventi;
      int cvent_id;

      cventi = meshi->cventinfo + j;
      cvent_id = CLAMP(cventi->cvent_id, 1, nventcolors - 1);
      if(cventi->useventcolor == 1){
        ventcolors[cvent_id] = cventi->color;
      }
      else{
        ventcolors[cvent_id] = cventi->surf[0]->color;
      }
    }
  }

}

/* ------------------ NodeInBlockage ------------------------ */

int NodeInBlockage(const meshdata *meshnode, int i, int j, int k, int *imesh, int *iblockage){
  int ii;
  float xn, yn, zn;

  xn = meshnode->xplt[i];
  yn = meshnode->yplt[j];
  zn = meshnode->zplt[k];

  *imesh = -1;

  for(ii = 0; ii < nmeshes; ii++){
    int jj;
    meshdata *meshii;
    blockagedata *bc;
    float xm_min, xm_max;
    float ym_min, ym_max;
    float zm_min, zm_max;
    float xb_min, xb_max;
    float yb_min, yb_max;
    float zb_min, zb_max;
    float *xplt, *yplt, *zplt;

    meshii = meshinfo + ii;
    if(meshnode == meshii)continue;

    xplt = meshii->xplt;
    yplt = meshii->yplt;
    zplt = meshii->zplt;

    xm_min = xplt[0];
    xm_max = meshii->xyz_bar[XXX];
    ym_min = yplt[0];
    ym_max = meshii->xyz_bar[YYY];
    zm_min = zplt[0];
    zm_max = meshii->xyz_bar[ZZZ];
    if(xn<xm_min || xn>xm_max)continue;
    if(yn<ym_min || yn>ym_max)continue;
    if(zn<zm_min || zn>zm_max)continue;


    for(jj = 0; jj < meshii->nbptrs; jj++){
      bc = meshii->blockageinfoptrs[jj];
      if(bc->hole == 1)continue;
      xb_min = xplt[bc->ijk[0]];
      xb_max = xplt[bc->ijk[1]];
      yb_min = yplt[bc->ijk[2]];
      yb_max = yplt[bc->ijk[3]];
      zb_min = zplt[bc->ijk[4]];
      zb_max = zplt[bc->ijk[5]];
      if(xb_min <= xn&&xn <= xb_max&&
        yb_min <= yn&&yn <= yb_max&&
        zb_min <= zn&&zn <= zb_max){
        *imesh = ii;
        *iblockage = jj;
        return 1;
      }
    }
  }
  return 0;
}

/* ------------------ NodeInInternalVent ------------------------ */

int NodeInInternalVent(const meshdata *meshi, int i, int j, int k, int dir, int mesh_boundary, int option){
  int ii;
  int imesh, iblockage;

  if(option == 1)return YES;
  if(NodeInBlockage(meshi, i, j, k, &imesh, &iblockage) == YES)return YES;
  for(ii = 0; ii < meshi->nvents; ii++){
    ventdata *vi;

    vi = meshi->ventinfo + ii;
    if(vi->hideboundary == 1){
      switch(dir){
      case XDIR:
        if(vi->imin == i&&i == vi->imax&&
          vi->jmin < j&&j < vi->jmax&&
          vi->kmin < k&&k < vi->kmax){
          if((i == 0 && meshi->is_extface[0] == MESH_INT) || (i == meshi->ibar&&meshi->is_extface[1] == MESH_INT)){
            if(NodeInBlockage(meshi, i, j, k, &imesh, &iblockage) == 1)return YES;
          }
          return NO;
        }
        break;
      case YDIR:
        if(vi->jmin == j&&j == vi->jmax&&
          vi->imin < i&&i < vi->imax&&
          vi->kmin < k&&k < vi->kmax){
          if((j == 0 && meshi->is_extface[2] == MESH_INT) || (j == meshi->jbar&&meshi->is_extface[3] == MESH_INT)){
            if(NodeInBlockage(meshi, i, j, k, &imesh, &iblockage) == 1)return YES;
          }
          return NO;
        }
        break;
      case ZDIR:
        if(vi->kmin == k&&k == vi->kmax&&
          vi->imin < i&&i < vi->imax&&
          vi->jmin < j&&j < vi->jmax){
          if((k == 0 && meshi->is_extface[4] == MESH_INT) || (k == meshi->kbar&&meshi->is_extface[5] == MESH_INT)){
            if(NodeInBlockage(meshi, i, j, k, &imesh, &iblockage) == 1)return YES;
          }
          return NO;
        }
        break;
      default:
        assert(FFALSE);
        break;
      }
    }
  }
  if(mesh_boundary==YES)return NO;
  return YES;
}

/* ------------------ NodeInExternalVent ------------------------ */

void NodeInExternalVent(int ipatch, int *patchblank, const meshdata *meshi,
  int i1, int i2, int j1, int j2, int k1, int k2, int option){
  int ii, dir = 0;

  if(i1 == i2)dir = 1;
  if(j1 == j2)dir = 2;
  if(k1 == k2)dir = 3;

  for(ii = 0; ii<meshi->nvents; ii++){
    ventdata *vi;
    int imin, jmin, kmin, imax, jmax, kmax;
    int doit;

    vi = meshi->ventinfo+ii;

    if(vi->dir2 != dir)continue;

    doit = 0;
    if(show_open_boundary==1&&vi->isOpenvent==1)doit = 1;
    if(show_mirror_boundary==1&&vi->isMirrorvent==1)doit = 1;
    if(vi->hideboundary!=1||option!=0)doit = 1;
    if(doit==0)continue;

    switch(dir){
      int i, j, k;

    case XDIR:
      if(vi->imin != i1)continue;
      if(vi->jmax < j1)continue;
      if(vi->jmin > j2)continue;
      if(vi->kmax < k1)continue;
      if(vi->kmin > k2)continue;
      jmin = vi->jmin;
      jmax = vi->jmax;
      if(jmin < j1)jmin = j1;
      if(jmax > j2)jmax = j2;
      kmin = vi->kmin;
      kmax = vi->kmax;
      if(kmin < k1)kmin = k1;
      if(kmax > k2)kmax = k2;
      for(k = kmin; k <= kmax; k++){
        for(j = jmin; j <= jmax; j++){
          int iii;

          iii = (k - k1)*(j2 + 1 - j1) + (j - j1);
          patchblank[iii] = GAS;
        }
      }
      break;
    case YDIR:
      if(vi->jmin != j1)continue;
      if(vi->imax < i1)continue;
      if(vi->imin > i2)continue;
      if(vi->kmax < k1)continue;
      if(vi->kmin > k2)continue;
      imin = vi->imin;
      imax = vi->imax;
      if(imin < i1)imin = i1;
      if(imax > i2)imax = i2;
      kmin = vi->kmin;
      kmax = vi->kmax;
      if(kmin < k1)kmin = k1;
      if(kmax > k2)kmax = k2;
      for(k = kmin; k <= kmax; k++){
        for(i = imin; i <= imax; i++){
          int iii;

          iii = (k - k1)*(i2 + 1 - i1) + (i - i1);
          patchblank[iii] = GAS;
        }
      }
      break;
    case ZDIR:
      if(vi->kmin != k1)continue;
      if(vi->imax < i1)continue;
      if(vi->imin > i2)continue;
      if(vi->jmax < j1)continue;
      if(vi->jmin > j2)continue;
      imin = vi->imin;
      imax = vi->imax;
      if(imin < i1)imin = i1;
      if(imax > i2)imax = i2;
      jmin = vi->jmin;
      jmax = vi->jmax;
      if(jmin < j1)jmin = j1;
      if(jmax > j2)jmax = j2;
      for(j = jmin; j <= jmax; j++){
        for(i = imin; i <= imax; i++){
          int iii;

          iii = (j - j1)*(i2 + 1 - i1) + (i - i1);
          patchblank[iii] = GAS;
        }
      }
      break;
    default:
      assert(FFALSE);
      break;
    }
  }
  switch(dir){
    int i, j, k;

  case XDIR:
    for(k = k1; k <= k2; k++){
      for(j = j1; j <= j2; j++){
        int iii, imesh, iblockage;

        iii = (k - k1)*(j2 + 1 - j1) + (j - j1);
        if(patchblank[iii] == GAS)continue;
        patchblank[iii] = NodeInBlockage(meshi, i1, j, k, &imesh, &iblockage);
      }
    }
    break;
  case YDIR:
    for(k = k1; k <= k2; k++){
      for(i = i1; i <= i2; i++){
        int iii, imesh, iblockage;

        iii = (k - k1)*(i2 + 1 - i1) + (i - i1);
        if(patchblank[iii] == GAS)continue;
        patchblank[iii] = NodeInBlockage(meshi, i, j1, k, &imesh, &iblockage);
      }
    }
    break;
  case ZDIR:
    for(j = j1; j <= j2; j++){
      for(i = i1; i <= i2; i++){
        int iii, imesh, iblockage;

        iii = (j - j1)*(i2 + 1 - i1) + (i - i1);
        if(patchblank[iii] == GAS)continue;
        patchblank[iii] = NodeInBlockage(meshi, i, j, k1, &imesh, &iblockage);
      }
    }
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ DrawOnlyThreshold ------------------------ */

void DrawOnlyThreshold(const meshdata *meshi){
  int n, nn, nn1, nn2;
  int nrow, ncol, irow, icol;
  float *xyzpatchcopy;
  int *patchblankcopy;
  float *patch_times;
  float *xyzpatch;
  int *patchblank;
  patchdata *patchi;
  float *color11, *color12, *color21, *color22;
  float *color_black;

  if(vis_threshold==0||vis_onlythreshold==0||do_threshold==0)return;

  patch_times = meshi->patch_times;
  xyzpatch = meshi->xyzpatch_threshold;
  patchblank = meshi->patchblank;
  patchi = patchinfo+meshi->patchfilenum;
  switch(patchi->compression_type){
  case UNCOMPRESSED:
    assert(meshi->cpatchval_iframe!=NULL);
    break;
  case COMPRESSED_ZLIB:
    assert(meshi->cpatchval_iframe_zlib!=NULL);
    break;
  default:
    assert(FFALSE);
  }
  patchi = patchinfo+meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  nn = 0;
  color_black = &char_color[0];
  glBegin(GL_TRIANGLES);
  glColor4fv(color_black);
  for(n = 0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst!=NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    if(pfi->vis==1&&pfi->dir==0){
      nrow = pfi->nrow;
      ncol = pfi->ncol;
      xyzpatchcopy = xyzpatch+3*pfi->start;
      patchblankcopy = patchblank+pfi->start;
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy+3*irow*ncol;
        patchblank1 = patchblankcopy+irow*ncol;
        nn1 = nn+irow*ncol;
        xyzp2 = xyzp1+3*ncol;
        patchblank2 = patchblank1+ncol;
        nn2 = nn1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            int nnulls;

            nnulls = 4;
            color11 = NULL;
            color12 = NULL;
            color21 = NULL;
            color22 = NULL;
            if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol]){
              color11 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1]){
              color12 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn2+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol]){
              color21 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1]){
              color22 = &char_color[0];
              nnulls--;
            }

            if(nnulls==0){
              glVertex3fv(xyzp1);
              glVertex3fv(xyzp1+3);
              glVertex3fv(xyzp2+3);

              glVertex3fv(xyzp1);
              glVertex3fv(xyzp2+3);
              glVertex3fv(xyzp2);
            }
            if(nnulls==1){
              if(color11!=NULL)glVertex3fv(xyzp1);
              if(color12!=NULL)glVertex3fv(xyzp1+3);
              if(color22!=NULL)glVertex3fv(xyzp2+3);
              if(color21!=NULL)glVertex3fv(xyzp2);
            }
          }
          patchblank1++; patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  glBegin(GL_TRIANGLES);
  glColor4fv(color_black);
  nn = 0;
  for(n = 0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst!=NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    if(pfi->vis==1&&pfi->dir>0){
      nrow = pfi->nrow;
      ncol = pfi->ncol;
      xyzpatchcopy = xyzpatch+3*pfi->start;
      patchblankcopy = patchblank+pfi->start;
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy+3*irow*ncol;
        patchblank1 = patchblankcopy+irow*ncol;
        nn1 = nn+irow*ncol;

        xyzp2 = xyzp1+3*ncol;
        patchblank2 = patchblank1+ncol;
        nn2 = nn1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            int nnulls;

            color11 = NULL;
            color12 = NULL;
            color21 = NULL;
            color22 = NULL;
            nnulls = 4;

            if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol]){
              color11 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1]){
              color12 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn2+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol]){
              color21 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1]){
              color22 = &char_color[0];
              nnulls--;
            }

            if(nnulls==0){
              glVertex3fv(xyzp1);
              glVertex3fv(xyzp1+3);
              glVertex3fv(xyzp2+3);

              glVertex3fv(xyzp1);
              glVertex3fv(xyzp2+3);
              glVertex3fv(xyzp2);
            }
            if(nnulls==1){
              if(color11!=NULL)glVertex3fv(xyzp1);
              if(color12!=NULL)glVertex3fv(xyzp1+3);
              if(color22!=NULL)glVertex3fv(xyzp2+3);
              if(color21!=NULL)glVertex3fv(xyzp2);
            }
          }
          patchblank1++; patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn = 0;
  for(n = 0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst != NULL && pfi->meshinfo != NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    if(pfi->vis==1&&pfi->dir<0){
      nrow = pfi->nrow;
      ncol = pfi->ncol;
      xyzpatchcopy = xyzpatch+3*pfi->start;
      patchblankcopy = patchblank+pfi->start;
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy+3*irow*ncol;
        patchblank1 = patchblankcopy+irow*ncol;
        nn1 = nn+irow*ncol;
        xyzp2 = xyzp1+3*ncol;
        patchblank2 = patchblank1+ncol;
        nn2 = nn1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            int nnulls;

            color11 = NULL;
            color12 = NULL;
            color21 = NULL;
            color22 = NULL;
            nnulls = 4;
            if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol]){
              color11 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1]){
              color12 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn2+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol]){
              color21 = &char_color[0];
              nnulls--;
            }
            if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1]){
              color22 = &char_color[0];
              nnulls--;
            }
            if(nnulls==0){
              glVertex3fv(xyzp1);
              glVertex3fv(xyzp2+3);
              glVertex3fv(xyzp1+3);

              glVertex3fv(xyzp1);
              glVertex3fv(xyzp2);
              glVertex3fv(xyzp2+3);
            }
            if(nnulls==1){
              if(color21!=NULL)glVertex3fv(xyzp2);
              if(color22!=NULL)glVertex3fv(xyzp2+3);
              if(color12!=NULL)glVertex3fv(xyzp1+3);
              if(color11!=NULL)glVertex3fv(xyzp1);
            }
          }
          patchblank1++; patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }
  glEnd();
}

/* ------------------ GetBoundaryDataZlib ------------------------ */

void GetBoundaryDataZlib(patchdata *patchi, unsigned char *data, int ndata,
  float *local_times, unsigned int *zipoffset, unsigned int *zipsize, int ntimes_local){
  FILE_m *stream;
  float local_time;
  unsigned int compressed_size;
  int npatches;
  int version;
  int return_code;
  int i;
  int local_skip;
  unsigned char *datacopy;
  unsigned int offset;
  int local_count;
  float time_max;

  // endian
  // completion (0/1)
  // fileversion (compressed format)
  // version  (bndf version)
  // global min max (used to perform conversion)
  // local min max  (min max found for this file)
  // npatch
  // i1,i2,j1,j2,k1,k2,idir,dummy,dummy (npatch times)
  // time
  // compressed size of frame
  // compressed buffer


#ifdef pp_BOUNDFRAME
  unsigned char *buffer=NULL;
  int nbuffer=0;

  if(patchi->frameinfo!=NULL&&patchi->frameinfo->bufferinfo!=NULL){
    buffer  = patchi->frameinfo->bufferinfo->buffer;
    nbuffer = patchi->frameinfo->bufferinfo->nbuffer;
  }
  stream = fopen_b((char *)patchi->file, buffer, nbuffer, "rb");
#else
  stream = fopen_m((char *)patchi->file, "rb");
#endif
  if(stream==NULL)return;

  fseek_m(stream, 12, SEEK_CUR);
  fread_m(&version, 4, 1, stream);
  fseek_m(stream, 16, SEEK_CUR);
  fread_m(&npatches, 4, 1, stream);
  if(version==0){
    local_skip = 6*npatches*4;
  }
  else{
    local_skip = 9*npatches*4;
  }
  return_code = fseek_m(stream, local_skip, SEEK_CUR);
  if(return_code!=0){
    fclose_m(stream);
    return;
  }
  datacopy = data;
  offset = 0;
  local_count = -1;
  i = -1;
  time_max = -1000000.0;
  CheckMemory;
  for(;;){
    int skip_frame;

    if(fread_m(&local_time, 4, 1, stream)!=1)break;
    skip_frame = 1;
    if(local_time>time_max){
      time_max = local_time;
      skip_frame = 0;
      local_count++;
    }
    if(fread_m(&compressed_size, 4, 1, stream) != 1)break;
    if(skip_frame==0&&local_count%tload_step==0){
      int count;

      count = fread_m(datacopy, 1, compressed_size, stream);
      if(count != compressed_size)break;
    }
    else{
      if(fseek_m(stream, compressed_size, SEEK_CUR) != 0)break;
    }

    if(skip_frame==1||local_count%tload_step!=0)continue;
    i++;
    if(i>=ntimes_local)break;
    assert(i<ntimes_local);
    local_times[i] = local_time;
    zipoffset[i] = offset;
    zipsize[i] = compressed_size;
    datacopy += compressed_size;
    offset += compressed_size;
  }
#ifndef pp_BOUNDFRAME
  fclose_m(stream);
#endif
}
/* ------------------ GetBoundaryHeader ------------------------ */

void GetBoundaryHeader(char *file, int *npatches, float *ppatchmin, float *ppatchmax){
  FILE *stream;
  float minmax[2];

  stream = fopen(file, "rb");

  if(stream==NULL)return;

  // endian
  // completion (0/1)
  // fileversion (compressed format)
  // version  (bndf version)
  // global min max (used to perform conversion)
  // local min max  (min max found for this file)
  // npatch
  // i1,i2,j1,j2,k1,k2,idir,dummy,dummy (npatch times)
  // time
  // compressed size of frame
  // compressed buffer

  FSEEK(stream, 16, SEEK_CUR);

  fread(minmax, 4, 2, stream);
  FSEEK(stream, 8, SEEK_CUR);
  fread(npatches, 4, 1, stream);
  *ppatchmin = minmax[0];
  *ppatchmax = minmax[1];
  fclose(stream);
}

/* ------------------ GetBoundaryHeader2 ------------------------ */

void GetBoundaryHeader2(char *file, patchfacedata *patchfaceinfo, int nmeshes_arg, int nobsts_arg){
  int i;
  int buffer[9];
  FILE *stream;
  int npatches;

  // endian
  // completion (0/1)
  // fileversion (compressed format)
  // version  (bndf version)
  // global min max (used to perform conversion)
  // local min max  (min max found for this file)
  // npatch
  // i1,i2,j1,j2,k1,k2,idir,dummy,dummy (npatch times)
  // time
  // compressed size of frame
  // compressed buffer

  stream = fopen(file, "rb");

  if(stream==NULL)return;

  FSEEK(stream, 12, SEEK_CUR);
  int version;
  fread(&version, 4, 1, stream);
  FSEEK(stream, 16, SEEK_CUR);
  fread(&npatches, 4, 1, stream);
  for(i = 0;i<npatches;i++){
    int obst_index, mesh_index;

    buffer[6] = 0;
    fread(buffer, 4, 9, stream);
    memcpy(patchfaceinfo->ib, buffer, 6*sizeof(int));
    patchfaceinfo->dir = buffer[6];
    obst_index = buffer[7] - 1;
    mesh_index = buffer[8] - 1;
    patchfaceinfo->obst_index = obst_index;
    patchfaceinfo->mesh_index = mesh_index;
    patchfaceinfo->meshinfo = NULL;
    patchfaceinfo->obst     = NULL;
    if(mesh_index >= 0 && mesh_index < nmeshes_arg)patchfaceinfo->meshinfo = meshinfo + mesh_index;
    if(patchfaceinfo->meshinfo != NULL && obst_index>=0 && obst_index<nobsts_arg)patchfaceinfo->obst = patchfaceinfo->meshinfo->blockageinfoptrs[obst_index];
  }
  fclose(stream);
}

/* ------------------ GetBoundarySizeInfo ------------------------ */

void GetBoundarySizeInfo(patchdata *patchi, int *nframes, int *buffersize){
  FILE *streamsize;
  FILE *stream;
  int nf = 0, bsize = 0;
  float local_time;
  char buffer[255];
  int full_size, compressed_size;
  int npatches;
  int buff[9];
  int version;
  int size;
  size_t return_code;
  int i;
  char sizefile[1024];
  int local_count;
  float time_max;

  // endian
  // completion (0/1)
  // fileversion (compressed format)
  // version  (bndf version)
  // global min max (used to perform conversion)
  // local min max  (min max found for this file)
  // npatch
  // i1,i2,j1,j2,k1,k2,idir,dummy,dummy (npatch times)
  // time
  // compressed size of frame
  // compressed buffer

  strcpy(sizefile, patchi->size_file);
  strcat(sizefile, ".szz");
  streamsize = FOPEN_2DIR(sizefile, "r");
  if(streamsize==NULL){
    *nframes = 0;
    *buffersize = 0;

    strcpy(sizefile, patchi->size_file);
    strcat(sizefile, ".sz");

    stream = fopen(patchi->file, "rb");
    if(stream==NULL){
      if(streamsize!=NULL)fclose(streamsize);
      return;
    }

    streamsize = FOPEN_2DIR(sizefile, "w");
    if(streamsize==NULL){
      fclose(stream);
      return;
    }

    FSEEK(stream, 12, SEEK_CUR);
    fread(&version, 4, 1, stream);
    FSEEK(stream, 16, SEEK_CUR);
    fread(&npatches, 4, 1, stream);
    size = 0;
    return_code = 0;
    for(i = 0;i<npatches;i++){
      if(version==0){
        return_code = fread(buff, 4, 6, stream);
      }
      else{
        return_code = fread(buff, 4, 9, stream);
      }
      if(return_code==0)break;
      size += (buff[1]+1-buff[0])*(buff[3]+1-buff[2])*(buff[5]+1-buff[4]);
    }
    if(return_code==0){
      fclose(stream);
      fclose(streamsize);
      return;
    }
    for(;;){
      return_code = fread(&local_time, 4, 1, stream);
      if(return_code==0)break;
      return_code = fread(&compressed_size, 4, 1, stream);
      if(return_code==0)break;
      return_code = FSEEK(stream, compressed_size, SEEK_CUR);
      if(return_code!=0)break;
      fprintf(streamsize, "%f %i %i\n", local_time, size, compressed_size);
    }
    fclose(stream);
    fclose(streamsize);
    streamsize = FOPEN_2DIR(sizefile, "r");
    if(streamsize==NULL)return;
  }

  local_count = -1;
  time_max = -1000000.0;
  for(;;){
    int frame_skip;

    if(fgets(buffer, 255, streamsize)==NULL)break;

    sscanf(buffer, "%f %i %i", &local_time, &full_size, &compressed_size);
    frame_skip = 1;
    if(local_time>time_max){
      time_max = local_time;
      frame_skip = 0;
    }
    if(frame_skip==1)continue;

    local_count++;
    if(local_count%tload_step!=0)continue;

    nf++;
    bsize += compressed_size;
  }
  *nframes = nf;
  *buffersize = bsize;
  fclose(streamsize);
}

/* ------------------ ComputeLoadedPatchHist ------------------------ */

void ComputeLoadedPatchHist(char *label, histogramdata **histptr, float *global_min, float *global_max){
  histogramdata *hist;
  int i, have_data=0;


  for(i = 0; i<npatchinfo; i++){
    patchdata *patchi;

    patchi = patchinfo+i;
    if(patchi->loaded==0||strcmp(patchi->label.shortlabel, label)!=0)continue;
    if(patchi->blocknumber>=0){
      if(patchi->patch_filetype==PATCH_STRUCTURED_NODE_CENTER||patchi->patch_filetype==PATCH_STRUCTURED_CELL_CENTER){
        have_data = 1;
        break;
      }
    }
    if(patchi->patch_filetype==PATCH_GEOMETRY_BOUNDARY){
      have_data = 1;
      break;
    }
  }
  if(have_data==0)return;

  hist = *histptr;
  if(*histptr!=NULL)FreeHistogram(*histptr);
  NewMemory((void **)&hist, sizeof(histogramdata));
  *histptr = hist;

  InitHistogram(hist, NHIST_BUCKETS, global_min, global_max);
  for(i = 0; i<npatchinfo; i++){
    patchdata *patchi;

    patchi = patchinfo+i;
    if(patchi->loaded==0||strcmp(patchi->label.shortlabel, label)!=0)continue;
    switch(patchi->patch_filetype){
      case PATCH_STRUCTURED_NODE_CENTER:
      case PATCH_STRUCTURED_CELL_CENTER:
        if(patchi->blocknumber>=0){
          int npatchvals;
          meshdata *meshi;

          meshi = meshinfo+patchi->blocknumber;
          npatchvals = patchi->ntimes*meshi->npatchsize;
          MergeVals2Histogram(meshi->patchval, NULL, NULL, npatchvals, hist);
        }
        break;
      case PATCH_GEOMETRY_BOUNDARY:
        MergeVals2Histogram(patchi->geom_vals, NULL, NULL, patchi->geom_nvals, hist);
        break;
      case PATCH_GEOMETRY_SLICE:
        continue;
        break;
      default:
	assert(FFALSE);
	break;
    }
  }
}

/* ------------------ GetPatchNTimes ------------------------ */

int GetPatchNTimes(char *file){
  FILE *stream;

  if(file == NULL) return 0;
  stream = fopen(file, "r");
  if(stream == NULL) return 0;

  int count = 0;
  char buffer[255];
  for(;;){

    if(fgets(buffer, 255, stream) == NULL) break;
    count++;
  }
  fclose(stream);
  return count;
}

// !  ------------------ GetPatchSizes1 ------------------------

void GetPatchSizes1(FILE_m **stream, const char *patchfilename, unsigned char *buffer, int nbuffer, int *npatch, int *headersize, int *error){

  *error = 0;
  assert(stream !=NULL);
  if(stream == NULL){
    printf("***Error: null pointer in getpatchsizes1 routine\n");
    *error = 1;
    return;
  }
#ifdef pp_BOUNDFRAME
  *stream = fopen_b((char *)patchfilename, buffer, nbuffer, "rb");
#else
  *stream = fopen_m((char *)patchfilename, "rb");
#endif
  if(*stream == NULL){
    printf(" The boundary file , %s, failed to open\n", patchfilename);
    *error = 1;
    return;
  }

  // skip over long, short and unit labels (each 30 characters in length);
  *error = fseek_m(*stream, 4+30+4, SEEK_SET);
  *error = fseek_m(*stream, 4+30+4, SEEK_CUR);
  *error = fseek_m(*stream, 4+30+4, SEEK_CUR);
  if(*error == 0){
    fseek_m(*stream, 4, SEEK_CUR); fread_m(npatch, sizeof(*npatch), 1, *stream); fseek_m(*stream, 4, SEEK_CUR);
  }
  *headersize = 3 * (4 + 30 + 4) + 4 + 4 + 4;
  return;
}

// !  ------------------ GetPatchSizes2 ------------------------

void GetPatchSizes2(FILE_m *stream, int npatch, int nmeshes_arg, int nobsts_arg, int *npatchsize,
                    patchfacedata *patchfaceinfo, int *headersize, int *framesize){
  int ijkp[9] = {0};

  *npatchsize = 0;

  int n;
  for(n = 0; n < npatch; n++){
    patchfacedata *pfi;
    int obst_index, mesh_index;

    fseek_m(stream, 4, SEEK_CUR); fread_m(ijkp, sizeof(*ijkp), 9, stream); fseek_m(stream, 4, SEEK_CUR);
    pfi = patchfaceinfo + n;
    memcpy(pfi->ib, ijkp, 6 * sizeof(int));
    pfi->dir        = ijkp[6];
    obst_index = ijkp[7] - 1;
    mesh_index = ijkp[8] - 1;
    pfi->meshinfo = NULL;
    pfi->obst     = NULL;
    if(mesh_index >= 0 && mesh_index < nmeshes_arg)pfi->meshinfo = meshinfo + ijkp[8] - 1;
    if(pfi->meshinfo != NULL && obst_index >= 0 && obst_index < nobsts_arg)pfi->obst = pfi->meshinfo->blockageinfoptrs[obst_index];
    pfi->obst_index = obst_index;
    pfi->mesh_index = mesh_index;
    int i1 = ijkp[0];
    int i2 = ijkp[1];
    int j1 = ijkp[2];
    int j2 = ijkp[3];
    int k1 = ijkp[4];
    int k2 = ijkp[5];
    *npatchsize += (i2 + 1 - i1) * (j2 + 1 - j1) * (k2 + 1 - k1);
  }
  *headersize += npatch * (4 + 6 * 4 + 4);
  *headersize += npatch * 4;
  *framesize = 8 + 4 + 8 * npatch + (*npatchsize) * 4;

  return;
}
/* ------------------ DrawFace ------------------------ */

void DrawFace(float *v11, float *v22, int dir){
  float v12[3], v21[3];

  memcpy(v12, v11, 3*sizeof(float));
  memcpy(v21, v11, 3*sizeof(float));
  switch(dir){
  case 0:
  case 1:
    v12[1] = v22[1];
    v21[2] = v22[2];
    break;
  case 2:
  case 3:
    v12[0] = v22[0];
    v21[2] = v22[2];
    break;
  case 4:
  case 5:
    v12[0] = v22[0];
    v21[1] = v22[1];
    break;
  default:
    assert(0);
    break;
  }
  glVertex3fv(v11); glVertex3fv(v12); glVertex3fv(v22);
  glVertex3fv(v11); glVertex3fv(v22); glVertex3fv(v21);

  glVertex3fv(v11); glVertex3fv(v22); glVertex3fv(v12);
  glVertex3fv(v11); glVertex3fv(v21); glVertex3fv(v22);
}

/* ------------------ DrawMeshBlockFaces ------------------------ */

void DrawMeshBlockFaces(void){
  int i;
  float block_color[3] = {1.0, 0.5, .25};

  glBegin(GL_TRIANGLES);
  glColor3fv(block_color);
  for(i = 0;i < nmeshes;i++){
    meshdata *meshi;
    int j, iface;
    blockagedata **bclist;

    meshi = meshinfo + i;
    for(iface = 0; iface < 6; iface++){
      bclist = meshi->bc_faces[iface];
      for(j = 0; j < meshi->n_bc_faces[iface]; j++){
        blockagedata *bc;
        float v11[3], v22[3];

        bc = bclist[j];
        if(bc->showtimelist != NULL && bc->showtimelist[itimes] == 0)continue;
        v11[0] = bc->xmin;
        v11[1] = bc->ymin;
        v11[2] = bc->zmin;
        v22[0] = bc->xmax;
        v22[1] = bc->ymax;
        v22[2] = bc->zmax;
        if(iface == 0)v22[0] = v11[0];
        if(iface == 1)v11[0] = v22[0];
        if(iface == 2)v22[1] = v11[1];
        if(iface == 3)v11[1] = v22[1];
        if(iface == 4)v22[2] = v11[2];
        if(iface == 5)v11[2] = v22[2];
        DrawFace(v11, v22, iface);
      }
    }
  }
  glEnd();
}

/* ------------------ IsBoundPlane ------------------------ */

int IsBoundPlane(int imesh, patchfacedata *pfi){
  int is_plane = 0;
  meshdata *meshi;
  int i1, i2, j1, j2, k1, k2;

  if(imesh != boundary_debug_mesh - 1)return 0;

  meshi = meshinfo + imesh;
  i1 = pfi->ib[0];
  i2 = pfi->ib[1];
  j1 = pfi->ib[2];
  j2 = pfi->ib[3];
  k1 = pfi->ib[4];
  k2 = pfi->ib[5];
  if(boundary_debug_plane[0] == 1 && j1 == 0 && j2 == meshi->jbar && k1 == 0 && k2 == meshi->kbar && i1 == i2 && i1 == 0)is_plane = 1;
  if(boundary_debug_plane[1] == 1 && j1 == 0 && j2 == meshi->jbar && k1 == 0 && k2 == meshi->kbar && i1 == i2 && i1 == meshi->ibar)is_plane = 1;
  if(boundary_debug_plane[2] == 1 && i1 == 0 && i2 == meshi->ibar && k1 == 0 && k2 == meshi->kbar && j1 == j2 && j1 == 0)is_plane = 1;
  if(boundary_debug_plane[3] == 1 && i1 == 0 && i2 == meshi->ibar && k1 == 0 && k2 == meshi->kbar && j1 == j2 && j1 == meshi->jbar)is_plane = 1;
  if(boundary_debug_plane[4] == 1 && i1 == 0 && i2 == meshi->ibar && j1 == 0 && j2 == meshi->jbar && k1 == k2 && k1 == 0)is_plane = 1;
  if(boundary_debug_plane[5] == 1 && i1 == 0 && i2 == meshi->ibar && j1 == 0 && j2 == meshi->jbar && k1 == k2 && k1 == meshi->kbar)is_plane = 1;
  return is_plane;
}

/* ------------------ GetPatchData ------------------------ */

void GetPatchData(int imesh, FILE_m *stream, int npatch, patchfacedata *patchfaceinfo,
  float *patchtime, float *pqq,
  int *npqq, int *file_sizeptr, int *error){
  int size, ibeg;
  int file_size;
  int count;

  file_size = 0;
  *error = 0;
  fseek_m(stream, 4, SEEK_CUR); count = fread_m(patchtime, sizeof(*patchtime), 1, stream); fseek_m(stream, 4, SEEK_CUR);
  if(count != 1)*error = 1;
  file_size = file_size + 4;
  if(*error != 0) return;
  ibeg = 0;
  *npqq = 0;

  int i;
  for(i = 0; i < npatch; i++){
    patchfacedata *pfi;

    pfi = patchfaceinfo + i;
    size = (pfi->ib[1] + 1 - pfi->ib[0]) * (pfi->ib[3] + 1 - pfi->ib[2]) * (pfi->ib[5] + 1 - pfi->ib[4]);
    *npqq += size;
    fseek_m(stream, 4, SEEK_CUR); count = fread_m(&pqq[ibeg], sizeof(*pqq), size, stream); fseek_m(stream, 4, SEEK_CUR);

    int is_plane;

    is_plane = IsBoundPlane(imesh, pfi);
    if(is_plane==1){
      int j;

      for(j = 0;j < size;j++){
        printf("%f ", pqq[ibeg + j]);
        if(j % pfi->ncol == pfi->ncol-1)printf("\n");
      }
      printf("\n");
    }
    if(count != size)*error = 1;
    // TODO: hardcodes float size.
    file_size += 4 * size;
    if(*error != 0) break;
    ibeg += size;
  }
  *file_sizeptr = file_size;
  return;
}

/* ------------------ ReadBoundaryBndf ------------------------ */

FILE_SIZE ReadBoundaryBndf(int ifile, int load_flag, int *errorcode){
  int error;
  int patchfilenum;
  float *xyzpatchcopy;
  float *xyzpatch_ignitecopy;
  int *patchblankcopy;
  int n;
  int ii;
  int headersize, framesize;
  int statfile;
  STRUCTSTAT statbuffer;
  int nbb;
  int ibartemp,jbartemp,kbartemp;
  float *xplttemp,*yplttemp,*zplttemp;
  int blocknumber;
  patchdata *patchi;
  meshdata *meshi;
  float patchmin_global, patchmax_global;
  int local_first,nsize;
  int npatchvals;
  char patchcsvfile[1024];
  int framestart;
#ifdef pp_BOUNDFRAME
  int time_frame = ALL_FRAMES;
#endif

  int nn;
  int filenum;
  int ncompressed_buffer;
  char *file;
  float read_time, total_time;
  FILE_m *stream = NULL;
  int wallcenter=0;
  FILE_SIZE return_filesize = 0;

  CheckMemory;
  patchi = patchinfo + ifile;
  if(patchi->loaded==0&&load_flag==UNLOAD)return 0;
  if(strcmp(patchi->label.shortlabel,"wc")==0)wallcenter=1;

  if(output_patchdata==1){
    sprintf(patchcsvfile,"%s_bndf_%04i.csv",fdsprefix,ifile);
  }

  for(n = 0; n < 6; n++){
    patchi->meshfaceinfo[n] = NULL;
  }

  START_TIMER(total_time);
  local_first=1;
  CheckMemory;
  patchfilenum=ifile;
  file = patchi->file;
  blocknumber = patchi->blocknumber;
  highlight_mesh = blocknumber;
  meshi = meshinfo+blocknumber;
  UpdateCurrentMesh(meshi);
  if(load_flag!=RELOAD&&load_flag!=UNLOAD&&meshi->patchfilenum >= 0 && meshi->patchfilenum < npatchinfo){
    patchdata *patchold;

    patchold = patchinfo + meshi->patchfilenum;
    if(patchold->loaded == 1){
      int errorcode2;

      ReadBoundaryBndf(meshi->patchfilenum, UNLOAD, &errorcode2);
    }
  }
  meshi->patchfilenum = ifile;
  filenum = meshi->patchfilenum;

#ifndef pp_BOUNDFRAME
#ifndef pp_FSEEK
  if(load_flag==RELOAD)load_flag = LOAD;
#endif
#endif

  if(load_flag!=RELOAD&&filenum>=0&&filenum<npatchinfo){
    patchi->loaded=0;
  }

  patchi->display=0;
  plotstate=GetPlotState(DYNAMIC_PLOTS);

  nbb = meshi->nbptrs;
  if(nbb==0)nbb=1;
  updatefaces=1;
  *errorcode=0;
  if(load_flag != RELOAD){
    FREEMEMORY(meshi->xyzpatch);
    FREEMEMORY(meshi->xyzpatch_threshold);
    FREEMEMORY(meshi->thresholdtime);
    FREEMEMORY(meshi->patchblank);
    FREEMEMORY(meshi->zipoffset);
    FREEMEMORY(meshi->zipsize);
    FREEMEMORY(meshi->patchventcolors);
    FREEMEMORY(meshi->cpatchval);
    FREEMEMORY(meshi->cpatchval_zlib);
    FREEMEMORY(meshi->cpatchval_iframe_zlib);
    FREEMEMORY(meshi->patchval);
    FREEMEMORY(meshi->patch_times);
    FREEMEMORY(meshi->patch_times_map);
#ifdef pp_BOUNDFRAME
    FRAMEFree(patchi->frameinfo);
    patchi->frameinfo = NULL;
#endif
  }

  if(load_flag==UNLOAD){
    boundary_loaded = 0;
    if(boundary_interface_unhide == 1 && have_removable_obsts == 1 && nmeshes>1){
      BlockageMenu(visBLOCKAsInput);
    }
    UpdateBoundaryType();
    UpdateUnitDefs();
    update_times = 1;
    patchi->npatches=0;
    patchi->ntimes_old=0;
    patchi->ntimes=0;
    updatemenu=1;
    PrintMemoryInfo;
    return 0;
  }

#ifdef pp_BOUNDFRAME
  patchi->frameinfo = FRAMELoadData(patchi->frameinfo, patchi->file, load_flag, time_frame, FORTRAN_FILE, GetBoundaryFrameInfo);
  patchi->ntimes = patchi->frameinfo->nframes;
  NewMemory((void **)&meshi->patch_times, patchi->ntimes*sizeof(float));
  memcpy(meshi->patch_times, patchi->frameinfo->times, patchi->ntimes*sizeof(float));
  FRAMEGetMinMax(patchi->frameinfo);
  update_frame = 1;
#endif

  if(ifile>=0&&ifile<npatchinfo){
    Global2GLUIBoundaryBounds(patchi->label.shortlabel);
  }

  if(colorlabelpatch!=NULL){
    for(n=0;n<MAXRGB;n++){
      FREEMEMORY(colorlabelpatch[n]);
    }
    FREEMEMORY(colorlabelpatch);
  }
  patchi->extreme_max=0;
  patchi->extreme_min=0;
  ibartemp=meshi->ibar;
  jbartemp=meshi->jbar;
  kbartemp=meshi->kbar;
  xplttemp=meshi->xplt;
  yplttemp=meshi->yplt;
  zplttemp=meshi->zplt;
  do_threshold=0;

  if(activate_threshold==1){
    if(
      strncmp(patchi->label.shortlabel,"TEMP",4) == 0||
      strncmp(patchi->label.shortlabel,"temp",4) == 0
      ){
      do_threshold=1;
    }
  }
  CheckMemory;
  if(patchi->compression_type==UNCOMPRESSED){
    unsigned char *buffer = NULL;
    int nbuffer = 0;
#ifdef pp_BOUNDFRAME
    if(patchi->frameinfo != NULL && patchi->frameinfo->bufferinfo != NULL){
      buffer  = patchi->frameinfo->bufferinfo->buffer;
      nbuffer = patchi->frameinfo->bufferinfo->nbuffer;
    }
#endif
    GetPatchSizes1(&stream, file, buffer, nbuffer, &patchi->npatches, &headersize, &error);
    if(error!=0){
      ReadBoundary(ifile,UNLOAD,&error);
      *errorcode=1;
      return 0;
    }
  }
  else{
    patchi->npatches=0;
    GetBoundaryHeader(file,&patchi->npatches,&glui_patchmin,&glui_patchmax);
    patchmin_global = glui_patchmin;
    patchmax_global = glui_patchmax;
  }
  CheckMemory;
  if(patchi->npatches>0){
    int abort = 0;
    if(NewResizeMemory(patchi->patchfaceinfo, sizeof(patchfacedata) * patchi->npatches) == 0)abort = 1;
    if(abort == 1){
      *errorcode = 1;
      if(patchi->compression_type == UNCOMPRESSED){
        fclose_m(stream);
      }
      ReadBoundary(ifile, UNLOAD, &error);
      return 0;
    }
  }

  if(patchi->compression_type==UNCOMPRESSED){
    GetPatchSizes2(stream, patchi->npatches, nmeshes, meshi->nbptrs, &meshi->npatchsize,
      patchi->patchfaceinfo, &headersize,&framesize);

    // loadpatchbysteps
    //  0 - load entire uncompressed data set
    //  1 - load compressed data set

    loadpatchbysteps=UNCOMPRESSED_ALLFRAMES;
    if(load_flag==LOAD||load_flag==RELOAD){
      statfile=STAT(file,&statbuffer);
      if(statfile==0&&framesize!=0){
        patchi->ntimes_old = patchi->ntimes;
        patchi->ntimes = (statbuffer.st_size-headersize)/framesize;
      }
    }
  }
  else{
    int nnsize=0;
    int i;

    GetBoundaryHeader2(file, patchi->patchfaceinfo, nmeshes, meshi->nbptrs);
    for(i=0;i<patchi->npatches;i++){
      int ii1, ii2, jj1, jj2, kk1, kk2;
      patchfacedata *pfi;

      pfi = patchi->patchfaceinfo + i;

      ii1 = pfi->ib[0];
      ii2 = pfi->ib[1];
      jj1 = pfi->ib[2];
      jj2 = pfi->ib[3];
      kk1 = pfi->ib[4];
      kk2 = pfi->ib[5];
      nnsize += (ii2+1-ii1)*(jj2+1-jj1)*(kk2+1-kk1);
    }
    meshi->npatchsize=nnsize;
    loadpatchbysteps=COMPRESSED_ALLFRAMES;
  }

  if(meshi->npatchsize>0){
    if(
       NewResizeMemory(meshi->xyzpatch,          3*sizeof(float)*meshi->npatchsize)==0||
       NewResizeMemory(meshi->xyzpatch_threshold,3*sizeof(float)*meshi->npatchsize)==0||
       NewResizeMemory(meshi->thresholdtime,     sizeof(float)*meshi->npatchsize)==0||
       NewResizeMemory(meshi->patchblank,        meshi->npatchsize*sizeof(int))==0
       ){
      *errorcode=1;
      patchi->loaded=0;
      patchi->display=0;
      if(patchi->compression_type==UNCOMPRESSED){
        fclose_m(stream);
      }
      ReadBoundary(ifile,UNLOAD,&error);
      return 0;
    }
  }
  for(n=0;n<meshi->npatchsize;n++){
    meshi->patchblank[n]=GAS;
  }
  xyzpatchcopy = meshi->xyzpatch;
  xyzpatch_ignitecopy = meshi->xyzpatch_threshold;
  patchblankcopy = meshi->patchblank;
  patchi->patchfaceinfo[0].start = 0;
  for(n=0;n<meshi->nbptrs;n++){
    blockagedata *bc;
    int j;

    bc=meshi->blockageinfoptrs[n];
    for(j=0;j<6;j++){
      bc->patchvis[j]=1;
    }
  }
  for(n=0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst==NULL)continue;
    if(pfi->dir==-1)pfi->obst->patchvis[0] = 0;
    if(pfi->dir== 1)pfi->obst->patchvis[1] = 0;
    if(pfi->dir==-2)pfi->obst->patchvis[2] = 0;
    if(pfi->dir== 2)pfi->obst->patchvis[3] = 0;
    if(pfi->dir==-3)pfi->obst->patchvis[4] = 0;
    if(pfi->dir== 3)pfi->obst->patchvis[5] = 0;
  }
  for(n=0;n<patchi->npatches;n++){
    float dxx, dyy, dzz;
    float dxx2, dyy2, dzz2;
    float dx_factor, dy_factor, dz_factor;
    int i1, i2, j1, j2, k1, k2;
    int *is_extface;
    float ig_factor_x, ig_factor_y, ig_factor_z;
    float block_factor_x, block_factor_y, block_factor_z;
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    pfi->internal_mesh_face = 0;
    i1 = pfi->ib[0];
    i2 = pfi->ib[1];
    j1 = pfi->ib[2];
    j2 = pfi->ib[3];
    k1 = pfi->ib[4];
    k2 = pfi->ib[5];

    // determine if a patch is on an external wall 
    if(i1 == i2){
      float val;

      val = meshi->xplt_orig[i1];
      if(val > xbar0FDS && val < xbarFDS){
        if(j1 == 0 && j2 == meshi->jbar && k1 == 0 && k2 == meshi->kbar)pfi->internal_mesh_face = 1;
      }
      if(j1 == 0 && j2 == meshi->jbar && k1 == 0 && k2 == meshi->kbar){
        if(i1 == 0)patchi->meshfaceinfo[0]           = pfi;
        if(i1 == meshi->ibar)patchi->meshfaceinfo[1] = pfi;
      }
    }
    else if(j1 == j2){
      float val;

      val = meshi->yplt_orig[j1];
      if(val > ybar0FDS && val < ybarFDS){
        if(i1 == 0 && i2 == meshi->ibar && k1 == 0 && k2 == meshi->kbar)pfi->internal_mesh_face = 1;
      }
      if(i1 == 0 && i2 == meshi->ibar && k1 == 0 && k2 == meshi->kbar){
        if(j1 == 0)patchi->meshfaceinfo[2]           = pfi;
        if(j1 == meshi->jbar)patchi->meshfaceinfo[3] = pfi;
      }
    }
    else{
      float val;

      val = meshi->zplt_orig[k1];
      if(val > zbar0FDS && val < zbarFDS){
        if(i1 == 0 && i2 == meshi->ibar && j1 == 0 && j2 == meshi->jbar)pfi->internal_mesh_face = 1;
      }
      if(i1 == 0 && i2 == meshi->ibar && j1 == 0 && j2 == meshi->jbar){
        if(k1 == 0)patchi->meshfaceinfo[4]           = pfi;
        if(k1 == meshi->kbar)patchi->meshfaceinfo[5] = pfi;
      }
      assert(k1 == k2);
    }

    // determine if patch is an internal mesh face


    if(pfi->dir==YDIR||pfi->dir==YDIRNEG)pfi->dir=-pfi->dir;
    dxx = 0.0;
    dyy = 0.0;
    dzz = 0.0;

    ig_factor_x = ABS(meshi->xplt[1] - meshi->xplt[0]) / 10.0;
    ig_factor_y = ABS(meshi->yplt[1] - meshi->yplt[0]) / 10.0;
    ig_factor_z = ABS(meshi->zplt[1] - meshi->zplt[0]) / 10.0;
    block_factor_x = ig_factor_x;
    block_factor_y = ig_factor_y;
    block_factor_z = ig_factor_z;

    switch(pfi->dir){
    case XDIRNEG:
      dxx = -meshi->xplt[1]*ig_factor_x;
      dxx2 = -meshi->xplt[1]*block_factor_x;
      break;
    case XDIR:
      dxx = meshi->xplt[1]*ig_factor_x;
      dxx2 = meshi->xplt[1]*block_factor_x;
      break;
    case YDIRNEG:
      dyy = meshi->yplt[1]*ig_factor_y;
      dyy2 = meshi->yplt[1]*block_factor_y;
      break;
    case YDIR:
      dyy = -meshi->yplt[1]*ig_factor_y;
      dyy2 = -meshi->yplt[1]*block_factor_y;
      break;
    case ZDIRNEG:
      dzz = -meshi->zplt[1]*ig_factor_z;
      dzz2 = -meshi->zplt[1]*block_factor_z;
      break;
    case ZDIR:
      dzz = meshi->zplt[1]*ig_factor_z;
      dzz2 = meshi->zplt[1]*block_factor_z;
      break;
    default:
      assert(FFALSE);
    }

    pfi->type=INTERIORwall;
    is_extface = meshi->is_extface;
    if(i1==i2){
      int ext_wall, mesh_boundary;

      pfi->ncol = j2 + 1 - j1;
      pfi->nrow = k2 + 1 - k1;

      ext_wall=0;
      mesh_boundary = NO;
      if(j1==0&&j2==jbartemp&&k1==0&&k2==kbartemp){
        int doit;

        doit = 0;
        if(show_bndf_mesh_interface==1){
          if( (i1==0&&meshi->nabors[MLEFT]==NULL)||
              (i2==ibartemp&&meshi->nabors[MRIGHT]==NULL)
             ){
            doit=1;
          }
        }
        else{
          if(i1==0||i2==ibartemp){
            doit=1;
          }
        }
        if(doit==1){
          mesh_boundary = YES;
          if(is_extface[0]==MESH_EXT&&i1 == 0){
            ext_wall = 1;
            pfi->type = LEFTwall;
          }
          if(is_extface[1]==MESH_EXT&&i2 == ibartemp){
            ext_wall = 1;
            pfi->type = RIGHTwall;
          }
        }
      }
      if(ext_wall==0){
        int k;

        // an internal wall so set blank to 1 then zero out where there are vents
        for(k=k1;k<=k2;k++){
          int j;

          if(k==k1){
            dz_factor=-meshi->zplt[1]*ig_factor_z;
          }
          else if(k==k2){
            dz_factor=meshi->zplt[1]*ig_factor_z;
          }
          else{
            dz_factor=0.0;
          }
          for(j=j1;j<=j2;j++){
            if(j==j1){
              dy_factor=-meshi->yplt[1]*ig_factor_y;
            }
            else if(j==j2){
              dy_factor=meshi->yplt[1]*ig_factor_y;
            }
            else{
              dy_factor=0.0;
            }
            *xyzpatchcopy++ = xplttemp[i1]+dxx2;
            *xyzpatchcopy++ = yplttemp[j];
            *xyzpatchcopy++ = zplttemp[k];
            *xyzpatch_ignitecopy++ = xplttemp[i1]+dxx;
            *xyzpatch_ignitecopy++ = yplttemp[j]+dy_factor;
            *xyzpatch_ignitecopy++ = zplttemp[k]+dz_factor;
            *patchblankcopy = NodeInInternalVent(meshi,i1,j,k,1,mesh_boundary,wallcenter);
            patchblankcopy++;
          }
        }
      }
      else{
        int iii;
        int k;

        // an external wall so set blank to 0 then set to one where there are dummy vents
        iii=0;
        for(k=k1;k<=k2;k++){
          int j;

          if(k==k1){
            dz_factor=-meshi->zplt[1]*ig_factor_z;
          }
          else if(k==k2){
            dz_factor=meshi->zplt[1]*ig_factor_z;
          }
          else{
            dz_factor=0.0;
          }
          for(j=j1;j<=j2;j++){
            if(j==j1){
              dy_factor=-meshi->yplt[1]*ig_factor_y;
            }
            else if(j==j2){
              dy_factor=meshi->yplt[1]*ig_factor_y;
            }
            else{
              dy_factor=0.0;
            }
            *xyzpatchcopy++ = xplttemp[i1]+dxx2;
            *xyzpatchcopy++ = yplttemp[j];
            *xyzpatchcopy++ = zplttemp[k];
            *xyzpatch_ignitecopy++ = xplttemp[i1]+dxx;
            *xyzpatch_ignitecopy++ = yplttemp[j]+dy_factor;
            *xyzpatch_ignitecopy++ = zplttemp[k]+dz_factor;
            patchblankcopy[iii++]=SOLID;
          }
        }
        NodeInExternalVent(n,patchblankcopy,meshi,i1,i2,j1,j2,k1,k2,wallcenter);
        patchblankcopy += (k2+1-k1)*(j2+1-j1);
      }
    }
    else if(j1==j2){
      int ext_wall, mesh_boundary;

      pfi->ncol = i2 + 1 - i1;
      pfi->nrow = k2 + 1 - k1;

      ext_wall=0;
      mesh_boundary = NO;
      if(i1==0&&i2==ibartemp&&k1==0&&k2==kbartemp){
        int doit;

        doit = 0;
        if(show_bndf_mesh_interface==1){
          if((j1==0&&meshi->nabors[MFRONT]==NULL)||
             (j2==jbartemp&&meshi->nabors[MBACK]==NULL)
            ){
            doit = 1;
          }
        }
        else{
          if(j1==0||j2==jbartemp){
            doit = 1;
          }
        }
        if(doit==1){
          mesh_boundary = YES;
          if(is_extface[2]==MESH_EXT&&j1 == 0){
            ext_wall = 1;
            pfi->type = FRONTwall;
          }
          if(is_extface[3]==MESH_EXT&&j2 == jbartemp){
            ext_wall = 1;
            pfi->type = BACKwall;
          }
        }
      }
      if(ext_wall==0){
        int k;

        for(k=k1;k<=k2;k++){
          int i;

          if(k==k1){
            dz_factor=-meshi->zplt[1]*ig_factor_z;
          }
          else if(k==k2){
            dz_factor=meshi->zplt[1]*ig_factor_z;
          }
          else{
            dz_factor=0.0;
          }
          for(i = i1;i <= i2;i++){
            if(i == i1){
              dx_factor = -meshi->xplt[1] * ig_factor_x;
            }
            else if(i == i2){
              dx_factor = meshi->xplt[1] * ig_factor_x;
            }
            else{
              dx_factor = 0.0;
            }
            *xyzpatchcopy++ = xplttemp[i];
            *xyzpatchcopy++ = yplttemp[j1] + dyy2;
            *xyzpatchcopy++ = zplttemp[k];
            *xyzpatch_ignitecopy++ = xplttemp[i] + dx_factor;
            *xyzpatch_ignitecopy++ = yplttemp[j1] + dyy;
            *xyzpatch_ignitecopy++ = zplttemp[k] + dz_factor;
            *patchblankcopy++ = NodeInInternalVent(meshi, i, j1, k, 2, mesh_boundary, wallcenter);
          }
        }
      }
      else{
        int iii;
        int k;

        // an external wall so set blank to 0 then zero out where there are vents
        iii=0;
        for(k=k1;k<=k2;k++){
          int i;

          if(k==k1){
            dz_factor=-meshi->zplt[1]*ig_factor_z;
          }
          else if(k==k2){
            dz_factor=meshi->zplt[1]*ig_factor_z;
          }
          else{
            dz_factor=0.0;
          }
          for(i=i1;i<=i2;i++){
            if(i==i1){
              dx_factor=-meshi->xplt[1]*ig_factor_x;
            }
            else if(i==i2){
              dx_factor=meshi->xplt[1]*ig_factor_x;
            }
            else{
              dx_factor=0.0;
            }
            *xyzpatchcopy++ = xplttemp[i];
            *xyzpatchcopy++ = yplttemp[j1]+dyy2;
            *xyzpatchcopy++ = zplttemp[k];
            *xyzpatch_ignitecopy++ = xplttemp[i]+dx_factor;
            *xyzpatch_ignitecopy++ = yplttemp[j1]+dyy;
            *xyzpatch_ignitecopy++ = zplttemp[k]+dz_factor;
            patchblankcopy[iii++]=SOLID;
          }
        }
        NodeInExternalVent(n,patchblankcopy,meshi,i1,i2,j1,j2,k1,k2,wallcenter);
        patchblankcopy += (k2+1-k1)*(i2+1-i1);
      }
    }
    else if(k1==k2){
      int ext_wall, mesh_boundary;

      pfi->ncol = i2 + 1 - i1;
      pfi->nrow = j2 + 1 - j1;

      ext_wall=0;
      mesh_boundary = NO;
      if(i1==0&&i2==ibartemp&&j1==0&&j2==jbartemp){
        int doit;

        doit = 0;
        if(show_bndf_mesh_interface==1){
          if((k1==0&&meshi->nabors[MDOWN]==NULL)||
             (k2==kbartemp&&meshi->nabors[MUP]==NULL)
            ){
            doit = 1;
          }
        }
        else{
          if(k1==0||k2==kbartemp){
            doit = 1;
          }
        }
        if(doit==1){
          mesh_boundary = YES;
          if(is_extface[4]==MESH_EXT&&k1 == 0){
            ext_wall = 1;
            pfi->type = DOWNwall;
          }
          if(is_extface[5]==MESH_EXT&&k2 == kbartemp){
            ext_wall = 1;
            pfi->type = UPwall;
          }
        }
      }
      if(ext_wall==0){
        int j;

        for(j=j1;j<=j2;j++){
          int i;

          if(j==j1){
            dy_factor=-meshi->yplt[1]*ig_factor_y;
          }
          else if(j==j2){
            dy_factor=meshi->yplt[1]*ig_factor_y;
          }
          else{
            dy_factor=0.0;
          }
          for(i=i1;i<=i2;i++){
            if(i==i1){
              dx_factor=-meshi->xplt[1]*ig_factor_x;
            }
            else if(i==i2){
              dx_factor=meshi->xplt[1]*ig_factor_x;
            }
            else{
              dx_factor=0.0;
            }
            *xyzpatchcopy++ = xplttemp[i];
            *xyzpatchcopy++ = yplttemp[j];
            *xyzpatchcopy++ = zplttemp[k1]+dzz2;
            *xyzpatch_ignitecopy++ = xplttemp[i]+dx_factor;
            *xyzpatch_ignitecopy++ = yplttemp[j]+dy_factor;
            *xyzpatch_ignitecopy++ = zplttemp[k1]+dzz;
            if(mesh_boundary == 1){
              *patchblankcopy++ = SOLID;
            }
            else{
              *patchblankcopy++ = NodeInInternalVent(meshi, i, j, k1, 3, mesh_boundary, wallcenter);
            }
          }
        }
      }
      else{
        int iii;
        int j;

      // an external wall so set blank to 0 then zero out where there are vents

        iii=0;
        for(j=j1;j<=j2;j++){
          int i;

          if(j==j1){
            dy_factor=-meshi->yplt[1]*ig_factor_y;
          }
          else if(j==j2){
            dy_factor=meshi->yplt[1]*ig_factor_y;
          }
          else{
            dy_factor=0.0;
          }
          for(i=i1;i<=i2;i++){
            if(i==i1){
              dx_factor=-meshi->xplt[1]*ig_factor_x;
            }
            else if(i==i2){
              dx_factor=meshi->xplt[1]*ig_factor_x;
            }
            else{
              dx_factor=0.0;
            }
            *xyzpatchcopy++ = xplttemp[i];
            *xyzpatchcopy++ = yplttemp[j];
            *xyzpatchcopy++ = zplttemp[k1]+dzz2;
            *xyzpatch_ignitecopy++ = xplttemp[i]+dx_factor;
            *xyzpatch_ignitecopy++ = yplttemp[j]+dy_factor;
            *xyzpatch_ignitecopy++ = zplttemp[k1]+dzz;
            patchblankcopy[iii++]=SOLID;
          }
        }
        NodeInExternalVent(n,patchblankcopy,meshi,i1,i2,j1,j2,k1,k2,wallcenter);
        patchblankcopy += (j2+1-j1)*(i2+1-i1);
      }
    }
    if(n!=patchi->npatches-1)(pfi+1)->start=pfi->start+pfi->nrow*pfi->ncol;
    pfi->vis=vis_boundary_type[pfi->type];
  }

  for(n=0;n<meshi->nbptrs;n++){
    blockagedata *bc;
    int j;

    bc=meshi->blockageinfoptrs[n];
    bc->patchvis[6]=0;
    for(j=0;j<6;j++){
      if(bc->patchvis[j]!=0){
        bc->patchvis[6] = 1;
        break;
      }
    }
  }

  meshi->patchval = NULL;
  switch(loadpatchbysteps){
  case UNCOMPRESSED_ALLFRAMES:
    if(meshi->patchval==NULL){
      NewResizeMemory(meshi->patchval,sizeof(float)*patchi->ntimes*meshi->npatchsize);
    }
    if(meshi->patchval==NULL){
      NewResizeMemory(meshi->patchval,sizeof(float)*patchi->ntimes*meshi->npatchsize);
    }
    break;
  case COMPRESSED_ALLFRAMES:
    GetBoundarySizeInfo(patchi, &patchi->ntimes, &ncompressed_buffer);
    NewResizeMemory(meshi->cpatchval_zlib,       sizeof(unsigned char)*ncompressed_buffer);
    NewResizeMemory(meshi->cpatchval_iframe_zlib,sizeof(unsigned char)*meshi->npatchsize);
    break;
  default:
    assert(FFALSE);
    break;
  }

  NewResizeMemory(meshi->patch_times,     sizeof(float)*patchi->ntimes);
  NewResizeMemory(meshi->patch_times_map, sizeof(unsigned char)*patchi->ntimes);
  NewResizeMemory(meshi->zipoffset,       sizeof(unsigned int)*patchi->ntimes);
  NewResizeMemory(meshi->zipsize,         sizeof(unsigned int)*patchi->ntimes);
  if(meshi->patch_times==NULL){
    *errorcode=1;
    fclose_m(stream);
    ReadBoundary(ifile,UNLOAD,&error);
    return 0;
  }
  if(loadpatchbysteps==COMPRESSED_ALLFRAMES){
    GetBoundaryDataZlib(patchi,meshi->cpatchval_zlib,ncompressed_buffer, 
      meshi->patch_times,meshi->zipoffset,meshi->zipsize,patchi->ntimes);
    framestart = 0;
    return_filesize += ncompressed_buffer;
  }
  else{
    if(meshi->patchval == NULL){
      *errorcode = 1;
      fclose_m(stream);
      ReadBoundary(ifile, UNLOAD, &error);
      return 0;
    }
    if(load_flag == RELOAD&&patchi->ntimes_old > 0){
      framestart = patchi->ntimes_old;
    }
    else{
      framestart = 0;
    }
  }
  START_TIMER(read_time);
  for(ii=framestart;ii<patchi->ntimes;){
    if(loadpatchbysteps==UNCOMPRESSED_ALLFRAMES){
#ifndef pp_BOUNDFRAME
      meshi->patchval_iframe = meshi->patchval + ii*meshi->npatchsize;
#endif
    }
    meshi->patch_timesi = meshi->patch_times + ii;

    error=0;
    if(loadpatchbysteps==UNCOMPRESSED_ALLFRAMES){
      if(ii==framestart&&framestart>0){
        int framesizes;

        framesizes = framesize*framestart-8;
        fseek_m(stream, framesizes, SEEK_CUR);
        local_first = 0;
      }
#ifdef pp_BOUNDFRAME
      return_filesize = patchi->frameinfo->filesize;
      meshi->npatch_times = patchi->frameinfo->nframes;
#else
      for(n=0;n<tload_step;n++){
        if(error==0){
          int npatchval_iframe;
          int filesize;
          int imesh;

          imesh = (int)(meshi - meshinfo);

          GetPatchData(imesh,stream,patchi->npatches,patchi->patchfaceinfo,
          meshi->patch_timesi,meshi->patchval_iframe,&npatchval_iframe,&filesize, &error);
          return_filesize += filesize;
        }
      }
#endif
    }
    if(do_threshold==1){
      if(local_first==1){
        nn=0;
        for(n=0;n<meshi->npatchsize;n++){
          meshi->thresholdtime[n]=-1.0;
        }
        local_first=0;
      }

      /* create char contour plot for each patch corresponding to a blockage */

      {

        nn=0;
        if(loadpatchbysteps == COMPRESSED_ALLFRAMES)UncompressBoundaryDataBNDF(meshi, ii);
        for(n=0;n<patchi->npatches;n++){
          float dval;
          int j;
#ifdef pp_BOUNDFRAME
          float *patchn;
#endif
          patchfacedata *pfi;

          pfi = patchi->patchfaceinfo + n;
          nsize=pfi->nrow*pfi->ncol;
          if(pfi->obst != NULL){
            switch(loadpatchbysteps){
            case UNCOMPRESSED_ALLFRAMES:
#ifdef pp_BOUNDFRAME
              patchn = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
              for(j=0;j<nsize;j++){
                  if(meshi->thresholdtime[nn+j]<0.0&&patchn[j]>=temp_threshold){
                    meshi->thresholdtime[nn+j]=meshi->patch_times[ii];
                  }
                }
#else
              for(j = 0; j < nsize; j++){
                if(meshi->thresholdtime[nn + j] < 0.0 && meshi->patchval_iframe[nn + j] >= temp_threshold){
                  meshi->thresholdtime[nn + j] = meshi->patch_times[ii];
                }
              }
#endif
              break;
            case COMPRESSED_ALLFRAMES:
              dval = (glui_patchmax-glui_patchmin)/255.0;
              for(j=0;j<nsize;j++){
                float val;
                int ival;

                ival = meshi->cpatchval_iframe_zlib[nn+j];
                val = glui_patchmin + dval*ival;
                if(meshi->thresholdtime[nn+j]<0.0&&val>=temp_threshold){
                  meshi->thresholdtime[nn+j]=meshi->patch_times[ii];
                }
              }
              break;
            default:
              assert(FFALSE);
              break;
            }
          }
          nn+=nsize;
        }
      }
    }
    CheckMemory;
    if(error!=0)break;
    if(use_tload_end!=0&&*meshi->patch_timesi>tload_end)break;

    switch(loadpatchbysteps){
      case UNCOMPRESSED_ALLFRAMES:
      case COMPRESSED_ALLFRAMES:
        ii++;
        break;
      default:
        assert(FFALSE);
        break;
    }
  }
  STOP_TIMER(read_time);
  CheckMemory;

  /* convert patch values into integers pointing to an rgb color table */

  if(loadpatchbysteps==UNCOMPRESSED_ALLFRAMES){
    if(output_patchdata==1){
      OutputBoundaryData(patchcsvfile,patchi,FIRST_TIME,NULL);
    }
    npatchvals = patchi->ntimes*meshi->npatchsize;
    if(npatchvals==0||NewResizeMemory(meshi->cpatchval,sizeof(unsigned char)*npatchvals)==0){
      *errorcode=1;
      fclose_m(stream);
      ReadBoundary(ifile,UNLOAD,&error);
      return 0;
    }
  }
  if(NewResizeMemory(colorlabelpatch,MAXRGB*sizeof(char *))==0){
    *errorcode=1;
    if(loadpatchbysteps!=COMPRESSED_ALLFRAMES){
      fclose_m(stream);
    }
    ReadBoundary(ifile,UNLOAD,&error);
    return 0;
  }
  for(n=0;n<MAXRGB;n++){
    colorlabelpatch[n]=NULL;
  }
  for(n=0;n<nrgb;n++){
    if(NewResizeMemory(colorlabelpatch[n],11)==0){
      *errorcode=1;
      if(loadpatchbysteps!=COMPRESSED_ALLFRAMES){
        fclose_m(stream);
      }
      ReadBoundary(ifile,UNLOAD,&error);
      return 0;
    }
  }

  patchi->loaded=1;
  iboundarytype=GetBoundaryType(patchi);
  switch(loadpatchbysteps){
    int patchstart, i;

    case UNCOMPRESSED_ALLFRAMES:

    patchstart = patchi->ntimes_old*meshi->npatchsize;

    if(patchi->have_bound_file==NO&&FileExistsOrig(patchi->bound_file)==NO){
      patchmin_global = 10000000000000.0;
      patchmax_global = -patchmin_global;
      for(i = 0; i < npatchvals; i++){
        patchmin_global = MIN(patchmin_global, meshi->patchval[i]);
        patchmax_global = MAX(patchmax_global, meshi->patchval[i]);
      }
      patchi->valmin_patch = patchmin_global;
      patchi->valmax_patch = patchmax_global;
      if(WriteFileBounds(patchi->bound_file, patchmin_global, patchmax_global)==1){
        patchi->have_bound_file = YES;
        patch_bounds_defined = 0;
      }
    }
    GetBoundaryColors3(patchi, meshi->patchval, patchstart, npatchvals, meshi->cpatchval,
                       &glui_patchmin, &glui_patchmax,
                       nrgb, colorlabelpatch, colorvaluespatch, boundarylevels256,
                       &patchi->extreme_min, &patchi->extreme_max, 0);
    break;
  case COMPRESSED_ALLFRAMES:
    GetBoundaryLabels(
      glui_patchmin, glui_patchmax,
      colorlabelpatch,colorvaluespatch,boundarylevels256,nrgb);
    break;
  default:
    assert(FFALSE);
    break;
  }

  GLUI2GlobalBoundaryBounds(patchi->label.shortlabel);

  patchi->loaded = 1;
  patchi->display = 1;
  patchi->hist_update = 1;

#ifdef pp_RECOMPUTE_DEBUG
  int recompute = 0;
#endif
  if(patchi->finalize==1){
    boundary_loaded = 1;
    if(boundary_interface_faces==1 && have_removable_obsts == 1 && nmeshes>1){
      boundary_interface_unhide = 1;
      BlockageMenu(visBLOCKHide);
    }
    CheckMemory;
    GLUIUpdateBoundaryListIndex(patchfilenum);
    cpp_boundsdata *bounds;

    if(runscript == 0){
      THREADcontrol(patchbound_threads, THREAD_JOIN);
    }
    int set_valmin_save, set_valmax_save;
    float qmin_save, qmax_save;
    GLUIGetMinMax(BOUND_PATCH, patchi->label.shortlabel, &set_valmin_save, &qmin_save, &set_valmax_save, &qmax_save);
    if(force_bound_update==1||patch_bounds_defined==0 || BuildGbndFile(BOUND_PATCH) == 1){
      GetGlobalPatchBounds(1,DONOT_SET_MINMAX_FLAG);
      SetLoadedPatchBounds(NULL, 0);
      GLUIPatchBoundsCPP_CB(BOUND_DONTUPDATE_COLORS);
#ifdef pp_RECOMPUTE_DEBUG
      recompute = 1;
#endif
    }
    else{
      bounds = GLUIGetBoundsData(BOUND_PATCH);
      if(bounds->set_valmin==BOUND_PERCENTILE_MIN||bounds->set_valmax==BOUND_PERCENTILE_MAX){
        float global_min=0.0, global_max=1.0;
        GLUIGetGlobalBoundsMinMax(BOUND_PATCH, bounds->label, &global_min, &global_max);
        GLUIPatchBoundsCPP_CB(BOUND_UPDATE_COLORS);
      }
    }
    if(set_valmin_save == 0){
      SetPatchMin(set_valmin_save, qmin_save, patchi->label.shortlabel);
    }
    if(set_valmax_save == 0){
      SetPatchMax(set_valmax_save, qmax_save, patchi->label.shortlabel);
    }
    if(set_valmin_save == 0 || set_valmax_save == 0){
      UpdateAllBoundaryColors(0);
    }
#define BOUND_PERCENTILE_DRAW          120
    GLUIPatchBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  }

  if(wallcenter==1){
    int i;

    InitVentColors();
    FREEMEMORY(meshi->patchventcolors);
    NewResizeMemory(meshi->patchventcolors,sizeof(float *)*npatchvals);
    for(i=0;i<npatchvals;i++){
      int vent_index;

      vent_index = CLAMP(meshi->patchval[i]+0.1,0,nventcolors-1);
      meshi->patchventcolors[i]=ventcolors[vent_index];
    }
    int mintimes = -1;
    for(i = 0;i < npatchinfo;i++){
      patchdata *pi;

      pi = patchinfo + i;
      if(pi->loaded == 0)continue;
      if(mintimes < 0){
        mintimes = pi->ntimes;
      }
      else{
        mintimes = MIN(mintimes, pi->ntimes);
      }
    }
    for(i = 0;i < npatchinfo;i++){
      patchdata *pi;
      meshdata *mi;

      pi = patchinfo + i;
      if(pi->loaded == 0)continue;
      mi = meshinfo + pi->blocknumber;
      mi->maxtimes_boundary = mintimes;
    }
  }
  if(cache_boundary_data==0){
    FREEMEMORY(meshi->patchval);
  }
  iboundarytype=GetBoundaryType(patchi);
  if(patchi->finalize==1){
    ShowBoundaryMenu(INI_EXTERIORwallmenu);
  }
  for(n = 0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    pfi->vis = vis_boundary_type[pfi->type];
  }
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  MakeTimesMap(meshi->patch_times, &meshi->patch_times_map, patchi->ntimes);
  if(patchi->finalize==1){
    UpdateTimes();
    ForceIdle();
    UpdateUnitDefs();
    UpdateChopColors();
  }
  PrintMemoryInfo;

  STOP_TIMER(total_time);

#ifdef pp_BOUNDFRAME
  printf("\n");
#else
  if(return_filesize > 1000000000){
    PRINTF(" - %.1f GB in %.1f s\n", (float)return_filesize / 1000000000., total_time);
  }
  else if(return_filesize > 1000000){
    PRINTF(" - %.1f MB in %.1f s\n", (float)return_filesize / 1000000., total_time);
  }
 else{
   PRINTF(" - %.0f kB in %.1f s\n", (float)return_filesize / 1000., total_time);
  }
#endif
#ifdef pp_RECOMPUTE_DEBUG
  if(recompute == 1)printf("***recomputing bounds\n");
#endif

  update_patch_bounds = ifile;

  GLUTPOSTREDISPLAY;
#ifdef pp_BOUNDFRAME
  if(patchi->frameinfo != NULL){
    patchi->frameinfo->total_time = total_time;
  }
#endif
  CheckMemory;
 // debug patch print
  if(outout_patch_faces == 1){
    printf("\n");
    printf("************************************\n");
    printf("loading boundary file for mesh: %i\n", patchi->blocknumber+1);
    printf("************************************\n");
    for(n = 0; n < patchi->npatches; n++){
      patchfacedata *pfi;

      pfi = patchi->patchfaceinfo + n;
      if(n == 0)printf("\n");
      printf("%i: (%i,%i,%i) (%i,%i,%i) direction: %i, obst index: %i, mesh index: %i,",
        n+1, pfi->ib[0], pfi->ib[2], pfi->ib[4], pfi->ib[1], pfi->ib[3], pfi->ib[5],
        pfi->dir, pfi->obst_index+1, pfi->mesh_index+1);
      if(pfi->internal_mesh_face == 1){
        printf(" internal mesh interface patch\n");
      }
      else{
        if(pfi->obst_index < 0){
          printf(" vent patch\n");
        }
        else{
          printf(" obst patch\n");
        }
      }
    }
  }
  return return_filesize;
}

/* ------------------ SetTimeState ------------------------ */

void SetTimeState(void){
  if(stept==0&&last_time_paused==1){
    int timestate;

    timestate = GetPlotState(DYNAMIC_PLOTS);
    if(timestate==DYNAMIC_PLOTS){
      update_stept = 1;
      if(global_times!=NULL){
        time_paused = global_times[itimes];
      }
      else{
        time_paused = 0.0;
      }
    }
  }
}

/* ------------------ ReadBoundary ------------------------ */

FILE_SIZE ReadBoundary(int ifile, int load_flag, int *errorcode){
  patchdata *patchi;
  FILE_SIZE return_filesize = 0;


  SetTimeState();
  patchi = patchinfo + ifile;
  if(patchi->structured == NO){
    return_filesize=ReadGeomData(patchi,NULL, load_flag,ALL_FRAMES, NULL, 1, errorcode);
  }
  else{
    assert(ifile>=0&&ifile<npatchinfo);
    return_filesize=ReadBoundaryBndf(ifile,load_flag,errorcode);
  }
  return return_filesize;
}

/* ------------------ GLUI2GlobalBoundaryBounds ------------------------ */

void GLUI2GlobalBoundaryBounds(const char *key){
  int i;

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(strcmp(key,"")==0||strcmp(patchi->label.shortlabel,key)==0){
      patchi->valmin_glui = glui_patchmin;
      patchi->valmax_glui = glui_patchmax;

      patchi->chopmin=patchchopmin;
      patchi->chopmax=patchchopmax;
      patchi->setchopmin=setpatchchopmin;
      patchi->setchopmax=setpatchchopmax;
    }
  }
}

/* ------------------ Global2GLUIBoundaryBounds ------------------------ */

void Global2GLUIBoundaryBounds(const char *key){
  int i;

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(strcmp(patchi->label.shortlabel,key)==0){
      glui_patchmin = patchi->valmin_glui;
      glui_patchmax = patchi->valmax_glui;

      patchchopmin=patchi->chopmin;
      patchchopmax=patchi->chopmax;
      setpatchchopmin=patchi->setchopmin;
      setpatchchopmax=patchi->setchopmax;

      patchmin_unit = (unsigned char *)patchi->label.unit;
      patchmax_unit = patchmin_unit;

      UpdateHideBoundarySurface();

      GLUI2GlobalBoundaryBounds(key);
      return;
    }
  }
}


/* ------------------ DrawMeshBoundaryFaces ------------------------ */

void DrawMeshBoundaryFaces(patchdata *patchi, float valmin, float valmax){
  meshdata *meshi;
  float *patchvals;
  unsigned char *cpatchvals;
 
  meshi = meshinfo + patchi->blocknumber;

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D, texture_patch_colorbar_id);

  glBegin(GL_TRIANGLES);

  int iface;
  for(iface = 0; iface < 6; iface++){
    int j;

    blockagedata **bclist;
    patchfacedata *pfi;
    int ncol;

    if(patchi->meshfaceinfo[iface] == NULL)continue;
    pfi = patchi->meshfaceinfo[iface];
    ncol = pfi->ncol;

#ifdef pp_BOUNDFRAME
    patchvals = ( float * )FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, pfi->start);
#else
    patchvals = meshi->patchval_iframe + pfi->start;
#endif
    if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;

    bclist = meshi->bc_faces[iface];
    for(j = 0; j < meshi->n_bc_faces[iface]; j++){
      blockagedata *bc;
      int irow;
      int rowbeg, rowend, colbeg, colend;

      bc = bclist[j];
      int draw_plane = 0;
      if(boundary_debug_mesh-1 == (int)(meshi-meshinfo) && boundary_debug_plane[iface] ==1 )draw_plane = 1;
      switch(iface){
      case 0:
      case 1:
        if(draw_plane==1){
          rowbeg = 0;
          rowend = meshi->jbar;
          colbeg = 0;
          colend = meshi->kbar;
        }
        else{
          rowbeg = bc->ijk[4];
          rowend = bc->ijk[5];
          colbeg = bc->ijk[2];
          colend = bc->ijk[3];
        }
        break;
      case 2:
      case 3:
        if(draw_plane==1){
          rowbeg = 0;
          rowend = meshi->ibar;
          colbeg = 0;
          colend = meshi->ibar;
        }
        else{
          rowbeg = bc->ijk[4];
          rowend = bc->ijk[5];
          colbeg = bc->ijk[0];
          colend = bc->ijk[1];
        }
        break;
      case 4:
      case 5:
        if(draw_plane==1){
          rowbeg = 0;
          rowend = meshi->ibar;
          colbeg = 0;
          colend = meshi->jbar;
        }
        else{
          rowbeg = bc->ijk[2];
          rowend = bc->ijk[3];
          colbeg = bc->ijk[0];
          colend = bc->ijk[1];
        }
        break;
      default:
	assert(0);
	break;
      }
      if(draw_plane==0&&bc->showtimelist != NULL && bc->showtimelist[itimes] == 0)continue;
      for(irow = rowbeg; irow<rowend; irow++){
        int icol;

        for(icol=colbeg; icol<colend; icol++){
          float r11, r12, r21, r22;
          float *xyzp1, *xyzp2;
          int i11, i12, i21, i22;

          i11 = IJKBF(irow, icol);
          i12 = IJKBF(irow, icol + 1);
          i21 = IJKBF(irow + 1, icol);
          i22 = IJKBF(irow + 1, icol + 1);

          r11 = CLAMP(BOUNDCONVERT(i11, valmin, valmax), 0.0, 1.0);
          r12 = CLAMP(BOUNDCONVERT(i12, valmin, valmax), 0.0, 1.0);
          r21 = CLAMP(BOUNDCONVERT(i21, valmin, valmax), 0.0, 1.0);
          r22 = CLAMP(BOUNDCONVERT(i22, valmin, valmax), 0.0, 1.0);

          xyzp1 = meshi->xyzpatch + 3*pfi->start + 3*i11;
          xyzp2 = meshi->xyzpatch + 3*pfi->start + 3*i21;
          if(ABS(r11-r22)<ABS(r12-r21)){
            glTexCoord1f(r11); glVertex3fv(xyzp1);
            glTexCoord1f(r12); glVertex3fv(xyzp1 + 3);
            glTexCoord1f(r22); glVertex3fv(xyzp2 + 3);
            glTexCoord1f(r11); glVertex3fv(xyzp1);
            glTexCoord1f(r22); glVertex3fv(xyzp2 + 3);
            glTexCoord1f(r12); glVertex3fv(xyzp1 + 3);

            glTexCoord1f(r11); glVertex3fv(xyzp1);
            glTexCoord1f(r22); glVertex3fv(xyzp2 + 3);
            glTexCoord1f(r21); glVertex3fv(xyzp2);
            glTexCoord1f(r11); glVertex3fv(xyzp1);
            glTexCoord1f(r21); glVertex3fv(xyzp2);
            glTexCoord1f(r22); glVertex3fv(xyzp2 + 3);
          }
          else{
            glTexCoord1f(r11); glVertex3fv(xyzp1);
            glTexCoord1f(r12); glVertex3fv(xyzp1 + 3);
            glTexCoord1f(r21); glVertex3fv(xyzp2);
            glTexCoord1f(r12); glVertex3fv(xyzp1 + 3);
            glTexCoord1f(r22); glVertex3fv(xyzp2 + 3);
            glTexCoord1f(r21); glVertex3fv(xyzp2);

            glTexCoord1f(r11); glVertex3fv(xyzp1);
            glTexCoord1f(r21); glVertex3fv(xyzp2);
            glTexCoord1f(r12); glVertex3fv(xyzp1 + 3);
            glTexCoord1f(r12); glVertex3fv(xyzp1 + 3);
            glTexCoord1f(r21); glVertex3fv(xyzp2);
            glTexCoord1f(r22); glVertex3fv(xyzp2 + 3);
          }
        }
      }
    }
  }

  glEnd();
  glDisable(GL_TEXTURE_1D);
}

/* ------------------ DrawBoundaryTexture ------------------------ */

void DrawBoundaryTexture(const meshdata *meshi){
  float r11, r12, r21, r22;
  int n;
  int nrow, ncol, irow, icol;
  float *patchvals;
  unsigned char *cpatchvals;
  float *xyzpatchcopy;
  int *patchblankcopy;
  float *patch_times;
  float *xyzpatch;
  int *patchblank;
  patchdata *patchi;
  float dboundx,dboundy,dboundz;
  float *xplt, *yplt, *zplt;

  CheckMemory;
  if(vis_threshold==1&&vis_onlythreshold==1&&do_threshold==1)return;

  if(hidepatchsurface==0){
    xplt=meshi->xplt;
    yplt=meshi->yplt;
    zplt=meshi->zplt;

    dboundx = (xplt[1]-xplt[0])/10.0;
    dboundy = (yplt[1]-yplt[0])/10.0;
    dboundz = (zplt[1]-zplt[0])/10.0;
  }

  patch_times=meshi->patch_times;
  xyzpatch=meshi->xyzpatch;
  patchblank=meshi->patchblank;
  patchi=patchinfo+meshi->patchfilenum;
  patchi = patchinfo + meshi->patchfilenum;

  int set_valmin, set_valmax;
  char *label;
  float ttmin, ttmax;

  label = patchi->label.shortlabel;
  GLUIGetOnlyMinMax(BOUND_PATCH, label, &set_valmin, &ttmin, &set_valmax, &ttmax);

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

   if(boundary_interface_faces==1)DrawMeshBoundaryFaces(patchi, ttmin, ttmax);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D,texture_patch_colorbar_id);

  CheckMemory;
  glBegin(GL_TRIANGLES);
  for(n=0;n<patchi->npatches;n++){
    int drawit;
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    CheckMemory;
    if(pfi->obst != NULL && pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0)continue;
    if(pfi->internal_mesh_face == 1)continue;

    drawit=0;
    if(pfi->vis==1&&pfi->dir==0)drawit=1;
    if(pfi->type !=INTERIORwall&&showpatch_both==1)drawit=1;
    if(drawit==1){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;
#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
#else
      patchvals = meshi->patchval_iframe + pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;

      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        patchblank2 = patchblank1 + ncol;
        xyzp2 = xyzp1 + 3*ncol;

        for(icol=0;icol<ncol-1;icol++){
          float cparm[4];

          cparm[0] = CLAMP(BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0.0, 1.0);
          cparm[1] = CLAMP(BOUNDCONVERT(IJKBF(irow, icol + 1), ttmin, ttmax), 0.0, 1.0);
          cparm[2] = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol), ttmin, ttmax), 0.0, 1.0);
          cparm[3] = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol + 1), ttmin, ttmax), 0.0, 1.0);
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            if(
               rgb_patch[4*(int)(255*cparm[0])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[1])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[2])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[3])+3]==0.0
               ){
              patchblank1++;
              patchblank2++;
              xyzp1+=3;
              xyzp2+=3;
              continue;
            }
            r11 = cparm[0];
            r12 = cparm[1];
            r21 = cparm[2];
            r22 = cparm[3];
            if(ABS(r11-r22)<ABS(r12-r21)){
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);

            }
            else{
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
            }
          }
          patchblank1++;
          patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  if(hidepatchsurface==1){
    glBegin(GL_TRIANGLES);
  }
  for(n=0;n<patchi->npatches;n++){
    int drawit;
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    CheckMemory;
    if(pfi->internal_mesh_face == 1)continue;
    if(pfi->obst!=NULL && pfi->obst->showtimelist!=NULL&& pfi->obst->showtimelist[itimes]==0)continue;

    drawit=0;
    if(pfi->vis==1&&pfi->dir>0){
      if(pfi->type ==INTERIORwall||showpatch_both==0){
        drawit=1;
      }
    }
    if(drawit==1){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;
#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);

#else
      if(patchi->compression_type != COMPRESSED_ZLIB)patchvals  = meshi->patchval_iframe + pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(pfi->dir){
          case XDIR:
            glTranslatef(dboundx,0.0,0.0);
            break;
          case YDIR:
            glTranslatef(0.0,-dboundy,0.0);
            break;
          case ZDIR:
            glTranslatef(0.00,0.0,dboundz);
            break;
          default:
            assert(FFALSE);
            break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;

        xyzp2 = xyzp1 + 3*ncol;
        patchblank2 = patchblank1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          float cparm[4];

          cparm[0] = CLAMP(BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0.0, 1.0);
          cparm[1] = CLAMP(BOUNDCONVERT(IJKBF(irow, icol + 1), ttmin, ttmax), 0.0, 1.0);
          cparm[2] = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol), ttmin, ttmax), 0.0, 1.0);
          cparm[3] = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol + 1), ttmin, ttmax), 0.0, 1.0);
          if(
               rgb_patch[4*(int)(255*cparm[0])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[1])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[2])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[3])+3]==0.0
               ){
            patchblank1++;
            patchblank2++;
            xyzp1+=3;
            xyzp2+=3;
            continue;
          }
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = cparm[0];
            r12 = cparm[1];
            r21 = cparm[2];
            r22 = cparm[3];
            if(ABS(cparm[0]-cparm[3])<ABS(cparm[1]-cparm[2])){
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
            }
            else{
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
            }
          }
          patchblank1++;
          patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
      if(hidepatchsurface==0){
        glEnd();
        glPopMatrix();
      }
    }
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  for(n=0;n<patchi->npatches;n++){
    int drawit;
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    CheckMemory;
    if(pfi->internal_mesh_face==1)continue;
    if(pfi->obst!=NULL && pfi->obst->showtimelist!=NULL && pfi->obst->showtimelist[itimes]==0)continue;

    drawit=0;
    if(pfi->vis==1&&pfi->dir<0){
      if(pfi->type ==INTERIORwall||showpatch_both==0){
        drawit=1;
      }
    }
    if(drawit==1){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;
#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
#else
      if(patchi->compression_type != COMPRESSED_ZLIB)patchvals  = meshi->patchval_iframe + pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(pfi->dir){
          case XDIRNEG:
            glTranslatef(-dboundx,0.0,0.0);
            break;
          case YDIRNEG:
            glTranslatef(0.0,dboundy,0.0);
            break;
          case ZDIRNEG:
            glTranslatef(0.0,0.0,-dboundz);
            break;
          default:
            assert(FFALSE);
            break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        patchblank2 = patchblank1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          float cparm[4];

          cparm[0] = CLAMP(BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0.0, 1.0);
          cparm[1] = CLAMP(BOUNDCONVERT(IJKBF(irow, icol + 1), ttmin, ttmax), 0.0, 1.0);
          cparm[2] = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol), ttmin, ttmax), 0.0, 1.0);
          cparm[3] = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol + 1), ttmin, ttmax), 0.0, 1.0);
          if(
               rgb_patch[4*(int)(255*cparm[0])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[1])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[2])+3]==0.0||
               rgb_patch[4*(int)(255*cparm[3])+3]==0.0
               ){
            patchblank1++;
            patchblank2++;
            xyzp1+=3;
            xyzp2+=3;
            continue;
          }
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = cparm[0];
            r12 = cparm[1];
            r21 = cparm[2];
            r22 = cparm[3];
            if(ABS(cparm[0]-cparm[3])<ABS(cparm[1]-cparm[2])){
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
            }
            else{
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
            }
          }
          patchblank1++;
          patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
      if(hidepatchsurface==0){
        glEnd();
        glPopMatrix();
      }
    }
  }
  if(hidepatchsurface==1){
    glEnd();
  }
  glDisable(GL_TEXTURE_1D);
}

/* ------------------ DrawBoundaryTextureThreshold ------------------------ */

void DrawBoundaryTextureThreshold(const meshdata *meshi){
  float r11, r12, r21, r22;
  int n,nn,nn1,nn2;
  int nrow, ncol, irow, icol;
  float *patchvals;
  unsigned char *cpatchvals;
#ifndef pp_BOUNDFRAME
  float *patchval_iframe;
#endif
  float *xyzpatchcopy;
  int *patchblankcopy;
  float *patch_times;
  float *xyzpatch;
  int *patchblank;
  patchdata *patchi;
  float *color11, *color12, *color21, *color22;
  float burn_color[4]={0.0,0.0,0.0,1.0};
  float clear_color[4]={1.0,1.0,1.0,1.0};

  if(vis_threshold==1&&vis_onlythreshold==1&&do_threshold==1)return;

  patch_times=meshi->patch_times;
  xyzpatch=meshi->xyzpatch;
  patchblank=meshi->patchblank;
  patchi=patchinfo+meshi->patchfilenum;
  switch(patchi->compression_type){
  case UNCOMPRESSED:
#ifndef pp_BOUNDFRAME
    assert(meshi->patchval_iframe!=NULL);
    patchval_iframe=meshi->patchval_iframe;
#endif
    break;
  case COMPRESSED_ZLIB:
    break;
  default:
    assert(FFALSE);
  }
  patchi = patchinfo + meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;

  int set_valmin, set_valmax;
  char *label;
  float ttmin, ttmax;

  label = patchi->label.shortlabel;
  GLUIGetMinMax(BOUND_PATCH, label, &set_valmin, &ttmin, &set_valmax, &ttmax);

  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D,texture_colorbar_id);

  nn =0;
  glBegin(GL_TRIANGLES);
  for(n=0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst!=NULL&&pfi->meshinfo!=NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    if(pfi->vis==1&&pfi->dir==0){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;

#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
#else
      patchvals = patchval_iframe + pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;
      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        patchblank2 = patchblank1 + ncol;
        nn2 = nn1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = CLAMP(BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0.0, 1.0);
            r12 = CLAMP(BOUNDCONVERT(IJKBF(irow, icol + 1), ttmin, ttmax), 0.0, 1.0);
            r21 = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol), ttmin, ttmax), 0.0, 1.0);
            r22 = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol + 1), ttmin, ttmax), 0.0, 1.0);

            color11=clear_color;
            color12=clear_color;
            color21=clear_color;
            color22=clear_color;
            if(meshi->thresholdtime[nn1+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol  ])color11=burn_color;
            if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1])color12=burn_color;
            if(meshi->thresholdtime[nn2+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol  ])color21=burn_color;
            if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1])color22=burn_color;
            if(color11==color12&&color11==color21&&color11==color22){
              glColor4fv(color11);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
            }
            else{
             if(ABS(r11-r22)<ABS(r12-r21)){
               glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
               glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
               glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
               glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
               glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
               glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
             }
             else{
               glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
               glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
               glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
               glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
               glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
               glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
             }
            }
          }
          patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  nn=0;
  glBegin(GL_TRIANGLES);
  for(n=0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst!=NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    if(pfi->vis==1&&pfi->dir>0){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;
#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
#else
      patchvals  = patchval_iframe + pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;
      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;

        xyzp2 = xyzp1 + 3*ncol;
        patchblank2 = patchblank1 + ncol;
        nn2 = nn1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = CLAMP(BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0.0, 1.0);
            r12 = CLAMP(BOUNDCONVERT(IJKBF(irow, icol + 1), ttmin, ttmax), 0.0, 1.0);
            r21 = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol), ttmin, ttmax), 0.0, 1.0);
            r22 = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol + 1), ttmin, ttmax), 0.0, 1.0);
            color11=clear_color;
            color12=clear_color;
            color21=clear_color;
            color22=clear_color;
            if(meshi->thresholdtime[nn1+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol  ])color11=burn_color;
            if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1])color12=burn_color;
            if(meshi->thresholdtime[nn2+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol  ])color21=burn_color;
            if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1])color22=burn_color;
            if(color11==color12&&color11==color21&&color11==color22){
              glColor4fv(color11);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
            }
            else{
             if(ABS(r11-r22)<ABS(r12-r21)){
                glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
                glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
                glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
                glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
                glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
                glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
              }
              else{
                glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
                glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
                glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
                glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
                glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
                glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
              }
            }
          }
          patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn=0;
  for(n=0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo;
    if(pfi->obst!=NULL&&pfi->meshinfo!=NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    if(pfi->vis==1&&pfi->dir<0){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;
#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
#else
      patchvals  = patchval_iframe + pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;
      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        patchblank2 = patchblank1 + ncol;
        nn2 = nn1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = CLAMP(BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0.0, 1.0);
            r12 = CLAMP(BOUNDCONVERT(IJKBF(irow, icol + 1), ttmin, ttmax), 0.0, 1.0);
            r21 = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol), ttmin, ttmax), 0.0, 1.0);
            r22 = CLAMP(BOUNDCONVERT(IJKBF(irow + 1, icol + 1), ttmin, ttmax), 0.0, 1.0);
            color11=clear_color;
            color12=clear_color;
            color21=clear_color;
            color22=clear_color;
            if(meshi->thresholdtime[nn1+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol  ])color11=burn_color;
            if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1])color12=burn_color;
            if(meshi->thresholdtime[nn2+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol  ])color21=burn_color;
            if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1])color22=burn_color;
            if(color11==color12&&color11==color21&&color11==color22){
              glColor4fv(color11);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
              glTexCoord1f(r12);glVertex3fv(xyzp1+3);
              glTexCoord1f(r11);glVertex3fv(xyzp1);
              glTexCoord1f(r21);glVertex3fv(xyzp2);
              glTexCoord1f(r22);glVertex3fv(xyzp2+3);
            }
            else{
             if(ABS(r11-r22)<ABS(r12-r21)){
                glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
                glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
                glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
                glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
                glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
                glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
              }
              else{
                glTexCoord1f(r11);glColor4fv(color11);glVertex3fv(xyzp1);
                glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
                glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
                glTexCoord1f(r12);glColor4fv(color12);glVertex3fv(xyzp1+3);
                glTexCoord1f(r21);glColor4fv(color21);glVertex3fv(xyzp2);
                glTexCoord1f(r22);glColor4fv(color22);glVertex3fv(xyzp2+3);
              }
            }
          }
          patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }
  glEnd();
  glDisable(GL_TEXTURE_1D);
}

/* ------------------ DrawBoundaryThresholdCellcenter ------------------------ */

void DrawBoundaryThresholdCellcenter(const meshdata *meshi){
  int n,nn,nn1;
  int nrow, ncol, irow, icol;
  float *xyzpatchcopy;
  int *patchblankcopy;
  float *patch_times;
  float *xyzpatch;
  int *patchblank;
  patchdata *patchi;
  float *color11;
  float burn_color[4]={0.0,0.0,0.0,1.0};
  float clear_color[4]={1.0,1.0,1.0,1.0};

  if(vis_threshold==1&&vis_onlythreshold==1&&do_threshold==1)return;

  patch_times=meshi->patch_times;
  xyzpatch=meshi->xyzpatch;
  patchblank=meshi->patchblank;
  patchi=patchinfo+meshi->patchfilenum;
  switch(patchi->compression_type){
  case UNCOMPRESSED:
    assert(meshi->cpatchval_iframe!=NULL);
    break;
  case COMPRESSED_ZLIB:
    assert(meshi->cpatchval_iframe_zlib!=NULL);
    break;
  default:
    assert(FFALSE);
  }
  patchi = patchinfo + meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  nn =0;
  glBegin(GL_TRIANGLES);
  for(n=0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst != NULL && pfi->meshinfo != NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    if(pfi->vis==1&&pfi->dir==0){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;

      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        patchblank2 = patchblank1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            color11=clear_color;
            if(meshi->thresholdtime[nn1+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol  ])color11=burn_color;

            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp1+3);
            glVertex3fv(xyzp2+3);

            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp2);
          }
          patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  nn=0;
  glBegin(GL_TRIANGLES);
  for(n=0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst!=NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    if(pfi->vis==1&&pfi->dir>0){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;

      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;

        xyzp2 = xyzp1 + 3*ncol;
        patchblank2 = patchblank1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            color11=clear_color;
            if(meshi->thresholdtime[nn1+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol  ])color11=burn_color;

            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp1+3);
            glVertex3fv(xyzp2+3);

            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp2);
          }
          patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn=0;
  for(n=0;n<patchi->npatches;n++){
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst != NULL && pfi->meshinfo != NULL){
      if(pfi->obst->showtimelist!=NULL&& pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
        nn += pfi->nrow*pfi->ncol;
        continue;
    }
    if(pfi->vis==1&&pfi->dir<0){
      nrow=pfi->nrow;
      ncol=pfi->ncol;
      xyzpatchcopy = xyzpatch + 3* pfi->start;
      patchblankcopy = patchblank + pfi->start;

      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        patchblank2 = patchblank1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            color11=clear_color;
            if(meshi->thresholdtime[nn1+icol  ]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol  ])color11=burn_color;

            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp1+3);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2);
            glVertex3fv(xyzp2+3);
          }
          patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }
  glEnd();
}

/* ------------------ MakeBoundaryMask ------------------------ */

void MakeBoundaryMask(patchdata *patchi){
  meshdata *meshi;
  int n;

  if(patchi->blocknumber < 0)return;
  meshi = meshinfo + patchi->blocknumber;
  if(meshi->boundary_mask != NULL|| meshi->npatchsize <= 0)return;
  NewMemory((void **)&meshi->boundary_mask, meshi->npatchsize);
  memset(meshi->boundary_mask, 0, meshi->npatchsize);
  for(n = 0;n < patchi->npatches; n++){
    int irow, ncol;
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    ncol = pfi->ncol;
    for(irow = 1; irow < pfi->nrow; irow++){
      int icol;

      for(icol = 1;icol < pfi->ncol;icol++){
        meshi->boundary_mask[pfi->start +IJKBF(irow, icol)] = 1;
      }
    }
  }
}

/* ------------------ DrawBoundaryCellCenter ------------------------ */

void DrawBoundaryCellCenter(const meshdata *meshi){
  int n, nn, nn1;
  int nrow, ncol, irow, icol;
  float *patchvals;
  unsigned char *cpatchvals;
#ifndef pp_BOUNDFRAME
  float *patchval_iframe;
#endif
  float *patch_times;
  patchdata *patchi;
  float *color11;

  float dboundx, dboundy, dboundz;
  float *xplt, *yplt, *zplt;
  float **patchventcolors;
  int set_valmin, set_valmax;
  char *label;
  float ttmin, ttmax;

  if(vis_threshold==1&&vis_onlythreshold==1&&do_threshold==1)return;

  if(hidepatchsurface==0){
    xplt = meshi->xplt;
    yplt = meshi->yplt;
    zplt = meshi->zplt;

    dboundx = (xplt[1]-xplt[0])/10.0;
    dboundy = (yplt[1]-yplt[0])/10.0;
    dboundz = (zplt[1]-zplt[0])/10.0;
  }

  patch_times = meshi->patch_times;
  patchventcolors = meshi->patchventcolors;
  patchi = patchinfo+meshi->patchfilenum;

  label = patchi->label.shortlabel;
  GLUIGetOnlyMinMax(BOUND_PATCH, label, &set_valmin, &ttmin, &set_valmax, &ttmax);
  if(ttmin>=ttmax){
    ttmin = 0.0;
    ttmax = 1.0;
  }

  switch(patchi->compression_type){
  case UNCOMPRESSED:
#ifndef pp_BOUNDFRAME
    patchval_iframe = meshi->patchval_iframe;
    if(patchval_iframe==NULL)return;
#endif
    break;
  case COMPRESSED_ZLIB:
    break;
  default:
    assert(FFALSE);
  }
  patchi = patchinfo+meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  nn = 0;
  glBegin(GL_TRIANGLES);
  for(n = 0;n<patchi->npatches;n++){
    int drawit;
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst != NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    drawit = 0;
    if(pfi->vis==1&&pfi->dir==0)drawit = 1;
#ifdef pp_PATCHFIX
    if(pfi->type==INTERIORwall)drawit = 1;
#else
    if(pfi->type==INTERIORwall&&showpatch_both==1)drawit = 1;
#endif
    if(pfi->obst == NULL && pfi->internal_mesh_face==1)drawit = 0;
    if(drawit==1){
      nrow = pfi->nrow;
      ncol = pfi->ncol;
#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
#else
      patchvals  = patchval_iframe+ pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = meshi->xyzpatch+3* pfi->start +3*irow*ncol;
        patchblank1 = meshi->patchblank+ pfi->start +irow*ncol;
        nn1 = nn+irow*ncol;
        xyzp2 = xyzp1+3*ncol;
        patchblank2 = patchblank1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          unsigned char cval;

          cval = CLAMP(255*BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0, 255);
          if(rgb_patch[4*cval+3]==0.0){
            patchblank1++;
            patchblank2++;
            xyzp1 += 3;
            xyzp2 += 3;
            continue;
          }
#ifdef pp_PATCHFIX
          {
#else
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
#endif
            if(patchventcolors==NULL){
              color11 = rgb_patch+4*cval;
              if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
                if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              }
            }
            else{
              color11 = patchventcolors[(irow*ncol+icol)];
            }
            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp1+3);
            glVertex3fv(xyzp2+3);

            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp2);
          }
          patchblank1++;
          patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);
#ifndef pp_PATCHFIX

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  nn = 0;
  if(hidepatchsurface==1){
    glBegin(GL_TRIANGLES);
  }
  for(n = 0;n<patchi->npatches;n++){
    int drawit;
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst!=NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    else if(pfi->internal_mesh_face==1){
      nn += pfi->nrow*pfi->ncol;
      continue;
    }
    drawit = 0;
    if(pfi->vis==1&&pfi->dir>0){
      if(pfi->type==INTERIORwall||showpatch_both==0){
        drawit = 1;
      }
    }
    if(pfi->obst == NULL && pfi->internal_mesh_face==1)drawit = 0;
    if(drawit==1){
      nrow = pfi->nrow;
      ncol = pfi->ncol;
#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
#else
      patchvals = patchval_iframe+ pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(pfi->dir){
        case XDIR:
          glTranslatef(dboundx, 0.0, 0.0);
          break;
        case YDIR:
          glTranslatef(0.0, -dboundy, 0.0);
          break;
        case ZDIR:
          glTranslatef(0.0, 0.0, dboundz);
          break;
        default:
          assert(FFALSE);
          break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = meshi->xyzpatch+3* pfi->start +3*irow*ncol;
        patchblank1 = meshi->patchblank+ pfi->start +irow*ncol;
        nn1 = nn+irow*ncol;

        xyzp2 = xyzp1+3*ncol;
        patchblank2 = patchblank1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          unsigned char cval;

          cval = CLAMP(255*BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0, 255);
          if(rgb_patch[4*cval+3]==0.0){
            patchblank1++;
            patchblank2++;
            xyzp1 += 3;
            xyzp2 += 3;
            continue;
          }
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            if(patchventcolors==NULL){
              color11 = rgb_patch+4*cval;
              if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
                if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              }
            }
            else{
              color11 = patchventcolors[irow*ncol+icol];
            }
            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp1+3);
            glVertex3fv(xyzp2+3);

            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp2);
          }
          patchblank1++;
          patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
      if(hidepatchsurface==0){
        glEnd();
        glPopMatrix();
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn = 0;
  for(n = 0;n<patchi->npatches;n++){
    int drawit;
    patchfacedata *pfi;

    pfi = patchi->patchfaceinfo + n;
    if(pfi->obst != NULL){
      if(pfi->obst->showtimelist!=NULL&&pfi->obst->showtimelist[itimes]==0){
        nn += pfi->nrow*pfi->ncol;
        continue;
      }
    }
    drawit = 0;
    if(pfi->vis==1&&pfi->dir<0){
      if(pfi->type==INTERIORwall||showpatch_both==0){
        drawit = 1;
      }
    }
    if(pfi->obst == NULL && pfi->internal_mesh_face==1)drawit = 0;
    if(drawit==1){
      nrow = pfi->nrow;
      ncol = pfi->ncol;
#ifdef pp_BOUNDFRAME
      patchvals = (float *)FRAMEGetSubFramePtr(patchi->frameinfo, meshi->patch_itime, n);
#else
      patchvals  = patchval_iframe+ pfi->start;
#endif
      if(patchi->compression_type == COMPRESSED_ZLIB)cpatchvals = meshi->cpatchval_iframe_zlib + pfi->start;
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(pfi->dir){
        case XDIRNEG:
          glTranslatef(-dboundx, 0.0, 0.0);
          break;
        case YDIRNEG:
          glTranslatef(0.0, dboundy, 0.0);
          break;
        case ZDIRNEG:
          glTranslatef(0.0, 0.0, -dboundz);
          break;
        default:
          assert(FFALSE);
          break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = meshi->xyzpatch+3* pfi->start +3*irow*ncol;
        patchblank1 = meshi->patchblank+ pfi->start +irow*ncol;
        nn1 = nn+irow*ncol;
        xyzp2 = xyzp1+3*ncol;
        patchblank2 = patchblank1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          unsigned char cval;

          cval = CLAMP(255*BOUNDCONVERT(IJKBF(irow, icol), ttmin, ttmax), 0, 255);
          if(rgb_patch[4*cval+3]==0.0){
            patchblank1++;
            patchblank2++;
            xyzp1 += 3;
            xyzp2 += 3;
            continue;
          }
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            if(patchventcolors==NULL){
              color11 = rgb_patch+4*cval;
              if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
                if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              }
            }
            else{
              color11 = patchventcolors[irow*ncol+icol];
            }
            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp1+3);

            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2);
            glVertex3fv(xyzp2+3);
          }
          patchblank1++;
          patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
      if(hidepatchsurface==0){
        glEnd();
        glPopMatrix();
      }
    }
    nn += pfi->nrow*pfi->ncol;
  }
  if(hidepatchsurface==1){
    glEnd();
  }
#endif
}

/* ------------------ DrawBoundaryFrame ------------------------ */

void DrawBoundaryFrame(int flag){
  int i;

  if(use_tload_begin==1 && global_times[itimes]<tload_begin)return;
  if(use_tload_end==1   && global_times[itimes]>tload_end)return;

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    IF_NOT_USEMESH_CONTINUE(USEMESH_DRAW,patchi->blocknumber);
    if(patchi->structured == NO && patchi->loaded == 1 && patchi->display == 1){
      if(flag == DRAW_OPAQUE){
        if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY){
          DrawGeomData(flag, NULL, patchi, GEOM_STATIC);
//          DrawGeomData(flag, NULL, patchi, GEOM_DYNAMIC); // only allow boundary files that do not move for now
        }
      }
      else{
        if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
          DrawGeomData(flag, NULL, patchi, GEOM_STATIC);
          DrawGeomData(flag, NULL, patchi, GEOM_DYNAMIC);
        }
      }
    }
  }
  if(flag == DRAW_TRANSPARENT)return;
  for(i = 0; i < npatchinfo; i++){
    patchdata *patchi;
    meshdata *meshi;

    patchi = patchinfo + i;
    IF_NOT_USEMESH_CONTINUE(USEMESH_DRAW, patchi->blocknumber);
    if(patchi->structured == NO)continue;
    meshi = meshinfo + patchi->blocknumber;
    if(meshi->use == 0)continue;
    if(patchi->loaded == 0 || patchi->display == 0 || patchi->shortlabel_index != iboundarytype)continue;
    if(patchi->npatches < 0)continue;

    if(vis_threshold==1&&do_threshold==1){
      if(patchi->patch_filetype==PATCH_STRUCTURED_CELL_CENTER){
        DrawBoundaryThresholdCellcenter(meshi);
      }
      else if(patchi->patch_filetype==PATCH_STRUCTURED_NODE_CENTER){
        DrawBoundaryTextureThreshold(meshi);
      }
    }
    else{
      if(patchi->patch_filetype==PATCH_STRUCTURED_CELL_CENTER){
        DrawBoundaryCellCenter(meshi);
      }
      else if(patchi->patch_filetype==PATCH_STRUCTURED_NODE_CENTER){
        DrawBoundaryTexture(meshi);
      }
    }
    if(vis_threshold==1&&vis_onlythreshold==1&&do_threshold==1)DrawOnlyThreshold(meshi);
  }
}

/* ------------------ UpdateBoundaryTypes ------------------------ */

void UpdateBoundaryTypes(void){
  int i;
  patchdata *patchi;

  nboundarytypes = 0;
  for(i=0;i<npatchinfo;i++){
    patchi = patchinfo+i;
    if(GetBoundaryIndex(patchi)==-1)boundarytypes[nboundarytypes++]=i;
  }
  for(i=0;i<npatchinfo;i++){
    patchi = patchinfo+i;
    patchi->shortlabel_index =GetBoundaryType(patchi);
  }
}

/* ------------------ GetBoundaryType ------------------------ */

int GetBoundaryType(const patchdata *patchi){
  int j;

  for(j=0;j<nboundarytypes;j++){
    patchdata *patchi2;

    patchi2 = patchinfo+boundarytypes[j];
    if(strcmp(patchi->label.shortlabel,patchi2->label.shortlabel)==0)return j;
  }
  return -1;
}

/* ------------------ UpdateBoundaryType ------------------------ */

void UpdateBoundaryType(void){
  int i;

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->boundary==1&&patchi->loaded==1&&patchi->display==1&&patchi->shortlabel_index==iboundarytype)return;
  }

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->boundary==1&&patchi->loaded==1&&patchi->display==1){
      iboundarytype = GetBoundaryIndex(patchi);
      return;
    }
  }
  iboundarytype = -1;
}

/* ------------------ BoundaryCompare ------------------------ */

int BoundaryCompare(const void *arg1, const void *arg2){
  patchdata *patchi, *patchj;

  patchi = patchinfo + *(int *)arg1;
  patchj = patchinfo + *(int *)arg2;

  if(strcmp(patchi->menulabel_base,patchj->menulabel_base)<0)return -1;
  if(strcmp(patchi->menulabel_base,patchj->menulabel_base)>0)return 1;
  if(strcmp(patchi->menulabel_suffix,patchj->menulabel_suffix)<0)return -1;
  if(strcmp(patchi->menulabel_suffix,patchj->menulabel_suffix)>0)return 1;
  if(patchi->blocknumber<patchj->blocknumber)return -1;
  if(patchi->blocknumber>patchj->blocknumber)return 1;
  return 0;
}

/* ------------------ UpdateBoundaryMenuLabels ------------------------ */

void UpdateBoundaryMenuLabels(void){
  int i;
  patchdata *patchi;
  char label[128];

  if(npatchinfo>0){
    for(i=0;i<npatchinfo;i++){
      patchi = patchinfo + i;
      STRCPY(patchi->menulabel, "");
      STRCPY(patchi->menulabel_suffix, "");
      if(nmeshes == 1){
        STRCAT(patchi->menulabel, patchi->label.longlabel);
      }
      else{
        meshdata *patchmesh;

        patchmesh = meshinfo + patchi->blocknumber;
        sprintf(label,"%s",patchmesh->label);
        STRCAT(patchi->menulabel,label);
      }
    STRCPY(patchi->menulabel_suffix, patchi->label.longlabel);
    if(patchi->structured == NO){
        if(patchi->filetype_label==NULL||strlen(patchi->filetype_label)==0||strcmp(patchi->filetype_label, "INCLUDE_GEOM")==0){
          if(strlen(patchi->gslicedir) != 0){
            STRCAT(patchi->menulabel, ", ");
            STRCAT(patchi->menulabel, patchi->gslicedir);
            STRCPY(patchi->menulabel_suffix, patchi->gslicedir);
          }
        }
        if(patchi->filetype_label!=NULL&&strlen(patchi->filetype_label)>0){
          if(strcmp(patchi->filetype_label, "INBOUND_FACES")==0){
            STRCPY(patchi->menulabel_suffix, "in boundary");
          }
          if(strcmp(patchi->filetype_label, "EXIMBND_FACES")==0){
            STRCPY(patchi->menulabel_suffix, "EXIM faces");
          }
          if(strcmp(patchi->filetype_label, "CUT_CELLS") == 0){
            STRCAT(patchi->menulabel_suffix, "Cut cell faces");
          }
        }
      }
      if(FILE_EXISTS(patchi->comp_file)==YES){
        patchi->file=patchi->comp_file;
        patchi->compression_type=COMPRESSED_ZLIB;
      }
      else{
        patchi->file=patchi->reg_file;
        patchi->compression_type=UNCOMPRESSED;
      }
      if(showfiles==1){
        STRCAT(patchi->menulabel,", ");
        STRCAT(patchi->menulabel,patchi->file);
      }
      if(patchi->compression_type==COMPRESSED_ZLIB){
        STRCAT(patchi->menulabel," (ZLIB)");
      }
    }
    FREEMEMORY(patchorderindex);
    NewMemory((void **)&patchorderindex, sizeof(int)*npatchinfo);
    for(i = 0;i < npatchinfo;i++){
      patchorderindex[i] = i;
    }
    qsort((int *)patchorderindex, (size_t)npatchinfo, sizeof(int), BoundaryCompare);
  }
}

#define COUNT_DUPLICATES 1
#define FIND_DUPLICATES 0

/* ------------------ CompareMeshResolution ------------------------ */

int CompareMeshResolution(int dir, meshdata *meshi, meshdata *meshj){
  float grid_eps;
  float *dxyzi, *dxyzj;

  dxyzi = meshi->dxyz_orig;
  dxyzj = meshj->dxyz_orig;
  grid_eps = MIN(dxyzi[dir],dxyzj[dir])/2.0;

  if(ABS(dxyzi[dir]-dxyzj[dir]) < grid_eps)return 0;
  if(dxyzi[dir] > dxyzj[dir])return 1;
  return -1;
}

/* ------------------ IsPatchDuplicate ------------------------ */


int IsBoundaryDuplicate(patchdata *patchi, int flag){
  int j;
  float *xyzmini, *xyzmaxi;
  meshdata *meshi;
  flowlabels *labeli;

  if(flag==FIND_DUPLICATES&&boundaryslicedup_option ==SLICEDUP_KEEPALL)return 0;
  if(patchi->structured == YES || patchi->patch_filetype != PATCH_GEOMETRY_SLICE)return 0;
  if(patchi->filetype_label==NULL||strcmp(patchi->filetype_label,"INCLUDE_GEOMETRY")!=0)return 0;
  if(patchi->dir == 0)return 0;
  xyzmini = patchi->xyz_min;
  xyzmaxi = patchi->xyz_max;
  meshi = meshinfo + patchi->blocknumber;
  labeli = &(patchi->label);
  for(j=0;j<npatchinfo;j++){ // identify duplicate slices
    patchdata *patchj;
    float *xyzminj, *xyzmaxj, grid_eps;
    meshdata *meshj;
    flowlabels *labelj;

    patchj = patchinfo + j;
    labelj = &(patchj->label);
    meshj = meshinfo + patchj->blocknumber;

    if(patchj==patchi||patchj->skip==1)continue;
    if(patchj->structured == YES||patchj->patch_filetype!=PATCH_GEOMETRY_SLICE)continue;
    if((patchi->dir != patchj->dir)||patchj->dir==0)continue;
    if(strcmp(labeli->longlabel, labelj->longlabel) != 0)continue;

    grid_eps = MAX(meshi->dxyz_orig[patchi->dir],meshj->dxyz_orig[patchi->dir]);

    xyzminj = patchj->xyz_min;
    xyzmaxj = patchj->xyz_max;
    if(MAXDIFF3(xyzmini, xyzminj) < grid_eps&&MAXDIFF3(xyzmaxi, xyzmaxj) < grid_eps){
      if(flag == COUNT_DUPLICATES)return 1;
      if(boundaryslicedup_option ==SLICEDUP_KEEPFINE  &&CompareMeshResolution(patchi->dir, meshi, meshj)>=0)return 1;
      if(boundaryslicedup_option ==SLICEDUP_KEEPCOARSE&&CompareMeshResolution(patchi->dir, meshi, meshj)<=0)return 1;
    }
  }
  return 0;
}

/* ------------------ CountBoundarySliceDups ------------------------ */

int CountBoundarySliceDups(void){
  int i, count;

  count = 0;
  for(i = 0; i < npatchinfo; i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    count += IsBoundaryDuplicate(patchi, COUNT_DUPLICATES);
  }
  return count;
}

/* ------------------ UpdateBoundarySliceDups ------------------------ */

void UpdateBoundarySliceDups(void){
  int i;

  for(i = 0;i < npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    patchi->skip = 0;
  }
  // look for duplicate patches
  for(i = 0;i < npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->structured == YES||patchi->patch_filetype!=PATCH_GEOMETRY_SLICE)continue;
    patchi->skip = IsBoundaryDuplicate(patchi, FIND_DUPLICATES);
  }
}

/* ------------------ GetBoundaryParams ------------------------ */

void GetBoundaryParams(void){
  int i;

  for(i = 0;i < npatchinfo;i++){
    patchdata *patchi;
    float *xyz_min, *xyz_max;
    int *ijk;
    meshdata *meshi;
    float *xplt, *yplt, *zplt;
    float dxyz[3];

    patchi = patchinfo + i;
    patchi->dir = 0;

    xyz_min = patchi->xyz_min;
    xyz_min[0] = 0.0;
    xyz_min[1] = 0.0;
    xyz_min[2] = 0.0;

    xyz_max = patchi->xyz_max;
    xyz_max[0] = 0.0;
    xyz_max[1] = 0.0;
    xyz_max[2] = 0.0;
    if(patchi->structured == YES || patchi->patch_filetype != PATCH_GEOMETRY_SLICE)continue;

    ijk = patchi->ijk;
    meshi = meshinfo + patchi->blocknumber;

    xplt = meshi->xplt;
    yplt = meshi->yplt;
    zplt = meshi->zplt;

    xyz_min[0] = xplt[ijk[0]];
    xyz_min[1] = yplt[ijk[2]];
    xyz_min[2] = zplt[ijk[4]];

    xyz_max[0] = xplt[ijk[1]];
    xyz_max[1] = yplt[ijk[3]];
    xyz_max[2] = zplt[ijk[5]];

    dxyz[0] = ABS(xyz_max[0] - xyz_min[0]);
    dxyz[1] = ABS(xyz_max[1] - xyz_min[1]);
    dxyz[2] = ABS(xyz_max[2] - xyz_min[2]);
    if(dxyz[0] < MIN(dxyz[1], dxyz[2]))patchi->dir = 0;
    if(dxyz[1] < MIN(dxyz[0], dxyz[2]))patchi->dir = 1;
    if(dxyz[2] < MIN(dxyz[0], dxyz[1]))patchi->dir = 2;
  }
  UpdateBoundarySliceDups();
  nboundaryslicedups = CountBoundarySliceDups();
}

/* ------------------ UncompressBoundaryDataGEOM ------------------------ */

void UncompressBoundaryDataGEOM(patchdata *patchi, int local_iframe){
  unsigned int n_compressed_data;
  uLongf n_uncompressed_data;
  unsigned char *compressed_data, *uncompressed_data;
  float *geom_vals;

  geom_vals         = patchi->geom_vals;
  compressed_data   = (unsigned char*)geom_vals + patchi->cvals_offsets[local_iframe];
  uncompressed_data = patchi->cbuffer;

  n_compressed_data   = patchi->cvals_sizes[local_iframe];
  n_uncompressed_data = patchi->cbuffer_size;
  UnCompressZLIB(uncompressed_data, &n_uncompressed_data, compressed_data, n_compressed_data);
}

/* ------------------ UncompressBoundaryDataBNDF ------------------------ */

void UncompressBoundaryDataBNDF(meshdata *meshi,int local_iframe){
  unsigned int countin;
  uLongf countout;
  unsigned char *compressed_data;

  compressed_data = meshi->cpatchval_zlib+meshi->zipoffset[local_iframe];
  countin = meshi->zipsize[local_iframe];
  countout=meshi->npatchsize;
  UnCompressZLIB(meshi->cpatchval_iframe_zlib,&countout,compressed_data,countin);
}

/* ------------------ UpdateHideBoundarySurface ------------------------ */

void UpdateHideBoundarySurface(void){
  int hidepatchsurface_old;

  hidepatchsurface_old=hidepatchsurface;
  if(setpatchchopmin==1||setpatchchopmax==1){
    hidepatchsurface=0;
  }
  else{
    hidepatchsurface=1;
  }
  if(hidepatchsurface_old!=hidepatchsurface)updatefacelists=1;
}
