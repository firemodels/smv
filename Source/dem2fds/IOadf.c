#include "options.h"
#ifdef pp_ADF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MALLOCC.h"
#include "IOadf.h"

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

/* ------------------ ADF_Read_w001001 ------------------------ */

int ADF_Read_w001001(int **tileinfo_arg, int *ntiles_arg, int **vals){
  FILE *stream;
  int i;
  int *vals_local;
  int npixels, nlines;

  int HCellType, CompFlag;
  double HPixelSizeX, HPixelSizeY;
  int HTilesPerRow, HTilesPerColumn;
  int HTileXSize, HTileYSize;
  double D_LRX, D_LRY, D_URX, D_URY;
  int *tileinfo;
  int nrows, ncols, ivals;

  int *tile_info, ntiles;

  ADF_Read_hdr(&HCellType, &CompFlag, &HPixelSizeX, &HPixelSizeY, &HTilesPerRow, &HTilesPerColumn, &HTileXSize, &HTileYSize);
  ADF_Read_dblbnd(&D_LRX, &D_LRY, &D_URX, &D_URY);
  ADF_Read_w001001x(&tile_info, &ntiles);

  *tileinfo_arg = tileinfo;
  *ntiles_arg = ntiles;

  ncols = HTilesPerRow*HTileXSize;
  nrows = HTilesPerColumn*HTileYSize;

  stream = fopen("w001001.adf", "rb");
  if(stream==NULL||ncols*nrows<1)return 1;

  NewMemory((void **)&vals_local, nrows*ncols*sizeof(int));
  *vals = vals_local;

  ivals = 0;
  for(i = 0; i<ntiles; i++){
    int RMin, tile_size, offset;
    short RMinshort, RTileSize;
    char RMinSize;
    unsigned char RTileType;

    offset     = 2*tile_info[2*i];
    tile_size  = 2*tile_info[2*i+1];

    fseek(stream, offset, SEEK_SET);
    fread(&RTileSize, 2, 1, stream);
    RTileSize = ShortSwitch(RTileSize);

    fseek(stream, offset+2, SEEK_SET);
    fread(&RTileType, 1, 1, stream);

    fseek(stream, offset+3, SEEK_SET);
    fread(&RMinSize, 1, 1, stream);

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

    if(RTileType==0){         //  0
    }
    else if(RTileType==8){    //  8
    }
    else if(RTileType==16){   // 10
    }
    else if(RTileType==223){  // DF
    }
    else if(RTileType==224){  // E0
    }
    else if(RTileType==240){  // F0
    }
    else if(RTileType==248){  // F8
    }
    else if(RTileType==252){  // FC
    }
    else{
      ASSERT(0);
    }

    printf("%i ", (int)RTileType);
    if(i%HTilesPerRow==HTilesPerRow-1){
      printf("\n");
    }
  }

  npixels = (D_URX-D_LRX)/HPixelSizeX;
  nlines = (D_URY-D_LRY)/HPixelSizeY;

  fclose(stream);
  return 0;
}

#endif
