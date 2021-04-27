#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "smokeviewvars.h"
#include "IOscript.h"

/* ------------------ GetPartFileBounds ------------------------ */

int GetPartFileBounds(char *file, float *valmin, float *valmax){
  FILE *stream;
  int i;

  if(file==NULL||strlen(file)==0)return 0;
  stream = fopen(file, "r");
  if(stream==NULL)return 0;

  while(!feof(stream)){
    char buffer[255];
    float time;
    int class_index, nclasses;

    CheckMemory;
    if(fgets(buffer, 255, stream)==NULL)break;
    sscanf(buffer, "%f %i", &time, &nclasses);

    for(class_index = 0; class_index<nclasses; class_index++){
      int nfilebounds;

      if(fgets(buffer, 255, stream)==NULL)break;
      sscanf(buffer, "%i", &nfilebounds);

      for(i = 0; i<nfilebounds; i++){
        float vmin, vmax;
        int prop_index;

        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, "%f %f", &vmin, &vmax);

        prop_index = GetPartPropIndex(class_index, i+2);
        if(vmin<=vmax){
          if(valmin[prop_index]>valmax[prop_index]){
            valmin[prop_index] = vmin;
            valmax[prop_index] = vmax;
          }
          else{
            valmin[prop_index] = MIN(vmin, valmin[prop_index]);
            valmax[prop_index] = MAX(vmax, valmax[prop_index]);
          }
        }
      }
    }
  }
  fclose(stream);
  return 1;
}

/* ------------------ GetGlobalPartBounds ------------------------ */

int GetGlobalPartBounds(int flag){
  int i;
  float *partmins = NULL, *partmaxs = NULL;
  int nloaded_files = 0;

  for(i = 0; i<npartinfo; i++){
    partdata *parti;
    int j;

    parti = partinfo+i;
    if(parti->loaded==1)nloaded_files++;

    if(npart5prop>0){
      if(parti->valmin_fds==NULL)NewMemory((void **)&parti->valmin_fds, npart5prop*sizeof(float));
      if(parti->valmax_fds==NULL)NewMemory((void **)&parti->valmax_fds, npart5prop*sizeof(float));
      if(parti->valmin_smv==NULL)NewMemory((void **)&parti->valmin_smv, npart5prop*sizeof(float));
      if(parti->valmax_smv==NULL)NewMemory((void **)&parti->valmax_smv, npart5prop*sizeof(float));
      for(j = 0; j<npart5prop; j++){
        parti->valmin_fds[j] = 1.0;
        parti->valmax_fds[j] = 0.0;
        parti->valmin_smv[j] = 1.0;
        parti->valmax_smv[j] = 0.0;
      }
    }
    parti->have_bound_file = GetPartFileBounds(parti->bound_file, parti->valmin_fds, parti->valmax_fds);
  }
  if(npart5prop>0){
    NewMemory((void **)&partmins, npart5prop*sizeof(float));
    NewMemory((void **)&partmaxs, npart5prop*sizeof(float));
  }
  for(i = 0; i<npart5prop; i++){
    partmins[i] = 1.0;
    partmaxs[i] = 0.0;
  }
  for(i = 0; i<npartinfo; i++){
    partdata *parti;
    int j;
    float *valmin_fds, *valmax_fds;

    parti = partinfo+i;
    if(flag==LOADED_FILES&&parti->loaded==0)continue;
    valmin_fds = parti->valmin_fds;
    valmax_fds = parti->valmax_fds;
    for(j = 0; j<npart5prop; j++){
      if(valmin_fds[j]<=valmax_fds[j]){
        if(partmins[j]>partmaxs[j]){
          partmins[j] = valmin_fds[j];
          partmaxs[j] = valmax_fds[j];
        }
        else{
          partmins[j] = MIN(valmin_fds[j], partmins[j]);
          partmaxs[j] = MAX(valmax_fds[j], partmaxs[j]);
        }
      }
    }
  }
  for(i = 1; i<npart5prop; i++){
    if(flag==ALL_FILES){
      part5propinfo[i].user_min          = partmins[i];
      part5propinfo[i].dlg_global_valmin = partmins[i];
      part5propinfo[i].user_max          = partmaxs[i];
      part5propinfo[i].dlg_global_valmax = partmaxs[i];
    }
  }
  npartbounds_cpp = npart5prop;
  if(npartbounds_cpp>0&&partbounds_cpp==NULL){ // only initialize once
    NewMemory((void **)&partbounds_cpp, npartbounds_cpp*sizeof(cpp_boundsdata));
    for(i = 0; i<npartbounds_cpp; i++){
      cpp_boundsdata *boundscppi;
      float valmin, valmax;

      boundscppi = partbounds_cpp+i;
      strcpy(boundscppi->label, part5propinfo[i].label->shortlabel);
      {
        char *unit;

        unit = part5propinfo[i].label->unit;
        if(unit!=NULL){
          strcpy(boundscppi->unit, unit);
        }
        else{
          strcpy(boundscppi->unit, "");
        }
      }

      boundscppi->cache = cache_part_data;
      boundscppi->set_valtype = 0;
      if(i==0){
        valmin = 0.0;
        valmax = 1.0;
      }
      else{
        valmin = part5propinfo[i].dlg_global_valmin;
        valmax = part5propinfo[i].dlg_global_valmax;
      }

      boundscppi->set_valmin = 0;
      boundscppi->valmin[BOUND_SET_MIN]        = valmin;
      boundscppi->valmin[BOUND_LOADED_MIN]     = valmin;
      boundscppi->valmin[BOUND_GLOBAL_MIN]     = valmin;
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = valmin;

      boundscppi->valmax[BOUND_SET_MAX]        = valmax;
      boundscppi->valmax[BOUND_LOADED_MAX]     = valmax;
      boundscppi->valmax[BOUND_GLOBAL_MAX]     = valmax;
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = valmax;
      boundscppi->set_valmax = 0;

      boundscppi->set_chopmin = 0;
      boundscppi->set_chopmax = 0;
      boundscppi->chopmin     = valmin;
      boundscppi->chopmax     = valmax;

      boundscppi->hist = NULL;
    }
  }
  FREEMEMORY(partmins);
  FREEMEMORY(partmaxs);
  return nloaded_files;
}

/* ------------------ GetPatchBoundsInfo ------------------------ */

boundsdata *GetPatchBoundsInfo(char *shortlabel){
  int i;

  for(i = 0; i<npatchbounds; i++){
    boundsdata *boundi;

    boundi = patchbounds+i;
    if(strcmp(boundi->shortlabel, shortlabel)==0)return boundi;
  }
  return NULL;
}

/* ------------------ WriteFileBounds ------------------------ */

int WriteFileBounds(char *file, float valmin, float valmax){
  FILE *stream;

  stream = fopen(file, "w");
  if(stream==NULL)return 0;
  fprintf(stream," %f %f %f", 0.0, valmin, valmax);
  fclose(stream);
  return 1;
}

/* ------------------ GetFileBounds ------------------------ */

int GetFileBounds(char *file, float *valmin, float *valmax){
  FILE *stream;
  char buffer[255];
  float t, vmin, vmax;

  stream = fopen(file, "r");
  if(stream==NULL||fgets(buffer, 255, stream)==NULL){
    *valmin = 1.0;
    *valmax = 0.0;
    if(stream!=NULL)fclose(stream);
    return 0;
  }
  sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
  *valmin = vmin;
  *valmax = vmax;
  for(;;){
    if(fgets(buffer, 255, stream)==NULL)break;
    sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
    *valmin = MIN(*valmin, vmin);
    *valmax = MAX(*valmax, vmax);
  }
  fclose(stream);
  return 1;
}

/* ------------------ GetGlobalPatchBounds ------------------------ */

void GetGlobalPatchBounds(void){
  int i;

  for(i = 0; i < npatchbounds; i++){
    boundsdata *boundi;

    boundi = patchbounds + i;
    boundi->dlg_global_valmin = 1.0;
    boundi->dlg_global_valmax = 0.0;
  }
  for(i = 0; i < npatchinfo; i++){
    patchdata *patchi;
    float valmin, valmax;
    boundsdata *boundi;

    patchi = patchinfo + i;

    if(patchi->valmin_fds>patchi->valmax_fds||
       current_script_command==NULL||current_script_command->command!=SCRIPT_LOADSLICERENDER){
      if(GetFileBounds(patchi->bound_file, &valmin, &valmax)==1){
        patchi->have_bound_file = YES;
      }
      if(valmin > valmax)continue;
      patchi->valmin_fds = valmin;
      patchi->valmax_fds = valmax;
    }
    else{
      valmin = patchi->valmin_fds;
      valmax = patchi->valmax_fds;
    }
    boundi = GetPatchBoundsInfo(patchi->label.shortlabel);
    if(boundi == NULL)continue;
    if(boundi->dlg_global_valmin > boundi->dlg_global_valmax){
      boundi->dlg_global_valmin = valmin;
      boundi->dlg_global_valmax = valmax;
    }
    else{
      boundi->dlg_global_valmin = MIN(boundi->dlg_global_valmin, valmin);
      boundi->dlg_global_valmax = MAX(boundi->dlg_global_valmax, valmax);
    }
  }
  for(i = 0; i < npatchbounds; i++){
    boundsdata *boundi;
    int j;

    boundi = patchbounds + i;
    boundi->dlg_valmin = boundi->dlg_global_valmin;
    boundi->dlg_valmax = boundi->dlg_global_valmax;
    for(j = 0; j < npatchinfo; j++){
      patchdata *patchj;

      patchj = patchinfo + j;
      if(strcmp(patchj->label.shortlabel, boundi->shortlabel) == 0){
        patchj->valmin = boundi->dlg_global_valmin;
        patchj->valmax = boundi->dlg_global_valmax;
      }
    }
  }

  npatchbounds_cpp = npatchbounds;
  if(npatchbounds_cpp>0&&patchbounds_cpp==NULL){ // only initialize once
    NewMemory((void **)&patchbounds_cpp, npatchbounds_cpp*sizeof(cpp_boundsdata));
    for(i = 0; i<npatchbounds_cpp; i++){
      cpp_boundsdata *boundscppi;
      boundsdata *boundi;

      boundscppi = patchbounds_cpp+i;
      boundi = patchbounds+i;
      strcpy(boundscppi->label, boundi->shortlabel);
      strcpy(boundscppi->unit, boundi->label->unit);

      boundscppi->cache = cache_boundary_data;
      boundscppi->set_valtype = 0;

      boundscppi->set_valmin = 0;
      boundscppi->valmin[BOUND_SET_MIN]        = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_LOADED_MIN]     = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_GLOBAL_MIN]     = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = boundi->dlg_global_valmin;

      boundscppi->valmax[BOUND_SET_MAX]        = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_LOADED_MAX]     = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_GLOBAL_MAX]     = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = boundi->dlg_global_valmax;
      boundscppi->set_valmax = 0;

      boundscppi->set_chopmin = boundi->setchopmin;
      boundscppi->set_chopmax = boundi->setchopmax;
      boundscppi->chopmin     = boundi->chopmin;
      boundscppi->chopmax     = boundi->chopmax;

      boundscppi->hist = NULL;
    }
  }
}

/* ------------------ GetPlot3DFileBounds ------------------------ */

void GetPlot3DFileBounds(char *file, float *valmin, float *valmax){
  FILE *stream;
  int i, compute_bounds = 0;
  char buffer[255];

  for(i = 0; i<MAXPLOT3DVARS; i++){
    if(valmin[i]>valmax[i]){
      compute_bounds = 1;
      break;
    }
  }
  if(compute_bounds==0)return;
  if(file==NULL||strlen(file)==0)return;
  stream = fopen(file, "r");
  if(stream==NULL)return;

  CheckMemory;

  for(i = 0; i<MAXPLOT3DVARS; i++){
    if(fgets(buffer, 255, stream)==NULL)break;
    sscanf(buffer, "%f %f", valmin+i, valmax+i);
  }
  fclose(stream);
}

/* ------------------ GetGlobalPlot3DBounds ------------------------ */

void GetGlobalPlot3DBounds(void){
  int i;

  for(i = 0; i<nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = plot3dinfo+i;
    GetPlot3DFileBounds(plot3di->bound_file, plot3di->valmin_fds, plot3di->valmax_fds);
  }
  for(i = 0; i<MAXPLOT3DVARS; i++){
    p3min_all[i] = 1.0;
    p3max_all[i] = 0.0;
  }
  for(i = 0; i<nplot3dinfo; i++){
    plot3ddata *plot3di;
    int j;
    float *valmin_fds, *valmax_fds;

    plot3di = plot3dinfo+i;
    valmin_fds = plot3di->valmin_fds;
    valmax_fds = plot3di->valmax_fds;
    for(j = 0; j<MAXPLOT3DVARS; j++){
      if(valmin_fds[j]<=valmax_fds[j]){
        if(p3min_all[j]>p3max_all[j]){
          p3min_all[j] = valmin_fds[j];
          p3max_all[j] = valmax_fds[j];
        }
        else {
          p3min_all[j] = MIN(valmin_fds[j], p3min_all[j]);
          p3max_all[j] = MAX(valmax_fds[j], p3max_all[j]);
        }
      }
    }
  }
  for(i = 0; i<MAXPLOT3DVARS; i++){
    p3min_global[i] = p3min_all[i];
    p3max_global[i] = p3max_all[i];
  }

  nplot3dbounds_cpp = 0;
  if(nplot3dinfo>0&&plot3dbounds_cpp==NULL){ // only initialize once
    int i;

    nplot3dbounds_cpp = MAXPLOT3DVARS;
    NewMemory((void **)&plot3dbounds_cpp, nplot3dbounds_cpp*sizeof(cpp_boundsdata));
    for(i = 0; i<nplot3dbounds_cpp; i++){
      cpp_boundsdata *boundscppi;

      boundscppi = plot3dbounds_cpp+i;
      strcpy(boundscppi->label, plot3dinfo->label[i].shortlabel);
      strcpy(boundscppi->unit, plot3dinfo->label[i].unit);

      boundscppi->cache = cache_plot3d_data;
      boundscppi->set_valtype = 0;

      boundscppi->set_valmin = 0;
      boundscppi->valmin[BOUND_SET_MIN]        = p3min_global[i];
      boundscppi->valmin[BOUND_LOADED_MIN]     = p3min_global[i];
      boundscppi->valmin[BOUND_GLOBAL_MIN]     = p3min_global[i];
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = p3min_global[i];

      boundscppi->set_valmax = 0;
      boundscppi->valmax[BOUND_SET_MAX]        = p3max_global[i];
      boundscppi->valmax[BOUND_LOADED_MAX]     = p3max_global[i];
      boundscppi->valmax[BOUND_GLOBAL_MAX]     = p3max_global[i];
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = p3max_global[i];

      boundscppi->set_chopmin = 0;
      boundscppi->set_chopmax = 0;
      boundscppi->chopmin = p3max_global[0];
      boundscppi->chopmax = p3max_global[0];

      boundscppi->hist = NULL;
    }
  }
}

/* ------------------ GetLoadedPlot3dBounds ------------------------ */

void GetLoadedPlot3dBounds(int *compute_loaded, float *loaded_min, float *loaded_max){
  int i;
  int plot3d_loaded = 0;

#define BOUNDS_LOADED 1
  for(i = 0; i<nplot3dinfo; i++) {
    plot3ddata *plot3di;

    plot3di = plot3dinfo+i;
    if(plot3di->loaded==0)continue;
    plot3d_loaded = 1;
    break;
  }
  if(plot3d_loaded==0){
    printf("***loaded plot3d bounds not available, using global bounds\n");
    for(i = 0; i<6; i++) {
      loaded_min[i] = p3min_global[i];
      loaded_max[i] = p3max_global[i];
    }
    return;
  }
  for(i = 0; i<6; i++) {
    if(compute_loaded!=NULL&&compute_loaded[i]!=BOUNDS_LOADED)continue;
    loaded_min[i] = 1.0;
    loaded_max[i] = 0.0;
  }
  for(i = 0; i<nplot3dinfo; i++){
    plot3ddata *plot3di;
    int j;

    plot3di = plot3dinfo+i;
    if(plot3di->loaded==0)continue;
    for(j = 0; j<6; j++){
      if(compute_loaded!=NULL&&compute_loaded[j]!=BOUNDS_LOADED)continue;
      if(loaded_min[j]>loaded_max[j]){
        loaded_min[j] = plot3di->valmin_fds[j];
        loaded_max[j] = plot3di->valmax_fds[j];
      }
      else{
        loaded_min[j] = MIN(plot3di->valmin_fds[j], loaded_min[j]);
        loaded_max[j] = MAX(plot3di->valmax_fds[j], loaded_max[j]);
      }
    }
  }
}

/* ------------------ GetGlobalSliceBounds ------------------------ */

void GetGlobalSliceBounds(void){
  int i;

  for(i = 0;i<nslicebounds;i++){
    boundsdata *boundi;

    boundi = slicebounds+i;
    boundi->dlg_global_valmin = 1.0;
    boundi->dlg_global_valmax = 0.0;
  }
  for(i = 0;i<nsliceinfo;i++){
    slicedata *slicei;
    float valmin, valmax;
    boundsdata *boundi;

    slicei = sliceinfo+i;
    if(slicei->is_fed==1)continue;
    if(slicei->valmin_fds>slicei->valmax_fds ||
       current_script_command==NULL||current_script_command->command!=SCRIPT_LOADSLICERENDER){

      if(GetFileBounds(slicei->bound_file, &valmin, &valmax)==1){
        slicei->have_bound_file = YES;
      }
      if(valmin>valmax)continue;
      slicei->valmin_fds = valmin;
      slicei->valmax_fds = valmax;
    }
    else{
      valmin = slicei->valmin_fds;
      valmax = slicei->valmax_fds;
      slicei->have_bound_file = YES;
    }
    boundi = GetSliceBoundsInfo(slicei->label.shortlabel);
    if(boundi==NULL)continue;
    if(boundi->dlg_global_valmin>boundi->dlg_global_valmax){
      boundi->dlg_global_valmin = valmin;
      boundi->dlg_global_valmax = valmax;
    }
    else{
      boundi->dlg_global_valmin = MIN(boundi->dlg_global_valmin,valmin);
      boundi->dlg_global_valmax = MAX(boundi->dlg_global_valmax, valmax);
    }
  }
  for(i = 0; i<nslicebounds; i++){
    boundsdata *boundi;

    boundi = slicebounds+i;
    boundi->dlg_valmin = boundi->dlg_global_valmin;
    boundi->dlg_valmax = boundi->dlg_global_valmax;
  }
  nslicebounds_cpp = nslicebounds;
  if(nslicebounds_cpp>0&&slicebounds_cpp==NULL){ // only initialize once
    NewMemory((void **)&slicebounds_cpp, nslicebounds_cpp*sizeof(cpp_boundsdata));
    for(i = 0; i<nslicebounds_cpp; i++){
      cpp_boundsdata *boundscppi;
      boundsdata *boundi;

      boundscppi = slicebounds_cpp + i;
      boundi     = slicebounds + i;
      strcpy(boundscppi->label, boundi->shortlabel);
      strcpy(boundscppi->unit, boundi->label->unit);

      boundscppi->cache = cache_slice_data;
      boundscppi->set_valtype = 0;

      boundscppi->set_valmin = 0;
      boundscppi->valmin[BOUND_SET_MIN]        = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_LOADED_MIN]     = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_GLOBAL_MIN]     = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = boundi->dlg_global_valmin;

      boundscppi->set_valmax = 0;
      boundscppi->valmax[BOUND_SET_MAX]        = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_LOADED_MAX]     = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_GLOBAL_MAX]     = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = boundi->dlg_global_valmax;

      boundscppi->set_chopmin = boundi->setchopmin;
      boundscppi->set_chopmax = boundi->setchopmax;
      boundscppi->chopmin     = boundi->chopmin;
      boundscppi->chopmax     = boundi->chopmax;

      boundscppi->hist = NULL;
    }
  }
}

/* ------------------ UpdateGlobalFEDSliceBounds ------------------------ */

void UpdateGlobalFEDSliceBounds(void){
  int i;

  for(i = 0; i<nsliceinfo; i++){
    slicedata *slicei;
    float valmin, valmax;
    boundsdata *boundi;

    slicei = sliceinfo+i;
    if(slicei->is_fed==0||slicei->have_bound_file==0)continue;
    if(slicei->valmin_fds>slicei->valmax_fds||
       current_script_command==NULL||current_script_command->command!=SCRIPT_LOADSLICERENDER){

      GetFileBounds(slicei->bound_file, &valmin, &valmax);

      if(valmin>valmax)continue;
      slicei->valmin_fds = valmin;
      slicei->valmax_fds = valmax;
    }
    else{
      valmin = slicei->valmin_fds;
      valmax = slicei->valmax_fds;
      slicei->have_bound_file = YES;
    }
    boundi = GetSliceBoundsInfo(slicei->label.shortlabel);
    if(boundi==NULL)continue;
    if(boundi->dlg_global_valmin>boundi->dlg_global_valmax){
      boundi->dlg_global_valmin = valmin;
      boundi->dlg_global_valmax = valmax;
    }
    else{
      boundi->dlg_global_valmin = MIN(boundi->dlg_global_valmin, valmin);
      boundi->dlg_global_valmax = MAX(boundi->dlg_global_valmax, valmax);
    }
  }
  for(i = 0; i<nslicebounds; i++){
    boundsdata *boundi;

    boundi = slicebounds+i;
    if(strcmp(boundi->label->shortlabel, "FED")==0){
      boundi->dlg_valmin = 0.0;;
      boundi->dlg_valmax = 3.0;
    }
  }
  nslicebounds_cpp = nslicebounds;
  if(nslicebounds_cpp>0){
    for(i = 0; i<nslicebounds_cpp; i++){
      cpp_boundsdata *boundscppi;
      boundsdata *boundi;

      boundscppi = slicebounds_cpp+i;
      boundi = slicebounds+i;
      if(strcmp(boundi->label->shortlabel, "FED")!=0)continue;

      strcpy(boundscppi->label, boundi->shortlabel);
      strcpy(boundscppi->unit, boundi->label->unit);

      boundscppi->cache = cache_slice_data;
      boundscppi->set_valtype = 0;

      boundscppi->set_valmin = BOUND_SET_MIN;
      boundscppi->valmin[BOUND_SET_MIN] = 0.0;
      boundscppi->valmin[BOUND_LOADED_MIN] = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_GLOBAL_MIN] = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = boundi->dlg_global_valmin;

      boundscppi->set_valmax = BOUND_SET_MAX;
      boundscppi->valmax[BOUND_SET_MAX] = 3.0;
      boundscppi->valmax[BOUND_LOADED_MAX] = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_GLOBAL_MAX] = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = boundi->dlg_global_valmax;

      boundscppi->set_chopmin = boundi->setchopmin;
      boundscppi->set_chopmax = boundi->setchopmax;
      boundscppi->chopmin = boundi->chopmin;
      boundscppi->chopmax = boundi->chopmax;

      boundscppi->hist = NULL;
    }
  }
}

/* ------------------ GetSliceBoundsInfo ------------------------ */

boundsdata *GetSliceBoundsInfo(char *shortlabel){
  int i;

  for(i = 0; i<nslicebounds; i++){
    boundsdata *boundi;

    boundi = slicebounds+i;
    if(strcmp(boundi->shortlabel, shortlabel)==0)return boundi;
  }
  return NULL;
}

/* ------------------ AdjustPart5Chops ------------------------ */

void AdjustPart5Chops(void){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    propi->imin=0;
    propi->imax=255;
    if(propi->setchopmin==1){
      float dval;

      dval = propi->valmax-propi->valmin;
      if(dval<=0.0)dval=1.0;
      propi->imin=CLAMP(255*(propi->chopmin-propi->valmin)/dval,0,255);
    }
    if(propi->setchopmax==1){
      float dval;

      dval = propi->valmax-propi->valmin;
      if(dval<=0.0)dval=1.0;
      propi->imax=CLAMP(255*(propi->chopmax-propi->valmin)/dval,0,255);
    }
  }
}

/* -----  ------------- ReadPartBounds ------------------------ */

int ReadPartBounds(partdata *parti,int read_bounds_arg){
  FILE *stream=NULL;
  int j, eof_local=0;
  int part_boundfile_version_local = 1;
  float *valmin_local, *valmax_local;

  parti->bounds_set = 0;
  if(parti->global_min==NULL)NewMemory((void **)&parti->global_min, npart5prop*sizeof(float));
  if(parti->global_max==NULL)NewMemory((void **)&parti->global_max, npart5prop*sizeof(float));

  valmin_local = parti->global_min;
  valmax_local = parti->global_max;
  for(j = 0; j<npart5prop; j++){
    valmin_local[j] =  1000000000.0;
    valmax_local[j] = -1000000000.0;
  }

  // make sure a size file exists

  stream = fopen(parti->size_file, "r");
  if(stream==NULL){
    CreatePartSizeFile(parti, parti->evac==1);
    stream = fopen(parti->size_file, "r");
    if(stream==NULL)return 0;
    fclose(stream);
  }

  // make sure a bound file exists

  stream = fopen(parti->bound_file, "r");
  if(stream==NULL){
    CreatePartBoundFile(parti);
    stream = fopen(parti->bound_file, "r");
    if(stream==NULL)return 0;
  }

// if valid global particle bounds file so we can return now and not read individual bound files

  if(read_bounds_arg==1){
    parti->bounds_set = 1;
    fclose(stream);
    return 0;
  }

  for(;;){
    float time_local;
    int nclasses_local, k, version_local =-1;
    char buffer_local[255];

    if(fgets(buffer_local, 255, stream)==NULL)break;
    sscanf(buffer_local, "%f %i %i", &time_local, &nclasses_local, &version_local);
    if(version_local!=part_boundfile_version_local){
      fclose(stream);
      return 0;
    }

    for(k = 0; k<nclasses_local; k++){
      int nbounds_local, npoints_local;

      if(fgets(buffer_local, 255, stream)==NULL){
        eof_local = 1;
        break;
      }
      sscanf(buffer_local, "%i %i", &nbounds_local, &npoints_local);
      for(j = 0; j<nbounds_local; j++){
        float vmin_local, vmax_local;
        int prop_index_local;

        if(fgets(buffer_local, 255, stream)==NULL){
          eof_local = 1;
          break;
        }
        sscanf(buffer_local, "%f %f", &vmin_local, &vmax_local);
        if(vmax_local>=vmin_local){
          parti->bounds_set = 1;

          prop_index_local = GetPartPropIndex(k,j+2);

          valmin_local[prop_index_local] = MIN(valmin_local[prop_index_local], vmin_local);
          valmax_local[prop_index_local] = MAX(valmax_local[prop_index_local], vmax_local);
        }
      }
      if(eof_local==1)break;
    }
    if(eof_local==1)break;
  }
  fclose(stream);
  return 1;
}

/* ------------------ MergeAllPartBounds ------------------------ */

void MergeAllPartBounds(void){
  int i;

  for(i = 0; i<npart5prop; i++){
    partpropdata *propi;

    propi = part5propinfo+i;
    if(strcmp(propi->label->shortlabel, "Uniform")==0)continue;
    propi->dlg_global_valmin =  1000000000.0;
    propi->dlg_global_valmax = -1000000000.0;
  }

  // find min/max over all particle files

  for(i = 0; i<npartinfo; i++){
    partdata *parti;
    int j;

    parti = partinfo+i;
    if(parti->bounds_set==0)continue;
    for(j = 0; j<npart5prop; j++){
      partpropdata *propj;

      propj = part5propinfo+j;
      if(strcmp(propj->label->shortlabel, "Uniform")==0)continue;
      propj->dlg_global_valmin = MIN(propj->dlg_global_valmin, parti->global_min[j]);
      propj->dlg_global_valmax = MAX(propj->dlg_global_valmax, parti->global_max[j]);
    }
  }
  if(global_have_global_bound_file==0){
    FILE *stream;

    stream = fopen(part_globalbound_filename, "w");
    if(stream!=NULL){
      global_have_global_bound_file = 1;
      global_part_boundsize = GetFileSizeSMV(partinfo->bound_file);
      fprintf(stream,"%i %i\n",npart5prop,(int)global_part_boundsize);
      for(i=0;i<npart5prop;i++){
        partpropdata *propi;
        float valmin, valmax;

        propi = part5propinfo+i;
        valmin = propi->dlg_global_valmin;
        valmax = propi->dlg_global_valmax;
        fprintf(stream, "%g %g\n", valmin, valmax);
      }
      fclose(stream);
    }
  }
  AdjustPart5Chops();
#ifdef _DEBUG
  //    PrintPartProp();
#endif
}

/* ------------------ PrintPartLoadSummary ------------------------ */

void PrintPartLoadSummary(int option_arg,int type_arg){
  int nsize_local;
  int j;

  nsize_local = 0;
  for(j = 0; j<npartinfo; j++){
    partdata *partj;

    partj = partinfo+j;
    if(type_arg==PART_SIZING&&partj->boundstatus==PART_BOUND_COMPUTING)nsize_local++;
    if(type_arg==PART_LOADING&&partj->loadstatus==FILE_LOADING)nsize_local++;
  }
  if(option_arg==1||(nsize_local<npartthread_ids&&nsize_local>0)){
    int isize_local;

#ifdef pp_PART_SIZE
    if(type_arg==PART_SIZING)printf("sizing: ");
#endif
    if(type_arg==PART_LOADING)printf("loading: ");
    isize_local = 0;
    for(j = 0; j<npartinfo; j++){
      partdata *partj;
      int doit;

      partj = partinfo+j;
      doit = 0;
#ifdef pp_PART_SIZE
      if(type_arg==PART_SIZING&&partj->boundstatus==PART_BOUND_COMPUTING)doit = 1;
#endif
      if(type_arg==PART_LOADING&&partj->loadstatus==FILE_LOADING)doit = 1;
      if(doit==1){
        printf("%s", partj->reg_file);
        if(isize_local!=nsize_local-1)printf(", ");
        isize_local++;
      }
    }
#ifdef pp_PART_SIZE
    printf("\n");
#endif
    printf("\n");
  }
}

/* ------------------ GetAllPartBounds ------------------------ */

void GetAllPartBounds(void){
  int i;
  FILE *stream = NULL;

  LOCK_PART_LOAD;
  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    if(parti->global_min==NULL){
      NewMemory((void **)&parti->global_min, npart5prop*sizeof(float));
    }
    if(parti->global_max==NULL){
      NewMemory((void **)&parti->global_max, npart5prop*sizeof(float));
    }
  }

  // find min/max for each particle file

  if(global_part_boundsize==0)global_part_boundsize = GetFileSizeSMV(partinfo->bound_file);

  stream = fopen(part_globalbound_filename, "r");
  if(stream!=NULL){
    int n;
    int part_boundsize_old_local;

    fscanf(stream, "%i %i", &n, &part_boundsize_old_local);
    if((int)global_part_boundsize==part_boundsize_old_local){      // only use global particle bound file if the size of the first particle .bnd file has not changed
      global_have_global_bound_file = 1;                           // (assume if any .bnd file has changed then the first .bnd file has changed)
    //  printf("reading global particle bound file: %s\n", part_globalbound_filename);
      n = MIN(npart5prop, n);
      for(i = 0; i<n; i++){
        partpropdata *propi;
        float valmin, valmax;

        propi = part5propinfo+i;
        fscanf(stream, "%f %f", &valmin, &valmax);
        propi->dlg_global_valmin = valmin;
        propi->dlg_global_valmax = valmax;
      }
      fclose(stream);
      for(i = 0; i<npartinfo; i++){
        partdata *parti;
        int j;

        parti = partinfo+i;
        parti->boundstatus = PART_BOUND_DEFINED;
        parti->bounds_set = 1;
        for(j = 0; j<npart5prop; j++){
          partpropdata *propj;

          propj = part5propinfo+j;
          parti->global_min[j] = propj->dlg_global_valmin;
          parti->global_max[j] = propj->dlg_global_valmax;
        }
      }
      UNLOCK_PART_LOAD;
      return;
    }
    fclose(stream);
  }
  UNLOCK_PART_LOAD;

  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    LOCK_PART_LOAD;
    if(parti->boundstatus!=PART_BOUND_UNDEFINED){
      UNLOCK_PART_LOAD;
      continue;
    }
    parti->boundstatus = PART_BOUND_COMPUTING;
#ifdef pp_PART_SIZE
    PrintPartLoadSummary(PART_BEFORE, PART_SIZING);
#endif
    UNLOCK_PART_LOAD;
    ReadPartBounds(parti,global_have_global_bound_file);
    LOCK_PART_LOAD;
#ifdef pp_PART_SIZE
    if(npartinfo>1)PrintPartLoadSummary(PART_AFTER, PART_SIZING);
#endif
    parti->boundstatus = PART_BOUND_DEFINED;
    UNLOCK_PART_LOAD;
  }
}
