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

/* ------------------ ADF_Read_dblbnd ------------------------ */

int ADF_Read_dblbnd(char *adf_dir, double *D_LLX, double *D_LLY, double *D_URX, double *D_URY){
  FILE *stream;

  stream = fopen_indir(adf_dir, "dblbnd.adf", "rb");
  if(stream==NULL)return 1;

  fseek(stream, 0, SEEK_SET);
  fread(D_LLX, 8, 1, stream);
  *D_LLX = DoubleSwitch(*D_LLX);

  fseek(stream, 8, SEEK_SET);
  fread(D_LLY, 8, 1, stream);
  *D_LLY = DoubleSwitch(*D_LLY);

  fseek(stream, 16, SEEK_SET);
  fread(D_URX, 8, 1, stream);
  *D_URX = DoubleSwitch(*D_URX);

  fseek(stream, 24, SEEK_SET);
  fread(D_URY, 8, 1, stream);
  *D_URY = DoubleSwitch(*D_URY);

  fclose(stream);
  return 0;
}

/* ------------------ ADF_Read_hdr ------------------------ */

int ADF_Read_hdr(char *adf_dir, int *HCellType, int *CompFlag, double *HPixelSizeX, double *HPixelSizeY, int *HTilesPerRow, int *HTilesPerColumn,
  int *HTileXSize, int *HTileYSize){
  FILE *stream;

  stream = fopen_indir(adf_dir, "hdr.adf", "rb");
  if(stream==NULL)return 1;

  fseek(stream, 16, SEEK_SET);
  fread(HCellType, 4, 1, stream);
  *HCellType = IntSwitch(*HCellType);

  fseek(stream, 20, SEEK_SET);
  fread(CompFlag, 4, 1, stream);
  *CompFlag = IntSwitch(*CompFlag);

  fseek(stream, 256, SEEK_SET);
  fread(HPixelSizeX, 8, 1, stream);
  *HPixelSizeX = DoubleSwitch(*HPixelSizeX);

  fseek(stream, 264, SEEK_SET);
  fread(HPixelSizeY, 8, 1, stream);
  *HPixelSizeY = DoubleSwitch(*HPixelSizeY);

  fseek(stream, 288, SEEK_SET);
  fread(HTilesPerRow, 4, 1, stream);
  *HTilesPerRow = IntSwitch(*HTilesPerRow);

  fseek(stream, 292, SEEK_SET);
  fread(HTilesPerColumn, 4, 1, stream);
  *HTilesPerColumn = IntSwitch(*HTilesPerColumn);

  fseek(stream, 296, SEEK_SET);
  fread(HTileXSize, 4, 1, stream);
  *HTileXSize = IntSwitch(*HTileXSize);

  fseek(stream, 304, SEEK_SET);
  fread(HTileYSize, 4, 1, stream);
  *HTileYSize = IntSwitch(*HTileYSize);

  fclose(stream);
  return 0;
}

/* ------------------ ADF_Read_w001001x ------------------------ */

int ADF_Read_w001001x(char *adf_dir, int **tile_info, int *ntiles){
  FILE *stream;
  int file_size;
  int i;
  int *tinfo = NULL;

  stream = fopen_indir(adf_dir, "w001001x.adf", "rb");
  if(stream==NULL)return 1;

  fseek(stream, 24, SEEK_SET);
  fread(&file_size, 4, 1, stream);
  file_size = IntSwitch(file_size);
  *ntiles = (2*file_size-100)/8;
  if(*ntiles<1){
    fclose(stream);
    return 1;
  }
  NewMemory((void **)&tinfo, 2*(*ntiles)*sizeof(int));
  *tile_info = tinfo;

  for(i = 0; i<*ntiles; i++){
    int offset, size;

    fseek(stream, 100+8*i, SEEK_SET);
    fread(&offset, 4, 1, stream);
    offset = IntSwitch(offset);

    fseek(stream, 100+8*i+4, SEEK_SET);
    fread(&size, 4, 1, stream);
    size = IntSwitch(size);

    tinfo[2*i]   = offset;
    tinfo[2*i+1] = size;
  }

  fclose(stream);
  return 0;
}

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

/* ------------------ ADF_CopyValues ------------------------ */

void ADF_CopyValues(unsigned char *val_in, int nx_in, int ny_in, int offsetx_in, int offsety_in, 
                int *val_out, int nx_out, int ny_out){
  int j_in;

  for(j_in = 0; j_in<ny_in; j_in++){
    int i_in, j_out;

    j_out = j_in+offsety_in;
    if(j_out>=ny_out)return;
    for(i_in = 0; i_in<nx_in; i_in++){
      int i_out, index_out, index_in;
      unsigned char cval;

      i_out = i_in+offsetx_in;
      if(i_out>=nx_out)break;

      index_in = j_in*nx_in+i_in;
      index_out = j_out*nx_out + i_out;
      cval = val_in[index_in];
      val_out[index_out] = (int)cval;
    }
  }
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
  wuifireinfo->tiles_ncolumns = n_columns;
  wuifireinfo->tiles_nrows = n_rows;

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

  ix = CLAMP(wuifireinfo->vals_nx*(longitude-wuifireinfo->long_min)/(wuifireinfo->long_max-wuifireinfo->long_min),0,wuifireinfo->vals_nx-1);
  iy = CLAMP(wuifireinfo->vals_ny*(latitude-wuifireinfo->lat_min)/(wuifireinfo->lat_max-wuifireinfo->lat_min), 0, wuifireinfo->vals_ny-1);
  index = iy*wuifireinfo->vals_nx+ix;
  val = wuifireinfo->vals[index];
  return val;
}

  /* ------------------ ADF_Read_w001001 ------------------------ */

int ADF_Read_w001001(char *adf_dir, wuifiredata *wuifireinfo){
  FILE *stream;
  int i;
  unsigned char *tile_vals, *tile_buffer=NULL;
  int max_tilebuffer = -1;
  int npixels, nlines;
  int itile, tile_size0;
  int *tile_info, ntiles;

  int HCellType, CompFlag;
  double HPixelSizeX, HPixelSizeY;
  int HTilesPerRow, HTilesPerColumn;
  int HTileXSize, HTileYSize;
  double D_LRX, D_LRY, D_URX, D_URY;
  int total_nrows, total_ncols;
  int icol, irow;
  int compression_type[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int nfail = 0;

  wuifireinfo->vals_ntypes = 0;

  if(ADF_Read_hdr(adf_dir, &HCellType, &CompFlag, &HPixelSizeX, &HPixelSizeY, &HTilesPerRow, &HTilesPerColumn, &HTileXSize, &HTileYSize)!=0)return 1;
  if(ADF_Read_dblbnd(adf_dir, &D_LRX, &D_LRY, &D_URX, &D_URY)!=0)return 1;
  if(ADF_Read_w001001x(adf_dir, &tile_info, &ntiles)!=0)return 1;
  if(ADF_GetInfo(adf_dir, wuifireinfo)!=0)return 1;

  npixels = (D_URX-D_LRX)/HPixelSizeX;
  nlines = (D_URY-D_LRY)/HPixelSizeY;

  wuifireinfo->vals_ny = nlines;
  wuifireinfo->vals_nx = npixels;

  total_ncols = HTilesPerRow*HTileXSize;
  total_nrows = HTilesPerColumn*HTileYSize;
  tile_size0 = HTileYSize*HTileXSize;

  stream = fopen_indir(adf_dir, "w001001.adf", "rb");
  if(stream==NULL||total_ncols*total_nrows<1)return 1;

  {
    int *vals_local;

    NewMemory((void **)&vals_local, npixels*nlines*sizeof(int));
    wuifireinfo->vals = (int *)vals_local;

    for(i = 0; i<npixels*nlines; i++){
      vals_local[i] = -1;
    }
  }
  NewMemory((void **)&tile_vals, tile_size0*sizeof(unsigned char));

  for(irow=-1,icol=-1,i = 0; i<ntiles; i++,icol++){
    int j;
    int RMin, tile_size, offset;
    short RMinshort, RTileSize;
    unsigned char RMinSize, RTileType;
    int nbuffer;

    if(i%HTilesPerRow==0){
      irow++;
      icol = 0;
    }

    offset     = 2*tile_info[2*i];
    tile_size  = 2*tile_info[2*i+1];

    fseek(stream, offset, SEEK_SET);
    fread(&RTileSize, 2, 1, stream);
    RTileSize = ShortSwitch(RTileSize);

    itile = 0;
    if(RTileSize>0){
      fseek(stream, offset+2, SEEK_SET);
      fread(&RTileType, 1, 1, stream);

      fseek(stream, offset+3, SEEK_SET);
      fread(&RMinSize, 1, 1, stream);

      if(RMinSize>0){
        fseek(stream, offset+4, SEEK_SET);
        if(RMinSize==4){
          fread(&RMin, 4, 1, stream);
          RMin = IntSwitch(RMin);
        }
        else{
          fread(&RMinshort, 2, 1, stream);
          RMinshort = ShortSwitch(RMinshort);
          RMin = (int)RMinshort;
        }
      }
      fseek(stream, offset+4+RMinSize, SEEK_SET);
      nbuffer = RTileSize*2-2-RMinSize;
      if(nbuffer>max_tilebuffer){
        max_tilebuffer = nbuffer+1000;
        FREEMEMORY(tile_buffer);
        NewMemory((void **)&tile_buffer, max_tilebuffer*sizeof(unsigned char));
      }
      fread(tile_buffer, nbuffer, 1, stream);

      if(RTileType==0){         // 0x00
        compression_type[0]++;
        for(j = 0; j<nbuffer; j++){
          tile_vals[j] = (unsigned char)RMin;
        }
        ADF_CopyValues(tile_vals, HTileXSize, HTileYSize, icol*HTileXSize, irow*HTileYSize, wuifireinfo->vals, npixels, nlines);
      }
      else if(RTileType==1){    // 0x01
        compression_type[1]++;
      }
      else if(RTileType==4){    // 0x04
        compression_type[2]++;
        for(j = 0; j<nbuffer; j++){
          unsigned char cval;

          cval = tile_buffer[j];
          tile_vals[2*j+0] = (cval&0xF0)>4;
          tile_vals[2*j+1] = cval&0x0F;
        }
        ADF_CopyValues(tile_vals, HTileXSize, HTileYSize, icol*HTileXSize, irow*HTileYSize, wuifireinfo->vals, npixels, nlines);
      }
      else if(RTileType==8){    // 0x08
        compression_type[3]++;
        ADF_CopyValues(tile_buffer, HTileXSize, HTileYSize, icol*HTileXSize, irow*HTileYSize, wuifireinfo->vals, npixels, nlines);
      }
      else if(RTileType==16){   // 0x10
        compression_type[4]++;
        nfail++;
      }
      else if(RTileType==32){   // 0x20
        compression_type[5]++;
        nfail++;
      }
      else if(RTileType==207){  // 0xCF
        compression_type[6]++;
        nfail++;
      }
      else if(RTileType==215){  // 0xD7
        compression_type[7]++;
        nfail++;
      }
      else if(RTileType==223){  // 0xDF
        compression_type[8]++;
        nfail++;
      }
      else if(RTileType==224){  // 0xE0
        compression_type[9]++;
        nfail++;
      }
      else if(RTileType==240){  // 0xF0
        compression_type[10]++;
        nfail++;
      }
      else if(RTileType==248||RTileType==252){  // 0xF8 0xFC
        if(RTileType==248)compression_type[11]++;
        if(RTileType==252)compression_type[12]++;
        for(j = 0; j<nbuffer; j+=2){
          int kkk;
          unsigned char size, val;

          size = tile_buffer[j];
          val = tile_buffer[j+1];
          for(kkk = itile; kkk<MIN(itile+size,tile_size0); kkk++){
            tile_vals[kkk] = val;
          }
          itile += size;
        }
        ADF_CopyValues(tile_vals, HTileXSize, HTileYSize, icol*HTileXSize, irow*HTileYSize, wuifireinfo->vals, npixels, nlines);
      }
      else if(RTileType==255){  // 0xFF
        compression_type[13]++;
        nfail++;
      }
      else{
        compression_type[14]++;
        ASSERT(0);
        nfail++;
      }
      if(RTileType!=248)(wuifireinfo->vals_ntypes)++;
    }
  }
  fclose(stream);
  FREEMEMORY(tile_buffer);
  FREEMEMORY(tile_vals);
  if(nfail>0){
    printf("***warning: number of un-handled tiles=%i\n", nfail);
    printf("tile representation distribution:\n");
    for(i = 0; i<15; i++){
      printf(" %i/%i ", i,compression_type[i]);
    }
    printf("\n");
  }
#ifdef _DEBUG
  if(nfail==0){
    printf("tile representation distribution:\n");
    for(i = 0; i<15; i++){
      printf(" %i/%i ", i,compression_type[i]);
    }
    printf("\n");
  }
#endif
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
  ADF2PNG(casename, wuifireinfo->vals, wuifireinfo->vals_ny, wuifireinfo->vals_nx);
  return wuifireinfo;
}


#endif
