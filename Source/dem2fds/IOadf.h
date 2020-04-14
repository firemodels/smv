#ifndef IOADF_H_DEFINED
#define IOADF_H_DEFINED

#include "smv_endian.h"

/* --------------------------  firedata ------------------------------------ */

typedef struct _wuifiredata {
  int tiles_ncolumns, tiles_nrows;
  int *vals, vals_nx, vals_ny, vals_ntypes;
  int *case_vals, case_nx, case_ny;
  int left_edge, right_edge, bottom_edge, top_edge;
  int fire_types[101];
  float lat_min, lat_max, long_min, long_max;
  float res_x, res_y;
} wuifiredata;


wuifiredata *ADF_GetFireData(char *adf_dir, char *casename);
int ADF_Read_hdr(char *adf_dir, int *HCellType, int *CompFlag, double *HPixelSizeX, double *HPixelSizeY, int *HTilesPerRow, int *HTilesPerColumn,
                 int *HTileXSize, int *HTileYSize);
int ADF_Read_w001001x(char *adf_dir, int **tile_info, int *ntiles);
int ADF_Read_w001001(char *adf_dir, wuifiredata *wuifireinfo);
int ADF_Read_dblbnd(char *adf_dir, double *D_LLX, double *D_LLY, double *D_URX, double *D_URY);
void ADF2PNG(char *basename, int *vals, int nrows, int ncols);
int ADF_GetFireIndex(wuifiredata *wuifireinfo, float longitude, float latitude);

#endif
