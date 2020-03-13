#include "options.h"
#ifdef pp_ADF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MALLOCC.h"
#include "IOadf.h"

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
  *ntiles = (2*file_size-100)/8-1;
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
    fread(&size, 4, 1, stream);
    size = IntSwitch(size);
    tinfo[2*i] = offset;
    tinfo[2*i+1] = size;
  }
  fclose(stream);
  return 0;
}

/* ------------------ ADF_Read_w001001x ------------------------ */

int ADF_Read_w001001(int *tileinfo, int ntiles){
  FILE *stream;
  int i;
  char RMinSize;
  short RMin;

  stream = fopen("w001001.adf", "rb");
  if(stream==NULL)return 1;

  fseek(stream, 103, SEEK_SET);
  fread(&RMinSize, 1, 1, stream);
  fread(&RMin, 2, 1, stream);
  RMin = ShortSwitch(RMin);

  for(i = 0; i<ntiles; i++){

  }
  return 0;
}

#endif
