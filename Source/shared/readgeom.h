#ifndef READGEOM_H_DEFINED
#define READGEOM_H_DEFINED
#include "dmalloc.h"
#include "gd.h"
#include "options.h"
#include "string_util.h"

#include "shared_structures.h"

#include <string.h>

#define GEOM_GEOM 0
#define GEOM_ISO 1
#define GEOM_SLICE 2
#define GEOM_BOUNDARY 3
#define GEOM_CGEOM 4

#define HEADER_SIZE 4
#define TRAILER_SIZE 4
#define FORTREAD(var, size, count, STREAM)                                     \
  FSEEK(STREAM, HEADER_SIZE, SEEK_CUR);                                        \
  returncode = fread(var, size, count, STREAM);                                \
  if(returncode != count) returncode = 0;                                     \
  FSEEK(STREAM, TRAILER_SIZE, SEEK_CUR)

#define FORTREADBR(var, count, STREAM)                                         \
  FORTREAD(var, 4, (count), STREAM);                                           \
  if(returncode == 0) break;

void InitGeom(geomdata *geomi, int geomtype, int fdsblock,
              int have_cface_normals_arg, int block_number);
void RotateU2V(float *u, float *v, float *axis, float *angle);
EXTERNCPP void AngleAxis2Quat(float angle, float *axis, float *quat);
EXTERNCPP void Quat2Rot(float quat[4], float rot[16]);
EXTERNCPP void MultQuat(float x[4], float y[4], float z[4]);
int CompareTransparentTriangles(const void *arg1, const void *arg2);
EXTERNCPP void XYZ2AzElev(float *xyz, float *azimuth, float *elevation);
void GetVertType(int nverts, int *triangles, int ntriangles, int *vert_type);
void AverageVerts2(float v1[3], int v1type, float v2[3], int v2type,
                   float mesh_bounds[6], float *vavg);
void AverageVerts3(float v1[3], int v1type, float v2[3], int v2type,
                   float v3[3], int v3type, float mesh_bounds[6], float *vavg);

#ifdef pp_DECIMATE
#define IJNODE(i, j) ((j) * nx + (i))
int PtInTriangle(float *xy, float *v0, float *v1, float *v2, float *zval);
void DecimateTerrain(vertdata *verts, int nverts, tridata *triangles,
                     int ntriangles, vertdata **verts_new, int *nverts_new,
                     tridata **triangles_new, int *ntriangles_new,
                     float *boxmin, float *boxmax, int nx, int ny);
void DecimateAllTerrains(void);
#endif

void GetTriangleNormal(float *v1, float *v2, float *v3, float *normal,
                       float *area);
int CompareVerts(const void *arg1, const void *arg2);
float DistXY(float *x, float *y);
float GetAngle(float d1, float d2, float d3);
float GetMinAngle(tridata *trii);
void InitGeomlist(geomlistdata *geomlisti);
void ReadGeomFile2(geomdata *geomi);
void ReadGeomHeader0(geomdata *geomi, int *geom_frame_index, int *ntimes_local);
void ReadGeomHeader2(geomdata *geomi, int *ntimes_local);
void ReadGeomHeader(geomdata *geomi, int *geom_frame_index, int *ntimes_local);
void GetGeomDataHeader(char *file, int *ntimes_local, int *nvals);
void Normalize(float *xyz, int n);
float Dist2Plane(float x, float y, float z, float xyzp[3], float xyzpn[3]);
void InitBoxClipInfo(clipdata *ci, float xmin, float xmax, float ymin,
                     float ymax, float zmin, float zmax);
void InitCircle(unsigned int npoints, circdata *circinfo);
float Dist(float v1[3], float v2[3]);
int GetInterval(float val, float *array, int n);
#endif
