#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "IOobjects.h"

#define HVACVAL(itime, iduct, icell) \
  (itime)*hvac_maxcells*hvac_n_ducts + (iduct)*hvac_maxcells + (icell)

unsigned char hvac_off_color[3] = {0, 255, 0}, hvac_on_color[3] = {255, 0, 0};
unsigned char *hvac_color_states[2] = {hvac_off_color, hvac_on_color};
#define HVACCOLORCONV(v,min,max) (max) < (min) ? 0 : 255*((v)-(min)/((max)-(min)))

/* ------------------ OffsetHvacNodes ------------------------ */

void OffsetHvacNodes(int *line, int *hvac_index, int flag){
  int offset_index;
  int i;

  offset_index = 0;
  for(i = 1;i < nhvacnodeinfo;i++){
    if(line[i] == 1){
      hvacnodedata *nodei;
      float *xyz, *xyz_orig;

      offset_index++;
      nodei = hvacnodeinfo + hvac_index[i];
      xyz      = nodei->xyz;
      xyz_orig = nodei->xyz_orig;
      if(flag == 0){
        xyz[1] = xyz_orig[1] + (float)offset_index * hvac_offset_inc;
        xyz[2] = xyz_orig[2] + (float)offset_index * hvac_offset_inc;
      }
      else if(flag == 1){
        xyz[0] = xyz_orig[0] + (float)offset_index * hvac_offset_inc;
        xyz[2] = xyz_orig[2] + (float)offset_index * hvac_offset_inc;
      }
      else{
        xyz[0] = xyz_orig[0] + (float)offset_index * hvac_offset_inc;
        xyz[1] = xyz_orig[1] + (float)offset_index * hvac_offset_inc;
      }
    }
    else{
      offset_index = 0;
    }
  }
}

#define NODE_XYZ 0.1

/* ------------------ CompareFloatYZ ------------------------ */

int CompareFloatYZ(const void *arg1, const void *arg2){
  int i, j;
  float *xyzi, *xyzj;

  i = *(int *)arg1;
  j = *(int *)arg2;
  xyzi = hvacnodeinfo[i].xyz_orig;
  xyzj = hvacnodeinfo[j].xyz_orig;
  if(xyzi[1] < xyzj[1] - NODE_XYZ)return -1;
  if(xyzi[1] > xyzj[1] + NODE_XYZ)return 1;
  if(xyzi[2] < xyzj[2] - NODE_XYZ)return -1;
  if(xyzi[2] > xyzj[2] + NODE_XYZ)return 1;
  return 0;
}

/* ------------------ CompareFloatXZ ------------------------ */

int CompareFloatXZ(const void *arg1, const void *arg2){
  int i, j;
  float *xyzi, *xyzj;

  i = *(int *)arg1;
  j = *(int *)arg2;
  xyzi = hvacnodeinfo[i].xyz_orig;
  xyzj = hvacnodeinfo[j].xyz_orig;
  if(xyzi[0] < xyzj[0] - NODE_XYZ)return -1;
  if(xyzi[0] > xyzj[0] + NODE_XYZ)return 1;
  if(xyzi[2] < xyzj[2] - NODE_XYZ)return -1;
  if(xyzi[2] > xyzj[2] + NODE_XYZ)return 1;
  return 0;
}

/* ------------------ CompareFloatXY ------------------------ */

int CompareFloatXY(const void *arg1, const void *arg2){
  int i, j;
  float *xyzi, *xyzj;

  i = *(int *)arg1;
  j = *(int *)arg2;
  xyzi = hvacnodeinfo[i].xyz_orig;
  xyzj = hvacnodeinfo[j].xyz_orig;
  if(xyzi[0] < xyzj[0] - NODE_XYZ)return -1;
  if(xyzi[0] > xyzj[0] + NODE_XYZ)return 1;
  if(xyzi[1] < xyzj[1] - NODE_XYZ)return -1;
  if(xyzi[1] > xyzj[1] + NODE_XYZ)return 1;
  return 0;
}


/* ------------------ CompareHvacConnect ------------------------ */

int CompareHvacConnect(const void *arg1, const void *arg2){
  hvacconnectdata *hi, *hj;
  int indexi, indexj;

  hi = (hvacconnectdata *)arg1;
  hj = (hvacconnectdata *)arg2;
  indexi = hi->index;
  indexj = hj->index;
  if(indexi < indexj)return -1;
  if(indexi > indexj)return 1;
  return 0;
}

/* ------------------ IsHVACVisible ------------------------ */

int IsHVACVisible(void){
  int i;

  for(i = 0; i < nhvacinfo; i++){
    hvacdata *hvaci;

    hvaci = hvacinfo + i;
    if(hvaci->display == 1)return 1;
  }
  return 0;
}

/* ------------------ HaveHVACConnect ------------------------ */

int HaveHVACConnect(int val, hvacconnectdata *vals, int nvals){
  int i;

  if(val == -1)return 1;
  for(i = 0;i < nvals;i++){
    if(val == vals[i].index)return 1;
  }
  return 0;
}

/* ------------------ GetHVACPathXYZ ------------------------ */

void GetHVACPathXYZ(float fraction, float *xyzs, int n, float *xyz){ 
  int i;
  float length=0.0, lengthf;
  float length1, length2;

  if(fraction<=0.0){
    memcpy(xyz, xyzs, 3*sizeof(float));
    return;
  }
  if(fraction>=1.0){
    memcpy(xyz, xyzs+3*(n-1), 3*sizeof(float));
    return;
  }
  for(i=0; i<n-1; i++){
    float *x1, *x2;
    float dx, dy, dz;

    x1 = xyzs+3*i;
    x2 = x1 + 3;
    dx = x1[0] - x2[0];
    dy = x1[1] - x2[1];
    dz = x1[2] - x2[2];
    length += sqrt(dx*dx+dy*dy+dz*dz);
  }
  lengthf = fraction*length;
  length1 = 0.0;
  for(i=0; i<n-1; i++){
    float *x1, *x2;
    float dx, dy, dz;

    x1 = xyzs+3*i;
    x2 = x1 + 3;
    dx = x1[0] - x2[0];
    dy = x1[1] - x2[1];
    dz = x1[2] - x2[2];
    length2 =length1 +  sqrt(dx*dx+dy*dy+dz*dz);
    if(lengthf>=length1&&lengthf<=length2){
      float f1;

      f1 = 0.5;
      if(length2>length1)f1 = (length2-lengthf)/(length2-length1);
      xyz[0] = f1*x1[0] + (1.0-f1)*x2[0];
      xyz[1] = f1*x1[1] + (1.0-f1)*x2[1];
      xyz[2] = f1*x1[2] + (1.0-f1)*x2[2];
      return;
    }
    length1 = length2;
  }
  memcpy(xyz, xyzs+3*(n-1), 3*sizeof(float));
}

/* ------------------ GetCellXYZs ------------------------ */

void GetCellXYZs(float *xyz, int nxyz, int ncells, float **xyz_cellptr, int *nxyz_cell, int **cell_indptr){
  float length, *xyzi;
  float *fractions, *fractions_cell, *fractions_both;
  float *xyz_cell;
  int *cell_ind;
  int i;

  length = 0.0;
  xyzi = xyz;
  NewMemory((void **)&fractions, nxyz*sizeof(float));
  NewMemory((void **)&fractions_cell, (ncells+1)*sizeof(float));
  NewMemory((void **)&fractions_both, (nxyz+ncells+1)*sizeof(float));
  fractions[0] = 0.0;
  for(i = 0;i < nxyz-1;i++){
    float dx, dy, dz, *xyzip1;

    xyzip1 = xyzi + 3;
    dx = xyzip1[0] - xyzi[0];
    dy = xyzip1[1] - xyzi[1];
    dz = xyzip1[2] - xyzi[2];
    length += sqrt(dx*dx + dy*dy + dz*dz);
    fractions[i+1] = length;
    xyzi += 3;
  }
  for(i = 1;i < nxyz-1;i++){
    fractions[i] /= length;
  }
  fractions[nxyz-1] = 1.0;
  
  fractions_cell[0]      = 0.0;
  for(i=1;i<ncells;i++){
    fractions_cell[i]=(float)i/(float)ncells;
  }
  fractions_cell[ncells] = 1.0;

  int i1, i2, nmerge;
  for(i1=0,i2=0,nmerge=0;i1<nxyz||i2<ncells;){
    if(i1>=nxyz){
      fractions_both[nmerge++] = fractions_cell[i2++];
      continue;
    }
    if(i2>=ncells){
      fractions_both[nmerge++] = fractions[i1++];
      continue;
    }
    if(fractions[i1]<fractions_cell[i2]){
      fractions_both[nmerge++] = fractions[i1++];
      continue;
    }
    if(fractions_cell[i2]<fractions[i1]){
      fractions_both[nmerge++] = fractions_cell[i2++];
      continue;
    }
    fractions_both[nmerge++] = fractions[i1++];
    i2++;
  }
  *nxyz_cell = nmerge;
  NewMemory((void **)&xyz_cell, 3*nmerge*sizeof(float));
  NewMemory((void **)&cell_ind, nmerge*sizeof(int));
  *xyz_cellptr = xyz_cell;
  *cell_indptr = cell_ind;
  for(i = 0;i < nmerge;i++){
    GetHVACPathXYZ(fractions_both[i], xyz, nxyz, xyz_cell + 3*i);
  }
  for(i = 0;i < nmerge-1;i++){
    float frac_avg;

    frac_avg = (fractions_both[i] + fractions_both[i + 1]) / 2.0;
    cell_ind[i] = CLAMP((int)(frac_avg*(float)ncells), 0, ncells - 1);
  }
  FREEMEMORY(fractions);
  FREEMEMORY(fractions_cell);
  FREEMEMORY(fractions_both);
}

/* ------------------ SetDuctLabelSymbolXYZ ------------------------ */

void SetDuctLabelSymbolXYZ(hvacductdata *ducti){
  int j;
  float *xyz1, *xyz2;

  if(ducti->nxyz_reg == 2){
    xyz1 = ducti->node_from->xyz;
    xyz2 = ducti->node_to->xyz;
    for(j = 0;j < 3;j++){
      ducti->xyz_symbol[j] = 0.50 * xyz1[j] + 0.50 * xyz2[j];
      ducti->xyz_label[j]  = 0.25 * xyz1[j] + 0.75 * xyz2[j];
    }
  }
  else{
    GetHVACPathXYZ(0.50, ducti->xyz_reg, ducti->nxyz_reg, ducti->xyz_symbol);
    GetHVACPathXYZ(0.75, ducti->xyz_reg, ducti->nxyz_reg, ducti->xyz_label);
  }
}

/* ------------------ InitHvacData ------------------------ */

void InitHvacData(hvacvaldata *hi){
  hi->vals   = NULL;
  hi->nvals  = 0;
  hi->vis    = 0;
  hi->valmax = 1.0;
  hi->valmin = 0.0;
}

/* ------------------ ReadHVACData ------------------------ */

void ReadHVACData(int flag){
  FILE *stream = NULL;
  float *node_buffer = NULL, *duct_buffer = NULL, *times;
  int max_node_buffer = 0, max_duct_buffer = 0;
  int parms[4], n_nodes, n_node_vars, n_ducts, n_duct_vars;
  int frame_size, header_size, nframes;
  FILE_SIZE file_size;
  int i, iframe;
  int *duct_ncells;

  if(hvacvalsinfo == NULL)return;
  FREEMEMORY(hvacvalsinfo->times);

  for(i = 0;i < hvacvalsinfo->n_duct_vars;i++){
    hvacvaldata *hi;

    hi = hvacvalsinfo->duct_vars + i;
    FREEMEMORY(hi->vals);
  }
  
  for(i = 0;i < hvacvalsinfo->n_node_vars;i++){
    hvacvaldata *hi;

    hi = hvacvalsinfo->node_vars + i;
    FREEMEMORY(hi->vals);
  }
  if(flag==HVAC_UNLOAD)return;

  stream = fopen(hvacvalsinfo->file, "rb");
  if(stream == NULL)return;

 // WRITE(LU_HVAC)N_NODE_OUT, N_NODE_VARS, N_DUCT_OUT, N_DUCT_VARS
  FSEEK(stream, 4, SEEK_CUR); fread(parms, 4, 4, stream); FSEEK(stream, 4, SEEK_CUR);
  n_nodes      = parms[0];
  n_node_vars  = parms[1];
  n_ducts      = parms[2];
  n_duct_vars  = parms[3];
  header_size  = 4 + 4 * 4 + 4;                       // n_node_out n_node_vars n_duct_out n_ductd_vars
  header_size += 4 + 4 * n_ducts + 4;                 // number of cells in each duct
  frame_size   = 4 + 4 + 4;                           // time
  frame_size  += n_nodes * (4 + 4 * n_node_vars + 4); // node data
  frame_size  += n_ducts * (4 + 4 * n_duct_vars + 4); // duct data
  file_size    = GetFileSizeSMV(hvacvalsinfo->file);
  nframes      = (file_size - header_size) / frame_size;

  NewMemory((void **)&duct_ncells, n_ducts * sizeof(int));
  FSEEK(stream, 4, SEEK_CUR); fread(duct_ncells, 4, n_ducts, stream); FSEEK(stream, 4, SEEK_CUR);

  FREEMEMORY(hvacvalsinfo->times);
  NewMemory((void **)&hvacvalsinfo->times, nframes * sizeof(float));
  hvacvalsinfo->ntimes = nframes;

  hvac_maxcells = duct_ncells[0];
  for(i=1;i<n_ducts;i++){
    hvac_maxcells = MAX(hvac_maxcells, duct_ncells[i]);
  }
  hvac_n_ducts   = n_ducts;

  for(i = 0;i < hvacvalsinfo->n_duct_vars;i++){
    hvacvaldata *hi;

    hi = hvacvalsinfo->duct_vars + i;
    NewMemory((void **)&hi->vals, n_ducts*hvac_maxcells*nframes * sizeof(float));
  }
  for(i = 0;i < hvacvalsinfo->n_node_vars;i++){
    hvacvaldata *hi;

    hi = hvacvalsinfo->node_vars + i;
    FREEMEMORY(hi->vals);
    NewMemory((void **)&hi->vals, n_nodes*nframes * sizeof(float));
  }

  rewind(stream);
  FSEEK(stream, header_size, SEEK_CUR); // skip over header

  times = hvacvalsinfo->times;
  for(iframe=0;iframe<nframes;iframe++){
    int j;
    float time;

    FSEEK(stream, 4, SEEK_CUR); fread(&time, 4, 1, stream); FSEEK(stream, 4, SEEK_CUR);
    times[iframe] = time;
    if(n_node_vars > max_node_buffer){
      FREEMEMORY(node_buffer);
      NewMemory((void **)&node_buffer, (n_node_vars+100) * sizeof(float));
      max_node_buffer = n_node_vars + 100;
    }
    for(j = 0;j < n_nodes;j++){
      int k;

      FSEEK(stream, 4, SEEK_CUR); fread(node_buffer, 4, n_node_vars, stream); FSEEK(stream, 4, SEEK_CUR);
      for(k=0;k<n_node_vars;k++){
        hvacvaldata *hk;

        hk = hvacvalsinfo->node_vars + k;
        hk->vals[iframe + j * nframes] = node_buffer[k];
      }
    }
  
    int iduct;
    for(iduct = 0;iduct < n_ducts;iduct++){
      int ntotalvals;

      ntotalvals = n_duct_vars*duct_ncells[iduct];
      if(ntotalvals > max_duct_buffer){
        FREEMEMORY(duct_buffer);
        NewMemory((void **)&duct_buffer, (ntotalvals + 100) * sizeof(float));
        max_duct_buffer = ntotalvals + 100;
      }
      int icell;
      for(icell = 0;icell < duct_ncells[iduct];icell++){
        int ivar;

        FSEEK(stream, 4, SEEK_CUR); fread(duct_buffer, 4, n_duct_vars, stream); FSEEK(stream, 4, SEEK_CUR);
        for(ivar = 0;ivar < n_duct_vars;ivar++){
          hvacvaldata *hk;

          hk = hvacvalsinfo->duct_vars + ivar;
          hk->vals[HVACVAL(iframe,iduct,icell)] = duct_buffer[ivar];
        }
      }
    }
  }
  fclose(stream);
  FREEMEMORY(duct_buffer);
  FREEMEMORY(node_buffer);

  for(i = 0;i < n_node_vars;i++){
    hvacvaldata *hi;
    float *vals;
    int j;

    hi = hvacvalsinfo->node_vars+i;

    vals = hi->vals;
    hi->valmin = vals[0];
    hi->valmax = hi->valmin;
    for(j=1;j<nframes*n_nodes;j++){
      hi->valmin = MIN(vals[j],hi->valmin);
      hi->valmax = MAX(vals[j],hi->valmax);
    }
  }
  for(i = 0;i < n_duct_vars;i++){
    hvacvaldata *hi;
    int iduct;

    hi = hvacvalsinfo->duct_vars+i;

    hi->valmin = hi->vals[0];
    hi->valmax = hi->valmin;
    for(iduct=0;iduct<n_ducts;iduct++){
      int icell;

      for(icell=0;icell<duct_ncells[iduct];icell++){
	int iframe2;
	
        for(iframe2=0;iframe2<nframes;iframe2++){
          int index;

          index = HVACVAL(iframe2, iduct, icell);
          hi->valmin = MIN(hi->vals[index],hi->valmin);
          hi->valmax = MAX(hi->vals[index],hi->valmax);
        }
      }
    }
  }
  for(i = 0;i < n_node_vars;i++){
    hvacvaldata *hi;

    hi = hvacvalsinfo->node_vars+i;
    GetColorbarLabels(hi->valmin, hi->valmax,nrgb,hi->colorlabels,hi->levels256);
  }
  for(i = 0;i < n_duct_vars;i++){
    hvacvaldata *hi;

    hi = hvacvalsinfo->duct_vars+i;
    GetColorbarLabels(hi->valmin, hi->valmax, nrgb, hi->colorlabels, hi->levels256);
  }
  FREEMEMORY(duct_ncells);
}

/* ------------------ SetHVACInfo ------------------------ */

void SetHVACInfo(void){
  int i;

  int *hvac_sort_yz, *hvac_sort_xz, *hvac_sort_xy;
  int *line_yz, *line_xz, *line_xy;

  if(hvacconnectinfo == NULL){
    NewMemory((void **)&hvacconnectinfo, (nhvacnodeinfo+nhvacductinfo+1)*sizeof(hvacconnectdata));
    nhvacconnectinfo = 0;
    for(i = 0;i < nhvacductinfo;i++){
      hvacductdata *ducti;

      ducti = hvacductinfo + i;
      if(HaveHVACConnect(ducti->connect_id, hvacconnectinfo, nhvacconnectinfo) == 1)continue;
      hvacconnectinfo[nhvacconnectinfo].index = ducti->connect_id;
      hvacconnectinfo[nhvacconnectinfo].display = 0;
      nhvacconnectinfo++;
    }
    for(i = 0;i < nhvacnodeinfo;i++){
      hvacnodedata *nodei;

      nodei = hvacnodeinfo + i;
      if(HaveHVACConnect(nodei->connect_id, hvacconnectinfo, nhvacconnectinfo) == 1)continue;
      hvacconnectinfo[nhvacconnectinfo].index = nodei->connect_id;
      hvacconnectinfo[nhvacconnectinfo].display = 1;
      nhvacconnectinfo++;
    }
    if(nhvacconnectinfo > 0){
      ResizeMemory((void **)&hvacconnectinfo, nhvacconnectinfo * sizeof(hvacconnectdata));
      qsort((int *)hvacconnectinfo, nhvacconnectinfo, sizeof(hvacconnectdata), CompareHvacConnect);
      for(i = 0;i < nhvacductinfo;i++){
        hvacductdata *ducti;
        int j;

        ducti = hvacductinfo + i;
        ducti->connect = NULL;
        for(j = 0;j<nhvacconnectinfo;j++){
          hvacconnectdata *hj;

          hj = hvacconnectinfo + j;
          if(ducti->connect_id == hj->index){
            ducti->connect = hj;
            break;
          }
        }
      }
      for(i = 0;i < nhvacnodeinfo;i++){
        hvacnodedata *nodei;
        int j;

        nodei = hvacnodeinfo + i;
        nodei->connect = NULL;
        for(j = 0;j<nhvacconnectinfo;j++){
          hvacconnectdata *hj;

          hj = hvacconnectinfo + j;
          if(nodei->connect_id == hj->index){
            nodei->connect = hj;
            break;
          }
        }
      }
    }
    else{
      FREEMEMORY(hvacconnectinfo);
    }
  }
  if(hvac_offset_nodes==1){
    NewMemory((void **)&hvac_sort_yz, nhvacnodeinfo * sizeof(int));
    NewMemory((void **)&hvac_sort_xz, nhvacnodeinfo * sizeof(int));
    NewMemory((void **)&hvac_sort_xy, nhvacnodeinfo * sizeof(int));
    NewMemory((void **)&line_yz, nhvacnodeinfo * sizeof(int));
    NewMemory((void **)&line_xz, nhvacnodeinfo * sizeof(int));
    NewMemory((void **)&line_xy, nhvacnodeinfo * sizeof(int));
    for(i = 0;i < nhvacnodeinfo;i++){
      hvac_sort_yz[i] = i;
      hvac_sort_xz[i] = i;
      hvac_sort_xy[i] = i;
    }
    qsort((int *)hvac_sort_yz, nhvacnodeinfo, sizeof(int), CompareFloatYZ);
    qsort((int *)hvac_sort_xz, nhvacnodeinfo, sizeof(int), CompareFloatXZ);
    qsort((int *)hvac_sort_xy, nhvacnodeinfo, sizeof(int), CompareFloatXY);
    line_yz[0]=0;
    line_xz[0]=0;
    line_xy[0]=0;
    for(i=1;i<nhvacnodeinfo;i++){
      line_yz[i]=0;
      line_xz[i]=0;
      line_xy[i]=0;
      if(CompareFloatYZ(hvac_sort_yz + i, hvac_sort_yz + i - 1) == 0)line_yz[i]=1;
      if(CompareFloatYZ(hvac_sort_xz + i, hvac_sort_xz + i - 1) == 0)line_xz[i]=1;
      if(CompareFloatYZ(hvac_sort_xy + i, hvac_sort_xy + i - 1) == 0)line_xy[i]=1;
    }
    OffsetHvacNodes(line_yz, hvac_sort_yz, 0);
    OffsetHvacNodes(line_xz, hvac_sort_xz, 1);
    OffsetHvacNodes(line_xy, hvac_sort_xy, 2);
    FREEMEMORY(hvac_sort_yz);
    FREEMEMORY(hvac_sort_xz);
    FREEMEMORY(hvac_sort_xy);
    FREEMEMORY(line_yz);
    FREEMEMORY(line_xz);
    FREEMEMORY(line_xy);
  }
  else{
    for(i = 0;i < nhvacnodeinfo;i++){
      hvacnodedata *nodei;

      nodei = hvacnodeinfo + i;
      memcpy(nodei->xyz, nodei->xyz_orig, 3*sizeof(float));
    }
  }

  for(i = 0;i < nhvacductinfo;i++){
    int j;
    hvacductdata *ducti;
    hvacnodedata *from_i, *to_i;
    float *xyz_from;
    float *xyz_to;

    ducti  = hvacductinfo + i;
    from_i = ducti->node_from;
    to_i   = ducti->node_to;
    if(from_i == NULL || to_i == NULL)continue;
    xyz_from = from_i->xyz;
    xyz_to   = to_i->xyz;
    for(j = 0;j < nhvacductinfo;j++){
      hvacductdata *ductj;
      hvacnodedata *from_j, *to_j;
      float diff[3];

      if(i == j)continue;
      ductj = hvacductinfo + j;
      from_j = ductj->node_from;
      to_j   = ductj->node_to;
      if(from_j == NULL || to_j == NULL)continue;
      if(from_i != from_j && from_i != to_j && to_i != from_j && to_i != to_j)continue;
      diff[0] = ABS(xyz_from[0] - xyz_to[0]);
      diff[1] = ABS(xyz_from[1] - xyz_to[1]);
      diff[2] = ABS(xyz_from[2] - xyz_to[2]);
      if(diff[0] < MIN(diff[1], diff[2])){
        ducti->metro_path = DUCT_YZX;
        ductj->metro_path = DUCT_ZYX;
      }
      else if(diff[1] < MIN(diff[0], diff[2])){
        ducti->metro_path = DUCT_XZY;
        ductj->metro_path = DUCT_ZXY;
      }
      else{
        ducti->metro_path = DUCT_XYZ;
        ductj->metro_path = DUCT_YXZ;
      }
    }
  }
#define COPYVALS3(xyz, x, y, z) \
  *(xyz+0) = (x);\
  *(xyz+1) = (y);\
  *(xyz+2) = (z)
  for(i=0;i<nhvacductinfo;i++){
    hvacductdata *ducti;
    hvacnodedata *node_from, *node_to;
    float *xyz0, *xyz1;

    ducti = hvacductinfo + i;
    node_from = hvacnodeinfo + ducti->node_id_from;
    node_to = hvacnodeinfo + ducti->node_id_to;
    if(node_from == NULL || node_to == NULL)continue;
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
    memcpy(ducti->xyz_label_metro,  xyz0, 3 * sizeof(float));
    memcpy(ducti->xyz_met,          xyz0, 3 * sizeof(float));
    memcpy(ducti->xyz_met+9,        xyz1, 3 * sizeof(float));
    ducti->nxyz_met = 3;
    switch(ducti->metro_path){
      case DUCT_XYZ:
        COPYVALS3(ducti->xyz_met + 3, xyz1[0], xyz0[1], xyz0[2]);
        COPYVALS3(ducti->xyz_met + 6, xyz1[0], xyz1[1], xyz0[2]);
        if(ABS(dxyz[0]) > ABS(dxyz[1])){
          ducti->xyz_symbol_metro[0] += 0.5*dxyz[0];
          ducti->xyz_label_metro[0]  += 0.75*dxyz[0];
          dxyz_metro[0] = 1.0;
        }
        else{
          ducti->xyz_symbol_metro[0] += dxyz[0];
          ducti->xyz_symbol_metro[1] += 0.5*dxyz[1];
          ducti->xyz_label_metro[0]  += dxyz[0];
          ducti->xyz_label_metro[1]  += 0.75*dxyz[1];
          dxyz_metro[1] = 1.0;
        }
        break;
      case DUCT_XZY:
        COPYVALS3(ducti->xyz_met + 3, xyz1[0], xyz0[1], xyz0[2]);
        COPYVALS3(ducti->xyz_met + 6, xyz1[0], xyz0[1], xyz1[2]);
        if(ABS(dxyz[0]) > ABS(dxyz[2])){
          ducti->xyz_symbol_metro[0] += 0.50*dxyz[0];
          ducti->xyz_label_metro[0]  += 0.75*dxyz[0];
          dxyz_metro[0] = 1.0;
        }
        else{
          ducti->xyz_symbol_metro[0] += dxyz[0];
          ducti->xyz_symbol_metro[2] += 0.5*dxyz[2];
          ducti->xyz_label_metro[0]  += dxyz[0];
          ducti->xyz_label_metro[2]  += 0.75*dxyz[2];
          dxyz_metro[2] = 1.0;
        }
        break;
      case DUCT_YXZ:
        COPYVALS3(ducti->xyz_met + 3, xyz0[0], xyz1[1], xyz0[2]);
        COPYVALS3(ducti->xyz_met + 6, xyz1[0], xyz1[1], xyz0[2]);
        if(ABS(dxyz[1]) > ABS(dxyz[0])){
          ducti->xyz_symbol_metro[1] += 0.50*dxyz[1];
          ducti->xyz_label_metro[1]  += 0.75*dxyz[1];
          dxyz_metro[1] = 1.0;
        }
        else{
          ducti->xyz_symbol_metro[0] += 0.50*dxyz[0];
          ducti->xyz_symbol_metro[1] += dxyz[1];
          ducti->xyz_label_metro[0]  += 0.75*dxyz[0];
          ducti->xyz_label_metro[1]  += dxyz[1];
          dxyz_metro[0] = 1.0;
        }
        break;
      case DUCT_YZX:
        COPYVALS3(ducti->xyz_met + 3, xyz0[0], xyz1[1], xyz0[2]);
        COPYVALS3(ducti->xyz_met + 6, xyz0[0], xyz1[1], xyz1[2]);
        if(ABS(dxyz[1]) > ABS(dxyz[2])){
          ducti->xyz_symbol_metro[1] += 0.50*dxyz[1];
          ducti->xyz_label_metro[1]  += 0.75*dxyz[1];
          dxyz_metro[1] = 1.0;
        }
        else{
          ducti->xyz_symbol_metro[1] += dxyz[1];
          ducti->xyz_symbol_metro[2] += 0.50*dxyz[2];
          ducti->xyz_label_metro[1]  += dxyz[1];
          ducti->xyz_label_metro[2]  += 0.75*dxyz[2];
          dxyz_metro[2] = 1.0;
        }
        break;
      case DUCT_ZXY:
        COPYVALS3(ducti->xyz_met + 3, xyz0[0], xyz0[1], xyz1[2]);
        COPYVALS3(ducti->xyz_met + 6, xyz1[0], xyz0[1], xyz1[2]);
        if(ABS(dxyz[2]) > ABS(dxyz[0])){
          ducti->xyz_symbol_metro[2] += 0.50*dxyz[2];
          ducti->xyz_label_metro[2]  += 0.75*dxyz[2];
          dxyz_metro[2] = 1.0;
        }
        else{
          ducti->xyz_symbol_metro[0] += 0.50*dxyz[0];
          ducti->xyz_symbol_metro[2] += dxyz[2];
          ducti->xyz_label_metro[0]  += 0.75 * dxyz[0];
          ducti->xyz_label_metro[2]  += dxyz[2];
          dxyz_metro[0] = 1.0;
        }
        break;
      case DUCT_ZYX:
        COPYVALS3(ducti->xyz_met + 3, xyz0[0], xyz0[1], xyz1[2]);
        COPYVALS3(ducti->xyz_met + 6, xyz0[0], xyz1[1], xyz1[2]);
        if(ABS(dxyz[2]) > ABS(dxyz[1])){
          ducti->xyz_symbol_metro[2] += 0.50*dxyz[2];
          ducti->xyz_label_metro[2]  += 0.75 * dxyz[2];
          dxyz_metro[2] = 1.0;
        }
        else{
          ducti->xyz_symbol_metro[1] += 0.50*dxyz[1];
          ducti->xyz_symbol_metro[2] += dxyz[2];
          ducti->xyz_label_metro[1]  += 0.75 * dxyz[1];
          ducti->xyz_label_metro[2]  += dxyz[2];
          dxyz_metro[1] = 1.0;
        }
        break;
      default:
        ASSERT(FFALSE);
        break;
    }
    GetCellXYZs(ducti->xyz_reg, ducti->nxyz_reg, ducti->nduct_cells,
      &ducti->xyz_reg_cell, &ducti->nxyz_reg_cell, &ducti->cell_reg);
    GetCellXYZs(ducti->xyz_met, ducti->nxyz_met, ducti->nduct_cells,
      &ducti->xyz_met_cell, &ducti->nxyz_met_cell, &ducti->cell_met);
  }
  for(i = 0;i < nhvacductinfo;i++){
    SetDuctLabelSymbolXYZ(hvacductinfo + i);
  }
}

/* ------------------ GetHVACState ------------------------ */

int GetHVACDuctState(hvacductdata *ducti){
  int i, n, *states;
  float *times;
  float current_time;

  if(global_times==NULL)return HVAC_STATE_INACTIVE;
  times = ducti->act_times;
  states = ducti->act_states;
  n = ducti->nact_times;
  if(n==0||times==NULL)return HVAC_STATE_INACTIVE;

  current_time = GetTime();

  if(current_time < times[0])return HVAC_STATE_INACTIVE;
  for(i = 0;i < n - 1;i++){
    if(current_time >= times[i] && current_time < times[i + 1])return states[i];
  }
  return states[n - 1];
}

/* ------------------ GetDuctDir ------------------------ */

int GetDuctDir(float *xyz){
  float eyedir[3];

  //cos(angle) = dir1 .dot. dir2 /(norm(dir1)*norm(dir2))
  eyedir[0] = ABS(xyz[0] - fds_eyepos[0]);
  eyedir[1] = ABS(xyz[1] - fds_eyepos[1]);
  eyedir[2] = ABS(xyz[2] - fds_eyepos[2]);
  if(eyedir[0]>MAX(eyedir[1],eyedir[2]))return 1;
  if(eyedir[1]>MAX(eyedir[0],eyedir[2]))return 0;
  return 2;
}

/* ------------------ DrawHVACDamper ------------------------ */

void DrawHVACDamper(hvacductdata *ducti, float *xyz, float diam, int state){
  float cyl_diam, cyl_height;
  unsigned char color2[3] = {0, 0, 0};
  unsigned char *color;
  float axis[3];
  float u[3] = {0.0, 0.0, 1.0}, *v, angle;

  color = hvac_color_states[state];
  color2[0] = CLAMP(255 * foregroundcolor[0], 0, 255);
  color2[1] = CLAMP(255 * foregroundcolor[1], 0, 255);
  color2[2] = CLAMP(255 * foregroundcolor[2], 0, 255);

  cyl_diam = diam / 4.0;
  cyl_height = 3.0 * diam;
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);

  if(hvac_metro_view == 1){
    v = ducti->normal_metro;
  }
  else{
    v = ducti->normal;
  }

  RotateU2V(u, v, axis, &angle);
  angle *= 180.0 / 3.14159;
  if(state == 0){
    glRotatef(-45.0, 0.0, 1.0, 0.0);
  }
  else{
    glRotatef(-90.0, 0.0, 1.0, 0.0);
  }

  glRotatef(angle, axis[0], axis[1], axis[2]);

  glTranslatef(0.0, 0.0, -cyl_height/2.0);
  DrawDisk(cyl_diam, cyl_height, color2);
  glPopMatrix();
}

/* ------------------ DrawHVACAircoil ------------------------ */

void DrawHVACAircoil(hvacductdata *ducti, float *xyz, float size, float diam, int state){
  unsigned char *color;
  float axis[3];
  float u[3] = {1.0, 0.0, 0.0}, *v, angle;

  color = hvac_color_states[state];

  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);

  if(hvac_metro_view == 1){
    v = ducti->normal_metro;
  }
  else{
    v = ducti->normal;
  }
  RotateU2V(u, v, axis, &angle);
  angle *= 180.0 / 3.14159;
  glRotatef(angle, axis[0], axis[1], axis[2]);

  glLineWidth(2.0);
  glColor3fv(foregroundcolor);
  glScalef(size, size, size);
  glBegin(GL_LINES);
  glVertex3f(-1.0,  0.0,  0.0);
  glVertex3f(-0.75, 0.0, -0.5);

  glVertex3f(-0.75, 0.0, -0.5);
  glVertex3f(-0.25, 0.0,  0.5);

  glVertex3f(-0.25, 0.0,  0.5);
  glVertex3f( 0.25, 0.0, -0.5);

  glVertex3f(0.25, 0.0, -0.5);
  glVertex3f(0.75, 0.0,  0.5);

  glVertex3f(0.75, 0.0, 0.5);
  glVertex3f(1.0 , 0.0, 0.0);
  glEnd();
  glPopMatrix();
}

/* ------------------ DrawHVACFan ------------------------ */

void DrawHVACFan(hvacductdata *ducti, float *xyz, float size, float diam, int state){
  int i;
  unsigned char *color;
  float axis[3];
  float u[3] = {1.0, 0.0, 0.0}, *v, angle;

  color = hvac_color_states[state];
  if(hvac_circ_x == NULL||hvac_circ_y==NULL){
    FREEMEMORY(hvac_circ_x);
    FREEMEMORY(hvac_circ_y);
    NewMemory((void **)&hvac_circ_x,2*HVAC_NCIRC*sizeof(float));
    NewMemory((void **)&hvac_circ_y,2*HVAC_NCIRC*sizeof(float));
    for(i=0;i<HVAC_NCIRC;i++){
      float arg;
      float r, xx, yy;

      arg = 2.0*PI*(float)i/(float)(HVAC_NCIRC-1);
      hvac_circ_x[i] = cos(arg);
      hvac_circ_y[i] = sin(arg);
      r = sin(2.0 * arg);
      xx = r * cos(arg);
      yy = r * sin(arg);
      hvac_circ_x[i + HVAC_NCIRC] = xx;
      hvac_circ_y[i + HVAC_NCIRC] = yy;
    }
  }
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);
  glLineWidth(2.0);

  if(hvac_metro_view == 1){
    v = ducti->normal_metro;
  }
  else{
    v = ducti->normal;
  }
  RotateU2V(u, v, axis, &angle);
  angle *= 180.0 / 3.14159;
  glRotatef(angle, axis[0], axis[1], axis[2]);
  glRotatef(90, 0.0,0.0,1.0);
  if(state==1&&global_times != NULL){
    float angle2, time2;

    time2 = global_times[itimes];
    angle2 = 360.0*time2 / 10.0;
    glRotatef(angle2, 0.0, 1.0, 0.0);
  }

  glScalef(size,size,size);
  glColor3fv(foregroundcolor);
  glBegin(GL_LINES);
  for(i=0;i<2*HVAC_NCIRC-1;i++){
    float x, y, xp1, yp1;

    if(i == HVAC_NCIRC - 1)continue;
    x   = hvac_circ_x[i];
    xp1 = hvac_circ_x[i+1];
    y   = hvac_circ_y[i];
    yp1 = hvac_circ_y[i+1];
    glVertex3f(x,   0.0, y);
    glVertex3f(xp1, 0.0, yp1);
  }
  glEnd();
  glPopMatrix();
}

/* ------------------ DrawHVACFilter ------------------------ */

void DrawHVACFilter(hvacductdata *ducti, float *xyz, float size){
  float axis[3];
  float u[3] = {0.0, 1.0, 0.0}, *v, angle;

  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  if(ducti!=NULL){
    if(hvac_metro_view == 1){
      v = ducti->normal_metro;
    }
    else{
      v = ducti->normal;
    }
    RotateU2V(u, v, axis, &angle);
    angle *= 180.0 / 3.14159;
    glRotatef(angle, axis[0], axis[1], axis[2]);
  }
  glRotatef(90.0, 0.0, 0.0, 1.0);
  glLineWidth(2.0);
  glScalef(size,size,size);
  glBegin(GL_LINES);
  glVertex3f(0.0, -0.5, -1.0);
  glVertex3f(0.0,  0.5, -1.0);

  glVertex3f(0.0, -0.5, -0.5);
  glVertex3f(0.0,  0.5, -0.5);

  glVertex3f(0.0, -0.5,  0.0);
  glVertex3f(0.0,  0.5,  0.0);

  glVertex3f(0.0, -0.5,  0.5);
  glVertex3f(0.0,  0.5,  0.5);

  glVertex3f(0.0, -0.5,  1.0);
  glVertex3f(0.0,  0.5,  1.0);

  glVertex3f(0.0, -0.5, -1.0);
  glVertex3f(0.0, -0.5,  1.0);
  
  glVertex3f(0.0,  0.5, -1.0);
  glVertex3f(0.0,  0.5,  1.0);
  glEnd();
  glPopMatrix();
}

/* ------------------ GetHVACDuctID ------------------------ */

hvacductdata *GetHVACDuctID(char *duct_name){
  int i;

  for(i = 0;i < nhvacductinfo;i++){
    hvacductdata *ducti;

    ducti = hvacductinfo + i;
    if(strcmp(ducti->duct_name, duct_name) == 0)return ducti;
  }
  return NULL;
}

/* ------------------ GetHVACNodeID ------------------------ */

hvacnodedata *GetHVACNodeID(char *node_name){
  int i;

  for(i = 0;i < nhvacnodeinfo;i++){
    hvacnodedata *nodei;

    nodei = hvacnodeinfo + i;
    if(strcmp(nodei->duct_name, node_name) == 0)return nodei;
  }
  return NULL;
}

/* ------------------ DrawHVAC ------------------------ */

void DrawHVAC(hvacdata *hvaci){
  int i, frame_index=0;
  unsigned char uc_color[3];
  float *last_color = NULL;

  if((hvacductvar_index >= 0||hvacnodevar_index>=0)&&global_times!=NULL){
    frame_index = GetTimeInterval(GetTime(), hvacvalsinfo->times, hvacvalsinfo->ntimes);
  }

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);

  // draw ducts
  
  glLineWidth(hvaci->duct_width);
  glBegin(GL_LINES);
  if(hvacductvar_index < 0||global_times==NULL){
    uc_color[0] = CLAMP(hvaci->duct_color[0], 0, 255);
    uc_color[1] = CLAMP(hvaci->duct_color[1], 0, 255);
    uc_color[2] = CLAMP(hvaci->duct_color[2], 0, 255);
    glColor3ubv(uc_color);
  }
  for(i = 0; i < nhvacductinfo; i++){
    hvacductdata *ducti;
    hvacnodedata *node_from, *node_to;
 
    ducti = hvacductinfo + i;
    if(hvac_show_networks==1&&strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
    if(hvac_show_connections==1&&ducti->connect != NULL && ducti->connect->display == 0)continue;

    node_from = hvacnodeinfo + ducti->node_id_from;
    node_to = hvacnodeinfo + ducti->node_id_to;
    if(node_from == NULL || node_to == NULL)continue;
    float *xyzs;
    int nxyzs, j, *cell_index;
    
    if(hvac_metro_view == 1){
      xyzs       = ducti->xyz_met_cell;
      cell_index = ducti->cell_met;
      nxyzs      = ducti->nxyz_met_cell-1;
    }
    else{
      xyzs       = ducti->xyz_reg_cell;
      cell_index = ducti->cell_reg;
      nxyzs      = ducti->nxyz_reg_cell-1;
    }
    if(global_times != NULL && hvacductvar_index >= 0){
      for(j = 0;j < nxyzs;j++){
        hvacvaldata *ductvar;
        float *xyz, *this_color;
        int cell;
        unsigned char ival;

        xyz = xyzs + 3 * j;
        cell = cell_index[j];
        ductvar = hvacvalsinfo->duct_vars + hvacductvar_index;

        int index = HVACVAL(frame_index, i, cell);
        ival = HVACCOLORCONV(ductvar->vals[index], ductvar->valmin, ductvar->valmax);
        this_color = rgb_full[ival];
        if(this_color != last_color){
          last_color = this_color;
          glColor3fv(this_color);
        }
        glVertex3fv(xyz);
        glVertex3fv(xyz+3);
      }
    }
    else{
      for(j = 0;j < nxyzs;j++){
        float *xyz;

        xyz = xyzs + 3 * j;
        glVertex3fv(xyz);
        glVertex3fv(xyz+3);
      }
    }
  }
  glEnd();

  if(hvac_cell_view==1){
    glPointSize(hvaci->node_size);
    glBegin(GL_POINTS);
    for(i = 0; i < nhvacductinfo; i++){
      hvacductdata *ducti;
      float *xyzs;
      int nxyzs, j, *cell_index;
 
      ducti = hvacductinfo + i;
      if(ducti->nduct_cells <= 1)continue;
    
      if(hvac_metro_view == 1){
        xyzs       = ducti->xyz_met_cell;
        cell_index = ducti->cell_met;
        nxyzs      = ducti->nxyz_met_cell-1;
      }
      else{
        xyzs       = ducti->xyz_reg_cell;
        cell_index = ducti->cell_reg;
        nxyzs      = ducti->nxyz_reg_cell-1;
      }
      for(j = 0;j < nxyzs;j++){
        float *xyz;

        xyz = xyzs + 3 * j;
        glVertex3fv(xyz);
      }
    }
    glEnd();
  }
  SNIFF_ERRORS("after hvac network");
  if(hvaci->show_duct_labels == 1){
    for(i = 0; i < nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];
      char label[256];
      float offset;

      ducti = hvacductinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
      if(hvac_show_connections == 1 && ducti->connect != NULL && ducti->connect->display == 0)continue;

      strcpy(label, ducti->duct_name);
      node_from = hvacnodeinfo + ducti->node_id_from;
      node_to   = hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      if(hvac_metro_view==1){
        memcpy(xyz, ducti->xyz_label_metro, 3*sizeof(float));
      }
      else{
        memcpy(xyz, ducti->xyz_label,       3*sizeof(float));
      }
      offset = 0.01/xyzmaxdiff;
      Output3Text(foregroundcolor, xyz[0]+offset, xyz[1]+offset, xyz[2]+offset, label);
    }
  }
  if(hvaci->show_component == DUCT_COMPONENT_TEXT){
    for(i = 0; i < nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];
      char label[256];

      ducti = hvacductinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
      if(hvac_show_connections == 1 && ducti->connect != NULL && ducti->connect->display == 0)continue;

      strcpy(label, "");
      strcat(label, ducti->c_component);
      node_from = hvacnodeinfo + ducti->node_id_from;
      node_to   = hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      if(hvac_metro_view == 1){
        memcpy(xyz, ducti->xyz_symbol_metro, 3*sizeof(float));
      }
      else{
        memcpy(xyz, ducti->xyz_symbol,       3*sizeof(float));
      }
      xyz[2] += 0.01 / xyzmaxdiff;
      Output3Text(foregroundcolor, xyz[0], xyz[1], xyz[2]+0.01/xyzmaxdiff, label);
    }
  }
  if(hvaci->show_component == DUCT_COMPONENT_SYMBOLS){
    for(i = 0; i < nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float *xyz;

      ducti = hvacductinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
      if(hvac_show_connections == 1 && ducti->connect != NULL && ducti->connect->display == 0)continue;
      node_from = hvacnodeinfo + ducti->node_id_from;
      node_to   = hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      float size;
      int state;

      state = GetHVACDuctState(ducti);
      size  = xyzmaxdiff / 40.0;
      size *= hvaci->component_size;
      if(hvac_metro_view == 1){
        xyz = ducti->xyz_symbol_metro;
      }
      else{
        xyz = ducti->xyz_symbol;
      }
      switch(ducti->component){
      case HVAC_NONE:
        break;
      case HVAC_FAN:
        DrawHVACFan(ducti, xyz, 0.75*2.0*size, 0.75*size, state);
        break;
      case HVAC_AIRCOIL:
        DrawHVACAircoil(ducti, xyz, 2.0*size, size, state);
        break;
      case HVAC_DAMPER:
        DrawHVACDamper(ducti, xyz, size, state);
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
    }
  }

  // draw nodes
  glPointSize(hvaci->node_size);
  glBegin(GL_POINTS);
  if(hvacnodevar_index < 0 || global_times == NULL){
    uc_color[0] = CLAMP(hvaci->node_color[0], 0, 255);
    uc_color[1] = CLAMP(hvaci->node_color[1], 0, 255);
    uc_color[2] = CLAMP(hvaci->node_color[2], 0, 255);
    glColor3ubv(uc_color);
  }
  for(i = 0; i < nhvacnodeinfo; i++){
    hvacnodedata *nodei;

    nodei = hvacnodeinfo + i;
    if(hvac_show_networks == 1 && strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
    if(hvac_show_connections == 1 && nodei->connect != NULL && nodei->connect->display == 0)continue;
    if(global_times != NULL && hvacnodevar_index >= 0){
      hvacvaldata *nodevar;
      unsigned char ival;

      //        duct: i
      //        time: itime
      //   var index: hvacductvar_index
      nodevar = hvacvalsinfo->node_vars + hvacnodevar_index;
      ival = HVACCOLORCONV(nodevar->vals[frame_index + i * hvacvalsinfo->ntimes], nodevar->valmin, nodevar->valmax);
      glColor3fv(rgb_full[ival]);
    }
    glVertex3fv(nodei->xyz);
  }
  glEnd();

  uc_color[0] = CLAMP(hvaci->node_color[0], 0, 255);
  uc_color[1] = CLAMP(hvaci->node_color[1], 0, 255);
  uc_color[2] = CLAMP(hvaci->node_color[2], 0, 255);
  glColor3ubv(uc_color);
  if(hvaci->show_node_labels == 1){
    for(i = 0; i < nhvacnodeinfo; i++){
      hvacnodedata* nodei;
      char label[256];
      float offset;

      nodei = hvacnodeinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      if(hvac_show_connections == 1 && nodei->connect != NULL && nodei->connect->display == 0)continue;
      offset = 0.01/xyzmaxdiff;
      strcpy(label, nodei->node_name);
      Output3Text(foregroundcolor, nodei->xyz[0]+offset, nodei->xyz[1]+offset, nodei->xyz[2]+offset, label);
    }
  }
  if(hvaci->show_filters == NODE_FILTERS_LABELS){
    for(i = 0; i < nhvacnodeinfo; i++){
      hvacnodedata *nodei;
      char label[256];
      float offset;

      nodei = hvacnodeinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      if(hvac_show_connections == 1 && nodei->connect != NULL && nodei->connect->display == 0)continue;
      strcpy(label, nodei->c_filter);
      offset = 0.01 / xyzmaxdiff;
      Output3Text(foregroundcolor, nodei->xyz[0]+offset, nodei->xyz[1] + offset, nodei->xyz[2] + offset, label);
    }
  }
  if(hvaci->show_filters == NODE_FILTERS_SYMBOLS){
    for(i = 0; i < nhvacnodeinfo; i++){
      hvacnodedata *nodei;
      float size;

      size  = xyzmaxdiff / 25.0;
      size *= hvaci->filter_size;
      nodei = hvacnodeinfo + i;
      if(hvac_show_networks == 1 && strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      if(hvac_show_connections == 1 && nodei->connect != NULL && nodei->connect->display == 0)continue;
      if(nodei->filter == HVAC_FILTER_NO)continue;
      DrawHVACFilter(nodei->duct, nodei->xyz, size);
    }
  }
  glPopMatrix();
}

/* ------------------ DrawHVACS ------------------------ */

void DrawHVACS(void){
  int i;

  for(i=0; i<nhvacinfo; i++){
    hvacdata *hvaci;

    hvaci = hvacinfo + i;
    if(hvac_show_networks==1&&hvaci->display==0)continue;
    DrawHVAC(hvaci);
  }
}
