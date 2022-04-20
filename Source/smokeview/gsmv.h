#ifndef GSMV_H_DEFINED
#define GSMV_H_DEFINED
#include <stdbool.h>
double tetrahedron_volume(double A[3], double B[3], double C[3], double D[3]);
bool in_box3(double xb[6], double v[3]);
double distance3(double v1[3], double v2[3]);
void get_verttype(int nverts, int *triangles, int ntriangles, int *vert_type);
void average_verts2(double v1[3], int v1type, double v2[3], int v2type,
                    double mesh_bounds[6], double *vavg);
void average_verts3(double v1[3], int v1type, double v2[3], int v2type,
                    double v3[3], int v3type, double mesh_bounds[6],
                    double *vavg);
void decimate(double *VERTS, int nverts, int *FACES, int NFACES,
              double mesh_bounds[6], double DELTA);
#endif
