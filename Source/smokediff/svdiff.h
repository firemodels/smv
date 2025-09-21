#ifndef SVDIFF_H_DEFINED
#define SVDIFF_H_DEFINED
#include "histogram.h"

//************************** pre-processing directives ****************************************

#ifdef INMAIN
#define EXTERN
#else
#define EXTERN extern
#endif

#include "string_util.h"


#ifdef _WIN32
#define STDCALLF extern void _stdcall
#else
#define STDCALLF extern void
#endif

#ifndef FILE_SIZE
#define FILE_SIZE unsigned long long
#endif

#define SLICE_NODE_CENTER 1
#define SLICE_CELL_CENTER 2
#define SLICE_TERRAIN 4

//************************** data structures ****************************************

typedef struct {
  int ibar, jbar, kbar;
  float xbar0, xbar, ybar0, ybar, zbar0, zbar;
  float dx, dy, dz;
  float *xplt, *yplt, *zplt;
} meshdata;

typedef struct _boundary {
  char *file;
  int version;
  struct _boundary *boundary2;
  FILE_SIZE filesize;
  int npatches;
  int *pi1, *pi2, *pj1, *pj2, *pk1, *pk2, *patchdir;
  int *patch2index, *patchsize, *qoffset;
  char keyword[255];
  int boundarytype;
  histogramdata *histogram;
  meshdata *boundarymesh;
  flowlabels label;
} boundary;

typedef struct _slice {
  char *file;
  int is1, is2, js1, js2, ks1, ks2;
  float xmin, xmax, ymin, ymax, zmin, zmax;
  FILE_SIZE filesize;
  int factor[3];
  int version;
  int volslice;
  struct _slice *slice2;
  char keyword[255];
  int slicetype;
  meshdata *slicemesh;
  histogramdata *histogram;
  flowlabels label;
} slice;

typedef struct _plot3d {
  char keyword[255];
  char *file;
  float time;
  struct _plot3d *plot3d2;
  float xmin, xmax, ymin, ymax, zmin, zmax;
  histogramdata *histogram[5];
  meshdata *plot3dmesh;
  flowlabels labels[5];
} plot3d;

typedef struct {
  slice *sliceinfo;
  meshdata *meshinfo;
  plot3d *plot3dinfo;
  boundary *boundaryinfo;
  char *dir;
  int nmeshes;
  int nsliceinfo, nplot3dinfo, nboundary_files;
} casedata;

//************************** headers ****************************************

int MeshMatch(meshdata *mesh1, meshdata *mesh2);
int ReadSMV(bufferstreamdata *streamsmv, FILE *stream_out, casedata *smvcase);
void SetupBoundary(FILE *stream_out);
void SetupSlice(FILE *stream_out);
void SetupPlot3D(FILE *stream_out);
plot3d *GetPlot3D(plot3d *plot3din, casedata *case2);
slice *GetSlice(slice *slicein, casedata *case2);
boundary *GetBoundary(boundary *boundaryin, casedata *case2);
void DiffBoundarYes(FILE *stream_out);
void DiffSlices(FILE *stream_out);
void DiffPlot3Ds(FILE *stream_out);
int SimilarGrid(meshdata *mesh1, meshdata *mesh2, int *factor);
int ExactGrid(meshdata *mesh1, meshdata *mesh2, int *factor);
int GetPatchIndex(int in1, boundary *boundaryin, boundary *boundaryout);

//************************** global variables ****************************************

EXTERN char pp[2];
EXTERN casedata *caseinfo;
EXTERN char *sourcedir1, *sourcedir2, *destdir;
EXTERN int test_mode, display_warnings;
EXTERN char type_label[1024];
EXTERN FILE *LOG_FILENAME;

#endif
