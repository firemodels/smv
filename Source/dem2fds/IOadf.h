#ifndef IOADF_H_DEFINED
#define IOADF_H_DEFINED

#include "smv_endian.h"

/* --------------------------  firedata ------------------------------------ */

typedef struct _wuifiredata {
  int ncols, nrows;
  int *vals, vals_ntypes;
  int *case_vals, case_nx, case_ny;
  int left_edge, right_edge, bottom_edge, top_edge;
  int fire_types[101];
  float lat_min, lat_max, long_min, long_max;
  float res_x, res_y;
} wuifiredata;

wuifiredata *ADF_GetFireData(char *adf_dir, char *casename);
int ADF_Read_w001001(char *adf_dir, wuifiredata *wuifireinfo);
void ADF2PNG(char *basename, int *vals, int nrows, int ncols);
int ADF_GetFireIndex(wuifiredata *wuifireinfo, float longitude, float latitude);
void ADF_GetSurfs(wuifiredata *wuifireinfo, struct _elevdata *fds_elevs, float *verts, int nverts, int *faces, int *surfs, int nfaces);

#endif
