#include "options.h"
#ifdef pp_ADF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MALLOCC.h"
#include "gd.h"
#include "dem_util.h"
#include "IOadf.h"
#include "datadefs.h"

/* ------------------ GetAndersonFireIndex ------------------------ */

#define NFIRE_TYPES 20
int GetAndersonFireIndex(int val){
  int j;
  int fire_type[NFIRE_TYPES] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 90, 91, 92, 93, 98, 99,9999};

  for(j = 0; j<NFIRE_TYPES-1; j++){
    if(val==fire_type[j])return j+1;
  }
  return NFIRE_TYPES;
}

/* ------------------ GetValIndex2 ------------------------ */

int GetValIndex2(int val, int *val_table, int ntable){
  int i;

  for(i = 0; i<ntable; i++){
    if(val==val_table[i])return i;
  }
  return -1;
}

/* ------------------ ADF2PNG ------------------------ */

void ADF2PNG(char *basename, int *vals, int nrows, int ncols){
  int i;
  gdImagePtr RENDERimage;
  FILE *RENDERfile = NULL;

  RENDERimage = gdImageCreateTrueColor(3*ncols, 3*nrows);

  for(i = 0; i<ncols; i++){
    int j;

    for(j = 0; j<nrows; j++){
      unsigned char r, g, b;
      int rgb_local, color_index, ii, jj;
      int val;

      val = vals[j*ncols+i];
      if(val==98){
        val = 98;
      }
      color_index = GetAndersonFireIndex(val)-1;
      r = (unsigned char)firecolors[3*color_index];
      g = (unsigned char)firecolors[3*color_index+1];
      b = (unsigned char)firecolors[3*color_index+2];
      rgb_local = (r<<16)|(g<<8)|b;
      for(ii=0;ii<3;ii++){
        for(jj=0;jj<3;jj++){
          gdImageSetPixel(RENDERimage, 3*i+ii, 3*j+jj, rgb_local);
        }
      }
    }
  }
  {
    char filename[1000];

    strcpy(filename,basename);
    strcat(filename,".png");
    RENDERfile = fopen(filename, "wb");
  }
  gdImagePng(RENDERimage, RENDERfile);
  fclose(RENDERfile);
}

/* ------------------ ADF_GetSurfs ------------------------ */

void ADF_GetSurfs(wuigriddata *wuifireinfo, struct _elevdata *fds_elevs, float *verts, int nverts, int *faces, int *surfs, int nfaces){
  int i, fire_index;
  float longitude, latitude;

  for(i = 0; i<nfaces; i++){
    int f1, f2, f3;
    float *v1, *v2, *v3, xavg, yavg;
    int firetype_index;

    f1 = faces[3*i+0]-1;
    f2 = faces[3*i+1]-1;
    f3 = faces[3*i+2]-1;
    v1 = verts+3*f1;
    v2 = verts+3*f2;
    v3 = verts+3*f3;
    xavg = (v1[0]+v2[0]+v3[0])/3.0;
    yavg = (v1[1]+v2[1]+v3[1])/3.0;
    longitude = fds_elevs->long_min + (xavg/fds_elevs->xmax)*(fds_elevs->long_max-fds_elevs->long_min);
    latitude = fds_elevs->lat_min   + (yavg/fds_elevs->ymax)*(fds_elevs->lat_max-fds_elevs->lat_min);
    fire_index = ADF_GetFireIndex(wuifireinfo, longitude, latitude);
    firetype_index = GetAndersonFireIndex(fire_index);
    surfs[i] = firetype_index;
  }
}

/* ------------------ ADF_GetFireIndex ------------------------ */

int ADF_GetFireIndex(wuigriddata *wuifireinfo, float longitude, float latitude){
  int ix, iy, index, val;

  ix = CLAMP(wuifireinfo->ncols*(longitude-wuifireinfo->long_min)/(wuifireinfo->long_max-wuifireinfo->long_min),0,wuifireinfo->ncols-1);
  iy = CLAMP(wuifireinfo->nrows*(wuifireinfo->lat_max - latitude)/(wuifireinfo->lat_max-wuifireinfo->lat_min), 0, wuifireinfo->nrows-1);
  index = iy*wuifireinfo->ncols+ix;
  val = wuifireinfo->vals[index];
  return val;
}

  /* ------------------ ADF_ReadIGrid ------------------------ */

int ADF_ReadIGrid(char *adf_dir, char *file, wuigriddata *wuifireinfo){
  FILE *stream;
  int i;
  int *vals_local = NULL, nvals_local;
  char *buffer = NULL;
  int size_buffer;

  wuifireinfo->vals_ntypes = 0;

  stream = fopen_indir(adf_dir, file, "r");
  if(stream==NULL)return 1;

  size_buffer = 100;
  NewMemory((void **)&buffer, size_buffer*sizeof(char));

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%i", &wuifireinfo->ncols);

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%i", &wuifireinfo->nrows);

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%f", &wuifireinfo->long_min);

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%f", &wuifireinfo->lat_min);

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%f", &wuifireinfo->dlong);

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%f", &wuifireinfo->dlat);

  wuifireinfo->long_max = wuifireinfo->long_min + (float)wuifireinfo->ncols*wuifireinfo->dlong;
  wuifireinfo->lat_max  = wuifireinfo->lat_min  + (float)wuifireinfo->nrows*wuifireinfo->dlat;

  fgets(buffer, size_buffer, stream);

  nvals_local = wuifireinfo->ncols*wuifireinfo->nrows;
  if(nvals_local<=0)return 1;

  FREEMEMORY(buffer);
  size_buffer = 5*wuifireinfo->ncols;
  NewMemory((void **)&buffer, size_buffer*sizeof(char));

  NewMemory((void **)&vals_local, nvals_local*sizeof(int));
  wuifireinfo->vals = vals_local;
  for(i = 0; i<wuifireinfo->nrows; i++){
    int j;
    char *tok;

    if(fgets(buffer, size_buffer, stream)==NULL)break;
    tok = strtok(buffer, " ");
    for(j = 0; j<wuifireinfo->ncols; j++){
      sscanf(tok, "%i", vals_local);
      tok = strtok(NULL, " ");
      vals_local++;
    }
  }
  return 0;
}

/* ------------------ ADF_ReadFGrid ------------------------ */

int ADF_ReadFGrid(char *adf_dir, char *file, wuigriddata *wuigridinfo){
  FILE *stream;
  int i, nvals_local, ncols, nrows, size_buffer;
  float *vals_local = NULL;
  char *buffer = NULL;

  stream = fopen_indir(adf_dir, file, "r");
  if(stream==NULL)return 1;

  size_buffer = 100;
  NewMemory((void **)&buffer, size_buffer*sizeof(char));

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%i", &ncols);
  wuigridinfo->ncols = ncols;

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%i", &nrows);
  wuigridinfo->nrows = nrows;

  nvals_local = wuigridinfo->ncols*wuigridinfo->nrows;
  if(nvals_local<=0)return 1;

  fgets(buffer, size_buffer, stream);
  fgets(buffer, size_buffer, stream);
  fgets(buffer, size_buffer, stream);
  fgets(buffer, size_buffer, stream);

  FREEMEMORY(buffer);
  size_buffer = 20*ncols;
  NewMemory((void **)&buffer, size_buffer*sizeof(char));

  NewMemory((void **)&vals_local, nvals_local*sizeof(float));
  wuigridinfo->fvals = vals_local;
  for(i = 0; i<nrows; i++){
    int j;
    char *tok;

    if(fgets(buffer, size_buffer, stream)==NULL)break;
    tok = strtok(buffer, " ");
    for(j = 0; j<ncols; j++){
      sscanf(tok, "%f", vals_local);
      tok = strtok(NULL, " ");
      vals_local++;
    }
  }
  FREEMEMORY(buffer);
  return 0;
}

/* ------------------ ADF_GetFireData ------------------------ */

wuigriddata *ADF_GetFireData(char *adf_dir, char *casename){
  wuigriddata *wuifireinfo;
  int ntypes;

  NewMemory((void **)&wuifireinfo, sizeof(wuigriddata));

  if(ADF_ReadIGrid(adf_dir, "w001001.asc", wuifireinfo)!=0){
    FREEMEMORY(wuifireinfo);
    return NULL;
  }
  ADF2PNG(casename, wuifireinfo->vals, wuifireinfo->nrows, wuifireinfo->ncols);
  return wuifireinfo;
}


#endif
