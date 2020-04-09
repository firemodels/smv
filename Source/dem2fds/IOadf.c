#include "options.h"
#ifdef pp_ADF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MALLOCC.h"
#include "IOadf.h"
#include "datadefs.h"
#include "gd.h"

/* ------------------ ADF_Read_dblbnd ------------------------ */

int ADF_Read_dblbnd(double *D_LLX, double *D_LLY, double *D_URX, double *D_URY){
  FILE *stream;

  stream = fopen("dblbnd.adf", "rb");
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

int ADF_Read_hdr(int *HCellType, int *CompFlag, double *HPixelSizeX, double *HPixelSizeY, int *HTilesPerRow, int *HTilesPerColumn,
  int *HTileXSize, int *HTileYSize){
  FILE *stream;

  stream = fopen("hdr.adf", "rb");
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

int ADF_Read_w001001x(int **tile_info, int *ntiles){
  FILE *stream;
  int file_size;
  int i;
  int *tinfo = NULL;

  stream = fopen("w001001x.adf", "rb");
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

/* ------------------ GetValIndex ------------------------ */

#define NFIRE_TYPES 20
int GetValIndex(int val){
  int j;
  int fire_type[NFIRE_TYPES] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 90, 91, 92, 93, 98, 99,9999};

  for(j = 0; j<NFIRE_TYPES-1; j++){
    if(val==fire_type[j])return j;
  }
  return NFIRE_TYPES-1;
}

/* ------------------ CopyValues ------------------------ */

void CopyValues(unsigned char *val_in, int nx_in, int ny_in, int offsetx_in, int offsety_in, 
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

void ADF2PNG(int *vals, int nrows, int ncols){
  int *val_table = NULL;
  int i, ntable=0;
  int *rgb;
  int dcolor;
  gdImagePtr RENDERimage;
  FILE *RENDERfile = NULL;

  NewMemory((void **)&val_table, nrows*ncols*sizeof(int));
  for(i = 0; i<nrows*ncols;i++){
    int j, ival;

    if(GetValIndex2(vals[i],val_table,ntable)==-1)val_table[ntable++] = vals[i];
  }
  NewMemory((void **)&rgb, 3*ntable*sizeof(int));
  dcolor = 256*256*256/(ntable+2);
  for(i = 0; i<ntable; i++){
    int icolor;
    int red, green, blue;

    icolor = (i+1)*dcolor;
    red = CLAMP(icolor%256, 0, 255);
    green = CLAMP((icolor/256)%256,0,255);
    blue = CLAMP(icolor/(256*256),0,255);
    rgb[3*i] = red;
    rgb[3*i+1] = green;
    rgb[3*i+2] = blue;
  }

  RENDERimage = gdImageCreateTrueColor(3*ncols, 3*nrows);

  for(i = 0; i<ncols; i++){
    int j;

    for(j = 0; j<nrows; j++){
      unsigned char r, g, b;
      int rgb_local, index, ii, jj;

      index = vals[j*ncols+i];
      r = (unsigned char)val_table[3*index];
      g = (unsigned char)val_table[3*index+1];
      b = (unsigned char)val_table[3*index+2];
      rgb_local = (r<<16)|(g<<8)|b;
      for(ii=0;ii<3;ii++){
        for(jj=0;jj<3;jj++){
          gdImageSetPixel(RENDERimage, 3*i+ii, 3*j+jj, rgb_local);
        }
      }
    }
  }
  RENDERfile = fopen("w001001.png", "wb");
  gdImagePng(RENDERimage, RENDERfile);
}

  /* ------------------ ADF_Read_w001001 ------------------------ */

int ADF_Read_w001001(int **tileinfo_arg, int *ntiles_arg, int **vals_out, int *nvals_rows, int *nvals_cols){
  FILE *stream;
  int i;
  unsigned char *tile_vals, *tile_buffer=NULL;
  int max_tilebuffer = -1;
  int npixels, nlines;
  int itile, tile_size0;

  int HCellType, CompFlag;
  double HPixelSizeX, HPixelSizeY;
  int HTilesPerRow, HTilesPerColumn;
  int HTileXSize, HTileYSize;
  double D_LRX, D_LRY, D_URX, D_URY;
  int total_nrows, total_ncols;
  int icol, irow;

  int *tile_info, ntiles;

  ADF_Read_hdr(&HCellType, &CompFlag, &HPixelSizeX, &HPixelSizeY, &HTilesPerRow, &HTilesPerColumn, &HTileXSize, &HTileYSize);
  ADF_Read_dblbnd(&D_LRX, &D_LRY, &D_URX, &D_URY);
  ADF_Read_w001001x(&tile_info, &ntiles);

  npixels = (D_URX-D_LRX)/HPixelSizeX;
  nlines = (D_URY-D_LRY)/HPixelSizeY;

  *nvals_rows = nlines;
  *nvals_cols = npixels;

  *tileinfo_arg = tile_info;
  *ntiles_arg = ntiles;

  total_ncols = HTilesPerRow*HTileXSize;
  total_nrows = HTilesPerColumn*HTileYSize;
  tile_size0 = HTileYSize*HTileXSize;

  stream = fopen("w001001.adf", "rb");
  if(stream==NULL||total_ncols*total_nrows<1)return 1;

  {
    int *vals_local;

    NewMemory((void **)&vals_local, npixels*nlines*sizeof(int));
    *vals_out = (int *)vals_local;

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
      }
      else if(RTileType==1){    // 0x01
      }
      else if(RTileType==4){    // 0x04
      }
      else if(RTileType==8){    // 0x08
      }
      else if(RTileType==16){   // 0x10
      }
      else if(RTileType==32){   // 0x20
      }
      else if(RTileType==207){  // 0xCF
      }
      else if(RTileType==215){  // 0xD7
      }
      else if(RTileType==223){  // 0xDF
      }
      else if(RTileType==224){  // 0xE0
      }
      else if(RTileType==240){  // 0xF0
      }
      else if(RTileType==248){  // 0xF8
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
        CopyValues(tile_vals, HTileXSize, HTileYSize, icol*HTileXSize, irow*HTileYSize, *vals_out, npixels, nlines);
      }
      else if(RTileType==252){  // 0xFC
      }
      else if(RTileType==255){  // 0xFF
      }
      else{
        ASSERT(0);
      }
    }
  }
  fclose(stream);
  FREEMEMORY(tile_buffer);
  FREEMEMORY(tile_vals);
  return 0;
}

#endif
