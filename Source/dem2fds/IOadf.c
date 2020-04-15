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

void ADF_GetSurfs(wuifiredata *wuifireinfo, struct _elevdata *fds_elevs, float *verts, int nverts, int *faces, int *surfs, int nfaces){
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

/* ------------------ ADF_GetInfo ------------------------ */

#define GET_BUFFER(flag) \
  if(flag==1){if(fgets(buffer, 1000, stream)==NULL){ fclose(stream); return 1; }} \
  colon = strchr(buffer, ':'); \
  if(colon==NULL){ fclose(stream); return 1; } \
  colon++;

int ADF_GetInfo(char *adf_dir, wuifiredata *wuifireinfo){
  char parent_dir[1000];
  FILE *stream;
  char buffer[1000];
  char key1[] = "Number of columns:";
  char key2[] = "Top edge Native:";
  char key3[] = "Top edge WGS84:";
  char *colon;
  int n_columns, n_rows;
  float res_x, res_y;
  int top_edge, bottom_edge, left_edge, right_edge;
  float lat_max, lat_min, long_min, long_max;

  if(adf_dir==NULL||strlen(adf_dir)==0)return 1;
  strcpy(parent_dir, adf_dir);
  strcat(parent_dir, dirseparator);
  strcat(parent_dir, "..");

  stream = fopen_indir(parent_dir, "output_parameters.txt", "r");

  //skip until key1 is located
  if(stream==NULL)return 1;
  for(;;){
    if(fgets(buffer, 1000, stream)==NULL){ fclose(stream); return 1; }
    if(strncmp(buffer, key1, strlen(key1))==0)break;
  }

  GET_BUFFER(0);
  sscanf(colon, "%i", &n_columns);

  GET_BUFFER(1);
  sscanf(colon, "%i", &n_rows);

  GET_BUFFER(1);
  sscanf(colon, "%f", &res_x);

  GET_BUFFER(1);
  sscanf(colon, "%f", &res_y);

  //skip until key2 is located
  if(stream==NULL)return 1;
  for(;;){
    if(fgets(buffer, 1000, stream)==NULL){ fclose(stream); return 1; }
    if(strncmp(buffer, key2, strlen(key2))==0)break;
  }

  GET_BUFFER(0);
  sscanf(colon, "%i", &top_edge);

  GET_BUFFER(1);
  sscanf(colon, "%i", &bottom_edge);

  GET_BUFFER(1);
  sscanf(colon, "%i", &left_edge);

  GET_BUFFER(1);
  sscanf(colon, "%i", &right_edge);

  //skip until key3 is located
  if(stream==NULL)return 1;
  for(;;){
    if(fgets(buffer, 1000, stream)==NULL){ fclose(stream); return 1; }
    if(strncmp(buffer, key3, strlen(key3))==0)break;
  }

  GET_BUFFER(0);
  sscanf(colon, "%f", &lat_max);

  GET_BUFFER(1);
  sscanf(colon, "%f", &lat_min);

  GET_BUFFER(1);
  sscanf(colon, "%f", &long_min);

  GET_BUFFER(1);
  sscanf(colon, "%f", &long_max);

  wuifireinfo->res_x = res_x;
  wuifireinfo->res_y = res_y;
  wuifireinfo->ncols = n_columns;
  wuifireinfo->nrows = n_rows;

  wuifireinfo->left_edge = left_edge;
  wuifireinfo->right_edge = right_edge;
  wuifireinfo->bottom_edge = bottom_edge;
  wuifireinfo->top_edge = top_edge;

  wuifireinfo->lat_min = lat_min;
  wuifireinfo->lat_max = lat_max;
  wuifireinfo->long_min = long_min;
  wuifireinfo->long_max = long_max;

  fclose(stream);
  return 0;
}

/* ------------------ ADF_GetFireIndex ------------------------ */

int ADF_GetFireIndex(wuifiredata *wuifireinfo, float longitude, float latitude){
  int ix, iy, index, val;

  ix = CLAMP(wuifireinfo->ncols*(longitude-wuifireinfo->long_min)/(wuifireinfo->long_max-wuifireinfo->long_min),0,wuifireinfo->ncols-1);
  iy = CLAMP(wuifireinfo->nrows*(wuifireinfo->lat_max - latitude)/(wuifireinfo->lat_max-wuifireinfo->lat_min), 0, wuifireinfo->nrows-1);
  index = iy*wuifireinfo->ncols+ix;
  val = wuifireinfo->vals[index];
  return val;
}

  /* ------------------ ADF_Read_w001001 ------------------------ */

int ADF_Read_w001001(char *adf_dir, wuifiredata *wuifireinfo){
  FILE *stream;
  int i;
  int *vals_local = NULL, nvals_local;
  char *buffer = NULL;
  int ncols, nrows;
  int size_buffer;

  wuifireinfo->vals_ntypes = 0;

  if(ADF_GetInfo(adf_dir, wuifireinfo)!=0)return 1;
  nvals_local = wuifireinfo->ncols*wuifireinfo->nrows;
  if(nvals_local<=0)return 1;

  stream = fopen_indir(adf_dir, "w001001.asc", "r");
  if(stream==NULL)return 1;

  size_buffer = 100;
  NewMemory((void **)&buffer, size_buffer*sizeof(char));
  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%i", &ncols);

  fgets(buffer, size_buffer, stream);
  sscanf(buffer+13, "%i", &nrows);

  if(ncols!=wuifireinfo->ncols||nrows!=wuifireinfo->nrows){
    fclose(stream);
    return 1;
  }

  fgets(buffer, size_buffer, stream);
  fgets(buffer, size_buffer, stream);
  fgets(buffer, size_buffer, stream);
  fgets(buffer, size_buffer, stream);

  FREEMEMORY(buffer);
  size_buffer = 5*ncols;
  NewMemory((void **)&buffer, size_buffer*sizeof(char));

  NewMemory((void **)&vals_local, nvals_local*sizeof(wuifiredata));
  wuifireinfo->vals = vals_local;
  for(i = 0; i<nrows; i++){
    int j;
    char *tok;

    if(fgets(buffer, size_buffer, stream)==NULL)break;
    tok = strtok(buffer, " ");
    for(j = 0; j<ncols; j++){
      sscanf(tok, "%i", vals_local);
      tok = strtok(NULL, " ");
      vals_local++;
    }
  }
  return 0;
}

/* ------------------ ADF_GetFireData ------------------------ */

wuifiredata *ADF_GetFireData(char *adf_dir, char *casename){
  wuifiredata *wuifireinfo;
  int ntypes;

  NewMemory((void **)&wuifireinfo, sizeof(wuifiredata));

  if(ADF_Read_w001001(adf_dir, wuifireinfo)!=0){
    FREEMEMORY(wuifireinfo);
    return NULL;
  }
  ADF2PNG(casename, wuifireinfo->vals, wuifireinfo->nrows, wuifireinfo->ncols);
  return wuifireinfo;
}


#endif
