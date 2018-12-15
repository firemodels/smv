#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include GLUT_H

#include "smv_endian.h"
#include "update.h"
#include "smokeviewvars.h"
#include "compress.h"

#define FIRST_TIME 1

/* ------------------ OutputBoundaryData ------------------------ */

void OutputBoundaryData(char *csvfile, char *patchfile, meshdata *meshi, int first_time, float *csvtime){
  int iframe;
  float *vals;
  float *xplt, *yplt, *zplt;
  FILE *csvstream=NULL;
  int max_frame;

  if(patchout_tmin > patchout_tmax)return;
  if(first_time== FIRST_TIME){
    csvstream = fopen(csvfile, "w");
  }
  else{
    csvstream = fopen(csvfile, "a");
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

    for(ipatch=0;ipatch<meshi->npatches;ipatch++){
      int i1, i2, j1, j2, k1, k2;
      int imin, imax, jmin, jmax, kmin, kmax;
      int i, j, k;

      i1 = meshi->pi1[ipatch];
      i2 = meshi->pi2[ipatch];
      j1 = meshi->pj1[ipatch];
      j2 = meshi->pj2[ipatch];
      k1 = meshi->pk1[ipatch];
      k2 = meshi->pk2[ipatch];
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

      fprintf(csvstream,"time:,%f,patch %i, of, %i\n",pt,ipatch+1,meshi->npatches);
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

/* ------------------ GetBoundaryFaceDir ------------------------ */

int GetBoundaryFaceDir(meshdata *meshi, int i1, int i2, int j1, int j2, int k1, int k2,
  int *blockonpatch, meshdata **meshonpatch){
  int i;

  *meshonpatch = NULL;
  if(i1 == i2){
    for(i = 0; i < meshi->nbptrs; i++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[i];
      if(j1 == bc->ijk[JMIN] && j2 == bc->ijk[JMAX] &&
        k1 == bc->ijk[KMIN] && k2 == bc->ijk[KMAX]){
        if(i1 == bc->ijk[IMIN]){
          bc->patchvis[0] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
        }
        if(i1 == bc->ijk[IMAX]){
          bc->patchvis[1] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
        }
        if(i1 == bc->ijk[IMIN])return(-1);
        if(i1 == bc->ijk[IMAX])return(1);
      }
    }
  }
  else if(j1 == j2){
    for(i = 0; i < meshi->nbptrs; i++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[i];
      if(i1 == bc->ijk[IMIN] && i2 == bc->ijk[IMAX] &&
        k1 == bc->ijk[KMIN] && k2 == bc->ijk[KMAX]){
        if(j1 == bc->ijk[JMIN]){
          bc->patchvis[2] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
        }
        if(j1 == bc->ijk[JMAX]){
          bc->patchvis[3] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
        }
        if(j1 == bc->ijk[JMIN])return(2);
        if(j1 == bc->ijk[JMAX])return(-2);
      }
    }
  }
  else if(k1 == k2){
    for(i = 0; i < meshi->nbptrs; i++){
      blockagedata *bc;

      bc = meshi->blockageinfoptrs[i];
      if(i1 == bc->ijk[IMIN] && i2 == bc->ijk[IMAX] &&
        j1 == bc->ijk[JMIN] && j2 == bc->ijk[JMAX]){
        if(k1 == bc->ijk[KMIN]){
          bc->patchvis[4] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
        }
        if(k1 == bc->ijk[KMAX]){
          bc->patchvis[5] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
        }
        if(k1 == bc->ijk[KMIN])return(-3);
        if(k1 == bc->ijk[KMAX])return(3);
      }
    }
  }
  *blockonpatch = -1;
  *meshonpatch = NULL;
  if(i1 == i2){
    if(i1 == 0 && j1 == 0 && j2 == meshi->jbar&&k1 == 0 && k2 == meshi->kbar){
      return(1);
    }
    if(i1 == meshi->ibar&&j1 == 0 && j2 == meshi->jbar&&k1 == 0 && k2 == meshi->kbar){
      return(-1);
    }
  }
  else if(j1 == j2){
    if(j1 == 0 && i1 == 0 && i2 == meshi->ibar&&k1 == 0 && k2 == meshi->kbar){
      return(-1);
    }
    if(j1 == meshi->jbar&&i1 == 0 && i2 == meshi->ibar&&k1 == 0 && k2 == meshi->kbar){
      return(1);
    }
  }
  else if(k1 == k2){
    if(k1 == 0 && j1 == 0 && j2 == meshi->jbar&&i1 == 0 && i2 == meshi->ibar){
      return(1);
    }
    if(k1 == meshi->kbar&&j1 == 0 && j2 == meshi->jbar&&i1 == 0 && i2 == meshi->ibar){
      return(-1);
    }
  }
  return(0);
}

/* ------------------ GetBoundaryFace2Dir ------------------------ */

int GetBoundaryFace2Dir(meshdata *meshi, int i1, int i2, int j1, int j2, int k1, int k2, int patchdir,
  int *blockonpatch, meshdata **meshonpatch){
  int i;
  blockagedata *bc;

  *meshonpatch = NULL;
  if(i1 == i2){
    for(i = 0; i < meshi->nbptrs; i++){
      bc = meshi->blockageinfoptrs[i];
      if(j1 == bc->ijk[JMIN] && j2 == bc->ijk[JMAX] &&
        k1 == bc->ijk[KMIN] && k2 == bc->ijk[KMAX]){
        if(i1 == bc->ijk[IMIN] && patchdir == XDIRNEG){
          bc->patchvis[0] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
          return(-1);
        }
        if(i1 == bc->ijk[IMAX] && patchdir == XDIR){
          bc->patchvis[1] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
          return(1);
        }
      }
    }
  }
  else if(j1 == j2){
    for(i = 0; i < meshi->nbptrs; i++){
      bc = meshi->blockageinfoptrs[i];
      if(i1 == bc->ijk[IMIN] && i2 == bc->ijk[IMAX] &&
        k1 == bc->ijk[KMIN] && k2 == bc->ijk[KMAX]){
        if(j1 == bc->ijk[JMIN] && patchdir == YDIRNEG){
          bc->patchvis[2] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
          return(2);
        }
        if(j1 == bc->ijk[JMAX] && patchdir == YDIR){
          bc->patchvis[3] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
          return(-2);
        }
      }
    }
  }
  else if(k1 == k2){
    for(i = 0; i < meshi->nbptrs; i++){
      bc = meshi->blockageinfoptrs[i];
      if(i1 == bc->ijk[IMIN] && i2 == bc->ijk[IMAX] &&
        j1 == bc->ijk[JMIN] && j2 == bc->ijk[JMAX]){
        if(k1 == bc->ijk[KMIN] && patchdir == ZDIRNEG){
          bc->patchvis[4] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
          return(-3);
        }
        if(k1 == bc->ijk[KMAX] && patchdir == ZDIR){
          bc->patchvis[5] = 0;
          *blockonpatch = i;
          *meshonpatch = meshi;
          return(3);
        }
      }
    }
  }
  *blockonpatch = -1;
  *meshonpatch = NULL;
  if(i1 == i2){
    if(i1 == 0 && j1 == 0 && j2 == meshi->jbar&&k1 == 0 && k2 == meshi->kbar){
      return(1);
    }
    if(i1 == meshi->ibar&&j1 == 0 && j2 == meshi->jbar&&k1 == 0 && k2 == meshi->kbar){
      return(-1);
    }
  }
  else if(j1 == j2){
    if(j1 == 0 && i1 == 0 && i2 == meshi->ibar&&k1 == 0 && k2 == meshi->kbar){
      return(-1);
    }
    if(j1 == meshi->jbar&&i1 == 0 && i2 == meshi->ibar&&k1 == 0 && k2 == meshi->kbar){
      return(1);
    }
  }
  else if(k1 == k2){
    if(k1 == 0 && j1 == 0 && j2 == meshi->jbar&&i1 == 0 && i2 == meshi->ibar){
      return(1);
    }
    if(k1 == meshi->kbar&&j1 == 0 && j2 == meshi->jbar&&i1 == 0 && i2 == meshi->ibar){
      return(-1);
    }
  }
  return(0);
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
  if(mesh_boundary==NO&&NodeInBlockage(meshi, i, j, k, &imesh, &iblockage) == YES)return YES;
  for(ii = 0; ii < meshi->nvents; ii++){
    ventdata *vi;

    vi = meshi->ventinfo + ii;
    if(vi->hideboundary == 1){
      switch(dir){
      case XDIR:
        if(vi->imin == i&&i == vi->imax&&
          vi->jmin < j&&j < vi->jmax&&
          vi->kmin < k&&k < vi->kmax){
          if((i == 0 && meshi->is_extface[0] == 0) || (i == meshi->ibar&&meshi->is_extface[1] == 0)){
            if(NodeInBlockage(meshi, i, j, k, &imesh, &iblockage) == 1)return YES;
          }
          return NO;
        }
        break;
      case YDIR:
        if(vi->jmin == j&&j == vi->jmax&&
          vi->imin < i&&i < vi->imax&&
          vi->kmin < k&&k < vi->kmax){
          if((j == 0 && meshi->is_extface[2] == 0) || (j == meshi->jbar&&meshi->is_extface[3] == 0)){
            if(NodeInBlockage(meshi, i, j, k, &imesh, &iblockage) == 1)return YES;
          }
          return NO;
        }
        break;
      case ZDIR:
        if(vi->kmin == k&&k == vi->kmax&&
          vi->imin < i&&i < vi->imax&&
          vi->jmin < j&&j < vi->jmax){
          if((k == 0 && meshi->is_extface[4] == 0) || (k == meshi->kbar&&meshi->is_extface[5] == 0)){
            if(NodeInBlockage(meshi, i, j, k, &imesh, &iblockage) == 1)return YES;
          }
          return NO;
        }
        break;
      default:
        ASSERT(FFALSE);
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

  for(ii = 0; ii < meshi->nvents; ii++){
    ventdata *vi;
    int imin, jmin, kmin, imax, jmax, kmax;

    vi = meshi->ventinfo + ii;
    if(vi->hideboundary == 1 && option == 0)continue;
    if(vi->dir2 != dir)continue;
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
      ASSERT(FFALSE);
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
        meshdata *meshblock;

        iii = (k - k1)*(i2 + 1 - i1) + (i - i1);
        if(patchblank[iii] == GAS)continue;
        patchblank[iii] = NodeInBlockage(meshi, i, j1, k, &imesh, &iblockage);
        if(imesh != -1){
          meshblock = meshinfo + imesh;
          ASSERT(iblockage >= 0 && iblockage < meshblock->nbptrs);
          meshi->blockonpatch[ipatch] = iblockage;
          meshi->meshonpatch[ipatch] = meshblock;
        }
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
    ASSERT(FFALSE);
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
  int *vis_boundaries;
  float *xyzpatch;
  int *patchdir, *boundary_row, *boundary_col;
  int *blockstart;
  int *patchblank;
  int iblock;
  blockagedata *bc;
  patchdata *patchi;
  float *color11, *color12, *color21, *color22;
  float *color_black;
  meshdata *meshblock;

  if(vis_threshold==0||vis_onlythreshold==0||do_threshold==0)return;

  patch_times = meshi->patch_times;
  vis_boundaries = meshi->vis_boundaries;
  xyzpatch = meshi->xyzpatch_threshold;
  patchdir = meshi->patchdir;
  boundary_row = meshi->boundary_row;
  boundary_col = meshi->boundary_col;
  blockstart = meshi->blockstart;
  patchblank = meshi->patchblank;
  patchi = patchinfo+meshi->patchfilenum;
  switch(patchi->compression_type){
  case UNCOMPRESSED:
    ASSERT(meshi->cpatchval_iframe!=NULL);
    break;
  case COMPRESSED_ZLIB:
    ASSERT(meshi->cpatchval_iframe_zlib!=NULL);
    break;
  default:
    ASSERT(FFALSE);
  }
  patchi = patchinfo+meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  nn = 0;
  color_black = &char_color[0];
  glBegin(GL_TRIANGLES);
  glColor4fv(color_black);
  for(n = 0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(vis_boundaries[n]==1&&patchdir[n]==0){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      xyzpatchcopy = xyzpatch+3*blockstart[n];
      patchblankcopy = patchblank+blockstart[n];
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
    nn += boundary_row[n]*boundary_col[n];
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  glBegin(GL_TRIANGLES);
  glColor4fv(color_black);
  nn = 0;
  for(n = 0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    if(iblock!=-1){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(meshi->vis_boundaries[n]==1&&meshi->patchdir[n]>0){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      xyzpatchcopy = xyzpatch+3*blockstart[n];
      patchblankcopy = patchblank+blockstart[n];
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
    nn += boundary_row[n]*boundary_col[n];
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn = 0;
  for(n = 0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(vis_boundaries[n]==1&&patchdir[n]<0){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      xyzpatchcopy = xyzpatch+3*blockstart[n];
      patchblankcopy = patchblank+blockstart[n];
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
    nn += boundary_row[n]*boundary_col[n];
  }
  glEnd();
}

/* ------------------ GetBoundaryDataZlib ------------------------ */

void GetBoundaryDataZlib(patchdata *patchi, unsigned char *data, int ndata,
  float *local_times, unsigned int *zipoffset, unsigned int *zipsize, int ntimes_local){
  FILE *stream;
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

  stream = fopen(patchi->file, "rb");
  if(stream==NULL)return;

  FSEEK(stream, 12, SEEK_CUR);
  fread(&version, 4, 1, stream);
  FSEEK(stream, 16, SEEK_CUR);
  fread(&npatches, 4, 1, stream);
  if(version==0){
    local_skip = 6*npatches*4;
  }
  else{
    local_skip = 9*npatches*4;
  }
  return_code = FSEEK(stream, local_skip, SEEK_CUR);
  if(return_code!=0){
    fclose(stream);
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

    if(fread(&local_time, 4, 1, stream)==0)break;
    skip_frame = 1;
    if(local_time>time_max){
      time_max = local_time;
      skip_frame = 0;
      local_count++;
    }
    if(fread(&compressed_size, 4, 1, stream)==0)break;
    if(skip_frame==0&&local_count%boundframestep==0){
      if(fread(datacopy, 1, compressed_size, stream)==0)break;
    }
    else{
      FSEEK(stream, compressed_size, SEEK_CUR);
    }

    if(skip_frame==1||local_count%boundframestep!=0)continue;
    i++;
    if(i>=ntimes_local)break;
    ASSERT(i<ntimes_local);
    local_times[i] = local_time;
    zipoffset[i] = offset;
    zipsize[i] = compressed_size;
    datacopy += compressed_size;
    offset += compressed_size;
  }
  fclose(stream);
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

void GetBoundaryHeader2(char *file,
  int *version,
  int *pi1, int *pi2,
  int *pj1, int *pj2,
  int *pk1, int *pk2,
  int *patchdir){
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
  fread(version, 4, 1, stream);
  FSEEK(stream, 16, SEEK_CUR);
  fread(&npatches, 4, 1, stream);
  for(i = 0;i<npatches;i++){
    buffer[6] = 0;
    if(*version==0){
      fread(buffer, 4, 6, stream);
    }
    else{
      fread(buffer, 4, 9, stream);
    }
    pi1[i] = buffer[0];
    pi2[i] = buffer[1];
    pj1[i] = buffer[2];
    pj2[i] = buffer[3];
    pk1[i] = buffer[4];
    pk2[i] = buffer[5];
    patchdir[i] = buffer[6];
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
  streamsize = fopen(sizefile, "r");
  if(streamsize==NULL){
    *nframes = 0;
    *buffersize = 0;

    strcpy(sizefile, patchi->size_file);
    strcat(sizefile, ".sz");
    streamsize = fopen(sizefile, "r");

    stream = fopen(patchi->file, "rb");
    if(stream==NULL){
      if(streamsize!=NULL)fclose(streamsize);
      return;
    }

    streamsize = fopen(sizefile, "w");
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
    streamsize = fopen(sizefile, "r");
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
    if(local_count%boundframestep!=0)continue;

    nf++;
    bsize += compressed_size;
  }
  *nframes = nf;
  *buffersize = bsize;
  fclose(streamsize);
}

/* ------------------ ReadBoundaryBndf ------------------------ */

FILE_SIZE ReadBoundaryBndf(int ifile, int flag, int *errorcode){
  int error;
  FILE_SIZE lenfile;
  int patchfilenum;
  float *xyzpatchcopy;
  float *xyzpatch_ignitecopy;
  int *patchblankcopy;
  int maxtimes_boundary;
  int n;
  int ii;
  int headersize, framesize;
  int statfile;
  STRUCTSTAT statbuffer;
  int nbb;
  int ibartemp,jbartemp,kbartemp;
  float *xplttemp,*yplttemp,*zplttemp;
  int blocknumber;
  patchdata *patchi,*patchbase;
  meshdata *meshi;
  float patchmin_global, patchmax_global;
  int local_first,nsize,iblock;
  int npatchvals;
  char patchcsvfile[1024];
  int framestart;
  int first_time=1;

  int nn;
  int filenum;
  char *patchscale;
  int ncompressed_buffer;
  char *file;
  float read_time, total_time;
  int file_unit, wallcenter=0;
  FILE_SIZE return_filesize = 0;

  patchi = patchinfo + ifile;
  if(patchi->loaded==0&&flag==UNLOAD)return 0;
  if(strcmp(patchi->label.shortlabel,"wc")==0)wallcenter=1;

  if(output_patchdata==1){
      sprintf(patchcsvfile,"%s_bndf_%04i.csv",fdsprefix,ifile);
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
  if(flag!=UNLOAD&&meshi->patchfilenum >= 0 && meshi->patchfilenum < npatchinfo){
    patchdata *patchold;

    patchold = patchinfo + meshi->patchfilenum;
    if(patchold->loaded == 1){
      int errorcode2;

      ReadBoundaryBndf(meshi->patchfilenum, UNLOAD, &errorcode2);
    }
  }
  meshi->patchfilenum = ifile;
  filenum = meshi->patchfilenum;

#ifndef pp_FSEEK
  if(flag==RELOAD)flag = LOAD;
#endif

  if(flag!=RELOAD&&filenum>=0&&filenum<npatchinfo){
    patchi->loaded=0;
  }

  patchi->display=0;
  plotstate=GetPlotState(DYNAMIC_PLOTS);

  nbb = meshi->nbptrs;
  if(nbb==0)nbb=1;
  updatefaces=1;
  *errorcode=0;
  if(flag != RELOAD){
    FREEMEMORY(meshi->blockonpatch);
    FREEMEMORY(meshi->meshonpatch);
    FREEMEMORY(meshi->patchdir);
    FREEMEMORY(meshi->patch_surfindex);
    FREEMEMORY(meshi->pi1);
    FREEMEMORY(meshi->pi2);
    FREEMEMORY(meshi->pj1);
    FREEMEMORY(meshi->pj2);
    FREEMEMORY(meshi->pk1);
    FREEMEMORY(meshi->pk2);
    FREEMEMORY(meshi->boundary_row);
    FREEMEMORY(meshi->boundary_col);
    FREEMEMORY(meshi->blockstart);
    FREEMEMORY(meshi->zipoffset);
    FREEMEMORY(meshi->zipsize);
    FREEMEMORY(meshi->boundarytype);
    FREEMEMORY(meshi->vis_boundaries);
    FREEMEMORY(meshi->xyzpatch);
    FREEMEMORY(meshi->xyzpatch_threshold);
    FREEMEMORY(meshi->patchventcolors);
    FREEMEMORY(meshi->cpatchval);
    FREEMEMORY(meshi->cpatchval_zlib);
    FREEMEMORY(meshi->cpatchval_iframe_zlib);
    FREEMEMORY(meshi->patchval);
    FREEMEMORY(meshi->thresholdtime);
    FREEMEMORY(meshi->patch_times);
    FREEMEMORY(meshi->patchblank);

    if(meshi->patch_contours != NULL){
      int i;

      ASSERT(meshi->npatches > 0 && meshi->maxtimes_boundary > 0);
      for(i = 0;i < meshi->npatches*meshi->maxtimes_boundary;i++){
        if(meshi->patch_contours[i] != NULL){
          FreeContour(meshi->patch_contours[i]);
        }
      }
      FREEMEMORY(meshi->patch_contours);
    }
  }

  if(flag==UNLOAD){
    int enableflag=1;
    int i;

    UpdateBoundaryType();
    UpdateUnitDefs();
    UpdateTimes();
    meshi->npatches=0;
    for(i=0;i<npatchinfo;i++){
      patchdata *patchii;

      patchii = patchinfo + i;
      if(patchii->loaded==1&&patchii->compression_type==COMPRESSED_ZLIB){
        enableflag=0;
        break;
      }
    }
    if(enableflag==1)EnableBoundaryGlui();
    patchi->ntimes_old=0;
    patchi->ntimes=0;
    updatemenu=1;
    PrintMemoryInfo;
    return 0;
  }
  if(ifile>=0&&ifile<npatchinfo){
    Global2LocalBoundaryBounds(patchi->label.shortlabel);
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

  UpdateBoundaryHist(patchi);

  lenfile = strlen(file);
  if(patchi->compression_type==UNCOMPRESSED){
    FORTgetpatchsizes1(&file_unit,file,&meshi->npatches,&headersize,&error,lenfile);
    if(error!=0){
      ReadBoundary(ifile,UNLOAD,&error);
      *errorcode=1;
      return 0;
    }
  }
  else{
    meshi->npatches=0;
    GetBoundaryHeader(file,&meshi->npatches,&patchmin,&patchmax);
    patchmin_global = patchmin;
    patchmax_global = patchmax;
  }
  if(meshi->npatches>0){
    if(
       NewResizeMemory(meshi->meshonpatch,       sizeof(meshdata *)*meshi->npatches)==0||
       NewResizeMemory(meshi->blockonpatch,      sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->patchdir,          sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->patch_surfindex,   sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->pi1,               sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->pi2,               sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->pj1,               sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->pj2,               sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->pk1,               sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->pk2,               sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->boundarytype,      sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->vis_boundaries,    sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->boundary_row,      sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->boundary_col,      sizeof(int)*meshi->npatches)==0||
       NewResizeMemory(meshi->blockstart,        sizeof(int)*(1+meshi->npatches))==0){
      *errorcode=1;
      if(patchi->compression_type==UNCOMPRESSED){
        FORTclosefortranfile(&file_unit);
      }
      ReadBoundary(ifile,UNLOAD,&error);
      return 0;
    }
  }

  if(patchi->compression_type==UNCOMPRESSED){
    FORTgetpatchsizes2(&file_unit,&patchi->version,
      &meshi->npatches,&meshi->npatchsize,
      meshi->pi1,meshi->pi2,meshi->pj1,meshi->pj2,meshi->pk1,meshi->pk2,meshi->patchdir,
      &headersize,&framesize);

    // loadpatchbysteps
    //  0 - load entire uncompressed data set
    //  1 - load uncompressed data set one frame at a time
    //  2 - load compressed data set

    loadpatchbysteps=UNCOMPRESSED_ALLFRAMES;
    if(flag==LOAD||flag==RELOAD){
      maxtimes_boundary = MAXFRAMES+51;
      statfile=STAT(file,&statbuffer);
      if(statfile==0&&framesize!=0){
        int file_frames;

        patchi->ntimes_old = patchi->ntimes;
        patchi->ntimes = (statbuffer.st_size-headersize)/framesize;
        file_frames=patchi->ntimes+51;
        if(file_frames<maxtimes_boundary)maxtimes_boundary=file_frames;
      }
      meshi->maxtimes_boundary=maxtimes_boundary;


  /*
  If the min and max boundary file values are specified then we don't have
  to read in the whole file to determine the bounds.  In this case, memory is allocated
  one time step at a time rather than for all time steps.
  */

      if(statfile==0&&(setpatchmin==1||setpatchmax==1)&&cache_boundarydata==0)loadpatchbysteps=UNCOMPRESSED_BYFRAME;
    }
  }
  else{
    int nnsize=0;
    int i;

    GetBoundaryHeader2(file,
      &patchi->version,
      meshi->pi1,meshi->pi2,
      meshi->pj1,meshi->pj2,
      meshi->pk1,meshi->pk2,
      meshi->patchdir);
    for(i=0;i<meshi->npatches;i++){
      int ii1, ii2, jj1, jj2, kk1, kk2;

      ii1=meshi->pi1[i];
      ii2=meshi->pi2[i];
      jj1=meshi->pj1[i];
      jj2=meshi->pj2[i];
      kk1=meshi->pk1[i];
      kk2=meshi->pk2[i];
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
        FORTclosefortranfile(&file_unit);
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
  meshi->blockstart[0]=0;
  for(n=0;n<meshi->nbptrs;n++){
    blockagedata *bc;
    int j;

    bc=meshi->blockageinfoptrs[n];
    for(j=0;j<6;j++){
      bc->patchvis[j]=1;
    }
  }

  for(n=0;n<meshi->npatches;n++){
    float dxx, dyy, dzz;
    float dxx2, dyy2, dzz2;
    float dx_factor, dy_factor, dz_factor;
    int i1, i2, j1, j2, k1, k2;
    int *is_extface;
    float ig_factor_x, ig_factor_y, ig_factor_z;
    float block_factor_x, block_factor_y, block_factor_z;


    i1=meshi->pi1[n];
    i2=meshi->pi2[n];
    j1=meshi->pj1[n];
    j2=meshi->pj2[n];
    k1=meshi->pk1[n];
    k2=meshi->pk2[n];
    if(patchi->version==0){
      meshi->patchdir[n]= GetBoundaryFaceDir(meshi,i1,i2,j1,j2,k1,k2,
        meshi->blockonpatch+n,meshi->meshonpatch+n);
    }
    else{
      int patchdir;

      patchdir=meshi->patchdir[n];
      GetBoundaryFace2Dir(meshi,i1,i2,j1,j2,k1,k2,patchdir,
        meshi->blockonpatch+n,meshi->meshonpatch+n);
      if(meshi->patchdir[n]==YDIR||meshi->patchdir[n]==YDIRNEG){
        meshi->patchdir[n]=-meshi->patchdir[n];
      }
    }
    meshi->patch_surfindex[n]=0;
    dxx = 0.0;
    dyy = 0.0;
    dzz = 0.0;

    ig_factor_x = ABS(meshi->xplt[1] - meshi->xplt[0]) / 10.0;
    ig_factor_y = ABS(meshi->yplt[1] - meshi->yplt[0]) / 10.0;
    ig_factor_z = ABS(meshi->zplt[1] - meshi->zplt[0]) / 10.0;
    block_factor_x = ig_factor_x;
    block_factor_y = ig_factor_y;
    block_factor_z = ig_factor_z;

    switch(meshi->patchdir[n]){
    case XDIRNEG:
      meshi->patch_surfindex[n]=0;
      dxx = -meshi->xplt[1]*ig_factor_x;
      dxx2 = -meshi->xplt[1]*block_factor_x;
      break;
    case XDIR:
      meshi->patch_surfindex[n]=1;
      dxx = meshi->xplt[1]*ig_factor_x;
      dxx2 = meshi->xplt[1]*block_factor_x;
      break;
    case YDIRNEG:
      meshi->patch_surfindex[n]=2;
      dyy = meshi->yplt[1]*ig_factor_y;
      dyy2 = meshi->yplt[1]*block_factor_y;
      break;
    case YDIR:
      meshi->patch_surfindex[n]=3;
      dyy = -meshi->yplt[1]*ig_factor_y;
      dyy2 = -meshi->yplt[1]*block_factor_y;
      break;
    case ZDIRNEG:
      meshi->patch_surfindex[n]=4;
      dzz = -meshi->zplt[1]*ig_factor_z;
      dzz2 = -meshi->zplt[1]*block_factor_z;
      break;
    case ZDIR:
      meshi->patch_surfindex[n]=5;
      dzz = meshi->zplt[1]*ig_factor_z;
      dzz2 = meshi->zplt[1]*block_factor_z;
      break;
    default:
      ASSERT(FFALSE);
    }


    meshi->boundarytype[n]=INTERIORwall;
    is_extface = meshi->is_extface;
    if(i1==i2){
      int ext_wall, mesh_boundary;

      meshi->boundary_col[n] = j2 + 1 - j1;
      meshi->boundary_row[n] = k2 + 1 - k1;

      ext_wall=0;
      mesh_boundary = NO;
      if(j1==0&&j2==jbartemp&&k1==0&&k2==kbartemp){
        if(i1==0||i2==ibartemp){
          mesh_boundary = YES;
//xxx
          if(is_extface[0]==1&&i1 == 0){
            ext_wall = 1;
            meshi->boundarytype[n] = LEFTwall;
          }
          if(is_extface[1]==1&&i2 == ibartemp){
            ext_wall = 1;
            meshi->boundarytype[n] = RIGHTwall;
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
            *patchblankcopy++ = NodeInInternalVent(meshi,i1,j,k,1,mesh_boundary,wallcenter);
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

      meshi->boundary_col[n] = i2 + 1 - i1;
      meshi->boundary_row[n] = k2 + 1 - k1;

      ext_wall=0;
      mesh_boundary = NO;
      if(i1==0&&i2==ibartemp&&k1==0&&k2==kbartemp){
        if(j1==0||j2==jbartemp){
          mesh_boundary = YES;
          if(is_extface[2]==1&&j1 == 0){
            ext_wall = 1;
            meshi->boundarytype[n] = FRONTwall;
          }
          if(is_extface[3]==1&&j2 == jbartemp){
            ext_wall = 1;
            meshi->boundarytype[n] = BACKwall;
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

      meshi->boundary_col[n] = i2 + 1 - i1;
      meshi->boundary_row[n] = j2 + 1 - j1;

      ext_wall=0;
      mesh_boundary = NO;
      if(i1==0&&i2==ibartemp&&j1==0&&j2==jbartemp){
        if(k1==0||k2==kbartemp){
          mesh_boundary = YES;
          if(is_extface[4]==1&&k1 == 0){
            ext_wall = 1;
            meshi->boundarytype[n] = DOWNwall;
          }
          if(is_extface[5]==1&&k2 == kbartemp){
            ext_wall = 1;
            meshi->boundarytype[n] = UPwall;
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
    meshi->blockstart[n+1]=meshi->blockstart[n]+meshi->boundary_row[n]*meshi->boundary_col[n];
    meshi->vis_boundaries[n]=vis_boundary_type[meshi->boundarytype[n]];
  }

  for(n=0;n<meshi->nbptrs;n++){
    blockagedata *bc;
    int j;

    bc=meshi->blockageinfoptrs[n];
    bc->patchvis[6]=0;
    for(j=0;j<6;j++){
      bc->patchvis[6]+=bc->patchvis[j];
    }
    if(bc->patchvis[6]!=0){
      bc->patchvis[6]=1;
    }
  }

  meshi->patchval = NULL;
  switch(loadpatchbysteps){
  case UNCOMPRESSED_ALLFRAMES:
    while(meshi->patchval==NULL&&maxtimes_boundary>100){
      maxtimes_boundary-=50;
      meshi->maxtimes_boundary=maxtimes_boundary;
      NewResizeMemory(meshi->patchval,sizeof(float)*maxtimes_boundary*meshi->npatchsize);
    }
    if(meshi->patchval==NULL){
      NewResizeMemory(meshi->patchval,sizeof(float)*maxtimes_boundary*meshi->npatchsize);
    }
    break;
  case UNCOMPRESSED_BYFRAME:
    npatchvals = meshi->npatchsize*meshi->maxtimes_boundary;
    if(
      NewResizeMemory(meshi->patchval, sizeof(float)*meshi->npatchsize)==0||
      NewResizeMemory(meshi->cpatchval,sizeof(unsigned char)*npatchvals)==0){
      *errorcode=1;
      FORTclosefortranfile(&file_unit);
      ReadBoundary(ifile,UNLOAD,&error);
      return 0;
    }
    break;
  case COMPRESSED_ALLFRAMES:
    GetBoundarySizeInfo(patchi, &maxtimes_boundary, &ncompressed_buffer);
    NewResizeMemory(meshi->cpatchval_zlib,       sizeof(unsigned char)*ncompressed_buffer);
    NewResizeMemory(meshi->cpatchval_iframe_zlib,sizeof(unsigned char)*meshi->npatchsize);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }

  NewResizeMemory(meshi->patch_times,sizeof(float)*maxtimes_boundary);
  NewResizeMemory(meshi->zipoffset,  sizeof(unsigned int)*maxtimes_boundary);
  NewResizeMemory(meshi->zipsize,    sizeof(unsigned int)*maxtimes_boundary);
  if(meshi->patch_times==NULL){
    *errorcode=1;
    FORTclosefortranfile(&file_unit);
    ReadBoundary(ifile,UNLOAD,&error);
    return 0;
  }
  if(loadpatchbysteps==COMPRESSED_ALLFRAMES){
    GetBoundaryDataZlib(patchi,meshi->cpatchval_zlib,ncompressed_buffer,
      meshi->patch_times,meshi->zipoffset,meshi->zipsize,maxtimes_boundary);
    meshi->npatch_times=maxtimes_boundary;
    framestart = 0;
    return_filesize += ncompressed_buffer;
  }
  else{
    if(meshi->patchval == NULL){
      *errorcode = 1;
      FORTclosefortranfile(&file_unit);
      ReadBoundary(ifile, UNLOAD, &error);
      return 0;
    }
    if(flag == RELOAD&&patchi->ntimes_old > 0){
      framestart = patchi->ntimes_old;
    }
    else{
      meshi->npatch_times = 0;
      framestart = 0;
    }
  }
  START_TIMER(read_time);
  for(ii=framestart;ii<maxtimes_boundary;){
    if(loadpatchbysteps==UNCOMPRESSED_BYFRAME){
      meshi->patchval_iframe = meshi->patchval;
      meshi->cpatchval_iframe = meshi->cpatchval + ii*meshi->npatchsize;
    }
    else if(loadpatchbysteps==UNCOMPRESSED_ALLFRAMES){
      meshi->patchval_iframe = meshi->patchval + ii*meshi->npatchsize;
    }
    meshi->patch_timesi = meshi->patch_times + ii;

    error=0;
    if(loadpatchbysteps==UNCOMPRESSED_ALLFRAMES||loadpatchbysteps==UNCOMPRESSED_BYFRAME){
      if(ii==framestart&&framestart>0){
        int framesizes;

        framesizes = framesize*framestart-8;
        FORTskipdata(&file_unit,&framesizes);
        local_first = 0;
      }
      for(n=0;n<boundframestep;n++){
        if(error==0){
          int npatchval_iframe;
          int filesize;

          FORTgetpatchdata(&file_unit,&meshi->npatches,
          meshi->pi1,meshi->pi2,meshi->pj1,meshi->pj2,meshi->pk1,meshi->pk2,
          meshi->patch_timesi,meshi->patchval_iframe,&npatchval_iframe,&filesize, &error);
          return_filesize += filesize;
        }
      }
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
        if(loadpatchbysteps==COMPRESSED_ALLFRAMES)UncompressBoundaryDataFrame(meshi,ii);
        for(n=0;n<meshi->npatches;n++){
          meshdata *meshblock;
          float dval;
          int j;

          iblock=meshi->blockonpatch[n];
          meshblock = meshi->meshonpatch[n];
          nsize=meshi->boundary_row[n]*meshi->boundary_col[n];
          ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
          if(iblock!=-1&&meshblock!=NULL){
            switch(loadpatchbysteps){
            case UNCOMPRESSED_ALLFRAMES:
            case UNCOMPRESSED_BYFRAME:
              for(j=0;j<nsize;j++){
                if(meshi->thresholdtime[nn+j]<0.0&&meshi->patchval_iframe[nn+j]>=temp_threshold){
                  meshi->thresholdtime[nn+j]=meshi->patch_times[ii];
                }
              }
              break;
            case COMPRESSED_ALLFRAMES:
              dval = (patchmax-patchmin)/255.0;
              for(j=0;j<nsize;j++){
                float val;
                int ival;

                ival = meshi->cpatchval_iframe_zlib[nn+j];
                val = patchmin + dval*ival;
                if(meshi->thresholdtime[nn+j]<0.0&&val>=temp_threshold){
                  meshi->thresholdtime[nn+j]=meshi->patch_times[ii];
                }
              }
              break;
            default:
              ASSERT(FFALSE);
              break;
            }
          }
          nn+=nsize;
        }
      }
    }
    if(loadpatchbysteps==UNCOMPRESSED_BYFRAME){
      if(output_patchdata==1){
        OutputBoundaryData(patchcsvfile,patchi->file,meshi,first_time,meshi->patch_timesi);
        first_time=0;
      }
      GetBoundaryColors2(
        meshi->patchval_iframe, meshi->npatchsize, meshi->cpatchval_iframe,
                 setpatchmin,&patchmin, setpatchmax,&patchmax,
                 &patchmin_global, &patchmax_global,
                 nrgb_full,
                 &patchi->extreme_min,&patchi->extreme_max);
    }
    CheckMemory;
    if(error!=0)break;
    if(settmax_b!=0&&*meshi->patch_timesi>tmax_b)break;

    switch(loadpatchbysteps){
      case UNCOMPRESSED_ALLFRAMES:
      case UNCOMPRESSED_BYFRAME:
        if(!(settmin_b!=0&&*meshi->patch_timesi<tmin_b)){
           meshi->npatch_times++;
          patchi->ntimes=meshi->npatch_times;
          if(meshi->npatch_times + 1 > maxtimes_boundary){
            PRINTF("reallocating memory\n");
            maxtimes_boundary = meshi->npatch_times + 50; /* this + 50 must match - 50 below */
            meshi->maxtimes_boundary=maxtimes_boundary;
            if(
              ResizeMemory((void **)&meshi->patchval,   maxtimes_boundary*meshi->npatchsize*sizeof(float))==0||
              ResizeMemory((void **)&meshi->patch_times,maxtimes_boundary*sizeof(float))==0
             ){
              *errorcode=1;
              ReadBoundary(ifile,UNLOAD,&error);
              FORTclosefortranfile(&file_unit);
              return 0;
            }
          }
          ii++;
        }
        break;
      case COMPRESSED_ALLFRAMES:
        ii++;
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
  }
  STOP_TIMER(read_time);

  /* convert patch values into integers pointing to an rgb color table */

  if(loadpatchbysteps==UNCOMPRESSED_ALLFRAMES){
    if(output_patchdata==1){
      OutputBoundaryData(patchcsvfile,patchi->file,meshi,FIRST_TIME,NULL);
    }
    npatchvals = meshi->npatch_times*meshi->npatchsize;
    if(npatchvals==0||NewResizeMemory(meshi->cpatchval,sizeof(unsigned char)*npatchvals)==0){
      *errorcode=1;
      FORTclosefortranfile(&file_unit);
      ReadBoundary(ifile,UNLOAD,&error);
      return 0;
    }
  }
  if(NewResizeMemory(colorlabelpatch,MAXRGB*sizeof(char *))==0){
    *errorcode=1;
    if(loadpatchbysteps!=COMPRESSED_ALLFRAMES){
      FORTclosefortranfile(&file_unit);
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
        FORTclosefortranfile(&file_unit);
      }
      ReadBoundary(ifile,UNLOAD,&error);
      return 0;
    }
  }
  patchscale = patchi->scale;
  patchbase = patchinfo + GetBoundaryIndex(patchi);
  patchi->loaded=1;
  iboundarytype=GetBoundaryType(patchi);
  switch(loadpatchbysteps){
  case UNCOMPRESSED_ALLFRAMES:
  {
    int patchstart;

    patchstart = patchi->ntimes_old*meshi->npatchsize;
    GetBoundaryColors3(patchi, meshi->patchval, patchstart, npatchvals, meshi->cpatchval,
      setpatchmin, &patchmin, setpatchmax, &patchmax,
      &patchmin_global, &patchmax_global,
      nrgb, colorlabelpatch, patchscale, boundarylevels256,
      &patchi->extreme_min, &patchi->extreme_max);
  }
    break;
  case UNCOMPRESSED_BYFRAME:
    GetBoundaryLabels(
      patchmin, patchmax,
      colorlabelpatch,patchscale,boundarylevels256,nrgb);
    break;
  case COMPRESSED_ALLFRAMES:
    GetBoundaryLabels(
      patchmin, patchmax,
      colorlabelpatch,patchscale,boundarylevels256,nrgb);
    break;
  default:
    ASSERT(FFALSE);
    break;
  }
  strcpy(patchbase->scale,patchi->scale);
  if(do_threshold==1){
    meshi->surface_tempmax=patchmax_global;
    meshi->surface_tempmin=patchmin_global;
  }

  Local2GlobalBoundaryBounds(patchi->label.shortlabel);
  UpdateBoundaryListIndex(patchfilenum);

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
  }
  if(cache_boundarydata==0){
    FREEMEMORY(meshi->patchval);
  }
  patchi->loaded=1;
  patchi->display=1;
  iboundarytype=GetBoundaryType(patchi);
  ShowBoundaryMenu(ShowEXTERIORwallmenu);
  for(n = 0;n<meshi->npatches;n++){
    meshi->vis_boundaries[n] = vis_boundary_type[meshi->boundarytype[n]];
  }
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  if(patchi->compression_type==COMPRESSED_ZLIB)DisableBoundaryGlui();
  UpdateTimes();
  UpdateUnitDefs();
  UpdateChopColors();
  PrintMemoryInfo;
  IdleCB();

  STOP_TIMER(total_time);

  if(return_filesize > 1000000000){
    PRINTF(" - %.1f GB in %.1f s\n", (float)return_filesize / 1000000000., total_time);
  }
  else if(return_filesize > 1000000){
    PRINTF(" - %.1f MB in %.1f s\n", (float)return_filesize / 1000000., total_time);
  }
 else{
   PRINTF(" - %.0f kB in %.1f s\n", (float)return_filesize / 1000., total_time);
  }
  glutPostRedisplay();
  return return_filesize;
}

#ifdef pp_GEOMC
/* ------------------ ReadGeomDataSize ------------------------ */

void GetGeomDataSize(char *filename,int *ntimes,int *nvars,int *error){

  float time;
  int one, version;
  int nvert_s, nvert_d, nface_s, nface_d;
  FILE *stream=NULL;
  int returncode;
  int nvars_local, ntimes_local;

  *error=1;
  if(filename==NULL)return;
  stream = fopen(filename,"rb");
  if(stream==NULL)printf(" The boundary element file name, %s, does not exist",filename);

  *error = 0;

  FORTREAD(&one, 1, stream);
  FORTREAD(&version, 1, stream);
  ntimes_local = 0;
  nvars_local = 0;
  for(;;){
    int nvals[4], nskip;

    FORTREAD(&time, 1, stream);
    if(returncode==0)break;
    FORTREAD(nvals, 4, stream);
    if(returncode==0)break;
    nvert_s = nvals[0];
    nface_s = nvals[1];
    nvert_d = nvals[2];
    nface_d = nvals[3];
    nskip = 0;
    if(nvert_s>0)nskip += 4 + 4*nvert_s + 4;
    if(nface_s>0)nskip += 4 + 4*nface_s + 4;
    if(nvert_d>0)nskip += 4 + 4*nvert_d + 4;
    if(nface_d>0)nskip += 4 + 4*nface_d + 4;
    if(fseek(stream, nskip, SEEK_CUR)!=0)break;
    nvars_local += nvert_s+nvert_d+nface_s+nface_d;
    ntimes_local++;
  }
  *nvars = nvars_local;
  *ntimes = ntimes_local;
  fclose(stream);
}

/* ------------------ ReadGeomData ------------------------ */

FILE_SIZE GetGeomData(char *filename, int ntimes, int nvals, float *times, int *nstatics, int *ndynamics, float *vals, int *error){
  FILE_SIZE file_size;

  int i, one, nvars;
  int nvert_s, ntri_s, nvert_d, ntri_d;
  int version;
  int returncode;
  float time;

  FILE *stream;

  file_size = 0;
  *error = 1;
  if(filename==NULL)return 0;
  stream = fopen(filename, "rb");
  if(stream==NULL){
    printf(" The boundary file name %s does not exist\n",filename);
    return 0;
  }

  *error = 0;
  FORTREAD(&one, 1, stream);
  FORTREAD(&version, 1, stream);
  file_size = 2*(4+4+4);
  nvars = 0;
  for(i = 0; i<ntimes; i++){
    int nvals[4];

    FORTREAD(&time, 1, stream);
    times[i] = time;
    if(returncode==0)break;
    file_size += (4+4+4);
    FORTREAD(nvals, 4, stream);
    nvert_s = nvals[0];
    ntri_s = nvals[1];
    nvert_d = nvals[2];
    ntri_d = nvals[3];
    file_size += (4+4*4+4);
    nstatics[i] = nvert_s+ntri_s;

    if(nvert_s>0){
      FORTREAD(vals+nvars, nvert_s, stream);
      if(returncode==0)break;
      file_size += (4+4*nvert_s+4);
    }
    nvars += nvert_s;

    if(ntri_s>0){
      FORTREAD(vals+nvars, ntri_s, stream);
      if(returncode==0)break;
      file_size += (4+4*ntri_s+4);
    }
    nvars += ntri_s;

    ndynamics[i] = nvert_d+ntri_d;
    if(nvert_d>0){
      FORTREAD(vals+nvars, nvert_d, stream);
      if(returncode==0)break;
      file_size += (4+4*nvert_d+4);
    }
    nvars += nvert_d;

    if(ntri_d>0){
      FORTREAD(vals+nvars, ntri_d, stream);
      if(returncode==0)break;
      file_size += (4+4*ntri_d+4);
    }
    nvars += ntri_d;
  }
  fclose(stream);
  return file_size;
}

#endif

/* ------------------ ReadGeomData ------------------------ */

FILE_SIZE ReadGeomData(patchdata *patchi, slicedata *slicei, int load_flag, int *errorcode){
  char *file;
  int ntimes_local;
  int i;
  int nvals;
  float patchmin_global, patchmax_global;
  int n;
  int error;
  FILE_SIZE return_filesize = 0;
  float total_time;

  // 1
  // time
  // nstatic
  // vals_1, ...vals_nstatic
  // ndynamic
  // vals_1, ... vals_ndyamic

  if(patchi->structured == YES)return 0;

  START_TIMER(total_time);
  file = patchi->file;

  patchi->loaded = 0;
  patchi->display = 0;
  if(slicei != NULL){
    slicei->loaded = 0;
    slicei->display = 0;
    slicei->ntimes = 0;
    slicei->times = NULL;
  }
  patchi->bounds.defined=0;

  FREEMEMORY(patchi->geom_nstatics);
  FREEMEMORY(patchi->geom_ndynamics);
  FREEMEMORY(patchi->geom_ivals_static);
  FREEMEMORY(patchi->geom_ivals_dynamic);
  FREEMEMORY(patchi->geom_vals);
  FREEMEMORY(patchi->geom_ivals);
  FREEMEMORY(patchi->geom_times);
  if(load_flag==UNLOAD){
    plotstate = GetPlotState(DYNAMIC_PLOTS);
    if(patchi->boundary==1)UpdateBoundaryType();
    UpdateUnitDefs();
    UpdateTimes();
    return 0;
  }
  if(patchi->skip == 1)return 0;

  //GetGeomDataHeader(file,&ntimes,&nvals);
  endian_smv = GetEndian();

#ifdef pp_GEOMC
  GetGeomDataSize(file, &ntimes_local, &nvals, &error);
#else
  {
    int lenfile;

    lenfile=strlen(file);
    FORTgetgeomdatasize(file, &ntimes_local, &nvals, &error, lenfile);
  }
#endif

  if(nvals==0){
    if(load_flag!=UPDATE_HIST)PRINTF(" - no data\n");
    return 0;
  }
  if(nvals>0&&ntimes_local>0){
    NewMemory((void **)&patchi->geom_nstatics, ntimes_local*sizeof(int));
    NewMemory((void **)&patchi->geom_ndynamics, ntimes_local*sizeof(int));
    NewMemory((void **)&patchi->geom_times, ntimes_local*sizeof(float));
    NewMemory((void **)&patchi->geom_ivals_static, ntimes_local*sizeof(int *));
    NewMemory((void **)&patchi->geom_ivals_dynamic, ntimes_local*sizeof(int *));
    NewMemory((void **)&patchi->geom_vals, nvals*sizeof(float));
    NewMemory((void **)&patchi->geom_ivals, nvals*sizeof(char));
  }

  if(load_flag == UPDATE_HIST){
#ifdef pp_GEOMC
    GetGeomData(file, ntimes_local, nvals, patchi->geom_times,
      patchi->geom_nstatics, patchi->geom_ndynamics, patchi->geom_vals, &error);
#else
    {
    int filesize,lenfile;

    lenfile=strlen(file);
    FORTgetgeomdata(file, &ntimes_local, &nvals, patchi->geom_times,
      patchi->geom_nstatics, patchi->geom_ndynamics, patchi->geom_vals, &filesize, &error, lenfile);
    }
#endif
    ResetHistogram(patchi->histogram, NULL, NULL);
    UpdateHistogram(patchi->geom_vals, NULL, nvals, patchi->histogram);
    CompleteHistogram(patchi->histogram);
    return 0;
  }
  else{
    int filesize;

#ifdef pp_GEOMC
    filesize=GetGeomData(file, ntimes_local, nvals, patchi->geom_times,
      patchi->geom_nstatics, patchi->geom_ndynamics, patchi->geom_vals, &error);
#else
    {
      int lenfile;

      lenfile=strlen(file);
      FORTgetgeomdata(file, &ntimes_local, &nvals, patchi->geom_times,
        patchi->geom_nstatics, patchi->geom_ndynamics, patchi->geom_vals, &filesize, &error, lenfile);
    }
#endif
    return_filesize += filesize;
  }

  patchi->ngeom_times = ntimes_local;
  patchi->geom_nvals = nvals;
  patchi->geom_ivals_static[0] = patchi->geom_ivals;
  patchi->geom_ivals_dynamic[0] = patchi->geom_ivals_static[0]+patchi->geom_nstatics[0];
  for(i = 1;i<ntimes_local;i++){
    patchi->geom_ivals_static[i] = patchi->geom_ivals_dynamic[i-1]+patchi->geom_ndynamics[i-1];
    patchi->geom_ivals_dynamic[i] = patchi->geom_ivals_static[i]+patchi->geom_nstatics[i];
  }

  patchi->loaded = 1;
  patchi->display = 1;

  if(slicei == NULL){
    if (colorlabelpatch != NULL) {
      for (n = 0; n < MAXRGB; n++) {
        FREEMEMORY(colorlabelpatch[n]);
      }
      FREEMEMORY(colorlabelpatch);
    }
    if (NewMemory((void **)&colorlabelpatch, MAXRGB * sizeof(char *)) == 0) {
      ReadGeomData(patchi, NULL, UNLOAD, &error);
      return 0;
    }
    for (n = 0; n < MAXRGB; n++) {
      colorlabelpatch[n] = NULL;
    }
    for (n = 0; n < nrgb; n++) {
      if (NewMemory((void **)&colorlabelpatch[n], 11) == 0) {
        ReadGeomData(patchi, NULL, UNLOAD, &error);
        return 0;
      }
    }
    GetBoundaryColors3(patchi, patchi->geom_vals, 0, patchi->geom_nvals, patchi->geom_ivals,
      setpatchmin, &patchmin, setpatchmax, &patchmax,
      &patchmin_global, &patchmax_global,
      nrgb, colorlabelpatch, patchi->scale, boundarylevels256,
      &patchi->extreme_min, &patchi->extreme_max);
    FREEMEMORY(patchi->geom_vals);  // slice files keep data loaded
  }
  else {
    int slicetype;
    boundsdata *sb;
    char *scale;
    float qmin, qmax;

    slicetype = GetSliceBoundsIndex(slicei);
    sb = slicebounds + slicetype;
    sb->label = &(slicei->label);

    slicei->loaded = 1;
    slicei->display = 1;
    slicei->ntimes = patchi->ngeom_times;
    slicei->times = patchi->geom_times;

    UpdateLoadedLists();
    GetSliceDataBounds(slicei, &qmin, &qmax);
    slicei->globalmin = qmin;
    slicei->globalmax = qmax;
    AdjustSliceBounds(slicei, &qmin, &qmax);
    slicei->valmin = qmin;
    slicei->valmax = qmax;
    slicei->valmin_data = qmin;
    slicei->valmax_data = qmax;
    for (i = 0; i < 256; i++){
      slicei->qval256[i] = (qmin*(255 - i) + qmax*i) / 255;
    }
    void UpdateSliceBounds(void);
    UpdateSliceBounds();
    slicefile_labelindex = GetSliceBoundsIndexFromLabel(patchi->label.shortlabel);
    UpdateAllSliceColors(slicefile_labelindex, errorcode);
    list_slice_index = slicefile_labelindex;
    SetSliceBounds(slicefile_labelindex);

    scale = sb->scale;
    GetSliceColors(patchi->geom_vals, patchi->geom_nvals, patchi->geom_ivals,
      slicemin, slicemax,
      nrgb_full, nrgb,
      sb->colorlabels, &scale, &sb->fscale, sb->levels256,
      &slicei->extreme_min, &slicei->extreme_max
    );
  }

  if(patchi->boundary == 1){
    iboundarytype = GetBoundaryType(patchi);
  }
  else {
    slicefile_labelindex = GetSliceBoundsIndexFromLabel(patchi->label.shortlabel);
  }
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  if(patchi->boundary==1)UpdateBoundaryType();
  UpdateUnitDefs();
  UpdateTimes();
  force_redisplay=1;
  UpdateFrameNumber(1);
  updatemenu = 1;
  STOP_TIMER(total_time);
  PRINTF(" - %.1f MB/%.1f s\n", (float)return_filesize/1000000., total_time);
  return return_filesize;
}

/* ------------------ ReadBoundary ------------------------ */

FILE_SIZE ReadBoundary(int ifile, int load_flag, int *errorcode){
  patchdata *patchi;
  FILE_SIZE return_filesize = 0;

  patchi = patchinfo + ifile;
  if(patchi->structured == NO){
    ASSERT(ifile>=0&&ifile<ngeominfo);
    if(load_flag == LOAD){
      UpdateBoundaryHist(patchi);
    }
    return_filesize=ReadGeomData(patchi,NULL, load_flag,errorcode);
  }
  else{
    ASSERT(ifile>=0&&ifile<npatchinfo);
    return_filesize=ReadBoundaryBndf(ifile,load_flag,errorcode);
  }
  if(load_flag==LOAD){

  }
  return return_filesize;
}

/* ------------------ Local2GlobalBoundaryBounds ------------------------ */

void Local2GlobalBoundaryBounds(const char *key){
  int i;

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(strcmp(patchi->label.shortlabel,key)==0){
      patchi->valmin=patchmin;
      patchi->valmax=patchmax;
      patchi->setvalmin=setpatchmin;
      patchi->setvalmax=setpatchmax;

      patchi->chopmin=patchchopmin;
      patchi->chopmax=patchchopmax;
      patchi->setchopmin=setpatchchopmin;
      patchi->setchopmax=setpatchchopmax;
    }
  }
}

/* ------------------ Global2LocalBoundaryBounds ------------------------ */

void Global2LocalBoundaryBounds(const char *key){
  int i;

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(strcmp(patchi->label.shortlabel,key)==0){
      patchmin=patchi->valmin;
      patchmax=patchi->valmax;
      setpatchmin=patchi->setvalmin;
      setpatchmax=patchi->setvalmax;

      patchchopmin=patchi->chopmin;
      patchchopmax=patchi->chopmax;
      setpatchchopmin=patchi->setchopmin;
      setpatchchopmax=patchi->setchopmax;
      patchmin_unit = (unsigned char *)patchi->label.unit;
      patchmax_unit = patchmin_unit;
      UpdateGluiBoundaryUnits();
      UpdateHideBoundarySurface();

      Local2GlobalBoundaryBounds(key);
      return;
    }
  }
}

/* ------------------ DrawBoundaryTexture ------------------------ */

void DrawBoundaryTexture(const meshdata *meshi){
  float r11, r12, r21, r22;
  int n;
  int nrow, ncol, irow, icol;
  unsigned char *cpatchval_iframe_copy;
  float *xyzpatchcopy;
  int *patchblankcopy;
  float *patch_times;
  int *vis_boundaries;
  float *xyzpatch;
  int *patchdir, *boundary_row, *boundary_col, *boundarytype;
  int *blockstart;
  int *patchblank;
  unsigned char *cpatchval_iframe;
  int iblock;
  blockagedata *bc;
  patchdata *patchi;
  meshdata *meshblock;
  float dboundx,dboundy,dboundz;
  float *xplt, *yplt, *zplt;

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
  vis_boundaries=meshi->vis_boundaries;
  xyzpatch=meshi->xyzpatch;
  patchdir=meshi->patchdir;
  boundarytype=meshi->boundarytype;
  boundary_row=meshi->boundary_row;
  boundary_col=meshi->boundary_col;
  blockstart=meshi->blockstart;
  patchblank=meshi->patchblank;
  patchi=patchinfo+meshi->patchfilenum;
  switch(patchi->compression_type){
  case UNCOMPRESSED:
    ASSERT(meshi->cpatchval_iframe!=NULL);
    cpatchval_iframe=meshi->cpatchval_iframe;
    break;
  case COMPRESSED_ZLIB:
    ASSERT(meshi->cpatchval_iframe_zlib!=NULL);
    cpatchval_iframe=meshi->cpatchval_iframe_zlib;
    break;
  default:
    ASSERT(FFALSE);
  }
  patchi = patchinfo + meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  if((use_transparency_data==1&&contour_type==LINE_CONTOURS)||setpatchchopmin==1||setpatchchopmax==1)TransparentOn();
  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D,texture_patch_colorbar_id);

  glBegin(GL_TRIANGLES);
  for(n=0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        continue;
      }
    }
    drawit=0;
    if(vis_boundaries[n]==1&&patchdir[n]==0)drawit=1;
    if(boundarytype[n]!=INTERIORwall&&showpatch_both==1)drawit=1;
    if(drawit==1){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe + blockstart[n];

      for(irow=0;irow<nrow-1;irow++){
        unsigned char *cpatchval1, *cpatchval2;
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        cpatchval1 = cpatchval_iframe_copy + irow*ncol;
        cpatchval2 = cpatchval1 + ncol;
        patchblank2 = patchblank1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = (float)((unsigned char)(*cpatchval1))/255.0;
            r12 = (float)((unsigned char)(*(cpatchval1+1)))/255.0;
            r21 = (float)((unsigned char)(*cpatchval2))/255.0;
            r22 = (float)((unsigned char)(*(cpatchval2+1)))/255.0;
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
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
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
  for(n=0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock=meshi->meshonpatch[n];
    if(iblock!=-1){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        continue;
      }
    }
    drawit=0;
    if(meshi->vis_boundaries[n]==1&&meshi->patchdir[n]>0){
      if(boundarytype[n]==INTERIORwall||showpatch_both==0){
        drawit=1;
      }
    }
    if(drawit==1){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe + blockstart[n];
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(meshi->patchdir[n]){
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
            ASSERT(FFALSE);
            break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow=0;irow<nrow-1;irow++){
        unsigned char *cpatchval1, *cpatchval2;
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        cpatchval1 = cpatchval_iframe_copy + irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;

        xyzp2 = xyzp1 + 3*ncol;
        cpatchval2 = cpatchval1 + ncol;
        patchblank2 = patchblank1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = (float)((unsigned char)(*cpatchval1))/255.0;
            r12 = (float)((unsigned char)(*(cpatchval1+1)))/255.0;
            r21 = (float)((unsigned char)(*cpatchval2))/255.0;
            r22 = (float)((unsigned char)(*(cpatchval2+1)))/255.0;
            if(ABS(*cpatchval1-*(cpatchval2+1))<ABS(*(cpatchval1+1)-*cpatchval2)){
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
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
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
  for(n=0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        continue;
      }
    }
    drawit=0;
// xxx
    if(vis_boundaries[n]==1&&patchdir[n]<0){
      if(boundarytype[n]==INTERIORwall||showpatch_both==0){
        drawit=1;
      }
    }
    if(drawit==1){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe + blockstart[n];
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(meshi->patchdir[n]){
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
            ASSERT(FFALSE);
            break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow=0;irow<nrow-1;irow++){
        unsigned char *cpatchval1, *cpatchval2;
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        cpatchval1 = cpatchval_iframe_copy + irow*ncol;
        cpatchval2 = cpatchval1 + ncol;
        patchblank2 = patchblank1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = (float)((unsigned char)(*cpatchval1))/255.0;
            r12 = (float)((unsigned char)(*(cpatchval1+1)))/255.0;
            r21 = (float)((unsigned char)(*cpatchval2))/255.0;
            r22 = (float)((unsigned char)(*(cpatchval2+1)))/255.0;
            if(ABS(*cpatchval1-*(cpatchval2+1))<ABS(*(cpatchval1+1)-*cpatchval2)){
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
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
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
  if((use_transparency_data==1&&contour_type==LINE_CONTOURS)||setpatchchopmin==1||setpatchchopmax==1)TransparentOff();
}

/* ------------------ DrawBoundaryTextureThreshold ------------------------ */

void DrawBoundaryTextureThreshold(const meshdata *meshi){
  float r11, r12, r21, r22;
  int n,nn,nn1,nn2;
  int nrow, ncol, irow, icol;
  unsigned char *cpatchval1, *cpatchval2;
  unsigned char *cpatchval_iframe_copy;
  float *xyzpatchcopy;
  int *patchblankcopy;
  float *patch_times;
  int *vis_boundaries;
  float *xyzpatch;
  int *patchdir, *boundary_row, *boundary_col;
  int *blockstart;
  int *patchblank;
  unsigned char *cpatchval_iframe;
  int iblock;
  blockagedata *bc;
  patchdata *patchi;
  float *color11, *color12, *color21, *color22;
  meshdata *meshblock;
  float burn_color[4]={0.0,0.0,0.0,1.0};
  float clear_color[4]={1.0,1.0,1.0,1.0};

  if(vis_threshold==1&&vis_onlythreshold==1&&do_threshold==1)return;

  patch_times=meshi->patch_times;
  vis_boundaries=meshi->vis_boundaries;
  xyzpatch=meshi->xyzpatch;
  patchdir=meshi->patchdir;
  boundary_row=meshi->boundary_row;
  boundary_col=meshi->boundary_col;
  blockstart=meshi->blockstart;
  patchblank=meshi->patchblank;
  patchi=patchinfo+meshi->patchfilenum;
  switch(patchi->compression_type){
  case UNCOMPRESSED:
    ASSERT(meshi->cpatchval_iframe!=NULL);
    cpatchval_iframe=meshi->cpatchval_iframe;
    break;
  case COMPRESSED_ZLIB:
    ASSERT(meshi->cpatchval_iframe_zlib!=NULL);
    cpatchval_iframe=meshi->cpatchval_iframe_zlib;
    break;
  default:
    ASSERT(FFALSE);
  }
  patchi = patchinfo + meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glEnable(GL_TEXTURE_1D);
  glBindTexture(GL_TEXTURE_1D,texture_colorbar_id);

  nn =0;
  glBegin(GL_TRIANGLES);
  for(n=0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(vis_boundaries[n]==1&&patchdir[n]==0){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe + blockstart[n];

      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        cpatchval1 = cpatchval_iframe_copy + irow*ncol;
        cpatchval2 = cpatchval1 + ncol;
        patchblank2 = patchblank1 + ncol;
        nn2 = nn1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = (float)((unsigned char)(*cpatchval1))/255.0;
            r12 = (float)((unsigned char)(*(cpatchval1+1)))/255.0;
            r21 = (float)((unsigned char)(*cpatchval2))/255.0;
            r22 = (float)((unsigned char)(*(cpatchval2+1)))/255.0;
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
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += boundary_row[n]*boundary_col[n];
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  nn=0;
  glBegin(GL_TRIANGLES);
  for(n=0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock=meshi->meshonpatch[n];
    if(iblock!=-1){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(meshi->vis_boundaries[n]==1&&meshi->patchdir[n]>0){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe + blockstart[n];
      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        cpatchval1 = cpatchval_iframe_copy + irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;

        xyzp2 = xyzp1 + 3*ncol;
        cpatchval2 = cpatchval1 + ncol;
        patchblank2 = patchblank1 + ncol;
        nn2 = nn1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = (float)((unsigned char)(*cpatchval1))/255.0;
            r12 = (float)((unsigned char)(*(cpatchval1+1)))/255.0;
            r21 = (float)((unsigned char)(*cpatchval2))/255.0;
            r22 = (float)((unsigned char)(*(cpatchval2+1)))/255.0;
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
              if(ABS(*cpatchval1-*(cpatchval2+1))<ABS(*(cpatchval1+1)-*cpatchval2)){
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
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += boundary_row[n]*boundary_col[n];
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn=0;
  for(n=0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(vis_boundaries[n]==1&&patchdir[n]<0){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe + blockstart[n];
      for(irow=0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy + 3*irow*ncol;
        patchblank1 = patchblankcopy + irow*ncol;
        nn1 = nn + irow*ncol;
        xyzp2 = xyzp1 + 3*ncol;
        cpatchval1 = cpatchval_iframe_copy + irow*ncol;
        cpatchval2 = cpatchval1 + ncol;
        patchblank2 = patchblank1 + ncol;
        nn2 = nn1 + ncol;

        for(icol=0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            r11 = (float)((unsigned char)(*cpatchval1))/255.0;
            r12 = (float)((unsigned char)(*(cpatchval1+1)))/255.0;
            r21 = (float)((unsigned char)(*cpatchval2))/255.0;
            r22 = (float)((unsigned char)(*(cpatchval2+1)))/255.0;
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
              if(ABS(*cpatchval1-*(cpatchval2+1))<ABS(*(cpatchval1+1)-*cpatchval2)){
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
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
          xyzp1+=3;
          xyzp2+=3;
        }
      }
    }
    nn += boundary_row[n]*boundary_col[n];
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
  int *vis_boundaries;
  float *xyzpatch;
  int *patchdir, *boundary_row, *boundary_col;
  int *blockstart;
  int *patchblank;
  int iblock;
  blockagedata *bc;
  patchdata *patchi;
  float *color11;
  meshdata *meshblock;
  float burn_color[4]={0.0,0.0,0.0,1.0};
  float clear_color[4]={1.0,1.0,1.0,1.0};

  if(vis_threshold==1&&vis_onlythreshold==1&&do_threshold==1)return;

  patch_times=meshi->patch_times;
  vis_boundaries=meshi->vis_boundaries;
  xyzpatch=meshi->xyzpatch;
  patchdir=meshi->patchdir;
  boundary_row=meshi->boundary_row;
  boundary_col=meshi->boundary_col;
  blockstart=meshi->blockstart;
  patchblank=meshi->patchblank;
  patchi=patchinfo+meshi->patchfilenum;
  switch(patchi->compression_type){
  case UNCOMPRESSED:
    ASSERT(meshi->cpatchval_iframe!=NULL);
    break;
  case COMPRESSED_ZLIB:
    ASSERT(meshi->cpatchval_iframe_zlib!=NULL);
    break;
  default:
    ASSERT(FFALSE);
  }
  patchi = patchinfo + meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  nn =0;
  glBegin(GL_TRIANGLES);
  for(n=0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(vis_boundaries[n]==1&&patchdir[n]==0){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];

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
    nn += boundary_row[n]*boundary_col[n];
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  nn=0;
  glBegin(GL_TRIANGLES);
  for(n=0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock=meshi->meshonpatch[n];
    if(iblock!=-1){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(meshi->vis_boundaries[n]==1&&meshi->patchdir[n]>0){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];

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
    nn += boundary_row[n]*boundary_col[n];
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn=0;
  for(n=0;n<meshi->npatches;n++){
    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc=meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    if(vis_boundaries[n]==1&&patchdir[n]<0){
      nrow=boundary_row[n];
      ncol=boundary_col[n];
      xyzpatchcopy = xyzpatch + 3*blockstart[n];
      patchblankcopy = patchblank + blockstart[n];

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
    nn += boundary_row[n]*boundary_col[n];
  }
  glEnd();
}

/* ------------------ DrawBoundaryCellCenter ------------------------ */

void DrawBoundaryCellCenter(const meshdata *meshi){
  int n, nn, nn1;
  int nrow, ncol, irow, icol;
  unsigned char *cpatchval1;
  unsigned char *cpatchval_iframe_copy;
  float *patch_times;
  int *vis_boundaries;
  int *patchdir, *boundary_row, *boundary_col, *boundarytype;
  int *blockstart;
  unsigned char *cpatchval_iframe;
  int iblock;
  blockagedata *bc;
  patchdata *patchi;
  float *color11;
  meshdata *meshblock;

  float dboundx, dboundy, dboundz;
  float *xplt, *yplt, *zplt;
  float **patchventcolors;

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
  vis_boundaries = meshi->vis_boundaries;
  patchdir = meshi->patchdir;
  boundarytype = meshi->boundarytype;
  boundary_row = meshi->boundary_row;
  boundary_col = meshi->boundary_col;
  blockstart = meshi->blockstart;
  patchventcolors = meshi->patchventcolors;
  patchi = patchinfo+meshi->patchfilenum;

  switch(patchi->compression_type){
  case UNCOMPRESSED:
    cpatchval_iframe = meshi->cpatchval_iframe;
    if(cpatchval_iframe==NULL)return;
    break;
  case COMPRESSED_ZLIB:
    ASSERT(meshi->cpatchval_iframe_zlib!=NULL);
    cpatchval_iframe = meshi->cpatchval_iframe_zlib;
    break;
  default:
    ASSERT(FFALSE);
  }
  patchi = patchinfo+meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  if((use_transparency_data==1&&contour_type==LINE_CONTOURS)||setpatchchopmin==1||setpatchchopmax==1)TransparentOn();
  nn = 0;
  glBegin(GL_TRIANGLES);
  for(n = 0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    drawit = 0;
    if(vis_boundaries[n]==1&&patchdir[n]==0)drawit = 1;
    if(boundarytype[n]!=INTERIORwall&&showpatch_both==1)drawit = 1;
    if(drawit==1){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      cpatchval_iframe_copy = cpatchval_iframe+blockstart[n];
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = meshi->xyzpatch+3*blockstart[n]+3*irow*ncol;
        patchblank1 = meshi->patchblank+blockstart[n]+irow*ncol;
        nn1 = nn+irow*ncol;
        xyzp2 = xyzp1+3*ncol;
        cpatchval1 = cpatchval_iframe_copy+irow*ncol;
        patchblank2 = patchblank1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            if(patchventcolors==NULL){
              color11 = rgb_patch+4*(*cpatchval1);
              if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
                if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              }
            }
            else{
              color11 = patchventcolors[(cpatchval1-cpatchval_iframe)];
            }
            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp1+3);
            glVertex3fv(xyzp2+3);

            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp2);
          }
          cpatchval1++;
          patchblank1++;
          patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
    }
    nn += boundary_row[n]*boundary_col[n];
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  nn = 0;
  if(hidepatchsurface==1){
    glBegin(GL_TRIANGLES);
  }
  for(n = 0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    if(iblock!=-1){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    drawit = 0;
    if(meshi->vis_boundaries[n]==1&&meshi->patchdir[n]>0){
      if(boundarytype[n]==INTERIORwall||showpatch_both==0){
        drawit = 1;
      }
    }
    if(drawit==1){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      cpatchval_iframe_copy = cpatchval_iframe+blockstart[n];
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(meshi->patchdir[n]){
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
          ASSERT(FFALSE);
          break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = meshi->xyzpatch+3*blockstart[n]+3*irow*ncol;
        cpatchval1 = cpatchval_iframe_copy+irow*ncol;
        patchblank1 = meshi->patchblank+blockstart[n]+irow*ncol;
        nn1 = nn+irow*ncol;

        xyzp2 = xyzp1+3*ncol;
        patchblank2 = patchblank1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            if(patchventcolors==NULL){
              color11 = rgb_patch+4*(*cpatchval1);
              if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
                if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              }
            }
            else{
              color11 = patchventcolors[(cpatchval1-cpatchval_iframe)];
            }
            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp1+3);
            glVertex3fv(xyzp2+3);

            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp2);
          }
          cpatchval1++;
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
    nn += boundary_row[n]*boundary_col[n];
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn = 0;
  for(n = 0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    drawit = 0;
    if(vis_boundaries[n]==1&&patchdir[n]<0){
      if(boundarytype[n]==INTERIORwall||showpatch_both==0){
        drawit = 1;
      }
    }
    if(drawit==1){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      cpatchval_iframe_copy = cpatchval_iframe+blockstart[n];
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(meshi->patchdir[n]){
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
          ASSERT(FFALSE);
          break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = meshi->xyzpatch+3*blockstart[n]+3*irow*ncol;
        patchblank1 = meshi->patchblank+blockstart[n]+irow*ncol;
        nn1 = nn+irow*ncol;
        xyzp2 = xyzp1+3*ncol;
        cpatchval1 = cpatchval_iframe_copy+irow*ncol;
        patchblank2 = patchblank1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            if(patchventcolors==NULL){
              color11 = rgb_patch+4*(*cpatchval1);
              if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
                if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              }
            }
            else{
              color11 = patchventcolors[(cpatchval1-cpatchval_iframe)];
            }
            glColor4fv(color11);
            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2+3);
            glVertex3fv(xyzp1+3);

            glVertex3fv(xyzp1);
            glVertex3fv(xyzp2);
            glVertex3fv(xyzp2+3);
          }
          cpatchval1++;
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
    nn += boundary_row[n]*boundary_col[n];
  }
  if(hidepatchsurface==1){
    glEnd();
  }
  if((use_transparency_data==1&&contour_type==LINE_CONTOURS)||setpatchchopmin==1||setpatchchopmax==1)TransparentOff();
}

/* ------------------ DrawBoundary ------------------------ */

void DrawBoundary(const meshdata *meshi){
  int n, nn, nn1, nn2;
  int nrow, ncol, irow, icol;
  unsigned char *cpatchval1, *cpatchval2;
  unsigned char *cpatchval_iframe_copy;
  float *xyzpatchcopy;
  int *patchblankcopy;
  float *patch_times;
  int *vis_boundaries;
  float *xyzpatch;
  int *patchdir, *boundary_row, *boundary_col, *boundarytype;
  int *blockstart;
  int *patchblank;
  unsigned char *cpatchval_iframe;
  int iblock;
  blockagedata *bc;
  patchdata *patchi;
  float *color11, *color12, *color21, *color22;
  meshdata *meshblock;
  float dboundx, dboundy, dboundz;
  float *xplt, *yplt, *zplt;

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
  vis_boundaries = meshi->vis_boundaries;
  xyzpatch = meshi->xyzpatch;
  patchdir = meshi->patchdir;
  boundarytype = meshi->boundarytype;
  boundary_row = meshi->boundary_row;
  boundary_col = meshi->boundary_col;
  blockstart = meshi->blockstart;
  patchblank = meshi->patchblank;
  patchi = patchinfo+meshi->patchfilenum;
  switch(patchi->compression_type){
  case UNCOMPRESSED:
    ASSERT(meshi->cpatchval_iframe!=NULL);
    cpatchval_iframe = meshi->cpatchval_iframe;
    break;
  case COMPRESSED_ZLIB:
    ASSERT(meshi->cpatchval_iframe_zlib!=NULL);
    cpatchval_iframe = meshi->cpatchval_iframe_zlib;
    break;
  default:
    ASSERT(FFALSE);
  }
  patchi = patchinfo+meshi->patchfilenum;

  if(patch_times[0]>global_times[itimes]||patchi->display==0)return;
  if(cullfaces==1)glDisable(GL_CULL_FACE);

  /* if a contour boundary does not match a blockage face then draw "both sides" of boundary */

  if((use_transparency_data==1&&contour_type==LINE_CONTOURS)||setpatchchopmin==1||setpatchchopmax==1)TransparentOn();
  nn = 0;
  glBegin(GL_TRIANGLES);
  for(n = 0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    drawit = 0;
    if(vis_boundaries[n]==1&&patchdir[n]==0)drawit = 1;
    if(boundarytype[n]!=INTERIORwall&&showpatch_both==1)drawit = 1;
    if(drawit==1){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      xyzpatchcopy = xyzpatch+3*blockstart[n];
      patchblankcopy = patchblank+blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe+blockstart[n];
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy+3*irow*ncol;
        patchblank1 = patchblankcopy+irow*ncol;
        nn1 = nn+irow*ncol;
        xyzp2 = xyzp1+3*ncol;
        cpatchval1 = cpatchval_iframe_copy+irow*ncol;
        cpatchval2 = cpatchval1+ncol;
        patchblank2 = patchblank1+ncol;
        nn2 = nn1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            color11 = rgb_patch+4*(*cpatchval1);
            color12 = rgb_patch+4*(*(cpatchval1+1));
            color21 = rgb_patch+4*(*cpatchval2);
            color22 = rgb_patch+4*(*(cpatchval2+1));
            if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
              if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1])color12 = &char_color[0];
              if(meshi->thresholdtime[nn2+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol])color21 = &char_color[0];
              if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1])color22 = &char_color[0];
            }
            if(ABS(*cpatchval1-*(cpatchval2+1))<ABS(*(cpatchval1+1)-*cpatchval2)){
              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);

              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);

              glColor4fv(color21);
              glVertex3fv(xyzp2);
            }
            else{
              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color21);
              glVertex3fv(xyzp2);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);

              glColor4fv(color21);
              glVertex3fv(xyzp2);
            }
          }
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
    }
    nn += boundary_row[n]*boundary_col[n];
  }
  glEnd();
  if(cullfaces==1)glEnable(GL_CULL_FACE);

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */

  nn = 0;
  if(hidepatchsurface==1){
    glBegin(GL_TRIANGLES);
  }
  for(n = 0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    if(iblock!=-1){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    drawit = 0;
    if(meshi->vis_boundaries[n]==1&&meshi->patchdir[n]>0){
      if(boundarytype[n]==INTERIORwall||showpatch_both==0){
        drawit = 1;
      }
    }
    if(drawit==1){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      xyzpatchcopy = xyzpatch+3*blockstart[n];
      patchblankcopy = patchblank+blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe+blockstart[n];
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(meshi->patchdir[n]){
        case XDIR:
          glTranslatef(dboundx, 0.0, 0.0);
          break;
        case YDIR:
          glTranslatef(0.0, -dboundy, 0.0);
          break;
        case ZDIR:
          glTranslatef(0.00, 0.0, dboundz);
          break;
        default:
          ASSERT(FFALSE);
          break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy+3*irow*ncol;
        cpatchval1 = cpatchval_iframe_copy+irow*ncol;
        patchblank1 = patchblankcopy+irow*ncol;
        nn1 = nn+irow*ncol;

        xyzp2 = xyzp1+3*ncol;
        cpatchval2 = cpatchval1+ncol;
        patchblank2 = patchblank1+ncol;
        nn2 = nn1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            color11 = rgb_patch+4*(*cpatchval1);
            color12 = rgb_patch+4*(*(cpatchval1+1));
            color21 = rgb_patch+4*(*cpatchval2);
            color22 = rgb_patch+4*(*(cpatchval2+1));
            if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
              if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1])color12 = &char_color[0];
              if(meshi->thresholdtime[nn2+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol])color21 = &char_color[0];
              if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1])color22 = &char_color[0];
            }
            if(ABS(*cpatchval1-*(cpatchval2+1))<ABS(*(cpatchval1+1)-*cpatchval2)){
              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);

              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);

              glColor4fv(color21);
              glVertex3fv(xyzp2);
            }
            else{
              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color21);
              glVertex3fv(xyzp2);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);

              glColor4fv(color21);
              glVertex3fv(xyzp2);
            }
          }
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
      if(hidepatchsurface==0){
        glEnd();
        glPopMatrix();
      }
    }
    nn += boundary_row[n]*boundary_col[n];
  }

  /* if a contour boundary DOES match a blockage face then draw "one sides" of boundary */
  nn = 0;
  for(n = 0;n<meshi->npatches;n++){
    int drawit;

    iblock = meshi->blockonpatch[n];
    meshblock = meshi->meshonpatch[n];
    ASSERT((iblock!=-1&&meshblock!=NULL)||(iblock==-1&&meshblock==NULL));
    if(iblock!=-1&&meshblock!=NULL){
      bc = meshblock->blockageinfoptrs[iblock];
      if(bc->showtimelist!=NULL&&bc->showtimelist[itimes]==0){
        nn += boundary_row[n]*boundary_col[n];
        continue;
      }
    }
    drawit = 0;
    if(vis_boundaries[n]==1&&patchdir[n]<0){
      if(boundarytype[n]==INTERIORwall||showpatch_both==0){
        drawit = 1;
      }
    }
    if(drawit==1){
      nrow = boundary_row[n];
      ncol = boundary_col[n];
      xyzpatchcopy = xyzpatch+3*blockstart[n];
      patchblankcopy = patchblank+blockstart[n];
      cpatchval_iframe_copy = cpatchval_iframe+blockstart[n];
      if(hidepatchsurface==0){
        glPushMatrix();
        switch(meshi->patchdir[n]){
        case XDIRNEG:
          glTranslatef(-dboundx, 0.0, 0.0);
          break;
        case YDIRNEG:
          glTranslatef(0.0, dboundy, 0.0);
          break;
        case ZDIRNEG:
          glTranslatef(0.00, 0.0, -dboundz);
          break;
        default:
          ASSERT(FFALSE);
          break;
        }
        glBegin(GL_TRIANGLES);
      }
      for(irow = 0;irow<nrow-1;irow++){
        int *patchblank1, *patchblank2;
        float *xyzp1, *xyzp2;

        xyzp1 = xyzpatchcopy+3*irow*ncol;
        patchblank1 = patchblankcopy+irow*ncol;
        nn1 = nn+irow*ncol;
        xyzp2 = xyzp1+3*ncol;
        cpatchval1 = cpatchval_iframe_copy+irow*ncol;
        cpatchval2 = cpatchval1+ncol;
        patchblank2 = patchblank1+ncol;
        nn2 = nn1+ncol;

        for(icol = 0;icol<ncol-1;icol++){
          if(*patchblank1==GAS&&*patchblank2==GAS&&*(patchblank1+1)==GAS&&*(patchblank2+1)==GAS){
            color11 = rgb_patch+4*(*cpatchval1);
            color12 = rgb_patch+4*(*(cpatchval1+1));
            color21 = rgb_patch+4*(*cpatchval2);
            color22 = rgb_patch+4*(*(cpatchval2+1));
            if(vis_threshold==1&&vis_onlythreshold==0&&do_threshold==1){
              if(meshi->thresholdtime[nn1+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol])color11 = &char_color[0];
              if(meshi->thresholdtime[nn1+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn1+icol+1])color12 = &char_color[0];
              if(meshi->thresholdtime[nn2+icol]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol])color21 = &char_color[0];
              if(meshi->thresholdtime[nn2+icol+1]>=0.0&&global_times[itimes]>meshi->thresholdtime[nn2+icol+1])color22 = &char_color[0];
            }
            if(ABS(*cpatchval1-*(cpatchval2+1))<ABS(*(cpatchval1+1)-*cpatchval2)){
              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color21);
              glVertex3fv(xyzp2);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);
            }
            else{
              glColor4fv(color11);
              glVertex3fv(xyzp1);

              glColor4fv(color21);
              glVertex3fv(xyzp2);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color12);
              glVertex3fv(xyzp1+3);

              glColor4fv(color21);
              glVertex3fv(xyzp2);

              glColor4fv(color22);
              glVertex3fv(xyzp2+3);
            }
          }
          cpatchval1++; cpatchval2++; patchblank1++; patchblank2++;
          xyzp1 += 3;
          xyzp2 += 3;
        }
      }
      if(hidepatchsurface==0){
        glEnd();
        glPopMatrix();
      }
    }
    nn += boundary_row[n]*boundary_col[n];
  }
  if(hidepatchsurface==1){
    glEnd();
  }
  if((use_transparency_data==1&&contour_type==LINE_CONTOURS)||setpatchchopmin==1||setpatchchopmax==1)TransparentOff();
}

/* ------------------ DrawBoundaryFrame ------------------------ */

void DrawBoundaryFrame(int flag){
  meshdata *meshi;
  int i;

  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    if(patchi->structured == NO && patchi->loaded == 1 && patchi->display == 1){
      if(flag == DRAW_OPAQUE){
        if(patchi->patch_filetype == PATCH_GEOMETRY_BOUNDARY){
          DrawGeomData(flag, patchi, GEOM_STATIC);
          DrawGeomData(flag, patchi, GEOM_DYNAMIC);
        }
      }
      else{
        if(patchi->patch_filetype == PATCH_GEOMETRY_SLICE){
          DrawGeomData(flag, patchi, GEOM_STATIC);
          DrawGeomData(flag, patchi, GEOM_DYNAMIC);
        }
      }
    }
  }
  if(flag == DRAW_TRANSPARENT)return;
  for(i=0;i<nmeshes;i++){
    meshi=meshinfo+i;
    if(meshi->npatches>0){
      int filenum;

      filenum=meshi->patchfilenum;
      if(filenum!=-1){
        patchdata *patchi;

        patchi = patchinfo + filenum;
        if(patchi->loaded==0||patchi->display==0||patchi->shortlabel_index!=iboundarytype)continue;
        if(usetexturebar!=0){
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
        }
        else{
          if(patchi->patch_filetype==PATCH_STRUCTURED_CELL_CENTER){
            DrawBoundaryCellCenter(meshi);
          }
          else if(patchi->patch_filetype==PATCH_STRUCTURED_NODE_CENTER){
            DrawBoundary(meshi);
          }
        }
        if(vis_threshold==1&&vis_onlythreshold==1&&do_threshold==1)DrawOnlyThreshold(meshi);
      }
    }
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
  return;

}

/* ------------------ BoundaryCompare ------------------------ */

int BoundaryCompare( const void *arg1, const void *arg2 ){
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
      else{
        STRCPY(patchi->menulabel_suffix, patchi->label.longlabel);
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

  dxyzi = meshi->dxyz;
  dxyzj = meshj->dxyz;
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

    grid_eps = MAX(meshi->dxyz[patchi->dir],meshj->dxyz[patchi->dir]);

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

/* ------------------ UncompressBoundaryDataFrame ------------------------ */

void UncompressBoundaryDataFrame(meshdata *meshi,int local_iframe){
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


/* ------------------ UpdateAllBoundaryBoundsST ------------------------ */

void UpdateAllBoundaryBoundsST(void){
  int i;
  int total=0;

  LOCK_COMPRESS;
  for(i=0;i<npatchinfo;i++){
    patchdata *patchi;

    patchi = patchinfo + i;
    total+= UpdateBoundaryHist(patchi);
    UpdateBoundaryBounds(patchi);
  }
  if(total==0){
    PRINTF("Boundary file bounds already computed.\n");
  }
  else{
    PRINTF("Bounds for %i boundary files computed\n",total);
  }
  UNLOCK_COMPRESS;
}

/* ------------------ UpdateBoundaryHist ------------------------ */

int UpdateBoundaryHist(patchdata *patchj){
  int i;
  int first=1;
  int sum=0;

  if(patchj->setvalmax==SET_MAX&&patchj->setvalmin==SET_MIN)return 0;

  for(i=0;i<npatchinfo;i++){
    int npatches, error;
    patchdata *patchi;
    int unit1;
    FILE_SIZE lenfile;
    int error1;
    int *pi1, *pi2, *pj1, *pj2, *pk1, *pk2, *patchdir, *patchsize;
    float patchtime1, *patchframe;
    int patchframesize;
    int j;
    time_t modtime;

    patchi = patchinfo + i;
    if(patchi->shortlabel_index != patchj->shortlabel_index)continue;
    if(patchi->patch_filetype != patchj->patch_filetype)continue;
    if(patchi->structured != patchj->structured)continue;

    modtime= FileModtime(patchi->file);
    if(modtime>patchi->modtime){
      patchi->modtime=modtime;
      patchi->inuse_getbounds=0;
      patchi->histogram->complete=0;
      patchi->bounds.defined=0;
    }
    if(patchi->inuse_getbounds==1||patchi->histogram->complete==1||patchi->bounds.defined==1)continue;

    patchi->inuse_getbounds=1;

    if(first==1){
      first=0;
    }
    sum++;
    lenfile=strlen(patchi->file);

    if (patchj->structured == YES) {
      FORTgetboundaryheader1(patchi->file, &unit1, &npatches, &error, lenfile);
      if (npatches == 0) {
        FORTclosefortranfile(&unit1);
        continue;
      }

      NewMemory((void **)&pi1, npatches * sizeof(int));
      NewMemory((void **)&pi2, npatches * sizeof(int));
      NewMemory((void **)&pj1, npatches * sizeof(int));
      NewMemory((void **)&pj2, npatches * sizeof(int));
      NewMemory((void **)&pk1, npatches * sizeof(int));
      NewMemory((void **)&pk2, npatches * sizeof(int));
      NewMemory((void **)&patchdir, npatches * sizeof(int));
      NewMemory((void **)&patchsize, npatches * sizeof(int));

      FORTgetboundaryheader2(&unit1, &patchi->version, &npatches, pi1, pi2, pj1, pj2, pk1, pk2, patchdir);

      patchframesize = 0;
      for (j = 0; j < npatches; j++) {
        int npatchsize;

        npatchsize = (pi2[j] + 1 - pi1[j]);
        npatchsize *= (pj2[j] + 1 - pj1[j]);
        npatchsize *= (pk2[j] + 1 - pk1[j]);
        patchframesize += npatchsize;
      }

      NewMemory((void **)&patchframe, patchframesize * sizeof(float));
      ResetHistogram(patchi->histogram, NULL, NULL);
      error1 = 0;
      while (error1 == 0) {
        int ndummy, filesize;

        FORTgetpatchdata(&unit1, &npatches,
          pi1, pi2, pj1, pj2, pk1, pk2, &patchtime1, patchframe, &ndummy, &filesize, &error1);
        UpdateHistogram(patchframe, NULL, patchframesize, patchi->histogram);
      }
      FORTclosefortranfile(&unit1);
      FREEMEMORY(patchframe);
      FREEMEMORY(pi1);
      FREEMEMORY(pi2);
      FREEMEMORY(pj1);
      FREEMEMORY(pj2);
      FREEMEMORY(pk1);
      FREEMEMORY(pk2);
      FREEMEMORY(patchdir);
      FREEMEMORY(patchsize);
      CompleteHistogram(patchi->histogram);
    }
    else{
      int error_code;

      ReadGeomData(patchi, NULL, UPDATE_HIST, &error_code);
      ReadGeomData(patchi, NULL, UNLOAD, &error_code);
    }
  }
  return sum;
}


