#include "options.h"
#include "gsmv.h"
#include "MALLOCC.h"
#include <stdbool.h>
#include "datadefs.h"
#include <string.h>
#include <math.h>

// !  +++++++++++++++++++++++ BOXTETRA_ROUTINES ++++++++++++++++++++++++++

// INTEGER, DIMENSION(0:3,0:5), TARGET :: BOX_PLANE2VERT

// INTEGER, DIMENSION(0:3,0:5), TARGET :: BOX_PLANE2EDGE

// INTEGER, DIMENSION(0:1,0:11), TARGET :: BOX_EDGE2VERT

// INTEGER, DIMENSION(0:2,0:3), TARGET :: TETRA_PLANE2VERT

// INTEGER, DIMENSION(0:2,0:3), TARGET :: TETRA_PLANE2EDGE

// INTEGER, DIMENSION(0:1,0:5) :: TETRA_EDGE2VERT

// INTEGER, PARAMETER :: MIN_X=0, MAX_X=1, MIN_Y=2, MAX_Y=3, MIN_Z=4, MAX_Z=5

// INTEGER :: I, J

// !       6-----------7
// !      /.          /|
// !    /  .        /  |
// !   4-----------5   |
// !   |   .       |   |
// !   |   .       |   |
// !   |   2.......|...3
// !   |  .        |  /
// !   | .         | /
// !   |.          |/
// !   0-----------1

// ! BOX_PLANE2VERT(edge,plane)
// DATA ( (BOX_PLANE2VERT(I,J), I=0,3),J=0,5) /&
//   0,2,4,6,&
//   1,3,5,7,&
//   0,1,4,5,&
//   2,3,6,7,&
//   0,1,2,3,&
//   4,5,6,7 &
//   /

// !       ------7------
// !      /.           /
// !     2 .         3 |
// !    /  .        /  |
// !   ------6------   |
// !   |  10       |  11
// !   |   .       |   |
// !   |   .....5..|...|
// !   8  .        9  /
// !   | 0         | 1
// !   |.          |/
// !   |----4------|

// DATA ( (BOX_PLANE2EDGE(I,J), I=0,3),J=0,5) /&
//   0,2,8,10,&
//   1,3,9,11,&
//   4,6,8,9,&
//   5,7,10,11,&
//   0,1,4,5,&
//   2,3,6,7 &
//   /

// !       6-----7-----7
// !      /.           /
// !     2 .         3 |
// !    /  .        /  |
// !   4-----6-----5   |
// !   |  10       |  11
// !   |   .       |   |
// !   |   2....5..|...3
// !   8  .        9  /
// !   | 0         | 1
// !   |.          |/
// !   0----4------1
// ! planes: 0-left 1-right 2-front 3-back 4-bottom 5-top
// ! edges: 0-bottom left  1-bottom right 2-top left   3-top right
// !        4-bottom front 5-bottom back  6-top front  7-top back
// !        8-front left   9-front right 10-back left 11-back right
// ! vertices: 0-bottom left front 1-bottom right front 2-bottom left back
// 3-bottom right back !           4-top left front    5-top right front 6-top
// left back    7-top right back

//  DATA ( (BOX_EDGE2VERT(I,J), I=0,1), J=0,11) /&
//   0,2,  1,3,  4,6,  5,7,&
//   0,1,  2,3,  4,5,  6,7,&
//   0,4,  1,5,  2,6,  3,7&
//   /

// !           3
// !          /.\
// !         / . \
// !        /  5  \
// !       /   .   \
// !      3    2    4
// !     /   .   .   \
// !    /  2       1  \
// !   / .           . \
// !  0-------0---------1

// DATA ( (TETRA_PLANE2VERT(I,J), I=0,2),J=0,3) /&
//   0,3,1,&
//   1,3,2,&
//   0,2,3,&
//   0,1,2&
//   /

// DATA ( (TETRA_PLANE2EDGE(I,J), I=0,2),J=0,3) /&
//   0,3,4,&
//   1,4,5,&
//   2,5,3,& !double check (was 2 3 5)
//   0,1,2&
//   /

//  DATA ( (TETRA_EDGE2VERT(I,J), I=0,1), J=0,5) /&
//   0,1,  1,2,  2,0,&
//   0,3,  1,3,  2,3&
//   /

// !  ------------------ TETRAHEDRON_VOLUME ------------------------

// !              D1
// !             /|\
// !            / | \
// !           /  |  \
// !          /   |   \
// !         /    |    \
// !        /     B4    \
// !       /     . .     \
// !      /     .    .    \
// !     /    .        .   \
// !    /   .            .  \
// !   /  .               .  \
// !  / .                    .\
// ! C2------------------------A3

double tetrahedron_volume(double A[3], double B[3], double C[3], double D[3]) {

  // ! determine the volume of a tetrahedron formed from vertices A, B, C and D

  // REAL(EB), DIMENSION(0:2), INTENT(IN) :: A, B, C, D
  // REAL(EB), DIMENSION(0:2) :: AMC, BMC, DMC, ACROSSB
  double AMC[3];
  double BMC[3];
  double DMC[3];
  double ACROSSB[3];

  AMC[0] = A[0] - C[0];
  AMC[1] = A[1] - C[1];
  AMC[2] = A[2] - C[2];

  BMC[0] = B[0] - C[0];
  BMC[1] = B[1] - C[1];
  BMC[2] = B[2] - C[2];

  DMC[0] = D[0] - C[0];
  DMC[1] = D[1] - C[1];
  DMC[2] = D[2] - C[2];

  CROSS(ACROSSB, AMC, BMC);
  return DOT3(ACROSSB, DMC) / 6.0;
}

// !  ------------------ IN_BOX3 ------------------------

bool in_box3(double xb[6], double v[3]) {
  if (xb[0] <= v[0] && v[0] <= xb[1] && xb[2] <= v[1] && v[1] <= xb[3] &&
      xb[4] <= v[2] && v[2] <= xb[5]) {
    return true;
  }
  return false;
}

// !  ------------------ distance3 ------------------------

double distance3(double v1[3], double v2[3]) {
  double dx = v1[0] - v2[0];
  double dy = v1[1] - v2[1];
  double dz = v1[2] - v2[2];
  return sqrt(dx * dx + dy * dy + dz * dz);
}
// !  ------------------ GET_VERTTYPE ------------------------

void get_verttype(int nverts, int *triangles, int ntriangles, int *vert_type) {
  // INTEGER, INTENT(IN) :: nverts, ntriangles

  // INTEGER, INTENT(OUT), DIMENSION(nverts) :: vert_type

  // INTEGER, INTENT(IN), DIMENSION(3*ntriangles), TARGET :: triangles

  // ! classify each vertex in a geometry as either interior or exterior
  // ! a vertex VI is interior if the vertices connected to I form a cycle or
  // loop ! ie VI is connected to v1, v2, v3 and v1 -> v2 -> v3 -> v1 ! if they
  // don't form a loop then it is an exterior vertex

  // ! exterior vertices (connected to a blockage or mesh boundary) won't be
  // moved or deleted

  // INTEGER, DIMENSION(nverts) :: vert_count, triangle_count

  // INTEGER :: I, J, K, vertj_index, TRIJ_INDEX, maxcount, vertk_index

  // INTEGER, POINTER, DIMENSION(:) :: trii
  int *trii;

  // INTEGER, DIMENSION(:,:), ALLOCATABLE :: vert_trilist

  // ! count number of triangles connected to each vertex
  int *triangle_count;
  NewMemory((void **)&triangle_count, nverts);
  memset(triangle_count, '\0', nverts * sizeof(*triangle_count));
  for (int i = 0; i < ntriangles; i++) {
    trii = &triangles[3 * i];
    for (int j = 0; j < 3; i++) {
      int vertj_index = trii[j];
      if (vertj_index >= 1 && vertj_index <= nverts)
        triangle_count[vertj_index]++;
    }
  }

  int maxcount = triangle_count[0];
  for (int i = 1; i < nverts; i++) {
    maxcount = MAX(maxcount, triangle_count[i]);
  }
  FreeMemory(triangle_count);

  // ! construct a list of triangles connected to each vertex
  // ! vert_trilist(I,1) contains number of triangles connected to vertex I
  // ! vert_trilist(I,2-> ) contains the triangle indices

  // ALLOCATE(vert_trilist(nverts,maxcount+1));
  int *vert_trilist;
  NewMemory((void **)&vert_trilist, nverts * (maxcount + 1));
  memset(vert_trilist, '\0', nverts * (maxcount + 1) * sizeof(*vert_trilist));

  for (int i = 0; i < ntriangles; i++) {
    trii = &triangles[3 * i];
    for (int j = 0; j < 3; j++) {
      int vertj_index = trii[j];
      if (vertj_index >= 1 && vertj_index <= nverts) {
        vert_trilist[vertj_index * nverts +
                     1]++; // bump triangle count by 1 for vertex j
        int k = vert_trilist[vertj_index * nverts + 1] + 1;
        vert_trilist[vertj_index * nverts + k] =
            i; // put triangle index into triangle count + 1
      }
    }
  }

  // int *vert_type;
  // NewMemory((void **)&vert_type,nverts);
  memset(vert_type, 1, nverts * sizeof(*vert_type));

  int *vert_count;
  NewMemory((void **)&vert_count, nverts);
  memset(vert_count, '\0', nverts * sizeof(*vert_count));

  // ! count vertices connected to each vertex
  // vert_type[1:nverts] = 1;
  for (int i = 1; i < nverts; i++) {
    // vert_count(1:nverts) = 0;
    memset(vert_count, 0, nverts * sizeof(*vert_count));
    for (int j = 1; j <= vert_trilist[i * nverts + 1];
         j++) { // loop over triangles connected to vertex I
      int trij_index = vert_trilist[i * nverts + j];
      for (int k = 1; k <= 3; k++) { // loop over vertices of triangle J
        int vertk_index = triangles[3 * trij_index - 3 + k];
        if (vertk_index != i)
          vert_count[vertk_index]++;
      }
    }
    for (int j = 1; j < nverts; j++) {
      if (vert_count[j] ==
          1) { // consider all vertices that are connected to vertex I
        vert_type[i] = 0; // if all of these neighbors have two neighbors among
                          // this set then I is interior
        goto OUTER; // if at least one of these neighbors has only one neigbor
                    // among this set then I is on the exterior
      }
    }
  OUTER:;
  }

  FreeMemory(vert_trilist);
  FreeMemory(vert_count);
}

// !  ------------------ average_verts2 ------------------------

void average_verts2(double v1[3], int v1type, double v2[3], int v2type,
                    double mesh_bounds[6], double *vavg) {
  // REAL(EB), DIMENSION(3), INTENT(IN) :: V1, V2

  // INTEGER, INTENT(IN) :: V1TYPE, V2TYPE

  // REAL(EB), DIMENSION(6), INTENT(IN) :: mesh_bounds

  // REAL(EB), DIMENSION(3), INTENT(OUT) :: vavg

  // REAL(EB) :: BOXEPS

  double BOXEPS = 0.001;

  if (v1type == 0) {
    vavg[0] = v1[0];
    vavg[1] = v1[1];
    vavg[2] = v1[2];
    return;
  }

  if (v2type == 0) {
    vavg[0] = v2[0];
    vavg[1] = v2[1];
    vavg[2] = v2[2];
    return;
  }

  if (fabs(v1[0] - mesh_bounds[0]) < BOXEPS ||
      fabs(v1[0] - mesh_bounds[1]) < BOXEPS) {
    vavg[0] = v1[0];
  } else if (fabs(v2[0] - mesh_bounds[0]) < BOXEPS ||
             fabs(v2[0] - mesh_bounds[1]) < BOXEPS) {
    vavg[0] = v2[0];
  } else {
    vavg[0] = (v1[0] + v2[0]) / 2.0;
  }

  if (fabs(v1[1] - mesh_bounds[2]) < BOXEPS ||
      fabs(v1[1] - mesh_bounds[4 - 1]) < BOXEPS) {
    vavg[1] = v1[1];
  } else if (fabs(v2[1] - mesh_bounds[2]) < BOXEPS ||
             fabs(v2[1] - mesh_bounds[4 - 1]) < BOXEPS) {
    vavg[1] = v2[1];
  } else {
    vavg[1] = (v1[1] + v2[1]) / 2.0;
  }

  if (fabs(v1[2] - mesh_bounds[5 - 1]) < BOXEPS ||
      fabs(v1[2] - mesh_bounds[6 - 1]) < BOXEPS) {
    vavg[2] = v1[2];
  } else if (fabs(v2[2] - mesh_bounds[5 - 1]) < BOXEPS ||
             fabs(v2[2] - mesh_bounds[6 - 1]) < BOXEPS) {
    vavg[2] = v2[2];
  } else {
    vavg[2] = (v1[2] + v2[2]) / 2.0;
  }
}

// !  ------------------ average_verts3 ------------------------

void average_verts3(double v1[3], int v1type, double v2[3], int v2type,
                    double v3[3], int v3type, double mesh_bounds[6],
                    double *vavg) {
  // REAL(EB), DIMENSION(3), INTENT(IN) :: V1, V2, V3

  // INTEGER, INTENT(IN) :: V1TYPE, V2TYPE, V3TYPE

  // REAL(EB), DIMENSION(6), INTENT(IN) :: mesh_bounds

  // REAL(EB), DIMENSION(3), INTENT(OUT) :: vavg

  // REAL(EB) :: BOXEPS

  double BOXEPS = 0.001;

  if (v1type == 0) {
    vavg[0] = v1[0];
    vavg[1] = v1[1];
    vavg[2] = v1[2];
    return;
  }

  if (v2type == 0) {
    vavg[0] = v2[0];
    vavg[1] = v2[1];
    vavg[2] = v2[2];
    return;
  }

  if (v3type == 0) {
    vavg[0] = v3[0];
    vavg[1] = v3[1];
    vavg[2] = v3[2];
    return;
  }

  if (fabs(v1[0] - mesh_bounds[0]) < BOXEPS ||
      fabs(v1[0] - mesh_bounds[1]) < BOXEPS) {
    vavg[0] = v1[0];
  } else if (fabs(v2[0] - mesh_bounds[0]) < BOXEPS ||
             fabs(v2[0] - mesh_bounds[1]) < BOXEPS) {
    vavg[0] = v2[0];
  } else if (fabs(v3[0] - mesh_bounds[0]) < BOXEPS ||
             fabs(v3[0] - mesh_bounds[1]) < BOXEPS) {
    vavg[0] = v3[0];
  } else {
    vavg[0] = (v1[0] + v2[0] + v3[0]) / 3.0;
  }

  if (fabs(v1[1] - mesh_bounds[2]) < BOXEPS ||
      fabs(v1[1] - mesh_bounds[4 - 1]) < BOXEPS) {
    vavg[1] = v1[1];
  } else if (fabs(v2[1] - mesh_bounds[2]) < BOXEPS ||
             fabs(v2[1] - mesh_bounds[4 - 1]) < BOXEPS) {
    vavg[1] = v2[1];
  } else if (fabs(v3[1] - mesh_bounds[2]) < BOXEPS ||
             fabs(v3[1] - mesh_bounds[4 - 1]) < BOXEPS) {
    vavg[1] = v3[1];
  } else {
    vavg[1] = (v1[1] + v2[1] + v3[1]) / 3.0;
  }

  if (fabs(v1[2] - mesh_bounds[5 - 1]) < BOXEPS ||
      fabs(v1[2] - mesh_bounds[6 - 1]) < BOXEPS) {
    vavg[2] = v1[2];
  } else if (fabs(v2[2] - mesh_bounds[5 - 1]) < BOXEPS ||
             fabs(v2[2] - mesh_bounds[6 - 1]) < BOXEPS) {
    vavg[2] = v2[2];
  } else if (fabs(v3[2] - mesh_bounds[5 - 1]) < BOXEPS ||
             fabs(v3[2] - mesh_bounds[6 - 1]) < BOXEPS) {
    vavg[2] = v3[2];
  } else {
    vavg[2] = (v1[2] + v2[2] + v3[2]) / 3.0;
  }
}

// !  ------------------ DECIMATE ------------------------

void decimate(double *VERTS, int nverts, int *FACES, int NFACES,
              double mesh_bounds[6], double DELTA) {
  // This routine reduces the size of a geometry by
  //  1) merging vertices that are "close" together
  //  2) eliminating redundent vertices
  //  3) eliminating "singular" triangles
#define V_MERGED -1
#define V_DISCARD 0
#define V_ORIGINAL 1

  int *vert_state;
  NewMemory((void **)&vert_state, nverts);
  int *vert_map;
  NewMemory((void **)&vert_map, nverts);
  int *vert_type;
  NewMemory((void **)&vert_type, nverts);

  double *V1;
  double *V2;
  double *V3;
  double *VERTFROM;
  double *VERTTO;

  // INTEGER, POINTER, DIMENSION(:) :: TRI_I, tri_from, tri_to
  int *tri_from;
  int *tri_to;
  // INTEGER, DIMENSION(3) :: tri_new
  int tri_new[3];
  // REAL(EB) :: D12, D13, D23

  // INTEGER :: I, IFROM, ITO, iter, max_iter
  int ITO;

  // REAL(EB), DIMENSION(3) :: vavg
  double vavg[3];

  // LOGICAL :: have_small
  bool have_small = true;
  int max_iter = 4;
  int iter = 0;
  while (have_small &&
         iter < max_iter) { // iterate until no further changes are made (or 10
                            // times whichever comes first)
    have_small = false;

    // ! vert_state
    // !    V_MERGE =   -1  -  merged vertex
    // !    V_DISCARD =  0  -  discard vertex
    // !    V_ORIGINAL = 1  -  vertex kept and not changed

    //  vert_state(1:nverts) = V_ORIGINAL;
    memset(vert_state, V_ORIGINAL, nverts * sizeof(*vert_state));

    for (int i = 0; i < nverts; i++) {
      vert_map[i] = i;
    }

    iter++;

    get_verttype(nverts, FACES, NFACES, vert_type);

    // ! combine vertices that are close together

    for (int i = 0; i < NFACES; i++) {
      int *tri_i = &FACES[3 * i];
      V1 = &VERTS[3 * tri_i[0]];
      V2 = &VERTS[3 * tri_i[1]];
      V3 = &VERTS[3 * tri_i[2]];

      if (vert_state[tri_i[0]] !=
              V_ORIGINAL || // only look at triangles that have not changed
          vert_state[tri_i[1]] != V_ORIGINAL ||
          vert_state[tri_i[2]] != V_ORIGINAL)
        goto TRI_LOOP;

      double D12 = distance3(V1, V2);
      double D13 = distance3(V1, V3);
      double D23 = distance3(V2, V3);
      if (D12 > DELTA && D13 > DELTA && D23 > DELTA)
        goto TRI_LOOP; // triangle too large, do not combine verts

      have_small = true;
      if (D12 < DELTA && D13 > DELTA &&
          D23 > DELTA) { // combine verts 1 and 2 leave 3 alone
        vert_state[tri_i[0]] = V_MERGED;
        vert_state[tri_i[1]] = V_DISCARD;
        average_verts2(V1, vert_type[tri_i[0]], V2, vert_type[tri_i[1]],
                       mesh_bounds, vavg);
        V1[0] = vavg[0];
        V1[1] = vavg[1];
        V1[2] = vavg[2];
        vert_map[tri_i[1]] = tri_i[0];
        tri_i[1] = tri_i[0];
      } else if (D13 < DELTA && D12 > DELTA &&
                 D23 > DELTA) { // combine verts 1 and 3
        vert_state[tri_i[0]] = V_MERGED;
        vert_state[tri_i[2]] = V_DISCARD;
        average_verts2(V1, vert_type[tri_i[0]], V3, vert_type[tri_i[2]],
                       mesh_bounds, vavg);
        V1[0] = vavg[0];
        V1[1] = vavg[1];
        V1[2] = vavg[2];
        vert_map[tri_i[2]] = tri_i[0];
        tri_i[2] = tri_i[0];
      } else if (D23 < DELTA && D12 > DELTA &&
                 D13 > DELTA) { // combine verts 2 and 3
        vert_state[tri_i[1]] = V_MERGED;
        vert_state[tri_i[2]] = V_DISCARD;
        average_verts2(V2, vert_type[tri_i[1]], V3, vert_type[tri_i[2]],
                       mesh_bounds, vavg);
        V2[0] = vavg[0];
        V2[1] = vavg[1];
        V2[2] = vavg[2];
        vert_map[tri_i[2]] = tri_i[1];
        tri_i[2] = tri_i[1];
      } else { // combine verts 1, 2 and 3
        vert_state[tri_i[0]] = V_MERGED;
        vert_state[tri_i[1]] = V_DISCARD;
        vert_state[tri_i[2]] = V_DISCARD;
        average_verts3(V1, vert_type[tri_i[0]], V2, vert_type[tri_i[1]], V3,
                       vert_type[tri_i[2]], mesh_bounds, vavg);
        V1[0] = vavg[0];
        V1[1] = vavg[1];
        V1[2] = vavg[2];
        vert_map[tri_i[1]] = tri_i[0];
        vert_map[tri_i[2]] = tri_i[0];
        tri_i[1] = tri_i[0];
        tri_i[2] = tri_i[0];
      }
    TRI_LOOP:;
    }

    // ! remap triangle vertices

    for (int i = 0; i < NFACES; i++) {
      int *tri_i = &FACES[3 * i];
      tri_i[0] = vert_map[tri_i[0]];
      tri_i[1] = vert_map[tri_i[1]];
      tri_i[2] = vert_map[tri_i[2]];
    }

    // ! construct new vertex list skipping over vertices that have been removed

    ITO = 0;
    for (int IFROM = 0; IFROM < nverts; IFROM++) {
      if (vert_state[IFROM] != V_DISCARD) {
        ITO++;
        VERTFROM = &VERTS[3 * IFROM];
        VERTTO = &VERTS[3 * ITO];
        VERTTO[0] = VERTFROM[0];
        VERTTO[1] = VERTFROM[1];
        VERTTO[2] = VERTFROM[2];
      }
      vert_map[IFROM] = ITO;
    }
    nverts = ITO;

    // ! eliminate singular triangles (as a result of merged vertices)

    ITO = 0;
    for (int IFROM = 0; IFROM < NFACES; IFROM++) {
      tri_from = &FACES[3 * IFROM];
      if (tri_from[0] != tri_from[1] && tri_from[0] != tri_from[2] &&
          tri_from[1] != tri_from[2]) {
        tri_new[0] = vert_map[tri_from[0]];
        tri_new[1] = vert_map[tri_from[1]];
        tri_new[2] = vert_map[tri_from[2]];
        if (tri_new[0] != 0 && tri_new[1] != 0 && tri_new[2] != 0) {
          ITO = ITO + 1;
          tri_to = &FACES[3 * ITO];
          tri_to[0] = tri_new[0];
          tri_to[1] = tri_new[1];
          tri_to[2] = tri_new[2];
        }
      }
    }
    NFACES = ITO;
  }
  FreeMemory(&vert_state);
}
