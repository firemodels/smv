#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "smokeviewvars.h"
#include "glui_bounds.h"
#include "IOscript.h"

#ifdef _DEBUG
#define ASSERT_BOUND_TYPE assert(file_type == BOUND_SLICE || file_type == BOUND_PATCH || file_type == BOUND_PLOT3D)
#else
#define ASSERT_BOUND_TYPE
#endif
#define IJK_SLICE(i,j,k)  ( ((i)-sd->is1)*sd->nslicej*sd->nslicek + ((j)-sd->js1)*sd->nslicek + ((k)-sd->ks1) )

/* ------------------ GetPartFileBounds ------------------------ */

int GetPartFileBounds(char *file, float *valmin, float *valmax, int *ntotal_points){
  FILE *stream;
  int i;

  *ntotal_points = 0;
  if(file==NULL||strlen(file)==0)return 0;
  stream = FOPEN_2DIR(file, "r");
  if(stream==NULL)return 0;

  while(!feof(stream)){
    char buffer[255];
    float time;
    int class_index, nclasses;

    CheckMemory;
    if(fgets(buffer, 255, stream)==NULL)break;
    sscanf(buffer, "%f %i", &time, &nclasses);

    for(class_index = 0; class_index<nclasses; class_index++){
      int nfilebounds, npoints;

      if(fgets(buffer, 255, stream)==NULL)break;
      sscanf(buffer, "%i %i", &nfilebounds, &npoints);
      *ntotal_points += npoints;

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


  if(part_bound_buffer == NULL && global_scase.npartinfo > 0 && npart5prop>0){
    NewMemory((void **)&part_bound_buffer, 2*global_scase.npartinfo*npart5prop*sizeof(float));
    for(i = 0; i < global_scase.npartinfo; i++){
      partdata *parti;
      int j;

      parti = global_scase.partinfo + i;
      if(parti->loaded == 1)nloaded_files++;
      parti->valmin_part = part_bound_buffer;
      part_bound_buffer += npart5prop;
      parti->valmax_part = part_bound_buffer;
      part_bound_buffer += npart5prop;
      for(j = 0; j < npart5prop; j++){
        parti->valmin_part[j] = 1.0;
        parti->valmax_part[j] = 0.0;
      }
      parti->have_bound_file = GetPartFileBounds(parti->bound_file, parti->valmin_part, parti->valmax_part, &parti->npoints_file);
    }
  }
  if(npart5prop>0){
    NewMemory((void **)&partmins, npart5prop*sizeof(float));
    NewMemory((void **)&partmaxs, npart5prop*sizeof(float));
  }
  for(i = 0; i<npart5prop; i++){
    partmins[i] = 1.0;
    partmaxs[i] = 0.0;
  }
  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;
    int j;
    float *valmin_part, *valmax_part;

    parti = global_scase.partinfo+i;
    if(flag==LOADED_FILES&&parti->loaded==0)continue;
    valmin_part = parti->valmin_part;
    valmax_part = parti->valmax_part;
    for(j = 0; j<npart5prop; j++){
      if(valmin_part[j]<=valmax_part[j]){
        if(partmins[j]>partmaxs[j]){
          partmins[j] = valmin_part[j];
          partmaxs[j] = valmax_part[j];
        }
        else{
          partmins[j] = MIN(valmin_part[j], partmins[j]);
          partmaxs[j] = MAX(valmax_part[j], partmaxs[j]);
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
      boundscppi->hist        = NULL;

      if(bound_part_init == 1){
        boundscppi->set_chopmin = 0;
        boundscppi->set_chopmax = 0;
        boundscppi->chopmin     = 0.0;
        boundscppi->chopmax     = 0.0;
      }
    }
  }
  bound_part_init = 0;
  FREEMEMORY(partmins);
  FREEMEMORY(partmaxs);
  return nloaded_files;
}

/* ------------------ GetGlobalPartBoundsReduced ------------------------ */

void *GetGlobalPartBoundsReduced(void *arg){
  GetGlobalPartBounds(0);
  THREAD_EXIT(partbound_threads);
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

  stream = FOPEN_2DIR(file, "w");
  if(stream==NULL)return 0;
  fprintf(stream," %f %f %f", 0.0, valmin, valmax);
  fclose(stream);
  return 1;
}

/* ------------------ GetFileBounds ------------------------ */

int GetFileBounds(char *file, int nbounds, float *valmin, float *valmax){
  FILE *stream;
  char buffer[255];
  int i;
  float vmins[MAXPLOT3DVARS] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  float vmaxs[MAXPLOT3DVARS] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};

  stream = FOPEN_2DIR(file, "r");
  if(stream==NULL){
    memcpy(valmin, vmins, nbounds * sizeof(float));
    memcpy(valmax, vmaxs, nbounds * sizeof(float));
    if(stream != NULL)fclose(stream);
    return 0;
  }

  if(nbounds==1){
    float t;

    for(i = 0;i < nbounds;i++){
      if(fgets(buffer, 255, stream) == NULL)break;
      sscanf(buffer, " %f %f %f", &t, vmins+i, vmaxs+i);
    }
  }
  else{
    for(i = 0;i < nbounds;i++){
      if(fgets(buffer, 255, stream) == NULL)break;
      sscanf(buffer, " %f %f", vmins+i, vmaxs+i);
    }
  }
  memcpy(valmin, vmins, nbounds * sizeof(float));
  memcpy(valmax, vmaxs, nbounds * sizeof(float));
  fclose(stream);
  return 1;
}

/* ------------------ GetBounds ------------------------ */

int GetBounds(char *file, float *valmin, float *valmax,
                   fileboundsdata **boundsinfoptr, int *nboundsinfoptr){
  int return_val;

  *valmin = 1.0;
  *valmax = 0.0;
  return_val = 0;

  if(*boundsinfoptr!=NULL){
    int i;

    for(i=0; i<*nboundsinfoptr; i++){
      fileboundsdata *bi;

      bi = *boundsinfoptr + i;
      if(strcmp(bi->file, file)==0){
        *valmin = bi->valmin;
        *valmax = bi->valmax;
        return_val = 1;
        break;
      }
    }
  }
  else{
    return_val = GetFileBounds(file, 1, valmin, valmax);
  }
  return return_val;
}

/* ------------------ CompareBoundFileName ------------------------ */

int CompareBoundFileName(const void *arg1, const void *arg2){
  char *x, *y;

  x = *(char **)arg1;
  y = *(char **)arg2;

  return strcmp(x, y);
}

/* ------------------ BoundsGet ------------------------ */

int BoundsGet(char *file, globalboundsdata *globalboundsinfo, char **sorted_filenames, int n_sorted_filenames, int nbounds, float *valmins, float *valmaxs){
  char **key_index;
  int defined = 0;

  key_index = (char **)bsearch((char *)&file, sorted_filenames, n_sorted_filenames, sizeof(char *), CompareBoundFileName);
  if(key_index != NULL){
    int index;
    globalboundsdata *fi;

    index = (int)(key_index - sorted_filenames);
    fi = globalboundsinfo + index;
    if(fi->defined == 1){
      memcpy(valmins, fi->valmins, nbounds * sizeof(float));
      memcpy(valmaxs, fi->valmaxs, nbounds * sizeof(float));
      defined = 1;
    }
  }
  if(defined == 0){
    int i;

    for(i = 0;i < nbounds;i++){
      valmins[i] = 0.0;
      valmaxs[i] = 1.0;
    }
  }
  return defined;
}

/* ------------------ GetNinfo ------------------------ */

int GetNinfo(int file_type){
  int ninfo = -1;


  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    ninfo = global_scase.slicecoll.nsliceinfo;
  }
  else if(file_type == BOUND_PATCH){
    ninfo = global_scase.npatchinfo;
  }
  else if(file_type == BOUND_PLOT3D){
    ninfo = global_scase.nplot3dinfo;
  }
  return ninfo;
}

/* ------------------ GetGlobalBoundsinfo ------------------------ */

globalboundsdata *GetGlobalBoundsinfo(int file_type){
  globalboundsdata *gi = NULL;

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    gi = sliceglobalboundsinfo;
  }
  else if(file_type == BOUND_PATCH){
    gi = patchglobalboundsinfo;
  }
  else if(file_type == BOUND_PLOT3D){
    gi = plot3dglobalboundsinfo;
  }
  return gi;
}

/* ------------------ GetGbndFilename ------------------------ */

char **GetSortedFilenames(int file_type){
  char **sorted_filenames = NULL;

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    sorted_filenames = sorted_slice_filenames;
  }
  else if(file_type == BOUND_PATCH){
    sorted_filenames = sorted_patch_filenames;
  }
  else if(file_type == BOUND_PLOT3D){
    sorted_filenames = sorted_plot3d_filenames;
  }
  return sorted_filenames;
}

/* ------------------ GetRegFile ------------------------ */

char *GetRegFile(int file_type, int i){
  char *reg_file = NULL;;

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    reg_file = global_scase.slicecoll.sliceinfo[i].reg_file;
  }
  else if(file_type == BOUND_PATCH){
    reg_file = global_scase.patchinfo[i].reg_file;
  }
  else if(file_type == BOUND_PLOT3D){
    reg_file = global_scase.plot3dinfo[i].reg_file;
  }
  return reg_file;
}

/* ------------------ MakeSliceMask ------------------------ */

void MakeSliceMask(slicedata *sd){
  int i;
  int iis1, iis2, jjs1, jjs2, kks1, kks2;

  NewMemory(( void ** )&sd->slice_mask, sd->nsliceijk);
  memset(sd->slice_mask, 0, sd->nsliceijk);

  iis1 = sd->iis1 + 1;
  if(sd->iis2 == sd->iis1)iis1 = sd->iis1;
  iis2 = sd->iis2;

  jjs1 = sd->jjs1 + 1;
  if(sd->jjs2 == sd->jjs1)jjs1 = sd->jjs1;
  jjs2 = sd->jjs2;

  kks1 = sd->kks1 + 1;
  if(sd->kks2 == sd->kks1)kks1 = sd->kks1;
  kks2 = sd->kks2;

  for(i = iis1; i <= iis2; i++){
    int j;

    for(j = jjs1; j <= jjs2; j++){
      int k;

      for(k = kks1; k <= kks2; k++){
        int ival;

        ival = IJK_SLICE(i, j, k);
        assert(ival >= 0 && ival < sd->nsliceijk);
        sd->slice_mask[ival] = 1;
      }
    }
  }
}

/* ------------------ BoundsUpdateDoit ------------------------ */

void BoundsUpdateDoit(int file_type){
  int i;
  int is_fds_running;
  int ninfo;
  globalboundsdata *globalboundsinfo;
  char **sorted_filenames;

  ninfo = GetNinfo(file_type);
  globalboundsinfo = GetGlobalBoundsinfo(file_type);
  sorted_filenames = GetSortedFilenames(file_type);

  ASSERT_BOUND_TYPE;
  if(file_type != BOUND_SLICE && file_type != BOUND_PATCH && file_type != BOUND_PLOT3D)return;
  is_fds_running = IsFDSRunning(&last_size_for_bound);
  for(i = 0; i < ninfo; i++){
    globalboundsdata *fi;
    int index;
    float valmin, valmax;
    char **key_index, *reg_file;
    patchdata *patchi;
    slicedata *slicei;
    plot3ddata *plot3di;

    if(file_type == BOUND_SLICE){
      slicei = global_scase.slicecoll.sliceinfo + i;
      if(slicei->loaded == 0)continue;
    }
    else if(file_type == BOUND_PATCH){
      patchi = global_scase.patchinfo + i;
      if(patchi->loaded == 0)continue;
    }
    else if(file_type == BOUND_PLOT3D){
      plot3di = global_scase.plot3dinfo + i;
      if(plot3di->loaded == 0)continue;
    }
    reg_file = GetRegFile(file_type, i);
    key_index = ( char ** )bsearch(( char * )&reg_file, sorted_filenames, ninfo, sizeof(char *), CompareBoundFileName);
    if(key_index == NULL)continue;
    index = ( int )(key_index - sorted_filenames);
    if(index<0 || index>global_scase.slicecoll.nsliceinfo - 1)continue;
    fi = globalboundsinfo + index;
    if(fi->defined == 1 && is_fds_running == 0)continue;
    valmin = 0.0;
    valmax = 1.0;
    if(file_type == BOUND_SLICE){
      float *vals;
      int j, nsize;

      if(slicei->slice_mask == NULL && slicei->slice_filetype == SLICE_CELL_CENTER){
        MakeSliceMask(slicei);
      }
      vals = slicei->qslicedata;
      valmin = 1000000000.0;
      valmax = -valmin;
      nsize = slicei->nslicei * slicei->nslicej * slicei->nslicek;
      if(slicei->slice_mask != NULL && slicei->slice_filetype == SLICE_CELL_CENTER){
        for(j = 0; j < slicei->ntimes * slicei->nsliceijk; j++){
          if(slicei->slice_mask[j % nsize] == 1){
            valmin = MIN(valmin, vals[j]);
            valmax = MAX(valmax, vals[j]);
          }
        }
      }
      else{
#ifdef pp_SLICEFRAME
        int itime;
        for(itime = 0; itime < slicei->ntimes; itime++){
          vals = ( float * )FRAMEGetFramePtr(slicei->frameinfo, slicei->itime);
          if(vals != NULL){
            for(j = 0; j < slicei->ntimes * slicei->nsliceijk; j++){
              valmin = MIN(valmin, vals[j]);
              valmax = MAX(valmax, vals[j]);
            }
          }
        }
#else
        for(j = 0; j < slicei->ntimes * slicei->nsliceijk; j++){
          valmin = MIN(valmin, vals[j]);
          valmax = MAX(valmax, vals[j]);
        }
#endif
      }
      fi->defined = 1;
    }
    else if(file_type == BOUND_PATCH){
      if(patchi->structured == 1){
        if(patchi->blocknumber >= 0){
          meshdata *meshi;
          float *vals;
          int j;

          meshi = global_scase.meshescoll.meshinfo + patchi->blocknumber;
          if(meshi->boundary_mask == NULL && patchi->patch_filetype == PATCH_STRUCTURED_CELL_CENTER){
            MakeBoundaryMask(patchi);
          }
          vals = meshi->patchval;
          valmin = 10000000000.0;
          valmax = -valmin;
          if(meshi->boundary_mask != NULL && patchi->patch_filetype == PATCH_STRUCTURED_CELL_CENTER){
            for(j = 0; j < patchi->ntimes * meshi->npatchsize; j++){
              if(meshi->boundary_mask[j % meshi->npatchsize] == 1){
                valmin = MIN(vals[j], valmin);
                valmax = MAX(vals[j], valmax);
              }
            }
          }
          else{
            for(j = 0; j < patchi->ntimes * meshi->npatchsize; j++){
              valmin = MIN(vals[j], valmin);
              valmax = MAX(vals[j], valmax);
            }
          }
        }
        fi->nbounds = 1;
        fi->valmins[0] = valmin;
        fi->valmaxs[0] = valmax;
      }
      else{
        float *vals;
        int j;

        vals = patchi->geom_vals;
        valmin = vals[0];
        valmax = valmin;
        for(j = 1; j < patchi->ngeom_times; j++){
          valmin = MIN(vals[j], valmin);
          valmax = MAX(vals[j], valmax);
        }
      }
      fi->defined = 1;
      fi->nbounds = 1;
      fi->valmins[0] = valmin;
      fi->valmaxs[0] = valmax;
    }
    else if(file_type == BOUND_PLOT3D){
      meshdata *meshi;
      int nx, ny, nz, nxy, nxyz;
      int ii, jj, kk, nn;
      float valmins[MAXPLOT3DVARS], valmaxs[MAXPLOT3DVARS];

      meshi = global_scase.meshescoll.meshinfo + plot3di->blocknumber;
      nx = meshi->ibar + 1;
      ny = meshi->jbar + 1;
      nz = meshi->kbar + 1;
      nxy = nx * ny;
      nxyz = nx * ny * nz;
      for(nn = 0; nn < plot3di->nplot3dvars; nn++){
        valmins[nn] = meshi->qdata[IJKN(0, 0, 0, nn)];
        valmaxs[nn] = valmins[nn];
        for(ii = 0; ii < nx; ii++){
          for(jj = 0; jj < ny; jj++){
            for(kk = 0; kk < nz; kk++){
              float val;

              val = meshi->qdata[IJKN(ii, jj, kk, nn)];
              valmins[nn] = MIN(val, valmins[nn]);
              valmaxs[nn] = MAX(val, valmaxs[nn]);

            }
          }
        }
        fi->valmins[nn] = valmins[nn];
        fi->valmaxs[nn] = valmaxs[nn];
      }
      fi->defined = 1;
      fi->nbounds = 5;
      if(global_scase.plot3dinfo != NULL)fi->nbounds = plot3di->nplot3dvars;
    }
  }
}

/* ------------------ FopenGbndFile ------------------------ */

FILE *FopenGbndFile(int file_type, char *mode){
  FILE *stream = NULL;
  char *file = NULL;

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    file = slice_gbnd_filename;
  }
  else if(file_type == BOUND_PATCH){
    file = patch_gbnd_filename;
  }
  else if(file_type == BOUND_PLOT3D){
    file = plot3d_gbnd_filename;
  }
  else{
    file = NULL;
  }
  if(file != NULL){
    stream = FOPEN_2DIR(file, mode);
  }
  return stream;
}

/* ------------------ GetNBounds ------------------------ */

int GetNBounds(int file_type){
  int nbounds = 1;

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    nbounds = 1;
  }
  else if(file_type == BOUND_PATCH){
    nbounds = 1;
  }
  else if(file_type == BOUND_PLOT3D){
    if(global_scase.plot3dinfo != NULL){
      nbounds = global_scase.plot3dinfo->nplot3dvars;
    }
    else{
      nbounds = 5;
    }
  }
  return nbounds;
}

/* ------------------ GetShortLabel ------------------------ */

char *GetShortLabel(int file_type, int i, int ilabel){
  char *shortlabel = NULL;;

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    shortlabel = global_scase.slicecoll.sliceinfo[i].label.shortlabel;
  }
  else if(file_type == BOUND_PATCH){
    shortlabel = global_scase.patchinfo[i].label.shortlabel;
  }
  else if(file_type == BOUND_PLOT3D){
    shortlabel = global_scase.plot3dinfo[i].label[ilabel].shortlabel;
  }
  return shortlabel;
}


/* ------------------ GetBoundLabel ------------------------ */

char *GetBoundLabel(int file_type, int ifile, char *label){
  int nbounds, i;

  nbounds = GetNBounds(file_type);
  strcpy(label, "");
  for(i = 0; i < nbounds; i++){
    char *shortlabel;

    shortlabel = GetShortLabel(file_type, ifile, i);
    if(shortlabel != NULL){
      strcat(label, shortlabel);
      if(i != nbounds - 1)strcat(label, ":");
    }
  }
  if(strlen(label) == 0)return NULL;
  return label;
}

/* ------------------ BoundsGlobalBounds2Gbnd ------------------------ */

void BoundsGlobalBounds2Gbnd(int file_type){
  int i, ninfo;
  FILE *stream = NULL;
  globalboundsdata *globalboundsinfo;

#ifdef pp_BOUND_DEBUG
  char label1[32];

  strcpy(label1, "");
  if(file_type == BOUND_SLICE){
    strcat(label1, "SLICE");
  }
  else if(file_type == BOUND_PATCH){
    strcat(label1, "PATCH");
  }
  else if(file_type == BOUND_PLOT3D){
    strcat(label1, "PLOT3D");
  }
#endif
  ninfo = GetNinfo(file_type);
  globalboundsinfo = GetGlobalBoundsinfo(file_type);
  int changed = 0;
  for(i = 0; i < ninfo; i++){
    globalboundsdata *fi;
    int j;

    fi = globalboundsinfo + i;
    if(fi->defined == 0)continue;
    if(fi->nbounds == 1){
      if(fi->valmaxs[0] != fi->valmaxs_save[0] || fi->valmins[0] != fi->valmins_save[0]){
        changed = 1;
        break;
      }
    }
    else{
      for(j = 0; j < fi->nbounds; j++){
        if(fi->valmaxs[j] != fi->valmaxs_save[j] || fi->valmins[j] != fi->valmins_save[j]){
          changed = 1;
          break;
        }
      }
    }
    if(changed == 1)break;
  }
  if(changed == 0)return;
#ifdef pp_BOUND_DEBUG
  printf("BoundsGlobalBounds2Gbnd(%s)\n", label1);
#endif
  for(i = 0; i < ninfo; i++){
    globalboundsdata *fi;

    fi = globalboundsinfo + i;
    if(fi->defined == 1){
      char label[256];
      int j;

      if(stream == NULL){
        stream = FopenGbndFile(file_type, "w");
        if(stream == NULL)return;
      }
      fprintf(stream, "%s ", fi->file);
      for(j = 0; j < fi->nbounds; j++){
        fprintf(stream, " %f %f ", fi->valmins[j], fi->valmaxs[j]);
      }
      if(GetBoundLabel(file_type, i, label) != NULL)fprintf(stream, " ! %s", label);
      fprintf(stream, "\n");
    }
  }
  if(stream != NULL)fclose(stream);
}

/* ------------------ BoundsUpdateWrapup ------------------------ */

void BoundsUpdateWrapup(int file_type){
  int i;
  int ninfo;
  char **sorted_filenames;
  globalboundsdata *globalboundsinfo;

  globalboundsinfo = GetGlobalBoundsinfo(file_type);
  ninfo = GetNinfo(file_type);
  sorted_filenames = GetSortedFilenames(file_type);
  BoundsGlobalBounds2Gbnd(file_type);

  for(i = 0; i < ninfo; i++){
    globalboundsdata *fi;
    char **key_index, *reg_file;
    int index;

    reg_file = GetRegFile(file_type, i);
    key_index = ( char ** )bsearch(( char * )&reg_file, sorted_filenames, ninfo, sizeof(char *), CompareBoundFileName);
    if(key_index == NULL)continue;
    index = ( int )(key_index - sorted_filenames);
    if(index<0 || index>global_scase.slicecoll.nsliceinfo - 1)continue;
    fi = globalboundsinfo + index;
    if(fi->defined == 0)continue;
    if(file_type == BOUND_SLICE){
      slicedata *slicei;

      slicei = global_scase.slicecoll.sliceinfo + i;
      slicei->valmin_slice = fi->valmins[0];
      slicei->valmax_slice = fi->valmaxs[0];
    }
    else if(file_type == BOUND_PATCH){
      patchdata *patchi;

      patchi = global_scase.patchinfo + i;
      patchi->valmin_patch = fi->valmins[0];
      patchi->valmax_patch = fi->valmaxs[0];
    }
    else if(file_type == BOUND_PLOT3D){
      plot3ddata *plot3di;

      plot3di = global_scase.plot3dinfo + i;
      memcpy(plot3di->valmin_plot3d, fi->valmins, plot3di->nplot3dvars * sizeof(float));
      memcpy(plot3di->valmax_plot3d, fi->valmaxs, plot3di->nplot3dvars * sizeof(float));
    }
  }
}

/* ------------------ SaveSortedFilenames ------------------------ */

void SaveSortedFilenames(int file_type, char **sorted_filenames){
  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    sorted_slice_filenames = sorted_filenames;
  }
  else if(file_type == BOUND_PATCH){
    sorted_patch_filenames = sorted_filenames;
  }
  else if(file_type == BOUND_PLOT3D){
    sorted_plot3d_filenames = sorted_filenames;
  }
}

/* ------------------ SaveGlobalBoundsinfo ------------------------ */

void SaveGlobalBoundsinfo(int file_type, globalboundsdata *globalboundsinfo){
  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    sliceglobalboundsinfo = globalboundsinfo;
  }
  else if(file_type == BOUND_PATCH){
    patchglobalboundsinfo = globalboundsinfo;
  }
  else if(file_type == BOUND_PLOT3D){
    plot3dglobalboundsinfo = globalboundsinfo;
  }
}

/* ------------------ DefineGbndFilename ------------------------ */

void DefineGbndFilename(int file_type){
  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    if(slice_gbnd_filename == NULL){
      NewMemory(( void ** )&slice_gbnd_filename, strlen(global_scase.fdsprefix) + strlen(".sf.gbnd") + 1);
      strcpy(slice_gbnd_filename, global_scase.fdsprefix);
      strcat(slice_gbnd_filename, ".sf.gbnd");
    }
  }
  else if(file_type == BOUND_PATCH){
    if(patch_gbnd_filename == NULL){
      NewMemory(( void ** )&patch_gbnd_filename, strlen(global_scase.fdsprefix) + strlen(".bf.gbnd") + 1);
      strcpy(patch_gbnd_filename, global_scase.fdsprefix);
      strcat(patch_gbnd_filename, ".bf.gbnd");
    }
  }
  else if(file_type == BOUND_PLOT3D){
    if(plot3d_gbnd_filename == NULL){
      NewMemory(( void ** )&plot3d_gbnd_filename, strlen(global_scase.fdsprefix) + strlen(".q.gbnd") + 1);
      strcpy(plot3d_gbnd_filename, global_scase.fdsprefix);
      strcat(plot3d_gbnd_filename, ".q.gbnd");
    }
  }
}

/* ------------------ GetBoundFile ------------------------ */

char *GetBoundFile(int file_type, int i){
  char *bound_file = NULL;

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    bound_file = global_scase.slicecoll.sliceinfo[i].bound_file;
  }
  else if(file_type == BOUND_PATCH){
    bound_file = global_scase.patchinfo[i].bound_file;
  }
  else if(file_type == BOUND_PLOT3D){
    bound_file = global_scase.plot3dinfo[i].bound_file;
  }
  return bound_file;
}

/* ------------------ BoundsBnd2Gbnd ------------------------ */

int BoundsBnd2Gbnd(int file_type){
  int i, ninfo;
  FILE *stream = NULL;

  ninfo = GetNinfo(file_type);
  for(i = 0; i < ninfo; i++){
    float valmins[MAXPLOT3DVARS], valmaxs[MAXPLOT3DVARS];
    char *reg_file, *bound_file;
    int nbounds;

    reg_file = GetRegFile(file_type, i);
    bound_file = GetBoundFile(file_type, i);
    nbounds = GetNBounds(file_type);
    if(GetFileBounds(bound_file, nbounds, valmins, valmaxs) == 1){
      char label[256];
      int j;

      if(stream == NULL){
        stream = FopenGbndFile(file_type, "w");
        if(stream == NULL)return 0;
      }
      fprintf(stream, "%s ", reg_file);
      for(j = 0; j < nbounds; j++){
        fprintf(stream, " %g %g ", valmins[j], valmaxs[j]);
      }
      if(GetBoundLabel(file_type, i, label) != NULL)fprintf(stream, "! %s", label);
      fprintf(stream, "\n");
    }
  }
  if(stream != NULL)fclose(stream);
  return 1;
}

/* ------------------ BoundsUpdateSetup ------------------------ */

void BoundsUpdateSetup(int file_type){
  int i;
  FILE *stream = NULL;

  int ninfo;
  globalboundsdata *globalboundsinfo;
  char **sorted_filenames;

  ninfo = GetNinfo(file_type);
  if(file_type == BOUND_PLOT3D){
    FREEMEMORY(plot3dglobalboundsinfo);
    FREEMEMORY(sorted_plot3d_filenames);
    globalboundsinfo = NULL;
    sorted_filenames = NULL;
  }
  else{
    globalboundsinfo = GetGlobalBoundsinfo(file_type);
    sorted_filenames = GetSortedFilenames(file_type);
  }

  if(sorted_filenames == NULL){
    NewMemory(( void ** )&sorted_filenames, ninfo * sizeof(char *));
    for(i = 0; i < ninfo; i++){
      sorted_filenames[i] = GetRegFile(file_type, i);
    }
    qsort(( char * )sorted_filenames, ninfo, sizeof(char *), CompareBoundFileName);
    SaveSortedFilenames(file_type, sorted_filenames);
  }
  if(globalboundsinfo == NULL){
    NewMemory(( void ** )&globalboundsinfo, ninfo * sizeof(globalboundsdata));
    for(i = 0; i < ninfo; i++){
      globalboundsinfo[i].file = sorted_filenames[i];
      globalboundsinfo[i].defined = 0;
    }
    SaveGlobalBoundsinfo(file_type, globalboundsinfo);
  }
  DefineGbndFilename(file_type);
  if(no_bounds == 0 || force_bounds == 1)BoundsBnd2Gbnd(file_type);
  stream = FopenGbndFile(file_type, "r");
  if(stream != NULL){
    for(;;){
      char buffer[255], file[255], *fileptr, **key_index;
      float valmins[MAXPLOT3DVARS], valmaxs[MAXPLOT3DVARS];
      globalboundsdata *fi;

      if(fgets(buffer, 255, stream) == NULL)break;
      if(file_type == BOUND_PLOT3D){
        sscanf(buffer, "%s %f %f %f %f %f %f %f %f %f %f %f %f ", file,
          valmins + 0, valmaxs + 0,
          valmins + 1, valmaxs + 1,
          valmins + 2, valmaxs + 2,
          valmins + 3, valmaxs + 3,
          valmins + 4, valmaxs + 4,
          valmins + 5, valmaxs + 5
        );
      }
      else{
        sscanf(buffer, "%s %f %f ", file, valmins + 0, valmaxs + 0);
      }
      fileptr = TrimFrontBack(file);
      key_index = ( char ** )bsearch(( char * )&fileptr, sorted_filenames, ninfo, sizeof(char *), CompareBoundFileName);
      if(key_index != NULL){
        int index;

        index = ( int )(key_index - sorted_filenames);
        fi = globalboundsinfo + index;
        fi->nbounds = 1;
        if(file_type == BOUND_PLOT3D){
          fi->nbounds = 5;
          if(global_scase.plot3dinfo != NULL)fi->nbounds = global_scase.plot3dinfo->nplot3dvars;
        }
        memcpy(fi->valmins, valmins, fi->nbounds * sizeof(float));
        memcpy(fi->valmaxs, valmaxs, fi->nbounds * sizeof(float));
        memcpy(fi->valmins_save, valmins, fi->nbounds * sizeof(float));
        memcpy(fi->valmaxs_save, valmaxs, fi->nbounds * sizeof(float));
        fi->defined = 1;
      }
    }
    fclose(stream);
    // RmGbndFile(file_type);
    if(no_bounds == 1 && force_bounds == 0){
      assert(FFALSE); // global bounds file shouldn't exist if the no_bounds option was set
    }
  }
}

/* ------------------ BoundsUpdate ------------------------ */

void BoundsUpdate(int file_type){
  char label1[256], label2[256], label3[256];

  if(GetNinfo(file_type) == 0)return;
  strcpy(label1, "BoundsUpdateSetup ");
  strcpy(label2, "BoundsUpdateDoit ");
  strcpy(label3, "BoundsUpdateWrapup ");
  if(file_type == BOUND_SLICE){
    strcat(label1, "SLICE");
    strcat(label2, "SLICE");
    strcat(label3, "SLICE");
#ifdef pp_BOUND_DEBUG
    fprintf(stderr, "BoundsUpdate(SLICE)\n");
#endif
  }
  else if(file_type == BOUND_PATCH){
    strcat(label1, "PATCH");
    strcat(label2, "PATCH");
    strcat(label3, "PATCH");
#ifdef pp_BOUND_DEBUG
    fprintf(stderr, "BoundsUpdate(PATCH)\n");
#endif
  }
  else if(file_type == BOUND_PLOT3D){
    strcat(label1, "PLOT3D");
    strcat(label2, "PLOT3D");
    strcat(label3, "PLOT3D");
#ifdef pp_BOUND_DEBUG
    fprintf(stderr, "BoundsUpdate(PLOT3D)\n");
#endif
  }
  INIT_PRINT_TIMER(bound_setup);
  BoundsUpdateSetup(file_type);
  PRINT_TIMER(bound_setup, label1);
  INIT_PRINT_TIMER(bound_doit);
  BoundsUpdateDoit(file_type);
  PRINT_TIMER(bound_doit, label2);
  INIT_PRINT_TIMER(bound_wrapup);
  BoundsUpdateWrapup(file_type);
  PRINT_TIMER(bound_wrapup, label3);
}

/* ------------------ GetGlobalPatchBounds ------------------------ */

void GetGlobalPatchBounds(int flag, int set_flag, char *label){
  int i;

  if(global_scase.npatchinfo==0)return;
  if(no_bounds == 1 && force_bounds==0)flag = 0;
  for(i = 0; i < npatchbounds; i++){
    boundsdata *boundi;

    boundi = patchbounds + i;
    boundi->dlg_global_valmin = 1.0;
    boundi->dlg_global_valmax = 0.0;
  }
  if(flag==1)BoundsUpdate(BOUND_PATCH);
  for(i = 0; i < global_scase.npatchinfo; i++){
    patchdata *patchi;
    float valmin, valmax;
    boundsdata *boundi;
    int doit;

    patchi = global_scase.patchinfo + i;

    doit = 0;
    if(patchi->valmin_patch > patchi->valmax_patch ||
      current_script_command == NULL || NOT_LOADRENDER)doit = 1;
    if(flag == 0){
      doit = 0;
      patchi->valmin_patch = 0.0;
      patchi->valmax_patch = 1.0;
    }
    if(force_bound_update == 1)doit = 1;
    if(label != NULL && strcmp(patchi->label.shortlabel, label) != 0)doit = 0;
    if(doit==1){
#ifdef pp_BOUNDFRAME
      if(patchi->frameinfo != NULL){
        valmin = patchi->frameinfo->valmin;
        valmax = patchi->frameinfo->valmax;
      }
      else{
        BoundsGet(patchi->reg_file, patchglobalboundsinfo, sorted_patch_filenames, global_scase.npatchinfo, 1, &valmin, &valmax);
      }
#else
      BoundsGet(patchi->reg_file, patchglobalboundsinfo, sorted_patch_filenames, global_scase.npatchinfo, 1, &valmin, &valmax);
#endif
      if(valmin > valmax)continue;
      patchi->valmin_patch = valmin;
      patchi->valmax_patch = valmax;
      patch_bounds_defined = 1;
    }
    else{
      valmin = patchi->valmin_patch;
      valmax = patchi->valmax_patch;
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
    if(label != NULL && strcmp(boundi->label->shortlabel, label) != 0)continue;
    boundi->dlg_valmin = boundi->dlg_global_valmin;
    boundi->dlg_valmax = boundi->dlg_global_valmax;
    for(j = 0; j < global_scase.npatchinfo; j++){
      patchdata *patchj;

      patchj = global_scase.patchinfo + j;
      if(strcmp(patchj->label.shortlabel, boundi->shortlabel) == 0){
        patchj->valmin_glui = boundi->dlg_global_valmin;
        patchj->valmax_glui = boundi->dlg_global_valmax;
      }
    }
  }

  npatchbounds_cpp = npatchbounds;
  if(npatchbounds_cpp>0){
    if(patchbounds_cpp == NULL){
      NewMemory((void **)&patchbounds_cpp, npatchbounds_cpp * sizeof(cpp_boundsdata));
    }
    for(i = 0; i<npatchbounds_cpp; i++){
      cpp_boundsdata *boundscppi;
      boundsdata *boundi;

      boundscppi = patchbounds_cpp+i;
      boundi = patchbounds+i;
      if(label != NULL && strcmp(boundi->label->shortlabel, label) != 0)continue;
      strcpy(boundscppi->label, boundi->shortlabel);
      strcpy(boundscppi->unit, boundi->label->unit);

      boundscppi->cache = cache_boundary_data;
      boundscppi->set_valtype = 0;

      if(set_flag==1)boundscppi->set_valmin = 0;
      boundscppi->valmin[BOUND_SET_MIN]        = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_LOADED_MIN]     = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_GLOBAL_MIN]     = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = boundi->dlg_global_valmin;

      boundscppi->valmax[BOUND_SET_MAX]        = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_LOADED_MAX]     = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_GLOBAL_MAX]     = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = boundi->dlg_global_valmax;

      if(bound_patch_init == 1){
        boundscppi->set_chopmin = 0;
        boundscppi->set_chopmax = 0;
        boundscppi->chopmin     = 0.0;
        boundscppi->chopmax     = 0.0;
      }

      if(set_flag==1)boundscppi->set_valmax = 0;
      boundscppi->hist = NULL;
    }
  }
  bound_patch_init = 0;
}

/* ------------------ GetGlobalPatchBoundsFull ------------------------ */

void *GetGlobalPatchBoundsFull(void *arg){
  THREADcontrol(patchbound_threads, THREAD_LOCK);
  GetGlobalPatchBounds(1,SET_MINMAX_FLAG,NULL);
  THREADcontrol(patchbound_threads, THREAD_UNLOCK);
  THREAD_EXIT(patchbound_threads);
}

/* ------------------ GetGlobalPatchBoundsReduced ------------------------ */

void GetGlobalPatchBoundsReduced(void){
  GetGlobalPatchBounds(0,SET_MINMAX_FLAG,NULL);
}

/* ------------------ GetPlot3DFileBounds ------------------------ */

int GetPlot3DFileBounds(char *file, float *valmin, float *valmax){
  FILE *stream;
  int i, compute_bounds = 0;
  char buffer[255];

  for(i = 0; i<MAXPLOT3DVARS; i++){
    if(valmin[i]>valmax[i]){
      compute_bounds = 1;
      break;
    }
  }
  if(compute_bounds==0)return 1;
  if(file==NULL||strlen(file)==0)return 0;
  stream = FOPEN_2DIR(file, "r");
  if(stream==NULL)return 0;

  CheckMemory;

  for(i = 0; i<MAXPLOT3DVARS; i++){
    if(fgets(buffer, 255, stream)==NULL)break;
    sscanf(buffer, "%f %f", valmin+i, valmax+i);
  }
  fclose(stream);
  return 1;
}

/* ------------------ GetGlobalPlot3DBounds ------------------------ */

void GetGlobalPlot3DBounds(void){
  int i;

  if(global_scase.nplot3dinfo <= 0)return;
  if(no_bounds==0 || force_bounds==1)BoundsUpdate(BOUND_PLOT3D);
  for(i = 0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo+i;
    plot3di->have_bound_file = BoundsGet(plot3di->reg_file, plot3dglobalboundsinfo, sorted_plot3d_filenames, global_scase.nplot3dinfo, plot3di->nplot3dvars, plot3di->valmin_plot3d, plot3di->valmax_plot3d);
  }
  for(i = 0; i<MAXPLOT3DVARS; i++){
    p3min_all[i] = 1.0;
    p3max_all[i] = 0.0;
  }
  for(i = 0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;
    int j;
    float *valmin_fds, *valmax_fds;

    plot3di = global_scase.plot3dinfo+i;
    if(plot3di->have_bound_file == 0)continue;
    valmin_fds = plot3di->valmin_plot3d;
    valmax_fds = plot3di->valmax_plot3d;
    for(j = 0; j<MAXPLOT3DVARS; j++){
      if(valmin_fds[j]<=valmax_fds[j]){
        if(p3min_all[j]>p3max_all[j]){
          p3min_all[j] = valmin_fds[j];
          p3max_all[j] = valmax_fds[j];
        }
        else{
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
  if(global_scase.nplot3dinfo>0&&plot3dbounds_cpp==NULL){ // only initialize once
    nplot3dbounds_cpp = global_scase.plot3dinfo[0].nplot3dvars;
    NewMemory((void **)&plot3dbounds_cpp, nplot3dbounds_cpp*sizeof(cpp_boundsdata));
    for(i = 0; i<nplot3dbounds_cpp; i++){
      cpp_boundsdata *boundscppi;

      boundscppi = plot3dbounds_cpp+i;
      strcpy(boundscppi->label, global_scase.plot3dinfo->label[i].shortlabel);
      strcpy(boundscppi->unit, global_scase.plot3dinfo->label[i].unit);

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
    }
  }
  GLUISetGlobalMinMaxAll(BOUND_PLOT3D, p3min_global, p3max_global, global_scase.plot3dinfo->nplot3dvars);
}

/* ------------------ GetLoadedPlot3dBounds ------------------------ */

void GetLoadedPlot3dBounds(int *compute_loaded, float *loaded_min, float *loaded_max){
  int i;
  int plot3d_loaded = 0;

#define BOUNDS_LOADED 1
  for(i = 0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;

    plot3di = global_scase.plot3dinfo+i;
    if(plot3di->loaded==0)continue;
    plot3d_loaded = 1;
    break;
  }
  if(plot3d_loaded==0){
    printf("***loaded plot3d bounds not available, using global bounds\n");
    for(i = 0; i< MAXPLOT3DVARS; i++){
      loaded_min[i] = p3min_global[i];
      loaded_max[i] = p3max_global[i];
    }
    return;
  }
  for(i = 0; i< MAXPLOT3DVARS; i++){
    if(compute_loaded!=NULL&&compute_loaded[i]!=BOUNDS_LOADED)continue;
    loaded_min[i] = 1.0;
    loaded_max[i] = 0.0;
  }
  for(i = 0; i<global_scase.nplot3dinfo; i++){
    plot3ddata *plot3di;
    int j;

    plot3di = global_scase.plot3dinfo+i;
    if(plot3di->loaded==0)continue;
    for(j = 0; j< MAXPLOT3DVARS; j++){
      if(compute_loaded!=NULL&&compute_loaded[j]!=BOUNDS_LOADED)continue;
      if(loaded_min[j]>loaded_max[j]){
        loaded_min[j] = plot3di->valmin_plot3d[j];
        loaded_max[j] = plot3di->valmax_plot3d[j];
      }
      else{
        loaded_min[j] = MIN(plot3di->valmin_plot3d[j], loaded_min[j]);
        loaded_max[j] = MAX(plot3di->valmax_plot3d[j], loaded_max[j]);
      }
    }
  }
}

/* ------------------ RmGbndFile ------------------------ */

void RmGbndFile(int file_type){

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    UNLINK(slice_gbnd_filename);
  }
  else if(file_type==BOUND_PATCH){
    UNLINK(patch_gbnd_filename);
  }
  else if(file_type == BOUND_PLOT3D){
    UNLINK(plot3d_gbnd_filename);
  }
}

/* ------------------ GetGbndFilename ------------------------ */

char *GetGbndFilename(int file_type){
  char *filename=NULL;

  ASSERT_BOUND_TYPE;
  if(file_type == BOUND_SLICE){
    filename = slice_gbnd_filename;
  }
  else if(file_type==BOUND_PATCH){
    filename = patch_gbnd_filename;\
  }
  else if(file_type == BOUND_PLOT3D){
    filename = plot3d_gbnd_filename;\
  }
  return filename;
}

/* ------------------ GetSliceBoundsInfo ------------------------ */

boundsdata *GetSliceBoundsInfo(char *shortlabel){
  int i;

  for(i = 0; i < nslicebounds; i++){
    boundsdata *boundi;

    boundi = slicebounds + i;
    if(strcmp(boundi->shortlabel, shortlabel) == 0)return boundi;
  }
  return NULL;
}

/* ------------------ GetGlobalSliceBounds ------------------------ */

void GetGlobalSliceBounds(int flag, int set_flag, char *label){
  int i;

  if(no_bounds == 1 && force_bounds==0)flag = 0;
  if(global_scase.slicecoll.nsliceinfo==0)return;
  for(i = 0;i<nslicebounds;i++){
    boundsdata *boundi;

    boundi = slicebounds+i;
    boundi->dlg_global_valmin = 1.0;
    boundi->dlg_global_valmax = 0.0;
  }
  if(no_bounds==0 || force_bounds==1)BoundsUpdate(BOUND_SLICE);
  INIT_PRINT_TIMER(slicebounds_timer);
  for(i = 0;i<global_scase.slicecoll.nsliceinfo;i++){
    slicedata *slicei;
    float valmin, valmax;
    boundsdata *boundi;
    int doit;

    slicei = global_scase.slicecoll.sliceinfo+i;
    if(label != NULL && strcmp(slicei->label.shortlabel, label) != 0)continue;
    if(slicei->valmin_slice>slicei->valmax_slice ||
       current_script_command==NULL || NOT_LOADRENDER)doit=1;
    if(flag==0){
       doit = 0;
       slicei->valmin_slice = 0.0;
       slicei->valmax_slice = 1.0;
    }
    if(force_bound_update == 1||global_scase.nzoneinfo>0)doit = 1;

    if(doit==1){
      BoundsGet(slicei->reg_file, sliceglobalboundsinfo, sorted_slice_filenames, global_scase.slicecoll.nsliceinfo, 1, &valmin, &valmax);
      if(valmin>valmax)continue;
      slicei->valmin_slice = valmin;
      slicei->valmax_slice = valmax;
      slice_bounds_defined = 1;
    }
    else{
      valmin = slicei->valmin_slice;
      valmax = slicei->valmax_slice;
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
    if(label!=NULL&&strcmp(boundi->label->shortlabel, label) != 0)continue;
    boundi->dlg_valmin = boundi->dlg_global_valmin;
    boundi->dlg_valmax = boundi->dlg_global_valmax;
  }
  nslicebounds_cpp = nslicebounds;
  if(nslicebounds_cpp>0){
    if(slicebounds_cpp==NULL){
      NewMemory((void **)&slicebounds_cpp, nslicebounds_cpp*sizeof(cpp_boundsdata));
    }
    for(i = 0; i<nslicebounds_cpp; i++){
      cpp_boundsdata *boundscppi;
      boundsdata *boundi;

      boundscppi = slicebounds_cpp + i;
      boundi     = slicebounds + i;
      if(label != NULL && strcmp(boundi->label->shortlabel, label) != 0)continue;
      strcpy(boundscppi->label, boundi->shortlabel);
      strcpy(boundscppi->unit, boundi->label->unit);

      boundscppi->cache = cache_slice_data;
      boundscppi->set_valtype = 0;

      if(set_flag == 1)boundscppi->set_valmin = 0;
      boundscppi->valmin[BOUND_SET_MIN]        = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_LOADED_MIN]     = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_GLOBAL_MIN]     = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = boundi->dlg_global_valmin;

      if(set_flag == 1)boundscppi->set_valmax = 0;
      boundscppi->valmax[BOUND_SET_MAX]        = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_LOADED_MAX]     = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_GLOBAL_MAX]     = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = boundi->dlg_global_valmax;
      boundscppi->hist = NULL;

      if(bound_slice_init == 1){
        boundscppi->chop_hide = 0;
        boundscppi->set_chopmin = 0;
        boundscppi->set_chopmax = 0;
        boundscppi->chopmin = 0.0;
        boundscppi->chopmax = 0.0;
      }
    }
  }
  bound_slice_init = 0;
  PRINT_TIMER(slicebounds_timer, "GlobalSliceBounds (not BoundsUpdate)");
}

/* ------------------ GetGlobalSliceBoundsFull ------------------------ */

void *GetGlobalSliceBoundsFull(void *arg){
  THREADcontrol(slicebound_threads, THREAD_LOCK);
  GetGlobalSliceBounds(1,SET_MINMAX_FLAG,NULL);
  THREADcontrol(slicebound_threads, THREAD_UNLOCK);
  THREAD_EXIT(slicebound_threads);
}

/* ------------------ GetGlobalSliceBoundsReduced ------------------------ */

void GetGlobalSliceBoundsReduced(void){
  GetGlobalSliceBounds(0,SET_MINMAX_FLAG,NULL);
}

/* ------------------ GetHVACDuctBounds ------------------------ */

void GetHVACDuctBounds(char *shortlabel, float *valminptr, float *valmaxptr){
  float valmin = 1.0, valmax = 0.0;
  int i;

  *valminptr = 1.0;
  *valmaxptr = 0.0;
  for(i=0;i< global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;i++){
    hvacvaldata *hi;

    hi = global_scase.hvaccoll.hvacductvalsinfo->duct_vars + i;
    if(strcmp(shortlabel, hi->label.shortlabel)!=0)continue;
    if(valmin<valmax){
      valmin = MIN(valmin,hi->valmin);
      valmax = MAX(valmax,hi->valmax);
    }
    else{
      valmin = hi->valmin;
      valmax = hi->valmax;
    }
  }
  *valminptr = valmin;
  *valmaxptr = valmax;
}

/* ------------------ GetHVACNodeBounds ------------------------ */

void GetHVACNodeBounds(char *shortlabel, float *valminptr, float *valmaxptr){
  float valmin = 1.0, valmax = 0.0;
  int i;

  *valminptr = 1.0;
  *valmaxptr = 0.0;
  for(i = 0;i < global_scase.hvaccoll.hvacnodevalsinfo->n_node_vars;i++){
    hvacvaldata *hi;

    hi = global_scase.hvaccoll.hvacnodevalsinfo->node_vars + i;
    if(strcmp(shortlabel, hi->label.shortlabel) != 0)continue;
    if(valmin < valmax){
      valmin = MIN(valmin, hi->valmin);
      valmax = MAX(valmax, hi->valmax);
    }
    else{
      valmin = hi->valmin;
      valmax = hi->valmax;
    }
  }
  *valminptr = valmin;
  *valmaxptr = valmax;
}

/* ------------------ GetGlobalHVACDuctBounds ------------------------ */

void GetGlobalHVACDuctBounds(int flag){
  int i;

  if(no_bounds == 1 && force_bounds==0)flag = 0;
  int nhvacboundsmax = 0;
  if(global_scase.hvaccoll.hvacductvalsinfo != NULL)nhvacboundsmax = global_scase.hvaccoll.hvacductvalsinfo->n_duct_vars;
  if(nhvacboundsmax == 0)return;
  if(flag==0)ReadHVACData(BOUNDS_ONLY);
  for(i = 0;i < nhvacductbounds;i++){
    boundsdata *boundi;
    float valmin, valmax;

    boundi = hvacductbounds + i;
    boundi->dlg_global_valmin = 1.0;
    boundi->dlg_global_valmax = 0.0;
    GetHVACDuctBounds(boundi->label->shortlabel, &valmin, &valmax);
    boundi->dlg_global_valmin = valmin;
    boundi->dlg_global_valmax = valmax;
    boundi->dlg_valmin = boundi->dlg_global_valmin;
    boundi->dlg_valmax = boundi->dlg_global_valmax;
  }
  nhvacductbounds_cpp = nhvacductbounds;
  if(nhvacductbounds_cpp > 0 && hvacductbounds_cpp == NULL){ // only initialize once
    NewMemory((void **)&hvacductbounds_cpp, nhvacductbounds_cpp * sizeof(cpp_boundsdata));
    for(i = 0; i < nhvacductbounds_cpp; i++){
      cpp_boundsdata *boundscppi;
      boundsdata *boundi;

      boundscppi = hvacductbounds_cpp + i;
      boundi = hvacductbounds + i;
      strcpy(boundscppi->label, boundi->shortlabel);
      strcpy(boundscppi->unit, boundi->label->unit);

      boundscppi->cache = cache_hvac_data;
      boundscppi->set_valtype = 0;

      boundscppi->set_valmin = 0;
      boundscppi->valmin[BOUND_SET_MIN] = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_LOADED_MIN] = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_GLOBAL_MIN] = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = boundi->dlg_global_valmin;

      boundscppi->set_valmax = 0;
      boundscppi->valmax[BOUND_SET_MAX] = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_LOADED_MAX] = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_GLOBAL_MAX] = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = boundi->dlg_global_valmax;

      if(bound_hvacduct_init == 1){
        boundscppi->set_chopmin = 0;
        boundscppi->set_chopmax = 0;
        boundscppi->chopmin     = 0.0;
        boundscppi->chopmax     = 0.0;
      }
    }
  }
  bound_hvacduct_init = 0;
}

/* ------------------ GetGlobalHVACNodeBounds ------------------------ */

void GetGlobalHVACNodeBounds(int flag){
  int i;

  if(no_bounds == 1 && force_bounds==0)flag = 0;
  int nhvacboundsmax = 0;
  if(global_scase.hvaccoll.hvacnodevalsinfo != NULL)nhvacboundsmax = global_scase.hvaccoll.hvacnodevalsinfo->n_duct_vars + global_scase.hvaccoll.hvacnodevalsinfo->n_node_vars;
  if(nhvacboundsmax == 0)return;
  if(flag == 0)ReadHVACData(BOUNDS_ONLY);
  for(i = 0;i < nhvacnodebounds;i++){
    boundsdata *boundi;
    float valmin, valmax;

    boundi = hvacnodebounds + i;
    boundi->dlg_global_valmin = 1.0;
    boundi->dlg_global_valmax = 0.0;
    GetHVACNodeBounds(boundi->label->shortlabel, &valmin, &valmax);
    boundi->dlg_global_valmin = valmin;
    boundi->dlg_global_valmax = valmax;
    boundi->dlg_valmin = boundi->dlg_global_valmin;
    boundi->dlg_valmax = boundi->dlg_global_valmax;
  }
  nhvacnodebounds_cpp = nhvacnodebounds;
  if(nhvacnodebounds_cpp > 0 && hvacnodebounds_cpp == NULL){ // only initialize once
    NewMemory((void **)&hvacnodebounds_cpp, nhvacnodebounds_cpp * sizeof(cpp_boundsdata));
    for(i = 0; i < nhvacnodebounds_cpp; i++){
      cpp_boundsdata *boundscppi;
      boundsdata *boundi;

      boundscppi = hvacnodebounds_cpp + i;
      boundi = hvacnodebounds + i;
      strcpy(boundscppi->label, boundi->shortlabel);
      strcpy(boundscppi->unit, boundi->label->unit);

      boundscppi->cache = cache_hvac_data;
      boundscppi->set_valtype = 0;

      boundscppi->set_valmin = 0;
      boundscppi->valmin[BOUND_SET_MIN] = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_LOADED_MIN] = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_GLOBAL_MIN] = boundi->dlg_global_valmin;
      boundscppi->valmin[BOUND_PERCENTILE_MIN] = boundi->dlg_global_valmin;

      boundscppi->set_valmax = 0;
      boundscppi->valmax[BOUND_SET_MAX] = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_LOADED_MAX] = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_GLOBAL_MAX] = boundi->dlg_global_valmax;
      boundscppi->valmax[BOUND_PERCENTILE_MAX] = boundi->dlg_global_valmax;

      if(bound_hvacnode_init == 1){
        boundscppi->set_chopmin = 0;
        boundscppi->set_chopmax = 0;
        boundscppi->chopmin     = 0.0;
        boundscppi->chopmax     = 0.0;
      }
    }
  }
  bound_hvacnode_init = 0;
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

  parti->bounds_set = 0;
  if(globalmin_part==NULL){
    NewMemory((void **)&globalmin_part, npart5prop*sizeof(float));
    for(j = 0; j<npart5prop; j++){
      globalmin_part[j] =  1000000000.0;
    }
  }
  if(globalmax_part==NULL){
    NewMemory((void **)&globalmax_part, npart5prop*sizeof(float));
    for(j = 0; j<npart5prop; j++){
      globalmax_part[j] = -1000000000.0;
    }
  }
#ifndef pp_PARTFRAME

  // make sure a size file exists

  if(parti->size_file!=NULL)stream = FOPEN_2DIR(parti->size_file, "r");
  if(stream==NULL){
    CreatePartSizeFile(parti);
    if(parti->size_file!=NULL)stream = FOPEN_2DIR(parti->size_file, "r");
    if(stream==NULL)return 0;
  }
  if(stream!=NULL){
    fclose(stream);
    stream = NULL;
  }
#endif

  // make sure a bound file exists

  if(parti->bound_file!=NULL)stream = FOPEN_2DIR(parti->bound_file, "r");
  if(parti->bound_file==NULL||stream==NULL){
    CreatePartBoundFile(parti);
    stream = FOPEN_2DIR(parti->bound_file, "r");
    if(stream==NULL)return 0;
  }
  if(stream!=NULL){
    fclose(stream);
    stream = NULL;
  }

// if valid global particle bounds file so we can return now and not read individual bound files

  if(read_bounds_arg==1){
    parti->bounds_set = 1;
    fclose(stream);
    return 0;
  }

  stream = FOPEN_2DIR(parti->bound_file, "r");
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

          globalmin_part[prop_index_local] = MIN(globalmin_part[prop_index_local], vmin_local);
          globalmax_part[prop_index_local] = MAX(globalmax_part[prop_index_local], vmax_local);
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

  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;
    int j;

    parti = global_scase.partinfo+i;
    if(parti->bounds_set==0)continue;
    for(j = 0; j<npart5prop; j++){
      partpropdata *propj;

      propj = part5propinfo+j;
      if(strcmp(propj->label->shortlabel, "Uniform")==0)continue;
      propj->dlg_global_valmin = MIN(propj->dlg_global_valmin, globalmin_part[j]);
      propj->dlg_global_valmax = MAX(propj->dlg_global_valmax, globalmax_part[j]);
    }
  }
  if(global_have_global_bound_file==0){
    FILE *stream;

    stream = FOPEN_2DIR(part_globalbound_filename, "w");
    if(stream!=NULL){
      global_have_global_bound_file = 1;
      global_part_boundsize = GetFileSizeSMV(global_scase.partinfo->bound_file);
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
  for(j = 0; j<global_scase.npartinfo; j++){
    partdata *partj;

    partj = global_scase.partinfo+j;
    if(type_arg==PART_SIZING&&partj->boundstatus==PART_BOUND_COMPUTING)nsize_local++;
    if(type_arg==PART_LOADING&&partj->loadstatus==FILE_LOADING)nsize_local++;
  }
  if(option_arg==1||(nsize_local<n_partload_threads&&nsize_local>0)){
    int isize_local;

    if(type_arg==PART_LOADING)printf("loading: ");
    isize_local = 0;
    for(j = 0; j<global_scase.npartinfo; j++){
      partdata *partj;
      int doit;

      partj = global_scase.partinfo+j;
      doit = 0;
      if(type_arg==PART_LOADING&&partj->loadstatus==FILE_LOADING)doit = 1;
      if(doit==1){
        printf("%s", partj->reg_file);
        if(isize_local!=nsize_local-1)printf(", ");
        isize_local++;
      }
    }
    printf("\n");
  }
}

/* ------------------ GetAllPartBounds ------------------------ */

void GetAllPartBounds(void){
  int i;
  FILE *stream = NULL;

  THREADcontrol(partload_threads, THREAD_LOCK);
  if(globalmin_part == NULL){
    NewMemory((void **)&globalmin_part, npart5prop*sizeof(float));
  }
  if(globalmax_part == NULL){
    NewMemory((void **)&globalmax_part, npart5prop*sizeof(float));
  }

  // find min/max for each particle file

  if(global_part_boundsize==0)global_part_boundsize = GetFileSizeSMV(global_scase.partinfo->bound_file);

  stream = FOPEN_2DIR(part_globalbound_filename, "r");
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
      for(i = 0; i<global_scase.npartinfo; i++){
        partdata *parti;
        parti              = global_scase.partinfo+i;
        parti->boundstatus = PART_BOUND_DEFINED;
        parti->bounds_set  = 1;
      }
      for(i = 0; i<npart5prop; i++){
        partpropdata *propi;

        propi = part5propinfo+i;
        globalmin_part[i] = propi->dlg_global_valmin;
        globalmax_part[i] = propi->dlg_global_valmax;
      }
      THREADcontrol(partload_threads, THREAD_UNLOCK);
      return;
    }
    fclose(stream);
  }
  THREADcontrol(partload_threads, THREAD_UNLOCK);

  for(i = 0; i<global_scase.npartinfo; i++){
    partdata *parti;

    parti = global_scase.partinfo+i;
    THREADcontrol(partload_threads, THREAD_LOCK);
    if(parti->boundstatus!=PART_BOUND_UNDEFINED){
      THREADcontrol(partload_threads, THREAD_UNLOCK);
      continue;
    }
    parti->boundstatus = PART_BOUND_COMPUTING;
    THREADcontrol(partload_threads, THREAD_UNLOCK);
    ReadPartBounds(parti,global_have_global_bound_file);
    THREADcontrol(partload_threads, THREAD_LOCK);
    parti->boundstatus = PART_BOUND_DEFINED;
    THREADcontrol(partload_threads, THREAD_UNLOCK);
  }
}
