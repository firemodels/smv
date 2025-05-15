#include "options_common.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "dmalloc.h"
#include "datadefs.h"
#include "histogram.h"
#include "isobox.h"
#include "string_util.h"

#include "file_util.h"
#include "stdio_buffer.h"

#include "readhvac.h"

/* ------------------ hvacval ------------------------ */

static inline int hvacval(hvacdatacollection *hvaccoll, int itime, int iduct,
                          int icell) {
  return (itime)*hvaccoll->hvac_maxcells * hvaccoll->hvac_n_ducts +
         (iduct)*hvaccoll->hvac_maxcells + (icell);
}

/* ------------------ GetHVACDuctID ------------------------ */

hvacductdata *GetHVACDuctID(hvacdatacollection *hvaccoll, char *duct_name) {
  int i;

  for(i = 0; i < hvaccoll->nhvacductinfo; i++) {
    hvacductdata *ducti;

    ducti = hvaccoll->hvacductinfo + i;
    if(strcmp(ducti->duct_name, duct_name) == 0) return ducti;
  }
  return NULL;
}

/* ------------------ GetHVACDuctValIndex ------------------------ */

int GetHVACDuctValIndex(hvacdatacollection *hvaccoll, char *shortlabel) {
  int i;

  for(i = 0; i < hvaccoll->hvacductvalsinfo->n_duct_vars; i++) {
    hvacvaldata *hi;

    hi = hvaccoll->hvacductvalsinfo->duct_vars + i;
    if(strcmp(hi->label.shortlabel, shortlabel) == 0) return i;
  }
  return -1;
}

/* ------------------ GetHVACNodeValIndex ------------------------ */

int GetHVACNodeValIndex(hvacdatacollection *hvaccoll, char *shortlabel) {
  int i;

  for(i = 0; i < hvaccoll->hvacnodevalsinfo->n_node_vars; i++) {
    hvacvaldata *hi;

    hi = hvaccoll->hvacnodevalsinfo->node_vars + i;
    if(strcmp(hi->label.shortlabel, shortlabel) == 0) return i;
  }
  return -1;
}

/* ------------------ GetHVACNodeID ------------------------ */

hvacnodedata *GetHVACNodeID(hvacdatacollection *hvaccoll, char *node_name) {
  int i;

  for(i = 0; i < hvaccoll->nhvacnodeinfo; i++) {
    hvacnodedata *nodei;

    nodei = hvaccoll->hvacnodeinfo + i;
    if(strcmp(nodei->duct_name, node_name) == 0) return nodei;
  }
  return NULL;
}

/* ------------------ InitHvacData ------------------------ */

void InitHvacData(hvacvaldata *hi) {
  hi->vals = NULL;
  hi->nvals = 0;
  hi->vis = 0;
  hi->valmax = 1.0;
  hi->valmin = 0.0;
}

/* ------------------ CompareHvacConnect ------------------------ */

int CompareHvacConnect(const void *arg1, const void *arg2) {
  hvacconnectdata *hi, *hj;
  int indexi, indexj;

  hi = (hvacconnectdata *)arg1;
  hj = (hvacconnectdata *)arg2;
  indexi = hi->index;
  indexj = hj->index;
  if(indexi < indexj) return -1;
  if(indexi > indexj) return 1;
  return 0;
}

/* ------------------ IsHVACVisible ------------------------ */

int IsHVACVisible(hvacdatacollection *hvaccoll) {
  for(int i = 0; i < hvaccoll->nhvacinfo; i++) {
    hvacdata *hvaci = hvaccoll->hvacinfo + i;
    if(hvaci->display == 1) return 1;
  }
  return 0;
}

/* ------------------ HaveHVACConnect ------------------------ */

int HaveHVACConnect(int val, hvacconnectdata *vals, int nvals) {
  int i;

  if(val == -1) return 1;
  for(i = 0; i < nvals; i++) {
    if(val == vals[i].index) return 1;
  }
  return 0;
}

/* ------------------ GetHVACPathXYZ ------------------------ */

void GetHVACPathXYZ(float fraction, float *xyzs, int n, float *xyz) {
  int i;
  float length = 0.0, lengthf;
  float length1, length2;

  if(fraction <= 0.0) {
    memcpy(xyz, xyzs, 3 * sizeof(float));
    return;
  }
  if(fraction >= 1.0) {
    memcpy(xyz, xyzs + 3 * (n - 1), 3 * sizeof(float));
    return;
  }
  for(i = 0; i < n - 1; i++) {
    float *x1, *x2;
    float dx, dy, dz;

    x1 = xyzs + 3 * i;
    x2 = x1 + 3;
    dx = x1[0] - x2[0];
    dy = x1[1] - x2[1];
    dz = x1[2] - x2[2];
    length += sqrt(dx * dx + dy * dy + dz * dz);
  }
  lengthf = fraction * length;
  length1 = 0.0;
  for(i = 0; i < n - 1; i++) {
    float *x1, *x2;
    float dx, dy, dz;

    x1 = xyzs + 3 * i;
    x2 = x1 + 3;
    dx = x1[0] - x2[0];
    dy = x1[1] - x2[1];
    dz = x1[2] - x2[2];
    length2 = length1 + sqrt(dx * dx + dy * dy + dz * dz);
    if(lengthf >= length1 && lengthf <= length2) {
      float f1;

      f1 = 0.5;
      if(length2 > length1) f1 = (length2 - lengthf) / (length2 - length1);
      xyz[0] = f1 * x1[0] + (1.0 - f1) * x2[0];
      xyz[1] = f1 * x1[1] + (1.0 - f1) * x2[1];
      xyz[2] = f1 * x1[2] + (1.0 - f1) * x2[2];
      return;
    }
    length1 = length2;
  }
  memcpy(xyz, xyzs + 3 * (n - 1), 3 * sizeof(float));
}

/* ------------------ GetCellXYZs ------------------------ */

void GetCellXYZs(float *xyz, int nxyz, int ncells, float **xyz_cellptr,
                 int *nxyz_cell, int **cell_indptr) {
  float length, *xyzi;
  float *fractions, *fractions_cell, *fractions_both;
  float *xyz_cell;
  int *cell_ind;
  int i;

  length = 0.0;
  xyzi = xyz;
  NewMemory((void **)&fractions, nxyz * sizeof(float));
  NewMemory((void **)&fractions_cell, (ncells + 1) * sizeof(float));
  NewMemory((void **)&fractions_both, (nxyz + ncells + 1) * sizeof(float));
  fractions[0] = 0.0;
  for(i = 0; i < nxyz - 1; i++) {
    float dx, dy, dz, *xyzip1;

    xyzip1 = xyzi + 3;
    dx = xyzip1[0] - xyzi[0];
    dy = xyzip1[1] - xyzi[1];
    dz = xyzip1[2] - xyzi[2];
    length += sqrt(dx * dx + dy * dy + dz * dz);
    fractions[i + 1] = length;
    xyzi += 3;
  }
  for(i = 1; i < nxyz - 1; i++) {
    fractions[i] /= length;
  }
  fractions[nxyz - 1] = 1.0;

  fractions_cell[0] = 0.0;
  for(i = 1; i < ncells; i++) {
    fractions_cell[i] = (float)i / (float)ncells;
  }
  fractions_cell[ncells] = 1.0;

  int i1, i2, nmerge;
  for(i1 = 0, i2 = 0, nmerge = 0; i1 < nxyz || i2 < ncells;) {
    if(i1 >= nxyz) {
      fractions_both[nmerge++] = fractions_cell[i2++];
      continue;
    }
    if(i2 >= ncells) {
      fractions_both[nmerge++] = fractions[i1++];
      continue;
    }
    if(fractions[i1] < fractions_cell[i2]) {
      fractions_both[nmerge++] = fractions[i1++];
      continue;
    }
    if(fractions_cell[i2] < fractions[i1]) {
      fractions_both[nmerge++] = fractions_cell[i2++];
      continue;
    }
    fractions_both[nmerge++] = fractions[i1++];
    i2++;
  }
  *nxyz_cell = nmerge;
  NewMemory((void **)&xyz_cell, 3 * nmerge * sizeof(float));
  NewMemory((void **)&cell_ind, nmerge * sizeof(int));
  *xyz_cellptr = xyz_cell;
  *cell_indptr = cell_ind;
  for(i = 0; i < nmerge; i++) {
    GetHVACPathXYZ(fractions_both[i], xyz, nxyz, xyz_cell + 3 * i);
  }
  assert(ncells >= 1);
  for(i = 0; i < nmerge - 1; i++) {
    if(ncells > 1) {
      float frac_avg;

      frac_avg = (fractions_both[i] + fractions_both[i + 1]) / 2.0;
      cell_ind[i] = CLAMP((int)(frac_avg * (float)ncells), 0, ncells - 1);
    }
    else {
      cell_ind[i] = 0;
    }
  }
  FREEMEMORY(fractions);
  FREEMEMORY(fractions_cell);
  FREEMEMORY(fractions_both);
}

/* ------------------ SetDuctLabelSymbolXYZ ------------------------ */

void SetDuctLabelSymbolXYZ(hvacductdata *ducti) {
  int j;
  float *xyz1, *xyz2;

  if(ducti->nxyz_reg == 2) {
    xyz1 = ducti->node_from->xyz;
    xyz2 = ducti->node_to->xyz;
    if(xyz1 != NULL && xyz2 != NULL){
      for(j = 0; j < 3; j++) {
        ducti->xyz_symbol[j] = 0.50 * xyz1[j] + 0.50 * xyz2[j];
        ducti->xyz_label[j] = 0.25 * xyz1[j] + 0.75 * xyz2[j];
      }
    }
    else{
      for(j = 0; j < 3; j++) {
        ducti->xyz_symbol[j] = 0.0;
        ducti->xyz_label[j] = 0.0;
      }
    }
  }
  else {
    GetHVACPathXYZ(0.50, ducti->xyz_reg, ducti->nxyz_reg, ducti->xyz_symbol);
    GetHVACPathXYZ(0.75, ducti->xyz_reg, ducti->nxyz_reg, ducti->xyz_label);
  }
}

/* ------------------ SetHVACInfo ------------------------ */

void SetHVACInfo(hvacdatacollection *hvaccoll) {
  int i;

  if(hvaccoll->hvacconnectinfo == NULL) {
    NewMemory((void **)&hvaccoll->hvacconnectinfo,
              (hvaccoll->nhvacnodeinfo + hvaccoll->nhvacductinfo + 1) *
                  sizeof(hvacconnectdata));
    hvaccoll->nhvacconnectinfo = 0;
    for(i = 0; i < hvaccoll->nhvacductinfo; i++) {
      hvacductdata *ducti;

      ducti = hvaccoll->hvacductinfo + i;
      if(HaveHVACConnect(ducti->connect_id, hvaccoll->hvacconnectinfo,
                         hvaccoll->nhvacconnectinfo) == 1)
        continue;
      hvaccoll->hvacconnectinfo[hvaccoll->nhvacconnectinfo].index =
          ducti->connect_id;
      hvaccoll->hvacconnectinfo[hvaccoll->nhvacconnectinfo].display = 0;
      hvaccoll->nhvacconnectinfo++;
    }
    for(i = 0; i < hvaccoll->nhvacnodeinfo; i++) {
      hvacnodedata *nodei;

      nodei = hvaccoll->hvacnodeinfo + i;
      if(HaveHVACConnect(nodei->connect_id, hvaccoll->hvacconnectinfo,
                         hvaccoll->nhvacconnectinfo) == 1)
        continue;
      hvaccoll->hvacconnectinfo[hvaccoll->nhvacconnectinfo].index =
          nodei->connect_id;
      hvaccoll->hvacconnectinfo[hvaccoll->nhvacconnectinfo].display = 1;
      hvaccoll->nhvacconnectinfo++;
    }
    if(hvaccoll->nhvacconnectinfo > 0) {
      ResizeMemory((void **)&hvaccoll->hvacconnectinfo,
                   hvaccoll->nhvacconnectinfo * sizeof(hvacconnectdata));
      qsort((int *)hvaccoll->hvacconnectinfo, hvaccoll->nhvacconnectinfo,
            sizeof(hvacconnectdata), CompareHvacConnect);
      for(i = 0; i < hvaccoll->nhvacductinfo; i++) {
        hvacductdata *ducti;
        int j;

        ducti = hvaccoll->hvacductinfo + i;
        ducti->connect = NULL;
        for(j = 0; j < hvaccoll->nhvacconnectinfo; j++) {
          hvacconnectdata *hj;

          hj = hvaccoll->hvacconnectinfo + j;
          if(ducti->connect_id == hj->index) {
            ducti->connect = hj;
            break;
          }
        }
      }
      for(i = 0; i < hvaccoll->nhvacnodeinfo; i++) {
        hvacnodedata *nodei;
        int j;

        nodei = hvaccoll->hvacnodeinfo + i;
        nodei->connect = NULL;
        for(j = 0; j < hvaccoll->nhvacconnectinfo; j++) {
          hvacconnectdata *hj;

          hj = hvaccoll->hvacconnectinfo + j;
          if(nodei->connect_id == hj->index) {
            nodei->connect = hj;
            break;
          }
        }
      }
    }
    else {
      FREEMEMORY(hvaccoll->hvacconnectinfo);
    }
  }
  for(i = 0; i < hvaccoll->nhvacnodeinfo; i++) {
    hvacnodedata *nodei;

    nodei = hvaccoll->hvacnodeinfo + i;
    memcpy(nodei->xyz, nodei->xyz_orig, 3 * sizeof(float));
  }

  for(i = 0; i < hvaccoll->nhvacductinfo; i++) {
    int j;
    hvacductdata *ducti;
    hvacnodedata *from_i, *to_i;
    float *xyz_from;
    float *xyz_to;

    ducti = hvaccoll->hvacductinfo + i;
    from_i = ducti->node_from;
    to_i = ducti->node_to;
    if(from_i == NULL || to_i == NULL) continue;
    xyz_from = from_i->xyz;
    xyz_to = to_i->xyz;
    for(j = 0; j < hvaccoll->nhvacductinfo; j++) {
      hvacductdata *ductj;
      hvacnodedata *from_j, *to_j;
      float diff[3];

      if(i == j) continue;
      ductj = hvaccoll->hvacductinfo + j;
      from_j = ductj->node_from;
      to_j = ductj->node_to;
      if(from_j == NULL || to_j == NULL) continue;
      if(from_i != from_j && from_i != to_j && to_i != from_j && to_i != to_j)
        continue;
      diff[0] = ABS(xyz_from[0] - xyz_to[0]);
      diff[1] = ABS(xyz_from[1] - xyz_to[1]);
      diff[2] = ABS(xyz_from[2] - xyz_to[2]);
      if(diff[0] < MIN(diff[1], diff[2])) {
        ducti->metro_path = DUCT_YZX;
        ductj->metro_path = DUCT_ZYX;
      }
      else if(diff[1] < MIN(diff[0], diff[2])) {
        ducti->metro_path = DUCT_XZY;
        ductj->metro_path = DUCT_ZXY;
      }
      else {
        ducti->metro_path = DUCT_XYZ;
        ductj->metro_path = DUCT_YXZ;
      }
    }
  }
#define COPYVALS3(xyz, x, y, z)                                                \
  *(xyz + 0) = (x);                                                            \
  *(xyz + 1) = (y);                                                            \
  *(xyz + 2) = (z)
  for(i = 0; i < hvaccoll->nhvacductinfo; i++) {
    hvacductdata *ducti;
    hvacnodedata *node_from, *node_to;
    float *xyz0, *xyz1;

    ducti = hvaccoll->hvacductinfo + i;
    node_from = hvaccoll->hvacnodeinfo + ducti->node_id_from;
    node_to = hvaccoll->hvacnodeinfo + ducti->node_id_to;
    if(node_from == NULL || node_to == NULL) continue;
    xyz0 = node_from->xyz;
    xyz1 = node_to->xyz;
    float *dxyz, *dxyz_metro;

    dxyz = ducti->normal;
    dxyz_metro = ducti->normal_metro;
    dxyz[0] = xyz1[0] - xyz0[0];
    dxyz[1] = xyz1[1] - xyz0[1];
    dxyz[2] = xyz1[2] - xyz0[2];
    dxyz_metro[0] = 0.0;
    dxyz_metro[1] = 0.0;
    dxyz_metro[2] = 0.0;
    memcpy(ducti->xyz_symbol_metro, xyz0, 3 * sizeof(float));
    memcpy(ducti->xyz_label_metro, xyz0, 3 * sizeof(float));
    memcpy(ducti->xyz_met, xyz0, 3 * sizeof(float));
    memcpy(ducti->xyz_met + 9, xyz1, 3 * sizeof(float));
    ducti->nxyz_met = 3;
    switch(ducti->metro_path) {
    case DUCT_XYZ:
      COPYVALS3(ducti->xyz_met + 3, xyz1[0], xyz0[1], xyz0[2]);
      COPYVALS3(ducti->xyz_met + 6, xyz1[0], xyz1[1], xyz0[2]);
      if(ABS(dxyz[0]) > ABS(dxyz[1])) {
        ducti->xyz_symbol_metro[0] += 0.5 * dxyz[0];
        ducti->xyz_label_metro[0] += 0.75 * dxyz[0];
        dxyz_metro[0] = 1.0;
      }
      else {
        ducti->xyz_symbol_metro[0] += dxyz[0];
        ducti->xyz_symbol_metro[1] += 0.5 * dxyz[1];
        ducti->xyz_label_metro[0] += dxyz[0];
        ducti->xyz_label_metro[1] += 0.75 * dxyz[1];
        dxyz_metro[1] = 1.0;
      }
      break;
    case DUCT_XZY:
      COPYVALS3(ducti->xyz_met + 3, xyz1[0], xyz0[1], xyz0[2]);
      COPYVALS3(ducti->xyz_met + 6, xyz1[0], xyz0[1], xyz1[2]);
      if(ABS(dxyz[0]) > ABS(dxyz[2])) {
        ducti->xyz_symbol_metro[0] += 0.50 * dxyz[0];
        ducti->xyz_label_metro[0] += 0.75 * dxyz[0];
        dxyz_metro[0] = 1.0;
      }
      else {
        ducti->xyz_symbol_metro[0] += dxyz[0];
        ducti->xyz_symbol_metro[2] += 0.5 * dxyz[2];
        ducti->xyz_label_metro[0] += dxyz[0];
        ducti->xyz_label_metro[2] += 0.75 * dxyz[2];
        dxyz_metro[2] = 1.0;
      }
      break;
    case DUCT_YXZ:
      COPYVALS3(ducti->xyz_met + 3, xyz0[0], xyz1[1], xyz0[2]);
      COPYVALS3(ducti->xyz_met + 6, xyz1[0], xyz1[1], xyz0[2]);
      if(ABS(dxyz[1]) > ABS(dxyz[0])) {
        ducti->xyz_symbol_metro[1] += 0.50 * dxyz[1];
        ducti->xyz_label_metro[1] += 0.75 * dxyz[1];
        dxyz_metro[1] = 1.0;
      }
      else {
        ducti->xyz_symbol_metro[0] += 0.50 * dxyz[0];
        ducti->xyz_symbol_metro[1] += dxyz[1];
        ducti->xyz_label_metro[0] += 0.75 * dxyz[0];
        ducti->xyz_label_metro[1] += dxyz[1];
        dxyz_metro[0] = 1.0;
      }
      break;
    case DUCT_YZX:
      COPYVALS3(ducti->xyz_met + 3, xyz0[0], xyz1[1], xyz0[2]);
      COPYVALS3(ducti->xyz_met + 6, xyz0[0], xyz1[1], xyz1[2]);
      if(ABS(dxyz[1]) > ABS(dxyz[2])) {
        ducti->xyz_symbol_metro[1] += 0.50 * dxyz[1];
        ducti->xyz_label_metro[1] += 0.75 * dxyz[1];
        dxyz_metro[1] = 1.0;
      }
      else {
        ducti->xyz_symbol_metro[1] += dxyz[1];
        ducti->xyz_symbol_metro[2] += 0.50 * dxyz[2];
        ducti->xyz_label_metro[1] += dxyz[1];
        ducti->xyz_label_metro[2] += 0.75 * dxyz[2];
        dxyz_metro[2] = 1.0;
      }
      break;
    case DUCT_ZXY:
      COPYVALS3(ducti->xyz_met + 3, xyz0[0], xyz0[1], xyz1[2]);
      COPYVALS3(ducti->xyz_met + 6, xyz1[0], xyz0[1], xyz1[2]);
      if(ABS(dxyz[2]) > ABS(dxyz[0])) {
        ducti->xyz_symbol_metro[2] += 0.50 * dxyz[2];
        ducti->xyz_label_metro[2] += 0.75 * dxyz[2];
        dxyz_metro[2] = 1.0;
      }
      else {
        ducti->xyz_symbol_metro[0] += 0.50 * dxyz[0];
        ducti->xyz_symbol_metro[2] += dxyz[2];
        ducti->xyz_label_metro[0] += 0.75 * dxyz[0];
        ducti->xyz_label_metro[2] += dxyz[2];
        dxyz_metro[0] = 1.0;
      }
      break;
    case DUCT_ZYX:
      COPYVALS3(ducti->xyz_met + 3, xyz0[0], xyz0[1], xyz1[2]);
      COPYVALS3(ducti->xyz_met + 6, xyz0[0], xyz1[1], xyz1[2]);
      if(ABS(dxyz[2]) > ABS(dxyz[1])) {
        ducti->xyz_symbol_metro[2] += 0.50 * dxyz[2];
        ducti->xyz_label_metro[2] += 0.75 * dxyz[2];
        dxyz_metro[2] = 1.0;
      }
      else {
        ducti->xyz_symbol_metro[1] += 0.50 * dxyz[1];
        ducti->xyz_symbol_metro[2] += dxyz[2];
        ducti->xyz_label_metro[1] += 0.75 * dxyz[1];
        ducti->xyz_label_metro[2] += dxyz[2];
        dxyz_metro[1] = 1.0;
      }
      break;
    default:
      assert(FFALSE);
      break;
    }
    GetCellXYZs(ducti->xyz_reg, ducti->nxyz_reg, ducti->nduct_cells,
                &ducti->xyz_reg_cell, &ducti->nxyz_reg_cell, &ducti->cell_reg);
    GetCellXYZs(ducti->xyz_met, ducti->nxyz_met, ducti->nduct_cells,
                &ducti->xyz_met_cell, &ducti->nxyz_met_cell, &ducti->cell_met);
  }
  for(i = 0; i < hvaccoll->nhvacductinfo; i++) {
    SetDuctLabelSymbolXYZ(hvaccoll->hvacductinfo + i);
  }
}

/* ------------------ ReadHVACData0 ------------------------ */

int ReadHVACData0(hvacdatacollection *hvaccoll, int flag,
                  FILE_SIZE *file_size) {
  FILE *stream = NULL;
  float *node_buffer = NULL, *duct_buffer = NULL, *ducttimes, *nodetimes;
  int max_node_buffer = 0, max_duct_buffer = 0;
  int parms[4], n_nodes, n_node_vars, n_ducts, n_duct_vars;
  int frame_size, header_size, nframes;
  int i, iframe;
  int *duct_ncells;

  if(hvaccoll->hvacductvalsinfo == NULL) return 1;
  if(hvaccoll->hvacnodevalsinfo == NULL) return 1;
  FREEMEMORY(hvaccoll->hvacductvalsinfo->times);
  FREEMEMORY(hvaccoll->hvacnodevalsinfo->times);

  for(i = 0; i < hvaccoll->hvacductvalsinfo->n_duct_vars; i++) {
    hvacvaldata *hi;

    hi = hvaccoll->hvacductvalsinfo->duct_vars + i;
    FREEMEMORY(hi->vals);
  }

  for(i = 0; i < hvaccoll->hvacnodevalsinfo->n_node_vars; i++) {
    hvacvaldata *hi;

    hi = hvaccoll->hvacnodevalsinfo->node_vars + i;
    FREEMEMORY(hi->vals);
  }
  hvaccoll->hvacductvalsinfo->loaded = 0;
  hvaccoll->hvacnodevalsinfo->loaded = 0;
  if(flag == 1 /*TODO: should be 'UNLOAD'*/) return 2;

  stream = fopen(hvaccoll->hvacductvalsinfo->file, "rb");
  if(stream == NULL) return 3;

  FSEEK(stream, 4, SEEK_CUR);
  fread(parms, 4, 4, stream);
  FSEEK(stream, 4, SEEK_CUR);
  n_nodes = parms[0];
  n_node_vars = parms[1];
  n_ducts = parms[2];
  n_duct_vars = parms[3];
  header_size = 4 + 4 * 4 + 4; // n_node_out n_node_vars n_duct_out n_ductd_vars
  header_size += 4 + 4 * n_ducts + 4; // number of cells in each duct
  frame_size = 4 + 4 + 4;             // time
  frame_size += n_nodes * (4 + 4 * n_node_vars + 4); // node data
  frame_size += n_ducts * (4 + 4 * n_duct_vars + 4); // duct data
  *file_size = GetFileSizeSMV(hvaccoll->hvacductvalsinfo->file);
  nframes = (*file_size - header_size) / frame_size;

  NewMemory((void **)&duct_ncells, n_ducts * sizeof(int));
  FSEEK(stream, 4, SEEK_CUR);
  fread(duct_ncells, 4, n_ducts, stream);
  FSEEK(stream, 4, SEEK_CUR);

  FREEMEMORY(hvaccoll->hvacductvalsinfo->times);
  NewMemory((void **)&hvaccoll->hvacductvalsinfo->times,
            nframes * sizeof(float));
  hvaccoll->hvacductvalsinfo->ntimes = nframes;

  FREEMEMORY(hvaccoll->hvacnodevalsinfo->times);
  NewMemory((void **)&hvaccoll->hvacnodevalsinfo->times,
            nframes * sizeof(float));
  hvaccoll->hvacnodevalsinfo->ntimes = nframes;

  hvaccoll->hvac_maxcells = duct_ncells[0];
  for(i = 1; i < n_ducts; i++) {
    hvaccoll->hvac_maxcells = MAX(hvaccoll->hvac_maxcells, duct_ncells[i]);
  }
  hvaccoll->hvac_n_ducts = n_ducts;

  for(i = 0; i < hvaccoll->hvacductvalsinfo->n_duct_vars; i++) {
    hvacvaldata *hi;

    hi = hvaccoll->hvacductvalsinfo->duct_vars + i;
    NewMemory((void **)&hi->vals,
              n_ducts * hvaccoll->hvac_maxcells * nframes * sizeof(float));
  }
  for(i = 0; i < hvaccoll->hvacnodevalsinfo->n_node_vars; i++) {
    hvacvaldata *hi;

    hi = hvaccoll->hvacnodevalsinfo->node_vars + i;
    FREEMEMORY(hi->vals);
    NewMemory((void **)&hi->vals, n_nodes * nframes * sizeof(float));
  }

  rewind(stream);
  FSEEK(stream, header_size, SEEK_CUR); // skip over header

  ducttimes = hvaccoll->hvacductvalsinfo->times;
  nodetimes = hvaccoll->hvacnodevalsinfo->times;
  for(iframe = 0; iframe < nframes; iframe++) {
    int j;
    float time;

    FSEEK(stream, 4, SEEK_CUR);
    fread(&time, 4, 1, stream);
    FSEEK(stream, 4, SEEK_CUR);
    ducttimes[iframe] = time;
    nodetimes[iframe] = time;
    if(n_node_vars > max_node_buffer) {
      FREEMEMORY(node_buffer);
      NewMemory((void **)&node_buffer, (n_node_vars + 100) * sizeof(float));
      max_node_buffer = n_node_vars + 100;
    }
    for(j = 0; j < n_nodes; j++) {
      int k;

      FSEEK(stream, 4, SEEK_CUR);
      fread(node_buffer, 4, n_node_vars, stream);
      FSEEK(stream, 4, SEEK_CUR);
      for(k = 0; k < n_node_vars; k++) {
        hvacvaldata *hk;

        hk = hvaccoll->hvacnodevalsinfo->node_vars + k;
        hk->vals[iframe + j * nframes] = node_buffer[k];
      }
    }

    int iduct;
    for(iduct = 0; iduct < n_ducts; iduct++) {
      int ntotalvals;

      ntotalvals = n_duct_vars * duct_ncells[iduct];
      if(ntotalvals > max_duct_buffer) {
        FREEMEMORY(duct_buffer);
        NewMemory((void **)&duct_buffer, (ntotalvals + 100) * sizeof(float));
        max_duct_buffer = ntotalvals + 100;
      }
      int icell;
      if(duct_buffer!=NULL){
        for(icell = 0; icell < duct_ncells[iduct]; icell++) {
          int ivar;

          FSEEK(stream, 4, SEEK_CUR);
          fread(duct_buffer, 4, n_duct_vars, stream);
          FSEEK(stream, 4, SEEK_CUR);
          for(ivar = 0; ivar < n_duct_vars; ivar++) {
            hvacvaldata *hk;

            hk = hvaccoll->hvacductvalsinfo->duct_vars + ivar;
            hk->vals[hvacval(hvaccoll, iframe, iduct, icell)] = duct_buffer[ivar];
          }
        }
      }
    }
  }
  fclose(stream);
  FREEMEMORY(duct_buffer);
  FREEMEMORY(node_buffer);

  for(i = 0; i < n_node_vars; i++) {
    hvacvaldata *hi;
    float *vals;
    int j;

    hi = hvaccoll->hvacnodevalsinfo->node_vars + i;

    vals = hi->vals;
    hi->valmin = vals[0];
    hi->valmax = hi->valmin;
    for(j = 1; j < nframes * n_nodes; j++) {
      hi->valmin = MIN(vals[j], hi->valmin);
      hi->valmax = MAX(vals[j], hi->valmax);
    }
  }
  for(i = 0; i < n_duct_vars; i++) {
    hvacvaldata *hi;
    int iduct;

    hi = hvaccoll->hvacductvalsinfo->duct_vars + i;

    hi->valmin = hi->vals[0];
    hi->valmax = hi->valmin;
    for(iduct = 0; iduct < n_ducts; iduct++) {
      int icell;

      for(icell = 0; icell < duct_ncells[iduct]; icell++) {
        int iframe2;

        for(iframe2 = 0; iframe2 < nframes; iframe2++) {
          int index;

          index = hvacval(hvaccoll, iframe2, iduct, icell);
          hi->valmin = MIN(hi->vals[index], hi->valmin);
          hi->valmax = MAX(hi->vals[index], hi->valmax);
        }
      }
    }
  }
  FREEMEMORY(duct_ncells);
  return 0;
}

/* ------------------ CompareLabel ------------------------ */

int CompareLabel(const void *arg1, const void *arg2) {
  char *x, *y;

  x = *(char **)arg1;
  y = *(char **)arg2;

  return strcmp(x, y);
}

/* ------------------ ParseHVACEntry ------------------------ */

int ParseHVACEntry(hvacdatacollection *hvaccoll, bufferstreamdata *stream,
                   int hvac_node_color[3], int hvac_duct_color[3]) {
  char buffer[256];
  // HVAC
  //  NODES
  //  n_nodes
  if(FGETS(buffer, 255, stream) == NULL) return 1;
  if(FGETS(buffer, 255, stream) == NULL) return 1;
  sscanf(buffer, "%i", &hvaccoll->nhvacnodeinfo);
  hvaccoll->nhvacnodeinfo = MAX(hvaccoll->nhvacnodeinfo, 0);
  if(hvaccoll->nhvacnodeinfo == 0) return 2;

  FREEMEMORY(hvaccoll->hvacnodeinfo);
  NewMemory((void **)&hvaccoll->hvacnodeinfo,
            hvaccoll->nhvacnodeinfo * sizeof(hvacnodedata));

  // node_id duct_label network_label
  // x y z filter_flag vent_label

  for(int i = 0; i < hvaccoll->nhvacnodeinfo; i++) {
    hvacnodedata *nodei;
    char *filter, *node_label, *network_label, *connect_id;

    nodei = hvaccoll->hvacnodeinfo + i;

    if(FGETS(buffer, 255, stream) == NULL) break;
    sscanf(buffer, "%i", &nodei->node_id);
    TrimBack(buffer);
    strtok(buffer, "%");
    node_label = strtok(NULL, "%");
    network_label = strtok(NULL, "%");
    connect_id = strtok(NULL, "%");
    nodei->node_name = GetCharPtr(node_label);
    network_label = TrimFrontBack(network_label);
    if(strcmp(network_label, "null") == 0) {
      nodei->network_name = GetCharPtr("Unassigned");
    }
    else {
      nodei->network_name = GetCharPtr(network_label);
    }
    nodei->duct = NULL;
    nodei->connect_id = -1;
    if(connect_id != NULL) sscanf(connect_id, "%i", &nodei->connect_id);

    if(FGETS(buffer, 255, stream) == NULL) break;
    sscanf(buffer, "%f %f %f", nodei->xyz, nodei->xyz + 1, nodei->xyz + 2);
    memcpy(nodei->xyz_orig, nodei->xyz, 3 * sizeof(float));
    strtok(buffer, "%");
    filter = strtok(NULL, "%");
    filter = TrimFrontBack(filter);
    nodei->filter = HVAC_FILTER_NO;
    strcpy(nodei->c_filter, "");
    if(filter != NULL && strcmp(filter, "FILTER") == 0) {
      nodei->filter = HVAC_FILTER_YES;
      strcpy(nodei->c_filter, "FI");
      hvaccoll->nhvacfilters++;
    }
  }
  // DUCTS
  // n_ducts
  // duct_id node_id1 node_id2 duct_name network_name
  // fan_type
  // duct_name
  // component Fan(F), Aircoil(A), Damper(D), none(-)
  // waypoint xyz
  if(FGETS(buffer, 255, stream) == NULL) return 1;
  if(FGETS(buffer, 255, stream) == NULL) return 1;
  sscanf(buffer, "%i", &hvaccoll->nhvacductinfo);
  hvaccoll->nhvacductinfo = MAX(hvaccoll->nhvacductinfo, 0);
  if(hvaccoll->nhvacductinfo == 0) {
    FREEMEMORY(hvaccoll->hvacnodeinfo);
    hvaccoll->nhvacnodeinfo = 0;
    return 1;
  }

  FREEMEMORY(hvaccoll->hvacductinfo);
  NewMemory((void **)&(hvaccoll->hvacductinfo),
            hvaccoll->nhvacductinfo * sizeof(hvacductdata));
  for(int i = 0; i < hvaccoll->nhvacductinfo; i++) {
    hvacductdata *ducti;
    char *duct_label, *network_label, *hvac_label, *connect_id;

    ducti = hvaccoll->hvacductinfo + i;
    if(FGETS(buffer, 255, stream) == NULL) break;
    sscanf(buffer, "%i %i %i", &ducti->duct_id, &ducti->node_id_from,
           &ducti->node_id_to);
    ducti->node_id_from--;
    ducti->node_id_to--;

    ducti->node_from = hvaccoll->hvacnodeinfo + ducti->node_id_from;
    ducti->node_to = hvaccoll->hvacnodeinfo + ducti->node_id_to;

    if(ducti->node_from->duct == NULL) ducti->node_from->duct = ducti;
    if(ducti->node_to->duct == NULL) ducti->node_to->duct = ducti;

    strtok(buffer, "%");
    duct_label = strtok(NULL, "%");
    network_label = strtok(NULL, "%");
    connect_id = strtok(NULL, "%");
    ducti->duct_name = GetCharPtr(duct_label);
    network_label = TrimFrontBack(network_label);
    if(strcmp(network_label, "null") == 0) {
      ducti->network_name = GetCharPtr("Unassigned");
    }
    else {
      ducti->network_name = GetCharPtr(network_label);
    }
    ducti->act_times = NULL;
    ducti->act_states = NULL;
    ducti->nact_times = 0;
    ducti->metro_path = DUCT_XYZ;
    ducti->connect_id = -1;
    ducti->xyz_reg = NULL;
    ducti->cell_met = NULL;
    ducti->cell_reg = NULL;
    ducti->nxyz_met = 0;
    ducti->nxyz_reg = 0;
    ducti->xyz_met_cell = NULL;
    ducti->xyz_reg_cell = NULL;
    ducti->nxyz_met_cell = 0;
    ducti->nxyz_reg_cell = 0;

    if(connect_id != NULL) sscanf(connect_id, "%i", &ducti->connect_id);

    if(FGETS(buffer, 255, stream) == NULL) break;
    if(FGETS(buffer, 255, stream) == NULL) break;
    sscanf(buffer, "%i", &ducti->nduct_cells);
    strtok(buffer, "%");
    hvac_label = strtok(NULL, "%");
    hvac_label = TrimFrontBack(hvac_label);

    char *c_component[4] = {"-", "F", "A", "D"};
    ducti->component = HVAC_NONE;
    if(hvac_label != NULL) {
      if(hvac_label[0] == 'F') ducti->component = HVAC_FAN;
      if(hvac_label[0] == 'A') ducti->component = HVAC_AIRCOIL;
      if(hvac_label[0] == 'D') ducti->component = HVAC_DAMPER;
    }
    if(ducti->component != HVAC_NONE) hvaccoll->nhvaccomponents++;
    strcpy(ducti->c_component, c_component[ducti->component]);

    if(FGETS(buffer, 255, stream) == NULL) break;
    if(FGETS(buffer, 255, stream) == NULL) break;
    int n_waypoints;
    sscanf(buffer, "%i", &n_waypoints);

    float *waypoints;

    NewMemory((void **)&waypoints, 3 * (n_waypoints + 2) * sizeof(float));
    ducti->xyz_reg = waypoints;
    ducti->nxyz_reg = n_waypoints + 2;

    hvacnodedata *node_from, *node_to;
    float *xyz0, *xyz1;

    node_from = hvaccoll->hvacnodeinfo + ducti->node_id_from;
    node_to = hvaccoll->hvacnodeinfo + ducti->node_id_to;
    xyz0 = node_from->xyz;
    xyz1 = node_to->xyz;

    memcpy(ducti->xyz_reg, xyz0, 3 * sizeof(float)); // first point
    memcpy(ducti->xyz_reg + 3 * (n_waypoints + 1), xyz1,
           3 * sizeof(float)); // last point

    waypoints += 3;
    for(int j = 0; j < n_waypoints;
        j++) { // points between first and last point
      if(FGETS(buffer, 255, stream) == NULL) break;
      sscanf(buffer, "%f %f %f", waypoints, waypoints + 1, waypoints + 2);
      waypoints += 3;
    }
  }
  char **hvac_network_labels = NULL;

  NewMemory((void **)&hvac_network_labels,
            (hvaccoll->nhvacnodeinfo + hvaccoll->nhvacductinfo) *
                sizeof(char *));
  for(int i = 0; i < hvaccoll->nhvacnodeinfo; i++) {
    hvac_network_labels[i] = hvaccoll->hvacnodeinfo[i].network_name;
  }
  for(int i = 0; i < hvaccoll->nhvacductinfo; i++) {
    hvac_network_labels[i + hvaccoll->nhvacnodeinfo] =
        hvaccoll->hvacductinfo[i].network_name;
  }
  qsort((char *)hvac_network_labels,
        (size_t)(hvaccoll->nhvacnodeinfo + hvaccoll->nhvacductinfo),
        sizeof(char *), CompareLabel);
  hvaccoll->nhvacinfo = 1;
  for(int i = 1; i < hvaccoll->nhvacnodeinfo + hvaccoll->nhvacductinfo; i++) {
    if(strcmp(hvac_network_labels[hvaccoll->nhvacinfo - 1],
              hvac_network_labels[i]) == 0)
      continue;
    hvac_network_labels[hvaccoll->nhvacinfo] = hvac_network_labels[i];
    hvaccoll->nhvacinfo++;
  }
  NewMemory((void **)&hvaccoll->hvacinfo,
            hvaccoll->nhvacinfo * sizeof(hvacdata));
  for(int i = 0; i < hvaccoll->nhvacinfo; i++) {
    hvacdata *hvaci;

    hvaci = hvaccoll->hvacinfo + i;
    hvaci->network_name = hvac_network_labels[i];
    hvaci->display = 0;
    hvaci->show_node_labels = 0;
    hvaci->show_duct_labels = 0;
    hvaci->show_filters = NODE_FILTERS_HIDE;
    hvaci->show_component = DUCT_COMPONENT_HIDE;
    hvaci->component_size = 1.0;
    hvaci->filter_size = 1.0;
    hvaci->node_size = 8.0;
    hvaci->cell_node_size = 8.0;
    hvaci->duct_width = 4.0;
    memcpy(hvaci->node_color, hvac_node_color, 3 * sizeof(int));
    memcpy(hvaci->duct_color, hvac_duct_color, 3 * sizeof(int));
  }
  FREEMEMORY(hvac_network_labels);
  SetHVACInfo(hvaccoll);
  return 0;
}

/* ------------------ ParseHVACValsEntry ------------------------ */

int ParseHVACValsEntry(hvacdatacollection *hvaccoll, bufferstreamdata *stream) {
  char buffer[256];
  FREEMEMORY(hvaccoll->hvacductvalsinfo);
  NewMemory((void **)&(hvaccoll->hvacductvalsinfo), sizeof(hvacvalsdata));
  hvaccoll->hvacductvalsinfo->times = NULL;
  hvaccoll->hvacductvalsinfo->loaded = 0;
  hvaccoll->hvacductvalsinfo->node_vars = NULL;
  hvaccoll->hvacductvalsinfo->duct_vars = NULL;

  if(FGETS(buffer, 255, stream) == NULL) return 1;
  hvaccoll->hvacductvalsinfo->file = GetCharPtr(TrimFrontBack(buffer));

  if(FGETS(buffer, 255, stream) == NULL) return 1;
  sscanf(buffer, "%i", &(hvaccoll->hvacductvalsinfo)->n_node_vars);

  if(hvaccoll->hvacductvalsinfo->n_node_vars > 0) {
    NewMemory((void **)&(hvaccoll->hvacductvalsinfo)->node_vars,
              hvaccoll->hvacductvalsinfo->n_node_vars * sizeof(hvacvaldata));
    for(int i = 0; i < hvaccoll->hvacductvalsinfo->n_node_vars; i++) {
      hvacvaldata *hi;
      flowlabels *labeli;

      hi = hvaccoll->hvacductvalsinfo->node_vars + i;
      InitHvacData(hi);
      labeli = &hi->label;
      ReadLabels(labeli, stream, NULL);
    }
  }

  if(FGETS(buffer, 255, stream) == NULL) return 1;
  sscanf(buffer, "%i", &hvaccoll->hvacductvalsinfo->n_duct_vars);

  if(hvaccoll->hvacductvalsinfo->n_duct_vars > 0) {
    NewMemory((void **)&hvaccoll->hvacductvalsinfo->duct_vars,
              hvaccoll->hvacductvalsinfo->n_duct_vars * sizeof(hvacvaldata));
    for(int i = 0; i < hvaccoll->hvacductvalsinfo->n_duct_vars; i++) {
      hvacvaldata *hi;
      flowlabels *labeli;

      hi = hvaccoll->hvacductvalsinfo->duct_vars + i;
      InitHvacData(hi);
      labeli = &hi->label;
      ReadLabels(labeli, stream, NULL);
    }
  }
  FREEMEMORY(hvaccoll->hvacnodevalsinfo);
  NewMemory((void **)&hvaccoll->hvacnodevalsinfo, sizeof(hvacvalsdata));
  memcpy(hvaccoll->hvacnodevalsinfo, hvaccoll->hvacductvalsinfo,
         sizeof(hvacvalsdata));
  return 0;
}
