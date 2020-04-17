#ifndef DEM_UTIL_H_DEFINED
#define DEM_UTIL_H_DEFINED
#define FDS_OBST 0
#define FDS_GEOM 1
#define LEN_BUFFER 1024
#define EARTH_RADIUS 6371000.0
#define INTERP1D(f,v1,v2) ((1.0-(f))*(v1)+(f)*(v2))
#define DONT_INTERPOLATE 0
#define INTERPOLATE 1

#include "dem_util.h"

#ifdef WIN32
#define STDCALLF extern void _stdcall
#else
#define STDCALLF extern void
#endif

//subroutine elev2geom(output_elev_file, xgrid, ibar, ygrid, jbar, vals, nvals)
#define FORTelev2geom _F(elev2geom)
STDCALLF FORTelev2geom(char *output_elev_file, float *xgrid, int *ibar, float *ygrid, int *jbar, float *vals, int *nvals, FILE_SIZE filelen);

/* --------------------------  elevdata ------------------------------------ */

typedef struct _elevdata {
  int ncols, nrows, nz, use_it;
  float xllcorner, yllcorner, cellsize;
  char *headerfile, *datafile;
  char filelabel[13];
  float lat_min, lat_max, long_min, long_max, dlong, dlat;
  float lat_min_orig, lat_max_orig, long_min_orig, long_max_orig;
  float val_min, val_max;
  float xmax, ymax, zmin, zmax;
  float xref, yref, longref, latref;
  float *valbuffer;
  gdImagePtr image;
} elevdata;

typedef struct _wuigriddata {
  int ncols, nrows;
  int *vals, vals_ntypes;
  float *fvals;
  int *case_vals, case_nx, case_ny;
  int fire_types[101];
  float lat_min, lat_max, long_min, long_max;
  float dlong, dlat;
} wuigriddata;


/* --------------------------  excludedata ------------------------------------ */

typedef struct {
  float xmin, xmax, ymin, ymax;
} excludedata;

EXTERNCPP void GenerateFDSInputFile(char *casename, char *casename_fds, elevdata *fds_elevs, int option, wuigriddata *wuifireinfo);
EXTERNCPP int GetElevations(char *elevfile, char *image_file, char *image_type, elevdata *fds_elevs);
EXTERNCPP wuigriddata *GetFireData(char *adf_dir, char *casename);

SVEXTERN char image_dir[1024], elev_dir[1024];
#define NFIRETYPES 20
#define MAXFIRETYPE 100
SVEXTERN int firehash[MAXFIRETYPE];
SVEXTERN char fire_dir[1024];
#ifdef INMAIN
SVEXTERN int have_firetypes[NFIRETYPES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
SVEXTERN int fireindices[NFIRETYPES] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 90, 91, 92, 93, 98, 99, 100};
SVEXTERN int firecolors[3*NFIRETYPES] = {
255, 255, 190, // FBFM1
255, 255,   0, // FBFM2
230, 197,   7, // FBFM3
255, 211, 127, // FBFM4
255, 169,   0, // FBFM5
206, 169, 102, // FBFM6
136, 111,  69, // FBFM7
211, 255, 190, // FBFM8
111, 168,   0, // FBFM9
 36, 116,   0, // FBFM10
232, 190, 255, // FBFM11
123, 141, 245, // FBFM12
197,   0, 255, // FBFM13

  0,   0,   0,  // UNKNOWN  FB90 - code 90 appears in data - not documented
103, 103, 103, // Urban    FB91
225, 225, 225, // Snow     FB92
255, 238, 238, // Agricul   FB93
  0,   7, 214, // Water    FB98
 77, 108, 111, // Barren   FB99
  0,   0,   0  // No Data  
};
SVEXTERN char *firetypes[NFIRETYPES] = {
"FBFM1", "FBFM2", "FBFM3", "FBFM4", "FBFM5", "FBFM6", "FBFM7", "FBFM8", "FBFM9", "FBFM10", "FBFM11", "FBFM12", "FBFM13",
"FB90", "FB91", "FB92", "FB93", "FB98", "FB99", "NODATA"};
#else
SVEXTERN int have_firetypes[NFIRETYPES];
SVEXTERN char image_dir[1024];
SVEXTERN char elev_dir[1024];
SVEXTERN int fireindices[NFIRETYPES];
SVEXTERN char *firetypes[NFIRETYPES];
SVEXTERN int firecolors[];
#endif
SVEXTERN int SVDECL(overlap_size,0), SVDECL(show_maps,0);
SVEXTERN char surf_id1[1024], surf_id2[1024], matl_id[1024];
SVEXTERN int SVDECL(nexcludeinfo, 0);
SVEXTERN excludedata SVDECL(*excludeinfo, NULL);
SVEXTERN float SVDECL(buff_dist, 10.0);
SVEXTERN int   SVDECL(nmeshx,1), SVDECL(nmeshy,1);
SVEXTERN float   SVDECL(*xplt, NULL), SVDECL(*yplt, NULL);
SVEXTERN int SVDECL(terrain_image_width, 2048);
SVEXTERN int SVDECL(terrain_image_height, 0);
SVEXTERN char image_type[10];
#endif
