#ifndef IOADF_H_DEFINED
#define IOADF_H_DEFINED

#include "smv_endian.h"

int ADF_Read_hdr(int *HCellType, int *CompFlag, double *HPixelSizeX, double *HPixelSizeY, int *HTilesPerRow, int *HTilesPerColumn,
                 int *HTileXSize, int *HTileYSize);
int ADF_Read_w001001x(int **tile_info, int *ntiles);
int ADF_Read_w001001(int **tileinfo, int *ntiles, int **vals, int *nrows, int *ncols);
int ADF_Read_dblbnd(double *D_LLX, double *D_LLY, double *D_URX, double *D_URY);
void ADF2PNG(int *vals, int nrows, int ncols);

#endif
