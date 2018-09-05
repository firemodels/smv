#ifndef ISODEFS_H_DEFINED
#define ISODEFS_H_DEFINED
#define INCPOINTS 100000

#if defined(WIN32)
#include <windows.h>
#endif

#ifdef pp_DRAWISO
#ifdef pp_OSX
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

#ifdef IN_ISOBOX
#define SV_EXTERN
#else
#define SV_EXTERN extern
#endif

/* iso-surface definitions */

typedef struct {
  float xyz[3],*color,distance;
  unsigned char flag, ctexturecolor, cnorm;
} isovert;

typedef struct {
  isovert *v1, *v2, *v3;
} isotri;

typedef struct {
  float level;
  float *color;
  int dataflag;
  int  nvertices, ntriangles, nnorm;      /* actual number */
  int maxvertices, maxtriangles, maxnorm; /* space reserved */
  int colorindex;
  int normtype, defined, plottype;
  int *triangles;
  int *closestnodes;

  isotri *iso_triangles; // new isosurface data structures
  int niso_triangles, niso_triangles_opaque, niso_triangles_transparent; // new isosurface datastructures
  isovert *iso_vertices;
  int niso_vertices;

  unsigned short *vertices, *tvertices;
  unsigned char *color8;
  int *sortedlist,*rank;
  float xmin, ymin, zmin, xyzmaxdiff;
  float *xvert, *yvert, *zvert, *tvert;
  float tmin, tmax;
  float *xnorm, *ynorm, *znorm;
  short *norm, *vertexnorm;
  unsigned char *comp_bufferframe, *full_bufferframe;
  int ncomp_bufferframe, nfull_bufferframe;
  unsigned char *s_norm;
  int cullfaces;
  int compression_type;
} isosurface;

typedef struct {
  int index;
  unsigned short vertex[3];
} sortdata;

typedef struct {
  int index;
  int sortedlist;
} rankdata;

typedef struct {
  int index;
  int closest;
} orderdata;

#define CCIsoSurface2File  _F(iso2file)
#define CCIsoSurfaceT2File _F(isot2file)
#define CCIsoHeader        _F(isoheader)
#define CCTIsoHeader       _F(tisoheader)

SV_EXTERN void CCIsoHeader(char *isofile,
                 char *isolonglabel, char *isoshortlabel, char *isounits,
                 float *levels, int *nlevels, int *error);
SV_EXTERN void CCTIsoHeader(char *isofile,
                 char *isolonglabel, char *isoshortlabel, char *isounits,
                 float *levels, int *nlevels, int *error);
SV_EXTERN void IsoOut(FILE *isostream,float t, int timeindex, isosurface *surface, int *error);
SV_EXTERN void CCIsoSurface2File(char *isofile, float *t, float *data, char *iblank, float *level, int *nlevels,
     float *xplt, int *nx, float *yplt, int *ny, float *zplt, int *nz,int *reduce_triangles, int *error);
SV_EXTERN void CCIsoSurfaceT2File(char *isofile, float *t, float *data, int *data2flag, float *data2, int *iblank,
						float *level, int *nlevels,
                   float *xplt, int *nx,
                   float *yplt, int *ny,
                   float *zplt, int *nz,
                   int *reduce_triangles, int *error);

SV_EXTERN int CompressIsoSurface(isosurface *surface, int reduce_triangles,
                        float xmin, float xmax,
                        float ymin, float ymax,
                        float zmin, float zmax);
SV_EXTERN int UpdateIsosurface(isosurface *surface,
                      const float *xvert,
                      const float *yvert,
                      const float *zvert,
                      const float *tvert,
                      const int *closestnodes,
                      int nvert,
                      const int *triangles,
                      int ntriangles);

SV_EXTERN void DrawIsosurface(const isosurface *isodata);
SV_EXTERN void FreeSurface(isosurface *surfacedata);
SV_EXTERN void InitIsoSurface(isosurface *surfacedata, float level, float *color, int colorindex);
SV_EXTERN int ResizeSurface(isosurface *surfacedata, int incvert, int inctrilist, int incnorm);
SV_EXTERN void GetIsoBox(float x[2], float y[2], float z[2], float *xyz0, float *vals, float level,
               float *xyzverts, int *nvert, int *triangles, int *ntriangles, int *polys, int *npolys);
SV_EXTERN int GetIsoHexaHedron(float *x, float *y, float *z, float *xyz0,
               float *vals, float *tvals, int *nodeindexes, float level,
               float *xvert, float *yvert, float *zvert, float *tvert, int *closestnodes, int *nvert,
               int *triangles, int *ntriangles,
               int *polys, int *npolys);
int GetIsoSurface(isosurface *surface,
                  const float *data,
                  const float *tdata,
                  const char *iblank_cell,
                  float level, float dlevel,
                  const float *xplt, int nx,
                  const float *yplt, int ny,
                  const float *zplt, int nz
                   );

SV_EXTERN void ReduceToUnit(float v[3]);
SV_EXTERN void CalcNormal(const float *v1, const float *v2, const float *v3, float *out);
SV_EXTERN void CalcNormal2(const unsigned short *v1,
                           const unsigned short *v2,
                           const unsigned short *v3,
                           float *out, float *area);
SV_EXTERN void CalcNormal2f(const float *v1,
                           const float *v2,
                           const float *v3,
                           float *out, float *area);
SV_EXTERN int GetNormalSurface(isosurface *surfacedata);
#endif





