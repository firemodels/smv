#include "options.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "smokeviewvars.h"

/* ------------------ GetGlobalSliceBounds ------------------------ */

void GetGlobalSliceBounds(void){
  int j;

  for(j = 0;j<nslicebounds;j++){
    boundsdata *boundi;

    boundi = slicebounds+j;
    boundi->global_valmin = 1.0;
    boundi->global_valmax = 0.0;
  }
  for(j = 0;j<nsliceinfo;j++){
    slicedata *slicej;
    int i, count;
    float valmin, valmax;

    slicej = sliceinfo+j;
    if(slicej->firstshort_slice!=1)continue;
    count = 0;
    for(i = 0;i<nsliceinfo;i++){
      slicedata *slicei;
      FILE *stream;

      slicei = sliceinfo+i;
      if(strcmp(slicei->label.shortlabel, slicej->label.shortlabel)!=0)continue;
      stream = fopen(slicei->bound_file, "r");
      if(stream==NULL)continue;
      while(!feof(stream)){
        char buffer[255];
        float t, vmin, vmax;

        if(fgets(buffer, 255, stream)==NULL)break;
        sscanf(buffer, " %f %f %f", &t, &vmin, &vmax);
        if(count==0){
          valmin = vmin;
          valmax = vmax;
        }
        else{
          valmin = MIN(valmin, vmin);
          valmax = MAX(valmax, vmax);
        }
        count++;
      }
      fclose(stream);
    }
    if(count>0){
      int bound_index;

      bound_index = GetSliceBoundsIndex(slicej);
      if(bound_index>=0){
        boundsdata *boundi;

        boundi = slicebounds+bound_index;
        boundi->global_valmin = valmin;
        boundi->global_valmax = valmax;

      }
    }
  }
}

/* ------------------ AdjustDataBounds ------------------------ */

void AdjustDataBounds(const float *pdata, int local_skip, int ndata,
                   int setpmin, float *pmin, int setpmax, float *pmax)
{
    int nsmall, nbig, *buckets=NULL, n, level, total, alpha05;
    float dp, pmin2, pmax2;

    if(setpmin==PERCENTILE_MIN||setpmax==PERCENTILE_MAX){
      dp = (*pmax - *pmin)/NBUCKETS;
      nsmall=0;
      nbig=NBUCKETS;
      if(NewMemory((void **)&buckets,NBUCKETS*sizeof(int))==0){
        fprintf(stderr,"*** Error: Unable to allocate memory in getdatabounds\n");
        return;
      }

      for(n=0;n<NBUCKETS;n++){
        buckets[n]=0;
      }
      for(n=local_skip;n<ndata;n++){
        level=0;
        if(dp!=0.0f)level = CLAMP((int)((pdata[n] - *pmin)/dp),0,NBUCKETS-1);
        buckets[level]++;
      }
      alpha05 = (int)(percentile_level*ndata);
      total = 0;
      for(n=0;n<NBUCKETS;n++){
        total += buckets[n];
        if(total>alpha05){
          nsmall=n;
          break;
        }
      }
      total = 0;
      for(n=NBUCKETS;n>0;n--){
        total += buckets[n-1];
        if(total>alpha05){
          nbig=n;
          break;
        }
      }
      pmin2 = *pmin + (nsmall-1)*dp;
      pmax2 = *pmin + (nbig+1)*dp;
      if(setpmin==PERCENTILE_MIN)*pmin = pmin2;
      if(setpmax==PERCENTILE_MAX)*pmax = pmax2;
      FreeMemory(buckets);
    }
    if(axislabels_smooth==1){
      SmoothLabel(pmin,pmax,nrgb);
    }
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
    propi->global_min =  1000000000.0;
    propi->global_max = -1000000000.0;
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
      propj->global_min = MIN(propj->global_min, parti->global_min[j]);
      propj->global_max = MAX(propj->global_max, parti->global_max[j]);
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
        valmin = propi->global_min;
        valmax = propi->global_max;
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

    if(type_arg==PART_SIZING)printf("sizing: ");
    if(type_arg==PART_LOADING)printf("loading: ");
    isize_local = 0;
    for(j = 0; j<npartinfo; j++){
      partdata *partj;
      int doit;

      partj = partinfo+j;
      doit = 0;
      if(type_arg==PART_SIZING&&partj->boundstatus==PART_BOUND_COMPUTING)doit = 1;
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

/* ------------------ GetPartBounds ------------------------ */

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
        propi->global_min = valmin;
        propi->global_max = valmax;
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
          parti->global_min[j] = propj->global_min;
          parti->global_max[j] = propj->global_max;
        }
      }
      UNLOCK_PART_LOAD;
      return;
    }
    else{
      printf("***warning: particle bound files have changed - re-generating global particle bound file\n");
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
    PrintPartLoadSummary(PART_BEFORE, PART_SIZING);
    UNLOCK_PART_LOAD;
    ReadPartBounds(parti,global_have_global_bound_file);
    LOCK_PART_LOAD;
    if(npartinfo>1)PrintPartLoadSummary(PART_AFTER, PART_SIZING);
    parti->boundstatus = PART_BOUND_DEFINED;
    UNLOCK_PART_LOAD;
  }
}

/* ------------------ AdjustPlot3DBounds ------------------------ */

void AdjustPlot3DBounds(int plot3dvar, int setpmin, float *pmin, int setpmax, float *pmax)
{
    int nsmall, nbig, *buckets=NULL, n, level, total, alpha05;
    float dp, pmin2, pmax2;
    plot3ddata *p;
    meshdata *meshi;
    int i;
    char *iblank;
    float *q;
    int ndata=0;
    int ntotal;

    if(setpmin==PERCENTILE_MIN||setpmax==PERCENTILE_MAX){
      dp = (*pmax - *pmin)/NBUCKETS;
      nsmall=0;
      nbig=NBUCKETS;
      if(NewMemory((void **)&buckets,NBUCKETS*sizeof(int))==0){
        fprintf(stderr,"*** Error: Unable to allocate memory in getdatabounds\n");
        return;
      }

      for(n=0;n<NBUCKETS;n++){
        buckets[n]=0;
      }
      for(i=0;i<nplot3dinfo;i++){
        p = plot3dinfo+i;
        if(p->loaded==0||p->display==0)continue;
        meshi=meshinfo+p->blocknumber;
        ntotal=(meshi->ibar+1)*(meshi->jbar+1)*(meshi->kbar+1);
        ndata += ntotal;
        iblank=meshi->c_iblank_node;
        q=meshi->qdata+plot3dvar*ntotal;
        for(n=0;n<ntotal;n++){
          if(iblank==NULL||*iblank++==GAS){
            level=0;
            if(dp!=0.0f)level = CLAMP((int)((q[n] - *pmin)/dp),0,NBUCKETS-1);
            buckets[level]++;
          }
        }
      }
      alpha05 = (int)(percentile_level*ndata);
      total = 0;
      for(n=0;n<NBUCKETS;n++){
        total += buckets[n];
        if(total>alpha05){
          nsmall=n;
          break;
        }
      }
      total = 0;
      for(n=NBUCKETS;n>0;n--){
        total += buckets[n-1];
        if(total>alpha05){
          nbig=n;
          break;
        }
      }
      pmin2 = *pmin + (nsmall-1)*dp;
      pmax2 = *pmin + (nbig+1)*dp;
      if(setpmin==PERCENTILE_MIN)*pmin = pmin2;
      if(setpmax==PERCENTILE_MAX)*pmax = pmax2;
      FreeMemory(buckets);
    }
    if(axislabels_smooth==1&&setpmin!=SET_MIN&&setpmax!=SET_MAX){
      SmoothLabel(pmin,pmax,nrgb);
    }
}

/* ------------------ SmoothLabel ------------------------ */

void SmoothLabel(float *a, float *b, int n){
  double delta, factor, logdelta;
  int ndigits;
  double half;

  half=0.5;

  delta = ((double)*b-(double)*a)/(double)(n-2);
  if(delta==0.0)return;
  logdelta = log10((double)delta);
  ndigits=logdelta-1;
  if(logdelta<=1)ndigits--;
  factor = 5*pow(10,ndigits);
  delta = (int)(delta/factor + half)*factor;

  *a = factor*(int)(*a/factor+half);
  *b = *a + (n-2)*delta;
}
