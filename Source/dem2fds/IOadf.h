#ifndef IOADF_H_DEFINED
#define IOADF_H_DEFINED

#include "smv_endian.h"

/* --------------------------  firedata ------------------------------------ */

typedef struct _wuigriddata {
  int ncols, nrows;
  int *vals, vals_ntypes;
  float *fvals;
  int *case_vals, case_nx, case_ny;
  int fire_types[101];
  float lat_min, lat_max, long_min, long_max;
  float dlong, dlat;
} wuigriddata;

wuigriddata *ADF_GetFireData(char *adf_dir, char *casename);
int ADF_ReadIGrid(char *adf_dir, char *file, wuigriddata *wuifireinfo);
int ADF_ReadFGrid(char *adf_dir, char *file, wuigriddata *wuifireinfo);
void ADF2PNG(char *basename, int *vals, int nrows, int ncols);
int ADF_GetFireIndex(wuigriddata *wuifireinfo, float longitude, float latitude);
void ADF_GetSurfs(wuigriddata *wuifireinfo, struct _elevdata *fds_elevs, float *verts, int nverts, int *faces, int *surfs, int nfaces);

#endif
