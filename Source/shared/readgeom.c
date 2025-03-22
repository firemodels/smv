#include "options.h"
#include "dmalloc.h"
#include "string_util.h"
#include <math.h>

#include <string.h>
#include "smokeviewdefs.h"
#include "isobox.h"
#include "histogram.h"
#include "datadefs.h"
#include "readgeom.h"
#include "stdio_m.h"
#include <assert.h>

/* ------------------ GetInterval ------------------------ */

int GetInterval(float val, float *array, int n) {
  int low, mid, high;

  if(val < array[0]) return -1;
  if(val > array[n - 1]) return -1;

  low = 0;
  high = n - 1;
  while(high - low > 1) {
    mid = (low + high) / 2;
    if(val >= array[mid]) {
      low = mid;
    }
    else {
      high = mid;
    }
  }
  assert(low < n);
  return low;
}

// !  ------------------ Dist ------------------------

float Dist(float v1[3], float v2[3]){
  float dx = v1[0] - v2[0];
  float dy = v1[1] - v2[1];
  float dz = v1[2] - v2[2];
  return sqrt(dx * dx + dy * dy + dz * dz);
}

/* ------------------ InitGeom ------------------------ */

void InitGeom(geomdata *geomi, int geomtype, int fdsblock,
              int have_cface_normals_arg, int block_number){
  geomi->block_number = block_number;
  geomi->file = NULL;
  geomi->topo_file = NULL;
  geomi->cache_defined = 0;
  geomi->display = 0;
  geomi->loaded = 0;
  geomi->geomlistinfo_0 = NULL;
  geomi->surfgeom = NULL;
  geomi->geomlistinfo = NULL;
  geomi->currentframe = NULL;
  geomi->times = NULL;
  geomi->ntimes = 0;
  geomi->times = NULL;
  geomi->timeslist = NULL;
  geomi->float_vals = NULL;
  geomi->int_vals = NULL;
  geomi->nfloat_vals = 0;
  geomi->nint_vals = 0;
  geomi->geomtype = geomtype;
  geomi->fdsblock = fdsblock;
  geomi->is_terrain = 0;
  geomi->file2_tris = NULL;
  geomi->nfile2_tris = 0;
  geomi->have_cface_normals = have_cface_normals_arg;
  geomi->ncface_normals = 0;
  geomi->cface_normals = NULL;
#ifdef pp_ISOFRAME
  geomi->frameinfo = NULL;
#endif
}

/* ------------------ RotateU2V ------------------------ */

void RotateU2V(float *u, float *v, float *axis, float *angle){
  float sum, cosangle, normu, normv;

  /*
  i  j  k
  ux uy uz
  vx vy vz
  */

  CROSS(axis, u, v);
  sum = NORM3(axis);
  normu = NORM3(u);
  normv = NORM3(v);
  if(sum > 0.0 && normu > 0.0 && normv > 0.0){
    axis[0] /= sum;
    axis[1] /= sum;
    axis[2] /= sum;
    cosangle = CLAMP(DOT3(u, v) / (normu * normv), -1.0, 1.0);
    *angle = acos(cosangle);
  }
  else{
    axis[0] = 0.0;
    axis[1] = 0.0;
    axis[2] = 1.0;
    *angle = 0.0;
  }
}

/* ------------------ AngleAxis2Quat ------------------------ */

void AngleAxis2Quat(float angle, float *axis, float *quat){
  float sum;
  float cosang, sinang;

  // angle is in radians
  // axis is a vector

  sum = sqrt(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);

  if(sum > 0.0){
    cosang = cos(angle / 2.0);
    sinang = sin(angle / 2.0);

    quat[0] = cosang;
    quat[1] = axis[0] * sinang / sum;
    quat[2] = axis[1] * sinang / sum;
    quat[3] = axis[2] * sinang / sum;
  }
  else{
    quat[0] = 1.0;
    quat[1] = 0.0;
    quat[2] = 0.0;
    quat[3] = 0.0;
  }
}

/* ------------------ Quat2Rot------------------ */

void Quat2Rot(float quat[4], float rot[16]){
  float w = 0.0, x = 0.0, y = 0.0, z = 0.0, sum;

  sum = sqrt(quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2] +
             quat[3] * quat[3]);
  if(sum != 0.0){
    w = quat[0] / sum;
    x = quat[1] / sum;
    y = quat[2] / sum;
    z = quat[3] / sum;
  }

  rot[0] = 1.0 - 2.0 * y * y - 2.0 * z * z;
  rot[1] = 2.0 * x * y + 2.0 * w * z;
  rot[2] = 2.0 * x * z - 2.0 * w * y;
  rot[3] = 0.0;

  rot[4] = 2.0 * x * y - 2.0 * w * z;
  rot[5] = 1.0 - 2.0 * x * x - 2.0 * z * z;
  rot[6] = 2.0 * y * z + 2.0 * w * x;
  rot[7] = 0.0;

  rot[8] = 2.0 * x * z + 2.0 * w * y;
  rot[9] = 2.0 * y * z - 2.0 * w * x;
  rot[10] = 1.0 - 2.0 * x * x - 2.0 * y * y;
  rot[11] = 0.0;

  rot[12] = 0.0;
  rot[13] = 0.0;
  rot[14] = 0.0;
  rot[15] = 1.0;
}

/* ------------------ MultQuat ------------------------ */

void MultQuat(float x[4], float y[4], float z[4]){
  float z2[4];

  z2[0] = x[0] * y[0] - x[1] * y[1] - x[2] * y[2] - x[3] * y[3];
  z2[1] = x[0] * y[1] + x[1] * y[0] + x[2] * y[3] - x[3] * y[2];
  z2[2] = x[0] * y[2] - x[1] * y[3] + x[2] * y[0] + x[3] * y[1];
  z2[3] = x[0] * y[3] + x[1] * y[2] - x[2] * y[1] + x[3] * y[0];
  z[0] = z2[0];
  z[1] = z2[1];
  z[2] = z2[2];
  z[3] = z2[3];
}

/* ------------------ CompareTransparentTriangles ------------------------ */

int CompareTransparentTriangles(const void *arg1, const void *arg2){
  tridata *tri, *trj;

  tri = *(tridata **)arg1;
  trj = *(tridata **)arg2;

  if(tri->distance < trj->distance) return 1;
  if(tri->distance > trj->distance) return -1;
  return 0;
}

/* ------------------ XYZ2AzElev ------------------------ */

void XYZ2AzElev(float *xyz, float *azimuth, float *elevation){
  float norm3;

  // x = ||xyz||cos(az)*cos(elev)
  // y = ||xyz||sin(az)*cos(elev)
  // z = ||xyz||sin(elev)
  // elev=asin(z/||xyz||)
  // az=atan(y/x)
  norm3 = NORM3(xyz);
  if(norm3 > 0.00001 && ABS(xyz[2] / norm3) <= 1.0){
    *elevation = RAD2DEG * asin(xyz[2] / norm3);
  }
  else{
    *elevation = 0.0;
  }
  *azimuth = RAD2DEG * atan2(xyz[1], xyz[0]);
}

// !  ------------------ GetVertType ------------------------

void GetVertType(int nverts, int *triangles, int ntriangles, int *vert_type){
  // ! classify each vertex in a geometry as either interior or exterior
  // ! a vertex VI is interior if the vertices connected to I form a cycle or
  // loop ! ie VI is connected to v1, v2, v3 and v1 -> v2 -> v3 -> v1 ! if they
  // don't form a loop then it is an exterior vertex

  // ! exterior vertices (connected to a blockage or mesh boundary) won't be
  // moved or deleted
  int *trii;

  // ! count number of triangles connected to each vertex
  int *tri_count;

  NewMemory((void **)&tri_count, nverts * sizeof(int));
  memset(tri_count, 0, nverts * sizeof(*tri_count));

  int i;
  for(i = 0; i < ntriangles; i++){
    trii = triangles + 3 * i;

    int j;
    for(j = 0; j < 3; j++){
      int vertj_index = trii[j];

      if(vertj_index >= 1 && vertj_index <= nverts) tri_count[vertj_index]++;
    }
  }

  int maxcount = tri_count[0];
  for(i = 1; i < nverts; i++){
    maxcount = MAX(maxcount, tri_count[i]);
  }
  FREEMEMORY(tri_count);

  // ! construct a list of triangles connected to each vertex
  // ! vert_trilist(I,1) contains number of triangles connected to vertex I
  // ! vert_trilist(I,2-> ) contains the triangle indices

  int *vert_trilist;
  NewMemory((void **)&vert_trilist, nverts * (maxcount + 1) * sizeof(int));
  memset(vert_trilist, 0, nverts * (maxcount + 1) * sizeof(*vert_trilist));

  for(i = 0; i < ntriangles; i++){
    trii = &triangles[3 * i];

    int j;
    for(j = 0; j < 3; j++){
      int vertj_index = trii[j];
      if(vertj_index >= 1 && vertj_index <= nverts){
        vert_trilist[vertj_index * nverts +
                     1]++; // bump triangle count by 1 for vertex j
        int k = vert_trilist[vertj_index * nverts + 1] + 1;
        vert_trilist[vertj_index * nverts + k] =
            i; // put triangle index into triangle count + 1
      }
    }
  }

  memset(vert_type, 1, nverts * sizeof(*vert_type));

  int *vert_count;
  NewMemory((void **)&vert_count, nverts * sizeof(int));
  memset(vert_count, 0, nverts * sizeof(*vert_count));

  // ! count vertices connected to each vertex
  for(i = 1; i < nverts; i++){
    memset(vert_count, 0, nverts * sizeof(*vert_count));

    int j;
    for(j = 1; j <= vert_trilist[i * nverts + 1];
         j++){ // loop over triangles connected to vertex I
      int trij_index = vert_trilist[i * nverts + j];

      int k;
      for(k = 1; k <= 3; k++){ // loop over vertices of triangle J
        int vertk_index = triangles[3 * trij_index - 3 + k];
        if(vertk_index != i) vert_count[vertk_index]++;
      }
    }
    for(j = 1; j < nverts; j++){
      if(vert_count[j] ==
          1){ // consider all vertices that are connected to vertex I
        vert_type[i] = 0; // if all of these neighbors have two neighbors among
                          // this set then I is interior
        break; // if at least one of these neighbors has only one neigbor
               // among this set then I is on the exterior
      }
    }
  }

  FREEMEMORY(vert_trilist);
  FREEMEMORY(vert_count);
}

// !  ------------------ AverageVerts2 ------------------------

void AverageVerts2(float v1[3], int v1type, float v2[3], int v2type,
                   float mesh_bounds[6], float *vavg){
  float BOXEPS = 0.001;

  if(v1type == 0){
    vavg[0] = v1[0];
    vavg[1] = v1[1];
    vavg[2] = v1[2];
    return;
  }

  if(v2type == 0){
    vavg[0] = v2[0];
    vavg[1] = v2[1];
    vavg[2] = v2[2];
    return;
  }

  if(fabs(v1[0] - mesh_bounds[0]) < BOXEPS ||
      fabs(v1[0] - mesh_bounds[1]) < BOXEPS){
    vavg[0] = v1[0];
  }
  else if(fabs(v2[0] - mesh_bounds[0]) < BOXEPS ||
           fabs(v2[0] - mesh_bounds[1]) < BOXEPS){
    vavg[0] = v2[0];
  }
  else{
    vavg[0] = (v1[0] + v2[0]) / 2.0;
  }

  if(fabs(v1[1] - mesh_bounds[2]) < BOXEPS ||
      fabs(v1[1] - mesh_bounds[4 - 1]) < BOXEPS){
    vavg[1] = v1[1];
  }
  else if(fabs(v2[1] - mesh_bounds[2]) < BOXEPS ||
           fabs(v2[1] - mesh_bounds[4 - 1]) < BOXEPS){
    vavg[1] = v2[1];
  }
  else{
    vavg[1] = (v1[1] + v2[1]) / 2.0;
  }

  if(fabs(v1[2] - mesh_bounds[5 - 1]) < BOXEPS ||
      fabs(v1[2] - mesh_bounds[6 - 1]) < BOXEPS){
    vavg[2] = v1[2];
  }
  else if(fabs(v2[2] - mesh_bounds[5 - 1]) < BOXEPS ||
           fabs(v2[2] - mesh_bounds[6 - 1]) < BOXEPS){
    vavg[2] = v2[2];
  }
  else{
    vavg[2] = (v1[2] + v2[2]) / 2.0;
  }
}

// !  ------------------ AverageVerts3 ------------------------

void AverageVerts3(float v1[3], int v1type, float v2[3], int v2type,
                   float v3[3], int v3type, float mesh_bounds[6], float *vavg){
  float BOXEPS = 0.001;

  if(v1type == 0){
    vavg[0] = v1[0];
    vavg[1] = v1[1];
    vavg[2] = v1[2];
    return;
  }

  if(v2type == 0){
    vavg[0] = v2[0];
    vavg[1] = v2[1];
    vavg[2] = v2[2];
    return;
  }

  if(v3type == 0){
    vavg[0] = v3[0];
    vavg[1] = v3[1];
    vavg[2] = v3[2];
    return;
  }

  if(fabs(v1[0] - mesh_bounds[0]) < BOXEPS ||
      fabs(v1[0] - mesh_bounds[1]) < BOXEPS){
    vavg[0] = v1[0];
  }
  else if(fabs(v2[0] - mesh_bounds[0]) < BOXEPS ||
           fabs(v2[0] - mesh_bounds[1]) < BOXEPS){
    vavg[0] = v2[0];
  }
  else if(fabs(v3[0] - mesh_bounds[0]) < BOXEPS ||
           fabs(v3[0] - mesh_bounds[1]) < BOXEPS){
    vavg[0] = v3[0];
  }
  else{
    vavg[0] = (v1[0] + v2[0] + v3[0]) / 3.0;
  }

  if(fabs(v1[1] - mesh_bounds[2]) < BOXEPS ||
      fabs(v1[1] - mesh_bounds[4 - 1]) < BOXEPS){
    vavg[1] = v1[1];
  }
  else if(fabs(v2[1] - mesh_bounds[2]) < BOXEPS ||
           fabs(v2[1] - mesh_bounds[4 - 1]) < BOXEPS){
    vavg[1] = v2[1];
  }
  else if(fabs(v3[1] - mesh_bounds[2]) < BOXEPS ||
           fabs(v3[1] - mesh_bounds[4 - 1]) < BOXEPS){
    vavg[1] = v3[1];
  }
  else{
    vavg[1] = (v1[1] + v2[1] + v3[1]) / 3.0;
  }

  if(fabs(v1[2] - mesh_bounds[5 - 1]) < BOXEPS ||
      fabs(v1[2] - mesh_bounds[6 - 1]) < BOXEPS){
    vavg[2] = v1[2];
  }
  else if(fabs(v2[2] - mesh_bounds[5 - 1]) < BOXEPS ||
           fabs(v2[2] - mesh_bounds[6 - 1]) < BOXEPS){
    vavg[2] = v2[2];
  }
  else if(fabs(v3[2] - mesh_bounds[5 - 1]) < BOXEPS ||
           fabs(v3[2] - mesh_bounds[6 - 1]) < BOXEPS){
    vavg[2] = v3[2];
  }
  else{
    vavg[2] = (v1[2] + v2[2] + v3[2]) / 3.0;
  }
}

#ifdef pp_DECIMATE
#define IJNODE(i, j) ((j) * nx + (i))

// !  ------------------ PtInTriangle ------------------------

int PtInTriangle(float *xy, float *v0, float *v1, float *v2, float *zval){
  float l[3];
  float denom;

  // (   1   1   1 ) ( l0 )    ( 1 )
  // ( v00 v10 v20 ) ( l1 ) =  ( x )
  // ( v01 v11 v21 ) ( l2 )    ( y )

  //      |   1   1   1 |
  // l0 = |   x v10 v20 | / denom
  //      |   y v11 v21 |

  //      |   1   1   1 |
  // l1 = |   v00 x v20 | / denom
  //      |   v01 y v21 |

  //      |   1   1   1 |
  // l2 = |   v00 v10 x | / denom
  //      |   v01 v11 y |

  denom = (v1[0] * v2[1] - v1[1] * v2[0]) - (v0[0] * v2[1] - v0[1] * v2[0]) +
          (v0[0] * v1[1] - v0[1] * v1[0]);
  if(denom == 0.0) return 0;

  l[0] = (v1[0] * v2[1] - v1[1] * v2[0]) - (xy[0] * v2[1] - xy[1] * v2[0]) +
         (xy[0] * v1[1] - xy[1] * v1[0]);
  l[0] /= denom;
  if(ABS(l[0]) > 1.0) return 0;

  l[1] = (xy[0] * v2[1] - xy[1] * v2[0]) - (v0[0] * v2[1] - v0[1] * v2[0]) +
         (v0[0] * xy[1] - v0[1] * xy[0]);
  l[1] /= denom;
  if(ABS(l[1]) > 1.0) return 0;

  l[2] = (v1[0] * xy[1] - v1[1] * xy[0]) - (v0[0] * xy[1] - v0[1] * xy[0]) +
         (v0[0] * v1[1] - v0[1] * v1[0]);
  l[2] /= denom;
  if(ABS(l[2]) > 1.0) return 0;

  *zval = l[0] * v0[2] + l[1] * v1[2] + l[2] * v2[2];
  return 1;
}

/* ------------------ DecimateTerrain ------------------------ */

void DecimateTerrain(vertdata *verts, int nverts, tridata *triangles,
                     int ntriangles, vertdata **verts_new, int *nverts_new,
                     tridata **triangles_new, int *ntriangles_new,
                     float *boxmin, float *boxmax, int nx, int ny){
  int i, j, nvnew, ntrinew;
  int *tri_new;
  float dx, dy;
  vertdata *vertnewptr;
  tridata *trinewptr;

  nvnew = nx * ny;
  NewMemory((void **)&vertnewptr, nvnew * sizeof(vertdata));
  NewMemory((void **)&tri_new, nvnew * sizeof(int));
  *nverts_new = nvnew;
  *verts_new = vertnewptr;

  ntrinew = 2 * (nx - 1) * (ny - 1);
  NewMemory((void **)&trinewptr, ntrinew * sizeof(tridata));
  *ntriangles_new = ntrinew;
  *triangles_new = trinewptr;

  for(i = 0; i < nvnew; i++){
    tri_new[i] = -1;
  }
  dx = (boxmax[0] - boxmin[0]) / (float)(nx - 1);
  dy = (boxmax[1] - boxmin[1]) / (float)(ny - 1);
  for(i = 0; i < ntriangles; i++){
    tridata *trii;
    float *v0, *v1, *v2;
    float xtmin, xtmax, ytmin, ytmax;
    int imin, imax, jmin, jmax;
    int ii;
    float xyz[3];

    trii = triangles + i;
    v0 = trii->verts[0]->xyz;
    v1 = trii->verts[1]->xyz;
    v2 = trii->verts[2]->xyz;
    xtmin = MIN(MIN(v0[0], v1[0]), v2[0]);
    xtmax = MAX(MAX(v0[0], v1[0]), v2[0]);
    ytmin = MIN(MIN(v0[1], v1[1]), v2[1]);
    ytmax = MAX(MAX(v0[1], v1[1]), v2[1]);

    imin = CLAMP((xtmin - boxmin[0]) / dx, 0, nx - 1);
    imax = CLAMP((xtmax - boxmin[0]) / dx + 1, 0, nx - 1);
    jmin = CLAMP((ytmin - boxmin[1]) / dy, 0, ny - 1);
    jmax = CLAMP((ytmax - boxmin[1]) / dy + 1, 0, ny - 1);
    for(ii = imin; ii <= imax; ii++){
      int jj, exit_loop;

      xyz[0] = boxmin[0] + (float)ii * dx;

      exit_loop = 0;
      for(jj = jmin; jj <= jmax; jj++){
        float zval;

        xyz[1] = boxmin[1] + (float)jj * dy;

        if(PtInTriangle(xyz, v0, v1, v2, &zval) == 1){
          int index;

          xyz[2] = zval;
          index = IJNODE(ii, jj);
          tri_new[index] = i;
          memcpy(vertnewptr[index].xyz, xyz, 3 * sizeof(float));
          exit_loop = 1;
          break;
        }
        if(exit_loop == 1) break;
      }
    }
  }
  int ij = 0;
  int ncount2 = 0;
  for(j = 0; j < ny - 1; j++){
    for(i = 0; i < nx - 1; i++){
      tridata *tri;
      int ival;

      tri = trinewptr + ij++;
      tri->verts[0] = *verts_new + IJNODE(i, j);
      tri->verts[1] = *verts_new + IJNODE(i + 1, j);
      tri->verts[2] = *verts_new + IJNODE(i, j + 1);
      ival = tri_new[IJNODE(i, j)];
      if(ival < 0){
        ncount2++;
        ival = 0;
      }
      tri->ival = ival;

      tri = trinewptr + ij++;
      tri->verts[0] = *verts_new + IJNODE(i, j + 1);
      tri->verts[1] = *verts_new + IJNODE(i + 1, j);
      tri->verts[2] = *verts_new + IJNODE(i + 1, j + 1);
      ival = tri_new[IJNODE(i, j)];
      if(ival < 0){
        ncount2++;
        ival = 0;
      }
      tri->ival = ival;
    }
  }
  fprintf(stderr, "ncount1=%i ncount2=%i\n", ntriangles, ncount2);
  FREEMEMORY(tri_new);
}

/* ------------------ DecimateAllTerrains ------------------------ */

void DecimateAllTerrains(void){
  int i;

  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    meshi->decimated = 0;
  }
  for(i = 0; i < global_scase.npatchinfo; i++){
    meshdata *meshi;
    patchdata *patchi;
    geomlistdata *geomlisti;
    int nx, ny;
    float *boxmin, *boxmax;

    patchi = global_scase.patchinfo + i;
    if(patchi->loaded == 0 || patchi->display == 0 || patchi->blocknumber < 0)
      continue;
    meshi = meshinfo + patchi->blocknumber;
    if(meshi->decimated == 1) continue;
    if(patchi->global_scase.geominfo == NULL || patchi->global_scase.geominfo->display == 0 ||
        patchi->global_scase.geominfo->loaded == 0)
      continue;

    meshi->decimated = 1;
    geomlisti = patchi->global_scase.geominfo->geomlistinfo - 1;
    boxmin = meshi->boxmin;
    boxmax = meshi->boxmax;
    nx = MAX((boxmax[0] - boxmin[0]) / terrain_decimate_delta + 1, 2);
    ny = MAX((boxmax[1] - boxmin[1]) / terrain_decimate_delta + 1, 2);
    DecimateTerrain(geomlisti->verts, geomlisti->nverts, geomlisti->triangles,
                    geomlisti->ntriangles, &meshi->dec_verts,
                    &meshi->ndec_verts, &meshi->dec_triangles,
                    &meshi->ndec_triangles, boxmin, boxmax, nx, ny);
  }
}
#endif

/* ------------------ GetTriangleNormal ------------------------ */

void GetTriangleNormal(float *v1, float *v2, float *v3, float *normal,
                       float *area){
  double u[3], v[3], normal_local[3];
  float norm;
  int i;

  for(i = 0; i < 3; i++){
    u[i] = (double)v2[i] - (double)v1[i];
    v[i] = (double)v3[i] - (double)v1[i];
  }

  // triangle area = 1/2 * | u x v |
  /*
     i   j  k
     ux uy uz
     vx vy vz
  */
  normal_local[0] = u[1] * v[2] - u[2] * v[1];
  normal_local[1] = u[2] * v[0] - u[0] * v[2];
  normal_local[2] = u[0] * v[1] - u[1] * v[0];
  norm = sqrt((float)(normal_local[0] * normal_local[0] +
                      normal_local[1] * normal_local[1] +
                      normal_local[2] * normal_local[2]));
  if(area != NULL) *area = norm / 2.0;
  if((float)norm == 0.0){
    normal[0] = 0.0;
    normal[1] = 0.0;
    normal[2] = 1.0;
  }
  else{
    normal[0] = (float)normal_local[0] / norm;
    normal[1] = (float)normal_local[1] / norm;
    normal[2] = (float)normal_local[2] / norm;
  }
}
/* ----------------------- CompareVerts ----------------------------- */

int CompareVerts(const void *arg1, const void *arg2){
  vertdata *verti, *vertj;
  float *xyzi, *xyzj;

  verti = (vertdata *)arg1;
  vertj = (vertdata *)arg2;
  xyzi = verti->xyz;
  xyzj = vertj->xyz;

  if(xyzi[0] < xyzj[0]) return -1;
  if(xyzi[0] > xyzj[0]) return 1;
  if(xyzi[1] < xyzj[1]) return -1;
  if(xyzi[1] > xyzj[1]) return 1;
  if(xyzi[2] < xyzj[2]) return -1;
  if(xyzi[2] > xyzj[2]) return 1;
  return 0;
}

/* ------------------ DistXY ------------------------ */

float DistXY(float *x, float *y){
  float r1, r2, r3;

  r1 = x[0] - y[0];
  r2 = x[1] - y[1];
  r3 = x[2] - y[2];
  return sqrt(r1 * r1 + r2 * r2 + r3 * r3);
}

/* ------------------ GetAngle ------------------------ */

float GetAngle(float d1, float d2, float d3){
  float angle_local;
  float arg;
  float denom;

  //         v1
  //        /  \
//       d3   d2
  //      /      \
//    v2---d1---v3

  //       d2^2 + d3^2 - d1^2
  // arg = ------------------
  //           2*d2*d3

  // d2==0.0 ==> d3==d1 ==> arg=0.0
  // d3==0.0 ==> d2==d1 ==> arg=0.0

  denom = 2.0 * d2 * d3;
  if(ABS(denom) > 0.0){
    arg = CLAMP((d2 * d2 + d3 * d3 - d1 * d1) / denom, -1.0, 1.0);
    angle_local = acos(arg) * RAD2DEG;
  }
  else{
    angle_local = acos(0.0) * RAD2DEG;
  }
  return angle_local;
}

/* ------------------ GetMinAngle ------------------------ */

float GetMinAngle(tridata *trii){
  float minangle;
  float d1, d2, d3;
  float *xyz1, *xyz2, *xyz3;
  float angle1, angle2, angle3;

  xyz1 = trii->verts[0]->xyz;
  xyz2 = trii->verts[1]->xyz;
  xyz3 = trii->verts[2]->xyz;
  d1 = DistXY(xyz1, xyz2);
  d2 = DistXY(xyz1, xyz3);
  d3 = DistXY(xyz2, xyz3);
  angle1 = GetAngle(d1, d2, d3);
  angle2 = GetAngle(d2, d1, d3);
  angle3 = GetAngle(d3, d1, d2);
  minangle = angle1;
  if(angle2 < minangle) minangle = angle2;
  if(angle3 < minangle) minangle = angle3;
  return minangle;
}

/* ------------------ InitGeomlist ------------------------ */

void InitGeomlist(geomlistdata *geomlisti){
  geomlisti->verts = NULL;
  geomlisti->vertvals = NULL;
  geomlisti->triangles = NULL;
  geomlisti->triangleptrs = NULL;
  geomlisti->connected_triangles = NULL;
  geomlisti->nverts = 0;
  geomlisti->ntriangles = 0;
  geomlisti->norms_defined = 0;
}

/* ------------------ ReadGeom ------------------------ */

void ReadGeomFile2(geomdata *geomi){
  FILE *stream;
  int ntris, *tris;

  if(geomi->file2 == NULL) return;
  stream = fopen(geomi->file2, "rb");
  if(stream == NULL) return;
  FSEEK(stream, 4, SEEK_CUR);
  fread(&ntris, 4, 1, stream);
  FSEEK(stream, 4, SEEK_CUR);
  if(ntris <= 0){
    fclose(stream);
    return;
  }
  NewMemory((void **)&tris, ntris * sizeof(int));
  FSEEK(stream, 4, SEEK_CUR);
  fread(tris, 4, ntris, stream);
  FSEEK(stream, 4, SEEK_CUR);
  geomi->file2_tris = tris;
  geomi->nfile2_tris = ntris;
  fclose(stream);
}

/* ------------------ ReadGeomHeader0 ------------------------ */

void ReadGeomHeader0(geomdata *geomi, int *geom_frame_index, int *ntimes_local){
  FILE_m *stream;
  int count_read;
  int one=0;
  int nvertfaces[2];
  float times_local[2];
  int nt;
  int version;
  int nfloat_vals, nint_vals;
  int *int_vals;
  float *float_vals;
  int nverts = 0, ntris = 0;

  //***format
  // one  (endian)
  // version  (version=1 for this file type, for isosurfaces)
  // nfloats
  // if(nfloats>0) float_1, ...., float_nfloats
  // nints
  // if(nints>0) int_1, ..., int_nints
  //*** static verts and triangles (verts and triangles that appear at all time
  // steps)
  // nverts, ntris
  // vert_1, ..., vert_nverts   (each vert_i is a triple x_i,y_i,z_i of floats)
  // tri_1a,tri_1b,tri_1c ..., tri_ntrisa,tri_ntrisb,tri_ntrisc
  // surf_1, ..., surf_ntris      (each tri_i is a triple I,J,K triangle int
  // indices )
  //*** for a time step
  // time             (float)
  // nverts, ntris    (int,int)
  // vert_1, ..., vert_nverts   (each vert_i is a triple x_i,y_i,z_i of floats)
  // tri_1a,tri_1b,tri_1c ..., tri_ntrisa,tri_ntrisb,tri_ntrisc
  // surf_1, ..., surf_ntris      (each tri_i is a triple I,J,K triangle int
  // indices )

  stream = fopen_b(geomi->file, NULL, 0, "rb");
  if(stream==NULL){
    *ntimes_local=-1;
    return;
  }
  FORTREAD_m(&one,4,1,stream);
  FORTREAD_m(&version, 4, 1, stream);

  // floating point header

  FORTREAD_m(&nfloat_vals, 4, 1, stream);
  if(nfloat_vals>0){
    NewMemoryMemID((void **)&float_vals,nfloat_vals*sizeof(float),geomi->memory_id);
    FORTREAD_m(float_vals, 4, nfloat_vals, stream);
    geomi->float_vals=float_vals;
    geomi->nfloat_vals=nfloat_vals;
  }

  // integer header

  FORTREAD_m(&nint_vals, 4, 1, stream);
  if(nint_vals>0){
    NewMemoryMemID((void **)&int_vals,nint_vals*sizeof(float),geomi->memory_id);
    FORTREAD_m(int_vals, 4, nint_vals, stream);
    geomi->int_vals=int_vals;
    geomi->nint_vals=nint_vals;
  }

  // static verts

  FORTREAD_m(nvertfaces, 4, 2, stream);
  nverts=nvertfaces[0];
  ntris=nvertfaces[1];

  // static vertices

  if(nverts>0){
    fseek_m(stream,4+3*nverts*4+4,SEEK_CUR);
  }

  // static triangles

  if(ntris>0){
    fseek_m(stream,4+3*ntris*4+4,SEEK_CUR);
    fseek_m(stream,4+ntris*4+4,SEEK_CUR);
  }

  nt=0;
  for(;;){
    FORTREAD_m(times_local, 4, 2, stream);
    if(count_read!=2)break;
    FORTREAD_m(nvertfaces, 4, 2, stream);
    if(count_read!=2)break;
    nverts=nvertfaces[0];
    ntris=nvertfaces[1];

    // dynamic vertices

    if(nverts>0){
      fseek_m(stream,4+3*nverts*4+4,SEEK_CUR);
    }

    // dynamic faces

    if(ntris>0){
      fseek_m(stream,4+3*ntris*4+4,SEEK_CUR);
      fseek_m(stream,4+ntris*4+4,SEEK_CUR);
    }

    if(geom_frame_index == NULL){
      // if(tload_step      >  1 && icount%tload_step != 0)continue;
      // if(use_tload_begin == 1 && times_local[0]     < tload_begin)continue;
      // if(use_tload_end   == 1 && times_local[0]     >tload_end)break;
    }
    nt++;
  }
  *ntimes_local=nt;
  fclose_b(stream);
}

/* ------------------ ReadGeomHeader2 ------------------------ */

void ReadGeomHeader2(geomdata *geomi, int *ntimes_local){
  FILE *stream;
  int one = 0;
  int nvertfacesvolumes[3];
  int nt;
  int returncode = 0;
  int version;
  int nverts = 0, ntris = 0, nvolumes = 0;
  int first_frame_all;
  int header[3];
  float time_local;

  //***format
  // one  (endian)
  // version  (version=1 for this file type - for isosurfaces)
  // dummy,dummy,first_frame_all     (dummy's not used, if first_frame_all=1
  // then verts/tris
  //                                  in first frame are displayed for all
  //                                  frames)
  //*** for each time step:
  // time             (float)
  // nverts, ntris, dummy    (int,int,int)
  // vert_1, ..., vert_nverts   (each vert_i is a triple x_i,y_i,z_i of floats)
  // tri_1a,tri_1b,tri_1c ..., tri_ntrisa,tri_ntrisb,tri_ntrisc
  //*** if cfaces:
  // loc_1, .., loc_ntris
  // surf_1, ..., surf_ntris
  // geom_1, ..., geom_ntris
  //*** if not cfaces:
  // surf_1, ..., surf_ntris
  // texture_1, ..., texture_ntris

  stream = fopen(geomi->file, "rb");
  if(stream == NULL){
    *ntimes_local = -1;
    return;
  }
  FSEEK(stream, 4, SEEK_CUR);
  fread(&one, 4, 1, stream);
  FSEEK(stream, 4, SEEK_CUR);
  FORTREAD(&version, 4, 1, stream);

  FORTREAD(header, 4, 3, stream);
  first_frame_all = header[2];

  nt = 0;
  if(first_frame_all == 1) nt = -1;
  for(;;){
    FORTREADBR(&time_local, 1, stream);
    FORTREADBR(nvertfacesvolumes, 3, stream);
    nverts = nvertfacesvolumes[0];
    ntris = nvertfacesvolumes[1];
    nvolumes = nvertfacesvolumes[2];

    // vertices

    if(nverts > 0){
      FSEEK(stream, 4 + 3 * nverts * 4 + 4, SEEK_CUR); // skip vertices
    }

    // faces

    if(ntris > 0){
      int skip = 0;

      skip = 4 + 3 * ntris * 4 + 4; // skip triangles
      if(geomi->geomtype == GEOM_CGEOM){
        skip += 4 + 4 * ntris + 4; // skip locations
        skip += 4 + 4 * ntris + 4; // skip surfs
        skip += 4 + 4 * ntris + 4; // skip geometries
      }
      else{
        skip += 4 + ntris * 4 + 4;     // skip surf
        skip += 4 + 6 * ntris * 4 + 4; // skip textures
      }
      FSEEK(stream, skip, SEEK_CUR);
    }

    // volumes

    if(nvolumes > 0){
      FSEEK(stream, 4 + 4 * nvolumes * 4 + 4, SEEK_CUR); // skip volumes
      FSEEK(stream, 4 + nvolumes * 4 + 4, SEEK_CUR);     // skip matl
    }
    nt++;
  }
  *ntimes_local = nt;
  fclose(stream);
}

/* ------------------ ReadGeomHeader ------------------------ */

void ReadGeomHeader(geomdata *geomi, int *geom_frame_index, int *ntimes_local){
  FILE_m *stream;
  int version, one=0, count_read;

  stream = fopen_b(geomi->file, NULL, 0, "rb");
  if(stream==NULL){
    *ntimes_local=-1;
    return;
  }
  FORTREAD_m(&one,4,1,stream);
  FORTREAD_m(&version, 4, 1, stream);
  if(count_read != 1){
    fclose_b(stream);
    *ntimes_local = -1;
    return;
  }
  fclose_b(stream);

  if(version <= 1){
    ReadGeomHeader0(geomi, geom_frame_index, ntimes_local);
  }
  else{
    ReadGeomHeader2(geomi, ntimes_local);
  }
}

/* ------------------ GetGeomDataHeader ------------------------ */

void GetGeomDataHeader(char *file, int *ntimes_local, int *nvals){
  FILE *stream;
  int one = 1;
  int nface_static, nface_dynamic;
  float time_local;
  int nt, nv;
  int returncode = 0;

  stream = fopen(file, "r");
  if(stream == NULL){
    *ntimes_local = -1;
    return;
  }
  FSEEK(stream, 4, SEEK_CUR);
  fread(&one, 4, 1, stream);
  FSEEK(stream, 4, SEEK_CUR);
  nt = -1;
  nv = 0;
  for(;;){
    FORTREADBR(&time_local, 1, stream);
    FORTREADBR(&nface_static, 1, stream);
    if(nface_static != 0) FSEEK(stream, 4 + nface_static * 4 + 4, SEEK_CUR);
    FORTREADBR(&nface_dynamic, 1, stream);
    if(nface_dynamic != 0) FSEEK(stream, 4 + nface_dynamic * 4 + 4, SEEK_CUR);
    nt++;
    nv += (nface_static + nface_dynamic);
  }
  *ntimes_local = nt;
  *nvals = nv;
  fclose(stream);
}

/* ----------------------- FreeCircle ----------------------------- */

void FreeCircle(circdata *circinfo){
  FREEMEMORY(circinfo->xcirc);
  FREEMEMORY(circinfo->ycirc);
  circinfo->ncirc = 0;
}

/* ----------------------- InitCircle ----------------------------- */

void InitCircle(unsigned int npoints, circdata *circinfo){
  float drad;
  int i;
  float *xcirc, *ycirc;
  int ncirc;

  if(circinfo->ncirc != 0) FreeCircle(circinfo);
  if(npoints < 2) return;
  ncirc = npoints;
  NewMemory((void **)&xcirc, (ncirc + 1) * sizeof(float));
  NewMemory((void **)&ycirc, (ncirc + 1) * sizeof(float));
  drad = 2.0 * PI / (float)ncirc;

  for(i = 0; i < ncirc; i++){
    xcirc[i] = cos(i * drad);
    ycirc[i] = sin(i * drad);
  }
  xcirc[ncirc] = xcirc[0];
  ycirc[ncirc] = ycirc[0];

  circinfo->xcirc = xcirc;
  circinfo->ycirc = ycirc;
  circinfo->ncirc = npoints;
}

/* ------------------ Normalize ------------------------ */

void Normalize(float *xyz, int n){
  float norm,norm2;
  int i;

  norm2 = 0.0;

  for(i=0;i<n;i++){
    norm2 += xyz[i]*xyz[i];
  }
  norm = sqrt(norm2);
  if(norm<0.00001){
    for(i=0;i<n-1;i++){
      xyz[i]=0.0;
    }
    xyz[n-1]=1.0;
  }
  else{
    for(i=0;i<n;i++){
      xyz[i]/=norm;
    }
  }
}
