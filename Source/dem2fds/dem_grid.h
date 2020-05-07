#ifndef DEM_GRID_H_DEFINED
#define DEM_GRID_H_DEFINED
//#define LEN_BUFFER 1024

#ifdef WIN32
#define STDCALLF extern void _stdcall
#else
#define STDCALLF extern void
#endif

#define GRID_ALLOCATE   -1
#define GRID_INIT       0
#define GRID_INT_DATA   1
#define GRID_FLOAT_DATA 2
#define GRID_IMAGE_DATA 3

/* --------------------------  griddata ------------------------------------ */

typedef struct _griddata {
  char *file, *image_file;
  gdImagePtr image;
  int image_nrows, image_ncols;
  int type;
  int ncols, nrows, kbar;
  int have_ncols, have_nrows;
  float xllcorner, yllcorner, cellsize;
  int have_xllcorner, have_yllcorner, have_cellsize;
  float dx, dy;
  int have_dx, have_dy;
  float latmin, latmax, longmin, longmax;
  float xmax, ymax, zmin, zmax;
  int nz;
  int have_latmin, have_latmax, have_longmin, have_longmax;
  float valmin, valmax;
  void *vals;
} griddata;


EXTERNCPP void FreeGridData(griddata *data);
EXTERNCPP int CopyGridData(griddata *data, char *file);
EXTERNCPP griddata *ReadGridData(char *directory, char *file, char *mode);
#endif
