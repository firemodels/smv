#ifndef IOADF_H_DEFINED
#define IOADF_H_DEFINED

#include "smv_endian.h"

/* --------------------------  firedata ------------------------------------ */

typedef struct _wuifiredata {
  int *vals, nx, ny, ntypes;
  int *case_vals, case_nx, case_ny;
  float latmin, latmax, longmin, longmax;
} wuifiredata;


wuifiredata *ADF_GetFireData(char *adf_dir, char *casename);
int ADF_Read_hdr(char *adf_dir, int *HCellType, int *CompFlag, double *HPixelSizeX, double *HPixelSizeY, int *HTilesPerRow, int *HTilesPerColumn,
                 int *HTileXSize, int *HTileYSize);
int ADF_Read_w001001x(char *adf_dir, int **tile_info, int *ntiles);
int ADF_Read_w001001(char *adf_dir, wuifiredata *wuifireinfo);
int ADF_Read_dblbnd(char *adf_dir, double *D_LLX, double *D_LLY, double *D_URX, double *D_URY);
void ADF2PNG(char *basename, int *vals, int nrows, int ncols);

#endif
